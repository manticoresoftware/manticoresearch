# 排序和排名

查询结果可以按全文排名权重、一个或多个属性或表达式进行排序。

**全文**查询默认返回按排序匹配结果。如果未指定任何排序方式，结果将按相关性排序，相当于 SQL 格式中的 `ORDER BY weight() DESC`。

**非全文**查询默认不进行任何排序。

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

<!--
以下示例数据：

DROP TABLE IF EXISTS test;
CREATE TABLE test(a int, b int, f text);
INSERT INTO test (a, b, f) VALUES
(2, 3, 'document');
-->

<!-- example alias -->

在排序子句中，您可以使用最多 5 列的任意组合，每列后跟 `asc` 或 `desc`。除 `weight()` 和 `random()` 函数（后者只能以 `ORDER BY random()` 的形式在 SQL 中使用）外，不允许函数和表达式作为排序子句的参数。然而，您可以在 SELECT 列表中使用任何表达式，并按其别名排序。

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

<!-- request JSON -->
```JSON
POST  /sql?mode=raw -d "select *, a + b alias from test order by alias desc;"
```

<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "id": {
          "type": "long long"
        }
      },
      {
        "a": {
          "type": "long"
        }
      },
      {
        "b": {
          "type": "long"
        }
      },
      {
        "f": {
          "type": "string"
        }
      },
      {
        "alias": {
          "type": "long"
        }
      }
    ],
    "data": [
      {
        "id": 1,
        "a": 2,
        "b": 3,
        "f": document,
        "alias": 5
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

## 通过 JSON 排序

<!-- example sorting 1 -->
`"sort"` 指定一个数组，其中每个元素可以是属性名称，若要按匹配权重排序则使用 `_score`，若要按随机匹配顺序排序则使用 `_random`。在这种情况下，属性默认按升序排序，`_score` 默认按降序排序。

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
##### PHP：

<!-- request PHP -->

```php
$search->setIndex("test")->match('Test document')->sort('_score')->sort('id');
```

<!-- intro -->

##### Python：

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
search_request.sort = ['_score', 'id']
```

<!-- intro -->

##### Python-asyncio：

<!-- request Python-asyncio -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
search_request.sort = ['_score', 'id']
```

<!-- intro -->

##### Javascript：

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Test document');
searchRequest.sort = ['_score', 'id'];
```

<!-- intro -->

##### java：

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

##### C#：

<!-- request C# -->
``` clike
var searchRequest = new SearchRequest("test");
searchRequest.FulltextFilter = new QueryFilter("Test document");
searchRequest.Sort = new List<Object> {"_score", "id"};

```

<!-- intro -->

##### Rust：

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

##### Typescript：

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

##### Go：

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
##### PHP：

<!-- request PHP -->

```php
$search->setIndex("test")->match('Test document')->sort('id', 'desc')->sort('_score');
```

<!-- intro -->

##### Python：

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
sort_by_id = manticoresearch.model.SortOrder('id', 'desc')
search_request.sort = [sort_by_id, '_score']
```

<!-- intro -->

##### Python-asyncio：

<!-- request Python-asyncio -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
sort_by_id = manticoresearch.model.SortOrder('id', 'desc')
search_request.sort = [sort_by_id, '_score']
```

<!-- intro -->

##### Javascript：

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Test document');
sortById = new Manticoresearch.SortOrder('id', 'desc');
searchRequest.sort = [sortById, 'id'];
```

<!-- intro -->

##### java：

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

##### C#：

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

##### Rust：

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

##### Typescript：

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

##### Go：

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
您也可以使用另一种语法，通过 `order` 属性指定排序顺序：


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
##### PHP：

<!-- request PHP -->

```php
$search->setIndex("test")->match('Test document')->sort('id', 'desc');
```

<!-- intro -->

##### Python：

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
sort_by_id = manticoresearch.model.SortOrder('id', 'desc')
search_request.sort = [sort_by_id]
```

<!-- intro -->

##### Python-asyncio：

<!-- request Python-asyncio -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
sort_by_id = manticoresearch.model.SortOrder('id', 'desc')
search_request.sort = [sort_by_id]
```

<!-- intro -->

##### Javascript：

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Test document');
sortById = new Manticoresearch.SortOrder('id', 'desc');
searchRequest.sort = [sortById];
```

<!-- intro -->

##### java：

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

##### C#：

<!-- request C# -->
``` clike
var searchRequest = new SearchRequest("test");
searchRequest.FulltextFilter = new QueryFilter("Test document");
searchRequest.Sort = new List<Object>();
var sortById = new SortOrder("id", SortOrder.OrderEnum.Desc);
searchRequest.Sort.Add(sortById);
```

<!-- intro -->

##### Rust：

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

##### Typescript：

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

##### Go：

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
在 JSON 查询中也支持按 MVA 属性排序。排序模式可以通过 `mode` 属性设置。支持以下模式：

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
##### PHP：

<!-- request PHP -->

```php
$search->setIndex("test")->match('Test document')->sort('id','desc','max');
```

<!-- intro -->

##### Python：

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
sort = manticoresearch.model.SortMVA('attr_mva', 'desc', 'max')
search_request.sort = [sort]
```

<!-- intro -->

##### Python-asyncio：

<!-- request Python-asyncio -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
sort = manticoresearch.model.SortMVA('attr_mva', 'desc', 'max')
search_request.sort = [sort]
```

<!-- intro -->

##### Javascript：

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Test document');
sort = new Manticoresearch.SortMVA('attr_mva', 'desc', 'max');
searchRequest.sort = [sort];
```

<!-- intro -->

##### java：

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

##### C#：

<!-- request C# -->
``` clike
var searchRequest = new SearchRequest("test");
searchRequest.FulltextFilter = new QueryFilter("Test document");
var sort = new SortMVA("attr_mva", SortMVA.OrderEnum.Desc, SortMVA.ModeEnum.Max);
searchRequest.Sort.Add(sort);
```

<!-- intro -->

##### Rust：

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

##### Typescript：

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

##### Go：

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
在按属性排序时，默认禁用匹配权重（得分）计算（不使用排名器）。您可以通过将 `track_scores` 属性设置为 `true` 来启用权重计算：

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
##### PHP：

<!-- request PHP -->

```php
$search->setIndex("test")->match('Test document')->sort('id','desc','max')->trackScores(true);
```

<!-- intro -->

##### Python：

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.track_scores = true
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
sort = manticoresearch.model.SortMVA('attr_mva', 'desc', 'max')
search_request.sort = [sort]
```

<!-- intro -->

##### Python-asyncio：

<!-- request Python-asyncio -->
``` python
search_request.index = 'test'
search_request.track_scores = true
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
sort = manticoresearch.model.SortMVA('attr_mva', 'desc', 'max')
search_request.sort = [sort]
```

<!-- intro -->

##### Javascript：

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.trackScores = true;
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Test document');
sort = new Manticoresearch.SortMVA('attr_mva', 'desc', 'max');
searchRequest.sort = [sort];
```

<!-- intro -->

##### java：

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

##### C#：

<!-- request C# -->
``` clike
var searchRequest = new SearchRequest("test");
searchRequest.SetTrackScores(true);
searchRequest.FulltextFilter = new QueryFilter("Test document");
var sort = new SortMVA("attr_mva", SortMVA.OrderEnum.Desc, SortMVA.ModeEnum.Max);
searchRequest.Sort.Add(sort);
```

<!-- intro -->

##### Rust：

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

##### Typescript：

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

##### Go：

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


## 排名概览

排名（也称为加权）搜索结果可以定义为一个过程，即针对与之匹配的给定查询，计算每个匹配文档的所谓相关性（权重）。因此，相关性最终只是附加到每个文档上的一个数字，用来估计该文档与查询的相关程度。然后可以基于这个数字和/或一些额外参数对搜索结果进行排序，从而使最受关注的结果出现在结果页面的更高处。

没有一种单一的标准适用于任何场景中对任何文档进行排名的方式。更重要的是，也永远不会存在这样的方式，因为相关性是*主观的*。也就是说，对你而言看起来相关的内容可能对我来说并不相关。因此，在一般情况下，计算相关性不仅很难；从理论上讲是不可能的。

所以 Manticore 中的排名是可配置的。它有一个所谓的**ranker**概念。ranker 可以正式定义为一个函数，它以文档和查询作为输入，输出一个相关性值。通俗来说，ranker 精确控制了 Manticore 将如何（使用哪种具体算法）为文档分配权重。

## 可用的内置ranker

Manticore 提供了若干适用于不同目的的内置ranker。它们中的许多都使用两个因素：短语接近度（也称为 LCS）和 BM25。短语接近度基于关键字的位置，而 BM25 基于关键字的频率。基本上，文档正文与查询之间的短语匹配程度越高，短语接近度越高（当文档包含查询的完整逐字引用时，它达到最大值）。而当文档包含更多罕见词时，BM25 值会更高。详细讨论我们稍后再讲。

当前实现的ranker包括：

* `proximity_bm25`，默认排名模式，使用并结合了短语接近度和BM25排名。
* `bm25`，一种统计排名模式，仅使用BM25排名（类似于大多数其他全文引擎）。此模式速度更快，但对于包含多个关键字的查询可能导致较差的质量。
* `none`，无排名模式。显然这是最快的模式。所有匹配都分配权重为1。有时称为布尔检索，仅匹配文档但不对其进行排名。
* `wordcount`，按关键字出现次数排名。该ranker计算每个字段的关键字出现次数，然后乘以字段权重，最后求和。
* `proximity` 返回原始的短语接近度值。此模式内部用于模拟 `SPH_MATCH_ALL` 查询。
* `matchany` 返回之前在 `SPH_MATCH_ANY` 模式下计算的排名，内部用于模拟 `SPH_MATCH_ANY` 查询。
* `fieldmask` 返回一个32位掩码，第N位对应第N个全文字段，编号从0开始。只有当相应字段包含满足查询的关键字出现时，该位才会被设置。
* `sph04` 一般基于默认的 `proximity_bm25` ranker，但额外提升当匹配发生在文本字段的开头或末尾时的权重。因此，如果字段等于精确查询，则 `sph04` 应该将它排在包含该查询但不完全相等的字段之前。（例如，当查询是“Hyde Park”时，标题为“Hyde Park”的文档应排在标题为“Hyde Park, London”或“The Hyde Park Cafe”的文档之前。）
* `expr` 允许你在运行时指定排名公式。它暴露了几个内部文本因素，并让你定义如何根据这些因素计算最终权重。你可以在[下面的小节](../Searching/Sorting_and_ranking.md#Quick-summary-of-the-ranking-factors)中找到关于其语法及可用因素的更多细节。

ranker 名称不区分大小写。示例：

```sql
SELECT ... OPTION ranker=sph04;
```

## 排名因素快速总结

| 名称                    | 级别      | 类型  | 简要说明                                                                                                        |
| ----------------------- | --------- | ----- | ------------------------------------------------------------------------------------------------------------------ |
| max_lcs                 | 查询      | int   | 当前查询的最大可能 LCS 值                                                                                            |
| bm25                    | 文档      | int   | BM25(1.2, 0) 的快速估计                                                                                              |
| bm25a(k1, b)            | 文档      | int   | 使用可配置的 K1、B 常量和语法支持的精确 BM25() 值                                                                        |
| bm25f(k1, b, {field=weight, ...}) | 文档 | int   | 带额外可配置字段权重的精确 BM25F() 值                                                                                 |
| field_mask              | 文档      | int   | 匹配字段的位掩码                                                                                                     |
| query_word_count        | 文档      | int   | 查询中唯一包含关键字的数量                                                                                             |
| doc_word_count          | 文档      | int   | 文档中匹配的唯一关键字数量                                                                                             |
| lcs                     | 字段      | int   | 查询和文档之间最长公共子序列（按词计）                                                                                 |
| user_weight             | 字段      | int   | 用户字段权重                                                                                                        |
| hit_count               | 字段      | int   | 关键字出现的总次数                                                                                                    |
| word_count              | 字段      | int   | 匹配的唯一关键字数量                                                                                                  |
| tf_idf                  | 字段     | 浮点型 | 匹配关键词的 tf*idf 之和 == 出现次数的 idf 之和                                                     |
| min_hit_pos             | 字段     | 整型   | 第一个匹配出现的位置，以词为单位，从1开始                                                               |
| min_best_span_pos       | 字段     | 整型   | 第一个最大LCS跨度的位置，以词为单位，从1开始                                                                 |
| exact_hit               | 字段     | 布尔型  | 查询是否 == 字段                                                                                             |
| min_idf                 | 字段     | 浮点型 | 匹配关键词的最小 idf                                                                                     |
| max_idf                 | 字段     | 浮点型 | 匹配关键词的最大 idf                                                                                     |
| sum_idf                 | 字段     | 浮点型 | 匹配关键词的 idf 之和                                                                                     |
| exact_order             | 字段     | 布尔型  | 查询关键词是否a) 全部匹配且 b) 按查询顺序                                                   |
| min_gaps                | 字段     | 整型   | 匹配跨度中关键词之间的最小间隔数                                        |
| lccs                    | 字段     | 整型   | 查询和文档之间的最长连续公共子序列，以词为单位                                         |
| wlccs                   | 字段     | 浮点型 | 加权最长连续公共子序列，连续关键词跨度的 idf 之和                            |
| atc                     | 字段     | 浮点型 | 聚合词项接近度，log(1+sum(idf1*idf2*pow(距离, -1.75)) 在最佳关键词对上               |

注意：对于使用**短语**、**邻近**或**NEAR**运算符且关键词超过31个的查询，依赖词频的排名因子（如 `tf`、`idf`、`bm25`、`hit_count`、`word_count`）可能会对第31个及以上位置的关键词进行低估。这是由于内部使用了32位掩码来跟踪这些复杂运算符中的词项出现情况。

### 文档级排名因子

**文档级因子**是排名引擎为每个匹配文档针对当前查询计算的数值。因此它不同于普通文档属性，属性不依赖于全文查询，而因子可能会依赖。这些因子可以在排名表达式的任何位置使用。目前实现的文档级因子包括：

* `bm25`（整型），文档级BM25估计（不进行关键词出现过滤）。
* `max_lcs`（整型），查询级可能的最大值，`sum(lcs*user_weight)`表达式可能达到的值。这可用于权重提升缩放。例如，`MATCHANY`排名器公式使用此值以确保任何字段中的完全短语匹配都高于所有字段中任何部分匹配的组合。
* `field_mask`（整型），匹配字段的32位掩码。
* `query_word_count`（整型），查询中唯一关键词的数量，根据排除的关键词数量进行调整。例如，`(one one one one)`和`(one !two)`查询都应赋值为1，因为只有一个唯一的非排除关键词。
* `doc_word_count`（整型），在整个文档中匹配的唯一关键词数量。

### 字段级排名因子

**字段级因子**是排名引擎为每个匹配的文档内文本字段针对当前查询计算的数值。由于可能有多个字段匹配查询，但最终权重需要是单一整型值，这些值需要折叠为单一值。为实现这一点，字段级因子只能在字段聚合函数中使用，**不能**在表达式的任何位置使用。例如，不能使用`(lcs+bm25)`作为排名表达式，因为`lcs`有多个值（每个匹配字段一个）。应使用`(sum(lcs)+bm25)`，该表达式对所有匹配字段的`lcs`求和，然后将`bm25`加到每字段和上。目前实现的字段级因子包括：

* `lcs`（整型），文档和查询之间最大逐字匹配的长度，以词计算。LCS代表最长公共子序列（或子集）。当仅匹配零散关键词时取最小值1，当整个查询以精确查询关键词顺序在字段中逐字匹配时取最大值（查询关键词数量）。例如，如果查询是'hello world'且字段包含从查询直接引用的这两个词（即相邻且完全按查询顺序），`lcs`将为2。如果查询是'hello world program'且字段包含'hello world'，`lcs`将为2。注意，查询关键词的任何子集都可以匹配，不仅限于相邻关键词。例如，如果查询是'hello world program'且字段包含'hello (test program)'，`lcs`也将为2，因为'hello'和'program'分别在查询中的相同位置匹配。最后，如果查询是'hello world program'且字段包含'hello world program'，`lcs`将为3。（希望这一点此时已经不足为奇了。）
* `user_weight`（整型），用户指定的每字段权重（参考[OPTION field_weights](../Searching/Options.md#field_weights)中的SQL）。如果未明确指定，权重默认为1。
* `hit_count`（整型），字段中匹配的关键词出现次数。注意单个关键词可能出现多次。例如，如果'hello'在字段中出现3次，'world'出现5次，`hit_count`将为8。
* `word_count` (整数)，字段中匹配的唯一关键词数量。例如，如果字段中出现了“hello”和“world”，`word_count` 将是 2，无论这两个关键词出现了多少次。
* `tf_idf` (浮点数)，字段中所有匹配关键词的 TF/IDF 之和。IDF 是逆文档频率，是介于 0 到 1 之间的浮点值，用来描述关键词的频率（基本上，出现在每个索引文档中的关键词 IDF 为 0，出现在单个文档中的唯一关键词 IDF 为 1）。TF 是词频，表示字段中匹配关键词的出现次数。顺带一提，`tf_idf` 实际上是通过对所有匹配出现计算 IDF 的总和来计算的，这在构造上等同于对所有匹配关键词计算 TF*IDF 的总和。
* `min_hit_pos` (整数)，第一个匹配关键词出现的位置，按单词计数

   因此，这是一个相对较低级的“原始”因子，您可能需要在将其用于排名之前进行*调整*。具体调整方法很大程度上取决于您的数据和最终公式，但这里有一些起点想法：(a) 当 word_count<2 时，任何基于 min_gaps 的提升都可以简单忽略；

    (b) 非平凡的 min_gaps 值（即 word_count>=2 时）可以被限定在某个“最坏情况”常数范围内，而平凡值（即 min_gaps=0 且 word_count<2）可以用该常数替代；

    (c) 可以应用类似 1/(1+min_gaps) 的转换函数（这样更好的、更小的 min_gaps 值会使其最大化，而更差的、更大的 min_gaps 值会缓慢下降）；等等。
* `lccs` (整数)。最长公共连续子序列。查询与文档之间的最长公共子短语长度，以关键词计算。

    LCCS 因子与 LCS 有些相似，但更具限制性。虽然即使没有两个查询词紧挨匹配，LCS 也可能大于 1，但只有当文档中有*完全相同且连续的查询子短语*时，LCCS 才会大于 1。例如，查询为（one two three four five）与文档为（one hundred three hundred five hundred）时，lcs=3，但 lccs=1，因为尽管三个关键词（one，three，five）在查询和文档中的位置相互对应，但没有两个匹配位置是相邻的。

    注意，LCCS 仍然不区分频繁和稀有关键词；对此，请参见 WLCCS。
* `wlccs` (浮点数)。加权最长公共连续子序列。查询与文档之间最长公共子短语的关键词 IDFs 之和。

    WLCCS 的计算类似于 LCCS，但每个“合适”的关键词出现会按关键词 IDF 增加，而不是像 LCS 和 LCCS 那样只加 1。这允许对稀有且更重要的关键词序列给予更高排名，即使这些序列比频繁关键词序列短。例如，查询 `(Zanzibar bed and breakfast)` 对文档 `(hotels of Zanzibar)` 产生 lccs=1，而对 `(London bed and breakfast)` 产生 lccs=3，尽管“Zanzibar”实际上比“bed and breakfast”短语中任一词都稍稀有。WLCCS 因子通过使用关键词频率解决了这一问题。
* `atc` (浮点数)。聚合词语接近度。基于接近度的度量，当文档包含更多组更紧密且更重要（稀有）的查询关键词时，该值增加。

    **警告：** 您应当与 OPTION idf='plain,tfidf_unnormalized' 一同使用 ATC（见[下面](../Searching/Sorting_and_ranking.md#Configuration-of-IDF-formula)）；否则，可能得到意外结果。

    ATC 的基本运作方式如下。对于文档中每个关键词*出现*，计算所谓的*词语接近度*。为此，我们检查该出现左右侧所有查询关键词的最近的其它出现（包括该关键词本身），按距离计算衰减系数 k = pow(distance, -1.75)，并对这些出现的 IDF 加权求和。结果是针对每个关键词出现，得到一个描述该出现“邻居”的“接近度”数值。然后，将这些每次出现的接近度乘以该关键词的 IDF，求和所有结果，最终计算该和的对数。

换句话说，我们处理文档中最佳（最接近）的匹配关键词对，计算它们的成对“接近度”为 IDF 乘以距离系数的乘积：

```
pair_tc = idf(pair_word1) * idf(pair_word2) * pow(pair_distance, -1.75)
```

然后我们对这些接近度求和，计算最终的对数衰减 ATC 值：

```
atc = log(1+sum(pair_tc))
```

注意最终的衰减对数正是您需要使用 OPTION idf=plain 的原因，否则 log() 内部的表达式可能为负。

关键词出现更接近对 ATC 的贡献远大于关键词更频繁。实际上，当关键词紧挨着时，distance=1，k=1；关键词间隔一个词时，distance=2，k=0.297；两个词之间，distance=3，k=0.146，依此类推。与此同时，IDF 衰减较慢。例如，在一百万文档集合中，匹配于 10、100 和 1000 个文档的关键词对应的 IDF 值分别是 0.833、0.667 和 0.500。因此，两个各出现在 10 个文档的较稀有关键词对且之间有两个词间隔的 pair_tc = 0.101，几乎抵消了一个由匹配 100 和 1000 文档的关键词组成且之间有一个词的 pair_tc = 0.099。此外，两词之间间隔三个词的两个*唯一*、仅出现在单个文档的关键词对的 pair_tc = 0.088，会输给两个紧挨着的、各出现在 1000 个文档的关键词对，后者 pair_tc = 0.25。因此，基本上，虽然 ATC 结合了关键词频率和接近度，但它仍更偏重接近度。

### 排名因子聚合函数

一个**字段聚合函数**是一个单参数函数，接受带有字段级因子的表达式，遍历所有匹配的字段，并计算最终结果。当前已实现的字段聚合函数包括：

* `sum`，它将参数表达式在所有匹配的字段上相加。例如 `sum(1)` 应返回匹配字段的数量。
* `top`，它返回所有匹配字段中参数的最高值。
* `max_window_hits`，管理命中位置的滑动窗口，以跟踪指定窗口大小内的最大命中数。它移除落在窗口外的过时命中，并添加最新命中，更新在该窗口内找到的最大命中数。

### 所有内置排名器的公式表达式

大多数其他排名器实际上可以使用基于表达式的排名器模拟。您只需提供适当的表达式。虽然这种模拟可能比使用内置的编译排名器慢，但如果您想从现有排名器开始微调排名公式，这仍可能很有趣。此外，这些公式以清晰、可读的方式描述了排名器的细节。

* proximity_bm25（默认排名器）= `sum(lcs*user_weight)*1000+bm25`
* bm25 = `sum(user_weight)*1000+bm25`
* none = `1`
* wordcount = `sum(hit_count*user_weight)`
* proximity = `sum(lcs*user_weight)`
* matchany = `sum((word_count+(lcs-1)*max_lcs)*user_weight)`
* fieldmask = `field_mask`
* sph04 = `sum((4*lcs+2*(min_hit_pos==1)+exact_hit)*user_weight)*1000+bm25`


### IDF公式的配置

Manticore中历史上默认的IDF（逆文档频率）等同于 `OPTION idf='normalized,tfidf_normalized'`，这些归一化可能会导致几个不期望的效果。

首先，`idf=normalized`会导致关键词惩罚。例如，如果您搜索 `the | something`，且 `the` 出现在超过50%的文档中，那么同时包含关键词 `the` 和 `something` 的文档将比只包含关键词 `something` 的文档权重更低。使用 `OPTION idf=plain` 可以避免这种情况。

普通IDF在 `[0, log(N)]` 范围内变化，关键词不会被惩罚；而归一化IDF在 `[-log(N), log(N)]` 范围内变化，过于频繁的关键词会被惩罚。

其次，`idf=tfidf_normalized`会导致查询间IDF漂移。历史上，我们额外将IDF除以查询关键词数量，以便所有关键词的 `sum(tf*idf)` 仍然保持在 `[0,1]` 范围内。然而，这意味着查询 `word1` 和 `word1 | nonmatchingword2` 会为完全相同的结果集分配不同的权重，因为 `word1` 和 `nonmatchingword2` 的IDF都会被除以2。`OPTION idf='tfidf_unnormalized'` 修复了这个问题。请注意，一旦禁用此归一化，BM25、BM25A、BM25F()排名因子将相应缩放。

IDF标志可以混合使用；`plain` 和 `normalized` 是互斥的；`tfidf_unnormalized` 和 `tfidf_normalized` 是互斥的；而在这种互斥组中未指定的标志将采用其默认值。这意味着 `OPTION idf=plain` 等同于完整的 `OPTION idf='plain,tfidf_normalized'` 规范。

<!-- proofread -->

