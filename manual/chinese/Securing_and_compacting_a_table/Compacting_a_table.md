# 紧凑表

随着时间的推移，RT表可能会碎片化成多个磁盘块和/或被已删除但未清理的数据污染，影响搜索性能。在这种情况下，需要进行优化。本质上，优化过程是将成对的磁盘块合并，移除先前通过DELETE语句删除的文档。

从Manticore 4开始，此过程[默认自动进行](../Server_settings/Searchd.md#auto_optimize)。但是，您也可以使用以下命令手动启动表紧凑。

## OPTIMIZE TABLE

<!-- example optimize -->
```sql
OPTIMIZE TABLE table_name [OPTION opt_name = opt_value [,...]]
```

`OPTIMIZE`语句将RT表添加到优化队列，该队列将在后台线程中处理。

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
POST /sql?mode=raw -d "OPTIMIZE TABLE rt"
```

<!-- end -->

### 优化的磁盘块数量

<!-- example optimize_cutoff -->

默认情况下，OPTIMIZE将RT表的磁盘块合并至不超过逻辑CPU核心数乘以2的数量。

但是，如果表中包含带有KNN索引的属性，则该阈值不同。在这种情况下，阈值设定为物理CPU核心数除以2，以提升KNN搜索性能。

您也可以使用`cutoff`选项手动控制优化后的磁盘块数量。

其他选项包括：
* 服务器设置 [optimize_cutoff](../Server_settings/Searchd.md#optimize_cutoff) 用于覆盖默认阈值
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
POST /sql?mode=raw -d "OPTIMIZE TABLE rt OPTION cutoff=4"
```

<!-- end -->

### 在前台运行

<!-- example optimize_sync -->

使用`OPTION sync=1`（默认0）时，命令会等待优化过程完成后才返回。如果连接中断，优化将继续在服务器上运行。

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
POST /sql?mode=raw -d "OPTIMIZE TABLE rt OPTION sync=1"
```

<!-- end -->

### 限制IO影响

优化过程可能耗时且IO密集。为了减少影响，所有实际的合并工作都在一个特殊的后台线程中串行执行，`OPTIMIZE`语句仅将任务加入其队列。优化线程可以进行IO限速，您可以通过[rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops)与[rt_merge_maxiosize](../Server_settings/Searchd.md#rt_merge_maxiosize)指令分别控制最大每秒IO次数和最大IO大小。

在优化期间，被优化的RT表几乎始终在线且可用于搜索和更新。合并成功时会锁定极短时间，以便重命名旧、新文件并更新表头。

### 优化集群表

只要未禁用[auto_optimize](../Server_settings/Searchd.md#auto_optimize)，表会自动优化。

如果您遇到意外的SST或希望集群所有节点上的表二进制完全一致，需：
1. 禁用 [auto_optimize](../Server_settings/Searchd.md#auto_optimize)。
2. 手动优化表：
<!-- example cluster_manual_drop -->
在某个节点，从集群中删除表：
<!-- request SQL -->
```sql
ALTER CLUSTER mycluster DROP myindex;
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "ALTER CLUSTER mycluster DROP myindex"
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
POST /sql?mode=raw -d "OPTIMIZE TABLE myindex"
```

<!-- end -->
<!-- example cluster_manual_add -->
将表重新添加回集群：
<!-- request SQL -->
```sql
ALTER CLUSTER mycluster ADD myindex;
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "ALTER CLUSTER mycluster ADD myindex"
```

<!-- end -->
当表重新添加后，通过优化过程创建的新文件将被复制到集群中其他节点。
其他节点上的任何本地更改将丢失。

表数据修改（插入、替换、删除、更新）要么：

1. 延迟执行，或
2. 发送至正在执行优化的节点。

注意，当表处于集群外时，插入/替换/删除/更新命令应不带集群名称前缀（针对SQL语句或HTTP JSON请求的cluster属性），否则会失败。
表重新加入集群后，必须恢复写操作，并再次包含集群名称前缀，否则会失败。

整个过程中，搜索操作可在任一节点照常进行。

<!-- proofread -->

