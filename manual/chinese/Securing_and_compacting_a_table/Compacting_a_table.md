# 压缩表

随着时间的推移，RT 表可能会分散成多个磁盘块和/或被删除但未清除的数据污染，从而影响搜索性能。在这种情况下，有必要进行优化。基本上，优化过程是将成对的磁盘块合并，移除先前通过 DELETE 语句删除的文档。

从 Manticore 4 开始，这一过程[默认自动进行](../Server_settings/Searchd.md#auto_optimize)。不过，你也可以使用以下命令手动启动表压缩。

## OPTIMIZE TABLE

<!-- example optimize -->
```sql
OPTIMIZE TABLE table_name [OPTION opt_name = opt_value [,...]]
```

`OPTIMIZE` 语句将 RT 表添加到优化队列，该队列将在后台线程中处理。

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

默认情况下，OPTIMIZE 会将 RT 表的磁盘块合并到小于或等于逻辑 CPU 核心数乘以 2 的数量。

但是，如果表中带有带有 KNN 索引的属性，则该阈值不同。在这种情况下，它设置为物理 CPU 核心数除以 2，以提升 KNN 搜索性能。

你也可以使用 `cutoff` 选项手动控制优化后磁盘块的数量。

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

### 前台运行

<!-- example optimize_sync -->

使用 `OPTION sync=1`（默认值为 0）时，命令将在返回之前等待优化过程完成。如果连接中断，优化将在服务器上继续运行。

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

优化可能是一个漫长且 I/O 密集的过程。为了减少影响，所有实际的合并工作都在一个特殊的后台线程中串行执行，而 `OPTIMIZE` 语句仅将任务加入其队列。优化线程可以进行 I/O 限速，你可以通过[rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops) 和 [rt_merge_maxiosize](../Server_settings/Searchd.md#rt_merge_maxiosize) 指令分别控制最大每秒 I/O 数和最大 I/O 大小。

优化期间，被优化的 RT 表几乎始终在线且可供搜索和更新使用。只有在一对磁盘块成功合并时，会短暂锁定，以便重命名旧文件和新文件并更新表头。

### 优化集群表

只要未禁用[auto_optimize](../Server_settings/Searchd.md#auto_optimize)，表将自动优化。

如果你遇到意外的 SSTs，或者希望所有节点上的表二进制完全一致，则需要：
1. 禁用 [auto_optimize](../Server_settings/Searchd.md#auto_optimize)。
2. 手动优化表：
<!-- example cluster_manual_drop -->
在某个节点上，从集群中删除表：
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
优化表：
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
将表重新添加到集群：
<!-- request SQL -->
```sql
ALTER CLUSTER mycluster ADD myindex;
```
<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "ALTER CLUSTER mycluster ADD myindex;"
```
<!-- end -->
当表被重新添加后，优化过程中产生的新文件将被复制到集群中的其他节点。
在其他节点对该表做的任何本地更改将丢失。

表数据的修改（插入、替换、删除、更新）应当：

1. 被推迟，或者
2. 定向到正在运行优化过程的节点。

请注意，在表不在集群时，插入/替换/删除/更新命令应当不带集群名称前缀（对 SQL 语句或 HTTP JSON 请求中的 cluster 属性），否则将失败。
一旦表被重新添加到集群，必须恢复对表的写操作并重新包含集群名称前缀，否则操作将失败。

在此过程中，任一节点上的搜索操作正常可用。

<!-- proofread -->

