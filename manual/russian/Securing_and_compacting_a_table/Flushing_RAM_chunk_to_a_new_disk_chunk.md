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

<!--
data for the following example:

DROP TABLE IF EXISTS rt;
CREATE TABLE rt(title text);
INSERT INTO rt(id,title) VALUES (1,'doc');
-->

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
POST /sql?mode=raw -d "FLUSH RAMCHUNK rt"
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

