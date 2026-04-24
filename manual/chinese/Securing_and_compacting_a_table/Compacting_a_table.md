# 压缩表

随着时间的推移，RT表可能会变得碎片化为许多磁盘块和/或被删除但未清除的数据污染，从而影响搜索性能。在这些情况下，需要进行优化。本质上，优化过程会合并磁盘块（N路合并），并移除之前使用DELETE语句删除的文档。

从Manticore 4开始，此过程[默认情况下自动执行](../Server_settings/Searchd.md#auto_optimize)。但是，您也可以使用以下命令手动启动表压缩。

## 优化表

<!--
以下示例的数据：

DROP TABLE IF EXISTS rt;
CREATE TABLE rt(title text);
INSERT INTO rt(title) VALUES
('doc one'),
('doc two'),
('doc three');
-->
<!-- example optimize -->
```sql
OPTIMIZE TABLE table_name [OPTION opt_name = opt_value [,...]]
```

`OPTIMIZE`语句将RT表添加到优化队列中，将在后台线程中进行处理。

<!-- intro -->
##### SQL：

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt;
```

<!-- intro -->
##### JSON：

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "OPTIMIZE TABLE rt"
```

<!-- end -->

### 优化的磁盘块数量

<!-- example optimize_cutoff -->

默认情况下，OPTIMIZE会将RT表的磁盘块合并到小于或等于逻辑CPU核心数乘以2的数量。

但是，如果表具有KNN索引的属性，则此阈值不同。在这种情况下，它设置为物理CPU核心数除以2，以提高KNN搜索性能。

您还可以使用`cutoff`选项手动控制优化的磁盘块数量。

其他选项包括：
* 服务器设置[optimize_cutoff](../Server_settings/Searchd.md#optimize_cutoff)用于覆盖默认阈值
* 每表设置[optimize_cutoff](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff)

<!-- intro -->
##### SQL：

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt OPTION cutoff=4;
```

<!-- intro -->
##### JSON：

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "OPTIMIZE TABLE rt OPTION cutoff=4"
```

<!-- end -->

### 前台运行

<!-- example optimize_sync -->

当使用`OPTION sync=1`（默认为0）时，命令将在优化过程完成之前等待返回。如果连接中断，优化将在服务器上继续运行。

<!-- intro -->
##### SQL：

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt OPTION sync=1;
```

<!-- intro -->
##### JSON：

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "OPTIMIZE TABLE rt OPTION sync=1"
```

<!-- end -->

### 限制IO影响

优化可能是一个漫长且I/O密集的过程。`OPTIMIZE`语句将任务添加到后台工作池中。您可以使用[parallel_chunk_merges](../Server_settings/Searchd.md#parallel_chunk_merges)控制并行运行的任务数量，使用[merge_chunks_per_job](../Server_settings/Searchd.md#merge_chunks_per_job)控制每个任务合并的块数。优化工作者可以被I/O限制，您可以使用[rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops)和[rt_merge_maxiosize](../Server_settings/Searchd.md#rt_merge_maxiosize)指令分别控制每秒的最大I/O数和最大I/O大小。

在优化期间，正在优化的RT表几乎始终在线，并且可用于搜索和更新。当一对磁盘块成功合并时，表会被短暂锁定，允许重命名旧文件和新文件并更新表头。

### 优化集群表

只要未禁用[auto_optimize](../Server_settings/Searchd.md#auto_optimize)，表将自动优化。

如果您遇到意外的SST或希望集群中所有节点的表二进制相同，则需要：
1. 禁用[auto_optimize](../Server_settings/Searchd.md#auto_optimize)。
2. 手动优化表

<!-- example cluster_manual_drop -->
在其中一个节点上，从集群中删除表：
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
<!--
以下示例的数据：

DROP TABLE IF EXISTS myindex;
CREATE TABLE myindex(title text);
INSERT INTO myindex(title) VALUES ('cluster doc');
-->
优化表：
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
将表重新添加到集群中：
<!-- request SQL -->
```sql
ALTER CLUSTER mycluster ADD myindex;
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "ALTER CLUSTER mycluster ADD myindex"
```

<!-- end -->
当表重新添加到集群中时，优化过程创建的新文件将被复制到集群中的其他节点。
其他节点上对表的任何本地更改都将丢失。

表数据修改（插入、替换、删除、更新）应：

1. 延迟，或
2. 指向正在运行优化过程的节点。

请注意，当表不在集群中时，插入/替换/删除/更新命令应不带集群名称前缀引用它（对于SQL语句或HTTP JSON请求中的集群属性），否则它们将失败。
一旦表重新添加到集群中，您必须恢复对表的写入操作并再次包含集群名称前缀，否则它们将失败。

在过程中，搜索操作在任何节点上都可以正常使用。

<!-- proofread -->
