# Сортировка и ранжирование

Результаты запроса могут быть отсортированы по весу ранжирования полного текста, одному или нескольким атрибутам или выражениям.

**Полнотекстовые** запросы возвращают совпадения, отсортированные по умолчанию. Если ничего не указано, они сортируются по релевантности, что эквивалентно `ORDER BY weight() DESC` в формате SQL.

**Неполнотекстовые** запросы по умолчанию не выполняют сортировку.

## Расширенная сортировка

Расширенный режим автоматически включается, когда вы явно задаете правила сортировки, добавляя оператор `ORDER BY` в формате SQL или используя опцию `sort` через HTTP JSON.

### Сортировка через SQL

Общий синтаксис:
```sql
SELECT ... ORDER BY
{attribute_name | expr_alias | weight() | random() } [ASC | DESC],
...
{attribute_name | expr_alias | weight() | random() } [ASC | DESC]
```

<!-- example alias -->

В операторе сортировки вы можете использовать любую комбинацию из 5 столбцов, каждый из которых должен быть завершен `asc` или `desc`. Функции и выражения не допускаются в качестве аргументов для оператора сортировки, за исключением функций `weight()` и `random()` (последняя может использоваться только через SQL в виде `ORDER BY random()`). Однако вы можете использовать любое выражение в списке SELECT и сортировать по его псевдониму.

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
`"sort"` указывает массив, где каждый элемент может быть именем атрибута или `_score`, если вы хотите сортировать по весам совпадений. В этом случае порядок сортировки по умолчанию — по возрастанию для атрибутов и по убыванию для `_score`.

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

        "title": "Тестовый документ 5"

      }

    },

    {

      "_id": 5406864699109146631,

      "_score": 2319,

      "_source": {

        "title": "Тестовый документ 4"

      }

    },

    {

      "_id": 5406864699109146630,

      "_score": 2319,

      "_source": {

        "title": "Тестовый документ 3"

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
$search->setIndex("test")->match('Тестовый документ')->sort('id', 'desc')->sort('_score');
```

<!-- intro -->

##### Python:

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Тестовый документ')
sort_by_id = manticoresearch.model.SortOrder('id', 'desc')
search_request.sort = [sort_by_id, '_score']
```

<!-- intro -->

##### Python-asyncio:

<!-- request Python-asyncio -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Тестовый документ')
sort_by_id = manticoresearch.model.SortOrder('id', 'desc')
search_request.sort = [sort_by_id, '_score']
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Тестовый документ');
sortById = new Manticoresearch.SortOrder('id', 'desc');
searchRequest.sort = [sortById, 'id'];
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
searchRequest.setIndex("test");
QueryFilter queryFilter = new QueryFilter();
queryFilter.setQueryString("Тестовый документ");
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
searchRequest.FulltextFilter = new QueryFilter("Тестовый документ");
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
    query_string: Some(serde_json::json!("Тестовый документ").into()),
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
    query_string: {'Тестовый документ'},
  },
  sort: [{'id': 'desc'}, '_score'],
}
```

<!-- intro -->

##### Go:

<!-- request go -->
```go
searchRequest.SetIndex("test")
query := map[string]interface{} {"query_string": "Тестовый документ"}
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
    "match": { "title": "Тестовый документ" }
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

        "title": "Тестовый документ 5"

      }

    },

    {

      "_id": 5406864699109146631,

      "_score": 2319,

      "_source": {

        "title": "Тестовый документ 4"

      }

    },

    {

      "_id": 5406864699109146630,

      "_score": 2319,

      "_source": {

        "title": "Тестовый документ 3"

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
$search->setIndex("test")->match('Тестовый документ')->sort('id', 'desc');
```

<!-- intro -->

##### Python:

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Тестовый документ')
sort_by_id = manticoresearch.model.SortOrder('id', 'desc')
search_request.sort = [sort_by_id]
```

<!-- intro -->

##### Python-asyncio:

<!-- request Python-asyncio -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Тестовый документ')
sort_by_id = manticoresearch.model.SortOrder('id', 'desc')
search_request.sort = [sort_by_id]
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Тестовый документ');
sortById = new Manticoresearch.SortOrder('id', 'desc');
searchRequest.sort = [sortById];
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
searchRequest.setIndex("тест");
QueryFilter queryFilter = new QueryFilter();
queryFilter.setQueryString("Тестовый документ");
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
var searchRequest = new SearchRequest("тест");
searchRequest.FulltextFilter = new QueryFilter("Тестовый документ");
searchRequest.Sort = new List<Object>();
var sortById = new SortOrder("id", SortOrder.OrderEnum.Desc);
searchRequest.Sort.Add(sortById);
```

<!-- intro -->

##### Rust:

<!-- request Rust -->
``` rust
let query = SearchQuery {
    query_string: Some(serde_json::json!("Тестовый документ").into()),
    ..Default::default(),
};
let mut sort_by_id = HashMap::new();
sort_by_id.insert("id".to_string(), "desc".to_string()); 
let sort = [HashMap; 1] = [sort_by_id];
let search_req = SearchRequest {
    table: "тест".to_string(),
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
  index: 'тест',
  query: {
    query_string: {'Тестовый документ'},
  },
  sort: { {'id': {'order':'desc'} },
}
```

<!-- intro -->

##### Go:

<!-- request go -->
```go
searchRequest.SetIndex("тест")
query := map[string]interface{} {"query_string": "Тестовый документ"}
searchRequest.SetQuery(query)
sort := map[string]interface{} { "id": {"order":"desc"} }
searchRequest.SetSort(sort)
```

<!-- end -->

<!-- example sorting 4 -->
Сортировка по атрибутам MVA также поддерживается в JSON-запросах. Режим сортировки можно установить с помощью свойства `mode`. Поддерживаются следующие режимы:

* `min`: сортировка по минимальному значению
* `max`: сортировка по максимальному значению

<!-- intro -->

<!-- request JSON -->

```json
{
  "table":"тест",
  "query":
  {
    "match": { "title": "Тестовый документ" }
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

        "title": "Тестовый документ 4"

      }

    },

    {

      "_id": 5406864699109146629,

      "_score": 2319,

      "_source": {

        "title": "Тестовый документ 2"

      }

    },

    {

      "_id": 5406864699109146628,

      "_score": 2319,

      "_source": {

        "title": "Тестовый документ 1"

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
$search->setIndex("тест")->match('Тестовый документ')->sort('id','desc','max');
```

<!-- intro -->

##### Python:

<!-- request Python -->
``` python
search_request.index = 'тест'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Тестовый документ')
sort = manticoresearch.model.SortMVA('attr_mva', 'desc', 'max')
search_request.sort = [sort]
```

<!-- intro -->

##### Python-asyncio:

<!-- request Python-asyncio -->
``` python
search_request.index = 'тест'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Тестовый документ')
sort = manticoresearch.model.SortMVA('attr_mva', 'desc', 'max')
search_request.sort = [sort]
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
searchRequest.index = "тест";
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Тестовый документ');
sort = new Manticoresearch.SortMVA('attr_mva', 'desc', 'max');
searchRequest.sort = [sort];
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
searchRequest.setIndex("тест");
QueryFilter queryFilter = new QueryFilter();
queryFilter.setQueryString("Тестовый документ");
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
var searchRequest = new SearchRequest("тест");
searchRequest.FulltextFilter = new QueryFilter("Тестовый документ");
var sort = new SortMVA("attr_mva", SortMVA.OrderEnum.Desc, SortMVA.ModeEnum.Max);
searchRequest.Sort.Add(sort);
```

<!-- intro -->

##### Rust:

<!-- request Rust -->
``` rust
let query = SearchQuery {
    query_string: Some(serde_json::json!("Тестовый документ").into()),
    ..Default::default(),
};
let mut sort_mva_opts = HashMap::new();
sort_mva_opts.insert("order".to_string(), "desc".to_string());
sort_mva_opts.insert("mode".to_string(), "max".to_string());
let mut sort_mva = HashMap::new();
sort_mva.insert("attr_mva".to_string(), sort_mva_opts); 

let search_req = SearchRequest {
    table: "тест".to_string(),
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
При сортировке по атрибуту вычисление веса (оценки) совпадения отключено по умолчанию (ранжировщик не используется). Вы можете включить вычисление веса, установив свойство `track_scores` в `true`:

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
# Сортировка и ранжирование

Результаты запроса могут быть отсортированы по весу ранжирования полного текста, одному или нескольким атрибутам или выражениям.

**Запросы полного текста** возвращают совпадения, отсортированные по умолчанию. Если ничего не указано, они сортируются по релевантности, что эквивалентно `ORDER BY weight() DESC` в формате SQL.

**Запросы не полного текста** по умолчанию не выполняют сортировку.

## Расширенная сортировка

Расширенный режим автоматически включается, когда вы явно указываете правила сортировки, добавляя оператор `ORDER BY` в формате SQL или используя параметр `sort` через HTTP JSON.

### Сортировка через SQL

Общий синтаксис:
```sql
SELECT ... ORDER BY
{attribute_name | expr_alias | weight() | random() } [ASC | DESC],
...
{attribute_name | expr_alias | weight() | random() } [ASC | DESC]
```

<!-- example alias -->

В операторе сортировки вы можете использовать любое сочетание до 5 столбцов, каждый из которых следует за `asc` или `desc`. Функции и выражения не допускаются в качестве аргументов для оператора сортировки, за исключением функций `weight()` и `random()` (последнюю можно использовать только через SQL в виде `ORDER BY random()`). Тем не менее, вы можете использовать любое выражение в списке SELECT и сортировать по его псевдониму.

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
`"sort"` указывает массив, где каждый элемент может быть именем атрибута или `_score`, если вы хотите сортировать по весам совпадений. В этом случае порядок сортировки по умолчанию восходящий для атрибутов и нисходящий для `_score`.

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

* `asc`: сортировать в восходящем порядке
* `desc`: сортировать в нисходящем порядке


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
You can also use another syntax and specify the sort order via the `order` property:
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
Sorting by MVA attributes is also supported in JSON queries. Sorting mode can be set via the `mode` property. The following modes are supported:
* `min`: sort by minimum value
* `max`: sort by maximum value
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
При сортировке по атрибуту вычисление веса совпадения (score) по умолчанию отключено (ранкер не используется). Вы можете включить вычисление веса, установив свойство `track_scores` в значение `true`:

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

##### Typescript:
<!-- request typescript -->
``` typescript
searchRequest = {
  index: 'test',
  track_scores: true,
  query: {
    query_string: {'Тестовый документ'},
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
query := map[string]interface{} {"query_string": "Тестовый документ"}
searchRequest.SetQuery(query)
sort := map[string]interface{} { "attr_mva": { "order":"desc", "mode":"max" } }
searchRequest.SetSort(sort)
```

<!-- end -->


## Обзор ранжирования

Ранжирование (также известное как взвешивание) результатов поиска можно определить как процесс вычисления так называемой релевантности (веса) для каждого соответствующего документа относительно данного запроса, который его совпадает. Таким образом, релевантность в конечном счете - это просто число, прикрепленное к каждому документу, оценивающее, насколько документ соответствует запросу. Результаты поиска могут быть отсортированы по этому числу и/или некоторым дополнительным параметрам, так что наиболее запрашиваемые результаты будут отображаться выше на странице результатов.

Нет единого стандартного универсального способа ранжирования любого документа в любом сценарии. Более того, такого способа никогда не может быть, потому что релевантность *субъективна*. То есть, то, что кажется вам релевантным, может не показаться релевантным мне. Следовательно, в общем случае это не просто сложно вычислить; это теоретически невозможно.

Таким образом, ранжирование в Manticore настраиваемо. У него есть понятие так называемого **ранжера**. Ранжер формально может быть определен как функция, которая принимает документ и запрос в качестве входных данных и выдает значение релевантности на выходе. Похожими словами, ранжер контролирует именно то, как (с помощью какого конкретного алгоритма) Manticore будет присваивать веса документам.

## Доступные встроенные ранжеры

Manticore поставляется с несколькими встроенными ранжерами, подходящими для различных целей. Многие из них используют два фактора: близость фраз (также известная как LCS) и BM25. Близость фраз работает на позициях ключевых слов, в то время как BM25 работает на частотах ключевых слов. По сути, чем лучше степень совпадения фразы между телом документа и запросом, тем выше близость фразы (она максимальна, когда документ содержит весь запрос в виде дословной цитаты). А BM25 выше, когда документ содержит больше редких слов. Подробное обсуждение мы оставим на потом.

В настоящее время реализованы следующие ранжеры:

* `proximity_bm25`, режим ранжирования по умолчанию, который использует и комбинирует как близость фраз, так и ранжирование BM25.
* `bm25`, статистический режим ранжирования, который использует только ранжирование BM25 (аналогично большинству других поисковых систем полного текста). Этот режим быстрее, но может привести к худшему качеству для запросов, содержащих более одного ключевого слова.
* `none`, режим без ранжирования. Этот режим, очевидно, самый быстрый. Ко всем совпадениям присваивается вес 1. Это иногда называют булевым поиском, который просто находит документы, но не ранжирует их.
* `wordcount`, ранжирование по количеству вхождений ключевых слов. Этот ранжер вычисляет количество вхождений ключевых слов по полям, затем умножает их на веса полей и суммирует полученные значения.
* `proximity` возвращает сырое значение близости фраз в качестве результата. Этот режим внутренне используется для эмуляции запросов `SPH_MATCH_ALL`.
* `matchany` возвращает ранг, как он был вычислен в режиме `SPH_MATCH_ANY` ранее, и внутренне используется для эмуляции запросов `SPH_MATCH_ANY`.
* `fieldmask` возвращает 32-битную маску с N-м битом, соответствующим N-му полю полного текста, нумеруемому с 0. Бит будет установлен только тогда, когда соответствующее поле имеет любые вхождения ключевых слов, удовлетворяющие запросу.
* `sph04` основан на ранжере по умолчанию 'proximity_bm25', но дополнительно увеличивает вес совпадений, когда они происходят в самом начале или в самом конце текстового поля. Таким образом, если поле соответствует точному запросу, `sph04` должен оценить его выше, чем поле, которое содержит точный запрос, но не равно ему. (Например, когда запрос "Hyde Park", документ с заголовком "Hyde Park" должен иметь более высокий ранг, чем документ с заголовком "Hyde Park, London" или "The Hyde Park Cafe".)
* `expr` позволяет вам указать формулу ранжирования во время выполнения. Она предоставляет несколько внутренних текстовых факторов и позволяет вам определить, как следует вычислить окончательный вес из этих факторов. Более подробную информацию о ее синтаксисе и справочник доступных факторов можно найти в [разделе ниже](../Searching/Sorting_and_ranking.md#Quick-summary-of-the-ranking-factors).

Имя ранжера не учитывает регистр. Пример:

```sql
SELECT ... OPTION ranker=sph04;
```

## Краткое резюме факторов ранжирования

| Name                    | Level     | Type  | Summary                                                                                                            |
| ----------------------- | --------- | ----- | ------------------------------------------------------------------------------------------------------------------ |
| max_lcs                 | query     | int   | максимальное возможное значение LCS для текущего запроса                                                             |
| bm25                    | document  | int   | быстлая оценка BM25(1.2, 0)                                                                                       |
| bm25a(k1, b)            | document  | int   | точное значение BM25() с настраиваемыми константами K1, B и поддержкой синтаксиса                               |
| bm25f(k1, b, {field=weight, ...}) | document | int   | точное значение BM25F() с дополнительными настраиваемыми весами полей                                             |
| field_mask              | document  | int   | битовая маска совпадающих полей                                                                                   |
| query_word_count        | document  | int   | количество уникальных включительных ключевых слов в запросе                                                         |
| doc_word_count          | документ  | int   | количество уникальных ключевых слов, найденных в документе                                                           |
| lcs                     | поле      | int   | Наибольшая Общая Подпоследовательность между запросом и документом, в словах                                         |
| user_weight             | поле      | int   | вес пользовательского поля                                                                                           |
| hit_count               | поле      | int   | общее количество вхождений ключевых слов                                                                             |
| word_count              | поле      | int   | количество уникальных найденных ключевых слов                                                                        |
| tf_idf                  | поле      | float | sum(tf*idf) по найденным ключевым словам == sum(idf) по вхождениям                                                |
| min_hit_pos             | поле      | int   | позиция первого найденного вхождения, в словах, с 1                         |
| min_best_span_pos       | поле      | int   | позиция первого максимального диапазона LCS, в словах, с 1                         |
| exact_hit               | поле      | bool  | совпадает ли запрос с полем                                                                                          |
| min_idf                 | поле      | float | min(idf) по найденным ключевым словам                                                                                |
| max_idf                 | поле      | float | max(idf) по найденным ключевым словам                                                                                |
| sum_idf                 | поле      | float | sum(idf) по найденным ключевым словам                                                                                |
| exact_order             | поле      | bool  | совпадают ли все ключевые слова запроса а) с найденными и б) в порядке запроса                                      |
| min_gaps                | поле      | int   | минимальное количество пробелов между найденными ключевыми словами по соответствующим диапазонам                     |
| lccs                    | поле      | int   | Наибольшая Общая Непрерывная Подпоследовательность между запросом и документом, в словах                             |
| wlccs                   | поле      | float | Взвешенная Наибольшая Общая Непрерывная Подпоследовательность, sum(idf) по смежным диапазонам ключевых слов               |
| atc                     | поле      | float | Консолидированная Близость Терминов, log(1+sum(idf1*idf2*pow(distance, -1.75)) по лучшим парам ключевых слов       |
### Факторы ранжирования на уровне документа
**Документный фактор** — это числовое значение, вычисляемое движком ранжирования для каждого найденного документа в соответствии с текущим запросом. Поэтому он отличается от простого атрибута документа тем, что атрибут не зависит от полного текстового запроса, тогда как факторы могут. Эти факторы могут использоваться в любом месте в выражении ранжирования. В настоящее время реализованные факторы на уровне документа:
* `bm25` (целое число), оценка BM25 на уровне документа (вычисляется без фильтрации по вхождениям ключевых слов).
* `max_lcs` (целое число), максимальное возможное значение на уровне запроса, которое выражение `sum(lcs*user_weight)` может когда-либо принимать. Это может быть полезно для масштабирования повышения веса. Например, формула ранжирования `MATCHANY` использует это для гарантии того, что полное совпадение фразы в любом поле имеет более высокий ранг, чем любая комбинация частичных совпадений во всех полях.
* `field_mask` (целое число), 32-битная маска документа на уровне документа с совпадающими полями. 
* `query_word_count` (целое число), количество уникальных ключевых слов в запросе, отрегулированное по количеству исключенных ключевых слов. Например, как в запросах `(one one one one)`, так и `(one !two)` этому фактору должно быть назначено значение 1, потому что существует всего одно уникальное не исключенное ключевое слово.
* `doc_word_count` (целое число), количество уникальных ключевых слов, найденных в документе.
### Факторы ранжирования на уровне поля
**Поле-фактор** — это числовое значение, вычисляемое движком ранжирования для каждого найденного текстового поля документа в соответствии с текущим запросом. Поскольку более одного поля может совпадать с запросом, но окончательный вес должен быть единым целым числом, эти значения должны быть свернуты в одно. Для этого факторы на уровне поля могут использоваться только в функциях агрегирования по полям, они **не могут** использоваться в любом другом месте в выражении. Например, вы не можете использовать `(lcs+bm25)` в качестве вашего выражения ранжирования, поскольку `lcs` принимает несколько значений (по одному в каждом совпадающем поле). Вместо этого вы должны использовать `(sum(lcs)+bm25)`, это выражение суммирует `lcs` по всем совпадающим полям, а затем добавляет `bm25` к этой сумме по полю. В настоящее время реализованные факторы на уровне поля:
* `lcs` (целое число), длина максимального точного совпадения между документом и запросом, посчитанная в словах. LCS означает Долгую Общую Последовательность (или Подмножество). Принимает минимальное значение 1, когда в поле совпадают только случайные ключевые слова, и максимальное значение количества ключевых слов запроса, когда весь запрос был точен в поле (в точном порядке ключевых слов запроса). Например, если запрос 'hello world' и поле содержит эти два слова, взятых из запроса (то есть, смежные друг с другом, и точно в порядке запроса), `lcs` будет 2. Например, если запрос 'hello world program' и поле содержит 'hello world', `lcs` будет 2. Обратите внимание, что любое подмножество ключевого слова запроса работает, не только подмножество смежных ключевых слов. Например, если запрос 'hello world program' и поле содержит 'hello (test program)', `lcs` будет 2, так как оба 'hello' и 'program' совпадают на тех же соответствующих позициях, на которых они были в запросе. Наконец, если запрос 'hello world program' и поле содержит 'hello world program', `lcs` будет 3. (Надеюсь, это не удивительно на данном этапе.)
* `user_weight` (целое число), пользовательский заданный вес на поле (смотрите [OPTION field_weights](../Searching/Options.md#field_weights) в SQL). Веса по умолчанию равны 1, если не указаны явно.
* `hit_count` (целое число), количество вхождений ключевых слов, которые совпали в поле. Обратите внимание, что одно ключевое слово может встречаться несколько раз. Например, если 'hello' встречается 3 раза в поле, а 'world' 5 раз, `hit_count` будет 8.
* `word_count` (целое число), количество уникальных ключевых слов, совпавших в поле. Например, если 'hello' и 'world' встречаются где угодно в поле, `word_count` будет 2, независимо от того, сколько раз оба ключевых слова встречаются.
* `tf_idf` (число с плавающей запятой), сумма TF/IDF по всем ключевым словам, совпавшим в поле. IDF — это Обратная Частота Документа, значение числа с плавающей запятой между 0 и 1, которое описывает, насколько часто встречается это ключевое слово (в основном, 0 для ключевого слова, которое встречается в каждом индексированном документе, и 1 для единственного уникального ключевого слова, которое встречается только в одном документе). TF — это Частота Терминов, количество вхождений совпавших ключевых слов в поле. В качестве примечания, `tf_idf` на самом деле вычисляется путем суммирования IDF по всем совпавшим вхождениям. Это по конструкции эквивалентно суммированию TF*IDF по всем совпавшим ключевым словам.
* `min_hit_pos` (целое число), позиция первого совпадения ключевого слова, считая в словах

   Следовательно, это относительно низкоуровневый, "сырой" фактор, который вы, вероятно, захотите *откорректировать* перед использованием для ранжирования. Специфические корректировки сильно зависят от ваших данных и итоговой формулы, но вот несколько идей, с которых можно начать: (a) любые усиления на основе min_gaps можно просто игнорировать, когда word_count<2;

    (b) нетривиальные значения min_gaps (т.е. когда word_count>=2) могут быть ограничены определенной "худшей" константой, тогда как тривиальные значения (т.е. когда min_gaps=0 и word_count<2) могут быть заменены этой константой;

    (c) может быть применена функция передачи, такая как 1/(1+min_gaps) (так, что лучшие, меньшие значения min_gaps будут максимизироваться, а худшие, большие значения min_gaps будут медленно уменьшаться); и так далее.
* `lccs` (целое число). Долгая Общая Непрерывная Последовательность. Длина самой длинной подфразы, общей между запросом и документом, вычисляемая по ключевым словам.

    Фактор LCCS несколько похож на LCS, но более ограничителен. Хотя LCS может быть больше 1, даже если ни два слова запроса не совпадают рядом друг с другом, LCCS будет больше 1 только в том случае, если в документе есть *точные*, смежные подфразы запроса. Например, (один два три четыре пять) запрос против (один сто триста пятьсот) документ даст lcs=3, но lccs=1, потому что, хотя взаимные расположения 3 ключевых слов (один, три, пять) совпадают между запросом и документом, ни 2 совпадающие позиции на самом деле не смежные.

    Обратите внимание, что LCCS все равно не различает между частыми и редкими ключевыми словами; для этого смотрите WLCCS.
* `wlccs` (число с плавающей запятой). Взвешенная Долгая Общая Непрерывная Последовательность. Сумма IDF ключевых слов самой длинной подфразы, общей между запросом и документом.

    WLCCS вычисляется аналогично LCCS, но каждое "подходящее" вхождение ключевого слова увеличивает его по IDF ключевого слова вместо просто на 1 (как с LCS и LCCS). Это позволяет ранжировать последовательности более редких и более важных ключевых слов выше, чем последовательности частых ключевых слов, даже если последние длиннее. Например, запрос `(Zanzibar bed and breakfast)` даст lccs=1 для документа `(hotels of Zanzibar)`, но lccs=3 против `(London bed and breakfast)`, даже если "Zanzibar" на самом деле несколько реже, чем вся фраза "bed and breakfast". Фактор WLCCS решает эту проблему, используя частоты ключевых слов.
* `atc` (число с плавающей запятой). Совокупная Близость Терминов. Мера, основанная на близости, которая увеличивается, когда документ содержит больше групп более близко расположенных и более важных (редких) ключевых слов запроса.

    **ПРЕДУПРЕЖДЕНИЕ:** вам следует использовать ATC с OPTION idf='plain,tfidf_unnormalized' (см. [ниже](../Searching/Sorting_and_ranking.md#Configuration-of-IDF-formula)); в противном случае вы можете получить неожиданные результаты.

    ATC функционирует следующим образом. Для каждого *вхождения* ключевого слова в документе мы вычисляем так называемую *близость терминов*. Для этого мы рассматриваем все другие ближайшие вхождения всех ключевых слов запроса (включая само ключевое слово) слева и справа от рассматриваемого вхождения, вычисляем коэффициент затухания расстояния как k = pow(distance, -1.75) для этих вхождений и суммируем затухшие IDF. В результате, для каждого вхождения каждого ключевого слова мы получаем значение "близости", которое описывает "соседей" этого вхождения. Затем мы умножаем эти близости по вхождениям на соответствующий IDF ключевого слова, суммируем их все и, наконец, вычисляем логарифм этой суммы.
Другими словами, мы обрабатываем лучшие (ближайшие) совпадающие пары ключевых слов в документе и вычисляем попарные "близости" как произведение их IDF, умноженное на коэффициент расстояния:

```
pair_tc = idf(pair_word1) * idf(pair_word2) * pow(pair_distance, -1.75)
```

Затем мы суммируем такие близости и вычисляем финальное значение ATC с логарифмическим ослаблением:

```
atc = log(1+sum(pair_tc))
```

Обратите внимание, что этот финальный ослабляющий логарифм является точно той причиной, по которой вам следует использовать OPTION idf=plain, потому что без него выражение внутри log() может быть отрицательным.

Более близкие вхождения ключевых слов вносят *гораздо* больший вклад в ATC, чем более частые ключевые слова. На самом деле, когда ключевые слова расположены очень близко друг к другу, distance=1 и k=1; когда между ними только одно слово, distance=2 и k=0.297; с двумя словами между ними, distance=3 и k=0.146 и так далее. В то же время, IDF ослабляет несколько медленнее. Например, в коллекции из 1 миллиона документов значения IDF для ключевых слов, совпадающих в 10, 100 и 1000 документах, будут соответственно 0.833, 0.667 и 0.500. Таким образом, пара ключевых слов с двумя довольно редкими ключевыми словами, которые встречаются всего в 10 документах каждое, но с 2 другими словами между ними, будет давать pair_tc = 0.101 и, таким образом, едва превзойдёт пару с ключевым словом на 100 документах и ключевым словом на 1000 документах с 1 другим словом между ними и pair_tc = 0.099. Более того, пара из двух *уникальных*, 1-документных ключевых слов с 3 словами между ними получит pair_tc = 0.088 и проиграет паре из двух 1000-документных ключевых слов, расположенных вплотную друг к другу, и дающих pair_tc = 0.25. Таким образом, в основном, хотя ATC сочетает как частоту ключевых слов, так и близость, он всё же немного предпочитает близость.

### Функции агрегации факторов ранжирования

**Функция агрегации по полям** представляет собой функцию с одним аргументом, которая принимает выражение с факторами на уровне полей, проходит по всем совпадающим полям и вычисляет окончательные результаты. В настоящее время реализованные функции агрегации по полям включают:

* `sum`, которая добавляет выражение аргумента в всех совпадающих полях. Например, `sum(1)` должен возвращать количество совпадающих полей.
* `top`, которая возвращает наивысшее значение аргумента во всех совпадающих полях.
* `max_window_hits`, управляет скользящим окном позиций попаданий, чтобы отслеживать максимальное количество попаданий в пределах заданного размера окна. Она удаляет устаревшие попадания, выходящие за пределы окна, и добавляет последнее попадание, обновляя максимальное количество попаданий, найденных в этом окне.

### Формулы для всех встроенных ранжировщиков

Большинство других ранжировщиков на самом деле можно эмулировать с использованием ранжировщика на основе выражений. Вам просто нужно предоставить подходящее выражение. Хотя такая эмуляция будет, вероятно, медленнее, чем использование встроенного, компилированного ранжировщика, это может быть интересно, если вы хотите точно подогнать вашу формулу ранжирования, начиная с одной из существующих. Кроме того, формулы описывают детали ранжировщика ясным и читаемым образом.

* proximity_bm25 (ранжировщик по умолчанию) = `sum(lcs*user_weight)*1000+bm25`
* bm25 = `sum(user_weight)*1000+bm25`
* none = `1`
* wordcount = `sum(hit_count*user_weight)`
* proximity = `sum(lcs*user_weight)`
* matchany = `sum((word_count+(lcs-1)*max_lcs)*user_weight)`
* fieldmask = `field_mask`
* sph04 = `sum((4*lcs+2*(min_hit_pos==1)+exact_hit)*user_weight)*1000+bm25`


### Конфигурация формулы IDF

Исторически сложившийся IDF (инверсная частота документа) в Manticore эквивалентен `OPTION idf='normalized,tfidf_normalized'`, и эти нормализации могут вызывать несколько нежелательных эффектов.

Во-первых, `idf=normalized` вызывает пенализацию ключевых слов. Например, если вы ищете `the | something`, и `the` встречается более чем в 50% документов, то документы с обоими ключевыми словами `the` и `[something` получат меньший вес, чем документы только с одним ключевым словом `something`. Использование `OPTION idf=plain` избегает этого.

Простой IDF варьируется в диапазоне `[0, log(N)]`, и ключевые слова никогда не подвергаются пенализации; в то время как нормализованный IDF варьируется в диапазоне `[-log(N), log(N)]`, и слишком частые ключевые слова подвергаются пенализации.

Во-вторых, `idf=tfidf_normalized` вызывает дрейф IDF по запросам. Исторически мы дополнительно делили IDF на количество ключевых слов в запросе, так что вся `sum(tf*idf)` по всем ключевым словам всё еще укладывалась в диапазон `[0,1]`. Однако это означает, что запросы `word1` и `word1 | nonmatchingword2` будут присваивать разные веса одной и той же выборке результатов, потому что IDFs для обоих `word1` и `nonmatchingword2` будут разделены на 2. `OPTION idf='tfidf_unnormalized'` исправляет это. Обратите внимание, что факторы ранжирования BM25, BM25A, BM25F() будут соответствующим образом масштабироваться, как только вы отключите эту нормализацию.

Флаги IDF могут сочетаться; `plain` и `normalized` являются взаимно исключающими; `tfidf_unnormalized` и `tfidf_normalized` являются взаимно исключающими; и неопределенные флаги в такой взаимно исключающей группе принимают свои значения по умолчанию. Это означает, что `OPTION idf=plain` эквивалентно полной спецификации `OPTION idf='plain,tfidf_normalized'`.    

<!-- proofread -->

































































