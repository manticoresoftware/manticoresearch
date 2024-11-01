# 压缩表

随着时间的推移，RT 表可能会分散到多个磁盘块中，或被删除但未清除的数据污染，从而影响搜索性能。在这些情况下，需要进行优化。优化过程本质上是将一对磁盘块合并，并删除先前使用 DELETE 语句删除的文档。

从 Manticore 4 开始，该过程默认为[自动进行](../Server_settings/Searchd.md#auto_optimize)。不过，您也可以使用以下命令手动启动表压缩。

## OPTIMIZE TABLE

<!-- example optimize -->
```sql
OPTIMIZE TABLE index_name [OPTION opt_name = opt_value [,...]]
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

默认情况下，OPTIMIZE 将 RT 表的磁盘块合并到数量等于 `# CPU 核心 * 2`。您可以使用 `cutoff` 选项来控制优化后的磁盘块数量。

其他选项包括：

- 服务器设置 [optimize_cutoff](../Server_settings/Searchd.md#optimize_cutoff) 用于覆盖默认阈值
- 每表设置 [optimize_cutoff](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff)

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt OPTION cutoff=4;
```
<!-- end -->

### 在前台运行

<!-- example optimize_sync -->

当使用 `OPTION sync=1`（默认值为 0）时，该命令将在优化过程完成之前等待返回。如果连接中断，优化将继续在服务器上运行。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt OPTION sync=1;
```
<!-- end -->

### 限制 I/O 影响

优化可能是一个耗时且 I/O 密集的过程。为了最小化影响，所有实际的合并工作在一个特殊的后台线程中串行执行，而 `OPTIMIZE` 语句只是将一个作业添加到其队列中。优化线程可以进行 I/O 限制，您可以使用 [rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops) 和 [rt_merge_maxiosize](../Server_settings/Searchd.md#rt_merge_maxiosize) 指令控制每秒的最大 I/O 数量和最大 I/O 大小。

在优化过程中，被优化的 RT 表几乎始终保持在线，可用于搜索和更新。在成功合并一对磁盘块时，它会被锁定很短的时间，以允许重命名旧文件和新文件，并更新表头。

### 优化集群表

只要未禁用 [auto_optimize](../Server_settings/Searchd.md#auto_optimize)，表将自动优化。

如果您遇到意外的 SST 或希望集群中所有节点上的表是二进制相同的，则需要：

1. 禁用 [auto_optimize](../Server_settings/Searchd.md#auto_optimize)。
2. 手动优化表：
<!-- example cluster_manual_drop -->
在一个节点上，从集群中删除表：
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
将表添加回集群：
<!-- request SQL -->

```sql
ALTER CLUSTER mycluster ADD myindex;
```
<!-- end -->

当表添加回集群时，优化过程创建的新文件将被复制到集群中的其他节点。对表的任何本地更改都将丢失。

表数据的修改（插入、替换、删除、更新）应：

1. 被推迟，或
2. 指向正在运行优化过程的节点。

请注意，当表不在集群中时，插入/替换/删除/更新命令应在没有集群名称前缀的情况下引用它（对于 SQL 语句或在 HTTP JSON 请求中使用的集群属性），否则将失败。一旦表添加回集群，您必须恢复对该表的写操作，并再次包含集群名称前缀，否则将失败。

在过程中，任何节点上的搜索操作仍然可以正常进行。

<!-- proofread -->
