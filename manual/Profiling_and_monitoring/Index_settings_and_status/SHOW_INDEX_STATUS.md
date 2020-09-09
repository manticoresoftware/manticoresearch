# SHOW INDEX STATUS 

<!-- example SHOW INDEX STATUS -->

`SHOW INDEX STATUS` is an SQL statement that displays various per-index statistics.

The syntax is:

```sql
SHOW INDEX index_name STATUS
```

Displayed statistics include:

* `index_type` show type, for now that is one of `disk`, `rt`, `percolate`, `template`, and `distributed`.
* `indexed_documents` and `indexed_bytes`: number of the documents indexed and their text size in bytes, respectively.
* `field_tokens_XXX`: sums of per-field lengths (in tokens) over the entire index (that is used internally in `BM25A` and `BM25F` functions for ranking purposes). Only available for indexes built with `index_field_lengths=1`.
* `ram_bytes`: total size (in bytes) of the RAM-resident index portion.
* `disk_bytes`: total size (in bytes) of all index files.
* `disk_mapped`: total size of file mappings, except doclists and hitlists files.
* `disk_mapped_cached`: total size of file mappings, cached in RAM.
* `ram_chunk`: size of RAM chunk of realtime or percolate index.
* `ram_chunk_segments_count`: RAM chunk internally consists from segments, usually there are no more than 32 of them.
* `disk_chunks`: number of disk chunks of realtime index.
* `mem_limit`: actual value of `rt_mem_limit` for the index.
* `ram_bytes_retired`: represents size of garbage in RAM chunk (say, deleted or replaced documents not yet finally wiped away).
* `tid` and `tid_saved`: represent save state or rt or percolate index. `tid` increased with each change (transaction). `tid_saved` represents max `tid` of the state saved in RAM chunk in '<index>.ram' file. When numbers are different, some changes are exist only in RAM, and also backed by binlog (if enabled). Performing 'flush rtindex', or scheduled periodical flush causes these changes to be saved. After flush binlog is cleared, and `tid_saved` represents actual new state.
* `query_time_*`: query execution time statistics of last 1 minute, 5 minutes, 15 minutes and total since server start; data is encapsulated as a JSON object which includes the number of queries and min, max, avg, 95 and 99 percentile values.
* `found_rows_*`: statistics of rows found by queries; provided for last 1 minute, 5 minutes, 15 minutes and total since server start; data is encapsulated as a JSON object which includes the number of queries and min, max, avg, 95 and 99 percentile values.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
mysql> SHOW INDEX statistic STATUS;
```

<!-- response SQL -->

```sql
+--------------------------+---------------------------------------------------------------------------------------------------------+
| Variable_name            | Value                                                                                                   |
+--------------------------+---------------------------------------------------------------------------------------------------------+
| index_type               | rt                                                                                                      |
| indexed_documents        | 999999                                                                                                  |
| indexed_bytes            | 55130037                                                                                                |
| ram_bytes                | 3802088499                                                                                              |
| disk_bytes               | 7658644242                                                                                              |
| disk_mapped              | 7314814402                                                                                              |
| disk_mapped_cached       | 3491561472                                                                                              |
| ram_chunk                | 310461171                                                                                               |
| ram_chunk_segments_count | 28                                                                                                      |
| disk_chunks              | 15                                                                                                      |
| mem_limit                | 536870912                                                                                               |
| ram_bytes_retired        | 0                                                                                                       |
| tid                      | 0                                                                                                       |
| tid_saved                | 0                                                                                                       |
| query_time_1min          | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"}                                |
| query_time_5min          | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"}                                |
| query_time_15min         | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"}                                |
| query_time_total         | {"queries":27, "avg_sec":0.002, "min_sec":0.001, "max_sec":0.013, "pct95_sec":0.003, "pct99_sec":0.013} |
| found_rows_1min          | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"}                                |
| found_rows_5min          | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"}                                |
| found_rows_15min         | {"queries":0, "avg":"-", "min":"-", "max":"-", "pct95":"-", "pct99":"-"}                                |
| found_rows_total         | {"queries":27, "avg":1, "min":1, "max":1, "pct95":1, "pct99":1}                                         |
+--------------------------+---------------------------------------------------------------------------------------------------------+
22 rows in set (0,35 sec)
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
<!-- end -->