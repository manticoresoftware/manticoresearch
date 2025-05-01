# 搜索选项

SQL [SELECT](../Searching/Full_text_matching/Basic_usage.md#SQL) 子句和 HTTP [/search](../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) 端点支持多种选项，可用于微调搜索行为。

## 选项

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

支持的选项有：

### accurate_aggregation
整数。启用或禁用在多个线程中运行 groupby 查询时的保证聚合精度。默认值为 0。

在运行 groupby 查询时，它可以在一个普通表上并行运行，具有多个伪分片（如果启用 `pseudo_sharding`）。类似的方法适用于 RT 表。每个分片/块执行查询，但组的数量受 `max_matches` 的限制。如果来自不同分片/块的结果集具有不同组，则组计数和聚合可能不准确。请注意，Manticore 尝试根据 groupby 属性的唯一值数量（从二级索引检索）将 `max_matches` 提高到 [`max_matches_increase_threshold`](../Searching/Options.md#max_matches_increase_threshold)。如果成功，则不会损失精度。

然而，如果 groupby 属性的唯一值数量很高，进一步增加 `max_matches` 可能不是一个好的策略，因为这可能导致性能下降和更高的内存使用。将 `accurate_aggregation` 设置为 1 强制 groupby 搜索在单个线程中运行，这解决了精度问题。请注意，当 `max_matches` 无法设定得高时，才会强制在单线程中运行；否则，具有 `accurate_aggregation=1` 的搜索仍将在多个线程中运行。

总的来说，将 `accurate_aggregation` 设置为 1 可确保在 RT 表和 `pseudo_sharding=1` 的普通表中组计数和聚合的准确性。缺点是搜索会运行得更慢，因为它们将被强制在单线程中操作。

然而，如果我们有一个 RT 表和一个包含相同数据的普通表，并且我们运行一个具有 `accurate_aggregation=1` 的查询，我们可能仍然会得到不同的结果。这是因为守护进程可能会为 RT 表和普通表选择不同的 `max_matches` 设置，原因在于[`max_matches_increase_threshold`](../Searching/Options.md#max_matches_increase_threshold) 设置。

### agent_query_timeout
整数。等待远程查询完成的最大时间（以毫秒为单位），见 [本节](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout)。

### boolean_simplify
`0` 或 `1`（默认 `1`）。`boolean_simplify=1` 启用 [简化查询](../Searching/Full_text_matching/Boolean_optimization.md) 以加速查询。

### comment
字符串，用户注释，将复制到查询日志文件中。

### cutoff
整数。指定要处理的最大匹配数。如果未设置，Manticore 将自动选择一个合适的值。

<!-- example cutoff_aggregation -->

* `N = 0`: 禁用匹配数的限制。
* `N > 0`: 指示 Manticore 一旦找到 `N` 个匹配文档就停止处理结果。
* 未设置: Manticore 会自动决定阈值。

当 Manticore 不能确定匹配文档的确切计数时，查询的 `total_relation` 字段将显示 `gte`，表示**大于或等于**。这表明匹配的实际计数至少为报告的 `total_found`（在 SQL 中）或 `hits.total`（在 JSON 中）。当计数准确时，`total_relation` 将显示 `eq`。

注意：在聚合查询中使用 `cutoff` 不建议，因为它可能产生不准确或不完整的结果。

<!-- request Example -->

在聚合查询中使用 `cutoff` 可能导致不正确或误导的结果，如下示例所示：
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

Compare it with the same query without `cutoff`:
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
整数。默认值为 `3500`。此选项设置 `count distinct` 返回的计数在普通表中被保证为精确的阈值以下。

接受的值范围从 `500` 到 `15500`。超出此范围的值将被限制。

当此选项设置为 0 时，它启用一种算法，确保准确计数。该算法收集 `{group, value}` 对，将其排序，并定期消除重复项。结果是在普通表中得到精确计数。然而，由于其内存消耗高和查询执行缓慢，这种方法不适合高基数数据集。

当 `distinct_precision_threshold` 设置为大于 `0` 的值时，Manticore 采用不同的算法。它将计数加载到哈希表中并返回表的大小。如果哈希表变得过大，其内容将移动到 `HyperLogLog` 数据结构中。此时，计数变为近似值，因为 HyperLogLog 是一种概率算法。这种方法在每组之间保持固定的最大内存使用，但在计数准确性上存在权衡。

`HyperLogLog` 的准确性和从哈希表转换到 HyperLogLog 的阈值源自 `distinct_precision_threshold` 设置。使用此选项时需要谨慎，因为将其值加倍也会将计算计数所需的最大内存加倍。最大内存使用可以用以下公式粗略估计：`64 * max_matches * distinct_precision_threshold`，尽管在实践中，计数计算通常使用的内存少于最坏情况下的情况。

### expand_keywords
`0` 或 `1`（默认值为 `0`）。在可能的情况下使用精确形式和/或星号扩展关键词。有关更多详细信息，请参阅 [expand_keywords](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expand_keywords)。

### field_weights
命名整数列表（每个字段的用户权重以进行排序）。

示例：
```sql
SELECT ... OPTION field_weights=(title=10, body=3)
```

### global_idf
使用 [global_idf](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#global_idf) 文件中的全局统计（频率）进行 IDF 计算。

### idf
用引号括起来的、以逗号分隔的 IDF 计算标志列表。已知标志包括：

* `normalized`：BM25 变体, idf = log((N-n+1)/n)，根据 Robertson 等人的研究
* `plain`：普通变体, idf = log(N/n)，根据 Sparck-Jones 的研究
* `tfidf_normalized`：另外将 IDF 除以查询词计数，以便 `TF*IDF` 适合 `[0, 1]` 范围
* `tfidf_unnormalized`：在 N 为集合大小，n 为匹配文档数量的情况下，未将 IDF 额外除以查询词计数

Manticore 中历史上的默认 IDF（逆文档频率）等同于 `OPTION idf='normalized,tfidf_normalized'`，并且这些归一化可能导致几个不必要的效果。

首先，`idf=normalized` 导致关键词受到惩罚。例如，如果您搜索 `the | something`，而 `the` 在超过 50% 的文档中出现，那么同时包含关键词 `the` 和 `something` 的文档将比仅包含关键词 `something` 的文档权重更低。使用 `OPTION idf=plain` 可以避免这一点。普通 IDF 在 `[0, log(N)]` 范围内变动，关键词永远不会受到惩罚；而归一化 IDF 在 `[-log(N), log(N)]` 范围内变动，频率过高的关键词会受到惩罚。

其次，`idf=tfidf_normalized` 导致 IDF 在查询间漂移。历史上，IDF 也被查询关键词计数除以，保证了所有关键词的 `sum(tf*idf)` 仍然在 `[0,1]` 范围内。然而，这意味着像 `word1` 和 `word1 | nonmatchingword2` 这样的查询会对完全相同的结果集分配不同的权重，因为 `word1` 和 `nonmatchingword2` 的 IDF 都会被除以 2。使用 `OPTION idf='tfidf_unnormalized'` 解决了这个问题。请记住，当您禁用此归一化时，BM25、BM25A、BM25F() 排序因子也将相应调整。

IDF 标志可以组合使用；`plain` 和 `normalized` 是互斥的；`tfidf_unnormalized` 和 `tfidf_normalized` 也是互斥的；在这些互斥组中未指定的标志默认为其原始设置。这意味着 `OPTION idf=plain` 与指定 `OPTION idf='plain,tfidf_normalized'` 在整体上是相同的。

### jieba_mode
指定查询的 Jieba 分词模式。

当使用 Jieba 中文分词时，有时候为文档和查询采用不同的分词模式是有帮助的。有关模式的完整列表，请参阅 [jieba_mode](Creating_a_table/NLP_and_tokenization/Morphology.md#jieba_mode)。

### index_weights
命名整数列表。每个表的用户权重以进行排序。

### local_df
`0` 或 `1`，自动对分布式表的所有本地部分进行 DFs 求和，确保在本地分片表中一致（且准确）地计算 IDF。默认情况下对 RT 表的磁盘块启用。查询词包含通配符时被忽略。

### low_priority
`0` 或 `1`（默认为 `0`）。设置 `low_priority=1` 将以较低的优先级执行查询，其作业的重新调度频率比其他正常优先级的查询低 10 倍。

### max_matches
整数。每个查询的最大匹配值。

服务器为每个表保留在 RAM 中的最大匹配数量，并可以返回给客户端。默认值为 1000。

为了控制和限制 RAM 使用，`max_matches` 设置确定在搜索每个表时将保留多少个匹配。每个找到的匹配仍会被处理，但只有最好的 N 个将保留在内存中，并最终返回给客户端。例如，假设一个表包含 2,000,000 个查询匹配。通常不需要检索所有匹配。相反，你需要扫描所有匹配，但只选择“最好”的 500 个，例如，根据一些标准（如按相关性、价格或其他因素排序）并将这 500 个匹配显示给最终用户，分为 20 到 100 个匹配的页面。仅跟踪最好的 500 个匹配在 RAM 和 CPU 效率上远比保留所有 2,000,000 个匹配、对其进行排序，然后丢弃除了搜索结果页面所需的前 20 个。`max_matches` 控制该“最佳 N”数量中的 N。

此参数显著影响每个查询的 RAM 和 CPU 使用。值在 1,000 到 10,000 之间通常是可以接受的，但更高的限制应谨慎使用。随意将 max_matches 增加到 1,000,000 意味着 `searchd` 必须为每个查询分配和初始化一个 1百万条目匹配缓冲区。这将不可避免地增加每个查询的 RAM 使用，在某些情况下，可能会显著影响性能。

请参见 [max_matches_increase_threshold](../Searching/Options.md#max_matches_increase_threshold) 以获取有关它如何影响 `max_matches` 选项行为的更多信息。

### max_matches_increase_threshold

整数。设置可增加到的 `max_matches` 阈值。默认值为 16384。

当启用 `pseudo_sharding` 且检测到 groupby 属性的唯一值数量少于此阈值时，Manticore 可能会增加 `max_matches` 以提高 groupby 和/或聚合的准确性。当伪分片在多个线程中执行查询或当 RT 表在磁盘块中进行并行搜索时，可能会发生精度损失。

如果 groupby 属性的唯一值数量少于该阈值，`max_matches` 将被设置为此数量。否则，将使用默认的 `max_matches`。

如果在查询选项中显式设置了 `max_matches`，则此阈值无效。

请记住，如果此阈值设置得过高，将导致内存消耗增加和整体性能下降。

你也可以使用 [accurate_aggregation](../Searching/Options.md#accurate_aggregation) 选项强制执行保证的 groupby/聚合准确性模式。

### max_query_time
设置以毫秒为单位的最大搜索查询时间。必须是非负整数。默认值为 0，这意味着“没有限制”。一旦指定时间到达，本地搜索查询将被停止。请注意，如果你执行的搜索查询多个本地表，此限制适用于每个表。请注意，由于不断跟踪是否该停止查询造成的开销，这可能会略微增加查询的响应时间。

### max_predicted_time
整数。最大预测搜索时间；参见 [predicted_time_costs](../Server_settings/Searchd.md#predicted_time_costs)。

### morphology
`none` 允许将所有查询术语替换为其确切形式，如果该表是用启用的 [index_exact_words](../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words) 构建的。这对于防止查询术语的词干提取或词形还原很有用。

### not_terms_only_allowed
<!-- example not_terms_only_allowed -->
`0` 或 `1` 允许查询的单独 [否定](../Searching/Full_text_matching/Operators.md#Negation-operator)。默认值为 0。另见相应的 [全局设置](../Server_settings/Searchd.md#not_terms_only_allowed)。

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

有关每个排名器的更多详细信息，请参见 [搜索结果排名](../Searching/Sorting_and_ranking.md#Available-built-in-rankers)。

### rand_seed
允许你为 `ORDER BY RAND()` 查询指定特定的整数种子值，例如：`... OPTION rand_seed=1234`。默认情况下，每个查询会自动生成一个新的不同的种子值。

### retry_count
整数。分布式重试计数。

### retry_delay
整数。分布式重试延迟，以毫秒为单位。

### scroll

字符串。用于使用 [滚动分页方法](../Searching/Pagination.md#Scroll-Search-Option) 进行分页结果的滚动令牌。

### sort_method
* `pq` - 优先级队列，默认设置
* `kbuffer` - 为已排序数据提供更快的排序，例如，按 id 排序的表数据
在这两种情况下，结果集是相同的；选择一个选项或另一个选项可能只是改善（或恶化）性能。

### threads
限制当前查询处理使用的最大线程数。默认 - 无限制（查询可以占用所有全局定义的 [threads](../Server_settings/Searchd.md#threads)）。
对于一批查询，选项必须附加到批次中的第一个查询上，并在创建工作队列时应用，并对整个批次有效。此选项与选项 [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query) 的含义相同，但仅适用于当前查询或查询批次。

### token_filter
引号内，冒号分隔的字符串 `library name:plugin name:optional string of settings`。当每个相关表执行全文搜索时，会为每个搜索创建一个查询时间的令牌过滤器，允许您实现一个自定义令牌生成器，根据自定义规则生成令牌。
```sql
SELECT * FROM index WHERE MATCH ('yes@no') OPTION token_filter='mylib.so:blend:@'
```
### expansion_limit
限制单个通配符的最大扩展关键字数量，默认值为 0 表示没有限制。有关更多详细信息，请参见 [expansion_limit](../Server_settings/Searchd.md#expansion_limit)。

## 查询优化器提示

<!-- example options_force -->

在少数情况下，Manticore 的内置查询分析器可能错误地理解查询，并决定是否使用 docid 索引、辅助索引或列式扫描。要覆盖查询优化器的决策，可以在查询中使用以下提示：

* `/*+ DocidIndex(id) */` 强制使用 docid 索引，`/*+ NO_DocidIndex(id) */` 告诉优化器忽略它
* `/*+ SecondaryIndex(<attr_name1>[, <attr_nameN>]) */` 强制使用辅助索引（如果可用），`/*+ NO_SecondaryIndex(id) */` 告诉优化器忽略它
* `/*+ ColumnarScan(<attr_name1>[, <attr_nameN>]) */` 强制使用列式扫描（如果属性是列式的），`/*+ NO_ColumnarScan(id) */` 告诉优化器忽略它

请注意，当执行带有过滤器的全文查询时，查询优化器在将全文树的结果与过滤器结果相交或使用标准的匹配-然后-过滤方法之间进行选择。指定 *任何* 提示将强制守护进程使用执行全文树结果与过滤器结果交集的代码路径。

有关查询优化器工作原理的更多信息，请参见 [基于成本的优化器](../Searching/Cost_based_optimizer.md) 页面。

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
