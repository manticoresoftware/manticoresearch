# Группировка результатов поиска

<!-- example general -->
Группировка результатов поиска часто полезна для получения количества совпадений в каждой группе или других агрегатов. Например, это удобно для создания графика, показывающего количество совпадающих блог-постов по месяцам, или для группировки результатов веб-поиска по сайту, постов на форумах по автору и т. д.

Manticore поддерживает группировку результатов поиска по одному или нескольким столбцам и вычисляемым выражениям. Результаты могут:

* Сортироваться внутри группы
* Возвращать более одной строки на группу
* Фильтроваться по группам
* Сортироваться по группам
* Агрегироваться с помощью [функций агрегации](../Searching/Grouping.md#Aggregation-functions)

<!-- intro -->
Общий синтаксис:

<!-- request SQL -->
Общий синтаксис
```sql
SELECT {* | SELECT_expr [, SELECT_expr ...]}
...
GROUP BY {field_name | alias } [, ...]
[HAVING where_condition]
[WITHIN GROUP ORDER BY field_name {ASC | DESC} [, ...]]
...

SELECT_expr: { field_name | function_name(...) }
where_condition: {aggregation expression alias | COUNT(*)}
```

<!-- request JSON -->
Формат JSON-запроса в настоящее время поддерживает базовую группировку, которая может извлекать агрегированные значения и их count(*).

```json
{
  "table": "<table_name>",
  "limit": 0,
  "aggs": {
    "<aggr_name>": {
      "terms": {
        "field": "<attribute>",
        "size": <int value>
      }
    }
  }
}
```

Стандартный вывод запроса возвращает набор результатов без группировки, который можно скрыть с помощью `limit` (или `size`).
Для агрегации необходимо задать `size` для размера результата группы.

<!-- end -->

<!-- example group1 -->
### Простая группировка
Группировка весьма проста — просто добавьте "GROUP BY smth" в конец вашего `SELECT`-запроса. Что может быть "smth":

* Любое не полнотекстовое поле из таблицы: integer, float, string, MVA (мульти-значный атрибут)
* Или, если вы использовали псевдоним в списке `SELECT`, вы также можете группировать по нему

Можно опустить любые [функции агрегации](../Searching/Grouping.md#Aggregation-functions) в списке `SELECT`, и это все равно будет работать:

<!-- intro -->
##### Пример:

<!-- request SQL -->
```sql
SELECT release_year FROM films GROUP BY release_year LIMIT 5;
```
<!-- response SQL -->
```sql
+--------------+
| release_year |
+--------------+
|         2004 |
|         2002 |
|         2001 |
|         2005 |
|         2000 |
+--------------+
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SELECT release_year FROM films GROUP BY release_year LIMIT 5;"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "release_year": {
          "type": "long"
        }
      }
    ],
    "data": [
      {
        "release_year": 2004
      },
            {
        "release_year": 2002
      },
            {
        "release_year": 2001
      },
            {
        "release_year": 2005
      },
            {
        "release_year": 2000
      }
    ],
    "total": 5,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->
<!-- example group2 -->
Тем не менее, чаще всего вы захотите получить агрегированные данные для каждой группы, например:

* `COUNT(*)` — просто получить количество элементов в каждой группе
* или `AVG(field)` — вычислить среднее значение поля в группе

Для HTTP JSON запросов использование одного `aggs` с `limit=0` на основном уровне запроса работает аналогично SQL-запросу с `GROUP BY` и `COUNT(*)`, обеспечивая эквивалентное поведение и производительность.

<!-- intro -->
##### Пример:

<!-- request SQL1 -->
```sql
SELECT release_year, count(*) FROM films GROUP BY release_year LIMIT 5;
```
<!-- response SQL1 -->
```sql
+--------------+----------+
| release_year | count(*) |
+--------------+----------+
|         2004 |      108 |
|         2002 |      108 |
|         2001 |       91 |
|         2005 |       93 |
|         2000 |       97 |
+--------------+----------+
```
<!-- request SQL2 -->
```sql
SELECT release_year, AVG(rental_rate) FROM films GROUP BY release_year LIMIT 5;
```
<!-- response SQL2 -->
```sql
+--------------+------------------+
| release_year | avg(rental_rate) |
+--------------+------------------+
|         2004 |       2.78629661 |
|         2002 |       3.08259249 |
|         2001 |       3.09989142 |
|         2005 |       2.90397978 |
|         2000 |       3.17556739 |
+--------------+------------------+
```

<!-- request JSON -->
``` json
POST /search -d '
    {
     "table" : "films",
     "limit": 0,
     "aggs" :
     {
        "release_year" :
         {
            "terms" :
             {
              "field":"release_year",
              "size":100
             }
         }
     }
    }
'
```
<!-- response JSON -->
``` json
{
  "took": 2,
  "timed_out": false,
  "hits": {
    "total": 10000,
    "hits": [

    ]
  },
  "release_year": {
    "group_brand_id": {
      "buckets": [
        {
          "key": 2004,
          "doc_count": 108
        },
        {
          "key": 2002,
          "doc_count": 108
        },
        {
          "key": 2000,
          "doc_count": 97
        },
        {
          "key": 2005,
          "doc_count": 93
        },
        {
          "key": 2001,
          "doc_count": 91
        }
      ]
    }
  }
}
```
<!-- request PHP -->
``` php
$index->setName('films');
$search = $index->search('');
$search->limit(0);
$search->facet('release_year','release_year',100);
$results = $search->get();
print_r($results->getFacets());
```
<!-- response PHP -->
``` php
Array
(
    [release_year] => Array
        (
            [buckets] => Array
                (
                    [0] => Array
                        (
                            [key] => 2009
                            [doc_count] => 99
                        )
                    [1] => Array
                        (
                            [key] => 2008
                            [doc_count] => 102
                        )
                    [2] => Array
                        (
                            [key] => 2007
                            [doc_count] => 93
                        )
                    [3] => Array
                        (
                            [key] => 2006
                            [doc_count] => 103
                        )
                    [4] => Array
                        (
                            [key] => 2005
                            [doc_count] => 93
                        )
                    [5] => Array
                        (
                            [key] => 2004
                            [doc_count] => 108
                        )
                    [6] => Array
                        (
                            [key] => 2003
                            [doc_count] => 106
                        )
                    [7] => Array
                        (
                            [key] => 2002
                            [doc_count] => 108
                        )
                    [8] => Array
                        (
                            [key] => 2001
                            [doc_count] => 91
                        )
                    [9] => Array
                        (
                            [key] => 2000
                            [doc_count] => 97
                        )
                )
        )
)
```
<!-- request Python -->
``` python
res =searchApi.search({"table":"films","limit":0,"aggs":{"release_year":{"terms":{"field":"release_year","size":100}}}})
```
<!-- response Python -->
``` python
{'aggregations': {u'release_year': {u'buckets': [{u'doc_count': 99,
                                                  u'key': 2009},
                                                 {u'doc_count': 102,
                                                  u'key': 2008},
                                                 {u'doc_count': 93,
                                                  u'key': 2007},
                                                 {u'doc_count': 103,
                                                  u'key': 2006},
                                                 {u'doc_count': 93,
                                                  u'key': 2005},
                                                 {u'doc_count': 108,
                                                  u'key': 2004},
                                                 {u'doc_count': 106,
                                                  u'key': 2003},
                                                 {u'doc_count': 108,
                                                  u'key': 2002},
                                                 {u'doc_count': 91,
                                                  u'key': 2001},
                                                 {u'doc_count': 97,
                                                  u'key': 2000}]}},
 'hits': {'hits': [], 'max_score': None, 'total': 1000},
 'profile': None,
 'timed_out': False,
 'took': 0}

```

<!-- request Python-asyncio -->
``` python
res = await searchApi.search({"table":"films","limit":0,"aggs":{"release_year":{"terms":{"field":"release_year","size":100}}}})
```
<!-- response Python-asyncio -->
``` python
{'aggregations': {u'release_year': {u'buckets': [{u'doc_count': 99,
                                                  u'key': 2009},
                                                 {u'doc_count': 102,
                                                  u'key': 2008},
                                                 {u'doc_count': 93,
                                                  u'key': 2007},
                                                 {u'doc_count': 103,
                                                  u'key': 2006},
                                                 {u'doc_count': 93,
                                                  u'key': 2005},
                                                 {u'doc_count': 108,
                                                  u'key': 2004},
                                                 {u'doc_count': 106,
                                                  u'key': 2003},
                                                 {u'doc_count': 108,
                                                  u'key': 2002},
                                                 {u'doc_count': 91,
                                                  u'key': 2001},
                                                 {u'doc_count': 97,
                                                  u'key': 2000}]}},
 'hits': {'hits': [], 'max_score': None, 'total': 1000},
 'profile': None,
 'timed_out': False,
 'took': 0}

```

<!-- request Javascript -->
``` javascript
res = await searchApi.search({"table":"films","limit":0,"aggs":{"release_year":{"terms":{"field":"release_year","size":100}}}});
```
<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"aggregations":{"release_year":{"buckets":[{"key":2009,"doc_count":99},{"key":2008,"doc_count":102},{"key":2007,"doc_count":93},{"key":2006,"doc_count":103},{"key":2005,"doc_count":93},{"key":2004,"doc_count":108},{"key":2003,"doc_count":106},{"key":2002,"doc_count":108},{"key":2001,"doc_count":91},{"key":2000,"doc_count":97}]}},"hits":{"total":1000,"hits":[]}}
```
<!-- request Java -->
``` java
HashMap<String,Object> aggs = new HashMap<String,Object>(){{
    put("release_year", new HashMap<String,Object>(){{
        put("terms", new HashMap<String,Object>(){{
            put("field","release_year");
            put("size",100);
        }});
    }});
}};

searchRequest = new SearchRequest();
searchRequest.setIndex("films");
searchRequest.setLimit(0);
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
searchRequest.setAggs(aggs);
searchResponse = searchApi.search(searchRequest);
```
<!-- response Java -->
``` java
class SearchResponse {
    took: 0
    timedOut: false
    aggregations: {release_year={buckets=[{key=2009, doc_count=99}, {key=2008, doc_count=102}, {key=2007, doc_count=93}, {key=2006, doc_count=103}, {key=2005, doc_count=93}, {key=2004, doc_count=108}, {key=2003, doc_count=106}, {key=2002, doc_count=108}, {key=2001, doc_count=91}, {key=2000, doc_count=97}]}}
    hits: class SearchResponseHits {
        maxScore: null
        total: 1000
        hits: []
    }
    profile: null
}
```

<!-- request C# -->
``` clike
var agg = new Aggregation("release_year", "release_year");
agg.Size = 100;
object query = new { match_all=null };
var searchRequest = new SearchRequest("films", query);
searchRequest.Aggs = new List<Aggregation> {agg};
var searchResponse = searchApi.Search(searchRequest);
```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 0
    timedOut: false
    aggregations: {release_year={buckets=[{key=2009, doc_count=99}, {key=2008, doc_count=102}, {key=2007, doc_count=93}, {key=2006, doc_count=103}, {key=2005, doc_count=93}, {key=2004, doc_count=108}, {key=2003, doc_count=106}, {key=2002, doc_count=108}, {key=2001, doc_count=91}, {key=2000, doc_count=97}]}}
    hits: class SearchResponseHits {
        maxScore: null
        total: 1000
        hits: []
    }
    profile: null
}
```

<!-- request Rust -->
``` rust
let query = SearchQuery::new();
let aggTerms1 = AggTerms::new {
    fields: "release_year".to_string(),
    size: Some(100),
};
let agg1 = Aggregation {
    terms: Some(Box::new(aggTerms1)),
    ..Default::default(),
};
let mut aggs = HashMap::new();
aggs.insert("release_year".to_string(), agg1);

let search_req = SearchRequest {
    table: "films".to_string(),
    query: Some(Box::new(query)),
    aggs: serde_json::json!(aggs),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```
<!-- response Rust -->
``` rust
class SearchResponse {
    took: 0
    timedOut: false
    aggregations: {release_year={buckets=[{key=2009, doc_count=99}, {key=2008, doc_count=102}, {key=2007, doc_count=93}, {key=2006, doc_count=103}, {key=2005, doc_count=93}, {key=2004, doc_count=108}, {key=2003, doc_count=106}, {key=2002, doc_count=108}, {key=2001, doc_count=91}, {key=2000, doc_count=97}]}}
    hits: class SearchResponseHits {
        maxScore: null
        total: 1000
        hits: []
    }
    profile: null
}
```

<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  limit: 0,
  aggs: {
    cat_id: {
      terms: { field: "cat", size: 1 }
    }
  }
});
```

<!-- response TypeScript -->
``` typescript
{
	"took":0,
	"timed_out":false,
	"aggregations":
	{
		"cat_id":
		{
			"buckets":
			[{
				"key":1,
				"doc_count":1
			}]
		}
	},
	"hits":
	{
		"total":5,
		"hits":[]
	}
}
```

<!-- request Go -->
``` go
query := map[string]interface{} {};
searchRequest.SetQuery(query);
aggTerms := manticoreclient.NewAggregationTerms()
aggTerms.SetField("cat")
aggTerms.SetSize(1)
aggregation := manticoreclient.NewAggregation()
aggregation.setTerms(aggTerms)
searchRequest.SetAggregation(aggregation)
res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```

<!-- response Go -->
``` go
{
	"took":0,
	"timed_out":false,
	"aggregations":
	{
		"cat_id":
		{
			"buckets":
			[{
				"key":1,
				"doc_count":1
			}]
		}
	},
	"hits":
	{
		"total":5,
		"hits":[]
	}
}
```

<!-- end -->

<!-- example sort1 -->
##### Сортировка групп
По умолчанию группы не сортируются, и обычно следующим шагом является их упорядочивание по чему-то, например, по полю, по которому идет группировка:

<!-- intro -->
##### Пример:

<!-- request SQL -->
```sql
SELECT release_year, count(*) from films GROUP BY release_year ORDER BY release_year asc limit 5;
```
<!-- response SQL -->
```sql
+--------------+----------+
| release_year | count(*) |
+--------------+----------+
|         2000 |       97 |
|         2001 |       91 |
|         2002 |      108 |
|         2003 |      106 |
|         2004 |      108 |
+--------------+----------+
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SELECT release_year, count(*) from films GROUP BY release_year ORDER BY release_year asc limit 5;"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "release_year": {
          "type": "long"
        }
      },
      {
        "count(*)": {
          "type": "long long"
        }
      }
    ],
    "data": [
      {
        "release_year": 2000,
        "count(*)": 97
      },
      {
        "release_year": 2001,
        "count(*)": 91
      },
      {
        "release_year": 2002,
        "count(*)": 108
      },
      {
        "release_year": 2003,
        "count(*)": 106
      },
      {
        "release_year": 2004,
        "count(*)": 108
      }
    ],
    "total": 5,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->
<!-- example sort2 -->
Или же вы можете сортировать по агрегату:

* по `count(*)`, чтобы показывать группы с наибольшим количеством элементов первыми
* по `avg(rental_rate)`, чтобы показывать фильмы с наивысшим рейтингом первыми. Обратите внимание, что в примере это сделано через псевдоним: `avg(rental_rate)` сначала назначается псевдониму `avg` в списке `SELECT`, а затем просто делается `ORDER BY avg`


<!-- intro -->
##### Пример:

<!-- request SQL1 -->
```sql
SELECT release_year, count(*) FROM films GROUP BY release_year ORDER BY count(*) desc LIMIT 5;
```
<!-- response SQL1 -->
```sql
+--------------+----------+
| release_year | count(*) |
+--------------+----------+
|         2004 |      108 |
|         2002 |      108 |
|         2003 |      106 |
|         2006 |      103 |
|         2008 |      102 |
+--------------+----------+
```

<!-- request SQL2 -->
```sql
SELECT release_year, AVG(rental_rate) avg FROM films GROUP BY release_year ORDER BY avg desc LIMIT 5;
```
<!-- response SQL2 -->
```sql
+--------------+------------+
| release_year | avg        |
+--------------+------------+
|         2006 | 3.26184368 |
|         2000 | 3.17556739 |
|         2001 | 3.09989142 |
|         2002 | 3.08259249 |
|         2008 | 2.99000049 |
+--------------+------------+
```


<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SELECT release_year, count(*) FROM films GROUP BY release_year ORDER BY count(*) desc LIMIT 5;"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "release_year": {
          "type": "long"
        }
      },
      {
        "count(*)": {
          "type": "long long"
        }
      }
    ],
    "data": [
      {
        "release_year": 2004,
        "count(*)": 108
      },
      {
        "release_year": 2004,
        "count(*)": 108
      },
      {
        "release_year": 2003,
        "count(*)": 106
      },
      {
        "release_year": 2006,
        "count(*)": 103
      },
      {
        "release_year": 2008,
        "count(*)": 102
      }
    ],
    "total": 5,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

<!-- example group3 -->
##### ГРУППИРОВКА ПО нескольким полям одновременно
В некоторых случаях может потребоваться группировка не по одному полю, а сразу по нескольким, например, по категории фильма и году выпуска:

<!-- intro -->
##### Пример:

<!-- request SQL -->
```sql
SELECT category_id, release_year, count(*) FROM films GROUP BY category_id, release_year ORDER BY category_id ASC, release_year ASC;
```
<!-- response SQL -->
```sql
+-------------+--------------+----------+
| category_id | release_year | count(*) |
+-------------+--------------+----------+
|           1 |         2000 |        5 |
|           1 |         2001 |        2 |
|           1 |         2002 |        6 |
|           1 |         2003 |        6 |
|           1 |         2004 |        5 |
|           1 |         2005 |       10 |
|           1 |         2006 |        4 |
|           1 |         2007 |        5 |
|           1 |         2008 |        7 |
|           1 |         2009 |       14 |
|           2 |         2000 |       10 |
|           2 |         2001 |        5 |
|           2 |         2002 |        6 |
|           2 |         2003 |        6 |
|           2 |         2004 |       10 |
|           2 |         2005 |        4 |
|           2 |         2006 |        5 |
|           2 |         2007 |        8 |
|           2 |         2008 |        8 |
|           2 |         2009 |        4 |
+-------------+--------------+----------+
```
<!-- request JSON -->
``` json
POST /search -d '
    {
    "size": 0,
    "table": "films",
    "aggs": {
        "cat_release": {
            "composite": {
                "size":5,
                "sources": [
                    { "category": { "terms": { "field": "category_id" } } },
                    { "release year": { "terms": { "field": "release_year" } } }
                ]
            }
        }
    }
    }
'
```
<!-- response JSON -->
``` json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1000,
    "total_relation": "eq",
    "hits": []
  },
  "aggregations": {
    "cat_release": {
      "after_key": {
        "category": 1,
        "release year": 2007
      },
      "buckets": [
        {
          "key": {
            "category": 1,
            "release year": 2008
          },
          "doc_count": 7
        },
        {
          "key": {
            "category": 1,
            "release year": 2009
          },
          "doc_count": 14
        },
        {
          "key": {
            "category": 1,
            "release year": 2005
          },
          "doc_count": 10
        },
        {
          "key": {
            "category": 1,
            "release year": 2004
          },
          "doc_count": 5
        },
        {
          "key": {
            "category": 1,
            "release year": 2007
          },
          "doc_count": 5
        }
      ]
    }
  }
}
```
<!-- end -->

<!-- example group4 -->
##### Дайте мне N строк
Иногда полезно получить не один элемент на группу, а несколько. Это легко достигается с помощью `GROUP N BY`. Например, в следующем случае мы получаем по два фильма на каждый год, а не по одному, как простой `GROUP BY release_year`.

<!-- intro -->
##### Пример:

<!-- request SQL -->
```sql
SELECT release_year, title FROM films GROUP 2 BY release_year ORDER BY release_year DESC LIMIT 6;
```
<!-- response SQL -->
```sql
+--------------+-----------------------------+
| release_year | title                       |
+--------------+-----------------------------+
|         2009 | ALICE FANTASIA              |
|         2009 | ALIEN CENTER                |
|         2008 | AMADEUS HOLY                |
|         2008 | ANACONDA CONFESSIONS        |
|         2007 | ANGELS LIFE                 |
|         2007 | ARACHNOPHOBIA ROLLERCOASTER |
+--------------+-----------------------------+
```


<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SELECT release_year, title FROM films GROUP 2 BY release_year ORDER BY release_year DESC LIMIT 6;"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "release_year": {
          "type": "long"
        }
      },
      {
        "title": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "release_year": 2009,
        "title": "ALICE FANTASIA"
      },
      {
        "release_year": 2009,
        "title": "ALIEN CENTER"
      },
      {
        "release_year": 2008,
        "title": "AMADEUS HOLY"
      },
      {
        "release_year": 2008,
        "title": "ANACONDA CONFESSIONS"
      },
      {
        "release_year": 2007,
        "title": "ANGELS LIFE"
      },
      {
        "release_year": 2007,
        "title": "ARACHNOPHOBIA ROLLERCOASTER"
      }
    ],
    "total": 6,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

<!-- example group5 -->
##### Сортировка внутри группы
Еще одно важное требование для аналитики — сортировать элементы внутри группы. Для этого используйте конструкцию `WITHIN GROUP ORDER BY ... {ASC|DESC}`. Например, получим фильм с наивысшим рейтингом для каждого года. Обратите внимание, что она работает параллельно просто с `ORDER BY`:

* `WITHIN GROUP ORDER BY` сортирует результаты **внутри группы**
* в то время как простой `GROUP BY` **сортирует сами группы**

Они работают совершенно независимо друг от друга.


<!-- intro -->
##### Пример:

<!-- request SQL -->
```sql
SELECT release_year, title, rental_rate FROM films GROUP BY release_year WITHIN GROUP ORDER BY rental_rate DESC ORDER BY release_year DESC LIMIT 5;
```
<!-- response SQL -->
```sql
+--------------+------------------+-------------+
| release_year | title            | rental_rate |
+--------------+------------------+-------------+
|         2009 | AMERICAN CIRCUS  |    4.990000 |
|         2008 | ANTHEM LUKE      |    4.990000 |
|         2007 | ATTACKS HATE     |    4.990000 |
|         2006 | ALADDIN CALENDAR |    4.990000 |
|         2005 | AIRPLANE SIERRA  |    4.990000 |
+--------------+------------------+-------------+
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SELECT release_year, title, rental_rate FROM films GROUP BY release_year WITHIN GROUP ORDER BY rental_rate DESC ORDER BY release_year DESC LIMIT 5;"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "release_year": {
          "type": "long"
        }
      },
      {
        "title": {
          "type": "string"
        }
      },
      {
        "rental_rate": {
          "type": "long"
        }
      }
    ],
    "data": [
      {
        "release_year": 2009,
        "title": "AMERICAN CIRCUS",
        "rental_rate": 4.990000
      },
      {
        "release_year": 2009,
        "title": "ANTHEM LUKE",
        "rental_rate": 4.990000
      },
      {
        "release_year": 2008,
        "title": "ATTACKS HATE",
        "rental_rate": 4.990000
      },
      {
        "release_year": 2008,
        "title": "ALADDIN CALENDAR",
        "rental_rate": 4.990000
      },
      {
        "release_year": 2007,
        "title": "AIRPLANE SIERRA",
        "rental_rate": 4.990000
      }
    ],
    "total": 5,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

<!-- example group6 -->
##### Фильтрация групп
`HAVING expression` — полезный оператор для фильтрации групп. В то время как `WHERE` применяется до группировки, `HAVING` работает с уже сгруппированными данными. Например, оставим только те годы, когда средняя аренда фильмов за этот год была выше 3. Мы получим всего четыре года:

<!-- intro -->
##### Пример:

<!-- request SQL -->
```sql
SELECT release_year, avg(rental_rate) avg FROM films GROUP BY release_year HAVING avg > 3;
```
<!-- response SQL -->
```sql
+--------------+------------+
| release_year | avg        |
+--------------+------------+
|         2002 | 3.08259249 |
|         2001 | 3.09989142 |
|         2000 | 3.17556739 |
|         2006 | 3.26184368 |
+--------------+------------+
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SELECT release_year, avg(rental_rate) avg FROM films GROUP BY release_year HAVING avg > 3;"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "release_year": {
          "type": "long"
        }
      },
      {
        "avg": {
          "type": "long long"
        }
      }
    ],
    "data": [
      {
        "release_year": 2002,
        "avg": 3.08259249
      },
      {
        "release_year": 2001,
        "avg": 3.09989142
      },
      {
        "release_year": 2000,
        "avg": 3.17556739
      },
      {
        "release_year": 2006,
        "avg": 3.26184368
      }
    ],
    "total": 4,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

**Примечание:** Значение `total_found` в [метаданных поискового запроса](../Node_info_and_management/SHOW_META.md#SHOW-META) отражает количество групп, которые соответствуют условию `HAVING`. Это обеспечивает правильную постраничную навигацию при использовании `HAVING` вместе с `GROUP BY`.

<!-- example group7 -->
##### GROUPBY()
Существует функция `GROUPBY()`, которая возвращает ключ текущей группы. Она полезна во многих случаях, особенно когда вы выполняете [GROUP BY по MVA](../Searching/Grouping.md#Grouping-by-MVA-%28multi-value-attributes%29) или по [JSON-значению](../Searching/Grouping.md#Grouping-by-a-JSON-node).

Её также можно использовать в `HAVING`, например, чтобы оставить только годы 2000 и 2002.

Обратите внимание, что использование `GROUPBY()` не рекомендуется, когда вы группируете сразу по нескольким полям. Она всё равно сработает, но так как ключ группы в этом случае является составным из значений полей, он может не выглядеть ожидаемым образом.

<!-- intro -->
##### Пример:

<!-- request SQL -->
```sql
SELECT release_year, count(*) FROM films GROUP BY release_year HAVING GROUPBY() IN (2000, 2002);
```
<!-- response SQL -->
```sql
+--------------+----------+
| release_year | count(*) |
+--------------+----------+
|         2002 |      108 |
|         2000 |       97 |
+--------------+----------+
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SELECT release_year, count(*) FROM films GROUP BY release_year HAVING GROUPBY() IN (2000, 2002);"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "release_year": {
          "type": "long"
        }
      },
      {
        "count": {
          "type": "long long"
        }
      }
    ],
    "data": [
      {
        "release_year": 2002,
        "count": 108
      },
      {
        "release_year": 2000,
        "count": 97
      }
    ],
    "total": 2,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->
<!-- example mva -->
##### Группировка по MVA (многоценностным атрибутам)
Manticore поддерживает группировку по [MVA](../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29). Чтобы продемонстрировать, как это работает, создадим таблицу "shoes" с MVA "sizes" и вставим несколько документов:
```sql
create table shoes(title text, sizes multi);
insert into shoes values(0,'nike',(40,41,42)),(0,'adidas',(41,43)),(0,'reebook',(42,43));
```
так что у нас есть:
```sql
SELECT * FROM shoes;
+---------------------+----------+---------+
| id                  | sizes    | title   |
+---------------------+----------+---------+
| 1657851069130080265 | 40,41,42 | nike    |
| 1657851069130080266 | 41,43    | adidas  |
| 1657851069130080267 | 42,43    | reebook |
+---------------------+----------+---------+
```
Если теперь выполнить GROUP BY по "sizes", будут обработаны все наши многоценностные атрибуты и возвращена агрегация для каждого, в данном случае просто подсчёт:

<!-- intro -->
##### Пример:

<!-- request SQL -->
```sql
SELECT groupby() gb, count(*) FROM shoes GROUP BY sizes ORDER BY gb asc;
```
<!-- response SQL -->
```sql
+------+----------+
| gb   | count(*) |
+------+----------+
|   40 |        1 |
|   41 |        2 |
|   42 |        2 |
|   43 |        2 |
+------+----------+
```

<!-- request JSON -->
``` json
POST /search -d '
    {
     "table" : "shoes",
     "limit": 0,
     "aggs" :
     {
        "sizes" :
         {
            "terms" :
             {
              "field":"sizes",
              "size":100
             }
         }
     }
    }
'
```
<!-- response JSON -->
``` json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 3,
    "hits": [

    ]
  },
  "aggregations": {
    "sizes": {
      "buckets": [
        {
          "key": 43,
          "doc_count": 2
        },
        {
          "key": 42,
          "doc_count": 2
        },
        {
          "key": 41,
          "doc_count": 2
        },
        {
          "key": 40,
          "doc_count": 1
        }
      ]
    }
  }
}
```
<!-- request PHP -->
``` php
$index->setName('shoes');
$search = $index->search('');
$search->limit(0);
$search->facet('sizes','sizes',100);
$results = $search->get();
print_r($results->getFacets());
```
<!-- response PHP -->
``` php
Array
(
    [sizes] => Array
        (
            [buckets] => Array
                (
                    [0] => Array
                        (
                            [key] => 43
                            [doc_count] => 2
                        )
                    [1] => Array
                        (
                            [key] => 42
                            [doc_count] => 2
                        )
                    [2] => Array
                        (
                            [key] => 41
                            [doc_count] => 2
                        )
                    [3] => Array
                        (
                            [key] => 40
                            [doc_count] => 1
                        )
                )
        )
)
```
<!-- request Python -->
``` python
res =searchApi.search({"table":"shoes","limit":0,"aggs":{"sizes":{"terms":{"field":"sizes","size":100}}}})
```
<!-- response Python -->
``` python
{'aggregations': {u'sizes': {u'buckets': [{u'doc_count': 2, u'key': 43},
                                          {u'doc_count': 2, u'key': 42},
                                          {u'doc_count': 2, u'key': 41},
                                          {u'doc_count': 1, u'key': 40}]}},
 'hits': {'hits': [], 'max_score': None, 'total': 3},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- request Javascript -->
``` javascript
res = await searchApi.search({"table":"shoes","limit":0,"aggs":{"sizes":{"terms":{"field":"sizes","size":100}}}});
```

<!-- request Python-asyncio -->
``` python
res = await searchApi.search({"table":"shoes","limit":0,"aggs":{"sizes":{"terms":{"field":"sizes","size":100}}}})
```
<!-- response Python-asyncio -->
``` python
{'aggregations': {u'sizes': {u'buckets': [{u'doc_count': 2, u'key': 43},
                                          {u'doc_count': 2, u'key': 42},
                                          {u'doc_count': 2, u'key': 41},
                                          {u'doc_count': 1, u'key': 40}]}},
 'hits': {'hits': [], 'max_score': None, 'total': 3},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Javascript -->
``` javascript
res = await searchApi.search({"table":"shoes","limit":0,"aggs":{"sizes":{"terms":{"field":"sizes","size":100}}}});
```

<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"aggregations":{"sizes":{"buckets":[{"key":43,"doc_count":2},{"key":42,"doc_count":2},{"key":41,"doc_count":2},{"key":40,"doc_count":1}]}},"hits":{"total":3,"hits":[]}}
```
<!-- request Java -->
``` java
HashMap<String,Object> aggs = new HashMap<String,Object>(){{
    put("release_year", new HashMap<String,Object>(){{
        put("terms", new HashMap<String,Object>(){{
            put("field","release_year");
            put("size",100);
        }});
    }});
}};

searchRequest = new SearchRequest();
searchRequest.setIndex("films");
searchRequest.setLimit(0);
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
searchRequest.setAggs(aggs);
searchResponse = searchApi.search(searchRequest);
```
<!-- response Java -->
``` java
class SearchResponse {
    took: 0
    timedOut: false
    aggregations: {release_year={buckets=[{key=43, doc_count=2}, {key=42, doc_count=2}, {key=41, doc_count=2}, {key=40, doc_count=1}]}}
    hits: class SearchResponseHits {
        maxScore: null
        total: 3
        hits: []
    }
    profile: null
}

```

<!-- request C# -->
``` clike
var agg = new Aggregation("release_year", "release_year");
agg.Size = 100;
object query = new { match_all=null };
var searchRequest = new SearchRequest("films", query);
searchRequest.Limit = 0;
searchRequest.Aggs = new List<Aggregation> {agg};
var searchResponse = searchApi.Search(searchRequest);
```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 0
    timedOut: false
    aggregations: {release_year={buckets=[{key=43, doc_count=2}, {key=42, doc_count=2}, {key=41, doc_count=2}, {key=40, doc_count=1}]}}
    hits: class SearchResponseHits {
        maxScore: null
        total: 3
        hits: []
    }
    profile: null
}

```

<!-- request Rust -->
``` rust
let query = SearchQuery::new();
let aggTerms1 = AggTerms::new {
    fields: "release_year".to_string(),
    size: Some(100),
};
let agg1 = Aggregation {
    terms: Some(Box::new(aggTerms1)),
    ..Default::default(),
};
let mut aggs = HashMap::new();
aggs.insert("release_year".to_string(), agg1);

let search_req = SearchRequest {
    table: "films".to_string(),
    query: Some(Box::new(query)),
    aggs: serde_json::json!(aggs),
    limit: serde_json::json!(0),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```
<!-- response Rust -->
``` rust
class SearchResponse {
    took: 0
    timedOut: false
    aggregations: {release_year={buckets=[{key=43, doc_count=2}, {key=42, doc_count=2}, {key=41, doc_count=2}, {key=40, doc_count=1}]}}
    hits: class SearchResponseHits {
        maxScore: null
        total: 3
        hits: []
    }
    profile: null
}

<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  aggs: {
    mva_agg: {
      terms: { field: "mva_field", size: 2 }
    }
  }
});
```

<!-- response TypeScript -->
``` typescript
{
	"took":0,
	"timed_out":false,
	"aggregations":
	{
		"mva_agg":
		{
			"buckets":
			[{
				"key":1,
				"doc_count":4
			},
			{
				"key":2,
				"doc_count":2
			}]
		}
	},
	"hits":
	{
		"total":4,
		"hits":[]
	}
}
```

<!-- request Go -->
``` go
query := map[string]interface{} {};
searchRequest.SetQuery(query);
aggTerms := manticoreclient.NewAggregationTerms()
aggTerms.SetField("mva_field")
aggTerms.SetSize(2)
aggregation := manticoreclient.NewAggregation()
aggregation.setTerms(aggTerms)
searchRequest.SetAggregation(aggregation)
res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```

<!-- response Go -->
``` go
{
	"took":0,
	"timed_out":false,
	"aggregations":
	{
		"mva_agg":
		{
			"buckets":
			[{
				"key":1,
				"doc_count":4
			},
			{
				"key":2,
				"doc_count":2
			}]
		}
	},
	"hits":
	{
		"total":5,
		"hits":[]
	}
}
```

<!-- end -->

<!-- example json -->
##### Grouping by a JSON node
If you have a field of type [JSON](../Creating_a_table/Data_types.md#JSON), you can GROUP BY any node from it. To demonstrate this, let's create a table "products" with a few documents, each having a color in the "meta" JSON field:
```sql
create table products(title text, meta json);
insert into products values(0,'nike','{"color":"red"}'),(0,'adidas','{"color":"red"}'),(0,'puma','{"color":"green"}');
```
This gives us:
```sql
SELECT * FROM products;
+---------------------+-------------------+--------+
| id                  | meta              | title  |
+---------------------+-------------------+--------+
| 1657851069130080268 | {"color":"red"}   | nike   |
| 1657851069130080269 | {"color":"red"}   | adidas |
| 1657851069130080270 | {"color":"green"} | puma   |
+---------------------+-------------------+--------+
```
To group the products by color, we can simply use `GROUP BY meta.color`, and to display the corresponding group key in the `SELECT` list, we can use `GROUPBY()`:

<!-- intro -->
##### Example:

<!-- request SQL -->
```sql
SELECT groupby() color, count(*) from products GROUP BY meta.color;
```
<!-- response SQL -->
```sql
+-------+----------+
| color | count(*) |
+-------+----------+
| red   |        2 |
| green |        1 |
+-------+----------+
```
<!-- request JSON -->
``` json
POST /search -d '
    {
     "table" : "products",
     "limit": 0,
     "aggs" :
     {
        "color" :
         {
            "terms" :
             {
              "field":"meta.color",
              "size":100
             }
         }
     }
    }
'
```
<!-- response JSON -->
``` json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 3,
    "hits": [

    ]
  },
  "aggregations": {
    "color": {
      "buckets": [
        {
          "key": "green",
          "doc_count": 1
        },
        {
          "key": "red",
          "doc_count": 2
        }
      ]
    }
  }
}
```
<!-- request PHP -->
``` php
$index->setName('products');
$search = $index->search('');
$search->limit(0);
$search->facet('meta.color','color',100);
$results = $search->get();
print_r($results->getFacets());
```
<!-- response PHP -->
``` php
Array
(
    [color] => Array
        (
            [buckets] => Array
                (
                    [0] => Array
                        (
                            [key] => green
                            [doc_count] => 1
                        )
                    [1] => Array
                        (
                            [key] => red
                            [doc_count] => 2
                        )
                )
        )
)

```
<!-- request Python -->
``` python
res =searchApi.search({"table":"products","limit":0,"aggs":{"color":{"terms":{"field":"meta.color","size":100}}}})
```
<!-- response Python -->
``` python
{'aggregations': {u'color': {u'buckets': [{u'doc_count': 1,
                                           u'key': u'green'},
                                          {u'doc_count': 2, u'key': u'red'}]}},
 'hits': {'hits': [], 'max_score': None, 'total': 3},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

```
<!-- request Python-asyncio -->
``` python
res = await searchApi.search({"table":"products","limit":0,"aggs":{"color":{"terms":{"field":"meta.color","size":100}}}})
```
<!-- response Python-asyncio -->
``` python
{'aggregations': {u'color': {u'buckets': [{u'doc_count': 1,
                                           u'key': u'green'},
                                          {u'doc_count': 2, u'key': u'red'}]}},
 'hits': {'hits': [], 'max_score': None, 'total': 3},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Javascript -->
``` javascript
res = await searchApi.search({"table":"products","limit":0,"aggs":{"color":{"terms":{"field":"meta.color","size":100}}}});
```
<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"aggregations":{"color":{"buckets":[{"key":"green","doc_count":1},{"key":"red","doc_count":2}]}},"hits":{"total":3,"hits":[]}}
```
<!-- request Java -->
``` java
HashMap<String,Object> aggs = new HashMap<String,Object>(){{
    put("color", new HashMap<String,Object>(){{
        put("terms", new HashMap<String,Object>(){{
            put("field","meta.color");
            put("size",100);
        }});
    }});
}};

searchRequest = new SearchRequest();
searchRequest.setIndex("products");
searchRequest.setLimit(0);
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
searchRequest.setAggs(aggs);
searchResponse = searchApi.search(searchRequest);
```
<!-- response Java -->
``` java
class SearchResponse {
    took: 0
    timedOut: false
    aggregations: {color={buckets=[{key=green, doc_count=1}, {key=red, doc_count=2}]}}
    hits: class SearchResponseHits {
        maxScore: null
        total: 3
        hits: []
    }
    profile: null
}

```

<!-- request C# -->
``` clike
var agg = new Aggregation("color", "meta.color");
agg.Size = 100;
object query = new { match_all=null };
var searchRequest = new SearchRequest("products", query);
searchRequest.Limit = 0;
searchRequest.Aggs = new List<Aggregation> {agg};
var searchResponse = searchApi.Search(searchRequest);
```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 0
    timedOut: false
    aggregations: {color={buckets=[{key=green, doc_count=1}, {key=red, doc_count=2}]}}
    hits: class SearchResponseHits {
        maxScore: null
        total: 3
        hits: []
    }
    profile: null
}

```

<!-- request Rust -->
``` rust
let query = SearchQuery::new();
let aggTerms1 = AggTerms::new {
    fields: "meta.color".to_string(),
    size: Some(100),
};
let agg1 = Aggregation {
    terms: Some(Box::new(aggTerms1)),
    ..Default::default(),
};
let mut aggs = HashMap::new();
aggs.insert("color".to_string(), agg1);

let search_req = SearchRequest {
    table: "products".to_string(),
    query: Some(Box::new(query)),
    aggs: serde_json::json!(aggs),
    limit: serde_json::json!(0),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```
<!-- response Rust -->
``` rust
class SearchResponse {
    took: 0
    timedOut: false
    aggregations: {color={buckets=[{key=green, doc_count=1}, {key=red, doc_count=2}]}}
    hits: class SearchResponseHits {
        maxScore: null
        total: 3
        hits: []
    }
    profile: null
}

```

<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  aggs: {
    json_agg: {
      terms: { field: "json_field.year", size: 1 }
    }
  }
});
```

<!-- response TypeScript -->
``` typescript
{
	"took":0,
	"timed_out":false,
	"aggregations":
	{
		"json_agg":
		{
			"buckets":
			[{
				"key":2000,
				"doc_count":2
			},
			{
				"key":2001,
				"doc_count":2
			}]
		}
	},
	"hits":
	{
		"total":4,
		"hits":[]
	}
}
```

<!-- request Go -->
``` go
query := map[string]interface{} {};
searchRequest.SetQuery(query);
aggTerms := manticoreclient.NewAggregationTerms()
aggTerms.SetField("json_field.year")
aggTerms.SetSize(2)
aggregation := manticoreclient.NewAggregation()
aggregation.setTerms(aggTerms)
searchRequest.SetAggregation(aggregation)
res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```

<!-- response Go -->
``` go
{
	"took":0,
	"timed_out":false,
	"aggregations":
	{
		"json_agg":
		{
			"buckets":
			[{
				"key":2000,
				"doc_count":2
			},
			{
				"key":2001,
				"doc_count":2
			}]
		}
	},
	"hits":
	{
		"total":4,
		"hits":[]
	}
}
```

<!-- end -->

## Функции агрегации
Помимо `COUNT(*)`, возвращающего количество элементов в каждой группе, вы можете использовать различные другие функции агрегации:
<!-- example distinct -->
##### COUNT(DISTINCT field)
В то время как `COUNT(*)` возвращает количество всех элементов в группе, `COUNT(DISTINCT field)` возвращает количество уникальных значений поля в группе, что может значительно отличаться от общего количества. Например, в группе может быть 100 элементов, но у всех одно и то же значение заданного поля. `COUNT(DISTINCT field)` помогает это определить. Чтобы продемонстрировать это, создадим таблицу "students" с именем студента, возрастом и направлением обучения:
```sql
CREATE TABLE students(name text, age int, major string);
INSERT INTO students values(0,'John',21,'arts'),(0,'William',22,'business'),(0,'Richard',21,'cs'),(0,'Rebecca',22,'cs'),(0,'Monica',21,'arts');
```

так что у нас есть:

```sql
MySQL [(none)]> SELECT * from students;
+---------------------+------+----------+---------+
| id                  | age  | major    | name    |
+---------------------+------+----------+---------+
| 1657851069130080271 |   21 | arts     | John    |
| 1657851069130080272 |   22 | business | William |
| 1657851069130080273 |   21 | cs       | Richard |
| 1657851069130080274 |   22 | cs       | Rebecca |
| 1657851069130080275 |   21 | arts     | Monica  |
+---------------------+------+----------+---------+
```

В примере видно, что если мы группируем по направлению (major) и выводим одновременно `COUNT(*)` и `COUNT(DISTINCT age)`, становится ясно, что есть два студента с направлением "cs" с двумя уникальными возрастами, а для направления "arts" также два студента, но только один уникальный возраст.

В одном запросе может быть не более одного `COUNT(DISTINCT)`.

** По умолчанию подсчёты приблизительные **

На самом деле, некоторые из них точные, а другие — приблизительные. Подробнее об этом ниже.

Manticore поддерживает два алгоритма для подсчёта количества уникальных значений. Один из них — устаревший алгоритм, который использует много памяти и обычно работает медленно. Он собирает пары `{group; value}`, сортирует их и периодически удаляет дубликаты. Преимущество этого подхода — гарантия точных подсчётов в простой таблице. Его можно включить, установив опцию [distinct_precision_threshold](../Searching/Options.md#distinct_precision_threshold) в значение `0`.

Другой алгоритм (включён по умолчанию) загружает подсчёты в хэш-таблицу и возвращает её размер. Если хэш-таблица становится слишком большой, её содержимое переносится в `HyperLogLog`. Здесь подсчёты становятся приблизительными, так как `HyperLogLog` — вероятностный алгоритм. Преимущество в том, что максимум используемой памяти на группу фиксирован и зависит от точности `HyperLogLog`. Общий расход памяти также зависит от настройки [max_matches](../Searching/Options.md#max_matches), отражающей число групп.

Опция [distinct_precision_threshold](../Searching/Options.md#distinct_precision_threshold) задаёт порог, ниже которого подсчёты гарантированно точны. Настройка точности `HyperLogLog` и порог для конверсии "хэш-таблица в HyperLogLog" выводятся из этого значения. Важно использовать эту опцию с осторожностью, так как её удвоение удвоит максимальное потребление памяти для подсчёта. Максимальное потребление памяти можно приблизительно оценить формулой: `64 * max_matches * distinct_precision_threshold`. Обратите внимание, что это худший случай, в большинстве случаев подсчёт будет использовать значительно меньше ОЗУ.

**`COUNT(DISTINCT)` для распределённой таблицы или realtime таблицы, состоящей из нескольких дисковых чанков, может вернуть неточные результаты**, но результат должен быть точным для распределённой таблицы, состоящей из локальных plain или realtime таблиц с одинаковой схемой (идентичный набор/порядок полей, но могут быть разные настройки токенизации).

<!-- intro -->
##### Пример:

<!-- request SQL -->
```sql
SELECT major, count(*), count(distinct age) FROM students GROUP BY major;
```
<!-- response SQL -->
```sql
+----------+----------+---------------------+
| major    | count(*) | count(distinct age) |
+----------+----------+---------------------+
| arts     |        2 |                   1 |
| business |        1 |                   1 |
| cs       |        2 |                   2 |
+----------+----------+---------------------+
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw - d "SELECT major, count(*), count(distinct age) FROM students GROUP BY major;"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "major": {
          "type": "string"
        }
      },
      {
        "count(*)": {
          "type": "long long"
        }
      },
      {
        "count(distinct age)": {
          "type": "long long"
        }
      }
    ],
    "data": [
      {
        "major": "arts",
        "count(*)": 2,
        "count(distinct age)": 1
      },
      {
        "major": "business",
        "count(*)": 1,
        "count(distinct age)": 1
      },
      {
        "major": "cs",
        "count(*)": 2,
        "count(distinct age)": 2
      }
    ],
    "total": 3,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

<!-- example concat -->
##### GROUP_CONCAT(field)

Часто хочется лучше понять содержимое каждой группы. Для этого можно использовать [GROUP N BY](../Searching/Grouping.md#Give-me-N-rows), но он вернёт дополнительные строки, которых вы, возможно, не хотите в выводе. `GROUP_CONCAT()` расширяет группировку, объединяя значения определённого поля в группе. Возьмём предыдущий пример и улучшим его, отобразив все возраста в каждой группе.

`GROUP_CONCAT(field)` возвращает список значений через запятую.

<!-- intro -->
##### Пример:

<!-- request SQL -->
```sql
SELECT major, count(*), count(distinct age), group_concat(age) FROM students GROUP BY major
```
<!-- response SQL -->
```sql
+----------+----------+---------------------+-------------------+
| major    | count(*) | count(distinct age) | group_concat(age) |
+----------+----------+---------------------+-------------------+
| arts     |        2 |                   1 | 21,21             |
| business |        1 |                   1 | 22                |
| cs       |        2 |                   2 | 21,22             |
+----------+----------+---------------------+-------------------+
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SELECT major, count(*), count(distinct age), group_concat(age) FROM students GROUP BY major"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "major": {
          "type": "string"
        }
      },
      {
        "count(*)": {
          "type": "long long"
        }
      },
      {
        "count(distinct age)": {
          "type": "long long"
        }
      },
      {
        "group_concat(age)": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "major": "arts",
        "count(*)": 2,
        "count(distinct age)": 1,
        "group_concat(age)": 21,21
      },
      {
        "major": "business",
        "count(*)": 1,
        "count(distinct age)": 1,
        "group_concat(age)": 22
      },
      {
        "major": "cs",
        "count(*)": 2,
        "count(distinct age)": 2,
        "group_concat(age)": 21,22
      }
    ],
    "total": 3,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->
<!-- example sum -->
##### SUM(), MIN(), MAX(), AVG()
Конечно, вы также можете получить сумму, среднее, минимум и максимум внутри группы.

<!-- intro -->
##### Пример:

<!-- request SQL -->
```sql
SELECT release_year year, sum(rental_rate) sum, min(rental_rate) min, max(rental_rate) max, avg(rental_rate) avg FROM films GROUP BY release_year ORDER BY year asc LIMIT 5;
```
<!-- response SQL -->
```sql
+------+------------+----------+----------+------------+
| year | sum        | min      | max      | avg        |
+------+------------+----------+----------+------------+
| 2000 | 308.030029 | 0.990000 | 4.990000 | 3.17556739 |
| 2001 | 282.090118 | 0.990000 | 4.990000 | 3.09989142 |
| 2002 | 332.919983 | 0.990000 | 4.990000 | 3.08259249 |
| 2003 | 310.940063 | 0.990000 | 4.990000 | 2.93339682 |
| 2004 | 300.920044 | 0.990000 | 4.990000 | 2.78629661 |
+------+------------+----------+----------+------------+
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SELECT release_year year, sum(rental_rate) sum, min(rental_rate) min, max(rental_rate) max, avg(rental_rate) avg FROM films GROUP BY release_year ORDER BY year asc LIMIT 5;"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "year": {
          "type": "long"
        }
      },
      {
        "sum": {
          "type": "long long"
        }
      },
      {
        "min": {
          "type": "long long"
        }
      },
      {
        "max": {
          "type": "long long"
        }
      },
      {
        "avg": {
          "type": "long long"
        }
      }
    ],
    "data": [
      {
        "year": 2000,
        "sum": 308.030029,
        "min": 0.990000,
        "max": 4.990000,
        "avg": 3.17556739
      },
      {
        "year": 2001,
        "sum": 282.090118,
        "min": 0.990000,
        "max": 4.990000,
        "avg": 3.09989142
      },
      {
        "year": 2002,
        "sum": 332.919983,
        "min": 0.99,
        "max": 4.990000,
        "avg": 3.08259249
      },
      {
        "year": 2003,
        "sum": 310.940063,
        "min": 0.990000,
        "max": 4.990000,
        "avg": 2.93339682
      },
      {
        "year": 2004,
        "sum": 300.920044,
        "min": 0.990000,
        "max": 4.990000,
        "avg": 2.78629661
      }
    ],
    "total": 5,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

<!-- example accuracy -->
## Точность группировки

Группировка выполняется в фиксированной памяти, зависящей от настройки [max_matches](../Searching/Options.md#max_matches). Если `max_matches` позволяет хранить все найденные группы, результаты будут 100% точными. Однако, если значение `max_matches` меньше, результаты будут менее точными.

Когда задействована параллельная обработка, ситуация может стать более сложной. Когда включен `pseudo_sharding` и/или используется RT таблица с несколькими дисковыми чанками, каждый чанк или псевдо-шейрд получает результат не больше чем `max_matches`. Это может привести к неточностям в агрегатах и подсчетах групп, когда результаты из разных потоков объединяются. Чтобы исправить это, можно использовать большее значение `max_matches` или отключить параллельную обработку.

Manticore попытается увеличить `max_matches` до [max_matches_increase_threshold](../Searching/Options.md#max_matches_increase_threshold), если обнаружит, что groupby может вернуть неточные результаты. Обнаружение основывается на количестве уникальных значений атрибута groupby, которое извлекается из вторичных индексов (если они есть).

Чтобы обеспечить точные агрегаты и/или подсчеты групп при использовании RT таблиц или `pseudo_sharding`, можно включить `accurate_aggregation`. Это попытается увеличить `max_matches` до порогового значения, а если порог недостаточно высокий, Manticore отключит параллельную обработку для запроса.

<!-- intro -->
##### Пример:

<!-- request SQL -->
```sql
MySQL [(none)]> SELECT release_year year, count(*) FROM films GROUP BY year limit 5;
+------+----------+
| year | count(*) |
+------+----------+
| 2004 |      108 |
| 2002 |      108 |
| 2001 |       91 |
| 2005 |       93 |
| 2000 |       97 |
+------+----------+

MySQL [(none)]> SELECT release_year year, count(*) FROM films GROUP BY year limit 5 option max_matches=1;
+------+----------+
| year | count(*) |
+------+----------+
| 2004 |       76 |
+------+----------+

MySQL [(none)]> SELECT release_year year, count(*) FROM films GROUP BY year limit 5 option max_matches=2;
+------+----------+
| year | count(*) |
+------+----------+
| 2004 |       76 |
| 2002 |       74 |
+------+----------+

MySQL [(none)]> SELECT release_year year, count(*) FROM films GROUP BY year limit 5 option max_matches=3;
+------+----------+
| year | count(*) |
+------+----------+
| 2004 |      108 |
| 2002 |      108 |
| 2001 |       91 |
+------+----------+
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SELECT release_year year, count(*) FROM films GROUP BY year limit 5;"
[
  {
    "columns": [
      {
        "year": {
          "type": "long"
        }
      },
      {
        "count(*)": {
          "type": "long long"
        }
      }
    ],
    "data": [
      {
        "year": 2004,
        "count(*)": 108
      },
      {
        "year": 2002,
        "count(*)": 108
      },
      {
        "year": 2001,
        "count(*)": 91
      },
      {
        "year": 2005,
        "count(*)": 93
      },
      {
        "year": 2000,
        "count(*)": 97
      },
    ],
    "total": 5,
    "error": "",
    "warning": ""
  }
]
POST /sql?mode=raw -d "SELECT release_year year, count(*) FROM films GROUP BY year limit 5 option max_matches=1;"
[
  {
    "columns": [
      {
        "year": {
          "type": "long"
        }
      },
      {
        "count(*)": {
          "type": "long long"
        }
      }
    ],
    "data": [
      {
        "year": 2004,
        "count(*)": 76
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
POST /sql?mode=raw -d "SELECT release_year year, count(*) FROM films GROUP BY year limit 5 option max_matches=2;"
[
  {
    "columns": [
      {
        "year": {
          "type": "long"
        }
      },
      {
        "count(*)": {
          "type": "long long"
        }
      }
    ],
    "data": [
      {
        "year": 2004,
        "count(*)": 76
      },
      {
        "year": 2002,
        "count(*)": 74
      }
    ],
    "total": 2,
    "error": "",
    "warning": ""
  }
]
POST /sql?mode=raw -d "SELECT release_year year, count(*) FROM films GROUP BY year limit 5 option max_matches=3;"
[
  {
    "columns": [
      {
        "year": {
          "type": "long"
        }
      },
      {
        "count(*)": {
          "type": "long long"
        }
      }
    ],
    "data": [
      {
        "year": 2004,
        "count(*)": 108
      },
      {
        "year": 2002,
        "count(*)": 108
      },
      {
        "year": 2001,
        "count(*)": 91
      }
    ],
    "total": 3,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->
<!-- proofread -->

