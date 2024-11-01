# REPLACE

<!-- example replace -->

`REPLACE` 的工作方式类似于 [INSERT](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md)，但它会在插入新文档之前将具有相同ID的先前文档标记为已删除。

如果您想进行原地更新，请参阅 [此部分](../../Data_creation_and_modification/Updating_documents/UPDATE.md)。

## SQL REPLACE

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
请注意，在此模式下只能通过 id 进行过滤。

> 注意：部分替换需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保 Buddy 已安装。

有关 `UPDATE` 与部分 `REPLACE` 的更多信息，请阅读 [此处](../../Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md#UPDATE-vs-partial-REPLACE)。

请参阅示例以获取更多详细信息。

## JSON REPLACE

* `/replace`:
  ```
  POST /replace
  {
    "index": "<table name>",
    "id": <document id>,
    "doc":
    {
      "<field1>": <value1>,
      ...
      "<fieldN>": <valueN>
    }
  }
  ```
  `/index` 是一个别名端点，工作方式相同。
* 类似 Elasticsearch 的端点 `<table>/_doc/<id>`:
  ```
  PUT/POST /<table name>/_doc/<id>
  {
    "<field1>": <value1>,
    ...
    "<fieldN>": <valueN>
  }
  ```
  > NOTE: Elasticsearch-like replace requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.
* 部分替换：
  ```
  POST /<{table | cluster:table}>/_update/<id>
  {
    "<field1>": <value1>,
    ...
    "<fieldN>": <valueN>
  }
  ```
   `<table name>`  可以是表名，也可以是 `cluster:table` 格式。如果需要，可以进行跨特定集群的更新。

  > 注意：部分替换需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保 Buddy 已安装。

请参阅示例以获取更多详细信息。

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
##### REPLACE ... SET:
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
  "index":"products",
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
  "_index":"products",
  "_id":1,
  "created":false,
  "result":"updated",
  "status":200
}

```

<!-- intro -->
##### Elasticsearch-like

<!-- request Elasticsearch-like -->

> 注意：部分替换需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保 Buddy 已安装。

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
"_index":"products",
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
"_index":"products",
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

> 注意：部分替换需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保 Buddy 已安装。

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
"_index":"products",
"updated":1
}
```

<!-- intro -->
##### Elasticsearch-like 在集群中进行部分替换：

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
"_index":"products",
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
indexApi.replace({"index" : "products", "id" : 1, "doc" : {"title" : "document one","price":10}})
```

<!-- response Python -->
```python
{'created': False,
 'found': None,
 'id': 1,
 'index': 'products',
 'result': 'updated'}
```
<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.replace({"index" : "products", "id" : 1, "doc" : {"title" : "document one","price":10}});
```

<!-- response javascript -->
```javascript
{"_index":"products","_id":1,"result":"updated"}
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
    "_index":"test",
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
    "_index":"test",
    "_id":1,
    "created":false
    "result":"updated"
    "status":200
}
```

<!-- end -->

`REPLACE` 适用于实时表和渗透表。你不能在普通表中替换数据。

当你执行 `REPLACE` 时，之前的文档不会被移除，而是被标记为已删除，因此表的大小会随着时间的推移增加，直到发生块合并。要强制进行块合并，请使用 [OPTIMIZE 语句](../../Securing_and_compacting_a_table/Compacting_a_table.md)。

## 批量替换

<!-- example bulk_replace -->

你可以一次替换多个文档。查看[批量添加文档](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-adding-documents)以获取更多信息。

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
{ "replace" : { "index" : "products", "id":1, "doc": { "title": "doc one", "tag" : 10 } } }
{ "replace" : { "index" : "products", "id":2, "doc": { "title": "doc two", "tag" : 20 } } }
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
        "_index":"products",
        "_id":1,
        "created":false,
        "result":"updated",
        "status":200
      }
    },
    {
      "replace":
      {
        "_index":"products",
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
    {"replace": {"index" : "products", "id" : 1, "doc" : {"title" : "document one"}}}, \
    {"replace": {"index" : "products", "id" : 2, "doc" : {"title" : "document two"}}} ]
api_resp = indexApi.bulk('\n'.join(map(json.dumps,docs)))
```

<!-- response Python -->
```python
{'error': None,
 'items': [{u'replace': {u'_id': 1,
                         u'_index': u'products',
                         u'created': False,
                         u'result': u'updated',
                         u'status': 200}},
           {u'replace': {u'_id': 2,
                         u'_index': u'products',
                         u'created': False,
                         u'result': u'updated',
                         u'status': 200}}]}

```
<!-- request javascript -->

``` javascript
docs = [
    {"replace": {"index" : "products", "id" : 1, "doc" : {"title" : "document one"}}},
    {"replace": {"index" : "products", "id" : 2, "doc" : {"title" : "document two"}}} ];
res =  await indexApi.bulk(docs.map(e=>JSON.stringify(e)).join('\n'));
```

<!-- response javascript -->
```javascript
{"items":[{"replace":{"_index":"products","_id":1,"created":false,"result":"updated","status":200}},{"replace":{"_index":"products","_id":2,"created":false,"result":"updated","status":200}}],"errors":false}

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
        "_index":"test",
        "_id":1,
        "created":false,
        "result":"updated",
        "status":200
      }
    },
    {
      "replace":
      {
        "_index":"test",
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
        "_index":"test",
        "_id":1,
        "created":false,
        "result":"updated",
        "status":200
      }
    },
    {
      "replace":
      {
        "_index":"test",
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
