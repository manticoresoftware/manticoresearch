# Запись куска RAM в файл

## FLUSH TABLE

<!-- example flush_rtindex -->

```sql
FLUSH TABLE rt_table
```

`FLUSH TABLE` принудительно записывает содержимое RAM-куска таблицы RT на диск.

Кусок RAM реального времени таблицы [RT](../Creating_a_table/Local_tables/Real-time_table.md#Real-time-table-files-structure) автоматически сбрасывается на диск во время корректного завершения работы или периодически каждые [rt_flush_period](../Server_settings/Searchd.md#rt_flush_period) секунд.

Выполнение команды `FLUSH TABLE` не только принудительно записывает содержимое RAM-куска на диск, но и запускает очистку бинарных логов.

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

