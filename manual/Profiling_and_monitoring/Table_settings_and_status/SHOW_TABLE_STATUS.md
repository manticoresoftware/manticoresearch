# SHOW TABLE STATUS

<!-- example SHOW TABLE STATUS -->

`SHOW TABLE STATUS` is an SQL statement that displays various per-table statistics.

The syntax is:

```sql
SHOW TABLE index_name STATUS
```

Displayed statistics include:

* `index_type`: for now that is one of `disk`, `rt`, `percolate`, `template`, and `distributed`.
* `indexed_documents` and `indexed_bytes`: number of indexed documents and their text size in bytes, respectively.
* `field_tokens_XXX`: sums of per-field lengths (in tokens) over the entire table (that is used internally in `BM25A` and `BM25F` functions for ranking purposes). Only available for tables built with `index_field_lengths=1`.
* `ram_bytes`: total size (in bytes) of RAM-resident table portion.
* `disk_bytes`: total size (in bytes) of all table files.
* `disk_mapped`: total size of file mappings.
* `disk_mapped_cached`: total size of file mappings actually cached in RAM.
* `disk_mapped_doclists` and `disk_mapped_cached_doclists`: part of the total and cached mappings belonging to document lists.
* `disk_mapped_hitlists` and `disk_mapped_cached_hitlists`: part of the total and cached mappings belonging to hit lists. Values for doclists and hitlists are shown separately since they're usually huge (say, about 90% size of the whole table).
* `killed_documents` and `killed_rate`: the first one indicates the number of deleted documents and the rate of deleted/indexed. Technically deletion of a document just means that the document gets suppressed in search output, but physically it still persists in a table and will be purged only after merging/optimizing the table.
* `ram_chunk`: size of RAM chunk of real-time or percolate table.
* `ram_chunk_segments_count`: RAM chunk internally consists of segments, usually there are no more than 32 of them. This line shows the current count.
* `disk_chunks`: number of disk chunks of the real-time table.
* `mem_limit`: actual value of `rt_mem_limit` for the table.
* `mem_limit_rate`: the rate after which the ram chunk will be flushed as a disk chunk, e.g. if `rt_mem_limit` is 128M and the rate is 50%, a new disk chunk will be saved as soon as the ram chunk exceeds 64M.
* `ram_bytes_retired`: represents size of garbage in RAM chunks (for example, deleted or replaced documents not yet finally wiped away).
* `tid` and `tid_saved`: represent the state of saving the table (real-time or percolate only). `tid` gets increased with each change (transaction). `tid_saved` shows max `tid` of the state saved in a RAM chunk in `<table>.ram` file. When the numbers are different, some changes exist only in RAM and also backed by binlog (if enabled). Performing `FLUSH TABLE` or scheduling periodical flushing causes these changes to be saved. After flushing the binlog gets cleared, and the `tid_saved` represents the actual new state.
* `query_time_*`: query execution time statistics of last 1 minute, 5 minutes, 15 minutes and total since server start; the data is encapsulated as a JSON object which includes the number of queries and min, max, avg, 95 and 99 percentile values.
* `found_rows_*`: statistics of rows found by queries; provided for last 1 minute, 5 minutes, 15 minutes and total since server start; the data is encapsulated as a JSON object which includes the number of queries and min, max, avg, 95 and 99 percentile values.

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
   {u'Key': u'index_type', u'Value': u'rt'}
    {u'Key': u'indexed_documents', u'Value': u'3'}
    {u'Key': u'indexed_bytes', u'Value': u'0'}
    {u'Key': u'ram_bytes', u'Value': u'6678'}
    {u'Key': u'disk_bytes', u'Value': u'611'}
    {u'Key': u'ram_chunk', u'Value': u'990'}
    {u'Key': u'ram_chunk_segments_count', u'Value': u'2'}
    {u'Key': u'mem_limit', u'Value': u'134217728'}
    {u'Key': u'ram_bytes_retired', u'Value': u'0'}
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
   {"Key": "index_type", "Value": "rt"}
    {"Key": "indexed_documents", "Value": "3"}
    {"Key": "indexed_bytes", "Value": "0"}
    {"Key": "ram_bytes", "Value": "6678"}
    {"Key": "disk_bytes", "Value": "611"}
    {"Key": "ram_chunk", "Value": "990"}
    {"Key": "ram_chunk_segments_count", "Value": "2"}
    {"Key": "mem_limit", "Value": "134217728"}
    {"Key": "ram_bytes_retired", "Value": "0"}
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
<!-- end -->
