# बदलें

<!-- उदाहरण बदलें -->

`बदलें` [INSERT](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real_time_table.md) की तरह काम करता है, लेकिन यह एक नए दस्तावेज़ को जोड़ने से पहले उसी आईडी के साथ पिछले दस्तावेज़ को हटाए गए के रूप में चिह्नित करता है।

यदि आप स्थान पर अपडेट की तलाश कर रहे हैं, तो कृपया [इस सेक्शन](../../Data_creation_and_modification/Updating_documents/UPDATE.md) को देखें।

## SQL बदलें

SQL `बदलें` कथन की व्याकरण इस प्रकार है:

**पूरे दस्तावेज़ को बदलने के लिए:**
```sql
REPLACE INTO table [(column1, column2, ...)]
    VALUES (value1, value2, ...)
    [, (...)]
```

**केवल चयनित फ़ील्ड्स को बदलने के लिए:**
```sql
REPLACE INTO table
    SET field1=value1[, ..., fieldN=valueN]
    WHERE id = <id>
```
नोट करें, इस मोड में आप केवल आईडी द्वारा फ़िल्टर कर सकते हैं।

> नोट: आंशिक बदलने के लिए [Manticore Buddy](../Installation/Manticore_Buddy.md) की आवश्यकता होती है। यदि यह काम नहीं करता है, तो सुनिश्चित करें कि बडी स्थापित है।

`UPDATE` बनाम आंशिक `बदलें` के बारे में अधिक पढ़ें [यहां](../../Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md#UPDATE-vs-partial-REPLACE)।

अधिक विवरण के लिए उदाहरण देखें।

## JSON बदलें

* `/replace`:
  ```
  POST /replace
  {
    "table": "<table name>",
    "id": <document id>,
    "doc":
    {
      "<field1>": <value1>,
      ...
      "<fieldN>": <valueN>
    }
  }
  ```
  `/index` एक उपनाम अंतिम बिंदु है और समान तरीके से काम करता है।
* Elasticsearch-जैसा अंतिम बिंदु `<table>/_doc/<id>`:
  ```
  PUT/POST /<table name>/_doc/<id>
  {
    "<field1>": <value1>,
    ...
    "<fieldN>": <valueN>
  }
  ```
  > नोट: Elasticsearch-जैसे बदलने के लिए [Manticore Buddy](../Installation/Manticore_Buddy.md) की आवश्यकता होती है। यदि यह काम नहीं करता है, तो सुनिश्चित करें कि बडी स्थापित है।
* आंशिक बदलें:
  ```
  POST /<{table | cluster:table}>/_update/<id>
  {
    "<field1>": <value1>,
    ...
    "<fieldN>": <valueN>
  }
  ```
  `<table name>` केवल तालिका नाम हो सकता है या `cluster:table` प्रारूप में हो सकता है। यह आवश्यकतानुसार किसी विशिष्ट क्लस्टर के पार अपडेट की अनुमति देता है।

  > नोट: आंशिक बदलने के लिए [Manticore Buddy](../Installation/Manticore_Buddy.md) की आवश्यकता होती है। यदि यह काम नहीं करता है, तो सुनिश्चित करें कि बडी स्थापित है।

अधिक विवरण के लिए उदाहरण देखें।

<!-- परिचय -->
##### SQL:
<!-- अनुरोध SQL -->

```sql
REPLACE INTO products VALUES(1, "document one", 10);
```

<!-- प्रतिक्रिया SQL -->

```sql
Query OK, 1 row affected (0.00 sec)
```

<!-- परिचय -->
##### बदलें ... सेट करें:
<!-- अनुरोध बदलें SET -->

```sql
REPLACE INTO products SET description='HUAWEI Matebook 15', price=10 WHERE id = 55;
```

<!-- प्रतिक्रिया बदलें ... सेट करें -->

```sql
Query OK, 1 row affected (0.00 sec)
```

<!-- परिचय -->
##### JSON

<!-- अनुरोध JSON -->

```json
POST /replace
-H "Content-Type: application/x-ndjson" -d '
{
  "table":"products",
  "id":1,
  "doc":
  {
    "title":"product one",
    "price":10
  }
}
'

```

<!-- प्रतिक्रिया JSON -->
```json
{
  "table":"products",
  "_id":1,
  "created":false,
  "result":"updated",
  "status":200
}

```

<!-- परिचय -->
##### Elasticsearch-जैसा

<!-- अनुरोध Elasticsearch-जैसा -->

> नोट: Elasticsearch-जैसे बदलने के लिए [Manticore Buddy](../Installation/Manticore_Buddy.md) की आवश्यकता होती है। यदि यह काम नहीं करता है, तो सुनिश्चित करें कि बडी स्थापित है।

```json
PUT /products/_doc/2
{
  "title": "product two",
  "price": 20
}

POST /products/_doc/3
{
  "title": "product three",
  "price": 10
}
```

<!-- प्रतिक्रिया Elasticsearch-जैसा -->
```json
{
"_id":2,
"table":"products",
"_primary_term":1,
"_seq_no":0,
"_shards":{
    "failed":0,
    "successful":1,
    "total":1
},
"_type":"_doc",
"_version":1,
"result":"updated"
}

{
"_id":3,
"table":"products",
"_primary_term":1,
"_seq_no":0,
"_shards":{
    "failed":0,
    "successful":1,
    "total":1
},
"_type":"_doc",
"_version":1,
"result":"updated"
}
```

<!-- परिचय -->
##### Elasticsearch-जैसा आंशिक बदलें:

<!-- अनुरोध Elasticsearch-जैसा आंशिक -->

> नोट: आंशिक बदलने के लिए [Manticore Buddy](../Installation/Manticore_Buddy.md) की आवश्यकता होती है। यदि यह काम नहीं करता है, तो सुनिश्चित करें कि बडी स्थापित है।

```json
POST /products/_update/55
{
  "doc": {
    "description": "HUAWEI Matebook 15",
    "price": 10
  }
}
```

<!-- प्रतिक्रिया Elasticsearch-जैसा आंशिक -->
```json
{
"table":"products",
"updated":1
}
```

<!-- परिचय -->
##### Elasticsearch-जैसा आंशिक बदलना क्लस्टर में:

<!-- अनुरोध Elasticsearch-जैसा आंशिक क्लस्टर में -->

```json
POST /cluster_name:products/_update/55
{
  "doc": {
    "description": "HUAWEI Matebook 15",
    "price": 10
  }
}
```

<!-- प्रतिक्रिया Elasticsearch-जैसा आंशिक क्लस्टर में -->
```json
{
"table":"products",
"updated":1
}
```

<!-- परिचय -->
##### PHP:

<!-- अनुरोध PHP -->

```php
$index->replaceDocument([
   'title' => 'document one',
    'price' => 10
],1);
```

<!-- प्रतिक्रिया PHP -->
```php
Array(
    [_index] => products
    [_id] => 1
    [created] => false
    [result] => updated
    [status] => 200
)
```
<!-- परिचय -->

##### Python:

<!-- अनुरोध Python -->
``` python
indexApi.replace({"table" : "products", "id" : 1, "doc" : {"title" : "document one","price":10}})
```

<!-- प्रतिक्रिया Python -->
```python
{'created': False,
 'found': None,
 'id': 1,
 'table': 'products',
 'result': 'updated'}
```

<!-- परिचय -->

##### Python-asyncio:

<!-- अनुरोध Python-asyncio -->
``` python
await indexApi.replace({"table" : "products", "id" : 1, "doc" : {"title" : "document one","price":10}})
```

<!-- response Python-asyncio -->
```python
{'created': False,
 'found': None,
 'id': 1,
 'table': 'products',
 'result': 'updated'}
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.replace({"table" : "products", "id" : 1, "doc" : {"title" : "document one","price":10}});
```

<!-- response javascript -->
```javascript
{"table":"products","_id":1,"result":"updated"}
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
docRequest = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
            put("title","document one");
            put("price",10);
}};
docRequest.index("products").id(1L).setDoc(doc);
sqlresult = indexApi.replace(docRequest);
```

<!-- response Java -->
```java
class SuccessResponse {
    index: products
    id: 1
    created: false
    result: updated
    found: null
}

```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "document one");
doc.Add("price", 10);
InsertDocumentRequest docRequest = new InsertDocumentRequest(index: "products", id: 1, doc: doc);
var sqlresult = indexApi.replace(docRequest);
```

<!-- response C# -->
```clike
class SuccessResponse {
    index: products
    id: 1
    created: false
    result: updated
    found: null
}

```

<!-- intro -->

##### Rust:

<!-- request Rust -->
``` rust
let mut doc = HashMap::new();
doc.insert("title".to_string(), serde_json::json!("document one"));
doc.insert("price".to_string(), serde_json::json!(10));
let insert_req = InsertDocumentRequest::new("products".to_string(), serde_json::json!(doc));
let insert_res = index_api.replace(insert_req).await;
```

<!-- response Rust -->
```rust
class SuccessResponse {
    index: products
    id: 1
    created: false
    result: updated
    found: null
}

```

<!-- intro -->

##### TypeScript:

<!-- request TypeScript -->
``` typescript
res = await indexApi.replace({
  index: 'test',
  id: 1,
  doc: { content: 'Text 11', name: 'Doc 11', cat: 3 },
});
```

<!-- response TypeScript -->
```json
{
    "table":"test",
    "_id":1,
    "created":false
    "result":"updated"
    "status":200
}
```

<!-- intro -->

##### Go:

<!-- request Go -->
``` go
replaceDoc := map[string]interface{} {"content": "Text 11", "name": "Doc 11", "cat": 3}
replaceRequest := manticoreclient.NewInsertDocumentRequest("test", replaceDoc)
replaceRequest.SetId(1)
res, _, _ := apiClient.IndexAPI.Replace(context.Background()).InsertDocumentRequest(*replaceRequest).Execute()
```

<!-- response Go -->
```go
{
    "table":"test",
    "_id":1,
    "created":false
    "result":"updated"
    "status":200
}
```

<!-- end -->

`REPLACE` वास्तविक समय और पर्कोलेट तालिकाओं के लिए उपलब्ध है। आप साधारण तालिका में डेटा नहीं बदल सकते हैं।

जब आप `REPLACE` चलाते हैं, तो पिछले दस्तावेज़ को हटाया नहीं जाता है, लेकिन इसे हटाए गए के रूप में चिह्नित किया जाता है, इसलिए तालिका का आकार बढ़ता है जब तक कि खंड मर्ज नहीं होता। खंड मर्ज को मजबूर करने के लिए, [OPTIMIZE कथन](../../Securing_and_compacting_a_table/Compacting_a_table.md) का उपयोग करें।

##.bulk.replace

<!-- example bulk_replace -->

आप एक साथ कई दस्तावेजों को बदल सकते हैं। अधिक जानकारी के लिए [बिकुल डॉक्यूमेंट जोड़ना](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-adding-documents) की जांच करें।

<!-- intro -->
##### HTTP:

<!-- request SQL -->

```sql
REPLACE INTO products(id,title,tag) VALUES (1, 'doc one', 10), (2,' doc two', 20);
```

<!-- response SQL -->

```sql
Query OK, 2 rows affected (0.00 sec)
```

<!-- request JSON -->

```json
POST /bulk
-H "Content-Type: application/x-ndjson" -d '
{ "replace" : { "table" : "products", "id":1, "doc": { "title": "doc one", "tag" : 10 } } }
{ "replace" : { "table" : "products", "id":2, "doc": { "title": "doc two", "tag" : 20 } } }
'
```

<!-- response JSON -->

```json
{
  "items":
  [
    {
      "replace":
      {
        "table":"products",
        "_id":1,
        "created":false,
        "result":"updated",
        "status":200
      }
    },
    {
      "replace":
      {
        "table":"products",
        "_id":2,
        "created":false,
        "result":"updated",
        "status":200
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
$index->replaceDocuments([
    [
        'id' => 1,
        'title' => 'document one',
        'tag' => 10
    ],
    [
        'id' => 2,
        'title' => 'document one',
        'tag' => 20
    ]
);
```

<!-- response PHP -->
```php
Array(
    [items] =>
    Array(
        Array(
            [_index] => products
            [_id] => 2
            [created] => false
            [result] => updated
            [status] => 200
        )
        Array(
            [_index] => products
            [_id] => 2
            [created] => false
            [result] => updated
            [status] => 200
        )
    )
    [errors => false
)
```
<!-- request Python -->

``` python
indexApi = manticoresearch.IndexApi(client)
docs = [     {"replace": {"table" : "products", "id" : 1, "doc" : {"title" : "document one"}}},     {"replace": {"table" : "products", "id" : 2, "doc" : {"title" : " दस्तावेज दो"}}} ]
api_resp = indexApi.bulk('
'.join(map(json.dumps,docs)))
```

<!-- response Python -->
```python
{'error': None,
 'items': [{u'replace': {u'_id': 1,
                         u'table': u'products',
                         u'created': False,
                         u'result': u'updated',
                         u'status': 200}},
           {u'replace': {u'_id': 2,
                         u'table': u'products',
                         u'created': False,
                         u'result': u'updated',
                         u'status': 200}}]}

```

<!-- request Python-asyncio -->

``` python
indexApi = manticoresearch.IndexApi(client)
docs = [     {"replace": {"table" : "products", "id" : 1, "doc" : {"title" : " दस्तावेज एक"}}},     {"replace": {"table" : "products", "id" : 2, "doc" : {"title" : " दस्तावेज दो"}}} ]
api_resp = await indexApi.bulk('
'.join(map(json.dumps,docs)))
```

<!-- response Python-asyncio -->
```python
{'error': None,
 'items': [{u'replace': {u'_id': 1,
                         u'table': u'products',
                         u'created': False,
                         u'result': u'updated',
                         u'status': 200}},
           {u'replace': {u'_id': 2,
                         u'table': u'products',
                         u'created': False,
                         u'result': u'updated',
                         u'status': 200}}]}

```

<!-- request javascript -->

``` javascript
docs = [
    {"replace": {"table" : "products", "id" : 1, "doc" : {"title" : " दस्तावेज एक"}}},
    {"replace": {"table" : "products", "id" : 2, "doc" : {"title" : " दस्तावेज दो"}}} ];
res =  await indexApi.bulk(docs.map(e=>JSON.stringify(e)).join('
'));
```

<!-- response javascript -->
```javascript
{"items":[{"replace":{"table":"products","_id":1,"created":false,"result":"updated","status":200}},{"replace":{"table":"products","_id":2,"created":false,"result":"updated","status":200}}],"errors":false}

```

<!-- request Java -->

``` java
body = "{"replace": {"index" : "products", "id" : 1, "doc" : {"title" : " दस्तावेज एक"}}}" +"
"+
    "{"replace": {"index" : "products", "id" : 2, "doc" : {"title" : " दस्तावेज दो"}}}"+"
" ;
indexApi.bulk(body);
```

<!-- response Java -->
```java
class BulkResponse {
    items: [{replace={_index=products, _id=1, created=false, result=updated, status=200}}, {replace={_index=products, _id=2, created=false, result=updated, status=200}}]
    error: null
    additionalProperties: {errors=false}
}
```

<!-- request C# -->

``` clike
string body = "{"replace": {"index" : "products", "id" : 1, "doc" : {"title" : " दस्तावेज एक"}}}" +"
"+
    "{"replace": {"index" : "products", "id" : 2, "doc" : {"title" : " दस्तावेज दो"}}}"+"
" ;
indexApi.Bulk(body);
```

<!-- response C# -->
```clike
class BulkResponse {
    items: [{replace={_index=products, _id=1, created=false, result=updated, status=200}}, {replace={_index=products, _id=2, created=false, result=updated, status=200}}]
    error: null
    additionalProperties: {errors=false}
}
```

<!-- request Rust -->

``` rust
string body = r#"{"replace": {"index" : "products", "id" : 1, "doc" : {"title" : " दस्तावेज एक"}}}
    {"replace": {"index" : "products", "id" : 2, "doc" : {"title" : " दस्तावेज दो"}}}
"#;
index_api.bulk(body).await;
```

<!-- response Rust -->
```rust
class BulkResponse {
    items: [{replace={_index=products, _id=1, created=false, result=updated, status=200}}, {replace={_index=products, _id=2, created=false, result=updated, status=200}}]
    error: null
    additionalProperties: {errors=false}
}
```

<!-- request TypeScript -->

``` typescript
replaceDocs = [
  {
    replace: {
      index: 'test',
      id: 1,
      doc: { content: 'Text 11', cat: 1, name: 'Doc 11' },
    },
  },
  {
    replace: {
      index: 'test',
      id: 2,
      doc: { content: 'Text 22', cat: 9, name: 'Doc 22' },
    },
  },
];

res = await indexApi.bulk(
  replaceDocs.map((e) => JSON.stringify(e)).join("
")
);
```

<!-- response TypeScript -->
```typescript
{
  "items":
  [
    {
      "replace":
      {
        "table":"test",
        "_id":1,
        "created":false,
        "result":"updated",
        "status":200
      }
    },
    {
      "replace":
      {
        "table":"test",
        "_id":2,
        "created":false,
        "result":"updated",
        "status":200
      }
    }
  ],
  "errors":false
}
```

<!-- request Go -->

``` go
body := "{"replace": {"index": "test", "id": 1, "doc": {"content": "Text 11", "name": "Doc 11", "cat": 1 }}}" + "
" +

"{"replace": {"index": "test", "id": 2, "doc": {"content": "Text 22", "name": "Doc 22", "cat": 9 }}}" +"
";
res, _, _ := apiClient.IndexAPI.Bulk(context.Background()).Body(body).Execute()
```

<!-- response Go -->
```go
{
  "items":
  [
    {
      "replace":
      {
        "table":"test",
        "_id":1,
        "created":false,
        "result":"updated",
        "status":200
      }
    },
    {
      "replace":
      {
        "table":"test",
        "_id":2,
        "created":false,
        "result":"updated",
        "status":200
      }
    }
  ],
  "errors":false
}
```

<!-- end -->

# प्रतिस्थापित करें

<!-- उदाहरण प्रतिस्थापन -->

`REPLACE` [INSERT](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md) की तरह कार्य करता है, लेकिन यह नए एक को सम्मिलित करने से पहले उसी आईडी के पिछले दस्तावेज़ को हटाने के रूप में चिह्नित करता है।

यदि आप इन-प्लेस अपडेट की तलाश कर रहे हैं, तो कृपया [इस अनुभाग](../../Data_creation_and_modification/Updating_documents/UPDATE.md) को देखें।

## SQL REPLACE

SQL `REPLACE` कथन की ओकर структура निम्नलिखित है:

**पूरे दस्तावेज़ को प्रतिस्थापित करने के लिए:**
```sql
REPLACE INTO table [(column1, column2, ...)]
    VALUES (value1, value2, ...)
    [, (...)]
```

**केवल चयनित क्षेत्रों को प्रतिस्थापित करने के लिए:**
```sql
REPLACE INTO table
    SET field1=value1[, ..., fieldN=valueN]
    WHERE id = <id>
```
ध्यान दें, आप इस मोड में केवल id द्वारा फ़िल्टर कर सकते हैं।

> नोट: आंशिक प्रतिस्थापन के लिए [Manticore Buddy](../Installation/Manticore_Buddy.md) की आवश्यकता है। यदि यह काम नहीं करता है, तो सुनिश्चित करें कि Buddy स्थापित है।

`UPDATE` बनाम आंशिक `REPLACE` के बारे में अधिक जानें [यहां](../../Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md#UPDATE-vs-partial-REPLACE)।

अधिक विवरण के लिए उदाहरण देखें।

## JSON REPLACE

* `/replace`:
  ```
  POST /replace
  {
    "table": "<table name>",
    "id": <document id>,
    "doc":
    {
      "<field1>": <value1>,
      ...
      "<fieldN>": <valueN>
    }
  }
  ```
  `/index` एक उपनाम एंडपॉइंट है और यह समान काम करता है।
* Elasticsearch-के सामान एंडपॉइंट `<table>/_doc/<id>`:
  ```
  PUT/POST /<table name>/_doc/<id>
  {
    "<field1>": <value1>,
    ...
    "<fieldN>": <valueN>
  }
  ```
  > नोट: Elasticsearch-जैसे प्रतिस्थापन के लिए [Manticore Buddy](../Installation/Manticore_Buddy.md) की आवश्यकता है। यदि यह काम नहीं करता है, तो सुनिश्चित करें कि Buddy स्थापित है।
* आंशिक प्रतिस्थापन:
  ```
  POST /<{table | cluster:table}>/_update/<id>
  {
    "<field1>": <value1>,
    ...
    "<fieldN>": <valueN>
  }
  ```
  `<table name>` केवल तालिका के नाम के रूप में या `cluster:table` के प्रारूप में हो सकता है। यदि आवश्यक हो तो यह एक विशिष्ट क्लस्टर के पार अपडेट की अनुमति देता है।

  > नोट: आंशिक प्रतिस्थापन के लिए [Manticore Buddy](../Installation/Manticore_Buddy.md) की आवश्यकता है। यदि यह काम नहीं करता है, तो सुनिश्चित करें कि Buddy स्थापित है।

अधिक विवरण के लिए उदाहरण देखें।

<!-- परिचय -->
##### SQL:
<!-- अनुरोध SQL -->

```sql
REPLACE INTO products VALUES(1, "document one", 10);
```

<!-- प्रतिक्रिया SQL -->

```sql
Query OK, 1 row affected (0.00 sec)
```

<!-- परिचय -->
##### REPLACE ... SET:
<!-- अनुरोध REPLACE SET -->

```sql
REPLACE INTO products SET description='HUAWEI Matebook 15', price=10 WHERE id = 55;
```

<!-- प्रतिक्रिया REPLACE ... SET -->

```sql
Query OK, 1 row affected (0.00 sec)
```

<!-- परिचय -->
##### JSON

<!-- अनुरोध JSON -->

```json
POST /replace
-H "Content-Type: application/x-ndjson" -d '
{
  "table":"products",
  "id":1,
  "doc":
  {
    "title":"product one",
    "price":10
  }
}
'

```

<!-- प्रतिक्रिया JSON -->
```json
{
  "table":"products",
  "_id":1,
  "created":false,
  "result":"updated",
  "status":200
}

```

<!-- परिचय -->
##### Elasticsearch-जैसे

<!-- अनुरोध Elasticsearch-जैसे -->

> नोट: Elasticsearch-जैसे प्रतिस्थापन के लिए [Manticore Buddy](../Installation/Manticore_Buddy.md) की आवश्यकता है। यदि यह काम नहीं करता है, तो सुनिश्चित करें कि Buddy स्थापित है।

```json
PUT /products/_doc/2
{
  "title": "product two",
  "price": 20
}

POST /products/_doc/3
{
  "title": "product three",
  "price": 10
}
```

<!-- प्रतिक्रिया Elasticsearch-जैसे -->
```json
{
"_id":2,
"table":"products",
"_primary_term":1,
"_seq_no":0,
"_shards":{
    "failed":0,
    "successful":1,
    "total":1
},
"_type":"_doc",
"_version":1,
"result":"updated"
}

{
"_id":3,
"table":"products",
"_primary_term":1,
"_seq_no":0,
"_shards":{
    "failed":0,
    "successful":1,
    "total":1
},
"_type":"_doc",
"_version":1,
"result":"updated"
}
```

<!-- परिचय -->
##### Elasticsearch-जैसे आंशिक प्रतिस्थापन:

<!-- अनुरोध Elasticsearch-जैसे आंशिक -->

> नोट: आंशिक प्रतिस्थापन के लिए [Manticore Buddy](../Installation/Manticore_Buddy.md) की आवश्यकता है। यदि यह काम नहीं करता है, तो सुनिश्चित करें कि Buddy स्थापित है।

```json
POST /products/_update/55
{
  "doc": {
    "description": "HUAWEI Matebook 15",
    "price": 10
  }
}
```

<!-- प्रतिक्रिया Elasticsearch-जैसे आंशिक -->
```json
{
"table":"products",
"updated":1
}
```

<!-- परिचय -->
##### Elasticsearch-जैसे आंशिक प्रतिस्थापन क्लस्टर में:

<!-- अनुरोध Elasticsearch-जैसे आंशिक क्लस्टर में -->

```json
POST /cluster_name:products/_update/55
{
  "doc": {
    "description": "HUAWEI Matebook 15",
    "price": 10
  }
}
```

<!-- प्रतिक्रिया Elasticsearch-जैसे आंशिक क्लस्टर में -->
```json
{
"table":"products",
"updated":1
}
```

<!-- परिचय -->
##### PHP:

<!-- अनुरोध PHP -->

```php
$index->replaceDocument([
   'title' => 'document one',
    'price' => 10
],1);
```

<!-- प्रतिक्रिया PHP -->
```php
Array(
    [_index] => products
    [_id] => 1
    [created] => false
    [result] => updated
    [status] => 200
)
```
<!-- परिचय -->

##### Python:

<!-- अनुरोध Python -->
``` python
indexApi.replace({"table" : "products", "id" : 1, "doc" : {"title" : "document one","price":10}})
```

<!-- प्रतिक्रिया Python -->
```python
{'created': False,
 'found': None,
 'id': 1,
 'table': 'products',
 'result': 'updated'}
```
<!-- परिचय -->

##### Javascript:

<!-- अनुरोध javascript -->
``` javascript
res = await indexApi.replace({"table" : "products", "id" : 1, "doc" : {"title" : "document one","price":10}});
```

<!-- response javascript -->
```javascript
{"table":"products","_id":1,"result":"updated"}
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
docRequest = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
            put("title","document one");
            put("price",10);
}};
docRequest.index("products").id(1L).setDoc(doc);
sqlresult = indexApi.replace(docRequest);
```

<!-- response Java -->
```java
class SuccessResponse {
    index: products
    id: 1
    created: false
    result: updated
    found: null
}

```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "document one");
doc.Add("price", 10);
InsertDocumentRequest docRequest = new InsertDocumentRequest(index: "products", id: 1, doc: doc);
var sqlresult = indexApi.replace(docRequest);
```

<!-- response C# -->
```clike
class SuccessResponse {
    index: products
    id: 1
    created: false
    result: updated
    found: null
}

```

<!-- intro -->

##### TypeScript:

<!-- request TypeScript -->
``` typescript
res = await indexApi.replace({
  index: 'test',
  id: 1,
  doc: { content: 'Text 11', name: 'Doc 11', cat: 3 },
});
```

<!-- response TypeScript -->
```json
{
    "table":"test",
    "_id":1,
    "created":false
    "result":"updated"
    "status":200
}
```

<!-- intro -->

##### Go:

<!-- request Go -->
``` go
replaceDoc := map[string]interface{} {"content": "Text 11", "name": "Doc 11", "cat": 3}
replaceRequest := manticoreclient.NewInsertDocumentRequest("test", replaceDoc)
replaceRequest.SetId(1)
res, _, _ := apiClient.IndexAPI.Replace(context.Background()).InsertDocumentRequest(*replaceRequest).Execute()
```

<!-- response Go -->
```go
{
    "table":"test",
    "_id":1,
    "created":false
    "result":"updated"
    "status":200
}
```

<!-- end -->

`REPLACE` वास्तविक समय और पर्कोलेट तालिकाओं के लिए उपलब्ध है। आप एक साधारण तालिका में डेटा नहीं बदल सकते।

जब आप एक `REPLACE` चलाते हैं, तो पिछला दस्तावेज़ हटा नहीं होता, लेकिन इसे हटा हुआ के रूप में चिह्नित किया जाता है, इसलिए तालिका का आकार तब तक बढ़ता है जब तक वर्क खंड का विलय नहीं होता। खंड विलय को मजबूर करने के लिए, [OPTIMIZE कथन](../../Securing_and_compacting_a_table/Compacting_a_table.md) का उपयोग करें।

## बल्क रिप्लेस

<!-- example bulk_replace -->

आप एक बार में कई दस्तावेज़ बदल सकते हैं। अधिक जानकारी के लिए [बुल्क दस्तावेज़ जोड़ने](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-adding-documents) की जांच करें।

<!-- intro -->
##### HTTP:

<!-- request SQL -->

```sql
REPLACE INTO products(id,title,tag) VALUES (1, 'doc one', 10), (2,' doc two', 20);
```

<!-- response SQL -->

```sql
Query OK, 2 rows affected (0.00 sec)
```

<!-- request JSON -->

```json
POST /bulk
-H "Content-Type: application/x-ndjson" -d '
{ "replace" : { "table" : "products", "id":1, "doc": { "title": "doc one", "tag" : 10 } } }
{ "replace" : { "table" : "products", "id":2, "doc": { "title": "doc two", "tag" : 20 } } }
'
```

<!-- response JSON -->

```json
{
  "items":
  [
    {
      "replace":
      {
        "table":"products",
        "_id":1,
        "created":false,
        "result":"updated",
        "status":200
      }
    },
    {
      "replace":
      {
        "table":"products",
        "_id":2,
        "created":false,
        "result":"updated",
        "status":200
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
$index->replaceDocuments([
    [
        'id' => 1,
        'title' => 'document one',
        'tag' => 10
    ],
    [
        'id' => 2,
        'title' => 'document one',
        'tag' => 20
    ]
);
```

<!-- response PHP -->
```php
Array(
    [items] =>
    Array(
        Array(
            [_index] => products
            [_id] => 2
            [created] => false
            [result] => updated
            [status] => 200
        )
        Array(
            [_index] => products
            [_id] => 2
            [created] => false
            [result] => updated
            [status] => 200
        )
    )
    [errors => false
)
```
<!-- request Python -->

``` python
indexApi = manticoresearch.IndexApi(client)
docs = [ \
    {"replace": {"table" : "products", "id" : 1, "doc" : {"title" : "document one"}}}, \
    {"replace": {"table" : "products", "id" : 2, "doc" : {"title" : "document two"}}} ]
api_resp = indexApi.bulk('\n'.join(map(json.dumps,docs)))
```

<!-- response Python -->
```python
{'error': None,
 'items': [{u'replace': {u'_id': 1,
                         u'table': u'products',
                         u'created': False,
                         u'result': u'updated',
                         u'status': 200}},
           {u'replace': {u'_id': 2,
                         u'table': u'products',
                         u'created': False,
                         u'result': u'updated',
                         u'status': 200}}]}

```
<!-- request javascript -->

``` javascript
docs = [
    {"replace": {"table" : "products", "id" : 1, "doc" : {"title" : "document one"}}},
    {"replace": {"table" : "products", "id" : 2, "doc" : {"title" : "document two"}}} ];
res =  await indexApi.bulk(docs.map(e=>JSON.stringify(e)).join('\n'));
```
<!-- response javascript -->
```javascript
{"items":[{"replace":{"table":"products","_id":1,"created":false,"result":"updated","status":200}},{"replace":{"table":"products","_id":2,"created":false,"result":"updated","status":200}}],"errors":false}

```

<!-- request Java -->

``` java
body = "{\"replace\": {\"index\" : \"products\", \"id\" : 1, \"doc\" : {\"title\" : \"document one\"}}}" +"\n"+
    "{\"replace\": {\"index\" : \"products\", \"id\" : 2, \"doc\" : {\"title\" : \"document two\"}}}"+"\n" ;
indexApi.bulk(body);
```

<!-- response Java -->
```java
class BulkResponse {
    items: [{replace={_index=products, _id=1, created=false, result=updated, status=200}}, {replace={_index=products, _id=2, created=false, result=updated, status=200}}]
    error: null
    additionalProperties: {errors=false}
}
```

<!-- request C# -->

``` clike
string body = "{\"replace\": {\"index\" : \"products\", \"id\" : 1, \"doc\" : {\"title\" : \"document one\"}}}" +"\n"+
    "{\"replace\": {\"index\" : \"products\", \"id\" : 2, \"doc\" : {\"title\" : \"document two\"}}}"+"\n" ;
indexApi.Bulk(body);
```

<!-- response C# -->
```clike
class BulkResponse {
    items: [{replace={_index=products, _id=1, created=false, result=updated, status=200}}, {replace={_index=products, _id=2, created=false, result=updated, status=200}}]
    error: null
    additionalProperties: {errors=false}
}
```

<!-- request TypeScript -->

``` typescript
replaceDocs = [
  {
    replace: {
      index: 'test',
      id: 1,
      doc: { content: 'Text 11', cat: 1, name: 'Doc 11' },
    },
  },
  {
    replace: {
      index: 'test',
      id: 2,
      doc: { content: 'Text 22', cat: 9, name: 'Doc 22' },
    },
  },
];

res = await indexApi.bulk(
  replaceDocs.map((e) => JSON.stringify(e)).join("\n")
);
```

<!-- response TypeScript -->
```typescript
{
  "items":
  [
    {
      "replace":
      {
        "table":"test",
        "_id":1,
        "created":false,
        "result":"updated",
        "status":200
      }
    },
    {
      "replace":
      {
        "table":"test",
        "_id":2,
        "created":false,
        "result":"updated",
        "status":200
      }
    }
  ],
  "errors":false
}
```

<!-- request Go -->

``` go
body := "{\"replace\": {\"index\": \"test\", \"id\": 1, \"doc\": {\"content\": \"Text 11\", \"name\": \"Doc 11\", \"cat\": 1 }}}" + "\n" +
	"{\"replace\": {\"index\": \"test\", \"id\": 2, \"doc\": {\"content\": \"Text 22\", \"name\": \"Doc 22\", \"cat\": 9 }}}" +"\n";
res, _, _ := apiClient.IndexAPI.Bulk(context.Background()).Body(body).Execute()
```

<!-- response Go -->
```go
{
  "items":
  [
    {
      "replace":
      {
        "table":"test",
        "_id":1,
        "created":false,
        "result":"updated",
        "status":200
      }
    },
    {
      "replace":
      {
        "table":"test",
        "_id":2,
        "created":false,
        "result":"updated",
        "status":200
      }
    }
  ],
  "errors":false
}
```

<!-- end -->

<!-- proofread -->

