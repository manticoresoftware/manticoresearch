# 搜索选项

SQL [SELECT](../Searching/Full_text_matching/Basic_usage.md#SQL) 子句和 HTTP [/search](../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) 端点支持许多选项，可用于微调搜索行为。

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

### accurate_aggregation
整数。启用或禁用运行多线程分组查询时保证聚合准确性。默认值是0。

当运行分组查询时，如果 `pseudo_sharding` 开启，普通表可以以多个伪分片并行执行。RT表也采用类似方式执行。每个分片/块执行查询，但分组数量受限于 `max_matches`。如果不同分片/块的结果集有不同的分组，分组计数和聚合可能不准确。需要注意的是，Manticore 会根据分组属性的唯一值数量（从二级索引获取）尝试将 `max_matches` 增加到 [`max_matches_increase_threshold`](../Searching/Options.md#max_matches_increase_threshold)。如果成功，准确性不会丢失。

然而，如果分组属性的唯一值数量较多，进一步增加 `max_matches` 可能不是好策略，因为这会导致性能下降和更高的内存使用。将 `accurate_aggregation` 设置为1会强制分组搜索在单线程下运行，从而解决准确性问题。需要注意的是，仅当无法将 `max_matches` 调高时，才强制单线程；否则，设置了 `accurate_aggregation=1` 的搜索仍将在多线程中运行。

总体来说，设置 `accurate_aggregation` 为1确保了RT表和开启了 `pseudo_sharding=1` 的普通表的分组计数和聚合准确性。缺点是搜索速度变慢，因为它被强制在单线程模式下运行。

但是，如果我们有一个RT表和一个包含相同数据的普通表并运行带有 `accurate_aggregation=1` 的查询，结果仍可能不同。这是因为守护进程可能根据 [`max_matches_increase_threshold`](../Searching/Options.md#max_matches_increase_threshold) 设置，为RT表和普通表选择不同的 `max_matches` 配置。

### agent_query_timeout
整数。等待远程查询完成的最大毫秒数，详见[此节](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout)。

### boolean_simplify
`0` 或 `1`（默认为 `1`）。`boolean_simplify=1` 启用[简化查询](../Searching/Full_text_matching/Boolean_optimization.md)以加快速度。

此选项也可在 [searchd 配置](../Server_settings/Searchd.md#boolean_simplify)中全局设置，以改变所有查询的默认行为。每个查询指定的选项会覆盖全局设置。

### comment
字符串，用户注释，会被写入查询日志文件。

### cutoff
整数。指定要处理的最大匹配数。如未设置，Manticore 会自动选择合适的值。

<!-- example cutoff_aggregation -->

* `N = 0`：禁用匹配数限制。
* `N > 0`：指示 Manticore 在找到 `N` 个匹配文档后停止处理结果。
* 未设置：Manticore 自动决定阈值。

当 Manticore 无法确定匹配文档的确切计数时，查询 [元信息](../Node_info_and_management/SHOW_META.md#SHOW-META)中的 `total_relation` 字段将显示为 `gte`，表示 **大于或等于**。这意味着实际匹配计数至少等于报告的 `total_found`（SQL）或 `hits.total`（JSON）。计数精确时，`total_relation` 会显示 `eq`。

注意：不推荐在聚合查询中使用 `cutoff`，因为可能会产生不准确或不完整的结果。

<!-- request Example -->

在聚合查询中使用 `cutoff` 可能导致错误或误导性的结果，如下面示例所示：
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

与不带 `cutoff` 的相同查询相比：
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
整数。默认值为 `3500`。此选项设定了在普通表中保证 `count distinct` 返回计数精确的阈值。

接受的值范围是 `500` 到 `15500`。超出此范围的值将被限制在该区间内。

当此选项设置为0时，启用一种确保计数精确的算法。该算法收集 `{group, value}` 对，排序并定期去重。结果是在普通表内获得精确计数。然而，由于其高内存消耗和查询速度慢，不适合高基数数据集。

当 `distinct_precision_threshold` 设置为大于0的值时，Manticore 采用另一种算法。它先将计数加载到哈希表中，并返回哈希表大小。如果哈希表过大，会将其内容转存到 `HyperLogLog` 数据结构中。此时计数变为近似，因为 HyperLogLog 是概率算法。这种方法保持了每个分组的最大固定内存使用，但计数准确性有所折中。

`HyperLogLog` 的准确性及从哈希表转换到 HyperLogLog 的阈值均由 `distinct_precision_threshold` 设置决定。使用此选项时需谨慎，因为将其值加倍也会使计算计数的最大内存需求加倍。最大内存使用大致可根据公式估算：`64 * max_matches * distinct_precision_threshold`，不过实际计数计算通常使用的内存少于最坏情况。

### expand_keywords
`0` 或 `1`（默认为 `0`）。在可能时，展开具有精确形式和/或通配符的关键词。详情请参阅 [expand_keywords](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expand_keywords)。

### field_weights
命名的整数列表（每字段用户排序权重）。

示例：
```sql
SELECT ... OPTION field_weights=(title=10, body=3)
```

### global_idf
使用来自 [global_idf](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#global_idf) 文件的全局统计（频率）进行 IDF 计算。

### idf
带引号的逗号分隔的 IDF 计算标志列表。已知标志如下：

* `normalized`：BM25 变体，idf = log((N-n+1)/n)，根据 Robertson 等人的定义
* `plain`：普通变体，idf = log(N/n)，根据 Sparck-Jones 的定义
* `tfidf_normalized`：额外将 IDF 除以查询词数，使得 `TF*IDF` 归一到 [0, 1] 范围内
* `tfidf_unnormalized`：不额外将 IDF 除以查询词数，其中 N 是集合大小，n 是匹配文档数

Manticore 历史上默认的 IDF（逆文档频率）等同于 `OPTION idf='normalized,tfidf_normalized'`，这些归一化可能导致若干不理想的效果。

首先，`idf=normalized` 会导致关键词惩罚。例如，如果您搜索 `the | something`，且 `the` 出现在超过 50% 的文档中，那么同时包含关键词 `the` 和 `something` 的文档相较于仅包含关键词 `something` 的文档将获得较低的权重。使用 `OPTION idf=plain` 可以避免此问题。普通 IDF 的值范围在 `[0, log(N)]`，关键词不会被惩罚；而归一化 IDF 的范围在 `[-log(N), log(N)]`，出现频率过高的关键词会被惩罚。

其次，`idf=tfidf_normalized` 会导致跨查询的 IDF 漂移。历史上，IDF 也会除以查询关键词数，确保所有关键词的 `sum(tf*idf)` 保持在 [0,1] 范围内。但是这意味着诸如 `word1` 和 `word1 | nonmatchingword2` 这样的查询会给相同的结果集分配不同权重，因为 `word1` 和 `nonmatchingword2` 的 IDF 都会被除以 2。使用 `OPTION idf='tfidf_unnormalized'` 可以解决该问题。需注意，当您禁用此归一化时，BM25、BM25A、BM25F() 等排名因子也会相应调整。

IDF 标志可以组合使用；`plain` 和 `normalized` 互斥；`tfidf_unnormalized` 和 `tfidf_normalized` 也互斥；未指定的互斥组内标志默认使用其原始设置。这意味着 `OPTION idf=plain` 等同于完整指定 `OPTION idf='plain,tfidf_normalized'`。

### jieba_mode
指定查询的 Jieba 分词模式。

使用 Jieba 中文分词时，有时对文档分词和查询分词使用不同模式会有所帮助。完整模式列表请参阅 [jieba_mode](../Creating_a_table/NLP_and_tokenization/Morphology.md#jieba_mode)。

### index_weights
命名的整数列表。每表的用户排序权重。

### local_df
`0` 或 `1`，自动汇总分布式表所有本地分片的 DF，确保在本地分片的分表中 IDF 一致且准确。默认对 RT 表的磁盘分片启用。带通配符的查询词将被忽略。

### low_priority
`0` 或 `1`（默认为 `0`）。设置 `low_priority=1` 将以较低优先级执行查询，其任务调度频率比普通优先级查询低 10 倍。

### max_matches
整数。每查询的最大匹配数。

服务器为每个表保留在 RAM 中的最大匹配数，并可返回给客户端。默认值为 1000。

此参数用于控制并限制内存使用，`max_matches` 设置决定搜索每个表时保留在内存中的匹配数量。每个找到的匹配仍会被处理，但最终只保留内存中的前 N 个最佳匹配并返回给客户端。例如，假设表中有 2,000,000 个匹配。通常您不需要获取全部，只需扫描全部匹配，再根据某些条件（例如按相关性、价格或其他因素排序）筛选出“最佳”的 500 个，然后分页显示这 500 个结果（每页 20 到 100 个匹配）。跟踪仅这 500 个最佳匹配在内存和 CPU 方面比保存全部 2,000,000 个匹配、排序并仅展现前 20 个更高效。`max_matches` 控制了这个“最佳 N”的数量。

此参数显著影响查询的内存和 CPU 使用率。一般 1,000 到 10,000 的值可接受，但应谨慎使用更高的限制。随意将 `max_matches` 增加到 1,000,000 会使 `searchd` 必须为每个查询分配和初始化一个具有 100 万条目的匹配缓冲区，势必增加每查询的内存使用，有时会明显影响性能。

有关该参数如何影响 `max_matches` 选项行为的更多信息，请参阅 [max_matches_increase_threshold](../Searching/Options.md#max_matches_increase_threshold)。

### max_matches_increase_threshold

整数。设置 `max_matches` 可增加的阈值。默认值为 16384。

当启用 `pseudo_sharding` 并检测到 groupby 属性的唯一值数量小于此阈值时，Manticore 可能会增加 `max_matches` 以提高 groupby 和/或聚合准确度。在伪分片多线程执行查询或 RT 表对磁盘分片进行并行搜索时可能出现准确度损失。

如果 groupby 属性的唯一值数量小于该阈值，`max_matches` 将设置为该数。否则，使用默认的 `max_matches`。

如果查询选项中显式设置了 `max_matches`，该阈值无效。

请注意，如果该阈值设置得过高，将导致内存消耗增加和整体性能下降。

你也可以使用 [accurate_aggregation](../Searching/Options.md#accurate_aggregation) 选项强制保证 groupby/aggregate 的准确模式。

### max_query_time
设置最大搜索查询时间，单位为毫秒。必须是非负整数。默认值为 0，表示“不限制”。本地搜索查询将在达到指定时间后停止。请注意，如果你执行的是查询多个本地表的搜索，此限制分别适用于每个表。请注意，这可能会略微增加查询的响应时间，因为要不断跟踪是否应停止查询所带来的开销。

### max_predicted_time
整数。最大预测搜索时间；详见 [predicted_time_costs](../Server_settings/Searchd.md#predicted_time_costs)。

### morphology
`none` 允许在构建表时启用了 [index_exact_words](../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words) 时，将所有查询词替换为其精确形式。这对于防止对查询词进行词干提取或词形还原很有用。

### not_terms_only_allowed
<!-- example not_terms_only_allowed -->
`0` 或 `1` 允许查询中单独的 [否定](../Searching/Full_text_matching/Operators.md#Negation-operator)。默认是 0。另见对应的 [全局设置](../Server_settings/Searchd.md#not_terms_only_allowed)。

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
<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "select * from tbl where match('-d');"
{
  "error": "table t: query error: query is non-computable (single NOT operator)"
}

POST /sql?mode=raw -d "select * from t where match('-d')  option not_terms_only_allowed=1;"
[
  {
    "columns": [
      {
        "id": {
          "type": "long long"
        }
      },
      {
        "f1": {
          "type": "string"
        }
      },
      {
        "f2": {
          "type": "long"
        }
      }
    ],
    "data": [
      {
        "id": 724024784404348900,
        "f1": "b",
        "f2": 2
      },
      {
        "id": 724024784404348900,
        "f1": "c",
        "f2": 3
      },
      {
        "id": 724024784404348900,
        "f1": "b",
        "f2": 2
      }
    ],
    "total": 3,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

### ranker
可选择以下排名器：
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

关于每个排名器的更多细节，请参阅 [搜索结果排名](../Searching/Sorting_and_ranking.md#Available-built-in-rankers)。

### rand_seed
允许你为 `ORDER BY RAND()` 查询指定一个特定的整数种子值，例如：`... OPTION rand_seed=1234`。默认情况下，每个查询都会自动生成一个全新且不同的种子值。

### retry_count
整数。分布式重试次数。

### retry_delay
整数。分布式重试延迟，单位为毫秒。

### scroll

字符串。用于分页的滚动令牌，使用 [Scroll pagination approach](../Searching/Pagination.md#Scroll-Search-Option)。

### sort_method
* `pq` - 优先队列，默认设置
* `kbuffer` - 针对已预排序数据提供更快排序，例如按 id 排序的表数据
两者产生的结果集相同；选择任一选项只是可能改善（或恶化）性能。

### threads
限制当前查询处理使用的最大线程数。默认 - 不限制（查询可占用全局定义的所有 [线程](../Server_settings/Searchd.md#threads)）。
对于一批查询，该选项必须附加在该批的第一个查询上，然后在创建工作队列时应用，并对整个批次有效。该选项与 [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query) 选项含义相同，但仅适用于当前查询或查询批次。

### token_filter
带引号的、用冒号分隔的字符串格式为 `library name:plugin name:optional string of settings`。当全文检索由涉及的每个表调用时，将为每次搜索创建一个查询时分词器过滤器，允许你实现根据自定义规则生成标记的自定义分词器。
```sql
SELECT * FROM index WHERE MATCH ('yes@no') OPTION token_filter='mylib.so:blend:@'
```
### expansion_limit
限制单个通配符的最大扩展关键词数量，默认值为 0 表示不限制。更多详情请参阅 [expansion_limit](../Server_settings/Searchd.md#expansion_limit)。

## 查询优化器提示

<!-- example options_force -->

在极少数情况下，Manticore 内置的查询分析器可能无法正确理解查询，也无法准确决定是否应使用 docid 索引、二级索引或列扫描。要覆盖查询优化器的决策，可在查询中使用以下提示：

* `/*+ DocidIndex(id) */` 强制使用 docid 索引，`/*+ NO_DocidIndex(id) */` 告诉优化器忽略该索引
* `/*+ SecondaryIndex(<attr_name1>[, <attr_nameN>]) */` 强制使用二级索引（如果可用），`/*+ NO_SecondaryIndex(id) */` 告诉优化器忽略该索引
* `/*+ ColumnarScan(<attr_name1>[, <attr_nameN>]) */` 强制使用列扫描（如果属性是列式），`/*+ NO_ColumnarScan(id) */` 告诉优化器忽略该扫描

注意，当在带过滤器的全文查询中执行时，查询优化器会在全文树结果与过滤结果相交与使用标准的先匹配然后过滤方法之间做出选择。指定*任何*提示都会强制守护进程使用执行全文树结果与过滤结果相交的代码路径。

关于查询优化器的工作原理的更多信息，请参阅 [基于成本的优化器](../Searching/Cost_based_optimizer.md) 页面。

<!-- request SQL -->

```sql
SELECT * FROM students where age > 21 /*+ SecondaryIndex(age) */
```

<!-- end -->
<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "SELECT * FROM students where age > 21 /*+ SecondaryIndex(age) */"
```


<!-- example comments -->
使用 MySQL/MariaDB 客户端时，请确保包含 `--comments` 参数，以启用查询中的提示。

<!-- request mysql -->
```bash
mysql -P9306 -h0 --comments
```
<!-- end -->

<!-- proofread -->

