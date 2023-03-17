# Freezing a table

<!-- example freeze -->

```sql
FREEZE tbl1[, tbl2, ...]
```

`FREEZE` readies a real-time/plain table for a secure [backup](../Securing_and_compacting_a_table/Backup_and_restore.md). Specifically, it:
1. Deactivates table compaction. If the table is currently being compacted, `FREEZE` will wait for completion.
2. Transfers the current RAM chunk to a disk chunk.
3. Flushes attributes.
4. Disables implicit operations that could modify the disk files.
5. Shows the actual file list associated with the table.

The built-in tool [manticore-backup](../Securing_and_compacting_a_table/Backup_and_restore.md) uses `FREEZE` to ensure data consistency. You can do the same if you want to create your own backup solution or need to freeze tables for other reasons. Just follow these steps:
1. `FREEZE` a table (or a few).
2. Capture the output of the `FREEZE` command and back up the specified files.
3. `UNFREEZE` the table(s) once finished.

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

The `file` column indicates the table's file paths within the [data_dir](../Server_settings/Searchd.md#data_dir) of the running instance. The `normalized` column displays the absolute paths for the same files. To back up a table, simply copy the provided files without additional preparation.

When a table is frozen, you cannot execute `UPDATE` queries; they will fail with the error message "index is locked now, try again later."

Also, `DELETE` and `REPLACE` queries have some restrictions while the table is frozen:
* If `DELETE` affects a document in the current RAM chunk - it is permitted.
* If `DELETE` impacts a document in a disk chunk but was previously deleted - it is allowed.
* If `DELETE` would alter an actual disk chunk - it will wait until the table is unfrozen.

Manually `FLUSH`ing a RAM chunk of a frozen table will report 'success', but no real saving will occur.

`DROP`/`TRUNCATE` of a frozen table **is** allowed since these operations are not implicit. We assume that if you truncate or drop a table, you don't need it backed up; therefore, it should not have been frozen initially.

`INSERT`ing into a frozen table is supported but limited: new data will be stored in RAM (as usual) until `rt_mem_limit` is reached; then, new insertions will wait until the table is unfrozen.

If you shut down the daemon with a frozen table, it will act as if it experienced a dirty shutdown (e.g., `kill -9`): newly inserted data will **not** be saved in the RAM-chunk on disk, and upon restart, it will be restored from a binary log (if any) or lost (if binary logging is disabled).

# Unfreezing a table

<!-- example unfreeze -->

```sql
UNFREEZE tbl1[, tbl2, ...]
```

`UNFREEZE` reactivates previously blocked operations and resumes the internal compaction service. All operations waiting for a table to unfreeze will also be unfrozen and complete normally.

<!-- request Example -->
```sql
UNFREEZE tbl;
```

<!-- end -->

<!-- proofread -->
