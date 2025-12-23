# SHOW TABLE STATUS

<!-- example SHOW TABLE STATUS -->

`SHOW TABLE STATUS` 是一个显示每个表各种统计信息的 SQL 语句。

语法为：

```sql
SHOW TABLE table_name STATUS
```

根据索引类型，显示的统计信息包含不同的行集：

* **template**: `index_type`。
* **distributed**: `index_type`，`query_time_1min`，`query_time_5min`，`query_time_15min`，`query_time_total`，`exact_query_time_1min`，`exact_query_time_5min`，`exact_query_time_15min`，`exact_query_time_total`，`found_rows_1min`，`found_rows_5min`，`found_rows_15min`，`found_rows_total`。
* **percolate**: `index_type`，`stored_queries`，`ram_bytes`，`disk_bytes`，`max_stack_need`，`average_stack_base`，`
  desired_thread_stack`，`tid`，`tid_saved`，`query_time_1min`，`query_time_5min`，`query_time_15min`，`query_time_total`，`exact_query_time_1min`，`exact_query_time_5min`，`exact_query_time_15min`，`exact_query_time_total`，`found_rows_1min`，`found_rows_5min`，`found_rows_15min`，`found_rows_total`。
* **plain**: `index_type`，`indexed_documents`，`indexed_bytes`，可能包含一组 `field_tokens_*` 和 `total_tokens`，`ram_bytes`，`disk_bytes`，`disk_mapped`，`disk_mapped_cached`，`disk_mapped_doclists`，`disk_mapped_cached_doclists`，`disk_mapped_hitlists`，`disk_mapped_cached_hitlists`，`killed_documents`，`killed_rate`，`query_time_1min`，`query_time_5min`，`query_time_15min`，`query_time_total`，`exact_query_time_1min`，`exact_query_time_5min`，`exact_query_time_15min`，`exact_query_time_total`，`found_rows_1min`，`found_rows_5min`，`found_rows_15min`，`found_rows_total`。
* **rt**: `index_type`，`indexed_documents`，`indexed_bytes`，可能包含一组 `field_tokens_*` 和 `total_tokens`，`ram_bytes`，`disk_bytes`，`disk_mapped`，`disk_mapped_cached`，`disk_mapped_doclists`，`disk_mapped_cached_doclists`，`disk_mapped_hitlists`，`disk_mapped_cached_hitlists`，`killed_documents`，`killed_rate`，`ram_chunk`，`ram_chunk_segments_count`，`disk_chunks`，`mem_limit`，`mem_limit_rate`，`ram_bytes_retired`，`optimizing`，`locked`，`tid`，`tid_saved`，`query_time_1min`，`query_time_5min`，`query_time_15min`，`query_time_total`，`exact_query_time_1min`，`exact_query_time_5min`，`exact_query_time_15min`，`exact_query_time_total`，`found_rows_1min`，`found_rows_5min`，`found_rows_15min`，`found_rows_total`。

以下是这些值的含义：

* `index_type`：当前为 `disk`、`rt`、`percolate`、`template` 和 `distributed` 之一。
* `indexed_documents`：已索引文档的数量。
* `indexed_bytes`：已索引文本的总体大小。注意，该值不是严格的，因为在全文索引中不可能严格还原存储的文本来测量它。
* `stored_queries`：表中存储的 percolate 查询数量。
* `field_tokens_XXX`：可选，整个表中每个字段的总长度（以令牌计）（内部用于 `BM25A` 和 `BM25F` 排名函数）。仅适用于使用 `index_field_lengths=1` 构建的表。
* `total_tokens`：可选，所有 `field_tokens_XXX` 的总和。
* `ram_bytes`：表占用的总 RAM。
* `disk_bytes`：表占用的总磁盘空间。
* `disk_mapped`：文件映射的总大小。
* `disk_mapped_cached`：实际缓存于 RAM 中的文件映射总大小。
* `disk_mapped_doclists` 和 `disk_mapped_cached_doclists`：属于文档列表的总映射和缓存映射部分。
* `disk_mapped_hitlists` 和 `disk_mapped_cached_hitlists`：属于命中列表的总映射和缓存映射部分。文档列表和命中列表的值分开显示，因为它们通常很大（例如，大约占整个表大小的 90%）。
* `killed_documents` 和 `killed_rate`：前者表示已删除文档的数量，后者表示删除文档与已索引文档的比例。从技术上讲，删除文档意味着在搜索结果中屏蔽它，但它仍然物理存在于表中，只有在合并/优化表后才会被清除。
* `ram_chunk`：实时或 percolate 表的 RAM 块大小。
* `ram_chunk_segments_count`：RAM 块内部由段组成，通常不超过 32。此行显示当前段数。
* `disk_chunks`：实时表中的磁盘块数量。
* `mem_limit`：表的 `rt_mem_limit` 实际值。
* `mem_limit_rate`：RAM 块被刷新为磁盘块的比例，例如，如果 `rt_mem_limit` 是 128M，比例是 50%，当 RAM 块超过 64M 时将保存一个新的磁盘块。
* `ram_bytes_retired`：表示 RAM 块中的垃圾大小（例如，已删除或替换但尚未永久移除的文档）。
* `optimizing`：值大于 0 表示表当前正在执行优化（即正在合并某些磁盘块）。
* `locked`：值大于 0 表示表当前被 [FREEZE](../../Securing_and_compacting_a_table/Freezing_and_locking_a_table.md#Freezing-a-table) 锁定。数字表示表被冻结的次数。例如，表可能先被 `manticore-backup` 冻结，然后又被复制冻结。只有当没有其他进程需要冻结时，才应完全解冻。
* `max_stack_need`：计算存储的 percolate 查询中最复杂部分所需的栈空间。这是动态值，取决于构建细节如编译器、优化、硬件等。
* `average_stack_base`：通常在开始计算 percolate 查询时占用的栈空间。
* `desired_thread_stack`：上述值的总和，向上取整到 128 字节边界。如果此值大于 `thread_stack`，则可能无法在此表上执行 `call pq`，因为某些存储的查询会失败。默认 `thread_stack` 值为 1M（即 1048576）；其他值应自行配置。
* `tid` 和 `tid_saved`：表示保存表的状态。`tid` 随每次更改（事务）递增。`tid_saved` 显示保存在 `<table>.ram` 文件中的 RAM 块状态的最大 `tid`。当数字不同时，表示有些更改仅存在于 RAM 中，同时也由 binlog 备份（如果启用）。执行 `FLUSH TABLE` 或安排周期性刷新将保存这些更改。刷新后，binlog 被清除，`tid_saved` 表示新的实际状态。
* `query_time_*`、`exact_query_time_*`：查询执行时间统计，针对最近 1 分钟、5 分钟、15 分钟和自服务器启动以来的总时间；数据封装为 JSON 对象，包括查询数量以及最小值、最大值、平均值、95 和 99 百分位值。
* `found_rows_*`：查询发现的行数统计；提供最近 1 分钟、5 分钟、15 分钟和自服务器启动以来的总数据；数据封装为 JSON 对象，包括查询数量以及最小值、最大值、平均值、95 和 99 百分位值。
* `command_*`：对该表成功执行特定命令的总次数计数器。
* `search_stats_ms_*`：搜索查询执行时间（毫秒）统计。* 表示时间窗口（例如 1min、5min、15min、total）。这些统计在 1、5 和 15 分钟的滑动窗口内计算，显示查询时间的平均值、最小值、最大值及 95/99 百分位值。
* `insert_replace_stats_ms_*`：插入和替换查询的执行时间（毫秒）统计。* 表示时间窗口（例如 1min、5min、15min、total）。这些统计在 1、5 和 15 分钟的滑动窗口内计算，显示查询时间的平均值、最小值、最大值及 95/99 百分位值。
* `update_stats_ms_*`：更新查询的执行时间（毫秒）统计。* 表示时间窗口（例如 1min、5min、15min、total）。这些统计在 1、5 和 15 分钟的滑动窗口内计算，显示查询时间的平均值、最小值、最大值及 95/99 百分位值。

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
##### JSON:
<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "SHOW TABLE t STATUS"
```

<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "Variable_name": {
          "type": "string"
        }
      },
      {
        "Value": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "Variable_name": "table_type",
        "Value": "rt"
      },
      {
        "Variable_name": "indexed_documents",
        "Value": "5"
      },
      {
        "Variable_name": "indexed_bytes",
        "Value": "28"
      },
      {
        "Variable_name": "ram_bytes",
        "Value": "23400"
      },
      {
        "Variable_name": "disk_bytes",
        "Value": "1969"
      },
      {
        "Variable_name": "disk_mapped",
        "Value": "196"
      },
      {
        "Variable_name": "disk_mapped_cached",
        "Value": "16384"
      },
      {
        "Variable_name": "disk_mapped_doclists",
        "Value": "0"
      },
      {
        "Variable_name": "disk_mapped_cached_doclists",
        "Value": "0"
      },
      {
        "Variable_name": "disk_mapped_hitlists",
        "Value": "0"
      },
      {
        "Variable_name": "disk_mapped_cached_hitlists",
        "Value": "0"
      },
      {
        "Variable_name": "killed_documents",
        "Value": "0"
      },
      {
        "Variable_name": "killed_rate",
        "Value": "0.00%"
      },
      {
        "Variable_name": "ram_chunk",
        "Value": "0"
      },
      {
        "Variable_name": "ram_chunk_segments_count",
        "Value": "0"
      },
      {
        "Variable_name": "disk_chunks",
        "Value": "1"
      },
      {
        "Variable_name": "mem_limit",
        "Value": "134217728"
      },
      {
        "Variable_name": "mem_limit_rate",
        "Value": "95.00%"
      },
      {
        "Variable_name": "ram_bytes_retired",
        "Value": "0"
      },
      {
        "Variable_name": "optimizing",
        "Value": "0"
      },
      {
        "Variable_name": "locked",
        "Value": "0"
      },
      {
        "Variable_name": "tid",
        "Value": "7"
      },
      {
        "Variable_name": "tid_saved",
        "Value": "7"
      },
      {
        "Variable_name": "query_time_1min",
        "Value": "{\"queries\":0, \"avg\":\"-\", \"min\":\"-\", \"max\":\"-\", \"pct95\":\"-\", \"pct99\":\"-\"}"
      },
      {
        "Variable_name": "query_time_5min",
        "Value": "{\"queries\":0, \"avg\":\"-\", \"min\":\"-\", \"max\":\"-\", \"pct95\":\"-\", \"pct99\":\"-\"}"
      },
      {
        "Variable_name": "query_time_15min",
        "Value": "{\"queries\":0, \"avg\":\"-\", \"min\":\"-\", \"max\":\"-\", \"pct95\":\"-\", \"pct99\":\"-\"}"
      },
      {
        "Variable_name": "query_time_total",
        "Value": "{\"queries\":30, \"avg_sec\":0.324, \"min_sec\":0.051, \"max_sec\":1.718, \"pct95_sec\":1.017, \"pct99_sec\":1.718}"
      },
      {
        "Variable_name": "found_rows_1min",
        "Value": "{\"queries\":0, \"avg\":\"-\", \"min\":\"-\", \"max\":\"-\", \"pct95\":\"-\", \"pct99\":\"-\"}"
      },
      {
        "Variable_name": "found_rows_5min",
        "Value": "{\"queries\":0, \"avg\":\"-\", \"min\":\"-\", \"max\":\"-\", \"pct95\":\"-\", \"pct99\":\"-\"}"
      },
      {
        "Variable_name": "found_rows_15min",
        "Value": "{\"queries\":0, \"avg\":\"-\", \"min\":\"-\", \"max\":\"-\", \"pct95\":\"-\", \"pct99\":\"-\"}"
      },
      {
        "Variable_name": "found_rows_total",
        "Value": "{\"queries\":30, \"avg\":2, \"min\":0, \"max\":5, \"pct95\":5, \"pct99\":5}"
      },
      {
        "Variable_name": "command_search",
        "Value": "30"
      },
      {
        "Variable_name": "command_excerpt",
        "Value": "3"
      },
      {
        "Variable_name": "command_update",
        "Value": "0"
      },
      {
        "Variable_name": "command_keywords",
        "Value": "0"
      },
      {
        "Variable_name": "command_status",
        "Value": "1"
      },
      {
        "Variable_name": "command_delete",
        "Value": "1"
      },
      {
        "Variable_name": "command_insert",
        "Value": "6"
      },
      {
        "Variable_name": "command_replace",
        "Value": "0"
      },
      {
        "Variable_name": "command_commit",
        "Value": "0"
      },
      {
        "Variable_name": "command_suggest",
        "Value": "0"
      },
      {
        "Variable_name": "command_callpq",
        "Value": "0"
      },
      {
        "Variable_name": "command_getfield",
        "Value": "0"
      },
      {
        "Variable_name": "insert_replace_stats_ms_avg",
        "Value": "N/A N/A N/A"
      },
      {
        "Variable_name": "insert_replace_stats_ms_min",
        "Value": "N/A N/A N/A"
      },
      {
        "Variable_name": "insert_replace_stats_ms_max",
        "Value": "N/A N/A N/A"
      },
      {
        "Variable_name": "insert_replace_stats_ms_pct95",
        "Value": "N/A N/A N/A"
      },
      {
        "Variable_name": "insert_replace_stats_ms_pct99",
        "Value": "N/A N/A N/A"
      },
      {
        "Variable_name": "search_stats_ms_avg",
        "Value": "N/A N/A N/A"
      },
      {
        "Variable_name": "search_stats_ms_min",
        "Value": "N/A N/A N/A"
      },
      {
        "Variable_name": "search_stats_ms_max",
        "Value": "N/A N/A N/A"
      },
      {
        "Variable_name": "search_stats_ms_pct95",
        "Value": "N/A N/A N/A"
      },
      {
        "Variable_name": "search_stats_ms_pct99",
        "Value": "N/A N/A N/A"
      },
      {
        "Variable_name": "update_stats_ms_avg",
        "Value": "N/A N/A N/A"
      },
      {
        "Variable_name": "update_stats_ms_min",
        "Value": "N/A N/A N/A"
      },
      {
        "Variable_name": "update_stats_ms_max",
        "Value": "N/A N/A N/A"
      },
      {
        "Variable_name": "update_stats_ms_pct95",
        "Value": "N/A N/A N/A"
      },
      {
        "Variable_name": "update_stats_ms_pct99",
        "Value": "N/A N/A N/A"
      }
    ],
    "total": 58,
    "error": "",
    "warning": ""
  }
]
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

