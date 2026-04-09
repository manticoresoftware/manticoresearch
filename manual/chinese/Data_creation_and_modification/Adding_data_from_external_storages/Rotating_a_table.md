# 旋转表

表旋转是一种过程，其中 searchd 服务器查找配置中定义的表的新版本。旋转仅支持 Plain 模式的操作。

有两种情况：

* 对于已经加载的 Plain 表
* 在配置中添加但尚未加载的表

在第一种情况下，indexer 无法将表的新版本投入使用，因为运行中的副本已被 `searchd` 锁定和加载。此时需要以带有 [--rotate](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) 参数调用 `indexer`。如果使用 rotate，`indexer` 会创建名称中带有 `.new.` 的新表文件，并向 `searchd` 发送 *HUP* 信号，通知其有新版本。`searchd` 将执行查找操作，将新版本的表替换旧版本并丢弃旧版本。在某些情况下，可能希望先创建新版本表但不立即执行旋转。比如，可能先希望检查新表版本的健康状态。此时，`indexer` 可以接受 `--nohup` 参数，禁止向服务器发送 HUP 信号。

新表可以通过旋转加载；但是，常规处理 HUP 信号的方式是只有在服务器启动后配置发生更改时才检查新表。如果表已在配置中定义，应首先运行不带旋转的 `indexer` 创建表，然后执行 [RELOAD TABLES](../../Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES) 语句。

还有两个专用语句可以用于对表执行旋转：

## RELOAD TABLE

```sql
RELOAD TABLE tbl [ FROM '/path/to/table_files' [ OPTION switchover=1 ] ];
```

`RELOAD TABLE` 命令通过 SQL 实现表旋转。

此命令有三种模式。第一种模式是不指定路径，Manticore 服务器会检查由 [path](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#path) 指定的目录中新表文件。新表文件必须命名为 `tbl.new.sp?`。

如果指定路径，服务器会在该目录搜索表文件，将它们移动到表的 [path](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#path)，将文件名从 `tbl.sp?` 重命名为 `tbl.new.sp?`，并旋转它们。

第三种模式由 `OPTION switchover=1` 激活，将索引切换到新的路径。此时，守护进程尝试直接从新路径加载表而不移动文件。如果加载成功，这个新索引将取代旧索引。

此外，守护进程会在由 [path](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#path) 指定的目录中写入唯一链接文件（`tbl.link`），保持持久重定向。

如果将已重定向的索引恢复到配置中指定的路径，守护进程会检测到这一点并删除链接文件。

一旦重定向，守护进程从新的链接路径获取表。旋转时，它会在新的重定向路径查找新表版本。需要注意的是，守护进程会检查配置中的常见错误，比如不同表的路径重复，但不会识别多个表通过重定向指向同一路径的情况。正常操作下，表通过 `.spl` 文件锁定，但禁用锁定可能会导致问题。如果出现错误（例如路径因某种原因无法访问），需要手动修正（或简单删除）链接文件。

`indextool` 会跟随链接文件，但其他工具（`indexer`、`index_converter` 等）不识别链接文件，始终使用配置文件中定义的路径，忽略任何重定向。因此，可以用 `indextool` 检查索引，它会从新位置读取；但更复杂的操作如合并将不识别任何链接文件。

```sql
mysql> RELOAD TABLE plain_table;
mysql> RELOAD TABLE plain_table FROM '/home/mighty/new_table_files';
mysql> RELOAD TABLE plain_table FROM '/home/mighty/new/place/for/table/table_files' OPTION switchover=1;
```

## RELOAD TABLES

```sql
RELOAD TABLES;
```

此命令的功能类似于 HUP 系统信号，触发表的旋转。但它并不完全等同于典型的 HUP 信号（例如通过 `kill -HUP` 命令或 `indexer --rotate` 触发）。该命令会主动查找需要旋转的表，并能重新读取配置。比如，若以配置文件启动的 Manticore 处于 Plain 模式，但配置指向一个不存在的 Plain 表，之后执行 `indexer --rotate` 不会让服务器识别新表，需执行 `RELOAD TABLES` 或重启服务器。

根据 [seamless_rotate](../../Server_settings/Searchd.md#seamless_rotate) 设置的值，新查询可能会被短暂挂起，客户端会收到临时错误。

```sql
mysql> RELOAD TABLES;
Query OK, 0 rows affected (0.01 sec)
```

## 无缝旋转

旋转假设旧表版本被丢弃，新表版本被加载并替换现有表。在交换期间，服务器还需处理对即将更新表的入站查询。为了避免查询阻塞，服务器默认实现了表的无缝旋转，如下所述。

表可能包含需要预缓存到内存的数据。目前，`.spa`、`.spb`、`.spi` 和 `.spm` 文件是完全预缓存的（它们分别包含属性数据、二进制大对象属性数据、关键词表和已删除行映射）。如果不使用无缝旋转，旋转表尽可能少占用内存，工作流程如下：

1. 临时拒绝新查询（返回“重试”错误码）。
2. `searchd` 等待所有正在运行的查询完成。
3. 释放旧表并重命名其文件。
4. 重命名新表文件并分配所需内存。
5. 将新表属性和字典数据预加载到内存。
6. `searchd` 继续从新表处理查询。

不过，如果属性或字典数据量较大，则预加载步骤可能需要明显时间——在预加载 1-5+ GB 文件时可能达数分钟。

开启无缝旋转后，旋转过程如下：

1. 分配新的表的RAM存储。
2. 异步预加载新表的属性和字典数据到RAM。
3. 成功后，释放旧表并重命名两个表的文件。
4. 失败时，释放新表。
5. 在任何时刻，查询要么由旧表副本提供，要么由新表副本提供。

无缝旋转的代价是在旋转期间内存使用的**峰值**更高（因为预加载新副本时，旧副本和新副本的`.spa/.spb/.spi/.spm`数据都需要在RAM中）。不过平均使用量保持不变。

示例：

```ini
seamless_rotate = 1
```
<!-- proofread -->

