# Search profiling

## How a query is interpreted

Example of a complex query:
```sql
"hello world" @title "example program"~5 @body python -(php|perl) @* code
```
The full meaning of this search is:

* Find the words 'hello' and 'world' adjacently in any field in a document;
* Additionally, the same document must also contain the words 'example' and 'program' in the title field, with up to, but not including, 5 words between the words in question; (E.g. "example PHP program" would be matched however "example script to introduce outside data into the correct context for your program" would not because two terms have 5 or more words between them)
* Additionally, the same document must contain the word 'python' in the body field, but not contain either 'php' or 'perl';
* Additionally, the same document must contain the word 'code' in any field.

OR operator precedence is higher than AND, so "looking for cat | dog | mouse" means "looking for ( cat | dog | mouse )" and not "(looking for cat) | dog | mouse".

To understand how a query will be executed, Manticore Search offer query profile tooling for viewing the query tree created by a query expression.

<!-- example profiling -->

## Profiling the query tree in SQL

When using SQL statement the full-text query profiling needs to be enabled before running the desired query:

```sql
SET profiling =1;
SELECT * FROM test WHERE MATCH('@title abc* @body hey');
```

To view the query tree, we must run `SHOW PLAN` right after the execution of the query:

```sql
SHOW PLAN;
```

The command will return the structure of the executed query. Please note that the 3 statements - SET profiling, the query and SHOW - must run on the same session.


## Profiling the query in HTTP JSON

When using the HTTP JSON protocol we can just enable `"profile":true` to get in response the full-text query tree structure.

```json
{
  "index":"test",
  "profile":true,
  "query":
  {
    "match_phrase": { "_all" : "had grown quite" }
  }
}
```
The response will contain a `profile` object in which we can find a member `query`.

`query` property contains the transformed full-text query tree. Each node contains:

 * `type`: node type. Can be AND, OR, PHRASE, KEYWORD etc.
 * `description`: query subtree for this node shown as a string (in `SHOW PLAN` format)
 * `children`: child nodes, if any
 * `max_field_pos`: maximum position within a field

 A keyword node will also provide:

 * `word`: transformed keyword.
 * `querypos`: position of this keyword in a query.
 * `excluded`: keyword excluded from query.
 * `expanded`: keyword added by prefix expansion.
 * `field_start`: keyword must occur at the very start of the field.
 * `field_end`: keyword must occur at the very end of the field.
 * `boost`: keyword IDF will be multiplied by this.


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
Variable: transformed_tree
   Value: AND(
  OR(fields=(title), KEYWORD(abcx, querypos=1, expanded), KEYWORD(abcm, querypos=1, expanded)),
  AND(fields=(body), KEYWORD(hey, querypos=2)))
1 row in set (0.00 sec)
```

<!-- request HTTP -->

``` rest
POST /search
{
  "index": "forum",
  "query": {"query_string": "i me"},
  "_source": { "excludes":["*"] },
  "limit": 1,
  "profile":true
}
```

<!-- response HTTP -->
``` rest
{
  "took":1503,
  "timed_out":false,
  "hits":
  {
    "total":406301,
    "hits":
    [
       {
          "_id":"406443",
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
<!-- response HTTP -->
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
                            [description] => AND(KEYWORD(me, querypos=2))
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
searchApi.search({"index":"forum","query":{"query_string":"i me"},"_source":{"excludes":["*"]},"limit":1,"profile":True})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'100', u'_score': 2500, u'_source': {}}],
          'total': 1},
 'profile': {u'query': {u'children': [{u'children': [{u'querypos': 1,
                                                      u'type': u'KEYWORD',
                                                      u'word': u'i'}],
                                       u'description': u'AND(KEYWORD(i, querypos=1))',
                                       u'type': u'AND'},
                                      {u'children': [{u'querypos': 2,
                                                      u'type': u'KEYWORD',
                                                      u'word': u'me'}],
                                       u'description': u'AND(KEYWORD(me, querypos=2))',
                                       u'type': u'AND'}],
                        u'description': u'AND( AND(KEYWORD(i, querypos=1)),  AND(KEYWORD(me, querypos=2)))',
                        u'type': u'AND'}},
 'timed_out': False,
 'took': 0}

```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.search({"index":"forum","query":{"query_string":"i me"},"_source":{"excludes":["*"]},"limit":1,"profile":true});
```
<!-- response javascript -->
``` javascript
{"hits": {"hits": [{"_id": "100", "_score": 2500, "_source": {}}],
          "total": 1},
 "profile": {"query": {"children": [{"children": [{"querypos": 1,
                                                      "type": "KEYWORD",
                                                      "word": "i"}],
                                       "description": "AND(KEYWORD(i, querypos=1))",
                                       "type": "AND"},
                                      {"children": [{"querypos": 2,
                                                      "type": "KEYWORD",
                                                      "word": "me"}],
                                       "description": "AND(KEYWORD(me, querypos=2))",
                                       "type": "AND"}],
                        "description": "AND( AND(KEYWORD(i, querypos=1)),  AND(KEYWORD(me, querypos=2)))",
                        "type": "AND"}},
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
    profile: {query={type=AND, description=AND( AND(KEYWORD(i, querypos=1)),  AND(KEYWORD(me, querypos=2))), children=[{type=AND, description=AND(KEYWORD(i, querypos=1)), children=[{type=KEYWORD, word=i, querypos=1}]}, {type=AND, description=AND(KEYWORD(me, querypos=2)), children=[{type=KEYWORD, word=me, querypos=2}]}]}}
}
```

<!-- end -->


<!-- example SHOW PLAN EXPANSION -->

In some cases the evaluated query tree can be rather different from the original one because of expansions and other transformations.

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
| transformed_tree | AND(
  OR(
    OR(
      AND(fields=(title), KEYWORD(wayne, querypos=1, expanded)),
      OR(
        AND(fields=(title), KEYWORD(ways, querypos=1, expanded)),
        AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded)))),
    AND(fields=(title), KEYWORD(way, querypos=1, expanded)),
    OR(fields=(title), KEYWORD(way*, querypos=1, expanded))),
  AND(fields=(content), KEYWORD(hey, querypos=2))) |
+------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
1 row in set (0.00 sec)
```

<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```http
POST /search
{
  "index": "forum",
  "query": {"query_string": "@title way* @content hey"},
  "_source": { "excludes":["*"] },
  "limit": 1,
  "profile":true
}
```

<!-- response HTTP -->
```
{
  "took":33,
  "timed_out":false,
  "hits":
  {
    "total":105,
    "hits":
    [
       {
          "_id":"711651",
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
      "description":"AND( OR( OR( AND(fields=(title), KEYWORD(wayne, querypos=1, expanded)),  OR( AND(fields=(title), KEYWORD(ways, querypos=1, expanded)),  AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded)))),  AND(fields=(title), KEYWORD(way, querypos=1, expanded)),  OR(fields=(title), KEYWORD(way*, querypos=1, expanded))),  AND(fields=(content), KEYWORD(hey, querypos=2)))",
      "children":
      [
        {
          "type":"OR",
          "description":"OR( OR( AND(fields=(title), KEYWORD(wayne, querypos=1, expanded)),  OR( AND(fields=(title), KEYWORD(ways, querypos=1, expanded)),  AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded)))),  AND(fields=(title), KEYWORD(way, querypos=1, expanded)),  OR(fields=(title), KEYWORD(way*, querypos=1, expanded)))",
          "children":
          [
            {
               "type":"OR",
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
                                            [description] => AND(fields=(title), KEYWORD(way, querypos=1, expanded))
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
                                                            [word] => way
                                                            [querypos] => 1
                                                            [expanded] => 1
                                                        )
                                                )
                                        )
                                    [2] => Array
                                        (
                                            [type] => OR
                                            [description] => OR(fields=(title), KEYWORD(way*, querypos=1, expanded))
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
                                                            [word] => way*
                                                            [querypos] => 1
                                                            [expanded] => 1
                                                        )
                                                )
                                        )
                                )
                        )
                    [1] => Array
                        (
                            [type] => AND
                            [description] => AND(fields=(content), KEYWORD(hey, querypos=2))
                            [fields] => Array
                                (
                                    [0] => content
                                )
                            [max_field_pos] => 0
                            [children] => Array
                                (
                                    [0] => Array
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
searchApi.search({"index":"forum","query":{"query_string":"@title way* @content hey"},"_source":{"excludes":["*"]},"limit":1,"profile":true})
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
javascript
<!-- request javascript -->

```javascript
res = await searchApi.search({"index":"forum","query":{"query_string":"@title way* @content hey"},"_source":{"excludes":["*"]},"limit":1,"profile":true});
```
<!-- response javascript -->
``` javascript
{"hits": {"hits": [{"_id": "2811025403043381551",
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
<!-- end -->


## Profiling without running a query

<!-- Example Explain_query -->
The SQL statement `EXPLAIN QUERY` allows displaying the execution tree of a provided full-text query without running an actual search query on the table.



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
Variable: transformed_tree
   Value: AND(
      OR(
            AND(fields=(title), KEYWORD(run, querypos=1, morphed)),
            AND(fields=(title), KEYWORD(running, querypos=1, morphed))))
  AND(fields=(body), KEYWORD(dog, querypos=2, morphed)))
```
<!-- end -->

<!-- Example Explain_query_dot -->
`EXPLAIN QUERY ... option format=dot` allows displaying the execution tree of a provided full-text query in hierarchical format suitable for visualization by existing tools, for example https://dreampuf.github.io/GraphvizOnline :

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
Variable: transformed_tree
   Value: digraph "transformed_tree"
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

## Viewing the match factors values
<!-- example factors -->
When expression ranker is used, it is possible to expose the values of the calculated factors using [PACKEDFACTORS()](../../Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29).

The function returns:

* the values of document level factors (like bm25, field_mask, doc_word_count)
* list with each field that returned a hit (like lcs, hit_count, word_count, sum_idf, min_hit_pos etc.)
* list with each keyword from the query and their tf and idf values


The values can be used to understand why certain documents get scored lower or higher in a search or to improve the existing ranking expression.

<!-- intro -->
Example:

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
