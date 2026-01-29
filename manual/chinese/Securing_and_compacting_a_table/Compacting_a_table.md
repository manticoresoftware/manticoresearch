# 紧凑表

随着时间的推移，RT 表可能会分散成多个磁盘块和/或被已删除但未清理的数据污染，影响搜索性能。在这些情况下，需要进行优化。基本上，优化过程会合并磁盘块（N 路合并），移除之前使用 DELETE 语句删除的文档。

从 Manticore 4 开始，这个过程默认[自动执行](../Server_settings/Searchd.md#auto_optimize)。但是，你也可以使用以下命令手动启动表紧凑操作。

## OPTIMIZE TABLE

<!-- example optimize -->
```sql
OPTIMIZE TABLE table_name [OPTION opt_name = opt_value [,...]]
```

`OPTIMIZE` 语句将 RT 表添加到优化队列，队列将在后台线程中处理。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt;
```
<!-- end -->

### 优化后的磁盘块数量

<!-- example optimize_cutoff -->

默认情况下，OPTIMIZE 会将 RT 表的磁盘块合并到不超过逻辑 CPU 核心数量乘以 2 的数量。

但是，如果表包含带有 KNN 索引的属性，这个阈值不同。在这种情况下，它被设置为物理 CPU 核心数量除以 2，以提升 KNN 搜索性能。

你也可以使用 `cutoff` 选项手动控制优化后的磁盘块数量。

附加选项包括：
* 服务器设置 [optimize_cutoff](../Server_settings/Searchd.md#optimize_cutoff) 来覆盖默认阈值
* 每表设置 [optimize_cutoff](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff)

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt OPTION cutoff=4;
```
<!-- end -->

### 前台运行

<!-- example optimize_sync -->

当使用 `OPTION sync=1`（默认是 0）时，命令会等待优化过程完成后才返回。如果连接中断，优化仍将在服务器上继续运行。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt OPTION sync=1;
```
<!-- end -->

### 限制 IO 影响

优化可能是一个耗时且 IO 密集的过程。`OPTIMIZE` 语句会将优化任务添加到后台工作池。你可以通过 [parallel_chunk_merges](../Server_settings/Searchd.md#parallel_chunk_merges) 设置并行运行的任务数量，通过 [merge_chunks_per_job](../Server_settings/Searchd.md#merge_chunks_per_job) 设置每个任务合并的磁盘块数量。优化工作线程可以进行 IO 限速，你可以通过 [rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops) 和 [rt_merge_maxiosize](../Server_settings/Searchd.md#rt_merge_maxiosize) 指令分别控制最大每秒 IO 次数和最大 IO 大小。

在优化期间，被优化的 RT 表几乎始终保持在线，既可用于搜索也可用于更新。只有在成功合并一对磁盘块时表才会被短暂锁定，用于重命名旧文件和新文件以及更新表头。

### 优化集群表

只要未禁用 [auto_optimize](../Server_settings/Searchd.md#auto_optimize)，表就会自动优化。

如果遇到意外的 SST 或希望集群中所有节点的表文件二进制完全一致，需要：
1. 禁用 [auto_optimize](../Server_settings/Searchd.md#auto_optimize)。
2. 手动优化表：
<!-- example cluster_manual_drop -->
在某个节点从集群中删除该表：
<!-- request SQL -->
```sql
ALTER CLUSTER mycluster DROP myindex;
```
<!-- end -->
<!-- example cluster_manual_optimize -->
优化该表：
<!-- request SQL -->
```sql
OPTIMIZE TABLE myindex;
```
<!-- end -->
<!-- example cluster_manual_add -->
将表重新添加回集群：
<!-- request SQL -->
```sql
ALTER CLUSTER mycluster ADD myindex;
```
<!-- end -->
当表被重新添加时，由优化过程产生的新文件将被复制到集群的其他节点。
其他节点上对该表所做的任何本地更改将会丢失。

对表数据的修改（插入、替换、删除、更新）应当：

1. 延后执行，或者
2. 定向到正在运行优化过程的节点。

请注意，表处于集群之外时，插入/替换/删除/更新命令应省略集群名称前缀（对于 SQL 语句或 HTTP JSON 请求中的 cluster 属性），否则会执行失败。
一旦表被重新加入集群，必须恢复写操作并重新包含集群名称前缀，否则将执行失败。

在整个过程中，任何节点均可照常进行搜索操作。

<!-- proofread -->
