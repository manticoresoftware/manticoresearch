# SHOW META

```sql
SHOW META [ LIKE pattern ]
```

<!-- example show meta -->
`SHOW META` 是一个 SQL 语句，用于显示有关处理查询的附加元信息，包括查询时间、关键词统计以及使用的辅助索引信息。语法如下：

包含的项目有：
* `total`：实际检索并发送给客户端的匹配数。该值通常受 [LIMIT/size](../Searching/Pagination.md#Pagination-of-search-results) 搜索选项的限制。
* `total_found`：
  - 索引中查询的匹配项估计总数。如果需要确切的匹配数量，请使用 `SELECT COUNT(*)`，而不要依赖此值。
  - 对于带有 `GROUP BY` 的查询，`total_found` 表示组的数量，而非单个匹配。
  - 当使用带有 `GROUP BY` 的 `HAVING` 时，`total_found` 反映应用 `HAVING` 过滤后的组数。这支持带有 `HAVING` 子句的正确分页。
  - 对于 [GROUP N BY](../Searching/Grouping.md#Give-me-N-rows) 查询，不论 `N` 的值，`total_found` 仍然表示组的数量。
* `total_relation`：指示 `total_found` 的值是准确的还是估计的。
  - 如果 Manticore 无法确定精确的 `total_found` 值，该字段将显示 `total_relation: gte`，意味着实际匹配数量 **大于或等于** 报告的 `total_found`。
  - 如果 `total_found` 是精确值，则显示 `total_relation: eq`。
* `time`：处理搜索查询所用的时间（以秒为单位）。
* `keyword[N]`：搜索查询中第 n 个使用的关键词。关键词可以是通配符，例如 `abc*`。
* `docs[N]`：包含搜索查询中第 n 个关键词的文档（或记录）总数。如果关键词是通配符，则该值表示所有展开子关键词文档数的总和，可能超过实际匹配的文档数。
* `hits[N]`：第 n 个关键词在所有文档中出现的总次数（或命中次数）。
* `index`：所用索引的信息（例如辅助索引）。

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
`SHOW META` 可以显示 I/O 和 CPU 计数器，但只有在 searchd 启动时使用了 `--iostats` 和 `--cpustats` 开关时，这些信息才可用。

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
额外的值如 `predicted_time`、`dist_predicted_time`、`local_fetched_docs`、`local_fetched_hits`、`local_fetched_skips` 以及其对应的 `dist_fetched_*` 等仅在 `searchd` 配置了 [预测时间成本](../Server_settings/Searchd.md#predicted_time_costs)，且查询的 `OPTION` 子句中包含 `predicted_time` 时才可用。

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

`SHOW META` 必须紧接查询之后在**同一**会话中执行。由于某些 MySQL 连接器/库使用连接池，单独执行 `SHOW META` 可能导致意外结果，如获取到其它查询的元数据。针对这种情况（且一般推荐），请使用包含查询和 `SHOW META` 的多语句查询。有些连接器/库支持在同一方法内多查询执行，另一些可能需要专用多查询方法或在连接设置时配置特殊选项。

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

也可以使用可选的 LIKE 子句，只选出匹配特定模式的变量。模式语法遵循标准 SQL 通配符，`%` 表示任意数量的任意字符，`_` 表示单个字符。

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

## SHOW META 与 facets

<!-- example show meta facets -->

使用 [分面搜索](../Searching/Faceted_search.md) 时，可在 `SHOW META` 输出中查看 `multiplier` 字段，了解执行了多少个优化分组的查询。

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

## SHOW META 与查询优化器

<!-- example of show meta vs query optimizer -->

当 [基于成本的查询优化器](../Searching/Cost_based_optimizer.md) 选择用 `DocidIndex`、`ColumnarScan` 或 `SecondaryIndex` 代替普通过滤时，`SHOW META` 命令会反映这一点。

`index` 变量显示执行查询时使用的辅助索引名称和类型。百分比表示在 RT 表中使用辅助索引的磁盘块数量或在普通表中使用的伪分片数量。

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

`SHOW META` 可在执行 [CALL PQ](../Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ) 语句后使用，此时会提供不同的输出。

`CALL PQ` 之后的 `SHOW META` 包含：

* `total` - 匹配文档所花费的总时间
* `queries_matched` - 匹配文档的存储查询数
* `document_matches` - 匹配表中存储查询的文档数
* `total_queries_stored` - 表中存储查询的总数
* `term_only_queries` - 表中含词项的查询数；其余查询使用扩展查询语法。

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
* `Time of matched queries` - 与任何文档匹配的查询所花费的总时间


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

