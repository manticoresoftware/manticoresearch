# Binary logging

Binary logging serves as a recovery mechanism for [real-time](../Creating_a_table/Local_tables/Real-time_table.md) table data, as well as attribute updates for [plain tables](../Creating_a_table/Local_tables/Plain_table.md#Plain-table) that would otherwise only be stored in RAM until a flush occurs. When binary logs are enabled, `searchd` records each transaction to the binlog file and utilizes it for recovery following an unclean shutdown. During a clean shutdown, RAM chunks are saved to disk, and all binlog files are subsequently deleted.

## Enabling and disabling binary logging

By default, binary logging is enabled to safeguard data integrity. On Linux systems, the default location for `binlog.*` files in [Plain mode](../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29) is `/var/lib/manticore/data/`. In [RT mode](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29), binary logs are stored in the `<data_dir>/binlog/` folder, unless specified otherwise.

### Global binary logging configuration

<!-- example binlog_path -->
To disable binary logging globally, set `binlog_path` to an empty value in the `searchd` configuration.
Disabling binary logging requires a restart of the daemon and puts data at risk if the system shuts down unexpectedly.

<!-- request Example -->
```ini
searchd {
...
    binlog_path = # disable logging
...
```
<!-- end -->

<!-- example binlog_path2 -->
You can use the following directive to set a custom path:

<!-- request Example -->
```ini
searchd {
...
    binlog_path = /var/data
...
```
<!-- end -->

### Per-table binary logging configuration

<!-- Example binlog0 -->
For more granular control, binary logging can be disabled at the table level for real-time tables by setting the `binlog` table parameter to `0`. This option is not available for percolate tables.

<!-- request Example -->
```sql
create table a (id bigint, s string attribute) binlog='0';
```
<!-- end -->

<!-- Example binlog_alter -->
For existing RT tables, binary logging can also be disabled by modifying the `binlog` parameter.

<!-- request Example -->
```sql
alter table FOO binlog='0';
```
<!-- end -->

<!-- Example binlog_alter2 -->
If binary logging was previously disabled, it can be re-enabled by setting the `binlog` parameter back to `1`:

<!-- request Example -->
```sql
alter table FOO binlog='1';
```
<!-- end -->

#### Important considerations:
* **Dependency on global settings**: per-table binary logging settings only take effect if binary logging is globally enabled in the searchd configuration (`binlog_path` must not be empty).
* **Forced flushing and transaction ID reset**: Modifying the binary logging status of a table forces an immediate [flush of the table](../Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#FLUSH-TABLE). This operation resets the table's transaction ID to `-1` and updates the table's metadata to reflect this change.

## Operations

When binary logging is turned on, every change made to an RT table is saved to a log file. If the system shuts down unexpectedly, these logs are used automatically when the system starts again to bring back all the changes that were logged.

### Log size

<!-- Example binlog_max_log_size -->
During normal operations, when the amount of data logged reaches a certain limit (set by `binlog_max_log_size`), a new log file starts. Old log files are kept until all changes in them are completely processed and saved to disk as a disk chunk. If this limit is set to `0`, the log files are kept until the system is properly shut down. By default, there's no limit to how large these files can grow.

<!-- request Example -->

```ini
searchd {
...
    binlog_max_log_size = 16M
....
```

<!-- end -->

### Log files

<!-- example binlog_filename_digits -->

Each binlog file is named with a zero-padded number, like `binlog.0000`, `binlog.0001`, etc., typically showing four digits. You can change how many digits the number has with the setting `binlog_filename_digits`. If you have more binlog files than the number of digits can accommodate, the number of digits will be automatically increased to fit all files.

**Important**: To change the number of digits, you must first save all table data and properly shut down the system. Then, delete the old log files and restart the system.

<!-- request Example -->
```ini
searchd {
...
    binlog_filename_digits = 6
...
```
<!-- end -->

### Binary logging strategies

<!-- Example binlog_common -->
You can choose between two ways to manage binary log files, which can be set with the `binlog_common` directive:
* Separate file for each table (default, `0`): Each table saves its changes in its own log file. This setup is good if you have many tables that get updated at different times. It allows tables to be updated without waiting for others. Also, if there is a problem with one table's log file, it does not affect the others.
* Single file for all tables (`1`): All tables use the same binary log file. This method makes it easier to handle files because there are fewer of them. However, this could keep files longer than needed if one table still needs to save its updates. This setting might also slow things down if many tables need to update at the same time because all changes have to wait to be written to one file.

<!-- request binlog_common -->

```ini
searchd {
...
    binlog_common = 1
...
```
<!-- end -->

### Binary flushing strategies

<!-- Example binlog_flush -->
There are 3 different binlog flushing strategies, controlled by the `binlog_flush` directive:

* `0` - Flush and sync every second. This provides the best performance, but up to 1 second's worth of committed transactions may be lost in the event of a server crash or an OS/hardware crash.
* `1` - Flush and sync every transaction. This has the worst performance, but guarantees that every committed transaction's data is saved.
* `2` - Flush every transaction and sync every second. This offers good performance, and every committed transaction is guaranteed to be saved in the case of a server crash. However, up to 1 second's worth of committed transactions may be lost in the event of an OS/hardware crash.

The default mode is to flush every transaction and sync every second (mode `2`).

<!-- request Example -->
```ini
searchd {
...
    binlog_flush = 1 # ultimate safety, low speed
...
}
```
<!-- end -->

### Recovery

During recovery after an unclean shutdown, binlogs are replayed, and all logged transactions since the last good on-disk state are restored. Transactions are checksummed, so in case of binlog file corruption, garbage data will **not** be replayed; such a broken transaction will be detected and will stop the replay.

### Flushing RT RAM chunks

<!-- Example rt_flush_period -->
Intensive updates to a small RT table that fully fits into a RAM chunk can result in an ever-growing binlog that can never be unlinked until a clean shutdown. Binlogs essentially serve as append-only deltas against the last known good saved state on disk, and they cannot be unlinked unless the RAM chunk is saved. An ever-growing binlog is not ideal for disk usage and crash recovery time. To address this issue, you can configure `searchd` to perform periodic RAM chunk flushes using the `rt_flush_period` directive. With periodic flushes enabled, `searchd` will maintain a separate thread that checks whether RT table RAM chunks need to be written back to disk. Once this occurs, the respective binlogs can be (and are) safely unlinked.

The default RT flush period is set to 10 hours.

<!-- request Example -->
```ini
searchd {
...
    rt_flush_period = 3600 # 1 hour
...
}
```
<!-- end -->

It's important to note that `rt_flush_period` only controls the frequency at which checks occur. There are no guarantees that a specific RAM chunk will be saved. For example, it doesn't make sense to regularly re-save a large RAM chunk that only receives a few rows worth of updates. Manticore automatically determines whether to perform the flush using a few heuristics.

<!-- proofread -->
