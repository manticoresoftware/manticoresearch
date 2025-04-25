# समूह खोज परिणाम

<!-- example general -->
समूह खोज परिणाम प्राप्त करने में अक्सर सहायक होते हैं, जैसे प्रति-समूह मेल की गणना या अन्य संचित आँकड़े। उदाहरण के लिए, यह प्रत्येक महीने के लिए मेल खाते ब्लॉग पोस्ट की संख्या को दर्शाने वाले ग्राफ बनाने के लिए उपयोगी है या साइट या लेखक द्वारा फोरम पोस्ट के लिए वेब खोज परिणामों को समूहित करने के लिए आदि।

Manticore एकल या कई स्तंभों और गणना की गई अभिव्यक्तियों द्वारा खोज परिणामों को समूहित करने का समर्थन करता है। परिणाम निम्नलिखित हो सकते हैं:

* समूह के भीतर क्रमबद्ध किए जा सकते हैं
* प्रति समूह एक से अधिक पंक्तियाँ लौटाईं जा सकती हैं
* समूहों को फ़िल्टर किया जा सकता है
* समूहों को क्रमबद्ध किया जा सकता है
* [संघटन कार्यों](../Searching/Grouping.md#Aggregation-functions) का उपयोग करके संघटित किया जा सकता है

<!-- intro -->
सामान्य सिंटैक्स इस प्रकार है:

<!-- request SQL -->
सामान्य सिंटैक्स
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
JSON क्वेरी प्रारूप इस समय एक बुनियादी समूहण का समर्थन करता है जो संचित मान और उनकी गिनती("*") को पुनर्प्राप्त कर सकता है।

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

मानक क्वेरी आउटपुट समूहित किए बिना परिणाम सेट लौटाता है, जिसे `limit` (या `size`) का उपयोग करके छिपाया जा सकता है।
समूह के परिणाम सेट के आकार के लिए `size` सेट करना आवश्यक है।

<!-- end -->

<!-- example group1 -->
### केवल समूहित करना
समूहित करना काफी सरल है - बस अपने `SELECT` क्वेरी के अंत में "GROUP BY smth" जोड़ें। कुछ भी हो सकता है:

* तालिका से कोई भी गैर-पूर्ण-टेक्स्ट क्षेत्र: पूर्णांक, फ़्लोट, स्ट्रिंग, MVA (बहु-मूल्य विशेषता)
* या, यदि आपने `SELECT` सूची में उपनाम का उपयोग किया है, तो आप इसे GROUP BY भी कर सकते हैं

आप `SELECT` सूची में किसी भी [संघटन कार्यों](../Searching/Grouping.md#Aggregation-functions) को छोड़ सकते हैं और यह अभी भी काम करेगा:

<!-- intro -->
##### उदाहरण:

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
हालांकि, अधिकांश मामलों में, आप प्रत्येक समूह के लिए कुछ संघटित डेटा प्राप्त करना चाहेंगे, जैसे:

* `COUNT(*)` बस प्रत्येक समूह में तत्वों की संख्या प्राप्त करने के लिए
* या `AVG(field)` समूह के भीतर क्षेत्र का औसत मूल्य निकालने के लिए

HTTP JSON अनुरोधों के लिए, मुख्य क्वेरी स्तर पर `limit=0` के साथ एक ही `aggs` बकेट का उपयोग SQL क्वेरी के समान काम करता है जिसमें `GROUP BY` और `COUNT(*)` होता है, जो समान व्यवहार और प्रदर्शन प्रदान करता है।

<!-- intro -->
##### उदाहरण:

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
##### Sorting groups
By default, groups are not sorted, and the next thing you typically want to do is order them by something, like the field you're grouping by:

<!-- intro -->
##### Example:

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
Alternatively, you can sort by the aggregation:

* by `count(*)` to display groups with the most elements first
* by `avg(rental_rate)` to show the highest-rated movies first. Note that in the example, it's done via an alias: `avg(rental_rate)` is first mapped to `avg` in the `SELECT` list, and then we simply do `ORDER BY avg`


<!-- intro -->
##### Example:

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
##### GROUP BY multiple fields at once
कुछ मामलों में, आप एकल क्षेत्र के लिए ही नहीं, बल्कि एक समय में कई क्षेत्रों के अनुसार समूहबद्ध करना चाह सकते हैं, जैसे कि किसी फिल्म की श्रेणी और वर्ष:

<!-- intro -->
##### उदाहरण:

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
##### मुझे N पंक्तियाँ दें
कभी-कभी यह देखना उपयोगी होता है कि एक समूह में केवल एक तत्व नहीं, बल्कि कई हैं। इसे `GROUP N BY` की मदद से आसानी से प्राप्त किया जा सकता है। उदाहरण के लिए, निम्नलिखित मामले में, हम प्रत्येक वर्ष के लिए केवल एक के बजाय दो फिल्में प्राप्त करते हैं, जो सरल `GROUP BY release_year` ने लौटाई होतीं।

<!-- intro -->
##### उदाहरण:

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
##### एक समूह के भीतर क्रमबद्ध करना
एक अन्य महत्वपूर्ण विश्लेषणात्मक आवश्यकता यह है कि समूह के भीतर तत्वों को क्रमबद्ध किया जाए। इसे प्राप्त करने के लिए, `WITHIN GROUP ORDER BY ... {ASC|DESC}` क्लॉज़ का उपयोग करें। उदाहरण के लिए, चलिए प्रत्येक वर्ष के लिए सबसे अधिक रेटेड फिल्म प्राप्त करते हैं। ध्यान दें कि यह केवल `ORDER BY` के साथ समानांतर में काम करता है:

* `WITHIN GROUP ORDER BY` परिणामों को **एक समूह के भीतर** क्रमबद्ध करता है
* जबकि केवल `GROUP BY` **समूहों को स्वयं क्रमबद्ध करता है**

ये दोनों पूरी तरह स्वतंत्र रूप से कार्य करते हैं।


<!-- intro -->
##### उदाहरण:

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
##### समूहों को फ़िल्टर करना
`HAVING expression` समूहों को फ़िल्टर करने के लिए एक सहायक क्लॉज़ है। जबकि `WHERE` समूह बनाने से पहले लागू होता है, `HAVING` समूहों के साथ कार्य करता है। उदाहरण के लिए, चलिए केवल उन वर्षों को रखते हैं जब उस वर्ष की फिल्मों की औसत किराया दर 3 से अधिक थी। हमें केवल चार वर्ष मिलते हैं:

<!-- intro -->
##### उदाहरण:

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

ध्यान दें कि `HAVING` `total_found` पर [search query meta info](../Node_info_and_management/SHOW_META.md#SHOW-META) को प्रभावित नहीं करता है।

<!-- example group7 -->
##### GROUPBY()
`GROUPBY()` एक ऐसा फ़ंक्शन है जो वर्तमान समूह की कुंजी लौटाता है। यह कई मामलों में उपयोगी है, विशेषकर जब आप [MVA के द्वारा GROUP BY करते हैं](../Searching/Grouping.md#Grouping-by-MVA-%28multi-value-attributes%29) या [JSON मान](../Searching/Grouping.md#Grouping-by-a-JSON-node) के द्वारा करते हैं।

इसका उपयोग `HAVING` में भी किया जा सकता है, उदाहरण के लिए, केवल 2000 और 2002 के वर्षों को बनाए रखने के लिए।

ध्यान दें कि जब आप एक बार में कई फ़ील्ड्स पर GROUP BY करते हैं, तो `GROUPBY()` का उपयोग करने की सिफारिश नहीं की जाती है। यह अभी भी काम करेगा, लेकिन चूंकि इस मामले में समूह की कुंजी फ़ील्ड मानों का यौगिक है, यह आपके अपेक्षित तरीके से नहीं दिखाई दे सकता है।

<!-- intro -->
##### उदाहरण:

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
##### MVA (multi-value attributes) द्वारा समूह बनाना
Manticore [MVA](../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) द्वारा समूह बनाने का समर्थन करता है। यह दिखाने के लिए कि यह कैसे काम करता है, चलो "shoes" नामक एक तालिका बनाते हैं जिसमें MVA "sizes" है और इसमें कुछ दस्तावेज़ जोड़ते हैं:
```sql
create table shoes(title text, sizes multi);
insert into shoes values(0,'nike',(40,41,42)),(0,'adidas',(41,43)),(0,'reebook',(42,43));
```
तो हमारे पास है:
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
अगर हम अब "sizes" पर GROUP BY करते हैं, तो यह हमारे सभी बहु-मूल्य गुणों को संसाधित करेगा और प्रत्येक के लिए एक एकत्रीकरण लौटाएगा, इस मामले में केवल गिनती:

<!-- intro -->
##### उदाहरण:

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
##### एक JSON नोड द्वारा समूहित करना
यदि आपके पास [JSON](../Creating_a_table/Data_types.md#JSON) प्रकार का एक फ़ील्ड है, तो आप इससे किसी भी नोड द्वारा समूहित कर सकते हैं। इसे प्रदर्शित करने के लिए, आइए हम एक टेबल "उत्पाद" बनाते हैं जिसमें कुछ दस्तावेज़ हैं, प्रत्येक में "मेटा" JSON फ़ील्ड में एक रंग होता है:
```sql
create table products(title text, meta json);
insert into products values(0,'nike','{"color":"red"}'),(0,'adidas','{"color":"red"}'),(0,'puma','{"color":"green"}');
```
यह हमें देता है:
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
उत्पादों को रंग द्वारा समूहित करने के लिए, हम बस `GROUP BY meta.color` का उपयोग कर सकते हैं, और चयन सूची में संबंधित समूह कुंजी प्रदर्शित करने के लिए, हम `GROUPBY()` का उपयोग कर सकते हैं:

<!-- intro -->
##### उदाहरण:

<!-- request SQL -->
```sql
SELECT groupby() color, count(*) from products GROUP BY meta.color;
```
<!-- response SQL -->
```sql
+-------+----------+
| रंग   | गिनती(*) |
+-------+----------+
| लाल   |        2 |
| हरा   |        1 |
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

## समेकन फ़ंक्शन
`COUNT(*)` के अलावा, जो प्रत्येक समूह में तत्वों की संख्या लौटाता है, आप विभिन्न अन्य समेकन फ़ंक्शन का उपयोग कर सकते हैं:
<!-- example distinct -->
##### COUNT(DISTINCT field)
जबकि `COUNT(*)` समूह में सभी तत्वों की संख्या लौटाता है, `COUNT(DISTINCT field)` समूह में फील्ड के अद्वितीय मानों की संख्या लौटाता है, जो कुल संख्या से पूरी तरह से अलग हो सकता है। उदाहरण के लिए, आपके पास समूह में 100 तत्व हो सकते हैं, लेकिन सभी एक निश्चित फ़ील्ड के लिए समान मान के साथ। `COUNT(DISTINCT field)` इसे निर्धारित करने में मदद करता है। इसे प्रदर्शित करने के लिए, हम "students" नामक एक तालिका बनाते हैं जिसमें छात्र का नाम, उम्र, और प्रमुख है:
```sql
CREATE TABLE students(name text, age int, major string);
INSERT INTO students values(0,'John',21,'arts'),(0,'William',22,'business'),(0,'Richard',21,'cs'),(0,'Rebecca',22,'cs'),(0,'Monica',21,'arts');
```

तो हमारे पास है:

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

उदाहरण में, आप देख सकते हैं कि यदि हम प्रमुख द्वारा GROUP BY करते हैं और दोनों `COUNT(*)` और `COUNT(DISTINCT age)` दिखाते हैं, तो यह स्पष्ट हो जाता है कि "cs" प्रमुख चुनने वाले दो छात्र हैं जिनकी दो अद्वितीय उम्र हैं, लेकिन "arts" प्रमुख के लिए भी दो छात्र हैं, फिर भी केवल एक अद्वितीय उम्र है।

प्रति क्वेरी अधिकतम एक `COUNT(DISTINCT)` हो सकता है।

** डिफ़ॉल्ट रूप से, गणनाएँ अनुमानित होती हैं **

वास्तव में, उनमें से कुछ सटीक होती हैं, जबकि अन्य अनुमानित होती हैं। इसके बारे में अधिक नीचे।

Manticore अद्वितीय मानों की गणना के लिए दो एल्गोरिदम का समर्थन करता है। एक एक पुराना एल्गोरिदम है जो बहुत अधिक मेमोरी का उपयोग करता है और आमतौर पर धीमा होता है। यह `{group; value}` जोड़ियों को एकत्र करता है, उन्हें क्रमबद्ध करता है, और समय-समय पर डुप्लिकेट को समाप्त करता है। इस दृष्टिकोण का लाभ यह है कि यह एक सामान्य तालिका के भीतर सटीक गणनाओं की गारंटी देता है। आप इसे [distinct_precision_threshold](../Searching/Options.md#distinct_precision_threshold) विकल्प को `0` पर सेट करके सक्षम कर सकते हैं।

दूसरा एल्गोरिदम (डिफ़ॉल्ट रूप से सक्षम) गणनाओं को एक हैश टेबल में लोड करता है और इसके आकार को लौटाता है। यदि हैश टेबल बहुत बड़ा हो जाता है, तो इसकी सामग्री को `HyperLogLog` में स्थानांतरित कर दिया जाता है। यहीं गणनाएँ अनुमानित हो जाती हैं क्योंकि `HyperLogLog` एक संभाव्यात्मक एल्गोरिदम है। इसका लाभ यह है कि प्रति समूह अधिकतम मेमोरी उपयोग निश्चित होता है और `HyperLogLog` की सटीकता पर निर्भर करता है। कुल मेमोरी उपयोग [max_matches](../Searching/Options.md#max_matches) सेटिंग पर भी निर्भर करता है, जो समूहों की संख्या को दर्शाता है।

[distinct_precision_threshold](../Searching/Options.md#distinct_precision_threshold) विकल्प उस थ्रेसहोल्ड को सेट करता है जिसके नीचे गणनाओं को सटीक होने की गारंटी दी जाती है। `HyperLogLog` सटीकता सेटिंग और "हैश टेबल से HyperLogLog" रूपांतरण के लिए थ्रेसहोल्ड इस सेटिंग से व्युत्पन्न होते हैं। इस विकल्प का उपयोग सावधानी से करना महत्वपूर्ण है क्योंकि इसे दोगुना करने से गणना के लिए आवश्यक अधिकतम मेमोरी भी दोगुनी हो जाएगी। अधिकतम मेमोरी उपयोग का मोटे तौर पर इस सूत्र का उपयोग करके अनुमान लगाया जा सकता है: `64 * max_matches * distinct_precision_threshold`। ध्यान दें कि यह सबसे खराब स्थिति का परिदृश्य है, और अधिकांश मामलों में, गणना काफी कम RAM का उपयोग करती है।

# खोज परिणामों का समूहबद्धन

<!-- उदाहरण सामान्य -->
खोज परिणामों का समूहबद्धन अक्सर प्रति-समूह मिलान गिनती या अन्य समुच्चय प्राप्त करने के लिए सहायक होता है। उदाहरण के लिए, यह एक ग्राफ बनाने के लिए उपयोगी है जो महीने के अनुसार मिलान करने वाले ब्लॉग पोस्ट की संख्या को दर्शाता है या साइट के अनुसार वेब खोज परिणामों या लेखक के अनुसार फोरम पोस्ट को समूहबद्ध करता है, आदि।

Manticore एकल या कई कॉलमों और गणितीय अभिव्यक्तियों द्वारा खोज परिणामों का समूहबद्धन का समर्थन करता है। परिणाम:

* एक समूह के भीतर क्रमबद्ध किए जा सकते हैं
* एक समूह के लिए एक से अधिक पंक्तियाँ लौटा सकते हैं
* समूहों को छान सकते हैं
* समूहों को क्रमबद्ध कर सकते हैं
* [संग्रहण कार्यों](../Searching/Grouping.md#Aggregation-functions) का उपयोग करके एकत्रित किए जा सकते हैं

<!-- परिचय -->
सामान्य वाक्यविन्यास है:

<!-- अनुरोध SQL -->
सामान्य वाक्यविन्यास
```sql
SELECT {* | SELECT_expr [, SELECT_expr ...]}
...
GROUP BY {field_name | alias } [, ...]
[HAVING where_condition]
[WITHIN GROUP ORDER BY field_name {ASC | DESC} [, ...]]
...

SELECT_expr: { field_name | function_name(...) }
where_condition: {संग्रहण अभिव्यक्ति उपनाम | COUNT(*)}
```

<!-- अनुरोध JSON -->
JSON क्वेरी प्रारूप वर्तमान में एक मूल समूहबद्धन का समर्थन करता है जो संग्रहण मान और उनकी गिनती(*) प्राप्त कर सकता है।

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

मानक क्वेरी उत्पादन समूहबद्धन के बिना परिणाम सेट लौटाता है, जिसे `limit` (या `size`) का उपयोग करके छिपाया जा सकता है।
संग्रहण को समूह के परिणाम सेट के आकार के लिए एक `size` सेट करने की आवश्यकता होती है।

<!-- अंत -->

<!-- उदाहरण समूह1 -->
### केवल समूहबद्धन
समूहबद्धन काफी सरल है - बस अपने `SELECT` क्वेरी के अंत में "GROUP BY smth" जोड़ें। कुछ भी हो सकता है:

* तालिका से कोई भी गैर-पूर्ण पाठ फ़ील्ड: पूर्णांक, फ्लोट, स्ट्रिंग, MVA (मल्टी-वैल्यू विशेषता)
* या, यदि आपने `SELECT` सूची में एक उपनाम का उपयोग किया है, तो आप इसे GROUP BY भी कर सकते हैं

आप `SELECT` सूची में कोई भी [संग्रहण कार्यों](../Searching/Grouping.md#Aggregation-functions) को छोड़ सकते हैं और यह फिर भी काम करेगा:

<!-- परिचय -->
##### उदाहरण:

<!-- अनुरोध SQL -->
```sql
SELECT release_year FROM films GROUP BY release_year LIMIT 5;
```
<!-- प्रतिक्रिया SQL -->
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
<!-- अंत -->
<!-- उदाहरण समूह2 -->
हालाँकि, अधिकांश मामलों में, आप प्रत्येक समूह के लिए कुछ एकत्रित डेटा प्राप्त करना चाहेंगे, जैसे:

* `COUNT(*)` केवल प्रत्येक समूह में तत्वों की संख्या प्राप्त करने के लिए
* या `AVG(field)` समूह के भीतर फ़ील्ड का औसत मान निकालने के लिए

HTTP JSON अनुरोधों के लिए, मुख्य क्वेरी स्तर पर `limit=0` के साथ एकल `aggs` बकेट का उपयोग SQL क्वेरी के समान तरीके से `GROUP BY` और `COUNT(*)` के साथ काम करता है, समान व्यवहार और प्रदर्शन प्रदान करता है।

<!-- परिचय -->
##### उदाहरण:

<!-- अनुरोध SQL1 -->
```sql
SELECT release_year, count(*) FROM films GROUP BY release_year LIMIT 5;
```
<!-- प्रतिक्रिया SQL1 -->
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
<!-- अनुरोध SQL2 -->
```sql
SELECT release_year, AVG(rental_rate) FROM films GROUP BY release_year LIMIT 5;
```
<!-- प्रतिक्रिया SQL2 -->
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

<!-- अनुरोध JSON -->
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
<!-- प्रतिक्रिया JSON -->
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
<!-- अनुरोध PHP -->
``` php
$index->setName('films');
$search = $index->search('');
$search->limit(0);
$search->facet('release_year','release_year',100);
$results = $search->get();
print_r($results->getFacets());
```
<!-- प्रतिक्रिया PHP -->
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
##### समूहों को क्रमबद्ध करना
डिफ़ॉल्ट रूप से, समूहों को क्रमबद्ध नहीं किया जाता है, और अगली चीज जो आप सामान्यतः करना चाहते हैं वह है उन्हें किसी चीज़ के अनुसार आदेश देना, जैसे कि क्षेत्र जिसके द्वारा आप समूह बना रहे हैं:

<!-- intro -->
##### उदाहरण:

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
वैकल्पिक रूप से, आप संचित डाटा के अनुसार क्रमबद्ध कर सकते हैं:

* `count(*)` द्वारा पहले सबसे अधिक तत्वों वाले समूह प्रदर्शित करने के लिए
* `avg(rental_rate)` द्वारा पहले सबसे उच्च रेटेड फ़िल्में दिखाने के लिए। ध्यान दें कि उदाहरण में, यह एक उपनाम के माध्यम से किया गया है: `avg(rental_rate)` पहले `SELECT` सूची में `avg` से मैप किया गया है, और फिर हम बस `ORDER BY avg` करते हैं।


<!-- intro -->
##### उदाहरण:

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
##### एक साथ कई क्षेत्रों द्वारा GROUP BY
कुछ मामलों में, आप केवल एक क्षेत्र द्वारा नहीं, बल्कि एक साथ कई क्षेत्रों द्वारा समूह बनाना चाहते हैं, जैसे कि एक फ़िल्म की श्रेणी और वर्ष:

<!-- intro -->
##### उदाहरण:

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
##### मुझे N पंक्तियाँ दें
कभी-कभी यह देखना उपयोगी होता है कि समूह में न केवल एकल तत्व है, बल्कि कई हैं। इसे `GROUP N BY` की मदद से आसानी से हासिल किया जा सकता है। उदाहरण के लिए, निम्नलिखित मामले में, हम प्रत्येक वर्ष के लिए केवल एक के बजाय दो फिल्मों को प्राप्त करते हैं, जो एक सरल `GROUP BY release_year` ने वापस किया होगा।

<!-- intro -->
##### उदाहरण:

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
##### एक समूह के अंदर क्रमबद्ध करना
एक और महत्वपूर्ण विश्लेषण संबंधी आवश्यकता यह है कि समूह के भीतर तत्वों को क्रमबद्ध किया जाए। इसे हासिल करने के लिए, `WITHIN GROUP ORDER BY ... {ASC|DESC}` खंड का उपयोग करें। उदाहरण के लिए, आइए प्रत्येक वर्ष के लिए सबसे उच्च रेटेड फिल्म प्राप्त करें। ध्यान दें कि यह केवल `ORDER BY` के साथ समानांतर में काम करता है:

* `WITHIN GROUP ORDER BY` परिणामों को **एक समूह के भीतर** क्रमबद्ध करता है
* जबकि केवल `GROUP BY` **स्वयं समूहों को क्रमबद्ध करता है**

ये दोनों पूरी तरह से स्वतंत्र रूप से काम करते हैं।


<!-- intro -->
##### उदाहरण:

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
##### समूहों को फ़िल्टर करना
`HAVING expression` समूहों को फ़िल्टर करने के लिए एक सहायक खंड है। जबकि `WHERE` समूह बनाने से पहले लागू होता है, `HAVING` समूहों के साथ काम करता है। उदाहरण के लिए, आइए केवल उन वर्षों को रखें जब उन वर्ष के लिए फिल्मों की औसत किराया दर 3 से अधिक थी। हमें केवल चार वर्ष मिलते हैं:

<!-- intro -->
##### उदाहरण:

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

ध्यान दें कि `HAVING` [search query meta info](../Node_info_and_management/SHOW_META.md#SHOW-META) में `total_found` को प्रभावित नहीं करता है।

<!-- example group7 -->
##### GROUPBY()
एक कार्य `GROUPBY()` है जो वर्तमान समूह की कुंजी को लौटाता है। यह कई मामलों में सहायक है, विशेष रूप से जब आप [MVA द्वारा GROUP BY](../Searching/Grouping.md#Grouping-by-MVA-%28multi-value-attributes%29) या [JSON मान](../Searching/Grouping.md#Grouping-by-a-JSON-node) करते हैं।

इसका उपयोग `HAVING` में भी किया जा सकता है, उदाहरण के लिए, केवल वर्ष 2000 और 2002 को रखने के लिए।

ध्यान दें कि `GROUPBY()` का उपयोग तब करने की सिफारिश नहीं की जाती है जब आप एक बार में कई फ़ील्ड के द्वारा GROUP BY करते हैं। यह तब भी काम करेगा, लेकिन चूंकि इस मामले में समूह की कुंजी फ़ील्ड मानों का यौगिक है, यह जिस तरह से आप उम्मीद करते हैं उस तरह से दिखाई नहीं दे सकता।

<!-- intro -->
##### उदाहरण:

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
##### MVA (multi-value attributes) द्वारा समूह बनाना
Manticore MVA द्वारा समूह बनाने का समर्थन करता है [MVA](../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29)। यह कैसे काम करता है, इसे प्रदर्शित करने के लिए, चलिए "शूज" नामक एक तालिका बनाते हैं जिसमें MVA "sizes" हो और इसमें कुछ दस्तावेज़ डालते हैं:
```sql
create table shoes(title text, sizes multi);
insert into shoes values(0,'nike',(40,41,42)),(0,'adidas',(41,43)),(0,'reebook',(42,43));
```
तो हमारे पास है:
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
यदि हम अब "sizes" द्वारा GROUP BY करते हैं, तो यह हमारे सभी मल्टी-वैल्यू विशेषताओं को संसाधित करेगा और प्रत्येक के लिए एक संक्षेपण लौटाएगा, इस मामले में केवल गणना:

<!-- intro -->
##### उदाहरण:

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
##### एक JSON नोड द्वारा समूह बनाना
यदि आपके पास [JSON](../Creating_a_table/Data_types.md#JSON) प्रकार का एक फ़ील्ड है, तो आप इसके किसी भी नोड द्वारा GROUP BY कर सकते हैं। इसे दर्शाने के लिए, आइए "products" नामक एक टेबल बनाते हैं जिसमें कुछ डोक्युमेंट हैं, प्रत्येक में "meta" JSON फ़ील्ड में एक रंग है:
```sql
create table products(title text, meta json);
insert into products values(0,'nike','{"color":"red"}'),(0,'adidas','{"color":"red"}'),(0,'puma','{"color":"green"}');
```
हमें यह प्राप्त होता है:
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
उत्पादों को रंग द्वारा समूहित करने के लिए, हम बस `GROUP BY meta.color` का उपयोग कर सकते हैं, और `SELECT` सूची में संबंधित समूह कुंजी प्रदर्शित करने के लिए, हम `GROUPBY()` का उपयोग कर सकते हैं:

<!-- intro -->
##### उदाहरण:

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

## समूहकरण कार्य
इसके अलावा `COUNT(*)`, जो प्रत्येक समूह में तत्वों की संख्या लौटाता है, आप विभिन्न अन्य समूहकरण कार्यों का उपयोग कर सकते हैं:
<!-- example distinct -->
##### COUNT(DISTINCT field)
जबकि `COUNT(*)` समूह में सभी तत्वों की संख्या लौटाता है, `COUNT(DISTINCT field)` समूह में क्षेत्र के अद्वितीय मानों की संख्या लौटाता है, जो कुल गिनती से पूरी तरह भिन्न हो सकता है। उदाहरण के लिए, आपके पास समूह में 100 तत्व हो सकते हैं, लेकिन सभी एक निश्चित क्षेत्र के लिए समान मान के साथ। `COUNT(DISTINCT field)` इसे निर्धारित करने में मदद करता है। इसे प्रदर्शित करने के लिए, आइए "students" नामक एक तालिका बनाएं जिसमें छात्र का नाम, आयु और विषय हो:
```sql
CREATE TABLE students(name text, age int, major string);
INSERT INTO students values(0,'John',21,'arts'),(0,'William',22,'business'),(0,'Richard',21,'cs'),(0,'Rebecca',22,'cs'),(0,'Monica',21,'arts');
```

तो हमारे पास है:

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

उदाहरण में, आप देख सकते हैं कि यदि हम विषय के अनुसार समूह बनाते हैं और दोनों `COUNT(*)` और `COUNT(DISTINCT age)` प्रदर्शित करते हैं, तो स्पष्ट हो जाता है कि "cs" विषय चुनने वाले दो छात्र हैं जिनके दो अद्वितीय उम्र हैं, लेकिन "arts" विषय के लिए भी दो छात्र हैं, फिर भी केवल एक अद्वितीय उम्र है।

प्रति क्वेरी अधिकतम एक `COUNT(DISTINCT)` हो सकता है।

** डिफ़ॉल्ट रूप से, गिनती लगभग होती है **

वास्तव में, उनमें से कुछ सटीक हैं, जबकि अन्य लगभग। इसके बारे में और जानकारी नीचे दी गई है।

Manticore अद्वितीय मानों की गिनती के लिए दो एल्गोरिदम का समर्थन करता है। एक एक विरासत एल्गोरिदम है जो बहुत सारी मेमोरी का उपयोग करता है और आमतौर पर धीमा होता है। यह `{group; value}` जोड़ों को एकत्र करता है, उन्हें वर्गीकृत करता है, और समय-समय पर डुप्लिकेट को हटा देता है। इस दृष्टिकोण का लाभ यह है कि यह एक साधारण तालिका के भीतर सटीक गिनती की गारंटी देता है। आप इसे [distinct_precision_threshold](../Searching/Options.md#distinct_precision_threshold) विकल्प को `0` पर सेट करके सक्षम कर सकते हैं।

दूसरा एल्गोरिदम (डिफ़ॉल्ट रूप से सक्षम) गिनती को एक हैश तालिका में लोड करता है और इसके आकार को लौटाता है। यदि हैश तालिका बहुत बड़ी हो जाती है, तो इसकी सामग्री को `HyperLogLog` में स्थानांतरित कर दिया जाता है। यहीं पर गिनतियाँ लगभग हो जाती हैं क्योंकि `HyperLogLog` एक संभाव्य एल्गोरिदम है। इसका लाभ यह है कि प्रति समूह अधिकतम मेमोरी सेवन निश्चित होता है और `HyperLogLog` की सटीकता पर निर्भर करता है। कुल मेमोरी उपयोग भी [max_matches](../Searching/Options.md#max_matches) सेटिंग पर निर्भर करता है, जो समूहों की संख्या को प्रतिबिंबित करता है।

[distinct_precision_threshold](../Searching/Options.md#distinct_precision_threshold) विकल्प उस सीमा को निर्धारित करता है जिसके तहत गिनतियाँ सटीक होने की गारंटी होती हैं। `HyperLogLog` सटीकता सेटिंग और "हैश तालिका से HyperLogLog" रूपांतरण के लिए सीमा इस सेटिंग से प्राप्त होती है। इस विकल्प का उपयोग सावधानी से करना महत्वपूर्ण है क्योंकि इसे दो गुना करना गिनती की गणनाओं के लिए आवश्यक अधिकतम मेमोरी को दो गुना कर देगा। अधिकतम मेमोरी उपयोग का मोटे तौर पर इस सूत्र का उपयोग करके अनुमान लगाया जा सकता है: `64 * max_matches * distinct_precision_threshold`। ध्यान दें कि यह सबसे खराब स्थिति का परिदृश्य है, और अधिकांश मामलों में, गणना की गिनतियाँ महत्वपूर्ण रूप से कम RAM का उपयोग करेंगी।

**`COUNT(DISTINCT)` एक वितरित तालिका या कई डिस्क टुकड़ों से मिलकर बने वास्तविक समय की तालिका के खिलाफ गलत परिणाम लौटाने की संभावना है**, लेकिन परिणाम एक वितरित तालिका के लिए सटीक होना चाहिए जिसमें स्थानीय साधारण या वास्तविक समय की तालिकाएँ हों जिनकी समान योजना हो (क्षेत्रों का समान सेट/क्रम, लेकिन विभिन्न टोकनाईज़ेशन सेटिंग्स हो सकती हैं)।

<!-- intro -->
##### उदाहरण:

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
<!-- end -->

<!-- example concat -->
##### GROUP_CONCAT(field)

अक्सर, आप प्रत्येक समूह की सामग्री को बेहतर समझना चाहते हैं। इसके लिए आप [GROUP N BY](../Searching/Grouping.md#Give-me-N-rows) का उपयोग कर सकते हैं, लेकिन यह अतिरिक्त पंक्तियाँ लौटाएगा जिन्हें आप आउटपुट में नहीं चाहते। `GROUP_CONCAT()` आपके समूहकरण को समृद्ध करता है, विशेष क्षेत्र के मानों को समूह में संयोजित करके। पिछले उदाहरण को लें और इसे इस प्रकार सुधारें कि प्रत्येक समूह में सभी उम्र प्रदर्शित हो।

`GROUP_CONCAT(field)` संकलित मानों की सूची को अंशांकित मानों के रूप में लौटाता है।

<!-- intro -->
##### उदाहरण:

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
<!-- end -->
<!-- example sum -->
##### SUM(), MIN(), MAX(), AVG()
ज़ाहिर है, आप एक समूह के भीतर योग, औसत, न्यूनतम और अधिकतम मान भी प्राप्त कर सकते हैं।

<!-- intro -->
##### उदाहरण:

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
<!-- end -->

<!-- example accuracy -->
## समूह बनाने की सटीकता

समूह बनाना निश्चित मेमोरी में किया जाता है, जो [max_matches](../Searching/Options.md#max_matches) सेटिंग पर निर्भर करता है। यदि `max_matches` सभी पाए गए समूहों को संग्रहित करने की अनुमति देता है, तो परिणाम 100% सटीक होंगे। हालाँकि, यदि `max_matches` का मान कम है, तो परिणाम कम सटीक होंगे।

जब समांतर प्रोसेसिंग शामिल होती है, तो यह अधिक जटिल हो सकता है। जब `pseudo_sharding` सक्षम किया जाता है और/या कई डिस्क चंक्स के साथ एक RT तालिका का उपयोग किया जाता है, तो प्रत्येक चंक या स्यूडो शार्ड को एक परिणाम सेट मिलता है जो `max_matches` से बड़ा नहीं होता है। जब विभिन्न थ्रेड्स से परिणाम सेट विलीन होते हैं, तो इससे समग्र और समूह गणनाओं में गलतियाँ हो सकती हैं। इसे ठीक करने के लिए, या तो एक बड़ा `max_matches` मान या समांतर प्रोसेसिंग को निष्क्रिय किया जा सकता है।

Manticore कोशिश करेगा कि यदि यह पहचानता है कि समूह द्वारा परिणाम गलत हो सकते हैं, तो `max_matches` को [max_matches_increase_threshold](../Searching/Options.md#max_matches_increase_threshold) तक बढ़ा दिया जाए। पहचान उस समूह द्वारा विशेषता के अद्वितीय मानों की संख्या पर आधारित होती है, जो द्वितीयक सूचकांकों से प्राप्त की जाती है (यदि मौजूद हो)।

जब RT तालिकाओं या `pseudo_sharding` का उपयोग करते हुए सटीक समग्र और/या समूह गणनाओं को सुनिश्चित करने के लिए, `accurate_aggregation` सक्षम किया जा सकता है। यह थ्रेशोल्ड तक `max_matches` बढ़ाने की कोशिश करेगा, और यदि थ्रेशोल्ड पर्याप्त उच्च नहीं है, तो Manticore क्वेरी के लिए समांतर प्रोसेसिंग को अक्षम कर देगा।

<!-- intro -->
##### उदाहरण:

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
<!-- end -->
