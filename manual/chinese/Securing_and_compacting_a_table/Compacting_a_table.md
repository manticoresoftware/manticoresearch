# 磁盘表压缩

随着时间的推移，RT表可能会因磁盘块的碎片化或删除但未清理的数据而变得分散，影响搜索性能。在这种情况下，优化是必要的。本质上，优化过程将结合磁盘块的对称性，使用DELETE操作删除之前删除的文档。

从Manticore 4开始，该过程会自动执行[默认](../Server_settings/Searchd.md#auto_optimize)。但你也可以使用以下命令手动启动表压缩。

## OPTIMIZE TABLE

<!-- example optimize -->
```sql
OPTIMIZE TABLE table_name [OPTION opt_name = opt_value [,...]]
```

`OPTIMIZE` 语句会将表添加到优化队列，该队列将在后台处理。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt;
```
<!-- end -->

### 磁盘块优化数量

<!-- example optimize_cutoff -->

默认情况下，`OPTIMIZE` 会将表的磁盘块合并到小于或等于逻辑CPU核心数乘以2的范围内。

然而，如果表具有KNN索引，该阈值将不同。在这种情况下，该阈值设置为物理CPU核心数除以2，以提高KNN搜索性能。

你可以手动控制磁盘块优化数量，使用`cutoff`选项。

其他选项包括：
* [optimize_cutoff](../Server_settings/Searchd.md#optimize_cutoff)：设置默认阈值
* [optimize_cutoff](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff)：表级别的设置

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt OPTION cutoff=4;
```
<!-- end -->

### 在前台运行

<!-- example optimize_sync -->

使用`OPTION sync=1`（默认为0），该命令将等待优化过程完成后再返回。如果连接中断，优化过程将继续运行在服务器上。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt OPTION sync=1;
```
<!-- end -->

### 控制IO影响

优化过程可能耗时且I/O密集。为最小化影响，所有实际合并工作将在特殊后台线程中执行，`OPTIMIZE` 仅添加一个任务到队列中。优化线程可以被I/O限制，同时可以设置最大每秒I/O和最大I/O大小，分别通过`rt_merge_iops`和`rt_merge_maxiosize`指令实现。

在优化期间，正在优化的表仍在线运行并可供搜索和更新几乎所有时间。当磁盘块成功合并时，表将被锁定，允许旧和新文件的重命名以及表头的更新。

### 对集群表的优化

只要[auto_optimize](../Server_settings/Searchd.md#auto_optimize)未禁用，表将自动优化。

如果你遇到意外的SST或希望所有集群节点上的表都为二进制相同，则需要：
1. 禁用[auto_optimize](../Server_settings/Searchd.md#auto_optimize)。
2. 手动优化表：
<!-- example cluster_manual_drop -->
On one of the nodes, drop the table from the cluster:
<!-- request SQL -->
```sql
ALTER CLUSTER mycluster DROP myindex;
```
<!-- end -->
<!-- example cluster_manual_optimize -->
Optimize the table:
<!-- request SQL -->
```sql
OPTIMIZE TABLE myindex;
```
<!-- end -->
<!-- example cluster_manual_add -->
Add back the table to the cluster:
<!-- request SQL -->
```sql
ALTER CLUSTER mycluster ADD myindex;
```
<!-- end -->
当表被添加回集群后，新创建的文件将复制到集群中的其他节点。
任何在其他节点上对表进行的本地更改都会丢失。

表数据修改（插入、替换、删除、更新）应要么推迟，要么指向优化过程中运行的节点。

1. 注意：虽然表离开集群，插入/替换/删除/更新命令应以集群名称前缀不包含（用于SQL或HTTP JSON请求的集群属性）来引用，否则它们将失败。
2. 一旦表被添加回集群，必须重新启用写入操作，并在添加后包含集群名称前缀，否则它们将失败。

搜索操作在任何节点上都可以正常使用。
Once the table is added back to the cluster, you must resume write operations on the table and include the cluster name prefix again, or they will fail.

Search operations are available as usual during the process on any of the nodes.

<!-- proofread -->

