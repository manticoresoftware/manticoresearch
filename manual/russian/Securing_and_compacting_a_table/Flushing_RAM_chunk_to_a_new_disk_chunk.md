# Сброс чанк RAM в новый чанк на диске

## FLUSH RAMCHUNK

<!-- example flush_ramchunk -->

```sql
FLUSH RAMCHUNK rt_table
```

Команда `FLUSH RAMCHUNK` создаёт новый чанк на диске в RT-таблице.

Обычно RT-таблица автоматически сбрасывает и преобразует содержимое чанка RAM в новый чанк на диске, когда выполняется одно из [специальных условий](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#RAM-chunk-flushing-conditions). Однако в некоторых случаях вы можете захотеть вызвать сброс вручную — и инструкция `FLUSH RAMCHUNK` позволяет это сделать.

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

