# Compacting an index

Over time, RT indexes can become fragmented into many disk chunks and/or tainted with deleted, but unpurged data, impacting search performance. When that happens, they can be optimized. Basically, the optimization pass merges together disk chunks pairs, purging off documents suppressed by K-list as it goes.

Starting from Manticore 4 it happens automaticaly by default, but you can also use the below commands to force index compaction.

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

The optimize process reduces the disk chunks by default to a number equal with  `# of CPU cores * 2`.  The number of optimized disk chunks can be controlled with option `cutoff`.

There's also a server setting [optimize_cutoff](../Server_settings/Searchd.md#optimize_cutoff) for overriding the above threshold.

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

**At the moment, OPTIMIZE needs to be issued manually, the indexes are not optimized automatically.** It will be changed in future releases.

### Optimizing clustered indexes

Currently indexes cannot be optimized directly while are being part of a cluster.

The following procedure should be used:


On one of the nodes drop the index from the cluster:

```sql
mysql> ALTER CLUSTER mycluster DROP myindex;
```

Optimize the index:

```sql
mysql> OPTIMIZE INDEX myindex;
```


Add back the index to the cluster:

```sql
mysql> ALTER CLUSTER mycluster ADD myindex;
```

When the index is added back, the new files created by the optimize process will be replicated to the other nodes in the cluster.
Any changes made locally to the index on other nodes will be lost.

Writes on the index should either be **stopped** or directed to the node were the optimize process is running.
Note that after the index is out of the cluster, writes must be made locally and the index name must not contain the cluster name as prefix (for SQL statements or cluster property for HTTP requests).
As soon as the index is added back to the cluster, writes can be resumed. At this point the writes operations on the index must include (again) the cluster prefix (for SQL statements or cluster property for HTTP requests).
Searches will be available as usual during the process on any of the nodes.

In future releases it's expected to remove the need of this process and simply perform OPTIMIZE without the need to take the index out of the cluster.
