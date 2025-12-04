# Сброс чанка ОЗУ в новый дисковый чанк

## FLUSH RAMCHUNK

<!-- example flush_ramchunk -->

```sql
FLUSH RAMCHUNK rt_table
```

Команда `FLUSH RAMCHUNK` создает новый дисковый чанк в RT таблице.

Обычно RT таблица автоматически сбрасывает и конвертирует содержимое чанка ОЗУ в новый дисковый чанк, когда выполняется одно из [специальных условий](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#RAM-chunk-flushing-conditions). Однако в некоторых случаях вы можете захотеть инициировать сброс вручную — и оператор `FLUSH RAMCHUNK` позволяет сделать это.

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

