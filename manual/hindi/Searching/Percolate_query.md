# पर्कोलेट क्वेरी

पर्कोलेट क्वेरीज को स्थायी क्वेरीज, संभावित खोज, दस्तावेज़ रूटिंग, उल्टे खोज, और विपरीत खोज के नाम से भी जाना जाता है.

खोज करने का पारंपरिक तरीका दस्तावेज़ों को संग्रहीत करने और उनके खिलाफ खोज क्वेरीज चलाने में निहित है. हालांकि, कुछ मामलों में हम चाहते हैं कि किसी नए दस्तावेज़ पर एक क्वेरी लागू की जाए ताकि मैच का संकेत मिले. ऐसी कुछ परिस्थितियाँ जहाँ ऐसा वांछनीय होता है, उनमें निगरानी प्रणालियाँ शामिल हैं जो डेटा संग्रहित करती हैं और उपयोगकर्ताओं को विशिष्ट घटनाओं के बारे में सूचित करती हैं, जैसे किसी मीट्रिक के लिए एक निश्चित सीमा तक पहुंचना या निगरानी किए गए डेटा में किसी विशेष मान का प्रकट होना. एक और उदाहरण समाचार समेकन है, जहाँ उपयोगकर्ता केवल कुछ विशिष्ट श्रेणियों या विषयों, या यहां तक कि विशिष्ट "keywords" के बारे में सूचित होना चाह सकते हैं.

इन परिस्थितियों में, पारंपरिक खोज सबसे उपयुक्त नहीं होती, क्योंकि यह मानती है कि वांछित खोज पूरे संग्रह पर की जाती है. यह प्रक्रिया उपयोगकर्ताओं की संख्या से कई गुणा हो जाती है, जिसके परिणामस्वरूप पूरे संग्रह पर कई क्वेरीज चलती हैं, जो एक महत्वपूर्ण अतिरिक्त भार पैदा कर सकती हैं. इस खंड में वर्णित वैकल्पिक दृष्टिकोण में क्वेरीज को संग्रहीत करने और फिर उन्हें आने वाले नए दस्तावेज़ या दस्तावेज़ों के समूह के खिलाफ परखने की बात की गई है.

Google Alerts, AlertHN, Bloomberg Terminal, और अन्य प्रणालियाँ जो उपयोगकर्ताओं को विशिष्ट सामग्री की सदस्यता लेने की अनुमति देती हैं, समान तकनीक का उपयोग करती हैं.

> * [percolate](../Creating_a_table/Local_tables/Percolate_table.md) देखें PQ तालिका बनाने के बारे में जानकारी के लिए.

> * [Adding rules to a percolate table](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md) देखें यह जानने के लिए कि पर्कोलेट नियम (जिसे PQ नियम भी कहा जाता है) कैसे जोड़े जाएं. यहाँ एक त्वरित उदाहरण है:


### CALL PQ के साथ पर्कोलेट क्वेरी करना

पर्कोलेट क्वेरीज के बारे में याद रखने वाली प्रमुख बात यह है कि आपकी खोज क्वेरीज पहले से ही तालिका में हैं. आपको जो प्रदान करना है, वह है दस्तावेज़ **यह जांचने के लिए कि क्या इनमें से कोई भी संग्रहीत नियमों से मेल खाता है**.

आप SQL या JSON इंटरफेस तथा प्रोग्रामिंग भाषा क्लाइंट का उपयोग करके पर्कोलेट क्वेरी कर सकते हैं. SQL दृष्टिकोण अधिक लचीलापन प्रदान करता है, जबकि HTTP विधि सरल है और आपकी अधिकांश आवश्यकताओं को पूरा करती है. नीचे दी गई तालिका आपको अंतरों को समझने में मदद कर सकती है.

| इच्छित व्यवहार              | SQL                                     | HTTP                                 |
| ----------------------------- | --------------------------------------- | ------------------------------------ |
| एक एकल दस्तावेज़ प्रदान करें     | `CALL PQ('tbl', '{doc1}')`              | `query.percolate.document{doc1}`     |
| एक एकल दस्तावेज़ प्रदान करें (वैकल्पिक) | `CALL PQ('tbl', 'doc1', 0 as docs_json)` | - |
| एकाधिक दस्तावेज़ प्रदान करें    | `CALL PQ('tbl', ('doc1', 'doc2'), 0 as docs_json)` | - |
| एकाधिक दस्तावेज़ प्रदान करें (वैकल्पिक) | `CALL PQ('tbl', ('{doc1}', '{doc2}'))` | - |
| एकाधिक दस्तावेज़ प्रदान करें (वैकल्पिक) | `CALL PQ('tbl', '[{doc1}, {doc2}]')` | - |
| मिलान करने वाले दस्तावेज़ आईडी लौटाएं  | 0/1 as docs (डिफ़ॉल्ट रूप से अक्षम)       | डिफ़ॉल्ट रूप से सक्षम                   |
| परिणाम में दिखाने के लिए दस्तावेज़ का अपना आईडी उपयोग करें | 'id field' as docs_id (डिफ़ॉल्ट रूप से अक्षम) | उपलब्ध नहीं |
| ध्यान दें कि इनपुट दस्तावेज़ JSON हैं | 1 as docs_json (डिफ़ॉल्ट रूप से 1) | डिफ़ॉल्ट रूप से सक्षम |
| ध्यान दें कि इनपुट दस्तावेज़ सामान्य पाठ हैं | 0 as docs_json (डिफ़ॉल्ट रूप से 1) | उपलब्ध नहीं |
| [Sparsed distribution mode](../Searching/Percolate_query.md#I-want-higher-performance-of-a-percolate-query) | डिफ़ॉल्ट | डिफ़ॉल्ट |
| [Sharded distribution mode](../Searching/Percolate_query.md#I-want-higher-performance-of-a-percolate-query) | शार्डेड के रूप में मोड | उपलब्ध नहीं |
| मिलान करने वाली क्वेरी के बारे में सभी जानकारी लौटाएं | 1 as query (डिफ़ॉल्ट रूप से 0) | डिफ़ॉल्ट रूप से सक्षम |
| अवैध JSON छोड़ें | 1 as skip_bad_json (डिफ़ॉल्ट रूप से 0) | उपलब्ध नहीं |
| [SHOW META](../Node_info_and_management/SHOW_META.md) में विस्तृत जानकारी | 1 as verbose (डिफ़ॉल्ट रूप से 0) | उपलब्ध नहीं |
| उस संख्या को परिभाषित करें जो दस्तावेज़ आईडी में जोड़ी जाएगी यदि कोई docs_id फ़ील्ड प्रदान नहीं किया गया (मुख्य रूप से [distributed PQ modes](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#Distributed-percolate-tables-%28DPQ-tables%29) में प्रासंगिक) | 1 as shift (डिफ़ॉल्ट रूप से 0) | उपलब्ध नहीं |

<!-- example create percolate -->
यह दिखाने के लिए कि यह कैसे काम करता है, यहाँ कुछ उदाहरण हैं. आइए दो फ़ील्ड्स के साथ एक PQ तालिका बनाते हैं:

* title (text)
* color (string)

और इसमें तीन नियम:

* केवल पूर्ण-पाठ. क्वेरी: `@title bag`
* पूर्ण-पाठ और फ़िल्टरिंग. क्वेरी: `@title shoes`. फ़िल्टर्स: `color='red'`
* पूर्ण-पाठ और अधिक जटिल फ़िल्टरिंग. क्वेरी: `@title shoes`. फ़िल्टर्स: `color IN('blue', 'green')`

<!-- intro -->
#### SQL
<!-- request SQL -->

```sql
CREATE TABLE products(title text, color string) type='pq';
INSERT INTO products(query) values('@title bag');
INSERT INTO products(query,filters) values('@title shoes', 'color='red'');
INSERT INTO products(query,filters) values('@title shoes', 'color in ('blue', 'green')');
select * from products;
```

<!-- response SQL -->

```sql
+---------------------+--------------+------+---------------------------+
| id                  | query        | tags | filters                   |
+---------------------+--------------+------+---------------------------+
| 1657852401006149635 | @title shoes |      | color IN ('blue, 'green') |
| 1657852401006149636 | @title shoes |      | color='red'               |
| 1657852401006149637 | @title bag   |      |                           |
+---------------------+--------------+------+---------------------------+
```

<!-- request JSON -->

```json
PUT /pq/products/doc/
{
  "query": {
    "match": {
      "title": "bag"
    }
  },
  "filters": ""
}

PUT /pq/products/doc/
{
  "query": {
    "match": {
      "title": "shoes"
    }
  },
  "filters": "color='red'"
}

PUT /pq/products/doc/
{
  "query": {
    "match": {
      "title": "shoes"
    }
  },
  "filters": "color IN ('blue', 'green')"
}
```

<!-- intro -->
#### JSON
<!-- response JSON -->

```json
{
  "table": "products",
  "type": "doc",
  "_id": 1657852401006149661,
  "result": "created"
}
{
  "table": "products",
  "type": "doc",
  "_id": 1657852401006149662,
  "result": "created"
}
{
  "table": "products",
  "type": "doc",
  "_id": 1657852401006149663,
  "result": "created"
}
```

<!-- intro -->
#### PHP
<!-- request PHP -->

```php

$index = [
    'table' => 'products',
    'body' => [
        'columns' => [
            'title' => ['type' => 'text'],
            'color' => ['type' => 'string']
        ],
        'settings' => [
            'type' => 'pq'
        ]
    ]
];
$client->indices()->create($index);

$query = [
    'table' => 'products',
    'body' => [ 'query'=>['match'=>['title'=>'bag']]]
];
$client->pq()->doc($query);
$query = [
    'table' => 'products',
    'body' => [ 'query'=>['match'=>['title'=>'shoes']],'filters'=>"color='red'"]
];
$client->pq()->doc($query);


$query = [
    'table' => 'products',
    'body' => [ 'query'=>['match'=>['title'=>'shoes']],'filters'=>"color IN ('blue', 'green')"]
];
$client->pq()->doc($query);
```
<!-- response PHP -->
``` php
Array(
  [table] => products
  [type] => doc
  [_id] => 1657852401006149661
  [result] => created
)
Array(
  [table] => products
  [type] => doc
  [_id] => 1657852401006149662
  [result] => created
)
Array(
  [table] => products
  [type] => doc
  [_id] => 1657852401006149663
  [result] => created
)
```
<!-- intro -->
Python
<!-- request Python -->

```python
utilsApi.sql('create table products(title text, color string) type='pq'')
indexApi.insert({"table" : "products", "doc" : {"query" : "@title bag" }})
indexApi.insert({"table" : "products",  "doc" : {"query" : "@title shoes", "filters": "color='red'" }})
indexApi.insert({"table" : "products",  "doc" : {"query" : "@title shoes","filters": "color IN ('blue', 'green')" }})
```
<!-- response Python -->
``` python
{'created': True,
 'found': None,
 'id': 0,
 'table': 'products',
 'result': 'created'}
{'created': True,
 'found': None,
 'id': 0,
 'table': 'products',
 'result': 'created'}
{'created': True,
 'found': None,
 'id': 0,
 'table': 'products',
 'result': 'created'}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await utilsApi.sql('create table products(title text, color string) type='pq'');
res = indexApi.insert({"table" : "products", "doc" : {"query" : "@title bag" }});
res = indexApi.insert({"table" : "products",  "doc" : {"query" : "@title shoes", "filters": "color='red'" }});
res = indexApi.insert({"table" : "products",  "doc" : {"query" : "@title shoes","filters": "color IN ('blue', 'green')" }});
```
<!-- response javascript -->
``` javascript
"table":"products","_id":0,"created":true,"result":"created"}
{"table":"products","_id":0,"created":true,"result":"created"}
{"table":"products","_id":0,"created":true,"result":"created"}
```
<!-- intro -->
java
<!-- request Java -->

```java
utilsApi.sql("create table products(title text, color string) type='pq'");
doc = new HashMap<String,Object>(){{
    put("query", "@title bag");
}};
newdoc = new InsertDocumentRequest();
newdoc.index("products").setDoc(doc);
indexApi.insert(newdoc);

doc = new HashMap<String,Object>(){{
    put("query", "@title shoes");
    put("filters", "color='red'");
}};
newdoc = new InsertDocumentRequest();
newdoc.index("products").setDoc(doc);
indexApi.insert(newdoc);

doc = new HashMap<String,Object>(){{
    put("query", "@title shoes");
    put("filters", "color IN ('blue', 'green')");
}};
newdoc = new InsertDocumentRequest();
newdoc.index("products").setDoc(doc);
indexApi.insert(newdoc);
```
<!-- response Java -->
``` java
{total=0, error=, warning=}
class SuccessResponse {
    index: products
    id: 0
    created: true
    result: created
    found: null
}
class SuccessResponse {
    index: products
    id: 0
    created: true
    result: created
    found: null
}
class SuccessResponse {
    index: products
    id: 0
    created: true
    result: created
    found: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
utilsApi.Sql("create table products(title text, color string) type='pq'");

Dictionary<string, Object> doc = new Dictionary<string, Object>(); 
doc.Add("query", "@title bag");
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", doc: doc);
indexApi.Insert(newdoc);

doc = new Dictionary<string, Object>(); 
doc.Add("query", "@title shoes");
doc.Add("filters", "color='red'");
newdoc = new InsertDocumentRequest(index: "products", doc: doc);
indexApi.Insert(newdoc);

doc = new Dictionary<string, Object>(); 
doc.Add("query", "@title bag");
doc.Add("filters", "color IN ('blue', 'green')");
newdoc = new InsertDocumentRequest(index: "products", doc: doc);
indexApi.Insert(newdoc);
```
<!-- response C# -->
``` clike
{total=0, error="", warning=""}

class SuccessResponse {
    index: products
    id: 0
    created: true
    result: created
    found: null
}
class SuccessResponse {
    index: products
    id: 0
    created: true
    result: created
    found: null
}
class SuccessResponse {
    index: products
    id: 0
    created: true
    result: created
    found: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->
```typescript
res = await utilsApi.sql("create table test_pq(title text, color string) type='pq'");
res = indexApi.insert({
  index: 'test_pq',
  doc: { query : '@title bag' }
});
res = indexApi.insert(
  index: 'test_pq',
  doc: { query: '@title shoes', filters: "color='red'" }
});
res = indexApi.insert({
  index: 'test_pq',
  doc: { query : '@title shoes', filters: "color IN ('blue', 'green')" }
});
```
<!-- response TypeScript -->
``` typescript
{
	"table":"test_pq",
	"_id":1657852401006149661,
	"created":true,
	"result":"created"
}
{
	"table":"test_pq",
	"_id":1657852401006149662,
	"created":true,
	"result":"created"
}
{
	"table":"test_pq",
	"_id":1657852401006149663,
	"created":true,
	"result":"created"
}
```

<!-- intro -->
चलो
<!-- request Go -->
```go
apiClient.UtilsAPI.Sql(context.Background()).Body("create table test_pq(title text, color string) type='pq'").Execute()

indexDoc := map[string]interface{} {"query": "@title bag"}
indexReq := manticoreclient.NewInsertDocumentRequest("test_pq", indexDoc)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute();

indexDoc = map[string]interface{} {"query": "@title shoes", "filters": "color='red'"}
indexReq = manticoreclient.NewInsertDocumentRequest("test_pq", indexDoc)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute();

indexDoc = map[string]interface{} {"query": "@title shoes", "filters": "color IN ('blue', 'green')"}
indexReq = manticoreclient.NewInsertDocumentRequest("test_pq", indexDoc)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute();
```
<!-- response Go -->
``` go
{
	"table":"test_pq",
	"_id":1657852401006149661,
	"created":true,
	"result":"created"
}
{
	"table":"test_pq",
	"_id":1657852401006149662,
	"created":true,
	"result":"created"
}
{
	"table":"test_pq",
	"_id":1657852401006149663,
	"created":true,
	"result":"created"
}
```

<!-- end -->

<!-- example single -->
##### बस मुझे बताओ कौन से PQ नियम मेरे एकल दस्तावेज़ से मेल खाते हैं

पहला दस्तावेज़ किसी भी नियम से मेल नहीं खाता। यह पहले दो से मेल खा सकता है, लेकिन उन्हें अतिरिक्त फ़िल्टर की आवश्यकता है।

दूसरा दस्तावेज़ एक नियम से मेल खाता है। ध्यान दें कि डिफ़ॉल्ट के रूप में CALL PQ एक दस्तावेज़ को JSON होने की उम्मीद करता है, लेकिन यदि आप `0 as docs_json` का उपयोग करते हैं, तो आप इसके बजाय एक साधारण स्ट्रिंग पास कर सकते हैं।

<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products', 'Beautiful shoes', 0 as docs_json);

CALL PQ('products', 'What a nice bag', 0 as docs_json);
CALL PQ('products', '{"title": "What a nice bag"}');
```
<!-- response SQL -->

```sql
+---------------------+
| id                  |
+---------------------+
| 1657852401006149637 |
+---------------------+

+---------------------+
| id                  |
+---------------------+
| 1657852401006149637 |
+---------------------+
```
<!-- intro -->
JSON:
<!-- request JSON -->

```json
POST /pq/products/_search
{
  "query": {
    "percolate": {
      "document": {
        "title": "What a nice bag"
      }
    }
  }
}
```
<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "max_score": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149644,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
PHP:
<!-- request PHP -->

```php
$percolate = [
    'table' => 'products',
    'body' => [
        'query' => [
            'percolate' => [
                'document' => [
                    'title' => 'What a nice bag'
                ]
            ]
        ]
    ]
];
$client->pq()->search($percolate);

```
<!-- response PHP -->

```php
Array
(
    [took] => 0
    [timed_out] =>
    [hits] => Array
        (
            [total] => 1
            [max_score] => 1
            [hits] => Array
                (
                    [0] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 1657852401006149644
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => bag
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 1
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
searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"What a nice bag"}}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title bag'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"क्या अच्छी थैली"}}}});
```
<!-- response javascript -->
``` javascript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233808,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
java
<!-- request Java -->

```java
PercolateRequest percolateRequest = new PercolateRequest();
query = new HashMap<String,Object>(){{
    put("percolate",new HashMap<String,Object >(){{
        put("document", new HashMap<String,Object >(){{
            put("title","क्या अच्छी थैली");
        }});
    }});
}};
percolateRequest.query(query);
searchApi.percolate("test_pq",percolateRequest);
```
<!-- response Javs -->
``` java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234109, _score=1, _source={query={ql=@title bag}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
Dictionary<string, Object> percolateDoc = new Dictionary<string, Object>(); 
percolateDoc.Add("document", new Dictionary<string, Object> {{ "title", "क्या अच्छी थैली" }});
Dictionary<string, Object> query = new Dictionary<string, Object> {{ "percolate", percolateDoc }}; 
PercolateRequest percolateRequest = new PercolateRequest(query=query);
searchApi.Percolate("test_pq",percolateRequest);

```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234109, _score=1, _source={query={ql=@title bag}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
res = await searchApi.percolate('test_pq', { query: { percolate: { document : { title : 'क्या अच्छी थैली' } } } } );
```
<!-- response TypeScript -->
``` typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
query := map[string]interface{} {"title": "क्या अच्छी थैली"}
percolateRequestQuery := manticoreclient.NewPercolateQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery)
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()

```
<!-- response Go -->
``` go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- end -->

<!-- example pq_rules -->
##### I want to know complete PQ rules matching my document
<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products', '{"title": "क्या अच्छी थैली"}', 1 as query);
```
<!-- response SQL -->

```sql
+---------------------+------------+------+---------+
| id                  | query      | tags | filters |
+---------------------+------------+------+---------+
| 1657852401006149637 | @title bag |      |         |
+---------------------+------------+------+---------+
```
<!-- intro -->
JSON:
<!-- request JSON -->

```json
POST /pq/products/_search
{
  "query": {
    "percolate": {
      "document": {
        "title": "क्या अच्छी थैली"
      }
    }
  }
}
```
<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "max_score": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149644,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
PHP:
<!-- request PHP -->

```php
$percolate = [
    'table' => 'products',
    'body' => [
        'query' => [
            'percolate' => [
                'document' => [
                    'title' => 'क्या सुंदर बैग है'
                ]
            ]
        ]
    ]
];
$client->pq()->search($percolate);

```
<!-- response PHP -->

```php
Array
(
    [took] => 0
    [timed_out] =>
    [hits] => Array
        (
            [total] => 1
            [max_score] => 1
            [hits] => Array
                (
                    [0] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 1657852401006149644
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => बैग
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 1
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
searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"क्या सुंदर बैग है"}}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title बैग'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"क्या सुंदर बैग है"}}}});
```
<!-- response javascript -->
``` javascript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233808,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title बैग"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
java
<!-- request Java -->

```java
PercolateRequest percolateRequest = new PercolateRequest();
query = new HashMap<String,Object>(){{
    put("percolate",new HashMap<String,Object >(){{
        put("document", new HashMap<String,Object >(){{
            put("title","क्या सुंदर बैग है");
        }});
    }});
}};
percolateRequest.query(query);
searchApi.percolate("test_pq",percolateRequest);
```
<!-- response Java -->
``` java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234109, _score=1, _source={query={ql=@title बैग}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
Dictionary<string, Object> percolateDoc = new Dictionary<string, Object>(); 
percolateDoc.Add("document", new Dictionary<string, Object> {{ "title", "क्या सुंदर बैग है" }});
Dictionary<string, Object> query = new Dictionary<string, Object> {{ "percolate", percolateDoc }}; 
PercolateRequest percolateRequest = new PercolateRequest(query=query);
searchApi.Percolate("test_pq",percolateRequest);

```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234109, _score=1, _source={query={ql=@title बैग}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
res = await searchApi.percolate('test_pq', { query: { percolate: { document : { title : 'क्या सुंदर बैग है' } } } } );
```
<!-- response TypeScript -->
``` typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title बैग"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
query := map[string]interface{} {"title": "क्या सुंदर बैग है"}
percolateRequestQuery := manticoreclient.NewPercolateQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery)
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()

```
<!-- response Go -->
``` go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- end -->

<!-- example multiple -->

##### कई दस्तावेजों के बारे में क्या?

ध्यान दें कि `CALL PQ` के साथ, आप विभिन्न तरीकों से कई दस्तावेज़ प्रदान कर सकते हैं:

* सरल दस्तावेज़ों की एक श्रृंखला को गोल ब्रैकेट में `('doc1', 'doc2')` के रूप में। यह `0 as docs_json` की आवश्यकता है।
* गोल ब्रैकेट में JSONs की एक श्रृंखला के रूप में `('{doc1}', '{doc2}')`
* या एक मानक JSON श्रृंखला के रूप में `'[{doc1}, {doc2}]'`

<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products', ('nice pair of shoes', 'beautiful bag'), 1 as query, 0 as docs_json);

CALL PQ('products', ('{"title": "nice pair of shoes", "color": "red"}', '{"title": "beautiful bag"}'), 1 as query);

CALL PQ('products', '[{"title": "nice pair of shoes", "color": "blue"}, {"title": "beautiful bag"}]', 1 as query);
```
<!-- response SQL -->

```sql
+---------------------+------------+------+---------+
| id                  | query      | tags | filters |
+---------------------+------------+------+---------+
| 1657852401006149637 | @title bag |      |         |
+---------------------+------------+------+---------+

+---------------------+--------------+------+-------------+
| id                  | query        | tags | filters     |
+---------------------+--------------+------+-------------+
| 1657852401006149636 | @title shoes |      | color='red' |
| 1657852401006149637 | @title bag   |      |             |
+---------------------+--------------+------+-------------+

+---------------------+--------------+------+---------------------------+
| id                  | query        | tags | filters                   |
+---------------------+--------------+------+---------------------------+
| 1657852401006149635 | @title shoes |      | color IN ('blue, 'green') |
| 1657852401006149637 | @title bag   |      |                           |
+---------------------+--------------+------+---------------------------+
```
<!-- intro -->
JSON:
<!-- request JSON -->

```json
POST /pq/products/_search
{
  "query": {
    "percolate": {
      "documents": [
        {"title": "nice pair of shoes", "color": "blue"},
        {"title": "beautiful bag"}
      ]
    }
  }
}
```
<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "max_score": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149644,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            2
          ]
        }
      },
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149646,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
PHP:
<!-- request PHP -->

```php
$percolate = [
    'table' => 'products',
    'body' => [
        'query' => [
            'percolate' => [
                'documents' => [
                    ['title' => 'nice pair of shoes','color'=>'blue'],
                    ['title' => 'beautiful bag']
                ]
            ]
        ]
    ]
];
$client->pq()->search($percolate);

```
<!-- response PHP -->

```php
Array
(
    [took] => 23
    [timed_out] =>
    [hits] => Array
        (
            [total] => 2
            [max_score] => 1
            [hits] => Array
                (
                    [0] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 2810781492890828819
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => bag
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 2
                                        )
                                )
                        )
                    [1] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 2810781492890828821
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => shoes
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 1
                                        )
                                )
                        )
                )
        )
)
```
<!-- intro -->
पायथन
<!-- request Python -->

```python
searchApi.percolate('products',{"query":{"percolate":{"documents":[{"title":"nice pair of shoes","color":"blue"},{"title":"beautiful bag"}]}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381494',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title bag'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [2]}},
                   {u'_id': u'2811025403043381496',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title shoes'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}

```
<!-- intro -->
जावास्क्रिप्ट
<!-- request javascript -->

```javascript
res = await searchApi.percolate('products',{"query":{"percolate":{"documents":[{"title":"nice pair of shoes","color":"blue"},{"title":"beautiful bag"}]}}});
```
<!-- response javascript -->
``` javascript
{
  "took": 6,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233808,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            2
          ]
        }
      },
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233810,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}

```

<!-- intro -->
जावा
<!-- request Java -->

```java
percolateRequest = new PercolateRequest();
query = new HashMap<String,Object>(){{
        put("percolate",new HashMap<String,Object >(){{
            put("documents", new ArrayList<Object>(){{
                    add(new HashMap<String,Object >(){{
                        put("title","nice pair of shoes");
                        put("color","blue");
                    }});
                    add(new HashMap<String,Object >(){{
                        put("title","beautiful bag");

                    }});

                     }});
        }});
    }};
percolateRequest.query(query);
searchApi.percolate("products",percolateRequest);

```
<!-- response Java -->
``` java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234133, _score=1, _source={query={ql=@title bag}}, fields={_percolator_document_slot=[2]}}, {_index=products, _type=doc, _id=2811045522851234135, _score=1, _source={query={ql=@title shoes}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
सी#
<!-- request C# -->

```clike
var doc1 = new Dictionary<string, Object>();
doc1.Add("title","nice pair of shoes");
doc1.Add("color","blue");
var doc2 = new Dictionary<string, Object>();
doc2.Add("title","beautiful bag");
var docs = new List<Object> {doc1, doc2};
Dictionary<string, Object> percolateDoc = new Dictionary<string, Object> {{ "documents", docs }}; 
Dictionary<string, Object> query = new Dictionary<string, Object> {{ "percolate", percolateDoc }}; 
PercolateRequest percolateRequest = new PercolateRequest(query=query);
searchApi.Percolate("products",percolateRequest);
```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234133, _score=1, _source={query={ql=@title bag}}, fields={_percolator_document_slot=[2]}}, {_index=products, _type=doc, _id=2811045522851234135, _score=1, _source={query={ql=@title shoes}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
टाइपस्क्रिप्ट
<!-- request TypeScript -->

```typescript
docs = [ {title : 'What a nice bag'}, {title : 'Really nice shoes'} ]; 
res = await searchApi.percolate('test_pq', { query: { percolate: { documents : docs } } } );
```
<!-- response TypeScript -->
``` typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
जाना
<!-- request Go -->

```go
doc1 := map[string]interface{} {"title": "क्या एक सुंदर बैग"}
doc2 := map[string]interface{} {"title": "वास्तव में सुंदर जूते"}
query := []interface{} {doc1, doc2}
percolateRequestQuery := manticoreclient.NewPercolateQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery)
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()

```
<!-- response Go -->
``` go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- end -->

<!-- example docs_1 -->
##### मैं जानना चाहता हूँ कि कौन से दस्तावेज़ किस नियम से मेल खाते हैं

विकल्प का उपयोग करना `1 as docs` आपको यह देखने की अनुमति देता है कि दिए गए दस्तावेज़ों में से कौन से दस्तावेज़ किस नियम से मेल खाते हैं।
<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products', '[{"title": "सुंदर जोड़ी जूते", "color": "नीला"}, {"title": "सुंदर बैग"}]', 1 as query, 1 as docs);
```

<!-- response SQL -->

```sql
+---------------------+-----------+--------------+------+---------------------------+
| id                  | documents | query        | tags | filters                   |
+---------------------+-----------+--------------+------+---------------------------+
| 1657852401006149635 | 1         | @title shoes |      | color IN ('blue, 'green') |
| 1657852401006149637 | 2         | @title bag   |      |                           |
+---------------------+-----------+--------------+------+---------------------------+
```
<!-- intro -->
JSON:
<!-- request JSON -->

```json
POST /pq/products/_search
{
  "query": {
    "percolate": {
      "documents": [
        {"title": "सुंदर जोड़ी जूते", "color": "नीला"},
        {"title": "सुंदर बैग"}
      ]
    }
  }
}
```
<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "max_score": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149644,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            2
          ]
        }
      },
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149646,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
PHP:
<!-- request PHP -->

```php
$percolate = [
    'table' => 'products',
    'body' => [
        'query' => [
            'percolate' => [
                'documents' => [
                    ['title' => 'सुंदर जोड़ी जूते','color'=>'नीला'],
                    ['title' => 'सुंदर बैग']
                ]
            ]
        ]
    ]
];
$client->pq()->search($percolate);

```
<!-- response PHP -->

```php
Array
(
    [took] => 23
    [timed_out] =>
    [hits] => Array
        (
            [total] => 2
            [max_score] => 1
            [hits] => Array
                (
                    [0] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 2810781492890828819
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => bag
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 2
                                        )
                                )
                        )
                    [1] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 2810781492890828821
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => जूते
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 1
                                        )
                                )
                        )
                )
        )
)
```
<!-- intro -->
पायथन
<!-- request Python -->

```python
searchApi.percolate('products',{"query":{"percolate":{"documents":[{"title":"शानदार जोड़ी जूते","color":"नीला"},{"title":"खूबसूरत बैग"}]}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381494',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title बैग'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [2]}},
                   {u'_id': u'2811025403043381496',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title जूते'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}

```
<!-- intro -->
जावास्क्रिप्ट
<!-- request javascript -->

```javascript
res = await searchApi.percolate('products',{"query":{"percolate":{"documents":[{"title":"शानदार जोड़ी जूते","color":"नीला"},{"title":"खूबसूरत बैग"}]}}});
```
<!-- response javascript -->
``` javascript
{
  "took": 6,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233808,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title बैग"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            2
          ]
        }
      },
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233810,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title जूते"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
जावा
<!-- request Java -->

```java
percolateRequest = new PercolateRequest();
query = new HashMap<String,Object>(){{
        put("percolate",new HashMap<String,Object >(){{
            put("documents", new ArrayList<Object>(){{
                    add(new HashMap<String,Object >(){{
                        put("title","शानदार जोड़ी जूते");
                        put("color","नीला");
                    }});
                    add(new HashMap<String,Object >(){{
                        put("title","खूबसूरत बैग");

                    }});

                     }});
        }});
    }};
percolateRequest.query(query);
searchApi.percolate("products",percolateRequest);

```
<!-- response Java -->
``` java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234133, _score=1, _source={query={ql=@title बैग}}, fields={_percolator_document_slot=[2]}}, {_index=products, _type=doc, _id=2811045522851234135, _score=1, _source={query={ql=@title जूते}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
var doc1 = new Dictionary<string, Object>();
doc1.Add("title","शानदार जोड़ी जूते");
doc1.Add("color","नीला");
var doc2 = new Dictionary<string, Object>();
doc2.Add("title","खूबसूरत बैग");
var docs = new List<Object> {doc1, doc2};
Dictionary<string, Object> percolateDoc = new Dictionary<string, Object> {{ "documents", docs }}; 
Dictionary<string, Object> query = new Dictionary<string, Object> {{ "percolate", percolateDoc }}; 
PercolateRequest percolateRequest = new PercolateRequest(query=query);
searchApi.Percolate("products",percolateRequest);
```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234133, _score=1, _source={query={ql=@title बैग}}, fields={_percolator_document_slot=[2]}}, {_index=products, _type=doc, _id=2811045522851234135, _score=1, _source={query={ql=@title जूते}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
टाइपस्क्रिप्ट
<!-- request TypeScript -->

```typescript
docs = [ {title : 'क्या शानदार बैग'}, {title : 'वास्तव में शानदार जूते'} ]; 
res = await searchApi.percolate('test_pq', { query: { percolate: { documents : docs } } } );
```
<!-- response TypeScript -->
``` typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title बैग"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title जूते"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
जाओ
<!-- request Go -->

```go
doc1 := map[string]interface{} {"title": "क्या सुंदर बैग है"}
doc2 := map[string]interface{} {"title": "वास्तव में सुंदर जूते"}
query := []interface{} {doc1, doc2}
percolateRequestQuery := manticoreclient.NewPercolateQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery)
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()

```
<!-- response Go -->
``` go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title बैग"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title जूते"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- end -->

<!-- example docs_id -->
#### स्थिर आईडी
डिफ़ॉल्ट रूप से, मिलान करने वाले दस्तावेज़ आईडी उस सूची में उनके सापेक्ष नंबरों के अनुरूप हैं जिसे आप प्रदान करते हैं। हालाँकि, कुछ मामलों में, प्रत्येक दस्तावेज़ पहले से ही अपनी खुद की आईडी रखता है। इस मामले के लिए, `CALL PQ` के लिए एक विकल्प `'id field name' as docs_id` है।

ध्यान दें कि यदि आईडी प्रदान किए गए फ़ील्ड नाम द्वारा नहीं मिलती है, तो PQ नियम परिणामों में नहीं दिखाया जाएगा।

यह विकल्प केवल SQL के माध्यम से `CALL PQ` के लिए उपलब्ध है।

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CALL PQ('products', '[{"id": 123, "title": "सुंदर जूते का जोड़ा", "color": "नीला"}, {"id": 456, "title": "सुंदर बैग"}]', 1 as query, 'id' as docs_id, 1 as docs);
```
<!-- response SQL -->

```sql
+---------------------+-----------+--------------+------+---------------------------+
| id                  | documents | query        | tags | filters                   |
+---------------------+-----------+--------------+------+---------------------------+
| 1657852401006149664 | 456       | @title बैग   |      |                           |
| 1657852401006149666 | 123       | @title जूते |      | color IN ('blue, 'green') |
+---------------------+-----------+--------------+------+---------------------------+
```
<!-- end -->

<!-- example invalid_json -->
##### मेरे पास अमान्य JSON हो सकते हैं, कृपया उन्हें छोड़ दें

जब आप अलग-अलग JSONs के साथ CALL PQ का उपयोग करते हैं, तो आप इनपुट में किसी भी अमान्य JSONs को छोड़ने के लिए विकल्प 1 as skip_bad_json का उपयोग कर सकते हैं। नीचे दिए गए उदाहरण में, 2रा क्वेरी एक अमान्य JSON के कारण विफल होती है, लेकिन 3रा क्वेरी 1 as skip_bad_json का उपयोग करके त्रुटि से बचती है। ध्यान रखें कि जब HTTP के माध्यम से JSON क्वेरी भेजी जाती है, तो यह विकल्प उपलब्ध नहीं होता है, क्योंकि उस मामले में पूरे JSON क्वेरी को वैध होना चाहिए।

<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products', ('{"title": "सुंदर जूते का जोड़ा", "color": "नीला"}', '{"title": "सुंदर बैग"}'));

CALL PQ('products', ('{"title": "सुंदर जूते का जोड़ा", "color": "नीला"}', '{"title": "सुंदर बैग}'));

CALL PQ('products', ('{"title": "सुंदर जूते का जोड़ा", "color": "नीला"}', '{"title": "सुंदर बैग}'), 1 as skip_bad_json);
```
<!-- response SQL -->

```sql
+---------------------+
| id                  |
+---------------------+
| 1657852401006149635 |
| 1657852401006149637 |
+---------------------+

ERROR 1064 (42000): Strings में अमान्य JSON वस्तुएं: 2

+---------------------+
| id                  |
+---------------------+
| 1657852401006149635 |
+---------------------+
```

<!-- end -->

##### मुझे एक पर्कोलेट क्वेरी की उच्च प्रदर्शन चाहिए
पर्कोलेट क्वेरी उच्च थ्रूपुट और बड़े डेटा मात्रा को ध्यान में रखते हुए डिज़ाइन की गई हैं। कम विलंबता और उच्च थ्रूपुट के लिए प्रदर्शन अनुकूलित करने के लिए, निम्नलिखित पर विचार करें।

एक पर्कोलेट तालिका के लिए वितरण के दो तरीके हैं और यह पर्कोलेट क्वेरी इसके खिलाफ कैसे काम कर सकती है:

* **Sparse (डिफ़ॉल्ट)।** आदर्श है: कई दस्तावेज़, दर्शक PQ तालिकाएं। जब आपका दस्तावेज़ सेट बड़ा होता है लेकिन PQ तालिका में संग्रहीत प्रश्नों का सेट छोटा होता है, तो विरल मोड लाभदायक होता है। इस मोड में, आप जो दस्तावेज़ पास करते हैं, उसकी मात्रा एजेंटों की संख्या के बीच विभाजित की जाएगी, इसलिए प्रत्येक नोड आपकी अनुरोध से दस्तावेज़ के केवल एक हिस्से को संसाधित करता है। मैन्टिकोर आपके दस्तावेज़ सेट को विभाजित करता है और दर्पणों के बीच टुकड़े वितरित करता है। एक बार जब एजेंट प्रश्नों को संसाधित करना समाप्त कर देते हैं, तो मैन्टिकोर परिणाम एकत्र करता है और उन्हें जोड़ता है, अंतिम प्रश्न सेट वापस करता है जैसे कि यह एक ही तालिका से आया। इस प्रक्रिया में सहायता के लिए [प्रतिलिपि](../References.md#Replication) का उपयोग करें।
* **शार्डेड।** आदर्श के लिए: कई PQ नियम, नियम PQ तालिकाओं में विभाजित। इस मोड में, सम्पूर्ण दस्तावेज़ सेट को वितरित PQ तालिका की सभी तालिकाओं में प्रसारित किया जाता है बिना प्रारंभिक रूप से दस्तावेज़ों को विभाजित किए। यह तब लाभदायक है जब अपेक्षाकृत छोटे दस्तावेज़ों के सेट को धकेलते हैं, लेकिन संग्रहीत क्वेरी की संख्या बड़ी होती है। इस मामले में, प्रत्येक नोड पर केवल PQ नियमों के एक हिस्से को संग्रहीत करना और फिर उन नोडों से वापस लौटे परिणामों को मिलाना अधिक उपयुक्त है जो विभिन्न PQ नियमों के खिलाफ समान दस्तावेज़ों के सेट को संसाधित करते हैं। इस मोड को स्पष्ट रूप से सेट करना आवश्यक है, क्योंकि इसका मतलब है नेटवर्क पेलोड में वृद्धि और यह विभिन्न PQs के साथ तालिकाओं की अपेक्षा करता है, जिसे [प्रतिकृति](../References.md#Replication) बॉक्स से बाहर नहीं कर सकता।

मान लें कि आपके पास तालिका `pq_d2` इस प्रकार परिभाषित है:

``` ini
table pq_d2
{
    type = distributed
    agent = 127.0.0.1:6712:pq
    agent = 127.0.0.1:6712:ptitle
}
```

<!-- example distributed pq modes 1 -->
'pq' और 'ptitle' में से प्रत्येक में शामिल हैं:


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT * FROM pq;
```

<!-- response sql -->

```sql
+------+-------------+------+-------------------+
| id   | query       | tags | filters           |
+------+-------------+------+-------------------+
|    1 | filter test |      | gid>=10           |
|    2 | angry       |      | gid>=10 OR gid<=3 |
+------+-------------+------+-------------------+
2 rows in set (0.01 sec)
```

<!-- request JSON -->

```json
POST /pq/pq/_search
```

<!-- response JSON -->

```json
{
    "took":0,
    "timed_out":false,
    "hits":{
        "total":2,
        "hits":[
            {
                "_id": 1,
                "_score":1,
                "_source":{
                    "query":{ "ql":"filter test" },
                    "tags":"",
                    "filters":"gid>=10"
                }
            },
            {
                "_id": 2,
                "_score":1,
                "_source":{
                    "query":{"ql":"angry"},
                    "tags":"",
                    "filters":"gid>=10 OR gid<=3"
                }
            }            
        ]
    }
}
```

<!-- request PHP -->

```php
$params = [
    'table' => 'pq',
    'body' => [
    ]
];
$response = $client->pq()->search($params);
```

<!-- response PHP -->

```php
(
    [took] => 0
    [timed_out] =>
    [hits] =>
        (
            [total] => 2
            [hits] =>
                (
                    [0] =>
                        (
                            [_id] => 1
                            [_score] => 1
                            [_source] =>
                                (
                                    [query] =>
                                        (
                                            [ql] => filter test
                                        )
                                    [tags] =>
                                    [filters] => gid>=10
                                )
                        ),
                    [1] =>
                        (
                            [_id] => 1
                            [_score] => 1
                            [_source] =>
                                (
                                    [query] =>
                                        (
                                            [ql] => angry
                                        )
                                    [tags] =>
                                    [filters] => gid>=10 OR gid<=3
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
searchApi.search({"table":"pq","query":{"match_all":{}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381501',
                    u'_score': 1,
                    u'_source': {u'filters': u"gid>=10",
                                 u'query': u'filter test',
                                 u'tags': u''}},
                   {u'_id': u'2811025403043381502',
                    u'_score': 1,
                    u'_source': {u'filters': u"gid>=10 OR gid<=3",
                                 u'query': u'angry',
                                 u'tags': u''}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
जावास्क्रिप्ट
<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"pq","query":{"match_all":{}}});
```
<!-- response javascript -->
``` javascript
{'hits': {'hits': [{u'_id': u'2811025403043381501',
                    u'_score': 1,
                    u'_source': {u'filters': u"gid>=10",
                                 u'query': u'filter test',
                                 u'tags': u''}},
                   {u'_id': u'2811025403043381502',
                    u'_score': 1,
                    u'_source': {u'filters': u"gid>=10 OR gid<=3",
                                 u'query': u'angry',
                                 u'tags': u''}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
जावास्क्रिप्ट
<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"pq","query":{"match_all":{}}});
```
<!-- response javascript -->
``` javascript
{"hits": {"hits": [{"_id": 2811025403043381501,
                    "_score": 1,
                    "_source": {"filters": u"gid>=10",
                                 "query": "filter test",
                                 "tags": ""}},
                   {"_id": 2811025403043381502,
                    "_score": 1,
                    "_source": {"filters": u"gid>=10 OR gid<=3",
                                 "query": "angry",
                                 "tags": ""}}],
          "total": 2},
  "timed_out": false,
 "took": 0}
```

<!-- intro -->
java
<!-- request Java -->

```java
Map<String,Object> query = new HashMap<String,Object>();
query.put("match_all",null);
SearchRequest searchRequest = new SearchRequest();
searchRequest.setIndex("pq");
searchRequest.setQuery(query);
SearchResponse searchResponse = searchApi.search(searchRequest);
```
<!-- response Java -->
``` java

class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: null
        hits: [{_id=2811045522851233962, _score=1, _source={filters=gid>=10, query=filter test, tags=}}, {_id=2811045522851233951, _score=1, _source={filters=gid>=10 OR gid<=3, query=angry,tags=}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
object query =  new { match_all=null };
SearchRequest searchRequest = new SearchRequest("pq", query);
SearchResponse searchResponse = searchApi.Search(searchRequest);
```
<!-- response C# -->
``` clike

class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: null
        hits: [{_id=2811045522851233962, _score=1, _source={filters=gid>=10, query=filter test, tags=}}, {_id=2811045522851233951, _score=1, _source={filters=gid>=10 OR gid<=3, query=angry,tags=}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
res = await searchApi.search({"table":"test_pq","query":{"match_all":{}}});
```
<!-- response TypeScript -->
``` typescript
{
	'hits':
	{
		'hits': 
		[{
			'_id': '2811025403043381501',
            '_score': 1,
            '_source': 
            {
            	'filters': "gid>=10",
                'query': 'filter test',
                'tags': ''
            }
        },
        {
         	'_id': 
         	'2811025403043381502',
            '_score': 1,
            '_source': 
            {
            	'filters': "gid>=10 OR gid<=3",
                 'query': 'angry',
                 'tags': ''
            }
        }],
    	'total': 2
	},
	'profile': None,
	'timed_out': False,
	'took': 0
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
query := map[string]interface{} {}
percolateRequestQuery := manticoreclient.NewPercolateRequestQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery) 
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()
```
<!-- response Go -->
``` go
{
	'hits':
	{
		'hits': 
		[{
			'_id': '2811025403043381501',
            '_score': 1,
            '_source': 
            {
            	'filters': "gid>=10",
                'query': 'filter test',
                'tags': ''
            }
        },
        {
         	'_id': 
         	'2811025403043381502',
            '_score': 1,
            '_source': 
            {
            	'filters': "gid>=10 OR gid<=3",
                 'query': 'angry',
                 'tags': ''
            }
        }],
    	'total': 2
	},
	'profile': None,
	'timed_out': False,
	'took': 0
}
```

<!-- end -->


<!-- example call_pq_example -->

And you execute `CALL PQ` on the distributed table with a couple of documents.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CALL PQ ('pq_d2', ('{"title":"angry test", "gid":3 }', '{"title":"filter test doc2", "gid":13}'), 1 AS docs);
```

<!-- response sql -->

```sql
+------+-----------+
| id   | documents |
+------+-----------+
|    1 | 2         |
|    2 | 1         |
+------+-----------+
```

<!-- request JSON -->

```json
POST /pq/pq/_search -d '
"query":
{
        "percolate":
        {
                "documents" : [
                    { "title": "angry test", "gid": 3 },
                    { "title": "filter test doc2", "gid": 13 }
                ]
        }
}
'
```

<!-- response JSON -->

```json
{
    "took":0,
    "timed_out":false,
    "hits":{
    "total":2,"hits":[
        {
            "_id": 2,
            "_score":1,
            "_source":{
                "query":{"title":"angry"},
                "tags":"",
                "filters":"gid>=10 OR gid<=3"
            }
        }
        {
            "_id": 1,
            "_score":1,
            "_source":{
                "query":{"ql":"filter test"},
                "tags":"",
                "filters":"gid>=10"
            }
        },
        ]
    }
}
```

<!-- request PHP -->

```php
$params = [
    'table' => 'pq',
    'body' => [
        'query' => [
            'percolate' => [
                'documents' => [
                    [
                        'title'=>'angry test',
                        'gid' => 3
                    ],
                    [
                        'title'=>'filter test doc2',
                        'gid' => 13
                    ],
                ]
            ]
        ]
    ]
];
$response = $client->pq()->search($params);
```

<!-- response PHP -->

```php
(
    [took] => 0
    [timed_out] =>
    [hits] =>
        (
            [total] => 2
            [hits] =>
                (
                    [0] =>
                        (
                            [_index] => pq  
                            [_type] => doc                            
                            [_id] => 2
                            [_score] => 1
                            [_source] =>
                                (
                                    [query] =>
                                        (
                                            [ql] => गुस्सा
                                        )
                                    [tags] =>
                                    [filters] => gid>=10 OR gid<=3
                                ),
                           [fields] =>
                                (
                                    [_percolator_document_slot] =>
                                        (
                                            [0] => 1
                                        )

                                )
                        ),
                    [1] =>
                        (
                            [_index] => pq                            
                            [_id] => 1
                            [_score] => 1
                            [_source] =>
                                (
                                    [query] =>
                                        (
                                            [ql] => फ़िल्टर परीक्षण
                                        )
                                    [tags] =>
                                    [filters] => gid>=10
                                )
                           [fields] =>
                                (
                                    [_percolator_document_slot] =>
                                        (
                                            [0] => 0
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
searchApi.percolate('pq',{"percolate":{"documents":[{"title":"गुस्सा परीक्षण","gid":3},{"title":"फ़िल्टर परीक्षण doc2","gid":13}]}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'table': u'pq',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'गुस्सा'},u'tags':u'',u'filters':u"gid>=10 OR gid<=3"},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}},
                    {u'_id': u'2811025403043381501',
                    u'table': u'pq',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'फ़िल्टर परीक्षण'},u'tags':u'',u'filters':u"gid>=10"},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate('pq',{"percolate":{"documents":[{"title":"गुस्सा परीक्षण","gid":3},{"title":"फ़िल्टर परीक्षण doc2","gid":13}]}});
```
<!-- response javascript -->
``` javascript
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'table': u'pq',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'गुस्सा'},u'tags':u'',u'filters':u"gid>=10 OR gid<=3"},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}},
                    {u'_id': u'2811025403043381501',
                    u'table': u'pq',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'फ़िल्टर परीक्षण'},u'tags':u'',u'filters':u"gid>=10"},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
java
<!-- request Java -->

```java
percolateRequest = new PercolateRequest();
query = new HashMap<String,Object>(){{
    put("percolate",new HashMap<String,Object >(){{
        put("documents", new ArrayList<Object>(){{
            add(new HashMap<String,Object >(){{
                put("title","गुस्सा परीक्षण");
                put("gid",3);
            }});
            add(new HashMap<String,Object >(){{
                put("title","फ़िल्टर परीक्षण doc2");
                put("gid",13);
            }});
        }});
    }});
}};
percolateRequest.query(query);
searchApi.percolate("pq",percolateRequest);
```
<!-- response java -->
``` java
class SearchResponse {
    took: 10
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=pq, _type=doc, _id=2811045522851234165, _score=1, _source={query={ql=@title गुस्सा}}, fields={_percolator_document_slot=[1]}}, {_index=pq, _type=doc, _id=2811045522851234166, _score=1, _source={query={ql=@title फ़िल्टर परीक्षण doc2}}, fields={_percolator_document_slot=[2]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
var doc1 = new Dictionary<string, Object>();
doc1.Add("title","गुस्सा परीक्षण");
doc1.Add("gid",3);
var doc2 = new Dictionary<string, Object>();
doc2.Add("title","फ़िल्टर परीक्षण doc2");
doc2.Add("gid",13);
var docs = new List<Object> {doc1, doc2};
Dictionary<string, Object> percolateDoc = new Dictionary<string, Object> {{ "documents", docs }}; 
Dictionary<string, Object> query = new Dictionary<string, Object> {{ "percolate", percolateDoc }}; 
PercolateRequest percolateRequest = new PercolateRequest(query=query);
searchApi.Percolate("pq",percolateRequest);
```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 10
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=pq, _type=doc, _id=2811045522851234165, _score=1, _source={query={ql=@title angry}}, fields={_percolator_document_slot=[1]}}, {_index=pq, _type=doc, _id=2811045522851234166, _score=1, _source={query={ql=@title filter test doc2}}, fields={_percolator_document_slot=[2]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
docs = [ {title : 'What a nice bag'}, {title : 'Really nice shoes'} ]; 
res = await searchApi.percolate('test_pq', { query: { percolate: { documents : docs } } } );
```
<!-- response TypeScript -->
``` typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
doc1 := map[string]interface{} {"title": "What a nice bag"}
doc2 := map[string]interface{} {"title": "Really nice shoes"}
query := []interface{} {doc1, doc2}
percolateRequestQuery := manticoreclient.NewPercolateQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery)
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()

```
<!-- response Go -->
``` go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- end -->

In the previous example, we used the default **sparse** mode. To demonstrate the **sharded** mode, let's create a distributed PQ table consisting of 2 local PQ tables and add 2 documents to "products1" and 1 document to "products2":
```sql
create table products1(title text, color string) type='pq';
create table products2(title text, color string) type='pq';
create table products_distributed type='distributed' local='products1' local='products2';

INSERT INTO products1(query) values('@title bag');
INSERT INTO products1(query,filters) values('@title shoes', 'color='red'');
INSERT INTO products2(query,filters) values('@title shoes', 'color in ('blue', 'green')');
```

<!-- example sharded -->
Now, if you add `'sharded' as mode` to `CALL PQ`, it will send the documents to all the agent's tables (in this case, just local tables, but they can be remote to utilize external hardware). This mode is not available via the JSON interface.

<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products_distributed', ('{"title": "nice pair of shoes", "color": "blue"}', '{"title": "beautiful bag"}'), 'sharded' as mode, 1 as query);
```
<!-- response SQL -->

```sql
+---------------------+--------------+------+---------------------------+
| id                  | query        | tags | filters                   |
+---------------------+--------------+------+---------------------------+
| 1657852401006149639 | @title bag   |      |                           |
| 1657852401006149643 | @title shoes |      | color IN ('blue, 'green') |
+---------------------+--------------+------+---------------------------+
```

<!-- end -->

Note that the syntax of agent mirrors in the configuration (when several hosts are assigned to one `agent` line, separated with `|`) has nothing to do with the `CALL PQ` query mode. Each `agent` always represents **one** node, regardless of the number of HA mirrors specified for that agent.

<!-- example verbose -->
##### How can I learn more about performance?
In some cases, you might want to get more details about the performance of a percolate query. For that purpose, there is the option `1 as verbose`, which is only available via SQL and allows you to save more performance metrics. You can see them using the `SHOW META` query, which you can run after `CALL PQ`. See [SHOW META](../Node_info_and_management/SHOW_META.md) for more info.

<!-- intro -->
1 as verbose:
<!-- request 1 as verbose -->

```sql
CALL PQ('products', ('{"title": "nice pair of shoes", "color": "blue"}', '{"title": "beautiful bag"}'), 1 as verbose); show meta;
```
<!-- response 1 as verbose -->

```sql
+---------------------+
| id                  |
+---------------------+
| 1657852401006149644 |
| 1657852401006149646 |
+---------------------+
+-------------------------+-----------+
| Variable name           | Value     |
+-------------------------+-----------+
| total                   | 0.000 sec |
| setup                   | 0.000 sec |
| queries_matched         | 2         |
| queries_failed          | 0         |
| document_matched        | 2         |
| total_queries_stored    | 3         |
| term_only_queries       | 3         |
| fast_rejected_queries   | 0         |
| time_per_query          | 27, 10    |
| time_of_matched_queries | 37        |
+-------------------------+-----------+
```
<!-- intro -->
0 as verbose (डिफ़ॉल्ट):
<!-- request 0 as verbose -->

```sql
CALL PQ('products', ('{"title": "nice pair of shoes", "color": "blue"}', '{"title": "beautiful bag"}'), 0 as verbose); show meta;
```
<!-- response 0 as verbose -->

```sql
+---------------------+
| id                  |
+---------------------+
| 1657852401006149644 |
| 1657852401006149646 |
+---------------------+
+-----------------------+-----------+
| Variable name         | Value     |
+-----------------------+-----------+
| total                 | 0.000 sec |
| queries_matched       | 2         |
| queries_failed        | 0         |
| document_matched      | 2         |
| total_queries_stored  | 3         |
| term_only_queries     | 3         |
| fast_rejected_queries | 0         |
+-----------------------+-----------+
```
<!-- end -->
# परकोलेट क्वेरी

परकोलेट क्वेरियों को स्थायी क्वेरियों, संभावित खोज, दस्तावेज़ रूटिंग, उल्टे खोज और विपरीत खोज के रूप में भी जाना जाता है।

खोज करने का पारंपरिक तरीका दस्तावेज़ों को स्टोर करना और उनके खिलाफ खोज क्वेरियों का प्रदर्शन करना होता है। हालाँकि, कुछ मामलों में हम एक नए आने वाले दस्तावेज़ पर क्वेरी लागू करना चाहते हैं ताकि मेल का संकेत दिया जा सके। कुछ परिदृश्यों में जहाँ इसकी आवश्यकता होती है, शामिल हैं निगरानी प्रणाली जो डेटा एकत्र करती हैं और उपयोगकर्ताओं को विशिष्ट घटनाओं के बारे में सूचित करती हैं, जैसे किसी मेट्रिक के लिए एक निश्चित सीमा को प्राप्त करना या निगरानी किए जा रहे डेटा में एक विशेष मान दिखाई देना। एक अन्य उदाहरण समाचार संग्रहण है, जहाँ उपयोगकर्ता केवल कुछ श्रेणियों या विषयों, या यहां तक कि विशिष्ट "कीवर्ड" के बारे में सूचित होना चाहते हैं।

इन परिस्थितियों में, पारंपरिक खोज सबसे अच्छा विकल्प नहीं है, क्योंकि यह मान लेती है कि वांछित खोज पूरे संग्रह पर की जाती है। यह प्रक्रिया उपयोगकर्ताओं की संख्या द्वारा गुणा की जाती है, जिसके परिणामस्वरूप पूरे संग्रह के ऊपर कई क्वेरियाँ चलती हैं, जो महत्वपूर्ण अतिरिक्त लोड उत्पन्न कर सकती हैं। इस अनुभाग में वर्णित वैकल्पिक दृष्टिकोण में क्वेरियों को स्टोर करना और उन्हें आने वाले नए दस्तावेज़ या दस्तावेज़ों के बैच के खिलाफ परीक्षण करना शामिल है।

गूगल अलर्ट्स, अलर्टएचएन, ब्लूमबर्ग टर्मिनल, और अन्य सिस्टम जो उपयोगकर्ताओं को विशिष्ट सामग्री की सदस्यता लेने की अनुमति देते हैं, इसी तरह की तकनीक का उपयोग करते हैं।

> * [परकोलेट बनाएँ](../Creating_a_table/Local_tables/Percolate_table.md) के लिए PQ तालिका बनाने की जानकारी के लिए देखें।
> * [परकोलेट तालिका में नियम जोड़ना](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md) सीखने के लिए कि परकोलेट नियम कैसे जोड़े जाते हैं (जिसे PQ नियम भी कहा जाता है)। यहाँ एक त्वरित उदाहरण है:


### CALL PQ के साथ परकोलेट क्वेरी करना

परकोलेट क्वेरियों के बारे में याद रखने वाली मुख्य बात यह है कि आपकी खोज क्वेरियाँ पहले से ही तालिका में हैं। आपको जो प्रदान करने की आवश्यकता है, वे दस्तावेज़ हैं **जिन्हें जांचना है कि क्या इनमें से कोई भी संग्रहीत नियमों से मेल खाती है**।

आप SQL या JSON इंटरफेस, साथ ही प्रोग्रामिंग भाषा क्लाइंट का उपयोग करके परकोलेट क्वेरी कर सकते हैं। SQL दृष्टिकोण अधिक लचीलापन देता है, जबकि HTTP विधि सरल होती है और अधिकांश चीजें प्रदान करती है जो आपको चाहिए। नीचे दी गई तालिका आपको अंतर समझने में मदद कर सकती है।

| वांछित व्यवहार               | SQL                                      | HTTP                                  |
| ----------------------------- | ---------------------------------------- | ------------------------------------- |
| एकल दस्तावेज़ प्रदान करें    | `CALL PQ('tbl', '{doc1}')`               | `query.percolate.document{doc1}`      |
| एकल दस्तावेज़ प्रदान करें (वैकल्पिक) | `CALL PQ('tbl', 'doc1', 0 as docs_json)` | - |
| कई दस्तावेज़ प्रदान करें     | `CALL PQ('tbl', ('doc1', 'doc2'), 0 as docs_json)` | - |
| कई दस्तावेज़ प्रदान करें (वैकल्पिक) | `CALL PQ('tbl', ('{doc1}', '{doc2}'))` | - |
| कई दस्तावेज़ प्रदान करें (वैकल्पिक) | `CALL PQ('tbl', '[{doc1}, {doc2}]')` | - |
| मिलान करने वाले दस्तावेज़ आईडी वापस करें | 0/1 as docs (डिफ़ॉल्ट रूप से बंद)      | डिफ़ॉल्ट रूप से सक्षम                  |
| परिणाम में दिखाने के लिए दस्तावेज़ के अपने आईडी का उपयोग करें | 'id field' as docs_id (डिफ़ॉल्ट रूप से बंद) | उपलब्ध नहीं है |
| इनपुट दस्तावेज़ JSON हैं मानें | 1 as docs_json (डिफ़ॉल्ट रूप से 1) | डिफ़ॉल्ट रूप से सक्षम |
| इनपुट दस्तावेज़ सामान्य पाठ हैं मानें | 0 as docs_json (डिफ़ॉल्ट रूप से 1) | उपलब्ध नहीं है |
| [Sparsed वितरण मोड](../Searching/Percolate_query.md#I-want-higher-performance-of-a-percolate-query) | डिफ़ॉल्ट | डिफ़ॉल्ट |
| [Sharded वितरण मोड](../Searching/Percolate_query.md#I-want-higher-performance-of-a-percolate-query) | मोड के रूप में शार्डेड | उपलब्ध नहीं है |
| मिलान करने वाली क्वेरी के बारे में सभी जानकारी लौटाएं | 1 as query (डिफ़ॉल्ट रूप से 0) | डिफ़ॉल्ट रूप से सक्षम |
| अवैध JSON छोड़ें | 1 as skip_bad_json (डिफ़ॉल्ट रूप से 0) | उपलब्ध नहीं है |
| [SHOW META](../Node_info_and_management/SHOW_META.md) में विस्तृत जानकारी | 1 as verbose (डिफ़ॉल्ट रूप से 0) | उपलब्ध नहीं है |
| दस्तावेज़ आईडी पर वह संख्या जोड़ें जो प्रदान की जाएगी यदि कोई docs_id फ़ील्ड प्रदान नहीं की गई है (अधिकतर [वितरित PQ मोड](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#Distributed-percolate-tables-%28DPQ-tables%29) में प्रासंगिक) | 1 as shift (डिफ़ॉल्ट रूप से 0) | उपलब्ध नहीं है |

<!-- उदाहरण create percolate -->
यह दिखाने के लिए कि यह कैसे काम करता है, यहाँ कुछ उदाहरण दिए गए हैं। चलो एक PQ तालिका बनाते हैं जिसमें दो फ़ील्ड हैं:

* शीर्षक (पाठ)
* रंग (स्ट्रिंग)

और इसमें तीन नियम हैं:

* केवल पूर्ण-पाठ। क्वेरी: `@title bag`
* पूर्ण-पाठ और फ़िल्टरिंग। क्वेरी: `@title shoes`। फ़िल्टर: `color='red'`
* पूर्ण-पाठ और अधिक जटिल फ़िल्टरिंग। क्वेरी: `@title shoes`। फ़िल्टर: `color IN('blue', 'green')`

<!-- परिचय -->
#### SQL
<!-- अनुरोध SQL -->

```sql
CREATE TABLE products(title text, color string) type='pq';
INSERT INTO products(query) values('@title bag');
INSERT INTO products(query,filters) values('@title shoes', 'color=\'red\'');
INSERT INTO products(query,filters) values('@title shoes', 'color in (\'blue\', \'green\')');
select * from products;
```

<!-- प्रतिक्रिया SQL -->

```sql
+---------------------+--------------+------+---------------------------+
| id                  | query        | tags | filters                   |
+---------------------+--------------+------+---------------------------+
| 1657852401006149635 | @title shoes |      | color IN ('blue, 'green') |
| 1657852401006149636 | @title shoes |      | color='red'               |
| 1657852401006149637 | @title bag   |      |                           |
+---------------------+--------------+------+---------------------------+
```

<!-- अनुरोध JSON -->

```json
PUT /pq/products/doc/
{
  "query": {
    "match": {
      "title": "bag"
    }
  },
  "filters": ""
}

PUT /pq/products/doc/
{
  "query": {
    "match": {
      "title": "shoes"
    }
  },
  "filters": "color='red'"
}

PUT /pq/products/doc/
{
  "query": {
    "match": {
      "title": "shoes"
    }
  },
  "filters": "color IN ('blue', 'green')"
}
```

<!-- intro -->
#### JSON
<!-- response JSON -->

```json
{
  "table": "products",
  "type": "doc",
  "_id": 1657852401006149661,
  "result": "created"
}
{
  "table": "products",
  "type": "doc",
  "_id": 1657852401006149662,
  "result": "created"
}
{
  "table": "products",
  "type": "doc",
  "_id": 1657852401006149663,
  "result": "created"
}
```

<!-- intro -->
#### PHP
<!-- request PHP -->

```php

$index = [
    'table' => 'products',
    'body' => [
        'columns' => [
            'title' => ['type' => 'text'],
            'color' => ['type' => 'string']
        ],
        'settings' => [
            'type' => 'pq'
        ]
    ]
];
$client->indices()->create($index);

$query = [
    'table' => 'products',
    'body' => [ 'query'=>['match'=>['title'=>'bag']]]
];
$client->pq()->doc($query);
$query = [
    'table' => 'products',
    'body' => [ 'query'=>['match'=>['title'=>'shoes']],'filters'=>"color='red'"]
];
$client->pq()->doc($query);


$query = [
    'table' => 'products',
    'body' => [ 'query'=>['match'=>['title'=>'shoes']],'filters'=>"color IN ('blue', 'green')"]
];
$client->pq()->doc($query);
```
<!-- response PHP -->
``` php
Array(
  [table] => products
  [type] => doc
  [_id] => 1657852401006149661
  [result] => created
)
Array(
  [table] => products
  [type] => doc
  [_id] => 1657852401006149662
  [result] => created
)
Array(
  [table] => products
  [type] => doc
  [_id] => 1657852401006149663
  [result] => created
)
```
<!-- intro -->
Python
<!-- request Python -->

```python
utilsApi.sql('create table products(title text, color string) type=\'pq\'')
indexApi.insert({"table" : "products", "doc" : {"query" : "@title bag" }})
indexApi.insert({"table" : "products",  "doc" : {"query" : "@title shoes", "filters": "color='red'" }})
indexApi.insert({"table" : "products",  "doc" : {"query" : "@title shoes","filters": "color IN ('blue', 'green')" }})
```
<!-- response Python -->
``` python
{'created': True,
 'found': None,
 'id': 0,
 'table': 'products',
 'result': 'created'}
{'created': True,
 'found': None,
 'id': 0,
 'table': 'products',
 'result': 'created'}
{'created': True,
 'found': None,
 'id': 0,
 'table': 'products',
 'result': 'created'}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await utilsApi.sql('create table products(title text, color string) type=\'pq\'');
res = indexApi.insert({"table" : "products", "doc" : {"query" : "@title bag" }});
res = indexApi.insert({"table" : "products",  "doc" : {"query" : "@title shoes", "filters": "color='red'" }});
res = indexApi.insert({"table" : "products",  "doc" : {"query" : "@title shoes","filters": "color IN ('blue', 'green')" }});
```
<!-- response javascript -->
``` javascript
"table":"products","_id":0,"created":true,"result":"created"}
{"table":"products","_id":0,"created":true,"result":"created"}
{"table":"products","_id":0,"created":true,"result":"created"}
```
<!-- intro -->
java
<!-- request Java -->

```java
utilsApi.sql("create table products(title text, color string) type='pq'");
doc = new HashMap<String,Object>(){{
    put("query", "@title bag");
}};
newdoc = new InsertDocumentRequest();
newdoc.index("products").setDoc(doc);
indexApi.insert(newdoc);

doc = new HashMap<String,Object>(){{
    put("query", "@title shoes");
    put("filters", "color='red'");
}};
newdoc = new InsertDocumentRequest();
newdoc.index("products").setDoc(doc);
indexApi.insert(newdoc);

doc = new HashMap<String,Object>(){{
    put("query", "@title shoes");
    put("filters", "color IN ('blue', 'green')");
}};
newdoc = new InsertDocumentRequest();
newdoc.index("products").setDoc(doc);
indexApi.insert(newdoc);
```
<!-- response Java -->
``` java
{total=0, error=, warning=}
class SuccessResponse {
    index: products
    id: 0
    created: true
    result: created
    found: null
}
class SuccessResponse {
    index: products
    id: 0
    created: true
    result: created
    found: null
}
class SuccessResponse {
    index: products
    id: 0
    created: true
    result: created
    found: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
utilsApi.Sql("create table products(title text, color string) type='pq'");

Dictionary<string, Object> doc = new Dictionary<string, Object>(); 
doc.Add("query", "@title bag");
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", doc: doc);
indexApi.Insert(newdoc);

doc = new Dictionary<string, Object>(); 
doc.Add("query", "@title shoes");
doc.Add("filters", "color='red'");
newdoc = new InsertDocumentRequest(index: "products", doc: doc);
indexApi.Insert(newdoc);

doc = new Dictionary<string, Object>(); 
doc.Add("query", "@title bag");
doc.Add("filters", "color IN ('blue', 'green')");
newdoc = new InsertDocumentRequest(index: "products", doc: doc);
indexApi.Insert(newdoc);
```
<!-- response C# -->
``` clike
{total=0, error="", warning=""}

class SuccessResponse {
    index: products
    id: 0
    created: true
    result: created
    found: null
}
class SuccessResponse {
    index: products
    id: 0
    created: true
    result: created
    found: null
}
class SuccessResponse {
    index: products
    id: 0
    created: true
    result: created
    found: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->
```typescript
res = await utilsApi.sql("create table test_pq(title text, color string) type='pq'");
res = indexApi.insert({
  index: 'test_pq',
  doc: { query : '@title bag' }
});
res = indexApi.insert(
  index: 'test_pq',
  doc: { query: '@title shoes', filters: "color='red'" }
});
res = indexApi.insert({
  index: 'test_pq',
  doc: { query : '@title shoes', filters: "color IN ('blue', 'green')" }
});
```
<!-- response TypeScript -->
``` typescript
{
	"table":"test_pq",
	"_id":1657852401006149661,
	"created":true,
	"result":"created"
}
{
	"table":"test_pq",
	"_id":1657852401006149662,
	"created":true,
	"result":"created"
}
{
	"table":"test_pq",
	"_id":1657852401006149663,
	"created":true,
	"result":"created"
}
```

<!-- intro -->
जाएं
<!-- request Go -->
```go
apiClient.UtilsAPI.Sql(context.Background()).Body("create table test_pq(title text, color string) type='pq'").Execute()

indexDoc := map[string]interface{} {"query": "@title bag"}
indexReq := manticoreclient.NewInsertDocumentRequest("test_pq", indexDoc)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute();

indexDoc = map[string]interface{} {"query": "@title shoes", "filters": "color='red'"}
indexReq = manticoreclient.NewInsertDocumentRequest("test_pq", indexDoc)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute();

indexDoc = map[string]interface{} {"query": "@title shoes", "filters": "color IN ('blue', 'green')"}
indexReq = manticoreclient.NewInsertDocumentRequest("test_pq", indexDoc)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute();
```
<!-- response Go -->
``` go
{
	"table":"test_pq",
	"_id":1657852401006149661,
	"created":true,
	"result":"created"
}
{
	"table":"test_pq",
	"_id":1657852401006149662,
	"created":true,
	"result":"created"
}
{
	"table":"test_pq",
	"_id":1657852401006149663,
	"created":true,
	"result":"created"
}
```

<!-- end -->

<!-- example single -->
##### बस मुझे बताएं कि PQ नियम मेरे एकल दस्तावेज़ से मेल खाते हैं

पहला दस्तावेज़ किसी भी नियम से मेल नहीं खाता। यह पहले दो से मेल खा सकता है, लेकिन उन्हें अतिरिक्त फ़िल्टर की आवश्यकता होती है।

दूसरा दस्तावेज़ एक नियम से मेल खाता है। ध्यान दें कि डिफ़ॉल्ट रूप से CALL PQ एक दस्तावेज़ की अपेक्षा करता है कि वह JSON हो, लेकिन यदि आप `0 as docs_json` का उपयोग करते हैं, तो आप इसके बजाय एक साधारण स्ट्रिंग पास कर सकते हैं।

<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products', 'Beautiful shoes', 0 as docs_json);

CALL PQ('products', 'What a nice bag', 0 as docs_json);
CALL PQ('products', '{"title": "What a nice bag"}');
```
<!-- response SQL -->

```sql
+---------------------+
| id                  |
+---------------------+
| 1657852401006149637 |
+---------------------+

+---------------------+
| id                  |
+---------------------+
| 1657852401006149637 |
+---------------------+
```
<!-- intro -->
JSON:
<!-- request JSON -->

```json
POST /pq/products/_search
{
  "query": {
    "percolate": {
      "document": {
        "title": "What a nice bag"
      }
    }
  }
}
```
<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "max_score": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149644,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
PHP:
<!-- request PHP -->

```php
$percolate = [
    'table' => 'products',
    'body' => [
        'query' => [
            'percolate' => [
                'document' => [
                    'title' => 'What a nice bag'
                ]
            ]
        ]
    ]
];
$client->pq()->search($percolate);

```
<!-- response PHP -->

```php
Array
(
    [took] => 0
    [timed_out] =>
    [hits] => Array
        (
            [total] => 1
            [max_score] => 1
            [hits] => Array
                (
                    [0] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 1657852401006149644
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => bag
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 1
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
searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"What a nice bag"}}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title bag'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"What a nice bag"}}}});
```
<!-- response javascript -->
``` javascript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233808,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
java
<!-- request Java -->

```java
PercolateRequest percolateRequest = new PercolateRequest();
query = new HashMap<String,Object>(){{
    put("percolate",new HashMap<String,Object >(){{
        put("document", new HashMap<String,Object >(){{
            put("title","what a nice bag");
        }});
    }});
}};
percolateRequest.query(query);
searchApi.percolate("test_pq",percolateRequest);
```
<!-- response Javs -->
``` java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234109, _score=1, _source={query={ql=@title bag}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
Dictionary<string, Object> percolateDoc = new Dictionary<string, Object>(); 
percolateDoc.Add("document", new Dictionary<string, Object> {{ "title", "what a nice bag" }});
Dictionary<string, Object> query = new Dictionary<string, Object> {{ "percolate", percolateDoc }}; 
PercolateRequest percolateRequest = new PercolateRequest(query=query);
searchApi.Percolate("test_pq",percolateRequest);

```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234109, _score=1, _source={query={ql=@title bag}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
res = await searchApi.percolate('test_pq', { query: { percolate: { document : { title : 'What a nice bag' } } } } );
```
<!-- response TypeScript -->
``` typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
query := map[string]interface{} {"title": "what a nice bag"}
percolateRequestQuery := manticoreclient.NewPercolateQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery)
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()

```
<!-- response Go -->
``` go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- end -->

<!-- example pq_rules -->
##### मुझे अपने दस्तावेज़ के साथ मेल खाने वाले पूरे PQ नियमों के बारे में जानना है
<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products', '{"title": "What a nice bag"}', 1 as query);
```
<!-- response SQL -->

```sql
+---------------------+------------+------+---------+
| id                  | query      | tags | filters |
+---------------------+------------+------+---------+
| 1657852401006149637 | @title bag |      |         |
+---------------------+------------+------+---------+
```
<!-- intro -->
JSON:
<!-- request JSON -->

```json
POST /pq/products/_search
{
  "query": {
    "percolate": {
      "document": {
        "title": "What a nice bag"
      }
    }
  }
}
```
<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "max_score": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149644,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
PHP:
<!-- request PHP -->

```php
$percolate = [
    'table' => 'products',
    'body' => [
        'query' => [
            'percolate' => [
                'document' => [
                    'title' => 'क्या प्यारा बैग है'
                ]
            ]
        ]
    ]
];
$client->pq()->search($percolate);

```
<!-- response PHP -->

```php
Array
(
    [took] => 0
    [timed_out] =>
    [hits] => Array
        (
            [total] => 1
            [max_score] => 1
            [hits] => Array
                (
                    [0] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 1657852401006149644
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => बैग
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 1
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
searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"क्या प्यारा बैग है"}}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title बैग'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"क्या प्यारा बैग है"}}}});
```
<!-- response javascript -->
``` javascript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233808,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title बैग"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
java
<!-- request Java -->

```java
PercolateRequest percolateRequest = new PercolateRequest();
query = new HashMap<String,Object>(){{
    put("percolate",new HashMap<String,Object >(){{
        put("document", new HashMap<String,Object >(){{
            put("title","क्या प्यारा बैग है");
        }});
    }});
}};
percolateRequest.query(query);
searchApi.percolate("test_pq",percolateRequest);
```
<!-- response Java -->
``` java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234109, _score=1, _source={query={ql=@title बैग}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
Dictionary<string, Object> percolateDoc = new Dictionary<string, Object>(); 
percolateDoc.Add("document", new Dictionary<string, Object> {{ "title", "क्या प्यारा बैग है" }});
Dictionary<string, Object> query = new Dictionary<string, Object> {{ "percolate", percolateDoc }}; 
PercolateRequest percolateRequest = new PercolateRequest(query=query);
searchApi.Percolate("test_pq",percolateRequest);

```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234109, _score=1, _source={query={ql=@title बैग}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
res = await searchApi.percolate('test_pq', { query: { percolate: { document : { title : 'क्या प्यारा बैग है' } } } } );
```
<!-- response TypeScript -->
``` typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title बैग"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
query := map[string]interface{} {"title": "क्या प्यारा बैग है"}
percolateRequestQuery := manticoreclient.NewPercolateQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery)
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()

```
<!-- response Go -->
``` go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- end -->

<!-- example multiple -->

##### एक से अधिक दस्तावेज़ के बारे में क्या?

ध्यान दें कि `CALL PQ` के साथ, आप विभिन्न तरीकों से कई दस्तावेज़ प्रदान कर सकते हैं:

* मौजूदा दस्तावेज़ों के एक सरणी के रूप में गोल ब्रैकेट्स में `('doc1', 'doc2')`। इसके लिए `0 as docs_json` की आवश्यकता है
* गोल ब्रैकेट्स में JSONs के एक सरणी के रूप में `('{doc1}', '{doc2}')`
* या मानक JSON सरणी के रूप में `'[{doc1}, {doc2}]'`

<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products', ('nice pair of shoes', 'beautiful bag'), 1 as query, 0 as docs_json);

CALL PQ('products', ('{"title": "nice pair of shoes", "color": "red"}', '{"title": "beautiful bag"}'), 1 as query);

CALL PQ('products', '[{"title": "nice pair of shoes", "color": "blue"}, {"title": "beautiful bag"}]', 1 as query);
```
<!-- response SQL -->

```sql
+---------------------+------------+------+---------+
| id                  | query      | tags | filters |
+---------------------+------------+------+---------+
| 1657852401006149637 | @title bag |      |         |
+---------------------+------------+------+---------+

+---------------------+--------------+------+-------------+
| id                  | query        | tags | filters     |
+---------------------+--------------+------+-------------+
| 1657852401006149636 | @title shoes |      | color='red' |
| 1657852401006149637 | @title bag   |      |             |
+---------------------+--------------+------+-------------+

+---------------------+--------------+------+---------------------------+
| id                  | query        | tags | filters                   |
+---------------------+--------------+------+---------------------------+
| 1657852401006149635 | @title shoes |      | color IN ('blue, 'green') |
| 1657852401006149637 | @title bag   |      |                           |
+---------------------+--------------+------+---------------------------+
```
<!-- intro -->
JSON:
<!-- request JSON -->

```json
POST /pq/products/_search
{
  "query": {
    "percolate": {
      "documents": [
        {"title": "nice pair of shoes", "color": "blue"},
        {"title": "beautiful bag"}
      ]
    }
  }
}
```
<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "max_score": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149644,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            2
          ]
        }
      },
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149646,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
PHP:
<!-- request PHP -->

```php
$percolate = [
    'table' => 'products',
    'body' => [
        'query' => [
            'percolate' => [
                'documents' => [
                    ['title' => 'nice pair of shoes','color'=>'blue'],
                    ['title' => 'beautiful bag']
                ]
            ]
        ]
    ]
];
$client->pq()->search($percolate);

```
<!-- response PHP -->

```php
Array
(
    [took] => 23
    [timed_out] =>
    [hits] => Array
        (
            [total] => 2
            [max_score] => 1
            [hits] => Array
                (
                    [0] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 2810781492890828819
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => bag
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 2
                                        )
                                )
                        )
                    [1] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 2810781492890828821
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => shoes
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 1
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
searchApi.percolate('products',{"query":{"percolate":{"documents":[{"title":"nice pair of shoes","color":"blue"},{"title":"beautiful bag"}]}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381494',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title bag'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [2]}},
                   {u'_id': u'2811025403043381496',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title shoes'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}

```
<!-- intro -->
जावास्क्रिप्ट
<!-- request javascript -->

```javascript
res = await searchApi.percolate('products',{"query":{"percolate":{"documents":[{"title":"nice pair of shoes","color":"blue"},{"title":"beautiful bag"}]}}});
```
<!-- response javascript -->
``` javascript
{
  "took": 6,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233808,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            2
          ]
        }
      },
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233810,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}

```

<!-- intro -->
जावा
<!-- request Java -->

```java
percolateRequest = new PercolateRequest();
query = new HashMap<String,Object>(){{
        put("percolate",new HashMap<String,Object >(){{
            put("documents", new ArrayList<Object>(){{
                    add(new HashMap<String,Object >(){{
                        put("title","nice pair of shoes");
                        put("color","blue");
                    }});
                    add(new HashMap<String,Object >(){{
                        put("title","beautiful bag");

                    }});

                     }});
        }});
    }};
percolateRequest.query(query);
searchApi.percolate("products",percolateRequest);

```
<!-- response Java -->
``` java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234133, _score=1, _source={query={ql=@title bag}}, fields={_percolator_document_slot=[2]}}, {_index=products, _type=doc, _id=2811045522851234135, _score=1, _source={query={ql=@title shoes}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
var doc1 = new Dictionary<string, Object>();
doc1.Add("title","nice pair of shoes");
doc1.Add("color","blue");
var doc2 = new Dictionary<string, Object>();
doc2.Add("title","beautiful bag");
var docs = new List<Object> {doc1, doc2};
Dictionary<string, Object> percolateDoc = new Dictionary<string, Object> {{ "documents", docs }}; 
Dictionary<string, Object> query = new Dictionary<string, Object> {{ "percolate", percolateDoc }}; 
PercolateRequest percolateRequest = new PercolateRequest(query=query);
searchApi.Percolate("products",percolateRequest);
```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234133, _score=1, _source={query={ql=@title bag}}, fields={_percolator_document_slot=[2]}}, {_index=products, _type=doc, _id=2811045522851234135, _score=1, _source={query={ql=@title shoes}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
टाइपस्क्रिप्ट
<!-- request TypeScript -->

```typescript
docs = [ {title : 'What a nice bag'}, {title : 'Really nice shoes'} ]; 
res = await searchApi.percolate('test_pq', { query: { percolate: { documents : docs } } } );
```
<!-- response TypeScript -->
``` typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
जाओ
<!-- request Go -->

```go
doc1 := map[string]interface{} {"title": "क्या सुंदर बैग है"}
doc2 := map[string]interface{} {"title": "वास्तव में सुंदर जूते"}
query := []interface{} {doc1, doc2}
percolateRequestQuery := manticoreclient.NewPercolateQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery)
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()

```
<!-- response Go -->
``` go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- end -->

<!-- example docs_1 -->
##### मैं जानना चाहता हूँ कि कौन से दस्तावेज़ कौन से नियमों से मेल खाते हैं

विकल्प `1 as docs` का उपयोग करने से आपको यह देखने की अनुमति मिलती है कि दिए गए दस्तावेज़ों में से कौन से नियमों से मेल खाते हैं।
<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products', '[{"title": "सुंदर जोड़ी जूते", "color": "नीला"}, {"title": "सुंदर बैग"}]', 1 as query, 1 as docs);
```

<!-- response SQL -->

```sql
+---------------------+-----------+--------------+------+---------------------------+
| id                  | documents | query        | tags | filters                   |
+---------------------+-----------+--------------+------+---------------------------+
| 1657852401006149635 | 1         | @title shoes |      | color IN ('blue, 'green') |
| 1657852401006149637 | 2         | @title bag   |      |                           |
+---------------------+-----------+--------------+------+---------------------------+
```
<!-- intro -->
JSON:
<!-- request JSON -->

```json
POST /pq/products/_search
{
  "query": {
    "percolate": {
      "documents": [
        {"title": "सुंदर जोड़ी जूते", "color": "नीला"},
        {"title": "सुंदर बैग"}
      ]
    }
  }
}
```
<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "max_score": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149644,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            2
          ]
        }
      },
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149646,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
PHP:
<!-- request PHP -->

```php
$percolate = [
    'table' => 'products',
    'body' => [
        'query' => [
            'percolate' => [
                'documents' => [
                    ['title' => 'सुंदर जोड़ी जूते','color'=>'नीला'],
                    ['title' => 'सुंदर बैग']
                ]
            ]
        ]
    ]
];
$client->pq()->search($percolate);

```
<!-- response PHP -->

```php
Array
(
    [took] => 23
    [timed_out] =>
    [hits] => Array
        (
            [total] => 2
            [max_score] => 1
            [hits] => Array
                (
                    [0] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 2810781492890828819
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => bag
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 2
                                        )
                                )
                        )
                    [1] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 2810781492890828821
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => जूते
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 1
                                        )
                                )
                        )
                )
        )
)
```
<!-- intro -->
पायथन
<!-- request Python -->

```python
searchApi.percolate('products',{"query":{"percolate":{"documents":[{"title":"सुंदर जोड़े के जूते","color":"नीला"},{"title":"खूबसूरत बैग"}]}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381494',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title बैग'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [2]}},
                   {u'_id': u'2811025403043381496',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title जूते'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}

```
<!-- intro -->
जावास्क्रिप्ट
<!-- request javascript -->

```javascript
res = await searchApi.percolate('products',{"query":{"percolate":{"documents":[{"title":"सुंदर जोड़े के जूते","color":"नीला"},{"title":"खूबसूरत बैग"}]}}});
```
<!-- response javascript -->
``` javascript
{
  "took": 6,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233808,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title बैग"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            2
          ]
        }
      },
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233810,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title जूते"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
जावा
<!-- request Java -->

```java
percolateRequest = new PercolateRequest();
query = new HashMap<String,Object>(){{
        put("percolate",new HashMap<String,Object >(){{
            put("documents", new ArrayList<Object>(){{
                    add(new HashMap<String,Object >(){{
                        put("title","सुंदर जोड़े के जूते");
                        put("color","नीला");
                    }});
                    add(new HashMap<String,Object >(){{
                        put("title","खूबसूरत बैग");

                    }});

                     }});
        }});
    }};
percolateRequest.query(query);
searchApi.percolate("products",percolateRequest);

```
<!-- response Java -->
``` java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234133, _score=1, _source={query={ql=@title बैग}}, fields={_percolator_document_slot=[2]}}, {_index=products, _type=doc, _id=2811045522851234135, _score=1, _source={query={ql=@title जूते}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
var doc1 = new Dictionary<string, Object>();
doc1.Add("title","सुंदर जोड़े के जूते");
doc1.Add("color","नीला");
var doc2 = new Dictionary<string, Object>();
doc2.Add("title","खूबसूरत बैग");
var docs = new List<Object> {doc1, doc2};
Dictionary<string, Object> percolateDoc = new Dictionary<string, Object> {{ "documents", docs }}; 
Dictionary<string, Object> query = new Dictionary<string, Object> {{ "percolate", percolateDoc }}; 
PercolateRequest percolateRequest = new PercolateRequest(query=query);
searchApi.Percolate("products",percolateRequest);
```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234133, _score=1, _source={query={ql=@title बैग}}, fields={_percolator_document_slot=[2]}}, {_index=products, _type=doc, _id=2811045522851234135, _score=1, _source={query={ql=@title जूते}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
टाइपस्क्रिप्ट
<!-- request TypeScript -->

```typescript
docs = [ {title : 'क्या खूबसूरत बैग'}, {title : 'वास्तव में अच्छे जूते'} ]; 
res = await searchApi.percolate('test_pq', { query: { percolate: { documents : docs } } } );
```
<!-- response TypeScript -->
``` typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title बैग"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title जूते"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
जाओ
<!-- request Go -->

```go
doc1 := map[string]interface{} {"title": "क्या एक अच्छा बैग"}
doc2 := map[string]interface{} {"title": "वास्तव में अच्छे जूते"}
query := []interface{} {doc1, doc2}
percolateRequestQuery := manticoreclient.NewPercolateQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery)
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()

```
<!-- response Go -->
``` go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title बैग"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title जूते"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- end -->

<!-- example docs_id -->
#### स्थैतिक आईडी
डिफ़ॉल्ट रूप से, मिलान दस्तावेज़ आईडी उस सूची में उनके सापेक्ष संख्या से मेल खाते हैं जो आप प्रदान करते हैं। हालांकि, कुछ मामलों में, प्रत्येक दस्तावेज़ पहले से ही अपनी खुद की आईडी रखता है। इस मामले के लिए, `CALL PQ` के लिए `'id field name' as docs_id` का एक विकल्प है।

ध्यान दें कि यदि आईडी को प्रदत्त फ़ील्ड नाम से नहीं खोजा जा सकता है, तो PQ नियम परिणामों में नहीं दिखाई देगा।

यह विकल्प केवल `CALL PQ` के लिए SQL के माध्यम से उपलब्ध है।

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CALL PQ('products', '[{"id": 123, "title": "अच्छे जूते की जोड़ी", "color": "नीला"}, {"id": 456, "title": "सुंदर बैग"}]', 1 as query, 'id' as docs_id, 1 as docs);
```
<!-- response SQL -->

```sql
+---------------------+-----------+--------------+------+---------------------------+
| id                  | documents | query        | tags | filters                   |
+---------------------+-----------+--------------+------+---------------------------+
| 1657852401006149664 | 456       | @title बैग   |      |                           |
| 1657852401006149666 | 123       | @title जूते |      | रंग IN ('नीला, 'हरा') |
+---------------------+-----------+--------------+------+---------------------------+
```
<!-- end -->

<!-- example invalid_json -->
##### मेरे पास अवैध JSON हो सकते हैं, कृपया उन्हें छोड़ दें

CALL PQ का उपयोग करते समय अलग-अलग JSON के साथ, आप अवैध JSON को छोड़ने के लिए 1 as skip_bad_json विकल्प का उपयोग कर सकते हैं। नीचे दिए गए उदाहरण में, दूसरा क्वेरी एक अवैध JSON के कारण विफल हो जाती है, लेकिन तीसरी क्वेरी 1 as skip_bad_json का उपयोग कर त्रुटि से बच जाती है। ध्यान रखें कि यह विकल्प HTTP के माध्यम से JSON क्वेरीज भेजते समय उपलब्ध नहीं है, क्योंकि इस मामले में पूरे JSON क्वेरी को वैध होना चाहिए।

<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products', ('{"title": "अच्छे जूते की जोड़ी", "color": "नीला"}', '{"title": "सुंदर बैग"}'));

CALL PQ('products', ('{"title": "अच्छे जूते की जोड़ी", "color": "नीला"}', '{"title": "सुंदर बैग}'));

CALL PQ('products', ('{"title": "अच्छे जूते की जोड़ी", "color": "नीला"}', '{"title": "सुंदर बैग}'), 1 as skip_bad_json);
```
<!-- response SQL -->

```sql
+---------------------+
| id                  |
+---------------------+
| 1657852401006149635 |
| 1657852401006149637 |
+---------------------+

ERROR 1064 (42000): स्ट्रिंग में खराब JSON ऑब्जेक्ट: 2

+---------------------+
| id                  |
+---------------------+
| 1657852401006149635 |
+---------------------+
```

<!-- end -->

##### मैं एक पेरकोलेट प्रश्न की उच्च प्रदर्शन चाहता हूँ
पेरकोलेट प्रश्न उच्च थ्रूपुट और बड़े डेटा वॉल्यूम के लिए डिज़ाइन किए गए हैं। निम्न लेटेंसी और उच्च थ्रूपुट के लिए प्रदर्शन को ऑप्टिमाइज़ करने के लिए, निम्नलिखित पर विचार करें।

एक पेरकोलेट तालिका के लिए वितरण के दो मोड हैं और इसके खिलाफ पेरकोलेट प्रश्न कैसे काम कर सकता है:

* **Sparse (डिफ़ॉल्ट)।** आदर्श: कई दस्तावेज़, मिरर PQ तालिकाएँ। जब आपका दस्तावेज़ सेट बड़ा होता है लेकिन PQ तालिका में संग्रहीत प्रश्नों का सेट छोटा होता है, तो स्पार्स मोड फायदेमंद होता है। इस मोड में, आप जो दस्तावेज़ भेजते हैं, उन्हें एजेंटों की संख्या के बीच विभाजित किया जाएगा, इसलिए प्रत्येक नोड आपके अनुरोध के दस्तावेज़ों के केवल एक हिस्से को संसाधित करता है। Manticore आपके दस्तावेज़ सेट को विभाजित करता है और उसके भागों को मिरर के बीच वितरित करता है। एक बार जब एजेंट प्रश्नों को संसाधित करना समाप्त कर लेते हैं, तो Manticore परिणामों को एकत्र करता है और मिलाता है, एक अंतिम प्रश्न सेट लौटाता है जैसे कि यह एक एकल तालिका से आया हो। प्रक्रिया की सहायता के लिए [प्रतिगमन](../References.md#Replication) का उपयोग करें।
* **शार्डेड।** आदर्श के लिए: कई PQ नियम, PQ तालिकाओं में विभाजित नियम। इस मोड में, पूरा दस्तावेज़ सेट वितरित PQ तालिका की सभी तालिकाओं में प्रसारित किया जाता है बिना प्रारंभ में दस्तावेज़ों को विभाजित किए। यह तब लाभदायक होता है जब एक अपेक्षाकृत छोटे दस्तावेज़ों के सेट को पंप करने के लिए, लेकिन संग्रहीत प्रश्नों की संख्या बड़ी होती है। इस मामले में, यह अधिक उपयुक्त है कि प्रत्येक नोड पर केवल PQ नियमों का एक भाग रखने के लिए और फिर उन नोड्स से लौटाए गए परिणामों को मर्ज करना जो भिन्न PQ नियमों के खिलाफ एक ही दस्तावेज़ सेट को प्रक्रिया करते हैं। इस मोड को स्पष्ट रूप से सेट करने की आवश्यकता है, क्योंकि इसका मतलब है नेटवर्क पेलोड का बढ़ना और यह भिन्न PQs वाली तालिकाओं की अपेक्षा करता है, जिसे [प्रतिकर्ता](../References.md#Replication) बिना किसी बदलाव के करने में असमर्थ है।

मान लीजिए आपके पास तालिका `pq_d2` इस प्रकार परिभाषित है:

``` ini
table pq_d2
{
    type = distributed
    agent = 127.0.0.1:6712:pq
    agent = 127.0.0.1:6712:ptitle
}
```

<!-- उदाहरण वितरित pq मोड 1 -->
'pq' और 'ptitle' में से प्रत्येक में शामिल हैं:


<!-- परिचय -->
##### SQL:

<!-- अनुरोध SQL -->

```sql
SELECT * FROM pq;
```

<!-- प्रतिक्रिया sql -->

```sql
+------+-------------+------+-------------------+
| id   | query       | tags | filters           |
+------+-------------+------+-------------------+
|    1 | filter test |      | gid>=10           |
|    2 | angry       |      | gid>=10 OR gid<=3 |
+------+-------------+------+-------------------+
2 rows in set (0.01 sec)
```

<!-- अनुरोध JSON -->

```json
POST /pq/pq/_search
```

<!-- प्रतिक्रिया JSON -->

```json
{
    "took":0,
    "timed_out":false,
    "hits":{
        "total":2,
        "hits":[
            {
                "_id": 1,
                "_score":1,
                "_source":{
                    "query":{ "ql":"filter test" },
                    "tags":"",
                    "filters":"gid>=10"
                }
            },
            {
                "_id": 2,
                "_score":1,
                "_source":{
                    "query":{"ql":"angry"},
                    "tags":"",
                    "filters":"gid>=10 OR gid<=3"
                }
            }            
        ]
    }
}
```

<!-- अनुरोध PHP -->

```php
$params = [
    'table' => 'pq',
    'body' => [
    ]
];
$response = $client->pq()->search($params);
```

<!-- प्रतिक्रिया PHP -->

```php
(
    [took] => 0
    [timed_out] =>
    [hits] =>
        (
            [total] => 2
            [hits] =>
                (
                    [0] =>
                        (
                            [_id] => 1
                            [_score] => 1
                            [_source] =>
                                (
                                    [query] =>
                                        (
                                            [ql] => filter test
                                        )
                                    [tags] =>
                                    [filters] => gid>=10
                                )
                        ),
                    [1] =>
                        (
                            [_id] => 1
                            [_score] => 1
                            [_source] =>
                                (
                                    [query] =>
                                        (
                                            [ql] => angry
                                        )
                                    [tags] =>
                                    [filters] => gid>=10 OR gid<=3
                                )
                        )
                )
        )
)
```
<!-- परिचय -->
पायथन
<!-- अनुरोध पायथन -->

```python
searchApi.search({"table":"pq","query":{"match_all":{}}})
```
<!-- प्रतिक्रिया पायथन -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381501',
                    u'_score': 1,
                    u'_source': {u'filters': u"gid>=10",
                                 u'query': u'filter test',
                                 u'tags': u''}},
                   {u'_id': u'2811025403043381502',
                    u'_score': 1,
                    u'_source': {u'filters': u"gid>=10 OR gid<=3",
                                 u'query': u'angry',
                                 u'tags': u''}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- परिचय -->
जावास्क्रिप्ट
<!-- अनुरोध जावास्क्रिप्ट -->

```javascript
res = await searchApi.search({"table":"pq","query":{"match_all":{}}});
```
<!-- प्रतिक्रिया जावास्क्रिप्ट -->
``` javascript
{'hits': {'hits': [{u'_id': u'2811025403043381501',
                    u'_score': 1,
                    u'_source': {u'filters': u"gid>=10",
                                 u'query': u'filter test',
                                 u'tags': u''}},
                   {u'_id': u'2811025403043381502',
                    u'_score': 1,
                    u'_source': {u'filters': u"gid>=10 OR gid<=3",
                                 u'query': u'angry',
                                 u'tags': u''}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- परिचय -->
जावास्क्रिप्ट
<!-- अनुरोध जावास्क्रिप्ट -->

```javascript
res = await searchApi.search({"table":"pq","query":{"match_all":{}}});
```
<!-- प्रतिक्रिया जावास्क्रिप्ट -->
``` javascript
{"hits": {"hits": [{"_id": 2811025403043381501,
                    "_score": 1,
                    "_source": {"filters": u"gid>=10",
                                 "query": "filter test",
                                 "tags": ""}},
                   {"_id": 2811025403043381502,
                    "_score": 1,
                    "_source": {"filters": u"gid>=10 OR gid<=3",
                                 "query": "angry",
                                 "tags": ""}}],
          "total": 2},
  "timed_out": false,
 "took": 0}
```

<!-- intro -->
java
<!-- request Java -->

```java
Map<String,Object> query = new HashMap<String,Object>();
query.put("match_all",null);
SearchRequest searchRequest = new SearchRequest();
searchRequest.setIndex("pq");
searchRequest.setQuery(query);
SearchResponse searchResponse = searchApi.search(searchRequest);
```
<!-- response Java -->
``` java

class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: null
        hits: [{_id=2811045522851233962, _score=1, _source={filters=gid>=10, query=filter test, tags=}}, {_id=2811045522851233951, _score=1, _source={filters=gid>=10 OR gid<=3, query=angry,tags=}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
object query =  new { match_all=null };
SearchRequest searchRequest = new SearchRequest("pq", query);
SearchResponse searchResponse = searchApi.Search(searchRequest);
```
<!-- response C# -->
``` clike

class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: null
        hits: [{_id=2811045522851233962, _score=1, _source={filters=gid>=10, query=filter test, tags=}}, {_id=2811045522851233951, _score=1, _source={filters=gid>=10 OR gid<=3, query=angry,tags=}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
res = await searchApi.search({"table":"test_pq","query":{"match_all":{}}});
```
<!-- response TypeScript -->
``` typescript
{
	'hits':
	{
		'hits': 
		[{
			'_id': '2811025403043381501',
            '_score': 1,
            '_source': 
            {
            	'filters': "gid>=10",
                'query': 'filter test',
                'tags': ''
            }
        },
        {
         	'_id': 
         	'2811025403043381502',
            '_score': 1,
            '_source': 
            {
            	'filters': "gid>=10 OR gid<=3",
                 'query': 'angry',
                 'tags': ''
            }
        }],
    	'total': 2
	},
	'profile': None,
	'timed_out': False,
	'took': 0
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
query := map[string]interface{} {}
percolateRequestQuery := manticoreclient.NewPercolateRequestQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery) 
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()
```
<!-- response Go -->
``` go
{
	'hits':
	{
		'hits': 
		[{
			'_id': '2811025403043381501',
            '_score': 1,
            '_source': 
            {
            	'filters': "gid>=10",
                'query': 'filter test',
                'tags': ''
            }
        },
        {
         	'_id': 
         	'2811025403043381502',
            '_score': 1,
            '_source': 
            {
            	'filters': "gid>=10 OR gid<=3",
                 'query': 'angry',
                 'tags': ''
            }
        }],
    	'total': 2
	},
	'profile': None,
	'timed_out': False,
	'took': 0
}
```

<!-- end -->


<!-- example call_pq_example -->

And you execute `CALL PQ` on the distributed table with a couple of documents.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CALL PQ ('pq_d2', ('{"title":"angry test", "gid":3 }', '{"title":"filter test doc2", "gid":13}'), 1 AS docs);
```

<!-- response sql -->

```sql
+------+-----------+
| id   | documents |
+------+-----------+
|    1 | 2         |
|    2 | 1         |
+------+-----------+
```

<!-- request JSON -->

```json
POST /pq/pq/_search -d '
"query":
{
        "percolate":
        {
                "documents" : [
                    { "title": "angry test", "gid": 3 },
                    { "title": "filter test doc2", "gid": 13 }
                ]
        }
}
'
```

<!-- response JSON -->

```json
{
    "took":0,
    "timed_out":false,
    "hits":{
    "total":2,"hits":[
        {
            "_id": 2,
            "_score":1,
            "_source":{
                "query":{"title":"angry"},
                "tags":"",
                "filters":"gid>=10 OR gid<=3"
            }
        }
        {
            "_id": 1,
            "_score":1,
            "_source":{
                "query":{"ql":"filter test"},
                "tags":"",
                "filters":"gid>=10"
            }
        },
        ]
    }
}
```

<!-- request PHP -->

```php
$params = [
    'table' => 'pq',
    'body' => [
        'query' => [
            'percolate' => [
                'documents' => [
                    [
                        'title'=>'angry test',
                        'gid' => 3
                    ],
                    [
                        'title'=>'filter test doc2',
                        'gid' => 13
                    ],
                ]
            ]
        ]
    ]
];
$response = $client->pq()->search($params);
```

<!-- response PHP -->

```php
(
    [took] => 0
    [timed_out] =>
    [hits] =>
        (
            [total] => 2
            [hits] =>
                (
                    [0] =>
                        (
                            [_index] => pq  
                            [_type] => doc                            
                            [_id] => 2
                            [_score] => 1
                            [_source] =>
                                (
                                    [query] =>
                                        (
                                            [ql] => गुस्से में
                                        )
                                    [tags] =>
                                    [filters] => gid>=10 OR gid<=3
                                ),
                           [fields] =>
                                (
                                    [_percolator_document_slot] =>
                                        (
                                            [0] => 1
                                        )

                                )
                        ),
                    [1] =>
                        (
                            [_index] => pq                            
                            [_id] => 1
                            [_score] => 1
                            [_source] =>
                                (
                                    [query] =>
                                        (
                                            [ql] => फ़िल्टर परीक्षण
                                        )
                                    [tags] =>
                                    [filters] => gid>=10
                                )
                           [fields] =>
                                (
                                    [_percolator_document_slot] =>
                                        (
                                            [0] => 0
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
searchApi.percolate('pq',{"percolate":{"documents":[{"title":"गुस्से में परीक्षण","gid":3},{"title":"फ़िल्टर परीक्षण doc2","gid":13}]}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'table': u'pq',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'गुस्से में'},u'tags':u'',u'filters':u"gid>=10 OR gid<=3"},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}},
                    {u'_id': u'2811025403043381501',
                    u'table': u'pq',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'फ़िल्टर परीक्षण'},u'tags':u'',u'filters':u"gid>=10"},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate('pq',{"percolate":{"documents":[{"title":"गुस्से में परीक्षण","gid":3},{"title":"फ़िल्टर परीक्षण doc2","gid":13}]}});
```
<!-- response javascript -->
``` javascript
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'table': u'pq',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'गुस्से में'},u'tags':u'',u'filters':u"gid>=10 OR gid<=3"},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}},
                    {u'_id': u'2811025403043381501',
                    u'table': u'pq',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'फ़िल्टर परीक्षण'},u'tags':u'',u'filters':u"gid>=10"},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
java
<!-- request Java -->

```java
percolateRequest = new PercolateRequest();
query = new HashMap<String,Object>(){{
    put("percolate",new HashMap<String,Object >(){{
        put("documents", new ArrayList<Object>(){{
            add(new HashMap<String,Object >(){{
                put("title","गुस्से में परीक्षण");
                put("gid",3);
            }});
            add(new HashMap<String,Object >(){{
                put("title","फ़िल्टर परीक्षण doc2");
                put("gid",13);
            }});
        }});
    }});
}};
percolateRequest.query(query);
searchApi.percolate("pq",percolateRequest);
```
<!-- response java -->
``` java
class SearchResponse {
    took: 10
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=pq, _type=doc, _id=2811045522851234165, _score=1, _source={query={ql=@title गुस्से में}}, fields={_percolator_document_slot=[1]}}, {_index=pq, _type=doc, _id=2811045522851234166, _score=1, _source={query={ql=@title फ़िल्टर परीक्षण doc2}}, fields={_percolator_document_slot=[2]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
var doc1 = new Dictionary<string, Object>();
doc1.Add("title","गुस्से में परीक्षण");
doc1.Add("gid",3);
var doc2 = new Dictionary<string, Object>();
doc2.Add("title","फ़िल्टर परीक्षण doc2");
doc2.Add("gid",13);
var docs = new List<Object> {doc1, doc2};
Dictionary<string, Object> percolateDoc = new Dictionary<string, Object> {{ "documents", docs }}; 
Dictionary<string, Object> query = new Dictionary<string, Object> {{ "percolate", percolateDoc }}; 
PercolateRequest percolateRequest = new PercolateRequest(query=query);
searchApi.Percolate("pq",percolateRequest);
```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 10
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=pq, _type=doc, _id=2811045522851234165, _score=1, _source={query={ql=@title angry}}, fields={_percolator_document_slot=[1]}}, {_index=pq, _type=doc, _id=2811045522851234166, _score=1, _source={query={ql=@title filter test doc2}}, fields={_percolator_document_slot=[2]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
docs = [ {title : 'What a nice bag'}, {title : 'Really nice shoes'} ]; 
res = await searchApi.percolate('test_pq', { query: { percolate: { documents : docs } } } );
```
<!-- response TypeScript -->
``` typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
doc1 := map[string]interface{} {"title": "What a nice bag"}
doc2 := map[string]interface{} {"title": "Really nice shoes"}
query := []interface{} {doc1, doc2}
percolateRequestQuery := manticoreclient.NewPercolateQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery)
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()

```
<!-- response Go -->
``` go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- end -->

In the previous example, we used the default **sparse** mode. To demonstrate the **sharded** mode, let's create a distributed PQ table consisting of 2 local PQ tables and add 2 documents to "products1" and 1 document to "products2":
```sql
create table products1(title text, color string) type='pq';
create table products2(title text, color string) type='pq';
create table products_distributed type='distributed' local='products1' local='products2';

INSERT INTO products1(query) values('@title bag');
INSERT INTO products1(query,filters) values('@title shoes', 'color=\'red\'');
INSERT INTO products2(query,filters) values('@title shoes', 'color in (\'blue\', \'green\')');
```

<!-- example sharded -->
Now, if you add `'sharded' as mode` to `CALL PQ`, it will send the documents to all the agent's tables (in this case, just local tables, but they can be remote to utilize external hardware). This mode is not available via the JSON interface.

<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products_distributed', ('{"title": "nice pair of shoes", "color": "blue"}', '{"title": "beautiful bag"}'), 'sharded' as mode, 1 as query);
```
<!-- response SQL -->

```sql
+---------------------+--------------+------+---------------------------+
| id                  | query        | tags | filters                   |
+---------------------+--------------+------+---------------------------+
| 1657852401006149639 | @title bag   |      |                           |
| 1657852401006149643 | @title shoes |      | color IN ('blue, 'green') |
+---------------------+--------------+------+---------------------------+
```

<!-- end -->

Note that the syntax of agent mirrors in the configuration (when several hosts are assigned to one `agent` line, separated with `|`) has nothing to do with the `CALL PQ` query mode. Each `agent` always represents **one** node, regardless of the number of HA mirrors specified for that agent.

<!-- example verbose -->
##### How can I learn more about performance?
In some cases, you might want to get more details about the performance of a percolate query. For that purpose, there is the option `1 as verbose`, which is only available via SQL and allows you to save more performance metrics. You can see them using the `SHOW META` query, which you can run after `CALL PQ`. See [SHOW META](../Node_info_and_management/SHOW_META.md) for more info.

<!-- intro -->
1 as verbose:
<!-- request 1 as verbose -->

```sql
CALL PQ('products', ('{"title": "nice pair of shoes", "color": "blue"}', '{"title": "beautiful bag"}'), 1 as verbose); show meta;
```
<!-- response 1 as verbose -->

```sql
+---------------------+
| id                  |
+---------------------+
| 1657852401006149644 |
| 1657852401006149646 |
+---------------------+
+-------------------------+-----------+
| Variable name           | Value     |
+-------------------------+-----------+
| total                   | 0.000 सेक |
| setup                   | 0.000 सेक |
| queries_matched         | 2         |
| queries_failed          | 0         |
| document_matched        | 2         |
| total_queries_stored    | 3         |
| term_only_queries       | 3         |
| fast_rejected_queries   | 0         |
| time_per_query          | 27, 10    |
| time_of_matched_queries | 37        |
+-------------------------+-----------+
```
<!-- intro -->
0 as verbose (default):
<!-- request 0 as verbose -->

```sql
CALL PQ('products', ('{"title": "nice pair of shoes", "color": "blue"}', '{"title": "beautiful bag"}'), 0 as verbose); show meta;
```
<!-- response 0 as verbose -->

```sql
+---------------------+
| id                  |
+---------------------+
| 1657852401006149644 |
| 1657852401006149646 |
+---------------------+
+-----------------------+-----------+
| Variable name         | Value     |
+-----------------------+-----------+
| total                 | 0.000 सेक |
| queries_matched       | 2         |
| queries_failed        | 0         |
| document_matched      | 2         |
| total_queries_stored  | 3         |
| term_only_queries     | 3         |
| fast_rejected_queries | 0         |
+-----------------------+-----------+
```
<!-- end -->
<!-- proofread -->
