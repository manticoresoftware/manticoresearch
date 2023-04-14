# Binary logging

Binary logging serves as a recovery mechanism for [Real-Time](../Creating_a_table/Local_tables/Real-time_table.md) table data, as well as attribute updates for plain tables that would otherwise only be stored in RAM until a flush occurs. When binary logs are enabled, `searchd` records each transaction to the binlog file and utilizes it for recovery following an unclean shutdown. During a clean shutdown, RAM chunks are saved to disk, and all binlog files are subsequently unlinked.

## Disabling binary logging

By default, binary logging is enabled. On Linux systems, the default location for `binlog.*` files is `/var/lib/manticore/data/`.
In [RT mode](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29), binary logs are stored in the `data_dir` folder, unless specified otherwise.

To disable binary logging, set `binlog_path` to empty:

```ini
searchd {
...
    binlog_path = # disable logging
...
```
Disabling binary logging can lead to better performance for Real-Time tables, but it also puts their data at risk.

You can use the following directive to set a custom path:

```ini
searchd {
...
    binlog_path = /var/data
...
```

## Operations

When logging is enabled, each transaction committed to an RT table is written to a log file. After an unclean shutdown, logs are automatically replayed upon startup, recovering any logged changes.

### Log size
During normal operation, a new binlog file is opened whenever the `binlog_max_log_size` limit is reached. Older, closed binlog files are retained until all transactions stored in them (from all tables) are flushed as a disk chunk. Setting the limit to 0 essentially prevents the binlog from being unlinked while `searchd` is running; however, it will still be unlinked upon a clean shutdown. By default, there is no limit to the log file size.

```ini
binlog_max_log_size = 16M
```

### Binary flushing strategies

There are 3 different binlog flushing strategies, controlled by the `binlog_flush` directive:

* 0 - Flush and sync every second. This provides the best performance, but up to 1 second's worth of committed transactions may be lost in the event of a server crash or an OS/hardware crash.
* 1 - Flush and sync every transaction. This has the worst performance, but guarantees that every committed transaction's data is saved.
* 2 - Flush every transaction and sync every second. This offers good performance, and every committed transaction is guaranteed to be saved in the case of a server crash. However, up to 1 second's worth of committed transactions may be lost in the event of an OS/hardware crash.

The default mode is to flush every transaction and sync every second (mode 2).

```ini
searchd {
...
    binlog_flush = 1 # ultimate safety, low speed
...
}
```

### Recovery

During recovery after an unclean shutdown, binlogs are replayed, and all logged transactions since the last good on-disk state are restored. Transactions are checksummed, so in case of binlog file corruption, garbage data will **not** be replayed; such a broken transaction will be detected and will stop the replay.


### Flushing RT RAM chunks

Intensive updates to a small RT table that fully fits into a RAM chunk can result in an ever-growing binlog that can never be unlinked until a clean shutdown. Binlogs essentially serve as append-only deltas against the last known good saved state on disk, and they cannot be unlinked unless the RAM chunk is saved. An ever-growing binlog is not ideal for disk usage and crash recovery time. To address this issue, you can configure `searchd` to perform periodic RAM chunk flushes using the `rt_flush_period` directive. With periodic flushes enabled, `searchd` will maintain a separate thread that checks whether RT table RAM chunks need to be written back to disk. Once this occurs, the respective binlogs can be (and are) safely unlinked.

```ini
searchd {
...
    rt_flush_period = 3600 # 1 hour
...
}
```
The default RT flush period is set to 10 hours.

It's important to note that `rt_flush_period` only controls the frequency at which *checks* occur. There are no *guarantees* that a specific RAM chunk will be saved. For example, it doesn't make sense to regularly re-save a large RAM chunk that only receives a few rows worth of updates. Manticore automatically determines whether to perform the flush using a few heuristics.

<!-- proofread -->