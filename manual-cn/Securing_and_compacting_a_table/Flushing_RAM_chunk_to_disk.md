# 将实时表刷新到磁盘

## FLUSH TABLE

<!-- example flush_rtindex -->

```sql
FLUSH TABLE rt_table
```

`FLUSH TABLE` 强制将 RT 表的 RAM 块内容刷新到磁盘。

实时表的 [RAM 块](../../Creating_a_table/Local_tables/Real-time_table.md#Real-time-table-files-structure) 在正常关闭时会自动刷新到磁盘，或每隔 [rt_flush_period](../Server_settings/Searchd.md#rt_flush_period) 秒定期刷新。

发出 `FLUSH TABLE` 命令不仅强制将 RAM 块的内容写入磁盘，还会触发二进制日志文件的清理。

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
