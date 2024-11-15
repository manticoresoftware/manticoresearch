# 轮换表

表的轮换是指在 `searchd` 服务器中检查配置中定义的表是否有新版本的过程。轮换仅支持在普通模式下运行。

有两种情况：

- 对于已经加载的普通表
- 配置中已添加但尚未加载的表

在第一种情况下，`indexer` 无法将表的新版本上线，因为运行中的副本被 `searchd` 锁定并加载。此时需要使用 `indexer` 并加上 [--rotate](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-命令行参数) 参数。如果使用了轮换，`indexer` 会创建带有 `.new.` 名称的新表文件，并向 `searchd` 发送 *HUP* 信号，通知其有关新版本的信息。`searchd` 将执行查找，并将新版本的表替换旧版本。在某些情况下，可能希望创建新版本的表，但不立即执行轮换。例如，可能需要先检查新表版本的健康状态。在这种情况下，`indexer` 可以使用 `--nohup` 参数来禁止向服务器发送 HUP 信号。

可以通过轮换加载新表；然而，HUP 信号的常规处理方式是仅在配置自服务器启动以来发生更改时检查新表。如果表已经在配置中定义，则应先运行 `indexer`，而不进行轮换操作，然后执行 [RELOAD TABLES](../../Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES) 语句。

此外，还有两个专用语句可用于表的轮换操作：

## RELOAD TABLE

```sql
RELOAD TABLE tbl [ FROM '/path/to/table_files' [ OPTION switchover=1 ] ];
```

`RELOAD TABLE` 命令通过 SQL 实现表轮换。

此命令有三种工作模式。在第一种模式下，如果没有指定路径，Manticore 服务器将检查由 [path](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#path) 指定的目录中的新表文件。新表文件必须命名为 `tbl.new.sp?`。

如果指定了路径，服务器会在该目录中搜索表文件，将其迁移到表的 [path](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#path)，并将其从 `tbl.sp?` 重命名为 `tbl.new.sp?`，然后轮换表。

第三种模式通过 `OPTION switchover=1` 激活，切换索引到新路径。在这种情况下，守护进程尝试直接从新路径加载表，而无需移动文件。如果加载成功，该新索引将取代旧索引。

此外，守护进程还会在 [path](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#path) 指定的目录中写入唯一的链接文件（`tbl.link`），以保持持久的重定向。

如果你将重定向的索引还原到配置中指定的路径，守护进程将检测到此情况并删除链接文件。

重定向后，守护进程将从新的链接路径中获取表。当轮换时，它将在重定向的新路径中查找新版本的表。需要注意的是，守护进程会检查配置中的常见错误，如不同表的重复路径。但是，它不会识别多个表通过重定向指向同一路径。在正常操作中，表会通过 `.spl` 文件进行锁定，但禁用锁定可能会导致问题。如果出现错误（例如路径不可访问），应手动更正（或简单删除）链接文件。

`indextool` 工具会跟随链接文件，但其他工具（如 `indexer`、`index_converter` 等）不会识别链接文件，并且始终使用配置文件中定义的路径，忽略任何重定向。因此，你可以使用 `indextool` 检查索引，它将从新位置读取。然而，更复杂的操作如合并将不识别任何链接文件。

```sql
mysql> RELOAD TABLE plain_table;
mysql> RELOAD TABLE plain_table FROM '/home/mighty/new_table_files';
mysql> RELOAD TABLE plain_table FROM '/home/mighty/new/place/for/table/table_files' OPTION switchover=1;
```

## RELOAD TABLES

```sql
RELOAD TABLES;
```

此命令的作用类似于 HUP 系统信号，触发表的轮换。然而，它并不完全等同于典型的 HUP 信号（该信号可以通过 `kill -HUP` 命令或 `indexer --rotate` 触发）。此命令主动查找任何需要轮换的表，并能够重新读取配置。假设你在普通模式下启动 Manticore，并使用的配置文件指向一个不存在的普通表。如果你尝试 `indexer --rotate` 该表，服务器将无法识别该新表，直到你执行 `RELOAD TABLES` 或重启服务器。

根据 [seamless_rotate](../../Server_settings/Searchd.md#seamless_rotate) 设置的值，新查询可能会短暂停滞，客户端将收到临时错误。

```sql
mysql> RELOAD TABLES;
Query OK, 0 rows affected (0.01 sec)
```

# 无缝轮换

轮换假设旧的表版本被丢弃，新表版本被加载并替换现有表。在此交换过程中，服务器还需要处理对即将更新的表进行的传入查询。为了避免查询中断，服务器默认实现了表的无缝轮换，具体如下所述。

表可能包含需要预缓存到 RAM 中的数据。目前，`.spa`、`.spb`、`.spi` 和 `.spm` 文件会完全预缓存（它们分别包含属性数据、BLOB 属性数据、关键字表和已删除行的映射）。如果没有无缝轮换，表轮换会尽量减少 RAM 的使用，其工作方式如下：

1. 新的查询会被暂时拒绝（返回 "retry" 错误码）。
2. `searchd` 等待所有正在运行的查询结束。
3. 旧表被释放，其文件被重命名。
4. 新表文件被重命名，所需的 RAM 被分配。
5. 新表的属性和字典数据被预加载到 RAM 中。
6. `searchd` 恢复从新表中服务查询。

然而，如果存在大量的属性或字典数据，那么预加载步骤可能需要相当长的时间——预加载 1-5 GB 的文件可能需要几分钟。

启用了无缝轮换时，轮换按以下步骤进行：

1. 分配新表的 RAM 存储。
2. 新表的属性和字典数据被异步预加载到 RAM 中。
3. 成功后，旧表被释放，并且新旧表的文件被重命名。
4. 如果失败，新表被释放。
5. 在任何时刻，查询要么从旧表副本中服务，要么从新表副本中服务。

无缝轮换的代价是轮换期间较高的 **峰值** 内存使用（因为在预加载新副本时，旧的和新的 `.spa/.spb/.spi/.spm` 数据都需要在 RAM 中）。然而，平均使用量保持不变。

示例

```ini
seamless_rotate = 1
```
<!-- proofread -->
