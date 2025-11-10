# 搜索选项

SQL 的 [SELECT](../Searching/Full_text_matching/Basic_usage.md#SQL) 子句和 HTTP 的 [/search](../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) 端点支持多种选项，可用于微调搜索行为。

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
整数。启用或禁用在多线程运行 groupby 查询时保证聚合准确性。默认值为 0。

运行 groupby 查询时，可以在带有多个伪分片的普通表上并行运行（如果启用了 `pseudo_sharding`）。类似的方法也适用于 RT 表。每个分片/块执行查询，但分组数量受 `max_matches` 限制。如果来自不同分片/块的结果集包含不同的分组，分组计数和聚合可能不准确。注意，Manticore 会尝试根据 groupby 属性的唯一值数量（从二级索引获取）将 `max_matches` 增加到 [`max_matches_increase_threshold`](../Searching/Options.md#max_matches_increase_threshold)。如果成功，则不会有准确性损失。

然而，如果 groupby 属性的唯一值数量很高，进一步增加 `max_matches` 可能不是好策略，因为这会导致性能下降和内存使用增加。将 `accurate_aggregation` 设置为 1 会强制 groupby 搜索在单线程中运行，从而解决准确性问题。注意，只有当 `max_matches` 无法设置得足够高时，才会强制单线程运行；否则，设置了 `accurate_aggregation=1` 的搜索仍会在多线程中运行。

总体来说，将 `accurate_aggregation` 设置为 1 可确保 RT 表和带有 `pseudo_sharding=1` 的普通表中的分组计数和聚合准确性。缺点是搜索速度会变慢，因为它们被强制在单线程中运行。

但是，如果我们有一个 RT 表和一个包含相同数据的普通表，并且运行带有 `accurate_aggregation=1` 的查询，仍可能得到不同的结果。这是因为守护进程可能会基于 [`max_matches_increase_threshold`](../Searching/Options.md#max_matches_increase_threshold) 设置为 RT 表和普通表选择不同的 `max_matches` 设置。

### agent_query_timeout
整数。等待远程查询完成的最大时间（毫秒），详见[此部分](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout)。

### boolean_simplify
`0` 或 `1`（默认值为 `1`）。`boolean_simplify=1` 启用[简化查询](../Searching/Full_text_matching/Boolean_optimization.md)以加快速度。

此选项也可以在 [searchd 配置](../Server_settings/Searchd.md#boolean_simplify)中全局设置，以更改所有查询的默认行为。每个查询的选项会覆盖全局设置。

### comment
字符串，用户注释，会被复制到查询日志文件中。

### cutoff
整数。指定要处理的最大匹配数。如果未设置，Manticore 会自动选择合适的值。

<!-- example cutoff_aggregation -->

* `N = 0`：禁用匹配数限制。
* `N > 0`：指示 Manticore 在找到 `N` 个匹配文档后停止处理结果。
* 未设置：Manticore 自动决定阈值。

当 Manticore 无法确定匹配文档的确切数量时，查询的[元信息](../Node_info_and_management/SHOW_META.md#SHOW-META)中的 `total_relation` 字段将显示 `gte`，表示**大于或等于**。这意味着实际匹配数至少是报告的 `total_found`（SQL）或 `hits.total`（JSON）。当计数准确时，`total_relation` 显示为 `eq`。

注意：不建议在聚合查询中使用 `cutoff`，因为它可能产生不准确或不完整的结果。

<!-- request Example -->

在聚合查询中使用 `cutoff` 可能导致错误或误导性的结果，如下例所示：
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
整数。默认值为 `3500`。此选项设置在普通表中 `count distinct` 返回的计数保证准确的阈值。

接受的值范围为 `500` 到 `15500`。超出此范围的值将被限制。

当此选项设置为 0 时，启用一种确保精确计数的算法。该算法收集 `{group, value}` 对，排序并定期消除重复项。结果是在普通表中获得精确计数。然而，由于其高内存消耗和查询执行缓慢，该方法不适合高基数数据集。

当 `distinct_precision_threshold` 设置为大于 0 的值时，Manticore 使用另一种算法。它将计数加载到哈希表中并返回表的大小。如果哈希表过大，其内容会被转移到 `HyperLogLog` 数据结构中。此时计数变为近似，因为 HyperLogLog 是一种概率算法。该方法保持每个分组的最大内存使用固定，但计数准确性有所折衷。

`HyperLogLog` 的准确性和从哈希表转换到 HyperLogLog 的阈值由 `distinct_precision_threshold` 设置决定。使用此选项时需谨慎，因为其值加倍会使计算计数所需的最大内存也加倍。最大内存使用量可大致估算为：`64 * max_matches * distinct_precision_threshold`，尽管实际计数计算通常使用的内存少于最坏情况。

### expand_keywords
`0` 或 `1`（默认 `0`）。在可能的情况下，扩展关键词为精确形式和/或通配符。详情请参阅 [expand_keywords](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expand_keywords)。

### field_weights
命名的整数列表（每字段用户权重，用于排名）。

示例：
```sql
SELECT ... OPTION field_weights=(title=10, body=3)
```

### global_idf
使用来自 [global_idf](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#global_idf) 文件的全局统计数据（频率）进行 IDF 计算。

### idf
带引号的、逗号分隔的 IDF 计算标志列表。已知标志有：

* `normalized`：BM25 变体，idf = log((N-n+1)/n)，依据 Robertson 等人的定义
* `plain`：普通变体，idf = log(N/n)，依据 Sparck-Jones 的定义
* `tfidf_normalized`：额外将 IDF 除以查询词数，使得 `TF*IDF` 适合于 [0, 1] 范围内
* `tfidf_unnormalized`：不额外将 IDF 除以查询词数，其中 N 是集合大小，n 是匹配文档数

Manticore 历史上的默认 IDF（逆文档频率）等同于 `OPTION idf='normalized,tfidf_normalized'`，这些归一化可能导致若干不期望的效果。

首先，`idf=normalized` 会导致关键词惩罚。例如，如果你搜索 `the | something`，且 `the` 出现在超过 50% 的文档中，那么同时包含关键词 `the` 和 `something` 的文档权重会低于仅包含关键词 `something` 的文档。使用 `OPTION idf=plain` 可以避免这种情况。普通 IDF 变化范围为 `[0, log(N)]`，关键词不会被惩罚；而归一化 IDF 变化范围为 `[-log(N), log(N)]`，频繁出现的关键词会被惩罚。

其次，`idf=tfidf_normalized` 会导致跨查询的 IDF 漂移。历史上，IDF 也被除以查询关键词数，确保所有关键词的 `sum(tf*idf)` 保持在 [0,1] 范围内。然而，这意味着像 `word1` 和 `word1 | nonmatchingword2` 这样的查询会对完全相同的结果集赋予不同权重，因为 `word1` 和 `nonmatchingword2` 的 IDF 都会被除以 2。使用 `OPTION idf='tfidf_unnormalized'` 可以解决此问题。请注意，当你禁用此归一化时，BM25、BM25A、BM25F() 排名因子将相应调整。

IDF 标志可以组合使用；`plain` 和 `normalized` 互斥；`tfidf_unnormalized` 和 `tfidf_normalized` 也互斥；未指定的互斥组标志默认为其原始设置。这意味着 `OPTION idf=plain` 等同于完整指定 `OPTION idf='plain,tfidf_normalized'`。

### jieba_mode
指定查询的 Jieba 分词模式。

使用 Jieba 中文分词时，有时对文档和查询使用不同的分词模式会有所帮助。完整模式列表请参阅 [jieba_mode](../Creating_a_table/NLP_and_tokenization/Morphology.md#jieba_mode)。

### index_weights
命名的整数列表。每表用户权重，用于排名。

### local_df
`0` 或 `1`，自动对分布式表的所有本地部分的 DF 求和，确保本地分片表中 IDF 的一致性（和准确性）。默认对 RT 表的磁盘分片启用。带通配符的查询词会被忽略。

### low_priority
`0` 或 `1`（默认 `0`）。设置 `low_priority=1` 会以较低优先级执行查询，其作业调度频率比正常优先级的查询低 10 倍。

### max_matches
整数。每查询最大匹配数值。

服务器为每个表保留在 RAM 中并返回给客户端的最大匹配数。默认值为 1000。

该设置用于控制和限制 RAM 使用，决定在搜索每个表时保留多少匹配。每个找到的匹配仍会被处理，但只保留最优的 N 个匹配在内存中，最终返回给客户端。例如，假设一个表中某查询有 2,000,000 个匹配。通常你不需要检索全部匹配，而是需要扫描所有匹配，但只选择“最佳”的 500 个（例如，基于相关性、价格或其他因素排序），并以每页 20 到 100 个匹配的形式展示给最终用户。仅跟踪最佳 500 个匹配比保留全部 2,000,000 个匹配、排序后再丢弃多余的匹配要高效得多。`max_matches` 控制这个“最佳 N”数量。

该参数显著影响每查询的 RAM 和 CPU 使用。一般接受的值为 1,000 到 10,000，但更高的限制应谨慎使用。随意将 max_matches 增加到 1,000,000 意味着 `searchd` 必须为每个查询分配并初始化一个包含一百万条目的匹配缓冲区。这必然会增加每查询的 RAM 使用，有时会明显影响性能。

更多关于 `max_matches` 行为影响的信息，请参阅 [max_matches_increase_threshold](../Searching/Options.md#max_matches_increase_threshold)。

### max_matches_increase_threshold

整数。设置 `max_matches` 可增加的阈值。默认值为 16384。

当启用 `pseudo_sharding` 并检测到 groupby 属性的唯一值数量少于此阈值时，Manticore 可能会增加 `max_matches` 以提高 groupby 和/或聚合的准确性。当伪分片在多个线程中执行查询，或 RT 表在磁盘分片中进行并行搜索时，可能会出现准确性损失。

如果 groupby 属性的唯一值数量少于阈值，`max_matches` 将被设置为该数量。否则，使用默认的 `max_matches`。

如果查询选项中显式设置了 `max_matches`，此阈值无效。

请注意，如果该阈值设置过高，会导致内存消耗增加和整体性能下降。

您还可以使用[accurate_aggregation](../Searching/Options.md#accurate_aggregation)选项强制执行保证的groupby/聚合准确模式。

### max_query_time
设置最大搜索查询时间，单位为毫秒。必须是非负整数。默认值为0，表示“不限制”。本地搜索查询将在指定时间到达后停止。请注意，如果您执行的是查询多个本地表的搜索，则此限制分别适用于每个表。请注意，由于不断跟踪是否该停止查询，这可能会略微增加查询的响应时间。

### max_predicted_time
整数。最大预测搜索时间；参见[predicted_time_costs](../Server_settings/Searchd.md#predicted_time_costs)。

### morphology
`none`允许将所有查询词替换为其精确形式，前提是表是在启用[index_exact_words](../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words)的情况下构建的。这对于防止查询词的词干提取或词形还原非常有用。

### not_terms_only_allowed
<!-- example not_terms_only_allowed -->
`0`或`1`允许查询中独立的[否定](../Searching/Full_text_matching/Operators.md#Negation-operator)。默认值为0。另请参见相应的[全局设置](../Server_settings/Searchd.md#not_terms_only_allowed)。

<!-- request SQL -->
```sql
MySQL [(none)]> select * from tbl where match('-donald');
ERROR 1064 (42000): index t: query error: query is non-computable (single NOT operator)
MySQL [(none)]> select * from t where match('-donald') option not_terms_only_allowed=1;
+---------------------+-----------+
| id                  | field     |
+---------------------+-----------+
| 1658178727135150081 | smth else |
+---------------------+-----------+
```
<!-- end -->

### ranker
可从以下选项中选择：
* `proximity_bm25`
* `bm25`
* `none`
* `wordcount`
* `proximity`
* `matchany`
* `fieldmask`
* `sph04`
* `expr`
* `export`

有关每个排序器的更多详细信息，请参阅[搜索结果排名](../Searching/Sorting_and_ranking.md#Available-built-in-rankers)。

### rand_seed
允许您为`ORDER BY RAND()`查询指定特定的整数种子值，例如：`... OPTION rand_seed=1234`。默认情况下，每个查询都会自动生成一个新的不同的种子值。

### retry_count
整数。分布式重试次数。

### retry_delay
整数。分布式重试延迟，单位为毫秒。

### scroll

字符串。用于使用[滚动分页方法](../Searching/Pagination.md#Scroll-Search-Option)进行结果分页的滚动令牌。

### sort_method
* `pq` - 优先队列，默认设置
* `kbuffer` - 为已预排序数据提供更快的排序，例如按id排序的表数据
两种情况下的结果集相同；选择其中一个选项可能仅仅是提高（或降低）性能。

### threads
限制当前查询处理使用的最大线程数。默认 - 无限制（查询可以占用全局定义的所有[线程](../Server_settings/Searchd.md#threads)）。
对于一批查询，该选项必须附加到批次中的第一个查询，然后在创建工作队列时应用，并对整个批次生效。此选项与选项[max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query)含义相同，但仅应用于当前查询或查询批次。

### token_filter
带引号的、用冒号分隔的字符串，格式为`library name:plugin name:optional string of settings`。每当涉及的每个表调用全文搜索时，都会为每次搜索创建一个查询时令牌过滤器，允许您实现根据自定义规则生成令牌的自定义分词器。
```sql
SELECT * FROM index WHERE MATCH ('yes@no') OPTION token_filter='mylib.so:blend:@'
```
### expansion_limit
限制单个通配符展开的最大关键字数，默认值为0表示无限制。更多详情请参阅[expansion_limit](../Server_settings/Searchd.md#expansion_limit)。

## 查询优化器提示

<!-- example options_force -->

在极少数情况下，Manticore内置的查询分析器可能无法正确理解查询并确定应使用docid索引、二级索引还是列扫描。要覆盖查询优化器的决策，您可以在查询中使用以下提示：

* `/*+ DocidIndex(id) */` 强制使用docid索引，`/*+ NO_DocidIndex(id) */` 告诉优化器忽略它
* `/*+ SecondaryIndex(<attr_name1>[, <attr_nameN>]) */` 强制使用二级索引（如果可用），`/*+ NO_SecondaryIndex(id) */` 告诉优化器忽略它
* `/*+ ColumnarScan(<attr_name1>[, <attr_nameN>]) */` 强制使用列扫描（如果属性是列式的），`/*+ NO_ColumnarScan(id) */` 告诉优化器忽略它

请注意，在执行带过滤器的全文查询时，查询优化器会决定是将全文树结果与过滤器结果相交，还是使用标准的先匹配后过滤方法。指定*任何*提示都会强制守护进程使用执行全文树结果与过滤器结果相交的代码路径。

有关查询优化器工作原理的更多信息，请参阅[基于成本的优化器](../Searching/Cost_based_optimizer.md)页面。

<!-- request SQL -->

```sql
SELECT * FROM students where age > 21 /*+ SecondaryIndex(age) */
```

<!-- end -->

<!-- example comments -->
使用MySQL/MariaDB客户端时，请确保包含`--comments`标志以启用查询中的提示。

<!-- request mysql -->
```bash
mysql -P9306 -h0 --comments
```
<!-- end -->

<!-- proofread -->

