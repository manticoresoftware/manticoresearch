# 紧凑表

随着时间的推移，RT 表可能会碎片化成许多磁盘块和/或被已删除但尚未清理的数据污染，影响搜索性能。在这些情况下，需要进行优化。实质上，优化过程是将成对的磁盘块合并，删除以前通过 DELETE 语句删除的文档。

从 Manticore 4 开始，这个过程[默认自动发生](../Server_settings/Searchd.md#auto_optimize)。不过，也可以使用以下命令手动启动表压缩。

## OPTIMIZE TABLE

<!-- example optimize -->
```sql
OPTIMIZE TABLE table_name [OPTION opt_name = opt_value [,...]]
```

`OPTIMIZE` 语句将 RT 表添加到优化队列，优化将由后台线程处理。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt;
```
<!-- end -->

### 优化后的磁盘块数量

<!-- example optimize_cutoff -->

默认情况下，OPTIMIZE 会将 RT 表的磁盘块合并到不超过逻辑 CPU 核心数乘以 2 的数量。

但是，如果表具有带有 KNN 索引的属性，则该阈值不同。在这种情况下，该阈值设置为物理 CPU 核心数除以 2，以提高 KNN 搜索性能。

你还可以使用 `cutoff` 选项手动控制优化后的磁盘块数量。

其他选项包括：
* 服务器设置 [optimize_cutoff](../Server_settings/Searchd.md#optimize_cutoff) 用于覆盖默认阈值
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

使用 `OPTION sync=1`（默认值为 0）时，命令将在返回之前等待优化过程完成。如果连接中断，优化将继续在服务器上运行。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt OPTION sync=1;
```
<!-- end -->

### 限制 I/O 影响

优化可能是一个耗时且 I/O 密集的过程。为了减少影响，所有实际的合并工作均由一个特殊后台线程串行执行，`OPTIMIZE` 语句仅向其队列添加任务。此优化线程可以进行 I/O 节流，你可以通过 [rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops) 和 [rt_merge_maxiosize](../Server_settings/Searchd.md#rt_merge_maxiosize) 指令，分别控制最大 I/O 请求次数和最大 I/O 大小。

在优化过程中，被优化的 RT 表几乎始终在线且可供搜索和更新使用。只有在一对磁盘块成功合并时，表会被短暂锁定，进行旧文件和新文件的重命名及表头更新。

### 优化集群表

只要未禁用 [auto_optimize](../Server_settings/Searchd.md#auto_optimize)，表将自动优化。

如果你遇到了意外的 SST，或者想让集群所有节点上的表具有二进制完全相同的数据，需要：
1. 禁用 [auto_optimize](../Server_settings/Searchd.md#auto_optimize)。
2. 手动优化表：
<!-- example cluster_manual_drop -->
在集群一个节点上，从集群中删除表：
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
将表重新添加到集群：
<!-- request SQL -->
```sql
ALTER CLUSTER mycluster ADD myindex;
```
<!-- end -->
表被重新添加后，优化过程中产生的新文件会被复制到集群中的其他节点。
其他节点上的任何本地更改都将丢失。

表数据的修改（插入、替换、删除、更新）应当：

1. 推迟执行，或
2. 定向到正在运行优化过程的节点。

请注意，在表不在集群中时，插入/替换/删除/更新命令应在 SQL 语句或 HTTP JSON 请求的 cluster 属性中去掉集群名称前缀，否则会失败。
表重新加入集群后，必须恢复写操作并重新包含集群名称前缀，否则操作会失败。

搜索操作在整个过程中在任何节点上均按常规可用。

<!-- proofread -->

