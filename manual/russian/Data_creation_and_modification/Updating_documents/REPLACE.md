# REPLACE

<!-- example replace -->

`REPLACE` работает аналогично [INSERT](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md), но помечает предыдущий документ с тем же ID как удалённый перед вставкой нового.

Если вас интересуют обновления на месте, пожалуйста, смотрите [этот раздел](../../Data_creation_and_modification/Updating_documents/UPDATE.md).

## SQL REPLACE

Синтаксис SQL-запроса `REPLACE` следующий:

**Чтобы заменить весь документ:**
```sql
REPLACE INTO table [(column1, column2, ...)]
    VALUES (value1, value2, ...)
    [, (...)]
```

**Чтобы заменить только выбранные поля:**
```sql
REPLACE INTO table
    SET field1=value1[, ..., fieldN=valueN]
    WHERE id = <id>
```
Обратите внимание, что в этом режиме можно фильтровать только по id.

> ПРИМЕЧАНИЕ: Частичное замещение требует [Manticore Buddy](Installation/Manticore_Buddy.md). Если это не работает, убедитесь, что Buddy установлен.

Подробнее о `UPDATE` и частичном `REPLACE` читайте [здесь](../../Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md#UPDATE-vs-partial-REPLACE).

Смотрите примеры для получения дополнительной информации.

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
  `/index` является псевдонимом конечной точки и работает так же.
* Точка доступа, похожая на Elasticsearch `<table>/_doc/<id>`:
  ```
  PUT/POST /<table name>/_doc/<id>
  {
    "<field1>": <value1>,
    ...
    "<fieldN>": <valueN>
  }
  ```
  > ПРИМЕЧАНИЕ: Замещение, похожее на Elasticsearch, требует [Manticore Buddy](Installation/Manticore_Buddy.md). Если это не работает, убедитесь, что Buddy установлен.
* Частичное замещение:
  ```
  POST /<{table | cluster:table}>/_update/<id>
  {
    "<field1>": <value1>,
    ...
    "<fieldN>": <valueN>
  }
  ```
  `<table name>` может быть просто названием таблицы или в формате `cluster:table`. Это позволяет выполнять обновления в пределах конкретного кластера при необходимости.

  > ПРИМЕЧАНИЕ: Частичное замещение требует [Manticore Buddy](Installation/Manticore_Buddy.md). Если это не работает, убедитесь, что Buddy установлен.

Смотрите примеры для получения дополнительной информации.

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

> ПРИМЕЧАНИЕ: Замещение, похожее на Elasticsearch, требует [Manticore Buddy](Installation/Manticore_Buddy.md). Если это не работает, убедитесь, что Buddy установлен.

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
##### Elasticsearch-like partial replace:

<!-- request Elasticsearch-like partial -->

> ПРИМЕЧАНИЕ: Частичное замещение требует [Manticore Buddy](Installation/Manticore_Buddy.md). Если это не работает, убедитесь, что Buddy установлен.

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
##### Elasticsearch-like partial replace in cluster:

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

##### Python-asyncio:

<!-- request Python-asyncio -->
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

`REPLACE` доступен для реал-тайм таблиц и перколейт таблиц. Вы не можете заменить данные в обычной таблице.

При выполнении `REPLACE` предыдущий документ не удаляется, а помечается как удалённый, поэтому размер таблицы растёт до тех пор, пока не произойдёт слияние чанков. Чтобы принудительно выполнить слияние чанков, используйте [OPTIMIZE statement](../../Securing_and_compacting_a_table/Compacting_a_table.md).

## Массовое замещение

<!-- example bulk_replace -->

Вы можете заменить несколько документов за один раз. Для получения дополнительной информации смотрите [массовое добавление документов](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-adding-documents).

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

<!-- request Python-asyncio -->

``` python
indexApi = manticoresearch.IndexApi(client)
docs = [ \
    {"replace": {"table" : "products", "id" : 1, "doc" : {"title" : "document one"}}}, \
    {"replace": {"table" : "products", "id" : 2, "doc" : {"title" : "document two"}}} ]
api_resp = await indexApi.bulk('\n'.join(map(json.dumps,docs)))
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

<!-- request Rust -->

``` rust
string body = r#"{"replace": {"index" : "products", "id" : 1, "doc" : {"title" : "document one"}}}
    {"replace": {"index" : "products", "id" : 2, "doc" : {"title" : "document two"}}}
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

