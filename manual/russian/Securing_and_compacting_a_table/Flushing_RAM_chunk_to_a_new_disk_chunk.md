# Сброс RAM chunk в новый disk chunk

## FLUSH RAMCHUNK

<!-- example flush_ramchunk -->

```sql
FLUSH RAMCHUNK rt_table
```

Команда `FLUSH RAMCHUNK` создает новый disk chunk в RT таблице.

Обычно RT таблица автоматически сбрасывает и конвертирует содержимое RAM chunk в новый disk chunk, когда выполняется одно из [специальных условий](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#ram-chunk-flushing-conditions). Однако в некоторых случаях может потребоваться вручную инициировать сброс — и оператор `FLUSH RAMCHUNK` позволяет сделать это.

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

