# 排序和排名

查询结果可以按全文排名权重、一个或多个属性或表达式进行排序。

**全文** 查询默认返回排序的匹配项。如果未指定任何内容，则按相关性排序，这相当于 SQL 格式中的 `ORDER BY weight() DESC`。

**非全文** 查询默认不执行任何排序。

## 高级排序

当您通过添加 SQL 格式的 `ORDER BY` 子句或通过 HTTP JSON 使用 `sort` 选项显式提供排序规则时，自动启用扩展模式。

### 通过 SQL 排序

通用语法：
```sql
SELECT ... ORDER BY
{attribute_name | expr_alias | weight() | random() } [ASC | DESC],
...
{attribute_name | expr_alias | weight() | random() } [ASC | DESC]
```

<!-- example alias -->

在排序子句中，您可以使用最多 5 列的任意组合，每列后跟 `asc` 或 `desc`。函数和表达式不允许作为排序子句的参数，除了 `weight()` 和 `random()` 函数（后者只能通过 SQL 以 `ORDER BY random()` 形式使用）。但是，您可以在 SELECT 列表中使用任何表达式，并按其别名排序。

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

## 通过 JSON 排序

<!-- example sorting 1 -->
`"sort"` 指定一个数组，其中每个元素可以是属性名，或者如果您想按匹配权重排序则为 `_score`，如果想要随机匹配顺序则为 `_random`。在这种情况下，属性的排序顺序默认为升序，`_score` 默认为降序。

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

##### Python-asyncio:

<!-- request Python-asyncio -->
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

##### Rust:

<!-- request Rust -->
``` rust
let query = SearchQuery {
    query_string: Some(serde_json::json!("Test document").into()),
    ..Default::default(),
};
let sort: [String; 2] = ["_score".to_string(), "id".to_string()];
let search_req = SearchRequest {
    table: "test".to_string(),
    query: Some(Box::new(query)),
    sort: Some(serde_json::json!(sort)),
    ..Default::default(),
};
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
您也可以显式指定排序顺序：

* `asc`：升序排序
* `desc`：降序排序


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

##### Python-asyncio:

<!-- request Python-asyncio -->
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

##### Rust:

<!-- request Rust -->
``` rust
let query = SearchQuery {
    query_string: Some(serde_json::json!("Test document").into()),
    ..Default::default(),
};
let sort_by_id = HashMap::new();
sort_by_id.insert("id".to_string(), "desc".to_string());
let mut sort = Vec::new();
sort.push(sort_by_id);
sort.push("_score".to_string());
let search_req = SearchRequest {
    table: "test".to_string(),
    query: Some(Box::new(query)),
    sort: Some(serde_json::json!(sort)),
    ..Default::default(),
};
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

##### Python-asyncio:

<!-- request Python-asyncio -->
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

##### Rust:

<!-- request Rust -->
``` rust
let query = SearchQuery {
    query_string: Some(serde_json::json!("Test document").into()),
    ..Default::default(),
};
let mut sort_by_id = HashMap::new();
sort_by_id.insert("id".to_string(), "desc".to_string());
let sort = [HashMap; 1] = [sort_by_id];
let search_req = SearchRequest {
    table: "test".to_string(),
    query: Some(Box::new(query)),
    sort: Some(serde_json::json!(sort)),
    ..Default::default(),
};
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
JSON 查询中也支持按 MVA 属性排序。排序模式可以通过 `mode` 属性设置。支持以下模式：

* `min`：按最小值排序
* `max`：按最大值排序

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

##### Python-asyncio:

<!-- request Python-asyncio -->
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

##### Rust:

<!-- request Rust -->
``` rust
let query = SearchQuery {
    query_string: Some(serde_json::json!("Test document").into()),
    ..Default::default(),
};
let mut sort_mva_opts = HashMap::new();
sort_mva_opts.insert("order".to_string(), "desc".to_string());
sort_mva_opts.insert("mode".to_string(), "max".to_string());
let mut sort_mva = HashMap::new();
sort_mva.insert("attr_mva".to_string(), sort_mva_opts);

let search_req = SearchRequest {
    table: "test".to_string(),
    query: Some(Box::new(query)),
    sort: Some(serde_json::json!(sort_mva)),
    ..Default::default(),
};
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
当按属性排序时，默认禁用匹配权重（分数）计算（不使用排名器）。您可以通过将 `track_scores` 属性设置为 `true` 来启用权重计算：

<!-- intro -->

<!-- request JSON -->

```json
{
  "table":"test",
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

##### Python-asyncio:

<!-- request Python-asyncio -->
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

##### Rust:

<!-- request Rust -->
``` rust
let query = SearchQuery {
    query_string: Some(serde_json::json!("Test document").into()),
    ..Default::default(),
};
let mut sort_mva_opts = HashMap::new();
sort_mva_opts.insert("order".to_string(), "desc".to_string());
sort_mva_opts.insert("mode".to_string(), "max".to_string());
let mut sort_mva = HashMap::new();
sort_mva.insert("attr_mva".to_string(), sort_mva_opts);

let search_req = SearchRequest {
    table: "test".to_string(),
    query: Some(Box::new(query)),
    sort: Some(serde_json::json!(sort_mva)),
    track_scores: Some(serde_json::json!(true)),
    ..Default::default(),
};
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

排名（也称为加权）搜索结果可以定义为一个过程，即针对每个匹配的文档计算所谓的相关性（权重），以对应匹配它的查询。因此，相关性最终只是附加到每个文档上的一个数字，用于估计该文档与查询的相关程度。然后可以基于该数字和/或一些附加参数对搜索结果进行排序，使得最受关注的结果出现在结果页的更高位置。

没有一种适用于所有场景的单一标准排名方法。更重要的是，永远不可能有这样的方式，因为相关性是*主观的*。也就是说，对你来说相关的内容，对我来说可能不相关。因此，在一般情况下，相关性不仅难以计算；从理论上讲是不可能的。

所以 Manticore 中的排名是可配置的。它有一个所谓的**ranker**（排名器）的概念。排名器可以形式化定义为一个函数，输入为文档和查询，输出为相关性值。通俗地说，排名器控制 Manticore 使用哪种具体算法来为文档分配权重。

## 可用的内置排名器

Manticore 附带了几个适用于不同用途的内置排名器。它们中的许多使用两个因素：短语接近度（也称为 LCS）和 BM25。短语接近度基于关键词位置，而 BM25 基于关键词频率。基本上，文档正文与查询之间的短语匹配程度越好，短语接近度越高（当文档包含整个查询的逐字引用时达到最大值）。而 BM25 在文档包含更多稀有词时更高。详细讨论留待后面。

当前实现的排名器有：

* `proximity_bm25`，默认排名模式，结合使用短语接近度和 BM25 排名。
* `bm25`，统计排名模式，仅使用 BM25 排名（类似大多数其他全文引擎）。此模式更快，但对于包含多个关键词的查询可能导致质量较差。
* `none`，无排名模式。此模式显然是最快的。所有匹配赋予权重 1。有时称为布尔搜索，只匹配文档但不排名。
* `wordcount`，按关键词出现次数排名。该排名器计算每个字段的关键词出现次数，然后乘以字段权重，最后求和。
* `proximity` 返回原始短语接近度值。此模式内部用于模拟 `SPH_MATCH_ALL` 查询。
* `matchany` 返回之前在 `SPH_MATCH_ANY` 模式下计算的排名，内部用于模拟 `SPH_MATCH_ANY` 查询。
* `fieldmask` 返回一个 32 位掩码，第 N 位对应第 N 个全文字段（从 0 开始编号）。只有当相应字段有满足查询的关键词出现时，该位才会被设置。
* `sph04` 通常基于默认的 `proximity_bm25` 排名器，但额外提升匹配出现在文本字段开头或结尾的权重。因此，如果字段等于精确查询，`sph04` 应该将其排名高于包含精确查询但不完全相等的字段。（例如，当查询为“Hyde Park”时，标题为“Hyde Park”的文档应排名高于标题为“Hyde Park, London”或“The Hyde Park Cafe”的文档。）
* `expr` 允许你在运行时指定排名公式。它暴露了几个内部文本因素，并允许你定义如何从这些因素计算最终权重。你可以在[下面的小节](../Searching/Sorting_and_ranking.md#Quick-summary-of-the-ranking-factors)中找到其语法和可用因素的参考详情。

排名器名称不区分大小写。示例：

```sql
SELECT ... OPTION ranker=sph04;
```

## 排名因素快速总结

| 名称                    | 级别     | 类型  | 概要                                                                                                            |
| ----------------------- | --------- | ----- | ------------------------------------------------------------------------------------------------------------------ |
| max_lcs                 | query     | int   | 当前查询的最大可能 LCS 值                                                                                         |
| bm25                    | document  | int   | BM25(1.2, 0) 的快速估计                                                                                           |
| bm25a(k1, b)            | document  | int   | 具有可配置 K1、B 常数和语法支持的精确 BM25() 值                                                                   |
| bm25f(k1, b, {field=weight, ...}) | document | int   | 具有额外可配置字段权重的精确 BM25F() 值                                                                           |
| field_mask              | document  | int   | 匹配字段的位掩码                                                                                                   |
| query_word_count        | document  | int   | 查询中唯一包含的关键词数量                                                                                         |
| doc_word_count          | document  | int   | 文档中匹配的唯一关键词数量                                                                                         |
| lcs                     | field     | int   | 查询与文档之间的最长公共子序列（以词为单位）                                                                       |
| user_weight             | field     | int   | 用户字段权重                                                                                                      |
| hit_count               | field     | int   | 关键词出现的总次数                                                                                                 |
| word_count              | field     | int   | 匹配的唯一关键词数量                                                                                               |
| tf_idf                  | field     | float | 匹配关键词的 sum(tf*idf) == 出现次数的 sum(idf)                                                     |
| min_hit_pos             | field     | int   | 第一个匹配出现位置，按词计，基于1                                                               |
| min_best_span_pos       | field     | int   | 第一个最大 LCS 区间位置，按词计，基于1                                                                 |
| exact_hit               | field     | bool  | 查询是否等于字段                                                                                             |
| min_idf                 | field     | float | 匹配关键词的最小 idf                                                                                     |
| max_idf                 | field     | float | 匹配关键词的最大 idf                                                                                     |
| sum_idf                 | field     | float | 匹配关键词的 idf 之和                                                                                     |
| exact_order             | field     | bool  | 是否所有查询关键词 a) 被匹配且 b) 按查询顺序排列                                                   |
| min_gaps                | field     | int   | 匹配区间中匹配关键词之间的最小间隙数                                        |
| lccs                    | field     | int   | 查询与文档之间的最长连续公共子序列，按词计                                         |
| wlccs                   | field     | float | 加权最长连续公共子序列，连续关键词区间的 idf 之和                            |
| atc                     | field     | float | 聚合词项接近度，log(1+最佳关键词对的 idf1*idf2*pow(距离, -1.75) 之和)               |

### 文档级排名因子

**文档级因子** 是排名引擎针对当前查询为每个匹配文档计算的数值。因此它不同于普通文档属性，后者不依赖全文查询，而因子可能依赖。这些因子可以在排名表达式中任何地方使用。目前实现的文档级因子有：

* `bm25`（整数），文档级 BM25 估计值（计算时不进行关键词出现过滤）。
* `max_lcs`（整数），查询级最大可能值，即 `sum(lcs*user_weight)` 表达式可能达到的最大值。这对于权重提升缩放很有用。例如，`MATCHANY` 排序公式使用它来保证任何字段中的完整短语匹配排名高于所有字段中部分匹配的任意组合。
* `field_mask`（整数），文档级 32 位匹配字段掩码。
* `query_word_count`（整数），查询中唯一关键词的数量，调整了排除关键词的数量。例如，`(one one one one)` 和 `(one !two)` 查询都应为该因子赋值为 1，因为只有一个唯一的非排除关键词。
* `doc_word_count`（整数），整个文档中匹配的唯一关键词数量。

### 字段级排名因子

**字段级因子** 是排名引擎针对当前查询为每个匹配的文档内文本字段计算的数值。由于查询可能匹配多个字段，但最终权重需要是单个整数值，这些值需要合并成一个。为此，字段级因子只能在字段聚合函数内使用，**不能**在表达式的其他地方使用。例如，不能使用 `(lcs+bm25)` 作为排名表达式，因为 `lcs` 在每个匹配字段中有多个值。应使用 `(sum(lcs)+bm25)`，该表达式先对所有匹配字段的 `lcs` 求和，再加上 `bm25`。目前实现的字段级因子有：

* `lcs`（整数），文档与查询之间最大逐字匹配长度，按词计。LCS 代表最长公共子序列（或子集）。当字段中仅匹配零散关键词时，最小值为1；当字段中逐字匹配整个查询（且顺序完全相同）时，最大值为查询关键词数。例如，查询为“hello world”，字段包含这两个词且相邻且顺序相同，则 `lcs` 为2。再如，查询为“hello world program”，字段包含“hello world”，`lcs` 为2。注意，查询关键词的任意子集均可，不仅限于相邻关键词。例如，查询为“hello world program”，字段包含“hello (test program)”，`lcs` 也为2，因为“hello”和“program”分别匹配了查询中的相应位置。最后，查询为“hello world program”，字段包含“hello world program”，`lcs` 为3。（这点应该不令人惊讶。）
* `user_weight`（整数），用户指定的每字段权重（参见 SQL 中的 [OPTION field_weights](../Searching/Options.md#field_weights)）。如果未显式指定，权重默认为1。
* `hit_count`（整数），字段中匹配的关键词出现次数。注意单个关键词可能出现多次。例如，“hello”在字段中出现3次，“world”出现5次，则 `hit_count` 为8。
* `word_count`（整数），字段中匹配的唯一关键词数量。例如，“hello”和“world”任意出现，`word_count` 为2，无论两者出现多少次。
* `tf_idf` (浮点数)，字段中所有匹配关键词的 TF/IDF 之和。IDF 是逆文档频率，是一个介于 0 和 1 之间的浮点值，用来描述关键词的频率（基本上，对于出现在所有索引文档中的关键词，IDF 为 0；对于只出现在单个文档中的唯一关键词，IDF 为 1）。TF 是词频，表示字段中匹配关键词出现的次数。顺便提一下，`tf_idf` 实际上是通过对所有匹配出现的 IDF 求和计算得出的。这在构造上等同于对所有匹配关键词求和 TF*IDF。
* `min_hit_pos` (整数)，第一个匹配关键词出现的位置，按词数计数

   因此，这是一个相对低级的“原始”因子，您可能需要在用于排序之前对其进行*调整*。具体调整高度依赖于您的数据和最终公式，但这里有一些起点建议：（a）当 word_count<2 时，可以简单忽略任何基于 min_gaps 的提升；

    （b）非平凡的 min_gaps 值（即 word_count>=2 时）可以用某个“最坏情况”常数进行限制，而平凡值（即 min_gaps=0 且 word_count<2 时）可以用该常数替代；

    （c）可以应用类似 1/(1+min_gaps) 的传递函数（这样更好、更小的 min_gaps 值会使其最大化，而更差、更大的 min_gaps 值会缓慢下降）；等等。
* `lccs` (整数)。最长公共连续子序列。查询和文档之间最长的公共子短语长度，以关键词计。

    LCCS 因子与 LCS 有些相似，但更具限制性。虽然即使没有两个查询词相邻匹配，LCS 也可以大于 1，但只有当文档中存在*完全*连续的查询子短语时，LCCS 才会大于 1。例如，查询 `(one two three four five)` 与文档 `(one hundred three hundred five hundred)` 的 lcs=3，但 lccs=1，因为虽然三个关键词（one、three、five）在查询和文档中的相对位置匹配，但没有两个匹配位置是相邻的。

    注意 LCCS 仍然不区分频繁和稀有关键词；有关这点，请参见 WLCCS。
* `wlccs` (浮点数)。加权最长公共连续子序列。查询和文档之间最长公共子短语中关键词的 IDF 之和。

    WLCCS 的计算方式类似于 LCCS，但每个“合适”的关键词出现会按关键词 IDF 增加，而不是像 LCS 和 LCCS 那样仅增加 1。这允许将稀有且更重要的关键词序列排名高于频繁关键词序列，即使后者更长。例如，查询 `(Zanzibar bed and breakfast)` 对文档 `(hotels of Zanzibar)` 的 lccs=1，但对 `(London bed and breakfast)` 的 lccs=3，尽管“Zanzibar”实际上比整个“bed and breakfast”短语更稀有。WLCCS 因子通过使用关键词频率解决了这个问题。
* `atc` (浮点数)。聚合词语接近度。基于接近度的度量，当文档包含更多更紧密且更重要（稀有）的查询关键词组时，该值增加。

    **警告：** 您应当使用 OPTION idf='plain,tfidf_unnormalized'（见[下文](../Searching/Sorting_and_ranking.md#Configuration-of-IDF-formula)）；否则，可能会得到意外结果。

    ATC 的基本操作如下。对于文档中的每个关键词*出现*，计算所谓的*词语接近度*。为此，我们检查该出现左右两侧所有查询关键词的最近出现（包括该关键词本身），计算距离衰减系数 k = pow(distance, -1.75)，并对衰减后的 IDF 求和。结果是，对于每个关键词的每个出现，我们得到一个描述该出现“邻居”的“接近度”值。然后将这些每次出现的接近度乘以对应关键词的 IDF，求和后取对数。

换句话说，我们处理文档中最佳（最近）的匹配关键词对，计算它们的成对“接近度”，即它们 IDF 的乘积乘以距离系数：

```
pair_tc = idf(pair_word1) * idf(pair_word2) * pow(pair_distance, -1.75)
```

然后我们对这些接近度求和，并计算最终的对数衰减 ATC 值：

```
atc = log(1+sum(pair_tc))
```

注意，这个最终的衰减对数正是您应使用 OPTION idf=plain 的原因，因为没有它，log() 内的表达式可能为负。

关键词出现越接近，对 ATC 的贡献*远远*大于关键词出现越频繁。实际上，当关键词紧挨着时，distance=1，k=1；中间隔一个词时，distance=2，k=0.297；中间隔两个词时，distance=3，k=0.146，依此类推。与此同时，IDF 衰减较慢。例如，在一百万文档集合中，匹配 10、100 和 1000 个文档的关键词的 IDF 分别为 0.833、0.667 和 0.500。因此，一个由两个相当稀有的关键词组成的关键词对（每个只出现在 10 个文档中），但中间隔两个词，会产生 pair_tc = 0.101，几乎抵消了一个由一个匹配 100 个文档和一个匹配 1000 个文档的关键词组成的关键词对（中间隔一个词），其 pair_tc = 0.099。此外，一对两个*唯一*、只出现在 1 个文档中的关键词，中间隔三个词，pair_tc = 0.088，会输给一对两个紧挨着的匹配 1000 个文档的关键词，pair_tc = 0.25。因此，基本上，虽然 ATC 结合了关键词频率和接近度，但它仍然更偏向于接近度。

### 排序因子聚合函数

**字段聚合函数**是一个单参数函数，接受包含字段级因子的表达式，遍历所有匹配字段，并计算最终结果。目前实现的字段聚合函数包括：

* `sum`，对所有匹配字段的参数表达式求和。例如，`sum(1)` 应返回匹配字段的数量。
* `top`，返回所有匹配字段中参数的最高值。
* `max_window_hits`，管理一个滑动窗口的命中位置，以跟踪指定窗口大小内的最大命中数。它会移除超出窗口范围的过时命中，并添加最新命中，更新该窗口内发现的最大命中数。

### 所有内置排序器的公式表达式

大多数其他排序器实际上可以通过基于表达式的排序器来模拟。你只需提供一个合适的表达式。虽然这种模拟可能比使用内置的、编译的排序器慢，但如果你想从现有排序器之一开始微调你的排序公式，这仍然可能很有趣。此外，这些公式以清晰、可读的方式描述了排序器的细节。

* proximity_bm25（默认排序器）= `sum(lcs*user_weight)*1000+bm25`
* bm25 = `sum(user_weight)*1000+bm25`
* none = `1`
* wordcount = `sum(hit_count*user_weight)`
* proximity = `sum(lcs*user_weight)`
* matchany = `sum((word_count+(lcs-1)*max_lcs)*user_weight)`
* fieldmask = `field_mask`
* sph04 = `sum((4*lcs+2*(min_hit_pos==1)+exact_hit)*user_weight)*1000+bm25`


### IDF 公式配置

Manticore 历史上的默认 IDF（逆文档频率）等同于 `OPTION idf='normalized,tfidf_normalized'`，这些归一化可能会导致一些不良影响。

首先，`idf=normalized` 会导致关键词惩罚。例如，如果你搜索 `the | something`，且 `the` 出现在超过 50% 的文档中，那么同时包含关键词 `the` 和 `something` 的文档权重会低于只包含关键词 `something` 的文档。使用 `OPTION idf=plain` 可以避免这种情况。

Plain IDF 变化范围为 `[0, log(N)]`，关键词永远不会被惩罚；而归一化 IDF 变化范围为 `[-log(N), log(N)]`，频繁出现的关键词会被惩罚。

其次，`idf=tfidf_normalized` 会导致查询间的 IDF 漂移。历史上，我们还将 IDF 除以查询关键词数，以使所有关键词的 `sum(tf*idf)` 仍然适合 `[0,1]` 范围。然而，这意味着查询 `word1` 和 `word1 | nonmatchingword2` 会对完全相同的结果集赋予不同的权重，因为 `word1` 和 `nonmatchingword2` 的 IDF 都会被除以 2。`OPTION idf='tfidf_unnormalized'` 解决了这个问题。请注意，一旦禁用此归一化，BM25、BM25A、BM25F() 排序因子将相应缩放。

IDF 标志可以混合使用；`plain` 和 `normalized` 互斥；`tfidf_unnormalized` 和 `tfidf_normalized` 互斥；未指定的互斥组标志将采用默认值。这意味着 `OPTION idf=plain` 等同于完整的 `OPTION idf='plain,tfidf_normalized'` 指定。

<!-- proofread -->

