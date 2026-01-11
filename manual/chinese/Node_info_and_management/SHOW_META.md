# SHOW META

```sql
SHOW META [ LIKE pattern ]
```

<!-- example show meta -->
`SHOW META` 是一个 SQL 语句，用于显示关于处理查询的附加元信息，包括查询时间、关键词统计，以及所使用的二级索引的信息。语法为：

包含的项有：
* `total`：实际检索并发送给客户端的匹配数。此值通常受限于 [LIMIT/size](../Searching/Pagination.md#Pagination-of-search-results) 搜索选项。
* `total_found`：
  - 索引中查询的估计匹配总数。如果需要确切的匹配数，请使用 `SELECT COUNT(*)`，而不是依赖此值。
  - 对于带有 `GROUP BY` 的查询，`total_found` 表示分组数，而非单个匹配。
  - 使用 `HAVING` 与 `GROUP BY` 时，`total_found` 反映在应用 `HAVING` 过滤器后的分组数。这使得使用 `HAVING` 子句时能够正确分页。
  - 对于 [GROUP N BY](../Searching/Grouping.md#Give-me-N-rows) 查询，`total_found` 仍表示分组数，与 `N` 的值无关。
* `total_relation`：指示 `total_found` 值是确切的还是估计的。
  - 如果 Manticore 无法确定精确的 `total_found` 值，此字段将显示 `total_relation: gte`，表示实际匹配数 **大于等于** 报告的 `total_found`。
  - 如果 `total_found` 值是确切的，将显示 `total_relation: eq`。
* `time`：处理搜索查询所花费的时间（秒）。
* `keyword[N]`：搜索查询中使用的第 n 个关键词。注意关键词可以是通配符形式，例如 `abc*`。
* `docs[N]`：包含搜索查询中第 n 个关键词的文档（或记录）总数。如果关键词是通配符形式，此值表示所有扩展子关键词文档数的总和，可能超过实际匹配文档数。
* `hits[N]`：搜索查询中第 n 个关键词在所有文档中的总出现次数（或命中数）。
* `index`：使用的索引信息（例如二级索引）。

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
`SHOW META` 可以显示 I/O 和 CPU 计数器，但仅在 searchd 是用 `--iostats` 和 `--cpustats` 开关分别启动时可用。

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
额外值，如 `predicted_time`、`dist_predicted_time`、`local_fetched_docs`、`local_fetched_hits`、`local_fetched_skips` 及其对应的 `dist_fetched_*`，仅在 searchd 配置了[预测时间成本](../Server_settings/Searchd.md#predicted_time_costs)且查询在 `OPTION` 子句中包含 `predicted_time` 时可用。

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

`SHOW META` 必须在 **相同** 会话中紧接查询之后执行。由于部分 MySQL 连接器/库使用连接池，分开执行 `SHOW META` 可能导致意外结果，比如获取到别的查询的元数据。在这类情况下（且通常推荐），应执行包含查询和 `SHOW META` 的多语句。有些连接器/库支持在同一方法内多语句执行，而另一些可能需要专用方法或连接时设置特定选项来支持多语句。

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

你也可以使用可选的 LIKE 子句，允许你只选择匹配特定模式的变量。模式语法遵循标准 SQL 通配符规则，其中 `%` 表示任意数量的任意字符，`_` 表示单个字符。

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

使用[分面搜索](../Searching/Faceted_search.md)时，你可以检查 `SHOW META` 输出中的 `multiplier` 字段，以确定在优化分组中执行了多少查询。

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

当[基于成本的查询优化器](../Searching/Cost_based_optimizer.md)选择使用 `DocidIndex`、`ColumnarScan` 或 `SecondaryIndex` 替代简单过滤时，这可以从 `SHOW META` 命令中反映出来。

`index` 变量显示了查询执行期间使用的二级索引名称及类型。百分比表示有多少磁盘块（针对 RT 表）或伪分片（针对普通表）使用了该二级索引。

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

`SHOW META` 可用于执行 [CALL PQ](../Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ) 语句后，其输出内容不同。

`CALL PQ` 后的 `SHOW META` 包含：

* `total` - 匹配文档所花费的总时间
* `queries_matched` - 匹配文档的存储查询数
* `document_matches` - 匹配表中存储查询的文档数
* `total_queries_stored` - 表中存储的查询总数
* `term_only_queries` - 表中包含词项的查询数；其余查询使用扩展查询语法。

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

使用 `CALL PQ` 带有 `verbose` 选项可以提供更详细的输出。

它包括以下额外条目：

* `Setup` - 初始匹配过程的设置时间，例如解析文档和设置选项
* `Queries failed` - 失败的查询数量
* `Fast rejected queries` - 未完全评估但快速匹配并被过滤或其他条件拒绝的查询数量
* `Time per query` - 每个查询的详细时间
* `Time of matched queries` - 匹配任何文档的查询所花费的总时间


<!-- intro -->
##### SQL：
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

