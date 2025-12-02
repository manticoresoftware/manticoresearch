# Сброс чанка RAM в файл

## FLUSH TABLE

<!-- example flush_rtindex -->

```sql
FLUSH TABLE rt_table
```

`FLUSH TABLE` принудительно сбрасывает содержимое RAM чанка RT-таблицы на диск.

Чанк памяти реального времени [RAM chunk](../Creating_a_table/Local_tables/Real-time_table.md#Real-time-table-files-structure) автоматически сбрасывается на диск при корректном завершении работы или периодически каждые [rt_flush_period](../Server_settings/Searchd.md#rt_flush_period) секунд.

Выполнение команды `FLUSH TABLE` не только принудительно записывает содержимое RAM чанка на диск, но и инициирует очистку файлов бинарного лога.

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

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "FLUSH TABLE rt;"
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

