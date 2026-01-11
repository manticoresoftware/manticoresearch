# SHOW TABLE STATUS

<!-- example SHOW TABLE STATUS -->

`SHOW TABLE STATUS` — это оператор SQL, который отображает различные статистические данные для каждой таблицы.

Синтаксис:

```sql
SHOW TABLE table_name STATUS
```

В зависимости от типа индекса отображаемая статистика включает разный набор строк:

* **template**: `index_type`.
* **distributed**: `index_type`, `query_time_1min`, `query_time_5min`,`query_time_15min`,`query_time_total`, `exact_query_time_1min`, `exact_query_time_5min`, `exact_query_time_15min`, `exact_query_time_total`, `found_rows_1min`, `found_rows_5min`, `found_rows_15min`, `found_rows_total`.
* **percolate**: `index_type`, `stored_queries`, `ram_bytes`, `disk_bytes`, `max_stack_need`, `average_stack_base`, `
  desired_thread_stack`, `tid`, `tid_saved`, `query_time_1min`, `query_time_5min`,`query_time_15min`,`query_time_total`, `exact_query_time_1min`, `exact_query_time_5min`, `exact_query_time_15min`, `exact_query_time_total`, `found_rows_1min`, `found_rows_5min`, `found_rows_15min`, `found_rows_total`.
* **plain**: `index_type`, `indexed_documents`, `indexed_bytes`, возможно набор `field_tokens_*` и `total_tokens`, `ram_bytes`, `disk_bytes`, `disk_mapped`, `disk_mapped_cached`, `disk_mapped_doclists`, `disk_mapped_cached_doclists`, `disk_mapped_hitlists`, `disk_mapped_cached_hitlists`, `killed_documents`, `killed_rate`, `query_time_1min`, `query_time_5min`,`query_time_15min`,`query_time_total`, `exact_query_time_1min`, `exact_query_time_5min`, `exact_query_time_15min`, `exact_query_time_total`, `found_rows_1min`, `found_rows_5min`, `found_rows_15min`, `found_rows_total`.
* **rt**: `index_type`, `indexed_documents`, `indexed_bytes`, возможно набор `field_tokens_*` и `total_tokens`, `ram_bytes`, `disk_bytes`, `disk_mapped`, `disk_mapped_cached`, `disk_mapped_doclists`, `disk_mapped_cached_doclists`, `disk_mapped_hitlists`, `disk_mapped_cached_hitlists`, `killed_documents`, `killed_rate`, `ram_chunk`, `ram_chunk_segments_count`, `disk_chunks`, `mem_limit`, `mem_limit_rate`, `ram_bytes_retired`, `optimizing`, `locked`, `tid`, `tid_saved`, `query_time_1min`, `query_time_5min`,`query_time_15min`,`query_time_total`, `exact_query_time_1min`, `exact_query_time_5min`, `exact_query_time_15min`, `exact_query_time_total`, `found_rows_1min`, `found_rows_5min`, `found_rows_15min`, `found_rows_total`.

Вот что означают эти значения:

* `index_type`: в настоящее время один из `disk`, `rt`, `percolate`, `template` и `distributed`.
* `indexed_documents`: количество индексированных документов.
* `indexed_bytes`: общий размер индексированного текста. Обратите внимание, что это значение не является строгим, поскольку в полнотекстовом индексе невозможно строго восстановить хранимый текст для его измерения.
* `stored_queries`: количество перколяторных запросов, хранящихся в таблице.
* `field_tokens_XXX`: необязательно, общая длина по полям (в токенах) по всей таблице (используется внутренне для функций ранжирования `BM25A` и `BM25F`). Доступно только для таблиц, созданных с `index_field_lengths=1`.
* `total_tokens`: необязательно, общий суммарный показатель всех `field_tokens_XXX`.
* `ram_bytes`: общий объем оперативной памяти, занятой таблицей.
* `disk_bytes`: общий объем дискового пространства, занятого таблицей.
* `disk_mapped`: общий размер отображаемых файлов.
* `disk_mapped_cached`: общий размер отображаемых файлов, фактически кэшированных в ОЗУ.
* `disk_mapped_doclists` и `disk_mapped_cached_doclists`: часть от общих и кэшированных отображений, принадлежащих спискам документов.
* `disk_mapped_hitlists` и `disk_mapped_cached_hitlists`: часть от общих и кэшированных отображений, принадлежащих спискам попаданий. Значения doclists и hitlists показываются отдельно, поскольку они обычно крупные (например, около 90% от всего размера таблицы).
* `killed_documents` и `killed_rate`: первое показывает количество удаленных документов, а второе — отношение удаленных к индексированным. Технически удаление документа означает подавление его в результатах поиска, но он физически всё ещё существует в таблице и будет удален только после слияния/оптимизации таблицы.
* `ram_chunk`: размер оперативной памяти, занятой сегментом таблицы реального времени или перколяторной таблицы.
* `ram_chunk_segments_count`: сегмент оперативной памяти внутренне состоит из сегментов, обычно не более 32. Эта строка показывает их текущее количество.
* `disk_chunks`: количество дисковых сегментов в таблице реального времени.
* `mem_limit`: фактическое значение `rt_mem_limit` для таблицы.
* `mem_limit_rate`: отношение, при котором сегмент оперативной памяти будет сброшен как дисковый сегмент, например, если `rt_mem_limit` составляет 128M и отношение 50%, новый дисковый сегмент будет сохранен при превышении сегментом оперативной памяти 64M.
* `ram_bytes_retired`: представляет размер мусора в сегментах оперативной памяти (например, удаленные или заменённые документы, еще не окончательно удалённые).
* `optimizing`: значение больше 0 указывает, что таблица в настоящее время оптимизируется (т.е. сейчас происходит слияние некоторых дисковых сегментов).
* `locked`: значение больше 0 указывает, что таблица в настоящее время заблокирована с помощью [FREEZE](../../Securing_and_compacting_a_table/Freezing_and_locking_a_table.md#Freezing-a-table). Число показывает, сколько раз таблица была заморожена. Например, таблица может быть заморожена `manticore-backup`, а затем снова заморожена репликацией. Полное размораживание происходит только тогда, когда ни один процесс больше не требует заморозки.
* `max_stack_need`: объем стека, необходимый для расчёта самых сложных из сохранённых перколяторных запросов. Это динамическое значение, зависит от деталей сборки, таких как компилятор, оптимизация, оборудование и т.д.
* `average_stack_base`: объем стека, который обычно занят в начале вычисления перколяторного запроса.
* `desired_thread_stack`: сумма вышеперечисленных значений, округленная до значения, кратного 128 байтам. Если это значение больше `thread_stack`, выполнение `call pq` для этой таблицы может привести к ошибкам в некоторых сохранённых запросах. Значение по умолчанию `thread_stack` — 1М (1048576); другие значения должны быть настроены.
* `tid` и `tid_saved`: отражают состояние сохранения таблицы. `tid` увеличивается при каждом изменении (транзакции). `tid_saved` показывает максимальный `tid` состояния, сохранённого в сегменте оперативной памяти в файле `<table>.ram`. Если числа отличаются, некоторые изменения существуют только в ОЗУ и также сохранены в binlog (если он включён). Выполнение `FLUSH TABLE` или плановое периодическое сбрасывание сохраняет эти изменения. После сброса binlog очищается, и `tid_saved` отображает новое фактическое состояние.
* `query_time_*`, `exact_query_time_*`: статистика времени выполнения запросов за последние 1 минуту, 5 минут, 15 минут и с момента запуска сервера; данные представлены в виде JSON-объекта, включая количество запросов и параметры min, max, avg, 95-й и 99-й перцентили.
* `found_rows_*`: статистика найденных запросами строк; предоставляется за последние 1 минуту, 5 минут, 15 минут и с начала работы сервера; данные представлены в виде JSON-объекта с количеством запросов и min, max, avg, 95-й и 99-й перцентили.
* `command_*`: счётчики общего количества успешных выполнений конкретных команд для этой таблицы.
* `search_stats_ms_*`: статистика времени выполнения (в миллисекундах) поисковых запросов. `*` указывает временное окно (например, 1min, 5min, 15min, total). Эти статистики рассчитываются на скользящих окнах в 1, 5 и 15 минут, показывая среднее, минимальное, максимальное и значения 95-го/99-го перцентилей времени выполнения запросов.
* `insert_replace_stats_ms_*`: статистика времени выполнения (в миллисекундах) запросов вставки и замены. `*` указывает временное окно (например, 1min, 5min, 15min, total). Эти статистики рассчитываются на скользящих окнах в 1, 5 и 15 минут, показывая среднее, минимальное, максимальное и значения 95-го/99-го перцентилей времени выполнения запросов.
* `update_stats_ms_*`: статистика времени выполнения (в миллисекундах) обновляющих запросов. `*` указывает временное окно (например, 1min, 5min, 15min, total). Эти статистики рассчитываются на скользящих окнах в 1, 5 и 15 минут, показывая среднее, минимальное, максимальное и значения 95-го/99-го перцентилей времени выполнения запросов.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
mysql> SHOW TABLE statistic STATUS;
```

<!-- response SQL -->

```sql
+-------------------------------+--------------------------------------------------------------------------+
| Variable_name                 | Value                                                                    |
+-------------------------------+--------------------------------------------------------------------------+
| index_type                    | rt                                                                       |
| indexed_documents             | 146000                                                                   |
| indexed_bytes                 | 149504000                                                                |
| ram_bytes                     | 87674788                                                                 |
| disk_bytes                    | 1762811                                                                  |
| disk_mapped                   | 794147                                                                   |
| disk_mapped_cached            | 802816                                                                   |
| disk_mapped_doclists          | 0                                                                        |
| disk_mapped_cached_doclists   | 0                                                                        |
| disk_mapped_hitlists          | 0                                                                        |
| disk_mapped_cached_hitlists   | 0                                                                        |
| killed_documents              | 0                                                                        |
| killed_rate                   | 0.00%                                                                    |
| ram_chunk                     | 86865484                                                                 |
| ram_chunk_segments_count      | 24                                                                       |
| disk_chunks                   | 1                                                                        |
| mem_limit                     | 134217728                                                                |
| mem_limit_rate                | 95.00%                                                                   |
| ram_bytes_retired             | 0                                                                        |
| optimizing                    | 1                                                                        |
| locked                        | 0                                                                        |
| tid                           | 0                                                                        |
| tid_saved                     | 0                                                                        |
| query_time_1min               | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| query_time_5min               | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| query_time_15min              | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| query_time_total              | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| found_rows_1min               | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| found_rows_5min               | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| found_rows_15min              | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| found_rows_total              | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| command_search                | 2                                                                        |
| command_excerpt               | 0                                                                        |
| command_update                | 3                                                                        |
| command_keywords              | 0                                                                        |
| command_status                | 2                                                                        |
| command_delete                | 0                                                                        |
| command_insert                | 1                                                                        |
| command_replace               | 0                                                                        |
| command_commit                | 0                                                                        |
| command_suggest               | 0                                                                        |
| command_callpq                | 0                                                                        |
| command_getfield              | 0                                                                        |
| insert_replace_stats_ms_avg   | 0.284 0.284 0.284                                                        |
| insert_replace_stats_ms_min   | 0.284 0.284 0.284                                                        |
| insert_replace_stats_ms_max   | 0.284 0.284 0.284                                                        |
| insert_replace_stats_ms_pct95 | 0.284 0.284 0.284                                                        |
| insert_replace_stats_ms_pct99 | 0.284 0.284 0.284                                                        |
| search_stats_ms_avg           | 0.000 0.000 0.000                                                        |
| search_stats_ms_min           | 0.000 0.000 0.000                                                        |
| search_stats_ms_max           | 0.000 0.000 0.000                                                        |
| search_stats_ms_pct95         | 0.000 0.000 0.000                                                        |
| search_stats_ms_pct99         | 0.000 0.000 0.000                                                        |
| update_stats_ms_avg           | 0.479 0.479 0.479                                                        |
| update_stats_ms_min           | 0.431 0.431 0.431                                                        |
| update_stats_ms_max           | 0.530 0.530 0.530                                                        |
| update_stats_ms_pct95         | 0.530 0.530 0.530                                                        |
| update_stats_ms_pct99         | 0.530 0.530 0.530                                                        |
+-------------------------------+--------------------------------------------------------------------------+
29 rows in set (0.00 sec)
```

<!-- intro -->
##### PHP:
<!-- request PHP -->

``` php
$index->status();
```

<!-- response PHP -->

```php
Array(
    [index_type] => rt
    [indexed_documents] => 3
    [indexed_bytes] => 0
    [ram_bytes] => 6678
    [disk_bytes] => 611
    [ram_chunk] => 990
    [ram_chunk_segments_count] => 2
    [mem_limit] => 134217728
    [ram_bytes_retired] => 0
    [optimizing] => 0
    [locked] => 0
    [tid] => 15
    [query_time_1min] => {"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}
    [query_time_5min] => {"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}
    [query_time_15min] => {"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}
    [query_time_total] => {"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}
    [found_rows_1min] => {"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}
    [found_rows_5min] => {"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}
    [found_rows_15min] => {"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}
    [found_rows_total] => {"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}
    [command_search] => 2
    [command_excerpt] => 0
    [command_update] => 3
    [command_keywords] => 0
    [command_status] => 2
    [command_delete] => 0
    [command_insert] => 1
    [command_replace] => 0
    [command_commit] => 0
    [command_suggest] => 0
    [command_callpq] => 0
    [command_getfield] => 0
    [insert_replace_stats_ms_avg] => 0.284 0.284 0.284
    [insert_replace_stats_ms_min] => 0.284 0.284 0.284
    [insert_replace_stats_ms_max] => 0.284 0.284 0.284
    [insert_replace_stats_ms_pct95] => 0.284 0.284 0.284
    [insert_replace_stats_ms_pct99] => 0.284 0.284 0.284
    [search_stats_ms_avg] => 0.000 0.000 0.000
    [search_stats_ms_min] => 0.000 0.000 0.000
    [search_stats_ms_max] => 0.000 0.000 0.000
    [search_stats_ms_pct95] => 0.000 0.000 0.000
    [search_stats_ms_pct99] => 0.000 0.000 0.000
    [update_stats_ms_avg] => 0.479 0.479 0.479
    [update_stats_ms_min] => 0.431 0.431 0.431
    [update_stats_ms_max] => 0.530 0.530 0.530
    [update_stats_ms_pct95] => 0.530 0.530 0.530
    [update_stats_ms_pct99] => 0.530 0.530 0.530
)
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('SHOW TABLE statistic STATUS')
```
<!-- response Python -->

```python
{u'columns': [{u'Key': {u'type': u'string'}},
              {u'Value': {u'type': u'string'}}],
 u'data': [
   {u'Key': u'table_type', u'Value': u'rt'}
    {u'Key': u'indexed_documents', u'Value': u'3'}
    {u'Key': u'indexed_bytes', u'Value': u'0'}
    {u'Key': u'ram_bytes', u'Value': u'6678'}
    {u'Key': u'disk_bytes', u'Value': u'611'}
    {u'Key': u'ram_chunk', u'Value': u'990'}
    {u'Key': u'ram_chunk_segments_count', u'Value': u'2'}
    {u'Key': u'mem_limit', u'Value': u'134217728'}
    {u'Key': u'ram_bytes_retired', u'Value': u'0'}
    {u'Key': u'optimizing', u'Value': u'0'}
    {u'Key': u'locked', u'Value': u'0'}
    {u'Key': u'tid', u'Value': u'15'}
    {u'Key': u'query_time_1min', u'Value': u'{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}'}
    {u'Key': u'query_time_5min', u'Value': u'{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}'}
    {u'Key': u'query_time_15min', u'Value': u'{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}'}
    {u'Key': u'query_time_total', u'Value': u'{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}'}
    {u'Key': u'found_rows_1min', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}
    {u'Key': u'found_rows_5min', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}
    {u'Key': u'found_rows_15min', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}
    {u'Key': u'found_rows_total', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}
    {u'Key': u'command_search', u'Value': u'2'}
    {u'Key': u'command_excerpt', u'Value': u'0'}
    {u'Key': u'command_update', u'Value': u'3'}
    {u'Key': u'command_keywords', u'Value': u'0'}
    {u'Key': u'command_status', u'Value': u'2'}
    {u'Key': u'command_delete', u'Value': u'0'}
    {u'Key': u'command_insert', u'Value': u'1'}
    {u'Key': u'command_replace', u'Value': u'0'}
    {u'Key': u'command_commit', u'Value': u'0'}
    {u'Key': u'command_suggest', u'Value': u'0'}
    {u'Key': u'command_callpq', u'Value': u'0'}
    {u'Key': u'command_getfield', u'Value': u'0'}
    {u'Key': u'insert_replace_stats_ms_avg', u'Value': u'0.284 0.284 0.284'}
    {u'Key': u'insert_replace_stats_ms_min', u'Value': u'0.284 0.284 0.284'}
    {u'Key': u'insert_replace_stats_ms_max', u'Value': u'0.284 0.284 0.284'}
    {u'Key': u'insert_replace_stats_ms_pct95', u'Value': u'0.284 0.284 0.284'}
    {u'Key': u'insert_replace_stats_ms_pct99', u'Value': u'0.284 0.284 0.284'}
    {u'Key': u'search_stats_ms_avg', u'Value': u'0.000 0.000 0.000'}
    {u'Key': u'search_stats_ms_min', u'Value': u'0.000 0.000 0.000'}
    {u'Key': u'search_stats_ms_max', u'Value': u'0.000 0.000 0.000'}
    {u'Key': u'search_stats_ms_pct95', u'Value': u'0.000 0.000 0.000'}
    {u'Key': u'search_stats_ms_pct99', u'Value': u'0.000 0.000 0.000'}
    {u'Key': u'update_stats_ms_avg', u'Value': u'0.479 0.479 0.479'}
    {u'Key': u'update_stats_ms_min', u'Value': u'0.431 0.431 0.431'}
    {u'Key': u'update_stats_ms_max', u'Value': u'0.530 0.530 0.530'}
    {u'Key': u'update_stats_ms_pct95', u'Value': u'0.530 0.530 0.530'}
    {u'Key': u'update_stats_ms_pct99', u'Value': u'0.530 0.530 0.530'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('SHOW TABLE statistic STATUS')
```
<!-- response Python-asyncio -->

```python
{u'columns': [{u'Key': {u'type': u'string'}},
              {u'Value': {u'type': u'string'}}],
 u'data': [
   {u'Key': u'table_type', u'Value': u'rt'}
    {u'Key': u'indexed_documents', u'Value': u'3'}
    {u'Key': u'indexed_bytes', u'Value': u'0'}
    {u'Key': u'ram_bytes', u'Value': u'6678'}
    {u'Key': u'disk_bytes', u'Value': u'611'}
    {u'Key': u'ram_chunk', u'Value': u'990'}
    {u'Key': u'ram_chunk_segments_count', u'Value': u'2'}
    {u'Key': u'mem_limit', u'Value': u'134217728'}
    {u'Key': u'ram_bytes_retired', u'Value': u'0'}
    {u'Key': u'optimizing', u'Value': u'0'}
    {u'Key': u'locked', u'Value': u'0'}
    {u'Key': u'tid', u'Value': u'15'}
    {u'Key': u'query_time_1min', u'Value': u'{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}'}
    {u'Key': u'query_time_5min', u'Value': u'{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}'}
    {u'Key': u'query_time_15min', u'Value': u'{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}'}
    {u'Key': u'query_time_total', u'Value': u'{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}'}
    {u'Key': u'found_rows_1min', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}
    {u'Key': u'found_rows_5min', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}
    {u'Key': u'found_rows_15min', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}
    {u'Key': u'found_rows_total', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}
    {u'Key': u'command_search', u'Value': u'2'}
    {u'Key': u'command_excerpt', u'Value': u'0'}
    {u'Key': u'command_update', u'Value': u'3'}
    {u'Key': u'command_keywords', u'Value': u'0'}
    {u'Key': u'command_status', u'Value': u'2'}
    {u'Key': u'command_delete', u'Value': u'0'}
    {u'Key': u'command_insert', u'Value': u'1'}
    {u'Key': u'command_replace', u'Value': u'0'}
    {u'Key': u'command_commit', u'Value': u'0'}
    {u'Key': u'command_suggest', u'Value': u'0'}
    {u'Key': u'command_callpq', u'Value': u'0'}
    {u'Key': u'command_getfield', u'Value': u'0'}
    {u'Key': u'insert_replace_stats_ms_avg', u'Value': u'0.284 0.284 0.284'}
    {u'Key': u'insert_replace_stats_ms_min', u'Value': u'0.284 0.284 0.284'}
    {u'Key': u'insert_replace_stats_ms_max', u'Value': u'0.284 0.284 0.284'}
    {u'Key': u'insert_replace_stats_ms_pct95', u'Value': u'0.284 0.284 0.284'}
    {u'Key': u'insert_replace_stats_ms_pct99', u'Value': u'0.284 0.284 0.284'}
    {u'Key': u'search_stats_ms_avg', u'Value': u'0.000 0.000 0.000'}
    {u'Key': u'search_stats_ms_min', u'Value': u'0.000 0.000 0.000'}
    {u'Key': u'search_stats_ms_max', u'Value': u'0.000 0.000 0.000'}
    {u'Key': u'search_stats_ms_pct95', u'Value': u'0.000 0.000 0.000'}
    {u'Key': u'search_stats_ms_pct99', u'Value': u'0.000 0.000 0.000'}
    {u'Key': u'update_stats_ms_avg', u'Value': u'0.479 0.479 0.479'}
    {u'Key': u'update_stats_ms_min', u'Value': u'0.431 0.431 0.431'}
    {u'Key': u'update_stats_ms_max', u'Value': u'0.530 0.530 0.530'}
    {u'Key': u'update_stats_ms_pct95', u'Value': u'0.530 0.530 0.530'}
    {u'Key': u'update_stats_ms_pct99', u'Value': u'0.530 0.530 0.530'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await utilsApi.sql('SHOW TABLE statistic STATUS');
```
<!-- response Javascript -->

```javascript
{"columns": [{"Key": {"type": "string"}},
              {"Value": {"type": "string"}}],
 "data": [
   {"Key": "table_type", "Value": "rt"}
    {"Key": "indexed_documents", "Value": "3"}
    {"Key": "indexed_bytes", "Value": "0"}
    {"Key": "ram_bytes", "Value": "6678"}
    {"Key": "disk_bytes", "Value": "611"}
    {"Key": "ram_chunk", "Value": "990"}
    {"Key": "ram_chunk_segments_count", "Value": "2"}
    {"Key": "mem_limit", "Value": "134217728"}
    {"Key": "ram_bytes_retired", "Value": "0"}
    {"Key": "optimizing", "Value": "0"}
    {"Key": "locked", "Value": "0"}
    {"Key": "tid", "Value": "15"}
    {"Key": "query_time_1min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
    {"Key": "query_time_5min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
    {"Key": "query_time_15min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
    {"Key": "query_time_total", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
    {"Key": "found_rows_1min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
    {"Key": "found_rows_5min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
    {"Key": "found_rows_15min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
    {"Key": "found_rows_total", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
    {"Key": "command_search", "Value": "2"}
    {"Key": "command_excerpt", "Value": "0"}
    {"Key": "command_update", "Value": "3"}
    {"Key": "command_keywords", "Value": "0"}
    {"Key": "command_status", "Value": "2"}
    {"Key": "command_delete", "Value": "0"}
    {"Key": "command_insert", "Value": "1"}
    {"Key": "command_replace", "Value": "0"}
    {"Key": "command_commit", "Value": "0"}
    {"Key": "command_suggest", "Value": "0"}
    {"Key": "command_callpq", "Value": "0"}
    {"Key": "command_getfield", "Value": "0"}
    {"Key": "insert_replace_stats_ms_avg", "Value": "0.284 0.284 0.284"}
    {"Key": "insert_replace_stats_ms_min", "Value": "0.284 0.284 0.284"}
    {"Key": "insert_replace_stats_ms_max", "Value": "0.284 0.284 0.284"}
    {"Key": "insert_replace_stats_ms_pct95", "Value": "0.284 0.284 0.284"}
    {"Key": "insert_replace_stats_ms_pct99", "Value": "0.284 0.284 0.284"}
    {"Key": "search_stats_ms_avg", "Value": "0.000 0.000 0.000"}
    {"Key": "search_stats_ms_min", "Value": "0.000 0.000 0.000"}
    {"Key": "search_stats_ms_max", "Value": "0.000 0.000 0.000"}
    {"Key": "search_stats_ms_pct95", "Value": "0.000 0.000 0.000"}
    {"Key": "search_stats_ms_pct99", "Value": "0.000 0.000 0.000"}
    {"Key": "update_stats_ms_avg", "Value": "0.479 0.479 0.479"}
    {"Key": "update_stats_ms_min", "Value": "0.431 0.431 0.431"}
    {"Key": "update_stats_ms_max", "Value": "0.530 0.530 0.530"}
    {"Key": "update_stats_ms_pct95", "Value": "0.530 0.530 0.530"}
    {"Key": "update_stats_ms_pct99", "Value": "0.530 0.530 0.530"}
],
 "error": "",
 "total": 0,
 "warning": ""}
```
<!-- intro -->
##### Java:

<!-- request Java -->

```java
utilsApi.sql("SHOW TABLE statistic STATUS");

```
<!-- response Java -->
```java
{columns=[{ Key : { type=string }},
              { Value : { type=string }}],
  data : [
   { Key=index_type, Value=rt}
    { Key=indexed_documents, Value=3}
    { Key=indexed_bytes, Value=0}
    { Key=ram_bytes, Value=6678}
    { Key=disk_bytes, Value=611}
    { Key=ram_chunk, Value=990}
    { Key=ram_chunk_segments_count, Value=2}
    { Key=mem_limit, Value=134217728}
    { Key=ram_bytes_retired, Value=0}
    { Key=optimizing, Value=0}
    { Key=locked, Value=0}
    { Key=tid, Value=15}
    { Key=query_time_1min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_5min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_15min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_total, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=found_rows_1min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_5min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_15min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_total, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=command_search, Value=2}
    { Key=command_excerpt, Value=0}
    { Key=command_update, Value=3}
    { Key=command_keywords, Value=0}
    { Key=command_status, Value=2}
    { Key=command_delete, Value=0}
    { Key=command_insert, Value=1}
    { Key=command_replace, Value=0}
    { Key=command_commit, Value=0}
    { Key=command_suggest, Value=0}
    { Key=command_callpq, Value=0}
    { Key=command_getfield, Value=0}
    { Key=insert_replace_stats_ms_avg, Value=0.284 0.284 0.284}
    { Key=insert_replace_stats_ms_min, Value=0.284 0.284 0.284}
    { Key=insert_replace_stats_ms_max, Value=0.284 0.284 0.284}
    { Key=insert_replace_stats_ms_pct95, Value=0.284 0.284 0.284}
    { Key=insert_replace_stats_ms_pct99, Value=0.284 0.284 0.284}
    { Key=search_stats_ms_avg, Value=0.000 0.000 0.000}
    { Key=search_stats_ms_min, Value=0.000 0.000 0.000}
    { Key=search_stats_ms_max, Value=0.000 0.000 0.000}
    { Key=search_stats_ms_pct95, Value=0.000 0.000 0.000}
    { Key=search_stats_ms_pct99, Value=0.000 0.000 0.000}
    { Key=update_stats_ms_avg, Value=0.479 0.479 0.479}
    { Key=update_stats_ms_min, Value=0.431 0.431 0.431}
    { Key=update_stats_ms_max, Value=0.530 0.530 0.530}
    { Key=update_stats_ms_pct95, Value=0.530 0.530 0.530}
    { Key=update_stats_ms_pct99, Value=0.530 0.530 0.530}
],
  error= ,
  total=0,
  warning= }
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("SHOW TABLE statistic STATUS");

```
<!-- response C# -->
```clike
{columns=[{ Key : { type=string }},
              { Value : { type=string }}],
  data : [
   { Key=index_type, Value=rt}
    { Key=indexed_documents, Value=3}
    { Key=indexed_bytes, Value=0}
    { Key=ram_bytes, Value=6678}
    { Key=disk_bytes, Value=611}
    { Key=ram_chunk, Value=990}
    { Key=ram_chunk_segments_count, Value=2}
    { Key=mem_limit, Value=134217728}
    { Key=ram_bytes_retired, Value=0}
    { Key=optimizing, Value=0}
    { Key=locked, Value=0}
    { Key=tid, Value=15}
    { Key=query_time_1min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_5min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_15min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_total, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=found_rows_1min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_5min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_15min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_total, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=command_search, Value=2}
    { Key=command_excerpt, Value=0}
    { Key=command_update, Value=3}
    { Key=command_keywords, Value=0}
    { Key=command_status, Value=2}
    { Key=command_delete, Value=0}
    { Key=command_insert, Value=1}
    { Key=command_replace, Value=0}
    { Key=command_commit, Value=0}
    { Key=command_suggest, Value=0}
    { Key=command_callpq, Value=0}
    { Key=command_getfield, Value=0}
    { Key=insert_replace_stats_ms_avg, Value=0.284 0.284 0.284}
    { Key=insert_replace_stats_ms_min, Value=0.284 0.284 0.284}
    { Key=insert_replace_stats_ms_max, Value=0.284 0.284 0.284}
    { Key=insert_replace_stats_ms_pct95, Value=0.284 0.284 0.284}
    { Key=insert_replace_stats_ms_pct99, Value=0.284 0.284 0.284}
    { Key=search_stats_ms_avg, Value=0.000 0.000 0.000}
    { Key=search_stats_ms_min, Value=0.000 0.000 0.000}
    { Key=search_stats_ms_max, Value=0.000 0.000 0.000}
    { Key=search_stats_ms_pct95, Value=0.000 0.000 0.000}
    { Key=search_stats_ms_pct99, Value=0.000 0.000 0.000}
    { Key=update_stats_ms_avg, Value=0.479 0.479 0.479}
    { Key=update_stats_ms_min, Value=0.431 0.431 0.431}
    { Key=update_stats_ms_max, Value=0.530 0.530 0.530}
    { Key=update_stats_ms_pct95, Value=0.530 0.530 0.530}
    { Key=update_stats_ms_pct99, Value=0.530 0.530 0.530}
],
  error="" ,
  total=0,
  warning="" }
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("SHOW TABLE statistic STATUS", Some(true)).await;

```
<!-- response Rust -->
```rust
{columns=[{ Key : { type=string }},
              { Value : { type=string }}],
  data : [
   { Key=index_type, Value=rt}
    { Key=indexed_documents, Value=3}
    { Key=indexed_bytes, Value=0}
    { Key=ram_bytes, Value=6678}
    { Key=disk_bytes, Value=611}
    { Key=ram_chunk, Value=990}
    { Key=ram_chunk_segments_count, Value=2}
    { Key=mem_limit, Value=134217728}
    { Key=ram_bytes_retired, Value=0}
    { Key=optimizing, Value=0}
    { Key=locked, Value=0}
    { Key=tid, Value=15}
    { Key=query_time_1min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_5min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_15min, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=query_time_total, Value={queries:1, avg_sec:0.001, min_sec:0.001, max_sec:0.001, pct95_sec:0.001, pct99_sec:0.001}}
    { Key=found_rows_1min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_5min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_15min, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=found_rows_total, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}
    { Key=command_search, Value=2}
    { Key=command_excerpt, Value=0}
    { Key=command_update, Value=3}
    { Key=command_keywords, Value=0}
    { Key=command_status, Value=2}
    { Key=command_delete, Value=0}
    { Key=command_insert, Value=1}
    { Key=command_replace, Value=0}
    { Key=command_commit, Value=0}
    { Key=command_suggest, Value=0}
    { Key=command_callpq, Value=0}
    { Key=command_getfield, Value=0}
    { Key=insert_replace_stats_ms_avg, Value=0.284 0.284 0.284}
    { Key=insert_replace_stats_ms_min, Value=0.284 0.284 0.284}
    { Key=insert_replace_stats_ms_max, Value=0.284 0.284 0.284}
    { Key=insert_replace_stats_ms_pct95, Value=0.284 0.284 0.284}
    { Key=insert_replace_stats_ms_pct99, Value=0.284 0.284 0.284}
    { Key=search_stats_ms_avg, Value=0.000 0.000 0.000}
    { Key=search_stats_ms_min, Value=0.000 0.000 0.000}
    { Key=search_stats_ms_max, Value=0.000 0.000 0.000}
    { Key=search_stats_ms_pct95, Value=0.000 0.000 0.000}
    { Key=search_stats_ms_pct99, Value=0.000 0.000 0.000}
    { Key=update_stats_ms_avg, Value=0.479 0.479 0.479}
    { Key=update_stats_ms_min, Value=0.431 0.431 0.431}
    { Key=update_stats_ms_max, Value=0.530 0.530 0.530}
    { Key=update_stats_ms_pct95, Value=0.530 0.530 0.530}
    { Key=update_stats_ms_pct99, Value=0.530 0.530 0.530}
],
  error="" ,
  total=0,
  warning="" }
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->

```typescript
res = await utilsApi.sql('SHOW TABLE statistic STATUS');
```
<!-- response TypeScript -->

```typescript
{
	"columns":
	[{
		"Key": {"type": "string"}
	},
    {
    	"Value": {"type": "string"}
    }],
	"data":
	[
		{"Key": "table_type", "Value": "rt"}
	    {"Key": "indexed_documents", "Value": "3"}
	    {"Key": "indexed_bytes", "Value": "0"}
	    {"Key": "ram_bytes", "Value": "6678"}
	    {"Key": "disk_bytes", "Value": "611"}
	    {"Key": "ram_chunk", "Value": "990"}
	    {"Key": "ram_chunk_segments_count", "Value": "2"}
	    {"Key": "mem_limit", "Value": "134217728"}
	    {"Key": "ram_bytes_retired", "Value": "0"}
	    {"Key": "optimizing", "Value": "0"}
	    {"Key": "locked", "Value": "0"}
	    {"Key": "tid", "Value": "15"}
	    {"Key": "query_time_1min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "query_time_5min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "query_time_15min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "query_time_total", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "found_rows_1min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	    {"Key": "found_rows_5min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	    {"Key": "found_rows_15min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	    {"Key": "found_rows_total", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	    {"Key": "command_search", "Value": "2"}
	    {"Key": "command_excerpt", "Value": "0"}
	    {"Key": "command_update", "Value": "3"}
	    {"Key": "command_keywords", "Value": "0"}
	    {"Key": "command_status", "Value": "2"}
	    {"Key": "command_delete", "Value": "0"}
	    {"Key": "command_insert", "Value": "1"}
	    {"Key": "command_replace", "Value": "0"}
	    {"Key": "command_commit", "Value": "0"}
	    {"Key": "command_suggest", "Value": "0"}
	    {"Key": "command_callpq", "Value": "0"}
	    {"Key": "command_getfield", "Value": "0"}
	    {"Key": "insert_replace_stats_ms_avg", "Value": "0.284 0.284 0.284"}
	    {"Key": "insert_replace_stats_ms_min", "Value": "0.284 0.284 0.284"}
	    {"Key": "insert_replace_stats_ms_max", "Value": "0.284 0.284 0.284"}
	    {"Key": "insert_replace_stats_ms_pct95", "Value": "0.284 0.284 0.284"}
	    {"Key": "insert_replace_stats_ms_pct99", "Value": "0.284 0.284 0.284"}
	    {"Key": "search_stats_ms_avg", "Value": "0.000 0.000 0.000"}
	    {"Key": "search_stats_ms_min", "Value": "0.000 0.000 0.000"}
	    {"Key": "search_stats_ms_max", "Value": "0.000 0.000 0.000"}
	    {"Key": "search_stats_ms_pct95", "Value": "0.000 0.000 0.000"}
	    {"Key": "search_stats_ms_pct99", "Value": "0.000 0.000 0.000"}
	    {"Key": "update_stats_ms_avg", "Value": "0.479 0.479 0.479"}
	    {"Key": "update_stats_ms_min", "Value": "0.431 0.431 0.431"}
	    {"Key": "update_stats_ms_max", "Value": "0.530 0.530 0.530"}
	    {"Key": "update_stats_ms_pct95", "Value": "0.530 0.530 0.530"}
	    {"Key": "update_stats_ms_pct99", "Value": "0.530 0.530 0.530"}
	],
	"error": "",
	"total": 0,
	"warning": ""
}
```

<!-- intro -->
##### Go:

<!-- request Go -->

```go
apiClient.UtilsAPI.Sql(context.Background()).Body("SHOW TABLE statistic STATUS").Execute()
```
<!-- response Go -->

```go
{
	"columns":
	[{
		"Key": {"type": "string"}
	},
    {
    	"Value": {"type": "string"}
    }],
	"data":
	[
		{"Key": "table_type", "Value": "rt"}
	    {"Key": "indexed_documents", "Value": "3"}
	    {"Key": "indexed_bytes", "Value": "0"}
	    {"Key": "ram_bytes", "Value": "6678"}
	    {"Key": "disk_bytes", "Value": "611"}
	    {"Key": "ram_chunk", "Value": "990"}
	    {"Key": "ram_chunk_segments_count", "Value": "2"}
	    {"Key": "mem_limit", "Value": "134217728"}
	    {"Key": "ram_bytes_retired", "Value": "0"}
	    {"Key": "optimizing", "Value": "0"}
	    {"Key": "locked", "Value": "0"}
	    {"Key": "tid", "Value": "15"}
	    {"Key": "query_time_1min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "query_time_5min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "query_time_15min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "query_time_total", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "found_rows_1min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	    {"Key": "found_rows_5min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	    {"Key": "found_rows_15min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	    {"Key": "found_rows_total", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	    {"Key": "command_search", "Value": "2"}
	    {"Key": "command_excerpt", "Value": "0"}
	    {"Key": "command_update", "Value": "3"}
	    {"Key": "command_keywords", "Value": "0"}
	    {"Key": "command_status", "Value": "2"}
	    {"Key": "command_delete", "Value": "0"}
	    {"Key": "command_insert", "Value": "1"}
	    {"Key": "command_replace", "Value": "0"}
	    {"Key": "command_commit", "Value": "0"}
	    {"Key": "command_suggest", "Value": "0"}
	    {"Key": "command_callpq", "Value": "0"}
	    {"Key": "command_getfield", "Value": "0"}
	    {"Key": "insert_replace_stats_ms_avg", "Value": "0.284 0.284 0.284"}
	    {"Key": "insert_replace_stats_ms_min", "Value": "0.284 0.284 0.284"}
	    {"Key": "insert_replace_stats_ms_max", "Value": "0.284 0.284 0.284"}
	    {"Key": "insert_replace_stats_ms_pct95", "Value": "0.284 0.284 0.284"}
	    {"Key": "insert_replace_stats_ms_pct99", "Value": "0.284 0.284 0.284"}
	    {"Key": "search_stats_ms_avg", "Value": "0.000 0.000 0.000"}
	    {"Key": "search_stats_ms_min", "Value": "0.000 0.000 0.000"}
	    {"Key": "search_stats_ms_max", "Value": "0.000 0.000 0.000"}
	    {"Key": "search_stats_ms_pct95", "Value": "0.000 0.000 0.000"}
	    {"Key": "search_stats_ms_pct99", "Value": "0.000 0.000 0.000"}
	    {"Key": "update_stats_ms_avg", "Value": "0.479 0.479 0.479"}
	    {"Key": "update_stats_ms_min", "Value": "0.431 0.431 0.431"}
	    {"Key": "update_stats_ms_max", "Value": "0.530 0.530 0.530"}
	    {"Key": "update_stats_ms_pct95", "Value": "0.530 0.530 0.530"}
	    {"Key": "update_stats_ms_pct99", "Value": "0.530 0.530 0.530"}
	],
	"error": "",
	"total": 0,
	"warning": ""
}
```

<!-- end -->

