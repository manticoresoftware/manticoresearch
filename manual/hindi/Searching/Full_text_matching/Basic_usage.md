# मिलान
The `MATCH` प्रावधान टेक्स्ट फ़ील्ड में पूर्ण-टेक्स्ट खोजों की अनुमति देता है। इनपुट क्वेरी स्ट्रिंग को [tokenized](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md) उस टेक्स्ट के लिए लागू किए गए समान सेटिंग्स का उपयोग करके टोकनाइज़ किया जाता है। इनपुट टेक्स्ट के टोकनाइज़ेशन के अलावा, क्वेरी स्ट्रिंग कई [पूर्ण-टेक्स्ट ऑपरेटरों](../../Searching/Full_text_matching/Operators.md) का समर्थन करता है जो कीवर्ड से एक मान्य मिलान कैसे प्रदान करना चाहिए, इस पर विभिन्न नियम लागू करते हैं।
पूर्ण-टेक्स्ट मिलान प्रावधानों को एक AND बूलियन के रूप में विशेषता [फिल्टरों](../../Searching/Filters.md) के साथ मिलाया जा सकता है। **पूर्ण-टेक्स्ट मिलानों और विशेषता फ़िल्टरों के बीच OR संबंधों का समर्थन नहीं है**।
फ़िल्टरिंग प्रक्रिया में मिलान क्वेरी हमेशा पहले निष्पादित होती है, इसके बाद [विशेषता फ़िल्टर](../../Searching/Filters.md) लगाया जाता है। विशेषता फ़िल्टर मिलान क्वेरी के परिणाम सेट पर लागू होते हैं। बिना मिलान प्रावधान के एक क्वेरी को फुलस्कैन कहा जाता है।
`SELECT` प्रावधान में अधिकतम एक `MATCH()` होना चाहिए।
[पूर्ण-टेक्स्ट क्वेरी सिंटैक्स](../../Searching/Full_text_matching/Operators.md) का उपयोग करते समय, मिलान सभी अनुक्रमित टेक्स्ट फ़ील्ड पर किया जाता है, जब तक कि अभिव्यक्ति एक फ़ील्ड के भीतर (जैसे वाक्यांश खोज) मिलान की मांग नहीं करती या इसे फ़ील्ड ऑपरेटरों द्वारा सीमित नहीं किया गया हो।
[JOIN](../../Searching/Joining.md) क्वेरियों का उपयोग करते समय, `MATCH()` एक वैकल्पिक दूसरा पैरामीटर स्वीकार कर सकता है जो यह निर्दिष्ट करता है कि पूर्ण-टेक्स्ट खोज को किस तालिका पर लागू किया जाना चाहिए। डिफ़ॉल्ट के अनुसार, पूर्ण-टेक्स्ट क्वेरी `JOIN` ऑपरेशन में बाएं तालिका पर लागू होती है:
```sql
SELECT * FROM table1 LEFT JOIN table2 ON table1.id = table2.id WHERE MATCH('search query', table2);
```
यह आपको एक जोड़ ऑपरेशन में विशिष्ट तालिकाओं पर पूर्ण-टेक्स्ट खोज करने की अनुमति देता है। JOINs के साथ MATCH का उपयोग करने के बारे में अधिक विवरण के लिए, [तालिकाएँ जोड़ने](../../Searching/Joining.md) अनुभाग देखें।
## SQL
<!-- example Example_1 -->
```sql
MATCH('search query' [, table_name])
```
- `'search query'`: पूर्ण-टेक्स्ट खोज क्वेरी स्ट्रिंग, जिसमें विभिन्न [पूर्ण-टेक्स्ट ऑपरेटर](../../Searching/Full_text_matching/Operators.md) शामिल हो सकते हैं।
- `table_name`: (वैकल्पिक) वह तालिका का नाम जिस पर पूर्ण-टेक्स्ट खोज लागू करनी है, `JOIN` क्वेरियों में डिफ़ॉल्ट बाईं तालिका से भिन्न तालिका निर्दिष्ट करने के लिए।
The [SELECT](../../Searching/Full_text_matching/Basic_usage.md#SQL) बयान एक [MATCH](../../Searching/Full_text_matching/Basic_usage.md) प्रावधान का उपयोग करता है, जो WHERE के बाद आना चाहिए, पूर्ण-टेक्स्ट खोज करने के लिए। `MATCH()` एक इनपुट स्ट्रिंग स्वीकार करता है जिसमें सभी [पूर्ण-टेक्स्ट ऑपरेटर](../../Searching/Full_text_matching/Operators.md) उपलब्ध हैं।
<!-- intro -->
##### SQL:
<!-- request SQL -->
```sql
SELECT * FROM myindex WHERE MATCH('"find me fast"/2');
```
<!-- response SQL -->
```sql
+------+------+----------------+
| id   | gid  | title          |
+------+------+----------------+
|    1 |   11 | first find me  |
|    2 |   12 | second find me |
+------+------+----------------+
2 rows in set (0.00 sec)
```
<!-- request MATCH with filters -->
एक अधिक जटिल क्वेरी का उदाहरण जो WHERE फ़िल्टर के साथ MATCH का उपयोग करता है।
```sql
SELECT * FROM myindex WHERE MATCH('cats|birds') AND (`title`='some title' AND `id`=123);
```
<!-- end -->
## HTTP JSON
<!-- example Example_11 -->
पूर्ण-टेक्स्ट मिलान `/search` एंडपॉइंट और HTTP-आधारित क्लाइंट्स में उपलब्ध है। पूर्ण-टेक्स्ट मिलान करने के लिए निम्नलिखित प्रावधानों का उपयोग किया जा सकता है:
### मिलान
"match" एक सरल क्वेरी है जो निर्दिष्ट फ़ील्ड में निर्दिष्ट कीवर्ड से मेल खाता है।
```json
"query":
{
  "match": { "field": "keyword" }
}
```
आप फ़ील्डों की सूची निर्दिष्ट कर सकते हैं:
```json
"match":
{
  "field1,field2": "keyword"
}
```
या आप सभी फ़ील्डों को खोजने के लिए `_all` या `*` का उपयोग कर सकते हैं।
आप "!field" का उपयोग करके एक को छोड़कर सभी फ़ील्डों को खोज सकते हैं:
```json
"match":
{
  "!field1": "keyword"
}
```
डिफ़ॉल्ट के अनुसार, कीवर्ड OR ऑपरेटर का उपयोग करके संयोजित होते हैं। हालाँकि, आप "operator" प्रावधान का उपयोग करके उस व्यवहार को बदल सकते हैं:
```json
"query":
{
  "match":
  {
    "content,title":
    {
      "query":"keyword",
      "operator":"or"
    }
  }
}
```
"operator" को "or" या "and" पर सेट किया जा सकता है।
`boost` संशोधन भी लागू किया जा सकता है। यह शब्द [IDF](../../Searching/Options.md#idf)_score को रैंकिंग स्कोर में इंगित किए गए गुणांक से बढ़ाता है जो IDF को अपने गणनाओं में शामिल करता है। यह मिलान प्रक्रिया को किसी भी प्रकार से प्रभावित नहीं करता है।
```json
"query":
{
  "match":
  {
    "field1":
    {
      "query": "keyword",
      "boost": 2.0
    }
  }
}
```
### match_phrase
"match_phrase" एक क्वेरी है जो पूरे वाक्यांश से मेल खाती है। यह SQL में वाक्यांश ऑपरेटर के समान है। यहाँ एक उदाहरण है:
```json
"query":
{
  "match_phrase": { "_all" : "had grown quite" }
}
```
### query_string
"query_string" एक इनपुट स्ट्रिंग को `MATCH()` सिंटैक्स में पूर्ण-टेक्स्ट क्वेरी के रूप में स्वीकार करता है।
```json
"query":
{
  "query_string": "Church NOTNEAR/3 street"
}
```
### match_all
"match_all" एक खाली ऑब्जेक्ट स्वीकार करता है और बिना किसी विशेषता फ़िल्टरिंग या पूर्ण-टेक्स्ट मिलान किए तालिका से दस्तावेज़ लौटाता है। आपके लिए सिर्फ `query` प्रावधान को अनुरोध में छोड़ देना भी समान प्रभाव डालेगा।
```json
"query":
{
  "match_all": {}
}
```
### अन्य फ़िल्टरों के साथ पूर्ण-टेक्स्ट फ़िल्टरिंग का संयोजन
सभी पूर्ण-टेक्स्ट मिलान प्रावधानों को [must](../../Searching/Filters.md#must), [must_not](../../Searching/Filters.md#must_not), और [should](../../Searching/Filters.md#should) ऑपरेटरों के साथ संयोजित किया जा सकता है [JSON `bool` क्वेरी](../../Searching/Filters.md#bool-query) के।
<!-- intro -->
उदाहरण:
<!-- request match -->
```json
POST /search
-d
'{
    "table" : "hn_small",
    "query":
    {
        "match":
        {
            "*" : "जोड़ो जो"
        }
    },
    "_source": ["कहानी_लेखक","टिप्पणी_लेखक"],
    "limit": 1
}'
```

<!-- response match -->
```json
{
   "took" : 3,
   "timed_out" : false,
   "hits" : {
      "hits" : [
         {
            "_id": 668018,
            "_score" : 3579,
            "_source" : {
               "story_author" : "IgorPartola",
               "comment_author" : "joe_the_user"
            }
         }
      ],
      "total" : 88063,
      "total_relation" : "eq"
   }
}
```
<!-- request match_phrase -->
```json
POST /search
-d
'{
    "table" : "hn_small",
    "query":
    {
        "match_phrase":
        {
            "*" : "जोड़ो जो"
        }
    },
    "_source": ["कहानी_लेखक","टिप्पणी_लेखक"],
    "limit": 1
}'
```
<!-- response match_phrase -->
```json
{
   "took" : 3,
   "timed_out" : false,
   "hits" : {
      "hits" : [
         {
            "_id": 807160,
            "_score" : 2599,
            "_source" : {
               "story_author" : "rbanffy",
               "comment_author" : "runjake"
            }
         }
      ],
      "total" : 2,
      "total_relation" : "eq"
   }
}
```

<!-- request query_string -->
```json
POST /search
-d
'{   "table" : "hn_small",
    "query":
    {
        "query_string": "@comment_text \"जोड़ो जो तेज \"/2"
    },
    "_source": ["कहानी_लेखक","टिप्पणी_लेखक"],
    "limit": 1
}'
```
<!-- response query_string -->
```json
{
  "took" : 3,
  "timed_out" : false,
  "hits" : {
      "hits" : [
         {
            "_id": 807160,
            "_score" : 2566,
            "_source" : {
               "story_author" : "rbanffy",
               "comment_author" : "runjake"
            }
         }
      ],
      "total" : 1864,
      "total_relation" : "eq"
   }
}
```
<!-- request PHP -->
```php
$search = new Search(new Client());
$result = $search->('@title मुझे जल्दी खोजो');
foreach($result as $doc)
{
    echo 'Document: '.$doc->getId();
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.': '.$value;
    }
}
```
<!-- response PHP -->
```php
Document: 1
title: पहले मुझे जल्दी खोजो
gid: 11
Document: 2
title: दूसरे मुझे जल्दी खोजो
gid: 12

```

<!-- intro -->
पाइथन
<!-- request Python -->

```python
searchApi.search({"table":"hn_small","query":{"query_string":"@comment_text \"जोड़ो जो तेज \"/2"}, "_source": ["कहानी_लेखक","टिप्पणी_लेखक"], "limit":1})
```
<!-- response Python -->
``` python
{'aggregations': None,
 'hits': {'hits': [{'_id': '807160',
                    '_score': 2566,
                    '_source': {'comment_author': 'runjake',
                                'story_author': 'rbanffy'}}],
          'max_score': None,
          'total': 1864,
          'total_relation': 'eq'},
 'profile': None,
 'timed_out': False,
 'took': 2,
 'warning': None}
```

<!-- intro -->
जावास्क्रिप्ट
<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"hn_small","query":{"query_string":"@comment_text \"जोड़ो जो तेज \"/2"}, "_source": ["कहानी_लेखक","टिप्पणी_लेखक"], "limit":1});
```
<!-- response javascript -->
```javascript
{
  took: 1,
  timed_out: false,
  hits:
   exports {
     total: 1864,
     total_relation: 'eq',
     hits:
      [ { _id: '807160',
          _score: 2566,
          _source: { कहानी_लेखक: 'rbanffy', टिप्पणी_लेखक: 'runjake' }
        }
      ]
   }
}
```

<!-- intro -->
java
<!-- request Java -->

```java

query = new HashMap<String,Object>();
query.put("query_string", "@comment_text \"जोड़ो जो तेज \"/2");
searchRequest = new SearchRequest();
searchRequest.setIndex("hn_small");
searchRequest.setQuery(query);
searchRequest.addSourceItem("कहानी_लेखक");
searchRequest.addSourceItem("टिप्पणी_लेखक");
searchRequest.limit(1);
searchResponse = searchApi.search(searchRequest);
```
<!-- response Java -->
```java
class SearchResponse {
    took: 1
    timedOut: false
    aggregations: null
    hits: class SearchResponseHits {
        maxScore: null
        total: 1864
        totalRelation: eq
        hits: [{_id=807160, _score=2566, _source={कहानी_लेखक=rbanffy, टिप्पणी_लेखक=runjake}}]
    }
    profile: null
    warning: null
}
```

<!-- intro -->
C#
<!-- request C# -->

```clike
object query =  new { query_string="@comment_text \"जोड़ो जो तेज \"/2" };
var searchRequest = new SearchRequest("hn_small", query);
searchRequest.Source = new List<string> {"कहानी_लेखक", "टिप्पणी_लेखक"};
searchRequest.Limit = 1;
SearchResponse searchResponse = searchApi.Search(searchRequest);
```
<!-- response C# -->
```clike
class SearchResponse {
    took: 1
    timedOut: false
    aggregations: null
    hits: class SearchResponseHits {
        maxScore: null
        total: 1864
        totalRelation: eq
        hits: [{_id=807160, _score=2566, _source={कहानी_लेखक=rbanffy, टिप्पणी_लेखक=runjake}}]
    }
    profile: null
    warning: null
}
```

<!-- intro -->
टाइपस्क्रिप्ट
<!-- request TypeScript -->

```typescript
res = await searchApi.search({
  index: 'test',
  query: { query_string: "टेस्ट दस्तावेज 1" },
  "_source": ["सामग्री", "शीर्षक"],
  limit: 1
});
```
<!-- response TypeScript -->
```json
{
  took: 1,
  timed_out: false,
  hits:
   exports {
     total: 5,
     total_relation: 'eq',
     hits:
      [ { _id: '1',
          _score: 2566,
          _source: { सामग्री: 'यह एक टेस्ट दस्तावेज 1 है', शीर्षक: 'डॉक 1' }
        }
      ]
   }
}
```

<!-- intro -->
जाओ
<!-- request Go -->

```go
searchRequest := manticoresearch.NewSearchRequest("test")
query := map[string]interface{} {"query_string": "test document 1"}
searchReq.SetSource([]string{"content", "title"})
searchReq.SetLimit(1)
resp, httpRes, err := search.SearchRequest(*searchRequest).Execute()
```
<!-- response Go -->
```json
{
  "hits": {
    "hits": [
      {
        "_id": 1,
        "_score": 2566,
        "_source": {
          "content": "यह एक परीक्षण दस्तावेज 1 है",
          "title": "Doc 1"
        }
      }
    ],
    "total": 5,
    "total_relation": "eq"
  },
  "timed_out": false,
  "took": 0
}
```
<!-- end -->
<!-- proofread -->
















































