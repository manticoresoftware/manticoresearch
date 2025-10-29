# 更新

<!-- example update -->

`UPDATE` 命令用于将指定表中现有文档的[逐行](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)属性值更改为新值。请注意，您不能更新全文字段或列存属性的内容。如果有此类需求，请使用 [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md)。

属性更新支持 RT、PQ 和普通表。只要属性存储在[逐行存储](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)中，所有属性类型都可以被更新。

请注意，文档 ID 不能被更新。

需要注意的是，更新属性会禁用其[二级索引](../../Server_settings/Searchd.md#secondary_indexes)。如果保持二级索引的连续性非常重要，建议使用完全或[部分替换](../../Data_creation_and_modification/Updating_documents/REPLACE.md?client=REPLACE+SET)文档。

关于 `UPDATE` 与部分 `REPLACE` 的更多内容，请参阅[这里](../../Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md#UPDATE-vs-partial-REPLACE)。

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

可以在一个语句中更新多个属性。示例：

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

在为32位属性赋予超出范围的值时，它们将被截断为低32位且不会提示。例如，如果您尝试将32位无符号整数更新为4294967297的值，实际存储的值为1，因为4294967297（十六进制为0x100000001）的低32位是1（十六进制0x00000001）。

<!-- example partial JSON update -->

`UPDATE` 可用于对数字数据类型或数字数据类型数组的 JSON 进行部分更新。只要确保不要用浮点数更新整数值，因为浮点数会被四舍五入。

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
##### Python-asyncio:

<!-- request Python-asyncio -->
``` python
indexApi = api = manticoresearch.IndexApi(client)
await indexApi.update({"table" : "products", "id" : 1, "doc" : {
    "meta.tags[0]": 100}})
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

更新其他数据类型或更改 JSON 属性中的属性类型需要进行完整的 JSON 更新。

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

当使用复制时，表名应该加上前缀 `cluster_name:`（在 SQL 中），这样更新才会传播到集群中的所有节点。对于通过 HTTP 的查询，应设置 `cluster` 属性。详见[设置复制](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md)获取更多信息。

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


## 通过 SQL 更新

下面是 SQL `UPDATE` 语句的语法：

```sql
UPDATE table SET col1 = newval1 [, ...] WHERE where_condition [OPTION opt_name = opt_value [, ...]] [FORCE|IGNORE INDEX(id)]
```


`where_condition` 的语法与 [SELECT](../../Searching/Full_text_matching/Basic_usage.md#SQL) 语句相同。

<!-- example MVA empty update -->

多值属性的值集合必须以逗号分隔的列表形式，括在括号内。要移除多值属性的所有值，只需赋值为 `()` 即可。

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


`OPTION` 子句是 Manticore 特有的扩展，允许你控制若干更新时的选项。其语法为：

```sql
OPTION <optionname>=<value> [ , ... ]
```

这些选项与 [SELECT](../../Searching/Full_text_matching/Basic_usage.md#SQL) 语句的选项相同。针对 `UPDATE` 语句，你可以使用以下选项：

*   'ignore_nonexistent_columns' - 如果设置为 **1**，表示更新时会静默忽略任何尝试更新当前表模式中不存在列的警告。默认值为 **0**。
*   'strict' - 该选项用于部分 JSON 属性的更新。默认情况下（strict=1），如果 `UPDATE` 查询尝试更新非数字属性，则会返回错误。设置 strict=0 时，如果多个属性被更新，其中一些不允许更新，则 `UPDATE` 不会返回错误，仅对允许的属性执行更改（其余忽略）。如果 `UPDATE` 的所有 `SET` 变更均不被允许，即使 strict=0，该命令亦会返回错误。

### 查询优化器提示

在极少数情况下，Manticore 内置的查询分析器可能无法正确理解查询，也无法判断是否应使用基于 ID 的表索引。这可能导致诸如 `UPDATE ... WHERE id = 123` 这类查询性能下降。
有关如何强制优化器使用 docid 索引的信息，请参见[查询优化器提示](../../Searching/Options.md#Query optimizer hints)。

## 通过 HTTP JSON 进行更新

通过 HTTP JSON 协议进行更新时，使用 `/update` 端点。语法类似于[/insert 端点](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md)，但此时 `doc` 属性是必需的。

服务器将返回一个 JSON 对象，说明操作是否成功。

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

需要更新的文档 ID 可以直接通过 `id` 属性设置，如上例所示；或者你可以通过查询更新文档，并将更新应用于所有符合查询条件的文档：

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

查询语法与[/search endpoint](../../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON)中的相同。请注意，不能同时指定 `id` 和 `query`。

## 刷新属性

```sql
FLUSH ATTRIBUTES
```

FLUSH ATTRIBUTES 命令确保所有活动表中内存中的属性更新都被刷新到磁盘。它返回一个标签，用于标识结果的磁盘状态，该状态表示自服务器启动以来实际执行的磁盘属性保存次数。

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
另请参见 [attr_flush_period](../../Data_creation_and_modification/Updating_documents/UPDATE.md#attr_flush_period) 设置。


## 批量更新

<!-- example bulk update -->

您可以使用 `/bulk` 端点在单个调用中执行多个更新操作。此端点仅支持 `Content-Type` 设置为 `application/x-ndjson` 的数据。数据应采用换行分隔的 JSON（NDJSON）格式。基本上，这意味着每行应包含恰好一个 JSON 语句，并以换行符 `\n` 以及可能的 `\r` 结束。


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

`/bulk` 端点支持插入、替换和删除。每条语句以操作类型开头（此处为 `update`）。以下是支持的操作列表：

* `insert`：插入文档。语法与[/insert endpoint](../../Quick_start_guide.md#Add-documents)相同。
* `create`：`insert` 的同义词。
* `replace`：替换文档。语法与[/replace](../../Data_creation_and_modification/Updating_documents/REPLACE.md)相同。
* `index`：`replace` 的同义词。
* `update`：更新文档。语法与[/update](../../Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-HTTP-JSON)相同。
* `delete`：删除文档。语法与[/delete endpoint](../../Data_creation_and_modification/Deleting_documents.md)相同。

也支持按查询更新和按查询删除。

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
##### Python-asyncio:

<!-- request Python-asyncio -->
``` python
docs = [ \
            { "update" : { "table" : "products", "doc": { "coeff" : 1000 }, "query": { "range": { "price": { "gte": 1000 } } } } }, \
            { "update" : { "table" : "products", "doc": { "coeff" : 0 }, "query": { "range": { "price": { "lt": 1000 } } } } } ]
await indexApi.bulk('\n'.join(map(json.dumps,docs)))
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

<!-- intro -->
##### Rust:

<!-- request Rust -->
``` rust
string  body = r#"{ "update" : { "index" : "products", "doc": { "coeff" : 1000 }, "query": { "range": { "price": { "gte": 1000 } } } }}
    { "update" : { "index" : "products", "doc": { "coeff" : 0 }, "query": { "range": { "price": { "lt": 1000 } } } } }
"#;
index_api.bulk(body).await;
```

<!-- response Rust -->
```rust
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

<!-- response TypeScript -->
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

<!-- request Go -->

``` go
body := "{\"update\": {\"index\": \"test\", \"id\": 1, \"doc\": {\"content\": \"Text 11\", \"name\": \"Doc 11\", \"cat\": 1 }}}" + "\n" +
	"{\"update\": {\"index\": \"test\", \"id\": 2, \"doc\": {\"content\": \"Text 22\", \"name\": \"Doc 22\", \"cat\": 9 }}}" +"\n";
res, _, _ := apiClient.IndexAPI.Bulk(context.Background()).Body(body).Execute()
```

<!-- response Go -->
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


<!-- end -->



请注意，批量操作在遇到第一个错误的查询时即停止。

## 与更新相关的设置

#### attr_update_reserve

```ini
attr_update_reserve=size
```

<!-- example attr_update_reserve -->
`attr_update_reserve` 是一个针对每个表的设置，用于确定为 blob 属性更新预留的空间。此设置为可选，默认值为 128k。

当 blob 属性（多值属性、多字符串、JSON）被更新时，其长度可能会变化。如果更新后的字符串（或多值属性，或 JSON）比旧字符串短，则在 `.spb` 文件中直接覆盖旧内容。然而，如果更新后的字符串更长，更新内容会写入 `.spb` 文件末尾。该文件是内存映射的，这意味着根据操作系统对内存映射文件的实现，调整其大小可能是一个相对缓慢的过程。

为了避免频繁调整大小，您可以使用此选项指定在 `.spb` 文件末尾预留的额外空间。


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
create table products(title text, price float) attr_update_reserve = '1M'
```

<!-- request JSON -->

```JSON
POST /cli -d "
create table products(title text, price float) attr_update_reserve = '1M'"
```

<!-- request PHP -->

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
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('create table products(title text, price float) attr_update_reserve = \'1M\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('create table products(title text, price float) attr_update_reserve = \'1M\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('create table products(title text, price float) attr_update_reserve = \'1M\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("create table products(title text, price float) attr_update_reserve = '1M'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("create table products(title text, price float) attr_update_reserve = '1M'", true);
```

<!-- intro -->
##### Rust:
<!-- request Rust -->
```rust
utils_api.sql("create table products(title text, price float) attr_update_reserve = '1M'", Some(true)).await;
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->

```typescript
utilsApi.sql("create table test(content text, name string, cat int) attr_update_reserve = '1M'");
```

<!-- intro -->
##### Go:

<!-- request Go -->

```go
apiClient.UtilsAPI.Sql(context.Background()).Body("create table test(content text, name string, cat int) attr_update_reserve = '1M'").Execute()
```

<!-- request CONFIG -->

```ini
table products {
  attr_update_reserve = 1M
  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->


#### attr_flush_period

```
attr_flush_period = 900 # persist updates to disk every 15 minutes
```

更新属性时，变更首先写入属性的内存副本中。此设置允许设置刷新更新到磁盘的时间间隔。默认值为 0，表示禁用周期性刷新，但在正常关闭时仍会进行刷新。

<!-- proofread -->

