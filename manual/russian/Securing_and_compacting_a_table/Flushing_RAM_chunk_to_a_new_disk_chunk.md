# Сброс RAM-чанка в новый дисковый чанк

## FLUSH RAMCHUNK

<!-- example flush_ramchunk -->

```sql
FLUSH RAMCHUNK rt_table
```

Команда `FLUSH RAMCHUNK` создает новый дисковый чанк в RT-таблице.

Обычно RT-таблица автоматически сбрасывает и преобразует содержимое RAM-чанка в новый дисковый чанк при выполнении одного из [специальных условий](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#RAM-chunk-flushing-conditions). Однако в некоторых случаях может потребоваться вручную инициировать сброс — и оператор `FLUSH RAMCHUNK` позволяет это сделать.

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

