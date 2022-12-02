# Freezing a table

<!-- example freeze -->

`FREEZE` prepares a real-time/plain table for a safe [backup](../Securing_and_compacting_a_table/Backup_and_restore.md). In particular it:
1. Disables table compaction. If the table is being compacted right now `FREEZE` will wait for it to finish.
2. Flushes current RAM chunk into a disk chunk.
3. Flushes attributes.
4. Disables implicit operations that may change the files on disk.
5. Displays actual list of the files belonging to the table.

Built-in tool [manticore-backup](../Securing_and_compacting_a_table/Backup_and_restore.md) uses `FREEZE` to guarantee data consistency. So can you if you want to make your own backup solution or need to freeze tables for whatever else reason. All you need to do is:
1. `FREEZE` a table.
2. Grab output of the `FREEZE` command and backup the provided files.
3. `UNFREEZE` the table once you are done.

<!-- request Example -->
```sql
FREEZE t;
```

<!-- response Example -->
```sql
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

<!-- end -->

The column `file` provides paths to the table's files inside [data_dir](../Server_settings/Searchd.md#data_dir) of the running instance. The column `normalized` shows absolute paths of the same files. If you want to back up a table it's safe to just copy the provided files with no other preparations.

When a table is frozen, you can't perform `UPDATE` queries on it; they will fail with the error message `index is locked now, try again later`.

Also, `DELETE` and `REPLACE` queries have some limitations while the table is frozen:
* If `DELETE` affects a document stored in a current RAM chunk - it is allowed.
* If `DELETE` affects a document in a disk chunk, but it was already deleted before - it is allowed.
* If `DELETE` is going to change an actual disk chunk - it will wait until the table is unfrozen.

Manual `FLUSH` of a RAM chunk of a frozen table will report 'success', however no actual save will happen.

`DROP`/`TRUNCATE` of a frozen table **is** allowed, since such operation is not implicit. We assume that if you truncate or drop a table - you don't need it backed up anyway, therefore it should not have been frozen in the first place.

`INSERT` into a frozen table is supported, but also limited: new data will be stored in RAM (as usual), until `rt_mem_limit` is reached; then new insertions will wait until the table is unfrozen.

If you shut down the daemon with a frozen table, it will behave as in case of a dirty shutdown (e.g. `kill -9`): new inserted data will **not** be saved in the RAM-chunk on disk, and on restart it will be restored from a binary log (if any), or lost (if binary logging is disabled).

# Unfreezing a table

<!-- example unfreeze -->
`UNFREEZE` re-enables previously blocked operations and restarts the internal compaction service. All the operations that are waiting for a table unfreeze get unfrozen too and finish their operations normally.

<!-- request Example -->
```sql
UNFREEZE tbl;
```

<!-- end -->
