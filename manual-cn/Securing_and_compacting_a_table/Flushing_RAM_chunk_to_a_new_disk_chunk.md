# 将RAM块刷新到新磁盘块

## FLUSH RAMCHUNK

<!-- example flush_ramchunk -->

```sql
FLUSH RAMCHUNK rt_table
```

`FLUSH RAMCHUNK` 命令在 RT 表中创建一个新的磁盘块。

通常，当 RAM 块达到最大允许的 [rt_mem_limit](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit) 大小时，RT 表会自动刷新并将 RAM 块的内容转换为新的磁盘块。然而，出于调试和测试的目的，强制创建新的磁盘块可能是有用的，而 `FLUSH RAMCHUNK` 语句正是用于实现这一点。

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
