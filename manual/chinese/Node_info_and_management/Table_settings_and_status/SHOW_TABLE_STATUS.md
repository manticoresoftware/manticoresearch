# SHOW TABLE STATUS

<!-- example SHOW TABLE STATUS -->

`SHOW TABLE STATUS` 是一个 SQL 语句，用于显示每个表的各种统计信息。

语法如下：

```sql
SHOW TABLE table_name STATUS
```

根据索引类型，显示的统计信息包括不同的行集：

* **template**: `index_type`。
* **distributed**: `index_type`，`query_time_1min`，`query_time_5min`，`query_time_15min`，`query_time_total`，`exact_query_time_1min`，`exact_query_time_5min`，`exact_query_time_15min`，`exact_query_time_total`，`found_rows_1min`，`found_rows_5min`，`found_rows_15min`，`found_rows_total`。
* **percolate**: `index_type`，`stored_queries`，`ram_bytes`，`disk_bytes`，`max_stack_need`，`average_stack_base`，`
  desired_thread_stack`，`tid`，`tid_saved`，`query_time_1min`，`query_time_5min`，`query_time_15min`，`query_time_total`，`exact_query_time_1min`，`exact_query_time_5min`，`exact_query_time_15min`，`exact_query_time_total`，`found_rows_1min`，`found_rows_5min`，`found_rows_15min`，`found_rows_total`。
* **plain**: `index_type`，`indexed_documents`，`indexed_bytes`，可能包括 `field_tokens_*` 和 `total_tokens` 集合，`ram_bytes`，`disk_bytes`，`disk_mapped`，`disk_mapped_cached`，`disk_mapped_doclists`，`disk_mapped_cached_doclists`，`disk_mapped_hitlists`，`disk_mapped_cached_hitlists`，`killed_documents`，`killed_rate`，`query_time_1min`，`query_time_5min`，`query_time_15min`，`query_time_total`，`exact_query_time_1min`，`exact_query_time_5min`，`exact_query_time_15min`，`exact_query_time_total`，`found_rows_1min`，`found_rows_5min`，`found_rows_15min`，`found_rows_total`。
* **rt**: `index_type`，`indexed_documents`，`indexed_bytes`，可能包括 `field_tokens_*` 和 `total_tokens` 集合，`ram_bytes`，`disk_bytes`，`disk_mapped`，`disk_mapped_cached`，`disk_mapped_doclists`，`disk_mapped_cached_doclists`，`disk_mapped_hitlists`，`disk_mapped_cached_hitlists`，`killed_documents`，`killed_rate`，`ram_chunk`，`ram_chunk_segments_count`，`disk_chunks`，`mem_limit`，`mem_limit_rate`，`ram_bytes_retired`，`optimizing`，`locked`，`tid`，`tid_saved`，`query_time_1min`，`query_time_5min`，`query_time_15min`，`query_time_total`，`exact_query_time_1min`，`exact_query_time_5min`，`exact_query_time_15min`，`exact_query_time_total`，`found_rows_1min`，`found_rows_5min`，`found_rows_15min`，`found_rows_total`。

以下是这些值的含义：

* `index_type`：当前取值为 `disk`、`rt`、`percolate`、`template` 和 `distributed` 之中的一种。
* `indexed_documents`：已索引文档的数量。
* `indexed_bytes`：已索引文本的总体大小。请注意，该值不是严格准确的，因为在全文索引中不可能严格地将存储的文本恢复以进行测量。
* `stored_queries`：存储在表中的 percolate 查询数量。
* `field_tokens_XXX`：可选，整个表中各字段的总长度（以令牌计），内部用于 `BM25A` 和 `BM25F` 排序函数。仅适用于使用 `index_field_lengths=1` 构建的表。
* `total_tokens`：可选，所有 `field_tokens_XXX` 的整体和。
* `ram_bytes`：表占用的总 RAM。
* `disk_bytes`：表占用的总磁盘空间。
* `disk_mapped`：文件映射的总大小。
* `disk_mapped_cached`：实际缓存在 RAM 中的文件映射大小。
* `disk_mapped_doclists` 和 `disk_mapped_cached_doclists`：属于文档列表的总映射和缓存映射部分。
* `disk_mapped_hitlists` 和 `disk_mapped_cached_hitlists`：属于命中列表的总映射和缓存映射部分。由于文档列表和命中列表通常较大（例如，占整个表大小约90%），故这两个值单独显示。
* `killed_documents` 和 `killed_rate`：分别表示已删除文档的数量及删除文档占已索引文档的比例。从技术上讲，删除文档意味着在搜索输出中屏蔽，但文档仍物理存在于表中，只有在合并/优化表后才会被真正清除。
* `ram_chunk`：实时或 percolate 表的 RAM 块大小。
* `ram_chunk_segments_count`：RAM 块内部由多个段组成，通常不超过 32 个。本行显示当前段数。
* `disk_chunks`：实时表中的磁盘块数量。
* `mem_limit`：表的 `rt_mem_limit` 实际值。
* `mem_limit_rate`：RAM 块被刷写成磁盘块的比例，例如，若 `rt_mem_limit` 为 128M，且比例为 50%，当 RAM 块超过 64M 时将保存一个新的磁盘块。
* `ram_bytes_retired`：表示 RAM 块内的垃圾大小（如已删除或替换但未永久移除的文档）。
* `optimizing`：大于 0 表明表当前正在执行优化（即正在合并某些磁盘块）。
* `locked`：大于 0 表示表当前被 [FREEZE](../../Securing_and_compacting_a_table/Freezing_and_locking_a_table.md#Freezing-a-table) 锁定。数字表示表被冻结的次数。例如，表可能被 `manticore-backup` 冻结一次，接着被复制功能再次冻结。只有当没有其它进程需要冻结时，表才能完全解冻。
* `max_stack_need`：计算存储的 percolate 查询时所需的最大堆栈空间。该值是动态的，取决于编译器、优化、硬件等构建详情。
* `average_stack_base`：通常在启动 percolate 查询计算时占用的堆栈空间。
* `desired_thread_stack`：以上两值之和，向上取整至 128 字节的整数倍。如果该值大于 `thread_stack`，则可能无法在此表上执行 `call pq`，因为某些存储查询会失败。默认的 `thread_stack` 为 1M (即1048576); 其它值需用户配置。
* `tid` 和 `tid_saved`：表示表保存状态。`tid` 随每次变更（事务）递增。`tid_saved` 显示在 `<table>.ram` 文件中保存的 RAM 块的最大 `tid`。当两值不同时，表示有些变更仅存在于 RAM 中，并由二进制日志支持（如果启用）。执行 `FLUSH TABLE` 或安排定期刷新会保存这些变更。刷新后，binlog 被清空，`tid_saved` 表示新的实际状态。
* `query_time_*`、`exact_query_time_*`：最近 1 分钟、5 分钟、15 分钟以及自服务器启动以来的查询执行时间统计；数据封装为 JSON 对象，包括查询数量及最小、最大、平均、95% 和 99% 百分位值。
* `found_rows_*`：查询所找到的行数统计；分别提供最近 1 分钟、5 分钟、15 分钟以及自服务器启动以来的统计；数据封装为 JSON 对象，包括查询数量及最小、最大、平均、95% 和 99% 百分位值。
* `command_*`：特定命令成功执行于此表的总次数计数器。
* `search_stats_ms_*`：搜索查询的执行时间（以毫秒为单位）统计信息。* 表示时间窗口（例如，1 分钟、5 分钟、15 分钟、总时间）。这些统计信息是基于 1、5 和 15 分钟的滑动窗口计算的，显示查询时间的平均值、最小值、最大值和 95%、99% 分位数值。对于分布式表，这些值反映本地和远程表贡献的聚合值，可能高于单个节点级别的查询墙/实际值。
* `insert_replace_stats_ms_*`：插入和替换查询执行时间（毫秒）的统计。*表示时间窗口（例如 1min, 5min, 15min, total）。这些统计在 1、5 和 15 分钟滑动窗口内计算，显示查询时间的平均值、最小值、最大值以及 95% / 99% 百分位数。
* `update_stats_ms_*`：更新查询执行时间（毫秒）的统计。*表示时间窗口（例如 1min, 5min, 15min, total）。这些统计在 1、5 和 15 分钟滑动窗口内计算，显示查询时间的平均值、最小值、最大值以及 95% / 99% 百分位数。

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

