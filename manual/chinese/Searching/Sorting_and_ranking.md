# 排序和排名

查询结果可以根据全文排名权重、一个或多个属性或表达式进行排序。

**全文** 查询默认返回按匹配程度排序的结果。如果没有指定，它们将按相关性排序，这在SQL格式中等同于 `ORDER BY weight() DESC`。

**非全文** 查询默认不执行任何排序。

## 高级排序

当你显式提供排序规则，通过在SQL格式中添加 `ORDER BY` 子句或通过HTTP JSON使用 `sort` 选项时，扩展模式会自动启用。

### 通过SQL排序

一般语法：
```sql
SELECT ... ORDER BY
{attribute_name | expr_alias | weight() | random() } [ASC | DESC],
...
{attribute_name | expr_alias | weight() | random() } [ASC | DESC]
```

<!-- example alias -->

在排序子句中，你可以使用最多5列的任意组合，每个后面跟着 `asc` 或 `desc`。除 `weight()` 和 `random()` 函数外，排序子句不允许使用函数和表达式作为参数（后者只能通过SQL以 `ORDER BY random()` 形式使用）。不过，你可以在SELECT列表中使用任何表达式，并按其别名进行排序。

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

## 通过JSON排序

<!-- example sorting 1 -->
`"sort"` 指定一个数组，其中每个元素可以是属性名称或 `_score`（如果你想按匹配权重排序）。在这种情况下，属性的默认排序顺序为升序，`_score` 为降序。

<!-- intro -->

<!-- request JSON -->

```json
{
  "table":"test",
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
你还可以显式指定排序顺序：

* `asc`: 以升序排列
* `desc`: 以降序排列


<!-- intro -->

<!-- request JSON -->

```json
{
  "table":"test",
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
You can also use another syntax and specify the sort order via the `order` property:


<!-- intro -->
<!-- request JSON -->

```json
{
  "table":"test",
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
Sorting by MVA attributes is also supported in JSON queries. Sorting mode can be set via the `mode` property. The following modes are supported:

* `min`: sort by minimum value
* `max`: sort by maximum value

<!-- intro -->
<!-- request JSON -->

```json
{
  "table":"test",
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
	          "title": "测试文档 4"
	        }
	      },
	      {
	        "_id": 5406864699109146629,
	        "_score": 2319,
	        "_source": {
	          "title": "测试文档 2"
	        }
	      },
	      {
	        "_id": 5406864699109146628,
	        "_score": 2319,
	        "_source": {
	          "title": "测试文档 1"
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
$search->setIndex("test")->match('测试文档')->sort('id','desc','max');
```

<!-- intro -->

##### Python:

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('测试文档')
sort = manticoresearch.model.SortMVA('attr_mva', 'desc', 'max')
search_request.sort = [sort]
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('测试文档');
sort = new Manticoresearch.SortMVA('attr_mva', 'desc', 'max');
searchRequest.sort = [sort];
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
searchRequest.setIndex("test");
QueryFilter queryFilter = new QueryFilter();
queryFilter.setQueryString("测试文档");
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
searchRequest.FulltextFilter = new QueryFilter("测试文档");
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
    query_string: {'测试文档'},
  },
  sort: { "attr_mva": { "order":"desc", "mode":"max" } },
}
```

<!-- intro -->

##### Go:

<!-- request go -->
```go
searchRequest.SetIndex("test")
query := map[string]interface{} {"query_string": "测试文档"}
searchRequest.SetQuery(query)
sort := map[string]interface{} { "attr_mva": { "order":"desc", "mode":"max" } }
searchRequest.SetSort(sort)
```

<!-- end -->

<!-- example sorting 5 -->
When sorting on an attribute, match weight (score) calculation is disabled by default (no ranker is used). You can enable weight calculation by setting the `track_scores` property to `true`:

<!-- intro -->

<!-- request JSON -->

```json
{
  "table":"test",
  "track_scores": true,
  "query":
  {
    "match": { "title": "测试文档" }
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
	          "title": "测试文档 4"
	        }
	      },
	      {
	        "_id": 5406864699109146629,
	        "_score": 2319,
	        "_source": {
	          "title": "测试文档 2"
	        }
	      },
	      {
	        "_id": 5406864699109146628,
	        "_score": 2319,
	        "_source": {
	          "title": "测试文档 1"
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
$search->setIndex("test")->match('测试文档')->sort('id','desc','max')->trackScores(true);
```

<!-- intro -->

##### Python:

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.track_scores = true
search_request.fulltext_filter = manticoresearch.model.QueryFilter('测试文档')
sort = manticoresearch.model.SortMVA('attr_mva', 'desc', 'max')
search_request.sort = [sort]
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.trackScores = true;
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('测试文档');
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
queryFilter.setQueryString("测试文档");
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
searchRequest.FulltextFilter = new QueryFilter("测试文档");
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
    query_string: {'测试文档'},
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
query := map[string]interface{} {"query_string": "测试文档"}
searchRequest.SetQuery(query)
sort := map[string]interface{} { "attr_mva": { "order":"desc", "mode":"max" } }
searchRequest.SetSort(sort)
```

<!-- end -->


## 排名概述

搜索结果的排名（也称为加权）可以定义为计算与匹配给定查询的每个匹配文档相关性（权重）的过程。因此，相关性最终只是附加到每个文档的数字，用于估计该文档与查询的相关程度。搜索结果可以根据该数字和/或一些附加参数进行排序，以便最受欢迎的结果出现在结果页面的上方。

没有单一的标准方式来在任何场景中排名任何文档。此外，永远也不会有这样的方式，因为相关性是*主观的*。换句话说，对您而言似乎相关的内容，对我而言可能并不相关。因此，在一般情况下，这不仅难以计算；而且理论上是不可能的。

所以 Manticore 中的排名是可配置的。它有一个所谓的**排名器**的概念。排名器可以正式定义为一个接受文档和查询作为输入并产生相关性值作为输出的函数。通俗来说，排名器精确控制 Manticore 如何（使用哪个特定算法）将权重分配给文档。

## 可用的内置排名器

Manticore 提供多种适用于不同目的的内置排名器。它们中的许多使用两个因素：短语近似度（也称为 LCS）和 BM25。短语近似度基于关键词位置，而 BM25 则基于关键词频率。本质上，文档主体与查询之间的短语匹配度越高，短语近似度就越高（当文档包含整个查询作为逐字引用时，它达到最大值）。而且当文档包含更多稀有词时，BM25 的值则更高。我们将把详细讨论留到后面。

当前实现的排名器包括：

* `proximity_bm25`，默认排名模式，使用并结合短语近似度和 BM25 排名。
* `bm25`，一种仅使用 BM25 排名的统计排名模式（类似于大多数其他全文引擎）。这种模式速度较快，但可能会导致包含多个关键字的查询质量较差。
* `none`，一种无排名模式。显然这种模式是最快的。所有匹配项的权重为 1。这有时被称为布尔搜索，匹配文档但不对其进行排名。
* `wordcount`，通过关键字出现次数进行排名。该排名器计算每个字段的关键字出现次数，然后将其乘以字段权重，并汇总结果值。
* `proximity` 返回原始的短语近似值作为结果。该模式在内部用于模拟 `SPH_MATCH_ALL` 查询。
* `matchany` 返回在早期的 `SPH_MATCH_ANY` 模式下计算的排名，并在内部用于模拟 `SPH_MATCH_ANY` 查询。
* `fieldmask` 返回一个 32 位掩码，其中第 N 位对应于第 N 个全文字段，编号从 0 开始。仅当相应字段有任何满足查询的关键字出现时，位才会被设置。
* `sph04` 通常基于默认的 `proximity_bm25` 排名器，但在匹配发生在文本字段的最开始或最结束时会额外提升匹配。因此，如果字段完全等于查询，`sph04` 应该比包含完全相同查询但不等于它的字段排名更高。（例如，当查询为“海德公园”时，标题为“海德公园”的文档应比标题为“海德公园，伦敦”或“海德公园咖啡馆”的文档排名更高。）
* `expr` 允许您在运行时指定排名公式。它暴露了多个内部文本因素，并让您定义如何从这些因素计算最终权重。有关其语法和可用因素的更多详细信息，可以在 [下面的小节](../Searching/Sorting_and_ranking.md#Quick-summary-of-the-ranking-factors) 中找到。

排名器名称不区分大小写。示例：

```sql
SELECT ... OPTION ranker=sph04;
```

## 排名因素的快速总结

| 名称                    | 级别     | 类型  | 摘要                                                                                                            |
| ----------------------- | --------- | ----- | ------------------------------------------------------------------------------------------------------------------ |
| max_lcs                 | 查询     | int   | 当前查询的最大 LCS 值                                                                                       |
| bm25                    | 文档     | int   | BM25(1.2, 0) 的快速估计                                                                                     |
| bm25a(k1, b)            | 文档     | int   | 具有可配置 K1、B 常数和语法支持的精确 BM25() 值                                                             |
| bm25f(k1, b, {field=weight, ...}) | 文档 | int   | 具有额外可配置字段权重的精确 BM25F() 值                                                                       |
| field_mask              | 文档     | int   | 匹配字段的位掩码                                                                                             |
| query_word_count        | 文档     | int   | 查询中唯一包含的关键字数量                                                                                   |
| doc_word_count          | document  | int   | 文档中匹配的唯一关键字数量                                                                           |
| lcs                     | field     | int   | 查询与文档之间的最长公共子序列，以词为单位                                                        |
| user_weight             | field     | int   | 用户字段权重                                                                                       |
| hit_count               | field     | int   | 关键字出现的总次数                                                                                |
| word_count              | field     | int   | 匹配的唯一关键字数量                                                                                |
| tf_idf                  | field     | float | 匹配关键字的sum(tf*idf) == occurrences的sum(idf)                                                  |
| min_hit_pos             | field     | int   | 第一个匹配的出现位置，以词为单位，基于1                                                             |
| min_best_span_pos       | field     | int   | 第一个最大LCS跨度位置，以词为单位，基于1                                                           |
| exact_hit               | field     | bool  | 查询是否等于字段                                                                                    |
| min_idf                 | field     | float | 匹配关键字的min(idf)                                                                               |
| max_idf                 | field     | float | 匹配关键字的max(idf)                                                                               |
| sum_idf                 | field     | float | 匹配关键字的sum(idf)                                                                               |
| exact_order             | field     | bool  | 所有查询关键字是否被 a) 匹配且 b) 按查询顺序排列                                                  |
| min_gaps                | field     | int   | 匹配关键字之间的最小间隔数量                                                                        |
| lccs                    | field     | int   | 查询与文档之间的最长公共连续子序列，以词为单位                                                   |
| wlccs                   | field     | float | 加权最长公共连续子序列，连续关键字跨度的sum(idf)                                                  |
| atc                     | field     | float | 聚合术语紧密度，log(1+sum(idf1*idf2*pow(distance, -1.75))，适用于最佳关键字对                      |

### 文档级排名因素

**文档级因素**是排名引擎针对每个匹配文档和当前查询计算的数值。与普通文档属性不同，属性不依赖于完整文本查询，而因素可能依赖。这些因素可以在排名表达式的任何位置使用。目前实现的文档级因素包括：

* `bm25`（整数），文档级的BM25估算值（计算时不进行关键字出现筛选）。
* `max_lcs`（整数），查询级的可能最大值，即表达式`sum(lcs*user_weight)`所能达到的最大值。这对于权重提升缩放非常有用。例如，`MATCHANY`排名公式使用此值来确保在任何字段中的完整短语匹配比所有字段中任何部分匹配的组合得分更高。
* `field_mask`（整数），文档级的32位匹配字段掩码。
* `query_word_count`（整数），查询中唯一关键字的数量，调整后排除关键字的数量。例如，`(one one one one)`和`(one !two)`查询应当将此因素赋值为1，因为只有一个唯一的非排除关键字。
* `doc_word_count`（整数），在整个文档中匹配的唯一关键字数量。

### 字段级排名因素

**字段级因素**是排名引擎针对当前查询计算的每个匹配文档正文字段的数值。由于一个查询可以匹配多个字段，而最终权重需要是一个整数值，因此这些值需要折叠成一个。为此，字段级因素只能在字段聚合函数内使用，不能在表达式的其他地方使用。例如，您不能将`(lcs+bm25)`用作排名表达式，因为`lcs`在每个匹配字段中都有多个值。您应该使用`(sum(lcs)+bm25)`，该表达式对所有匹配字段的`lcs`进行求和，然后再将`bm25`加到该字段的总和上。目前实现的字段级因素包括：
* `lcs` (整数)，文档和查询之间的最大逐字匹配的长度，以单词计数。LCS代表最长公共子序列（或子集）。当在一个字段中仅匹配到零散关键词时，取最小值为1，当整个查询在字段中逐字匹配（按查询关键词的确切顺序）时，取最大值为查询关键词的计数。例如，如果查询为“hello world”，而字段包含这两个来自查询的单词（即，相邻并且完全按照查询顺序），`lcs`将为2。例如，如果查询为“hello world program”，而字段包含“hello world”，`lcs`将为2。请注意，查询关键词的任何子集都有效，而不仅仅是相邻关键词的子集。例如，如果查询为“hello world program”，而字段包含“hello (test program)”，`lcs`仍然为2，因为“hello”和“program”在查询中对应的位置匹配。最后，如果查询为“hello world program”，而字段包含“hello world program”，`lcs`将为3。（希望这在此时并不令人惊讶。）
* `user_weight` (整数)，用户指定的每个字段权重（请参阅[OPTION field_weights](../Searching/Options.md#field_weights)中的SQL）。如果未明确指定，则权重默认为1。
* `hit_count` (整数)，在字段中匹配的关键词出现次数。请注意，单个关键词可能出现多次。例如，如果“hello”在字段中出现3次，而“world”出现5次，`hit_count`将为8。
* `word_count` (整数)，在字段中匹配的唯一关键词的数量。例如，如果“hello”和“world”在字段中的任何地方出现，`word_count`将为2，无论这两个关键词出现多少次。
* `tf_idf` (浮点数)，在字段中所有匹配关键词的TF/IDF总和。IDF是逆文档频率，是一个介于0和1之间的浮点值，描述了关键词的频率（基本上，0表示在每个索引文档中都出现的关键词，1表示仅在单个文档中出现的独特关键词）。TF是术语频率，表示在字段中匹配的关键词出现次数。作为附注，`tf_idf`实际上是通过对所有匹配出现的IDF求和来计算的。这在构造上等同于对所有匹配关键词的TF*IDF求和。
* `min_hit_pos` (整数)，第一个匹配的关键词出现的位置，以单词计数

   因此，这是一个相对低级的“原始”因素，您可能希望在使用它进行排序之前进行*调整*。具体的调整在很大程度上依赖于您的数据和最终公式，但以下是一些起始的想法：（a）当word_count<2时，任何基于min_gaps的增强可以简单忽略；

    （b）非平凡的min_gaps值（即，当word_count>=2时）可以用某个“最坏情况”常量进行限制，而平凡值（即，当min_gaps=0且word_count<2时）可以用该常量替换；

    （c）可以应用像1/(1+min_gaps)这样的转换函数（使得更好、更小的min_gaps值会使其最大化，而更差、更大的min_gaps值会慢慢减少）；等等。
* `lccs` (整数)。最长公共连续子序列。计算查询与文档之间的最长公共子短语的长度，以关键词计数。

    LCCS因素与LCS有些相似，但更具限制性。虽然LCS即使没有两个查询单词相邻匹配时也可以大于1，但LCCS仅在文档中存在*精确*的相邻查询子短语时才会大于1。例如，（一个两个三个四个五个）查询与（一个一百三个一百五百）文档之间将产生lcs=3，但lccs=1，因为尽管3个关键词（一个、三个、五个）的相互位置在查询和文档之间匹配，但实际上并没有两个匹配位置是相邻的。

    请注意，LCCS仍然不会区分频繁和稀有关键词；有关这一点，请参见WLCCS。
* `wlccs` (浮点数)。加权最长公共连续子序列。查询和文档之间最长公共子短语的关键词IDF总和。

    WLCCS的计算方法类似于LCCS，但每个“合适”的关键词出现会通过关键词IDF增加，而不仅仅是通过1（如LCS和LCCS）。这允许对更稀有和更重要的关键词进行更高的排名，即使后者的长度更长。例如，查询“（桑给巴尔床和早餐）”在文档“（桑给巴尔的酒店）”中将产生lccs=1，但在“（伦敦床和早餐）”中产生lccs=3，即使“桑给巴尔”实际上比整个“床和早餐”短语稍微稀有。WLCCS因素通过使用关键词频率来解决这个问题。
* `atc` (浮点数)。聚合术语紧密度。一个基于接近性的度量，当文档包含更多组的更密切和更重要（稀有）查询关键词时，该值增加。

    **警告：** 您应该在OPTION idf='plain,tfidf_unnormalized'的情况下使用ATC（请参见[下文](../Searching/Sorting_and_ranking.md#Configuration-of-IDF-formula)）；否则，您可能会得到意想不到的结果。

    ATC基本上按如下方式操作。对于文档中的每个关键词*出现*，我们计算所谓的*术语紧密度*。为此，我们检查所有其他与主题出现的所有查询关键词（包括关键词本身）最接近的出现，计算这些出现的距离衰减系数k = pow(distance, -1.75)，并对衰减后的IDF进行求和。因此，对于每个关键词的每个出现，我们获得一个描述该出现的“邻居”的“紧密度”值。然后，我们将这些每次出现的紧密度乘以各自关键词的IDF，将所有值相加，最后计算该总和的对数。
换句话说，我们在文档中处理最佳（最近）匹配的关键词对，并计算它们的成对“接近度”，作为它们的IDF乘以距离系数的乘积：

```
pair_tc = idf(pair_word1) * idf(pair_word2) * pow(pair_distance, -1.75)
```

然后，我们将这样的接近度相加，并计算最终的、带有对数衰减的ATC值：

```
atc = log(1+sum(pair_tc))
```

请注意，这个最终的衰减对数恰恰是你应该使用OPTION idf=plain的原因，因为如果没有它，log()内部的表达式可能会是负数。

更靠近的关键词出现对ATC的贡献*要大得多*，而不是更多频繁的关键词。实际上，当关键词彼此紧挨着时，距离=1，k=1；当它们之间只有一个词时，距离=2，k=0.297；有两个词在它们之间时，距离=3，k=0.146，依此类推。同时，IDF的衰减速度稍慢。例如，在一个100万个文档的集合中，在10、100和1000个文档中匹配的关键词的IDF值分别为0.833、0.667和0.500。所以，一个由两个相对稀有的关键词组成的关键词对（每个词在10个文档中出现）但中间有2个其他词会产生pair_tc = 0.101，因此几乎不超过一个在100个文档和1000个文档中关键词中间有1个其他词的关键词对，其pair_tc = 0.099。此外，由两个*唯一*的、每个在1个文档中的关键词组成的关键词对，如果之间有3个词，将得到pair_tc = 0.088，并输给一个两个1000-doc关键词紧挨在一起的关键词对，其pair_tc = 0.25。因此，基本上，虽然ATC确实结合了关键词的频率和接近度，但它仍然稍微偏向接近度。

### 排名因素聚合函数

一个**字段聚合函数**是一个接受带有字段级因子的表达式的单参数函数，遍历所有匹配的字段，并计算最终结果。当前实现的字段聚合函数包括：

* `sum`，用于在所有匹配字段中添加参数表达式。例如 `sum(1)` 应返回匹配字段的数量。
* `top`，返回所有匹配字段中参数的最高值。
* `max_window_hits`，管理命中位置的滑动窗口，以跟踪指定窗口大小内的最大命中数量。它会删除超出窗口的过时命中并添加最新的命中，更新该窗口内找到的最大命中数量。

### 所有内置排名器的公式表达式

其他大多数排名器实际上可以使用基于表达式的排名器进行模拟。您只需要提供一个适当的表达式。虽然这种模拟可能会比使用内置的编译排名器更慢，但如果您想从现有的排名公式开始微调自己的排名公式，这仍然可能很有趣。此外，这些公式以清晰、可读的方式描述了排名器的细节。

* proximity_bm25（默认排名器）=`sum(lcs*user_weight)*1000+bm25`
* bm25=`sum(user_weight)*1000+bm25`
* none=`1`
* wordcount=`sum(hit_count*user_weight)`
* proximity=`sum(lcs*user_weight)`
* matchany=`sum((word_count+(lcs-1)*max_lcs)*user_weight)`
* fieldmask=`field_mask`
* sph04=`sum((4*lcs+2*(min_hit_pos==1)+exact_hit)*user_weight)*1000+bm25`


### IDF公式的配置

Manticore中历史默认的IDF（逆文档频率）相当于`OPTION idf='normalized,tfidf_normalized'`，这些归一化可能会导致几个不希望的效果。

首先，`idf=normalized`会导致关键词惩罚。例如，如果你搜索 `the | something` 并且 `the` 出现在超过50%的文档中，那么同时包含关键词 `the` 和`[something` 的文档会比仅包含关键词 `something` 的文档权重更低。使用 `OPTION idf=plain` 可以避免这种情况。

纯IDF在`[0, log(N)]`范围内变化，并且关键词永远不会受到惩罚；而归一化IDF在`[-log(N), log(N)]`范围内变化，并且过于频繁的关键词会受到惩罚。

其次，`idf=tfidf_normalized`导致IDF在查询中漂移。历史上，我们还将IDF除以查询关键词数量，以确保所有关键词的 `sum(tf*idf)` 仍然适合`[0,1]`范围。然而，这意味着查询 `word1` 和 `word1 | nonmatchingword2`会对完全相同的结果集分配不同的权重，因为 `word1` 和 `nonmatchingword2` 的IDF将被除以2。`OPTION idf='tfidf_unnormalized'` 修复了这个问题。请注意，一旦您禁用此归一化，BM25、BM25A、BM25F()排名因子将相应地进行缩放。

IDF标志可以混合；`plain`和`normalized`是互斥的；`tfidf_unnormalized`和`tfidf_normalized`是互斥的；而在这种互斥组中未指定的标志将采用其默认值。这意味着 `OPTION idf=plain`等同于完整的`OPTION idf='plain,tfidf_normalized'`规范。    

<!-- proofread -->

















