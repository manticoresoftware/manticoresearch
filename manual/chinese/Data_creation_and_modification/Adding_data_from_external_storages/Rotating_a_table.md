# 旋转表

表旋转是一个过程，其中 searchd 服务器查找配置中定义的表的新版本。旋转仅支持在纯模式下操作。

有两种情况：

* 对于已经加载的纯表
* 配置中添加了表，但尚未加载

在第一种情况下，索引器无法将表的新版本上线，因为正在运行的副本被 `searchd` 锁定并加载。在这种情况下，需要使用带有 [--rotate](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) 参数的 `indexer` 命令。如果使用 rotate，`indexer` 会创建带有 `.new.` 名称的新表文件，并向 `searchd` 发送 *HUP* 信号，通知它有新版本。`searchd` 会执行查找，将新版本的表替换旧版本并丢弃旧版本。在某些情况下，可能希望创建表的新版本，但不立即执行旋转。例如，可能希望先检查新表版本的健康状况。在这种情况下，`indexer` 可以接受 `--nohup` 参数，禁止向服务器发送 HUP 信号。

新表可以通过旋转加载；然而，HUP 信号的常规处理是在服务器启动后配置发生变化时才检查新表。如果表已经在配置中定义，应该先运行不带旋转的 `indexer` 创建表，然后执行 [RELOAD TABLES](../../Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES) 语句。

还有两个专用语句可用于对表执行旋转：

## RELOAD TABLE

```sql
RELOAD TABLE tbl [ FROM '/path/to/table_files' [ OPTION switchover=1 ] ];
```

`RELOAD TABLE` 命令通过 SQL 实现表旋转。

该命令有三种模式。第一种模式，不指定路径时，Manticore 服务器检查由 [path](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#path) 指定目录中的新表文件。新表文件必须命名为 `tbl.new.sp?`。

如果指定路径，服务器将在该目录中搜索表文件，将它们移动到表的 [path](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#path) 目录，将文件名从 `tbl.sp?` 重命名为 `tbl.new.sp?`，然后执行旋转。

第三种模式，通过 `OPTION switchover=1` 激活，将索引切换到新路径。此时，守护进程尝试直接从新路径加载表而不移动文件。如果加载成功，新索引将取代旧索引。

此外，守护进程会在由 [path](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#path) 指定的目录中写入唯一的链接文件（`tbl.link`），以保持持久重定向。

如果将重定向的索引恢复到配置中指定的路径，守护进程会检测到并删除链接文件。

一旦重定向，守护进程将从新链接的路径检索表。旋转时，它会在新重定向的路径查找新表版本。请注意，守护进程会检查配置中的常见错误，如不同表之间路径重复。但它不会识别多个表通过重定向指向同一路径。在正常操作中，表通过 `.spl` 文件锁定，但禁用锁定可能导致问题。如果出现错误（例如路径因任何原因不可访问），应手动修正（或简单删除）链接文件。

`indextool` 会遵循链接文件，但其他工具（`indexer`、`index_converter` 等）不识别链接文件，始终使用配置文件中定义的路径，忽略任何重定向。因此，可以使用 `indextool` 检查索引，它会从新位置读取。但更复杂的操作如合并不会识别任何链接文件。

```sql
mysql> RELOAD TABLE plain_table;
mysql> RELOAD TABLE plain_table FROM '/home/mighty/new_table_files';
mysql> RELOAD TABLE plain_table FROM '/home/mighty/new/place/for/table/table_files' OPTION switchover=1;
```

## RELOAD TABLES

```sql
RELOAD TABLES;
```

该命令的功能类似于 HUP 系统信号，触发表的旋转。然而，它并不完全等同于典型的 HUP 信号（可能来自 `kill -HUP` 命令或 `indexer --rotate`）。该命令主动搜索需要旋转的表，并能够重新读取配置。假设你以纯模式启动 Manticore，配置文件指向一个不存在的纯表。如果随后尝试 `indexer --rotate` 该表，服务器不会识别新表，直到执行 `RELOAD TABLES` 或重启服务器。

根据 [seamless_rotate](../../Server_settings/Searchd.md#seamless_rotate) 设置的值，新查询可能会短暂阻塞，客户端会收到临时错误。

```sql
mysql> RELOAD TABLES;
Query OK, 0 rows affected (0.01 sec)
```

## 无缝旋转

旋转假设旧表版本被丢弃，新表版本被加载并替换现有版本。在此交换过程中，服务器还需要处理针对将被更新的表发起的查询。为避免查询阻塞，服务器默认实现了表的无缝旋转，具体如下。

表可能包含需要预先缓存到内存的数据。目前，`.spa`、`.spb`、`.spi` 和 `.spm` 文件会被完全预缓存（它们分别包含属性数据、blob 属性数据、关键字表和已删除行映射）。如果没有无缝旋转，旋转表时尽量少用内存，工作流程如下：

1. 新查询暂时被拒绝（返回“重试”错误码）。
2. `searchd` 等待所有当前运行的查询完成。
3. 旧表被释放，其文件被重命名。
4. 新表文件被重命名并分配所需内存。
5. 新表的属性和字典数据预加载到内存。
6. `searchd` 恢复从新表提供查询服务。

然而，如果属性或字典数据量很大，预加载步骤可能需要显著时间——预加载 1-5+ GB 文件时可能需要几分钟。

启用无缝旋转后，旋转的工作流程如下：

1. 分配新的表RAM存储。
2. 异步预加载新的表属性和字典数据到RAM。
3. 成功后，释放旧表并重命名两个表的文件。
4. 失败时，释放新表。
5. 在任何时刻，查询要么从旧表副本，要么从新表副本提供服务。

无缝旋转的代价是在旋转期间内存的**峰值**使用量更高（因为在预加载新副本时，旧副本和新副本的`.spa/.spb/.spi/.spm`数据都需要在RAM中）。但是，平均使用量保持不变。

示例：

```ini
seamless_rotate = 1
```
<!-- proofread -->

