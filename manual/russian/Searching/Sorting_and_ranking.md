# Сортировка и ранжирование

Результаты запроса могут быть отсортированы по весу полнотекстового ранжирования, одному или нескольким атрибутам или выражениям.

**Полнотекстовые** запросы возвращают совпадения, отсортированные по умолчанию. Если ничего не указано, они сортируются по релевантности, что эквивалентно `ORDER BY weight() DESC` в формате SQL.

**Не полнотекстовые** запросы по умолчанию не выполняют сортировку.

## Расширенная сортировка

Расширенный режим автоматически включается, когда вы явно указываете правила сортировки, добавляя клаузу `ORDER BY` в формате SQL или используя опцию `sort` через HTTP JSON.

### Сортировка через SQL

Общий синтаксис:
```sql
SELECT ... ORDER BY
{attribute_name | expr_alias | weight() | random() } [ASC | DESC],
...
{attribute_name | expr_alias | weight() | random() } [ASC | DESC]
```

<!-- example alias -->

В клаузе сортировки вы можете использовать любую комбинацию до 5 колонок, каждая из которых сопровождается `asc` или `desc`. Функции и выражения не допускаются в качестве аргументов для клаузулы сортировки, за исключением функций `weight()` и `random()` (последняя может использоваться только через SQL в виде `ORDER BY random()`). Однако вы можете использовать любое выражение в списке SELECT и сортировать по его алиасу.

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

## Сортировка через JSON

<!-- example sorting 1 -->
`"sort"` задаёт массив, где каждый элемент может быть названием атрибута или `_score`, если вы хотите сортировать по весам совпадений, или `_random`, если хотите случайный порядок совпадений. В этом случае порядок сортировки по умолчанию — по возрастанию для атрибутов и по убыванию для `_score`.

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
Вы также можете явно указать порядок сортировки:

* `asc`: сортировать по возрастанию
* `desc`: сортировать по убыванию


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
Вы также можете использовать другой синтаксис и задать порядок сортировки через свойство `order`:


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
Сортировка по MVA-атрибутам также поддерживается в JSON-запросах. Режим сортировки можно задать через свойство `mode`. Поддерживаются следующие режимы:

* `min`: сортировка по минимальному значению
* `max`: сортировка по максимальному значению

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
При сортировке по атрибуту вычисление веса совпадения (score) по умолчанию отключено (ранжировщик не используется). Вы можете включить вычисление веса, установив свойство `track_scores` в значение `true`:

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


## Обзор ранжирования

Ранжирование (также известное как взвешивание) результатов поиска можно определить как процесс вычисления так называемой релевантности (веса) для каждого совпадающего документа относительно данного запроса, который с ним совпал. Итак, релевантность — в конечном итоге просто число, прикрепленное к каждому документу, которое оценивает, насколько документ релевантен запросу. Результаты поиска затем могут быть отсортированы на основе этого числа и/или некоторых дополнительных параметров, так что наиболее востребованные результаты будут отображаться выше на странице результатов.

Не существует единого универсального способа ранжирования любого документа в любой ситуации. Более того, такого способа вообще не может существовать, потому что релевантность *субъективна*. То есть, что кажется релевантным вам, может не казаться релевантным мне. Следовательно, в общем случае, это не только трудно вычислить; это теоретически невозможно.

Поэтому ранжирование в Manticore настраивается. Существует понятие так называемого **ранжировщика**. Ранжировщик формально можно определить как функцию, которая принимает на вход документ и запрос, и выдаёт значение релевантности на выходе. Проще говоря, ранжировщик контролирует, как именно (с помощью какого конкретного алгоритма) Manticore присвоит веса документам.

## Доступные встроенные ранжировщики

Manticore поставляется с несколькими встроенными ранжировщиками, подходящими для разных целей. Многие из них используют два фактора: близость фраз (также известная как LCS) и BM25. Близость фраз работает с позициями ключевых слов, а BM25 — с частотами ключевых слов. По сути, чем лучше степень совпадения фразы между телом документа и запросом, тем выше близость фраз (она достигает максимума, когда документ содержит весь запрос как дословную цитату). И BM25 выше, когда документ содержит больше редких слов. Подробное обсуждение оставим на потом.

В настоящее время реализованы следующие ранжировщики:

* `proximity_bm25` — режим ранжирования по умолчанию, который использует и комбинирует как близость фраз, так и ранжирование BM25.
* `bm25` — статистический режим ранжирования, который использует только BM25 (аналогично большинству других полнотекстовых движков). Этот режим быстрее, но может привести к ухудшению качества для запросов, содержащих более одного ключевого слова.
* `none` — режим без ранжирования. Этот режим, очевидно, самый быстрый. Вес 1 присваивается всем совпадениям. Иногда это называют булевым поиском, который просто находит документы, но не ранжирует их.
* `wordcount` — ранжирование по количеству вхождений ключевых слов. Этот ранжировщик вычисляет количество вхождений ключевых слов по каждому полю, затем умножает их на веса полей и суммирует полученные значения.
* `proximity` возвращает сырое значение близости фраз в качестве результата. Этот режим внутренне используется для эмуляции запросов `SPH_MATCH_ALL`.
* `matchany` возвращает ранг, как он вычислялся ранее в режиме `SPH_MATCH_ANY`, и внутренне используется для эмуляции запросов `SPH_MATCH_ANY`.
* `fieldmask` возвращает 32-битную маску, где N-й бит соответствует N-му полнотекстовому полю, нумерация начинается с 0. Бит будет установлен только тогда, когда в соответствующем поле есть какие-либо вхождения ключевых слов, удовлетворяющих запросу.
* `sph04` в целом основан на ранжировщике по умолчанию 'proximity_bm25', но дополнительно повышает рейтинг совпадений, если они встречаются в самом начале или в самом конце текстового поля. Таким образом, если поле полностью равно точному запросу, `sph04` должен ранжировать его выше, чем поле, которое содержит точный запрос, но не равно ему. (Например, когда запрос — "Hyde Park", документ с заголовком "Hyde Park" должен иметь более высокий ранг, чем документ с заголовком "Hyde Park, London" или "The Hyde Park Cafe".)
* `expr` позволяет указать формулу ранжирования во время выполнения. Он открывает несколько внутренних текстовых факторов и позволяет определить, как из этих факторов вычислять итоговый вес. Подробнее о его синтаксисе и справочник доступных факторов можно найти в [подразделе ниже](../Searching/Sorting_and_ranking.md#Quick-summary-of-the-ranking-factors).

Имя ранжировщика не чувствительно к регистру. Пример:

```sql
SELECT ... OPTION ranker=sph04;
```

## Краткое резюме факторов ранжирования

| Имя                     | Уровень   | Тип   | Краткое описание                                                                                                |
| ----------------------- | --------- | ----- | ----------------------------------------------------------------------------------------------------------------|
| max_lcs                 | запрос    | int   | максимальное возможное значение LCS для текущего запроса                                                         |
| bm25                    | документ  | int   | быстрая оценка BM25(1.2, 0)                                                                                       |
| bm25a(k1, b)            | документ  | int   | точное значение BM25() с настраиваемыми константами K1, B и поддержкой синтаксиса                                |
| bm25f(k1, b, {field=weight, ...}) | документ | int   | точное значение BM25F() с дополнительными настраиваемыми весами полей                                            |
| field_mask              | документ  | int   | битовая маска совпавших полей                                                                                      |
| query_word_count        | документ  | int   | количество уникальных включённых ключевых слов в запросе                                                          |
| doc_word_count          | документ  | int   | количество уникальных ключевых слов, совпавших в документе                                                        |
| lcs                     | поле      | int   | наиболее длинная общая подпоследовательность между запросом и документом, в словах                                 |
| user_weight             | поле      | int   | пользовательский вес поля                                                                                           |
| hit_count               | поле      | int   | общее количество вхождений ключевых слов                                                                          |
| word_count              | поле      | int   | количество уникальных совпавших ключевых слов                                                                      |
| tf_idf                  | field     | float | сумма(tf*idf) по совпавшим ключевым словам == сумма(idf) по вхождениям                                           |
| min_hit_pos             | field     | int   | позиция первого совпавшего вхождения в словах, с 1                                                            |
| min_best_span_pos       | field     | int   | позиция первого максимального LCS-диапазона в словах, с 1                                                       |
| exact_hit               | field     | bool  | совпадает ли запрос с полем                                                                                     |
| min_idf                 | field     | float | мин(idf) по совпавшим ключевым словам                                                                          |
| max_idf                 | field     | float | макс(idf) по совпавшим ключевым словам                                                                          |
| sum_idf                 | field     | float | сумма(idf) по совпавшим ключевым словам                                                                         |
| exact_order             | field     | bool  | совпали ли все ключевые слова запроса и идут ли они в запросном порядке                                         |
| min_gaps                | field     | int   | минимальное число разрывов между совпавшими ключевыми словами в диапазонах совпадения                            |
| lccs                    | field     | int   | Наибольшая общая непрерывная подпоследовательность между запросом и документом, в словах                          |
| wlccs                   | field     | float | Взвешенная наибольшая общая непрерывная подпоследовательность, сумма(idf) по непрерывным диапазонам ключевых слов|
| atc                     | field     | float | Аггрегированная близость терминов, log(1+sum(idf1*idf2*pow(distance, -1.75)) по лучшим парам слов                |

### Факторы ранжирования на уровне документа

**Фактор на уровне документа** — это числовое значение, рассчитываемое ранжировщиком для каждого совпавшего документа относительно текущего запроса. Он отличается от обычного атрибута документа тем, что атрибут не зависит от полного текстового запроса, а факторы зависят. Эти факторы могут использоваться в любом месте выражения ранжирования. В настоящее время реализованы следующие факторы на уровне документа:

* `bm25` (integer) — оценка BM25 на уровне документа (вычисляется без фильтрации по вхождениям ключевых слов).
* `max_lcs` (integer) — максимальное возможное значение на уровне запроса, которое может принимать выражение `sum(lcs*user_weight)`. Это может быть полезно для масштабирования усиления веса. Например, формула ранжировщика `MATCHANY` использует это, чтобы гарантировать, что полное совпадение фразы в любом поле будет ранжироваться выше, чем любая комбинация частичных совпадений во всех полях.
* `field_mask` (integer) — 32-битная маска на уровне документа для совпавших полей.
* `query_word_count` (integer) — количество уникальных ключевых слов в запросе с учётом количества исключённых ключевых слов. Например, и запросы `(one one one one)` и `(one !two)` должны присваивать значение 1 этому фактору, поскольку уникальных не исключённых ключевых слов всего одно.
* `doc_word_count` (integer) — количество уникальных ключевых слов, совпавших во всём документе.

### Факторы ранжирования на уровне поля

**Фактор на уровне поля** — числовое значение, вычисляемое ранжировщиком для каждого совпавшего текстового поля документа относительно текущего запроса. Поскольку может совпасть более одного поля, но итоговый вес должен быть одним числом, эти значения необходимо сворачивать в одно. Для этого факторы на уровне поля можно использовать только внутри функций агрегации по полям и **нельзя** использовать их в выражении отдельно. Например, выражение `(lcs+bm25)` использовать нельзя, так как `lcs` имеет несколько значений (одно на каждое совпавшее поле). Следует использовать `(sum(lcs)+bm25)`, которая суммирует `lcs` по всем совпавшим полям, а затем прибавляет `bm25` к этой сумме. В настоящее время реализованы следующие факторы на уровне поля:

* `lcs` (integer) — длина максимального точного совпадения между документом и запросом, в словах. LCS означает Longest Common Subsequence (наибольшая общая подпоследовательность). Минимальное значение — 1, если совпал только отдельный ключевой термин, максимум — число ключевых слов запроса, если весь запрос совпал в поле дословно (в точном порядке ключевых слов запроса). Например, если запрос 'hello world', а поле содержит эти два слова, взятых из запроса (то есть рядом и в том же порядке), `lcs` будет 2. Если запрос 'hello world program', а поле содержит 'hello world', `lcs` будет 2. Обратите внимание, что работает любой поднабор ключевых слов запроса, не обязательно подряд идущих. Например, если запрос 'hello world program', а поле содержит 'hello (test program)', `lcs` также будет 2, потому что совпали 'hello' и 'program' в соответствующих позициях запроса. И наконец, если запрос 'hello world program' и поле содержит 'hello world program', `lcs` будет 3. (Надеюсь, это уже не удивительно.)
* `user_weight` (integer) — пользовательский вес для поля (см. [OPTION field_weights](../Searching/Options.md#field_weights) в SQL). Вес по умолчанию равен 1, если явно не задан.
* `hit_count` (integer) — число вхождений ключевых слов, совпавших в поле. Учтите, что одно ключевое слово может встречаться несколько раз. Например, если 'hello' встречается 3 раза в поле, а 'world' — 5 раз, `hit_count` будет 8.
* `word_count` (integer) — количество уникальных ключевых слов, совпавших в поле. Например, если 'hello' и 'world' встречаются в поле, `word_count` будет 2, независимо от количества их вхождений.
* `tf_idf` (float), сумма TF/IDF по всем ключевым словам, найденным в поле. IDF — обратная частота документа, числовое значение с плавающей точкой от 0 до 1, описывающее, насколько часто встречается ключевое слово (по сути, 0 для ключевого слова, которое встречается в каждом проиндексированном документе, и 1 для уникального ключевого слова, встречающегося только в одном документе). TF — частота термина, количество совпадений ключевого слова в поле. Отметим, что `tf_idf` вычисляется суммированием IDF по всем совпадениям. По конструкции это эквивалентно суммированию TF*IDF по всем найденным ключевым словам.
* `min_hit_pos` (integer), позиция первого совпадения ключевого слова, считающаяся в словах

   Следовательно, это относительно низкоуровневый, «сырой» фактор, который, вероятно, потребуется *подстроить* перед использованием для ранжирования. Конкретные корректировки зависят во многом от ваших данных и итоговой формулы, но вот несколько идей для начала: (a) любые бусты, основанные на min_gaps, можно просто игнорировать, если word_count<2;

    (b) нетривиальные значения min_gaps (то есть, когда word_count>=2) можно ограничить определённой «худшей» константой, а тривиальные значения (то есть, когда min_gaps=0 и word_count<2) заменить на эту константу;

    (c) можно применить функцию преобразования, например, 1/(1+min_gaps) (чтобы лучшие, меньшие значения min_gaps максимизировали её, а худшие, большие значения медленно снижали); и так далее.
* `lccs` (integer). Длина самой длинной общей непрерывной подпоследовательности. Длина самой длинной общей подфразы между запросом и документом, вычисляемая в ключевых словах.

    Фактор LCCS отчасти похож на LCS, но более ограничительный. В то время как LCS может быть больше 1, даже если никакие две слова запроса не расположены рядом, LCCS будет больше 1 только если в документе есть *точные*, непрерывные подфразы запроса. Например, запрос (one two three four five) и документ (one hundred three hundred five hundred) дадут lcs=3, но lccs=1, потому что хотя взаимная позиция трёх ключевых слов (one, three, five) совпадает между запросом и документом, ни две совпадающие позиции фактически не являются соседними.

    Обратите внимание, что LCCS по-прежнему не различает частотные и редкие ключевые слова; для этого см. WLCCS.
* `wlccs` (float). Взвешенная длина самой длинной общей непрерывной подпоследовательности. Сумма IDF ключевых слов самой длинной общей подфразы между запросом и документом.

    WLCCS вычисляется аналогично LCCS, но каждое «подходящее» вхождение ключевого слова увеличивает значение на IDF этого ключевого слова, а не просто на 1 (как в LCS и LCCS). Это позволяет ранжировать последовательности более редких и важных ключевых слов выше, чем последовательности частых ключевых слов, даже если последние длиннее. Например, запрос `(Zanzibar bed and breakfast)` даст lccs=1 для документа `(hotels of Zanzibar)`, но lccs=3 для `(London bed and breakfast)`, несмотря на то, что «Zanzibar» на самом деле несколько реже всей фразы «bed and breakfast». Фактор WLCCS решает эту проблему, используя частоты ключевых слов.
* `atc` (float). Aggregate Term Closeness (агрегированная близость терминов). Метрика, основанная на близости позиций; увеличивается, когда документ содержит больше групп более близко расположенных и более важных (редких) ключевых слов из запроса.

    **ВНИМАНИЕ:** следует использовать ATC с OPTION idf='plain,tfidf_unnormalized' (см. [ниже](../Searching/Sorting_and_ranking.md#Configuration-of-IDF-formula)); иначе могут быть неожиданные результаты.

    ATC работает следующим образом. Для каждого вхождения ключевого слова в документе вычисляется так называемая *близость термина*. Для этого рассматриваются все другие ближайшие вхождения всех ключевых слов запроса (включая само ключевое слово) слева и справа от текущего вхождения, рассчитывается коэффициент затухания расстояния как k = pow(distance, -1.75) для этих вхождений и суммируются затухающие IDF. В итоге для каждого вхождения каждого ключевого слова получается значение «близости», описывающее «соседей» этого вхождения. Затем эти значения близости домножаются на соответствующий IDF ключевого слова, суммируются, и в конце вычисляется логарифм этой суммы.

Иными словами, мы обрабатываем лучшие (ближайшие) пары совпавших ключевых слов в документе и вычисляем парные «близости» как произведение их IDF, масштабированное коэффициентом расстояния:

```
pair_tc = idf(pair_word1) * idf(pair_word2) * pow(pair_distance, -1.75)
```

Далее суммируем такие близости и вычисляем итоговое значение ATC с логарифмическим затуханием:

```
atc = log(1+sum(pair_tc))
```

Обратите внимание, что именно этот финальный затухающий логарифм является причиной, по которой следует использовать OPTION idf=plain, поскольку без него выражение внутри log() может быть отрицательным.

Более близкие вхождения ключевых слов дают *гораздо* больший вклад в ATC, чем более частые ключевые слова. Действительно, когда ключевые слова находятся непосредственно рядом, distance=1 и k=1; если между ними одно слово, distance=2 и k=0.297; при двух словах между ними distance=3 и k=0.146 и так далее. В то же время IDF затухает медленнее. Например, для коллекции из миллиона документов значения IDF для ключевых слов, совпадающих в 10, 100 и 1000 документах, будут соответственно 0.833, 0.667 и 0.500. Таким образом, пара ключевых слов с двумя достаточно редкими ключевыми словами, встречающимися по 10 документов, но с двумя словами между ними, даст pair_tc = 0.101 и едва превзойдёт пару с ключевыми словами из 100 и 1000 документов с одним словом между ними и pair_tc = 0.099. Более того, пара из двух *уникальных*, вызывающих появление в одном документе ключевых слов с тремя словами между ними даст pair_tc = 0.088 и уступит паре двух ключевых слов из 1000 документов, расположенных рядом и дающих pair_tc = 0.25. Так что, в сущности, хотя ATC объединяет и частоту ключевых слов, и близость, он всё же несколько отдаёт предпочтение близости.

### Функции агрегации факторов ранжирования

**Функция агрегации по полю** — это функция с одним аргументом, принимающая выражение со факторами на уровне поля, проходящая по всем совпавшим полям и вычисляющая итоговый результат. В настоящее время реализованы следующие функции агрегации по полю:

* `sum`, которая суммирует выражение по всем совпавшим полям. Например, `sum(1)` должна возвращать количество совпавших полей.
* `top`, который возвращает наибольшее значение аргумента по всем сопоставленным полям.
* `max_window_hits`, управляет скользящим окном позиций хитов для отслеживания максимального количества хитов в пределах указанного размера окна. Он удаляет устаревшие хиты, выходящие за пределы окна, и добавляет последний хит, обновляя максимальное количество хитов, найденных в этом окне.

### Формульные выражения для всех встроенных ранжеров

Большинство других ранжеров на самом деле можно эмулировать, используя ранжер на основе выражений. Вам просто нужно предоставить соответствующее выражение. Хотя эта эмуляция, вероятно, будет медленнее, чем использование встроенного, скомпилированного ранжера, она все же может быть интересна, если вы хотите тонко настроить свою формулу ранжирования, начиная с одной из существующих. Кроме того, формулы ясно и читаемо описывают детали ранжера.

* proximity_bm25 (ранжер по умолчанию) = `sum(lcs*user_weight)*1000+bm25`
* bm25 = `sum(user_weight)*1000+bm25`
* none = `1`
* wordcount = `sum(hit_count*user_weight)`
* proximity = `sum(lcs*user_weight)`
* matchany = `sum((word_count+(lcs-1)*max_lcs)*user_weight)`
* fieldmask = `field_mask`
* sph04 = `sum((4*lcs+2*(min_hit_pos==1)+exact_hit)*user_weight)*1000+bm25`


### Настройка формулы IDF

Исторически стандартным IDF (обратной документной частотой) в Manticore является `OPTION idf='normalized,tfidf_normalized'`, и эти нормализации могут привести к нескольким нежелательным эффектам.

Во-первых, `idf=normalized` вызывает штрафование ключевых слов. Например, если вы ищете `the | something` и `the` встречается более чем в 50% документов, то документы с обоими ключевыми словами `the` и `something` будут иметь меньший вес, чем документы только с ключевым словом `something`. Использование `OPTION idf=plain` избегает этого.

Plain IDF варьируется в диапазоне `[0, log(N)]`, и ключевые слова никогда не штрафуются; в то время как нормализованная IDF варьируется в диапазоне `[-log(N), log(N)]`, и слишком частотные ключевые слова штрафуются.

Во-вторых, `idf=tfidf_normalized` вызывает дрейф IDF между запросами. Исторически мы дополнительно делили IDF на количество ключевых слов в запросе, чтобы сумма `sum(tf*idf)` по всем ключевым словам оставалась в диапазоне `[0,1]`. Однако это означает, что запросы `word1` и `word1 | nonmatchingword2` присваивали разные веса абсолютно одинаковому набору результатов, потому что IDF как для `word1`, так и для `nonmatchingword2` делились на 2. `OPTION idf='tfidf_unnormalized'` исправляет это. Обратите внимание, что ранжирующие факторы BM25, BM25A, BM25F() будут соответственно масштабированы, как только вы отключите эту нормализацию.

Флаги IDF могут смешиваться; `plain` и `normalized` являются взаимоисключающими; `tfidf_unnormalized` и `tfidf_normalized` также взаимоисключающие; и неуказанные флаги в такой взаимоисключающей группе принимают свои значения по умолчанию. Это означает, что `OPTION idf=plain` эквивалентен полной спецификации `OPTION idf='plain,tfidf_normalized'`.

<!-- proofread -->

