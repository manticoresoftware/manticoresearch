# SHOW META

```sql
SHOW META [ LIKE pattern ]
```

<!-- example show meta -->
`SHOW META` 是一个 SQL 语句，用于显示有关处理查询的附加元信息，包括查询时间、关键字统计以及使用的二级索引信息。语法如下：

包含的项目有：
* `total`：实际检索并发送给客户端的匹配数。该值通常受 [LIMIT/size](../Searching/Pagination.md#Pagination-of-search-results) 搜索选项的限制。
* `total_found`：
  - 索引中查询的估计匹配总数。如果需要精确的匹配数，请使用 `SELECT COUNT(*)`，而不是依赖此值。
  - 对于带有 `GROUP BY` 的查询，`total_found` 表示组的数量，而非单个匹配数。
  - 对于 [GROUP N BY](../Searching/Grouping.md#Give-me-N-rows) 查询，`total_found` 仍表示组的数量，无论 `N` 的值是多少。
* `total_relation`：指示 `total_found` 值是精确的还是估计的。
  - 如果 Manticore 无法确定精确的 `total_found` 值，此字段将显示 `total_relation: gte`，表示实际匹配数**大于或等于**报告的 `total_found`。
  - 如果 `total_found` 值是精确的，则显示 `total_relation: eq`。
* `time`：处理搜索查询所用的时间（秒）。
* `keyword[N]`：搜索查询中使用的第 n 个关键字。注意关键字可以是通配符形式，例如 `abc*`。
* `docs[N]`：包含搜索查询中第 n 个关键字的文档（或记录）总数。如果关键字是通配符形式，此值表示所有展开子关键字的文档总和，可能超过实际匹配文档数。
* `hits[N]`：第 n 个关键字在所有文档中出现的总次数（命中数）。
* `index`：所使用索引的信息（例如二级索引）。

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SELECT id, story_author FROM hn_small WHERE MATCH('one|two|three') and comment_ranking > 2 limit 5;
show meta;
```

<!-- response SQL -->

```sql
+---------+--------------+
| id      | story_author |
+---------+--------------+
|  151171 | anewkid      |
|  302758 | bks          |
|  805806 | drRoflol     |
| 1099245 | tnorthcutt   |
|  303252 | whiten       |
+---------+--------------+
5 rows in set (0.00 sec)

+----------------+---------------------------------------+
| Variable_name  | Value                                 |
+----------------+---------------------------------------+
| total          | 5                                     |
| total_found    | 2308                                  |
| total_relation | eq                                    |
| time           | 0.001                                 |
| keyword[0]     | one                                   |
| docs[0]        | 224387                                |
| hits[0]        | 310327                                |
| keyword[1]     | three                                 |
| docs[1]        | 18181                                 |
| hits[1]        | 21102                                 |
| keyword[2]     | two                                   |
| docs[2]        | 63251                                 |
| hits[2]        | 75961                                 |
| index          | comment_ranking:SecondaryIndex (100%) |
+----------------+---------------------------------------+
14 rows in set (0.00 sec)
```

<!-- end -->

<!-- example show meta iostats cpustats -->
`SHOW META` 可以显示 I/O 和 CPU 计数器，但仅当 searchd 启动时带有 `--iostats` 和 `--cpustats` 参数时才可用。

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SELECT id,channel_id FROM records WHERE MATCH('one|two|three') limit 5;

SHOW META;
```

<!-- response SQL -->

```sql
+--------+--------------+
| id     | story_author |
+--------+--------------+
| 300263 | throwaway37  |
| 713503 | mahmud       |
| 716804 | mahmud       |
| 776906 | jimbokun     |
| 753332 | foxhop       |
+--------+--------------+
5 rows in set (0.01 sec)

+-----------------------+--------+
| Variable_name         | Value  |
+-----------------------+--------+
| total                 | 5      |
| total_found           | 266385 |
| total_relation        | eq     |
| time                  | 0.011  |
| cpu_time              | 18.004 |
| agents_cpu_time       | 0.000  |
| io_read_time          | 0.000  |
| io_read_ops           | 0      |
| io_read_kbytes        | 0.0    |
| io_write_time         | 0.000  |
| io_write_ops          | 0      |
| io_write_kbytes       | 0.0    |
| agent_io_read_time    | 0.000  |
| agent_io_read_ops     | 0      |
| agent_io_read_kbytes  | 0.0    |
| agent_io_write_time   | 0.000  |
| agent_io_write_ops    | 0      |
| agent_io_write_kbytes | 0.0    |
| keyword[0]            | one    |
| docs[0]               | 224387 |
| hits[0]               | 310327 |
| keyword[1]            | three  |
| docs[1]               | 18181  |
| hits[1]               | 21102  |
| keyword[2]            | two    |
| docs[2]               | 63251  |
| hits[2]               | 75961  |
+-----------------------+--------+
27 rows in set (0.00 sec)
```

<!-- end -->

<!-- example show meta predicted_time -->
额外的值，如 `predicted_time`、`dist_predicted_time`、`local_fetched_docs`、`local_fetched_hits`、`local_fetched_skips` 及其对应的 `dist_fetched_*`，仅在 `searchd` 配置了[预测时间成本](../Server_settings/Searchd.md#predicted_time_costs)且查询的 `OPTION` 子句中包含 `predicted_time` 时可用。

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SELECT id,story_author FROM hn_small WHERE MATCH('one|two|three') limit 5 option max_predicted_time=100;

SHOW META;
```

<!-- response SQL -->

```sql
+--------+--------------+
| id     | story_author |
+--------+--------------+
| 300263 | throwaway37  |
| 713503 | mahmud       |
| 716804 | mahmud       |
| 776906 | jimbokun     |
| 753332 | foxhop       |
+--------+--------------+
5 rows in set (0.01 sec)

mysql> show meta;
+---------------------+--------+
| Variable_name       | Value  |
+---------------------+--------+
| total               | 5      |
| total_found         | 266385 |
| total_relation      | eq     |
| time                | 0.012  |
| local_fetched_docs  | 307212 |
| local_fetched_hits  | 407390 |
| local_fetched_skips | 24     |
| predicted_time      | 56     |
| keyword[0]          | one    |
| docs[0]             | 224387 |
| hits[0]             | 310327 |
| keyword[1]          | three  |
| docs[1]             | 18181  |
| hits[1]             | 21102  |
| keyword[2]          | two    |
| docs[2]             | 63251  |
| hits[2]             | 75961  |
+---------------------+--------+
17 rows in set (0.00 sec)
```

<!-- end -->

<!-- example show meta single statement -->

`SHOW META` 必须在**同一**会话中紧接查询之后执行。由于某些 MySQL 连接器/库使用连接池，单独执行 `SHOW META` 可能导致意外结果，例如获取到其他查询的元数据。在这些情况下（且通常推荐），应执行包含查询和 `SHOW META` 的多语句。一些连接器/库支持在同一方法中执行多查询，而其他可能需要专用方法或在连接设置时配置特定选项。

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SELECT id,story_author FROM hn_small WHERE MATCH('one|two|three') LIMIT 5; SHOW META;
```

<!-- response SQL -->

```sql
+--------+--------------+
| id     | story_author |
+--------+--------------+
| 300263 | throwaway37  |
| 713503 | mahmud       |
| 716804 | mahmud       |
| 776906 | jimbokun     |
| 753332 | foxhop       |
+--------+--------------+
5 rows in set (0.01 sec)

+----------------+--------+
| Variable_name  | Value  |
+----------------+--------+
| total          | 5      |
| total_found    | 266385 |
| total_relation | eq     |
| time           | 0.011  |
| keyword[0]     | one    |
| docs[0]        | 224387 |
| hits[0]        | 310327 |
| keyword[1]     | three  |
| docs[1]        | 18181  |
| hits[1]        | 21102  |
| keyword[2]     | two    |
| docs[2]        | 63251  |
| hits[2]        | 75961  |
+----------------+--------+
13 rows in set (0.00 sec)
```

<!-- end -->

<!-- example SHOW META LIKE -->

你也可以使用可选的 LIKE 子句，只选择匹配特定模式的变量。模式语法遵循标准 SQL 通配符，其中 `%` 表示任意数量的任意字符，`_` 表示单个字符。

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SHOW META LIKE 'total%';
```

<!-- response SQL -->

```sql
+----------------+--------+
| Variable_name  | Value  |
+----------------+--------+
| total          | 5      |
| total_found    | 266385 |
| total_relation | eq     |
+----------------+--------+
3 rows in set (0.00 sec)
```

<!-- end -->

## SHOW META 和 facets

<!-- example show meta facets -->

使用[分面搜索](../Searching/Faceted_search.md)时，可以查看 `SHOW META` 输出中的 `multiplier` 字段，以确定优化分组中执行了多少查询。

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SELECT * FROM facetdemo FACET brand_id FACET price FACET categories;
SHOW META LIKE 'multiplier';
```

<!-- response SQL -->

```sql
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
| id   | price | brand_id | title               | brand_name  | property    | j                                     | categories |
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
|    1 |   306 |        1 | Product Ten Three   | Brand One   | Six_Ten     | {"prop1":66,"prop2":91,"prop3":"One"} | 10,11      |
...

+----------+----------+
| brand_id | count(*) |
+----------+----------+
|        1 |     1013 |
...

+-------+----------+
| price | count(*) |
+-------+----------+
|   306 |        7 |
...

+------------+----------+
| categories | count(*) |
+------------+----------+
|         10 |     2436 |
...

+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| multiplier    | 4     |
+---------------+-------+
1 row in set (0.00 sec)
```

<!-- end -->

## SHOW META 和查询优化器

<!-- example of show meta vs query optimizer -->

当[基于成本的查询优化器](../Searching/Cost_based_optimizer.md)选择使用 `DocidIndex`、`ColumnarScan` 或 `SecondaryIndex` 而非简单过滤器时，`SHOW META` 命令会反映这一点。

`index` 变量显示查询执行期间使用的二级索引的名称和类型。百分比表示使用二级索引的磁盘块数量（RT 表情况）或伪分片数量（普通表情况）。

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SELECT count(*) FROM taxi1 WHERE tip_amount = 5;
SHOW META;
```

<!-- response SQL -->

```sql
+----------------+----------------------------------+
| Variable_name  | Value                            |
+----------------+----------------------------------+
| total          | 1                                |
| total_found    | 1                                |
| total_relation | eq                               |
| time           | 0.016                            |
| index          | tip_amount:SecondaryIndex (100%) |
+----------------+----------------------------------+
5 rows in set (0.00 sec)
```

<!-- end -->

## SHOW META 用于 PQ 表

<!-- example show meta PQ -->

`SHOW META` 可在执行[CALL PQ](../Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ) 语句后使用，此时输出不同。

`CALL PQ` 语句后执行的 `SHOW META` 包含：

* `total` - 匹配文档所花费的总时间
* `queries_matched` - 匹配文档的存储查询数量
* `document_matches` - 匹配表中存储查询的文档数量
* `total_queries_stored` - 表中存储的查询总数
* `term_only_queries` - 表中包含词项的查询数量；其余查询使用扩展查询语法。

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CALL PQ ('pq', ('{"title":"angry", "gid":3 }')); SHOW META;
```

<!-- response SQL -->

```sql
+------+
| id   |
+------+
|    2 |
+------+
1 row in set (0.00 sec)

+-----------------------+-----------+
| Variable name         | Value     |
+-----------------------+-----------+
| total                 | 0.000 sec |
| queries_matched       | 1         |
| queries_failed        | 0         |
| document_matched      | 1         |
| total_queries_stored  | 2         |
| term_only_queries     | 2         |
| fast_rejected_queries | 1         |
+-----------------------+-----------+
7 rows in set (0.00 sec)
```

<!-- end -->

<!-- example call pq verbose meta  -->

使用带有 `verbose` 选项的 `CALL PQ` 可提供更详细的输出。

它包括以下附加条目：

* `Setup` - 匹配过程初始设置所花费的时间，例如解析文档和设置选项
* `Queries failed` - 失败的查询数量
* `Fast rejected queries` - 未完全评估但通过过滤器或其他条件快速匹配并拒绝的查询数量
* `Time per query` - 每个查询的详细时间
* `Time of matched queries` - 匹配到任何文档的查询所花费的总时间


<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CALL PQ ('pq', ('{"title":"angry", "gid":3 }'), 1 as verbose); SHOW META;
```

<!-- response SQL -->

```sql
+------+
| id   |
+------+
|    2 |
+------+
1 row in set (0.00 sec)

+-------------------------+-----------+
| Variable name           | Value     |
+-------------------------+-----------+
| total                   | 0.000 sec |
| setup                   | 0.000 sec |
| queries_matched         | 1         |
| queries_failed          | 0         |
| document_matched        | 1         |
| total_queries_stored    | 2         |
| term_only_queries       | 2         |
| fast_rejected_queries   | 1         |
| time_per_query          | 69        |
| time_of_matched_queries | 69        |
+-------------------------+-----------+
10 rows in set (0.00 sec)
```

<!-- end -->

<!-- proofread -->

