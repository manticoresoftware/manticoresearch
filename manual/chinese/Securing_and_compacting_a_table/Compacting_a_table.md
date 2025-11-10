# 紧凑表

随着时间的推移，RT 表可能会碎片化成多个磁盘块和/或被已删除但未清除的数据污染，影响搜索性能。在这些情况下，需要进行优化。基本上，优化过程是将成对的磁盘块合并，移除之前通过 DELETE 语句删除的文档。

从 Manticore 4 开始，这个过程默认[自动进行](../Server_settings/Searchd.md#auto_optimize)。但是，你也可以使用以下命令手动启动表的紧凑。

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
<!-- end -->

### 优化后的磁盘块数量

<!-- example optimize_cutoff -->

默认情况下，OPTIMIZE 会将 RT 表的磁盘块合并到不超过逻辑 CPU 核心数乘以 2 的数量。

但是，如果表中有带 KNN 索引的属性，这个阈值会不同。在这种情况下，阈值设置为物理 CPU 核心数除以 2，以提升 KNN 搜索性能。

你也可以使用 `cutoff` 选项手动控制优化后的磁盘块数量。

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

使用 `OPTION sync=1`（默认值为 0）时，命令会等待优化过程完成后才返回。如果连接中断，优化仍会在服务器上继续运行。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt OPTION sync=1;
```
<!-- end -->

### 限制 IO 影响

优化可能是一个耗时且 IO 密集的过程。为了减少影响，所有实际的合并工作都在一个特殊的后台线程中串行执行，`OPTIMIZE` 语句只是将任务添加到其队列中。优化线程可以进行 IO 限速，你可以通过 [rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops) 和 [rt_merge_maxiosize](../Server_settings/Searchd.md#rt_merge_maxiosize) 指令分别控制每秒最大 IO 数和最大 IO 大小。

在优化过程中，被优化的 RT 表几乎始终在线且可用于搜索和更新。只有在成功合并一对磁盘块时，表会被短暂锁定，以便重命名旧文件和新文件并更新表头。

### 优化集群表

只要没有禁用 [auto_optimize](../Server_settings/Searchd.md#auto_optimize)，表会自动优化。

如果你遇到意外的 SST 文件，或者希望集群中所有节点的表二进制完全一致，你需要：
1. 禁用 [auto_optimize](../Server_settings/Searchd.md#auto_optimize)。
2. 手动优化表：
<!-- example cluster_manual_drop -->
在某个节点上，从集群中删除表：
<!-- request SQL -->
```sql
ALTER CLUSTER mycluster DROP myindex;
```
<!-- end -->
<!-- example cluster_manual_optimize -->
优化表：
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
当表被重新添加时，优化过程中创建的新文件会被复制到集群中的其他节点。
其他节点上对表的任何本地更改都会丢失。

表数据的修改（插入、替换、删除、更新）应当：

1. 延后执行，或
2. 定向到正在运行优化过程的节点。

注意，在表不在集群中时，插入/替换/删除/更新命令应当不带集群名称前缀（对于 SQL 语句或 HTTP JSON 请求中的 cluster 属性），否则会失败。
一旦表重新加入集群，必须恢复对表的写操作并再次包含集群名称前缀，否则操作会失败。

在此过程中，任何节点上的搜索操作照常可用。

<!-- proofread -->

