# Flushing RAM chunk to a new disk chunk

## FLUSH RAMCHUNK

<!-- example flush_ramchunk -->

```sql
FLUSH RAMCHUNK rt_table
```

The `FLUSH RAMCHUNK` command creates a new disk chunk in an RT table.

Normally, an RT table would automatically flush and convert the contents of the RAM chunk into a new disk chunk once the RAM chunk reaches the maximum allowed [rt_mem_limit](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit) size. However, for debugging and testing purposes, it might be useful to forcibly create a new disk chunk, and the `FLUSH RAMCHUNK` statement does exactly that.

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
