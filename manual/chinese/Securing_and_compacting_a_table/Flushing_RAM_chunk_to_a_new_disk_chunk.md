# 将 RAM chunk 刷写到新的磁盘 chunk

## FLUSH RAMCHUNK

<!-- example flush_ramchunk -->

```sql
FLUSH RAMCHUNK rt_table
```

`FLUSH RAMCHUNK` 命令会在 RT 表中创建一个新的磁盘 chunk。

通常，RT 表会在满足[特殊条件](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#ram-chunk-flushing-conditions)中的某一项时，自动将 RAM chunk 的内容刷写并转换为新的磁盘 chunk。但在某些情况下，你可能希望手动触发刷写——此时 `FLUSH RAMCHUNK` 语句允许你这样做。

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

