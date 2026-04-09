# Сортировка и ранжирование

Результаты запросов могут быть отсортированы по весу полнотекстового ранжирования, одному или нескольким атрибутам или выражениям.

**Полнотекстовые** запросы возвращают совпадения отсортированными по умолчанию. Если ничего не указано, они сортируются по релевантности, что эквивалентно `ORDER BY weight() DESC` в формате SQL.

**Не полнотекстовые** запросы по умолчанию не выполняют никакой сортировки.

## Расширенная сортировка

Расширенный режим автоматически включается, когда вы явно задаете правила сортировки, добавляя предложение `ORDER BY` в формате SQL или используя опцию `sort` через HTTP JSON.

### Сортировка через SQL

Общий синтаксис:
```sql
SELECT ... ORDER BY
{attribute_name | expr_alias | weight() | random() } [ASC | DESC],
...
{attribute_name | expr_alias | weight() | random() } [ASC | DESC]
```

<!-- example alias -->

В предложении сортировки можно использовать любую комбинацию до 5 столбцов, каждый из которых может сопровождаться `asc` или `desc`. Функции и выражения не допускаются в качестве аргументов для предложения сортировки, за исключением функций `weight()` и `random()` (последняя может использоваться только через SQL в виде `ORDER BY random()`). Однако вы можете использовать любое выражение в списке SELECT и сортировать по его псевдониму.

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
`"sort"` задает массив, где каждый элемент может быть именем атрибута, `_score`, если вы хотите сортировать по весам совпадений, или `_random`, если нужен случайный порядок совпадений. В этом случае порядок сортировки по умолчанию — по возрастанию для атрибутов и по убыванию для `_score`.

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

* `min`: сортировать по минимальному значению
* `max`: сортировать по максимальному значению

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
При сортировке по атрибуту вычисление веса совпадения (оценки) по умолчанию отключено (ранкер не используется). Вы можете включить вычисление веса, установив свойство `track_scores` в `true`:

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

Ранжирование (также известное как взвешивание) результатов поиска можно определить как процесс вычисления так называемой релевантности (веса) для каждого найденного документа относительно данного запроса, по которому он был найден. Таким образом, релевантность — это, в конце концов, просто число, прикреплённое к каждому документу, которое оценивает, насколько документ релевантен запросу. Результаты поиска затем могут быть отсортированы на основе этого числа и/или некоторых дополнительных параметров, чтобы наиболее востребованные результаты отображались выше на странице результатов.

Не существует одного стандартного универсального способа ранжирования любого документа в любой ситуации. Более того, такого способа никогда не может быть, потому что релевантность является *субъективной*. То есть, что кажется релевантным вам, может не казаться таковым мне. Следовательно, в общих случаях, это не просто трудно вычислить; это теоретически невозможно.

Поэтому ранжирование в Manticore настраиваемо. В нем есть понятие так называемого **ранкера**. Ранкер формально можно определить как функцию, которая принимает на вход документ и запрос и производит на выходе значение релевантности. Проще говоря, ранкер контролирует именно то, как (с помощью какого конкретного алгоритма) Manticore будет присваивать веса документам.

## Доступные встроенные ранкеры

Manticore поставляется с несколькими встроенными ранкерами, подходящими для разных целей. Многие из них используют два фактора: близость фраз (известная также как LCS) и BM25. Близость фраз работает с позициями ключевых слов, в то время как BM25 работает с частотами ключевых слов. По сути, чем лучше степень совпадения фразы между телом документа и запросом, тем выше близость фраз (максимум достигается, когда документ содержит весь запрос в точном виде). А BM25 становится выше, когда документ содержит более редкие слова. Подробное обсуждение мы оставим на потом.

В настоящее время реализованные ранкеры:

* `proximity_bm25` — режим ранжирования по умолчанию, который использует и комбинирует как близость фраз, так и ранжирование BM25.
* `bm25` — статистический режим ранжирования, который использует только BM25 (аналогично большинству других полнотекстовых движков). Этот режим быстрее, но может давать худшее качество для запросов, содержащих более одного ключевого слова.
* `none` — режим без ранжирования. Это, очевидно, самый быстрый режим. Всем совпадениям присваивается вес 1. Иногда это называется булевым поиском, который просто находит документы, но не ранжирует их.
* `wordcount` — ранжирование по количеству вхождений ключевых слов. Этот ранкер вычисляет количество вхождений ключевых слов для каждого поля, затем умножает их на веса полей и суммирует полученные значения.
* `proximity` возвращает сырое значение близости фраз как результат. Этот режим используется внутри для эмуляции запросов `SPH_MATCH_ALL`.
* `matchany` возвращает ранг, как он вычислялся в режиме `SPH_MATCH_ANY`, и используется внутри для эмуляции запросов `SPH_MATCH_ANY`.
* `fieldmask` возвращает 32-битную маску, в которой N-й бит соответствует N-му полнотекстовому полю, нумерация с 0. Бит устанавливается только когда соответствующее поле содержит вхождения ключевых слов, удовлетворяющих запросу.
* `sph04` в целом основан на ранкере по умолчанию 'proximity_bm25', но дополнительно усиливает совпадения, если они происходят в самом начале или самом конце текстового поля. Таким образом, если поле ровно равно точному запросу, `sph04` должен ранжировать его выше, чем поле, которое содержит точный запрос, но не равно ему. (Например, если запрос «Hyde Park», документ с заголовком «Hyde Park» должен иметь более высокий ранг, чем документ с заголовком «Hyde Park, London» или «The Hyde Park Cafe».)
* `expr` позволяет задать формулу ранжирования во время выполнения. В нем доступны несколько внутренних текстовых факторов, и вы можете определить, как вычисляется итоговый вес из этих факторов. Подробнее о синтаксисе и справке по доступным факторам можно узнать в [подразделе ниже](../Searching/Sorting_and_ranking.md#Quick-summary-of-the-ranking-factors).

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
| bm25f(k1, b, {field=weight, ...}) | document | int   | точное значение BM25F() с дополнительными настраиваемыми весами полей                                             |
| field_mask              | document  | int   | битовая маска совпавших полей                                                                                        |
| query_word_count        | document  | int   | количество уникальных включённых ключевых слов в запросе                                                           |
| doc_word_count          | document  | int   | количество уникальных ключевых слов, найденных в документе                                                        |
| lcs                     | field     | int   | самая длинная общая подпоследовательность между запросом и документом, в словах                                     |
| user_weight             | field     | int   | вес поля, заданный пользователем                                                                                    |
| hit_count               | field     | int   | общее количество вхождений ключевых слов                                                                            |
| word_count              | field     | int   | количество уникальных совпавших ключевых слов                                                                       |
| tf_idf                  | field     | float | сумма(tf*idf) по совпавшим ключевым словам == сумма(idf) по вхождениям                                              |
| min_hit_pos             | field     | int   | позиция первого совпавшего вхождения, в словах, начиная с 1                                                          |
| min_best_span_pos       | field     | int   | позиция первого максимального LCS интервала, в словах, начиная с 1                                                  |
| exact_hit               | field     | bool  | соответствует ли запрос полю                                                                                         |
| min_idf                 | field     | float | минимальное значение idf по совпавшим ключевым словам                                                               |
| max_idf                 | field     | float | максимальное значение idf по совпавшим ключевым словам                                                              |
| sum_idf                 | field     | float | сумма значений idf по совпавшим ключевым словам                                                                     |
| exact_order             | field     | bool  | совпадают ли все ключевые слова запроса а) были найдены и б) расположены в порядке запроса                          |
| min_gaps                | field     | int   | минимальное количество пропусков между совпавшими ключевыми словами в пределах совпадающих интервалов                |
| lccs                    | field     | int   | самая длинная общая непрерывная подпоследовательность между запросом и документом, в словах                           |
| wlccs                   | field     | float | взвешенная самая длинная общая непрерывная подпоследовательность, сумма(idf) по непрерывным интервалам ключевых слов |
| atc                     | field     | float | агрегированная близость терминов, log(1+sum(idf1*idf2*pow(distance, -1.75)) по лучшим парам ключевых слов            |

Примечание: Для запросов с использованием операторов **Phrase**, **Proximity** или **NEAR** содержащих более 31 ключевого слова, факторы ранжирования, основанные на частоте термина (например, `tf`, `idf`, `bm25`, `hit_count`, `word_count`), могут быть недоучтены для ключевых слов с позиции 31 и далее. Это связано с внутренней 32-битной маской, используемой для отслеживания вхождений терминов в этих сложных операторах.

### Факторы ранжирования на уровне документа

**Фактор уровня документа** — это числовое значение, вычисляемое механизмом ранжирования для каждого совпавшего документа относительно текущего запроса. Он отличается от простого атрибута документа тем, что атрибут не зависит от полного текста запроса, а факторы могут зависеть. Эти факторы могут использоваться в любом месте выражения для ранжирования. В настоящее время реализованы следующие факторы уровня документа:

* `bm25` (целое число), оценка BM25 на уровне документа (вычисляется без фильтрации вхождений ключевых слов).
* `max_lcs` (целое число), максимальное возможное значение на уровне запроса, которое может принять выражение `sum(lcs*user_weight)`. Это может быть полезно для масштабирования коэффициентов веса. Например, формула ранжирования `MATCHANY` использует это, чтобы гарантировать, что полное совпадение фразы в любом поле будет выше любой комбинации частичных совпадений во всех полях.
* `field_mask` (целое число), 32-битовая маска совпавших полей на уровне документа.
* `query_word_count` (целое число), количество уникальных ключевых слов в запросе с учётом исключённых ключевых слов. Например, запросы `(one one one one)` и `(one !two)` должны назначать значению этого фактора 1, потому что есть только одно уникальное не исключённое ключевое слово.
* `doc_word_count` (целое число), количество уникальных ключевых слов, совпавших во всем документе.

### Факторы ранжирования на уровне поля

**Фактор уровня поля** — это числовое значение, вычисляемое ранжировочным движком для каждого совпавшего текстового поля документа относительно текущего запроса. Так как запрос может совпадать более чем с одним полем, но итоговый вес должен быть одним целым числом, эти значения необходимо свести к одному. Для этого факторы уровня поля можно использовать только внутри функции агрегирования по полям, их **нельзя** использовать в выражении отдельно. Например, нельзя использовать `(lcs+bm25)` в качестве выражения ранжирования, так как `lcs` принимает несколько значений (по одному в каждом совпавшем поле). Вместо этого следует использовать `(sum(lcs)+bm25)`, выражение суммирует `lcs` по всем совпавшим полям, а затем добавляет `bm25` к этой сумме по полям. В настоящее время реализованы следующие факторы уровня поля:

* `lcs` (целое число), длина максимального точного совпадения между документом и запросом, считаемая в словах. LCS означает Longest Common Subsequence (или Подпоследовательность). Принимает минимальное значение 1, если в поле были найдены только отдельные ключевые слова, и максимальное значение — количество ключевых слов запроса, если в поле было найдено точное совпадение всего запроса (в точном порядке ключевых слов). Например, если запрос 'hello world', и поле содержит эти два слова из запроса (то есть, расположены подряд и в точном порядке из запроса), `lcs` будет равен 2. Если запрос 'hello world program', а поле содержит 'hello world', `lcs` будет 2. Обратите внимание, что рабочими являются любые подмножества ключевых слов из запроса, а не только смежные. Например, если запрос 'hello world program', а поле содержит 'hello (test program)', `lcs` также будет 2, так как и 'hello', и 'program' совпадают в тех же позициях, что и в запросе. Наконец, если запрос 'hello world program', а поле содержит 'hello world program', `lcs` будет 3. (Это, надеюсь, уже не удивительно.)
* `user_weight` (целое число), пользовательский вес для каждого поля (см. [OPTION field_weights](../Searching/Options.md#field_weights) в SQL). По умолчанию веса равны 1, если явно не указаны.
* `hit_count` (целое число), количество совпавших ключевых слов в поле. Обратите внимание, что одно ключевое слово может встретиться несколько раз. Например, если 'hello' встречается 3 раза в поле, а 'world' – 5 раз, `hit_count` будет 8.
* `word_count` (целое число), количество уникальных ключевых слов, совпавших в поле. Например, если 'hello' и 'world' встречаются в поле, `word_count` будет 2, независимо от числа повторений этих слов.
* `tf_idf` (число с плавающей точкой), сумма TF/IDF по всем совпавшим ключевым словам в поле. IDF — обратная частотность документа, плавающее значение от 0 до 1, описывающее частоту ключевого слова (в основном, 0 для слова, встречающегося во всех индексируемых документах, и 1 для уникального слова, встречающегося в одном документе). TF — частота термина, количество совпадений ключевого слова в поле. Кстати, `tf_idf` фактически вычисляется как сумма IDF по всем совпавшим вхождениям, что по построению эквивалентно сумме TF*IDF по ключевым словам.
* `min_hit_pos` (целое число), позиция первого совпавшего ключевого слова, считаемая в словах

   Следовательно, это относительно низкоуровневый, «сырой» фактор, который, скорее всего, потребуется *подкорректировать* перед использованием в ранжировании. Конкретные корректировки сильно зависят от ваших данных и итоговой формулы, но вот несколько идей для начала: (а) любые бусты на основе min_gaps можно просто игнорировать, если word_count<2;

    (b) не тривиальные значения min_gaps (т.е. когда word_count>=2) могут быть ограничены определённой константой "худшего случая", а тривиальные значения (т.е. когда min_gaps=0 и word_count<2) могут быть заменены этой константой;

    (c) можно применить функцию преобразования типа 1/(1+min_gaps) (так, чтобы лучшие, меньшие min_gaps максимизировали значение, а худшие, большие min_gaps медленно уменьшали его); и так далее.
* `lccs` (целое число). Longest Common Contiguous Subsequence — Длина самой длинной общей непрерывной подпоследовательности между запросом и документом, измеряемой по ключевым словам.

    Фактор LCCS отчасти похож на LCS, но более строгий. В то время как LCS может быть больше 1, даже если никакие два слова из запроса не стоят рядом, LCCS будет больше 1 только если в документе есть *точные*, непрерывные подфразы из запроса. Например, запрос (one two three four five) и документ (one hundred three hundred five hundred) дадут lcs=3, но lccs=1, поскольку совпадают позиции трёх ключевых слов (one, three, five), но никакие 2 соседних совпадения не расположены подряд.

    Обратите внимание, что LCCS по-прежнему не учитывает частоту слов; для этого смотрите WLCCS.
* `wlccs` (число с плавающей точкой). Weighted Longest Common Contiguous Subsequence — Взвешенная длина самой длинной общей непрерывной подпоследовательности, сумма IDF ключевых слов самой длинной общей подфразы между запросом и документом.

    WLCCS вычисляется аналогично LCCS, но каждое «подходящее» совпавшее слово увеличивает значение на IDF данного слова, а не просто на 1 (как в LCS и LCCS). Это позволяет ранжировать последовательности из более редких и важных ключевых слов выше, чем последовательности частых ключевых слов, даже если последние длиннее. Например, запрос `(Zanzibar bed and breakfast)` даёт lccs=1 для документа `(hotels of Zanzibar)`, но lccs=3 для `(London bed and breakfast)`, хотя «Zanzibar» на самом деле несколько реже, чем вся фраза «bed and breakfast». Фактор WLCCS решает эту проблему, учитывая частоты ключевых слов.
* `atc` (число с плавающей точкой). Aggregate Term Closeness — агрегированная близость терминов. Мера близости, которая растёт, если документ содержит больше групп более тесно расположенных и более важных (редких) ключевых слов из запроса.

    **ВНИМАНИЕ:** следует использовать ATC с OPTION idf='plain,tfidf_unnormalized' (см. [ниже](../Searching/Sorting_and_ranking.md#Configuration-of-IDF-formula)); иначе можно получить неожиданные результаты.

ATC по сути работает следующим образом. Для каждого *вхождения* ключевого слова в документе мы вычисляем так называемую *близость терминов*. Для этого мы рассматриваем все остальные ближайшие вхождения всех ключевых слов запроса (включая само ключевое слово) слева и справа от рассматриваемого вхождения, вычисляем коэффициент затухания по расстоянию как k = pow(distance, -1.75) для этих вхождений и суммируем затухающие IDF. В результате для каждого вхождения каждого ключевого слова мы получаем значение "близости", которое описывает "соседей" этого вхождения. Затем мы умножаем эти значения близости для каждого вхождения на соответствующий IDF ключевого слова-субъекта, суммируем их все и, наконец, вычисляем логарифм этой суммы.

Другими словами, мы обрабатываем наилучшие (ближайшие) совпадающие пары ключевых слов в документе и вычисляем попарные "близости" как произведение их IDF, масштабированное коэффициентом расстояния:

```
pair_tc = idf(pair_word1) * idf(pair_word2) * pow(pair_distance, -1.75)
```

Затем мы суммируем такие близости и вычисляем окончательное, логарифмически затухающее значение ATC:

```
atc = log(1+sum(pair_tc))
```

Обратите внимание, что этот финальный затухающий логарифм является именно той причиной, по которой следует использовать OPTION idf=plain, потому что без него выражение внутри log() может быть отрицательным.

Более близкие вхождения ключевых слов вносят *гораздо* больший вклад в ATC, чем более частые ключевые слова. Действительно, когда ключевые слова находятся прямо рядом друг с другом, distance=1 и k=1; когда между ними всего одно слово, distance=2 и k=0.297, с двумя словами между ними, distance=3 и k=0.146, и так далее. В то же время IDF затухает несколько медленнее. Например, в коллекции из 1 миллиона документов значения IDF для ключевых слов, которые встречаются в 10, 100 и 1000 документах, будут соответственно 0.833, 0.667 и 0.500. Таким образом, пара ключевых слов с двумя довольно редкими ключевыми словами, которые встречаются всего в 10 документах каждый, но с 2 другими словами между ними, даст pair_tc = 0.101 и, таким образом, едва перевесит пару с ключевыми словами из 100 и 1000 документов с 1 другим словом между ними и pair_tc = 0.099. Более того, пара двух *уникальных*, 1-документных ключевых слов с 3 словами между ними получит pair_tc = 0.088 и проиграет паре из двух 1000-документных ключевых слов, расположенных прямо рядом друг с другом и дающих pair_tc = 0.25. Итак, в основном, хотя ATC и сочетает в себе как частоту ключевых слов, так и близость, он все же несколько отдает предпочтение близости.

### Функции агрегации факторов ранжирования

**Функция агрегации полей** — это функция с одним аргументом, которая принимает выражение с факторами на уровне полей, перебирает все совпадающие поля и вычисляет окончательные результаты. В настоящее время реализованные функции агрегации полей включают:

* `sum`, которая суммирует выражение-аргумент по всем совпадающим полям. Например, `sum(1)` должна возвращать количество совпадающих полей.
* `top`, которая возвращает наибольшее значение аргумента среди всех совпадающих полей.
* `max_window_hits`, управляет скользящим окном позиций совпадений для отслеживания максимального количества совпадений в пределах указанного размера окна. Она удаляет устаревшие совпадения, выходящие за пределы окна, и добавляет последнее совпадение, обновляя максимальное количество совпадений, найденных в этом окне.

### Выражения формул для всех встроенных ранкеров

Большинство других ранкеров фактически можно эмулировать с помощью ранкера на основе выражений. Вам просто нужно предоставить соответствующее выражение. Хотя эта эмуляция, вероятно, будет медленнее, чем использование встроенного скомпилированного ранкера, она все же может быть интересна, если вы хотите точно настроить свою формулу ранжирования, начиная с одной из существующих. Кроме того, формулы описывают детали ранкера в понятной, читаемой форме.

* proximity_bm25 (ранкер по умолчанию) = `sum(lcs*user_weight)*1000+bm25`
* bm25 = `sum(user_weight)*1000+bm25`
* none = `1`
* wordcount = `sum(hit_count*user_weight)`
* proximity = `sum(lcs*user_weight)`
* matchany = `sum((word_count+(lcs-1)*max_lcs)*user_weight)`
* fieldmask = `field_mask`
* sph04 = `sum((4*lcs+2*(min_hit_pos==1)+exact_hit)*user_weight)*1000+bm25`


### Конфигурация формулы IDF

Исторически используемый по умолчанию IDF (обратная частота документа) в Manticore эквивалентен `OPTION idf='normalized,tfidf_normalized'`, и эти нормализации могут вызывать несколько нежелательных эффектов.

Во-первых, `idf=normalized` вызывает штраф за ключевые слова. Например, если вы ищете `the | something` и `the` встречается более чем в 50% документов, то документы с обоими ключевыми словами `the` и`[something` получат меньший вес, чем документы только с одним ключевым словом  `something`. Использование `OPTION idf=plain` позволяет избежать этого.

Простой IDF варьируется в диапазоне `[0, log(N)]`, и ключевые слова никогда не штрафуются; в то время как нормализованный IDF варьируется в диапазоне `[-log(N), log(N)]`, и слишком частые ключевые слова штрафуются.

Во-вторых, `idf=tfidf_normalized` вызывает смещение IDF между запросами. Исторически мы дополнительно делили IDF на количество ключевых слов в запросе, чтобы вся `sum(tf*idf)` по всем ключевым словам все еще укладывалась в диапазон `[0,1]`. Однако это означает, что запросы `word1` и `word1 | nonmatchingword2` будут присваивать разный вес точно такому же набору результатов, потому что IDF как для `word1`, так и для `nonmatchingword2` будут разделены на 2. `OPTION idf='tfidf_unnormalized'` исправляет это. Обратите внимание, что факторы ранжирования BM25, BM25A, BM25F() будут соответствующим образом масштабированы, как только вы отключите эту нормализацию.

Флаги IDF можно комбинировать; `plain` и `normalized` являются взаимоисключающими; `tfidf_unnormalized` и `tfidf_normalized` являются взаимоисключающими; и неуказанные флаги в такой взаимоисключающей группе принимают свои значения по умолчанию. Это означает, что `OPTION idf=plain` эквивалентно полной спецификации `OPTION idf='plain,tfidf_normalized'`.

<!-- proofread -->

