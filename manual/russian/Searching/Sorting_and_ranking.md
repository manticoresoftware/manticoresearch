# Сортировка и ранжирование

Результаты запроса можно сортировать по весу ранжирования полнотекстового поиска, одному или нескольким атрибутам или выражениям.

**Полнотекстовые** запросы по умолчанию возвращают совпадения, отсортированные по релевантности. Если ничего не указано, они сортируются по релевантности, что эквивалентно `ORDER BY weight() DESC` в SQL-формате.

**Не полнотекстовые** запросы по умолчанию не выполняют сортировку.

## Расширенная сортировка

Расширенный режим автоматически включается, когда вы явно задаёте правила сортировки, добавляя в запрос клаузы `ORDER BY` в SQL-формате или используя опцию `sort` через HTTP JSON.

### Сортировка через SQL

Общий синтаксис:
```sql
SELECT ... ORDER BY
{attribute_name | expr_alias | weight() | random() } [ASC | DESC],
...
{attribute_name | expr_alias | weight() | random() } [ASC | DESC]
```

<!-- example alias -->

В клаузе сортировки можно использовать любую комбинацию до 5 колонок, каждая из которых сопровождается `asc` или `desc`. Функции и выражения не допускаются как аргументы для клауззы сортировки, за исключением функций `weight()` и `random()` (последнюю можно использовать только через SQL в виде `ORDER BY random()`). Однако вы можете использовать любое выражение в списке SELECT и сортировать по его псевдониму.

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
Опция `"sort"` задаёт массив, в котором каждый элемент может быть именем атрибута или `_score`, если вы хотите сортировать по весам совпадений, или `_random`, если хотите случайный порядок совпадений. В этом случае порядок сортировки по умолчанию — по возрастанию для атрибутов и по убыванию для `_score`.

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
Также можно явно указать порядок сортировки:

* `asc`: сортировка по возрастанию
* `desc`: сортировка по убыванию


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
Можно также использовать другой синтаксис и указать порядок сортировки через свойство `order`:


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
В JSON-запросах также поддерживается сортировка по атрибутам MVA. Режим сортировки можно задать через свойство `mode`. Поддерживаются следующие режимы:

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
При сортировке по атрибуту вычисление веса совпадения (score) по умолчанию отключено (ранжировщик не используется). Вы можете включить вычисление веса, установив свойство `track_scores` в `true`:

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

Ранжирование (также известное как взвешивание) результатов поиска можно определить как процесс вычисления так называемой релевантности (веса) для каждого совпавшего документа относительно данного запроса, который его совпал. Таким образом, релевантность в конечном итоге — это просто число, прикреплённое к каждому документу, которое оценивает, насколько релевантен документ запросу. Затем результаты поиска могут быть отсортированы на основе этого числа и/или некоторых дополнительных параметров, чтобы наиболее востребованные результаты появлялись выше на странице результатов.

Не существует универсального стандарта для ранжирования любого документа в любой ситуации. Более того, такой способ никогда не сможет существовать, потому что релевантность — *субъективна*. То есть то, что кажется релевантным вам, может не казаться релевантным мне. Следовательно, в общем случае, это не просто трудно вычислить; это теоретически невозможно.

Поэтому ранжирование в Manticore конфигурируемо. В нём есть понятие так называемого **ранкера**. Ранкер можно формально определить как функцию, которая принимает документ и запрос на вход и выдает значение релевантности на выходе. По-простому, ранкер контролирует, как именно (с использованием какого конкретного алгоритма) Manticore будет присваивать весовые значения документам.

## Доступные встроенные ранкеры

Manticore поставляется с несколькими встроенными ранкерами, подходящими для разных целей. Многие из них используют два фактора: близость фраз (также известная как LCS) и BM25. Близость фраз работает с позициями ключевых слов, в то время как BM25 работает с частотами ключевых слов. По сути, чем лучше степень совпадения фраз между телом документа и запросом, тем выше близость фраз (она достигает максимума, когда документ содержит весь запрос дословно). А BM25 выше, когда в документе больше редких слов. Детальное обсуждение мы отложим на потом.

В настоящее время реализованы следующие ранкеры:

* `proximity_bm25` — режим ранжирования по умолчанию, который использует и комбинирует как близость фраз, так и BM25.
* `bm25` — статистический режим ранжирования, который использует только BM25 (похоже на большинство других полнотекстовых движков). Этот режим быстрее, но может ухудшить качество запросов, содержащих более одного ключевого слова.
* `none` — режим без ранжирования. Этот режим, разумеется, самый быстрый. Вес 1 присваивается всем совпадениям. Иногда это называют булевым поиском, который просто находит документы, но не ранжирует их.
* `wordcount` — ранжирование по количеству вхождений ключевых слов. Этот ранкер вычисляет количество вхождений ключевых слов в каждом поле, затем умножает их на веса полей и суммирует полученные значения.
* `proximity` возвращает чистое значение близости фраз. Этот режим внутренне используется для эмуляции запросов с `SPH_MATCH_ALL`.
* `matchany` возвращает ранг, как он вычислялся в режиме `SPH_MATCH_ANY` ранее, и также внутренне используется для эмуляции запросов с `SPH_MATCH_ANY`.
* `fieldmask` возвращает 32-битную маску, где N-й бит соответствует N-му полнотекстовому полю, нумерация с 0. Бит устанавливается, только если соответствующее поле содержит вхождения ключевых слов, удовлетворяющих запросу.
* `sph04` в целом основан на ранкере по умолчанию 'proximity_bm25', но дополнительно усиливает совпадения, если они встречаются либо в самом начале, либо в самом конце текстового поля. Таким образом, если поле равно точному запросу, `sph04` должен ранжировать его выше, чем поле, содержащее точный запрос, но не равное ему. (Например, когда запрос "Hyde Park", документ с заголовком "Hyde Park" будет иметь более высокий ранг, чем "Hyde Park, London" или "The Hyde Park Cafe".)
* `expr` позволяет указать формулу ранжирования во время выполнения. Он раскрывает несколько внутренних текстовых факторов и позволяет определить, как вычислять итоговый вес из этих факторов. Больше подробностей о его синтаксисе и список доступных факторов можно найти в [подразделе ниже](../Searching/Sorting_and_ranking.md#Quick-summary-of-the-ranking-factors).

Имя ранкера не чувствительно к регистру. Пример:

```sql
SELECT ... OPTION ranker=sph04;
```

## Краткое резюме факторов ранжирования

| Название                 | Уровень   | Тип   | Краткое описание                                                                                                    |
| ------------------------ | --------- | ----- | ------------------------------------------------------------------------------------------------------------------  |
| max_lcs                  | запрос    | int   | максимальное возможное значение LCS для текущего запроса                                                             |
| bm25                     | документ  | int   | быстрая оценка BM25(1.2, 0)                                                                                           |
| bm25a(k1, b)             | документ  | int   | точное значение BM25() с настраиваемыми константами K1, B и поддержкой синтаксиса                                   |
| bm25f(k1, b, {field=weight, ...}) | документ | int | точное значение BM25F() с дополнительной настройкой весов полей                                                      |
| field_mask               | документ  | int   | битовая маска совпавших полей                                                                                         |
| query_word_count         | документ  | int   | количество уникальных включённых ключевых слов в запросе                                                              |
| doc_word_count           | документ  | int   | количество уникальных ключевых слов, совпавших в документе                                                          |
| lcs                      | поле      | int   | Длиннейшая общая подпоследовательность между запросом и документом, в словах                                         |
| user_weight              | поле      | int   | вес пользовательского поля                                                                                            |
| hit_count                | поле      | int   | общее количество вхождений ключевых слов                                                                             |
| word_count               | поле      | int   | количество уникальных совпавших ключевых слов                                                                         |
| tf_idf                  | field     | float | сумма(tf*idf) по совпавшим ключевым словам == сумма(idf) по вхождениям                                                     |
| min_hit_pos             | field     | int   | позиция первого совпавшего вхождения, в словах, с 1                                                                  |
| min_best_span_pos       | field     | int   | позиция первого максимального LCS спана, в словах, с 1                                                                |
| exact_hit               | field     | bool  | совпадает ли запрос с полем                                                                                            |
| min_idf                 | field     | float | min(idf) по совпавшим ключевым словам                                                                                 |
| max_idf                 | field     | float | max(idf) по совпавшим ключевым словам                                                                                 |
| sum_idf                 | field     | float | сумма(idf) по совпавшим ключевым словам                                                                                |
| exact_order             | field     | bool  | совпадают ли все ключевые слова запроса а) были найдены и б) в порядке запроса                                         |
| min_gaps                | field     | int   | минимальное количество пропусков между совпавшими ключевыми словами в пределах совпадающих спанов                      |
| lccs                    | field     | int   | Наибольшая общая непрерывная подпоследовательность между запросом и документом, в словах                               |
| wlccs                   | field     | float | Взвешенная наибольшая общая непрерывная подпоследовательность, сумма(idf) по непрерывным спанам ключевых слов          |
| atc                     | field     | float | Аггрегированная близость терминов, log(1+sum(idf1*idf2*pow(distance, -1.75)) по лучшим парам ключевых слов            |

### Факторы ранжирования на уровне документа

**Фактор на уровне документа** — это числовое значение, вычисляемое движком ранжирования для каждого совпавшего документа относительно текущего запроса. Он отличается от простого атрибута документа тем, что атрибут не зависит от полного текстового запроса, а факторы могут зависеть. Эти факторы могут использоваться в любом месте выражения ранжирования. В настоящее время реализованы следующие факторы на уровне документа:

* `bm25` (целое), оценка BM25 на уровне документа (вычисляется без фильтрации вхождений ключевых слов).
* `max_lcs` (целое), максимальное возможное значение на уровне запроса, которое выражение `sum(lcs*user_weight)` может принимать. Это может быть полезно для масштабирования усиления веса. Например, формула ранжирования `MATCHANY` использует это, чтобы гарантировать, что полное совпадение фразы в любом поле ранжируется выше, чем любая комбинация частичных совпадений во всех полях.
* `field_mask` (целое), 32-битная маска совпавших полей на уровне документа.
* `query_word_count` (целое), количество уникальных ключевых слов в запросе, скорректированное с учётом количества исключённых ключевых слов. Например, запросы `(one one one one)` и `(one !two)` должны присваивать этому фактору значение 1, так как есть только одно уникальное не исключённое ключевое слово.
* `doc_word_count` (целое), количество уникальных ключевых слов, совпавших во всём документе.

### Факторы ранжирования на уровне поля

**Фактор на уровне поля** — это числовое значение, вычисляемое движком ранжирования для каждого совпавшего текстового поля документа относительно текущего запроса. Поскольку запрос может совпадать с несколькими полями, а итоговый вес должен быть одним целым числом, эти значения нужно сворачивать в одно. Для этого факторы на уровне поля могут использоваться только внутри функций агрегации по полям, их **нельзя** использовать в любом другом месте выражения. Например, нельзя использовать `(lcs+bm25)` как выражение ранжирования, так как `lcs` принимает несколько значений (по одному в каждом совпавшем поле). Следует использовать `(sum(lcs)+bm25)`, это выражение суммирует `lcs` по всем совпавшим полям, а затем добавляет `bm25` к этой сумме по полям. В настоящее время реализованы следующие факторы на уровне поля:

* `lcs` (целое), длина максимального дословного совпадения между документом и запросом, в словах. LCS означает Наибольшую Общую Подпоследовательность (или Подмножество). Минимальное значение 1, если в поле совпали только отдельные ключевые слова, максимальное — количество ключевых слов запроса, если весь запрос совпал в поле дословно (в точном порядке ключевых слов запроса). Например, если запрос 'hello world', а поле содержит эти два слова, взятые из запроса (то есть расположены рядом и именно в порядке запроса), `lcs` будет 2. Если запрос 'hello world program', а поле содержит 'hello world', `lcs` будет 2. Обратите внимание, что работает любое подмножество ключевых слов запроса, не обязательно смежных. Например, если запрос 'hello world program', а поле содержит 'hello (test program)', `lcs` также будет 2, потому что совпали 'hello' и 'program' в тех же позициях, что и в запросе. Наконец, если запрос 'hello world program', а поле содержит 'hello world program', `lcs` будет 3. (Надеюсь, это уже не удивительно.)
* `user_weight` (целое), вес, указанный пользователем для поля (см. [OPTION field_weights](../Searching/Options.md#field_weights) в SQL). По умолчанию вес равен 1, если явно не указан.
* `hit_count` (целое), количество вхождений ключевых слов, совпавших в поле. Обратите внимание, что одно ключевое слово может встречаться несколько раз. Например, если 'hello' встречается 3 раза в поле, а 'world' 5 раз, `hit_count` будет 8.
* `word_count` (целое), количество уникальных ключевых слов, совпавших в поле. Например, если 'hello' и 'world' встречаются в поле, `word_count` будет 2, независимо от количества вхождений каждого слова.
* `tf_idf` (float), сумма TF/IDF по всем ключевым словам, найденным в поле. IDF — обратная частота документов, число с плавающей точкой от 0 до 1, описывающее, насколько часто встречается ключевое слово (примерно 0 для ключевого слова, встречающегося в каждом проиндексированном документе, и 1 для уникального ключевого слова, встречающегося только в одном документе). TF — частота термина, количество совпадений ключевого слова в поле. Кстати, `tf_idf` фактически вычисляется как сумма IDF по всем найденным совпадениям. Это по сути эквивалентно суммированию TF*IDF по всем совпавшим ключевым словам.
* `min_hit_pos` (integer), позиция первого найденного совпадения ключевого слова, считается в словах

   Следовательно, это относительно низкоуровневый, «сырой» фактор, который вы, вероятно, захотите *отрегулировать* перед использованием его для ранжирования. Конкретные настройки сильно зависят от ваших данных и итоговой формулы, но вот несколько идей для начала: (a) любые бусты, основанные на min_gaps, могут просто игнорироваться, если word_count < 2;

    (b) нетривиальные значения min_gaps (то есть когда word_count >= 2) могут быть ограничены определенной «наихудшей» константой, а тривиальные значения (то есть когда min_gaps = 0 и word_count < 2) могут быть заменены этой константой;

    (c) может применяться функция преобразования, например 1/(1+min_gaps) (чтобы лучшие, меньшие значения min_gaps максимизировали её, а худшие, большие значения min_gaps уменьшали плавно); и так далее.
* `lccs` (integer). Longest Common Contiguous Subsequence — наиболее длинная общая непрерывная подпоследовательность. Длина самой длинной подфразы, общей для запроса и документа, вычисленная в ключевых словах.

    Фактор LCCS похож на LCS, но более строгий. В то время как LCS может быть больше 1, даже если ни два слова запроса не стоят рядом друг с другом, LCCS будет больше 1 только если в документе есть *точные*, непрерывные подфразы запроса. Например, запрос (one two three four five) и документ (one hundred three hundred five hundred) дадут lcs=3, но lccs=1, поскольку, хотя взаимное расположение 3 ключевых слов (one, three, five) совпадает между запросом и документом, ни две совпадающие позиции на самом деле не являются смежными.

    Обратите внимание, что LCCS по-прежнему не различает частотные и редкие ключевые слова; для этого см. WLCCS.
* `wlccs` (float). Weighted Longest Common Contiguous Subsequence — взвешенная наиболее длинная общая непрерывная подпоследовательность. Сумма IDF ключевых слов самой длинной подфразы, общей для запроса и документа.

    WLCCS вычисляется аналогично LCCS, но каждое «подходящее» вхождение ключевого слова увеличивает её на IDF ключевого слова, а не просто на 1 (как в LCS и LCCS). Это позволяет ранжировать выше последовательности более редких и важных ключевых слов, чем последовательности часто встречающихся ключевых слов, даже если последние длиннее. Например, запрос `(Zanzibar bed and breakfast)` даст lccs=1 для документа `(hotels of Zanzibar)`, но lccs=3 для `(London bed and breakfast)`, хотя «Zanzibar» на самом деле несколько реже, чем вся фраза «bed and breakfast». Фактор WLCCS решает эту проблему, используя частоты ключевых слов.
* `atc` (float). Aggregate Term Closeness — агрегированная близость терминов. Мера близости, которая увеличивается, если в документе содержится больше групп более близко расположенных и более важных (редких) ключевых слов из запроса.

    **ПРЕДУПРЕЖДЕНИЕ:** следует использовать ATC с OPTION idf='plain,tfidf_unnormalized' (см. [ниже](../Searching/Sorting_and_ranking.md#Configuration-of-IDF-formula)); иначе могут возникнуть неожиданные результаты.

    ATC работает таким образом. Для каждого *вхождения* ключевого слова в документе мы вычисляем так называемую *близость термина*. Для этого рассматриваем все другие ближайшие вхождения всех ключевых слов запроса (включая само ключевое слово) слева и справа от данного вхождения, вычисляем коэффициент затухания расстояния как k = pow(distance, -1.75) для этих вхождений и суммируем затухающие IDF. В результате для каждого вхождения каждого ключевого слова получаем значение «близости», описывающее «соседей» этого вхождения. Затем множим эти значения близости для каждого вхождения на соответствующий IDF ключевого слова, суммируем всё это и в конце вычисляем логарифм от суммы.

Проще говоря, мы обрабатываем лучшие (ближайшие) пары совпавших ключевых слов в документе и вычисляем попарные «близости» как произведение их IDF, умноженное на коэффициент расстояния:

```
pair_tc = idf(pair_word1) * idf(pair_word2) * pow(pair_distance, -1.75)
```

Затем суммируем такие близости и вычисляем итоговое логарифмическое затухание ATC:

```
atc = log(1+sum(pair_tc))
```

Обратите внимание, что именно из-за этого конечного логарифмического затухания вы должны использовать OPTION idf=plain, потому что без него выражение внутри log() могло бы быть отрицательным.

Более близкое расположение ключевых слов вносит *гораздо* больший вклад в ATC, чем более частотные ключевые слова. Действительно, когда ключевые слова расположены рядом, distance=1 и k=1; если между ними одно слово, distance=2 и k=0.297, при двух словах между distance=3 и k=0.146 и т.д. При этом IDF затухает несколько медленнее. Например, в коллекции из миллиона документов IDF ключевых слов, совпадающих в 10, 100 и 1000 документов соответственно, равны примерно 0.833, 0.667 и 0.500. Таким образом, пара ключевых слов с двумя достаточно редкими словами, каждое из которых встречается только в 10 документах и между которыми два слова, даст pair_tc = 0.101 и едва превзойдет пару из ключевых слов, которые встречаются в 100 и 1000 документах, с одним словом между ними и pair_tc = 0.099. Более того, пара из двух *уникальных* ключевых слов (по одному документу на каждое) с тремя словами между ними получит pair_tc = 0.088 и уступит паре из двух ключевых слов, встречающихся в 1000 документах, расположенных вплотную и дающих pair_tc = 0.25. Таким образом, хотя ATC и сочетает оба фактора — частоту ключевых слов и их близость, он по-прежнему несколько отдает предпочтение близости.

### Функции агрегации факторов ранжирования

**Функция агрегации поля** — это функция с одним аргументом, которая принимает выражение с факторами уровня поля, перебирает все совпавшие поля и вычисляет итоговый результат. В настоящее время реализованы следующие функции агрегации поля:

* `sum`, которая суммирует аргументное выражение по всем совпавшим полям. Например, `sum(1)` должна возвращать количество совпавших полей.
* `top`, который возвращает наибольшее значение аргумента по всем совпадающим полям.
* `max_window_hits`, управляет скользящим окном позиций попаданий для отслеживания максимального количества попаданий в пределах заданного размера окна. Он удаляет устаревшие попадания, выходящие за пределы окна, и добавляет последнее попадание, обновляя максимальное количество попаданий, найденных в этом окне.

### Формулы выражений для всех встроенных ранжировщиков

Большинство других ранжировщиков на самом деле можно эмулировать с помощью ранжировщика на основе выражений. Вам просто нужно предоставить соответствующее выражение. Хотя такая эмуляция, вероятно, будет медленнее, чем использование встроенного, скомпилированного ранжировщика, она может быть интересна, если вы хотите тонко настроить свою формулу ранжирования, начиная с одной из существующих. Кроме того, формулы описывают детали ранжировщика ясно и читаемо.

* proximity_bm25 (ранжировщик по умолчанию) = `sum(lcs*user_weight)*1000+bm25`
* bm25 = `sum(user_weight)*1000+bm25`
* none = `1`
* wordcount = `sum(hit_count*user_weight)`
* proximity = `sum(lcs*user_weight)`
* matchany = `sum((word_count+(lcs-1)*max_lcs)*user_weight)`
* fieldmask = `field_mask`
* sph04 = `sum((4*lcs+2*(min_hit_pos==1)+exact_hit)*user_weight)*1000+bm25`


### Конфигурация формулы IDF

Исторически стандартный IDF (обратная частота документа) в Manticore эквивалентен `OPTION idf='normalized,tfidf_normalized'`, и эти нормализации могут вызывать несколько нежелательных эффектов.

Во-первых, `idf=normalized` вызывает штрафование ключевых слов. Например, если вы ищете `the | something` и `the` встречается более чем в 50% документов, то документы с обоими ключевыми словами `the` и `something` получат меньший вес, чем документы только с одним ключевым словом `something`. Использование `OPTION idf=plain` избегает этого.

Plain IDF варьируется в диапазоне `[0, log(N)]`, и ключевые слова никогда не штрафуются; в то время как нормализованный IDF варьируется в диапазоне `[-log(N), log(N)]`, и слишком частые ключевые слова штрафуются.

Во-вторых, `idf=tfidf_normalized` вызывает дрейф IDF между запросами. Исторически мы дополнительно делили IDF на количество ключевых слов в запросе, чтобы сумма `sum(tf*idf)` по всем ключевым словам оставалась в диапазоне `[0,1]`. Однако это означает, что запросы `word1` и `word1 | nonmatchingword2` будут присваивать разные веса точно одному и тому же набору результатов, потому что IDF для `word1` и `nonmatchingword2` будет делиться на 2. `OPTION idf='tfidf_unnormalized'` исправляет это. Обратите внимание, что факторы ранжирования BM25, BM25A, BM25F() будут масштабироваться соответствующим образом после отключения этой нормализации.

Флаги IDF могут комбинироваться; `plain` и `normalized` взаимоисключающие; `tfidf_unnormalized` и `tfidf_normalized` взаимоисключающие; и неуказанные флаги в такой взаимоисключающей группе принимают значения по умолчанию. Это означает, что `OPTION idf=plain` эквивалентен полной спецификации `OPTION idf='plain,tfidf_normalized'`.

<!-- proofread -->

