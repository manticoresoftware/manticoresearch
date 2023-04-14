# SHOW META

```sql
SHOW META [ LIKE pattern ]
```

<!-- example show meta -->
`SHOW META` is an SQL statement that displays extra meta-information about the processed query, including query time and keyword statistics. The syntax is:

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SELECT id,story_author FROM hn_small WHERE MATCH('one|two|three') limit 5;
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

+----------------+--------+
| Variable_name  | Value  |
+----------------+--------+
| total          | 5      |
| total_found    | 266385 |
| total_relation | eq     |
| time           | 0.012  |
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

<!-- example show meta iostats cpustats -->
`SHOW META` can display I/O and CPU counters, but they will only be available if searchd was started with the `--iostats` and `--cpustats` switches, respectively.

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

`SHOW META` must be executed immediately after the query in the **same** session. Since some MySQL connectors/libraries use connection pools, running `SHOW META` in a separate statement can lead to unexpected results, such as retrieving metadata from another query. In these cases (and generally recommended), run a multiple statement containing both the query and `SHOW META`. Some connectors/libraries support multi-queries within the same method for a single statement, while others may require the use of a dedicated method for multi-queries or setting specific options during connection setup.

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

## SHOW META and facets

<!-- example show meta facets -->

When utilizing [faceted search](../Searching/Faceted_search.md), you can examine the `multiplier` field in the `SHOW META` output to determine how many queries were executed in an optimized group.

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

## SHOW META and query optimizer

<!-- example of show meta vs query optimizer -->

When the [cost-based query optimizer](../Searching/Cost_based_optimizer.md) chooses to use `DocidIndex`, `ColumnarScan`, or `SecondaryIndex` instead of a plain filter, this is reflected in the `SHOW META` command.

The `index` variable displays the names and types of secondary indexes used during query execution. The percentage indicates how many disk chunks (in the case of an RT index) or pseudo shards (in the case of a plain index) utilized the secondary index.

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

## SHOW META for PQ tables

<!-- example show meta PQ -->

`SHOW META` can be used after executing a [CALL PQ](../Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ) statement, in which case it provides different output.

`SHOW META` following a `CALL PQ` statement includes:

* `Total` - Total time spent on matching the document(s)
* `Queries matched` - Number of stored queries that match the document(s)
* `Document matches` - Number of documents that matched the queries stored in the table
* `Total queries stored` - Total number of queries stored in the table
* `Term only queries` - Number of queries in the table that have terms; the remaining queries use extended query syntax.

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

Using `CALL PQ` with a `verbose` option provides more detailed output.

It includes the following additional entries:

* `Setup` - Time spent on the initial setup of the matching process, such as parsing docs and setting options
* `Queries failed` - Number of queries that failed
* `Fast rejected queries` - Number of queries that were not fully evaluated but quickly matched and rejected using filters or other conditions
* `Time per query` - Detailed time for each query
* `Time of matched queries` - Total time spent on queries that matched any documents


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