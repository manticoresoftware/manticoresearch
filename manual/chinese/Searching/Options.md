# 搜索选项

SQL [SELECT](../Searching/Full_text_matching/Basic_usage.md#SQL) 子句和 HTTP [/search](../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) 端点支持多种选项，用于微调搜索行为。

## OPTION

### 通用语法

<!-- example options -->

**SQL**:

```sql
SELECT ... [OPTION <optionname>=<value> [ , ... ]] [/*+ [NO_][ColumnarScan|DocidIndex|SecondaryIndex(<attribute>[,...])]] /*]
```

**HTTP**:
```json
POST /search
{
    "table" : "table_name",
    "options":
    {
        "optionname": "value",
        "optionname2": <value2>
    }
}
```


<!-- intro -->
SQL:
<!-- request SQL -->
```sql
SELECT * FROM test WHERE MATCH('@title hello @body world')
OPTION ranker=bm25, max_matches=3000,
field_weights=(title=10, body=3), agent_query_timeout=10000
```

<!-- response SQL -->
```sql
+------+-------+-------+
| id   | title | body  |
+------+-------+-------+
|    1 | hello | world |
+------+-------+-------+
1 row in set (0.00 sec)
```

<!-- intro -->
JSON:
<!-- request JSON -->

```json
POST /search
{
    "table" : "test",
    "query": {
      "match": {
        "title": "hello"
      },
      "match": {
        "body": "world"
      }
    },
    "options":
    {
        "ranker": "bm25",
        "max_matches": 3000,
        "field_weights": {
            "title": 10,
            "body": 3
        },
        "agent_query_timeout": 10000
    }
}
```

<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "total_relation": "eq",
    "hits": [
      {
        "_id": 1,
        "_score": 10500,
        "_source": {
          "title": "hello",
          "body": "world"
        }
      }
    ]
  }
}
```

<!-- end -->

支持的选项有：

### accurate_aggregation
整数。在多线程运行 groupby 查询时启用或禁用保证聚合准确性。默认值为 0。

在运行 groupby 查询时，如果 `pseudo_sharding` 开启，则可以在带有多个伪分片的普通表上并行执行。RT 表也采用类似方法。每个分片/数据块执行查询，但分组数量受 `max_matches` 限制。如果不同分片/数据块的结果集包含不同分组，分组计数和聚合结果可能不准确。注意，Manticore 会尝试根据通过二级索引获得的 groupby 属性唯一值数量，将 `max_matches` 增加到不超过 [`max_matches_increase_threshold`](../Searching/Options.md#max_matches_increase_threshold) 的值。如果成功，则不会损失准确度。

然而，如果 groupby 属性的唯一值数量很高，继续增加 `max_matches` 可能不是好策略，因为这会导致性能下降和内存使用增加。设置 `accurate_aggregation` 为 1 会强制 groupby 搜索在单线程中运行，从而解决准确性问题。注意，单线程运行仅在 `max_matches` 无法设置得足够高时才强制生效；否则，带有 `accurate_aggregation=1` 的搜索仍将在多线程中运行。

总体来说，设置 `accurate_aggregation` 为 1 可确保 RT 表和设有 `pseudo_sharding=1` 的普通表中的分组计数和聚合准确性。缺点是搜索速度变慢，因为被迫在单线程中运行。

但是，如果我们有一张 RT 表和一张包含相同数据的普通表，并且运行带有 `accurate_aggregation=1` 的查询，仍然可能得到不同结果。这是因为守护进程可能基于 [`max_matches_increase_threshold`](../Searching/Options.md#max_matches_increase_threshold) 设置，为 RT 表和普通表选择不同的 `max_matches`。

### agent_query_timeout
整数。等待远程查询完成的最长时间（毫秒），详见[本节](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout)。

### boolean_simplify
`0` 或 `1`（默认 `1`）。`boolean_simplify=1` 启用[简化查询](../Searching/Full_text_matching/Boolean_optimization.md)以加速执行。

该选项也可在 [searchd 配置](../Server_settings/Searchd.md#boolean_simplify) 中全局设置，改变所有查询的默认行为。每次查询的选项会覆盖全局设置。

### comment
字符串，用户注释，会复制到查询日志文件。

### cutoff
整数。指定处理的最大匹配数。如未设置，Manticore 会自动选择合适值。

<!-- example cutoff_aggregation -->

* `N = 0`：禁用匹配数量限制。
* `N > 0`：指示 Manticore 一旦找到 `N` 个匹配文档即停止处理结果。
* 未设置：Manticore 自动决定阈值。

当 Manticore 无法确定匹配文档的精确计数时，查询[元信息](../Node_info_and_management/SHOW_META.md#SHOW-META)中的 `total_relation` 字段将显示 `gte`，表示**大于或等于**。这意味着实际匹配数至少为报告的 `total_found`（SQL）或 `hits.total`（JSON）。计数精确时，`total_relation` 显示 `eq`。

注意：不建议在聚合查询中使用 `cutoff`，因为可能产生不准确或不完整的结果。

<!-- request Example -->

在聚合查询中使用 `cutoff` 会导致错误或误导结果，如下例所示：
```
drop table if exists t
--------------

Query OK, 0 rows affected (0.02 sec)

--------------
create table t(a int)
--------------

Query OK, 0 rows affected (0.04 sec)

--------------
insert into t(a) values(1),(2),(3),(1),(2),(3)
--------------

Query OK, 6 rows affected (0.00 sec)

--------------
select avg(a) from t option cutoff=1 facet a
--------------

+----------+
| avg(a)   |
+----------+
| 1.000000 |
+----------+
1 row in set (0.00 sec)
--- 1 out of 1 results in 0ms ---

+------+----------+
| a    | count(*) |
+------+----------+
|    1 |        1 |
+------+----------+
1 row in set (0.00 sec)
--- 1 out of 1 results in 0ms ---
```

与不使用 `cutoff` 的相同查询对比：
```
--------------
select avg(a) from t facet a
--------------

+----------+
| avg(a)   |
+----------+
| 2.000000 |
+----------+
1 row in set (0.00 sec)
--- 1 out of 1 results in 0ms ---

+------+----------+
| a    | count(*) |
+------+----------+
|    1 |        2 |
|    2 |        2 |
|    3 |        2 |
+------+----------+
3 rows in set (0.00 sec)
--- 3 out of 3 results in 0ms ---
```

<!-- end -->

### distinct_precision_threshold
整数。默认值为 `3500`。该选项设置在普通表中 `count distinct` 返回计数能保证精确的阈值。

接受的值在 `500` 至 `15500` 范围内。超出该范围的值会被限制。

当此选项设为 0 时，启用一种算法以确保精确计数。该算法收集 `{group, value}` 对，排序，并周期性地消除重复项。结果是在普通表中实现精确计数。但该方法因内存消耗大且查询执行慢，不适用于高基数数据集。

当 `distinct_precision_threshold` 设为大于 0 的值时，Manticore 采用另一算法。它将计数加载到哈希表中，并返回表的大小。如果哈希表过大，其内容会移动到 `HyperLogLog` 数据结构中。此时计数变为近似，因为 HyperLogLog 是概率算法。该方法保持每组固定的最大内存使用，但计数准确性有所折衷。

`HyperLogLog` 的准确性和从哈希表转换到 HyperLogLog 的阈值由 `distinct_precision_threshold` 设置决定。请谨慎使用该选项，因为其值翻倍会使计算计数的最大内存需求也翻倍。最大内存使用可粗略估算为：`64 * max_matches * distinct_precision_threshold`，尽管实际计数时通常使用的内存低于最坏情况。

### expand_keywords












































































































































































