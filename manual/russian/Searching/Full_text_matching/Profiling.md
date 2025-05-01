# Профилирование поиска

## Как интерпретируется запрос

Рассмотрим следующий пример сложного запроса:

```sql

"hello world" @title "example program"~5 @body python -(php|perl) @* code

```

Полное значение этого поиска:

* Найти слова «hello» и «world», стоящие рядом, в любом поле документа;

* Кроме того, тот же документ должен содержать слова «example» и «program» в поле заголовка, при этом между ними может быть до 5 слов, но не включая 5 слов; (например, «example PHP program» сработает, а «example script to introduce outside data into the correct context for your program» — нет, так как между терминами 5 или более слов)

* Далее, тот же документ должен содержать слово «python» в поле body, при этом исключая «php» или «perl»;

* Наконец, тот же документ должен включать слово «code» в любом поле.

Оператор OR имеет больший приоритет, чем AND, поэтому выражение «looking for cat | dog | mouse» означает «looking for (cat | dog | mouse)», а не «(looking for cat) | dog | mouse».

Чтобы понять, как будет выполняться запрос, Manticore Search предоставляет инструменты профилирования запросов для анализа дерева запроса, сгенерированного выражением запроса.

<!-- example profiling -->

## Профилирование дерева запроса в SQL

Чтобы включить профилирование полнотекстового запроса через SQL, его необходимо активировать перед выполнением нужного запроса:

```sql

SET profiling =1;

SELECT * FROM test WHERE MATCH('@title abc* @body hey');

```

Чтобы просмотреть дерево запроса, выполните команду `SHOW PLAN` сразу после запуска запроса:

```sql

SHOW PLAN;

```

Эта команда вернёт структуру выполненного запроса. Имейте в виду, что три оператора — SET profiling, сам запрос и SHOW — должны выполняться в рамках одной и той же сессии.


## Профилирование запроса в HTTP JSON

При использовании протокола HTTP JSON достаточно включить `"profile":true`, чтобы в ответе получить структуру дерева полнотекстового запроса.

```json

{

  "table":"test",

  "profile":true,

  "query":

  {

    "match_phrase": { "_all" : "had grown quite" }

  }

}

```

Ответ будет включать объект `profile`, содержащий член `query`.

Свойство `query` содержит преобразованное дерево полнотекстового запроса. Каждый узел состоит из:

* `type`: тип узла, который может быть AND, OR, PHRASE, KEYWORD и т.д.

* `description`: поддерево запроса для этого узла, представленное в виде строки (в формате `SHOW PLAN`)

* `children`: любые дочерние узлы, если они присутствуют

* `max_field_pos`: максимальная позиция в поле

 Узел ключевого слова дополнительно будет включать:

* `word`: преобразованное ключевое слово.

* `querypos`: позиция этого ключевого слова в запросе.

* `excluded`: ключевое слово, исключённое из запроса.

* `expanded`: ключевое слово, добавленное посредством расширения по префиксу.

* `field_start`: ключевое слово должно появляться в начале поля.

* `field_end`: ключевое слово должно появляться в конце поля.

* `boost`: IDF ключевого слова будет умножен на это значение.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql

SET profiling=1;

SELECT * FROM test WHERE MATCH('@title abc* @body hey');

SHOW PLAN G

```
<!-- response SQL -->

```sql

*************************** 1. row ***************************

Variable: transformed_tree

   Value: AND(

  OR(fields=(title), KEYWORD(abcx, querypos=1, expanded), KEYWORD(abcm, querypos=1, expanded)),

  AND(fields=(body), KEYWORD(hey, querypos=2)))

1 row in set (0.00 sec)

```

<!-- request JSON -->

```JSON

POST /search

{

  "table": "forum",

  "query": {"query_string": "i me"},

  "_source": { "excludes":["*"] },

  "limit": 1,

  "profile":true

}

```

<!-- response JSON -->
```JSON

{

  "took":1503,

  "timed_out":false,

  "hits":

  {

    "total":406301,

    "hits":

    [

       {

          "_id": 406443,

          "_score":3493,

          "_source":{}

       }

    ]

  },

  "profile":

  {

    "query":

    {

      "type":"AND",

      "description":"AND( AND(KEYWORD(i, querypos=1)),  AND(KEYWORD(me, querypos=2)))",

      "children":

      [

        {

          "type":"AND",

          "description":"AND(KEYWORD(i, querypos=1))",

          "children":

          [

            {

              "type":"KEYWORD",

              "word":"i",

              "querypos":1

            }

          ]

        },

        {

          "type":"AND",

          "description":"AND(KEYWORD(me, querypos=2))",

          "children":

          [

            {

              "type":"KEYWORD",

              "word":"me",

              "querypos":2

            }

          ]

        }

      ]

    }

  }

}

```

<!-- request PHP -->

```php

$result = $index->search('i me')->setSource(['excludes'=>['*']])->setLimit(1)->profile()->get();

print_r($result->getProfile());

```
<!-- response PHP -->

``` php

Array

(

    [query] => Array

        (

            [type] => AND

            [description] => AND( AND(KEYWORD(i, querypos=1)),  AND(KEYWORD(me, querypos=2)))

            [children] => Array

                (

                    [0] => Array

                        (

                            [type] => AND

                            [description] => AND(KEYWORD(i, querypos=1))

                            [children] => Array

                                (

                                    [0] => Array

                                        (

                                            [type] => KEYWORD

                                            [word] => i

                                            [querypos] => 1

                                        )

                                )

                        )

                    [1] => Array

                        (

                            [type] => AND
                            [description] => ИЛИ(KEYWORD(me, querypos=2))
                            [children] => Array
                                (
                                    [0] => Array
                                        (
                                            [type] => KEYWORD
                                            [word] => me
                                            [querypos] => 2
                                        )
                                )
                        )
                )
        )
)
```

<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.search({"table":"forum","query":{"query_string":"i me"},"_source":{"excludes":["*"]},"limit":1,"profile":True})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'100', u'_score': 2500, u'_source': {}}],
          'total': 1},
 'profile': {u'query': {u'children': [{u'children': [{u'querypos': 1,
                                                      u'type': u'KEYWORD',
                                                      u'word': u'i'}],
                                       u'description': u'ИЛИ(KEYWORD(i, querypos=1))',
                                       u'type': u'ИЛИ'},
                                      {u'children': [{u'querypos': 2,
                                                      u'type': u'KEYWORD',
                                                      u'word': u'me'}],
                                       u'description': u'ИЛИ(KEYWORD(me, querypos=2))',
                                       u'type': u'ИЛИ'}],
                        u'description': u'ИЛИ( ИЛИ(KEYWORD(i, querypos=1)),  ИЛИ(KEYWORD(me, querypos=2)))',
                        u'type': u'ИЛИ'}},
 'timed_out': False,
 'took': 0}

```

<!-- intro -->
Python-asyncio
<!-- request Python-asyncio -->

```python
await searchApi.search({"table":"forum","query":{"query_string":"i me"},"_source":{"excludes":["*"]},"limit":1,"profile":True})
```
<!-- response Python-asyncio -->
``` python
{'hits': {'hits': [{u'_id': u'100', u'_score': 2500, u'_source': {}}],
          'total': 1},
 'profile': {u'query': {u'children': [{u'children': [{u'querypos': 1,
                                                      u'type': u'KEYWORD',
                                                      u'word': u'i'}],
                                       u'description': u'ИЛИ(KEYWORD(i, querypos=1))',
                                       u'type': u'ИЛИ'},
                                      {u'children': [{u'querypos': 2,
                                                      u'type': u'KEYWORD',
                                                      u'word': u'me'}],
                                       u'description': u'ИЛИ(KEYWORD(me, querypos=2))',
                                       u'type': u'ИЛИ'}],
                        u'description': u'ИЛИ( ИЛИ(KEYWORD(i, querypos=1)),  ИЛИ(KEYWORD(me, querypos=2)))',
                        u'type': u'ИЛИ'}},
 'timed_out': False,
 'took': 0}

```

<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"forum","query":{"query_string":"i me"},"_source":{"excludes":["*"]},"limit":1,"profile":true});
```
<!-- response javascript -->
``` javascript
{"hits": {"hits": [{"_id": 100, "_score": 2500, "_source": {}}],
          "total": 1},
 "profile": {"query": {"children": [{"children": [{"querypos": 1,
                                                      "type": "KEYWORD",
                                                      "word": "i"}],
                                       "description": "ИЛИ(KEYWORD(i, querypos=1))",
                                       "type": "ИЛИ"},
                                      {"children": [{"querypos": 2,
                                                      "type": "KEYWORD",
                                                      "word": "me"}],
                                       "description": "ИЛИ(KEYWORD(me, querypos=2))",
                                       "type": "ИЛИ"}],
                        "description": "ИЛИ( ИЛИ(KEYWORD(i, querypos=1)),  ИЛИ(KEYWORD(me, querypos=2)))",
                        "type": "ИЛИ"}},
 "timed_out": False,
 "took": 0}

```

<!-- intro -->
java
<!-- request Java -->

```java

query = new HashMap<String,Object>();
query.put("query_string","i me");
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
searchRequest.setQuery(query);
searchRequest.setProfile(true);
searchRequest.setLimit(1);
searchRequest.setSort(new ArrayList<String>(){{
    add("*");
}});
searchResponse = searchApi.search(searchRequest);
```
<!-- response Java -->
```java
class SearchResponse {
    took: 18
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        hits: [{_id=100, _score=2500, _source={}}]
        aggregations: null
    }
    profile: {query={type=ИЛИ, description=ИЛИ( ИЛИ(KEYWORD(i, querypos=1)),  ИЛИ(KEYWORD(me, querypos=2))), children=[{type=ИЛИ, description=ИЛИ(KEYWORD(i, querypos=1)), children=[{type=KEYWORD, word=i, querypos=1}]}, {type=ИЛИ, description=ИЛИ(KEYWORD(me, querypos=2)), children=[{type=KEYWORD, word=me, querypos=2}]}]}}
}
```

<!-- intro -->
C#
<!-- request C# -->

```clike
object query =  new { query_string="i me" };
var searchRequest = new SearchRequest("forum", query);
searchRequest.Profile = true;
searchRequest.Limit = 1;
searchRequest.Sort = new List<Object> { "*" };
var searchResponse = searchApi.Search(searchRequest);
```
<!-- response C# -->
```clike
class SearchResponse {
    took: 18
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        hits: [{_id=100, _score=2500, _source={}}]
        aggregations: null
    }
    profile: {query={type=AND, description=AND( AND(KEYWORD(i, querypos=1)),  AND(KEYWORD(me, querypos=2))), children=[{type=AND, description=AND(KEYWORD(i, querypos=1)), children=[{type=KEYWORD, word=i, querypos=1}]}, {type=AND, description=AND(KEYWORD(me, querypos=2)), children=[{type=KEYWORD, word=me, querypos=2}]}]}}
}
```

<!-- intro -->
Рустав
<!-- request Rust -->

```rust
let query = SearchQuery {
     query_string: Some(serde_json::json!("i me").into()),
    ..Default::default()
};
let search_req = SearchRequest {
    table: "forum".to_string(),
    query: Some(Box::new(query)),
    sort: serde_json::json!(["*"]),
    limit: serde_json::json!(1),
    profile: serde_json::json!(true),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```
<!-- response Rust -->
```rust
class SearchResponse {
    took: 18
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        hits: [{_id=100, _score=2500, _source={}}]
        aggregations: null
    }
    profile: {query={type=AND, description=AND( AND(KEYWORD(i, querypos=1)),  AND(KEYWORD(me, querypos=2))), children=[{type=AND, description=AND(KEYWORD(i, querypos=1)), children=[{type=KEYWORD, word=i, querypos=1}]}, {type=AND, description=AND(KEYWORD(me, querypos=2)), children=[{type=KEYWORD, word=me, querypos=2}]}]}}
}
```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
res = await searchApi.search({
  index: 'test',
  query: { query_string: 'Text' }, 
  _source: { excludes: ['*'] },
  limit: 1,
  profile: true
});
```
<!-- response TypeScript -->
``` typescript
{

"hits": 

{

"hits": 

[{

"_id": 1,

"_score": 1480,

"_source": {}

}],
        "total": 1

},

"profile":

{

"query": {

"children": 

[{

"children": 

[{

"querypos": 1,
                    "type": "KEYWORD",
                    "word": "i"
                }],

"description": "AND(KEYWORD(i, querypos=1))",

"type": "AND"

},
            {
            
"children": 
            
[{
            
"querypos": 2,
                    "type": "KEYWORD",
                    "word": "me"
                }],
                "description": "AND(KEYWORD(me, querypos=2))",

"type": "AND"

}],
            "description": "AND( AND(KEYWORD(i, querypos=1)),  AND(KEYWORD(me, querypos=2)))",
            "type": "AND"

}

},

"timed_out": False,

"took": 0
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
searchRequest := manticoresearch.NewSearchRequest("test")
query := map[string]interface{} {"query_string": "Text"}
source := map[string]interface{} { "excludes": []string {"*"} }
searchRequest.SetQuery(query)
searchRequest.SetSource(source)
searchReq.SetLimit(1)
searchReq.SetProfile(true)
res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```
<!-- response Go -->
``` Go
{

"hits": 

{

"hits": 

[{

"_id": 1,

"_score": 1480,

"_source": {}

}],
        "total": 1

},

"profile":

{

"query": {

"children": 

[{

"children": 

[{

"querypos": 1,
                    "type": "KEYWORD",
                    "word": "i"
                }],

"description": "AND(KEYWORD(i, querypos=1))",

"type": "AND"

},
            {
            
"children": 
            
[{
            
"querypos": 2,
                    "type": "KEYWORD",
                    "word": "me"
                }],
                "description": "AND(KEYWORD(me, querypos=2))",

"type": "AND"

}],
            "description": "AND( AND(KEYWORD(i, querypos=1)),  AND(KEYWORD(me, querypos=2)))",
            "type": "AND"

}

},

"timed_out": False,

"took": 0
}
```

<!-- end -->


<!-- example SHOW PLAN EXPANSION -->

In some instances, the evaluated query tree may significantly differ from the original one due to expansions and other transformations.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SET profiling=1;

SELECT id FROM forum WHERE MATCH('@title way* @content hey') LIMIT 1;

SHOW PLAN;
```

<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.00 sec)

+--------+
| id     |
+--------+
| 711651 |
+--------+
1 row in set (0.04 sec)

+------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Variable         | Value                                                                                                                                                                                                                                                                                                                                                                                                                   |
+------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| transformed_tree | И И
  ИЛИ(
    ИЛИ(
      И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(wayne, querypos=1, expanded)),
      ИЛИ(
        И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(ways, querypos=1, expanded)),
        И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(wayyy, querypos=1, expanded)))),
    И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(way, querypos=1, expanded)),
    ИЛИ(поля=(title), КЛЮЧЕВОЕ_СЛОВО(way*, querypos=1, expanded))),
  И(поля=(content), КЛЮЧЕВОЕ_СЛОВО(hey, querypos=2))) |
+------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
1 строка в наборе (0.00 сек)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
  "table": "forum",
  "query": {"query_string": "@title way* @content hey"},
  "_source": { "excludes":["*"] },
  "limit": 1,
  "profile":true
}
```

<!-- response JSON -->
```JSON
{
  "took":33,
  "timed_out":false,
  "hits":
  {
    "total":105,
    "hits":
    [
       {
          "_id": 711651,
          "_score":2539,
          "_source":{}
       }
    ]
  },
  "profile":
  {
    "query":
    {
      "type":"AND",
      "description":"AND( ИЛИ( ИЛИ( И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(wayne, querypos=1, expanded)),  ИЛИ( И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(ways, querypos=1, expanded)),  И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(wayyy, querypos=1, expanded)))),  И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(way, querypos=1, expanded)),  ИЛИ(поля=(title), КЛЮЧЕВОЕ_СЛОВО(way*, querypos=1, expanded))),  И(поля=(content), КЛЮЧЕВОЕ_СЛОВО(hey, querypos=2)))",
      "children":
      [
        {
          "type":"ИЛИ",
          "description":"ИЛИ( ИЛИ( И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(wayne, querypos=1, expanded)),  ИЛИ( И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(ways, querypos=1, expanded)),  И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(wayyy, querypos=1, expanded)))),  И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(way, querypos=1, expanded)),  ИЛИ(поля=(title), КЛЮЧЕВОЕ_СЛОВО(way*, querypos=1, expanded)))",
          "children":
          [
            {
               "type":"ИЛИ",
               "description":"ИЛИ( И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(wayne, querypos=1, expanded)),  ИЛИ( И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(ways, querypos=1, expanded)),  И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(wayyy, querypos=1, expanded))))",
               "children":
               [
                 {
                   "type":"И",
                   "description":"И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(wayne, querypos=1, expanded))",
                   "fields":["title"],
                   "max_field_pos":0,
                   "children":
                   [
                     {
                       "type":"КЛЮЧЕВОЕ_СЛОВО",
                       "word":"wayne",
                       "querypos":1,
                       "expanded":true
                     }
                   ]
                 },
                 {
                   "type":"ИЛИ",
                   "description":"ИЛИ( И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(ways, querypos=1, expanded)),  И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(wayyy, querypos=1, expanded)))",
                   "children":
                   [
                     {
                       "type":"И",
                       "description":"И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(ways, querypos=1, expanded))",
                       "fields":["title"],
                       "max_field_pos":0,
                       "children":
                       [
                         {
                           "type":"КЛЮЧЕВОЕ_СЛОВО",
                           "word":"ways",
                           "querypos":1,
                           "expanded":true
                         }
                       ]
                     },
                     {
                       "type":"И",
                       "description":"И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(wayyy, querypos=1, expanded))",
                       "fields":["title"],
                       "max_field_pos":0,
                       "children":
                       [
                         {
                           "type":"КЛЮЧЕВОЕ_СЛОВО",
                           "word":"wayyy",
                           "querypos":1,
                           "expanded":true
                         }
                       ]
                     }
                   ]
                 }
               ]
            },
            {
              "type":"И",
              "description":"И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(way, querypos=1, expanded))",
              "fields":["title"],
              "max_field_pos":0,
              "children":
              [
                 {
                    "type":"КЛЮЧЕВОЕ_СЛОВО",
                    "word":"way",
                    "querypos":1,
                    "expanded":true
                 }
              ]
            },
            {
              "type":"ИЛИ",
              "description":"ИЛИ(поля=(title), КЛЮЧЕВОЕ_СЛОВО(way*, querypos=1, expanded))",
              "fields":["title"],
              "max_field_pos":0,
              "children":
              [
                {
                  "type":"КЛЮЧЕВОЕ_СЛОВО",
                  "word":"way*",
                  "querypos":1,
                  "expanded":true
                }
              ]
            }
          ]
        },
        {
          "type":"И",
          "description":"И(поля=(content), КЛЮЧЕВОЕ_СЛОВО(hey, querypos=2))",
          "fields":["content"],
          "max_field_pos":0,
          "children":
          [
            {
              "type":"КЛЮЧЕВОЕ_СЛОВО",
              "word":"привет",
              "querypos":2
            }
          ]
        }
      ]
    }
  }
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$result = $index->search('@title way* @content привет')->setSource(['excludes'=>['*']])->setLimit(1)->profile()->get();
print_r($result->getProfile());

```

<!-- response PHP -->
```php
Array
(
    [query] => Array
        (
            [type] => AND
            [description] => AND( OR( OR( AND(fields=(title), KEYWORD(wayne, querypos=1, expanded)),  OR( AND(fields=(title), KEYWORD(ways, querypos=1, expanded)),  AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded)))),  AND(fields=(title), KEYWORD(way, querypos=1, expanded)),  OR(fields=(title), KEYWORD(way*, querypos=1, expanded))),  AND(fields=(content), KEYWORD(привет, querypos=2)))
            [children] => Array
                (
                    [0] => Array
                        (
                            [type] => OR
                            [description] => OR( OR( AND(fields=(title), KEYWORD(wayne, querypos=1, expanded)),  OR( AND(fields=(title), KEYWORD(ways, querypos=1, expanded)),  AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded)))),  AND(fields=(title), KEYWORD(way, querypos=1, expanded)),  OR(fields=(title), KEYWORD(way*, querypos=1, expanded)))
                            [children] => Array
                                (
                                    [0] => Array
                                        (
                                            [type] => OR
                                            [description] => OR( AND(fields=(title), KEYWORD(wayne, querypos=1, expanded)),  OR( AND(fields=(title), KEYWORD(ways, querypos=1, expanded)),  AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded))))
                                            [children] => Array
                                                (
                                                    [0] => Array
                                                        (
                                                            [type] => AND
                                                            [description] => AND(fields=(title), KEYWORD(wayne, querypos=1, expanded))
                                                            [fields] => Array
                                                                (
                                                                    [0] => title
                                                                )
                                                            [max_field_pos] => 0
                                                            [children] => Array
                                                                (
                                                                    [0] => Array
                                                                        (
                                                                            [type] => KEYWORD
                                                                            [word] => wayne
                                                                            [querypos] => 1
                                                                            [expanded] => 1
                                                                        )
                                                                )
                                                        )
                                                    [1] => Array
                                                        (
                                                            [type] => OR
                                                            [description] => OR( AND(fields=(title), KEYWORD(ways, querypos=1, expanded)),  AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded)))
                                                            [children] => Array
                                                                (
                                                                    [0] => Array
                                                                        (
                                                                            [type] => AND
                                                                            [description] => AND(fields=(title), KEYWORD(ways, querypos=1, expanded))
                                                                            [fields] => Array
                                                                                (
                                                                                    [0] => title
                                                                                )

                                                                            [max_field_pos] => 0
                                                                            [children] => Array
                                                                                (
                                                                                    [0] => Array
                                                                                        (
                                                                                            [type] => KEYWORD
                                                                                            [word] => ways
                                                                                            [querypos] => 1
                                                                                            [expanded] => 1
                                                                                        )
                                                                                )
                                                                        )
                                                                    [1] => Массив
                                                                        (
                                                                            [type] => AND
                                                                            [description] => AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded))
                                                                            [fields] => Массив
                                                                                (
                                                                                    [0] => title
                                                                                )
                                                                            [max_field_pos] => 0
                                                                            [children] => Массив
                                                                                (
                                                                                    [0] => Массив
                                                                                        (
                                                                                            [type] => KEYWORD
                                                                                            [word] => wayyy
                                                                                            [querypos] => 1
                                                                                            [expanded] => 1
                                                                                        )
                                                                                )
                                                                        )
                                                                )
                                                        )
                                                )
                                        )
                                    [1] => Массив
                                        (
                                            [type] => AND
                                            [description] => AND(fields=(title), KEYWORD(way, querypos=1, expanded))
                                            [fields] => Массив
                                                (
                                                    [0] => title
                                                )
                                            [max_field_pos] => 0
                                            [children] => Массив
                                                (
                                                    [0] => Массив
                                                        (
                                                            [type] => KEYWORD
                                                            [word] => way
                                                            [querypos] => 1
                                                            [expanded] => 1
                                                        )
                                                )
                                        )
                                    [2] => Массив
                                        (
                                            [type] => OR
                                            [description] => OR(fields=(title), KEYWORD(way*, querypos=1, expanded))
                                            [fields] => Массив
                                                (
                                                    [0] => title
                                                )
                                            [max_field_pos] => 0
                                            [children] => Массив
                                                (
                                                    [0] => Массив
                                                        (
                                                            [type] => KEYWORD
                                                            [word] => way*
                                                            [querypos] => 1
                                                            [expanded] => 1
                                                        )
                                                )
                                        )
                                )
                        )
                    [1] => Массив
                        (
                            [type] => AND
                            [description] => AND(fields=(content), KEYWORD(hey, querypos=2))
                            [fields] => Массив
                                (
                                    [0] => content
                                )
                            [max_field_pos] => 0
                            [children] => Массив
                                (
                                    [0] => Массив
                                        (
                                            [type] => KEYWORD
                                            [word] => hey
                                            [querypos] => 2
                                        )
                                )
                        )
                )
        )
)

```


<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.search({"table":"forum","query":{"query_string":"@title way* @content hey"},"_source":{"excludes":["*"]},"limit":1,"profile":true})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381551',
                    u'_score': 2643,
                    u'_source': {}}],
          'total': 1},
 'profile': {u'query': {u'children': [{u'children': [{u'expanded': True,
                                                      u'querypos': 1,
                                                      u'type': u'KEYWORD',
                                                      u'word': u'way*'}],
                                       u'description': u'AND(fields=(title), KEYWORD(way*, querypos=1, expanded))',
                                       u'fields': [u'title'],
                                       u'type': u'AND'},
                                      {u'children': [{u'querypos': 2,
                                                      u'type': u'KEYWORD',
                                                      u'word': u'hey'}],
                                       u'description': u'AND(fields=(content), KEYWORD(hey, querypos=2))',
                                       u'fields': [u'content'],
                                       u'type': u'AND'}],
                        u'description': u'AND( AND(fields=(title), KEYWORD(way*, querypos=1, expanded)),  AND(fields=(content), KEYWORD(hey, querypos=2)))',
                        u'type': u'AND'}},
 'timed_out': False,
 'took': 0}

```

<!-- intro -->
Python-asyncio
<!-- request Python-asyncio -->

```python
await searchApi.search({"table":"forum","query":{"query_string":"@title way* @content hey"},"_source":{"excludes":["*"]},"limit":1,"profile":true})
```
<!-- response Python-asyncio -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381551',
                    u'_score': 2643,
                    u'_source': {}}],
          'total': 1},
 'profile': {u'query': {u'children': [{u'children': [{u'expanded': True,
                                                      u'querypos': 1,
                                                      u'type': u'KEYWORD',
                                                      u'word': u'way*'}],
                                       u'description': u'AND(fields=(title), KEYWORD(way*, querypos=1, expanded))',
                                       u'fields': [u'title'],
                                       u'type': u'AND'},
                                      {u'children': [{u'querypos': 2,
                                                      u'type': u'KEYWORD',
                                                      u'word': u'hey'}],
                                       u'description': u'AND(fields=(content), KEYWORD(hey, querypos=2))',
                                       u'fields': [u'content'],
                                       u'type': u'AND'}],
                        u'description': u'AND( AND(fields=(title), KEYWORD(way*, querypos=1, expanded)),  AND(fields=(content), KEYWORD(hey, querypos=2)))',
                        u'type': u'AND'}},
 'timed_out': False,
 'took': 0}

```

<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"forum","query":{"query_string":"@title way* @content hey"},"_source":{"excludes":["*"]},"limit":1,"profile":true});
```
<!-- response javascript -->
``` javascript
{"hits": {"hits": [{"_id": 2811025403043381551,
                    "_score": 2643,
                    "_source": {}}],
          "total": 1},
 "profile": {"query": {"children": [{"children": [{"expanded": True,
                                                      "querypos": 1,
                                                      "type": "KEYWORD",
                                                      "word": "way*"}],
                                       "description": "AND(fields=(title), KEYWORD(way*, querypos=1, expanded))",
                                       "fields": ["title"],
                                       "type": "AND"},
                                      {"children": [{"querypos": 2,
                                                      "type": "KEYWORD",
                                                      "word": "hey"}],
                                       "description": "AND(fields=(content), KEYWORD(hey, querypos=2))",
                                       "fields": ["content"],
                                       "type": "AND"}],
                        "description": "AND( AND(fields=(title), KEYWORD(way*, querypos=1, expanded)),  AND(fields=(content), KEYWORD(hey, querypos=2)))",
                        "type": "AND"}},
 "timed_out": False,
 "took": 0}
```

<!-- intro -->
java
<!-- request Java -->

```java

query = new HashMap<String,Object>();
query.put("query_string","@title way* @content hey");
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
searchRequest.setQuery(query);
searchRequest.setProfile(true);
searchRequest.setLimit(1);
searchRequest.setSort(new ArrayList<String>(){{
    add("*");
}});
searchResponse = searchApi.search(searchRequest);
```
<!-- response Java -->
```java
class SearchResponse {
    took: 18
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        hits: [{_id=2811025403043381551, _score=2643, _source={}}]
        aggregations: null
    }
    profile: {query={type=AND, description=AND( AND(fields=(title), KEYWORD(way*, querypos=1, expanded)),  AND(fields=(content), KEYWORD(hey, querypos=2))), children=[{type=AND, description=AND(fields=(title), KEYWORD(way*, querypos=1, expanded)), fields=[title], children=[{type=KEYWORD, word=way*, querypos=1, expanded=true}]}, {type=AND, description=AND(fields=(content), KEYWORD(hey, querypos=2)), fields=[content], children=[{type=KEYWORD, word=hey, querypos=2}]}]}}
}
```
<!-- intro -->
C#
<!-- request C# -->

```clike
object query =  new { query_string="@title way* @content hey" };
var searchRequest = new SearchRequest("forum", query);
searchRequest.Profile = true;
searchRequest.Limit = 1;
searchRequest.Sort = new List<Object> { "*" };
var searchResponse = searchApi.Search(searchRequest);
```
<!-- response C# -->
```clike
class SearchResponse {
    took: 18
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        hits: [{_id=2811025403043381551, _score=2643, _source={}}]
        aggregations: null
    }
    profile: {query={type=AND, description=AND( AND(fields=(title), KEYWORD(way*, querypos=1, expanded)),  AND(fields=(content), KEYWORD(hey, querypos=2))), children=[{type=AND, description=AND(fields=(title), KEYWORD(way*, querypos=1, expanded)), fields=[title], children=[{type=KEYWORD, word=way*, querypos=1, expanded=true}]}, {type=AND, description=AND(fields=(content), KEYWORD(hey, querypos=2)), fields=[content], children=[{type=KEYWORD, word=hey, querypos=2}]}]}}
}
```

<!-- intro -->
Rust
<!-- request Rust -->

```rust
let query = SearchQuery {
     query_string: Some(serde_json::json!("@title way* @content hey").into()),
    ..Default::default()
};
let search_req = SearchRequest {
    table: "forum".to_string(),
    query: Some(Box::new(query)),
    sort: serde_json::json!(["*"]),
    limit: serde_json::json!(1),
    profile: serde_json::json!(true),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```
<!-- response Rust -->
```rust
class SearchResponse {
    took: 18
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        hits: [{_id=2811025403043381551, _score=2643, _source={}}]
        aggregations: null
    }
    profile: {query={type=AND, description=AND( AND(fields=(title), KEYWORD(way*, querypos=1, expanded)),  AND(fields=(content), KEYWORD(hey, querypos=2))), children=[{type=AND, description=AND(fields=(title), KEYWORD(way*, querypos=1, expanded)), fields=[title], children=[{type=KEYWORD, word=way*, querypos=1, expanded=true}]}, {type=AND, description=AND(fields=(content), KEYWORD(hey, querypos=2)), fields=[content], children=[{type=KEYWORD, word=hey, querypos=2}]}]}}
}
```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
res = await searchApi.search({
  index: 'test',
  query: { query_string: '@content 1'},
  _source: { excludes: ["*"] },
  limit:1,
  profile":true
});
```
<!-- response TypeScript -->
``` typescript
# Поиск профилирования

## Как интерпретируется запрос

Рассмотрим этот сложный пример запроса:
```sql
"hello world" @title "пример программы"~5 @body python -(php|perl) @* code
```
Полное значение этого поиска:

* Найти слова 'hello' и 'world' рядом друг с другом в любом поле документа;
* Кроме того, в том же документе также должны быть слова 'пример' и 'программа' в поле заголовка, с максимум 5 словами между ними, но не включая их; (Например, "пример PHP программа" совпадет, но "пример сценарий для представления внешних данных в правильном контексте для вашей программы" не совпадет, так как между двумя терминами 5 или более слов)
* Кроме того, в том же документе должно быть слово 'python' в теле, исключая 'php' или 'perl';
* Наконец, в том же документе должно быть включено слово 'code' в любом поле.

Оператор OR имеет приоритет над AND, поэтому "поиск кошки | собаки | мыши" означает "поиск (кошки | собаки | мыши)", а не "(поиск кошки) | собаки | мыши".

Чтобы понять, как запрос будет выполнен, Manticore Search предоставляет инструменты профилирования запросов для анализа дерева запросов, созданного выражением запроса.

<!-- example profiling -->

## Профилирование дерева запроса в SQL

Чтобы включить профилирование полнотекстового запроса с помощью SQL-запроса, вы должны активировать его перед выполнением желаемого запроса:

```sql
SET profiling =1;
SELECT * FROM test WHERE MATCH('@title abc* @body hey');
```

Чтобы просмотреть дерево запроса, выполните команду `SHOW PLAN` сразу после выполнения запроса:

```sql
SHOW PLAN;
```

Эта команда вернет структуру выполненного запроса. Имейте в виду, что 3 оператора - SET profiling, запрос и SHOW - должны выполняться в одной сессии.


## Профилирование запроса в HTTP JSON

При использовании протокола HTTP JSON мы можем просто включить `"profile":true`, чтобы получить в ответ структуру дерева полнотекстового запроса.

```json
{
  "table":"test",
  "profile":true,
  "query":
  {
    "match_phrase": { "_all" : "had grown quite" }
  }
}
```
Ответ будет включать объект `profile`, содержащий член `query`.

Свойство `query` содержит преобразованное дерево полнотекстового запроса. Каждый узел состоит из:

* `type`: тип узла, который может быть AND, OR, PHRASE, KEYWORD и т. д.
* `description`: поддерево запроса для этого узла, представленное как строка (в формате `SHOW PLAN`)
* `children`: любые дочерние узлы, если они есть
* `max_field_pos`: максимальная позиция в поле

 Узел ключевого слова дополнительно будет включать:

* `word`: преобразованное ключевое слово.
* `querypos`: позиция этого ключевого слова в запросе.
* `excluded`: ключевое слово, исключенное из запроса.
* `expanded`: ключевое слово, добавленное путем расширения префикса.
* `field_start`: ключевое слово должно появляться в начале поля.
* `field_end`: ключевое слово должно появляться в конце поля.
* `boost`: IDF ключевого слова будет умножен на это значение.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SET profiling=1;
SELECT * FROM test WHERE MATCH('@title abc* @body hey');
SHOW PLAN \G
```
<!-- response SQL -->

```sql
*************************** 1\. row ***************************
Переменная: transformed_tree
   Значение: AND(
  OR(fields=(title), KEYWORD(abcx, querypos=1, expanded), KEYWORD(abcm, querypos=1, expanded)),
  AND(fields=(body), KEYWORD(hey, querypos=2)))
1 row in set (0.00 sec)
```

<!-- request JSON -->

```JSON
POST /search
{
  "table": "forum",
  "query": {"query_string": "i me"},
  "_source": { "excludes":["*"] },
  "limit": 1,
  "profile":true
}
```

<!-- response JSON -->
```JSON
{
  "took":1503,
  "timed_out":false,
  "hits":
  {
    "total":406301,
    "hits":
    [
       {
          "_id": 406443,
          "_score":3493,
          "_source":{}
       }
    ]
  },
  "profile":
  {
    "query":
    {
      "type":"AND",
      "description":"AND( AND(KEYWORD(i, querypos=1)),  AND(KEYWORD(me, querypos=2)))",
      "children":
      [
        {
          "type":"AND",
          "description":"AND(KEYWORD(i, querypos=1))",
          "children":
          [
            {
              "type":"KEYWORD",
              "word":"i",
              "querypos":1
            }
          ]
        },
        {
          "type":"AND",
          "description":"AND(KEYWORD(me, querypos=2))",
          "children":
          [
            {
              "type":"KEYWORD",
              "word":"me",
              "querypos":2
            }
          ]
        }
      ]
    }
  }
}
```

<!-- request PHP -->
```php
$result = $index->search('i me')->setSource(['excludes'=>['*']])->setLimit(1)->profile()->get();
print_r($result->getProfile());
```
<!-- response PHP -->
``` php
Array
(
    [query] => Array
        (
            [type] => AND
            [description] => AND( AND(KEYWORD(i, querypos=1)),  AND(KEYWORD(me, querypos=2)))
            [children] => Array
                (
                    [0] => Array
                        (
                            [type] => AND
                            [description] => AND(KEYWORD(i, querypos=1))
                            [children] => Array
                                (
                                    [0] => Array
                                        (
                                            [type] => KEYWORD
                                            [word] => i
                                            [querypos] => 1
                                        )
                                )
                        )
                    [1] => Array
                        (
                            [type] => AND
                            [description] => И(КЛЮЧЕВОЕ_СЛОВО(я, querypos=2))
                            [children] => Array
                                (
                                    [0] => Array
                                        (
                                            [type] => КЛЮЧЕВОЕ_СЛОВО
                                            [word] => я
                                            [querypos] => 2
                                        )
                                )
                        )
                )
        )
)
```

<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.search({"table":"forum","query":{"query_string":"я я"},"_source":{"excludes":["*"]},"limit":1,"profile":True})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'100', u'_score': 2500, u'_source': {}}],
          'total': 1},
 'profile': {u'query': {u'children': [{u'children': [{u'querypos': 1,
                                                      u'type': u'КЛЮЧЕВОЕ_СЛОВО',
                                                      u'word': u'я'}],
                                       u'description': u'И(КЛЮЧЕВОЕ_СЛОВО(я, querypos=1))',
                                       u'type': u'И'},
                                      {u'children': [{u'querypos': 2,
                                                      u'type': u'КЛЮЧЕВОЕ_СЛОВО',
                                                      u'word': u'я'}],
                                       u'description': u'И(КЛЮЧЕВОЕ_СЛОВО(я, querypos=2))',
                                       u'type': u'И'}],
                        u'description': u'И( И(КЛЮЧЕВОЕ_СЛОВО(я, querypos=1)),  И(КЛЮЧЕВОЕ_СЛОВО(я, querypos=2)))',
                        u'type': u'И'}},
 'timed_out': False,
 'took': 0}

```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"forum","query":{"query_string":"я я"},"_source":{"excludes":["*"]},"limit":1,"profile":true});
```
<!-- response javascript -->
``` javascript
{"hits": {"hits": [{"_id": 100, "_score": 2500, "_source": {}}],
          "total": 1},
 "profile": {"query": {"children": [{"children": [{"querypos": 1,
                                                      "type": "КЛЮЧЕВОЕ_СЛОВО",
                                                      "word": "я"}],
                                       "description": "И(КЛЮЧЕВОЕ_СЛОВО(я, querypos=1))",
                                       "type": "И"},
                                      {"children": [{"querypos": 2,
                                                      "type": "КЛЮЧЕВОЕ_СЛОВО",
                                                      "word": "я"}],
                                       "description": "И(КЛЮЧЕВОЕ_СЛОВО(я, querypos=2))",
                                       "type": "И"}],
                        "description": "И( И(КЛЮЧЕВОЕ_СЛОВО(я, querypos=1)),  И(КЛЮЧЕВОЕ_СЛОВО(я, querypos=2)))",
                        "type": "И"}},
 "timed_out": False,
 "took": 0}

```

<!-- intro -->
java
<!-- request Java -->

```java

query = new HashMap<String,Object>();
query.put("query_string","я я");
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
searchRequest.setQuery(query);
searchRequest.setProfile(true);
searchRequest.setLimit(1);
searchRequest.setSort(new ArrayList<String>(){{
    add("*");
}});
searchResponse = searchApi.search(searchRequest);
```
<!-- response Java -->
```java
class SearchResponse {
    took: 18
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        hits: [{_id=100, _score=2500, _source={}}]
        aggregations: null
    }
    profile: {query={type=И, description=И( И(КЛЮЧЕВОЕ_СЛОВО(я, querypos=1)),  И(КЛЮЧЕВОЕ_СЛОВО(я, querypos=2))), children=[{type=И, description=И(КЛЮЧЕВОЕ_СЛОВО(я, querypos=1)), children=[{type=КЛЮЧЕВОЕ_СЛОВО, word=я, querypos=1}]}, {type=И, description=И(КЛЮЧЕВОЕ_СЛОВО(я, querypos=2)), children=[{type=КЛЮЧЕВОЕ_СЛОВО, word=я, querypos=2}]}]}}
}
```

<!-- intro -->
C#
<!-- request C# -->

```clike
object query =  new { query_string="я я" };
var searchRequest = new SearchRequest("forum", query);
searchRequest.Profile = true;
searchRequest.Limit = 1;
searchRequest.Sort = new List<Object> { "*" };
var searchResponse = searchApi.Search(searchRequest);
```
<!-- response C# -->
```clike
class SearchResponse {
    took: 18
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        hits: [{_id=100, _score=2500, _source={}}]
        aggregations: null
    }
    profile: {query={type=И, description=И( И(КЛЮЧЕВОЕ_СЛОВО(я, querypos=1)),  И(КЛЮЧЕВОЕ_СЛОВО(я, querypos=2))), children=[{type=И, description=И(КЛЮЧЕВОЕ_СЛОВО(я, querypos=1)), children=[{type=КЛЮЧЕВОЕ_СЛОВО, word=я, querypos=1}]}, {type=И, description=И(КЛЮЧЕВОЕ_СЛОВО(я, querypos=2)), children=[{type=КЛЮЧЕВОЕ_СЛОВО, word=я, querypos=2}]}]}}
}
```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
res = await searchApi.search({
  index: 'test',
  query: { query_string: 'Текст' }, 
  _source: { excludes: ['*'] },
  limit: 1,
  profile: true
});
```
<!-- response TypeScript -->
``` typescript
{
	"hits": 
	{
		"hits": 
		[{
			"_id": 1,
			"_score": 1480,
			"_source": {}
		}],
        "total": 1
	},
	"profile":
	{
		"query": {
			"children": 
			[{
				"children": 
				[{
					"querypos": 1,
                    "type": "КЛЮЧЕВОЕ_СЛОВО",
                    "word": "я"
                }],
				"description": "И(КЛЮЧЕВОЕ_СЛОВО(я, querypos=1))",
				"type": "И"
			},
            {
            	"children": 
            	[{
            		"querypos": 2,
                    "type": "КЛЮЧЕВОЕ_СЛОВО",
                    "word": "me"
                }],
                "description": "И(КЛЮЧЕВОЕ_СЛОВО(me, querypos=2))",
				"type": "И"
			}],
            "description": "И( И(КЛЮЧЕВОЕ_СЛОВО(i, querypos=1)),  И(КЛЮЧЕВОЕ_СЛОВО(me, querypos=2)))",
            "type": "И"
		}
	},
	"timed_out": Ложь,
	"took": 0
}
```

<!-- intro -->
Идти
<!-- request Go -->

```go
searchRequest := manticoresearch.NewSearchRequest("test")
query := map[string]interface{} {"query_string": "Text"}
source := map[string]interface{} { "excludes": []string {"*"} }
searchRequest.SetQuery(query)
searchRequest.SetSource(source)
searchReq.SetLimit(1)
searchReq.SetProfile(true)
res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```
<!-- response Go -->
``` Go
{
	"hits": 
	{
		"hits": 
		[{
			"_id": 1,
			"_score": 1480,
			"_source": {}
		}],
        "total": 1
	},
	"profile":
	{
		"query": {
			"children": 
			[{
				"children": 
				[{
					"querypos": 1,
                    "type": "КЛЮЧЕВОЕ_СЛОВО",
                    "word": "i"
                }],
				"description": "И(КЛЮЧЕВОЕ_СЛОВО(i, querypos=1))",
				"type": "И"
			},
            {
            	"children": 
            	[{
            		"querypos": 2,
                    "type": "КЛЮЧЕВОЕ_СЛОВО",
                    "word": "me"
                }],
                "description": "И(КЛЮЧЕВОЕ_СЛОВО(me, querypos=2))",
				"type": "И"
			}],
            "description": "И( И(КЛЮЧЕВОЕ_СЛОВО(i, querypos=1)),  И(КЛЮЧЕВОЕ_СЛОВО(me, querypos=2)))",
            "type": "И"
		}
	},
	"timed_out": Ложь,
	"took": 0
}
```

<!-- end -->


<!-- example SHOW PLAN EXPANSION -->

В некоторых случаях, оцененное дерево запросов может существенно отличаться от оригинального из-за расширений и других трансформаций.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SET profiling=1;

SELECT id FROM forum WHERE MATCH('@title way* @content hey') LIMIT 1;

SHOW PLAN;
```

<!-- response SQL -->

```sql
Запрос выполнен, затронуто 0 строк (0.00 сек)

+--------+
| id     |
+--------+
| 711651 |
+--------+
1 строка в наборе (0.04 сек)

+------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Переменная       | Значение                                                                                                                                                                                                                                                                                                                                                                                                                   |
+------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| transformed_tree | И(
  ИЛИ(
    ИЛИ(
      И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(wayne, querypos=1, expanded)),
      ИЛИ(
        И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(ways, querypos=1, expanded)),
        И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(wayyy, querypos=1, expanded)))),
    И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(way, querypos=1, expanded)),
    ИЛИ(поля=(title), КЛЮЧЕВОЕ_СЛОВО(way*, querypos=1, expanded))),
  И(поля=(content), КЛЮЧЕВОЕ_СЛОВО(hey, querypos=2))) |
+------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
1 строка в наборе (0.00 сек)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /search
{
  "table": "forum",
  "query": {"query_string": "@title way* @content hey"},
  "_source": { "excludes":["*"] },
  "limit": 1,
  "profile":true
}
```

<!-- response JSON -->
```JSON
{
  "took":33,
  "timed_out":false,
  "hits":
  {
    "total":105,
    "hits":
    [
       {
          "_id": 711651,
          "_score":2539,
          "_source":{}
       }
    ]
  },
  "profile":
  {
    "query":
    {
      "type":"И",
      "description":"И( ИЛИ( ИЛИ( И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(wayne, querypos=1, expanded)),  ИЛИ( И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(ways, querypos=1, expanded)),  И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(wayyy, querypos=1, expanded)))),  И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(way, querypos=1, expanded)),  ИЛИ(поля=(title), КЛЮЧЕВОЕ_СЛОВО(way*, querypos=1, expanded))),  И(поля=(content), КЛЮЧЕВОЕ_СЛОВО(hey, querypos=2)))",
      "children":
      [
        {
          "type":"ИЛИ",
          "description":"ИЛИ( ИЛИ( И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(wayne, querypos=1, expanded)),  ИЛИ( И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(ways, querypos=1, expanded)),  И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(wayyy, querypos=1, expanded)))),  И(поля=(title), КЛЮЧЕВОЕ_СЛОВО(way, querypos=1, expanded)),  ИЛИ(поля=(title), КЛЮЧЕВОЕ_СЛОВО(way*, querypos=1, expanded)))",
          "children":
          [
            {
               "type":"ИЛИ",
               "description":"OR( AND(fields=(title), KEYWORD(wayne, querypos=1, expanded)),  OR( AND(fields=(title), KEYWORD(ways, querypos=1, expanded)),  AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded))))",
               "children":
               [
                 {
                   "type":"AND",
                   "description":"AND(fields=(title), KEYWORD(wayne, querypos=1, expanded))",
                   "fields":["title"],
                   "max_field_pos":0,
                   "children":
                   [
                     {
                       "type":"KEYWORD",
                       "word":"wayne",
                       "querypos":1,
                       "expanded":true
                     }
                   ]
                 },
                 {
                   "type":"OR",
                   "description":"OR( AND(fields=(title), KEYWORD(ways, querypos=1, expanded)),  AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded)))",
                   "children":
                   [
                     {
                       "type":"AND",
                       "description":"AND(fields=(title), KEYWORD(ways, querypos=1, expanded))",
                       "fields":["title"],
                       "max_field_pos":0,
                       "children":
                       [
                         {
                           "type":"KEYWORD",
                           "word":"ways",
                           "querypos":1,
                           "expanded":true
                         }
                       ]
                     },
                     {
                       "type":"AND",
                       "description":"AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded))",
                       "fields":["title"],
                       "max_field_pos":0,
                       "children":
                       [
                         {
                           "type":"KEYWORD",
                           "word":"wayyy",
                           "querypos":1,
                           "expanded":true
                         }
                       ]
                     }
                   ]
                 }
               ]
            },
            {
              "type":"AND",
              "description":"AND(fields=(title), KEYWORD(way, querypos=1, expanded))",
              "fields":["title"],
              "max_field_pos":0,
              "children":
              [
                 {
                    "type":"KEYWORD",
                    "word":"way",
                    "querypos":1,
                    "expanded":true
                 }
              ]
            },
            {
              "type":"OR",
              "description":"OR(fields=(title), KEYWORD(way*, querypos=1, expanded))",
              "fields":["title"],
              "max_field_pos":0,
              "children":
              [
                {
                  "type":"KEYWORD",
                  "word":"way*",
                  "querypos":1,
                  "expanded":true
                }
              ]
            }
          ]
        },
        {
          "type":"AND",
          "description":"AND(fields=(content), KEYWORD(hey, querypos=2))",
          "fields":["content"],
          "max_field_pos":0,
          "children":
          [
            {
              "type":"KEYWORD",
              "word":"hey",
              "querypos":2
            }
          ]
        }
      ]
    }
  }
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$result = $index->search('@title way* @content hey')->setSource(['excludes'=>['*']])->setLimit(1)->profile()->get();
print_r($result->getProfile());

```

<!-- response PHP -->
```php
Array
(
    [query] => Array
        (
            [type] => AND
            [description] => AND( OR( OR( AND(fields=(title), KEYWORD(wayne, querypos=1, expanded)),  OR( AND(fields=(title), KEYWORD(ways, querypos=1, expanded)),  AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded)))),  AND(fields=(title), KEYWORD(way, querypos=1, expanded)),  OR(fields=(title), KEYWORD(way*, querypos=1, expanded))),  AND(fields=(content), KEYWORD(hey, querypos=2)))
            [children] => Array
                (
                    [0] => Array
                        (
                            [type] => OR
                            [description] => OR( OR( AND(fields=(title), KEYWORD(wayne, querypos=1, expanded)),  OR( AND(fields=(title), KEYWORD(ways, querypos=1, expanded)),  AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded)))),  AND(fields=(title), KEYWORD(way, querypos=1, expanded)),  OR(fields=(title), KEYWORD(way*, querypos=1, expanded)))
                            [children] => Array
                                (
                                    [0] => Array
                                        (
                                            [type] => OR
                                            [description] => OR( AND(fields=(title), KEYWORD(wayne, querypos=1, expanded)),  OR( AND(fields=(title), KEYWORD(ways, querypos=1, expanded)),  AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded))))
                                            [children] => Array
                                                (
                                                    [0] => Array
                                                        (
                                                            [type] => AND
                                                            [description] => AND(fields=(title), KEYWORD(wayne, querypos=1, expanded))
                                                            [fields] => Array
                                                                (
                                                                    [0] => title
                                                                )
                                                            [max_field_pos] => 0
                                                            [children] => Array
                                                                (
                                                                    [0] => Array
                                                                        (
                                                                            [type] => KEYWORD
                                                                            [word] => wayne
                                                                            [querypos] => 1
                                                                            [expanded] => 1
                                                                        )
                                                                )
                                                        )
                                                    [1] => Array
                                                        (
                                                            [type] => OR
                                                            [description] => OR( AND(fields=(title), KEYWORD(ways, querypos=1, expanded)),  AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded)))
                                                            [children] => Array
                                                                (
                                                                    [0] => Array
                                                                        (
                                                                            [type] => AND
                                                                            [description] => AND(fields=(title), KEYWORD(ways, querypos=1, expanded))
                                                                            [fields] => Array
                                                                                (
                                                                                    [0] => title
                                                                                )

                                                                            [max_field_pos] => 0
                                                                            [children] => Array
                                                                                (
                                                                                    [0] => Array
                                                                                        (
                                                                                            [type] => KEYWORD
                                                                                            [word] => ways
                                                                                            [querypos] => 1
                                                                                            [expanded] => 1
                                                                                        )
                                                                                )
                                                                        )
                                                                    [1] => Array
                                                                        (
                                                                            [type] => AND
                                                                            [description] => AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded))
                                                                            [fields] => Array
                                                                                (
                                                                                    [0] => title
                                                                                )
                                                                            [max_field_pos] => 0
                                                                            [children] => Array
                                                                                (
                                                                                    [0] => Array
                                                                                        (
                                                                                            [type] => KEYWORD
                                                                                            [word] => wayyy
                                                                                            [querypos] => 1
                                                                                            [expanded] => 1
                                                                                        )
                                                                                )
                                                                        )
                                                                )
                                                        )
                                                )
                                        )
                                    [1] => Array
                                        (
                                            [type] => AND
                                            [description] => И И(fields=(title), КЛЮЧЕВОЕ(way, querypos=1, expanded))
                                            [fields] => Массив
                                                (
                                                    [0] => title
                                                )
                                            [max_field_pos] => 0
                                            [children] => Массив
                                                (
                                                    [0] => Массив
                                                        (
                                                            [type] => КЛЮЧЕВОЕ
                                                            [word] => way
                                                            [querypos] => 1
                                                            [expanded] => 1
                                                        )
                                                )
                                        )
                                    [2] => Массив
                                        (
                                            [type] => ИЛИ
                                            [description] => ИЛИ(fields=(title), КЛЮЧЕВОЕ(way*, querypos=1, expanded))
                                            [fields] => Массив
                                                (
                                                    [0] => title
                                                )
                                            [max_field_pos] => 0
                                            [children] => Массив
                                                (
                                                    [0] => Массив
                                                        (
                                                            [type] => КЛЮЧЕВОЕ
                                                            [word] => way*
                                                            [querypos] => 1
                                                            [expanded] => 1
                                                        )
                                                )
                                        )
                                )
                        )
                    [1] => Массив
                        (
                            [type] => И
                            [description] => И(fields=(content), КЛЮЧЕВОЕ(hey, querypos=2))
                            [fields] => Массив
                                (
                                    [0] => content
                                )
                            [max_field_pos] => 0
                            [children] => Массив
                                (
                                    [0] => Массив
                                        (
                                            [type] => КЛЮЧЕВОЕ
                                            [word] => hey
                                            [querypos] => 2
                                        )
                                )
                        )
                )
        )
)

```


<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.search({"table":"forum","query":{"query_string":"@title way* @content hey"},"_source":{"excludes":["*"]},"limit":1,"profile":true})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381551',
                    u'_score': 2643,
                    u'_source': {}}],
          'total': 1},
 'profile': {u'query': {u'children': [{u'children': [{u'expanded': True,
                                                      u'querypos': 1,
                                                      u'type': u'КЛЮЧЕВОЕ',
                                                      u'word': u'way*'}],
                                       u'description': u'И(fields=(title), КЛЮЧЕВОЕ(way*, querypos=1, expanded))',
                                       u'fields': [u'title'],
                                       u'type': u'И'},
                                      {u'children': [{u'querypos': 2,
                                                      u'type': u'КЛЮЧЕВОЕ',
                                                      u'word': u'hey'}],
                                       u'description': u'И(fields=(content), КЛЮЧЕВОЕ(hey, querypos=2))',
                                       u'fields': [u'content'],
                                       u'type': u'И'}],
                        u'description': u'И( И(fields=(title), КЛЮЧЕВОЕ(way*, querypos=1, expanded)),  И(fields=(content), КЛЮЧЕВОЕ(hey, querypos=2)))',
                        u'type': u'И'}},
 'timed_out': False,
 'took': 0}

```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"forum","query":{"query_string":"@title way* @content hey"},"_source":{"excludes":["*"]},"limit":1,"profile":true});
```
<!-- response javascript -->
``` javascript
{"hits": {"hits": [{"_id": 2811025403043381551,
                    "_score": 2643,
                    "_source": {}}],
          "total": 1},
 "profile": {"query": {"children": [{"children": [{"expanded": True,
                                                      "querypos": 1,
                                                      "type": "КЛЮЧЕВОЕ",
                                                      "word": "way*"}],
                                       "description": "И(fields=(title), КЛЮЧЕВОЕ(way*, querypos=1, expanded))",
                                       "fields": ["title"],
                                       "type": "AND"},
                                      {"children": [{"querypos": 2,
                                                      "type": "KEYWORD",
                                                      "word": "hey"}],
                                       "description": "AND(fields=(content), KEYWORD(hey, querypos=2))",
                                       "fields": ["content"],
                                       "type": "AND"}],
                        "description": "AND( AND(fields=(title), KEYWORD(way*, querypos=1, expanded)),  AND(fields=(content), KEYWORD(hey, querypos=2)))",
                        "type": "AND"}},
 "timed_out": False,
 "took": 0}
```

<!-- intro -->
java
<!-- request Java -->

```java

query = new HashMap<String,Object>();
query.put("query_string","@title way* @content hey");
searchRequest = new SearchRequest();
searchRequest.setIndex("forum");
searchRequest.setQuery(query);
searchRequest.setProfile(true);
searchRequest.setLimit(1);
searchRequest.setSort(new ArrayList<String>(){{
    add("*");
}});
searchResponse = searchApi.search(searchRequest);
```
<!-- response Java -->
```java
class SearchResponse {
    took: 18
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        hits: [{_id=2811025403043381551, _score=2643, _source={}}]
        aggregations: null
    }
    profile: {query={type=AND, description=AND( AND(fields=(title), KEYWORD(way*, querypos=1, expanded)),  AND(fields=(content), KEYWORD(hey, querypos=2))), children=[{type=AND, description=AND(fields=(title), KEYWORD(way*, querypos=1, expanded)), fields=[title], children=[{type=KEYWORD, word=way*, querypos=1, expanded=true}]}, {type=AND, description=AND(fields=(content), KEYWORD(hey, querypos=2)), fields=[content], children=[{type=KEYWORD, word=hey, querypos=2}]}]}}
}
```

<!-- intro -->
C#
<!-- request C# -->

```clike
object query =  new { query_string="@title way* @content hey" };
var searchRequest = new SearchRequest("forum", query);
searchRequest.Profile = true;
searchRequest.Limit = 1;
searchRequest.Sort = new List<Object> { "*" };
var searchResponse = searchApi.Search(searchRequest);
```
<!-- response C# -->
```clike
class SearchResponse {
    took: 18
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        hits: [{_id=2811025403043381551, _score=2643, _source={}}]
        aggregations: null
    }
    profile: {query={type=AND, description=AND( AND(fields=(title), KEYWORD(way*, querypos=1, expanded)),  AND(fields=(content), KEYWORD(hey, querypos=2))), children=[{type=AND, description=AND(fields=(title), KEYWORD(way*, querypos=1, expanded)), fields=[title], children=[{type=KEYWORD, word=way*, querypos=1, expanded=true}]}, {type=AND, description=AND(fields=(content), KEYWORD(hey, querypos=2)), fields=[content], children=[{type=KEYWORD, word=hey, querypos=2}]}]}}
}
```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
res = await searchApi.search({
  index: 'test',
  query: { query_string: '@content 1'},
  _source: { excludes: ["*"] },
  limit:1,
  profile":true
});
```
<!-- response TypeScript -->
``` typescript
{
	"hits": 
	{
		"hits": 
		[{
			"_id": 1,
            "_score": 1480,
            "_source": {}
        }],
        "total": 1
    },
 	"profile": 
 	{
 		"query": 
 		{
 			"children": 
 			[{
 				"children": 
 				[{
 					"expanded": True,
                    "querypos": 1,
                    "type": "KEYWORD",
                    "word": "1*"
                }],
                "description": "AND(fields=(content), KEYWORD(1*, querypos=1, expanded))",
                "fields": ["content"],
                "type": "AND"
            }],
            "description": "AND(fields=(content), KEYWORD(1*, querypos=1))",
            "type": "AND"
        }},
	"timed_out": False,
	"took": 0
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
searchRequest := manticoresearch.NewSearchRequest("test")
query := map[string]interface{} {"query_string": "1*"}
source := map[string]interface{} { "excludes": []string {"*"} }
searchRequest.SetQuery(query)
searchRequest.SetSource(source)
searchReq.SetLimit(1)
searchReq.SetProfile(true)
res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```
<!-- response Go -->
``` go
{
	"hits": 
	{
		"hits": 
		[{
			"_id": 1,
            "_score": 1480,
            "_source": {}
        }],
        "total": 1
    },
 	"profile": 
 	{
 		"query": 
 		{
 			"children": 
 			[{
 				"children": 
 				[{
 					"expanded": True,
                    "querypos": 1,
                    "type": "KEYWORD",
                    "word": "1*"
                }],
                "description": "AND(fields=(content), KEYWORD(1*, querypos=1, expanded))",
                "fields": ["content"],
                "type": "AND"
            }],
            "description": "AND(fields=(content), KEYWORD(1*, querypos=1))",
            "type": "AND"
        }},
	"timed_out": False,
	"took": 0
}
```

<!-- end -->


## Профилирование без выполнения запроса

<!-- Example Explain_query -->
SQL-оператор `EXPLAIN QUERY` позволяет отобразить дерево выполнения для данного полнотекстового запроса без выполнения фактического поискового запроса в таблице.



<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
EXPLAIN QUERY index_base '@title running @body dog'\G
```
<!-- response SQL -->

```sql
 EXPLAIN QUERY index_base '@title running @body dog'\G
*************************** 1\. row ***************************
Переменная: transformed_tree
   Значение: AND(
      OR(
            AND(fields=(title), KEYWORD(run, querypos=1, morphed)),
            AND(fields=(title), KEYWORD(running, querypos=1, morphed))))
  AND(fields=(body), KEYWORD(dog, querypos=2, morphed)))
```
<!-- end -->

<!-- Пример Explain_query_dot -->
`EXPLAIN QUERY ... option format=dot` позволяет отображать дерево выполнения предоставленного полнотекстового запроса в иерархическом формате, подходящем для визуализации с помощью существующих инструментов, таких как https://dreampuf.github.io/GraphvizOnline:

![EXPLAIN QUERY graphviz example](graphviz.png)

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
EXPLAIN QUERY tbl 'i me' option format=dot\G
```
<!-- response SQL -->

```sql
EXPLAIN QUERY tbl 'i me' option format=dot\G
*************************** 1. row ***************************
Переменная: transformed_tree
   Значение: digraph "transformed_tree"
{

0 [shape=record,style=filled,bgcolor="lightgrey" label="AND"]
0 -> 1
1 [shape=record,style=filled,bgcolor="lightgrey" label="AND"]
1 -> 2
2 [shape=record label="i | { querypos=1 }"]
0 -> 3
3 [shape=record,style=filled,bgcolor="lightgrey" label="AND"]
3 -> 4
4 [shape=record label="me | { querypos=2 }"]
}
```
<!-- end -->

## Просмотр значений факторов совпадения
<!-- example factors -->
При использовании ранжировщика выражений возможно раскрыть значения вычисленных факторов с помощью функции [PACKEDFACTORS()](../../Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29).

Функция возвращает:

* Значения факторов на уровне документа (таких как bm25, field_mask, doc_word_count)
* Список каждого поля, которое сгенерировало попадание (включая lcs, hit_count, word_count, sum_idf, min_hit_pos и т.д.)
* Список каждого ключевого слова из запроса вместе с их значениями tf и idf


Эти значения могут быть использованы для понимания, почему определенные документы получают более низкие или высокие баллы в поиске или для уточнения существующего выражения ранжирования.

<!-- intro -->
Пример:

<!-- request SQL -->
```sql
SELECT id, PACKEDFACTORS() FROM test1 WHERE MATCH('test one') OPTION ranker=expr('1')\G
```

<!-- response SQL -->
```sql
             id: 1
packedfactors(): bm25=569, bm25a=0.617197, field_mask=2, doc_word_count=2,
    field1=(lcs=1, hit_count=2, word_count=2, tf_idf=0.152356,
        min_idf=-0.062982, max_idf=0.215338, sum_idf=0.152356, min_hit_pos=4,
        min_best_span_pos=4, exact_hit=0, max_window_hits=1, min_gaps=2,
        exact_order=1, lccs=1, wlccs=0.215338, atc=-0.003974),
    word0=(tf=1, idf=-0.062982),
    word1=(tf=1, idf=0.215338)
1 row in set (0.00 sec)
```
<!-- end -->
<!-- proofread -->
