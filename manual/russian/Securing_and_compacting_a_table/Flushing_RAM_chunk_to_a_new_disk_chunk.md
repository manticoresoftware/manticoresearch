# Сброс RAM-чанка на новый диск-чанк

## СБРОС RAM-ЧАНКА

<!-- example flush_ramchunk -->

```sql
FLUSH RAMCHUNK rt_table
```

Команда `FLUSH RAMCHUNK` создает новый диск-чанк в RT таблице.

Обычно RT таблица автоматически сбрасывает и преобразует содержимое RAM-чанка в новый диск-чанк, как только RAM-чанк достигает максимального разрешенного размера [rt_mem_limit](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit). Тем не менее, для целей отладки и тестирования может быть полезно принудительно создать новый диск-чанк, и оператор `FLUSH RAMCHUNK` делает именно это.

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
