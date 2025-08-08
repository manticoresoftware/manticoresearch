# Сброс фрагмента RAM на новый фрагмент диска

## FLUSH RAMCHUNK

<!-- example flush_ramchunk -->

```sql
FLUSH RAMCHUNK rt_table
```

Команда `FLUSH RAMCHUNK` создает новый фрагмент диска в RT-таблице.

Обычно RT-таблица автоматически сбрасывает и конвертирует содержимое фрагмента RAM в новый фрагмент диска, когда выполняется одно из [специальных условий](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#RAM-chunk-flushing-conditions). Однако в некоторых случаях вы можете захотеть инициировать сброс вручную — и команда `FLUSH RAMCHUNK` позволяет это сделать.

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

