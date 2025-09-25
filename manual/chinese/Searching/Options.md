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
整数。启用或禁用在多线程中运行 groupby 查询时保证聚合的准确性。默认值为 0。

执行 groupby 查询时，可以在启用了 `pseudo_sharding` 的普通表上并行运行多个伪分片。类似的方法适用于 RT 表。每个分片/块执行查询，但分组数受限于 `max_matches`。如果来自不同分片/块的结果集包含不同的组，则组计数和聚合可能不准确。注意，Manticore 会尝试根据从二级索引中获取的 groupby 属性的唯一值数量，将 `max_matches` 增加到 [`max_matches_increase_threshold`](../Searching/Options.md#max_matches_increase_threshold) 以上。如果成功，将不会出现准确度损失。

然而，如果 groupby 属性的唯一值数量很高，进一步增加 `max_matches` 可能不是良策，因为这样会导致性能下降和内存使用增加。设置 `accurate_aggregation` 为 1 会强制在单线程中运行 groupby 搜索，从而解决准确性问题。注意，只有当 `max_matches` 无法设置得足够高时，才会强制单线程运行；否则，设置了 `accurate_aggregation=1` 的搜索仍会在多线程中执行。

总体来说，将 `accurate_aggregation` 设置为 1 可确保 RT 表和启用了 `pseudo_sharding=1` 的普通表中的组计数和聚合准确性。缺点是搜索会变慢，因为它们被强制在单线程中运行。

不过，如果我们同时有一个 RT 表和一个包含相同数据的普通表，并且运行带有 `accurate_aggregation=1` 的查询，仍然可能得到不同的结果。这是因为由于 [`max_matches_increase_threshold`](../Searching/Options.md#max_matches_increase_threshold) 设置，守护进程可能为 RT 表和普通表选择不同的 `max_matches` 配置。

### agent_query_timeout
整数。等待远程查询完成的最长时间（毫秒），详见 [此部分](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout)。

### boolean_simplify
`0` 或 `1`（默认值为 `1`）。`boolean_simplify=1` 启用[简化查询](../Searching/Full_text_matching/Boolean_optimization.md)以加快速度。

此选项也可以在 [searchd 配置](../Server_settings/Searchd.md#boolean_simplify)中全局设置，用以更改所有查询的默认行为。单个查询设置会覆盖全局设置。

### comment
字符串，用户注释，将被复制到查询日志文件中。

### cutoff
整数。指定要处理的最大匹配数。如果未设置，Manticore 会自动选择合适的值。

<!-- example cutoff_aggregation -->

* `N = 0`：禁用匹配数量限制。
* `N > 0`：指示 Manticore 在找到 `N` 个匹配文档后立即停止处理结果。
* 未设置：Manticore 自动决定阈值。

当 Manticore 无法确定匹配文档的准确计数时，查询的 [元信息](../Node_info_and_management/SHOW_META.md#SHOW-META) 中的 `total_relation` 字段将显示为 `gte`，表示**大于或等于**。这表示实际匹配数至少为报告的 `total_found`（SQL）或 `hits.total`（JSON）。当计数准确时，`total_relation` 显示为 `eq`。

注意：不建议在聚合查询中使用 `cutoff`，因为它可能导致结果不准确或不完整。

<!-- request Example -->

在聚合查询中使用 `cutoff` 可能导致不正确或误导的结果，如以下示例所示：
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

与不带 `cutoff` 的相同查询对比：
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
整数。默认值为 `3500`。此选项设置普通表中由 `count distinct` 返回的计数保证精确的阈值。

接受的取值范围是 `500` 到 `15500`。超出此范围的值将被截断。

当此选项设置为 0 时，启用一种算法以确保精确计数。该算法收集 `{group, value}` 对，进行排序，并定期消除重复项。结果是对普通表中的计数精确。但由于高内存消耗和查询执行缓慢，此方法不适合高基数数据集。

当 `distinct_precision_threshold` 大于 0 时，Manticore 采用不同算法。它将计数加载到哈希表中，并返回表的大小。如果哈希表过大，其内容会移动到 `HyperLogLog` 数据结构中。这时计数变为近似计数，因为 HyperLogLog 是一种概率算法。该方法维持了每组的固定最大内存使用，但计数的准确度有所折中。

`HyperLogLog` 的准确性和从哈希表转变为 HyperLogLog 的阈值是由 `distinct_precision_threshold` 设置决定的。应谨慎使用此选项，因为其值翻倍将使计算计数所需的最大内存也翻倍。最大内存使用量的粗略估算公式为：`64 * max_matches * distinct_precision_threshold`，尽管实际计数计算通常消耗的内存低于最坏情况。

### expand_keywords
`0` 或 `1`（默认为 `0`）。在可能的情况下，使用精确形式和/或星号扩展关键词。更多细节请参见 [expand_keywords](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expand_keywords)。

### field_weights
命名的整数列表（每字段用户权重，用于排名）。

示例：
```sql
SELECT ... OPTION field_weights=(title=10, body=3)
```

### global_idf
使用来自 [global_idf](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#global_idf) 文件的全局统计数据（频率）进行 IDF 计算。

### idf
带引号的逗号分隔的 IDF 计算标志列表。已知的标志有：

* `normalized`：BM25 变体，idf = log((N-n+1)/n)，依据 Robertson 等人的定义
* `plain`：普通变体，idf = log(N/n)，依据 Sparck-Jones 的定义
* `tfidf_normalized`：额外将 IDF 除以查询词数，使得 `TF*IDF` 适配于 [0, 1] 范围内
* `tfidf_unnormalized`：不额外将 IDF 除以查询词数，其中 N 是集合大小，n 是匹配文档数

Manticore 中历史默认的 IDF（反文档频率）相当于 `OPTION idf='normalized,tfidf_normalized'`，而这些归一化可能导致一些不良影响。

首先，`idf=normalized` 会导致关键词惩罚。例如，如果你搜索 `the | something`，且 `the` 出现在超过 50% 的文档中，那么同时含有关键词 `the` 和 `something` 的文档权重会小于仅含关键词 `something` 的文档。使用 `OPTION idf=plain` 可以避免这种情况。普通 IDF 在 `[0, log(N)]` 范围内变化，关键词不会受到惩罚；而归一化 IDF 在 `[-log(N), log(N)]` 范围内变化，过于频繁的关键词会被惩罚。

其次，`idf=tfidf_normalized` 导致跨查询的 IDF 漂移。历史上，IDF 还会除以查询关键词数，保证所有关键词的 `sum(tf*idf)` 保持在 [0,1] 范围内。然而，这意味着像 `word1` 和 `word1 | nonmatchingword2` 这样的查询会对完全相同的结果集赋予不同权重，因为两者的 IDF 都会被除以 2。使用 `OPTION idf='tfidf_unnormalized'` 可以解决该问题。请注意，当禁用此归一化时，BM25、BM25A、BM25F() 排名因子将相应调整。

IDF 标志可以组合；`plain` 与 `normalized` 互斥；`tfidf_unnormalized` 与 `tfidf_normalized` 也互斥；在这些互斥组中未指定的标志默认为其原始设置。这意味着 `OPTION idf=plain` 等同于完整指定 `OPTION idf='plain,tfidf_normalized'`。

### jieba_mode
指定查询的 Jieba 分词模式。

使用 Jieba 中文分词时，有时对文档和查询使用不同的分词模式有帮助。完整的模式列表请参见 [jieba_mode](../Creating_a_table/NLP_and_tokenization/Morphology.md#jieba_mode)。

### index_weights
命名的整数列表。每表用户权重，用于排名。

### local_df
`0` 或 `1`，自动将本地分布式表所有分片的 DF 求和，确保本地分片表 IDF 的一致性和准确性。默认对 RT 表的磁盘分块启用。带通配符的查询词会被忽略。

### low_priority
`0` 或 `1`（默认为 `0`）。设置 `low_priority=1` 会以较低优先级执行查询，其作业调度频率比普通优先级查询低 10 倍。

### max_matches
整数。每查询最大匹配数。

服务器为每个表在内存中保留的最大匹配数，并可返回给客户端。默认值为 1000。

`max_matches` 参数用于控制和限制内存使用，它决定了搜索时对每个表保留多少匹配。虽然会处理每个找到的匹配，但只保留内存中最优的 N 个，并将这些返回给客户端。例如，假设某表对某查询产生了 2,000,000 条匹配。通常，您不需要返回全部结果，而是需要扫描所有匹配，然后选择 “最优”的 500 条（例如，按相关性、价格等排序），并将这 500 条以页面形式显示给终端用户。仅跟踪这 500 条比保留全部 2,000,000 条，排序后丢弃绝大多数要节省大量内存和 CPU。`max_matches` 就是控制这个 “最优 N” 的参数。

此参数显著影响每查询的内存和 CPU 使用。通常 1,000 到 10,000 的范围是可接受的，但更高的值需要谨慎。轻率地将 max_matches 调整到 1,000,000 意味着每个查询都需要为一百万个匹配条目分配并初始化缓冲区，从而显著提升每查询内存使用，有时还会影响性能。

有关此参数如何影响 `max_matches` 选项行为的更多信息，请参阅 [max_matches_increase_threshold](../Searching/Options.md#max_matches_increase_threshold)。

### max_matches_increase_threshold

整数。设置 `max_matches` 可以被增加到的阈值。默认值为 16384。

当启用 `pseudo_sharding` 时，Manticore 可能会增加 `max_matches` 以提升 groupby 和/或聚合的准确性，条件是检测到 groupby 属性的唯一值数量低于此阈值。伪分片在多线程执行查询或 RT 表在磁盘块上进行并行搜索时，可能导致准确性降低。

如果 groupby 属性的唯一值数量低于阈值，则 `max_matches` 会被设置为该数值。否则，使用默认的 `max_matches`。

如果查询选项中显式设置了 `max_matches`，该阈值不会生效。

请注意，若此阈值设置过高，会导致内存消耗增加且整体性能下降。

你还可以使用[accurate_aggregation](../Searching/Options.md#accurate_aggregation) 选项强制保证 groupby/aggregate 的准确模式。

### max_query_time
设置搜索查询的最大时间，单位为毫秒。必须为非负整数。默认值为 0，表示“不限制”。本地搜索查询将在指定时间到达后停止。请注意，如果你执行的是查询多个本地表的搜索，则该时限将分别应用于每个表。请注意，由于不断跟踪是否到了停止查询时间，可能会略微增加查询的响应时间。

### max_predicted_time
整数。最大预测搜索时间；参见 [predicted_time_costs](../Server_settings/Searchd.md#predicted_time_costs)。

### morphology
`none` 允许用完全匹配的形式替换所有查询词语，前提是该表是在启用了 [index_exact_words](../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words) 的情况下构建的。这对于防止查询词语的词干提取或词形还原非常有用。

### not_terms_only_allowed
<!-- example not_terms_only_allowed -->
`0` 或 `1` 允许查询使用独立的[否定](../Searching/Full_text_matching/Operators.md#Negation-operator)。默认值为 0。另见相应的[全局设置](../Server_settings/Searchd.md#not_terms_only_allowed)。

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

有关每个 ranker 的更多详细信息，请参阅[搜索结果排序](../Searching/Sorting_and_ranking.md#Available-built-in-rankers)。

### rand_seed
允许你为 `ORDER BY RAND()` 查询指定特定的整数种子值，例如：`... OPTION rand_seed=1234`。默认情况下，每个查询都会自动生成一个新的不同的种子值。

### retry_count
整数。分布式重试次数。

### retry_delay
整数。分布式重试延迟，单位为毫秒。

### scroll

字符串。用于通过[滚动分页方法](../Searching/Pagination.md#Scroll-Search-Option)分页结果的滚动令牌。

### sort_method
* `pq` - 优先队列，默认设置
* `kbuffer` - 对已经预排序的数据（例如按 id 排序的表数据）提供更快的排序
两种方法的结果集相同；选择其中一种可能只是为了提高（或降低）性能。

### threads
限制当前查询处理使用的最大线程数。默认 - 不限制（查询可以占用所有全局定义的[线程](../Server_settings/Searchd.md#threads)）。
对于一批查询，该选项必须附加在批次中的第一个查询上，在工作队列创建时应用，并对整个批次生效。该选项的含义与选项 [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query) 相同，但仅应用于当前查询或一批查询。

### token_filter
加引号、冒号分隔的字符串，格式为 `库名:插件名:可选的设置字符串`。在全文搜索被涉及的每个表调用时，为每次搜索创建查询时令牌过滤器，允许你根据自定义规则实现自定义分词器生成令牌。
```sql
SELECT * FROM index WHERE MATCH ('yes@no') OPTION token_filter='mylib.so:blend:@'
```
### expansion_limit
限制单个通配符扩展的关键词最大数量，默认值为 0 表示无限制。更多细节请参见 [expansion_limit](../Server_settings/Searchd.md#expansion_limit)。

## 查询优化器提示

<!-- example options_force -->

在极少数情况下，Manticore 内置的查询分析器可能错误地理解查询，并决定是否使用 docid 索引、二级索引或列扫描。要覆盖查询优化器的决策，可以在查询中使用以下提示：

* `/*+ DocidIndex(id) */` 强制使用 docid 索引，`/*+ NO_DocidIndex(id) */` 告诉优化器忽略它
* `/*+ SecondaryIndex(<attr_name1>[, <attr_nameN>]) */` 强制使用二级索引（如果可用），`/*+ NO_SecondaryIndex(id) */` 告诉优化器忽略它
* `/*+ ColumnarScan(<attr_name1>[, <attr_nameN>]) */` 强制使用列扫描（如果属性是列式存储），`/*+ NO_ColumnarScan(id) */` 告诉优化器忽略它

注意，当执行带过滤器的全文查询时，查询优化器会在对全文树和过滤器结果求交集，或使用标准的先匹配再过滤方法之间做出决定。指定*任何*提示都会强制守护进程使用对全文树和过滤器结果求交集的代码路径。

有关查询优化器如何工作的更多信息，请参阅[基于成本的优化器](../Searching/Cost_based_optimizer.md) 页面。

<!-- request SQL -->

```sql
SELECT * FROM students where age > 21 /*+ SecondaryIndex(age) */
```

<!-- end -->

<!-- example comments -->
使用 MySQL/MariaDB 客户端时，确保加上 `--comments` 参数以启用查询中的提示。

<!-- request mysql -->
```bash
mysql -P9306 -h0 --comments
```
<!-- end -->

<!-- proofread -->

