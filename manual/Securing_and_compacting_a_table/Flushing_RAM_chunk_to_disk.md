# Flushing RAM chunk to disk

## FLUSH TABLE

<!-- example flush_rtindex -->

```sql
FLUSH TABLE rtindex
```

`FLUSH TABLE` forcibly flushes RT table RAM chunk contents to disk.

Backing up an RT table is as simple as copying over its data files, followed by the [binary log](../Logging/Binary_logging.md). However, recovering from that backup means that all the transactions in the log since the last successful RAM chunk write would need to be replayed. Those writes normally happen either on a clean shutdown, or periodically with a (big enough!) interval between writes specified in  [rt_flush_period](../Server_settings/Searchd.md#rt_flush_period) directive. So such a backup made at an arbitrary point in time just might end up with way too much binary log data to replay.

`FLUSH TABLE` forcibly writes the RAM chunk contents to disk, and also causes the subsequent cleanup of (now redundant) binary log files. Thus,  recovering from a backup made just after `FLUSH TABLE` should be almost instant.


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
