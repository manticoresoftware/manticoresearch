# 刷新RAM块到磁盘

## 刷新表

<!-- example flush_rtindex -->

```sql
FLUSH TABLE rt_table
```

`FLUSH TABLE` 强制将RT表RAM块内容刷新到磁盘。

实时表 [RAM块](../../Creating_a_table/Local_tables/Real-time_table.md#Real-time-table-files-structure) 在干净关机期间自动刷新到磁盘，或者每隔 [rt_flush_period](../Server_settings/Searchd.md#rt_flush_period) 秒定期刷新。

发出 `FLUSH TABLE` 命令不仅强制将RAM块内容写入磁盘，还触发二进制日志文件的清理。

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
