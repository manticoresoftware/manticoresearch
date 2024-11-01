# SHOW META

```sql
SHOW META [ LIKE pattern ]
```

<!-- example show meta -->

`SHOW META` 是一个 SQL 语句，用于显示有关已处理查询的附加元信息，包括查询时间、关键词统计和使用的二级索引信息。其语法如下：

返回的项包括：

- `total`：实际检索到并发送给客户端的匹配结果数量。
- `total_found`：索引中查询的估计总匹配数。如果需要精确的匹配数量，建议使用 `SELECT COUNT(*)`。
- `total_relation`：如果 Manticore 无法精确计算 `total` 值，该字段将显示 `total_relation: gte`，表示实际数量 **大于或等于** `total_found`。如果 `total` 值是精确的，则显示 `total_relation: eq`。
- `time`：处理搜索查询所用的时间（以秒为单位）。
- `keyword[N]`：搜索查询中使用的第 N 个关键词。请注意，关键词可以是通配符形式，例如 `abc*`。
- `docs[N]`：包含搜索查询中第 N 个关键词的文档（或记录）的总数。如果关键词是通配符形式，此值表示所有扩展子关键词的文档总数，可能超过实际匹配文档数。
- `hits[N]`：第 N 个关键词在所有文档中的总出现次数（或命中次数）。
- `index`：有关所使用的索引的信息（例如二级索引）。

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
`SHOW META` 可以显示 I/O 和 CPU 计数器，但这些计数器仅在 `searchd` 使用 `--iostats` 和 `--cpustats` 参数启动时才可用。

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
Additional values, such as `predicted_time`, `dist_predicted_time`, `local_fetched_docs`, `local_fetched_hits`, `local_fetched_skips`, and their respective `dist_fetched_*` counterparts, will only be available if `searchd` was configured with [predicted time costs](../Server_settings/Searchd.md#predicted_time_costs) and the query included `predicted_time` in the `OPTION` clause.

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

额外的值，如 `predicted_time`、`dist_predicted_time`、`local_fetched_docs`、`local_fetched_hits`、`local_fetched_skips` 及其相应的 `dist_fetched_*` 值，只有在 `searchd` 配置了[预测时间成本](../Server_settings/Searchd.md#predicted_time_costs) 并且查询在 `OPTION` 子句中包含 `predicted_time` 时才会显示。

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

You can also use the optional LIKE clause, which allows you to select only the variables that match a specific pattern. The pattern syntax follows standard SQL wildcards, where `%` represents any number of any characters, and `_` represents a single character.

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

## SHOW META 与分面

<!-- example show meta facets -->

在使用[分面搜索](../Searching/Faceted_search.md)时，您可以通过查看 `SHOW META` 输出中的 `multiplier` 字段，确定在一个优化组中执行了多少查询。

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

当[基于成本的查询优化器](../Searching/Cost_based_optimizer.md) 选择使用 `DocidIndex`、`ColumnarScan` 或 `SecondaryIndex` 代替普通过滤器时，这会在 `SHOW META` 命令中反映出来。

`index` 变量显示了查询执行过程中使用的二级索引的名称和类型。百分比表示使用二级索引的磁盘块（对于 RT 表）或伪分片（对于普通表）的比例。

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

## SHOW META 针对 PQ 表

<!-- example show meta PQ -->

在执行 [CALL PQ](../Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ) 语句后，可以使用 `SHOW META` 来获取不同的输出。

`SHOW META` 在 `CALL PQ` 语句之后的输出包括：

- `Total` - 匹配文档所花费的总时间
- `Queries matched` - 匹配文档的已存储查询数量
- `Document matches` - 匹配表中存储查询的文档数量
- `Total queries stored` - 表中存储的查询总数
- `Term only queries` - 表中仅包含词项的查询数量；其余查询使用扩展查询语法。

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
| Name                  | Value     |
+-----------------------+-----------+
| Total                 | 0.000 sec |
| Queries matched       | 1         |
| Queries failed        | 0         |
| Document matched      | 1         |
| Total queries stored  | 2         |
| Term only queries     | 2         |
| Fast rejected queries | 1         |
+-----------------------+-----------+
7 rows in set (0.00 sec)
```

<!-- end -->

<!-- example call pq verbose meta  -->

使用 `CALL PQ` 配合 `verbose` 选项可以提供更详细的输出信息。

它包括以下附加条目：

- **Setup** - 用于匹配过程初始设置的时间，例如解析文档和设置选项的时间。
- **Queries failed** - 失败的查询数量。
- **Fast rejected queries** - 未完全评估但通过筛选器或其他条件快速匹配并被拒绝的查询数量。
- **Time per query** - 每个查询的详细时间。
- **Time of matched queries** - 花费在匹配到文档的查询上的总时间。

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
| Name                    | Value     |
+-------------------------+-----------+
| Total                   | 0.000 sec |
| Setup                   | 0.000 sec |
| Queries matched         | 1         |
| Queries failed          | 0         |
| Document matched        | 1         |
| Total queries stored    | 2         |
| Term only queries       | 2         |
| Fast rejected queries   | 1         |
| Time per query          | 69        |
| Time of matched queries | 69        |
+-------------------------+-----------+
10 rows in set (0.00 sec)
```

<!-- end -->

<!-- proofread -->
