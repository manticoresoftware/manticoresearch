# 将 RAM 块刷新到新的磁盘块

## FLUSH RAMCHUNK

<!-- example flush_ramchunk -->

```sql
FLUSH RAMCHUNK rt_table
```

`FLUSH RAMCHUNK` 命令会在 RT 表中创建一个新的磁盘块。

通常，当满足[特殊条件](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#RAM-chunk-flushing-conditions)之一时，RT 表会自动刷新并将 RAM 块的内容转换为新的磁盘块。但在某些情况下，你可能希望手动触发刷新——`FLUSH RAMCHUNK` 语句允许你这样做。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
FLUSH RAMCHUNK rt;
```
<!-- response mysql -->
```sql
Query OK, 0 rows affected (0.05 sec)
```
<!-- end -->
<!-- proofread -->

