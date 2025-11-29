# Сортировка и ранжирование

Результаты запроса могут быть отсортированы по весу полнотекстового ранжирования, одному или нескольким атрибутам или выражениям.

**Полнотекстовые** запросы возвращают совпадения, отсортированные по умолчанию. Если ничего не указано, они сортируются по релевантности, что эквивалентно `ORDER BY weight() DESC` в формате SQL.

**Не полнотекстовые** запросы по умолчанию не выполняют никакой сортировки.

## Расширенная сортировка

Расширенный режим автоматически включается, когда вы явно задаёте правила сортировки, добавляя к запросу `ORDER BY` в формате SQL или используя опцию `sort` через HTTP JSON.

### Сортировка через SQL

Общий синтаксис:
```sql
SELECT ... ORDER BY
{attribute_name | expr_alias | weight() | random() } [ASC | DESC],
...
{attribute_name | expr_alias | weight() | random() } [ASC | DESC]
```

<!-- example alias -->

В клаузе сортировки можно использовать любую комбинацию до 5 столбцов, за каждым из которых следует `asc` или `desc`. Функции и выражения не допускаются в качестве аргументов для клаузулы сортировки, за исключением функций `weight()` и `random()` (последняя может использоваться только через SQL в виде `ORDER BY random()`). Однако вы можете использовать любое выражение в списке SELECT и сортировать по его псевдониму.

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

## Сортировка через JSON

<!-- example sorting 1 -->
`"sort"` задаёт массив, где каждый элемент может быть именем атрибута или `_score`, если вы хотите сортировать по весам совпадений, или `_random`, если хотите случайный порядок совпадений. В этом случае порядок сортировки по умолчанию — по возрастанию для атрибутов и по убыванию для `_score`.

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
Вы также можете использовать другой синтаксис и указать порядок сортировки через свойство `order`:


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

Ранжирование (также известное как взвешивание) результатов поиска можно определить как процесс вычисления так называемой релевантности (веса) для каждого найденного документа относительно данного запроса, который его сопоставил. Таким образом, релевантность — это, в конечном счёте, просто число, прикреплённое к каждому документу, которое оценивает, насколько документ релевантен запросу. Результаты поиска затем могут быть отсортированы на основе этого числа и/или некоторых дополнительных параметров, чтобы наиболее востребованные результаты появлялись выше на странице результатов.

Не существует единого универсального способа ранжирования любого документа в любой ситуации. Более того, такого способа никогда не может быть, потому что релевантность *субъективна*. То есть то, что кажется релевантным вам, может не казаться релевантным мне. Следовательно, в общих случаях это не просто трудно вычислить; это теоретически невозможно.

Поэтому ранжирование в Manticore настраивается. В нём есть понятие так называемого **ранкера**. Ранкер формально можно определить как функцию, которая принимает на вход документ и запрос и выдаёт значение релевантности на выходе. Простыми словами, ранкер контролирует, как именно (с помощью какого конкретного алгоритма) Manticore будет присваивать веса документам.

## Доступные встроенные ранкеры

Manticore поставляется с несколькими встроенными ранкерами, подходящими для разных целей. Многие из них используют два фактора: близость фраз (также известную как LCS) и BM25. Близость фраз работает с позициями ключевых слов, в то время как BM25 — с частотами ключевых слов. По сути, чем лучше степень совпадения фразы между телом документа и запросом, тем выше близость фраз (она достигает максимума, когда документ содержит весь запрос как дословную цитату). А BM25 выше, когда документ содержит более редкие слова. Подробное обсуждение мы оставим на потом.

В настоящее время реализованы следующие ранкеры:

* `proximity_bm25` — режим ранжирования по умолчанию, который использует и комбинирует как близость фраз, так и ранжирование BM25.
* `bm25` — статистический режим ранжирования, который использует только BM25 (аналогично большинству других полнотекстовых движков). Этот режим быстрее, но может давать худшее качество для запросов, содержащих более одного ключевого слова.
* `none` — режим без ранжирования. Этот режим, очевидно, самый быстрый. Всем совпадениям присваивается вес 1. Иногда это называют булевым поиском, который просто находит документы, но не ранжирует их.
* `wordcount` — ранжирование по количеству вхождений ключевых слов. Этот ранкер вычисляет количество вхождений ключевых слов по каждому полю, затем умножает их на веса полей и суммирует полученные значения.
* `proximity` возвращает сырое значение близости фраз в качестве результата. Этот режим внутренне используется для эмуляции запросов `SPH_MATCH_ALL`.
* `matchany` возвращает ранг, как он вычислялся ранее в режиме `SPH_MATCH_ANY`, и внутренне используется для эмуляции запросов `SPH_MATCH_ANY`.
* `fieldmask` возвращает 32-битную маску, где N-й бит соответствует N-му полнотекстовому полю, нумерация с 0. Бит будет установлен только если в соответствующем поле есть вхождения ключевых слов, удовлетворяющих запросу.
* `sph04` в целом основан на ранкере по умолчанию 'proximity_bm25', но дополнительно повышает рейтинг совпадений, когда они встречаются в самом начале или в самом конце текстового поля. Таким образом, если поле равно точному запросу, `sph04` должен ранжировать его выше, чем поле, которое содержит точный запрос, но не равно ему. (Например, если запрос "Hyde Park", документ с заголовком "Hyde Park" должен иметь более высокий ранг, чем документ с заголовком "Hyde Park, London" или "The Hyde Park Cafe".)
* `expr` позволяет задавать формулу ранжирования во время выполнения. Он предоставляет несколько внутренних текстовых факторов и позволяет определить, как из этих факторов вычислять итоговый вес. Подробнее о синтаксисе и справочник доступных факторов можно найти в [подразделе ниже](../Searching/Sorting_and_ranking.md#Quick-summary-of-the-ranking-factors).

Имя ранкера не чувствительно к регистру. Пример:

```sql
SELECT ... OPTION ranker=sph04;
```

## Краткое резюме факторов ранжирования

| Name                    | Level     | Type  | Summary                                                                                                            |
| ----------------------- | --------- | ----- | ------------------------------------------------------------------------------------------------------------------ |
| max_lcs                 | query     | int   | максимальное возможное значение LCS для текущего запроса                                                           |
| bm25                    | document  | int   | быстрая оценка BM25(1.2, 0)                                                                                        |
| bm25a(k1, b)            | document  | int   | точное значение BM25() с настраиваемыми константами K1, B и поддержкой синтаксиса                                 |
| bm25f(k1, b, {field=weight, ...}) | document | int   | точное значение BM25F() с дополнительной настройкой весов полей                                                   |
| field_mask              | document  | int   | битовая маска совпавших полей                                                                                      |
| query_word_count        | document  | int   | количество уникальных включённых ключевых слов в запросе                                                           |
| doc_word_count          | document  | int   | количество уникальных ключевых слов, совпавших в документе                                                          |
| lcs                     | field     | int   | Наибольшая общая подпоследовательность между запросом и документом, в словах                                        |
| user_weight             | field     | int   | пользовательский вес поля                                                                                            |
| hit_count               | field     | int   | общее количество вхождений ключевых слов                                                                            |
| word_count              | field     | int   | количество уникальных совпавших ключевых слов                                                                        |
| tf_idf                  | поле     | число с плавающей точкой | сумма(tf*idf) по совпавшим ключевым словам == сумма(idf) по вхождениям                                                     |
| min_hit_pos             | поле     | целое   | позиция первого совпадения, в словах, с базы 1                                                               |
| min_best_span_pos       | поле     | целое   | позиция первого максимального диапазона LCS, в словах, с базы 1                                                                 |
| exact_hit               | поле     | булево  | равен ли запрос полю                                                                                             |
| min_idf                 | поле     | число с плавающей точкой | мин(idf) по совпавшим ключевым словам                                                                                     |
| max_idf                 | поле     | число с плавающей точкой | макс(idf) по совпавшим ключевым словам                                                                                     |
| sum_idf                 | поле     | число с плавающей точкой | сумма(idf) по совпавшим ключевым словам                                                                                     |
| exact_order             | поле     | булево  | были ли все ключевые слова запроса a) совпавшими и b) в порядке запроса                                                   |
| min_gaps                | поле     | целое   | минимальное количество разрывов между совпавшими ключевыми словами в диапазонах совпадения                                        |
| lccs                    | поле     | целое   | Длина самой длинной общей непрерывной подпоследовательности между запросом и документом, в словах                                         |
| wlccs                   | поле     | число с плавающей точкой | Взвешенная самая длинная общая непрерывная подпоследовательность, сумма(idf) по непрерывным диапазонам ключевых слов                            |
| atc                     | поле     | число с плавающей точкой | Агрегированная близость терминов, log(1+сумма(idf1*idf2*pow(расстояние, -1.75)) по лучшим парам ключевых слов               |

Примечание: Для запросов с использованием операторов **Phrase**, **Proximity** или **NEAR** с более чем 31 ключевыми словами, факторы ранжирования, зависящие от частоты термина (такие как `tf`, `idf`, `bm25`, `hit_count`, `word_count`), могут быть занижены для ключевых слов в позиции 31 и выше. Это связано с внутренней 32-битной маской, используемой для отслеживания вхождений терминов в этих сложных операторах.

### Факторы ранжирования на уровне документа

**Фактор на уровне документа** - это числовое значение, вычисляемое механизмом ранжирования для каждого совпавшего документа относительно текущего запроса. Таким образом, он отличается от обычного атрибута документа тем, что атрибут не зависит от полнотекстового запроса, в то время как факторы могут зависеть. Эти факторы могут использоваться в любом месте выражения ранжирования. В настоящее время реализованы следующие факторы на уровне документа:

* `bm25` (целое), оценка BM25 на уровне документа (вычисленная без фильтрации вхождений ключевых слов).
* `max_lcs` (целое), максимальное возможное значение на уровне запроса, которое может принять выражение `sum(lcs*user_weight)`. Это может быть полезно для масштабирования усиления веса. Например, формула ранжера `MATCHANY` использует это, чтобы гарантировать, что полное совпадение фразы в любом поле ранжируется выше, чем любая комбинация частичных совпадений во всех полях.
* `field_mask` (целое), 32-битная маска совпавших полей на уровне документа.
* `query_word_count` (целое), количество уникальных ключевых слов в запросе, скорректированное на количество исключенных ключевых слов. Например, как запрос `(one one one one)`, так и `(one !two)` должны присваивать этому фактору значение 1, потому что есть только одно уникальное неисключенное ключевое слово.
* `doc_word_count` (целое), количество уникальных ключевых слов, совпавших во всем документе.

### Факторы ранжирования на уровне полей

**Фактор на уровне полей** - это числовое значение, вычисляемое механизмом ранжирования для каждого совпавшего текстового поля документа относительно текущего запроса. Поскольку более одного поля может быть совпавшим запросом, но итоговый вес должен быть одним целым значением, эти значения должны быть объединены в одно. Для этого факторы на уровне полей могут использоваться только внутри функции агрегации полей, они **не могут** использоваться где угодно в выражении. Например, нельзя использовать `(lcs+bm25)` в качестве выражения ранжирования, так как `lcs` имеет несколько значений (по одному в каждом совпавшем поле). Вместо этого следует использовать `(sum(lcs)+bm25)`, это выражение суммирует `lcs` по всем совпавшим полям, а затем добавляет `bm25` к этой сумме по полям. В настоящее время реализованы следующие факторы на уровне полей:

* `lcs` (целое), длина максимального дословного совпадения между документом и запросом, подсчитанная в словах. LCS означает Longest Common Subsequence (или Subset). Принимает минимальное значение 1, когда были совпадены только отдельные ключевые слова в поле, и максимальное значение равное количеству ключевых слов запроса, когда весь запрос был совпавшим в поле дословно (в точном порядке ключевых слов запроса). Например, если запрос - 'hello world', и поле содержит эти два слова, процитированные из запроса (то есть рядом друг с другом и точно в порядке запроса), `lcs` будет равен 2. Например, если запрос - 'hello world program', и поле содержит 'hello world', `lcs` будет равен 2. Обратите внимание, что работает любое подмножество ключевых слов запроса, а не только подмножество смежных ключевых слов. Например, если запрос - 'hello world program', и поле содержит 'hello (test program)', `lcs` будет 2 так же, потому что и 'hello', и 'program' совпали в тех же соответственных позициях, что и в запросе. Наконец, если запрос - 'hello world program', и поле содержит 'hello world program', `lcs` будет 3. (Надеюсь, это неудивительно к этому моменту.)
* `user_weight` (целое), указанный пользователем вес для каждого поля (см. [OPTION field_weights](../Searching/Options.md#field_weights) в SQL). Веса по умолчанию равны 1, если не указаны явно.
* `hit_count` (целое), количество вхождений ключевых слов, совпавших в поле. Обратите внимание, что одно ключевое слово может встречаться несколько раз. Например, если 'hello' встречается 3 раза в поле и 'world' - 5 раз, `hit_count` будет 8.
* `word_count` (целое число), количество уникальных ключевых слов, совпавших в поле. Например, если 'привет' и 'мир' встречаются где-либо в поле, `word_count` будет равен 2, независимо от того, сколько раз встретились оба ключевых слова.
* `tf_idf` (число с плавающей точкой), сумма TF/IDF по всем ключевым словам, совпавшим в поле. IDF - это обратная частота документа, значение с плавающей точкой между 0 и 1, которое описывает, насколько часто встречается ключевое слово (по сути, 0 для ключевого слова, встречающегося в каждом проиндексированном документе, и 1 для уникального ключевого слова, встречающегося только в одном документе). TF - это частота термина, количество вхождений ключевого слова в поле. Попутно отметим, что `tf_idf` фактически вычисляется путем суммирования IDF по всем совпавшим вхождениям. Это по определению эквивалентно суммированию TF*IDF по всем совпавшим ключевым словам.
* `min_hit_pos` (целое число), позиция первого вхождения совпавшего ключевого слова, считается в словах

   Таким образом, это относительно низкоуровневый, "сырой" фактор, который, скорее всего, вы захотите *скорректировать* перед использованием для ранжирования. Конкретные корректировки сильно зависят от ваших данных и результирующей формулы, но вот несколько идей для начала: (a) любые повышающие коэффициенты на основе min_gaps можно просто игнорировать, когда word_count<2;

    (b) значения min_gaps, не являющиеся тривиальными (т.е. когда word_count>=2), можно ограничить определенной "худшей" константой, в то время как тривиальные значения (т.е. когда min_gaps=0 и word_count<2) можно заменить этой константой;

    (c) можно применить передаточную функцию типа 1/(1+min_gaps) (чтобы лучшие, меньшие значения min_gaps максимизировали её, а худшие, большие значения снижались медленно); и так далее.
* `lccs` (целое число). Longest Common Contiguous Subsequence (Самая длинная общая непрерывная подпоследовательность). Длина самой длинной общей подфразы между запросом и документом, вычисленная в ключевых словах.

    Фактор LCCS somewhat похож на LCS, но более ограничен. Если LCS может быть больше 1 даже в том случае, если ни два слова запроса не совпадают рядом, то LCCS будет больше 1 только в том случае, если есть *точные*, смежные подфразы запроса в документе. Например, запрос (один два три четыре пять) и документ (один сто три сто пять сто) даст lcs=3, но lccs=1, потому что хотя взаимное расположение 3 ключевых слов (один, три, пять) совпадает между запросом и документом, никакие 2 совпадающие позиции фактически не смежны.

    Обратите внимание, что LCCS все еще не различает частые и редкие ключевые слова; для этого см. WLCCS.
* `wlccs` (число с плавающей точкой). Weighted Longest Common Contiguous Subsequence (Взвешенная самая длинная общая непрерывная подпоследовательность). Сумма IDF ключевых слов самой длинной общей подфразы между запросом и документом.

    WLCCS рассчитывается аналогично LCCS, но каждое "подходящее" вхождение ключевого слова увеличивает его на IDF ключевого слова вместо простого увеличения на 1 (как в LCS и LCCS). Это позволяет выше ранжировать последовательности более редких и важных ключевых слов по сравнению с последовательностями частых ключевых слов, даже если последние длиннее. Например, запрос `(Занзибар постель и завтрак)` даст lccs=1 для документа `(отели Занзибара)`, но lccs=3 против `(Лондон постель и завтрак)`, даже несмотря на то, что "Занзибар" на самом деле немного реже, чем вся фраза "постель и завтрак". Фактор WLCCS решает эту проблему, используя частоты ключевых слов.
* `atc` (число с плавающей точкой). Aggregate Term Closeness (Агрегированная близость термина). Мера близости, которая увеличивается, когда документ содержит больше групп более близко расположенных и более важных (редких) ключевых слов запроса.

    **ПРЕДУПРЕЖДЕНИЕ:** вы должны использовать ATC с OPTION idf='plain,tfidf_unnormalized' (см. [ниже](../Searching/Sorting_and_ranking.md#Configuration-of-IDF-formula)); в противном случае вы можете получить неожиданные результаты.

    ATC, по существу, работает следующим образом. Для каждого *вхождения* ключевого слова в документе мы вычисляем так называемую *близость термина*. Для этого мы изучаем все ближайшие вхождения всех ключевых слов запроса (включая само ключевое слово) слева и справа от рассматриваемого вхождения, вычисляем коэффициент затухания расстояния как k = pow(расстояние, -1.75) для этих вхождений и суммируем затухающие IDF. В результате для каждого вхождения каждого ключевого слова мы получаем значение "близости", которое описывает "соседей" этого вхождения. Затем мы умножаем эти значения близости на соответствующий IDF ключевого слова, суммируем их все и, наконец, вычисляем логарифм этой суммы.

Иными словами, мы обрабатываем лучшие (ближайшие) совпавшие пары ключевых слов в документе и вычисляем попарную "близость" как произведение их IDF, масштабированное по коэффициенту расстояния:

```
pair_tc = idf(pair_word1) * idf(pair_word2) * pow(pair_distance, -1.75)
```

Затем мы суммируем такие близости и вычисляем окончательное, сглаженное логарифмом значение ATC:

```
atc = log(1+sum(pair_tc))
```

Обратите внимание, что именно этот заключительный демпфирующий логарифм является точной причиной, по которой вам следует использовать OPTION idf=plain, потому что без него выражение внутри log() может быть отрицательным.

Более близкие вхождения ключевых слов вносят *гораздо* больший вклад в ATC, чем более частые ключевые слова. Действительно, когда ключевые слова находятся прямо рядом, расстояние=1 и k=1; когда между ними одно слово, расстояние=2 и k=0.297, с двумя словами между ними, расстояние=3 и k=0.146, и так далее. В то же время IDF затухает несколько медленнее. Например, в коллекции из 1 миллиона документов значения IDF для ключевых слов, совпадающих в 10, 100 и 1000 документах, будут соответственно 0.833, 0.667 и 0.500. Таким образом, пара ключевых слов с двумя довольно редкими ключевыми словами, встречающимися всего в 10 документах каждое, но с 2 другими словами между ними, даст pair_tc = 0.101 и едва перевесит пару с ключевым словом из 100 документов и ключевым словом из 1000 документов с 1 другим словом между ними и pair_tc = 0.099. Более того, пара из двух *уникальных*, встречающихся в 1 документе ключевых слов с 3 словами между ними получит pair_tc = 0.088 и проиграет паре ключевых слов из 1000 документов, расположенных прямо рядом и дающих pair_tc = 0.25. Таким образом, хотя ATC и объединяет частоту и близость ключевых слов, он все же несколько благоприятствует близости.

### Функции агрегации факторов ранжирования

**Функция агрегации поля** - это функция с одним аргументом, которая принимает выражение с факторами на уровне полей, выполняет итерацию по всем совпадающим полям и вычисляет окончательные результаты. В настоящее время реализованы следующие функции агрегации полей:

* `sum`, который суммирует аргумент-выражение по всем совпадающим полям. Например, `sum(1)` должен вернуть количество совпадающих полей.
* `top`, который возвращает наивысшее значение аргумента по всем совпадающим полям.
* `max_window_hits`, управляет скользящим окном позиций попаданий для отслеживания максимального количества попаданий в пределах указанного размера окна. Он удаляет устаревшие попадания, выходящие за пределы окна, и добавляет последнее попадание, обновляя максимальное количество найденных попаданий в этом окне.

### Формульные выражения для всех встроенных ранжировщиков

Большинство других ранжировщиков на самом деле можно эмулировать с помощью ранжировщика на основе выражений. Вам просто нужно предоставить соответствующее выражение. Хотя эта эмуляция, вероятно, будет медленнее, чем использование встроенного скомпилированного ранжировщика, она может быть интересна, если вы хотите тонко настроить формулу ранжирования, начав с одной из существующих. Кроме того, формулы четко и понятно описывают детали ранжировщика.

* proximity_bm25 (ранжировщик по умолчанию) = `sum(lcs*user_weight)*1000+bm25`
* bm25 = `sum(user_weight)*1000+bm25`
* none = `1`
* wordcount = `sum(hit_count*user_weight)`
* proximity = `sum(lcs*user_weight)`
* matchany = `sum((word_count+(lcs-1)*max_lcs)*user_weight)`
* fieldmask = `field_mask`
* sph04 = `sum((4*lcs+2*(min_hit_pos==1)+exact_hit)*user_weight)*1000+bm25`


### Конфигурация формулы IDF

Исторически используемый по умолчанию IDF (обратная частота документа) в Manticore эквивалентен `OPTION idf='normalized,tfidf_normalized'`, и эти нормализации могут вызывать несколько нежелательных эффектов.

Во-первых, `idf=normalized` вызывает штрафование ключевых слов. Например, при поиске `the | something`, если `the` встречается более чем в 50% документов, то документы с обоими ключевыми словами `the` и `something` получат меньший вес, чем документы только с одним ключевым словом `something`. Использование `OPTION idf=plain` позволяет избежать этого.

Простой IDF варьируется в диапазоне `[0, log(N)]`, и ключевые слова никогда не штрафуются; в то время как нормализованный IDF варьируется в диапазоне `[-log(N), log(N)]`, и слишком частые ключевые слова штрафуются.

Во-вторых, `idf=tfidf_normalized` вызывает дрейф IDF между запросами. Исторически мы дополнительно делили IDF на количество ключевых слов запроса, чтобы общая сумма `sum(tf*idf)` по всем ключевым словам всё ещё укладывалась в диапазон `[0,1]`. Однако это означает, что запросы `word1` и `word1 | nonmatchingword2` будут присваивать разные веса одному и тому же набору результатов, потому что IDF для обоих `word1` и `nonmatchingword2` будут делиться на 2. `OPTION idf='tfidf_unnormalized'` исправляет это. Обратите внимание, что факторы ранжирования BM25, BM25A, BM25F() будут масштабироваться соответственно после отключения этой нормализации.

Флаги IDF можно смешивать; `plain` и `normalized` являются взаимоисключающими; `tfidf_unnormalized` и `tfidf_normalized` являются взаимоисключающими; а неуказанные флаги в таких взаимоисключающих группах принимают значения по умолчанию. Это означает, что `OPTION idf=plain` эквивалентен полной спецификации `OPTION idf='plain,tfidf_normalized'`.

<!-- proofread -->

