# 排序和排名

查询结果可以按照全文排名权重、一个或多个属性或表达式进行排序。

**全文**查询返回匹配项默认按顺序排序。如果没有指定排序方式，则默认按相关性排序，这等同于SQL格式中的`ORDER BY weight() DESC`。

**非全文**查询默认不进行任何排序。

## 高级排序

当您显式提供排序规则时，扩展模式会自动启用。这可以通过在SQL格式中添加`ORDER BY`子句或通过HTTP JSON使用`sort`选项来实现。

### 通过SQL排序

通用语法：
```sql
SELECT ... ORDER BY
{attribute_name | expr_alias | weight() | random() } [ASC | DESC],
...
{attribute_name | expr_alias | weight() | random() } [ASC | DESC]
```

<!-- example alias -->

在排序子句中，可以使用最多5列的任意组合，每列后跟`asc`或`desc`。排序子句不允许使用函数和表达式作为参数，除非是`weight()`和`random()`函数（后者只能通过SQL以`ORDER BY random()`的形式使用）。但是，可以在SELECT列表中使用任何表达式并按其别名排序。

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
`"sort"`指定一个数组，其中每个元素可以是属性名称或`_score`（如果您想按匹配权重排序）或`_random`（如果您想按随机匹配顺序排序）。在这种情况下，默认情况下，属性的排序顺序为升序，而`_score`的排序顺序为降序。

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

* `asc`：按升序排序
* `desc`：按降序排序


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
您还可以使用另一种语法并通过`order`属性指定排序顺序：


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
JSON查询中也支持按MVA属性排序。排序模式可以通过`mode`属性设置。支持以下模式：

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
在按属性排序时，默认情况下匹配权重（得分）计算被禁用（不使用排名器）。可以通过将`track_scores`属性设置为`true`来启用权重计算：

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


## 排名概述

搜索结果的排名（也称为加权）可以定义为针对每个与给定查询匹配的文档计算所谓的相关性（权重）的过程。因此，相关性最终只是附加在每个文档上的一个数字，用以估计该文档与查询的相关程度。然后可以基于该数字和/或一些额外参数对搜索结果进行排序，使得最受欢迎的结果会出现在结果页的更高位置。

没有一种单一的、适用于所有场景的标准文档排名方法。而且，也不可能有这样一种方法，因为相关性是*主观*的。也就是说，对你来说相关的内容，可能对我来说不相关。因此，在一般情况下，不仅计算起来困难；从理论上讲也是不可能的。

所以，Manticore 中的排名是可配置的。它有一个所谓的**ranker**（排名器）的概念。排名器可以形式化定义为一个接受文档和查询作为输入，并输出相关性值的函数。通俗来说，排名器控制着 Manticore 将如何（使用哪种具体算法）给文档分配权重。

## 内置排名器

Manticore 提供了几种适用于不同用途的内置排名器。它们中的许多使用两个因素：短语接近度（也称为LCS）和BM25。短语接近度根据关键词位置工作，而BM25根据关键词频率工作。基本上，文档和查询之间的短语匹配程度越高，短语接近度越高（当文档包含查询的完整逐字引用时达到最大值）。而当文档包含更多罕见词时，BM25分值越高。详细讨论稍后进行。

目前实现的排名器有：

* `proximity_bm25`，默认的排名模式，同时使用并结合了短语接近度和BM25排名。
* `bm25`，一种只使用BM25的统计排名模式（类似大多数其他全文引擎）。该模式速度更快，但对于包含多个关键词的查询可能导致质量较差。
* `none`，无排名模式。该模式显然最快。所有匹配项均赋值权重1。有时称为布尔搜索，只匹配文档但不排序。
* `wordcount`，按关键词出现次数排名。该排名器计算每个字段的关键词出现次数，再乘以字段权重，最后求和。
* `proximity` 返回原始的短语接近值作为结果。该模式在内部用于模拟 `SPH_MATCH_ALL` 查询。
* `matchany` 返回在早期 `SPH_MATCH_ANY` 模式中计算的排名值，并在内部用于模拟 `SPH_MATCH_ANY` 查询。
* `fieldmask` 返回一个32位掩码，第N位对应第N个全文字段，编号从0开始。只有当相应字段含有满足查询的关键词出现时，该位才会被置位。
* `sph04` 基本上基于默认的 `proximity_bm25` 排名器，但额外提升当匹配发生在文本字段开头或结尾的情况。因此，如果某字段完全等于查询，`sph04` 应该将其排名高于包含该查询但不完全相等的字段。（例如，当查询为“Hyde Park”时，标题为“Hyde Park”的文档应排名高于标题为“Hyde Park, London”或“The Hyde Park Cafe”的文档。）
* `expr` 允许你在运行时指定排名公式。它暴露了多个内部文本因素，允许你定义如何从这些因素计算最终权重。你可以在[下方小节](../Searching/Sorting_and_ranking.md#Quick-summary-of-the-ranking-factors)中找到其语法细节和可用因素参考。

排名器名称不区分大小写。示例：

```sql
SELECT ... OPTION ranker=sph04;
```

## 排名因素快速总结

| Name                    | Level     | Type  | Summary                                                                                                            |
| ----------------------- | --------- | ----- | ------------------------------------------------------------------------------------------------------------------ |
| max_lcs                 | query     | int   | 当前查询的最大可能LCS值                                                                                              |
| bm25                    | document  | int   | BM25(1.2, 0) 的快速估算值                                                                                            |
| bm25a(k1, b)            | document  | int   | 精确的BM25()值，支持配置K1、B常数和语法                                                                             |
| bm25f(k1, b, {field=weight, ...}) | document | int   | 精确的BM25F()值，额外支持配置字段权重                                                                               |
| field_mask              | document  | int   | 匹配字段的位掩码                                                                                                    |
| query_word_count        | document  | int   | 查询中唯一包含的关键字数量                                                                                            |
| doc_word_count          | document  | int   | 文档中匹配的唯一关键字数量                                                                                            |
| lcs                     | field     | int   | 查询与文档之间的最长公共子序列，按词数计                                                                             |
| user_weight             | field     | int   | 用户字段权重                                                                                                        |
| hit_count               | field     | int   | 关键字出现的总次数                                                                                                  |
| word_count              | field     | int   | 唯一匹配关键字的数量                                                                                                |
| tf_idf                  | field     | float | 匹配关键字的tf*idf之和 == 所有出现的idf之和                                                                          |
| min_hit_pos             | field     | int   | 首次匹配出现位置，按词数，1起始                                                                                        |
| min_best_span_pos       | field     | int   | 首个最大LCS跨度位置，按词数，1起始                                                                                    |
| exact_hit               | field     | bool  | 查询是否与字段完全相等                                                                                              |
| min_idf                 | field     | float | 匹配关键字中最小的idf                                                                                                |
| max_idf                 | field     | float | 匹配关键字中最大的idf                                                                                                |
| sum_idf                 | field     | float | 匹配关键字的idf之和                                                                                                |
| exact_order             | field     | bool  | 是否所有查询关键字a)均匹配且b)顺序与查询中一致                                                                     |
| min_gaps                | field     | int   | 匹配跨度中匹配关键字之间的最小间隙数                                                                                  |
| lccs                    | field     | int   | 查询与文档之间的最长连续公共子序列，按词数计                                                                          |
| wlccs                   | field     | float | 加权最长连续公共子序列，连续关键字跨度上的idf之和                                                                    |
| atc                     | field     | float | 聚合词项接近度，log(1+匹配关键字最佳对的 idf1*idf2*pow(distance, -1.75)之和)                                        |

注意：对于使用**短语**、**邻近**或**NEAR**操作符并包含超过31个关键字的查询，依赖词频的排名因子（如 `tf`、`idf`、`bm25`、`hit_count`、`word_count`）可能会对第31个及以后位置的关键字计数不足。这是由于内部使用32位掩码来跟踪这些复杂操作符中的词项出现情况所致。

### 文档级排名因子

**文档级因子**是排名引擎针对当前查询为每个匹配文档计算的数值。所以它不同于单纯的文档属性，后者不依赖于全文查询，而因子可能依赖。这些因子可以在排名表达式的任何位置使用。目前已实现的文档级因子有：

* `bm25`（整数），文档级BM25估算值（不考虑关键字出现次数过滤）。
* `max_lcs`（整数），一个查询级的最大可能值，`sum(lcs*user_weight)`表达式可达到的最大值。这对于权重提升的缩放很有用。例如，`MATCHANY`排序公式使用该值保证任意字段的完整短语匹配排名高于所有字段的部分匹配组合。
* `field_mask`（整数），文档级的32位匹配字段掩码。
* `query_word_count`（整数），查询中唯一关键字的数量，已调整排除关键词数。例如，查询 `(one one one one)` 和 `(one !two)` 都应赋值为1，因为仅有一个唯一非排除关键字。
* `doc_word_count`（整数），整个文档中匹配的唯一关键字数量。

### 字段级排名因子

一个**字段级因子**是排名引擎针对当前查询计算的每个匹配文档内文本字段的数值。由于一个查询可以匹配不止一个字段，但最终权重需要是一个单一的整数值，这些值需要被合并成一个。为此，字段级因子只能在字段聚合函数中使用，**不能**在表达式的其他任何地方使用。例如，你不能使用 `(lcs+bm25)` 作为你的排名表达式，因为 `lcs` 对每个匹配字段会有多个值。你应该使用 `(sum(lcs)+bm25)`，该表达式会对所有匹配字段中的 `lcs` 求和，然后加上 `bm25`。当前实现的字段级因子有：

* `lcs` (整数)，文档和查询之间最长精确匹配的长度，按单词计数。LCS 代表最长公共子序列（或子集）。当字段中只匹配了零散关键词时取最小值 1，当字段完全逐字匹配整个查询时取最大值，即查询关键词数量（且词序完全相同）。例如，若查询是 'hello world' 且字段包含这两个词按查询顺序相邻出现，则 `lcs` 为 2。再如，查询为 'hello world program'，字段包含 'hello world'，则 `lcs` 为 2。注意，匹配的关键词可以是查询关键词的任意子集，而不必是相邻关键词的子集。例如，若查询为 'hello world program'，字段包含 'hello (test program)'，`lcs` 也为 2，因为 'hello' 和 'program' 分别在字段中与查询位置对应匹配。最后，若查询为 'hello world program'，字段包含 'hello world program'，则 `lcs` 为 3。（此时应该不意外了。）
* `user_weight` (整数)，用户指定的每字段权重（参考 SQL 中的 [OPTION field_weights](../Searching/Options.md#field_weights)）。若未显式指定，权重默认为 1。
* `hit_count` (整数)，字段中匹配到的关键词出现次数。注意，单个关键词可出现多次。例如，若字段中 'hello' 出现了 3 次，'world' 出现了 5 次，`hit_count` 为 8。
* `word_count` (整数)，字段中匹配到的不同关键词数量。例如，若字段中出现 'hello' 和 'world'，`word_count` 为 2，不论它们出现次数多少。
* `tf_idf` (浮点数)，字段中所有匹配关键词的 TF/IDF 之和。IDF 表示逆文档频率，是 0 到 1 之间的浮点值，描述关键词的频率（基本上，出现于每个索引文档的关键词 IDF 为 0，而只出现于单一文档的关键词 IDF 为 1）。TF 是术语频率，字段中匹配关键词出现次数。顺带一提，`tf_idf` 实际是通过对所有匹配出现累加 IDF 计算的。按照构造方法，这等价于对所有匹配关键词累加 TF*IDF。
* `min_hit_pos` (整数)，第一个匹配关键词出现的位置，按单词计数。

   因此，这是一个相对低层次的“原始”因子，通常你会想在使用它进行排名之前对它进行*调整*。具体调整高度依赖你的数据和最终公式，但这里先给几个思路：（a）当 `word_count<2` 时，可以简单忽略任何基于 `min_gaps` 的提升；

    （b）当 `word_count>=2` 且 `min_gaps` 非平凡时，可以用某个“最坏情况”的常量限制它，而平凡值（即 `min_gaps=0` 且 `word_count<2`）可以被该常量替代；

    （c）可以应用诸如 `1/(1+min_gaps)` 的传递函数（使得越好、越小的 `min_gaps` 取值该函数值越大，而更差、越大的 `min_gaps` 会缓慢下降）；等等。
* `lccs` (整数)，最长公共连续子序列。查询和文档之间最长公共子短语的长度，按关键词计数。

    LCCS 因子与 LCS 有些相似，但限制更严格。虽然 LCS 可以大于 1，即使没有两个查询词是相邻匹配，LCCS 只有当文档中存在*精确*、连续的查询子短语时才会大于 1。例如，查询为（one two three four five），文档为（one hundred three hundred five hundred）时，`lcs=3`，但 `lccs=1`，因为虽然 3 个关键词（one, three, five）的排列对应查询中一致，但没有两个匹配词位置是相邻的。

    注意，LCCS 仍然不区分频繁和稀有关键词；相关内容参见 WLCCS。
* `wlccs` (浮点数)，加权最长公共连续子序列。查询和文档之间最长公共子短语的关键词 IDF 之和。

    WLCCS 的计算类似于 LCCS，但每个“合适”的关键词匹配会增加该关键词的 IDF，而不是像 LCS 和 LCCS 那样仅加 1。这允许排名系统将更稀有且重要的关键词序列排名高于更频繁的关键词序列，即使后者较长。例如，查询 `(Zanzibar bed and breakfast)` 对文档 `(hotels of Zanzibar)` 产生 `lccs=1`，但对 `(London bed and breakfast)` 产生 `lccs=3`，尽管 “Zanzibar” 实际上比整个 “bed and breakfast” 短语更稀有。WLCCS 因子通过使用关键词频率解决了此问题。
* `atc` (浮点数)，聚合术语接近度（Aggregate Term Closeness）。一种基于邻近的度量，当文档包含更多且更紧密且更重要（稀有）的查询关键词组合时，该值增加。

    **警告：** 你应该在 OPTION `idf='plain,tfidf_unnormalized'` 下使用 ATC（参见[下文](../Searching/Sorting_and_ranking.md#Configuration-of-IDF-formula)）；否则可能得到意外的结果。

    ATC 的基本操作如下。对于文档中的每个关键词 *出现*，我们计算所谓的 *项接近度*。为此，我们检查所有查询关键词（包括关键词本身）在主题出现左侧和右侧的所有最近的其他出现，计算这些出现的距离衰减系数 k = pow(distance, -1.75)，然后对这些衰减的逆文档频率（IDF）求和。结果，对于每个关键词的每个出现，我们获得一个“接近度”值，描述该出现的“邻居”。然后，我们将这些每个出现的接近度乘以其相应的主题关键词IDF，将它们全部相加，最后计算该和的对数。

换句话说，我们处理文档中最佳（最近）匹配的关键词对，并计算它们的“接近度”作为它们的IDF按距离系数缩放的乘积：

```
pair_tc = idf(pair_word1) * idf(pair_word2) * pow(pair_distance, -1.75)
```

然后，我们对这些接近度求和，并计算最终的对数衰减ATC值：

```
atc = log(1+sum(pair_tc))
```

请注意，这个最终的对数衰减正是你应该使用 OPTION idf=plain 的原因，因为如果没有它，对数内部的表达式可能是负数。

更接近的关键词出现对ATC的贡献 *远多* 于更频繁的关键词。事实上，当关键词紧挨着时，距离=1且k=1；当它们之间只有一个词时，距离=2且k=0.297，两个词之间时，距离=3且k=0.146，依此类推。同时，IDF的衰减速度较慢。例如，在一个100万文档集合中，匹配10、100和1000个文档的关键词的IDF值分别为0.833、0.667和0.500。因此，一对两个相对罕见的关键词，每个出现10次，但中间有2个词，其pair_tc = 0.101，几乎无法超过一对一个出现100次和一个出现1000次的关键词，中间只有一个词，其pair_tc = 0.099。此外，一对两个 *唯一* 的1次文档关键词，中间有3个词，其pair_tc = 0.088，会输给一对两个1000次文档关键词紧挨着出现，其pair_tc = 0.25。因此，虽然ATC确实结合了关键词频率和接近度，但它仍然更倾向于接近度。

### 排序因子聚合函数

**字段聚合函数**是一个单参数函数，接受字段级因子的表达式，遍历所有匹配的字段，并计算最终结果。目前实现的字段聚合函数包括：

* `sum`，将参数表达式在所有匹配的字段上相加。例如 `sum(1)` 应该返回匹配字段的数量。
* `top`，返回参数在所有匹配字段中的最高值。
* `max_window_hits`，管理一个滑动窗口的命中位置，跟踪指定窗口大小内的最大命中数。它会移除窗口外的过时命中，并添加最新的命中，更新窗口内找到的最大命中数。

### 所有内置排序器的公式表达式

大多数其他排序器实际上可以通过基于表达式的排序器来模拟。你只需要提供一个适当的表达式即可。虽然这种模拟可能比使用内置编译排序器慢，但如果你希望从现有的排序器之一开始微调你的排序公式，这仍然可能很有趣。此外，公式以清晰可读的方式描述了排序器的细节。

* proximity_bm25（默认排序器）= `sum(lcs*user_weight)*1000+bm25`
* bm25 = `sum(user_weight)*1000+bm25`
* none = `1`
* wordcount = `sum(hit_count*user_weight)`
* proximity = `sum(lcs*user_weight)`
* matchany = `sum((word_count+(lcs-1)*max_lcs)*user_weight)`
* fieldmask = `field_mask`
* sph04 = `sum((4*lcs+2*(min_hit_pos==1)+exact_hit)*user_weight)*1000+bm25`


### IDF公式的配置

Manticore历史上默认的IDF（逆文档频率）等同于 OPTION idf='normalized,tfidf_normalized'，而这些规范化可能会导致一些不希望的效果。

首先，`idf=normalized`会导致关键词惩罚。例如，如果你搜索 `the | something`，而 `the` 出现在超过50%的文档中，那么包含两个关键词 `the` 和 `[something` 的文档将比只包含一个关键词 `something` 的文档权重更低。使用 OPTION idf=plain 可以避免这种情况。

Plain IDF在 `[0, log(N)]` 范围内变化，关键词永远不会被惩罚；而normalized IDF在 `[-log(N), log(N)]` 范围内变化，过于频繁的关键词会被惩罚。

其次，`idf=tfidf_normalized`会导致IDF在查询之间漂移。历史上，我们还额外将IDF除以查询关键词数量，使得所有关键词的 `sum(tf*idf)` 仍然适合 `[0,1]` 范围。然而，这意味着查询 `word1` 和 `word1 | nonmatchingword2` 会对完全相同的返回集分配不同的权重，因为 `word1` 和 `nonmatchingword2` 的IDF都会被除以2。`OPTION idf='tfidf_unnormalized'`可以修复这个问题。请注意，一旦禁用这种规范化，BM25、BM25A、BM25F() 排序因子将相应地进行缩放。

IDF标志可以混合；`plain` 和 `normalized` 是互斥的；`tfidf_unnormalized` 和 `tfidf_normalized` 也是互斥的；在这样的互斥组中未指定的标志将采用默认值。这意味着 OPTION idf=plain 等同于完整的 OPTION idf='plain,tfidf_normalized' 指定。

<!-- proofread -->

