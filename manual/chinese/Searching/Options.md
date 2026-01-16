# 搜索选项

SQL [SELECT](../Searching/Full_text_matching/Basic_usage.md#SQL) 子句和 HTTP [/search](../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) 端点支持许多选项，可用于微调搜索行为。

## OPTION

### 一般语法

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
整数。启用或禁用在多线程运行 groupby 查询时的保证聚合准确性。默认值为 0。

当运行 groupby 查询时，可以在普通表上使用多个伪分片并行运行（如果 `pseudo_sharding` 开启）。类似的方法适用于 RT 表。每个分片/块执行查询，但组的数量受 `max_matches` 限制。如果不同分片/块的结果集包含不同的组，组计数和聚合可能会不准确。注意 Manticore 会根据 groupby 属性的唯一值数量（从二级索引中检索）将 `max_matches` 增加到 [`max_matches_increase_threshold`](../Searching/Options.md#max_matches_increase_threshold)。如果成功，将不会出现准确性损失。

然而，如果 groupby 属性的唯一值数量很高，进一步增加 `max_matches` 可能不是一个好策略，因为它可能导致性能下降和内存使用增加。将 `accurate_aggregation` 设置为 1 会强制 groupby 搜索在单线程中运行，从而解决准确性问题。注意，仅当 `max_matches` 无法设置得足够高时，才会强制单线程运行；否则，带有 `accurate_aggregation=1` 的搜索仍会在多线程中运行。

总体而言，将 `accurate_aggregation` 设置为 1 可确保 RT 表和 `pseudo_sharding=1` 的普通表中的组计数和聚合准确性。缺点是搜索会变慢，因为它们将被强制在单线程中运行。

然而，如果我们有一个 RT 表和一个包含相同数据的普通表，并运行带有 `accurate_aggregation=1` 的查询，我们仍可能收到不同的结果。这是因为守护进程可能会根据 [`max_matches_increase_threshold`](../Searching/Options.md#max_matches_increase_threshold) 设置为 RT 表和普通表选择不同的 `max_matches` 值。

### agent_query_timeout
整数。等待远程查询完成的最大时间（以毫秒为单位），请参见 [此部分](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout)。

### boolean_simplify
`0` 或 `1`（默认为 `1`）。`boolean_simplify=1` 启用 [查询简化](../Searching/Full_text_matching/Boolean_optimization.md) 以加快速度。

此选项也可以在 [searchd 配置](../Server_settings/Searchd.md#boolean_simplify) 中全局设置，以更改所有查询的默认行为。每个查询的选项将覆盖全局设置。

### comment
字符串，用户评论会被复制到查询日志文件中。

### cutoff
整数。指定要处理的最大匹配数。如果未设置，Manticore 将自动选择一个适当的值。

<!-- example cutoff_aggregation -->

* `N = 0`：禁用匹配数的限制。
* `N > 0`：指示 Manticore 在找到 `N` 个匹配文档后立即停止处理结果。
* 未设置：Manticore 自动决定阈值。

当 Manticore 无法确定匹配文档的确切数量时，查询 [元信息](../Node_info_and_management/SHOW_META.md#SHOW-META) 中的 `total_relation` 字段将显示 `gte`，表示 **大于或等于**。这表明实际匹配数至少为报告的 `total_found`（在 SQL 中）或 `hits.total`（在 JSON 中）。当计数准确时，`total_relation` 将显示 `eq`。

注意：在聚合查询中使用 `cutoff` 不推荐，因为它可能导致不准确或不完整的结果。

<!-- request Example -->

在聚合查询中使用 `cutoff` 可能导致不正确或误导性的结果，如下例所示：
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

与不使用 `cutoff` 的相同查询进行比较：
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
整数。默认值为 `3500`。此选项设置在普通表中 `count distinct` 返回的计数保证精确的阈值。

接受的值范围为 `500` 到 `15500`。超出此范围的值将被限制。

当此选项设置为 0 时，它启用一种确保精确计数的算法。该算法收集 `{group, value}` 对，对其进行排序，并定期消除重复项。结果是在普通表中精确的计数。然而，由于其高内存消耗和慢查询执行，这种方法不适合高基数数据集。

当 `distinct_precision_threshold` 设置为大于 `0` 的值时，Manticore 采用不同的算法。它将计数加载到哈希表中并返回表的大小。如果哈希表变得太大，其内容将被移动到 `HyperLogLog` 数据结构中。此时，计数变得近似，因为 HyperLogLog 是一种概率算法。这种方法保持每组的最大内存使用量固定，但计数准确性存在权衡。

`HyperLogLog` 的精度和从哈希表转换到 HyperLogLog 的阈值源自 `distinct_precision_threshold` 设置。使用此选项时需谨慎，因为将其值加倍也会使计算计数所需的最大内存加倍。最大内存使用量可以大致通过此公式估算：`64 * max_matches * distinct_precision_threshold`，尽管在实践中，计数计算通常使用的内存少于最坏情况下的内存。

### expand_keywords
`0` 或 `1`（默认为 `0`）。尽可能扩展精确形式和/或带星号的关键词。有关更多详细信息，请参阅 [expand_keywords](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expand_keywords)。

### expand_blended
`0`、`off`、`1` 或任何 `blend_mode` 选项的组合（默认为 `0`）。在查询解析期间，将混合关键词（包含通过 [blend_chars](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_chars) 配置的字符的标记）扩展为其构成变体。启用后，像 "well-being"（如果 `-` 在 `blend_chars` 中配置）这样的关键词将被扩展为 "well-being"、"wellbeing"、"well" 和 "being" 等变体，然后在查询树中被分组为 OR 子树。

支持的值为：
* `0` 或 `off` - 禁用混合扩展（默认）。混合关键词将按正常方式处理，不进行扩展。
* `1` - 启用混合扩展，并使用表的 [blend_mode](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_mode) 设置来确定生成哪些变体。
* 任何混合模式选项 - 启用混合扩展并使用指定的混合模式，覆盖表的 `blend_mode` 设置。

有关选项的更多详细信息，请参阅 [blend_mode](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_mode)。

### field_weights
命名整数列表（按字段的用户权重用于排序）。

示例：
```sql
SELECT ... OPTION field_weights=(title=10, body=3)
```

### global_idf
使用 [global_idf](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#global_idf) 文件中的全局统计信息（频率）进行 IDF 计算。

### idf
带引号的、逗号分隔的 IDF 计算标志列表。已知标志包括：

* `normalized`：BM25 变体，idf = log((N-n+1)/n)，如 Robertson 等人所述
* `plain`：普通变体，idf = log(N/n)，如 Sparck-Jones 所述
* `tfidf_normalized`：此外将 IDF 除以查询词数，使 `TF*IDF` 保持在 [0, 1] 范围内
* `tfidf_unnormalized`：不将 IDF 除以查询词数，其中 N 是集合大小，n 是匹配文档数

Manticore 历史默认的 IDF（逆文档频率）等同于 `OPTION idf='normalized,tfidf_normalized'`，这些归一化可能会导致一些不良影响。

首先，`idf=normalized` 会导致关键词惩罚。例如，如果你搜索 `the | something`，并且 `the` 出现在超过 50% 的文档中，那么同时包含 `the` 和 `something` 的文档的权重将低于仅包含 `something` 的文档。使用 `OPTION idf=plain` 可避免此问题。普通 IDF 的范围在 [0, log(N)] 内，关键词永远不会被惩罚；而归一化 IDF 的范围在 [-log(N), log(N)] 内，过于频繁的关键词会被惩罚。

其次，`idf=tfidf_normalized` 会导致 IDF 在查询之间漂移。历史上，IDF 也会除以查询关键词数，确保所有关键词的 `sum(tf*idf)` 总和保持在 [0,1] 范围内。然而，这意味着像 `word1` 和 `word1 | nonmatchingword2` 这样的查询会为完全相同的结果集分配不同的权重，因为 `word1` 和 `nonmatchingword2` 的 IDF 都会被除以 2。使用 `OPTION idf='tfidf_unnormalized'` 可解决此问题。请注意，当禁用此归一化时，BM25、BM25A、BM25F() 排名因素将相应调整。

IDF 标志可以组合；`plain` 和 `normalized` 互斥；`tfidf_unnormalized` 和 `tfidf_normalized` 也互斥；未指定的标志在这些互斥组中默认使用其原始设置。这意味着 `OPTION idf=plain` 等同于完整指定 `OPTION idf='plain,tfidf_normalized'`。

### jieba_mode
指定查询的 Jieba 分词模式。

在使用 Jieba 中文分词时，有时需要为文档和查询使用不同的分词模式。有关完整模式列表，请参阅 [jieba_mode](../Creating_a_table/NLP_and_tokenization/Morphology.md#jieba_mode)。

### index_weights
命名整数列表。按表的用户权重用于排序。

### local_df
`0` 或 `1`，自动汇总分布式表的所有本地部分的 DF，确保本地分片表的 IDF 一致（且准确）。默认启用用于 RT 表的磁盘分片。带有通配符的查询术语被忽略。

### low_priority
`0` 或 `1`（默认为 `0`）。设置 `low_priority=1` 以较低优先级执行查询，其任务重新调度的频率比其他正常优先级查询低 10 倍。

### max_matches
整数。每查询最大匹配值。

服务器为每个表在 RAM 中保留的最大匹配数，并可返回给客户端。默认值为 1000。

引入 `max_matches` 设置是为了控制和限制 RAM 使用，该设置决定了在搜索每个表时将保留多少匹配项在 RAM 中。每个找到的匹配项仍会被处理，但只有最佳的 N 个匹配项会被保留在内存中，并最终返回给客户端。例如，假设一个表包含 2,000,000 个匹配项用于某个查询。很少需要检索所有匹配项。相反，你需要扫描所有匹配项，但仅选择基于某些标准（例如按相关性、价格或其他因素排序）的“最佳”500 个匹配项，并将这 500 个匹配项以每页 20 到 100 个匹配项的形式显示给最终用户。仅跟踪最佳的 500 个匹配项比保留所有 2,000,000 个匹配项、排序后丢弃除前 20 个外的所有内容，对 RAM 和 CPU 更加高效。`max_matches` 控制该“最佳 N”中的 N 值。

此参数会显著影响每个查询的RAM和CPU使用量。值在1,000到10,000之间通常是可以接受的，但提高限制时应谨慎使用。随意将max_matches增加到1,000,000意味着`searchd`必须为每个查询分配并初始化一个包含100万条匹配项的缓冲区。这不可避免地会增加每个查询的RAM使用量，并且在某些情况下可能明显影响性能。

有关它如何影响`max_matches`选项行为的更多信息，请参阅[max_matches_increase_threshold](../Searching/Options.md#max_matches_increase_threshold)。

### max_matches_increase_threshold

整数。设置`max_matches`可以增加的阈值。默认值为16384。

当启用`pseudo_sharding`且检测到分组属性的唯一值数量小于此阈值时，Manticore可能会增加`max_matches`以提高分组和/或聚合的准确性。当伪分片在多个线程中执行查询或RT表在磁盘块中进行并行搜索时，可能会发生准确性下降。

如果分组属性的唯一值数量小于阈值，`max_matches`将设置为该数量。否则，将使用默认的`max_matches`。

如果在查询选项中显式设置了`max_matches`，则此阈值无效。

请注意，如果此阈值设置得过高，会导致内存消耗增加和整体性能下降。

您还可以使用[accurate_aggregation](../Searching/Options.md#accurate_aggregation)选项强制启用保证的分组/聚合准确性模式。

### max_query_time
设置最大搜索查询时间（以毫秒为单位）。必须是非负整数。默认值为0，表示“不限制”。本地搜索查询一旦达到指定时间就会停止。请注意，如果您执行的搜索查询了多个本地表，此限制适用于每个表。请注意，由于不断跟踪是否需要停止查询所产生的开销，这可能会略微增加查询的响应时间。

### max_predicted_time
整数。最大预测搜索时间；请参阅[predicted_time_costs](../Server_settings/Searchd.md#predicted_time_costs)。

### morphology
`none`允许在表使用[index_exact_words](../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words)启用时，将所有查询术语替换为它们的确切形式。这对于防止对查询术语进行词干提取或词形还原很有用。

### not_terms_only_allowed
<!-- example not_terms_only_allowed -->
`0`或`1`允许查询中使用独立的[否定](../Searching/Full_text_matching/Operators.md#Negation-operator)。默认值为0。另请参阅相应的[全局设置](../Server_settings/Searchd.md#not_terms_only_allowed)。

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
从以下选项中选择：
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

有关每个排序器的更多详细信息，请参阅[搜索结果排序](../Searching/Sorting_and_ranking.md#Available-built-in-rankers)。

### rand_seed
允许您为`ORDER BY RAND()`查询指定特定的整数种子值，例如：`... OPTION rand_seed=1234`。默认情况下，每个查询都会自动生成一个新的且不同的种子值。

### retry_count
整数。分布式重试次数。

### retry_delay
整数。分布式重试延迟（以毫秒为单位）。

### scroll

字符串。用于使用[Scroll分页方法](../Searching/Pagination.md#Scroll-Search-Option)分页结果的滚动令牌。

### sort_method
* `pq` - 优先队列，默认设置
* `kbuffer` - 为已预先排序的数据（例如按id排序的表数据）提供更快的排序
两种情况下结果集相同；选择其中一个选项可能会简单地提高（或降低）性能。

### threads
限制当前查询处理使用的最大线程数。默认值 - 无限制（查询可以占用全局定义的[threads](../Server_settings/Searchd.md#threads)中的所有线程）。
对于查询批次，该选项必须附加到批次中的第一个查询，并在创建工作队列时应用，对整个批次有效。此选项与选项[max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query)含义相同，但仅适用于当前查询或查询批次。

### token_filter
用引号括起的、以冒号分隔的`library name:plugin name:optional string of settings`字符串。当每个表调用全文搜索时，为每个搜索创建一个查询时的标记过滤器，允许您实现自定义分词器，根据自定义规则生成标记。
```sql
SELECT * FROM index WHERE MATCH ('yes@no') OPTION token_filter='mylib.so:blend:@'
```
### expansion_limit
限制单个通配符扩展的关键字最大数量，默认值为0表示无限制。有关更多详细信息，请参阅[expansion_limit](../Server_settings/Searchd.md#expansion_limit)。

## 查询优化器提示

<!-- example options_force -->

在极少数情况下，Manticore内置的查询分析器可能无法正确理解查询并确定是否应使用docid索引、二级索引或列扫描。要覆盖查询优化器的决策，您可以在查询中使用以下提示：

* `/*+ DocidIndex(id) */` 强制使用docid索引，`/*+ NO_DocidIndex(id) */` 告诉优化器忽略它
* `/*+ SecondaryIndex(<attr_name1>[, <attr_nameN>]) */` 强制使用二级索引（如果可用），`/*+ NO_SecondaryIndex(id) */` 告诉优化器忽略它
* `/*+ ColumnarScan(<attr_name1>[, <attr_nameN>]) */` 强制使用列扫描（如果属性是列式的），`/*+ NO_ColumnarScan(id) */` 告诉优化器忽略它

请注意，当使用过滤器执行全文查询时，查询优化器会决定是对全文树的结果与过滤器结果进行交集运算，还是采用标准的先匹配后过滤方法。指定任何提示都将强制守护进程使用对全文树结果与过滤器结果进行交集运算的代码路径。

有关查询优化器工作原理的更多信息，请参阅[基于成本的优化器](../Searching/Cost_based_optimizer.md)页面。

<!-- request SQL -->

```sql
SELECT * FROM students where age > 21 /*+ SecondaryIndex(age) */
```

<!-- end -->

<!-- example comments -->
当使用MySQL/MariaDB客户端时，请确保包含`--comments`标志以在查询中启用提示。

<!-- request mysql -->
```bash
mysql -P9306 -h0 --comments
```
<!-- end -->

<!-- proofread -->

