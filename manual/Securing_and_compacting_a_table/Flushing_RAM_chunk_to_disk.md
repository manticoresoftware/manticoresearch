# Flushing RAM chunk to disk

## FLUSH TABLE

<!-- example flush_rtindex -->

```sql
FLUSH TABLE rt_table
```

`FLUSH TABLE` forcefully flushes RT table RAM chunk contents to disk.

The real-time table [RAM chunk](../../Creating_a_table/Local_tables/Real-time_table.md#Real-time-table-files-structure) is automatically flushed to disk during a clean shutdown, or periodically every [rt_flush_period](../Server_settings/Searchd.md#rt_flush_period) seconds.

Issuing a `FLUSH TABLE` command not only forces the RAM chunk contents to be written to disk but also triggers the cleanup of binary log files.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
FLUSH TABLE rt;
```
<!-- response mysql -->
```sql
Query OK, 0 rows affected (0.05 sec)
```
<!-- end -->

<!-- proofread -->
