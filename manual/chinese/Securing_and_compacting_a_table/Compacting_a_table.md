# 压缩表格

随着时间的推移，RT 表可能会变得碎片化，形成多个磁盘块和/或被删除但尚未清除的数据污染，从而影响搜索性能。在这些情况下，需要进行优化。基本上，优化过程会将一对磁盘块合并，并删除先前使用 DELETE 语句删除的文档。

从 Manticore 4 开始，该过程将[默认自动发生](../Server_settings/Searchd.md#auto_optimize)。但是，您也可以使用以下命令手动启动表格压缩。

## OPTIMIZE TABLE

<!-- example optimize -->
```sql
OPTIMIZE TABLE table_name [OPTION opt_name = opt_value [,...]]
```

`OPTIMIZE` 语句将 RT 表添加到优化队列中，该队列将在后台线程中处理。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt;
```
<!-- end -->

### 优化后的磁盘块数量

<!-- example optimize_cutoff -->

默认情况下，OPTIMIZE 将 RT 表的磁盘块合并为不超过逻辑 CPU 核心数量的两倍。

但是，如果表中有带有 KNN 索引的属性，则该阈值会有所不同。在这种情况下，它被设置为物理 CPU 核心数量的一半，以提高 KNN 搜索性能。

您还可以使用 `cutoff` 选项手动控制优化后的磁盘块数量。

其他选项包括：
* 服务器设置 [optimize_cutoff](../Server_settings/Searchd.md#optimize_cutoff) 用于覆盖默认阈值
* 每个表的设置 [optimize_cutoff](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff)

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt OPTION cutoff=4;
```
<!-- end -->

### 在前台运行

<!-- example optimize_sync -->

当使用 `OPTION sync=1`（默认值为 0）时，命令将等待优化过程完成后再返回。如果连接中断，优化将继续在服务器上运行。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt OPTION sync=1;
```
<!-- end -->

### 限制 IO 影响

优化可能是一个漫长且 I/O 密集的过程。为了最小化影响，所有实际的合并工作都是在特殊的后台线程中顺序执行的，`OPTIMIZE` 语句只是将一个任务添加到其队列中。优化线程可以被限制 I/O，您可以通过 [rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops) 和 [rt_merge_maxiosize](../Server_settings/Searchd.md#rt_merge_maxiosize) 指令分别控制每秒的最大 I/O 数和最大 I/O 大小。

在优化过程中，被优化的 RT 表几乎始终保持在线并可供搜索和更新。在成功合并一对磁盘块时，它会被锁定一段非常短的时间，以允许旧文件和新文件重命名以及更新表头。

### 优化集群表格

只要 [auto_optimize](../Server_settings/Searchd.md#auto_optimize) 未被禁用，表格会自动优化。

如果您遇到意外的 SST 或希望集群中所有节点的表格在二进制上完全相同，则需要：
1. 禁用 [auto_optimize](../Server_settings/Searchd.md#auto_optimize)。
2. 手动优化表格：
<!-- example cluster_manual_drop -->
在其中一个节点上，从集群中删除表格：
<!-- request SQL -->
```sql
ALTER CLUSTER mycluster DROP myindex;
```
<!-- end -->
<!-- example cluster_manual_optimize -->
优化该表格：
<!-- request SQL -->
```sql
OPTIMIZE TABLE myindex;
```
<!-- end -->
<!-- example cluster_manual_add -->
将表格重新添加到集群中：
<!-- request SQL -->
```sql
ALTER CLUSTER mycluster ADD myindex;
```
<!-- end -->
当表格重新添加后，由优化过程创建的新文件将被复制到集群中的其他节点。
在其他节点上对该表格所做的任何本地更改将丢失。

表数据的修改（插入、替换、删除、更新）应当：

1. 被推迟，或
2. 被定向到优化过程正在运行的节点。

请注意，当表格脱离集群时，插入/替换/删除/更新命令应不带集群名称前缀引用它（对于 SQL 语句或在 HTTP JSON 请求中集群属性），否则它们将失败。
一旦表格重新添加到集群中，您必须恢复对该表格的写入操作，并再次包含集群名称前缀，否则它们将失败。

在该过程期间，在任何节点上搜索操作照常可用。

<!-- proofread -->
