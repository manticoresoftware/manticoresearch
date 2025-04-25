# ПОКАЗАТЬ СТАТУС ТАБЛИЦЫ

<!-- пример ПОКАЗАТЬ СТАТУС ТАБЛИЦЫ -->

`ПОКАЗАТЬ СТАТУС ТАБЛИЦЫ` — это SQL команда, которая отображает различные статистики по каждой таблице.

Синтаксис:

```sql
ПОКАЗАТЬ ТАБЛИЦУ имя_таблицы СТАТУС
```

В зависимости от типа индекса, отображаемая статистика включает различные наборы строк:

* **шаблон**: `тип_индекса`.
* **распределенный**: `тип_индекса`, `время_запроса_1мин`, `время_запроса_5мин`,`время_запроса_15мин`,`время_запроса_всего`, `точное_время_запроса_1мин`, `точное_время_запроса_5мин`, `точное_время_запроса_15мин`, `точное_время_запроса_всего`, `найденные_строки_1мин`, `найденные_строки_5мин`, `найденные_строки_15мин`, `найденные_строки_всего`.
* **перколировать**: `тип_индекса`, `храненые_запросы`, `байты_рам`, `байты_диска`, `максимальная_необходимость_стека`, `средняя_база_стека`, `
  необходимый_поток_стека`, `tid`, `tid_сохраненный`, `время_запроса_1мин`, `время_запроса_5мин`,`время_запроса_15мин`,`время_запроса_всего`, `точное_время_запроса_1мин`, `точное_время_запроса_5мин`, `точное_время_запроса_15мин`, `точное_время_запроса_всего`, `найденные_строки_1мин`, `найденные_строки_5мин`, `найденные_строки_15мин`, `найденные_строки_всего`.
* **простой**: `тип_индекса`, `индексированные_документы`, `индексированные_байты`, может быть набором `токены_поля_*` и `общие_токены`, `байты_рам`, `байты_диска`, `диск_отображен`, `диск_отображен_кэш`, `диск_отображен_списки_документов`, `диск_отображен_кэш_списки_документов`, `диск_отображен_списки_поиска`, `диск_отображен_кэш_списки_поиска`, `убитые_документы`, `коэффициент_убийства`, `время_запроса_1мин`, `время_запроса_5мин`,`время_запроса_15мин`,`время_запроса_всего`, `точное_время_запроса_1мин`, `точное_время_запроса_5мин`, `точное_время_запроса_15мин`, `точное_время_запроса_всего`, `найденные_строки_1мин`, `найденные_строки_5мин`, `найденные_строки_15мин`, `найденные_строки_всего`.
* **rt**: `тип_индекса`, `индексированные_документы`, `индексированные_байты`, может быть набором `токены_поля_*` и `общие_токены`, `байты_рам`, `байты_диска`, `диск_отображен`, `диск_отображен_кэш`, `диск_отображен_списки_документов`, `диск_отображен_кэш_списки_документов`, `диск_отображен_списки_поиска`, `диск_отображен_кэш_списки_поиска`, `убитые_документы`, `коэффициент_убийства`, `байт_чука_рам`, `количество_сегментов_чука_рам`, `диск_чанков`, `лимит_памяти`, `коэффициент_лимита_памяти`, `байты_рам_выведены`, `оптимизация`, `заблокирован`, `tid`, `tid_сохраненный`, `время_запроса_1мин`, `время_запроса_5мин`,`время_запроса_15мин`,`время_запроса_всего`, `точное_время_запроса_1мин`, `точное_время_запроса_5мин`, `точное_время_запроса_15мин`, `точное_время_запроса_всего`, `найденные_строки_1мин`, `найденные_строки_5мин`, `найденные_строки_15мин`, `найденные_строки_всего`.

Вот значение этих величин:

* `тип_индекса`: в настоящее время один из `диск`, `rt`, `перколировать`, `шаблон` и `распределенный`.
* `индексированные_документы`: количество индексированных документов.
* `индексированные_байты`: общий размер индексированного текста. Обратите внимание, что это значение не строго, так как в полнотекстовом индексе невозможно строго восстановить сохраненный текст для его измерения.
* `храненые_запросы`: число перколируемых запросов, хранящихся в таблице.
* `токены_поля_XXX`: необязательный, общие длины по каждому полю (в токенах) по всей таблице (используется внутренне для функций ранжирования `BM25A` и `BM25F`). Доступно только для таблиц, построенных с `index_field_lengths=1`.
* `общие_токены`: необязательные, общая сумма всех `токены_поля_XXX`.
* `байты_рам`: всего ОЗУ, занятого таблицей.
* `байты_диска`: всего дискового пространства, занятого таблицей.
* `диск_отображен`: общий размер сопоставлений файлов.
* `диск_отображен_кэш`: общий размер сопоставлений файлов, фактически кэшированных в ОЗУ.
* `диск_отображен_списки_документов` и `диск_отображен_кэш_списки_документов`: часть общего и кэшированного сопоставления, принадлежащего спискам документов.
* `диск_отображен_списки_поиска` и `диск_отображен_кэш_списки_поиска`: часть общего и кэшированного сопоставления, принадлежащего спискам поиска. Значения списков документов и списков поиска показаны отдельно, так как они обычно велики (например, около 90% от общего размера таблицы).
* `убитые_документы` и `коэффициент_убийства`: первое указывает на количество удаленных документов и коэффициент удаленных/индексированных. Технически, удаление документа означает подавление его в выводе поиска, но он все еще физически существует в таблице и будет удален только после слияния/оптимизации таблицы.
* `байт_чука`: размер блока ОЗУ реального времени или перколируемой таблицы.
* `количество_сегментов_чука_рам`: блок ОЗУ внутренне состоит из сегментов, обычно не более 32. Эта строка показывает текущее количество.
* `диск_чанков`: количество дисковых чанков в таблице реального времени.
* `лимит_памяти`: фактическое значение `rt_mem_limit` для таблицы.
* `коэффициент_лимита_памяти`: коэффицент, по которому блок ОЗУ будет сброшен как дисковый блок, например, если `rt_mem_limit` составляет 128M, а коэффициент составляет 50%, новый дисковый блок будет сохранен, когда блок ОЗУ превышает 64M.
* `байты_рам_выведены`: представляет размер мусора в блоках ОЗУ (например, удаленные или замененные документы, которые еще не были окончательно удалены).
* `оптимизация`: значение больше 0 указывает на то, что таблица в данный момент выполняет оптимизацию (т.е. она объединяет некоторые дисковые чанки прямо сейчас).
* `заблокирован`: значение больше 0 указывает на то, что таблица в данный момент заблокирована [FREEZE](../../Securing_and_compacting_a_table/Freezing_a_table.md#Freezing-a-table). Число представляет собой количество раз, когда таблица была заморожена. Например, таблица может быть заморожена с помощью `manticore-backup`, а затем заморожена снова с помощью репликации. Она должна быть полностью разморожена, когда ни один другой процесс не требует ее заморозки.
* `максимальная_необходимость_стека`: пространство стека, необходимое для вычисления самых сложных запросов из сохраненных перколируемых запросов. Это динамическое значение, зависящее от деталей сборки, таких как компилятор, оптимизация, аппаратное обеспечение и т. д.
* `средняя_база_стека`: пространство стека, которое обычно занимает начало вычисления перколируемого запроса.
* `необходимый_поток_стека`: сумма вышеуказанных значений, округленная до 128 байтов. Если это значение больше, чем `thread_stack`, вы не сможете выполнить `call pq` по этой таблице, так как некоторые сохраненные запросы не будут выполнены. Значение по умолчанию для `thread_stack` составляет 1M (что является 1048576); другие значения необходимо настроить.
* `tid` и `tid_saved`: представляют состояние сохранения таблицы. `tid` увеличивается с каждым изменением (транзакцией). `tid_saved` показывает максимальный `tid` состояния, сохраненного в участке ОЗУ в файле `<table>.ram`. Когда числа различаются, некоторые изменения существуют только в ОЗУ и также отражаются в бинарном логе (если включен). Выполнение `FLUSH TABLE` или планирование периодического сброса сохраняет эти изменения. После сброса бинарный лог очищается, и `tid_saved` представляет новое актуальное состояние.
* `query_time_*`, `exact_query_time_*`: статистика времени выполнения запросов за последние 1 минуту, 5 минут, 15 минут и всего времени с момента запуска сервера; данные заключены в объект JSON, включая число запросов и значения мин, макс, среднее, 95 и 99 процентиль.
* `found_rows_*`: статистика строк, найденных по запросам; предоставлена за последние 1 минуту, 5 минут, 15 минут и всего времени с момента запуска сервера; данные заключены в объект JSON, включая число запросов и значения мин, макс, среднее, 95 и 99 процентиль.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
mysql> SHOW TABLE statistic STATUS;
```

<!-- response SQL -->

```sql
+-----------------------------+--------------------------------------------------------------------------+
| Variable_name               | Value                                                                    |
+-----------------------------+--------------------------------------------------------------------------+
| index_type                  | rt                                                                       |
| indexed_documents           | 146000                                                                   |
| indexed_bytes               | 149504000                                                                |
| ram_bytes                   | 87674788                                                                 |
| disk_bytes                  | 1762811                                                                  |
| disk_mapped                 | 794147                                                                   |
| disk_mapped_cached          | 802816                                                                   |
| disk_mapped_doclists        | 0                                                                        |
| disk_mapped_cached_doclists | 0                                                                        |
| disk_mapped_hitlists        | 0                                                                        |
| disk_mapped_cached_hitlists | 0                                                                        |
| killed_documents            | 0                                                                        |
| killed_rate                 | 0.00%                                                                    |
| ram_chunk                   | 86865484                                                                 |
| ram_chunk_segments_count    | 24                                                                       |
| disk_chunks                 | 1                                                                        |
| mem_limit                   | 134217728                                                                |
| mem_limit_rate              | 95.00%                                                                   |
| ram_bytes_retired           | 0                                                                        |
| optimizing                  | 1                                                                        |
| locked                      | 0                                                                        |
| tid                         | 0                                                                        |
| tid_saved                   | 0                                                                        |
| query_time_1min             | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| query_time_5min             | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| query_time_15min            | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| query_time_total            | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| found_rows_1min             | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| found_rows_5min             | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| found_rows_15min            | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| found_rows_total            | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
+-----------------------------+--------------------------------------------------------------------------+
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
    {u'Key': u'found_rows_total', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}],
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
    {u'Key': u'found_rows_total', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}],
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
    {"Key": "found_rows_total", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}],
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
    { Key=found_rows_total, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}],
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
    { Key=found_rows_total, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}],
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
    { Key=found_rows_total, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}],
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
  {"Key": "оптимизация", "Value": "0"}

  {"Key": "зам_locked", "Value": "0"}

  {"Key": "tid", "Value": "15"}

  {"Key": "время_запроса_1мин", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}

  {"Key": "время_запроса_5мин", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}

  {"Key": "время_запроса_15мин", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}

  {"Key": "время_запроса_всего", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}

  {"Key": "найденные_строки_1мин", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}

  {"Key": "найденные_строки_5мин", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}

  {"Key": "найденные_строки_15мин", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}

  {"Key": "найденные_строки_всего", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}

],

"ошибка": "",

"всего": 0,

"предупреждение": ""
}
```

<!-- intro -->
##### Перейти:

<!-- request Go -->

```go
apiClient.UtilsAPI.Sql(context.Background()).Body("SHOW TABLE statistic STATUS").Execute()
```
<!-- response Go -->

```go
{

"колонки": 

[{

"Key": {"type": "string"}

# ПОКАЗАТЬ СТАТУС ТАБЛИЦЫ

<!-- example SHOW TABLE STATUS -->

`SHOW TABLE STATUS` — это SQL-оператор, который отображает различные статистические данные по таблицам.

Синтаксис:

```sql
SHOW TABLE table_name STATUS
```

В зависимости от типа индекса, отображаемая статистика включает разный набор строк:

* **template**: `index_type`.
* **distributed**: `index_type`, `query_time_1min`, `query_time_5min`,`query_time_15min`,`query_time_total`, `exact_query_time_1min`, `exact_query_time_5min`, `exact_query_time_15min`, `exact_query_time_total`, `found_rows_1min`, `found_rows_5min`, `found_rows_15min`, `found_rows_total`.
* **percolate**: `index_type`, `stored_queries`, `ram_bytes`, `disk_bytes`, `max_stack_need`, `average_stack_base`, `
  desired_thread_stack`, `tid`, `tid_saved`, `query_time_1min`, `query_time_5min`,`query_time_15min`,`query_time_total`, `exact_query_time_1min`, `exact_query_time_5min`, `exact_query_time_15min`, `exact_query_time_total`, `found_rows_1min`, `found_rows_5min`, `found_rows_15min`, `found_rows_total`.
* **plain**: `index_type`, `indexed_documents`, `indexed_bytes`, may be set of `field_tokens_*` and `total_tokens`, `ram_bytes`, `disk_bytes`, `disk_mapped`, `disk_mapped_cached`, `disk_mapped_doclists`, `disk_mapped_cached_doclists`, `disk_mapped_hitlists`, `disk_mapped_cached_hitlists`, `killed_documents`, `killed_rate`, `query_time_1min`, `query_time_5min`,`query_time_15min`,`query_time_total`, `exact_query_time_1min`, `exact_query_time_5min`, `exact_query_time_15min`, `exact_query_time_total`, `found_rows_1min`, `found_rows_5min`, `found_rows_15min`, `found_rows_total`.
* **rt**: `index_type`, `indexed_documents`, `indexed_bytes`, may be set of `field_tokens_*` and `total_tokens`, `ram_bytes`, `disk_bytes`, `disk_mapped`, `disk_mapped_cached`, `disk_mapped_doclists`, `disk_mapped_cached_doclists`, `disk_mapped_hitlists`, `disk_mapped_cached_hitlists`, `killed_documents`, `killed_rate`, `ram_chunk`, `ram_chunk_segments_count`, `disk_chunks`, `mem_limit`, `mem_limit_rate`, `ram_bytes_retired`, `optimizing`, `locked`, `tid`, `tid_saved`, `query_time_1min`, `query_time_5min`,`query_time_15min`,`query_time_total`, `exact_query_time_1min`, `exact_query_time_5min`, `exact_query_time_15min`, `exact_query_time_total`, `found_rows_1min`, `found_rows_5min`, `found_rows_15min`, `found_rows_total`.

Здесь приведено описание этих значений:

* `index_type`: в данный момент одно из значений `disk`, `rt`, `percolate`, `template` и `distributed`.
* `indexed_documents`: количество проиндексированных документов.
* `indexed_bytes`: общий размер проиндексированного текста. Обратите внимание, что это значение не строгое, поскольку для полнотекстового индекса невозможно точно восстановить исходный текст для измерения его объёма.
* `stored_queries`: количество запросов перколяции, сохранённых в таблице.
* `field_tokens_XXX`: опционально, суммарная длина по каждому полю (в токенах) по всей таблице (используется внутренне для функций ранжирования `BM25A` и `BM25F`). Доступно только для таблиц, созданных с параметром `index_field_lengths=1`.
* `total_tokens`: опционально, общая сумма всех `field_tokens_XXX`.
* `ram_bytes`: общий объём оперативной памяти, занимаемой таблицей.
* `disk_bytes`: общее дисковое пространство, занимаемое таблицей.
* `disk_mapped`: общий размер отображения файлов в память.
* `disk_mapped_cached`: общий размер файловых отображений, фактически кэшируемых в оперативной памяти.
* `disk_mapped_doclists` и `disk_mapped_cached_doclists`: доля от общего и кэшируемого отображения, относящаяся к спискам документов.
* `disk_mapped_hitlists` и `disk_mapped_cached_hitlists`: доля от общего и кэшируемого отображения, относящаяся к спискам совпадений. Значения для списков документов и списков совпадений показываются отдельно, так как обычно они очень велики (например, около 90% от общего размера таблицы).
* `killed_documents` и `killed_rate`: первое указывает количество удалённых документов, а второе — соотношение удалённых к проиндексированным. Технически, удаление документа означает исключение его из результатов поиска, однако он физически сохраняется в таблице и будет окончательно удалён только после слияния/оптимизации таблицы.
* `ram_chunk`: размер блока оперативной памяти для таблицы реального времени или таблицы перколяции.
* `ram_chunk_segments_count`: блок оперативной памяти внутренне состоит из сегментов, обычно не более 32. Эта строка показывает текущее количество.
* `disk_chunks`: количество дисковых блоков в таблице реального времени.
* `mem_limit`: фактическое значение `rt_mem_limit` для таблицы.
* `mem_limit_rate`: скорость, с которой блок оперативной памяти будет сбрасываться как дисковый блок, например, если `rt_mem_limit` равен 128M, а скорость — 50%, новый дисковый блок будет создан при превышении блока оперативной памяти 64M.
* `ram_bytes_retired`: представляет размер "мусора" в блоках оперативной памяти (например, удалённых или заменённых документов, ещё не окончательно удалённых).
* `optimizing`: значение больше 0 указывает на то, что таблица в данный момент проходит оптимизацию (то есть сейчас сливаются некоторые дисковые блоки).
* `locked`: значение больше 0 указывает на то, что таблица в настоящее время заблокирована с помощью [FREEZE](../../Securing_and_compacting_a_table/Freezing_a_table.md#Freezing-a-table). Число отражает, сколько раз таблица была заморожена. Например, таблица может быть заморожена `manticore-backup`, а затем снова заморожена репликацией. Таблица должна быть полностью разморожена только тогда, когда ни один другой процесс не требует, чтобы она оставалась замороженной.
* `max_stack_need`: объём стека, необходимый для вычисления самого сложного из сохранённых перколяционных запросов. Это динамическое значение, зависящее от деталей сборки, таких как компилятор, оптимизация, аппаратное обеспечение и т.д.
* `average_stack_base`: объём стека, который обычно используется при начале вычисления перколяционного запроса.
* `desired_thread_stack`: сумма вышеуказанных значений, округлённая до ближайшего кратного 128 байтам. Если это значение больше, чем `thread_stack`, вы не сможете выполнить `call pq` для этой таблицы, так как некоторые сохранённые запросы завершатся неудачей. Значение по умолчанию для `thread_stack` — 1M (то есть 1048576); другие значения должны быть настроены.
* `tid` и `tid_saved`: представляют состояние сохранения таблицы. `tid` увеличивается с каждым изменением (транзакцией). `tid_saved` показывает максимальный `tid` состояния, сохраненного в кусочке ОЗУ в файле `<table>.ram`. Когда числа различаются, некоторые изменения существуют только в ОЗУ и также сохранены в binlog (если включен). Выполнение `FLUSH TABLE` или планирование периодической очистки сохраняет эти изменения. После очистки binlog очищается, и `tid_saved` представляет новое фактическое состояние.
* `query_time_*`, `exact_query_time_*`: статистика времени выполнения запросов за последние 1 минуту, 5 минут, 15 минут и всего времени с момента запуска сервера; данные инкапсулированы в объект JSON, включая количество запросов и значения минимального, максимального, среднего, 95-го и 99-го процентилей.
* `found_rows_*`: статистика строк, найденных по запросам; предоставлена за последние 1 минуту, 5 минут, 15 минут и всего времени с момента запуска сервера; данные инкапсулированы в объект JSON, включая количество запросов и значения минимального, максимального, среднего, 95-го и 99-го процентилей.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
mysql> SHOW TABLE statistic STATUS;
```

<!-- response SQL -->

```sql
+-----------------------------+--------------------------------------------------------------------------+
| Variable_name               | Value                                                                    |
+-----------------------------+--------------------------------------------------------------------------+
| index_type                  | rt                                                                       |
| indexed_documents           | 146000                                                                   |
| indexed_bytes               | 149504000                                                                |
| ram_bytes                   | 87674788                                                                 |
| disk_bytes                  | 1762811                                                                  |
| disk_mapped                 | 794147                                                                   |
| disk_mapped_cached          | 802816                                                                   |
| disk_mapped_doclists        | 0                                                                        |
| disk_mapped_cached_doclists | 0                                                                        |
| disk_mapped_hitlists        | 0                                                                        |
| disk_mapped_cached_hitlists | 0                                                                        |
| killed_documents            | 0                                                                        |
| killed_rate                 | 0.00%                                                                    |
| ram_chunk                   | 86865484                                                                 |
| ram_chunk_segments_count    | 24                                                                       |
| disk_chunks                 | 1                                                                        |
| mem_limit                   | 134217728                                                                |
| mem_limit_rate              | 95.00%                                                                   |
| ram_bytes_retired           | 0                                                                        |
| optimizing                  | 1                                                                        |
| locked                      | 0                                                                        |
| tid                         | 0                                                                        |
| tid_saved                   | 0                                                                        |
| query_time_1min             | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| query_time_5min             | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| query_time_15min            | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| query_time_total            | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| found_rows_1min             | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| found_rows_5min             | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| found_rows_15min            | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
| found_rows_total            | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"} |
+-----------------------------+--------------------------------------------------------------------------+
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
    {u'Key': u'found_rows_total', u'Value': u'{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}'}],
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
    {"Key": "found_rows_total", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}],
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
    { Key=found_rows_total, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}],
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
    { Key=found_rows_total, Value={queries:1, avg:3, min:3, max:3, pct95:3, pct99:3}}],
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
	    {"Key": "query_time_1min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec:0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "query_time_5min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "query_time_15min", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "query_time_total", "Value": "{"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001}"}
	    {"Key": "found_rows_1min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	    {"Key": "found_rows_5min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	    {"Key": "found_rows_15min", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	    {"Key": "found_rows_total", "Value": "{"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}"}
	],
	"error": "",
	"total": 0,
	"warning": ""
}
```

<!-- end -->
