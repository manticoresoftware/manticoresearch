# Compacting an index

Over time, RT indexes can become fragmented into many disk chunks and/or tainted with deleted, but unpurged data, impacting search performance. When that happens, they can be optimized. Basically, the optimization pass merges together disk chunks pairs, purging off documents suppressed previously by DELETEs.

Starting Manticore 4 it happens [automaticaly by default](../Server_settings/Searchd.md#auto_optimize), but you can also use the below commands to force index compaction.

## OPTIMIZE INDEX

<!-- example optimize -->
```sql
OPTIMIZE INDEX index_name [OPTION opt_name = opt_value [,...]]
```

`OPTIMIZE` statement enqueues an RT index for optimization in a background thread.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
OPTIMIZE INDEX rt;
```
<!-- end -->

### Number of optimized disk chunks

<!-- example optimize_cutoff -->

OPTIMIZE merges the RT index's disk chunks down to the number which equals to `# of CPU cores * 2` by default.  The number of optimized disk chunks can be controlled with option `cutoff`.

There's also:
* server setting [optimize_cutoff](../Server_settings/Searchd.md#optimize_cutoff) for overriding the above threshold
* per-table setting [optimize_cutoff](../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#optimize_cutoff)

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
OPTIMIZE INDEX rt OPTION cutoff=4;
```
<!-- end -->

### Running in foreground

<!-- example optimize_sync -->

If `OPTION sync=1` is used (0 by default), the command will wait until the optimization process is done (in case the connection interrupts the optimization will continue to run on the server).

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
OPTIMIZE INDEX rt OPTION sync=1;
```
<!-- end -->

### Throttling the IO impact

Optimize can be a lengthy and IO intensive process, so to limit the impact, all the actual merge work is executed serially in a special background thread, and the `OPTIMIZE` statement simply adds a job to its queue. Currently, there is no way to check the index or queue status (that might be added in the future to the `SHOW INDEX STATUS` and `SHOW STATUS` statements respectively). The optimization thread can be IO-throttled, you can control the maximum number of IOs per second and the maximum IO size with [rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops) and [rt_merge_maxiosize](../Server_settings/Searchd.md#rt_merge_maxiosize) directives respectively.

The RT index being optimized stays online and available for both searching and updates at (almost) all times during the optimization. It gets locked for a very short time when a pair of disk chunks is merged successfully, to rename the old and the new files, and update the index header.

### Optimizing clustered indexes

As long as you don't have [auto_optimize](../Server_settings/Searchd.md#auto_optimize) disabled indexes are optimized automatically

In case you are experiencing unexpected SSTs or want indexes across all nodes of the cluster be binary identical you need to:
1. Disable [auto_optimize](../Server_settings/Searchd.md#auto_optimize).
2. Optimize indexes manually:
<!-- example cluster_manual_drop -->
On one of the nodes drop the index from the cluster:
<!-- request SQL -->
```sql
ALTER CLUSTER mycluster DROP myindex;
```
<!-- end -->
<!-- example cluster_manual_optimize -->
Optimize the index:
<!-- request SQL -->
```sql
OPTIMIZE INDEX myindex;
```
<!-- end -->
<!-- example cluster_manual_add -->
Add back the index to the cluster:
<!-- request SQL -->
```sql
ALTER CLUSTER mycluster ADD myindex;
```
<!-- end -->
When the index is added back, the new files created by the optimize process will be replicated to the other nodes in the cluster.
Any changes made locally to the index on other nodes will be lost.

Index data modifications (inserts, replaces, deletes, updates) should:
1. either be **postponed**
2. or directed to the node where the optimize process is running.

Note, while the index is out of the cluster, insert/replace/delete/update commands should refer to it without cluster name prefix (for SQL statements or cluster property fin case of a HTTP JSON request), otherwise they will fail.
As soon as the index is added back to the cluster, writes can be resumed. At this point write operations on the index must include the cluster name prefix again, or they will fail.
Search operations are available as usual during the process on any of the nodes.
