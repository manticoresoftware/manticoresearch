# 搜索分析

## 查询的解释方法

考虑以下复杂查询示例：
```sql
"hello world" @title "example program"~5 @body python -(php|perl) @* code
```
这个搜索的完整含义是：

* 在文档的任何字段中定位'hello'和'world'连续出现；
* 此外，同一文档还必须包含‘example’和‘program’两个词在标题字段中，且两者之间最多间隔不到5个词；（例如，“example PHP program”会匹配，但是“example script to introduce outside data into the correct context for your program”则不会，因为这两者之间间隔了5个或更多词）
* 此外，同一文档中必须在正文字段中包含‘python’一词，同时排除‘php’或‘perl’；
* 最后，同一文档必须在任一字段中包含‘code’一词。

OR 运算符优先级高于 AND，所以“looking for cat | dog | mouse”表示“looking for (cat | dog | mouse)”而不是“(looking for cat) | dog | mouse”。

为了理解查询如何执行，Manticore Search 提供查询分析工具以检查通过查询表达式生成的查询树。

<!-- example profiling -->

## 在 SQL 中分析查询树

要通过 SQL 语句启用全文查询分析，必须在执行所需查询之前激活它：

```sql
SET profiling =1;
SELECT * FROM test WHERE MATCH('@title abc* @body hey');
```

要查看查询树，请在运行查询后立即执行 `SHOW PLAN` 命令：

```sql
SHOW PLAN;
```

此命令将返回已执行查询的结构。请记住，3个语句 —— SET profiling、查询和 SHOW —— 必须在同一会话中执行。


## 在 HTTP JSON 中分析查询

使用 HTTP JSON 协议时，我们只需启用 `"profile":true` 以在响应中获取全文查询树结构。

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
响应将包括一个包含 `query` 成员的 `profile` 对象。

`query` 属性包含转换后的全文查询树。每个节点包含：

* `type`: 节点类型，可以是 AND、OR、PHRASE、KEYWORD 等。
* `description`: 以字符串表示的此节点的查询子树（在 `SHOW PLAN` 格式中）
* `children`: 任何子节点（如果存在）
* `max_field_pos`: 字段中的最大位置

 一个关键词节点还将包括：

* `word`: 转换后的关键词。
* `querypos`: 此关键词在查询中的位置。
* `excluded`: 从查询中排除的关键词。
* `expanded`: 通过前缀扩展添加的关键词。
* `field_start`: 关键词必须出现在字段的开始。
* `field_end`: 关键词必须出现在字段的结束。
* `boost`: 关键词的IDF将乘以这个值。


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
    profile: {query={type=AND, description=AND( AND(KEYWORD(i, querypos=1)),  AND(KEYWORD(me, querypos=2))), children=[{type=AND, description=AND(KEYWORD(i, querypos=1)), children=[{type=KEYWORD, word=i, querypos=1}]}, {type=AND, description=AND(KEYWORD(me, querypos=2)), children=[{type=KEYWORD, word=me, querypos=2}]}]}
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
    profile: {query={type=AND, description=AND( AND(KEYWORD(i, querypos=1)),  AND(KEYWORD(me, querypos=2))), children=[{type=AND, description=AND(KEYWORD(i, querypos=1)), children=[{type=KEYWORD, word=i, querypos=1}]}, {type=AND, description=AND(KEYWORD(me, querypos=2)), children=[{type=KEYWORD, word=me, querypos=2}]}]}
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

在某些情况下，由于扩展和其他转换，评估后的查询树可能与原始查询树有显著不同。

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
      "description":"AND( OR( OR( AND(fields=(title), KEYWORD(wayne, querypos=1, expanded)),  OR( AND(fields=(title), KEYWORD(ways, querypos=1, expanded)),  AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded)))),  AND(fields=(title), KEYWORD(way, querypos=1, expanded)),  OR(fields=(title), KEYWORD(way*, querypos=1, expanded))),  AND(fields=(content), KEYWORD(hey, querypos=2))",
      "children":
      [
        {
          "type":"OR",
          "description":"OR( OR( AND(fields=(title), KEYWORD(wayne, querypos=1, expanded)),  OR( AND(fields=(title), KEYWORD(ways, querypos=1, expanded)),  AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded)))),  AND(fields=(title), KEYWORD(way, querypos=1, expanded)),  OR(fields=(title), KEYWORD(way*, querypos=1, expanded))",
          "children":
          [
            {
               "type":"OR",
               "description":"或( 与(字段=(标题), 关键字(wayne, querypos=1, expanded)),  或( 与(字段=(标题), 关键字(ways, querypos=1, expanded)),  与(字段=(标题), 关键字(wayyy, querypos=1, expanded))))",
               "children":
               [
                 {
                   "type":"与",
                   "description":"与(字段=(标题), 关键字(wayne, querypos=1, expanded))",
                   "fields":["标题"],
                   "max_field_pos":0,
                   "children":
                   [
                     {
                       "type":"关键字",
                       "word":"wayne",
                       "querypos":1,
                       "expanded":true
                     }
                   ]
                 },
                 {
                   "type":"或",
                   "description":"或( 与(字段=(标题), 关键字(ways, querypos=1, expanded)),  与(字段=(标题), 关键字(wayyy, querypos=1, expanded)))",
                   "children":
                   [
                     {
                       "type":"与",
                       "description":"与(字段=(标题), 关键字(ways, querypos=1, expanded))",
                       "fields":["标题"],
                       "max_field_pos":0,
                       "children":
                       [
                         {
                           "type":"关键字",
                           "word":"ways",
                           "querypos":1,
                           "expanded":true
                         }
                       ]
                     },
                     {
                       "type":"与",
                       "description":"与(字段=(标题), 关键字(wayyy, querypos=1, expanded))",
                       "fields":["标题"],
                       "max_field_pos":0,
                       "children":
                       [
                         {
                           "type":"关键字",
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
              "type":"与",
              "description":"与(字段=(标题), 关键字(way, querypos=1, expanded))",
              "fields":["标题"],
              "max_field_pos":0,
              "children":
              [
                 {
                    "type":"关键字",
                    "word":"way",
                    "querypos":1,
                    "expanded":true
                 }
              ]
            },
            {
              "type":"或",
              "description":"或(字段=(标题), 关键字(way*, querypos=1, expanded))",
              "fields":["标题"],
              "max_field_pos":0,
              "children":
              [
                {
                  "type":"关键字",
                  "word":"way*",
                  "querypos":1,
                  "expanded":true
                }
              ]
            }
          ]
        },
        {
          "type":"与",
          "description":"与(字段=(内容), 关键字(hey, querypos=2))",
          "fields":["内容"],
          "max_field_pos":0,
          "children":
          [
            {
              "type":"关键字",
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
            [type] => 与
            [description] => 与( 或( 或( 与(字段=(标题), 关键字(wayne, querypos=1, expanded)),  或( 与(字段=(标题), 关键字(ways, querypos=1, expanded)),  与(字段=(标题), 关键字(wayyy, querypos=1, expanded)))),  与(字段=(标题), 关键字(way, querypos=1, expanded)),  或(字段=(标题), 关键字(way*, querypos=1, expanded))),  与(字段=(内容), 关键字(hey, querypos=2)))
            [children] => Array
                (
                    [0] => Array
                        (
                            [type] => 或
                            [description] => 或( 或( 与(字段=(标题), 关键字(wayne, querypos=1, expanded)),  或( 与(字段=(标题), 关键字(ways, querypos=1, expanded)),  与(字段=(标题), 关键字(wayyy, querypos=1, expanded)))),  与(字段=(标题), 关键字(way, querypos=1, expanded)),  或(字段=(标题), 关键字(way*, querypos=1, expanded)))
                            [children] => Array
                                (
                                    [0] => Array
                                        (
                                            [type] => 或
                                            [description] => 或( 与(字段=(标题), 关键字(wayne, querypos=1, expanded)),  或( 与(字段=(标题), 关键字(ways, querypos=1, expanded)),  与(字段=(标题), 关键字(wayyy, querypos=1, expanded))))
                                            [children] => Array
                                                (
                                                    [0] => Array
                                                        (
                                                            [type] => 与
                                                            [description] => 与(字段=(标题), 关键字(wayne, querypos=1, expanded))
                                                            [fields] => 数组
                                                                (
                                                                    [0] => title
                                                                )
                                                            [max_field_pos] => 0
                                                            [children] => 数组
                                                                (
                                                                    [0] => 数组
                                                                        (
                                                                            [type] => 关键词
                                                                            [word] => wayne
                                                                            [querypos] => 1
                                                                            [expanded] => 1
                                                                        )
                                                                )
                                                        )
                                                    [1] => 数组
                                                        (
                                                            [type] => 或
                                                            [description] => OR( AND(fields=(title), KEYWORD(ways, querypos=1, expanded)),  AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded)))
                                                            [children] => 数组
                                                                (
                                                                    [0] => 数组
                                                                        (
                                                                            [type] => AND
                                                                            [description] => AND(fields=(title), KEYWORD(ways, querypos=1, expanded))
                                                                            [fields] => 数组
                                                                                (
                                                                                    [0] => title
                                                                                )

                                                                            [max_field_pos] => 0
                                                                            [children] => 数组
                                                                                (
                                                                                    [0] => 数组
                                                                                        (
                                                                                            [type] => 关键词
                                                                                            [word] => ways
                                                                                            [querypos] => 1
                                                                                            [expanded] => 1
                                                                                        )
                                                                                )
                                                                        )
                                                                    [1] => 数组
                                                                        (
                                                                            [type] => AND
                                                                            [description] => AND(fields=(title), KEYWORD(wayyy, querypos=1, expanded))
                                                                            [fields] => 数组
                                                                                (
                                                                                    [0] => title
                                                                                )
                                                                            [max_field_pos] => 0
                                                                            [children] => 数组
                                                                                (
                                                                                    [0] => 数组
                                                                                        (
                                                                                            [type] => 关键词
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
                                    [1] => 数组
                                        (
                                            [type] => 和
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


## 在不运行查询的情况下进行分析

<!-- Example Explain_query -->
SQL语句 `EXPLAIN QUERY` 能够显示给定全文查询的执行树，而无需在表上执行实际搜索查询。



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
`EXPLAIN QUERY ... option format=dot` 允许以适合现有工具可视化的层次格式显示提供的全文查询的执行树，例如 https://dreampuf.github.io/GraphvizOnline：

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

## 查看匹配因子的值
<!-- example factors -->
使用表达式排名器时，可以通过 [PACKEDFACTORS()](../../Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29) 函数揭示计算因子的值。

该函数返回：

* 文档级因子的值（例如 bm25，field_mask，doc_word_count）
* 生成命中的每个字段的列表（包括 lcs，hit_count，word_count，sum_idf，min_hit_pos 等）
* 查询中每个关键字的 tf 和 idf 值列表


这些值可以用来理解为什么某些文档在搜索中得分较低或较高，或用于改进现有的排名表达式。

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
