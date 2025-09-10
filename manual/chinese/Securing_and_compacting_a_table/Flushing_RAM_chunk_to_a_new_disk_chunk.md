# 将 RAM 块刷新到新的磁盘块

## FLUSH RAMCHUNK

<!-- example flush_ramchunk -->

```sql
FLUSH RAMCHUNK rt_table
```

`FLUSH RAMCHUNK` 命令在 RT 表中创建一个新的磁盘块。

通常，当满足[特殊条件](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#RAM-chunk-flushing-conditions)之一时，RT 表会自动将 RAM 块的内容刷新并转换为新的磁盘块。但在某些情况下，您可能希望手动触发刷新——这时 `FLUSH RAMCHUNK` 语句就能满足您的需求。

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

