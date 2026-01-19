# 搜索选项

SQL [SELECT](../Searching/Full_text_matching/Basic_usage.md#SQL) 子句和HTTP [/search](../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) 端点支持一些选项，可以用来精细调整搜索行为。

## 选项

### 基本语法

<!-- example options -->

**SQL**：

```sql
SELECT ... [OPTION <optionname>=<value> [ , ... ]] [/*+ [NO_][ColumnarScan|DocidIndex|SecondaryIndex(<attribute>[,...])]] /*]
```

**HTTP**：
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
SQL：
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
JSON：
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

### 准确聚合
整数。在多线程运行分组查询时启用或禁用保证聚合准确性的功能。默认值为 0。

在运行分组查询时，可以在没有伪分片的情况下将其并行运行到一个普通表上的几个伪分片上（如果启用了 `pseudo_sharding`）。类似的方法也适用于 RT 表。每个分片/块执行查询，但分组的数量由 `max_matches` 限制。如果来自不同分片/块的结果集有不同的分组，则分组计数和聚合可能不准确。请注意，Manticore 尝试根据分组属性的唯一值数量（从次要索引检索）增加 `max_matches` 至 [`max_matches_increase_threshold`](../Searching/Options.md#max_matches_increase_threshold)。如果成功，将不会丢失准确性。

然而，如果分组属性的唯一值数量很高，进一步增加 `max_matches` 可能不是一个好策略，因为它可能导致性能下降和更高的内存使用。将 `accurate_aggregation` 设置为 1 强制分组搜索在一个线程中运行，从而解决了准确性问题。请注意，当 `max_matches` 无法设置足够高时，才会强制执行单线程操作；否则，`accurate_aggregation=1` 的搜索仍然会在多个线程中运行。

总体而言，将 `accurate_aggregation` 设置为 1 确保了 RT 表和 `pseudo_sharding=1` 的普通表中的分组计数和聚合准确性。缺点是搜索会变慢，因为它们将被强制在一个线程中运行。

然而，如果我们有一个 RT 表和包含相同数据的普通表，并且运行带有 `accurate_aggregation=1` 的查询，我们可能会收到不同的结果。这是因为守护进程可能会由于 [`max_matches_increase_threshold`](../Searching/Options.md#max_matches_increase_threshold) 设置的不同而为 RT 和普通表选择不同的 `max_matches` 设置。

### agent_query_timeout
整数。等待远程查询完成的最大毫秒数，参见 [此部分](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout)。

### boolean_simplify
`0` 或 `1`（默认为 `1`）。`boolean_simplify=1` 启用 [简化查询](../Searching/Full_text_matching/Boolean_optimization.md) 以加快查询速度。

此选项也可以全局设置在 [searchd 配置](../Server_settings/Searchd.md#boolean_simplify) 中，以更改所有查询的默认行为。查询级别的选项会覆盖全局设置。

### comment
字符串，用户评论会被复制到查询日志文件中。

### cutoff
整数。指定要处理的最大匹配项数。未设置时，Manticore 会自动选择适当的值。

<!-- example cutoff_aggregation -->

* `N = 0`：禁用匹配项数的限制。
* `N > 0`：指示 Manticore 在找到 `N` 个匹配文档后停止处理结果。
* 未设置：Manticore 会自动决定阈值。

当 Manticore 无法确定匹配文档的确切数量时，查询 [元信息](../Node_info_and_management/SHOW_META.md#SHOW-META) 中的 `total_relation` 字段将显示 `gte`，即 **大于或等于**。这表示实际的匹配数量至少为报告的 `total_found`（在 SQL 中）或 `hits.total`（在 JSON 中）。当数量确切时，`total_relation` 将显示 `eq`。

注意：在聚合查询中使用 `cutoff` 不推荐，因为它可能导致不准确或不完整的结果。

<!-- request Example -->

在聚合查询中使用 `cutoff` 可能会导致不准确或误导性的结果，如下例所示：
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

与没有 `cutoff` 的相同查询进行比较：
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
整数。默认值为 `3500`。此选项设置了 `count distinct` 返回精确计数的阈值，在普通表中。

接受的值范围从 `500` 到 `15500`。超出此范围的值将被钳位。

当此选项设置为 `0` 时，启用确保精确计数的算法。该算法收集 `{group, value}` 对，对其进行排序，并定期消除重复项。结果是在普通表中具有精确计数。但是，这种方法不适合高基数的数据集，因为它消耗大量内存并且查询执行缓慢。

当 `distinct_precision_threshold` 设置为大于 `0` 的值时，Manticore 使用不同的算法。它将计数加载到哈希表中并返回表的大小。如果哈希表变得太大，其内容将移动到 `HyperLogLog` 数据结构。此时，计数变得近似，因为 `HyperLogLog` 是一种概率算法。这种方法保持每个组的最大固定内存使用量，但计数准确性有所权衡。

`HyperLogLog` 的准确性以及从哈希表转换为 `HyperLogLog` 的阈值是从 `distinct_precision_threshold` 设置推导出来的。重要的是谨慎使用此选项，因为将其值加倍也会加倍计算计数所需的最大内存。最大内存使用量可以通过以下公式粗略估计：`64 * max_matches * distinct_precision_threshold`，尽管实际上，计数计算通常使用的内存比最坏情况下的更少。

### expand_keywords
`0` 或 `1`（默认值为 `0`）。在可能的情况下扩展关键词为精确形式和/或通配符。详情请参见 [expand_keywords](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expand_keywords)。

### field_weights
命名的整数列表（按字段的用户权重，用于排名）。

示例：
```sql
SELECT ... OPTION field_weights=(title=10, body=3)
```

### global_idf
使用 [global_idf](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#global_idf) 文件中的全局统计数据（频率）进行 IDF 计算。

### idf
带引号的、用逗号分隔的 IDF 计算标志列表。已知标志包括：

* `normalized`：BM25 变体，idf = log((N-n+1)/n)，根据 Robertson 等人
* `plain`：纯变体，idf = log(N/n)，根据 Sparck-Jones
* `tfidf_normalized`：额外按查询词数除以 IDF，使 `TF*IDF` 适合于 [0, 1] 范围内
* `tfidf_unnormalized`：不额外按查询词数除以 IDF，其中 N 是集合大小，n 是匹配文档数

Manticore 历史默认的逆文档频率（IDF）等价于 `OPTION idf='normalized,tfidf_normalized'`，这些归一化可能导致若干不期望的效果。

首先，`idf=normalized` 会导致关键词惩罚。例如，如果搜索 `the | something` 且 `the` 出现在超过 50% 的文档中，则同时含有 `the` 和 `something` 两个关键词的文档权重会低于只含有一个关键词 `something` 的文档。使用 `OPTION idf=plain` 可避免此问题。纯 IDF 变动范围为 `[0, log(N)]`，关键词不会被惩罚；而归一化的 IDF 变动范围为 `[-log(N), log(N)]`，过于频繁的关键词会被惩罚。

其次，`idf=tfidf_normalized` 会导致 IDF 在查询间漂移。历史上，IDF 还会被查询关键词数量除以，确保所有关键词的 `sum(tf*idf)` 保持在 [0,1] 范围内。然而，这意味着像 `word1` 和 `word1 | nonmatchingword2` 这样的查询会给同一结果集分配不同权重，因为 `word1` 和 `nonmatchingword2` 的 IDF 都被除以了 2。使用 `OPTION idf='tfidf_unnormalized'` 可解决此问题。请记住，当您禁用此归一化时，BM25、BM25A、BM25F() 排名因子将相应调整。

IDF 标志可以组合；`plain` 和 `normalized` 互斥；`tfidf_unnormalized` 和 `tfidf_normalized` 也互斥；在此类互斥组中未指定的标志默认为其原始设置。这意味着 `OPTION idf=plain` 等同于完整指定 `OPTION idf='plain,tfidf_normalized'`。

### jieba_mode
指定查询的 Jieba 分词模式。

使用 Jieba 中文分词时，文档和查询的分词模式可能不同，从而带来帮助。完整模式列表请参见 [jieba_mode](../Creating_a_table/NLP_and_tokenization/Morphology.md#jieba_mode)。

### index_weights
命名的整数列表。按表的用户权重，用于排名。

### local_df
`0` 或 `1`，自动对分布式表的所有本地部分汇总文档频率，确保局部分片表中的 IDF 一致且准确。默认在 RT 表的磁盘区块启用。带通配符的查询词被忽略。

### low_priority
`0` 或 `1`（默认值为 `0`）。设置 `low_priority=1` 以较低优先级执行查询，其作业调度频率比正常优先级查询低 10 倍。

### max_matches
整数。每个查询的最大匹配数值。

服务器为每个表保留在 RAM 中的最大匹配数，并可返回给客户端。默认值为 1000。

此设置用于控制和限制内存使用量。`max_matches` 确定在搜索每个表时保留在内存中的匹配数量。每个匹配都会被处理，但只有最优的 N 个匹配会被保存在内存中，并最终返回给客户端。例如，假设某表针对一个查询有 2,000,000 个匹配。您很少需要检索全部匹配。相反，您需要扫描所有匹配，但只选择“最优”的 500 个（基于某些标准，例如相关性、价格或其他因素），并分页显示这 500 个匹配，单页 20 到 100 条。仅跟踪最优的 500 个匹配比保留全部 2,000,000 个匹配且排序，然后丢弃除前20个结果外的结果要高效得多。`max_matches` 控制这个“最优 N 数量”。

此参数显著影响每个查询的内存和 CPU 使用。通常 1,000 到 10,000 的值是可接受的，但过高值需要谨慎使用。任意增加 max_matches 到 1,000,000 意味着 `searchd` 会为每个查询分配和初始化一个包含一百万条目的匹配缓冲区，这必然增加每查询的内存使用，有时会明显影响性能。

有关其如何影响 `max_matches` 选项行为的附加信息，请参见 [max_matches_increase_threshold](../Searching/Options.md#max_matches_increase_threshold)。

### max_matches_increase_threshold

整数。设置可增加的 `max_matches` 阈值。默认值是 16384。

当启用 `pseudo_sharding` 并且检测到 groupby 属性唯一值数量小于此阈值时，Manticore 可能会增大 `max_matches` 以提高 groupby 和/或聚合的准确性。精度损失可能出现在 pseudo-sharding 在多线程中执行查询，或 RT 表对磁盘区块进行并行搜索时。

如果 groupby 属性的唯一值数量小于此阈值，则 `max_matches` 会被设置为该数量。否则，使用默认的 `max_matches`。

如果查询选项中显式设置了 `max_matches`，此阈值无效。

请注意，阈值设置过高会导致内存使用增加和整体性能下降。

您还可以使用 [accurate_aggregation](../Searching/Options.md#accurate_aggregation) 选项强制执行保障的 groupby/聚合准确模式。

### max_query_time
设置最大搜索查询时间，单位为毫秒。必须是非负整数。默认值为 0，表示“不限制”。本地搜索查询将在指定时间结束后停止。请注意，如果您执行的是查询多个本地表的搜索，则此限制分别适用于每个表。需注意，由于不断检查是否应停止查询的开销，这可能会稍微增加查询的响应时间。

### morphology
`none` 允许用它们的精确形式替换所有查询词，如果表格是使用启用的 [index_exact_words](../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words) 构建的。这对于防止查询词被词干化或词形还原非常有用。

### not_terms_only_allowed
<!-- example not_terms_only_allowed -->
`0` 或 `1` 允许查询中使用独立的[否定](../Searching/Full_text_matching/Operators.md#Negation-operator)操作符。默认值是 0。另请参见相应的[全局设置](../Server_settings/Searchd.md#not_terms_only_allowed)。

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

有关每个排序器的更多详细信息，请参阅 [搜索结果排名](../Searching/Sorting_and_ranking.md#Available-built-in-rankers)。

### rand_seed
允许您为 `ORDER BY RAND()` 查询指定特定的整数种子值，例如：`... OPTION rand_seed=1234`。默认情况下，每个查询都会自动生成一个新的不同的种子值。

### retry_count
整数。分布式重试次数。

### retry_delay
整数。分布式重试延迟，单位为毫秒。

### scroll

字符串。使用[滚动分页方法](../Searching/Pagination.md#Scroll-Search-Option)进行结果分页的滚动令牌。

### sort_method
* `pq` - 优先队列，默认设置
* `kbuffer` - 对已预排序的数据提供更快的排序，例如按 id 排序的表数据
两种情况下的结果集是相同的；选择其中一个选项可能仅仅是改善（或恶化）性能。

### threads
限制当前查询处理使用的最大线程数。默认 - 无限制（查询可以使用所有全局定义的 [threads](../Server_settings/Searchd.md#threads)）。
对于一批查询，该选项必须附加于该批中的第一个查询，然后在创建工作队列时应用，并对整个批次生效。此选项与 [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query) 具有相同含义，但仅应用于当前查询或查询批次。

### token_filter
带引号的、用冒号分隔的字符串格式为 `library name:plugin name:optional string of settings`。每当涉及的每个表调用全文搜索时，都会为该查询创建一个查询时令牌过滤器，允许您实现根据自定义规则生成令牌的自定义分词器。
```sql
SELECT * FROM index WHERE MATCH ('yes@no') OPTION token_filter='mylib.so:blend:@'
```
### expansion_limit
限制单个通配符的最大扩展关键字数量，默认值为 0，表示无限制。更多详细信息，请参阅 [expansion_limit](../Server_settings/Searchd.md#expansion_limit)。

## 查询优化器提示

<!-- example options_force -->

在极少数情况下，Manticore 内置的查询分析器可能无法正确理解查询并确定应使用 docid 索引、二级索引还是列扫描。为了覆盖查询优化器的决策，您可以在查询中使用以下提示：

* `/*+ DocidIndex(id) */`  强制使用 docid 索引，`/*+ NO_DocidIndex(id) */`  告诉优化器忽略它
* `/*+ SecondaryIndex(<attr_name1>[, <attr_nameN>]) */`  强制使用二级索引（如果可用），`/*+ NO_SecondaryIndex(id) */`  告诉优化器忽略它
* `/*+ ColumnarScan(<attr_name1>[, <attr_nameN>]) */`  强制使用列扫描（如果属性为列式），`/*+ NO_ColumnarScan(id) */` 告诉优化器忽略它

请注意，当执行带有过滤器的全文查询时，查询优化器会决定是将全文树结果与过滤器结果进行交集，还是使用标准的匹配后过滤方法。指定*任何*提示将强制守护进程使用执行全文树结果与过滤器结果交集的代码路径。

有关查询优化器工作原理的更多信息，请参阅[基于成本的优化器](../Searching/Cost_based_optimizer.md)页面。

<!-- request SQL -->

```sql
SELECT * FROM students where age > 21 /*+ SecondaryIndex(age) */
```

<!-- end -->

<!-- example comments -->
使用 MySQL/MariaDB 客户端时，请确保添加 `--comments` 标志以启用查询中的提示。

<!-- request mysql -->
```bash
mysql -P9306 -h0 --comments
```
<!-- end -->

<!-- proofread -->

