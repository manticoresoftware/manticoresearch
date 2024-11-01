#  UPDATE

<!-- example update -->

`UPDATE` 命令用于修改指定表中现有文档的 [行式存储](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) 属性值。请注意，无法更新全文字段或列式属性的内容。如果有这样的需求，请使用 [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md) 命令。

属性更新支持 RT、PQ 和普通表。只要属性存储在 [行式存储](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) 中，所有类型的属性都可以更新。

需要注意的是，文档 ID 不能被更新。

还需要了解的是，更新属性会禁用其 [二级索引](../../Server_settings/Searchd.md#secondary_indexes)。如果保持二级索引的连续性至关重要，请考虑完全或[部分替换](../../Data_creation_and_modification/Updating_documents/REPLACE.md?client=REPLACE+SET) 文档。

更多关于 `UPDATE` 和部分 `REPLACE` 的区别可以参考 [这里](../../Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md#UPDATE-vs-partial-REPLACE)。

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
  "index":"products",
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
  "_index":"products",
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
indexApi.update({"index" : "products", "id" : 1, "doc" : {"price":10}})
```

<!-- response Python -->
```python
{'id': 1, 'index': 'products', 'result': 'updated', 'updated': None}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.update({"index" : "products", "id" : 1, "doc" : {"price":10}});
```

<!-- response javascript -->
```javascript
{"_index":"products","_id":1,"result":"updated"}
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
	"_index":"test",
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
	"_index":"test",
	"_id":1,
	"result":"updated"
}
```

<!-- end -->

<!-- example update multiple attributes -->

可以在单个语句中更新多个属性。例如：

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
  "index":"products",
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
  "_index":"products",
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
indexApi.update({"index" : "products", "id" : 1, "doc" : {
    "price": 100000000000,
    "coeff": 3465.23,
    "tags1": [3,6,4],
    "tags2": []}})
```

<!-- response Python -->
```python
{'id': 1, 'index': 'products', 'result': 'updated', 'updated': None}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.update({"index" : "products", "id" : 1, "doc" : {
    "price": 100000000000,
    "coeff": 3465.23,
    "tags1": [3,6,4],
    "tags2": []}});
```

<!-- response javascript -->
```javascript
{"_index":"products","_id":1,"result":"updated"}
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
  "_index":"test",
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
  "_index":"test",
  "_id":1,
  "result":"updated"
}
```

<!-- end -->

当为 32 位属性分配超出范围的值时，它们会被自动裁剪为其低 32 位，且不会有任何提示。例如，如果尝试将 32 位无符号整数更新为值 4294967297，实际上会存储值 1，因为 4294967297（十六进制表示为 0x100000001）的低 32 位为 1（十六进制表示为 0x00000001）。

<!-- example partial JSON update -->

`UPDATE` 还可以用于对数值数据类型或数值数据类型数组执行部分 JSON 更新。请确保不要用浮点值更新整数值，因为浮点值将被四舍五入。

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
	"index":"products",
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
	"index":"products",
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
   "_index":"products",
   "_id":100,
   "created":true,
   "result":"created",
   "status":201
}

{
  "_index":"products",
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
indexApi.update({"index" : "products", "id" : 1, "doc" : {
    "meta.tags[0]": 100}})
```

<!-- response Python -->
```python
{'id': 1, 'index': 'products', 'result': 'updated', 'updated': None}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.update({"index" : "products", "id" : 1, "doc" : {
   "meta.tags[0]": 100}});
```

<!-- response javascript -->
```javascript
{"_index":"products","_id":1,"result":"updated"}
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
res = await indexApi.update({"index" : "test", "id" : 1, "doc" : { "meta.tags[0]": 100} });
```

<!-- response TypeScript -->
```typescript
{"_index":"test","_id":1,"result":"updated"}
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
	"_index":"test",
	"_id":1,
	"result":"updated"
}
```

<!-- end -->

<!-- example full JSON update -->

要更新其他数据类型或更改 JSON 属性中的属性类型，则需要进行完整的 JSON 更新。

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
	"index":"products",
	"id":1,
	"doc":
	{
		"title":"title",
		"data":"{\"tags\":[1,2,3]}"
	}
}

POST /update
{
	"index":"products",
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
  "_index":"products",
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
indexApi.insert({"index" : "products", "id" : 100, "doc" : {"title" : "title", "meta" : {"tags":[1,2,3]}}})
indexApi.update({"index" : "products", "id" : 100, "doc" : {"meta" : {"tags":['one','two','three']}}})
```

<!-- response Python -->
```python

{'created': True,
 'found': None,
 'id': 100,
 'index': 'products',
 'result': 'created'}
{'id': 100, 'index': 'products', 'result': 'updated', 'updated': None}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.insert({"index" : "products", "id" : 100, "doc" : {"title" : "title", "meta" : {"tags":[1,2,3]}}});
res = await indexApi.update({"index" : "products", "id" : 100, "doc" : {"meta" : {"tags":['one','two','three']}}});
```

<!-- response javascript -->
```javascript
{"_index":"products","_id":100,"created":true,"result":"created"}
{"_index":"products","_id":100,"result":"updated"}

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
	"_index":"test",
	"_id":1,
	"created":true,
	"result":"created"
}

{
	"_index":"test",
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
	"_index":"test",
	"_id":1,
	"created":true,
	"result":"created"
}

{
	"_index":"test",
	"_id":1,
	"result":"updated"
}
```

<!-- end -->

<!-- example cluster update -->

在使用复制时，表名前应加上 `cluster_name:`（在 SQL 中），这样更新将被传播到集群中的所有节点。对于通过 HTTP 进行的查询，您应设置一个 `cluster` 属性。有关更多信息，请参阅[设置复制](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md)。

```json
{
  "cluster":"nodes4",
  "index":"test",
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
	"index":"products",
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
indexApi.update({"cluster":"weekly", "index" : "products", "id" : 1, "doc" : {"enabled" : 0}})

```
<!-- intro -->
##### Javascript:

<!-- request javascript -->
``` javascript
res = wait indexApi.update({"cluster":"weekly", "index" : "products", "id" : 1, "doc" : {"enabled" : 0}});

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


## 通过 SQL 进行更新

以下是 SQL `UPDATE` 语句的语法：

```sql
UPDATE table SET col1 = newval1 [, ...] WHERE where_condition [OPTION opt_name = opt_value [, ...]] [FORCE|IGNORE INDEX(id)]
```

`where_condition` 的语法与 [SELECT](../../Searching/Full_text_matching/Basic_usage.md#SQL) 语句中的相同。

<!-- example MVA empty update -->

多值属性的值集必须作为括号中的逗号分隔列表来指定。要从多值属性中删除所有值，只需将 `()` 赋给该属性即可。

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
	"index":"products",
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
  "_index":"products",
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

indexApi.update({"index" : "products", "id" : 1, "doc" : {"tags1": []}})
```

<!-- response Python -->
```python
{'id': 1, 'index': 'products', 'result': 'updated', 'updated': None}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->
``` javascript

indexApi.update({"index" : "products", "id" : 1, "doc" : {"tags1": []}})
```

<!-- response javascript -->
```javascript
{"_index":"products","_id":1,"result":"updated"}
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
	"_index":"test",
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
	"_index":"test",
	"_id":1,
	"result":"updated"
}
```

<!-- end -->

`OPTION` 子句是 Manticore 特有的扩展，它允许你控制许多与每次更新相关的选项。其语法为：

```sql
OPTION <optionname>=<value> [ , ... ]
```

这些选项与 [SELECT](../../Searching/Full_text_matching/Basic_usage.md#SQL) 语句中的相同。特别是对于 `UPDATE` 语句，可以使用以下选项：

- `ignore_nonexistent_columns` - 如果设置为 **1**，表示更新时将静默忽略关于尝试更新不存在于当前表模式中的列的任何警告。默认值为 **0**。
- `strict` - 该选项用于部分 JSON 属性更新。默认情况下（strict=1），如果 `UPDATE` 查询尝试更新非数字属性，将导致错误。设置 strict=0 时，如果多个属性更新且某些不允许更新，`UPDATE` 不会导致错误，并且只会对允许的属性进行更改（其余部分将被忽略）。如果 `SET` 中的所有更改均不允许，即使设置 strict=0，该命令也会导致错误。

### 查询优化器提示

在少数情况下，Manticore 内置的查询分析器可能无法正确理解查询并确定是否应使用按 ID 查询的表。这可能导致像 `UPDATE ... WHERE id = 123` 这样的查询性能较差。 有关如何强制优化器使用 `docid` 索引的信息，请参见 [查询优化器提示](../../Searching/Options.md#Query optimizer hints)。

## 通过 HTTP JSON 进行更新

使用 HTTP JSON 协议进行更新是通过 `/update` 端点完成的。其语法与 [/insert 端点](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md) 类似，但这次 `doc` 属性是必需的。

服务器将以 JSON 对象响应，表明操作是否成功。

<!-- example JSON update -->

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /update
{
  "index":"test",
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
  "_index": "test",
  "_id": 1,
  "result": "updated"
}
```

<!-- end -->

<!-- example JSON Example_2 -->

需要更新的文档 ID 可以直接使用 `id` 属性进行设置，如前面的示例所示，或者你也可以通过查询更新文档，并将更新应用于所有匹配该查询的文档：

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /update

{
  "index":"test",
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
  "_index":"products",
  "updated":1
}
```

<!-- end -->

查询语法与 [/search 端点](../../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) 中相同。注意，你不能同时指定 `id` 和 `query`。

## 刷新属性

```sql
FLUSH ATTRIBUTES
```

`FLUSH ATTRIBUTES` 命令确保所有活跃表中的内存属性更新被刷新到磁盘。它返回一个标识结果磁盘状态的标签，表示自服务器启动以来实际执行的磁盘属性保存次数。

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
另请参阅 [attr_flush_period](../../Data_creation_and_modification/Updating_documents/UPDATE.md#attr_flush_period) 设置。


## 批量更新

<!-- example bulk update -->

你可以使用 `/bulk` 端点在一次调用中执行多个更新操作。该端点仅适用于 `Content-Type` 设置为 `application/x-ndjson` 的数据。数据应以换行符分隔的 JSON (NDJSON) 格式编写。也就是说，每行应该包含一个完整的 JSON 语句并以换行符 `\n`（可能还包括 `\r`）结尾。


<!-- intro -->
##### JSON:

<!-- request JSON --Content-type=application/x-ndjson -->

```json
POST /bulk

{ "update" : { "index" : "products", "id" : 1, "doc": { "price" : 10 } } }
{ "update" : { "index" : "products", "id" : 2, "doc": { "price" : 20 } } }
```

<!-- response JSON -->

```json
{
   "items":
   [
      {
         "update":
         {
            "_index":"products",
            "_id":1,
            "result":"updated"
         }
      },
      {
         "update":
         {
            "_index":"products",
            "_id":2,
            "result":"updated"
         }
      }
   ],
   "errors":false
}
```

<!-- end -->

`/bulk` 端点支持插入、替换和删除操作。每条语句都以一个操作类型开头（在这种情况下是 `update`）。以下是支持的操作列表：

- `insert`：插入文档。其语法与 [/insert 端点](../../Quick_start_guide.md#Add-documents) 相同。
- `create`：`insert` 的同义词。
- `replace`：替换文档。其语法与 [/replace](../../Data_creation_and_modification/Updating_documents/REPLACE.md) 相同。
- `index`：`replace` 的同义词。
- `update`：更新文档。其语法与 [/update](../../Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-HTTP-JSON) 相同。
- `delete`：删除文档。其语法与 [/delete 端点](../../Data_creation_and_modification/Deleting_documents.md) 相同。

同时支持按查询更新和按查询删除。

<!-- example bulk by query -->

<!-- intro -->
##### JSON:

<!-- request JSON --Content-type=application/x-ndjson -->

```json
POST /bulk

{ "update" : { "index" : "products", "doc": { "coeff" : 1000 }, "query": { "range": { "price": { "gte": 1000 } } } } }
{ "update" : { "index" : "products", "doc": { "coeff" : 0 }, "query": { "range": { "price": { "lt": 1000 } } } } }
```

<!-- response JSON -->

```json
{
  "items":
  [
    {
      "update":
      {
        "_index":"products",
        "updated":1
      }
    },
    {
      "update":
      {
        "_index":"products",
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
            'index' => 'products',
             'doc' => [
                'coeff' => 100
            ],
            'query' => [
                'range' => ['price'=>['gte'=>1000]]
            ]   
        ]
    ],
    ['update'=>[
            'index' => 'products',
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
            { "update" : { "index" : "products", "doc": { "coeff" : 1000 }, "query": { "range": { "price": { "gte": 1000 } } } } }, \
            { "update" : { "index" : "products", "doc": { "coeff" : 0 }, "query": { "range": { "price": { "lt": 1000 } } } } } ]
indexApi.bulk('\n'.join(map(json.dumps,docs)))
```

<!-- response Python -->
```python
{'error': None,
 'items': [{u'update': {u'_index': u'products', u'updated': 1}},
           {u'update': {u'_index': u'products', u'updated': 3}}]}

```
<!-- intro -->
##### Javascript:

<!-- request javascript -->
``` javascript
docs = [
            { "update" : { "index" : "products", "doc": { "coeff" : 1000 }, "query": { "range": { "price": { "gte": 1000 } } } } },
            { "update" : { "index" : "products", "doc": { "coeff" : 0 }, "query": { "range": { "price": { "lt": 1000 } } } } } ];
res =  await indexApi.bulk(docs.map(e=>JSON.stringify(e)).join('\n'));
```

<!-- response javascript -->
```javascript
{"items":[{"update":{"_index":"products","updated":1}},{"update":{"_index":"products","updated":3}}],"errors":false}

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
        "_index":"test",
        "updated":1
      }
    },
    {
      "update":
      {
        "_index":"test",
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
        "_index":"test",
        "updated":1
      }
    },
    {
      "update":
      {
        "_index":"test",
        "updated":1
      }
    }
  ],
  "errors":false
}
```


<!-- end -->



请注意，批量操作在遇到第一个导致错误的查询时会停止执行。

## 与更新相关的设置

#### attr_update_reserve

```ini
attr_update_reserve=size
```

<!-- example attr_update_reserve -->

`attr_update_reserve` 是一个针对每个表的设置，用于确定为 blob 属性更新保留的空间。此设置是可选的，默认值为 128k。

当 blob 属性（例如 MVA、多值属性，字符串，JSON）更新时，其长度可能发生变化。如果更新后的字符串（或 MVA、JSON）比旧的短，则会覆盖 `.spb` 文件中的旧内容。然而，如果更新后的字符串更长，更新内容将写入 `.spb` 文件的末尾。这个文件是内存映射的，这意味着根据操作系统对内存映射文件的实现，重新调整其大小可能是一个相对较慢的过程。

为了避免频繁调整大小，可以使用此选项指定在 `.spb` 文件末尾保留的额外空间。

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
    'index' => 'products'
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

当更新属性时，首先将更改写入属性的内存副本中。此设置允许设置将更新刷新到磁盘的间隔时间。默认值为 0，表示禁用定期刷新，但在正常关闭时仍会执行刷新操作。

<!-- proofread -->
