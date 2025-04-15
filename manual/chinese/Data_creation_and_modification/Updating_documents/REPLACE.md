# 替换

<!-- example replace -->

`REPLACE` 的工作方式类似于 [INSERT](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md)，但它会在插入新文档之前，将具有相同 ID 的先前文档标记为已删除。

如果您在寻找原地更新的信息，请参见 [本节](../../Data_creation_and_modification/Updating_documents/UPDATE.md)。

## SQL 替换

SQL `REPLACE` 语句的语法如下：

**替换整个文档：**
```sql
REPLACE INTO table [(column1, column2, ...)]
    VALUES (value1, value2, ...)
    [, (...)]
```

**仅替换选定字段：**
```sql
REPLACE INTO table
    SET field1=value1[, ..., fieldN=valueN]
    WHERE id = <id>
```
注意，在这种模式下，您只能按 ID 进行过滤。

> 注意：部分替换需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保已安装 Buddy。

了解更多关于 `UPDATE` 与部分 `REPLACE` 的信息 [这里](../../Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md#UPDATE-vs-partial-REPLACE)。

查看示例以获取更多详细信息。

## JSON 替换

* `/replace`：
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
  `/index` 是一个别名端点，功能相同。
* Elasticsearch-like 端点 `<table>/_doc/<id>`：
  ```
  PUT/POST /<table name>/_doc/<id>
  {
    "<field1>": <value1>,
    ...
    "<fieldN>": <valueN>
  }
  ```
  > 注意：Elasticsearch-like 替换需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保已安装 Buddy。
* 部分替换：
  ```
  POST /<{table | cluster:table}>/_update/<id>
  {
    "<field1>": <value1>,
    ...
    "<fieldN>": <valueN>
  }
  ```
  `<table name>` 可以是仅表名或格式为 `cluster:table`。如果需要，这允许跨特定集群进行更新。

  > 注意：部分替换需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保已安装 Buddy。

查看示例以获取更多详细信息。

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
REPLACE INTO products VALUES(1, "document one", 10);
```

<!-- response SQL -->

```sql
Query OK, 1 row affected (0.00 sec)
```

<!-- intro -->
##### 替换 ... 设置：
<!-- request REPLACE SET -->

```sql
REPLACE INTO products SET description='HUAWEI Matebook 15', price=10 WHERE id = 55;
```

<!-- response REPLACE ... SET -->

```sql
Query OK, 1 row affected (0.00 sec)
```

<!-- intro -->
##### JSON

<!-- request JSON -->

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

<!-- response JSON -->
```json
{
  "table":"products",
  "_id":1,
  "created":false,
  "result":"updated",
  "status":200
}

```

<!-- intro -->
##### Elasticsearch-like

<!-- request Elasticsearch-like -->

> 注意：Elasticsearch-like 替换需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保已安装 Buddy。

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

<!-- response Elasticsearch-like -->
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

<!-- intro -->
##### Elasticsearch-like 部分替换：

<!-- request Elasticsearch-like partial -->

> 注意：部分替换需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保已安装 Buddy。

```json
POST /products/_update/55
{
  "doc": {
    "description": "HUAWEI Matebook 15",
    "price": 10
  }
}
```

<!-- response Elasticsearch-like partial -->
```json
{
"table":"products",
"updated":1
}
```

<!-- intro -->
##### Elasticsearch-like 部分替换在集群中：

<!-- request Elasticsearch-like partial in cluster -->

```json
POST /cluster_name:products/_update/55
{
  "doc": {
    "description": "HUAWEI Matebook 15",
    "price": 10
  }
}
```

<!-- response Elasticsearch-like partial in cluster -->
```json
{
"table":"products",
"updated":1
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->replaceDocument([
   'title' => 'document one',
    'price' => 10
],1);
```

<!-- response PHP -->
```php
Array(
    [_index] => products
    [_id] => 1
    [created] => false
    [result] => updated
    [status] => 200
)
```
<!-- intro -->

##### Python:

<!-- request Python -->
``` python
indexApi.replace({"table" : "products", "id" : 1, "doc" : {"title" : "document one","price":10}})
```

<!-- response Python -->
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

`REPLACE` 是可用于实时和穿透表的。您不能在普通表中替换数据。

当您运行 `REPLACE` 时，前一个文档不会被删除，而是被标记为已删除，因此表的大小会增加，直到发生块合并。要强制进行块合并，请使用 [OPTIMIZE 语句](../../Securing_and_compacting_a_table/Compacting_a_table.md)。

## 批量替换

<!-- example bulk_replace -->

您可以一次替换多个文档。有关更多信息，请查看 [批量添加文档](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-adding-documents)。

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

