# Compacting a Table

Over time, RT tables may become fragmented into numerous disk chunks and/or contaminated with deleted, yet unpurged data, affecting search performance. In these cases, optimization is necessary. Essentially, the optimization process combines pairs of disk chunks, removing documents that were previously deleted using DELETE statements.

Beginning with Manticore 4, this process occurs [automatically by default](../Server_settings/Searchd.md#auto_optimize). However, you can also use the following commands to manually initiate table compaction.

## OPTIMIZE TABLE

<!-- example optimize -->
```sql
OPTIMIZE TABLE index_name [OPTION opt_name = opt_value [,...]]
```

`OPTIMIZE` statement adds an RT table to the optimization queue, which will be processed in a background thread.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt;
```
<!-- end -->

### Number of optimized disk chunks

<!-- example optimize_cutoff -->

By default, OPTIMIZE merges the RT table's disk chunks down to a number equal to `# of CPU cores * 2`. You can control the number of optimized disk chunks using the `cutoff` option.

Additional options include:
* Server setting [optimize_cutoff](../Server_settings/Searchd.md#optimize_cutoff) for overriding the default threshold
* Per-table setting [optimize_cutoff](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff)

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt OPTION cutoff=4;
```
<!-- end -->

### Running in foreground

<!-- example optimize_sync -->

When using `OPTION sync=1` (0 by default), the command will wait for the optimization process to complete before returning. If the connection is interrupted, the optimization will continue running on the server.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
OPTIMIZE TABLE rt OPTION sync=1;
```
<!-- end -->

### Throttling the IO impact

Optimization can be a lengthy and I/O-intensive process. To minimize the impact, all actual merge work is executed serially in a special background thread, and the `OPTIMIZE` statement simply adds a job to its queue. Currently, there is no way to check the table or queue status (though this may be added in the future to the `SHOW TABLE STATUS` and `SHOW STATUS` statements, respectively). The optimization thread can be I/O-throttled, and you can control the maximum number of I/Os per second and the maximum I/O size with the [rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops) and [rt_merge_maxiosize](../Server_settings/Searchd.md#rt_merge_maxiosize) directives, respectively.

During optimization, the RT table being optimized remains online and available for both searching and updates nearly all the time. It is locked for a very brief period when a pair of disk chunks is successfully merged, allowing for the renaming of old and new files and updating the table header.

### Optimizing clustered tables

As long as [auto_optimize](../Server_settings/Searchd.md#auto_optimize) is not disabled, tables are optimized automatically.

If you are experiencing unexpected SSTs or want tables across all nodes of the cluster to be binary identical, you need to:
1. Disable [auto_optimize](../Server_settings/Searchd.md#auto_optimize).
2. Manually optimize tables:
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
When the table is added back, the new files created by the optimization process will be replicated to the other nodes in the cluster.
Any local changes made to the table on other nodes will be lost.

Table data modifications (inserts, replaces, deletes, updates) should either:

1. Be postponed, or
2. Be directed to the node where the optimization process is running.

Note that while the table is out of the cluster, insert/replace/delete/update commands should refer to it without the cluster name prefix (for SQL statements or the cluster property in case of an HTTP JSON request), otherwise they will fail.
Once the table is added back to the cluster, you must resume write operations on the table and include the cluster name prefix again, or they will fail.

Search operations are available as usual during the process on any of the nodes.

<!-- proofread -->
