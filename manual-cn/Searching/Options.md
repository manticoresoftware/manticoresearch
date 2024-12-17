# 搜索选项

SQL 的 [SELECT](../Searching/Full_text_matching/Basic_usage.md#SQL) 子句和 HTTP 的 [/search](../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) 端点支持许多选项，可用于微调搜索行为。

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
    "index" : "index_name",
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
    "index" : "test",
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
整数。启用或禁用在运行groupby查询时保证聚合精度（多线程查询时）。默认值为 0。

在运行 groupby 查询时，可以在开启 `pseudo_sharding` 时在普通表上并行运行（多个伪分片）。类似方法适用于 RT 表。每个分片/块执行查询，但组的数量受到 `max_matches` 的限制。如果来自不同分片/块的结果集中包含不同的组，则组的计数和聚合结果可能会不准确。请注意，Manticore 会尝试根据 groupby 属性的唯一值数量（从二级索引中检索）将 `max_matches` 增加到 [`max_matches_increase_threshold`](../Searching/Options.md#max_matches_increase_threshold)。如果成功，则不会失去精度。

然而，如果 groupby 属性的唯一值数量很高，进一步增加 `max_matches` 可能不是一个好策略，因为它可能会导致性能下降和更高的内存使用。将 `accurate_aggregation` 设置为 1 强制 groupby 查询在单线程中运行，以解决精度问题。请注意，只有当 `max_matches` 无法设置得足够高时，才会强制单线程运行；否则，即使设置了 `accurate_aggregation=1`，查询仍将以多线程方式运行。

总的来说，将 `accurate_aggregation` 设置为 1 可以确保在 RT 表和带有 `pseudo_sharding=1` 的普通表中的组计数和聚合结果的准确性。缺点是搜索将变慢，因为它们将被迫在单线程中运行。

然而，如果我们有一个 RT 表和一个包含相同数据的普通表，并运行带有 `accurate_aggregation=1` 的查询，我们可能仍会收到不同的结果。这是因为守护进程可能会因 [`max_matches_increase_threshold`](../Searching/Options.md#max_matches_increase_threshold) 设置而为 RT 表和普通表选择不同的 `max_matches` 设置。

### agent_query_timeout
整数。等待远程查询完成的最长时间（以毫秒为单位），详见[本节](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout)。

### boolean_simplify
`0` 或 `1`（默认值为 `0`）。`boolean_simplify=1` 启用[简化查询](../Searching/Full_text_matching/Boolean_optimization.md)，以加快查询速度。

### comment
字符串，用户注释会被复制到查询日志文件中。

### cutoff
### cutoff

整数。最大找到的匹配项阈值。如果未指定，系统会自动选择值。

- `N = 0` 禁用阈值。
- `N > 0`：指示 Manticore 一旦找到 `N` 个文档就停止查找结果。
- 如果未设置：Manticore 会自动决定该值。

如果 Manticore 无法计算出确切的匹配文档数量，您将在查询的[元信息](../Node_info_and_management/SHOW_META.md#SHOW-META)中看到 `total_relation: gte`，这表示实际计数**大于或等于**总数（SQL 中的 `total_found`，通过 HTTP 的 JSON 中的 `hits.total`）。如果总值是精确的，您将获得 `total_relation: eq`。

<!-- example cutoff_aggregation -->
注意：在聚合查询中使用 `cutoff` 不推荐，因为它可能会产生无意义的结果。

<!-- request Example -->

在聚合查询中使用 `cutoff` 可能会导致不正确的结果。
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
整数。默认值为 `3500`。此选项设置一个阈值，当使用 `count distinct` 时，小于该阈值的计数在普通表中被保证是精确的。

接受的值范围为 `500` 到 `15500`。超出此范围的值将被限制在该范围内。

当此选项设置为 0 时，启用一种算法来确保精确计数。该算法收集 `{group, value}` 对，将它们排序并定期消除重复项。结果是在普通表中获得精确计数。然而，这种方法不适用于高基数数据集，因为它的内存消耗很高，查询执行速度较慢。

当 `distinct_precision_threshold` 设置为大于 0 时，Manticore 使用另一种算法。它将计数加载到一个哈希表中并返回表的大小。如果哈希表变得过大，其内容将移入 `HyperLogLog` 数据结构。此时，计数变得近似，因为 HyperLogLog 是一种概率算法。此方法为每个组维持固定的最大内存使用量，但存在计数精度的折衷。

`HyperLogLog` 的精度和从哈希表转换为 `HyperLogLog` 的阈值来自于 `distinct_precision_threshold` 的设置。使用此选项时应谨慎，因为将其值加倍也会加倍计算计数所需的最大内存量。最大内存使用量可以通过以下公式粗略估算：`64 * max_matches * distinct_precision_threshold`，尽管在实际操作中，计数计算往往使用的内存比最坏情况更少。

### expand_keywords
`0` 或 `1`（默认值为 `0`）。在可能的情况下，扩展关键字为精确形式和/或星号。有关更多详细信息，请参阅 [expand_keywords](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expand_keywords)。

### field_weights
命名整数列表（用于排名的每字段用户权重）。

示例：
```sql
SELECT ... OPTION field_weights=(title=10, body=3)
```

### global_idf
使用来自 [global_idf](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#global_idf) 文件的全局统计数据（频率）来进行 IDF 计算。

### idf
带引号的逗号分隔的 IDF 计算标志列表。已知的标志包括：

- `normalized`：BM25 变体，idf = log((N-n+1)/n)，如 Robertson 等人所述
- `plain`：普通变体，idf = log(N/n)，如 Sparck-Jones 所述
- `tfidf_normalized`：进一步除以查询词数，以便 `TF*IDF` 适合 [0, 1] 范围
- `tfidf_unnormalized`：不再进一步除以查询词数，其中 N 为集合大小，n 为匹配文档的数量

在 Manticore 中历史上的默认 IDF（逆文档频率）等同于 `OPTION idf='normalized,tfidf_normalized'`，这些归一化可能会导致一些不想要的效果。

首先，`idf=normalized` 会导致关键词惩罚。例如，如果您搜索 `the | something` 且 `the` 出现在 50% 以上的文档中，则同时包含关键字 `the` 和 `something` 的文档将比仅包含关键字 `something` 的文档获得更少的权重。使用 `OPTION idf=plain` 可以避免这种情况。Plain IDF 的范围为 `[0, log(N)]`，关键词永远不会受到惩罚；而 normalized IDF 的范围为 `[-log(N), log(N)]`，过于频繁的关键词会受到惩罚。

其次，`idf=tfidf_normalized` 会导致 IDF 在不同查询中的漂移。历史上，IDF 还除以查询词数，确保所有关键词的 `sum(tf*idf)` 总和保持在 [0,1] 范围内。然而，这意味着像 `word1` 和 `word1 | nonmatchingword2` 这样的查询会为完全相同的结果集分配不同的权重，因为 `word1` 和 `nonmatchingword2` 的 IDF 都会被除以 2。使用 `OPTION idf='tfidf_unnormalized'` 可以解决此问题。请记住，当您禁用此归一化时，BM25、BM25A、BM25F() 排名因素将相应调整。

IDF 标志可以组合；`plain` 和 `normalized` 互斥；`tfidf_unnormalized` 和 `tfidf_normalized` 也互斥；在这些互斥组中未指定的标志将默认为其原始设置。这意味着 `OPTION idf=plain` 与指定 `OPTION idf='plain,tfidf_normalized'` 是一样的。

### index_weights
命名整数列表。用于排名的每表用户权重。

### local_df
`0` 或 `1`，自动汇总分布式表所有本地部分的 DF，以确保跨本地分片表的 IDF 一致性（和准确性）。默认为 RT 表的磁盘块启用。带有通配符的查询词将被忽略。

### low_priority
`0` 或 `1`（默认值为 `0`）。设置 `low_priority=1` 使查询以较低优先级执行，重新调度其作业的频率是正常优先级查询的十分之一。

### max_matches
整数。每次查询的最大匹配值。

该参数控制服务器为每个表保留在内存中的最大匹配项数，并可以返回给客户端。默认值是 1000。

`max_matches` 设置旨在控制和限制内存使用。它决定在搜索每个表时可以在内存中保留多少个匹配项。每个找到的匹配项都会被处理，但只有最好的 N 个匹配项会被保留在内存中，并最终返回给客户端。例如，假设一个表包含 2,000,000 个匹配项，通常您不需要检索所有这些匹配项。相反，您需要扫描所有的匹配项，但只根据某些标准（如相关性、价格或其他因素）选择最好的 500 个，并向最终用户显示这 500 个匹配项。通过只跟踪最好的 500 个匹配项，比起保留全部 2,000,000 个并对其进行排序然后丢弃不需要的部分，可以大大提高内存和 CPU 的效率。

该参数显著影响每次查询的内存和 CPU 使用情况。通常，1,000 到 10,000 的值是可接受的，但更高的限制应该谨慎使用。如果随意将 `max_matches` 增加到 1,000,000，那么 `searchd` 将不得不为每个查询分配并初始化一个 100 万条目的匹配缓冲区。这将不可避免地增加每个查询的内存使用量，在某些情况下也可能显著影响性能。

有关 `max_matches` 选项行为的其他信息，请参阅 [max_matches_increase_threshold](../Searching/Options.md#max_matches_increase_threshold)。

### max_matches_increase_threshold

整数。设置 `max_matches` 可以增加的阈值。默认值为 16384。

当 `pseudo_sharding` 启用时，如果 Manticore 检测到 groupby 属性的唯一值数量小于该阈值，可能会增加 `max_matches` 以提高 groupby 和/或聚合的准确性。当伪分片在多个线程中执行查询，或者 RT 表在磁盘块中执行并行搜索时，可能会出现精度损失。

如果 groupby 属性的唯一值数量小于该阈值，则 `max_matches` 将设置为此数值。否则，将使用默认的 `max_matches`。

如果查询选项中明确设置了 `max_matches`，则此阈值无效。

请注意，如果该阈值设置过高，将导致内存使用增加和总体性能下降。

您还可以通过使用 [accurate_aggregation](../Searching/Options.md#accurate_aggregation) 选项强制启用保证 groupby/聚合精度的模式。

### max_query_time
设置最大查询时间，以毫秒为单位。必须为非负整数。默认值为 0，表示“不限制”。一旦指定的时间已过，本地查询将被停止。请注意，如果您正在执行查询多个本地表的搜索，此限制将分别适用于每个表。由于不断跟踪是否停止查询的额外开销，可能会略微增加查询的响应时间。

### max_predicted_time
整数。设置最大预计搜索时间；详见 [predicted_time_costs](../Server_settings/Searchd.md#predicted_time_costs)。

### morphology
`none` 允许在表构建时启用了 [index_exact_words](../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words) 的情况下，将所有查询词替换为其精确形式。这有助于防止查询词的词干提取或词形还原。

### not_terms_only_allowed
<!-- example not_terms_only_allowed -->
`0` 或 `1` 允许单独使用[否定](../Searching/Full_text_matching/Operators.md#否定操作符)运算符。默认值为 0。另请参阅相关的[全局设置](../Server_settings/Searchd.md#not_terms_only_allowed)。

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
选择以下选项之一：
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

有关每个 ranker 的更多详细信息，请参阅[搜索结果排名](../Searching/Sorting_and_ranking.md#可用的内置排序器)。

### rand_seed
允许您为 `ORDER BY RAND()` 查询指定一个特定的整数种子值，例如：`... OPTION rand_seed=1234`。默认情况下，系统会为每次查询自动生成一个新的不同的种子值。

### retry_count
整数。分布式查询重试次数。

### retry_delay
整数。分布式查询重试延迟时间，单位为毫秒。

### sort_method
* `pq` - 优先队列，默认设置
* `kbuffer` - 提供更快的排序，适用于已经预排序的数据，例如按 id 排序的表数据

### threads
限制用于处理当前查询的最大线程数。默认值为无限制（查询可以占用所有全局定义的 [threads](../Server_settings/Searchd.md#threads)）。对于一批查询，此选项必须附加到该批次中的第一个查询，并在创建工作队列时应用，并在整个批次中有效。此选项与全局选项 [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query) 具有相同的含义，但仅适用于当前查询或查询批次。

### token_filter
带引号的、以冒号分隔的字符串，格式为 `库名:插件名:可选的设置字符串`。每次调用全文搜索时，为每个涉及的表创建一个查询时间令牌过滤器，允许您实现自定义的分词器，根据自定义规则生成令牌。
```sql
SELECT * FROM index WHERE MATCH ('yes@no') OPTION token_filter='mylib.so:blend:@'
```
### expansion_limit
限制单个通配符扩展的最大关键字数，默认值为 0 表示无限制。有关更多详细信息，请参阅 [expansion_limit](../Server_settings/Searchd.md#expansion_limit)。

## 查询优化器提示

<!-- example options_force -->

在极少数情况下，Manticore 内置的查询分析器可能会错误理解查询并确定是否应使用 docid 索引、二级索引或列扫描。要覆盖查询优化器的决策，您可以在查询中使用以下提示：

- `/*+ DocidIndex(id) */` 强制使用 docid 索引，`/*+ NO_DocidIndex(id) */` 告诉优化器忽略它
- `/*+ SecondaryIndex(<attr_name1>[, <attr_nameN>]) */` 强制使用二级索引（如果可用），`/*+ NO_SecondaryIndex(id) */` 告诉优化器忽略它
- `/*+ ColumnarScan(<attr_name1>[, <attr_nameN>]) */` 强制使用列扫描（如果该属性是列式的），`/*+ NO_ColumnarScan(id) */` 告诉优化器忽略它

注意，当执行带有过滤器的全文查询时，查询优化器在交叉匹配全文树的结果与过滤器结果之间进行选择，或者使用标准的“先匹配再过滤”的方法。指定*任何*提示将强制守护进程使用交叉匹配全文树结果与过滤器结果的代码路径。

有关查询优化器如何工作的更多信息，请参阅[基于成本的优化器](../Searching/Cost_based_optimizer.md)页面。

<!-- request SQL -->

```sql
SELECT * FROM students where age > 21 /*+ SecondaryIndex(age) */
```

<!-- end -->

<!-- example comments -->
使用 MySQL/MariaDB 客户端时，请确保包括 `--comments` 标志以启用查询中的提示。

<!-- request mysql -->
```bash
mysql -P9306 -h0 --comments
```
<!-- end -->

<!-- proofread -->
