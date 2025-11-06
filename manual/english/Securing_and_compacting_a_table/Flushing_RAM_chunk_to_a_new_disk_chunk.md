# Flushing RAM chunk to a new disk chunk

## FLUSH RAMCHUNK

<!-- example flush_ramchunk -->

```sql
FLUSH RAMCHUNK rt_table
```

The `FLUSH RAMCHUNK` command creates a new disk chunk in an RT table.

Normally, an RT table automatically flushes and converts the contents of the RAM chunk into a new disk chunk when one of the [special conditions](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#RAM-chunk-flushing-conditions) is met. In some cases, though, you may want to trigger the flush manually — and the `FLUSH RAMCHUNK` statement lets you do that.

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

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "FLUSH RAMCHUNK rt;"
```
<!-- response JSON -->
```JSON
[
  {
    "total": 0,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->
<!-- proofread -->

