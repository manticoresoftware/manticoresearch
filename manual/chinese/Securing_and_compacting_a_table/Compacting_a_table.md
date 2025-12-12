# 表的压缩

随着时间推移，RT 表可能会分散成多个磁盘块和/或被已删除但尚未清除的数据污染，影响搜索性能。在这些情况下，需要进行优化。基本上，优化过程将成对的磁盘块合并，移除先前通过 DELETE 语句删除的文档。

从 Manticore 4 开始，这一过程[默认自动进行](../Server_settings/Searchd.md#auto_optimize)。不过，你也可以使用以下命令手动启动表压缩。

## OPTIMIZE TABLE

<!-- example optimize -->
```sql
OPTIMIZE TABLE table_name [OPTION opt_name = opt_value [,...]]
```

`OPTIMIZE` 语句会将 RT 表添加到优化队列，由后台线程处理。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt;
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "OPTIMIZE TABLE rt;"
```

<!-- end -->

### 优化后的磁盘块数量

<!-- example optimize_cutoff -->

默认情况下，OPTIMIZE 会将 RT 表的磁盘块合并至小于或等于逻辑 CPU 核心数乘以 2 的数量。

不过，如果表具有 KNN 索引的属性，该阈值会不同。此时，阈值被设置为物理 CPU 核心数除以 2，以提高 KNN 搜索性能。

你也可以通过 `cutoff` 选项手动控制优化后的磁盘块数量。

其他选项包括：
* 覆盖默认阈值的服务器设置 [optimize_cutoff](../Server_settings/Searchd.md#optimize_cutoff)
* 每表设置 [optimize_cutoff](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff)

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt OPTION cutoff=4;
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "OPTIMIZE TABLE rt OPTION cutoff=4;"
```

<!-- end -->

### 前台执行

<!-- example optimize_sync -->

使用 `OPTION sync=1`（默认 0）时，命令会等待优化进程完成后才返回。如果连接中断，优化将继续在服务器端运行。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt OPTION sync=1;
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "OPTIMIZE TABLE rt OPTION sync=1;"
```

<!-- end -->

### 限制 IO 影响

优化可能是一个耗时且 IO 密集的过程。为了减少影响，所有实际合并工作在一个专门的后台线程中串行执行，而 `OPTIMIZE` 语句仅向其队列添加任务。优化线程可以进行 IO 限流，你可以通过[rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops)和[rt_merge_maxiosize](../Server_settings/Searchd.md#rt_merge_maxiosize) 指令分别控制每秒最大 IO 数和最大 IO 大小。

优化过程中，正在优化的 RT 表几乎保持在线，持续支持查询和更新。只有在成功合并一对磁盘块并重命名旧、新文件及更新表头时，表才会被短暂锁定。

### 优化集群表

只要未禁用 [auto_optimize](../Server_settings/Searchd.md#auto_optimize)，表都会自动优化。

如果出现意外的 SST，或希望集群所有节点的表二进制完全一致，需要：
1. 禁用 [auto_optimize](../Server_settings/Searchd.md#auto_optimize)。
2. 手动优化表：
<!-- example cluster_manual_drop -->
在某个节点，从集群中删除该表：
<!-- request SQL -->
```sql
ALTER CLUSTER mycluster DROP myindex;
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "ALTER CLUSTER mycluster DROP myindex;"
```

<!-- end -->
<!-- example cluster_manual_optimize -->
优化该表：
<!-- request SQL -->
```sql
OPTIMIZE TABLE myindex;
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "OPTIMIZE TABLE myindex;"
```

<!-- end -->
<!-- example cluster_manual_add -->
将该表重新加入集群：
<!-- request SQL -->
```sql
ALTER CLUSTER mycluster ADD myindex;
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "ALTER CLUSTER mycluster ADD myindex;"
```

<!-- end -->
当表重新加入时，优化过程中生成的新文件会被复制到集群其他节点。
其他节点对该表的任何本地更改将丢失。

表数据修改（插入、替换、删除、更新）应当：

1. 延后，或
2. 定向到正在执行优化的节点。

请注意，在表不在集群内期间，插入/替换/删除/更新命令应直接指向该表（SQL 语句中不带集群名称前缀，或 HTTP JSON 请求中不带 cluster 属性），否则操作会失败。
表重新加入集群后，必须恢复对表的写操作并包含集群名称前缀，否则操作会失败。

搜索操作在该过程中在任何节点上照常可用。

<!-- proofread -->

