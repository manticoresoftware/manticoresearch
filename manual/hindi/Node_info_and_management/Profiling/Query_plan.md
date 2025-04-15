# प्रश्न योजना

<!-- उदाहरण SHOW PLAN -->

`SHOW PLAN` SQL बयानी और `"plan": N` JSON इंटरफ़ेस विकल्प प्रश्न निष्पादन योजना प्रदर्शित करते हैं। योजना वास्तविक निष्पादन के दौरान उत्पन्न और संग्रहीत की जाती है, इसलिए SQL के मामले में, वर्तमान सत्र में उस बयान को चलाने से पहले प्रोफाइलिंग सक्षम करना आवश्यक है **पहले**। यह `SET profiling=1` बयान के साथ किया जा सकता है।

SQL मोड में दो आइटम लौटाए जाते हैं:
* `transformed_tree`, जो पूर्ण-टेक्स्ट प्रश्न विघटन को प्रदर्शित करता है।
* `enabled_indexes`, जो प्रभावी माध्यमिक सूचियों के बारे में जानकारी प्रदान करता है।

JSON प्रश्न में प्रश्न निष्पादन योजना देखने के लिए, प्रश्न में `"plan": N` जोड़ें। परिणाम परिणाम सेट में `plan` संपत्ति के रूप में दिखाई देगा। `N` निम्नलिखित में से एक हो सकता है:
* 1 - केवल जड़ नोड की पाठ योजना को प्रदर्शित करता है, जो `SHOW PLAN` SQL बायन में लौटाई गई योजना के समान है। यह सबसे संकुचित रूप है।
* 2 - केवल JSON ऑब्जेक्ट योजना प्रदर्शित करता है, जो संसाधन के लिए उपयोगी है।
* 3 - हर नोड का पाठ विवरण के साथ JSON ऑब्जेक्ट प्रदर्शित करता है। ध्यान दें कि बाल नोड के लिए विवरण भी मौजूद है और माता-पिता के विवरण का एक भाग दोहराता है, जिससे समग्र प्रतिनिधित्व काफी बड़ा हो जाता है।


<!-- परिचय -->
##### SQL:
<!-- अनुरोध SQL -->

```sql
सेट प्रोफाइलिंग=1;

चयन * फरोम hn_small जहाँ मैच('dog|cat') लिमिट 0;

दिखाओ योजना;
```

<!-- प्रतिक्रिया SQL -->

```sql
*************************** 1. पंक्ति ***************************
चर: transformed_tree
   मान: OR(
  AND(KEYWORD(dog, querypos=1)),
  AND(KEYWORD(cat, querypos=2)))
*************************** 2. पंक्ति ***************************
चर: enabled_indexes
   मान:
2 पंक्तियाँ सेट में (0.00 सेकंड)
```

<!-- परिचय -->
##### JSON:

<!-- अनुरोध JSON -->

```json
POST /search
{
  "table": "hn_small",
  "query": {"query_string": "dog|cat"},
  "_source": { "excludes":["*"] },
  "limit": 0,
  "plan": 3
}
```

<!-- प्रतिक्रिया JSON -->
```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 4453,
    "total_relation": "eq",
    "hits": []
  },
  "plan": {
    "query": {
      "type": "OR",
      "description": "OR( AND(KEYWORD(dog, querypos=1)),  AND(KEYWORD(cat, querypos=2)))",
      "children": [
        {
          "type": "AND",
          "description": "AND(KEYWORD(dog, querypos=1))",
          "children": [
            {
              "type": "KEYWORD",
              "word": "dog",
              "querypos": 1
            }
          ]
        },
        {
          "type": "AND",
          "description": "AND(KEYWORD(cat, querypos=2))",
          "children": [
            {
              "type": "KEYWORD",
              "word": "cat",
              "querypos": 2
            }
          ]
        }
      ]
    }
  }
}
```

<!-- समाप्त -->

<!-- उदाहरण SHOW PLAN EXPANSION -->

कुछ मामलों में, मूल्यांकित प्रश्न वृक्ष असली वृक्ष से काफी भिन्न हो सकता है जो विस्तार और अन्य रूपांतरणों के कारण हैं।

<!-- परिचय -->
##### SQL:
<!-- अनुरोध SQL -->

```sql
SET profiling=1;

SELECT id FROM forum WHERE MATCH('@title way* @content hey') LIMIT 1;

SHOW PLAN;
```

<!-- प्रतिक्रिया SQL -->

```sql
प्रश्न ठीक है, 0 पंक्तियाँ प्रभावित (0.00 सेकंड)

+--------+
| id     |
+--------+
| 711651 |
+--------+
1 पंक्ति सेट में (0.04 सेकंड)

+------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| चर               | मान                                                                                                                                                                                                                                                                                                                                                                                                                   |
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
1 पंक्ति सेट में (0.00 सेकंड)
```

<!-- परिचय -->
##### JSON पूर्ण प्रारूप:

<!-- अनुरोध JSON -->

```JSON
POST /search
{
  "table": "forum",
  "query": {"query_string": "@title way* @content hey"},
  "_source": { "excludes":["*"] },
  "limit": 1,
  "plan": 3
}
```

<!-- प्रतिक्रिया JSON -->
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
  "plan":
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

##### JSON ऑब्जेक्ट प्रारूप:

<!-- request JSON -->

```JSON
POST /search
{
  "table": "forum",
  "query": {"query_string": "@title way* @content hey"},
  "_source": { "excludes":["*"] },
  "limit": 1,
  "plan": 2
}
```

<!-- response JSON -->

```JSON
{
  "took": 33,
  "timed_out": false,
  "hits": {
    "total": 105,
    "hits": [
      {
        "_id": 711651,
        "_score": 2539,
        "_source": {}
      }
    ]
  },
  "plan": {
    "query": {
      "type": "AND",
      "children": [
        {
          "type": "OR",
          "children": [
            {
              "type": "OR",
              "children": [
                {
                  "type": "AND",
                  "fields": [
                    "title"
                  ],
                  "max_field_pos": 0,
                  "children": [
                    {
                      "type": "KEYWORD",
                      "शब्द": "wayne",
                      "querypos": 1,
                      "expanded": true
                    }
                  ]
                },
                {
                  "type": "या",
                  "children": [
                    {
                      "type": "और",
                      "fields": [
                        "शीर्षक"
                      ],
                      "max_field_pos": 0,
                      "children": [
                        {
                          "type": "कीवर्ड",
                          "शब्द": "ways",
                          "querypos": 1,
                          "expanded": true
                        }
                      ]
                    },
                    {
                      "type": "और",
                      "fields": [
                        "शीर्षक"
                      ],
                      "max_field_pos": 0,
                      "children": [
                        {
                          "type": "कीवर्ड",
                          "शब्द": "wayyy",
                          "querypos": 1,
                          "expanded": true
                        }
                      ]
                    }
                  ]
                }
              ]
            },
            {
              "type": "और",
              "fields": [
                "शीर्षक"
              ],
              "max_field_pos": 0,
              "children": [
                {
                  "type": "कीवर्ड",
                  "शब्द": "way",
                  "querypos": 1,
                  "expanded": true
                }
              ]
            },
            {
              "type": "या",
              "fields": [
                "शीर्षक"
              ],
              "max_field_pos": 0,
              "children": [
                {
                  "type": "कीवर्ड",
                  "शब्द": "way*",
                  "querypos": 1,
                  "expanded": true
                }
              ]
            }
          ]
        },
        {
          "type": "और",
          "fields": [
            "सामग्री"
          ],
          "max_field_pos": 0,
          "children": [
            {
              "type": "कीवर्ड",
              "शब्द": "hey",
              "querypos": 2
            }
          ]
        }
      ]
    }
  }
}
```

<!-- intro -->

##### JSON संक्षिप्त प्रारूप:

<!-- request JSON -->

```JSON
POST /search
{
  "table": "forum",
  "query": {"query_string": "@title way* @content hey"},
  "_source": { "excludes":["*"] },
  "limit": 1,
  "plan": 1
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
  "plan":
  {
    "query":
    {
      "description":"और( या( या( और(fields=(title), कीवर्ड(wayne, querypos=1, expanded)),  या( और(fields=(title), कीवर्ड(ways, querypos=1, expanded)),  और(fields=(title), कीवर्ड(wayyy, querypos=1, expanded)))),  और(fields=(title), कीवर्ड(way, querypos=1, expanded)),  या(fields=(title), कीवर्ड(way*, querypos=1, expanded))),  और(fields=(content), कीवर्ड(hey, querypos=2)))"
    }
  }
}
```

<!-- end -->

See also [EXPLAIN QUERY](../../Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query). It displays the execution tree of a full-text query **without actually executing the query**. Note that when using `SHOW PLAN` after a query to a real-time table, the result will be based on a random disk/RAM chunk. Therefore, if you have recently modified the table's tokenization settings, or if the chunks vary significantly in terms of dictionaries, etc., you might not get the result you are expecting. Take this into account and consider using `EXPLAIN QUERY` as well.

## JSON परिणाम सेट नोट्स

`query` संपत्ति में परिवर्तित पूर्ण-पाठ प्रश्न वृक्ष होता है। प्रत्येक नोड में शामिल हैं:

* `type`: नोड प्रकार। हो सकता है `AND`, `OR`, `PHRASE`, `KEYWORD`, आदि।
* `description`: इस नोड के लिए प्रश्न उपवृक्ष जो एक स्ट्रिंग के रूप में दर्शाया गया है (में `SHOW PLAN` प्रारूप)।
* `children`: बच्चा नोड, यदि कोई हो।
* `max_field_pos`: एक फ़ील्ड के भीतर अधिकतम स्थिति।
* `शब्द`: परिवर्तित कीवर्ड। केवल कीवर्ड नोड।
* `querypos`: इस कीवर्ड की स्थिति एक प्रश्न में। केवल कीवर्ड नोड।
* `excluded`: प्रश्न से बाहर किया गया कीवर्ड। केवल कीवर्ड नोड।
* `expanded`: कीवर्ड जो उपसर्ग विस्तार द्वारा जोड़ा गया है। केवल कीवर्ड नोड।
* `field_start`: कीवर्ड को फ़ील्ड की शुरुआत में होना चाहिए। केवल कीवर्ड नोड।
* `field_end`: कीवर्ड को फ़ील्ड के अंत में होना चाहिए। केवल कीवर्ड नोड।
* `boost`: कीवर्ड IDF को इसके द्वारा गुणा किया जाएगा। केवल कीवर्ड नोड।

## डॉट प्रारूप के लिए SHOW PLAN
`SHOW PLAN format=dot` पूर्ण-पाठ प्रश्न निष्करण वृक्ष को एक श्रेणीबद्ध प्रारूप में लौटाने की अनुमति देता है जो मौजूदा टूल द्वारा दृश्यकरण के लिए उपयुक्त है, जैसे कि https://dreampuf.github.io/GraphvizOnline:

```sql
MySQL [(कोई नहीं)]> show plan option format=dot\G
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

![SHOW PLAN graphviz उदाहरण](graphviz.png)
<!-- proofread -->
