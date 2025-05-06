# SHOW TABLE STATUS

<!-- example SHOW TABLE STATUS -->

`SHOW TABLE STATUS` is an SQL statement that displays various per-table statistics.

The syntax is:

```sql
SHOW TABLE table_name STATUS
```

Depending on index type, displayed statistic includes different set of rows:

* **template**: `index_type`.
* **distributed**: `index_type`, `query_time_1min`, `query_time_5min`,`query_time_15min`,`query_time_total`, `exact_query_time_1min`, `exact_query_time_5min`, `exact_query_time_15min`, `exact_query_time_total`, `found_rows_1min`, `found_rows_5min`, `found_rows_15min`, `found_rows_total`.
* **percolate**: `index_type`, `stored_queries`, `ram_bytes`, `disk_bytes`, `max_stack_need`, `average_stack_base`, `
  desired_thread_stack`, `tid`, `tid_saved`, `query_time_1min`, `query_time_5min`,`query_time_15min`,`query_time_total`, `exact_query_time_1min`, `exact_query_time_5min`, `exact_query_time_15min`, `exact_query_time_total`, `found_rows_1min`, `found_rows_5min`, `found_rows_15min`, `found_rows_total`.
* **plain**: `index_type`, `indexed_documents`, `indexed_bytes`, may be set of `field_tokens_*` and `total_tokens`, `ram_bytes`, `disk_bytes`, `disk_mapped`, `disk_mapped_cached`, `disk_mapped_doclists`, `disk_mapped_cached_doclists`, `disk_mapped_hitlists`, `disk_mapped_cached_hitlists`, `killed_documents`, `killed_rate`, `query_time_1min`, `query_time_5min`,`query_time_15min`,`query_time_total`, `exact_query_time_1min`, `exact_query_time_5min`, `exact_query_time_15min`, `exact_query_time_total`, `found_rows_1min`, `found_rows_5min`, `found_rows_15min`, `found_rows_total`.
* **rt**: `index_type`, `indexed_documents`, `indexed_bytes`, may be set of `field_tokens_*` and `total_tokens`, `ram_bytes`, `disk_bytes`, `disk_mapped`, `disk_mapped_cached`, `disk_mapped_doclists`, `disk_mapped_cached_doclists`, `disk_mapped_hitlists`, `disk_mapped_cached_hitlists`, `killed_documents`, `killed_rate`, `ram_chunk`, `ram_chunk_segments_count`, `disk_chunks`, `mem_limit`, `mem_limit_rate`, `ram_bytes_retired`, `optimizing`, `locked`, `tid`, `tid_saved`, `query_time_1min`, `query_time_5min`,`query_time_15min`,`query_time_total`, `exact_query_time_1min`, `exact_query_time_5min`, `exact_query_time_15min`, `exact_query_time_total`, `found_rows_1min`, `found_rows_5min`, `found_rows_15min`, `found_rows_total`.

Here is the meaning of these values:

* `index_type`: currently one of `disk`, `rt`, `percolate`, `template`, and `distributed`.
* `indexed_documents`: number of indexed documents.
* `indexed_bytes`: overall size of indexed text. Notice, this value is not strict, since in full-text index that is impossible to strictly recover back stored text to measure it.
* `stored_queries`: number of percolate queries, stored in the table.
* `field_tokens_XXX`: optional, total per-field lengths (in tokens) across the entire table (used internally for `BM25A` and `BM25F` ranking functions). Only available for tables built with `index_field_lengths=1`.
* `total_tokens`: optional, overall sum of all `field_tokens_XXX`.
* `ram_bytes`: total RAM occupied by table.
* `disk_bytes`: total disk space, occupied by table.
* `disk_mapped`: total size of file mappings.
* `disk_mapped_cached`: total size of file mappings actually cached in RAM.
* `disk_mapped_doclists` and `disk_mapped_cached_doclists`: portion of total and cached mappings belonging to document lists.
* `disk_mapped_hitlists` and `disk_mapped_cached_hitlists`: portion of total and cached mappings belonging to hit lists. Doclists and hitlists values are shown separately since they're typically large (e.g., about 90% of the whole table's size).
* `killed_documents` and `killed_rate`: the first indicates the number of deleted documents and the rate of deleted/indexed. Technically, deleting a document means suppressing it in search output, but it still physically exists in the table and will only be purged after merging/optimizing the table.
* `ram_chunk`: size of the RAM chunk of real-time or percolate table.
* `ram_chunk_segments_count`: RAM chunk is internally composed of segments, typically no more than 32. This line shows the current count.
* `disk_chunks`: number of disk chunks in the real-time table.
* `mem_limit`: actual value of `rt_mem_limit` for the table.
* `mem_limit_rate`: the rate at which the RAM chunk will be flushed as a disk chunk, e.g., if `rt_mem_limit` is 128M and the rate is 50%, a new disk chunk will be saved when the RAM chunk exceeds 64M.
* `ram_bytes_retired`: represents the size of garbage in RAM chunks (e.g., deleted or replaced documents not yet permanently removed).
* `optimizing`: a value greater than 0 indicates that the table is currently performing optimization (i.e. it is merging some disk chunks right now).
* `locked`: a value greater than 0 indicates that the table is currently locked by [FREEZE](../../Securing_and_compacting_a_table/Freezing_a_table.md#Freezing-a-table). The number represents how many times the table has been frozen. For instance, a table might be frozen by `manticore-backup` and then frozen again by replication. It should only be completely unfrozen when no other process requires it to be frozen.
* `max_stack_need`: stack space we need to calculate most complex from the stored percolate queries. That is dynamic value, depends on build details as compiler, optimization, hardware, etc.
* `average_stack_base`: stack space which is usually occupied on start of calculation of percolate query.
* `desired_thread_stack`: sum of above values, rounded up to 128 bytes edge. If this value is greater than `thread_stack`, you may not execute `call pq` over this table, as some stored queries will fail. Default `thread_stack` value is 1M (which is 1048576); other values should be configured.
* `tid` and `tid_saved`: represent the state of saving the table. `tid` increases with each change (transaction). `tid_saved` shows the max `tid` of the state saved in a RAM chunk in `<table>.ram` file. When the numbers differ, some changes exist only in RAM and are also backed by binlog (if enabled). Performing `FLUSH TABLE` or scheduling periodic flushing saves these changes. After flushing, the binlog is cleared, and `tid_saved` represents the new actual state.
* `query_time_*`, `exact_query_time_*`: query execution time statistics for the last 1 minute, 5 minutes, 15 minutes, and total since server start; data is encapsulated as a JSON object, including the number of queries and min, max, avg, 95, and 99 percentile values.
* `found_rows_*`: statistics of rows found by queries; provided for the last 1 minute, 5 minutes, 15 minutes, and total since server start; data is encapsulated as a JSON object, including the number of queries and min, max, avg, 95, and 99 percentile values.
* `command_*`: counters for the total number of times a specific command has been successfully executed against this table.
* `search_stats_ms_*`: statistics on the execution time (in milliseconds) for search queries. The * indicates the time window (e.g., 1min, 5min, 15min, total). These stats are calculated over sliding windows of 1, 5, and 15 minutes, showing average, minimum, maximum, and 95th/99th percentile values for query times.
* `insert_replace_stats_ms_*`: statistics on the execution time (in milliseconds) for insert and replace queries. The * indicates the time window (e.g., 1min, 5min, 15min, total). These stats are calculated over sliding windows of 1, 5, and 15 minutes, showing average, minimum, maximum, and 95th/99th percentile values for query times.
* `update_stats_ms_*`: statistics on the execution time (in milliseconds) for update queries. The * indicates the time window (e.g., 1min, 5min, 15min, total). These stats are calculated over sliding windows of 1, 5, and 15 minutes, showing average, minimum, maximum, and 95th/99th percentile values for query times.

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
