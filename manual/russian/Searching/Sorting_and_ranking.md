# Сортировка и ранжирование

Результаты запроса могут быть отсортированы по весу ранжирования полнотекстового поиска, одному или нескольким атрибутам или выражениям.

**Полнотекстовые** запросы возвращают совпадения, отсортированные по умолчанию. Если ничего не указано, они сортируются по релевантности, что эквивалентно `ORDER BY weight() DESC` в SQL формате.

**Неполнтекстовые** запросы по умолчанию не выполняют никакой сортировки.

## Расширенная сортировка

Расширенный режим автоматически включается, когда вы явно задаёте правила сортировки, добавляя клаузу `ORDER BY` в SQL формате или используя опцию `sort` через HTTP JSON.

### Сортировка через SQL

Общий синтаксис:
```sql
SELECT ... ORDER BY
{attribute_name | expr_alias | weight() | random() } [ASC | DESC],
...
{attribute_name | expr_alias | weight() | random() } [ASC | DESC]
```

<!--
data for the following example:

DROP TABLE IF EXISTS test;
CREATE TABLE test(a int, b int, f text);
INSERT INTO test (a, b, f) VALUES
(2, 3, 'document');
-->

<!-- example alias -->

В клаузе сортировки вы можете использовать любую комбинацию до 5 столбцов, каждый из которых может сопровождаться `asc` или `desc`. Функции и выражения в качестве аргументов сортировки не допускаются, за исключением функций `weight()` и `random()` (последняя может использоваться только в SQL в форме `ORDER BY random()`). Однако, вы можете использовать любое выражение в списке SELECT и сортировать по его алиасу.

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

## Сортировка через JSON

<!-- example sorting 1 -->
Опция `"sort"` задаёт массив, где каждый элемент может быть именем атрибута или `_score`, если вы хотите сортировать по весу совпадения, или `_random`, если хотите случайный порядок совпадений. В этом случае порядок сортировки по умолчанию: по возрастанию для атрибутов и по убыванию для `_score`.

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
Сортировка по MVA атрибутам также поддерживается в JSON запросах. Режим сортировки можно задать через свойство `mode`. Поддерживаются следующие режимы:

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
При сортировке по атрибуту вычисление веса совпадения (оценки) по умолчанию отключено (ранжировщик не используется). Вы можете включить вычисление веса, установив свойство `track_scores` в `true`:

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

Ранжирование (также известное как взвешивание) результатов поиска можно определить как процесс вычисления так называемой релевантности (веса) для каждого найденного документа относительно данного запроса, который его совпал. Итак, релевантность в конечном итоге — это просто число, прикрепленное к каждому документу, которое оценивает, насколько документ релевантен запросу. Результаты поиска затем могут быть отсортированы на основе этого числа и/или некоторых дополнительных параметров, так чтобы наиболее востребованные результаты появлялись выше на странице результатов.

Нет универсального стандартного способа ранжировать любой документ в любой ситуации. Более того, такого способа никогда не может быть, потому что релевантность является *субъективной*. То есть то, что кажется релевантным вам, может не казаться релевантным мне. Следовательно, в общих случаях это не просто сложно вычислить; это теоретически невозможно.

Поэтому ранжирование в Manticore настраиваемое. В нём есть понятие так называемого **ранжировщика**. Ранжировщик формально можно определить как функцию, которая принимает документ и запрос в качестве входных данных и выдает значение релевантности как результат. Простыми словами, ранжировщик контролирует, как именно (с помощью какого именно алгоритма) Manticore будет назначать веса документам.

## Доступные встроенные ранжировщики

Manticore поставляется с несколькими встроенными ранжировщиками, подходящими для разных целей. Многие из них используют два фактора: близость фраз (также известная как LCS) и BM25. Близость фраз работает с позициями ключевых слов, тогда как BM25 учитывает частоты ключевых слов. По сути, чем выше степень совпадения фразы между текстом документа и запросом, тем выше близость фраз (она достигает максимума, если документ содержит весь запрос дословно). А BM25 выше, если документ содержит более редкие слова. Подробное обсуждение мы оставим на потом.

В настоящее время реализованы следующие ранжировщики:

* `proximity_bm25` — режим ранжирования по умолчанию, который использует и объединяет как близость фраз, так и ранжирование BM25.
* `bm25` — статистический режим ранжирования, использующий только BM25 (аналогично большинству других полнотекстовых движков). Этот режим быстрее, но может привести к худшему качеству для запросов, содержащих более одного ключевого слова.
* `none` — режим без ранжирования. Очевидно, что этот режим самый быстрый. Всем совпадениям назначается вес 1. Это иногда называют булевым поиском, который просто находит документы, но не ранжирует их.
* `wordcount` — ранжирование по количеству вхождений ключевых слов. Этот ранжировщик вычисляет количество вхождений ключевых слов в каждом поле, затем умножает их на веса полей и суммирует полученные значения.
* `proximity` возвращает исходное значение близости фраз в качестве результата. Этот режим внутренне используется для эмуляции запросов с `SPH_MATCH_ALL`.
* `matchany` возвращает ранг, как он вычислялся ранее в режиме `SPH_MATCH_ANY` и внутренне используется для эмуляции запросов `SPH_MATCH_ANY`.
* `fieldmask` возвращает 32-битную маску, где N-й бит соответствует N-му полнотекстовому полю, начиная нумерацию с 0. Бит устанавливается только тогда, когда в соответствующем поле есть какие-либо вхождения ключевых слов, удовлетворяющих запросу.
* `sph04` в целом основан на ранжировщике по умолчанию `proximity_bm25`, но дополнительно повышает рейтинг совпадений, когда они находятся в самом начале или в самом конце текстового поля. Таким образом, если поле равно точному запросу, `sph04` должен ранжировать его выше, чем поле, которое содержит точный запрос, но не равно ему. (Например, когда запрос — "Hyde Park", документ с заголовком "Hyde Park" должен получить более высокий ранг, чем документ с заголовком "Hyde Park, London" или "The Hyde Park Cafe".)
* `expr` позволяет задавать формулу ранжирования во время выполнения. Он предоставляет несколько внутренних текстовых факторов и позволяет определить, как из этих факторов вычислять итоговый вес. Подробнее о его синтаксисе и справочник доступных факторов можно найти в [подразделе ниже](../Searching/Sorting_and_ranking.md#Quick-summary-of-the-ranking-factors).

Название ранжировщика нечувствительно к регистру. Пример:

```sql
SELECT ... OPTION ranker=sph04;
```

## Краткое резюме факторов ранжирования

| Название                 | Уровень   | Тип   | Краткое описание                                                                                                |
| ----------------------- | --------- | ----- | ---------------------------------------------------------------------------------------------------------------- |
| max_lcs                 | запрос    | int   | максимальное возможное значение LCS для текущего запроса                                                         |
| bm25                    | документ  | int   | быстрая оценка BM25(1.2, 0)                                                                                      |
| bm25a(k1, b)            | документ  | int   | точное значение BM25() с настраиваемыми константами K1, B и поддержкой синтаксиса                               |
| bm25f(k1, b, {field=weight, ...}) | документ | int   | точное значение BM25F() с дополнительной настройкой весов полей                                                  |
| field_mask              | документ  | int   | битовая маска совпавших полей                                                                                     |
| query_word_count        | документ  | int   | количество уникальных включаемых ключевых слов в запросе                                                          |
| doc_word_count          | документ  | int   | количество уникальных ключевых слов, совпадающих в документе                                                       |
| lcs                     | поле      | int   | Максимальная общая подпоследовательность между запросом и документом в словах                                      |
| user_weight             | поле      | int   | пользовательский вес поля                                                                                          |
| hit_count               | поле      | int   | общее количество вхождений ключевых слов                                                                          |
| word_count              | поле      | int   | количество уникальных совпавших ключевых слов                                                                      |
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
* `word_count` (целое число), количество уникальных ключевых слов, найденных в поле. Например, если 'hello' и 'world' встречаются где-либо в поле, `word_count` будет равно 2, независимо от того, сколько раз оба ключевых слова встречаются.
* `tf_idf` (число с плавающей точкой), сумма TF/IDF по всем ключевым словам, найденным в поле. IDF — это обратная частота документа, число с плавающей точкой от 0 до 1, описывающее, насколько часто встречается ключевое слово (фактически 0 для ключевого слова, которое встречается в каждом проиндексированном документе, и 1 для уникального ключевого слова, которое встречается только в одном документе). TF — это частота термина, количество совпадений ключевого слова в поле. К слову, `tf_idf` фактически вычисляется суммированием IDF по всем встреченным случаям. По конструкции это эквивалентно суммированию TF*IDF по всем совпавшим ключевым словам.
* `min_hit_pos` (целое число), позиция первого совпавшего ключевого слова, считая в словах

   Следовательно, это относительно низкоуровневый, "сырой" фактор, который вам, скорее всего, захочется *скорректировать* перед использованием в ранжировании. Конкретные корректировки сильно зависят от ваших данных и итоговой формулы, но вот несколько идей для начала: (a) любые усиления на основе min_gaps можно просто игнорировать, если word_count<2;

    (b) нетривиальные значения min_gaps (т.е. при word_count>=2) можно ограничить некоторой "худшей" константой, тогда как тривиальные значения (т.е. когда min_gaps=0 и word_count<2) можно заменить этой константой;

    (c) можно применить функцию преобразования вроде 1/(1+min_gaps) (так что лучшие, меньшие значения min_gaps максимизируют её, а худшие, большие значения плавно уменьшают); и так далее.
* `lccs` (целое число). Длина самой длинной общей смежной подпоследовательности. Длина самой длинной подфразы, общей для запроса и документа, измеряемая в ключевых словах.

    Фактор LCCS похож на LCS, но более строг. В то время как LCS может быть больше 1, даже если ни два слова запроса не соответствуют друг другу подряд, LCCS будет больше 1 только если в документе есть *точные*, смежные подфразы из запроса. Например, запрос `(one two three four five)` и документ `(one hundred three hundred five hundred)` дадут lcs=3, но lccs=1, потому что хотя взаимное расположение 3 ключевых слов (one, three, five) совпадает между запросом и документом, ни две совпадающие позиции на самом деле не являются смежными.

    Обратите внимание, что LCCS всё еще не различает частотные и редкие ключевые слова; для этого смотрите WLCCS.
* `wlccs` (число с плавающей точкой). Взвешенная длина самой длинной общей смежной подпоследовательности. Сумма IDF ключевых слов самой длинной подфразы, общей для запроса и документа.

    WLCCS считается аналогично LCCS, но каждое "подходящее" вхождение ключевого слова увеличивает его на IDF этого слова, а не просто на 1 (как в LCS и LCCS). Это позволяет ранжировать выше последовательности более редких и важных ключевых слов, чем более длинные, но часто встречающиеся. Например, запрос `(Zanzibar bed and breakfast)` даст lccs=1 для документа `(hotels of Zanzibar)`, но lccs=3 для `(London bed and breakfast)`, хотя "Zanzibar" фактически встречается реже, чем вся фраза "bed and breakfast". Фактор WLCCS решает эту задачу, используя частоты ключевых слов.
* `atc` (число с плавающей точкой). Суммарная близость терминов. Мера близости, которая увеличивается, когда в документе есть больше групп более близко расположенных и более важных (редких) ключевых слов из запроса.

    **ВНИМАНИЕ:** следует использовать ATC с ПАРАМЕТРОМ idf='plain,tfidf_unnormalized' (см. [ниже](../Searching/Sorting_and_ranking.md#Configuration-of-IDF-formula)); иначе могут быть неожиданные результаты.

    ATC работает следующим образом. Для каждого вхождения ключевого слова в документе вычисляется так называемая *близость термина*. Для этого мы рассматриваем все ближайшие вхождения всех ключевых слов запроса (включая само слово) слева и справа от рассмотренного вхождения, вычисляем коэффициент затухания расстояния как k = pow(distance, -1.75) для этих вхождений и суммируем ослабленные IDF. В результате для каждого вхождения каждого ключевого слова получается значение "близости", описывающее "соседей" этого вхождения. Затем эти значения близости умножаются на IDF соответствующего ключевого слова, суммируются все вместе и затем вычисляется логарифм от этой суммы.

Другими словами, мы обрабатываем лучшие (ближайшие) пары совпавших ключевых слов в документе и вычисляем попарную "близость" как произведение их IDF, масштабированное коэффициентом расстояния:

```
pair_tc = idf(pair_word1) * idf(pair_word2) * pow(pair_distance, -1.75)
```

Затем мы суммируем такие близости и вычисляем итоговое значение ATC с логарифмическим затуханием:

```
atc = log(1+sum(pair_tc))
```

Обратите внимание, что именно этот финальный логарифмический затухающий множитель является причиной использования ПАРАМЕТРА idf=plain, так как без него выражение внутри log() могло бы быть отрицательным.

Наличие более близких вхождений ключевых слов влияет на ATC гораздо сильнее, чем более высокая частота ключевых слов. В самом деле, если ключевые слова стоят рядом, расстояние=1 и k=1; если между ними одно слово, расстояние=2 и k=0.297, с двумя словами между ними расстояние=3 и k=0.146 и так далее. В то же время IDF ослабевает немного медленнее. Например, в коллекции из миллиона документов IDF для ключевых слов, встречающихся в 10, 100 и 1000 документах соответственно, будут 0.833, 0.667 и 0.500. Таким образом, пара ключевых слов, каждое из которых встречается всего в 10 документах, но с двумя словами между ними, даст pair_tc = 0.101 и едва превзойдет пару с ключевыми словами из 100 и 1000 документов с одним словом между ними, pair_tc = 0.099. Более того, пара из двух *уникальных*, встречающихся в одном документе ключевых слов с тремя словами между ними даст pair_tc = 0.088 и проиграет паре из двух ключевых слов из 1000 документов, расположенных рядом, с pair_tc = 0.25. Таким образом, хотя ATC объединяет и частоту ключевых слов, и их близость, она всё же несколько предпочитает близость.

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

