# Binary logging

Binary logging serves as a recovery mechanism for [Real-Time](../Creating_a_table/Local_tables/Real-time_table.md) table data, as well as attribute updates for plain tables that would otherwise only be stored in RAM until a flush occurs. When binary logs are enabled, `searchd` records each transaction to the binlog file and utilizes it for recovery following an unclean shutdown. During a clean shutdown, RAM chunks are saved to disk, and all binlog files are subsequently unlinked.

## Disabling binary logging

By default, binary logging is enabled. On Linux systems, the default location for `binlog.*` files is `/var/lib/manticore/data/`.
In [RT mode](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29), binary logs are stored in the `data_dir` folder, unless specified otherwise.

### Global scope

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

Notice, that disabling/enabling binary logging implies daemon's restart.

### Per-table scope

You can disable binary logging on table level, by set `binlog` param to '0'. That works specifically for real-time tables, and doesn't work for percolates.

You can do it during table creation:

```sql
create table a (id bigint, s string attribute) binlog='0';
```

Also, you can disable binary logging for existing table:

```sql
alter table FOO binlog='0';
```

To enable binary logging, alter param `binlog` to '1':

```sql
alter table FOO binlog='1';
```

Notice, per-table enabling/disabling works only when global-scope binlogging (i.e. binlog path in the config) is enabled.
When you change table's binlogging state, it will be forcibly saved (aka `flush rtindex...`), and then table's transaction ID will be set to -1 and stored in the table's meta.


## Operations

When logging is enabled, each transaction committed to an RT table is written to a log file. After an unclean shutdown, logs are automatically replayed upon startup, recovering any logged changes.

### Log size
During normal operation, a new binlog file is opened whenever the `binlog_max_log_size` limit is reached. Older, closed binlog files are retained until all transactions stored in them (from all tables) are flushed as a disk chunk. Setting the limit to 0 essentially prevents the binlog from being unlinked while `searchd` is running; however, it will still be unlinked upon a clean shutdown. By default, there is no limit to the log file size.

```ini
binlog_max_log_size = 16M
```

### Log files

Each binlog file has a name, as binlog.NNNN, where NNNN is a number, zero padded. By default, it is 4 digits, so binlog files are named like 'binlog.0000', 'binlog.0001', etc. If desired, number of digits can be set with `binlog_filename_digits` directive:

```ini
binlog_filename_digits = 6
```

Notice, you need to abandon your binlog before change the value; otherwise actual one will be taken from existing binlog meta. So, if you want to change, N of digits, say, to 6 - after you put your value to config, you need to flush all your tables; than perform clean shutdown of the daemon; than manually drop all binlog.* files and restart the daemon.

### Binary logging strategies

There are 2 different binlog strategies, controlled by `binlog_common` directive:

* 0 - Provide separate binlog file for each table. That is default value.
* 1 - Provide one single binlog for all the tables. Only one single files is opened for binlogging all the tables.

```ini
binlog_common = 1
```

If directive `binlog_common` is absent, default value is taken from env variable `MANTICORE_BINLOG_COMMON`. If that variable is also absent/not set, default value is 0, i.e. 'provide separate binlog files for each table'.

One single binlog for all the tables was default option during the years. It keeps a few binlog files which are easy to manually maintain when necessary. On the other side, when you have several tables with random insert/flush behavior, binlog files will persist until all participating tables flushed. Even if you totally drop a table, it's data will live in binlog, if another tables are still 'dirty'. Also, writing to binlog is serialized with separate transactions. If you change several tables simultaneously, all the changes will be sequenced to binlog.

Separate binlog for each table keep 1 opened file for each table (it maybe sensible for ones struggling with low ulimit). With separate binlogs several tables may write their changes simultaneously (however parallel changes of a one single table still be serialized with the binlog serving the table). Also flushing/dropping a table will cause it's binlog to be abandoned and deleted immediately; as it is not shared with another tables. Also, in case of unclean shutdown, even if a binlog file is damaged, it will affect only one table and will not ruine others.


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
