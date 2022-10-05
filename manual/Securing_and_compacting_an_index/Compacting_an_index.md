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

# Back up an index

Automatic maintaining of rt-index with flushing, optimize, saving ram/disk chunks, binlogging, etc. implies it has quite complex structure.

Say, during optimize pass we have a couple of original disk chunks, and also new chunk produced by merging these twin. Then, at one moment we create new 'version' of the index, where instead original couple of chunks new one placed. That is done seamless, so that if some long-running query uses original chunks, it will see 'old' version of the index with these chunks in place, and new arriving queries will at the same time work with 'new' version where just one merged chunk in game. 

Same is true when flushing RAM chunk: we merge all suitable RAM segments into new disk chunk, and finally put new-arrived chunk into the set of disk chunks, and abandon participated RAM segments from RAM chunk. During this operation, we also provide seamless index, so that long queries started before such substitution see previous version of the index with big RAM chunk, and new one see new one where we have +1 disk chunk, and RAM chunk abandoned.

Moreover, these operations also transparent for replaces/updates. If you update an attribute in a document which belongs to a merging disk chunk, this update will be applied both to that chunk and to resulting chunk after merge. If you delete a document during merge - it will be deleted in original chunk, and also resulting merged chunk will either have this document marked deleted, or it will have no such document at all, if deletion happened on the early stage of merging.

With all these things in mind, quite actual question is 'how can I make backup copy of an index'? That is - one, consistent set of actual files without all 'semi-merged' or 'semi-saved' intermediate chunks.

## Freeze an index

Special command `freeze` will prepare your index for safe back-up. It will do following actions:

1. Disable optimization. All started operations will first finish.
2. Flush current RAM chunk into disk chunk.
3. Flush attributes.
4. Disable implicit operations which may change files on the disk.
5. Display actual list of the files belonging to the index.

```sql
freeze t;
+-------------------+---------------------------------+
| file              | normalized                      |
+-------------------+---------------------------------+
| data/t/t.0.spa    | /work/anytest/data/t/t.0.spa    |
| data/t/t.0.spd    | /work/anytest/data/t/t.0.spd    |
| data/t/t.0.spds   | /work/anytest/data/t/t.0.spds   |
| data/t/t.0.spe    | /work/anytest/data/t/t.0.spe    |
| data/t/t.0.sph    | /work/anytest/data/t/t.0.sph    |
| data/t/t.0.sphi   | /work/anytest/data/t/t.0.sphi   |
| data/t/t.0.spi    | /work/anytest/data/t/t.0.spi    |
| data/t/t.0.spm    | /work/anytest/data/t/t.0.spm    |
| data/t/t.0.spp    | /work/anytest/data/t/t.0.spp    |
| data/t/t.0.spt    | /work/anytest/data/t/t.0.spt    |
| data/t/t.meta     | /work/anytest/data/t/t.meta     |
| data/t/t.ram      | /work/anytest/data/t/t.ram      |
| data/t/t.settings | /work/anytest/data/t/t.settings |
+-------------------+---------------------------------+
13 rows in set (0.01 sec)
```
'file' column of `freeze` command output provides paths to files in daemon's datadir folder. 'Normalized' column gives same files with absolute paths. You don't need to deal with any kind of intermediate 'semi-merged', or 'semi-saved' states, just use this list of files and backup all of them.

When index is frozen, you can't perform `update` queries on it; they will fail with error message 'index is locked now,
try again later'.

Also, `delete` queries may be limited, and `replace` also (as they internally works as `delete` + `insert`). The limitation works this way:

* if `delete` affects a document stored in current RAM chunk - it is allowed.
* if `delete` affects a document in a disk chunk, but it was already deleted before - it is allowed.
* if `delete` is going to change actual disk chunk - it will be stalled.

Manual flushing of RAM chunk will report 'success', however actual save will not happen.

Drop/truncate of frozen index **IS** allowed, since such operation is not implicit. We imply, that if you truncate or drop an index - you don't need it backed up anyway.

Insertion into frozen index is supported, but limited: new data will be stored in RAM (as usual), until rt-mem-limit reached; then new insertions will stall until index is unfrozen.

If you shut down the daemon with frozen index, it will behave as in case of 'dirty' shutdown: new inserted data will NOT be saved into RAM-chunk, and on restart it will be restored from binlog (if any), or lost (if binlogging is disabled).

## Unfreeze an index

Unfreezing is much simpler; it just reenable previously blocked operations, and also restarts internal 'optimize' service. All possibly stalled operations will be recovered.

## Suggested back-up actions

1. Freeze an index
2. Grab output of 'freeze' command and backup provided files
3. Unfreeze an index

This may be done in some kind of automation scripts, like cronjobs, etc. However, notice, that as 'udate' command is discared on frozen indexes, that is not fully seamless, beware.