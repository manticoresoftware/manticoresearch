# खोज प्रोफाइलिंग
## एक क्वेरी का व्याख्या कैसे किया जाता है
इस जटिल क्वेरी उदाहरण पर विचार करें:
```sql
"नमस्ते दुनिया" @title "उदाहरण कार्यक्रम"~5 @body पायथन -(पीएचपी|पर्ल) @* कोड
```
इस खोज का पूरा अर्थ है:
* किसी दस्तावेज़ के किसी भी फ़ील्ड में शब्द 'नमस्ते' और 'दुनिया' को निकटता से ढूंढना;
* अतिरिक्त रूप से, उसी दस्तावेज़ में शीर्षक फ़ील्ड में शब्द 'उदाहरण' और 'कार्यक्रम' भी होने चाहिए, उनके बीच 5 शब्दों तक, लेकिन शामिल नहीं; (उदाहरण के लिए, "उदाहरण पीएचपी कार्यक्रम" मेल खाएगा, लेकिन "उदाहरण स्क्रिप्ट बाहरी डेटा को आपके कार्यक्रम के लिए सही संदर्भ में पेश करने के लिए" मेल नहीं खाएगा, क्योंकि उन दोनों शब्दों के बीच 5 या उससे अधिक शब्द हैं)
* इसके अलावा, उसी दस्तावेज़ में शरीर फ़ील्ड में 'पायथन' शब्द होना चाहिए, जबकि 'पीएचपी' या 'पर्ल' को बाहर करना;
* अंततः, उसी दस्तावेज़ में किसी भी फ़ील्ड में 'कोड' शब्द होना चाहिए।
OR ऑपरेटर AND पर प्राथमिकता लेता है, इसलिए "बिल्ली | कुत्ता | चूहा" का मतलब है "बिल्ली के लिए खोज (बिल्ली | कुत्ता | चूहा)" न कि "(बिल्ली के लिए खोज) | कुत्ता | चूहा"।
एक क्वेरी कैसे निष्पादित की जाएगी, इसे समझने के लिए, Manticore Search क्वेरी प्रोफाइलिंग उपकरण प्रदान करता है ताकि एक क्वेरी अभिव्यक्ति द्वारा उत्पन्न क्वेरी वृक्ष का परीक्षण किया जा सके।
<!-- उदाहरण प्रोफाइलिंग -->
## SQL में क्वेरी वृक्ष का प्रोफाइलिंग
एक SQL कथन के साथ पूर्ण-पाठ क्वेरी प्रोफाइलिंग को सक्षम करने के लिए, आपको इच्छित क्वेरी को निष्पादित करने से पहले इसे सक्रिय करना होगा:
```sql
SET profiling =1;
SELECT * FROM test WHERE MATCH('@title abc* @body hey');
```
क्वेरी वृक्ष को देखने के लिए, क्वेरी चलाने के तुरंत बाद `SHOW PLAN` कमांड निष्पादित करें:
```sql
SHOW PLAN;
```
यह कमांड निष्पादित क्वेरी की संरचना लौटाएगा। ध्यान रखें कि 3 कथन - SET profiling, क्वेरी और SHOW - को एक ही सत्र के भीतर निष्पादित किया जाना चाहिए।
## HTTP JSON में क्वेरी का प्रोफाइलिंग
HTTP JSON प्रोटोकॉल का उपयोग करते समय हम केवल `"profile":true` सक्षम कर सकते हैं ताकि हमें प्रतिक्रिया में पूर्ण-पाठ क्वेरी वृक्ष की संरचना मिल सके।
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
प्रतिक्रिया में एक `profile` ऑब्जेक्ट शामिल होगा जिसमें एक `query` सदस्य होगा।
`query` प्रॉपर्टी परिवर्तित पूर्ण-पाठ क्वेरी वृक्ष को धारण करती है। प्रत्येक नोड में होता है:
* `type`: नोड का प्रकार, जो AND, OR, PHRASE, KEYWORD, आदि हो सकता है।
* `description`: इस नोड के लिए क्वेरी उपवृक्ष को एक स्ट्रिंग के रूप में दर्शाया गया है (को `SHOW PLAN` प्रारूप में)
* `children`: कोई भी चाइल्ड नोड्स, यदि मौजूद हों
* `max_field_pos`: एक फ़ील्ड के भीतर अधिकतम स्थिति
 एक कीवर्ड नोड में अतिरिक्त रूप से शामिल होगा:
* `word`: परिवर्तित कीवर्ड।
* `querypos`: क्वेरी में इस कीवर्ड की स्थिति।
* `excluded`: क्वेरी से बाहर रखा गया कीवर्ड।
* `expanded`: प्रीफिक्स विस्तार द्वारा जोड़ा गया कीवर्ड।
* `field_start`: कीवर्ड को फ़ील्ड की शुरुआत में होना चाहिए।
* `field_end`: कीवर्ड को फ़ील्ड के अंत में होना चाहिए।
* `boost`: कीवर्ड का IDF इस मान से गुणा किया जाएगा।
<!-- परिचय -->
##### SQL:
<!-- अनुरोध SQL -->
```sql
SET profiling=1;
SELECT * FROM test WHERE MATCH('@title abc* @body hey');
SHOW PLAN \G
```
<!-- प्रतिक्रिया SQL -->
```sql
*************************** 1\. row ***************************
Variable: transformed_tree
   Value: AND(
  OR(fields=(title), KEYWORD(abcx, querypos=1, expanded), KEYWORD(abcm, querypos=1, expanded)),
  AND(fields=(body), KEYWORD(hey, querypos=2)))
1 row in set (0.00 sec)
```
<!-- अनुरोध JSON -->
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
<!-- प्रतिक्रिया JSON -->
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
<!-- अनुरोध PHP -->
```php
$result = $index->search('i me')->setSource(['excludes'=>['*']])->setLimit(1)->profile()->get();
print_r($result->getProfile());
```
<!-- प्रतिक्रिया PHP -->
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
पाइथन
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
जावास्क्रिप्ट
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
जावा
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
जाने दो
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

कुछ उदाहरणों में, मूल्यांकन किया गया क्वेरी ट्री मूल से काफी भिन्न हो सकता है जो विस्तार और अन्य परिवर्तनों के कारण होता है।

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
जावा
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
गो
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


## प्रोफाइलिंग बिना क्वेरी चलाए

<!-- Example Explain_query -->
SQL कथन `EXPLAIN QUERY` किसी दिए गए पूर्ण-टेक्स्ट क्वेरी के लिए कार्यान्वयन वृक्ष को प्रदर्शित करने की अनुमति देता है बिना तालिका पर वास्तविक खोज क्वेरी किए।



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
   Value: और(
      या(
            और(fields=(title), कीवर्ड(run, querypos=1, morphed)),
            और(fields=(title), कीवर्ड(running, querypos=1, morphed))))
  और(fields=(body), कीवर्ड(dog, querypos=2, morphed)))
```
<!-- end -->

<!-- Example Explain_query_dot -->
`EXPLAIN QUERY ... option format=dot` एक प्रदान की गई पूर्ण-पाठ क्वेरी के निष्पादन वृक्ष को दृश्यकरण के लिए उपयुक्त हाइरार्किकल प्रारूप में प्रदर्शित करने की अनुमति देता है, जैसे कि https://dreampuf.github.io/GraphvizOnline:

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

0 [shape=record,style=filled,bgcolor="lightgrey" label="और"]
0 -> 1
1 [shape=record,style=filled,bgcolor="lightgrey" label="और"]
1 -> 2
2 [shape=record label="i | { querypos=1 }"]
0 -> 3
3 [shape=record,style=filled,bgcolor="lightgrey" label="और"]
3 -> 4
4 [shape=record label="me | { querypos=2 }"]
}
```
<!-- end -->

## मेल खाने वाले कारकों के मान देखना
<!-- example factors -->
जब एक अभिव्यक्ति रेंकर का उपयोग किया जाता है, तो रेंडर किए गए कारकों के मानों को [PACKEDFACTORS()](../../Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29) फ़ंक्शन के साथ प्रकट करना संभव है।

फ़ंक्शन लौटाता है:

* दस्तावेज़-स्तरीय कारकों के मान (जैसे bm25, field_mask, doc_word_count)
* प्रत्येक फ़ील्ड की एक सूची जिसने हिट उत्पन्न किया (जिसमें lcs, hit_count, word_count, sum_idf, min_hit_pos, आदि शामिल हैं)
* क्वेरी से प्रत्येक कीवर्ड की एक सूची उनके tf और idf मानों के साथ


ये मान समझने के लिए उपयोग किए जा सकते हैं कि कुछ दस्तावेज़ों को खोज में कम या अधिक स्कोर क्यों मिलते हैं या मौजूदा रैंकिंग अभिव्यक्ति को सुधारने के लिए।

<!-- intro -->
उदाहरण:

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





























