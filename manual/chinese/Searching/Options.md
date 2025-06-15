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

支持的选项包括：

### accurate_aggregation
整数。启用或禁用在多线程执行 groupby 查询时保证聚合准确性。默认值为 0。

执行 groupby 查询时，可以在带有多个伪分片的普通表上并行运行（如果启用了 `pseudo_sharding`）。类似的方法也适用于 RT 表。每个分片/块执行查询，但分组数量受限于 `max_matches`。如果来自不同分片/块的结果集包含不同的分组，分组合计和聚合可能不准确。请注意，Manticore 尝试根据分组属性的唯一值数量（从二级索引获取）将 `max_matches` 增加到 [`max_matches_increase_threshold`](../Searching/Options.md#max_matches_increase_threshold) 所设置的阈值。如果成功，则不会丢失准确性。

然而，如果分组属性的唯一值数量很高，进一步增加 `max_matches` 可能不是一个好策略，因为这会导致性能下降和更高的内存使用。将 `accurate_aggregation` 设置为 1 会强制 groupby 查询在单线程下运行，从而解决准确性问题。注意，单线程强制仅在无法将 `max_matches` 设置得足够高时生效；否则，设置 `accurate_aggregation=1` 的搜索仍将使用多线程执行。

总体来说，将 `accurate_aggregation` 设置为 1 可以确保 RT 表和开启 `pseudo_sharding=1` 的普通表中的分组计数和聚合准确性。缺点是搜索速度会变慢，因为必须强制单线程运行。

不过，如果我们有一个 RT 表和一个包含相同数据的普通表，并且使用 `accurate_aggregation=1` 运行查询，仍然可能得到不同的结果。这是因为 daemon 可能会基于 [`max_matches_increase_threshold`](../Searching/Options.md#max_matches_increase_threshold) 设置，为 RT 表和普通表选择不同的 `max_matches` 参数。

### agent_query_timeout
整数。远程查询的最大等待时间，单位为毫秒，详情见 [此处](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout)。

### boolean_simplify
`0` 或 `1`（默认值为 `1`）。`boolean_simplify=1` 启用[简化查询](../Searching/Full_text_matching/Boolean_optimization.md)以加快速度。

### comment
字符串，用户注释，会被写入查询日志文件。

### cutoff
整数。指定处理的最大匹配数。如果未设置，Manticore 将自动选择适当的值。

<!-- example cutoff_aggregation -->

* `N = 0`：禁用匹配数量限制。
* `N > 0`：指示 Manticore 一旦找到 `N` 个匹配文档即停止处理结果。
* 未设置：Manticore 自动决定阈值。

当 Manticore 无法确定匹配文档的准确数量时，查询[元信息](../Node_info_and_management/SHOW_META.md#SHOW-META)中的 `total_relation` 字段会显示 `gte`，表示**大于或等于**。这意味着实际匹配数量至少是报告的 `total_found`（SQL）或 `hits.total`（JSON）。当数量精确时，`total_relation` 显示为 `eq`。

注意：不建议在聚合查询中使用 `cutoff`，因为可能产生不准确或不完整的结果。

<!-- request Example -->

在聚合查询中使用 `cutoff` 可能导致结果不正确或误导，示例如下：
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

与未使用 `cutoff` 的相同查询对比：
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
整数。默认值为 `3500`。该选项设置了在普通表中保证 `count distinct` 返回结果为精确计数的阈值。

接受的数值范围是 `500` 到 `15500`，超出范围的数值会被限制在该区间内。

当该选项设置为 0 时，启用一种算法保证计数精确。该算法收集 `{group, value}` 对，进行排序，并定期消除重复，确保在普通表中的精确计数。不过该方法不适合高基数数据集，因为其内存消耗大且查询较慢。

当 `distinct_precision_threshold` 设置为大于 0 的值时，Manticore 采用不同的算法。它将计数加载到哈希表中并返回其大小。如果哈希表过大，则内容会转移到 `HyperLogLog` 数据结构，此时计数变为近似值，因为 HyperLogLog 是概率算法。该方法保证每组最大内存使用固定，但计数精度有所权衡。

`HyperLogLog` 的准确性及从哈希表转为 HyperLogLog 的阈值基于 `distinct_precision_threshold` 设置。使用此选项时需谨慎，因为将该值加倍也会使最大内存需求加倍。最大内存使用量可粗略估计为：`64 * max_matches * distinct_precision_threshold`，但实际计数通常用的内存比最糟糕情况下少。

### expand_keywords
`0` 或 `1`（默认 `0`）。尽可能使用精确形式和/或星号扩展关键字。更多详情请参见 [expand_keywords](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expand_keywords)。

### field_weights
命名整数列表（用于排名的每字段用户权重）。

示例：
```sql
SELECT ... OPTION field_weights=(title=10, body=3)
```

### global_idf
使用来自 [global_idf](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#global_idf) 文件的全局统计（频率）进行 IDF 计算。

### idf
带引号的、逗号分隔的 IDF 计算标志列表。已知的标志包括：

* `normalized`：BM25 变体，idf = log((N-n+1)/n)，依据 Robertson 等人的公式
* `plain`：普通变体，idf = log(N/n)，依据 Sparck-Jones 的定义
* `tfidf_normalized`：额外除以查询词数，使得 `TF*IDF` 适合于 [0, 1] 范围
* `tfidf_unnormalized`：不额外除以查询词数，其中 N 是集合大小，n 是匹配文档数量

Manticore 中历史默认的 IDF（逆文档频率）等价于 `OPTION idf='normalized,tfidf_normalized'`，而这些归一化可能导致一些不希望出现的效果。

首先，`idf=normalized` 会导致关键词惩罚。例如，如果你搜索 `the | something`，且 “the” 出现在超过 50% 的文档中，那么同时包含 “the” 和 “something” 两个关键词的文档权重将低于仅包含 “something” 的文档。使用 `OPTION idf=plain` 可以避免这种情况。普通 IDF 变化范围在 `[0, log(N)]`，关键词不会被惩罚；而归一化 IDF 变化范围在 `[-log(N), log(N)]`，频繁的关键词会被惩罚。

其次，`idf=tfidf_normalized` 会导致 IDF 在查询间漂移。历史上，IDF 也除以了查询关键词数量，确保所有关键词的 `sum(tf*idf)` 保持在 [0,1] 之间。但这意味着例如 `word1` 查询和 `word1 | nonmatchingword2` 查询会对完全相同的结果集赋予不同权重，因为 `word1` 和 `nonmatchingword2` 的 IDF 都会除以 2。使用 `OPTION idf='tfidf_unnormalized'` 可以解决此问题。需注意，禁用此归一化时，BM25、BM25A、BM25F() 排名因子也会相应调整。

IDF 标志可以组合；`plain` 和 `normalized` 互斥；`tfidf_unnormalized` 和 `tfidf_normalized` 也互斥；未指明的互斥组中的标志默认使用其原始设置。这意味着 `OPTION idf=plain` 与完整指定 `OPTION idf='plain,tfidf_normalized'` 是相同的。

### jieba_mode
指定查询的 Jieba 分词模式。

使用 Jieba 中文分词时，文档和查询的分词模式有时可使用不同模式。完整模式列表请参见 [jieba_mode](../Creating_a_table/NLP_and_tokenization/Morphology.md#jieba_mode)。

### index_weights
命名整数列表。每表用户排名权重。

### local_df
`0` 或 `1`，自动对分布式表的所有本地部分的 DF 进行求和，确保局部分片表的一致（准确）IDF。默认对 RT 表的磁盘分片启用。带通配符的查询词被忽略。

### low_priority
`0` 或 `1`（默认 `0`）。设置 `low_priority=1` 会以较低优先级执行查询，其任务重新调度频率是普通优先级查询的 1/10。

### max_matches
整数。单查询最大匹配数值。

服务器在内存中保留的每个表的最大匹配数，可返回给客户端。默认值为 1000。

为了控制和限制内存使用，`max_matches` 设置决定在搜索每个表时会保留多少匹配。所有找到的匹配仍然会处理，但仅保留排名前 N 的匹配在内存中，并最终返回给客户端。例如，假设某个表查询有 2,000,000 个匹配。通常无需检索所有匹配，而是扫描全部匹配但只挑选排名前 500 个（例如按相关性、价格或其他因素排序），并将这 500 个匹配以 20 到 100 个每页的方式展示给用户。仅跟踪排名前 500 个匹配比保留全部 2,000,000 个匹配更节省内存和 CPU，且避免了对所有匹配排序和丢弃操作。`max_matches` 就是控制这 “前 N” 的参数。

该参数极大影响单查询的内存和 CPU 使用。一般 1,000 至 10,000 之间是可接受的，但应谨慎使用更高的值。糟糕地将 max_matches 增加到 1,000,000 意味着每条查询时，`searchd` 必须为匹配缓冲区分配和初始化一百万条目，这必然增加单查询的内存占用，且在某些情况下会显著影响性能。

更多信息请参见 [max_matches_increase_threshold](../Searching/Options.md#max_matches_increase_threshold) 如何影响 `max_matches` 选项的行为。

### max_matches_increase_threshold

整数。设置 `max_matches` 可提升的阈值，默认值为 16384。

当启用 `pseudo_sharding` 时，若检测到 groupby 属性的唯一值数量少于此阈值，Manticore 可能会提升 `max_matches` 以提高 groupby 和/或聚合的准确性。伪分片在多线程执行查询或 RT 表在磁盘分片并行搜索时，可能出现准确性损失。

若 groupby 属性的唯一值数量少于此阈值，`max_matches` 将被设置为该数量。否则，使用默认的 `max_matches`。

若在查询选项中显式设置了 `max_matches`，该阈值不起作用。

请注意，若将此阈值设置得过高，会导致内存消耗增加和整体性能下降。

你还可以使用 [accurate_aggregation](../Searching/Options.md#accurate_aggregation) 选项强制保证 groupby/聚合的准确模式。

### max_query_time
设置最大搜索查询时间（毫秒）。必须是非负整数。默认值为0，表示“不限制”。本地搜索查询将在指定时间到达后停止。请注意，如果执行的是查询多个本地表的搜索，则该限制适用于每个表。请注意，由于持续跟踪是否应停止查询所产生的开销，此操作可能会略微增加查询的响应时间。

### max_predicted_time
整数。最大预测搜索时间；参见 [predicted_time_costs](../Server_settings/Searchd.md#predicted_time_costs)。

### morphology
`none` 允许用精确形式替换所有查询词（如果表是在启用了 [index_exact_words](../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words) 的情况下构建的）。这对于防止词干提取或词形还原查询词很有用。

### not_terms_only_allowed
<!-- example not_terms_only_allowed -->
`0` 或 `1` 允许单独的 [否定](../Searching/Full_text_matching/Operators.md#Negation-operator) 查询。默认是0。另请参阅对应的 [全局设置](../Server_settings/Searchd.md#not_terms_only_allowed)。

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

有关每个排序器的更多详细信息，请参阅 [搜索结果排序](../Searching/Sorting_and_ranking.md#Available-built-in-rankers)。

### rand_seed
允许为 `ORDER BY RAND()` 查询指定一个特定的整数种子值，例如：`... OPTION rand_seed=1234`。默认情况下，每个查询都会自动生成一个新的不同的种子值。

### retry_count
整数。分布式重试次数。

### retry_delay
整数。分布式重试延迟，单位毫秒。

### scroll

字符串。用于使用 [滚动分页方法](../Searching/Pagination.md#Scroll-Search-Option) 分页结果的滚动令牌。

### sort_method
* `pq` - 优先队列，默认设置
* `kbuffer` - 为已经预排序的数据（例如，按id排序的表数据）提供更快的排序
两种情况下结果集相同；选择任一选项仅可能改善（或恶化）性能。

### threads
限制当前查询处理使用的最大线程数。默认 - 无限制（查询可以使用全局定义的所有 [线程](../Server_settings/Searchd.md#threads)）。
对于一批查询，该选项必须附加在该批次的第一个查询上，随后在工作队列创建时生效并且对整个批次有效。该选项与 [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query) 选项意义相同，但仅应用于当前查询或查询批次。

### token_filter
带引号的、以冒号分隔的字符串，格式为 `库名:插件名:可选设置字符串`。为每个执行全文检索的表创建查询时的令牌过滤器，允许实现根据自定义规则生成令牌的自定义分词器。
```sql
SELECT * FROM index WHERE MATCH ('yes@no') OPTION token_filter='mylib.so:blend:@'
```
### expansion_limit
限制单个通配符扩展的最大关键字数，默认值为0，表示无限制。更多细节请参见 [expansion_limit](../Server_settings/Searchd.md#expansion_limit)。

## 查询优化器提示

<!-- example options_force -->

在极少数情况下，Manticore内置的查询分析器可能无法正确理解查询以及决定是否应使用docid索引、二级索引或列扫描。为了覆盖查询优化器的决定，您可以在查询中使用以下提示：

* `/*+ DocidIndex(id) */`  强制使用docid索引， `/*+ NO_DocidIndex(id) */` 告知优化器忽略它
* `/*+ SecondaryIndex(<attr_name1>[, <attr_nameN>]) */`  强制使用二级索引（如果可用）， `/*+ NO_SecondaryIndex(id) */` 告知优化器忽略它
* `/*+ ColumnarScan(<attr_name1>[, <attr_nameN>]) */`  强制使用列扫描（如果属性是列式的）， `/*+ NO_ColumnarScan(id) */` 告知优化器忽略它

请注意，在执行带过滤器的全文查询时，查询优化器会决定是将全文树结果与过滤器结果相交，还是使用标准的先匹配再过滤方法。指定*任何*提示将强制守护进程使用执行全文树结果与过滤器结果相交的代码路径。

有关查询优化器工作原理的更多信息，请参阅 [基于成本的优化器](../Searching/Cost_based_optimizer.md) 页面。

<!-- request SQL -->

```sql
SELECT * FROM students where age > 21 /*+ SecondaryIndex(age) */
```

<!-- end -->

<!-- example comments -->
使用 MySQL/MariaDB 客户端时，请确保包含 `--comments` 标志以启用查询中的提示。

<!-- request mysql -->
```bash
mysql -P9306 -h0 --comments
```
<!-- end -->

<!-- proofread -->

