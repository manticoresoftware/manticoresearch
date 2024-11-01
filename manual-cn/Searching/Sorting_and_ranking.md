# 排序和排名

查询结果可以按全文排名权重、一个或多个属性或表达式进行排序。

**全文**查询默认返回按排序后的匹配项。如果未指定任何内容，结果将按相关性排序，这相当于 SQL 格式中的 `ORDER BY weight() DESC`。

**非全文**查询默认不进行任何排序。

## 高级排序

当您明确提供排序规则时，扩展模式会自动启用。可以通过在 SQL 格式中添加 `ORDER BY` 子句，或通过 HTTP JSON 使用 `sort` 选项。

### 通过 SQL 进行排序

通用语法：
```sql
SELECT ... ORDER BY
{attribute_name | expr_alias | weight() | random() } [ASC | DESC],
...
{attribute_name | expr_alias | weight() | random() } [ASC | DESC]
```

<!-- example alias -->

在排序子句中，您最多可以使用 5 列的任意组合，每列后面跟随 `asc` 或 `desc`。排序子句不允许函数和表达式作为参数，除了 `weight()` 和 `random()` 函数（后者仅能在 SQL 中以 `ORDER BY random()` 的形式使用）。但是，您可以在 SELECT 列表中使用任何表达式，并按其别名进行排序。

<!-- request SQL -->
```sql
select *, a + b alias from test order by alias desc;
```

<!-- response SQL -->
```
+------+------+------+----------+-------+
| id   | a    | b    | f        | alias |
+------+------+------+----------+-------+
|    1 |    2 |    3 | document |     5 |
+------+------+------+----------+-------+
```

<!-- end -->

## 通过 JSON 进行排序

<!-- example sorting 1 -->
`"sort"` 指定一个数组，其中每个元素可以是属性名称，或者是 `_score`，如果您希望按匹配权重进行排序。在这种情况下，属性的排序顺序默认为升序，而 `_score` 的排序顺序默认为降序。

<!-- intro -->

<!-- request JSON -->

```json
{
  "index":"test",
  "query":
  {
    "match": { "title": "Test document" }
  },
  "sort": [ "_score", "id" ],
  "_source": "title",
  "limit": 3
}
```

<!-- response JSON -->

``` json
    {
	  "took": 0,
	  "timed_out": false,
	  "hits": {
	    "total": 5,
	    "total_relation": "eq",
	    "hits": [
	      {
	        "_id": 5406864699109146628,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 1"
	        }
	      },
	      {
	        "_id": 5406864699109146629,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 2"
	        }
	      },
	      {
	        "_id": 5406864699109146630,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 3"
	        }
	      }
	    ]
	  }
	}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$search->setIndex("test")->match('Test document')->sort('_score')->sort('id');
```

<!-- intro -->

##### Python:

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
search_request.sort = ['_score', 'id']
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Test document');
searchRequest.sort = ['_score', 'id'];
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
searchRequest.setIndex("test");
QueryFilter queryFilter = new QueryFilter();
queryFilter.setQueryString("Test document");
searchRequest.setFulltextFilter(queryFilter);
List<Object> sort = new ArrayList<Object>( Arrays.asList("_score", "id") );
searchRequest.setSort(sort);

```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
var searchRequest = new SearchRequest("test");
searchRequest.FulltextFilter = new QueryFilter("Test document");
searchRequest.Sort = new List<Object> {"_score", "id"};

```

<!-- intro -->

##### Typescript:

<!-- request typescript -->
``` typescript
searchRequest = {
  index: 'test',
  query: {
    query_string: {'Test document'},
  },
  sort: ['_score', 'id'],
}
```

<!-- intro -->

##### Go:

<!-- request go -->
```go
searchRequest.SetIndex("test")
query := map[string]interface{} {"query_string": "Test document"}
searchRequest.SetQuery(query)
sort := map[string]interface{} {"_score": "asc", "id": "asc"}
searchRequest.SetSort(sort)
```

<!-- end -->

<!-- example sorting 2 -->

您还可以显式指定排序顺序：

- `asc`：升序排序
- `desc`：降序排序

<!-- intro -->

<!-- request JSON -->

```json
{
  "index":"test",
  "query":
  {
    "match": { "title": "Test document" }
  },
  "sort":
  [
    { "id": "desc" },
    "_score"
  ],
  "_source": "title",
  "limit": 3
}
```

<!-- response JSON -->

``` json
	{
	  "took": 0,
	  "timed_out": false,
	  "hits": {
	    "total": 5,
	    "total_relation": "eq",
	    "hits": [
	      {
	        "_id": 5406864699109146632,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 5"
	        }
	      },
	      {
	        "_id": 5406864699109146631,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 4"
	        }
	      },
	      {
	        "_id": 5406864699109146630,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 3"
	        }
	      }
	    ]
	  }
	}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$search->setIndex("test")->match('Test document')->sort('id', 'desc')->sort('_score');
```

<!-- intro -->

##### Python:

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
sort_by_id = manticoresearch.model.SortOrder('id', 'desc')
search_request.sort = [sort_by_id, '_score']
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Test document');
sortById = new Manticoresearch.SortOrder('id', 'desc');
searchRequest.sort = [sortById, 'id'];
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
searchRequest.setIndex("test");
QueryFilter queryFilter = new QueryFilter();
queryFilter.setQueryString("Test document");
searchRequest.setFulltextFilter(queryFilter);
List<Object> sort = new ArrayList<Object>();
SortOrder sortById = new SortOrder();
sortById.setAttr("id");
sortById.setOrder(SortOrder.OrderEnum.DESC);
sort.add(sortById);
sort.add("_score");
searchRequest.setSort(sort);

```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
var searchRequest = new SearchRequest("test");
searchRequest.FulltextFilter = new QueryFilter("Test document");
searchRequest.Sort = new List<Object>();
var sortById = new SortOrder("id", SortOrder.OrderEnum.Desc);
searchRequest.Sort.Add(sortById);
searchRequest.Sort.Add("_score");

```

<!-- intro -->

##### Typescript:

<!-- request typescript -->
``` typescript
searchRequest = {
  index: 'test',
  query: {
    query_string: {'Test document'},
  },
  sort: [{'id': 'desc'}, '_score'],
}
```

<!-- intro -->

##### Go:

<!-- request go -->
```go
searchRequest.SetIndex("test")
query := map[string]interface{} {"query_string": "Test document"}
searchRequest.SetQuery(query)
sortById := map[string]interface{} {"id": "desc"}
sort := map[string]interface{} {"id": "desc", "_score": "asc"}
searchRequest.SetSort(sort)
```

<!-- end -->

<!-- example sorting 3 -->
您还可以使用另一种语法，通过 `order` 属性指定排序顺序：


<!-- intro -->

<!-- request JSON -->

```json
{
  "index":"test",
  "query":
  {
    "match": { "title": "Test document" }
  },
  "sort":
  [
    { "id": { "order":"desc" } }
  ],
  "_source": "title",
  "limit": 3
}
```

<!-- response JSON -->

``` json
	{
	  "took": 0,
	  "timed_out": false,
	  "hits": {
	    "total": 5,
	    "total_relation": "eq",
	    "hits": [
	      {
	        "_id": 5406864699109146632,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 5"
	        }
	      },
	      {
	        "_id": 5406864699109146631,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 4"
	        }
	      },
	      {
	        "_id": 5406864699109146630,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 3"
	        }
	      }
	    ]
	  }
	}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$search->setIndex("test")->match('Test document')->sort('id', 'desc');
```

<!-- intro -->

##### Python:

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
sort_by_id = manticoresearch.model.SortOrder('id', 'desc')
search_request.sort = [sort_by_id]
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Test document');
sortById = new Manticoresearch.SortOrder('id', 'desc');
searchRequest.sort = [sortById];
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
searchRequest.setIndex("test");
QueryFilter queryFilter = new QueryFilter();
queryFilter.setQueryString("Test document");
searchRequest.setFulltextFilter(queryFilter);
List<Object> sort = new ArrayList<Object>();
SortOrder sortById = new SortOrder();
sortById.setAttr("id");
sortById.setOrder(SortOrder.OrderEnum.DESC);
sort.add(sortById);
searchRequest.setSort(sort);

```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
var searchRequest = new SearchRequest("test");
searchRequest.FulltextFilter = new QueryFilter("Test document");
searchRequest.Sort = new List<Object>();
var sortById = new SortOrder("id", SortOrder.OrderEnum.Desc);
searchRequest.Sort.Add(sortById);

```

<!-- intro -->

##### Typescript:

<!-- request typescript -->
``` typescript
searchRequest = {
  index: 'test',
  query: {
    query_string: {'Test document'},
  },
  sort: { {'id': {'order':'desc'} },
}
```

<!-- intro -->

##### Go:

<!-- request go -->
```go
searchRequest.SetIndex("test")
query := map[string]interface{} {"query_string": "Test document"}
searchRequest.SetQuery(query)
sort := map[string]interface{} { "id": {"order":"desc"} }
searchRequest.SetSort(sort)
```

<!-- end -->

<!-- example sorting 4 -->

在 JSON 查询中也支持按 MVA（多值属性）排序。可以通过 `mode` 属性设置排序模式，支持以下模式：

- `min`：按最小值排序
- `max`：按最大值排序

<!-- intro -->

<!-- request JSON -->

```json
{
  "index":"test",
  "query":
  {
    "match": { "title": "Test document" }
  },
  "sort":
  [
    { "attr_mva": { "order":"desc", "mode":"max" } }
  ],
  "_source": "title",
  "limit": 3
}
```

<!-- response JSON -->

``` json
	{
	  "took": 0,
	  "timed_out": false,
	  "hits": {
	    "total": 5,
	    "total_relation": "eq",
	    "hits": [
	      {
	        "_id": 5406864699109146631,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 4"
	        }
	      },
	      {
	        "_id": 5406864699109146629,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 2"
	        }
	      },
	      {
	        "_id": 5406864699109146628,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 1"
	        }
	      }
	    ]
	  }
	}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$search->setIndex("test")->match('Test document')->sort('id','desc','max');
```

<!-- intro -->

##### Python:

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
sort = manticoresearch.model.SortMVA('attr_mva', 'desc', 'max')
search_request.sort = [sort]
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Test document');
sort = new Manticoresearch.SortMVA('attr_mva', 'desc', 'max');
searchRequest.sort = [sort];
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
searchRequest.setIndex("test");
QueryFilter queryFilter = new QueryFilter();
queryFilter.setQueryString("Test document");
searchRequest.setFulltextFilter(queryFilter);
SortMVA sort = new SortMVA();
sort.setAttr("attr_mva");
sort.setOrder(SortMVA.OrderEnum.DESC);
sort.setMode(SortMVA.ModeEnum.MAX);
searchRequest.setSort(sort);

```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
var searchRequest = new SearchRequest("test");
searchRequest.FulltextFilter = new QueryFilter("Test document");
var sort = new SortMVA("attr_mva", SortMVA.OrderEnum.Desc, SortMVA.ModeEnum.Max);
searchRequest.Sort.Add(sort);
```

<!-- intro -->

##### Typescript:

<!-- request typescript -->
``` typescript
searchRequest = {
  index: 'test',
  query: {
    query_string: {'Test document'},
  },
  sort: { "attr_mva": { "order":"desc", "mode":"max" } },
}
```

<!-- intro -->

##### Go:

<!-- request go -->
```go
searchRequest.SetIndex("test")
query := map[string]interface{} {"query_string": "Test document"}
searchRequest.SetQuery(query)
sort := map[string]interface{} { "attr_mva": { "order":"desc", "mode":"max" } }
searchRequest.SetSort(sort)
```

<!-- end -->

<!-- example sorting 5 -->
当按属性排序时，默认情况下会禁用匹配权重（评分）计算（不使用排序器）。您可以通过将 `track_scores` 属性设置为 `true` 来启用权重计算：

<!-- intro -->

<!-- request JSON -->

```json
{
  "index":"test",
  "track_scores": true,
  "query":
  {
    "match": { "title": "Test document" }
  },
  "sort":
  [
    { "attr_mva": { "order":"desc", "mode":"max" } }
  ],
  "_source": "title",
  "limit": 3
}
```

<!-- response JSON -->

``` json
	{
	  "took": 0,
	  "timed_out": false,
	  "hits": {
	    "total": 5,
	    "total_relation": "eq",
	    "hits": [
	      {
	        "_id": 5406864699109146631,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 4"
	        }
	      },
	      {
	        "_id": 5406864699109146629,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 2"
	        }
	      },
	      {
	        "_id": 5406864699109146628,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 1"
	        }
	      }
	    ]
	  }
	}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$search->setIndex("test")->match('Test document')->sort('id','desc','max')->trackScores(true);
```

<!-- intro -->

##### Python:

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.track_scores = true
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
sort = manticoresearch.model.SortMVA('attr_mva', 'desc', 'max')
search_request.sort = [sort]
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.trackScores = true;
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Test document');
sort = new Manticoresearch.SortMVA('attr_mva', 'desc', 'max');
searchRequest.sort = [sort];
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
searchRequest.setIndex("test");
searchRequest.setTrackScores(true);
QueryFilter queryFilter = new QueryFilter();
queryFilter.setQueryString("Test document");
searchRequest.setFulltextFilter(queryFilter);
SortMVA sort = new SortMVA();
sort.setAttr("attr_mva");
sort.setOrder(SortMVA.OrderEnum.DESC);
sort.setMode(SortMVA.ModeEnum.MAX);
searchRequest.setSort(sort);

```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
var searchRequest = new SearchRequest("test");
searchRequest.SetTrackScores(true);
searchRequest.FulltextFilter = new QueryFilter("Test document");
var sort = new SortMVA("attr_mva", SortMVA.OrderEnum.Desc, SortMVA.ModeEnum.Max);
searchRequest.Sort.Add(sort);
```

<!-- intro -->

##### Typescript:

<!-- request typescript -->
``` typescript
searchRequest = {
  index: 'test',
  track_scores: true,
  query: {
    query_string: {'Test document'},
  },
  sort: { "attr_mva": { "order":"desc", "mode":"max" } },
}
```

<!-- intro -->

##### Go:

<!-- request go -->
```go
searchRequest.SetIndex("test")
searchRequest.SetTrackScores(true)
query := map[string]interface{} {"query_string": "Test document"}
searchRequest.SetQuery(query)
sort := map[string]interface{} { "attr_mva": { "order":"desc", "mode":"max" } }
searchRequest.SetSort(sort)
```

<!-- end -->


## 排名概述

排名（也称为加权）是指计算每个与查询匹配的文档相对于该查询的相关性（权重）。相关性是一个附加到每个文档的数字，用于估算文档与查询的匹配程度。然后，搜索结果可以基于此数字以及其他参数进行排序，以便最相关的结果在结果页面中排名靠前。

没有一种适用于所有场景的标准化排名方式，且不可能有，因为相关性是**主观的**。对某人来说相关的内容，可能对另一个人来说并不相关。因此，通常情况下，计算相关性不仅困难，而且理论上是不可能的。

因此，在 Manticore 中，排名是可配置的。它引入了所谓的**排序器（ranker）**。排序器可以正式定义为一个函数，接受文档和查询作为输入，输出相关性值。简单来说，排序器控制 Manticore 如何使用特定算法为文档分配权重。

## 可用的内置排序器

Manticore 提供了几种内置排序器，适用于不同的目的。许多排序器使用两个因素：短语接近度（也称为 LCS）和 BM25。短语接近度基于关键词的位置，而 BM25 则基于关键词的频率。短语接近度越高，文档和查询的短语匹配度越好；BM25 越高，文档包含的稀有词汇越多。

当前实现的排序器有：

- **`proximity_bm25`**：默认排序模式，结合了短语接近度和 BM25 排序。
- **`bm25`**：仅使用 BM25 排序的统计模式（类似于大多数全文引擎）。这种模式更快，但对于包含多个关键词的查询，质量可能较差。
- **`none`**：无排序模式。此模式是最快的，所有匹配项的权重为 1。通常称为布尔搜索，仅匹配文档而不对其进行排名。
- **`wordcount`**：按关键词出现次数进行排序。该排序器计算每个字段的关键词出现次数，然后乘以字段权重，并将结果值相加。
- **`proximity`**：返回原始短语接近度值。此模式用于模拟 `SPH_MATCH_ALL` 查询。
- **`matchany`**：返回在 `SPH_MATCH_ANY` 模式下计算的排名，内部用于模拟 `SPH_MATCH_ANY` 查询。
- **`fieldmask`**：返回 32 位掩码，其中第 N 位对应于第 N 个全文字段（从 0 开始编号）。当相应字段有任何满足查询的关键词出现时，该位会被设置。
- **`sph04`**：基于 `proximity_bm25` 排序器，另外当匹配项出现在文本字段的开头或结尾时提升权重。因此，当字段等于查询时，它的排名应高于仅包含查询的字段。
- **`expr`**：允许在运行时指定排名公式。它公开了几个内部文本因素，并允许您定义如何从这些因素计算最终权重。详情可参阅[相关部分](../Searching/Sorting_and_ranking.md#Quick-summary-of-the-ranking-factors)。

排序器名称不区分大小写。示例：

```sql
SELECT ... OPTION ranker=sph04;
```

## 排名因素快速概述

| 名称                              | 级别     | 类型  | 概述                                                         |
| --------------------------------- | -------- | ----- | ------------------------------------------------------------ |
| max_lcs                           | 查询级别 | int   | 当前查询的最大可能 LCS 值                                    |
| bm25                              | 文档级别 | int   | BM25(1.2, 0) 的快速估算                                      |
| bm25a(k1, b)                      | 文档级别 | int   | 使用可配置的 K1、B 常量的 BM25() 精确值                      |
| bm25f(k1, b, {field=weight, ...}) | 文档级别 | int   | 使用额外可配置字段权重的 BM25F() 精确值                      |
| field_mask                        | 文档级别 | int   | 匹配字段的位掩码                                             |
| query_word_count                  | 文档级别 | int   | 查询中唯一包含的关键词数量                                   |
| doc_word_count                    | 文档级别 | int   | 文档中匹配的唯一关键词数量                                   |
| lcs                               | 字段级别 | int   | 查询和文档之间的最长公共子序列，按单词计算                   |
| user_weight                       | 字段级别 | int   | 用户指定的字段权重                                           |
| hit_count                         | 字段级别 | int   | 匹配字段中的关键词总出现次数                                 |
| word_count                        | 字段级别 | int   | 匹配字段中唯一关键词的数量                                   |
| tf_idf                            | 字段级别 | float | 匹配关键词的 tf*idf 之和                                     |
| min_hit_pos                       | 字段级别 | int   | 第一次匹配关键词出现的位置（按单词计算）                     |
| min_best_span_pos                 | 字段级别 | int   | 最长 LCS 匹配的位置（按单词计算）                            |
| exact_hit                         | 字段级别 | bool  | 查询是否与字段完全匹配                                       |
| min_idf                           | 字段级别 | float | 匹配关键词的最小 idf                                         |
| max_idf                           | 字段级别 | float | 匹配关键词的最大 idf                                         |
| sum_idf                           | 字段级别 | float | 匹配关键词的 idf 之和                                        |
| exact_order                       | 字段级别 | bool  | 查询关键词是否全部匹配且顺序一致                             |
| min_gaps                          | 字段级别 | int   | 匹配关键词之间的最小间隙数                                   |
| lccs                              | 字段级别 | int   | 查询和文档之间最长的公共连续子序列，按单词计算               |
| wlccs                             | 字段级别 | float | 加权的最长公共连续子序列，按匹配关键词的 idf 之和计算        |
| atc                               | 字段级别 | float | 聚合词条接近度，按最优关键词对计算的距离和 idf 之和计算的对数 |



### 文档级别排名因素

**文档级别因素**是排名引擎为每个匹配文档相对于当前查询计算的数值。这些因素与普通文档属性不同，因为普通文档属性与全文查询无关，而文档级别因素可能会与查询有关。这些因素可以在排名表达式的任何地方使用。目前实现的文档级别因素有：

- **`bm25`** (整数)：文档级别的 BM25 估算值（未经过关键词出现次数过滤的计算结果）。
- **`max_lcs`** (整数)：查询级别的最大可能值，`sum(lcs*user_weight)` 表达式可以达到的最大值。这可以用于权重提升的缩放。例如，`MATCHANY` 排序器公式使用此值来保证任意字段中的完整短语匹配的排名高于所有字段中的部分匹配组合。
- **`field_mask`** (整数)：文档级别的匹配字段的 32 位掩码。
- **`query_word_count`** (整数)：查询中的唯一关键词数量，考虑到排除的关键词。例如，查询 `(one one one one)` 和 `(one !two)` 的 `query_word_count` 都应为 1，因为只有一个唯一的非排除关键词。
- **`doc_word_count`** (整数)：文档中匹配的唯一关键词数量。

### 字段级别排名因素

**字段级别因素**是排名引擎为每个与查询匹配的文档中的字段计算的数值。由于查询可以匹配多个字段，而最终权重需要是一个单一的整数值，因此这些字段级别的数值需要折叠为一个单一值。为实现这一点，字段级别因素只能在字段聚合函数中使用，不能在表达式的任何地方直接使用。例如，您不能直接使用 `(lcs+bm25)` 作为排名表达式，因为 `lcs` 在每个匹配字段中有多个值。您应使用 `(sum(lcs)+bm25)`，该表达式对所有匹配字段的 `lcs` 求和后，再加上 `bm25`。目前实现的字段级别因素有：

* `lcs` (整数)：查询与文档之间的最大逐字匹配长度，按单词计数。LCS 代表“最长公共子序列”。当字段中仅匹配到孤立的关键词时，`lcs` 的最小值为 1；当查询中的所有关键词按顺序完全匹配字段时，`lcs` 的最大值等于查询中关键词的数量。
* `user_weight` (整数)：用户指定的每字段权重（参考 SQL 中的 [OPTION field_weights](../Searching/Options.md#field_weights)）。如果未明确指定，默认权重为 1。
* `hit_count` (整数)：字段中匹配到的关键词总出现次数。单个关键词可能会多次出现。例如，如果“hello”在字段中出现了 3 次，“world”出现了 5 次，则 `hit_count` 为 8。
* `word_count` (整数)：字段中匹配到的唯一关键词数量。例如，如果“hello”和“world”在字段中任何地方出现，则 `word_count`为 2，而不考虑两个关键词出现的次数。
* `tf_idf` **``**(浮点数)：字段中匹配关键词的 TF/IDF 之和。IDF 是逆文档频率（Inverse Document Frequency），是介于 0 到 1 之间的浮点值，用来描述关键词的罕见程度。TF 是词频，即字段中关键词的出现次数。实际上，`tf_idf` 是通过对匹配到的所有出现次数的 IDF 求和计算出来的，这相当于对所有匹配关键词的 TF*IDF 进行求和。
* `min_hit_pos(整数): 第一次匹配到关键词的位置，按单词计数。

   这是一个相对底层的“原始”因素，通常在使用它进行排名之前需要进行*调整*。具体的调整取决于您的数据和公式设计，但这里有一些建议：(a) 当 `word_count < 2` 时，可以忽略任何基于 `min_gaps` 的加权；

   (b) 当 `word_count >= 2` 时，非平凡的 `min_gaps` 值（即，间隙数不为零）可以设定一个“最坏情况下”的常数进行约束，而平凡的 `min_gaps` 值（即 `min_gaps = 0` 且 `word_count < 2`）则可以替换为该常数；

   (c) 可以应用类似于 `1/(1+min_gaps)` 的转换函数（这样较小的 `min_gaps` 值会使结果更好，而较大的 `min_gaps` 值会逐渐减小影响）；等等。
* `lccs` (整数): 最长公共连续子序列。查询和文档之间最长的公共子短语长度，按关键词计算。

    `lccs` 与 `lcs` 相似，但更严格。虽然 `lcs` 可以大于 1 即使查询词不在相邻位置匹配，但 `lccs` 只有在文档中包含*精确*的连续查询子短语时才会大于 1。例如，查询 "one two three four five" 对比文档 "one hundred three hundred five hundred" 的 `lcs=3`，但 `lccs=1`，因为虽然匹配了 3 个关键词（one, three, five），但没有 2 个匹配位置是相邻的。

    请注意，`lccs` 仍然不区分关键词的频率；如需区分，请参阅 `wlccs`。
* `wlccs` (浮点数): 加权最长公共连续子序列。关键词的 IDF 之和，用于查询和文档之间最长的公共子短语。

    `wlccs` 的计算方式与 `lccs` 类似，但每次适合的关键词匹配都会增加该关键词的 IDF，而不是简单地增加 1（如同在 `lcs` 和 `lccs` 中）。这使得稀有且更重要的关键词序列相比频繁出现的关键词序列排名更高，即使后者更长。例如，查询 `(Zanzibar bed and breakfast)` 在文档 `(hotels of Zanzibar)` 中 `lccs=1`，但在文档 `(London bed and breakfast)` 中 `lccs=3`，即使 "Zanzibar" 实际上比 "bed and breakfast" 整个短语更稀有。`wlccs` 通过使用关键词频率解决了这个问题。
* `atc` (浮点数): 聚合词条接近度。当文档包含更多紧密位置的且更重要（稀有）的查询关键词组时，该值会增加。

    **警告：**使用 `atc` 时，您应设置 `OPTION idf='plain,tfidf_unnormalized'`，否则可能会得到意外结果。

    `atc` 的计算方式如下。对于文档中的每个关键词*出现*，我们计算所谓的*词条接近度*。为此，我们检查左右两侧的其他最近的查询关键词出现，计算距离衰减系数 `k = pow(distance, -1.75)`，然后将衰减后的 IDF 求和。这样，我们为每个关键词出现生成了一个接近度值，描述了该关键词周围的“邻居”。然后我们将每个关键词的接近度乘以该关键词的 IDF，求和后最终计算该和的对数。

换句话说，我们处理文档中最佳（最近）的关键词对，并计算其 IDF 乘以距离系数的“接近度”：

```
pair_tc = idf(pair_word1) * idf(pair_word2) * pow(pair_distance, -1.75)
```

然后我们对这些接近度求和，计算出最终的对数衰减 `atc` 值：

```
atc = log(1+sum(pair_tc))
```

请注意，最终的对数衰减正是我们应使用 `OPTION idf=plain` 的原因，因为没有它，`log()` 内部的表达式可能为负数。

关键词越靠近对 `atc` 的贡献比关键词更频繁的出现要大得多。确实，当关键词紧邻时，`distance=1` 且 `k=1`；当关键词之间只有一个词时，`distance=2` 且 `k=0.297`；有两个词时，`distance=3` 且 `k=0.146`，依此类推。同时，IDF 的衰减速度较慢。例如，在 100 万个文档集合中，关键词分别在 10、100 和 1000 个文档中匹配时，IDF 值分别为 0.833、0.667 和 0.500。因此，两个关键词分别只出现在 10 个文档中但它们之间相隔 2 个词的 `pair_tc` 为 0.101，而 100 个文档和 1000 个文档中关键词之间相隔 1 个词的 `pair_tc` 为 0.099。此外，两个唯一关键词（只出现在 1 个文档中）但它们之间相隔 3 个词的 `pair_tc` 为 0.088，而两个紧邻的 1000 个文档中的关键词的 `pair_tc` 为 0.25。因此，尽管 `atc` 结合了关键词频率和接近度，它仍然更倾向于接近度。

### 排名因素聚合函数

**字段聚合函数** 是一个接受字段级别因素表达式的单参数函数，它遍历所有匹配的字段并计算最终结果。当前实现的字段聚合函数包括：

- **`sum`**：对所有匹配字段中的参数表达式求和。例如，`sum(1)` 应返回匹配字段的数量。
- **`top`**：返回所有匹配字段中的参数的最高值。
- **`max_window_hits`**：管理一个命中位置的滑动窗口，用于跟踪指定窗口大小内的最大命中数。它移除超出窗口范围的过时命中并添加最新的命中，从而更新窗口内找到的最大命中数。

### 所有内置排序器的公式表达式

大多数其他排序器实际上可以使用基于表达式的排序器进行模拟。只需提供一个合适的表达式即可。虽然这种模拟可能比使用内置的、编译过的排序器要慢，但如果您希望从现有排序器开始微调您的排名公式，这可能仍然很有趣。此外，这些公式以清晰、易读的方式描述了排序器的细节。

- **proximity_bm25**（默认排序器）= `sum(lcs*user_weight)*1000+bm25`
- **bm25** = `sum(user_weight)*1000+bm25`
- **none** = `1`
- **wordcount** = `sum(hit_count*user_weight)`
- **proximity** = `sum(lcs*user_weight)`
- **matchany** = `sum((word_count+(lcs-1)*max_lcs)*user_weight)`
- **fieldmask** = `field_mask`
- **sph04** = `sum((4*lcs+2*(min_hit_pos==1)+exact_hit)*user_weight)*1000+bm25`

### IDF 公式的配置

Manticore 历史上的默认 IDF（逆文档频率）相当于 `OPTION idf='normalized,tfidf_normalized'`，这些归一化可能会导致一些不希望的效果。

首先，`idf=normalized` 会导致关键词被惩罚。例如，如果您搜索 `the | something`，且 `the` 出现在超过 50% 的文档中，那么同时包含关键词 `the` 和 `something` 的文档的权重会低于仅包含关键词 `something` 的文档。使用 `OPTION idf=plain` 可以避免这种情况。

`plain` IDF 的范围为 `[0, log(N)]`，且关键词不会受到惩罚；而 `normalized` IDF 的范围为 `[-log(N), log(N)]`，出现过于频繁的关键词会受到惩罚。

其次，`idf=tfidf_normalized` 会导致 IDF 随查询发生漂移。历史上，我们还会将 IDF 除以查询关键词的数量，以确保所有关键词的 `sum(tf*idf)` 仍然适合 `[0,1]` 范围。然而，这意味着查询 `word1` 和 `word1 | nonmatchingword2` 会为相同的结果集分配不同的权重，因为 `word1` 和 `nonmatchingword2` 的 IDF 会分别被除以 2。使用 `OPTION idf='tfidf_unnormalized'` 可以修复这一问题。请注意，一旦禁用此归一化，BM25、BM25A、BM25F() 排名因素将相应地进行缩放。

IDF 标志可以混合使用；`plain` 和 `normalized` 互斥；`tfidf_unnormalized` 和 `tfidf_normalized` 也互斥；而在这些互斥组中未指定的标志将采用默认值。这意味着 `OPTION idf=plain` 相当于完整的 `OPTION idf='plain,tfidf_normalized'` 规范。

<!-- proofread -->
