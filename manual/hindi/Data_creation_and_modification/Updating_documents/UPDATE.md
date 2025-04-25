# UPDATE


<!-- example update -->


`UPDATE` कमांड एक निर्दिष्ट टेबल में नए मूल्यों के साथ मौजूदा दस्तावेजों के [पंक्ति-वार](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) विशेषताओं के मानों को बदलता है। ध्यान दें कि आप फुलटेक्स्ट फ़ील्ड या कॉलमर विशेषता की सामग्री को अपडेट नहीं कर सकते। यदि इसके लिए आवश्यकता हो, तो [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md) का उपयोग करें।


विशेषता अपडेट RT, PQ, और सामान्य तालिकाओं के लिए समर्थित हैं। सभी विशेषता प्रकारों को अपडेट किया जा सकता है जब तक कि उन्हें [पंक्ति-वार संग्रहण](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) में संग्रहीत किया गया हो।


ध्यान दें कि दस्तावेज़ ID को अपडेट नहीं किया जा सकता।


यह महत्वपूर्ण है कि एक विशेषता को अपडेट करने से इसकी [द्वितीयक सूचकांक](../../Server_settings/Searchd.md#secondary_indexes) अक्षम हो जाती है। यदि द्वितीयक सूचकांक निरंतरता बनाए रखना महत्वपूर्ण है, तो इसके बजाय पूरी तरह से या [आंशिक रूप से प्रतिस्थापित](../../Data_creation_and_modification/Updating_documents/REPLACE.md?client=REPLACE+SET) करने पर विचार करें।


`UPDATE` और आंशिक `REPLACE` के बारे में अधिक पढ़ें [यहाँ](../../Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md#UPDATE-vs-partial-REPLACE)।


<!-- intro -->

##### SQL:

<!-- request SQL -->


```sql

UPDATE products SET enabled=0 WHERE id=10;

```


<!-- response -->


```sql

Query OK, 1 row affected (0.00 sec)

```


<!-- intro -->

##### JSON:


<!-- request JSON -->


```JSON

POST /update

{

  "table":"products",

  "id":10,

  "doc":

  {

    "enabled":0

  }

}

```


<!-- response JSON -->
```JSON

{

  "table":"products",

  "updated":1

}

```


<!-- intro -->

##### PHP:


<!-- request PHP -->


```php

$index->updateDocument([

    'enabled'=>0

],10);

```


<!-- response PHP -->
```php

Array(

    [_index] => products

    [_id] => 10

    [result] => updated

)

```


<!-- intro -->

##### Python:


<!-- request Python -->
``` python

indexApi = api = manticoresearch.IndexApi(client)

indexApi.update({"table" : "products", "id" : 1, "doc" : {"price":10}})

```


<!-- response Python -->
```python

{'id': 1, 'table': 'products', 'result': 'updated', 'updated': None}

```


<!-- intro -->

##### Python-asyncio:


<!-- request Python-asyncio -->
``` python

indexApi = api = manticoresearch.IndexApi(client)

await indexApi.update({"table" : "products", "id" : 1, "doc" : {"price":10}})

```


<!-- response Python-asyncio -->
```python

{'id': 1, 'table': 'products', 'result': 'updated', 'updated': None}

```


<!-- intro -->

##### Javascript:


<!-- request javascript -->
``` javascript

res = await indexApi.update({"table" : "products", "id" : 1, "doc" : {"price":10}});

```


<!-- response javascript -->
```javascript

{"table":"products","_id":1,"result":"updated"}

```
<!-- intro -->

##### java:


<!-- request Java -->
``` java

UpdateDocumentRequest updateRequest = new UpdateDocumentRequest();

doc = new HashMap<String,Object >(){{

    put("price",10);

}};

updateRequest.index("products").id(1L).setDoc(doc);

indexApi.update(updateRequest);

```


<!-- response Java -->
```java

class UpdateResponse {

    index: products

    updated: null

    id: 1

    result: updated

}

```


<!-- intro -->

##### C#:


<!-- request C# -->
``` clike

Dictionary<string, Object> doc = new Dictionary<string, Object>();

doc.Add("price", 10);

UpdateDocumentRequest updateRequest = new UpdateDocumentRequest(index: "products", id: 1, doc: doc);

indexApi.Update(updateRequest);

```


<!-- response C# -->
```clike

class UpdateResponse {

    index: products

    updated: null

    id: 1

    result: updated

}

```


<!-- intro -->

##### Rust:


<!-- request Rust -->
``` rust

let mut doc = HashMap::new();

doc.insert("price".to_string(), serde_json::json!(10));

let update_req = UpdateDocumentRequest {

    table: serde_json::json!("products"),

    doc: serde_json::json!(doc),

    id: serde_json::json!(1),

    ..Default::default(),

};

let update_res = index_api.update(update_req).await;

```


<!-- response Rust -->
```rust

class UpdateResponse {

    index: products

    updated: null

    id: 1

    result: updated

}

```


<!-- intro -->

##### TypeScript:


<!-- request TypeScript -->
``` typescript

res = await indexApi.update({ index: "test", id: 1, doc: { cat: 10 } });

```


<!-- response TypeScript -->
```typescript

{

"table":"test",

"_id":1,

"result":"updated"

}

```


<!-- intro -->

##### Go:


<!-- request Go -->
``` go

updateDoc = map[string]interface{} {"cat":10}

updateRequest = openapiclient.NewUpdateDocumentRequest("test", updateDoc)

updateRequest.SetId(1)

res, _, _ = apiClient.IndexAPI.Update(context.Background()).UpdateDocumentRequest(*updateRequest).Execute()

```


<!-- response Go -->
```go

{

"table":"test",

"_id":1,

"result":"updated"

}

```


<!-- end -->


<!-- example update multiple attributes -->


कई विशेषताओं को एक ही बयान में अपडेट किया जा सकता है। उदाहरण:


<!-- intro -->

##### SQL:
<!-- request SQL -->


```sql

UPDATE products

SET price=100000000000,

    coeff=3465.23,

    tags1=(3,6,4),

    tags2=()

WHERE MATCH('phone') AND enabled=1;

```


<!-- response -->


```sql

Query OK, 148 rows affected (0.0 sec)

```


<!-- intro -->

##### JSON:


<!-- request JSON -->


```JSON

POST /update

{

  "table":"products",

  "doc":

  {

    "price":100000000000,

    "coeff":3465.23,

    "tags1":[3,6,4],

    "tags2":[]

  },

  "query":

  {

"match": { "*": "phone" },

"equals": { "enabled": 1 }
  }
}
```

<!-- response JSON -->
```JSON
{
  "table":"products",
  "updated":148
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$query= new BoolQuery();
$query->must(new Match('phone','*'));
$query->must(new Equals('enabled',1));
$index->updateDocuments([
    'price' => 100000000000,
    'coeff' => 3465.23,
    'tags1' => [3,6,4],
    'tags2' => []
    ],
    $query
);
```

<!-- response PHP -->
```php
Array(
    [_index] => products
    [updated] => 148
)
```

<!-- intro -->
##### Python:

<!-- request Python -->
``` python
indexApi = api = manticoresearch.IndexApi(client)
indexApi.update({"table" : "products", "id" : 1, "doc" : {
    "price": 100000000000,
    "coeff": 3465.23,
    "tags1": [3,6,4],
    "tags2": []}})
```

<!-- response Python -->
```python
{'id': 1, 'table': 'products', 'result': 'updated', 'updated': None}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->
``` python
indexApi = api = manticoresearch.IndexApi(client)
await indexApi.update({"table" : "products", "id" : 1, "doc" : {
    "price": 100000000000,
    "coeff": 3465.23,
    "tags1": [3,6,4],
    "tags2": []}})
```

<!-- response Python-asyncio -->
```python
{'id': 1, 'table': 'products', 'result': 'updated', 'updated': None}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.update({"table" : "products", "id" : 1, "doc" : {
    "price": 100000000000,
    "coeff": 3465.23,
    "tags1": [3,6,4],
    "tags2": []}});
```

<!-- response javascript -->
```javascript
{"table":"products","_id":1,"result":"updated"}
```
<!-- intro -->
##### java:

<!-- request Java -->
``` java
UpdateDocumentRequest updateRequest = new UpdateDocumentRequest();
doc = new HashMap<String,Object >(){{
    put("price",10);
    put("coeff",3465.23);
    put("tags1",new int[]{3,6,4});
    put("tags2",new int[]{});
}};
updateRequest.index("products").id(1L).setDoc(doc);
indexApi.update(updateRequest);
```

<!-- response Java -->
```java
class UpdateResponse {
    index: products
    updated: null
    id: 1
    result: updated
}
```

<!-- intro -->
##### C#:

<!-- request C# -->
``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("price", 10);
doc.Add("coeff", 3465.23);
doc.Add("tags1", new List<int> {3,6,4});
doc.Add("tags2", new List<int> {});
UpdateDocumentRequest updateRequest = new UpdateDocumentRequest(index: "products", id: 1, doc: doc);
indexApi.Update(updateRequest);

```

<!-- response C# -->
```clike
class UpdateResponse {
    index: products
    updated: null
    id: 1
    result: updated
}
```

<!-- intro -->
##### Rust:

<!-- request Rust -->
``` rust
let mut doc = HashMap::new();
doc.insert("price".to_string(), serde_json::json!(10));
doc.insert("coeff".to_string(), serde_json::json!(3465.23));
doc.insert("tags1".to_string(), serde_json::json!([3,6,4]));
doc.insert("tags2".to_string(), serde_json::json!([]));
let update_req = UpdateDocumentRequest {
    table: serde_json::json!("products"),
    doc: serde_json::json!(doc),
    id: serde_json::json!(1),
    ..Default::default(),
};
let update_res = index_api.update(update_req).await;
```

<!-- response Rust -->
```rust
class UpdateResponse {
    index: products
    updated: null
    id: 1
    result: updated
}
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->
``` typescript
res = await indexApi.update({ index: "test", id: 1, doc: { name: "Doc 21", cat: "10" } });
```

<!-- response TypeScript -->
```go
{
  "table":"test",
  "_id":1,
  "result":"updated"
}
```

<!-- intro -->
##### Go:

<!-- request Go -->
``` go
updateDoc = map[string]interface{} {"name":"Doc 21", "cat":10}
updateRequest = manticoreclient.NewUpdateDocumentRequest("test", updateDoc)
updateRequest.SetId(1)
res, _, _ = apiClient.IndexAPI.Update(context.Background()).UpdateDocumentRequest(*updateRequest).Execute()
```

<!-- response Go -->
```go
{
  "table":"test",
  "_id":1,
  "result":"updated"
}
```

<!-- end -->

When assigning out-of-range values to 32-bit attributes, they will be trimmed to their lower 32 bits without a prompt. For example, if you try to update the 32-bit unsigned int with a value of 4294967297, the value of 1 will actually be stored, because the lower 32 bits of 4294967297 (0x100000001 in hex) amount to 1 (0x00000001 in hex).

<!-- example partial JSON update -->

`UPDATE` can be used to perform partial JSON updates on numeric data types or arrays of numeric data types. Just make sure you don't update an integer value with a float value as it will be rounded off.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
insert into products (id, title, meta) values (1,'title','{"tags":[1,2,3]}');

update products set meta.tags[0]=100 where id=1;
```

<!-- response -->

```sql
Query OK, 1 row affected (0.00 sec)

Query OK, 1 row affected (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /insert
{

"table":"products",

"id":100,

"doc":

{

"title":"title",

"meta": {
            "tags":[1,2,3]
        }

}
}

POST /update
{

"table":"products",

"id":100,

"doc":

{

"meta.tags[0]":100

}
}
```

<!-- response JSON -->
```JSON
{
   "table":"products",
   "_id":100,
   "created":true,
   "result":"created",
   "status":201
}

{
  "table":"products",
  "updated":1
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->insertDocument([
    'title' => 'शीर्षक',
    'meta' => ['tags' => [1,2,3]]
],1);
$index->updateDocument([
    'meta.tags[0]' => 100
],1);
```

<!-- response PHP -->
```php
Array(
    [_index] => products
    [_id] => 1
    [created] => true
    [result] => बनाया गया
)

Array(
    [_index] => products
    [updated] => 1
)
```

<!-- intro -->
##### Python:

<!-- request Python -->
``` python
indexApi = api = manticoresearch.IndexApi(client)
indexApi.update({"table" : "products", "id" : 1, "doc" : {
    "meta.tags[0]": 100}})
```

<!-- response Python -->
```python
{'id': 1, 'table': 'products', 'result': 'अपडेट किया गया', 'updated': None}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->
``` python
indexApi = api = manticoresearch.IndexApi(client)
await indexApi.update({"table" : "products", "id" : 1, "doc" : {
    "meta.tags[0]": 100}})
```

<!-- response Python-asyncio -->
```python
{'id': 1, 'table': 'products', 'result': 'अपडेट किया गया', 'updated': None}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.update({"table" : "products", "id" : 1, "doc" : {
   "meta.tags[0]": 100}});
```

<!-- response javascript -->
```javascript
{"table":"products","_id":1,"result":"अपडेट किया गया"}
```

<!-- intro -->
##### java:

<!-- request Java -->
``` java
UpdateDocumentRequest updateRequest = new UpdateDocumentRequest();
doc = new HashMap<String,Object >(){{
    put("meta.tags[0]",100);
}};
updateRequest.index("products").id(1L).setDoc(doc);
indexApi.update(updateRequest);
```

<!-- response Java -->
```java
class UpdateResponse {
    index: products
    updated: null
    id: 1
    result: अपडेट किया गया
}
```

<!-- intro -->
##### C#:

<!-- request C# -->
``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("meta.tags[0]", 100);
UpdateDocumentRequest updateRequest = new UpdateDocumentRequest(index: "products", id: 1, doc: doc);
indexApi.Update(updateRequest);

```

<!-- response C# -->
```clike
class UpdateResponse {
    index: products
    updated: null
    id: 1
    result: अपडेट किया गया
}
```

<!-- intro -->
##### Rust:

<!-- request Rust -->
``` rust
let mut doc = HashMap::new();
doc.insert("meta.tags[0]".to_string(), serde_json::json!(100));
let update_req = UpdateDocumentRequest {
    table: serde_json::json!("products"),
    doc: serde_json::json!(doc),
    id: serde_json::json!(1),
    ..Default::default(),
};
let update_res = index_api.update(update_req).await;
```

<!-- response Rust -->
```rust
class UpdateResponse {
    index: products
    updated: null
    id: 1
    result: अपडेट किया गया
}
```


<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->
``` typescript
res = await indexApi.update({"table" : "test", "id" : 1, "doc" : { "meta.tags[0]": 100} });
```

<!-- response TypeScript -->
```typescript
{"table":"test","_id":1,"result":"अपडेट किया गया"}
```

<!-- intro -->
##### Go:

<!-- request Go -->
``` go
updateDoc = map[string]interface{} {"meta.tags[0]":100}
updateRequest = manticoreclient.NewUpdateDocumentRequest("test", updateDoc)
updateRequest.SetId(1)
res, _, _ = apiClient.IndexAPI.Update(context.Background()).UpdateDocumentRequest(*updateRequest).Execute()
```

<!-- response Go -->
```go
{

"table":"test",

"_id":1,

"result":"अपडेट किया गया"
}
```

<!-- end -->

<!-- example full JSON update -->

अन्य डेटा प्रकारों को अपडेट करना या JSON गुण में संपत्ति प्रकार को बदलना एक पूर्ण JSON अपडेट की आवश्यकता होती है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
insert into products values (1,'शीर्षक','{"tags":[1,2,3]}');

update products set data='{"tags":["one","two","three"]}' where id=1;
```

<!-- response -->

```sql
Query OK, 1 row affected (0.00 sec)

Query OK, 1 row affected (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /insert
{

"table":"products",

"id":1,

"doc":

{

"title":"शीर्षक",

"data":"{"tags":[1,2,3]}"

}
}

POST /update
{

"table":"products",

"id":1,

"doc":

{

"data":"{"tags":["one","two","three"]}"

}
}
```

<!-- response JSON -->
```JSON
{
  "table":"products",
  "updated":1
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->insertDocument([
    'title'=> 'शीर्षक',
    'data' => [
         'tags' => [1,2,3]
    ]
],1);

$index->updateDocument([
    'data' => [
            'one', 'two', 'three'
    ]
],1);
```

<!-- response PHP -->
```php
Array(
    [_index] => products
    [_id] => 1
    [created] => true
    [result] => बनाया गया
)

Array(
    [_index] => products
    [updated] => 1
)
```
<!-- intro -->
##### Python:

<!-- request Python -->
``` python
indexApi.insert({"table" : "products", "id" : 100, "doc" : {"title" : "शीर्षक", "meta" : {"tags":[1,2,3]}}})
indexApi.update({"table" : "products", "id" : 100, "doc" : {"meta" : {"tags":['one','two','three']}}})
```

<!-- response Python -->
```python

{'created': True,
 'found': None,
 'id': 100,
 'table': 'products',
 'result': 'बनाया गया'}
{'id': 100, 'table': 'products', 'result': 'अपडेट किया गया', 'updated': None}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->
``` python
await indexApi.insert({"table" : "products", "id" : 100, "doc" : {"title" : "title", "meta" : {"tags":[1,2,3]}}})
await indexApi.update({"table" : "products", "id" : 100, "doc" : {"meta" : {"tags":['one','two','three']}}})
```

<!-- response Python-asyncio -->
```python

{'created': True,
 'found': None,
 'id': 100,
 'table': 'products',
 'result': 'created'}
{'id': 100, 'table': 'products', 'result': 'updated', 'updated': None}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.insert({"table" : "products", "id" : 100, "doc" : {"title" : "title", "meta" : {"tags":[1,2,3]}}});
res = await indexApi.update({"table" : "products", "id" : 100, "doc" : {"meta" : {"tags":['one','two','three']}}});
```

<!-- response javascript -->
```javascript
{"table":"products","_id":100,"created":true,"result":"created"}
{"table":"products","_id":100,"result":"updated"}

```


<!-- intro -->
##### java:

<!-- request Java -->
``` java
InsertDocumentRequest newdoc = new InsertDocumentRequest();
doc = new HashMap<String,Object>(){{
    put("title","title");
    put("meta",
        new HashMap<String,Object>(){{
            put("tags",new int[]{1,2,3});
        }});

}};
newdoc.index("products").id(100L).setDoc(doc);        
indexApi.insert(newdoc);

updatedoc = new UpdateDocumentRequest();
doc = new HashMap<String,Object >(){{
    put("meta",
        new HashMap<String,Object>(){{
            put("tags",new String[]{"one","two","three"});
        }});
}};
updatedoc.index("products").id(100L).setDoc(doc);
indexApi.update(updatedoc);

```

<!-- response Java -->
```java
class SuccessResponse {
    index: products
    id: 100
    created: true
    result: created
    found: null
}

class UpdateResponse {
    index: products
    updated: null
    id: 100
    result: updated
}
```

<!-- intro -->
##### C#:

<!-- request C# -->
``` clike
Dictionary<string, Object> meta = new Dictionary<string, Object>();
meta.Add("tags", new List<int> {1,2,3});
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "title");
doc.Add("meta", meta);
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", id: 100, doc: doc);
indexApi.Insert(newdoc);

meta = new Dictionary<string, Object>();
meta.Add("tags", new List<string> {"one","two","three"});
doc = new Dictionary<string, Object>();
doc.Add("meta", meta);
UpdateDocumentRequest updatedoc = new UpdateDocumentRequest(index: "products", id: 100, doc: doc);
indexApi.Update(updatedoc);
```

<!-- response C# -->
```clike
class SuccessResponse {
    index: products
    id: 100
    created: true
    result: created
    found: null
}

class UpdateResponse {
    index: products
    updated: null
    id: 100
    result: updated
}
```

<!-- intro -->
##### Rust:

<!-- request Rust -->
``` rust
let mut meta = HashMap::new();
meta.insert("tags".to_string(), serde_json::json!([1,2,3]));
let mut doc = HashMap::new();
doc.insert("title".to_string(), serde_json::json!("title"));
doc.insert("meta".to_string(), serde_json::json!(meta));
let insert_req = InsertDocumentRequest {
    table: serde_json::json!("products"),
    doc: serde_json::json!(doc),
    id: serde_json::json!(100),
    ..Default::default(),
};
let insert_res = index_api.insert(insert_req).await;

meta = HashMap::new();
meta.insert("tags".to_string(), serde_json::json!(["one","two","three"]));
doc = HashMap::new();
doc.insert("meta".to_string(), serde_json::json!(meta));
let update_req = UpdateDocumentRequest {
    table: serde_json::json!("products"),
    doc: serde_json::json!(doc),
    id: serde_json::json!(100),
    ..Default::default(),
};
let update_res = index_api.update(update_req).await;

```

<!-- response Rust -->
```rust
class SuccessResponse {
    index: products
    id: 100
    created: true
    result: created
    found: null
}

class UpdateResponse {
    index: products
    updated: null
    id: 100
    result: updated
}
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->
``` typescript
res = await indexApi.insert({
  index: 'test',
  id: 1,
  doc: { content: 'Text 1', name: 'Doc 1', meta: { tags:[1,2,3] } }
})
res = await indexApi.update({ index: 'test', id: 1, doc: { meta: { tags:['one','two','three'] } } });
```

<!-- response TypeScript -->
```typescript
{

"table":"test",

"_id":1,

"created":true,

"result":"created"
}

{

"table":"test",

"_id":1,

"result":"updated"
}
```

<!-- intro -->
##### TypeScript:

<!-- request Go -->
``` go
metaField := map[string]interface{} {"tags": []int{1, 2, 3}}
insertDoc := map[string]interface{} {"name": "Doc 1", "meta": metaField}}
insertRequest := manticoreclient.NewInsertDocumentRequest("test", insertDoc)
insertRequest.SetId(1)
res, _, _ := apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*insertRequest).Execute();

metaField = map[string]interface{} {"tags": []string{"one", "two", "three"}}
updateDoc := map[string]interface{} {"meta": metaField}
updateRequest := manticoreclient.NewUpdateDocumentRequest("test", updateDoc)
res, _, _ = apiClient.IndexAPI.Update(context.Background()).UpdateDocumentRequest(*updateRequest).Execute()
```

<!-- response Go -->
```go
{

"table":"test",

"_id":1,

"created":true,

"result":"created"
}

{

"table":"test",

"_id":1,

"result":"updated"
}
```

<!-- end -->

<!-- example cluster update -->
पुनरुत्पादन का उपयोग करते समय, तालिका के नाम के साथ `cluster_name:` (SQL में) पहले जोड़ा जाना चाहिए ताकि अद्यतनों को क्लस्टर के सभी नोड्स पर फैलाया जा सके। HTTP के माध्यम से क्वेरी के लिए, आपको `cluster` प्रॉपर्टी सेट करनी चाहिए। अधिक जानकारी के लिए [पुनरुत्पादन सेट करना](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md) देखें।

```json
{
  "cluster":"nodes4",
  "table":"test",
  "id":1,
  "doc":
  {
    "gid" : 100,
    "price" : 1000
  }
}
```

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
update weekly:posts set enabled=0 where id=1;
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /update
{

"cluster":"weekly",

"table":"products",

"id":1,

"doc":

{

"enabled":0

}
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->setName('products')->setCluster('weekly');
$index->updateDocument(['enabled'=>0],1);
```
<!-- intro -->
##### Python:

<!-- request Python -->
``` python
indexApi.update({"cluster":"weekly", "table" : "products", "id" : 1, "doc" : {"enabled" : 0}})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->
``` python
await indexApi.update({"cluster":"weekly", "table" : "products", "id" : 1, "doc" : {"enabled" : 0}})
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->
``` javascript
res = wait indexApi.update({"cluster":"weekly", "table" : "products", "id" : 1, "doc" : {"enabled" : 0}});

```
<!-- intro -->
##### java:

<!-- request Java -->
``` java
updatedoc = new UpdateDocumentRequest();
doc = new HashMap<String,Object >(){{
    put("enabled",0);
}};
updatedoc.index("products").cluster("weekly").id(1L).setDoc(doc);
indexApi.update(updatedoc);

```

<!-- intro -->
##### C#:

<!-- request C# -->
``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("enabled", 0);
UpdateDocumentRequest updatedoc = new UpdateDocumentRequest(index: "products", cluster: "weekly", id: 1, doc: doc);
indexApi.Update(updatedoc);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->
``` rust
let mut doc = HashMap::new();
doc.insert("enabled".to_string(), serde_json::json!(0));
let update_req = UpdateDocumentRequest {
    table: serde_json::json!("products"),
    cluster: serde_json::json!("weekly"),
    doc: serde_json::json!(doc),
    id: serde_json::json!(1),
};
let update_res = index_api.update(update_req).await;
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->
``` typescript
res = wait indexApi.update( {cluster: 'test_cluster', index : 'test', id : 1, doc : {name : 'Doc 11'}} );
```

<!-- intro -->
##### Go:

<!-- request Go -->
``` go
updateDoc = map[string]interface{} {"name":"Doc 11"}
updateRequest = manticoreclient.NewUpdateDocumentRequest("test", updateDoc)
updateRequest.SetCluster("test_cluster")
updateRequest.SetId(1)
res, _, _ = apiClient.IndexAPI.Update(context.Background()).UpdateDocumentRequest(*updateRequest).Execute()
```

<!-- end -->


## SQL के माध्यम से अद्यतनों

यहां SQL `UPDATE` स्टेटमेंट का सिंटैक्स है:

```sql
UPDATE table SET col1 = newval1 [, ...] WHERE where_condition [OPTION opt_name = opt_value [, ...]] [FORCE|IGNORE INDEX(id)]
```


`where_condition` का वही सिंटैक्स है जो [SELECT](../../Searching/Full_text_matching/Basic_usage.md#SQL) स्टेटमेंट में है।

<!-- example MVA empty update -->

मल्टी-वैल्यू एट्रिब्यूट वैल्यू सेट्स को कोमा से पृथक सूचियों के रूप में परिभाषित किया जाना चाहिए। एक मल्टी-वैल्यू एट्रिब्यूट से सभी मान हटाने के लिए, बस इसका मान `()` असाइन करें।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
UPDATE products SET tags1=(3,6,4) WHERE id=1;

UPDATE products SET tags1=() WHERE id=1;
```

<!-- response SQL -->

```sql
Query OK, 1 row affected (0.00 sec)

Query OK, 1 row affected (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /update

{

"table":"products",

"_id":1,

"doc":

{

"tags1": []

}
}
```

<!-- response JSON -->
```JSON
{
  "table":"products",
  "updated":1
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->updateDocument(['tags1'=>[]],1);
```

<!-- response PHP -->
```php
Array(
    [_index] => products
    [updated] => 1
)
```

<!-- intro -->
##### Python:

<!-- request Python -->
``` python

indexApi.update({"table" : "products", "id" : 1, "doc" : {"tags1": []}})
```

<!-- response Python -->
```python
{'id': 1, 'table': 'products', 'result': 'updated', 'updated': None}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->
``` python
await indexApi.update({"table" : "products", "id" : 1, "doc" : {"tags1": []}})
```

<!-- response Python-asyncio -->
```python
{'id': 1, 'table': 'products', 'result': 'updated', 'updated': None}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->
``` javascript

indexApi.update({"table" : "products", "id" : 1, "doc" : {"tags1": []}})
```

<!-- response javascript -->
```javascript
{"table":"products","_id":1,"result":"updated"}
```
<!-- intro -->
##### java:

<!-- request Java -->
``` java
updatedoc = new UpdateDocumentRequest();
doc = new HashMap<String,Object >(){{
    put("tags1",new int[]{});
}};
updatedoc.index("products").id(1L).setDoc(doc);
indexApi.update(updatedoc);

```

<!-- response Java -->
```java
class UpdateResponse {
    index: products
    updated: null
    id: 1
    result: updated
}
```
<!-- intro -->
##### C#:

<!-- request C# -->
``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("tags1", new List<int> {});
UpdateDocumentRequest updatedoc = new UpdateDocumentRequest(index: "products", id: 1, doc: doc);
indexApi.Update(updatedoc);
```

<!-- response C# -->
```clike
class UpdateResponse {
    index: products
    updated: null
    id: 1
    result: updated
}
```

<!-- intro -->
##### Rust:

<!-- request Rust -->
``` rust
let mut doc = HashMap::new();
doc.insert("tags1".to_string(), serde_json::json!([]));
let update_req = UpdateDocumentRequest {
    table: serde_json::json!("products"),
    doc: serde_json::json!(doc),
    id: serde_json::json!(1),
    ..Default::default(),
};
let update_res = index_api.update(update_req).await;
```

<!-- response Rust -->
```rust
class UpdateResponse {
    index: products
    updated: null
    id: 1
    result: updated
}
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->
``` typescript
res = await indexApi.update({ index: 'test', id: 1, doc: { cat: 10 } });
```

<!-- response TypeScript -->
```typescript
{

"table":"test",

"_id":1,

"result":"updated"
}
```

<!-- intro -->
##### Go:

<!-- request Go -->
``` go
updateDoc = map[string]interface{} {"cat":10}
updateRequest = manticoreclient.NewUpdateDocumentRequest("test", updateDoc)
updateRequest.SetId(1)
res, _, _ = apiClient.IndexAPI.Update(context.Background()).UpdateDocumentRequest(*updateRequest).Execute()
```

<!-- response Go -->
```go
{

"table":"test",

"_id":1,

"result":"updated"
}
```

<!-- end -->


`OPTION` क्लॉज़ एक Manticore-specific extension है जो आपको कई अपडेट विकल्पों को नियंत्रित करने देता है। इसका व्याकरण है:

```sql
OPTION <optionname>=<value> [ , ... ]
```

विकल्प [SELECT](../../Searching/Full_text_matching/Basic_usage.md#SQL) कथन के समान हैं। विशेष रूप से `UPDATE` कथन के लिए, आप इन विकल्पों का उपयोग कर सकते हैं:

*   'ignore_nonexistent_columns' - यदि **1** पर सेट किया गया हो, तो यह इंगीत करता है कि अपडेट मौजूदा तालिका स्कीमा में नहीं होने वाले कॉलम को अपडेट करने के बारे में किसी भी चेतावनी को चुपचाप नजरअंदाज कर देगा। डिफ़ॉल्ट मान **0** है।
*   'strict' - यह विकल्प आंशिक JSON विशेषता अपडेट में प्रयोग किया जाता है। डिफ़ॉल्ट रूप से (strict=1), `UPDATE` एक त्रुटि उत्पन्न करेगा यदि `UPDATE` क्वेरी गैर-सांख्यिक विशेषताओं पर अपडेट करने की कोशिश करती है। strict=0 के साथ, यदि एकाधिक विशेषताएँ अपडेट होती हैं और कुछ अनुमत नहीं हैं, तो `UPDATE` एक त्रुटि उत्पन्न नहीं करेगा और केवल अनुमत विशेषताओं पर परिवर्तन करेगा (अन्य को नजरअंदाज करते हुए)। यदि `UPDATE` के `SET` परिवर्तनों में से कोई भी अनुमति नहीं है, तो कमांड strict=0 के साथ भी एक त्रुटि उत्पन्न करेगा।

### क्वेरी ऑप्टिमाइज़र हिंट्स

दुर्लभ मामलों में, Manticore का अंतर्निहित क्वेरी विश्लेषक एक क्वेरी को समझने और यह निर्धारित करने में गलत हो सकता है कि पहचानकर्ता द्वारा तालिका का उपयोग किया जाना चाहिए या नहीं। यह `UPDATE ... WHERE id = 123` जैसी क्वेरी के लिए खराब प्रदर्शन का परिणाम कर सकता है।
एक दस्तावेज़ पहचानकर्ता अनुक्रमणिका का उपयोग करने के लिए ऑप्टिमाइज़र को मजबूर करने के तरीके के बारे में जानकारी के लिए, [Query optimizer hints](../../Searching/Options.md#Query optimizer hints) देखें।

## HTTP JSON के माध्यम से अपडेट्स

HTTP JSON प्रोटोकॉल का उपयोग करके अपडेट `/update` अंत बिंदु के माध्यम से किया जाता है। इसका व्याकरण [/insert अंत बिंदु](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real_time_table.md) के समान है, लेकिन इस बार `doc` संपत्ति अनिवार्य है।

सर्वर एक JSON ऑब्जेक्ट के साथ प्रतिक्रिया करेगा जिसमें बताया जाएगा कि ऑपरेशन सफल था या नहीं।

<!-- उदाहरण JSON अपडेट -->

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /update
{
  "table":"test",
  "id":1,
  "doc":
   {
     "gid" : 100,
     "price" : 1000
    }
}
```

<!-- response JSON -->
``` JSON
{
  "table": "test",
  "_id": 1,
  "result": "updated"
}
```

<!-- end -->

<!-- उदाहरण JSON उदाहरण_2 -->

उस दस्तावेज़ की पहचान जो अपडेट किया जाना आवश्यक है, को सीधे `id` संपत्ति का उपयोग करके सेट किया जा सकता है, जैसा कि पिछले उदाहरण में दिखाया गया है, या आप क्वेरी द्वारा दस्तावेज़ अपडेट कर सकते हैं और उन सभी दस्तावेज़ों पर अपडेट लागू कर सकते हैं जो क्वेरी से मेल खाते हैं:

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /update

{
  "table":"test",
  "doc":
  {
    "price" : 1000
  },
  "query":
  {
    "match": { "*": "apple" }
  }
}
```

<!-- response JSON -->

```json
{
  "table":"products",
  "updated":1
}
```

<!-- end -->

क्वेरी का व्याकरण [/search अंत बिंदु](../../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) में जैसा ही है। ध्यान दें कि आप एक साथ `id` और `query` निर्दिष्ट नहीं कर सकते।

## एट्रिब्यूट्स को फ्लश करना

```sql
FLUSH ATTRIBUTES
```

FLUSH ATTRIBUTES कमांड सुनिश्चित करता है कि सभी सक्रिय तालिकाओं में सभी मेमोरी में विशेषता अपडेट डिस्क पर फ्लश किए जाएँ। यह एक टैग लौटाता है जो परिणाम की ऑन-डिस्क स्थिति को पहचानता है, जो दर्शाता है कि सर्वर शुरू होने के बाद कितनी वास्तविक डिस्क विशेषता बचत की गई है।

```sql
mysql> UPDATE testindex SET channel_id=1107025 WHERE id=1;
Query OK, 1 row affected (0.04 sec)

mysql> FLUSH ATTRIBUTES;
+------+
| tag  |
+------+
|    1 |
+------+
1 row in set (0.19 sec)
```
[attr_flush_period](../../Data_creation_and_modification/Updating_documents/UPDATE.md#attr_flush_period) सेटिंग भी देखें।


## बल्क अपडेट्स

<!-- उदाहरण बल्क अपडेट -->
आप एक ही कॉल में कई अपडेट ऑपरेशन `/bulk` एंडपॉइंट का उपयोग करके कर सकते हैं। यह एंडपॉइंट केवल डेटा के साथ काम करता है जिसमें `Content-Type` `application/x-ndjson` पर सेट है। डेटा को न्यूलाइन-सीमित JSON (NDJSON) के रूप में स्वरूपित किया जाना चाहिए। मूल रूप से, इसका अर्थ है कि प्रत्येक पंक्ति में एक JSON कथन होना चाहिए और यह एक न्यूलाइन `
` से समाप्त होना चाहिए और, संभवतः, एक ``।


<!-- intro -->
##### JSON:

<!-- request JSON --Content-type=application/x-ndjson -->

```json
POST /bulk

{ "update" : { "table" : "products", "id" : 1, "doc": { "price" : 10 } } }
{ "update" : { "table" : "products", "id" : 2, "doc": { "price" : 20 } } }
```

<!-- response JSON -->

```json
{
   "items":
   [
      {
         "update":
         {
            "table":"products",
            "_id":1,
            "result":"updated"
         }
      },
      {
         "update":
         {
            "table":"products",
            "_id":2,
            "result":"updated"
         }
      }
   ],
   "errors":false
}
```

<!-- end -->

`/bulk` एंडपॉइंट अबाधित, प्रतिस्थापन और मिटाने का समर्थन करता है। प्रत्येक कथन एक क्रिया प्रकार से शुरू होता है (इस मामले में, `update`)। यहां समर्थित कार्यों की सूची है:

* `insert`: एक दस्तावेज़ डालता है। वाक्यविन्यास [/insert एंडपॉइंट](../../Quick_start_guide.md#Add-documents) में समान है।
* `create`: `insert` के लिए एक पर्यायवाची
* `replace`: एक दस्तावेज़ को प्रतिस्थापित करता है। वाक्यविन्यास [/replace](../../Data_creation_and_modification/Updating_documents/REPLACE.md) में समान है।
* `index`: `replace` के लिए एक पर्यायवाची
* `update`: एक दस्तावेज़ को अपडेट करता है। वाक्यविन्यास [/update](../../Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-HTTP-JSON) में समान है।
* `delete`: एक दस्तावेज़ को मिटाता है। वाक्यविन्यास [/delete एंडपॉइंट](../../Data_creation_and_modification/Deleting_documents.md) में समान है।

क्वेरी द्वारा अपडेट और क्वेरी द्वारा मिटाने का भी समर्थन किया जाता है।

<!-- example bulk by query -->

<!-- intro -->
##### JSON:

<!-- request JSON --Content-type=application/x-ndjson -->

```json
POST /bulk

{ "update" : { "table" : "products", "doc": { "coeff" : 1000 }, "query": { "range": { "price": { "gte": 1000 } } } } }
{ "update" : { "table" : "products", "doc": { "coeff" : 0 }, "query": { "range": { "price": { "lt": 1000 } } } } }
```

<!-- response JSON -->

```json
{
  "items":
  [
    {
      "update":
      {
        "table":"products",
        "updated":1
      }
    },
    {
      "update":
      {
        "table":"products",
        "updated":3
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
    ['update'=>[
            'table' => 'products',
             'doc' => [
                'coeff' => 100
            ],
            'query' => [
                'range' => ['price'=>['gte'=>1000]]
            ]   
        ]
    ],
    ['update'=>[
            'table' => 'products',
             'doc' => [
                'coeff' => 0
            ],
            'query' => [
                'range' => ['price'=>['lt'=>1000]]
            ]   
        ]
    ]
]);
```

<!-- response PHP -->

```php
Array(
    [items] => Array (
        Array(
            [update] => Array(
                [_index] => products
                [updated] => 1
            )
        )   
        Array(
             [update] => Array(
                 [_index] => products
                 [updated] => 3
             )
        )    
)

```


<!-- intro -->
##### Python:

<!-- request Python -->
``` python
docs = [             { "update" : { "table" : "products", "doc": { "coeff" : 1000 }, "query": { "range": { "price": { "gte": 1000 } } } } },             { "update" : { "table" : "products", "doc": { "coeff" : 0 }, "query": { "range": { "price": { "lt": 1000 } } } } } ]
indexApi.bulk('
'.join(map(json.dumps,docs)))
```

<!-- response Python -->
```python
{'error': None,
 'items': [{u'update': {u'table': u'products', u'updated': 1}},
           {u'update': {u'table': u'products', u'updated': 3}}]}

```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->
``` python
docs = [             { "update" : { "table" : "products", "doc": { "coeff" : 1000 }, "query": { "range": { "price": { "gte": 1000 } } } } },             { "update" : { "table" : "products", "doc": { "coeff" : 0 }, "query": { "range": { "price": { "lt": 1000 } } } } } ]
await indexApi.bulk('
'.join(map(json.dumps,docs)))
```

<!-- response Python-asyncio -->
```python
{'error': None,
 'items': [{u'update': {u'table': u'products', u'updated': 1}},
           {u'update': {u'table': u'products', u'updated': 3}}]}

```

<!-- intro -->
##### Javascript:

<!-- request javascript -->
``` javascript
docs = [
            { "update" : { "table" : "products", "doc": { "coeff" : 1000 }, "query": { "range": { "price": { "gte": 1000 } } } } },
            { "update" : { "table" : "products", "doc": { "coeff" : 0 }, "query": { "range": { "price": { "lt": 1000 } } } } } ];
res =  await indexApi.bulk(docs.map(e=>JSON.stringify(e)).join('
'));
```

<!-- response javascript -->
```javascript
{"items":[{"update":{"table":"products","updated":1}},{"update":{"table":"products","updated":3}}],"errors":false}

```

<!-- intro -->
##### java:

<!-- request Java -->
``` java
String   body = "{ "update" : { "index" : "products", "doc": { "coeff" : 1000 }, "query": { "range": { "price": { "gte": 1000 } } } }} "+"
"+
    "{ "update" : { "index" : "products", "doc": { "coeff" : 0 }, "query": { "range": { "price": { "lt": 1000 } } } } }"+"
";         
indexApi.bulk(body);
```

<!-- response Java -->
```java
class BulkResponse {
    items: [{update={_index=products, _id=1, created=false, result=updated, status=200}}, {update={_index=products, _id=2, created=false, result=updated, status=200}}]
    error: null
    additionalProperties: {errors=false}
}
```

<!-- intro -->
##### C#:

<!-- request C# -->
``` clike
string   body = "{ "update" : { "index" : "products", "doc": { "coeff" : 1000 }, "query": { "range": { "price": { "gte": 1000 } } } }} "+"
"+
    "{ "update" : { "index" : "products", "doc": { "coeff" : 0 }, "query": { "range": { "price": { "lt": 1000 } } } } }"+"
";         
indexApi.Bulk(body);
```

<!-- response C# -->
```clike
class BulkResponse {
    items: [{update={_index=products, _id=1, created=false, result=updated, status=200}}, {update={_index=products, _id=2, created=false, result=updated, status=200}}]
    error: null
# अपडेट

<!-- example update -->

The `UPDATE` command changes [row-wise](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) attribute values of existing documents in a specified table with new values. Note that you can't update the contents of a fulltext field or a columnar attribute. If there's such a need, use [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md).

RT, PQ, and plain तालिकाओं के लिए गुण अद्यतन समर्थित हैं। सभी गुण प्रकार अपडेट किए जा सकते हैं बशर्ते वे [row-wise storage](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) में संग्रहीत हों।

ध्यान दें कि दस्तावेज़ आईडी को अपडेट नहीं किया जा सकता।

यह जानना महत्वपूर्ण है कि किसी गुण को अपडेट करने से उसका [secondary index](../../Server_settings/Searchd.md#secondary_indexes) अक्षम हो जाता है। यदि सेकेंडरी इंडेक्स निरंतरता बनाए रखना महत्वपूर्ण है, तो इसके बजाय दस्तावेज़ को पूर्ण रूप से या [आंशिक रूप से बदलने](../../Data_creation_and_modification/Updating_documents/REPLACE.md?client=REPLACE+SET) पर विचार करें।

`UPDATE` बनाम आंशिक `REPLACE` के बारे में अधिक पढ़ने के लिए [यहाँ](../../Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md#UPDATE-vs-partial-REPLACE) जाएँ।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
UPDATE products SET enabled=0 WHERE id=10;
```

<!-- response -->

```sql
क्वेरी ठीक है, 1 पंक्ति प्रभावित हुई (0.00 सेकंड)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /update

{
  "table":"products",
  "id":10,
  "doc":
  {
    "enabled":0
  }
}
```

<!-- response JSON -->
```JSON
{
  "table":"products",
  "updated":1
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->updateDocument([
    'enabled'=>0
],10);
```

<!-- response PHP -->
```php
Array(
    [_index] => products
    [_id] => 10
    [result] => updated
)
```

<!-- intro -->
##### Python:

<!-- request Python -->
``` python
indexApi = api = manticoresearch.IndexApi(client)
indexApi.update({"table" : "products", "id" : 1, "doc" : {"price":10}})
```

<!-- response Python -->
```python
{'id': 1, 'table': 'products', 'result': 'updated', 'updated': None}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.update({"table" : "products", "id" : 1, "doc" : {"price":10}});
```

<!-- response javascript -->
```javascript
{"table":"products","_id":1,"result":"updated"}
```
<!-- intro -->
##### java:

<!-- request Java -->
``` java
UpdateDocumentRequest updateRequest = new UpdateDocumentRequest();
doc = new HashMap<String,Object >(){{
    put("price",10);
}};
updateRequest.index("products").id(1L).setDoc(doc);
indexApi.update(updateRequest);
```

<!-- response Java -->
```java
class UpdateResponse {
    index: products
    updated: null
    id: 1
    result: updated
}
```

<!-- intro -->
##### C#:

<!-- request C# -->
``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("price", 10);
UpdateDocumentRequest updateRequest = new UpdateDocumentRequest(index: "products", id: 1, doc: doc);
indexApi.Update(updateRequest);
```

<!-- response C# -->
```clike
class UpdateResponse {
    index: products
    updated: null
    id: 1
    result: updated
}
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->
``` typescript
res = await indexApi.update({ index: "test", id: 1, doc: { cat: 10 } });
```

<!-- response TypeScript -->
```typescript
{
	"table":"test",
	"_id":1,
	"result":"updated"
}
```

<!-- intro -->
##### Go:

<!-- request Go -->
``` go
updateDoc = map[string]interface{} {"cat":10}
updateRequest = openapiclient.NewUpdateDocumentRequest("test", updateDoc)
updateRequest.SetId(1)
res, _, _ = apiClient.IndexAPI.Update(context.Background()).UpdateDocumentRequest(*updateRequest).Execute()
```

<!-- response Go -->
```go
{
	"table":"test",
	"_id":1,
	"result":"updated"
}
```

<!-- end -->

<!-- example update multiple attributes -->

Multiple attributes can be updated in a single statement. Example:

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
UPDATE products
SET price=100000000000,
    coeff=3465.23,
    tags1=(3,6,4),
    tags2=()
WHERE MATCH('phone') AND enabled=1;
```

<!-- response -->

```sql
क्वेरी ठीक है, 148 पंक्तियाँ प्रभावित हुईं (0.0 सेकंड)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /update
{
  "table":"products",
  "doc":
  {
    "price":100000000000,
    "coeff":3465.23,
    "tags1":[3,6,4],
    "tags2":[]
  },
  "query":
  {
	"match": { "*": "phone" },
	"equals": { "enabled": 1 }
  }
}
```

<!-- response JSON -->
```JSON
{
  "table":"products",
  "updated":148
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$query= new BoolQuery();
$query->must(new Match('phone','*'));
$query->must(new Equals('enabled',1));
$index->updateDocuments([
    'price' => 100000000000,
    'coeff' => 3465.23,
    'tags1' => [3,6,4],
    'tags2' => []
    ],
    $query
);
```

<!-- response PHP -->
```php
Array(
    [_index] => products
    [updated] => 148
)
```

<!-- intro -->
##### Python:

<!-- request Python -->
``` python
indexApi = api = manticoresearch.IndexApi(client)
indexApi.update({"table" : "products", "id" : 1, "doc" : {
    "price": 100000000000,
    "coeff": 3465.23,
    "tags1": [3,6,4],
    "tags2": []}})
```

<!-- response Python -->
```python

{'id': 1, 'table': 'products', 'result': 'updated', 'updated': None}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.update({"table" : "products", "id" : 1, "doc" : {
    "price": 100000000000,
    "coeff": 3465.23,
    "tags1": [3,6,4],
    "tags2": []}});
```

<!-- response javascript -->
```javascript
{"table":"products","_id":1,"result":"updated"}
```
<!-- intro -->
##### java:

<!-- request Java -->
``` java
UpdateDocumentRequest updateRequest = new UpdateDocumentRequest();
doc = new HashMap<String,Object >(){{
    put("price",10);
    put("coeff",3465.23);
    put("tags1",new int[]{3,6,4});
    put("tags2",new int[]{});
}};
updateRequest.index("products").id(1L).setDoc(doc);
indexApi.update(updateRequest);
```

<!-- response Java -->
```java
class UpdateResponse {
    index: products
    updated: null
    id: 1
    result: updated
}
```

<!-- intro -->
##### C#:

<!-- request C# -->
``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("price", 10);
doc.Add("coeff", 3465.23);
doc.Add("tags1", new List<int> {3,6,4});
doc.Add("tags2", new List<int> {});
UpdateDocumentRequest updateRequest = new UpdateDocumentRequest(index: "products", id: 1, doc: doc);
indexApi.Update(updateRequest);

```

<!-- response C# -->
```clike
class UpdateResponse {
    index: products
    updated: null
    id: 1
    result: updated
}
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->
``` typescript
res = await indexApi.update({ index: "test", id: 1, doc: { name: "Doc 21", cat: "10" } });
```

<!-- response TypeScript -->
```go
{
  "table":"test",
  "_id":1,
  "result":"updated"
}
```

<!-- intro -->
##### Go:

<!-- request Go -->
``` go
updateDoc = map[string]interface{} {"name":"Doc 21", "cat":10}
updateRequest = manticoreclient.NewUpdateDocumentRequest("test", updateDoc)
updateRequest.SetId(1)
res, _, _ = apiClient.IndexAPI.Update(context.Background()).UpdateDocumentRequest(*updateRequest).Execute()
```

<!-- response Go -->
```go
{
  "table":"test",
  "_id":1,
  "result":"updated"
}
```

<!-- end -->

When assigning out-of-range values to 32-bit attributes, they will be trimmed to their lower 32 bits without a prompt. For example, if you try to update the 32-bit unsigned int with a value of 4294967297, the value of 1 will actually be stored, because the lower 32 bits of 4294967297 (0x100000001 in hex) amount to 1 (0x00000001 in hex).

<!-- example partial JSON update -->

`UPDATE` can be used to perform partial JSON updates on numeric data types or arrays of numeric data types. Just make sure you don't update an integer value with a float value as it will be rounded off.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
insert into products (id, title, meta) values (1,'title','{"tags":[1,2,3]}');

update products set meta.tags[0]=100 where id=1;
```

<!-- response -->

```sql
Query OK, 1 row affected (0.00 sec)

Query OK, 1 row affected (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /insert
{
	"table":"products",
	"id":100,
	"doc":
	{
		"title":"title",
		"meta": {
            "tags":[1,2,3]
        }
	}
}

POST /update
{
	"table":"products",
	"id":100,
	"doc":
	{
		"meta.tags[0]":100
	}
}
```

<!-- response JSON -->
```JSON
{
   "table":"products",
   "_id":100,
   "created":true,
   "result":"created",
   "status":201
}

{
  "table":"products",
  "updated":1
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->insertDocument([
    'title' => 'title',
    'meta' => ['tags' => [1,2,3]]
],1);
$index->updateDocument([
    'meta.tags[0]' => 100
],1);
```

<!-- response PHP -->
```php
Array(
    [_index] => products
    [_id] => 1
    [created] => true
    [result] => created
)

Array(
    [_index] => products
    [updated] => 1
)
```

<!-- intro -->
##### Python:

<!-- request Python -->
``` python
indexApi = api = manticoresearch.IndexApi(client)
indexApi.update({"table" : "products", "id" : 1, "doc" : {
    "meta.tags[0]": 100}})
```

<!-- response Python -->
```python
{'id': 1, 'table': 'products', 'result': 'updated', 'updated': None}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.update({"table" : "products", "id" : 1, "doc" : {
   "meta.tags[0]": 100}});
```

<!-- response javascript -->
```javascript
{"table":"products","_id":1,"result":"updated"}
```

<!-- intro -->
##### java:

<!-- request Java -->
``` java
UpdateDocumentRequest updateRequest = new UpdateDocumentRequest();
doc = new HashMap<String,Object >(){{
    put("meta.tags[0]",100);
}};
updateRequest.index("products").id(1L).setDoc(doc);
indexApi.update(updateRequest);
```

<!-- response Java -->
```java
class UpdateResponse {
    index: products
    updated: null
    id: 1
    result: updated
}
```

<!-- intro -->
##### C#:

<!-- request C# -->
``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("meta.tags[0]", 100);
UpdateDocumentRequest updateRequest = new UpdateDocumentRequest(index: "products", id: 1, doc: doc);
indexApi.Update(updateRequest);

```

<!-- response C# -->
```clike
class UpdateResponse {
    index: products
    updated: null
    id: 1
    result: updated
}
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->
``` typescript
res = await indexApi.update({"table" : "test", "id" : 1, "doc" : { "meta.tags[0]": 100} });
```

<!-- response TypeScript -->
```typescript
{"table":"test","_id":1,"result":"updated"}
```

<!-- intro -->
##### Go:

<!-- request Go -->
``` go
updateDoc = map[string]interface{} {"meta.tags[0]":100}
updateRequest = manticoreclient.NewUpdateDocumentRequest("test", updateDoc)
updateRequest.SetId(1)
res, _, _ = apiClient.IndexAPI.Update(context.Background()).UpdateDocumentRequest(*updateRequest).Execute()
```

<!-- response Go -->
```go
{
	"table":"test",
	"_id":1,
	"result":"updated"
}
```

<!-- end -->

<!-- example full JSON update -->

अन्य डेटा प्रकारों को अपडेट करना या JSON गुण में गुण के प्रकार को बदलना पूर्ण JSON अपडेट की आवश्यकता है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
insert into products values (1,'title','{"tags":[1,2,3]}');

update products set data='{"tags":["one","two","three"]}' where id=1;
```

<!-- response -->

```sql
Query OK, 1 row affected (0.00 sec)

Query OK, 1 row affected (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /insert
{
	"table":"products",
	"id":1,
	"doc":
	{
		"title":"title",
		"data":"{\"tags\":[1,2,3]}"
	}
}

POST /update
{
	"table":"products",
	"id":1,
	"doc":
	{
		"data":"{\"tags\":[\"one\",\"two\",\"three\"]}"
	}
}
```

<!-- response JSON -->
```JSON
{
  "table":"products",
  "updated":1
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->insertDocument([
    'title'=> 'title',
    'data' => [
         'tags' => [1,2,3]
    ]
],1);

$index->updateDocument([
    'data' => [
            'one', 'two', 'three'
    ]
],1);
```

<!-- response PHP -->
```php
Array(
    [_index] => products
    [_id] => 1
    [created] => true
    [result] => created
)

Array(
    [_index] => products
    [updated] => 1
)
```
<!-- intro -->
##### Python:

<!-- request Python -->
``` python
indexApi.insert({"table" : "products", "id" : 100, "doc" : {"title" : "title", "meta" : {"tags":[1,2,3]}}})
indexApi.update({"table" : "products", "id" : 100, "doc" : {"meta" : {"tags":['one','two','three']}}})
```

<!-- response Python -->
```python

{'created': True,
 'found': None,
 'id': 100,
 'table': 'products',
 'result': 'created'}
{'id': 100, 'table': 'products', 'result': 'updated', 'updated': None}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.insert({"table" : "products", "id" : 100, "doc" : {"title" : "title", "meta" : {"tags":[1,2,3]}}});
res = await indexApi.update({"table" : "products", "id" : 100, "doc" : {"meta" : {"tags":['one','two','three']}}});
```

<!-- response javascript -->
```javascript
{"table":"products","_id":100,"created":true,"result":"created"}
{"table":"products","_id":100,"result":"updated"}

```


<!-- intro -->
##### java:

<!-- request Java -->
``` java
InsertDocumentRequest newdoc = new InsertDocumentRequest();
doc = new HashMap<String,Object>(){{
    put("title","title");
    put("meta",
        new HashMap<String,Object>(){{
            put("tags",new int[]{1,2,3});
        }});

}};
newdoc.index("products").id(100L).setDoc(doc);        
indexApi.insert(newdoc);

updatedoc = new UpdateDocumentRequest();
doc = new HashMap<String,Object >(){{
    put("meta",
        new HashMap<String,Object>(){{
            put("tags",new String[]{"one","two","three"});
        }});
}};
updatedoc.index("products").id(100L).setDoc(doc);
indexApi.update(updatedoc);

```

<!-- response Java -->
```java
class SuccessResponse {
    index: products
    id: 100
    created: true
    result: created
    found: null
}

class UpdateResponse {
    index: products
    updated: null
    id: 100
    result: updated
}
```

<!-- intro -->
##### C#:

<!-- request C# -->
``` clike
Dictionary<string, Object> meta = new Dictionary<string, Object>();
meta.Add("tags", new List<int> {1,2,3});
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "title");
doc.Add("meta", meta);
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", id: 100, doc: doc);
indexApi.Insert(newdoc);

meta = new Dictionary<string, Object>();
meta.Add("tags", new List<string> {"one","two","three"});
doc = new Dictionary<string, Object>();
doc.Add("meta", meta);
UpdateDocumentRequest updatedoc = new UpdateDocumentRequest(index: "products", id: 100, doc: doc);
indexApi.Update(updatedoc);
```

<!-- response C# -->
```clike
class SuccessResponse {
    index: products
    id: 100
    created: true
    result: created
    found: null
}

class UpdateResponse {
    index: products
    updated: null
    id: 100
    result: updated
}
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->
``` typescript
res = await indexApi.insert({
  index: 'test',
  id: 1,
  doc: { content: 'Text 1', name: 'Doc 1', meta: { tags:[1,2,3] } }
})
res = await indexApi.update({ index: 'test', id: 1, doc: { meta: { tags:['one','two','three'] } } });
```

<!-- response TypeScript -->
```typescript
{
	"table":"test",
	"_id":1,
	"created":true,
	"result":"created"
}

{
	"table":"test",
	"_id":1,
	"result":"updated"
}
```

<!-- intro -->
##### TypeScript:

<!-- request Go -->
``` go
metaField := map[string]interface{} {"tags": []int{1, 2, 3}}
insertDoc := map[string]interface{} {"name": "Doc 1", "meta": metaField}}
insertRequest := manticoreclient.NewInsertDocumentRequest("test", insertDoc)
insertRequest.SetId(1)
res, _, _ := apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*insertRequest).Execute();
metaField = map[string]interface{} {"tags": []string{"one", "two", "three"}}
updateDoc := map[string]interface{} {"meta": metaField}
updateRequest := manticoreclient.NewUpdateDocumentRequest("test", updateDoc)
res, _, _ = apiClient.IndexAPI.Update(context.Background()).UpdateDocumentRequest(*updateRequest).Execute()
```
<!-- response Go -->
```go
{
table":"test", 
_id":1,
created":true,
result":"created"
}
{
table":"test",
_id":1, 
result":"updated"
}
```
<!-- end -->
<!-- example cluster update -->
प्रतिकृति का उपयोग करते समय, क्लस्टर के सभी नोड्स में अपडेट प्रसारित करने के लिए तालिका नाम के पहले `cluster_name:` (SQL में) जोड़ा जाना चाहिए। HTTP के माध्यम से क्वेरी के लिए, आपको एक `cluster` प्रॉपर्टी सेट करनी चाहिए। अधिक जानकारी के लिए [प्रतिकृति सेट करना](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md) देखें।
```json
{
  "cluster":"nodes4",
  "table":"test",
  "id":1,
  "doc":
  {
    "gid" : 100,
    "price" : 1000
  }
}
```
<!-- intro -->
##### SQL:
<!-- request SQL -->
```sql
update weekly:posts set enabled=0 where id=1;
```
<!-- intro -->
##### JSON:
<!-- request JSON -->
```JSON
POST /update
{
cluster":"weekly",
table":"products",
id":1,
doc":

"enabled":0

}
```
<!-- intro -->
##### PHP:
<!-- request PHP -->
```php
$index->setName('products')->setCluster('weekly');
$index->updateDocument(['enabled'=>0],1);
```
<!-- intro -->
##### Python:
<!-- request Python -->
``` python
indexApi.update({"cluster":"weekly", "table" : "products", "id" : 1, "doc" : {"enabled" : 0}})
```
<!-- intro -->
##### Javascript:
<!-- request javascript -->
``` javascript
res = wait indexApi.update({"cluster":"weekly", "table" : "products", "id" : 1, "doc" : {"enabled" : 0}});
```
<!-- intro -->
##### java:
<!-- request Java -->
``` java
updatedoc = new UpdateDocumentRequest();
doc = new HashMap<String,Object >(){{
  put("enabled",0);
}};
updatedoc.index("products").cluster("weekly").id(1L).setDoc(doc);
indexApi.update(updatedoc);
```
<!-- intro -->
##### C#:
<!-- request C# -->
``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("enabled", 0);
UpdateDocumentRequest updatedoc = new UpdateDocumentRequest(index: "products", cluster: "weekly", id: 1, doc: doc);
indexApi.Update(updatedoc);
```
<!-- intro -->
##### TypeScript:
<!-- request TypeScript -->
``` typescript
res = wait indexApi.update( {cluster: 'test_cluster', index : 'test', id : 1, doc : {name : 'Doc 11'}} );
```
<!-- intro -->
##### Go:
<!-- request Go -->
``` go
updateDoc = map[string]interface{} {"name":"Doc 11"}
updateRequest = manticoreclient.NewUpdateDocumentRequest("test", updateDoc)
updateRequest.SetCluster("test_cluster")
updateRequest.SetId(1)
res, _, _ = apiClient.IndexAPI.Update(context.Background()).UpdateDocumentRequest(*updateRequest).Execute()
```
<!-- end -->
## SQL के माध्यम से अपडेट
SQL `UPDATE` स्टेटमेंट के लिए सिंटैक्स यहाँ दिया गया है:
```sql
UPDATE table SET col1 = newval1 [, ...] WHERE where_condition [OPTION opt_name = opt_value [, ...]] [FORCE|IGNORE INDEX(id)]
```
`where_condition` का सिंटैक्स [SELECT](../../Searching/Full_text_matching/Basic_usage.md#SQL) स्टेटमेंट के समान है।
<!-- example MVA empty update -->
मल्टी-वैल्यू एट्रिब्यूट वैल्यू सेट को कॉमा-सेपरेटेड लिस्ट के रूप में कोष्ठक में निर्दिष्ट किया जाना चाहिए। मल्टी-वैल्यू एट्रिब्यूट से सभी वैल्यू हटाने के लिए, बस उसे `()` असाइन करें।
<!-- intro -->
##### SQL:
<!-- request SQL -->
```sql
UPDATE products SET tags1=(3,6,4) WHERE id=1;
UPDATE products SET tags1=() WHERE id=1;
```
<!-- response SQL -->
```sql
Query OK, 1 row affected (0.00 sec)
Query OK, 1 row affected (0.00 sec)
```
<!-- intro -->
##### JSON:
<!-- request JSON -->
```JSON
POST /update
{
table":"products",
_id":1,
doc":

"tags1": []

}
```
<!-- response JSON -->
```JSON
{
  "table":"products",
  "updated":1
}
```
<!-- intro -->
##### PHP:
<!-- request PHP -->
```php
$index->updateDocument(['tags1'=>[]],1);
```
<!-- response PHP -->
```php
Array(
  [_index] => products
  [updated] => 1
)
```
<!-- intro -->
##### Python:
<!-- request Python -->
``` python
indexApi.update({"table" : "products", "id" : 1, "doc" : {"tags1": []}})
```
<!-- response Python -->
```python
{'id': 1, 'table': 'products', 'result': 'updated', 'updated': None}
```
<!-- intro -->
##### Javascript:
<!-- request javascript -->
``` javascript
indexApi.update({"table" : "products", "id" : 1, "doc" : {"tags1": []}})
```
<!-- response javascript -->
```javascript
{"table":"products","_id":1,"result":"updated"}
```
<!-- intro -->
##### java:
<!-- request Java -->
``` java
updatedoc = new UpdateDocumentRequest();
doc = new HashMap<String,Object >(){{
  put("tags1",new int[]{});
}};
updatedoc.index("products").id(1L).setDoc(doc);
indexApi.update(updatedoc);
```
<!-- response Java -->
```java
class UpdateResponse {
  index: products
  updated: null
  id: 1
  result: updated
}
```
<!-- intro -->
##### C#:

<!-- request C# -->
``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("tags1", new List<int> {});
UpdateDocumentRequest updatedoc = new UpdateDocumentRequest(index: "products", id: 1, doc: doc);
indexApi.Update(updatedoc);
```

<!-- response C# -->
```clike
class UpdateResponse {
    index: products
    updated: null
    id: 1
    result: updated
}
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->
``` typescript
res = await indexApi.update({ index: 'test', id: 1, doc: { cat: 10 } });
```

<!-- response TypeScript -->
```typescript
{
	"table":"test",
	"_id":1,
	"result":"updated"
}
```

<!-- intro -->
##### Go:

<!-- request Go -->
``` go
updateDoc = map[string]interface{} {"cat":10}
updateRequest = manticoreclient.NewUpdateDocumentRequest("test", updateDoc)
updateRequest.SetId(1)
res, _, _ = apiClient.IndexAPI.Update(context.Background()).UpdateDocumentRequest(*updateRequest).Execute()
```

<!-- response Go -->
```go
{
	"table":"test",
	"_id":1,
	"result":"updated"
}
```

<!-- end -->


`OPTION` क्लॉज़ एक Manticore-विशिष्ट एक्स्टेंशन है जो आपको कई प्रति-अपडेट विकल्पों को नियंत्रित करने देता है। इसका सिंटैक्स है:

```sql
OPTION <optionname>=<value> [ , ... ]
```

विकल्प वही हैं जो [SELECT](../../Searching/Full_text_matching/Basic_usage.md#SQL) स्टेटमेंट के लिए हैं। विशेष रूप से `UPDATE` स्टेटमेंट के लिए, आप इन विकल्पों का उपयोग कर सकते हैं:

*   'ignore_nonexistent_columns' - यदि **1** पर सेट किया गया है, तो इसका संकेत है कि अपडेट किसी भी चेतावनी को चुपचाप अनदेखा करेगा जिसमें यह बताया गया है कि वर्तमान तालिका योजना में कोई ऐसी कॉलम नहीं है जिसे अपडेट करने की कोशिश की जा रही है। डिफ़ॉल्ट मूल्य **0** है।
*   'strict' - इस विकल्प का उपयोग आंशिक JSON विशेषता अपडेट में किया जाता है। डिफ़ॉल्ट रूप से (strict=1), `UPDATE` एक त्रुटि उत्पन्न करेगा यदि `UPDATE` क्वेरी गैर-रेखीय गुणों पर अपडेट करने की कोशिश करती है। strict=0 के साथ, यदि कई गुण अपडेट किए जा रहे हैं और कुछ अनुमत नहीं हैं, तो `UPDATE` एक त्रुटि का परिणाम नहीं करेगा और केवल अनुमत गुणों पर परिवर्तन करेगा (बाकी को अनदेखा किया जाएगा)। यदि `UPDATE` के `SET` परिवर्तनों के कोई भी अनुमत नहीं हैं, तो आदेश एक त्रुटि का परिणाम देगा भले ही वह strict=0 हो।

### क्वेरी ऑप्टिमाइज़र संकेत

दुर्लभ मामलों में, Manticore का बिल्ट-इन क्वेरी विश्लेषक एक क्वेरी को समझने में गलत हो सकता है और यह निर्धारित कर सकता है कि क्या एक तालिका आईडी द्वारा उपयोग की जानी चाहिए। इससे ऐसे क्वेरियों के लिए खराब प्रदर्शन हो सकता है जैसे `UPDATE ... WHERE id = 123`।
ऑप्टिमाइज़र को एक दस्तावेज़ पहचानकर्ता अनुक्रमणिका का उपयोग करने के लिए मजबूर करने के बारे में जानकारी के लिए, देखें [क्वेरी ऑप्टिमाइज़र संकेत](../../Searching/Options.md#Query optimizer hints)।

## HTTP JSON के माध्यम से अपडेट

HTTP JSON प्रोटोकॉल का उपयोग करके अपडेट `/update` एंडपॉइंट के माध्यम से किए जाते हैं। इसका सिंटैक्स [/insert एंडपॉइंट](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md) के समान है, लेकिन इस बार `doc` प्रॉपर्टी अनिवार्य है।

सर्वर यह बताने वाले JSON ऑब्जेक्ट के साथ उत्तर देगा कि क्या संचालन सफल रहा या नहीं।

<!-- example JSON update -->

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /update
{
  "table":"test",
  "id":1,
  "doc":
   {
     "gid" : 100,
     "price" : 1000
    }
}
```

<!-- response JSON -->
``` JSON
{
  "table": "test",
  "_id": 1,
  "result": "updated"
}
```

<!-- end -->

<!-- example JSON Example_2 -->

उस दस्तावेज़ का आईडी जिसे अपडेट करने की आवश्यकता है, उसे सीधे `id` प्रॉपर्टी का उपयोग करके सेट किया जा सकता है, जैसा कि पिछले उदाहरण में दिखाया गया है, या आप क्वेरी द्वारा दस्तावेज़ों को अपडेट कर सकते हैं और उन सभी दस्तावेज़ों पर अपडेट लागू कर सकते हैं जो क्वेरी से मेल खाते हैं:

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /update

{
  "table":"test",
  "doc":
  {
    "price" : 1000
  },
  "query":
  {
    "match": { "*": "apple" }
  }
}
```

<!-- response JSON -->

```json
{
  "table":"products",
  "updated":1
}
```

<!-- end -->

क्वेरी सिंटैक्स [/search एंडपॉइंट](../../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) में समान है। ध्यान दें कि आप `id` और `query` को एक ही समय पर निर्दिष्ट नहीं कर सकते।

## एट्रिब्यूट्स को फ्लश करना

```sql
FLUSH ATTRIBUTES
```

FLUSH ATTRIBUTES कमांड यह सुनिश्चित करता है कि सभी सक्रिय तालिकाओं में सभी इन-मेमोरी एट्रिब्यूट अपडेट डिस्क पर फ्लश किए गए हैं। यह एक टैग लौटाता है जो डिस्क पर परिणाम की स्थिति को पहचानता है, जो सर्वर प्रारंभ होने के बाद किए गए वास्तविक डिस्क एट्रिब्यूट सहेजने की संख्या का प्रतिनिधित्व करता है।

```sql
mysql> UPDATE testindex SET channel_id=1107025 WHERE id=1;
Query OK, 1 row affected (0.04 sec)

mysql> FLUSH ATTRIBUTES;
+------+
| tag  |
+------+
|    1 |
+------+
1 row in set (0.19 sec)
```
देखें [attr_flush_period](../../Data_creation_and_modification/Updating_documents/UPDATE.md#attr_flush_period) सेटिंग।


## बल्क अपडेट

<!-- example bulk update -->

आप `/bulk` एंडपॉइंट का उपयोग करके एकल कॉल में कई अपडेट संचालन कर सकते हैं। यह एंडपॉइंट केवल उस डेटा के साथ काम करता है जिसका `Content-Type` `application/x-ndjson` सेट किया गया हो। डेटा को नई-पंक्ति-सीमांकित JSON (NDJSON) के रूप में प्रारूपित किया जाना चाहिए। मूल रूप से, इसका अर्थ है कि प्रत्येक पंक्ति में सटीक एक JSON कथन होना चाहिए और इसे एक नई पंक्ति `\n` के साथ समाप्त होना चाहिए और संभवतः एक `\r` के साथ भी।


<!-- intro -->
##### JSON:

<!-- request JSON --Content-type=application/x-ndjson -->

```json
POST /bulk

{ "update" : { "table" : "products", "id" : 1, "doc": { "price" : 10 } } }
{ "update" : { "table" : "products", "id" : 2, "doc": { "price" : 20 } } }
```

<!-- response JSON -->

```json
{
   "items":
   [
      {
         "update":
         {
            "table":"products",
            "_id":1,
            "result":"updated"
         }
      },
      {
         "update":
         {
            "table":"products",
            "_id":2,
            "result":"updated"
         }
      }
   ],
   "errors":false
}
```

<!-- end -->

`/bulk` अंत बिंदु सम्मिलन, प्रतिस्थापन और हटाने का समर्थन करता है। प्रत्येक कथन एक क्रिया प्रकार से शुरू होता है (इस मामले में, `update`)। समर्थित क्रियाओं की एक सूची यहां दी गई है:

* `insert`: एक दस्तावेज़ सम्मिलित करता है। वाक्य रचना [/insert endpoint](../../Quick_start_guide.md#Add-documents) के समान है।
* `create`: `insert` का पर्याय
* `replace`: एक दस्तावेज़ को प्रतिस्थापित करता है। वाक्य रचना [/replace](../../Data_creation_and_modification/Updating_documents/REPLACE.md) के समान है।
* `index`: `replace` का पर्याय
* `update`: एक दस्तावेज़ को अद्यतन करता है। वाक्य रचना [/update](../../Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-HTTP-JSON) के समान है।
* `delete`: एक दस्तावेज़ को हटाता है। वाक्य रचना [/delete endpoint](../../Data_creation_and_modification/Deleting_documents.md) के समान है।

क्वेरी द्वारा अद्यतन और क्वेरी द्वारा हटाना भी समर्थित है।

<!-- example bulk by query -->

<!-- intro -->
##### JSON:

<!-- request JSON --Content-type=application/x-ndjson -->

```json
POST /bulk

{ "update" : { "table" : "products", "doc": { "coeff" : 1000 }, "query": { "range": { "price": { "gte": 1000 } } } } }
{ "update" : { "table" : "products", "doc": { "coeff" : 0 }, "query": { "range": { "price": { "lt": 1000 } } } } }
```

<!-- response JSON -->

```json
{
  "items":
  [
    {
      "update":
      {
        "table":"products",
        "updated":1
      }
    },
    {
      "update":
      {
        "table":"products",
        "updated":3
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
    ['update'=>[
            'table' => 'products',
             'doc' => [
                'coeff' => 100
            ],
            'query' => [
                'range' => ['price'=>['gte'=>1000]]
            ]   
        ]
    ],
    ['update'=>[
            'table' => 'products',
             'doc' => [
                'coeff' => 0
            ],
            'query' => [
                'range' => ['price'=>['lt'=>1000]]
            ]   
        ]
    ]
]);
```

<!-- response PHP -->

```php
Array(
    [items] => Array (
        Array(
            [update] => Array(
                [_index] => products
                [updated] => 1
            )
        )   
        Array(
             [update] => Array(
                 [_index] => products
                 [updated] => 3
             )
        )    
)

```


<!-- intro -->
##### Python:

<!-- request Python -->
``` python
docs = [ \
            { "update" : { "table" : "products", "doc": { "coeff" : 1000 }, "query": { "range": { "price": { "gte": 1000 } } } } }, \
            { "update" : { "table" : "products", "doc": { "coeff" : 0 }, "query": { "range": { "price": { "lt": 1000 } } } } } ]
indexApi.bulk('\n'.join(map(json.dumps,docs)))
```

<!-- response Python -->
```python
{'error': None,
 'items': [{u'update': {u'table': u'products', u'updated': 1}},
           {u'update': {u'table': u'products', u'updated': 3}}]}

```
<!-- intro -->
##### Javascript:

<!-- request javascript -->
``` javascript
docs = [
            { "update" : { "table" : "products", "doc": { "coeff" : 1000 }, "query": { "range": { "price": { "gte": 1000 } } } } },
            { "update" : { "table" : "products", "doc": { "coeff" : 0 }, "query": { "range": { "price": { "lt": 1000 } } } } } ];
res =  await indexApi.bulk(docs.map(e=>JSON.stringify(e)).join('\n'));
```

<!-- response javascript -->
```javascript
{"items":[{"update":{"table":"products","updated":1}},{"update":{"table":"products","updated":3}}],"errors":false}

```

<!-- intro -->
##### java:

<!-- request Java -->
``` java
String   body = "{ \"update\" : { \"index\" : \"products\", \"doc\": { \"coeff\" : 1000 }, \"query\": { \"range\": { \"price\": { \"gte\": 1000 } } } }} "+"\n"+
    "{ \"update\" : { \"index\" : \"products\", \"doc\": { \"coeff\" : 0 }, \"query\": { \"range\": { \"price\": { \"lt\": 1000 } } } } }"+"\n";         
indexApi.bulk(body);
```

<!-- response Java -->
```java
class BulkResponse {
    items: [{update={_index=products, _id=1, created=false, result=updated, status=200}}, {update={_index=products, _id=2, created=false, result=updated, status=200}}]
    error: null
    additionalProperties: {errors=false}
}
```

<!-- intro -->
##### C#:

<!-- request C# -->
``` clike
string   body = "{ \"update\" : { \"index\" : \"products\", \"doc\": { \"coeff\" : 1000 }, \"query\": { \"range\": { \"price\": { \"gte\": 1000 } } } }} "+"\n"+
    "{ \"update\" : { \"index\" : \"products\", \"doc\": { \"coeff\" : 0 }, \"query\": { \"range\": { \"price\": { \"lt\": 1000 } } } } }"+"\n";         
indexApi.Bulk(body);
```

<!-- response C# -->
```clike
class BulkResponse {
    items: [{update={_index=products, _id=1, created=false, result=updated, status=200}}, {update={_index=products, _id=2, created=false, result=updated, status=200}}]
    error: null
    additionalProperties: {errors=false}
}
```

<!-- request TypeScript -->

``` typescript
updateDocs = [
  {
    update: {
      index: 'test',
      id: 1,
      doc: { content: 'Text 11', cat: 1, name: 'Doc 11' },
    },
  },
  {
    update: {
      index: 'test',
      id: 2,
      doc: { content: 'Text 22', cat: 9, name: 'Doc 22' },
    },
  },
];

res = await indexApi.bulk(
updateDocs.map((e) => JSON.stringify(e)).join("\n")
);
```

<!-- प्रतिक्रिया TypeScript -->
```typescript
{
  "items":
  [
    {
      "update":
      {
        "table":"test",
        "updated":1
      }
    },
    {
      "update":
      {
        "table":"test",
        "updated":1
      }
    }
  ],
  "errors":false
}
```

<!-- अनुरोध Go -->

``` go
body := "{\"update\": {\"index\": \"test\", \"id\": 1, \"doc\": {\"content\": \"Text 11\", \"name\": \"Doc 11\", \"cat\": 1 }}}" + "\n" +
	"{\"update\": {\"index\": \"test\", \"id\": 2, \"doc\": {\"content\": \"Text 22\", \"name\": \"Doc 22\", \"cat\": 9 }}}" +"\n";
res, _, _ := apiClient.IndexAPI.Bulk(context.Background()).Body(body).Execute()
```

<!-- प्रतिक्रिया Go -->
```go
{
  "items":
  [
    {
      "update":
      {
        "table":"test",
        "updated":1
      }
    },
    {
      "update":
      {
        "table":"test",
        "updated":1
      }
    }
  ],
  "errors":false
}
```


<!-- समाप्त -->



ध्यान रखें कि बल्क ऑपरेशन पहली क्वेरी पर रुक जाता है जो त्रुटि के साथ परिणाम देती है।

## अपडेट से संबंधित सेटिंग्स

#### attr_update_reserve

```ini
attr_update_reserve=size
```

<!-- attr_update_reserve का उदाहरण -->
`attr_update_reserve` एक टेबल-विशिष्ट सेटिंग है जो ब्लॉब विशेषता अपडेट के लिए आरक्षित स्थान को निर्धारित करती है। यह सेटिंग वैकल्पिक है, जिसका डिफ़ॉल्ट मान 128k है।

जब ब्लॉब विशेषताओं (MVAs, स्ट्रिंग्स, JSON) को अपडेट किया जाता है, तो उनकी लंबाई बदल सकती है। यदि अपडेट की गई स्ट्रिंग (या MVA, या JSON) पुरानी से छोटी है, तो यह `.spb` फ़ाइल में पुरानी को ओवरराइट कर देती है। हालांकि, यदि अपडेट की गई स्ट्रिंग लंबी है, तो अपडेट `.spb` फ़ाइल के अंत में लिखे जाते हैं। यह फ़ाइल मेमोरी-मैप्ड है, जिसका अर्थ है कि इसका आकार बदलना काफी धीमी प्रक्रिया हो सकती है, जो OS के मेमोरी-मैप्ड फ़ाइलों के कार्यान्वयन पर निर्भर करता है।

बार-बार आकार बदलने से बचने के लिए, आप इस विकल्प का उपयोग करके `.spb` फ़ाइल के अंत में अतिरिक्त स्थान आरक्षित कर सकते हैं।


<!-- परिचय -->
##### SQL:

<!-- अनुरोध SQL -->

```sql
create table products(title text, price float) attr_update_reserve = '1M'
```

<!-- अनुरोध JSON -->

```JSON
POST /cli -d "
create table products(title text, price float) attr_update_reserve = '1M'"
```

<!-- अनुरोध PHP -->

```php
$params = [
    'body' => [
        'settings' => [
            'attr_update_reserve' => '1M'
        ],
        'columns' => [
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ]
    ],
    'table' => 'products'
];
$index = new \Manticoresearch\Index($client);
$index->create($params);
```
<!-- परिचय -->
##### Python:

<!-- अनुरोध Python -->

```python
utilsApi.sql('create table products(title text, price float) attr_update_reserve = \'1M\'')
```
<!-- परिचय -->
##### Javascript:

<!-- अनुरोध javascript -->

```javascript
res = await utilsApi.sql('create table products(title text, price float) attr_update_reserve = \'1M\'');
```

<!-- परिचय -->
##### Java:
<!-- अनुरोध Java -->
```java
utilsApi.sql("create table products(title text, price float) attr_update_reserve = '1M'");
```

<!-- परिचय -->
##### C#:
<!-- अनुरोध C# -->
```clike
utilsApi.Sql("create table products(title text, price float) attr_update_reserve = '1M'");
```

<!-- परिचय -->
##### TypeScript:

<!-- अनुरोध TypeScript -->

```typescript
utilsApi.sql("create table test(content text, name string, cat int) attr_update_reserve = '1M'");
```

<!-- परिचय -->
##### Go:

<!-- अनुरोध Go -->

```go
apiClient.UtilsAPI.Sql(context.Background()).Body("create table test(content text, name string, cat int) attr_update_reserve = '1M'").Execute()
```

<!-- अनुरोध CONFIG -->

```ini
table products {
  attr_update_reserve = 1M
  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- समाप्त -->


#### attr_flush_period

```
attr_flush_period = 900 # हर 15 मिनट में अपडेट को डिस्क पर संरक्षित करें
```

विशेषताओं को अपडेट करते समय, परिवर्तन पहले विशेषताओं की इन-मेमोरी कॉपी में लिखे जाते हैं। यह सेटिंग अपडेट को डिस्क पर फ्लश करने के बीच के अंतराल को सेट करने की अनुमति देती है। यह डिफ़ॉल्ट रूप से 0 पर सेट है, जो आवधिक फ्लशिंग को अक्षम करता है, लेकिन फ़्लशिंग सामान्य शट-डाउन पर अभी भी होगी।

<!-- परीक्षण समाप्त -->






























































