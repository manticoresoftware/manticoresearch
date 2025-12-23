# 排序和排名

查询结果可以按全文排名权重、一个或多个属性或表达式进行排序。

**全文**查询默认返回按匹配度排序的结果。如果未指定，则默认按相关性排序，这相当于 SQL 格式中的 `ORDER BY weight() DESC`。

**非全文**查询默认不进行任何排序。

## 高级排序

当您在 SQL 格式中明确通过添加 `ORDER BY` 子句或通过 HTTP JSON 使用 `sort` 选项提供排序规则时，会自动启用扩展模式。

### 通过 SQL 进行排序

通用语法：
```sql
SELECT ... ORDER BY
{attribute_name | expr_alias | weight() | random() } [ASC | DESC],
...
{attribute_name | expr_alias | weight() | random() } [ASC | DESC]
```

<!--
以下示例用到的数据：

DROP TABLE IF EXISTS test;
CREATE TABLE test(a int, b int, f text);
INSERT INTO test (a, b, f) VALUES
(2, 3, 'document');
-->

<!-- example alias -->

在排序子句中，您可以使用最多 5 列的任意组合，每列后面跟 `asc` 或 `desc`。不允许将函数和表达式作为排序子句的参数，除了 `weight()` 和 `random()` 函数（后者仅能通过 SQL 的 `ORDER BY random()` 形式使用）。不过，您可以在 SELECT 列表中使用任意表达式，并按其别名排序。

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
POST /search
{
  "table": "test",
  "expressions": {
    "alias": "a+b"
  },
  "sort": {"alias":"desc"}
}
```

<!-- response JSON -->
```JSON
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "total_relation": "eq",
    "hits": [
      {
        "_id": 1,
        "_score": 1,
        "_source": {
          "a": 2,
          "b": 3,
          "f": "document",
          "alias": 5
        }
      }
    ]
  }
}
```

<!-- end -->

## 通过 JSON 进行排序

<!-- example sorting 1 -->
`"sort"` 指定一个数组，其中每个元素可以是属性名，若想按匹配权重排序，则为 `_score`，若想随机匹配顺序，则为 `_random`。在这种情况下，属性的默认排序顺序为升序，`_score` 的默认排序顺序为降序。

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
JSON 查询中也支持通过 MVA 属性进行排序。排序模式可以通过 `mode` 属性设置。支持以下模式：

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
在根据属性排序时，默认会禁用匹配权重（得分）的计算（不使用排序器）。您可以通过将 `track_scores` 属性设置为 `true` 来启用权重计算：

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

排名（也称为加权）搜索结果可以定义为一个过程，即针对每个与查询匹配的文档计算所谓的相关性（权重）。因此，相关性最终只是附加到每个文档上的一个数字，用于估计该文档与查询的相关程度。然后，搜索结果可以基于这个数字和/或一些额外参数进行排序，从而使最受关注的结果在结果页上排在更高的位置。

没有一种统一的通用方法可以在任何场景下对任何文档进行排名。而且，也不可能有这样的方式，因为相关性是*主观的*。也就是说，对你来说相关的东西，对我来说可能不相关。因此，在一般情况下，这不仅难以计算；从理论上讲是不可能的。

所以 Manticore 中的排名是可配置的。它具有所谓的**ranker**的概念。ranker 可以形式上定义为一个函数，接受一个文档和一个查询作为输入，输出一个相关性值。通俗地讲，ranker 精确控制 Manticore 使用哪种特定算法给文档分配权重。

## 内置排名器

Manticore 配备了多种适用于不同用途的内置排名器。它们中的许多使用两个因素：短语接近度（也称为 LCS）和 BM25。短语接近度基于关键字的位置，而 BM25 基于关键字的频率。从本质上讲，文档正文与查询之间的短语匹配程度越好，短语接近度越高（当文档包含整个查询作为逐字引用时达到最大值）。而当文档包含更多稀有词时，BM25 值越高。详细讨论稍后介绍。

目前实现的排名器有：

* `proximity_bm25`，默认排名模式，结合使用短语接近度和 BM25 排名。
* `bm25`，基于统计的排名模式，仅使用 BM25 排名（类似大多数其他全文引擎）。该模式速度更快，但对于包含多个关键字的查询，结果质量可能较差。
* `none`，无排名模式。此模式显然是最快的。所有匹配项的权重均赋值为1。有时称为布尔搜索，仅匹配文档但不排序。
* `wordcount`，通过关键字出现次数排名。该排名器计算每个字段关键字出现次数，然后乘以字段权重，最后求和。
* `proximity` 返回原始短语接近度值。此模式内用于模拟 `SPH_MATCH_ALL` 查询。
* `matchany` 返回先前 `SPH_MATCH_ANY` 模式计算的排名值，内用于模拟 `SPH_MATCH_ANY` 查询。
* `fieldmask` 返回一个32位掩码，第 N 位对应第 N 个全文字段，编号从0开始。仅当相应字段中存在满足查询的关键字时，该位才被设置。
* `sph04` 通常基于默认的 `proximity_bm25` 排名器，但在匹配发生于文本字段开头或结尾时，会额外提升排名。因此，如果字段与精确查询相等，`sph04` 会将其排名高于包含精确查询但不相等的字段。（例如，查询为“Hyde Park”时，标题为“Hyde Park”的文档排名应该高于标题为“Hyde Park, London”或“The Hyde Park Cafe”的文档。）
* `expr` 允许你在运行时指定排名公式。它暴露了若干内部文本因素，允许你定义如何根据这些因素计算最终权重。关于其语法和可用因素的详细介绍可见 [下面的小节](../Searching/Sorting_and_ranking.md#Quick-summary-of-the-ranking-factors)。

排名器名称不区分大小写。示例：

```sql
SELECT ... OPTION ranker=sph04;
```

## 排名因素快速总结

| 名称                    | 级别       | 类型  | 摘要                                                                                                               |
| ----------------------- | ---------- | ----- | ----------------------------------------------------------------------------------------------------------------- |
| max_lcs                 | 查询       | int   | 当前查询的最大可能 LCS 值                                                                                         |
| bm25                    | 文档       | int   | BM25(1.2, 0) 的快速估计                                                                                           |
| bm25a(k1, b)            | 文档       | int   | 具有可配置 K1、B 常数和语法支持的精确 BM25() 值                                                                  |
| bm25f(k1, b, {field=weight, ...}) | 文档 | int   | 精确的 BM25F() 值，具有额外可配置的字段权重                                                                      |
| field_mask              | 文档       | int   | 匹配字段的位掩码                                                                                                  |
| query_word_count        | 文档       | int   | 查询中包含的唯一关键字数                                                                                           |
| doc_word_count          | 文档       | int   | 文档中匹配的唯一关键字数                                                                                           |
| lcs                     | 字段       | int   | 查询与文档之间的最长公共子序列（以单词计）                                                                         |
| user_weight             | 字段       | int   | 用户字段权重                                                                                                      |
| hit_count               | 字段       | int   | 关键字出现的总次数                                                                                                |
| word_count              | 字段       | int   | 唯一匹配关键字的数量                                                                                              |
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
* `word_count`（整数），字段中匹配的唯一关键词数量。例如，如果“hello”和“world”出现在字段中的任意位置，`word_count` 将为2，无论这两个关键词出现了多少次。
* `tf_idf`（浮点数），字段中匹配的所有关键词的 TF/IDF 之和。IDF 是逆文档频率，是一个介于0和1之间的浮点值，用来描述关键词的频率（基本上，对于在每个索引文档中都出现的关键词，IDF为0；对于仅在单个文档中出现的唯一关键词，IDF为1）。TF 是词频，表示关键词在字段中匹配的出现次数。顺便提一下，`tf_idf` 实际上是通过对所有匹配出现的 IDF 求和计算的。这在结构上等同于对所有匹配关键词的 TF*IDF 求和。
* `min_hit_pos`（整数），第一个匹配关键词出现的位置，以词数计数

   因此，这是一个相对低级的、“原始的”因素，你可能需要在用它进行排名前进行*调整*。具体调整很大程度上取决于你的数据及最终公式，但这里有一些起始思路：（a）当 word_count<2 时，可以简单忽略基于 min_gaps 的任何提升；

    （b）非平凡的 min_gaps 值（即，当 word_count>=2 时）可以用某个“最坏情况”的常量限制，而平凡值（即当 min_gaps=0 且 word_count<2 时）可以用该常量替代；

    （c）可以应用类似 1/(1+min_gaps) 的传递函数（这样更好、更小的 min_gaps 值将最大化它，更差、更大的 min_gaps 值会缓慢下降）；等等。
* `lccs`（整数）。最长公共连续子序列。查询和文档之间最长的公共子短语长度，按关键词计算。

    LCCS 因子与 LCS 有些相似，但更为严格。虽然当没有两个查询词是相邻匹配时，LCS 仍可大于1，LCCS 只有在文档中存在*完全*连续的查询子短语时才大于1。例如，（one two three four five）查询与（one hundred three hundred five hundred）文档会得到 lcs=3，但 lccs=1，因为虽然3个关键词（one，three，five）在查询和文档中的相对位置匹配，但没有两个匹配位置是相邻的。

    注意，LCCS 仍然不会区分频繁和稀有关键词；对此，见 WLCCS。
* `wlccs`（浮点数）。加权最长公共连续子序列。查询和文档之间最长公共子短语中关键词 IDF 的总和。

    WLCCS 的计算方式与 LCCS 类似，但每个“合适的”关键词出现通过其关键词 IDF 增加，而不仅仅是像 LCS 和 LCCS 中加1。这使得排名能优先考虑较少见且更重要的关键词序列，而不是仅仅更长但常见的关键词序列。例如，查询 `(Zanzibar bed and breakfast)` 对于文档 `(hotels of Zanzibar)` 得到 lccs=1，但对 `(London bed and breakfast)` 得到 lccs=3，尽管“Zanzibar”实际上比整个“bed and breakfast”短语稍微稀有一些。WLCCS 因子通过使用关键词频率解决了这个问题。
* `atc`（浮点数）。聚合词项接近性。一个基于接近性的度量，当文档包含更多更紧密且更重要（稀有）的查询关键词组时，该值会增加。

    **警告：** 你应当配合 OPTION idf='plain,tfidf_unnormalized'（见[下面](../Searching/Sorting_and_ranking.md#Configuration-of-IDF-formula)）使用 ATC，否则可能得到意外结果。

    ATC 的运作大致如下。对于文档中每个关键词*出现*，我们计算所谓的*词项接近性*。为此，我们考察该出现左右两侧所有查询关键词（包括该关键词本身）最近的出现，计算距离衰减系数 k = pow(distance, -1.75)，并将衰减后的 IDF 求和。结果，对于每个关键词的每次出现，我们得到一个表示该出现“邻居”的“接近度”值。然后，将这些每次出现的接近度乘以相应关键词的 IDF，全部求和，最后取该和的对数。

换句话说，我们处理文档中最优（最近）的匹配关键词对，并计算它们的成对“接近度”为它们 IDF 的乘积乘以距离系数：

```
pair_tc = idf(pair_word1) * idf(pair_word2) * pow(pair_distance, -1.75)
```

然后将这些接近度求和，计算最终的对数衰减 ATC 值：

```
atc = log(1+sum(pair_tc))
```

注意，这个最终的衰减对数正是你应使用 OPTION idf=plain 的原因，因为如果不这样，log() 内的表达式可能为负。

关键词出现更靠近对 ATC 的贡献远远大于关键词更频繁。实际上，当关键词紧挨着时，distance=1，k=1；中间有一个词时，distance=2，k=0.297；中间有两个词时，distance=3，k=0.146，以此类推。同时，IDF 的衰减较慢。例如，在100万文档的集合中，分别匹配10、100和1000文档的关键词的IDF值是0.833、0.667和0.500。因此，两组比较稀有的关键词（各出现在10个文档中），中间有两词距离，pair_tc=0.101，勉强超过一组分别出现在100和1000个文档中，间隔一个词距离的 pair_tc=0.099。此外，两组独一无二、只出现在1个文档中的关键词，间隔三个词距离，pair_tc=0.088，会输给两组1000文档关键词紧挨着、pair_tc=0.25的情况。基本上，虽然 ATC 综合了关键词频率和接近度，但仍稍微偏重接近度。

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

