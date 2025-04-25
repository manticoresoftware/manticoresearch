# दस्तावेज़ हटाना

दस्तावेज़ हटाना केवल [RT मोड](../../Read_this_first.md#Real-time-mode-vs-plain-mode) में निम्नलिखित तालिका प्रकारों के लिए समर्थित है:
* [रीयल-टाइम](../Creating_a_table/Local_tables/Real-time_table.md) तालिकाएँ
* [पर्कोलेट](../Creating_a_table/Local_tables/Percolate_table.md) तालिकाएँ
* वितरित तालिकाएँ जो केवल स्थानीय या दूरस्थ एजेंटों के रूप में RT तालिकाएँ शामिल करती हैं।

आप किसी तालिका से मौजूदा दस्तावेज़ों को उनके ID या कुछ शर्तों के आधार पर हटा सकते हैं।

इसके अलावा, [थोक हटाने](../Data_creation_and_modification/Deleting_documents.md#Bulk-deletion) के लिए एक विकल्प उपलब्ध है जिससे आप कई दस्तावेज़ों को हटा सकते हैं।

दस्तावेज़ों का हटाया जाना SQL और JSON इंटरफेस दोनों के माध्यम से किया जा सकता है।

SQL के लिए, सफल संचालन के लिए प्रतिक्रिया में हटाए गए पंक्तियों की संख्या दिखाई जाएगी।

JSON के लिए, `json/delete` एंडपॉइंट का उपयोग किया जाता है। सर्वर एक JSON ऑब्जेक्ट के साथ प्रतिक्रिया देगा जिसमें बताया जाएगा कि संचालन सफल था या नहीं और हटाए गए पंक्तियों की संख्या।

तालिका से सभी दस्तावेज़ों को हटाने के लिए हटाने के बजाय [तालिका का ट्रंकेशन](../Emptying_a_table.md) उपयोग करने की सिफारिश की जाती है, क्योंकि यह एक बहुत तेज़ प्रक्रिया है।


<!-- example delete 2 -->
इस उदाहरण में हम तालिका नाम `test` से पूर्ण-टेक्स्ट प्रश्न `test document` से मेल खाने वाले सभी दस्तावेज़ों को हटाते हैं:


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
mysql> SELECT * FROM TEST;
+------+------+-------------+------+
| id   | gid  | mva1        | mva2 |
+------+------+-------------+------+
|  100 | 1000 | 100,201     | 100  |
|  101 | 1001 | 101,202     | 101  |
|  102 | 1002 | 102,203     | 102  |
|  103 | 1003 | 103,204     | 103  |
|  104 | 1004 | 104,204,205 | 104  |
|  105 | 1005 | 105,206     | 105  |
|  106 | 1006 | 106,207     | 106  |
|  107 | 1007 | 107,208     | 107  |
+------+------+-------------+------+
8 rows in set (0.00 sec)

mysql> DELETE FROM TEST WHERE MATCH ('test document');
Query OK, 2 rows affected (0.00 sec)

mysql> SELECT * FROM TEST;
+------+------+-------------+------+
| id   | gid  | mva1        | mva2 |
+------+------+-------------+------+
|  100 | 1000 | 100,201     | 100  |
|  101 | 1001 | 101,202     | 101  |
|  102 | 1002 | 102,203     | 102  |
|  103 | 1003 | 103,204     | 103  |
|  104 | 1004 | 104,204,205 | 104  |
|  105 | 1005 | 105,206     | 105  |
+------+------+-------------+------+
6 rows in set (0.00 sec)
```

<!-- request JSON -->

``` json
POST /delete -d '
    {
        "table":"test",
        "query":
        {
            "match": { "*": "test document" }
        }
    }'
```

* JSON के लिए `query` में पूर्ण-टेक्स्ट खोज के लिए एक क्लॉज़ है; इसका सिंटैक्स [JSON/update](../Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-HTTP-JSON) में उसी प्रकार का है।

<!-- response JSON -->

``` json
{
    "table":"test",
    "deleted":2,
}
```    
<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->deleteDocuments(new MatchPhrase('test document','*'));
```

<!-- response json -->

``` php
Array(
    [_index] => test
    [deleted] => 2
)
```
<!-- intro -->

##### Python:

<!-- request Python -->
``` python
indexApi.delete({"table" : "test", "query": { "match": { "*": "test document" }}})
```

<!-- response Python -->
```python
{'deleted': 5, 'id': None, 'table': 'test', 'result': None}
```

<!-- intro -->

##### Python-asyncio:

<!-- request Python-asyncio -->
``` python
indexApi.delete({"table" : "test", "query": { "match": { "*": "test document" }}})
```

<!-- response Python-asyncio -->
```python
{'deleted': 5, 'id': None, 'table': 'test', 'result': None}
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.delete({"table" : "test", "query": { "match": { "*": "test document" }}});
```

<!-- response javascript -->
```javascript
{"table":"test","deleted":5}
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest();
query = new HashMap<String,Object>();
query.put("match",new HashMap<String,Object>(){{
    put("*","test document");
}});
deleteRequest.index("test").setQuery(query);
indexApi.delete(deleteRequest);

```

<!-- response Java -->
```java
class DeleteResponse {
    index: test
    deleted: 5
    id: null
    result: null
}
```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
Dictionary<string, Object> match = new Dictionary<string, Object>();
match.Add("*", "test document");
Dictionary<string, Object> query = new Dictionary<string, Object>();
query.Add("match", match);
DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest(index: "test", query: query);
indexApi.Delete(deleteRequest);

```

<!-- response C# -->
```clike
class DeleteResponse {
    index: test
    deleted: 5
    id: null
    result: null
}
```

<!-- intro -->

##### Rust:

<!-- request Rust -->
``` rust
let match_expr = HashMap::new();
match_expr.insert("*".to_string(), serde_json::json!("test document"));
let query = SearchQuery {
     match: Some(serde_json::json!(match_expr).into()),
    ..Default::default()
};

let delete_req = DeleteDocumentRequest::new("test".to_string());
index_api.delete(delete_req).await;
```

<!-- response Rust -->
```rust
class DeleteResponse {
    index: test
    deleted: 5
    id: null
    result: null
}
```

<!-- intro -->

##### TypeScript:

<!-- request TypeScript -->
``` typescript
res = await indexApi.delete({
  index: 'test',
  query: { match: { '*': 'test document' } },
});
```
<!-- response TypeScript -->
```json
{"table":"test","deleted":5}
```

<!-- intro -->

##### जाओ:

<!-- request Go -->
```go
deleteRequest := manticoresearch.NewDeleteDocumentRequest("test")
matchExpr := map[string]interface{} {"*": "test document"}
deleteQuery := map[string]interface{} {"match": matchExpr }
deleteRequest.SetQuery(deleteQuery)
```

<!-- response Go -->
```json
{"table":"test","deleted":5}
```

<!-- end -->

<!-- example delete 3 -->
यहाँ - `test` नामक तालिका से `id` जो 1 के बराबर है, एक दस्तावेज़ को मिटाना:


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
mysql> DELETE FROM TEST WHERE id=1;
Query OK, 1 rows affected (0.00 sec)
```

<!-- request JSON -->

``` json
POST /delete -d '
    {
        "table": "test",
        "id": 1
    }'
```

* JSON के लिए `id` वह पंक्ति `id` है जिसे मिटाना चाहिए।

<!-- response JSON -->

``` json
{
    "table": "test",
    "_id": 1,
    "found": true,
    "result": "deleted"      
}
```    
<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->deleteDocument(1);
```

<!-- response json -->

``` php
Array(
    [_index] => test
    [_id] => 1
    [found] => true
    [result] => deleted
)
```
<!-- intro -->

##### पायथन:

<!-- request Python -->
``` python
indexApi.delete({"table" : "test", "id" : 1})
```

<!-- response Python -->
```python
{'deleted': None, 'id': 1, 'table': 'test', 'result': 'deleted'}
```

<!-- intro -->

##### Python-asyncio:

<!-- request Python-asyncio -->
``` python
indexApi.delete({"table" : "test", "id" : 1})
```

<!-- response Python-asyncio -->
```python
{'deleted': None, 'id': 1, 'table': 'test', 'result': 'deleted'}
```

<!-- intro -->

##### जावास्क्रिप्ट:

<!-- request javascript -->
``` javascript
res = await indexApi.delete({"table" : "test", "id" : 1});
```

<!-- response javascript -->
```javascript
{"table":"test","_id":1,"result":"deleted"}
```

<!-- intro -->

##### जावा:

<!-- request Java -->
``` java
DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest();
deleteRequest.index("test").setId(1L);
indexApi.delete(deleteRequest);

```

<!-- response Java -->
```java
class DeleteResponse {
    index: test
    _id: 1
    result: deleted
}
```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest(index: "test", id: 1);
indexApi.Delete(deleteRequest);

```

<!-- response C# -->
```java
class DeleteResponse {
    index: test
    _id: 1
    result: deleted
}
```

<!-- intro -->

##### रस्सी:

<!-- request Rust -->
``` rust
let delete_req = DeleteDocumentRequest {
    table: "test".to_string(),
    id: serde_json::json!(1),
    ..Default::default(),
};

index_api.delete(delete_req).await;

```

<!-- response Rust -->
```rust
class DeleteResponse {
    index: test
    _id: 1
    result: deleted
}
```

<!-- intro -->

##### TypeScript:

<!-- request TypeScript -->
``` typescript
res = await indexApi.delete({ index: 'test', id: 1 });
```

<!-- response TypeScript -->
```json
{"table":"test","_id":1,"result":"deleted"}
```

<!-- intro -->

##### जाओ:

<!-- request Go -->
```go
deleteRequest := manticoresearch.NewDeleteDocumentRequest("test")
deleteRequest.SetId(1)
```

<!-- response Go -->
```json
{"table":"test","_id":1,"result":"deleted"}
```

<!-- end -->

<!-- example delete 4 -->
यहाँ, `test` नामक तालिका से मेल खाने वाले `id` के साथ दस्तावेज़ मिटाए जाते हैं:

ध्यान दें कि `id=N` या `id IN (X,Y)` के साथ मिटाने के रूप सबसे तेज़ होते हैं, क्योंकि वे दस्तावेज़ों को खोज किए बिना हटा देते हैं।
यह भी ध्यान दें कि प्रतिक्रिया में केवल पहले मिटाए गए दस्तावेज़ का id संबंधित `_id` फ़ील्ड में होता है।

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
DELETE FROM TEST WHERE id IN (1,2);

```

<!-- response SQL -->

```sql
Query OK, 2 rows affected (0.00 sec)
```

<!-- request JSON -->

``` json
POST /delete -d '
    {
        "table":"test",
        "id": [1,2]
    }'
```

<!-- response JSON -->

``` json
    {
        "table":"test",
        "_id":1,
        "found":true,
        "result":"deleted"      
    }
```    

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->deleteDocumentsByIds([1,2]);
```

<!-- response json -->

``` php
Array(
    [_index] => test
    [_id] => 1
    [found] => true
    [result] => deleted
)
```

<!-- end -->

<!-- example delete 5 -->
Manticore SQL `DELETE` कथन के लिए जटिल शर्तें उपयोग करने की अनुमति देता है।

उदाहरण के लिए यहाँ हम उन दस्तावेज़ों को मिटा रहे हैं जो पूर्ण-पाठ खोज `test document` से मेल खाते हैं और जिनके गुण `mva1` की मान 206 से अधिक है या `mva1` मान 100 या 103 हैं, `test` नामक तालिका से:


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
DELETE FROM TEST WHERE MATCH ('test document') AND ( mva1>206 or mva1 in (100, 103) );

SELECT * FROM TEST;
```

<!-- response SQL -->

```sql
Query OK, 4 rows affected (0.00 sec)

+------+------+-------------+------+
| id   | gid  | mva1        | mva2 |
+------+------+-------------+------+
|  101 | 1001 | 101,202     | 101  |
|  102 | 1002 | 102,203     | 102  |
|  104 | 1004 | 104,204,205 | 104  |
|  105 | 1005 | 105,206     | 105  |
+------+------+-------------+------+
6 rows in set (0.00 sec)
```
<!-- end -->
<!-- example delete 6 -->
यहाँ `क्लस्टर` के तालिका `परीक्षा` में दस्तावेज़ को हटाने का एक उदाहरण है। ध्यान दें कि हमें पुनरुत्पादन क्लस्टर के भीतर तालिका से एक पंक्ति को हटाने के लिए तालिका संपत्ति के साथ क्लस्टर नाम संपत्ति प्रदान करनी चाहिए:


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
delete from cluster:test where id=100;
```

<!-- request JSON -->
``` json
POST /delete -d '
    {
      "cluster": "cluster",
      "table": "test",
      "id": 100
    }'
```
* JSON के लिए `cluster` वह नाम है [पुनरुत्पादन क्लस्टर](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster)। जिसमें आवश्यक तालिका शामिल है

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->setCluster('cluster');
$index->deleteDocument(100);
```

<!-- response JSON -->

``` php
Array(
    [_index] => test
    [_id] => 100
    [found] => true
    [result] => deleted
)
```
<!-- intro -->

##### Python:

<!-- request Python -->
``` python
indexApi.delete({"cluster":"cluster","table" : "test", "id" : 1})
```

<!-- response Python -->
```python
{'deleted': None, 'id': 1, 'table': 'test', 'result': 'deleted'}
```

<!-- intro -->

##### Python-asyncio:

<!-- request Python-asyncio -->
``` python
indexApi.delete({"cluster":"cluster","table" : "test", "id" : 1})
```

<!-- response Python-asyncio -->
```python
{'deleted': None, 'id': 1, 'table': 'test', 'result': 'deleted'}
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
indexApi.delete({"cluster":"cluster_1","table" : "test", "id" : 1})
```

<!-- response javascript -->
```javascript
{"table":"test","_id":1,"result":"deleted"}
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest();
deleteRequest.cluster("cluster").index("test").setId(1L);
indexApi.delete(deleteRequest);

```

<!-- response Java -->
```java
class DeleteResponse {
    index: test
    _id: 1
    result: deleted
}
```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest(index: "test", cluster: "cluster", id: 1);
indexApi.Delete(deleteRequest);

```

<!-- response C# -->
```clike
class DeleteResponse {
    index: test
    _id: 1
    result: deleted
}
```

<!-- intro -->

##### Rust:

<!-- request Rust -->
``` rust
let delete_req = DeleteDocumentRequest {
    table: "test".to_string(),
    cluster: "cluster".to_string(),
    id: serde_json::json!(1),
    ..Default::default(),
};

index_api.delete(delete_req).await;

```

<!-- response Rust -->
```rust
class DeleteResponse {
    index: test
    _id: 1
    result: deleted
}
```

<!-- intro -->

##### TypeScript:

<!-- request TypeScript -->
``` typescript
res = await indexApi.delete({ cluster: 'cluster_1', index: 'test', id: 1 });
```

<!-- response TypeScript -->
```json
{"table":"test","_id":1,"result":"deleted"}
```

<!-- intro -->

##### Go:

<!-- request Go -->
```go
deleteRequest := manticoresearch.NewDeleteDocumentRequest("test")
deleteRequest.SetCluster("cluster_1")
deleteRequest.SetId(1)
```

<!-- response Go -->
```json
{"table":"test","_id":1,"result":"deleted"}
```

<!-- end -->


## Bulk deletion

<!-- example bulk delete -->

आप एक ही कॉल में `/bulk` एंडपॉइंट का उपयोग करके एक से अधिक हटाने की क्रियाएँ भी कर सकते हैं। यह एंडपॉइंट केवल उस डेटा के साथ काम करता है जिसमें `Content-Type` `application/x-ndjson` सेट किया गया है। डेटा को न्यू लाइन-डिलीमिटेड JSON (NDJSON) के रूप में स्वरूपित किया जाना चाहिए। मूल रूप से, इसका अर्थ है कि प्रत्येक पंक्ति को बिल्कुल एक JSON बयान होना चाहिए और एक न्यूलाइन `
` के साथ समाप्त होना चाहिए और, संभवतः, एक ``।


<!-- intro -->
##### JSON:

<!-- request JSON --Content-type=application/x-ndjson -->

```json
POST /bulk

{ "delete" : { "table" : "test", "id" : 1 } }
{ "delete" : { "table" : "test", "query": { "equals": { "int_data" : 20 } } } }
```

<!-- response JSON -->

```json
{
   "items":
   [
      {
         "bulk":
         {
            "table":"test",
            "_id":0,
            "created":0,
            "deleted":2,
            "updated":0,
            "result":"created",
            "status":201
         }
      }
   ],
   "errors":false
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php

$client->bulk([
    ['delete' => [
            'table' => 'test',
            'id' => 1
        ]
    ],
    ['delete'=>[
            'table' => 'test',
            'query' => [
                'equals' => ['int_data' => 20]
            ]
        ]
    ]
]);
```

<!-- response PHP -->

```php
Array(
    [items] => Array
        (
            [0] => Array
                (
                    [bulk] => Array
                        (
                            [_index] => test
                            [_id] => 0
                            [created] => 0
                            [deleted] => 2
                            [updated] => 0
                            [result] => created
                            [status] => 201
                        )

                )

        )

    [current_line] => 3
    [skipped_lines] => 0
    [errors] =>
    [error] =>
)

```


<!-- intro -->
##### Python:

<!-- request Python -->
``` python
docs = [             { "delete" : { "table" : "test", "id": 1 } },             { "delete" : { "table" : "test", "query": { "equals": { "int_data": 20 } } } } ]
indexApi.bulk('
'.join(map(json.dumps,docs)))
```

<!-- response Python -->
```python
{
    'error': None,
    'items': [{u'delete': {u'table': test', u'deleted': 2}}]
}

```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->
``` python
docs = [             { "delete" : { "table" : "test", "id": 1 } },             { "delete" : { "table" : "test", "query": { "equals": { "int_data": 20 } } } } ]
indexApi.bulk('
'.join(map(json.dumps,docs)))
```

<!-- response Python-asyncio -->
```python
{
    'error': None,
    'items': [{u'delete': {u'table': test', u'deleted': 2}}]
}

```

<!-- intro -->
##### Javascript:

<!-- request javascript -->
``` javascript
docs = [
            { "delete" : { "table" : "test", "id": 1 } },
            { "delete" : { "table" : "test", "query": { "equals": { "int_data": 20 } } } } ];
res =  await indexApi.bulk(docs.map(e=>JSON.stringify(e)).join('
'));
```

<!-- response javascript -->
```javascript
{"items":[{"delete":{"table":"test","deleted":2}}],"errors":false}

```

<!-- intro -->
##### java:

<!-- request Java -->
``` java
String   body = "{ "delete" : { "table" : "test", "id": 1 } } "+"
"+
    "{ "delete" : { "table" : "test", "query": { "equals": { "int_data": 20 } } } }"+"
";         
indexApi.bulk(body);
```

<!-- response Java -->
```java
class BulkResponse {
    items: [{delete={_index=test, _id=0, created=false, deleted=2, result=created, status=200}}]
    error: null
    additionalProperties: {errors=false}
}
```

<!-- intro -->
##### C#:

<!-- request C# -->
``` clike
string   body = "{ "delete" : { "table" : "test", "id": 1 } } "+"
"+
    "{ "delete" : { "table" : "test", "query": { "equals": { "int_data": 20 } } } }"+"
";         
indexApi.Bulk(body);
```

<!-- response C# -->
```clike
class BulkResponse {
    items: [{replace={_index=test, _id=0, created=false, deleted=2, result=created, status=200}}]
    error: null
    additionalProperties: {errors=false}
}
```

<!-- intro -->
##### Rust:

<!-- request Rust -->
``` rust
let bulk_body = r#"{ "delete" : { "table" : "test", "id": 1 } }
    { "delete" : { "table" : "test", "query": { "equals": { "int_data": 20 } } } }"         
"#;
index_api.bulk(bulk_body).await;
```

<!-- response Rust -->
```rust
class BulkResponse {
    items: [{replace={_index=test, _id=0, created=false, deleted=2, result=created, status=200}}]
    error: null
    additionalProperties: {errors=false}
}
```


<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->
``` typescript
docs = [
  { "delete" : { "table" : "test", "id": 1 } },
  { "delete" : { "table" : "test", "query": { "equals": { "int_data": 20 } } } }
];
body = await indexApi.bulk(
  docs.map((e) => JSON.stringify(e)).join("
")
);            
res = await indexApi.bulk(body);
```

<!-- response TypeScript -->
```json
{"items":[{"delete":{"table":"test","deleted":2}}],"errors":false}

```

<!-- intro -->

##### Go:

<!-- request Go -->
```go
docs = []string {
  `{ "delete" : { "table" : "test", "id": 1 } }`,
  `{ "delete" : { "table" : "test", "query": { "equals": { "int_data": 20 } } } }`
]
body = strings.Join(docs, "
")
resp, httpRes, err := manticoreclient.IndexAPI.Bulk(context.Background()).Body(body).Execute()
```

<!-- response Go -->
```json
{"items":[{"delete":{"table":"test","deleted":2}}],"errors":false}
```

<!-- end -->


<!-- end -->
<!-- proofread -->


# दस्तावेज़ हटाना

दस्तावेज़ हटाना केवल [RT मोड](../../Read_this_first.md#Real-time-mode-vs-plain-mode) में निम्नलिखित तालिका प्रकारों के लिए समर्थित है:
* [वास्तविक समय](../Creating_a_table/Local_tables/Real-time_table.md) तालिकाएं
* [पर्कोलेट](../Creating_a_table/Local_tables/Percolate_table.md) तालिकाएं
* वितरित तालिकाएं जो केवल RT तालिकाओं को स्थानीय या दूरस्थ एजेंट के रूप में शामिल करती हैं.

आप किसी तालिका से मौजूदा दस्तावेज़ों को उनके आईडी या कुछ शर्तों के आधार पर हटा सकते हैं.

इसके अलावा, [बुल्क डिलीशन](../Data_creation_and_modification/Deleting_documents.md#Bulk-deletion) उपलब्ध है जिससे कई दस्तावेज़ों को हटाया जा सकता है.

दस्तावेज़ों का हटाना SQL और JSON दोनों इंटरफेस के माध्यम से पूरा किया जा सकता है.

SQL के लिए, सफल ऑपरेशन के लिए प्रतिक्रिया उस संख्या को सूचित करेगी जो हटाई गई पंक्तियों की होगी.

JSON के लिए, `json/delete` एंडपॉइंट का उपयोग किया जाता है। सर्वर एक JSON ऑब्जेक्ट के साथ प्रतिक्रिया देगा जो यह बताएगा कि ऑपरेशन सफल था या नहीं और हटाई गई पंक्तियों की संख्या.

किसी तालिका से सभी दस्तावेज़ों को हटाने के लिए [तालिका की ट्रंकिंग](../Emptying_a_table.md) का उपयोग करने की सिफारिश की जाती है, क्योंकि यह एक बहुत तेज़ ऑपरेशन है.


<!-- example delete 2 -->
इस उदाहरण में हम 'test' नाम की तालिका से पूर्ण-पाठ क्वेरी `test document` से मेल खाने वाले सभी दस्तावेज़ों को हटाते हैं:


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
mysql> SELECT * FROM TEST;
+------+------+-------------+------+
| id   | gid  | mva1        | mva2 |
+------+------+-------------+------+
|  100 | 1000 | 100,201     | 100  |
|  101 | 1001 | 101,202     | 101  |
|  102 | 1002 | 102,203     | 102  |
|  103 | 1003 | 103,204     | 103  |
|  104 | 1004 | 104,204,205 | 104  |
|  105 | 1005 | 105,206     | 105  |
|  106 | 1006 | 106,207     | 106  |
|  107 | 1007 | 107,208     | 107  |
+------+------+-------------+------+
8 rows in set (0.00 sec)

mysql> DELETE FROM TEST WHERE MATCH ('test document');
Query OK, 2 rows affected (0.00 sec)

mysql> SELECT * FROM TEST;
+------+------+-------------+------+
| id   | gid  | mva1        | mva2 |
+------+------+-------------+------+
|  100 | 1000 | 100,201     | 100  |
|  101 | 1001 | 101,202     | 101  |
|  102 | 1002 | 102,203     | 102  |
|  103 | 1003 | 103,204     | 103  |
|  104 | 1004 | 104,204,205 | 104  |
|  105 | 1005 | 105,206     | 105  |
+------+------+-------------+------+
6 rows in set (0.00 sec)
```

<!-- request JSON -->

``` json
POST /delete -d '
    {
        "table":"test",
        "query":
        {
            "match": { "*": "test document" }
        }
    }'
```

* JSON के लिए `query` एक पूर्ण-पाठ खोज के लिए एक क्लॉज़ शामिल करता है; इसकी वही सिंटैक्स है जो [JSON/update](../Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-HTTP-JSON) में है.

<!-- response JSON -->

``` json
{
    "table":"test",
    "deleted":2,
}
```    
<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->deleteDocuments(new MatchPhrase('test document','*'));
```

<!-- response json -->

``` php
Array(
    [_index] => test
    [deleted] => 2
)
```
<!-- intro -->

##### Python:

<!-- request Python -->
``` python
indexApi.delete({"table" : "test", "query": { "match": { "*": "test document" }}})
```

<!-- response Python -->
```python
{'deleted': 5, 'id': None, 'table': 'test', 'result': None}
```
<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.delete({"table" : "test", "query": { "match": { "*": "test document" }}});
```

<!-- response javascript -->
```javascript
{"table":"test","deleted":5}
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest();
query = new HashMap<String,Object>();
query.put("match",new HashMap<String,Object>(){{
    put("*","test document");
}});
deleteRequest.index("test").setQuery(query);
indexApi.delete(deleteRequest);

```

<!-- response Java -->
```java
class DeleteResponse {
    index: test
    deleted: 5
    id: null
    result: null
}
```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
Dictionary<string, Object> match = new Dictionary<string, Object>();
match.Add("*", "test document");
Dictionary<string, Object> query = new Dictionary<string, Object>();
query.Add("match", match);
DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest(index: "test", query: query);
indexApi.Delete(deleteRequest);

```

<!-- response C# -->
```clike
class DeleteResponse {
    index: test
    deleted: 5
    id: null
    result: null
}
```

<!-- intro -->

##### TypeScript:

<!-- request TypeScript -->
``` typescript
res = await indexApi.delete({
  index: 'test',
  query: { match: { '*': 'test document' } },
});
```

<!-- response TypeScript -->
```json
{"table":"test","deleted":5}
```

<!-- intro -->

##### Go:

<!-- request Go -->
```go
deleteRequest := manticoresearch.NewDeleteDocumentRequest("test")
matchExpr := map[string]interface{} {"*": "test document"}
deleteQuery := map[string]interface{} {"match": matchExpr }
deleteRequest.SetQuery(deleteQuery)
```

<!-- response Go -->
```json
{"table":"test","deleted":5}
```

<!-- end -->

<!-- example delete 3 -->
यहाँ - 'test' नाम की तालिका से 'id' के समान 1 वाले दस्तावेज़ को हटा रहे हैं:


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
mysql> DELETE FROM TEST WHERE id=1;
Query OK, 1 rows affected (0.00 sec)
```

<!-- request JSON -->

``` json
POST /delete -d '
    {
        "table": "test",
        "id": 1
    }'
```
* `id` के लिए JSON में वह पंक्ति `id` है जिसे हटाया जाना चाहिए।

<!-- response JSON -->

``` json
{
    "table": "test",
    "_id": 1,
    "found": true,
    "result": "deleted"      
}
```    
<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->deleteDocument(1);
```

<!-- response json -->

``` php
Array(
    [_index] => test
    [_id] => 1
    [found] => true
    [result] => deleted
)
```
<!-- intro -->

##### Python:

<!-- request Python -->
``` python
indexApi.delete({"table" : "test", "id" : 1})
```

<!-- response Python -->
```python
{'deleted': None, 'id': 1, 'table': 'test', 'result': 'deleted'}
```
<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.delete({"table" : "test", "id" : 1});
```

<!-- response javascript -->
```javascript
{"table":"test","_id":1,"result":"deleted"}
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest();
deleteRequest.index("test").setId(1L);
indexApi.delete(deleteRequest);

```

<!-- response Java -->
```java
class DeleteResponse {
    index: test
    _id: 1
    result: deleted
}
```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest(index: "test", id: 1);
indexApi.Delete(deleteRequest);

```

<!-- response C# -->
```java
class DeleteResponse {
    index: test
    _id: 1
    result: deleted
}
```

<!-- intro -->

##### TypeScript:

<!-- request TypeScript -->
``` typescript
res = await indexApi.delete({ index: 'test', id: 1 });
```

<!-- response TypeScript -->
```json
{"table":"test","_id":1,"result":"deleted"}
```

<!-- intro -->

##### Go:

<!-- request Go -->
```go
deleteRequest := manticoresearch.NewDeleteDocumentRequest("test")
deleteRequest.SetId(1)
```

<!-- response Go -->
```json
{"table":"test","_id":1,"result":"deleted"}
```

<!-- end -->

<!-- example delete 4 -->
यहाँ, `id` जो मान `test` नामक तालिका से मेल खाते हैं, उन दस्तावेजों को हटा दिया जाता है:

ध्यान दें कि `id=N` या `id IN (X,Y)` के साथ delete फॉर्म सबसे तेज हैं, क्योंकि वे खोज किए बिना दस्तावेजों को हटा देते हैं।
यह भी ध्यान दें कि प्रतिक्रिया में केवल पहले हटाए गए दस्तावेज की आईडी उस संबंधित `_id` फील्ड में होती है।

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
DELETE FROM TEST WHERE id IN (1,2);

```

<!-- response SQL -->

```sql
Query OK, 2 rows affected (0.00 sec)
```

<!-- request JSON -->

``` json
POST /delete -d '
    {
        "table":"test",
        "id": [1,2]
    }'
```

<!-- response JSON -->

``` json
    {
        "table":"test",
        "_id":1,
        "found":true,
        "result":"deleted"      
    }
```    

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->deleteDocumentsByIds([1,2]);
```

<!-- response json -->

``` php
Array(
    [_index] => test
    [_id] => 1
    [found] => true
    [result] => deleted
)
```

<!-- end -->

<!-- example delete 5 -->
Manticore SQL `DELETE` कथन के लिए जटिल शर्तों का उपयोग करने की अनुमति देता है।

उदाहरण के लिए यहाँ हम उन दस्तावेजों को हटा रहे हैं जो पूर्ण-पाठ क्वेरी `test document` से मेल खाते हैं और जिनमें विशेषता `mva1` 206 से अधिक है या `mva1` के मान 100 या 103 हैं `test` नामक तालिका से:


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
DELETE FROM TEST WHERE MATCH ('test document') AND ( mva1>206 or mva1 in (100, 103) );

SELECT * FROM TEST;
```

<!-- response SQL -->

```sql
Query OK, 4 rows affected (0.00 sec)

+------+------+-------------+------+
| id   | gid  | mva1        | mva2 |
+------+------+-------------+------+
|  101 | 1001 | 101,202     | 101  |
|  102 | 1002 | 102,203     | 102  |
|  104 | 1004 | 104,204,205 | 104  |
|  105 | 1005 | 105,206     | 105  |
+------+------+-------------+------+
6 rows in set (0.00 sec)
```
<!-- end -->

<!-- example delete 6 -->
यहाँ `cluster` क्लस्टर में `test` तालिका के दस्तावेजों को हटाने का एक उदाहरण है। ध्यान दें कि हमें पुनरावृत्ति क्लस्टर में एक तालिका से पंक्ति हटाने के लिए तालिका गुण के साथ-साथ क्लस्टर नाम गुण भी प्रदान करना होगा:


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
delete from cluster:test where id=100;
```

<!-- request JSON -->
``` json
POST /delete -d '
    {
      "cluster": "cluster",
      "table": "test",
      "id": 100
    }'
```
* JSON के लिए `cluster` [पुनरावृत्ति क्लस्टर](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) का नाम है जिसमें आवश्यक तालिका है

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->setCluster('cluster');
$index->deleteDocument(100);
```

<!-- response JSON -->

``` php
Array(
    [_index] => test
    [_id] => 100
    [found] => true
    [result] => deleted
)
```
<!-- intro -->

##### Python:

<!-- request Python -->
``` python
indexApi.delete({"cluster":"cluster","table" : "test", "id" : 1})
```

<!-- response Python -->
```python
{'deleted': None, 'id': 1, 'table': 'test', 'result': 'deleted'}
```
<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
indexApi.delete({"cluster":"cluster_1","table" : "test", "id" : 1})
```

<!-- response javascript -->
```javascript
{"table":"test","_id":1,"result":"deleted"}
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest();
deleteRequest.cluster("cluster").index("test").setId(1L);
indexApi.delete(deleteRequest);

```

<!-- response Java -->
```java
class DeleteResponse {
    index: test
    _id: 1
    result: deleted
}
```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest(index: "test", cluster: "cluster", id: 1);
indexApi.Delete(deleteRequest);

```

<!-- response C# -->
```clike
class DeleteResponse {
    index: test
    _id: 1
    result: deleted
}
```

<!-- intro -->

##### TypeScript:

<!-- request TypeScript -->
``` typescript
res = await indexApi.delete({ cluster: 'cluster_1', index: 'test', id: 1 });
```

<!-- response TypeScript -->
```json
{"table":"test","_id":1,"result":"deleted"}
```

<!-- intro -->

##### Go:

<!-- request Go -->
```go
deleteRequest := manticoresearch.NewDeleteDocumentRequest("test")
deleteRequest.SetCluster("cluster_1")
deleteRequest.SetId(1)
```

<!-- response Go -->
```json
{"table":"test","_id":1,"result":"deleted"}
```

<!-- end -->


## Bulk deletion

<!-- example bulk delete -->

You can also perform multiple delete operations in a single call using the `/bulk` endpoint. This endpoint only works with data that has `Content-Type` set to `application/x-ndjson`. The data should be formatted as newline-delimited JSON (NDJSON). Essentially, this means that each line should contain exactly one JSON statement and end with a newline `\n` and, possibly, a `\r`.


<!-- intro -->
##### JSON:

<!-- request JSON --Content-type=application/x-ndjson -->

```json
POST /bulk

{ "delete" : { "table" : "test", "id" : 1 } }
{ "delete" : { "table" : "test", "query": { "equals": { "int_data" : 20 } } } }
```

<!-- response JSON -->

```json
{
   "items":
   [
      {
         "bulk":
         {
            "table":"test",
            "_id":0,
            "created":0,
            "deleted":2,
            "updated":0,
            "result":"created",
            "status":201
         }
      }
   ],
   "errors":false
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php

$client->bulk([
    ['delete' => [
            'table' => 'test',
            'id' => 1
        ]
    ],
    ['delete'=>[
            'table' => 'test',
            'query' => [
                'equals' => ['int_data' => 20]
            ]
        ]
    ]
]);
```

<!-- response PHP -->

```php
Array(
    [items] => Array
        (
            [0] => Array
                (
                    [bulk] => Array
                        (
                            [_index] => test
                            [_id] => 0
                            [created] => 0
                            [deleted] => 2
                            [updated] => 0
                            [result] => created
                            [status] => 201
                        )

                )

        )

    [current_line] => 3
    [skipped_lines] => 0
    [errors] =>
    [error] =>
)

```


<!-- intro -->
##### Python:

<!-- request Python -->
``` python
docs = [ \
            { "delete" : { "table" : "test", "id": 1 } }, \
            { "delete" : { "table" : "test", "query": { "equals": { "int_data": 20 } } } } ]
indexApi.bulk('\n'.join(map(json.dumps,docs)))
```

<!-- response Python -->
```python
{
    'error': None,
    'items': [{u'delete': {u'table': test', u'deleted': 2}}]
}

```
<!-- intro -->
##### Javascript:

<!-- request javascript -->
``` javascript
docs = [
            { "delete" : { "table" : "test", "id": 1 } },
            { "delete" : { "table" : "test", "query": { "equals": { "int_data": 20 } } } } ];
res =  await indexApi.bulk(docs.map(e=>JSON.stringify(e)).join('\n'));
```

<!-- response javascript -->
```javascript
{"items":[{"delete":{"table":"test","deleted":2}}],"errors":false}

```

<!-- intro -->
##### java:

<!-- request Java -->
``` java
String   body = "{ "delete" : { "table" : "test", "id": 1 } } "+"\n"+
    "{ "delete" : { "table" : "test", "query": { "equals": { "int_data": 20 } } } }"+"\n";         
indexApi.bulk(body);
```

<!-- response Java -->
```java
class BulkResponse {
    items: [{delete={_index=test, _id=0, created=false, deleted=2, result=created, status=200}}]
    error: null
    additionalProperties: {errors=false}
}
```

<!-- intro -->
##### C#:

<!-- request C# -->
``` clike
string   body = "{ "delete" : { "table" : "test", "id": 1 } } "+"\n"+
    "{ "delete" : { "table" : "test", "query": { "equals": { "int_data": 20 } } } }"+"\n";         
indexApi.Bulk(body);
```

<!-- response C# -->
```clike
class BulkResponse {
    items: [{replace={_index=test, _id=0, created=false, deleted=2, result=created, status=200}}]
    error: null
    additionalProperties: {errors=false}
}
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->
``` typescript
docs = [
  { "delete" : { "table" : "test", "id": 1 } },
  { "delete" : { "table" : "test", "query": { "equals": { "int_data": 20 } } } }
];
body = await indexApi.bulk(
  docs.map((e) => JSON.stringify(e)).join("\n")
);            
res = await indexApi.bulk(body);
```

<!-- response TypeScript -->
```json
{"items":[{"delete":{"table":"test","deleted":2}}],"errors":false}

```

<!-- intro -->

##### Go:

<!-- request Go -->
```go
docs = []string {
  `{ "delete" : { "table" : "test", "id": 1 } }`,
  `{ "delete" : { "table" : "test", "query": { "equals": { "int_data": 20 } } } }`
]
body = strings.Join(docs, "\n")
resp, httpRes, err := manticoreclient.IndexAPI.Bulk(context.Background()).Body(body).Execute()
```

<!-- response Go -->
```json
{"items":[{"delete":{"table":"test","deleted":2}}],"errors":false}
```

<!-- end -->


<!-- end -->
<!-- proofread -->

