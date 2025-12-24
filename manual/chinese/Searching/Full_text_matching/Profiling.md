# 搜索性能分析

## 查询如何被解析

考虑这个复杂的查询示例：
```sql
"hello world" @title "example program"~5 @body python -(php|perl) @* code
```
此搜索的完整含义是：

* 在文档的任何字段中定位相邻出现的单词“hello”和“world”；
* 此外，同一文档的标题字段中必须包含单词“example”和“program”，且它们之间最多（但不包括）有5个单词；（例如，“example PHP program”会匹配，但“example script to introduce outside data into the correct context for your program”则不会，因为这两个术语之间有5个或更多单词）
* 再者，同一文档的正文字段中必须包含单词“python”，同时排除“php”或“perl”；
* 最后，同一文档必须在任何字段中包含单词“code”。

OR运算符的优先级高于AND，因此“looking for cat | dog | mouse”意味着“looking for (cat | dog | mouse)”，而不是“(looking for cat) | dog | mouse”。

为了理解查询将如何执行，Manticore Search提供了查询性能分析工具来检查查询表达式生成的查询树。

<!-- example profiling -->

## 在SQL中分析查询树

要通过SQL语句启用全文查询性能分析，必须在执行目标查询前激活它：

```sql
SET profiling =1;
SELECT * FROM test WHERE MATCH('@title abc* @body hey');
```

要查看查询树，请在运行查询后立即执行`SHOW PLAN`命令：

```sql
SHOW PLAN;
```

此命令将返回已执行查询的结构。请注意，这3条语句——SET profiling、查询本身和SHOW——必须在同一会话中执行。


## 在HTTP JSON中分析查询

使用HTTP JSON协议时，我们只需启用`"profile":true`即可在响应中获取全文查询树结构。

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
响应将包含一个`profile`对象，其中有一个`query`成员。

`query`属性保存转换后的全文查询树。每个节点包含：

* `type`：节点类型，可以是AND、OR、PHRASE、KEYWORD等。
* `description`：此节点的查询子树，以字符串形式表示（采用`SHOW PLAN`格式）
* `children`：存在的任何子节点
* `max_field_pos`：字段内的最大位置

关键字节点还将额外包括：

* `word`：转换后的关键字。
* `querypos`：此关键字在查询中的位置。
* `excluded`：从查询中排除的关键字。
* `expanded`：通过前缀扩展添加的关键字。
* `field_start`：关键字必须出现在字段的开头。
* `field_end`：关键字必须出现在字段的末尾。
* `boost`：此关键字的IDF将乘以该值。


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
searchApi.search({"table":"forum","query":{"query_string":"i me"},"_source":{"excludes":["*"]},"limit":1,"profile":True})
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
res = await searchApi.search({"table":"forum","query":{"query_string":"i me"},"_source":{"excludes":["*"]},"limit":1,"profile":true});
```
<!-- response javascript -->
``` javascript
{"hits": {"hits": [{"_id": 100, "_score": 2500, "_source": {}}],
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
Rust
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

在某些情况下，由于扩展和其他转换，评估后的查询树可能与原始查询树有显著差异。

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


## 不运行查询的性能分析

<!-- Example Explain_query -->
SQL语句`EXPLAIN QUERY`允许显示给定全文查询的执行树，而无需对表执行实际的搜索查询。



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
`EXPLAIN QUERY ... option format=dot`允许以分层格式显示所提供全文查询的执行树，适合通过现有工具（如https://dreampuf.github.io/GraphvizOnline）进行可视化：

![EXPLAIN QUERY graphviz示例](graphviz.png)

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

## 查看匹配因子值
<!-- example factors -->
使用表达式排序器时，可以通过[PACKEDFACTORS()](../../Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29)函数显示计算出的因子值。

该函数返回：

* 文档级别因素的值（如bm25、field_mask、doc_word_count）
* 每个生成命中结果的字段列表（包括lcs、hit_count、word_count、sum_idf、min_hit_pos等）
* 查询中每个关键词及其tf和idf值的列表


这些值可以用于理解某些文档在搜索中为何获得较低或较高的分数，或者用于细化现有的排名表达式。

<!-- intro -->
示例：

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

