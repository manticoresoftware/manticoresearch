# वर्गीकरण और रैंकिंग

क्वेरी परिणामों को पूर्ण-शब्द रैंकिंग वजन, एक या अधिक गुण या अभिव्यक्तियों द्वारा वर्गीकृत किया जा सकता है।

**पूर्ण-शब्द** क्वेरी मिलान को डिफ़ॉल्ट रूप से वर्गीकृत करती हैं। यदि कुछ निर्दिष्ट नहीं किया गया है, तो उन्हें प्रासंगिकता के अनुसार वर्गीकृत किया जाता है, जो SQL प्रारूप में `ORDER BY weight() DESC` के बराबर है।

**गैर-पूर्ण-शब्द** क्वेरी डिफ़ॉल्ट रूप से कोई वर्गीकरण नहीं करती हैं।

## उन्नत वर्गीकरण

विस्तारित मोड स्वचालित रूप से सक्षम होता है जब आप स्पष्ट रूप से वर्गीकरण नियम प्रदान करते हैं SQL प्रारूप में `ORDER BY` खंड जोड़कर या HTTP JSON के माध्यम से `sort` विकल्प का उपयोग करके।

### SQL के माध्यम से वर्गीकरण

सामान्य वाक्यविन्यास:
```sql
SELECT ... ORDER BY
{attribute_name | expr_alias | weight() | random() } [ASC | DESC],
...
{attribute_name | expr_alias | weight() | random() } [ASC | DESC]
```

<!-- उदाहरण उपनाम -->

क्रम खंड में, आप 5 कॉलम तक के किसी भी संयोजन का उपयोग कर सकते हैं, प्रत्येक के बाद `asc` या `desc`। क्रम खंड के लिए तर्क के रूप में फ़ंक्शन और अभिव्यक्तियों की अनुमति नहीं है, केवल `weight()` और `random()` फ़ंक्शन (जो केवल SQL के माध्यम से `ORDER BY random()` के रूप में उपयोग किया जा सकता है)। हालाँकि, आप SELECT सूची में किसी भी अभिव्यक्ति का उपयोग कर सकते हैं और उसके उपनाम द्वारा वर्गीकृत कर सकते हैं।

<!-- अनुरोध SQL -->
```sql
select *, a + b alias from test order by alias desc;
```

<!-- प्रतिक्रिया SQL -->
```
+------+------+------+----------+-------+
| id   | a    | b    | f        | alias |
+------+------+------+----------+-------+
|    1 |    2 |    3 | document |     5 |
+------+------+------+----------+-------+
```

<!-- अंत -->

## JSON के माध्यम से वर्गीकरण

<!-- उदाहरण वर्गीकरण 1 -->
`"sort"` एक ऐरे को निर्दिष्ट करता है जहाँ प्रत्येक तत्व एक गुण नाम या यदि आप मिलान वजन के द्वारा वर्गीकृत करना चाहते हैं तो `_score` हो सकता है। इस मामले में, गुणों के लिए क्रम सामान्य रूप से चढ़ता है और `_score` के लिए घटता है।

<!-- परिचय -->

<!-- अनुरोध JSON -->

```json
{
  "table":"test",
  "query":
  {
    "match": { "title": "Test document" }
  },
  "sort": [ "_score", "id" ],
  "_source": "title",
  "limit": 3
}
```

<!-- प्रतिक्रिया JSON -->

``` json
    {

"took": 0,

"timed_out": false,

"hits": {

  "total": 5,

  "total_relation": "eq",

  "hits": [

    {

      "_id": 5406864699109146628,

      "_score": 2319,

      "_source": {

        "title": "Test document 1"

      }

    },

    {

      "_id": 5406864699109146629,

      "_score": 2319,

      "_source": {

        "title": "Test document 2"

      }

    },

    {

      "_id": 5406864699109146630,

      "_score": 2319,

      "_source": {

        "title": "Test document 3"

      }

    }

  ]

}

}
```    

<!-- परिचय -->
##### PHP:

<!-- अनुरोध PHP -->

```php
$search->setIndex("test")->match('Test document')->sort('_score')->sort('id');
```

<!-- परिचय -->

##### Python:

<!-- अनुरोध Python -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
search_request.sort = ['_score', 'id']
```

<!-- परिचय -->

##### Python-asyncio:

<!-- अनुरोध Python-asyncio -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
search_request.sort = ['_score', 'id']
```

<!-- परिचय -->

##### Javascript:

<!-- अनुरोध javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Test document');
searchRequest.sort = ['_score', 'id'];
```

<!-- परिचय -->

##### java:

<!-- अनुरोध Java -->
``` java
searchRequest.setIndex("test");
QueryFilter queryFilter = new QueryFilter();
queryFilter.setQueryString("Test document");
searchRequest.setFulltextFilter(queryFilter);
List<Object> sort = new ArrayList<Object>( Arrays.asList("_score", "id") );
searchRequest.setSort(sort);

```

<!-- परिचय -->

##### C#:

<!-- अनुरोध C# -->
``` clike
var searchRequest = new SearchRequest("test");
searchRequest.FulltextFilter = new QueryFilter("Test document");
searchRequest.Sort = new List<Object> {"_score", "id"};

```

<!-- परिचय -->

##### Rust:

<!-- अनुरोध Rust -->
``` rust
let query = SearchQuery {
    query_string: Some(serde_json::json!("Test document").into()),
    ..Default::default(),
};
let sort: [String; 2] = ["_score".to_string(), "id".to_string()];
let search_req = SearchRequest {
    table: "test".to_string(),
    query: Some(Box::new(query)),
    sort: Some(serde_json::json!(sort)),
    ..Default::default(),
};
```

<!-- परिचय -->

##### Typescript:

<!-- अनुरोध typescript -->
``` typescript
searchRequest = {
  index: 'test',
  query: {
    query_string: {'Test document'},
  },
  sort: ['_score', 'id'],
}
```

<!-- परिचय -->

##### Go:

<!-- अनुरोध go -->
```go
searchRequest.SetIndex("test")
query := map[string]interface{} {"query_string": "Test document"}
searchRequest.SetQuery(query)
sort := map[string]interface{} {"_score": "asc", "id": "asc"}
searchRequest.SetSort(sort)
```

<!-- अंत -->

<!-- उदाहरण वर्गीकरण 2 -->
आप क्रम को स्पष्ट रूप से निर्दिष्ट भी कर सकते हैं:

* `asc`: बढ़ते क्रम में वर्गीकृत करें
* `desc`: घटते क्रम में वर्गीकृत करें


<!-- परिचय -->

<!-- अनुरोध JSON -->

```json
{
  "table":"test",
  "query":
  {
    "match": { "title": "Test document" }
  },
  "sort":
  [
    { "id": "desc" },
    "_score"
  ],
  "_source": "title",
  "limit": 3
}
```

<!-- प्रतिक्रिया JSON -->

``` json
{

"took": 0,

"timed_out": false,

"hits": {

  "total": 5,

  "total_relation": "eq",

  "hits": [

    {

      "_id": 5406864699109146632,

      "_score": 2319,

      "_source": {

        "title": "परीक्षण दस्तावेज़ 5"

      }

    },

    {

      "_id": 5406864699109146631,

      "_score": 2319,

      "_source": {

        "title": "परीक्षण दस्तावेज़ 4"

      }

    },

    {

      "_id": 5406864699109146630,

      "_score": 2319,

      "_source": {

        "title": "परीक्षण दस्तावेज़ 3"

      }

    }

  ]

}

}
```    

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$search->setIndex("test")->match('परीक्षण दस्तावेज़')->sort('id', 'desc')->sort('_score');
```

<!-- intro -->

##### Python:

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('परीक्षण दस्तावेज़')
sort_by_id = manticoresearch.model.SortOrder('id', 'desc')
search_request.sort = [sort_by_id, '_score']
```

<!-- intro -->

##### Python-asyncio:

<!-- request Python-asyncio -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('परीक्षण दस्तावेज़')
sort_by_id = manticoresearch.model.SortOrder('id', 'desc')
search_request.sort = [sort_by_id, '_score']
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('परीक्षण दस्तावेज़');
sortById = new Manticoresearch.SortOrder('id', 'desc');
searchRequest.sort = [sortById, 'id'];
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
searchRequest.setIndex("test");
QueryFilter queryFilter = new QueryFilter();
queryFilter.setQueryString("परीक्षण दस्तावेज़");
searchRequest.setFulltextFilter(queryFilter);
List<Object> sort = new ArrayList<Object>();
SortOrder sortById = new SortOrder();
sortById.setAttr("id");
sortById.setOrder(SortOrder.OrderEnum.DESC);
sort.add(sortById);
sort.add("_score");
searchRequest.setSort(sort);

```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
var searchRequest = new SearchRequest("test");
searchRequest.FulltextFilter = new QueryFilter("परीक्षण दस्तावेज़");
searchRequest.Sort = new List<Object>();
var sortById = new SortOrder("id", SortOrder.OrderEnum.Desc);
searchRequest.Sort.Add(sortById);
searchRequest.Sort.Add("_score");
```

<!-- intro -->

##### Rust:

<!-- request Rust -->
``` rust
let query = SearchQuery {
    query_string: Some(serde_json::json!("परीक्षण दस्तावेज़").into()),
    ..Default::default(),
};
let sort_by_id = HashMap::new();
sort_by_id.insert("id".to_string(), "desc".to_string()); 
let mut sort = Vec::new();
sort.push(sort_by_id);
sort.push("_score".to_string());
let search_req = SearchRequest {
    table: "test".to_string(),
    query: Some(Box::new(query)),
    sort: Some(serde_json::json!(sort)),
    ..Default::default(),
};
```

<!-- intro -->

##### Typescript:

<!-- request typescript -->
``` typescript
searchRequest = {
  index: 'test',
  query: {
    query_string: {'परीक्षण दस्तावेज़'},
  },
  sort: [{'id': 'desc'}, '_score'],
}
```

<!-- intro -->

##### Go:

<!-- request go -->
```go
searchRequest.SetIndex("test")
query := map[string]interface{} {"query_string": "परीक्षण दस्तावेज़"}
searchRequest.SetQuery(query)
sortById := map[string]interface{} {"id": "desc"}
sort := map[string]interface{} {"id": "desc", "_score": "asc"}
searchRequest.SetSort(sort)
```

<!-- end -->

<!-- example sorting 3 -->
You can also use another syntax and specify the sort order via the `order` property:


<!-- intro -->

<!-- request JSON -->

```json
{
  "table":"test",
  "query":
  {
    "match": { "title": "परीक्षण दस्तावेज़" }
  },
  "sort":
  [
    { "id": { "order":"desc" } }
  ],
  "_source": "title",
  "limit": 3
}
```

<!-- response JSON -->

``` json

{

"took": 0,

"timed_out": false,

"hits": {

  "total": 5,

  "total_relation": "eq",

  "hits": [

    {

      "_id": 5406864699109146632,

      "_score": 2319,

      "_source": {

        "title": "परीक्षण दस्तावेज़ 5"

      }

    },

    {

      "_id": 5406864699109146631,

      "_score": 2319,

      "_source": {

        "title": "परीक्षण दस्तावेज़ 4"

      }

    },

    {

      "_id": 5406864699109146630,

      "_score": 2319,

      "_source": {

        "title": "परीक्षण दस्तावेज़ 3"

      }

    }

  ]

}

}
```    

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$search->setIndex("test")->match('परीक्षण दस्तावेज़')->sort('id', 'desc');
```

<!-- intro -->

##### Python:

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('परीक्षण दस्तावेज़')
sort_by_id = manticoresearch.model.SortOrder('id', 'desc')
search_request.sort = [sort_by_id]
```

<!-- intro -->

##### Python-asyncio:

<!-- request Python-asyncio -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('परीक्षण दस्तावेज़')
sort_by_id = manticoresearch.model.SortOrder('id', 'desc')
search_request.sort = [sort_by_id]
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Test document');
sortById = new Manticoresearch.SortOrder('id', 'desc');
searchRequest.sort = [sortById];
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
searchRequest.setIndex("test");
QueryFilter queryFilter = new QueryFilter();
queryFilter.setQueryString("Test document");
searchRequest.setFulltextFilter(queryFilter);
List<Object> sort = new ArrayList<Object>();
SortOrder sortById = new SortOrder();
sortById.setAttr("id");
sortById.setOrder(SortOrder.OrderEnum.DESC);
sort.add(sortById);
searchRequest.setSort(sort);

```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
var searchRequest = new SearchRequest("test");
searchRequest.FulltextFilter = new QueryFilter("Test document");
searchRequest.Sort = new List<Object>();
var sortById = new SortOrder("id", SortOrder.OrderEnum.Desc);
searchRequest.Sort.Add(sortById);
```

<!-- intro -->

##### Rust:

<!-- request Rust -->
``` rust
let query = SearchQuery {
    query_string: Some(serde_json::json!("Test document").into()),
    ..Default::default(),
};
let mut sort_by_id = HashMap::new();
sort_by_id.insert("id".to_string(), "desc".to_string()); 
let sort = [HashMap; 1] = [sort_by_id];
let search_req = SearchRequest {
    table: "test".to_string(),
    query: Some(Box::new(query)),
    sort: Some(serde_json::json!(sort)),
    ..Default::default(),
};
```

<!-- intro -->

##### Typescript:

<!-- request typescript -->
``` typescript
searchRequest = {
  index: 'test',
  query: {
    query_string: {'Test document'},
  },
  sort: { {'id': {'order':'desc'} },
}
```

<!-- intro -->

##### Go:

<!-- request go -->
```go
searchRequest.SetIndex("test")
query := map[string]interface{} {"query_string": "Test document"}
searchRequest.SetQuery(query)
sort := map[string]interface{} { "id": {"order":"desc"} }
searchRequest.SetSort(sort)
```

<!-- end -->

<!-- example sorting 4 -->
JSON प्रश्नों में MVA गुणों द्वारा क्रमबद्ध करने का समर्थन भी किया जाता है। क्रमबद्धता मोड `mode` संपत्ति के माध्यम से सेट किया जा सकता है। निम्नलिखित मोड समर्थित हैं:

* `min`: न्यूनतम मान द्वारा क्रमबद्ध करें
* `max`: अधिकतम मान द्वारा क्रमबद्ध करें

<!-- intro -->

<!-- request JSON -->

```json
{
  "table":"test",
  "query":
  {
    "match": { "title": "Test document" }
  },
  "sort":
  [
    { "attr_mva": { "order":"desc", "mode":"max" } }
  ],
  "_source": "title",
  "limit": 3
}
```

<!-- response JSON -->

``` json

{

"took": 0,

"timed_out": false,

"hits": {

  "total": 5,

  "total_relation": "eq",

  "hits": [

    {

      "_id": 5406864699109146631,

      "_score": 2319,

      "_source": {

        "title": "Test document 4"

      }

    },

    {

      "_id": 5406864699109146629,

      "_score": 2319,

      "_source": {

        "title": "Test document 2"

      }

    },

    {

      "_id": 5406864699109146628,

      "_score": 2319,

      "_source": {

        "title": "Test document 1"

      }

    }

  ]

}

}
```    

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$search->setIndex("test")->match('Test document')->sort('id','desc','max');
```

<!-- intro -->

##### Python:

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
sort = manticoresearch.model.SortMVA('attr_mva', 'desc', 'max')
search_request.sort = [sort]
```

<!-- intro -->

##### Python-asyncio:

<!-- request Python-asyncio -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
sort = manticoresearch.model.SortMVA('attr_mva', 'desc', 'max')
search_request.sort = [sort]
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Test document');
sort = new Manticoresearch.SortMVA('attr_mva', 'desc', 'max');
searchRequest.sort = [sort];
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
searchRequest.setIndex("test");
QueryFilter queryFilter = new QueryFilter();
queryFilter.setQueryString("Test document");
searchRequest.setFulltextFilter(queryFilter);
SortMVA sort = new SortMVA();
sort.setAttr("attr_mva");
sort.setOrder(SortMVA.OrderEnum.DESC);
sort.setMode(SortMVA.ModeEnum.MAX);
searchRequest.setSort(sort);

```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
var searchRequest = new SearchRequest("test");
searchRequest.FulltextFilter = new QueryFilter("Test document");
var sort = new SortMVA("attr_mva", SortMVA.OrderEnum.Desc, SortMVA.ModeEnum.Max);
searchRequest.Sort.Add(sort);
```

<!-- intro -->

##### Rust:

<!-- request Rust -->
``` rust
let query = SearchQuery {
    query_string: Some(serde_json::json!("Test document").into()),
    ..Default::default(),
};
let mut sort_mva_opts = HashMap::new();
sort_mva_opts.insert("order".to_string(), "desc".to_string());
sort_mva_opts.insert("mode".to_string(), "max".to_string());
let mut sort_mva = HashMap::new();
sort_mva.insert("attr_mva".to_string(), sort_mva_opts); 

let search_req = SearchRequest {
    table: "test".to_string(),
    query: Some(Box::new(query)),
    sort: Some(serde_json::json!(sort_mva)),
    ..Default::default(),
};
```

<!-- intro -->

##### Typescript:
<!-- request typescript -->
``` typescript
searchRequest = {
  index: 'test',
  query: {
    query_string: {'Test document'},
  },
  sort: { "attr_mva": { "order":"desc", "mode":"max" } },
}
```

<!-- intro -->

##### Go:

<!-- request go -->
```go
searchRequest.SetIndex("test")
query := map[string]interface{} {"query_string": "Test document"}
searchRequest.SetQuery(query)
sort := map[string]interface{} { "attr_mva": { "order":"desc", "mode":"max" } }
searchRequest.SetSort(sort)
```

<!-- end -->

<!-- example sorting 5 -->
जब किसी विशेषता पर ترتيب दिया जाता है, तो मेल वजन (स्कोर) की गणना डिफ़ॉल्ट रूप से असक्रिय होती है (कोई रेंकर उपयोग नहीं होता)। आप `track_scores` गुण को `true` पर सेट करके वजन गणना सक्षम कर सकते हैं:

<!-- intro -->

<!-- request JSON -->

```json
{
  "table":"test",
  "track_scores": true,
  "query":
  {
    "match": { "title": "Test document" }
  },
  "sort":
  [
    { "attr_mva": { "order":"desc", "mode":"max" } }
  ],
  "_source": "title",
  "limit": 3
}
```

<!-- response JSON -->

``` json

{

"took": 0,

"timed_out": false,

"hits": {

  "total": 5,

  "total_relation": "eq",

  "hits": [

    {

      "_id": 5406864699109146631,

      "_score": 2319,

      "_source": {

        "title": "Test document 4"

      }

    },

    {

      "_id": 5406864699109146629,

      "_score": 2319,

      "_source": {

        "title": "Test document 2"

      }

    },

    {

      "_id": 5406864699109146628,

      "_score": 2319,

      "_source": {

        "title": "Test document 1"

      }

    }

  ]

}

}
```    

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$search->setIndex("test")->match('Test document')->sort('id','desc','max')->trackScores(true);
```

<!-- intro -->

##### Python:

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.track_scores = true
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
sort = manticoresearch.model.SortMVA('attr_mva', 'desc', 'max')
search_request.sort = [sort]
```

<!-- intro -->

##### Python-asyncio:

<!-- request Python-asyncio -->
``` python
search_request.index = 'test'
search_request.track_scores = true
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
sort = manticoresearch.model.SortMVA('attr_mva', 'desc', 'max')
search_request.sort = [sort]
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.trackScores = true;
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Test document');
sort = new Manticoresearch.SortMVA('attr_mva', 'desc', 'max');
searchRequest.sort = [sort];
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
searchRequest.setIndex("test");
searchRequest.setTrackScores(true);
QueryFilter queryFilter = new QueryFilter();
queryFilter.setQueryString("Test document");
searchRequest.setFulltextFilter(queryFilter);
SortMVA sort = new SortMVA();
sort.setAttr("attr_mva");
sort.setOrder(SortMVA.OrderEnum.DESC);
sort.setMode(SortMVA.ModeEnum.MAX);
searchRequest.setSort(sort);

```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
var searchRequest = new SearchRequest("test");
searchRequest.SetTrackScores(true);
searchRequest.FulltextFilter = new QueryFilter("Test document");
var sort = new SortMVA("attr_mva", SortMVA.OrderEnum.Desc, SortMVA.ModeEnum.Max);
searchRequest.Sort.Add(sort);
```

<!-- intro -->

##### Rust:

<!-- request Rust -->
``` rust
let query = SearchQuery {
    query_string: Some(serde_json::json!("Test document").into()),
    ..Default::default(),
};
let mut sort_mva_opts = HashMap::new();
sort_mva_opts.insert("order".to_string(), "desc".to_string());
sort_mva_opts.insert("mode".to_string(), "max".to_string());
let mut sort_mva = HashMap::new();
sort_mva.insert("attr_mva".to_string(), sort_mva_opts); 

let search_req = SearchRequest {
    table: "test".to_string(),
    query: Some(Box::new(query)),
    sort: Some(serde_json::json!(sort_mva)),
    track_scores: Some(serde_json::json!(true)),
    ..Default::default(),
};
```

<!-- intro -->

##### Typescript:

<!-- request typescript -->
``` typescript
searchRequest = {
  index: 'test',
  track_scores: true,
  query: {
    query_string: {'Test document'},
  },
  sort: { "attr_mva": { "order":"desc", "mode":"max" } },
}
```

<!-- intro -->

##### Go:

<!-- request go -->
```go
searchRequest.SetIndex("test")
searchRequest.SetTrackScores(true)
query := map[string]interface{} {"query_string": "Test document"}
searchRequest.SetQuery(query)
sort := map[string]interface{} { "attr_mva": { "order":"desc", "mode":"max" } }
searchRequest.SetSort(sort)
```

# क्रमबद्ध करना और रैंकिंग

क्वेरी परिणामों को पूर्ण-पाठ रैंकिंग वजन, एक या एक से अधिक गुणों या अभिव्यक्तियों द्वारा क्रमबद्ध किया जा सकता है।

**पूर्ण-पाठ** क्वेरियां डिफ़ॉल्ट रूप से मेल को क्रमबद्ध लौटाती हैं। यदि कुछ निर्दिष्ट नहीं किया गया है, तो उन्हें प्रासंगिकता के अनुसार क्रमबद्ध किया जाता है, जो SQL प्रारूप में `ORDER BY weight() DESC` के समकक्ष है।

**गैर-पूर्ण-पाठ** क्वेरियां डिफ़ॉल्ट रूप से कोई क्रमबद्धता नहीं करती हैं।

## उन्नत क्रमबद्धता

विस्तारित मोड तब स्वचालित रूप से सक्षम होता है जब आप स्पष्ट रूप से SQL प्रारूप में `ORDER BY` खंड जोड़कर या HTTP JSON के माध्यम से `sort` विकल्प का उपयोग करके क्रमबद्धता के नियम प्रदान करते हैं।

### SQL के माध्यम से क्रमबद्धता

सामान्य वाक्यSyntax:
```sql
SELECT ... ORDER BY
{attribute_name | expr_alias | weight() | random() } [ASC | DESC],
...
{attribute_name | expr_alias | weight() | random() } [ASC | DESC]
```

<!-- example alias -->

क्रमबद्धता खंड में, आप 5 स्तंभों तक के किसी भी संयोजन का उपयोग कर सकते हैं, प्रत्येक के बाद `asc` या `desc` होता है। क्रमबद्धता खंड के लिए तर्क के रूप में फ़ंक्शंस और अभिव्यक्तियों की अनुमति नहीं है, केवल `weight()` और `random()` फ़ंक्शंस (जो केवल SQL के माध्यम से `ORDER BY random()` के रूप में उपयोग किया जा सकता है) के अपवाद के साथ। हालाँकि, आप SELECT सूची में कोई भी अभिव्यक्ति उपयोग कर सकते हैं और उसके उपनाम द्वारा क्रमबद्ध कर सकते हैं।

<!-- request SQL -->
```sql
select *, a + b alias from test order by alias desc;
```

<!-- response SQL -->
```
+------+------+------+----------+-------+
| id   | a    | b    | f        | alias |
+------+------+------+----------+-------+
|    1 |    2 |    3 | document |     5 |
+------+------+------+----------+-------+
```

<!-- end -->

## JSON के माध्यम से क्रमबद्धता

<!-- example sorting 1 -->
`"sort"` एक सूची निर्दिष्ट करता है जहाँ प्रत्येक तत्व एक属性 नाम या `_score` हो सकता है यदि आप मेल के वजन द्वारा क्रमबद्ध होना चाहते हैं। इस स्थिति में, गुणों के लिए क्रमबद्धता डिफ़ॉल्ट रूप से आरोही होती है और `_score` के लिए अवरोही होती है।

<!-- intro -->

<!-- request JSON -->

```json
{
  "table":"test",
  "query":
  {
    "match": { "title": "Test document" }
  },
  "sort": [ "_score", "id" ],
  "_source": "title",
  "limit": 3
}
```

<!-- response JSON -->

``` json
    {
	  "took": 0,
	  "timed_out": false,
	  "hits": {
	    "total": 5,
	    "total_relation": "eq",
	    "hits": [
	      {
	        "_id": 5406864699109146628,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 1"
	        }
	      },
	      {
	        "_id": 5406864699109146629,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 2"
	        }
	      },
	      {
	        "_id": 5406864699109146630,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 3"
	        }
	      }
	    ]
	  }
	}
```    

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$search->setIndex("test")->match('Test document')->sort('_score')->sort('id');
```

<!-- intro -->

##### Python:

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
search_request.sort = ['_score', 'id']
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Test document');
searchRequest.sort = ['_score', 'id'];
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
searchRequest.setIndex("test");
QueryFilter queryFilter = new QueryFilter();
queryFilter.setQueryString("Test document");
searchRequest.setFulltextFilter(queryFilter);
List<Object> sort = new ArrayList<Object>( Arrays.asList("_score", "id") );
searchRequest.setSort(sort);

```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
var searchRequest = new SearchRequest("test");
searchRequest.FulltextFilter = new QueryFilter("Test document");
searchRequest.Sort = new List<Object> {"_score", "id"};

```

<!-- intro -->

##### Typescript:

<!-- request typescript -->
``` typescript
searchRequest = {
  index: 'test',
  query: {
    query_string: {'Test document'},
  },
  sort: ['_score', 'id'],
}
```

<!-- intro -->

##### Go:

<!-- request go -->
```go
searchRequest.SetIndex("test")
query := map[string]interface{} {"query_string": "Test document"}
searchRequest.SetQuery(query)
sort := map[string]interface{} {"_score": "asc", "id": "asc"}
searchRequest.SetSort(sort)
```

<!-- end -->

<!-- example sorting 2 -->
आप क्रमबद्धता के क्रम को स्पष्ट रूप से निर्दिष्ट भी कर सकते हैं:

* `asc`: आरोही क्रम में क्रमबद्ध करें
* `desc`: अवरोही क्रम में क्रमबद्ध करें


<!-- intro -->

<!-- request JSON -->

```json
{
  "table":"test",
  "query":
  {
    "match": { "title": "Test document" }
  },
  "sort": 
  [
    { "id": "desc" },
    "_score"
  ],
  "_source": "title",
  "limit": 3
}
```

<!-- response JSON -->

``` json
	{
	  "took": 0,
	  "timed_out": false,
	  "hits": {
	    "total": 5,
	    "total_relation": "eq",
	    "hits": [
	      {
	        "_id": 5406864699109146632,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 5"
	        }
	      },
	      {
	        "_id": 5406864699109146631,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 4"
	        }
	      },
	      {
	        "_id": 5406864699109146630,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 3"
	        }
	      }
	    ]
	  }
	}
```    

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$search->setIndex("test")->match('Test document')->sort('id', 'desc')->sort('_score');
```

<!-- intro -->

##### Python:

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
sort_by_id = manticoresearch.model.SortOrder('id', 'desc')
search_request.sort = [sort_by_id, '_score']
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Test document');
sortById = new Manticoresearch.SortOrder('id', 'desc');
searchRequest.sort = [sortById, 'id'];
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
searchRequest.setIndex("test");
QueryFilter queryFilter = new QueryFilter();
queryFilter.setQueryString("Test document");
searchRequest.setFulltextFilter(queryFilter);
List<Object> sort = new ArrayList<Object>();
SortOrder sortById = new SortOrder();
sortById.setAttr("id");
sortById.setOrder(SortOrder.OrderEnum.DESC);
sort.add(sortById);
sort.add("_score");
searchRequest.setSort(sort);

```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
var searchRequest = new SearchRequest("test");
searchRequest.FulltextFilter = new QueryFilter("Test document");
searchRequest.Sort = new List<Object>();
var sortById = new SortOrder("id", SortOrder.OrderEnum.Desc);
searchRequest.Sort.Add(sortById);
searchRequest.Sort.Add("_score");

```

<!-- intro -->

##### Typescript:

<!-- request typescript -->
``` typescript
searchRequest = {
  index: 'test',
  query: {
    query_string: {'Test document'},
  },
  sort: [{'id': 'desc'}, '_score'],
}
```

<!-- intro -->

##### Go:

<!-- request go -->
```go
searchRequest.SetIndex("test")
query := map[string]interface{} {"query_string": "Test document"}
searchRequest.SetQuery(query)
sortById := map[string]interface{} {"id": "desc"}
sort := map[string]interface{} {"id": "desc", "_score": "asc"}
searchRequest.SetSort(sort)
```

<!-- end -->

<!-- example sorting 3 -->
आप एक और वाक्यविन्यास का उपयोग कर सकते हैं और `order` संपत्ति के माध्यम से सॉर्ट क्रम निर्दिष्ट कर सकते हैं:

<!-- intro -->

<!-- request JSON -->

```json
{
  "table":"test",
  "query":
  {
    "match": { "title": "Test document" }
  },
  "sort":
  [
    { "id": { "order":"desc" } }
  ],
  "_source": "title",
  "limit": 3
}
```

<!-- response JSON -->

``` json
	{
	  "took": 0,
	  "timed_out": false,
	  "hits": {
	    "total": 5,
	    "total_relation": "eq",
	    "hits": [
	      {
	        "_id": 5406864699109146632,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 5"
	        }
	      },
	      {
	        "_id": 5406864699109146631,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 4"
	        }
	      },
	      {
	        "_id": 5406864699109146630,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 3"
	        }
	      }
	    ]
	  }
	}
```    

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$search->setIndex("test")->match('Test document')->sort('id', 'desc');
```

<!-- intro -->

##### Python:

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
sort_by_id = manticoresearch.model.SortOrder('id', 'desc')
search_request.sort = [sort_by_id]
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Test document');
sortById = new Manticoresearch.SortOrder('id', 'desc');
searchRequest.sort = [sortById];
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
searchRequest.setIndex("test");
QueryFilter queryFilter = new QueryFilter();
queryFilter.setQueryString("Test document");
searchRequest.setFulltextFilter(queryFilter);
List<Object> sort = new ArrayList<Object>();
SortOrder sortById = new SortOrder();
sortById.setAttr("id");
sortById.setOrder(SortOrder.OrderEnum.DESC);
sort.add(sortById);
searchRequest.setSort(sort);

```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
var searchRequest = new SearchRequest("test");
searchRequest.FulltextFilter = new QueryFilter("Test document");
searchRequest.Sort = new List<Object>();
var sortById = new SortOrder("id", SortOrder.OrderEnum.Desc);
searchRequest.Sort.Add(sortById);

```

<!-- intro -->

##### Typescript:

<!-- request typescript -->
``` typescript
searchRequest = {
  index: 'test',
  query: {
    query_string: {'Test document'},
  },
  sort: { {'id': {'order':'desc'} },
}
```

<!-- intro -->

##### Go:

<!-- request go -->
```go
searchRequest.SetIndex("test")
query := map[string]interface{} {"query_string": "Test document"}
searchRequest.SetQuery(query)
sort := map[string]interface{} { "id": {"order":"desc"} }
searchRequest.SetSort(sort)
```

<!-- end -->

<!-- example sorting 4 -->
JSON क्वेरी में MVA विशेषताओं द्वारा सॉर्टिंग भी समर्थित है। `mode` संपत्ति के माध्यम से सॉर्टिंग मोड सेट किया जा सकता है। निम्नलिखित मोड समर्थित हैं:

* `min`: न्यूनतम मूल्य द्वारा सॉर्ट करें
* `max`: अधिकतम मूल्य द्वारा सॉर्ट करें

<!-- intro -->

<!-- request JSON -->

```json
{
  "table":"test",
  "query":
  {
    "match": { "title": "Test document" }
  },
  "sort":
  [
    { "attr_mva": { "order":"desc", "mode":"max" } }
  ],
  "_source": "title",
  "limit": 3
}
```

<!-- response JSON -->

``` json
	{
	  "took": 0,
	  "timed_out": false,
	  "hits": {
	    "total": 5,
	    "total_relation": "eq",
	    "hits": [
	      {
	        "_id": 5406864699109146631,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 4"
	        }
	      },
	      {
	        "_id": 5406864699109146629,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 2"
	        }
	      },
	      {
	        "_id": 5406864699109146628,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 1"
	        }
	      }
	    ]
	  }
	}
```    

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$search->setIndex("test")->match('Test document')->sort('id','desc','max');
```

<!-- intro -->

##### पायथन:

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
sort = manticoresearch.model.SortMVA('attr_mva', 'desc', 'max')
search_request.sort = [sort]
```

<!-- intro -->

##### जावास्क्रिप्ट:

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Test document');
sort = new Manticoresearch.SortMVA('attr_mva', 'desc', 'max');
searchRequest.sort = [sort];
```

<!-- intro -->

##### जावा:

<!-- request Java -->
``` java
searchRequest.setIndex("test");
QueryFilter queryFilter = new QueryFilter();
queryFilter.setQueryString("Test document");
searchRequest.setFulltextFilter(queryFilter);
SortMVA sort = new SortMVA();
sort.setAttr("attr_mva");
sort.setOrder(SortMVA.OrderEnum.DESC);
sort.setMode(SortMVA.ModeEnum.MAX);
searchRequest.setSort(sort);

```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
var searchRequest = new SearchRequest("test");
searchRequest.FulltextFilter = new QueryFilter("Test document");
var sort = new SortMVA("attr_mva", SortMVA.OrderEnum.Desc, SortMVA.ModeEnum.Max);
searchRequest.Sort.Add(sort);
```

<!-- intro -->

##### टाइपस्क्रिप्ट:

<!-- request typescript -->
``` typescript
searchRequest = {
  index: 'test',
  query: {
    query_string: {'Test document'},
  },
  sort: { "attr_mva": { "order":"desc", "mode":"max" } },
}
```

<!-- intro -->

##### गो:

<!-- request go -->
```go
searchRequest.SetIndex("test")
query := map[string]interface{} {"query_string": "Test document"}
searchRequest.SetQuery(query)
sort := map[string]interface{} { "attr_mva": { "order":"desc", "mode":"max" } }
searchRequest.SetSort(sort)
```

<!-- end -->

<!-- example sorting 5 -->
When sorting on an attribute, match weight (score) calculation is disabled by default (no ranker is used). You can enable weight calculation by setting the `track_scores` property to `true`:

<!-- intro -->

<!-- request JSON -->

```json
{
  "table":"test",
  "track_scores": true,
  "query":
  {
    "match": { "title": "Test document" }
  },
  "sort":
  [
    { "attr_mva": { "order":"desc", "mode":"max" } }
  ],
  "_source": "title",
  "limit": 3
}
```

<!-- response JSON -->

``` json
	{
	  "took": 0,
	  "timed_out": false,
	  "hits": {
	    "total": 5,
	    "total_relation": "eq",
	    "hits": [
	      {
	        "_id": 5406864699109146631,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 4"
	        }
	      },
	      {
	        "_id": 5406864699109146629,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 2"
	        }
	      },
	      {
	        "_id": 5406864699109146628,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 1"
	        }
	      }
	    ]
	  }
	}
```    

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$search->setIndex("test")->match('Test document')->sort('id','desc','max')->trackScores(true);
```

<!-- intro -->

##### पायथन:

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.track_scores = true
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
sort = manticoresearch.model.SortMVA('attr_mva', 'desc', 'max')
search_request.sort = [sort]
```

<!-- intro -->

##### जावास्क्रिप्ट:

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.trackScores = true;
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Test document');
sort = new Manticoresearch.SortMVA('attr_mva', 'desc', 'max');
searchRequest.sort = [sort];
```

<!-- intro -->

##### जावा:

<!-- request Java -->
``` java
searchRequest.setIndex("test");
searchRequest.setTrackScores(true);
QueryFilter queryFilter = new QueryFilter();
queryFilter.setQueryString("Test document");
searchRequest.setFulltextFilter(queryFilter);
SortMVA sort = new SortMVA();
sort.setAttr("attr_mva");
sort.setOrder(SortMVA.OrderEnum.DESC);
sort.setMode(SortMVA.ModeEnum.MAX);
searchRequest.setSort(sort);

```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
var searchRequest = new SearchRequest("test");
searchRequest.SetTrackScores(true);
searchRequest.FulltextFilter = new QueryFilter("Test document");
var sort = new SortMVA("attr_mva", SortMVA.OrderEnum.Desc, SortMVA.ModeEnum.Max);
searchRequest.Sort.Add(sort);
```

<!-- intro -->

##### टाइपस्क्रिप्ट:
<!-- request typescript -->
``` typescript
searchRequest = {
  index: 'test',
  track_scores: true,
  query: {
    query_string: {'Test document'},
  },
  sort: { "attr_mva": { "order":"desc", "mode":"max" } },
}
```

<!-- intro -->

##### जाएं:

<!-- request go -->
```go
searchRequest.SetIndex("test")
searchRequest.SetTrackScores(true)
query := map[string]interface{} {"query_string": "Test document"}
searchRequest.SetQuery(query)
sort := map[string]interface{} { "attr_mva": { "order":"desc", "mode":"max" } }
searchRequest.SetSort(sort)
```

<!-- end -->


## रैंकिंग अवलोकन

खोज परिणामों की रैंकिंग (जिसे वेटिंग भी कहा जाता है) को एक प्रक्रिया के रूप में परिभाषित किया जा सकता है जो एक दिए गए मेल खाए हुए दस्तावेज़ के लिए एक तरह के प्रासंगिकता (वजन) की गणना करता है जिसे एक दिए गए प्रश्न के संबंध में मेल खाता है। इसलिए प्रासंगिकता, अंत में, बस एक संख्या है जो हर दस्तावेज़ से जुड़ी होती है जो यह अनुमान लगाती है कि दस्तावेज़ प्रश्न के लिए कितना प्रासंगिक है। खोज परिणामों को फिर इस संख्या के आधार पर और/या कुछ अतिरिक्त पैरामीटर के आधार पर छांटा जा सकता है, ताकि सबसे अधिक खोजे गए परिणाम परिणाम पृष्ठ पर उच्चतर दिखाई दें।

किसी भी स्थिति में किसी भी दस्तावेज़ को रैंक करने के लिए एकल मानक एक-आकार-का-फिट नहीं है। इसके अलावा, कभी भी ऐसा तरीका नहीं हो सकता, क्योंकि प्रासंगिकता *विषयात्मक* है। जैसे, जो आपको प्रासंगिक लगता है वो मुझे प्रासंगिक नहीं लग सकता। इसलिए, सामान्य मामलों में, यह केवल गणना करने में मुश्किल नहीं है; यह सिद्धांत रूप से असंभव है।

इसलिए Manticore में रैंकिंग कॉन्फ़िगर करने योग्य है। इसमें एक तथाकथित **रैंकर्स** का अर्थ है। एक रैंकर्स को औपचारिक रूप से एक फ़ंक्शन के रूप में परिभाषित किया जा सकता है जो एक दस्तावेज़ और एक प्रश्न को अपने इनपुट के रूप में लेता है और आउटपुट के रूप में एक प्रासंगिकता मूल्य उत्पन्न करता है। आम शब्दों में, एक रैंकर्स सटीक रूप से यह नियंत्रित करता है कि कैसे (किस विशिष्ट एल्गोरिदम का उपयोग करके) Manticore दस्तावेज़ों को वजन देगा।

## उपलब्ध अंतर्निर्मित रैंकर्स

Manticore में विभिन्न उद्देश्यों के लिए उपयुक्त कई अंतर्निर्मित रैंकर्स शामिल हैं। उनमें से कई दो कारकों का उपयोग करते हैं: वाक्यांश निकटता (जिसे LCS भी कहा जाता है) और BM25। वाक्यांश निकटता कीवर्ड स्थानों पर काम करती है, जबकि BM25 कीवर्ड आवृत्तियों पर काम करता है। मौलिक रूप से, दस्तावेज़ शरीर और प्रश्न के बीच वाक्यांश मेल के लिए जितना अच्छा डिग्री होगा, वाक्यांश निकटता उतनी ही अधिक होगी (यह अधिकतम हो जाती है जब दस्तावेज़ पूरे प्रश्न को एक सटीक उद्धरण के रूप में शामिल करता है)। और BM25 अधिक होता है जब दस्तावेज़ में अधिक दुर्लभ शब्द होते हैं। हम इसके विस्तृत चर्चा को बाद में सुरक्षित रखेंगे।

वर्तमान में लागू रैंकर्स हैं:

* `proximity_bm25`, डिफ़ॉल्ट रैंकिंग मोड जो वाक्यांश निकटता और BM25 रैंकिंग दोनों का उपयोग और संयोजन करता है।
* `bm25`, एक सांख्यिकीय रैंकिंग मोड जो केवल BM25 रैंकिंग का उपयोग करता है (जो अधिकांश अन्य पूर्ण-पाठ इंजनों के समान है)। यह मोड तेजी से होता है लेकिन एक से अधिक कीवर्ड वाले प्रश्नों के लिए गुणवत्ता में कमी का कारण बन सकता है।
* `none`, कोई रैंकिंग मोड। यह मोड स्पष्ट रूप से सबसे तेज़ है। सभी मेल पर 1 का वजन लगाया जाता है। इसे कभी-कभी बूलियन खोज कहा जाता है, जो केवल दस्तावेज़ों का मेल खाती है लेकिन उन्हें रैंक नहीं करती है।
* `wordcount`, कीवर्ड कीOccurrences की गणना द्वारा रैंकिंग। यह रैंकर्स फ़ील्ड-प्रतिशत कीवर्ड कीOccurrences की गणना करता है, फिर उन्हें फ़ील्ड वजन द्वारा गुणा करता है, और परिणामस्वरूप मानों को जोड़ता है।
* `proximity` परिणाम के रूप में कच्चे वाक्यांश निकटता मूल्य को लौटाता है। इस मोड का आंतरिक उपयोग `SPH_MATCH_ALL` प्रश्नों की नकल के लिए किया जाता है।
* `matchany` रैंक को लौटाता है जैसा कि पहले `SPH_MATCH_ANY` मोड में गणना की गई थी और आंतरिक रूप से `SPH_MATCH_ANY` प्रश्नों की नकल के लिए उपयोग किया जाता है।
* `fieldmask` एक 32-बिट मास्क लौटाता है जिसमें N-था बिट उस N-थे पूर्ण-पाठ फ़ील्ड के लिए है, जो 0 से संख्या करता है। बिट तब ही सेट होगा जब संबंधित फ़ील्ड में कोई कीवर्ड कीOccurrences हो जो प्रश्न को संतुष्ट करता हो।
* `sph04` सामान्यतः डिफ़ॉल्ट 'proximity_bm25' रैंकर्स पर आधारित है, लेकिन जब वे एक टेक्स्ट फ़ील्ड की बहुत शुरुआत या बहुत अंत में होते हैं, तो मेल को अतिरिक्त रूप से बूस्ट करता है। इसलिए, यदि कोई फ़ील्ड एकदम प्रश्न के बराबर है, तो `sph04` इसे उस फ़ील्ड की तुलना में अधिक रैंक करेगा जो प्रश्न के समान है लेकिन उसके बराबर नहीं है। (उदाहरण के लिए, जब प्रश्न "Hyde Park" है, तो "Hyde Park" शीर्षक वाले दस्तावेज़ को "Hyde Park, London" या "The Hyde Park Cafe" के शीर्षक वाले एक से अधिक रैंक किया जाना चाहिए।)
* `expr` आपको रनटाइम पर रैंकिंग फ़ॉर्मूला निर्दिष्ट करने की अनुमति देता है। यह कई आंतरिक टेक्स्ट कारकों को उजागर करता है और आपको परिभाषित करने देता है कि अंतिम वजन उन कारकों से कैसे गणना की जानी चाहिए। आप इसकी सिंटैक्स और उपलब्ध कारकों का संदर्भ [नीचे के उपखंड](../Searching/Sorting_and_ranking.md#Quick-summary-of-the-ranking-factors) में प्राप्त कर सकते हैं।

रैंकर्स का नाम केस-संवेदनशील नहीं है। उदाहरण:

```sql
SELECT ... OPTION ranker=sph04;
```

## रैंकिंग कारकों का त्वरित सारांश

| नाम                    | स्तर     | प्रकार  | सारांश                                                                                                            |
| ----------------------- | --------- | ----- | ------------------------------------------------------------------------------------------------------------------ |
| max_lcs                 | प्रश्न     | int   | वर्तमान प्रश्न के लिए अधिकतम संभावित LCS मान                                                                   |
| bm25                    | दस्तावेज़  | int   | BM25(1.2, 0) का त्वरित अनुमान                                                                                     |
| bm25a(k1, b)            | दस्तावेज़  | int   | कॉन्फ़िगर करने योग्य K1, B स्थिरांक और सिंटैक्स समर्थन के साथ सटीक BM25() मान                                         |
| bm25f(k1, b, {field=weight, ...}) | दस्तावेज़ | int   | अतिरिक्त कॉन्फ़िगर करने योग्य फ़ील्ड वजन के साथ सटीक BM25F() मान                                                       |
| field_mask              | दस्तावेज़  | int   | मेल खाते क्षेत्रों का बिट मास्क                                                                                         |
| query_word_count        | दस्तावेज़  | int   | प्रश्न में अद्वितीय समग्र कीवर्ड की संख्या                                                                     |
| doc_word_count          | दस्तावेज़  | int   | दस्तावेज़ में मेल खा रहे अद्वितीय कीवर्ड की संख्या                                                                   |
| lcs                     | क्षेत्र     | int   | क्वेरी और दस्तावेज़ के बीच सबसे लंबी सामान्य उपअनुक्रम, शब्दों में                                                 |
| user_weight             | क्षेत्र     | int   | उपयोगकर्ता क्षेत्र वजन                                                                                                 |
| hit_count               | क्षेत्र     | int   | कीवर्ड की कुल बार घटनाएँ                                                                                             |
| word_count              | क्षेत्र     | int   | मेल खा रहे अद्वितीय कीवर्ड की संख्या                                                                                 |
| tf_idf                  | क्षेत्र     | float | मेल खा रहे कीवर्ड पर sum(tf*idf) == घटनाओं पर sum(idf)                                                          |
| min_hit_pos             | क्षेत्र     | int   | पहली मेल खा रही घटना का स्थान, शब्दों में, 1-आधारित                                                              |
| min_best_span_pos       | क्षेत्र     | int   | पहले अधिकतम LCS स्पैन का स्थान, शब्दों में, 1-आधारित                                                               |
| exact_hit               | क्षेत्र     | bool  | क्या क्वेरी == क्षेत्र                                                                                               |
| min_idf                 | क्षेत्र     | float | मेल खा रहे कीवर्ड पर min(idf)                                                                                      |
| max_idf                 | क्षेत्र     | float | मेल खा रहे कीवर्ड पर max(idf)                                                                                      |
| sum_idf                 | क्षेत्र     | float | मेल खा रहे कीवर्ड पर sum(idf)                                                                                      |
| exact_order             | क्षेत्र     | bool  | क्या सभी क्वेरी कीवर्ड a) मेल खा रहे हैं और b) क्वेरी क्रम में हैं                                               |
| min_gaps                | क्षेत्र     | int   | मेल खा रहे कीवर्ड के बीच न्यूनतम गैप्स की संख्या                                                                    |
| lccs                    | क्षेत्र     | int   | क्वेरी और दस्तावेज़ के बीच सबसे लंबी सामान्य सन्निकट उपअनुक्रम, शब्दों में                                         |
| wlccs                   | क्षेत्र     | float | वेटेड लॉन्गेस्ट कॉमन कंटिग्यूअस सबसिक्वेंस, सन्निकट कीवर्ड स्पैन पर sum(idf)                                     |
| atc                     | क्षेत्र     | float | एग्रीगेट टर्म क्लोज़नेस, log(1+sum(idf1*idf2*pow(distance, -1.75)) सर्वोत्तम कीवर्ड जोड़ों पर                      |

### दस्तावेज़ स्तर रैंकिंग कारक

एक **दस्तावेज़ स्तर का कारक** एक संख्या है जिसे रैंकिंग इंजन द्वारा हर मेल खा रहे दस्तावेज़ के लिए वर्तमान क्वेरी के संबंध में गणना की जाती है। इसलिए यह एक साधारण दस्तावेज़ विशेषता से भिन्न है क्योंकि विशेषता पूर्ण टेक्स्ट क्वेरी पर निर्भर नहीं करती, जबकि कारक ऐसा कर सकते हैं। इन कारकों का उपयोग रैंकिंग अभिव्यक्ति में कहीं भी किया जा सकता है। वर्तमान में लागू किए गए दस्तावेज़ स्तर के कारक हैं:

* `bm25` (integer), एक दस्तावेज़ स्तर का BM25 अनुमान (कीवर्ड घटना फ़िल्टरिंग किए बिना गणना की गई)।
* `max_lcs` (integer), एक क्वेरी-स्तर का अधिकतम संभव मूल्य जो `sum(lcs*user_weight)` अभिव्यक्ति ले सकती है। यह वजन वृद्धि स्केलिंग के लिए उपयोगी हो सकता है। उदाहरण के लिए, `MATCHANY` रैंकिंग सूत्र इसका उपयोग करता है यह सुनिश्चित करने के लिए कि किसी भी क्षेत्र में पूर्ण वाक्यांश मेल सभी क्षेत्रों में आंशिक मेल के किसी भी संयोजन की तुलना में उच्च रैंक करता है।
* `field_mask` (integer), मेल खा रहे क्षेत्रों के लिए एक दस्तावेज़ स्तर का 32-बिट मास्क।
* `query_word_count` (integer), एक क्वेरी में अद्वितीय कीवर्ड की संख्या, बाहर किए गए कीवर्ड की संख्या के लिए समायोजित। उदाहरण के लिए, `(one one one one)` और `(one !two)` दोनों क्वेरियों को इस कारक को 1 का मान सौंपना चाहिए, क्योंकि यहां केवल एक अद्वितीय नॉन-एक्सक्लूडेड कीवर्ड है।
* `doc_word_count` (integer), पूरे दस्तावेज़ में मेल खा रहे अद्वितीय कीवर्ड की संख्या।

### क्षेत्र स्तर रैंकिंग कारक

एक **क्षेत्र स्तर का कारक** एक संख्या है जिसे रैंकिंग इंजन द्वारा हर मेल खा रहे दस्तावेज़ टेक्स्ट क्षेत्र के लिए वर्तमान क्वेरी के संबंध में गणना की जाती है। चूंकि एक क्वेरी द्वारा एक से अधिक क्षेत्र मेल खा सकते हैं, लेकिन अंतिम वजन एकल पूर्णांक मान होना आवश्यक है, इसलिए इन मूल्यों को एक एकल मूल्य में समाहित करना आवश्यक है। ऐसा करने के लिए, क्षेत्र स्तर के कारक केवल एक क्षेत्र समेकन कार्य के भीतर ही उपयोग किए जा सकते हैं, इन्हें अभिव्यक्ति में कहीं भी उपयोग नहीं किया जा सकता। उदाहरण के लिए, आप `(lcs+bm25)` को अपनी रैंकिंग अभिव्यक्ति के रूप में उपयोग नहीं कर सकते, क्योंकि `lcs` कई मान लेता है (हर मेल खा रहे क्षेत्र में एक)। आपको इसके बजाय `(sum(lcs)+bm25)` का उपयोग करना चाहिए, यह अभिव्यक्ति सभी मेल खा रहे क्षेत्रों पर `lcs` को जोड़ती है, और फिर उस प्रति-क्षेत्र योग में `bm25` जोड़ती है। वर्तमान में लागू किए गए क्षेत्र स्तर के कारक हैं:

* `lcs` (integer), दस्तावेज़ और क्वेरी के बीच अधिकतम शुद्ध मिलान की लंबाई, जो शब्दों में गिनी जाती है। LCS का अर्थ है लॉन्गेस्ट कॉमन सबसिक्वेंस (या सबसेट)। जब केवल बिखरे हुए कीवर्ड किसी फ़ील्ड में मिलते हैं, तो इसका न्यूनतम मान 1 होता है, और जब संपूर्ण क्वेरी किसी फ़ील्ड में शुद्ध रूप से मिलती है (सही क्वेरी कीवर्ड क्रम में), तो इसका अधिकतम मान क्वेरी कीवर्ड की संख्या होती है। उदाहरण के लिए, यदि क्वेरी 'hello world' है और फ़ील्ड इन दो शब्दों को शामिल करता है जो क्वेरी से उद्धृत हैं (यानी, एक-दूसरे के निकट और ठीक क्वेरी क्रम में), तो `lcs` 2 होगा। उदाहरण के लिए, यदि क्वेरी 'hello world program' है और फ़ील्ड में 'hello world' है, तो `lcs` 2 होगा। ध्यान दें कि क्वेरी कीवर्ड का कोई भी उपसमुच्चय काम करता है, न कि केवल निकटवर्ती कीवर्ड का उपसमुच्चय। उदाहरण के लिए, यदि क्वेरी 'hello world program' है और फ़ील्ड में 'hello (test program)' है, तो `lcs` भी 2 होगा, क्योंकि दोनों 'hello' और 'program' ने क्वेरी में अपने संबंधित स्थानों पर मेल खाया। अंततः, यदि क्वेरी 'hello world program' है और फ़ील्ड में 'hello world program' है, तो `lcs` 3 होगा। (उम्मीद है कि इस बिंदु पर यह अप्रत्याशित नहीं है।)
* `user_weight` (integer), उपयोगकर्ता द्वारा निर्दिष्ट प्रति-फ़ील्ड वजन (SQL में [OPTION field_weights](../Searching/Options.md#field_weights) देखें)। यदि स्पष्ट रूप से निर्दिष्ट नहीं किया गया है तो वजन डिफ़ॉल्ट रूप से 1 होते हैं।
* `hit_count` (integer), उस फ़ील्ड में कीवर्ड की मौखिकता संख्या जो मेल खाती है। ध्यान दें कि एक ही कीवर्ड कई बार हो सकता है। उदाहरण के लिए, यदि 'hello' फ़ील्ड में 3 बार होता है और 'world' 5 बार होता है, तो `hit_count` 8 होगा।
* `word_count` (integer), उस फ़ील्ड में मेल खाने वाले अनोखे कीवर्ड की संख्या। उदाहरण के लिए, यदि 'hello' और 'world' कहीं भी एक फ़ील्ड में होती हैं, तो `word_count` 2 होगा, चाहे दोनों कीवर्ड कितनी भी बार हो।
* `tf_idf` (float), उस फ़ील्ड में मेल खा रहे सभी कीवर्ड पर TF/IDF का योग। IDF इनवर्स डॉक्यूमेंट फ़्रीक्वेंसी है, 0 और 1 के बीच का एक फ्लोटिंग पॉइंट मान जो बताता है कि कीवर्ड कितनी बार होता है (आम तौर पर, 0 एक कीवर्ड के लिए जो हर इंडेक्स किए गए दस्तावेज़ में होता है, और 1 एक अद्वितीय कीवर्ड के लिए जो केवल एक ही दस्तावेज़ में होता है)। TF टर्म फ़्रीक्वेंसी है, फ़ील्ड में मेल खा रहे कीवर्ड की मौखिकता की संख्या। एक पक्ष नोट के रूप में, `tf_idf` वास्तव में सभी मेल खाती मौलिकताओं पर IDF को जोड़कर गणना की जाती है। यह मूल रूप से TF*IDF के सभी मेल खाते कीवर्ड पर योग जोड़ने के बराबर है।
* `min_hit_pos` (integer), पहले मेल खा रहे कीवर्ड की मौखिकता की स्थिति, जो शब्दों में गिनी जाती है

   इसलिए, यह एक अपेक्षाकृत निम्न-स्तरीय, "कच्चा" कारक है जिसे आप रैंकिंग के लिए उपयोग करने से पहले *समायोजित* करना चाहेंगे। विशिष्ट समायोजन आपके डेटा और परिणामी फ़ॉर्मूला पर बहुत निर्भर करते हैं, लेकिन यहाँ कुछ विचार दिए गए हैं जिनसे आप शुरू कर सकते हैं: (क) किसी भी min_gaps-आधारित बूस्ट को जब word_count<2 हो तब सीधे नजरअंदाज किया जा सकता है;

    (ख) गैर-तुच्छ min_gaps मान (यानी, जब word_count>=2) एक निश्चित "सबसे खराब-केस" स्थिरांक के साथ सीमित किया जा सकता है, जबकि तुच्छ मान (यानी, जब min_gaps=0 और word_count<2) उस स्थिरांक द्वारा प्रतिस्थापित किए जा सकते हैं;

    (ग) 1/(1+min_gaps) जैसे एक ट्रांसफर फ़ंक्शन को लागू किया जा सकता है (ताकि बेहतर, छोटे min_gaps मान इसे अधिकतम कर सकें, और खराब, बड़े min_gaps मान धीरे-धीरे कम हो जाएं); और इसी तरह।
* `lccs` (integer)। लॉन्गेस्ट कॉमन कंटिगुअस सबसिक्वेंस। क्वेरी और दस्तावेज़ के बीच सबसे लंबे उपवाक्यांश की लंबाई, जो कीवर्ड में गणना की गई है।

    LCCS कारक LCS के समान है लेकिन अधिकrestrictive है। जबकि LCS 1 से बड़ा हो सकता है अगर कोई दो क्वेरी शब्द एक-दूसरे के निकट मेल नहीं खाते हैं, LCCS केवल तब 1 से बड़ा होगा जब दस्तावेज़ में *सटीक*, सन्निहित क्वेरी उपवाक्यांश हों। उदाहरण के लिए, (one two three four five) क्वेरी बनाम (one hundred three hundred five hundred) दस्तावेज़ का lcs=3 होगा, लेकिन lccs=1, क्योंकि हालांकि तीन कीवर्ड (one, three, five) के आपसी आसन्न स्थान क्वेरी और दस्तावेज़ के बीच मेल खाते हैं, लेकिन वास्तव में कोई 2 मेल खाने वाले स्थान आसन्न नहीं हैं।

    ध्यान दें कि LCCS अब भी अक्सर और दुर्लभ कीवर्ड के बीच में अंतर नहीं करता है; इसके लिए, WLCCS देखें।
* `wlccs` (float)। वेटेड लॉन्गेस्ट कॉमन कंटिगुअस सबसिक्वेंस। क्वेरी और दस्तावेज़ के बीच सबसे लंबे उपवाक्यांश के कीवर्ड के IDFs का योग।

    WLCCS का गणना LCCS के समान किया जाता है, लेकिन हर "उपयुक्त" कीवर्ड मौखिकता इसे कीवर्ड IDF द्वारा 1 के स्थान पर बढ़ाती है (जैसे LCS और LCCS के साथ)। इससे दुर्लभ और अधिक महत्वपूर्ण कीवर्ड की रैंकिंग उच्च हो जाती है, भले ही बाद वाले लंबे हों। उदाहरण के लिए, एक क्वेरी `(Zanzibar bed and breakfast)` को एक `(hotels of Zanzibar)` दस्तावेज़ के लिए lccs=1 मिलेगा, लेकिन `(London bed and breakfast)` के खिलाफ lccs=3, भले ही "Zanzibar" वास्तव में "bed and breakfast" वाक्यांश की तुलना में कुछ हद तक दुर्लभ है। WLCCS कारक इस समस्या को कीवर्ड आवृत्तियों का उपयोग करके संबोधित करता है।
* `atc` (float)। एग्रीगेट टर्म क्लोज़नेस। एक निकटता-आधारित माप जो तब बढ़ता है जब दस्तावेज़ में निकटता में अधिक महत्वपूर्ण (दुर्लभ) क्वेरी कीवर्ड के अधिक समूह होते हैं।

    **WARNING:** आपको OPTION idf='plain,tfidf_unnormalized' के साथ ATC का उपयोग करना चाहिए (देखें [नीचे](../Searching/Sorting_and_ranking.md#Configuration-of-IDF-formula)); अन्यथा, आपको अप्रत्याशित परिणाम मिल सकते हैं।

    ATC मूल रूप से इस प्रकार कार्य करता है। दस्तावेज़ में हर कीवर्ड *मौखिकता* के लिए, हम所谓的 *term closeness* की गणना करते हैं। ऐसा करने के लिए, हम सभी अन्य निकटतम मौखिकताओं की जांच करते हैं सभी क्वेरी कीवर्ड (कीवर्ड स्वयं सहित) के विषय मौखिकता के बाएं और दाएं, एक दूरता dampening गुणांक की गणना करते हैं जैसे k = pow(distance, -1.75) इन मौखिकताओं के लिए, और dampened IDFs का योग करते हैं। परिणामस्वरूप, प्रत्येक कीवर्ड की प्रत्येक मौखिकता के लिए, हमें एक "निकटता" मान मिलता है जो उस मौखिकता के "पड़ोसी" को वर्णित करता है। फिर हम इन प्रत्येक मौखिकता की निकटता को उनके संबंधित विषय कीवर्ड IDF द्वारा गुणा करते हैं, उन्हें सभी जोड़ते हैं, और अंततः उस योग का लॉगरिदम गणना करते हैं।
दूसरे शब्दों में, हम दस्तावेज़ में सबसे अच्छे (निकटतम) मेल खाते कीवर्ड जोड़ी को संसाधित करते हैं, और उनके IDFs के उत्पाद के रूप में जोड़ी "निकटता" की गणना करते हैं, जिसे दूरी गुणांक द्वारा स्केल किया जाता है:

```
pair_tc = idf(pair_word1) * idf(pair_word2) * pow(pair_distance, -1.75)
```

फिर हम ऐसी निकटता को जोड़ते हैं, और अंतिम, लॉग-डैम्पेंड ATC मान की गणना करते हैं:

```
atc = log(1+sum(pair_tc))
```

ध्यान दें कि यह अंतिम डैम्पिंग लॉगारिदम ठीक वही कारण है जिसके लिए आपको OPTION idf=plain का उपयोग करना चाहिए, क्योंकि इसके बिना, log() के अंदर का एक्सप्रेशन नकारात्मक हो सकता है।

निकटतम कीवर्ड की उपस्थिति ATC में अधिक योगदान देती है *बहुत* अधिक अधिकतम कीवर्ड की तुलना में। वास्तव में, जब कीवर्ड एक-दूसरे के बगल में होते हैं, तो दूरी=1 और k=1 होती है; जब उनके बीच केवल एक शब्द होता है, तो दूरी=2 और k=0.297 होती है, दो शब्दों के बीच, दूरी=3 और k=0.146 होती है, और इसी तरह। उसी समय, IDF कुछ धीमी गति से कमजोर होता है। उदाहरण के लिए, 1 मिलियन दस्तावेज़ संग्रह में, 10, 100 और 1000 दस्तावेज़ों में मेल खाने वाले कीवर्ड के लिए IDF मान क्रमशः 0.833, 0.667 और 0.500 होंगे। इसलिए, एक कीवर्ड जोड़ी जिसमें दो काफी दुर्लभ कीवर्ड होते हैं जो केवल 10 दस्तावेज़ों में होते हैं लेकिन उनके बीच 2 अन्य शब्द होते हैं, वह pair_tc = 0.101 देती है और इस प्रकार एक 100-doc और 1000-doc कीवर्ड के साथ केवल 1 अन्य शब्द के साथ एक जोड़ी को केवल थोड़ा अधिक बनाती है और pair_tc = 0.099। इसके अलावा, दो *विशिष्ट*, 1-doc कीवर्ड की एक जोड़ी जिनके बीच 3 शब्द हैं, उसे pair_tc = 0.088 मिलेगा और वह एक जोड़ी की तुलना में हार जाएगी जिसमें दो 1000-doc कीवर्ड होते हैं जो एक दूसरे के ठीक बगल में स्थित होते हैं और जो pair_tc = 0.25 उत्पन्न करते हैं। तो, मूलतः, जबकि ATC वास्तव में दोनों कीवर्ड की आवृत्ति और निकटता को जोड़ता है, यह फिर भी निकटता को कुछ हद तक महत्व देता है।

### रैंकिंग कारक संघनन कार्य

एक **क्षेत्र संघनन कार्य** एक एकल-आर्गुमेंट कार्य है जो क्षेत्र स्तर के कारकों के साथ एक अभिव्यक्ति स्वीकार करता है, सभी मेल खाते क्षेत्रों पर दोहराता है, और अंतिम परिणामों की गणना करता है। वर्तमान में लागू क्षेत्र संघनन कार्यों में शामिल हैं:

* `sum`, जो सभी मेल खाते क्षेत्रों में आर्गुमेंट अभिव्यक्ति को जोड़ता है। उदाहरण के लिए `sum(1)` मेल खाने वाले क्षेत्रों की संख्या लौटाना चाहिए।
* `top`, जो सभी मेल खाते क्षेत्रों में आर्गुमेंट का सबसे उच्च मान लौटाता है।
* `max_window_hits`, हिट स्थितियों की एक स्लाइडिंग विंडो का प्रबंधन करता है ताकि निर्दिष्ट विंडो आकार के भीतर अधिकतम हिट की संख्या को ट्रैक किया जा सके। यह विंडो के बाहर वाली पुरानी हिट को हटा देता है और नवीनतम हिट को जोड़ता है, उस विंडो के भीतर मिली अधिकतम हिट की संख्या को अपडेट करता है।

### सभी अंतर्निहित रैंकरों के लिए फॉर्मूला अभिव्यक्तियाँ

अधिकांश अन्य रैंकर्स को वास्तव में अभिव्यक्ति-आधारित रेंकर का उपयोग करके अनुकरण किया जा सकता है। आपको केवल एक उचित अभिव्यक्ति प्रदान करने की आवश्यकता है। जबकि इस अनुकरण का उपयोग करना अंतर्निर्मित, संकलित रेंकर की तुलना में अपेक्षाकृत धीमा हो सकता है, यह अभी भी दिलचस्प हो सकता है यदि आप मौजूदा में से किसी एक से अपने रैंकिंग फ़ॉर्मूले को ठीक करना चाहते हैं। इसके अलावा, फॉर्मूले रेंकर के विवरण को स्पष्ट, पठनीय तरीके से बताते हैं।

* proximity_bm25 (डिफ़ॉल्ट रेंकर) = `sum(lcs*user_weight)*1000+bm25`
* bm25 = `sum(user_weight)*1000+bm25`
* none = `1`
* wordcount = `sum(hit_count*user_weight)`
* proximity = `sum(lcs*user_weight)`
* matchany = `sum((word_count+(lcs-1)*max_lcs)*user_weight)`
* fieldmask = `field_mask`
* sph04 = `sum((4*lcs+2*(min_hit_pos==1)+exact_hit)*user_weight)*1000+bm25`


### IDF सूत्र की कॉन्फ़िगरेशन

Manticore में ऐतिहासिक रूप से डिफ़ॉल्ट IDF (Inverse Document Frequency) `OPTION idf='normalized,tfidf_normalized'` के बराबर है, और ये सामान्यीकरण कई अवांछित प्रभाव पैदा कर सकते हैं।

पहले, `idf=normalized` कीवर्ड दंड का कारण बनता है। उदाहरण के लिए, यदि आप `the | something` के लिए खोज करते हैं और `the` 50% से अधिक दस्तावेज़ों में दिखाई देता है, तो दोनों कीवर्ड `the` और `[something` वाले दस्तावेज़ों को केवल एक कीवर्ड `something` वाले दस्तावेज़ों की तुलना में कम वजन मिलेगा। `OPTION idf=plain` का उपयोग करने से इससे बचा जा सकता है।

प्लेन IDF `[0, log(N)]` रेंज में भिन्न होता है, और कीवर्ड कभी दंडित नहीं होते हैं; जबकि सामान्यीकृत IDF `[-log(N), log(N)]` रेंज में भिन्न होता है, और बहुत बार दिखाई देने वाले कीवर्ड दंडित होते हैं।

दूसरे, `idf=tfidf_normalized` क्वेरीज़ पर IDF प्रवृत्ति का कारण बनता है। ऐतिहासिक रूप से, हम ने IDF को क्वेरी कीवर्ड गणना द्वारा भी विभाजित किया, ताकि सभी कीवर्ड पर `sum(tf*idf)` `[0,1]` रेंज में बने रहे। हालांकि, इसका मतलब है कि क्वेरी `word1` और `word1 | nonmatchingword2` बिल्कुल समान परिणाम सेट को अलग-अलग वजन सौंपेगा, क्योंकि `word1` और `nonmatchingword2` के लिए IDFs दोनों को 2 से विभाजित किया जाएगा। `OPTION idf='tfidf_unnormalized'` इसे सही करता है। ध्यान दें कि BM25, BM25A, BM25F() रैंकिंग कारक इस सामान्यीकरण को बंद करने पर उचित रूप से स्केल होंगे।

IDF फ़्लैग को मिलाया जा सकता है; `plain` और `normalized` एक-दूसरे को परस्पर अनन्य हैं; `tfidf_unnormalized` और `tfidf_normalized` एक-दूसरे को परस्पर अनन्य हैं; और ऐसे परस्पर अनन्य समूह में निर्दिष्ट नहीं किए गए फ़्लैग अपने डिफ़ॉल्ट लेते हैं। इसका मतलब है कि `OPTION idf=plain` एक पूर्ण `OPTION idf='plain,tfidf_normalized'` विनिर्देशन के बराबर है।    

<!-- proofread -->



