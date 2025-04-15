# 旋转表格

表格旋转是一个过程，其中搜索服务器查找配置中定义的表格的新版本。旋转仅在简单模式下支持。

可能有两种情况：

* 对于已经加载的简单表格
* 在配置中添加的，但尚未加载的表格

在第一种情况下，索引器无法将新版本的表格上线，因为正在运行的副本被 `searchd` 锁定和加载。在这种情况下，需要使用 [--rotate](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) 参数调用 `indexer`。如果使用旋转，`indexer` 会创建新表格文件，文件名中包含 `.new.`，并向 `searchd` 发送 *HUP* 信号，告知其新版本。`searchd` 会执行查找并将新版本的表格放置到位，同时丢弃旧的版本。在某些情况下，可能希望创建表格的新版本，但并不希望立即执行旋转。例如，可能希望先检查新表格版本的健康状况。在这种情况下，`indexer` 可以接受 `--nohup` 参数，从而禁止向服务器发送 HUP 信号。

新表格可以通过旋转加载；然而，HUP 信号的常规处理方式是仅在配置自服务器启动后发生更改时检查新表格。如果表格已在配置中定义，则应通过运行 `indexer` 而不进行旋转来首先创建表格，并执行 [RELOAD TABLES](../../Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES) 语句。

还可以使用两个专用语句来对表格执行旋转：

## RELOAD TABLE

```sql
RELOAD TABLE tbl [ FROM '/path/to/table_files' [ OPTION switchover=1 ] ];
```

`RELOAD TABLE` 命令通过 SQL 启用表格旋转。

该命令以三种模式运行。在第一种模式下，不指定路径，Manticore 服务器检查指定目录中的新表格文件。新表格文件必须命名为 `tbl.new.sp?`。

如果指定了路径，服务器将在该目录中搜索表格文件，将它们移动到表格 [path](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#path)，并将其从 `tbl.sp?` 重命名为 `tbl.new.sp?`，然后进行旋转。

第三种模式通过 `OPTION switchover=1` 激活，切换索引到新路径。在这里，守护进程尝试直接从新路径加载表格，而不移动文件。如果加载成功，这个新索引将取代旧索引。

此外，守护进程在由 [path](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#path) 指定的目录中写入一个唯一链接文件 (`tbl.link`)，保持持久重定向。

如果您将重定向的索引恢复到配置中指定的路径，守护进程会检测到这一点并删除链接文件。

一旦重定向，守护进程会从新链接的路径检索表格。在旋转时，它在新重定向的路径查找新表格版本。请记住，守护进程检查配置中的常见错误，例如不同表格间的重复路径。然而，它不会识别多个表格是否通过重定向指向同一路径。在正常操作中，表格会被 `.spl` 文件锁定，但禁用锁定可能会导致问题。如果出现错误（例如，由于任何原因路径无法访问），您应该手动更正（或直接删除）链接文件。

`indextool` 会跟随链接文件，但其他工具（`indexer`、`index_converter` 等）不识别链接文件，并一致使用配置文件中定义的路径，忽略任何重定向。因此，您可以使用 `indextool` 检查索引，它将从新位置读取。然而，更复杂的操作（如合并）将不承认任何链接文件。

```sql
mysql> RELOAD TABLE plain_table;
mysql> RELOAD TABLE plain_table FROM '/home/mighty/new_table_files';
mysql> RELOAD TABLE plain_table FROM '/home/mighty/new/place/for/table/table_files' OPTION switchover=1;
```

## RELOAD TABLES

```sql
RELOAD TABLES;
```

该命令的功能类似于 HUP 系统信号，触发表格的旋转。然而，它并不完全反映典型的 HUP 信号（可以来自 `kill -HUP` 命令或 `indexer --rotate`）。该命令主动搜索任何需要旋转的表格，并能够重新读取配置。假设您在简单模式下启动 Manticore，配置文件指向一个不存在的简单表格。如果您随后尝试 `indexer --rotate` 该表格，则在执行 `RELOAD TABLES` 或重新启动服务器之前，服务器将无法识别新表格。

根据 [seamless_rotate](../../Server_settings/Searchd.md#seamless_rotate) 设置的值，新查询可能会短暂停滞，客户将收到临时错误。

```sql
mysql> RELOAD TABLES;
Query OK, 0 rows affected (0.01 sec)
```

# 无缝旋转

旋转假设旧的表格版本被丢弃，新的表格版本被加载并替换现有版本。在此交换过程中，服务器还需要服务对即将更新的表格的传入查询。为了避免查询的停滞，服务器默认实现表格的无缝旋转，如下所述。

表格可能包含需要在 RAM 中预缓存的数据。此时，`.spa`、`.spb`、`.spi` 和 `.spm` 文件完全预缓存（它们分别包含属性数据、blob 属性数据、关键字表和已删除行映射）。在没有无缝旋转的情况下，旋转表格尝试尽可能少地使用 RAM，工作原理如下：
1. 新查询会被临时拒绝（返回“重试”错误代码）。
2. `searchd` 等待所有正在运行的查询完成。
3. 旧表被释放，其文件被重命名。
4. 新表文件被重命名，并分配所需的 RAM。
5. 新表属性和字典数据预加载到 RAM 中。
6. `searchd` 从新表恢复服务查询。

然而，如果有很多属性或字典数据，那么预加载步骤可能会花费明显的时间——在预加载 1-5+ GB 文件的情况下，可能需要几分钟。

启用无缝轮换时，轮换过程如下：

1. 分配新表 RAM 存储。
2. 新表属性和字典数据异步预加载到 RAM。
3. 成功时，旧表被释放，两个表的文件被重命名。
4. 失败时，新表被释放。
5. 在任何给定时刻，查询要么来自旧表，要么来自新表副本。

无缝轮换的代价是在轮换期间更高的 **峰值** 内存使用（因为在预加载新副本时，旧副本和新副本的 `.spa/.spb/.spi/.spm` 数据需要在 RAM 中）。然而，平均使用量保持不变。

示例：

```ini
seamless_rotate = 1
```
<!-- proofread -->

