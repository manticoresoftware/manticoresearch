# SHOW INDEX STATUS 

<!-- example SHOW INDEX STATUS -->

`SHOW INDEX STATUS` is an SQL statement that displays various per-index statistics.

The syntax is:

```sql
SHOW INDEX index_name STATUS
```

Displayed statistics include:

* `indexed_documents` and `indexed_bytes`: number of the documents indexed and their text size in bytes, respectively.
* `field_tokens_XXX`: sums of per-field lengths (in tokens) over the entire index (that is used internally in `BM25A` and `BM25F` functions for ranking purposes). Only available for indexes built with `index_field_lengths=1`.
* `ram_bytes`: total size (in bytes) of the RAM-resident index portion.
* `query_time_*`: query execution time statistics of last 1 minute, 5 minutes, 15 minutes and total since server start; data is encapsulated as a JSON object which includes the number of queries and min, max, avg, 95 and 99 percentile values.
* `found_rows_*`: statistics of rows found by queries; provided for last 1 minute, 5 minutes, 15 minutes and total since server start; data is encapsulated as a JSON object which includes the number of queries and min, max, avg, 95 and 99 percentile values.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
mysql> SHOW INDEX products STATUS;
```

<!-- response SQL -->

```sql
+--------------------------+--------------------------------------------------------------------------------------------------------+
| Variable_name            | Value                                                                                                  |
+--------------------------+--------------------------------------------------------------------------------------------------------+
| index_type               | rt                                                                                                     |
| indexed_documents        | 3                                                                                                      |
| indexed_bytes            | 0                                                                                                      |
| ram_bytes                | 6678                                                                                                   |
| disk_bytes               | 611                                                                                                    |
| ram_chunk                | 990                                                                                                    |
| ram_chunk_segments_count | 2                                                                                                      |
| disk_chunks              | 0                                                                                                      |
| mem_limit                | 134217728                                                                                              |
| ram_bytes_retired        | 0                                                                                                      |
| tid                      | 15                                                                                                     |
| tid_saved                | 8                                                                                                      |
| query_time_1min          | {"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001} |
| query_time_5min          | {"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001} |
| query_time_15min         | {"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001} |
| query_time_total         | {"queries":1, "avg_sec":0.001, "min_sec":0.001, "max_sec":0.001, "pct95_sec":0.001, "pct99_sec":0.001} |
| found_rows_1min          | {"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}                                         |
| found_rows_5min          | {"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}                                         |
| found_rows_15min         | {"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}                                         |
| found_rows_total         | {"queries":1, "avg":3, "min":3, "max":3, "pct95":3, "pct99":3}                                         |
+--------------------------+--------------------------------------------------------------------------------------------------------+
20 rows in set (0.00 sec)
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