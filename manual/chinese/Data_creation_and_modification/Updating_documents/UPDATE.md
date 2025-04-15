# 更新


<!-- example update -->


`UPDATE` 命令将指定表中现有文档的[行式存储](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)属性值更新为新值。请注意，您无法更新全文本字段或列式属性的内容。如果有此需求，请使用 [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md).


属性更新支持 RT、PQ 以及普通表。只要属性存储在[行式存储](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)中，所有属性类型都可以更新。


请注意，文档 ID 不能更新。


需要特别注意的是，更新属性会禁用其[辅助索引](../../Server_settings/Searchd.md#secondary_indexes)。如果保持辅助索引的连续性至关重要，建议完全或[部分替换](../../Data_creation_and_modification/Updating_documents/REPLACE.md?client=REPLACE+SET)文档。


关于 `UPDATE` 与部分 `REPLACE` 的更多信息，请阅读[此处](../../Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md#UPDATE-vs-partial-REPLACE).


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

可以在单个语句中更新多个属性。示例：

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

当为32位属性分配超出范围的值时，这些值将被修剪为其低32位，且不会发出提示。例如，如果您尝试用4294967297的值更新32位无符号整数，实际存储的值将是1，因为4294967297的低32位（16进制为0x100000001）等于1（16进制为0x00000001）。

<!-- example partial JSON update -->

`UPDATE`可用于对数值数据类型或数值数据类型数组执行部分JSON更新。只需确保不要用浮点值更新整数值，因为它将被四舍五入。

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

更新其他数据类型或在JSON属性中更改属性类型需要进行完整的JSON更新。

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
insertDoc := map[string]interface{} {"name": "文档 1", "meta": metaField}}
insertRequest := manticoreclient.NewInsertDocumentRequest("test", insertDoc)
insertRequest.SetId(1)
res, _, _ := apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*insertRequest).Execute();

metaField = map[string]interface{} {"tags": []string{"一", "二", "三"}}
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
	"result":"创建"
}

{
	"table":"test",
	"_id":1,
	"result":"更新"
}
```

<!-- end -->

<!-- example cluster update -->

当使用复制时，表名应以 `cluster_name:` 开头（在 SQL 中），以便更新将传播到集群中的所有节点。对于通过 HTTP 的查询，您应设置 `cluster` 属性。有关更多信息，请参见 [设置复制](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md)。

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
res = wait indexApi.update( {cluster: 'test_cluster', index : 'test', id : 1, doc : {name : '文档 11'}} );
```

<!-- intro -->
##### Go:

<!-- request Go -->
``` go
updateDoc = map[string]interface{} {"name":"文档 11"}
updateRequest = manticoreclient.NewUpdateDocumentRequest("test", updateDoc)
updateRequest.SetCluster("test_cluster")
updateRequest.SetId(1)
res, _, _ = apiClient.IndexAPI.Update(context.Background()).UpdateDocumentRequest(*updateRequest).Execute()
```

<!-- end -->


## 通过 SQL 更新

这是 SQL `UPDATE` 语句的语法：

```sql
UPDATE table SET col1 = newval1 [, ...] WHERE where_condition [OPTION opt_name = opt_value [, ...]] [FORCE|IGNORE INDEX(id)]
```


`where_condition` 的语法与 [SELECT](../../Searching/Full_text_matching/Basic_usage.md#SQL) 语句中的相同。

<!-- example MVA empty update -->

多值属性值集必须指定为用逗号分隔的列表，并用括号括起来。要从多值属性中移除所有值，只需将 `()` 赋值给它。

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
    result: 更新
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

*   'ignore_nonexistent_columns' - 如果设置为 **1**，则表示更新将静默忽略任何关于尝试更新当前表模式中不存在的列的警告。默认值为 **0**。
*   'strict' - 此选项用于部分 JSON 属性更新。默认情况下（strict=1），如果 `UPDATE` 查询尝试对非数字属性进行更新，则会导致错误。使用 strict=0，如果更新了多个属性而某些属性不被允许，则 `UPDATE` 不会导致错误，并且将仅对被允许的属性进行更改（其余属性将被忽略）。如果 `UPDATE` 的 `SET` 更改均不被允许，即使 strict=0，该命令也会导致错误。

### Query optimizer hints

在极少数情况下，Manticore 的内置查询分析器可能无法正确理解查询，并确定是否应通过 ID 使用表。这可能导致查询的性能降低，例如 `UPDATE ... WHERE id = 123`。
有关如何强制优化器使用 docid 索引的信息，请参见 [Query optimizer hints](../../Searching/Options.md#Query optimizer hints)。

## Updates via HTTP JSON

使用 HTTP JSON 协议的更新通过 `/update` 端点执行。语法与 [/insert endpoint](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md) 相似，但这次 `doc` 属性是强制性的。

服务器将响应一个 JSON 对象，说明操作是否成功。

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

需要更新的文档 ID 可以使用 `id` 属性直接设置，如前面示例所示，或者可以通过查询更新文档，并将更新应用于所有与查询匹配的文档：

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

查询语法与 [/search endpoint](../../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) 中相同。请注意，不能同时指定 `id` 和 `query`。

## Flushing attributes

```sql
FLUSH ATTRIBUTES
```

FLUSH ATTRIBUTES 命令确保所有活动表中的内存属性更新已被写入磁盘。它返回一个标记，标识服务器启动以来在磁盘上执行的实际属性保存数量。

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


## Bulk updates

<!-- example bulk update -->

您可以通过 `/bulk` 端点在一次调用中执行多个更新操作。此端点仅适用于 `Content-Type` 设置为 `application/x-ndjson` 的数据。数据应格式化为换行分隔的 JSON（NDJSON）。基本上，这意味着每一行应仅包含一个 JSON 语句，并以换行符 `\n` 和可能的 `\r` 结尾。


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
            "result":"更新"
         }
      }
   ],
   "errors":false
}
```

<!-- end -->

`/bulk` 端点支持插入、替换和删除。 每个语句以动作类型开头（在这种情况下为 `update`）。 这是支持的动作列表：

* `insert`：插入文档。 语法与[/insert endpoint](../../Quick_start_guide.md#Add-documents)相同。
* `create`：`insert` 的同义词
* `replace`：替换文档。 语法与 [/replace](../../Data_creation_and_modification/Updating_documents/REPLACE.md)相同。
* `index`：`replace` 的同义词
* `update`：更新文档。 语法与 [/update](../../Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-HTTP-JSON)相同。
* `delete`：删除文档。 语法与 [/delete endpoint](../../Data_creation_and_modification/Deleting_documents.md)相同。

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
    items: [{update={_index=products, _id=1, created=false, result=更新, status=200}}, {update={_index=products, _id=2, created=false, result=更新, status=200}}]
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
    items: [{update={_index=products, _id=1, created=false, result=更新, status=200}}, {update={_index=products, _id=2, created=false, result=更新, status=200}}]
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
      doc: { content: '文本 11', cat: 1, name: '文档 11' },
    },
  },
  {
    update: {
      index: 'test',
      id: 2,
      doc: { content: '文本 22', cat: 9, name: '文档 22' },
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



请记住，批量操作在第一个出错的查询处停止。

## 与更新相关的设置

#### attr_update_reserve

```ini
attr_update_reserve=size
```

<!-- example attr_update_reserve -->
`attr_update_reserve` 是一个针对每个表的设置，用于确定为 blob 属性更新预留的空间。这是一个可选设置，默认值为 128k。

当更新 blob 属性（MVA、字符串、JSON）时，它们的长度可能会发生变化。如果更新后的字符串（或 MVA、或 JSON）比旧的短，它会在 `.spb` 文件中覆盖旧的。但是，如果更新后的字符串更长，则更新将写入 `.spb` 文件的末尾。由于该文件是内存映射的，调整其大小可能会相当慢，具体取决于操作系统对内存映射文件的实现。

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

