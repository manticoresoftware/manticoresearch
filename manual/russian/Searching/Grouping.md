# Группировка результатов поиска

<!-- example general -->
Группировка результатов поиска часто полезна для получения количества совпадений по группам или других агрегатов. Например, это полезно для создания графика, иллюстрирующего количество совпадающих блога постов в месяц или группировки результатов веб-поиска по сайтам или постам на форумах по авторам и т. д.

Manticore поддерживает группировку результатов поиска по одному или нескольким столбцам и вычисляемым выражениям. Результаты могут:

* Быть отсортированными внутри группы
* Возвращать более одной строки на группу
* Иметь отфильтрованные группы
* Иметь отсортированные группы
* Быть агрегированными с использованием [агрегационных функций](../Searching/Grouping.md#Aggregation-functions)

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
where_condition: {агрегационное выражение alias | COUNT(*)}
```

<!-- request JSON -->
Формат JSON-запроса в настоящее время поддерживает базовую группировку, которая может извлекать агрегатные значения и их count(*).

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
Агрегация требует установки `size` для размера набора результатов группы.

<!-- end -->

<!-- example group1 -->
### Просто группировка
Группировка довольно проста - просто добавьте "GROUP BY smth" в конец вашего `SELECT` запроса. чем-то может быть:

* Любое поле, не содержащее текст из таблицы: целое число, число с плавающей запятой, строка, MVA (многозначный атрибут)
* Или, если вы использовали псевдоним в списке `SELECT`, вы также можете группировать по нему

Вы можете опустить любые [агрегационные функции](../Searching/Grouping.md#Aggregation-functions) в списке `SELECT`, и это всё равно будет работать:

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
<!-- end -->
<!-- example group2 -->
В большинстве случаев, однако, вы захотите получить некоторые агрегированные данные для каждой группы, такие как:

* `COUNT(*)`, чтобы просто получить количество элементов в каждой группе
* или `AVG(field)`, чтобы вычислить среднее значение поля внутри группы

Для HTTP JSON-запросов использование одного `aggs` ведра с `limit=0` на главном уровне запроса работает аналогично SQL-запросу с `GROUP BY` и `COUNT(*)`, предоставляя эквивалентное поведение и производительность.

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
По умолчанию группы не сортируются, и обычно вы хотите упорядочить их по чему-то, например, по полю, по которому вы группируете:

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
<!-- end -->
<!-- example sort2 -->
В качестве альтернативы вы можете сортировать по агрегации:

* по `count(*)`, чтобы сначала отобразить группы с наибольшим количеством элементов
* по `avg(rental_rate)`, чтобы сначала показать самые высоко оцененные фильмы. Обратите внимание, что в примере это сделано через псевдоним: `avg(rental_rate)` сначала сопоставляется с `avg` в списке `SELECT`, а затем мы просто делаем `ORDER BY avg`


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
<!-- end -->

<!-- example group3 -->
##### ГРУППИРОВКА по нескольким полям одновременно
В некоторых случаях вы можете захотеть группировать не только по одному полю, но и по нескольким полям одновременно, например, по категории фильма и году:

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
Иногда полезно видеть не только один элемент на группу, но и несколько. Это можно легко достичь с помощью `GROUP N BY`. Например, в следующем случае мы получаем два фильма для каждого года, а не только один, как это было бы с простым `GROUP BY release_year`.

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
<!-- end -->

<!-- example group5 -->
##### Сортировка внутри группы
Еще одним важным требованием аналитики является сортировка элементов внутри группы. Для этого используйте предложение `WITHIN GROUP ORDER BY ... {ASC|DESC}`. Например, давайте получим фильм с самым высоким рейтингом для каждого года. Обратите внимание, что это работает параллельно с просто `ORDER BY`:

* `WITHIN GROUP ORDER BY` сортирует результаты **внутри группы**
* в то время как просто `GROUP BY` **сортирует сами группы**

Эти два работают совершенно независимо.


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
<!-- end -->

<!-- example group6 -->
##### Фильтрация групп
`HAVING expression` — это полезное предложение для фильтрации групп. В то время как `WHERE` применяется до группировки, `HAVING` работает с группами. Например, давайте оставим только те года, когда средняя ставка аренды фильмов за тот год была выше 3. Мы получаем только четыре года:

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
<!-- end -->

Обратите внимание, что `HAVING` не влияет на `total_found` в [метаинформации поискового запроса](../Node_info_and_management/SHOW_META.md#SHOW-META).

<!-- example group7 -->
##### GROUPBY()
Существует функция `GROUPBY()`, которая возвращает ключ текущей группы. Она полезна во многих случаях, особенно когда вы [GROUP BY MVA](../Searching/Grouping.md#Grouping-by-MVA-%28multi-value-attributes%29) или [JSON значение](../Searching/Grouping.md#Grouping-by-a-JSON-node).

Ее также можно использовать в `HAVING`, например, чтобы оставить только года 2000 и 2002.

Обратите внимание, что использование `GROUPBY()` не рекомендуется, когда вы GROUP BY несколько полей одновременно. Он все еще будет работать, но поскольку ключ группы в этом случае является составным из значений полей, он может не отображаться так, как вы ожидаете.

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
<!-- end -->
<!-- example mva -->
##### Группировка по MVA (атрибуты с несколькими значениями)
Manticore поддерживает группировку по [MVA](../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29). Чтобы продемонстрировать, как это работает, давайте создадим таблицу "shoes" с MVA "sizes" и вставим в нее несколько документов:
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
Если мы теперь сделаем GROUP BY "sizes", он обработает все наши атрибуты с несколькими значениями и вернет агрегацию для каждого, в данном случае просто количество:

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
##### Группировка по узлу JSON

Если у вас есть поле типа [JSON](../Creating_a_table/Data_types.md#JSON), вы можете ГРУППИРОВАТЬ по любому узлу из него. Для демонстрации этого давайте создадим таблицу "products" с несколькими документами, каждый из которых имеет цвет в поле "meta" JSON:

```sql

create table products(title text, meta json);

insert into products values(0,'nike','{"color":"red"}'),(0,'adidas','{"color":"red"}'),(0,'puma','{"color":"green"}');

```

Это даст нам:

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

Чтобы сгруппировать продукты по цвету, мы можем просто использовать `GROUP BY meta.color`, а чтобы отобразить соответствующий ключ группы в списке `SELECT`, мы можем использовать `GROUPBY()`:

<!-- intro -->

##### Пример:

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
