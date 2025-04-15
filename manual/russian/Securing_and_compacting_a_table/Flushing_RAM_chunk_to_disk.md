# Сброс RAM чанка на диск

## СБРОС ТАБЛИЦЫ

<!-- example flush_rtindex -->

```sql
FLUSH TABLE rt_table
```

`FLUSH TABLE` принудительно сбрасывает содержимое RT таблицы в RAM на диск.

Реальная таблица [RAM чанк](../../Creating_a_table/Local_tables/Real-time_table.md#Real-time-table-files-structure) автоматически сбрасывается на диск во время корректного завершения работы, или периодически каждые [rt_flush_period](../Server_settings/Searchd.md#rt_flush_period) секунд.

Выполнение команды `FLUSH TABLE` не только принудительно записывает содержимое RAM чанка на диск, но также инициирует очистку бинарных файлов журнала.

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
