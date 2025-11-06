# 排序与排名

查询结果可以按全文排名权重、一个或多个属性或表达式进行排序。

**全文**查询默认返回已排序的匹配结果。如果未指定排序方式，则按相关性排序，这相当于SQL格式中的`ORDER BY weight() DESC`。

**非全文**查询默认不进行任何排序。

## 高级排序

当您通过在SQL格式中添加`ORDER BY`子句或通过HTTP JSON使用`sort`选项显式提供排序规则时，会自动启用扩展模式。

### 通过SQL排序

通用语法：
```sql
SELECT ... ORDER BY
{attribute_name | expr_alias | weight() | random() } [ASC | DESC],
...
{attribute_name | expr_alias | weight() | random() } [ASC | DESC]
```

<!-- example alias -->

在排序子句中，您最多可以使用多达5个列的任意组合，每个列后面跟`asc`或`desc`。除`weight()`和`random()`函数（后者只能通过SQL以`ORDER BY random()`形式使用）外，不允许使用函数和表达式作为排序子句的参数。但是，您可以在SELECT列表中使用任何表达式，并按其别名进行排序。

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

## 通过JSON排序

<!-- example sorting 1 -->
`"sort"`指定一个数组，其中每个元素可以是属性名，或者如果您想按匹配权重排序则为`_score`，如果您想随机匹配顺序则为`_random`。在这种情况下，属性的排序顺序默认为升序，而`_score`默认为降序。

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
您还可以使用另一种语法，通过`order`属性指定排序顺序：


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
JSON查询也支持按照MVA属性排序。排序模式可以通过`mode`属性设置。支持以下模式：

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
当对属性进行排序时，匹配权重（得分）计算默认被禁用（不使用排名器）。您可以通过将`track_scores`属性设置为`true`来启用权重计算：

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

搜索结果的排名（也称为加权）可以定义为针对给定查询匹配的每个文档计算所谓的相关性（权重）的过程。因此，相关性最终只是附加在每个文档上的一个数字，用以估计该文档与查询的相关程度。然后，可以基于这个数字和/或一些额外参数对搜索结果进行排序，使得最受欢迎的结果出现在结果页的更高位置。

没有一种适用于任何场景中任何文档的单一标准的通用排名方法。而且，实际上也不可能有这样的方法，因为相关性是*主观的*。换句话说，对你来说看似相关的内容，对我来说可能就不相关。因此，在一般情况下，计算相关性不仅困难，从理论上讲甚至是不可能的。

所以 Manticore 中的排名是可配置的。它有一个所谓的**ranker**（排序器）的概念。排序器可以形式化定义为一个函数，该函数将文档和查询作为输入，输出一个相关性值。通俗来说，排序器控制 Manticore 使用哪种具体算法给文档分配权重。

## 内置的可用排序器

Manticore 自带多个内置排序器，适用于不同的目的。许多排序器使用两个因素：词组邻近度（也称为最长公共子序列 LCS）和 BM25。词组邻近度基于关键词的位置，而 BM25 基于关键词的频率。基本上，文档正文与查询之间词组匹配的程度越高，词组邻近度越高（当文档包含完整的查询作为逐字引用时，词组邻近度达到最大）。而 BM25 在文档包含更多罕见词时更高。详细的讨论留到后面。

当前实现的排序器有：

* `proximity_bm25`，默认的排名模式，结合使用词组邻近度和 BM25 排名。
* `bm25`，一种统计排名模式，仅使用 BM25 排名（类似于大多数其他全文引擎）。此模式更快，但对于包含多个关键词的查询，排名质量可能较差。
* `none`，无排名模式。此模式显然是最快的。所有匹配分配权重为1。有时称作布尔搜索，它仅匹配文档但不进行排名。
* `wordcount`，按关键词出现次数排名。该排序器计算每个字段的关键词出现次数，乘以字段权重后求和。
* `proximity` 返回词组邻近度的原始值。此模式内部用于模拟 `SPH_MATCH_ALL` 查询。
* `matchany` 返回之前在 `SPH_MATCH_ANY` 模式下计算的排名，内部用于模拟 `SPH_MATCH_ANY` 查询。
* `fieldmask` 返回一个 32 位掩码，第 N 位对应第 N 个全文字段，字段编号从0开始。仅当对应字段有满足查询的关键词出现时，该位才置位。
* `sph04` 基于默认的 `proximity_bm25` 排序器，但额外提升文本字段开头或结尾出现匹配的权重。因此，如果某字段等于确切查询，`sph04` 应将其排名高于包含该确切查询但不完全等于它的字段。（例如，当查询为“Hyde Park”，文档标题为“Hyde Park”应排名高于标题为“Hyde Park, London”或“The Hyde Park Cafe”的文档。）
* `expr` 允许你在运行时指定排名公式。它暴露了若干内部文本因素，允许你定义最终权重如何从这些因素计算。关于其语法及可用因素参考，见[下文小节](../Searching/Sorting_and_ranking.md#Quick-summary-of-the-ranking-factors)。

排序器名称不区分大小写。例如：

```sql
SELECT ... OPTION ranker=sph04;
```

## 排名因素快速总结

| 名称                    | 级别     | 类型  | 说明                                                                                                            |
| ----------------------- | --------- | ----- | ------------------------------------------------------------------------------------------------------------------ |
| max_lcs                 | query     | int   | 当前查询的最大可能 LCS 值                                                                                      |
| bm25                    | document  | int   | BM25(1.2, 0) 的快速估计值                                                                                      |
| bm25a(k1, b)            | document  | int   | 精确的 BM25() 值，可配置 K1、B 常量及其语法支持                                                               |
| bm25f(k1, b, {field=weight, ...}) | document | int   | 精确的 BM25F() 值，带额外可配置字段权重                                                                       |
| field_mask              | document  | int   | 匹配字段的位掩码                                                                                               |
| query_word_count        | document  | int   | 查询中唯一包含的关键词数量                                                                                      |
| doc_word_count          | document  | int   | 文档中匹配的唯一关键词数量                                                                                      |
| lcs                     | field     | int   | 查询和文档之间最长公共子序列（以词计算）                                                                        |
| user_weight             | field     | int   | 用户字段权重                                                                                                   |
| hit_count               | field     | int   | 关键词出现的总次数                                                                                              |
| word_count              | field     | int   | 匹配的唯一关键词数                                                                                              |
| tf_idf                  | field     | float | 匹配关键词的 sum(tf*idf) == 出现次数的 sum(idf)                                                     |
| min_hit_pos             | field     | int   | 第一个匹配出现位置，按词计，基于1                                                               |
| min_best_span_pos       | field     | int   | 第一个最大LCS区间位置，按词计，基于1                                                                 |
| exact_hit               | field     | bool  | 查询是否等于字段内容                                                                                             |
| min_idf                 | field     | float | 匹配关键词的最小 idf                                                                                     |
| max_idf                 | field     | float | 匹配关键词的最大 idf                                                                                     |
| sum_idf                 | field     | float | 匹配关键词的 idf 总和                                                                                     |
| exact_order             | field     | bool  | 是否所有查询关键词均 a) 匹配且 b) 符合查询顺序                                                   |
| min_gaps                | field     | int   | 匹配区间中匹配关键词之间的最小间隙数                                        |
| lccs                    | field     | int   | 查询和文档之间的最长连续公共子序列（按词计）                                         |
| wlccs                   | field     | float | 加权最长连续公共子序列，连续关键词区间的 sum(idf)                            |
| atc                     | field     | float | 聚合词项接近度，log(1+sum(idf1*idf2*pow(distance, -1.75))，对关键词最佳匹配对计算               |

### 文档级排名因子

**文档级因子** 是排名引擎针对当前查询为每个匹配文档计算的数值。它不同于普通的文档属性，后者不依赖完整文本查询，而因子可能依赖。这些因子可以在排名表达式中任意使用。目前实现的文档级因子包括：

* `bm25`（整数），文档级 BM25 估值（计算时不做关键词出现过滤）。
* `max_lcs`（整数），查询级最大可能值，即表达式 `sum(lcs*user_weight)` 可以达到的最大值。该值对于权重提升缩放非常有用。例如，`MATCHANY` 排序器的公式使用该值来保证任何字段中的完整短语匹配排名高于所有字段部分匹配的组合。
* `field_mask`（整数），文档级匹配字段的32位掩码。
* `query_word_count`（整数），查询中唯一关键词数，经过排除关键词调整。例如，查询 `(one one one one)` 和 `(one !two)` 都应赋值为1，因为只有一个唯一的非排除关键词。
* `doc_word_count`（整数），文档中匹配的唯一关键词数量。

### 字段级排名因子

**字段级因子** 是排名引擎针对当前查询为文档中每个匹配文本字段计算的数值。由于一个查询可以匹配多个字段，但最终权重需为单一整数，因此这些值需要合并成单一值。为此，字段级因子只能在字段聚合函数内使用，**不能**在表达式中任意使用。例如，不能使用 `(lcs+bm25)` 作为排名表达式，因为 `lcs` 对每个匹配字段取多值。应使用 `(sum(lcs)+bm25)`，该表达式将所有匹配字段的 `lcs` 求和后再加上 `bm25`。目前实现的字段级因子包括：

* `lcs`（整数），文档与查询之间的最长逐字匹配长度，按词计算。LCS 代表最长公共子序列（或子集）。当字段中仅匹配离散关键词时，取最小值1；当字段完整匹配查询且顺序完全一致时，取最大值（查询关键词数目）。例如，查询为“hello world”，字段包含这两个相邻且顺序一致的查询词，则 `lcs` 为2。再比如，查询是“hello world program”，字段包含“hello world”，则 `lcs` 为2。注意，任何子集均可，而非仅限连贯子集。例如，查询“hello world program”，字段包含“hello (test program)”，同样得到 `lcs=2`，因为“hello”和“program”各自在原查询位置匹配。最后，如果查询是“hello world program”，字段包含“hello world program”，则 `lcs` 为3。（这点应该不难理解。）
* `user_weight`（整数），用户指定的每字段权重（参见 SQL 的 [OPTION field_weights](../Searching/Options.md#field_weights)）。默认权重为1。
* `hit_count`（整数），字段中匹配关键词出现的总次数。注意单个关键词可多次出现。例如，若 “hello” 出现3次，“world” 出现5次，则 `hit_count` 为8。
* `word_count`（整数），字段中匹配的唯一关键词数。例如，若字段出现 “hello” 和 “world”，则 `word_count` 为2，不论它们出现次数多少。
* `tf_idf`（浮点数），字段中所有匹配关键词的TF/IDF之和。IDF是逆文档频率，是介于0和1之间的浮点值，描述关键词的出现频率（基本上，对于出现在每个索引文档中的关键词为0，对于只出现在单个文档中的唯一关键词为1）。TF是词频，表示字段中匹配关键词出现的次数。顺便说一句，`tf_idf` 实际上是通过对所有匹配出现次数的IDF求和计算的。这从构造上等价于对所有匹配关键词的TF*IDF求和。
* `min_hit_pos`（整数），第一个匹配关键词出现的位置，按单词计数

   因此，这是一个相对低级的“原始”因子，在用于排序前你可能想对其进行*调整*。具体调整高度依赖于你的数据和最终公式，不过这里有几个初步思路：（a）当 word_count<2 时，可以简单忽略基于 min_gaps 的任何提升；

    （b）非平凡的 min_gaps 值（即当 word_count>=2 时）可以用某个“最坏情况”的常数进行限制，而平凡值（即当 min_gaps=0 且 word_count<2 时）可以用该常数替代；

    （c）可以应用类似于 1/(1+min_gaps) 的传递函数（这样更好、更小的 min_gaps 值会使其最大化，而较差、更大的 min_gaps 值则会缓慢下降）；等等。
* `lccs`（整数）。最长公共连续子序列。查询和文档之间共有的最长子短语的长度，以关键词计。

    LCCS 因子与 LCS 有点相似但更严格。虽然 LCS 即使查询中没有两个词是相邻匹配的，值也可以大于1，但 LCCS 只有当文档中存在*精确*、连续的查询子短语时才会大于1。例如，查询（one two three four five）与文档（one hundred three hundred five hundred）的匹配会产生 lcs=3，但 lccs=1，因为尽管三个关键词（one、three、five）的相互位置在查询和文档中匹配，但没有两个匹配位置是相邻的。

    注意 LCCS 仍未区分频繁和稀有关键词；有关这方面，请参见 WLCCS。
* `wlccs`（浮点数）。加权最长公共连续子序列。查询和文档之间共有的最长子短语的关键词IDF之和。

    WLCCS 的计算方法类似于 LCCS，但每个“合适”的关键词出现都会按关键词IDF增加，而不仅仅是增加1（如同 LCS 和 LCCS）。这允许将包含较稀有、较重要关键词的序列排序置于包含频繁关键词但序列更长的序列之上。例如，查询 `(Zanzibar bed and breakfast)` 针对文档 `(hotels of Zanzibar)` 会产生 lccs=1，但针对 `(London bed and breakfast)` 会产生 lccs=3，尽管“Zanzibar”实际上比整个“bed and breakfast”短语要稀有。WLCCS 因子通过使用关键词频率解决了这个问题。
* `atc`（浮点数）。聚合词语接近度。基于接近度的度量，当文档包含更多、位置更接近、且更重要（稀有）的查询关键词组时，该值增加。

    **警告：** 你应当在 OPTION idf='plain,tfidf_unnormalized'（详见[下文](../Searching/Sorting_and_ranking.md#Configuration-of-IDF-formula)）的条件下使用 ATC；否则，可能得到意外结果。

    ATC 基本上按如下方式运行。对于文档中的每个关键词*出现*，我们计算所谓的*词语接近度*。为此，我们检查该出现左右两侧所有最近的所有查询关键词的其他出现（包含该关键词本身），计算距离的衰减系数 k = pow(distance, -1.75)，并对IDF进行加权求和。结果是，对于每个关键词的每个出现，我们获得一个描述该出现“邻居”的“接近度”值。然后，我们用各自关键词的IDF乘以这些每出现的接近度，将它们求和，最后对求和结果取对数。

换句话说，我们处理文档中最佳（最近）的匹配关键词对，并计算它们的成对“接近度”为它们IDF的乘积再乘以距离系数：

```
pair_tc = idf(pair_word1) * idf(pair_word2) * pow(pair_distance, -1.75)
```

然后我们将这样的接近度求和，并计算最终的对数衰减ATC值：

```
atc = log(1+sum(pair_tc))
```

注意，最终衰减对数正是你应当使用 OPTION idf=plain 的原因，因为没有它的话，log() 内的表达式可能为负。

关键词出现越接近，对 ATC 的贡献越*大*，远超关键词出现频率的影响。实际上，当两个关键词紧邻时，distance=1，k=1；当它们之间仅有一个词时，distance=2，k=0.297，两个词时，distance=3，k=0.146，依此类推。与此同时，IDF 衰减较慢。例如，在包含100万文档的集合中，匹配在10、100和1000文档中的关键词对应的IDF分别是0.833、0.667和0.500。因此，一对出现于仅10个文档中的两个稀有关键词但之间有2个词的关键词对的 pair_tc=0.101，几乎压过了包含100文档关键词和1000文档关键词，且中间有1个词的 pair_tc=0.099。此外，一对两个*唯一*出现于1个文档的关键词，中间有3词，pair_tc=0.088，会输给一对两个1000文档关键词紧邻出现的 pair_tc=0.25。基本上，ATC 综合了关键词频率和接近度，但它仍稍稍偏好接近度。

### 排序因子聚合函数

**字段聚合函数** 是只接受一个表达式（该表达式包含字段级因子），对所有匹配字段迭代并计算最终结果的单参数函数。目前已实现的字段聚合函数包括：

* `sum`，对所有匹配字段的参数表达式求和。例如，`sum(1)` 应返回匹配字段的数量。
* `top`，返回所有匹配字段中参数的最高值。
* `max_window_hits`，管理一个命中位置的滑动窗口，用于跟踪在指定窗口大小内的最大命中次数。它会移除过期的、超出窗口范围的命中，并添加最新命中，更新该窗口内找到的最大命中数。

### 所有内置排序器的公式表达式

大多数其他排序器实际上可以通过基于表达式的排序器来模拟。您只需提供一个合适的表达式。虽然这种模拟很可能比使用内置的、编译的排序器更慢，但如果您想以现有的排序器之一为基础微调排序公式，这仍可能很有意义。此外，这些公式以清晰、可读的方式描述了排序器的细节。

* proximity_bm25（默认排序器）= `sum(lcs*user_weight)*1000+bm25`
* bm25 = `sum(user_weight)*1000+bm25`
* none = `1`
* wordcount = `sum(hit_count*user_weight)`
* proximity = `sum(lcs*user_weight)`
* matchany = `sum((word_count+(lcs-1)*max_lcs)*user_weight)`
* fieldmask = `field_mask`
* sph04 = `sum((4*lcs+2*(min_hit_pos==1)+exact_hit)*user_weight)*1000+bm25`


### IDF公式的配置

Manticore历史默认的IDF（逆文档频率）等同于 `OPTION idf='normalized,tfidf_normalized'`，而这些归一化可能导致若干不希望的效果。

首先，`idf=normalized`会对关键词进行惩罚。例如，如果您搜索 `the | something` 并且`the`出现在超过50%的文档中，那么同时包含关键词`the`和`something`的文档权重会低于只包含关键词`something`的文档。使用 `OPTION idf=plain` 可以避免这个问题。

Plain IDF的取值范围为 `[0, log(N)]`，关键词永远不会被惩罚；而归一化IDF的取值范围为 `[-log(N), log(N)]`，频繁出现的关键词会被惩罚。

其次，`idf=tfidf_normalized` 会导致IDF在查询间漂移。历史上，我们额外将IDF除以查询关键词数，这样所有关键词的 `sum(tf*idf)` 才能保持在 `[0,1]` 范围内。然而，这意味着查询 `word1` 和 `word1 | nonmatchingword2` 对完全相同的结果集分配了不同的权重，因为 `word1` 和 `nonmatchingword2` 的IDF都会被除以2。`OPTION idf='tfidf_unnormalized'` 可以修正这个问题。注意，一旦您禁用该归一化，BM25、BM25A、BM25F() 排序因子将相应缩放。

IDF 标记可以混合使用；`plain` 和 `normalized` 互相排斥；`tfidf_unnormalized` 和 `tfidf_normalized` 互相排斥；在此类互斥组中未指定的标记会采用默认值。也就是说，`OPTION idf=plain` 等同于完整的 `OPTION idf='plain,tfidf_normalized'` 指定。

<!-- proofread -->

