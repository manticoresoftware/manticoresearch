# UPDATE

<!-- example update -->

The `UPDATE` command changes [row-wise](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) attribute values of existing documents in a specified table with new values. Note that you can't update the contents of a fulltext field or a columnar attribute. If there's such a need, use [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md).

Attribute updates are supported for RT, PQ, and plain tables. All attribute types can be updated as long as they are stored in the [row-wise storage](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages).

Note that the document ID cannot be updated.

It's important to be aware that updating an attribute disables its [secondary index](../../Server_settings/Searchd.md#secondary_indexes). If maintaining secondary index continuity is critical, consider fully or [partially replacing](../../Data_creation_and_modification/Updating_documents/REPLACE.md?client=REPLACE+SET) the document instead.

Read more about `UPDATE` vs. partial `REPLACE` [here](../../Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md#UPDATE-vs-partial-REPLACE).

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

Updating other data types or changing property type in a JSON attribute requires a full JSON update.

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

When using replication, the table name should be prepended with `cluster_name:` (in SQL) so that updates will be propagated to all nodes in the cluster. For queries via HTTP, you should set a `cluster` property. See [setting up replication](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md) for more information.

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


## Updates via SQL

Here is the syntax for the SQL `UPDATE` statement:

```sql
UPDATE table SET col1 = newval1 [, ...] WHERE where_condition [OPTION opt_name = opt_value [, ...]] [FORCE|IGNORE INDEX(id)]
```


`where_condition` has the same syntax as in the [SELECT](../../Searching/Full_text_matching/Basic_usage.md#SQL) statement.

<!-- example MVA empty update -->

Multi-value attribute value sets must be specified as comma-separated lists in parentheses. To remove all values from a multi-value attribute, just assign `()` to it.

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


`OPTION` clause is a Manticore-specific extension that lets you control a number of per-update options. The syntax is:

```sql
OPTION <optionname>=<value> [ , ... ]
```

The options are the same as for the [SELECT](../../Searching/Full_text_matching/Basic_usage.md#SQL) statement. Specifically for the `UPDATE` statement, you can use these options:

*   'ignore_nonexistent_columns' - If set to **1**, it indicates that the update will silently ignore any warnings about trying to update a column which does not exist in the current table schema. The default value is **0**.
*   'strict' - This option is used in partial JSON attribute updates. By default (strict=1), `UPDATE` will result in an error if the `UPDATE` query tries to perform an update on non-numeric properties. With strict=0, if multiple properties are updated and some are not allowed, the `UPDATE` will not result in an error and will perform the changes only on allowed properties (with the rest being ignored). If none of the `SET` changes of the `UPDATE` re permitted, the command will result in an error even with strict=0.

### Query optimizer hints

In rare cases, Manticore's built-in query analyzer may be incorrect in understanding a query and determining whether a table by ID should be used. This can result in poor performance for queries like `UPDATE ... WHERE id = 123`.
For information on how to force the optimizer to use a docid index, see [Query optimizer hints](../../Searching/Options.md#Query optimizer hints).

## Updates via HTTP JSON

Updates using HTTP JSON protocol are performed via the `/update` endpoint. The syntax is similar to the [/insert endpoint](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md), but this time the `doc` property is mandatory.

The server will respond with a JSON object stating if the operation was successful or not.

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

The ID of the document that needs to be updated can be set directly using the `id` property, as shown in the previous example, or you can update documents by query and apply the update to all the documents that match the query:

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

The query syntax is the same as in the [/search endpoint](../../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON). Note that you can't specify `id` and `query` at the same time.

## Flushing attributes

```sql
FLUSH ATTRIBUTES
```

The FLUSH ATTRIBUTES command ensures that all in-memory attribute updates in all active tables are flushed to disk. It returns a tag that identifies the result on-disk state, which represents the number of actual disk attribute saves performed since the server startup.

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
See also [attr_flush_period](../../Data_creation_and_modification/Updating_documents/UPDATE.md#attr_flush_period) setting.


## Bulk updates

<!-- example bulk update -->

You can perform multiple update operations in a single call using the `/bulk` endpoint. This endpoint only works with data that has `Content-Type` set to `application/x-ndjson`. The data should be formatted as newline-delimited JSON (NDJSON). Essentially, this means that each line should contain exactly one JSON statement and end with a newline `\n` and, possibly, a `\r`.


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

The `/bulk` endpoint supports inserts, replaces, and deletes. Each statement begins with an action type (in this case, `update`). Here's a list of the supported actions:

* `insert`: Inserts a document. The syntax is the same as in the [/insert endpoint](../../Quick_start_guide.md#Add-documents).
* `create`: a synonym for `insert`
* `replace`: Replaces a document. The syntax is the same as in the [/replace](../../Data_creation_and_modification/Updating_documents/REPLACE.md).
* `index`: a synonym for `replace`
* `update`: Updates a document. The syntax is the same as in the [/update](../../Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-HTTP-JSON).
* `delete`: Deletes a document. The syntax is the same as in the [/delete endpoint](../../Data_creation_and_modification/Deleting_documents.md).

Updates by query and deletes by query are also supported.

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



Keep in mind that the bulk operation stops at the first query that results in an error.

## Settings related with updates

#### attr_update_reserve

```ini
attr_update_reserve=size
```

<!-- example attr_update_reserve -->
`attr_update_reserve` is a per-table setting that determines the space reserved for blob attribute updates. This setting is optional, with a default value of 128k.

When blob attributes (MVAs, strings, JSON) are updated, their length may change. If the updated string (or MVA, or JSON) is shorter than the old one, it overwrites the old one in the `.spb` file. However, if the updated string is longer, updates are written to the end of the `.spb` file. This file is memory-mapped, which means resizing it may be a rather slow process, depending on the OS implementation of memory-mapped files.

To avoid frequent resizes, you can specify the extra space to be reserved at the end of the `.spb` file using this option.


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
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('create table products(title text, price float) attr_update_reserve = \'1M\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("create table products(title text, price float) attr_update_reserve = '1M'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("create table products(title text, price float) attr_update_reserve = '1M'");
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

When updating attributes the changes are first written to in-memory copy of attributes. This setting allows to set the interval between flushing the updates to disk. It defaults to 0, which disables the periodic flushing, but flushing will still occur at normal shut-down.

<!-- proofread -->
