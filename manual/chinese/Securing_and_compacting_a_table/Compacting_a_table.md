# 压缩表格

随着时间的推移，RT表可能会变得碎片化，分散在多个磁盘块中，并且可能被删除但未清除的数据污染，从而影响搜索性能。在这种情况下，需要进行优化。本质上，优化过程会合并磁盘块对，移除使用DELETE语句之前已删除的文档。

从Manticore 4开始，默认情况下，此过程会自动发生[参见服务器设置中的Searchd.md](../Server_settings/Searchd.md#auto_optimize)。但是，您也可以使用以下命令手动启动表压缩。

## OPTIMIZE TABLE

<!-- example optimize -->
```sql
OPTIMIZE TABLE table_name [OPTION opt_name = opt_value [,...]]
```

`OPTIMIZE`语句将RT表添加到优化队列中，该队列将在后台线程中处理。

<!-- intro -->
##### SQL：

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt;
```
<!-- end -->

### 被优化的磁盘块数量

<!-- example optimize_cutoff -->

默认情况下，`OPTIMIZE`会将RT表的磁盘块合并到逻辑CPU核心数乘以2的数量以下。

然而，如果表具有KNN索引的属性，这个阈值不同。在这种情况下，它设置为物理CPU核心数除以2，以提高KNN搜索性能。

您还可以通过`cutoff`选项手动控制被优化的磁盘块数量。

其他选项包括：
* 服务器设置[optimize_cutoff](../Server_settings/Searchd.md#optimize_cutoff)以覆盖默认阈值
* 表设置[optimize_cutoff](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff)

<!-- intro -->
##### SQL：

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt OPTION cutoff=4;
```
<!-- end -->

### 在前台运行

<!-- example optimize_sync -->

当使用`OPTION sync=1`（默认为0）时，该命令会在优化过程完成后再返回。如果连接中断，优化将继续在服务器上运行。

<!-- intro -->
##### SQL：

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt OPTION sync=1;
```
<!-- end -->

### 控制IO影响

优化可能是一个耗时且I/O密集型的过程。为了最小化其影响，实际合并工作是在一个特殊的后台线程中串行执行的，而`OPTIMIZE`语句只是将其添加到队列中。优化线程可以进行I/O限制，并可以通过[rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops)和[rt_merge_maxiosize](../Server_settings/Searchd.md#rt_merge_maxiosize)指令分别控制每秒的最大I/O次数和最大I/O大小。

在优化过程中，正在优化的RT表几乎一直在线并可供搜索和更新使用。当成功合并一对磁盘块时，表会被锁定一个非常短暂的时间，以便重命名旧文件和新文件，并更新表头。

### 压缩集群表

只要[auto_optimize](../Server_settings/Searchd.md#auto_optimize)没有禁用，表就会自动优化。

如果您遇到意外的SST或希望集群中所有节点上的表二进制相同，则需要：
1. 禁用[auto_optimize](../Server_settings/Searchd.md#auto_optimize)。
2. 手动优化表：
<!-- example cluster_manual_drop -->
在一个节点上从集群中删除表：
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
将表重新添加回集群：
<!-- request SQL -->
```sql
ALTER CLUSTER mycluster ADD myindex;
```
<!-- end -->
当表重新添加回集群时，优化过程中创建的新文件将被复制到集群中的其他节点。
其他节点上对该表所做的任何本地更改都将丢失。

对表数据的修改（插入、替换、删除、更新）应选择以下之一：

1. 推迟这些操作，
2. 将它们定向到正在运行优化过程的节点。

请注意，在表退出集群期间，插入/替换/删除/更新命令应不带集群名称前缀（对于SQL语句或HTTP JSON请求中的集群属性），否则将失败。
一旦表重新添加到集群，您必须恢复对该表的写入操作，并再次包含集群名称前缀，否则这些操作将失败。

在优化过程中，可以在任何节点上正常使用搜索操作。

<!-- proofread -->

