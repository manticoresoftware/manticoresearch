# 混合搜索

混合搜索在一个查询中结合了[全文（BM25）搜索](../Searching/Full_text_matching/Basic_usage.md)与[KNN向量搜索](../Searching/KNN.md)，使用**倒数排名融合（RRF）**融合结果。这利用了两种检索方法的优势：BM25的关键词精确性与向量相似度的语义理解。

全文搜索擅长精确关键词匹配和稀有术语，但会遗漏概念上相似的内容。向量搜索捕捉语义含义，但在模糊查询中可能产生噪声。混合搜索结合两者，因此在**任一或两者**信号中得分高的文档会被展示。

## 倒数排名融合（RRF）

RRF是一种基于排名的融合算法。它基于**排名位置**而非原始分数进行操作，避免了需要归一化不兼容的分数尺度（BM25分数是无界的；KNN距离有不同的尺度）。

### 公式

```
RRF_score(d) = SUM over all result sets r:  weight_r / (rank_constant + rank_r(d))
```

其中：
- `d` 是一个文档
- `rank_r(d)` 是文档在结果集 `r` 中的1-based位置（按该检索器的分数排序）
- `rank_constant` 是一个平滑常数（默认：60，可通过 [`rank_constant`](../Searching/Options.md#rank_constant) 选项配置）
- `weight_r` 是每个检索器的可选权重（默认：1.0）

如果文档未出现在特定结果集中，该集合对该文档的贡献为0。

### rank_constant 参数

- `rank_constant=60` 是默认值。
- 较低的值（例如10）会放大排名靠前项目的差异。
- 较高的值（例如100）会使排名间的影响力更均匀分布。

## 基本混合查询

在 WHERE 子句中结合 `MATCH(...)` 和 `KNN(...)`，并使用 `OPTION fusion_method='rrf'`：

<!-- example hybrid_basic -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT id, hybrid_score()
FROM t
WHERE match('machine learning')
  AND knn(vec, (0.1, 0.1, 0.1, 0.1))
OPTION fusion_method='rrf';
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /search
{
  "table": "t",
  "knn": {
    "field": "vec",
    "query_vector": [0.1, 0.1, 0.1, 0.1]
  },
  "query": { "match": { "title": "machine learning" } },
  "options": { "fusion_method": "rrf" }
}
```

<!-- end -->

这会将文本搜索和 KNN 搜索作为独立的并行子查询运行，然后使用 RRF 融合结果。如果没有 `fusion_method='rrf'`，查询会作为常规 KNN 搜索运行，仅通过文本匹配进行过滤（预混合行为）。

在该预混合模式下，KNN 排名仍优先。如果 `knn_dist()` 可用且未显式按其排序，Manticore 会在排序顺序前添加 `knn_dist() ASC`。实际上，`ORDER BY weight() DESC` 成为次要的平局打破者，而非全局 BM25 排序。

### 可用函数

- [`hybrid_score()`](../Functions/Other_functions.md#HYBRID_SCORE%28%29) - RRF 融合分数（仅在混合查询中可用）
- [`weight()`](../Functions/Searching_and_ranking_functions.md#WEIGHT%28%29) - BM25 文本匹配分数
- [`knn_dist()`](../Functions/Other_functions.md#KNN_DIST%28%29) - 向量距离（如果有多个 KNN 子查询，则取最小值）

### 选项

| 选项 | 类型 | 默认值 | 描述 |
|--------|------|---------|-------------|
| [`fusion_method`](../Searching/Options.md#fusion_method) | 字符串 | *(无)* | 设置为 `'rrf'` 以启用混合搜索。必需。 |
| [`rank_constant`](../Searching/Options.md#rank_constant) | 整数 | 60 | RRF 公式中的平滑常数 |
| [`window_size`](../Searching/Options.md#window_size) | 整数 | 0（自动） | 每个子查询在融合前检索的结果数量。当为0时，从 KNN `k`（带过采样）和查询 `LIMIT` 自动计算 |
| [`fusion_weights`](../Searching/Options.md#fusion_weights) | 元组 | *(全部为1.0)* | RRF 评分中每个子查询的权重 |

## 调整 rank_constant

<!-- example hybrid_rank_constant -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
-- Default rank_constant=60 (gentler ranking)
SELECT id, hybrid_score() FROM t
WHERE match('machine learning') AND knn(vec, (0.1, 0.1, 0.1, 0.1))
OPTION fusion_method='rrf';

-- rank_constant=10 (sharper top-rank differences)
SELECT id, hybrid_score() FROM t
WHERE match('machine learning') AND knn(vec, (0.1, 0.1, 0.1, 0.1))
OPTION fusion_method='rrf', rank_constant=10;
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /search
{
  "table": "t",
  "knn": { "field": "vec", "query_vector": [0.1, 0.1, 0.1, 0.1] },
  "query": { "match": { "title": "machine learning" } },
  "options": { "fusion_method": "rrf", "rank_constant": 10 }
}
```

<!-- end -->

## 属性过滤

普通属性过滤器可与混合搜索一起使用，并应用于文本和 KNN 子查询。针对 `weight()` 及其依赖表达式的过滤器在融合之后应用，使用最终的文本权重。当前 Manticore 限制：这些权重过滤器不能用于包含 `OR` 的过滤树。

例如，按普通属性过滤：

<!-- example hybrid_filters -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT id, category, hybrid_score()
FROM t
WHERE match('machine learning')
  AND knn(vec, (0.1, 0.1, 0.1, 0.1))
  AND category = 1
OPTION fusion_method='rrf';
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /search
{
  "table": "t",
  "knn": { "field": "vec", "query_vector": [0.1, 0.1, 0.1, 0.1] },
  "query": {
    "bool": {
      "must": [
        { "match": { "title": "machine learning" } },
        { "equals": { "category": 1 } }
      ]
    }
  },
  "options": { "fusion_method": "rrf" }
}
```

<!-- end -->

## 排序

默认情况下，结果按 `hybrid_score() DESC` 排序。您可以覆盖此设置：

本节适用于真正的混合查询，即使用 `OPTION fusion_method='rrf'` 的查询。没有 `fusion_method='rrf'`，包含 `KNN(...)` 的查询不会被融合，仍保持 KNN 优先，因此 `ORDER BY weight() DESC` 不会产生全局按权重排序的结果集。

<!-- example hybrid_sorting -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
-- Sort by hybrid score ascending
SELECT id, hybrid_score() FROM t
WHERE match('machine learning') AND knn(vec, (0.1, 0.1, 0.1, 0.1))
ORDER BY hybrid_score() ASC
OPTION fusion_method='rrf';

-- Sort by text weight
SELECT id, weight() FROM t
WHERE match('machine learning') AND knn(vec, (0.1, 0.1, 0.1, 0.1))
ORDER BY weight() DESC, id ASC
OPTION fusion_method='rrf';

-- Sort by KNN distance
SELECT id, knn_dist() FROM t
WHERE match('machine learning') AND knn(vec, (0.1, 0.1, 0.1, 0.1))
ORDER BY knn_dist() ASC
OPTION fusion_method='rrf';

-- Sort the fused results by an attribute
SELECT id, price FROM t
WHERE match('machine learning') AND knn(vec, (0.1, 0.1, 0.1, 0.1))
ORDER BY price ASC
OPTION fusion_method='rrf';
```

<!-- end -->

请注意，子句顺序是 `WHERE ... ORDER BY ... LIMIT ... OPTION ...`：`OPTION fusion_method='rrf'` 应放在 `LIMIT` 之后。

## 分面

SQL 中的 [FACET](../Searching/Faceted_search.md) 和 JSON 中的 `aggs` 可以用于混合查询。分面计数覆盖所有融合后的文档，而不仅仅是 `LIMIT` 返回的行。这些文档与 [SHOW META](../Node_info_and_management/SHOW_META.md) 中 `total_found` 统计的文档相同：全文匹配结果与 KNN 候选项的并集。与结果一样，计数会遵循所有过滤条件，包括在融合之后应用的 `hybrid_score()`、`knn_dist()` 和 `weight()` 过滤条件。

<!-- example hybrid_facets -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT id, hybrid_score()
FROM t
WHERE match('learning')
  AND knn(vec, (0.1, 0.1, 0.1, 0.1))
LIMIT 3
OPTION fusion_method='rrf'
FACET category ORDER BY category ASC;
```

<!-- response SQL -->

```sql
+------+----------------+
| id   | hybrid_score() |
+------+----------------+
|    1 |     0.03278688 |
|    2 |     0.03200205 |
|    6 |     0.03125763 |
+------+----------------+
3 rows in set (0.00 sec)

+----------+----------+
| category | count(*) |
+----------+----------+
|        1 |        2 |
|        2 |        2 |
|        3 |        2 |
+----------+----------+
3 rows in set (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /search
{
  "table": "t",
  "knn": { "field": "vec", "query": [0.1, 0.1, 0.1, 0.1] },
  "query": { "match": { "*": "learning" } },
  "options": { "fusion_method": "rrf" },
  "_source": ["id"],
  "limit": 3,
  "aggs": {
    "categories": {
      "terms": { "field": "category", "size": 10 },
      "sort": [ { "category": { "order": "asc" } } ]
    }
  }
}
```

<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 6,
    "total_relation": "eq",
    "hits": [
      { "_id": 1, "_score": 1533, "_knn_dist": 0.0, "_hybrid_score": 0.03278688, "_source": {} },
      { "_id": 2, "_score": 1533, "_knn_dist": 0.01, "_hybrid_score": 0.03200205, "_source": {} },
      { "_id": 6, "_score": 1533, "_knn_dist": 1.95999992, "_hybrid_score": 0.03125763, "_source": {} }
    ]
  },
  "aggregations": {
    "categories": {
      "buckets": [
        { "key": 1, "doc_count": 2 },
        { "key": 2, "doc_count": 2 },
        { "key": 3, "doc_count": 2 }
      ]
    }
  }
}
```

<!-- end -->

如果只想统计融合得分较高的文档，可以按 `hybrid_score()` 过滤。在上面的查询中添加 `AND hs > 0.02`（并在选择列表中添加 `hybrid_score() hs`）后，只剩下文档 1、2 和 6，`FACET category` 返回 `1` → 2 和 `3` → 1。

[facet_filter_mode](../Searching/Faceted_search.md)、`FILTERS` 和 `EXCLUDE FILTERS` 的工作方式与普通查询相同。保留主查询过滤条件较少的分面，例如查询按 `category` 过滤时 `auto` 模式下的 `FACET category`，会统计使用该分面所保留的过滤条件运行混合查询后融合得到的文档。当混合查询带有 `GROUP BY` 时，分面统计的是文档，而不是分组。

使用与主查询相同过滤条件的分面会复用其融合后的文档，因此开销与普通查询中的分面大致相同。保留过滤条件较少的分面，以及带 `GROUP BY` 的查询中的任何分面，都会重新运行全文和 KNN 子查询。

## 无匹配文本

如果文本查询未匹配任何文档，仅 KNN 结果会对 RRF 分数做出贡献：

```sql
SELECT id, hybrid_score() FROM t
WHERE match('xyznonexistent') AND knn(vec, (0.1, 0.1, 0.1, 0.1))
OPTION fusion_method='rrf';
-- Returns results ranked purely by KNN rank
```

## 多个 KNN 搜索

单个混合查询可以将文本搜索与不同向量属性上的多个 KNN 搜索结合。所有内容通过 RRF 融合：

<!-- example hybrid_multi_knn -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
-- Three-way fusion: text + vec1 KNN + vec2 KNN
SELECT id, hybrid_score()
FROM t
WHERE match('machine learning')
  AND knn(vec1, (0.1, 0.1, 0.1, 0.1))
  AND knn(vec2, (1.0, 0.0, 0.0, 0.0))
OPTION fusion_method='rrf';

-- KNN-only fusion (no text), two vector searches
SELECT id, hybrid_score()
FROM t
WHERE knn(vec1, (0.1, 0.1, 0.1, 0.1))
  AND knn(vec2, (1.0, 0.0, 0.0, 0.0))
OPTION fusion_method='rrf';
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /search
{
  "table": "t",
  "knn": [
    { "field": "vec1", "query_vector": [0.1, 0.1, 0.1, 0.1] },
    { "field": "vec2", "query_vector": [1.0, 0.0, 0.0, 0.0] }
  ],
  "query": { "match": { "title": "machine learning" } },
  "options": { "fusion_method": "rrf" }
}
```

<!-- end -->

没有 `fusion_method` 的多个 KNN 搜索会产生错误。

## 加权融合（fusion_weights）

默认情况下，所有子查询贡献相等（权重1.0）。要为文本与 KNN 搜索赋予不同的重要性，请使用 [`fusion_weights`](../Searching/Options.md#fusion_weights) 并使用显式别名：

<!-- example hybrid_weights -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT id, hybrid_score()
FROM t
WHERE match('machine learning') AS text
  AND knn(vec1, (0.1, 0.1, 0.1, 0.1)) AS dense1
  AND knn(vec2, (1.0, 0.0, 0.0, 0.0)) AS dense2
OPTION fusion_method='rrf',
       fusion_weights=(text=0.7, dense1=0.2, dense2=0.1);
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /search
{
  "table": "t",
  "knn": [
    { "field": "vec1", "query_vector": [0.1, 0.1, 0.1, 0.1], "name": "dense1" },
    { "field": "vec2", "query_vector": [1.0, 0.0, 0.0, 0.0], "name": "dense2" }
  ],
  "query": { "match": { "title": "machine learning" } },
  "options": {
    "fusion_method": "rrf",
    "fusion_weights": { "query": 0.7, "dense1": 0.2, "dense2": 0.1 }
  }
}
```

<!-- end -->

### 别名规则

**SQL:**
- 使用 `AS alias` 对 `MATCH(...)` 和 `KNN(...)` 命名。没有隐式/默认别名。
- 省略的别名默认权重为1.0。
- 引用不存在的别名会产生错误。

**JSON:**
- `"query"` 是全文子查询的固定别名。
- 每个 KNN 条目通过 `"name"` 属性设置 KNN 别名。
- 命名为 `"query"` 的 KNN 条目与文本别名冲突并产生错误。
- `fusion_weights` 中不支持隐式别名（无显式 `"name"` 的字段名）。

### 部分权重

您可以仅为部分子查询指定权重；其余默认为1.0：

```sql
-- Only boost text, KNN searches default to weight 1.0
SELECT id, hybrid_score()
FROM t
WHERE match('machine learning') AS text
  AND knn(vec1, (0.1, 0.1, 0.1, 0.1)) AS dense1
  AND knn(vec2, (1.0, 0.0, 0.0, 0.0)) AS dense2
OPTION fusion_method='rrf', fusion_weights=(text=2.0);
```

## 简化语法：hybrid_match()

对于在 float_vector 属性上配置了[自动嵌入](../Searching/KNN.md#Auto-Embeddings-%28Recommended%29)的表，`hybrid_match()` 提供了一个简写，可自动从单个查询字符串运行文本和 KNN 搜索：

<!-- example hybrid_match -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
-- Explicit vector field
SELECT id, hybrid_score() FROM t WHERE hybrid_match('machine learning', vec);

-- Auto-detect vector field (requires exactly one auto-embedding attribute)
SELECT id, hybrid_score() FROM t WHERE hybrid_match('machine learning');

-- With custom k and rank_constant
SELECT id, hybrid_score() FROM t
WHERE hybrid_match('machine learning', vec, {k=3})
OPTION rank_constant=10;

-- With attribute filter
SELECT id, hybrid_score() FROM t
WHERE hybrid_match('machine learning', vec) AND category=1;
```

<!-- end -->

`hybrid_match()` 自动执行以下操作：
1. 将文本查询作为 BM25 全文搜索运行
2. 从同一文本字符串生成嵌入
3. 使用该嵌入运行 KNN 搜索
4. 通过 RRF 融合结果

**要求**：向量属性必须为[自动嵌入](../Searching/KNN.md#Auto-Embeddings-%28Recommended%29)配置`model_name`和`from`。如果没有配置，`hybrid_match()`将返回错误。

## 简化的JSON语法：hybrid

<!-- example hybrid_json -->

对于具有自动嵌入的表，JSON中可以使用`"hybrid"`属性作为简写：

<!-- intro -->
##### JSON：

<!-- request JSON -->

```json
POST /search
{
  "table": "hj",
  "hybrid": { "query": "machine learning" }
}

POST /search
{
  "table": "hj",
  "hybrid": { "query": "machine learning", "field": "vec" }
}

POST /search
{
  "table": "hj",
  "hybrid": { "query": "machine learning" },
  "options": { "rank_constant": 10 }
}
```

<!-- end -->

`"hybrid"`属性不能与`"knn"`一起使用。

## 混合查询中的自动嵌入KNN

当向量属性具有[自动嵌入](../Searching/KNN.md#Auto-Embeddings-%28Recommended%29)时，可以在`knn`对象中使用`"query"`（字符串）代替`"query_vector"`（数组）：

```json
POST /search
{
  "table": "ht",
  "knn": { "field": "vec", "query": "machine learning", "k": 5 },
  "query": { "match": { "title": "machine learning" } },
  "options": { "fusion_method": "rrf" }
}
```

字符串会在查询时自动嵌入。如果没有配置自动嵌入，这将返回错误。

## 执行架构

内部地，混合查询会被拆分为N+1个并行子查询：

1. 任务0：全文（BM25）子查询（如果文本查询为空则跳过，以避免用全扫描结果污染RRF）
2. 任务1..N：每个`knn(...)`条目对应一个KNN子查询

所有子查询并发运行。在全部完成之后，RRF融合：
1. 从每个子查询收集排序结果
2. 对于每个文档，累积其出现在每个子查询中的RRF得分贡献
3. 按融合后的RRF得分降序排序
4. 将[`knn_dist()`](../Functions/Other_functions.md#KNN_DIST%28%29)设置为每个文档在所有KNN子查询中的最小距离
5. 保留来自文本子查询的[`weight()`](../Functions/Searching_and_ranking_functions.md#WEIGHT%28%29)

如果查询包含[分面](../Searching/Hybrid_search.md#Facets)，之后每个分面都会作为普通分面查询在融合后的文档上运行。

<!-- proofread -->
