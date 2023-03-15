# REPLACE

<!-- example replace -->

`REPLACE` works similarly to [INSERT](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md), but it marks the previous document with the same ID as deleted before inserting a new one.

For HTTP JSON protocol, two request formats are available: Manticore and Elasticsearch-like. You can find both examples in the provided examples.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
REPLACE INTO products VALUES(1, "document one", 10);
```

<!-- response -->

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
##### Elasticsearch

<!-- request Elasticsearch -->

```json
PUT /products/_doc/2
{
  "title": "product two",
  "price": 20
}

POST /products/_doc/
{
  "title": "product three",
  "price": 10
}
```

<!-- response Elasticsearch -->
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
"result":"created"
}

{
"_id":2235747273424240642,
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
"result":"created"
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

##### javascript:

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
<!-- end -->

`REPLACE` is available for both RT and PQ tables.

When you run a `REPLACE`, the previous document is not removed, but it's marked as deleted, so the table size grows until chunk merging happens, and the marked documents won't be included. To force a chunk merge, use the [OPTIMIZE statement](../../Securing_and_compacting_a_table/Compacting_a_table.md).

The syntax of the `REPLACE` statement is the same as the [INSERT statement syntax](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md).

```sql
REPLACE INTO table [(column1, column2, ...)]
    VALUES (value1, value2, ...)
    [, (...)]
```

To use the HTTP JSON interface with `REPLACE`, use the `/replace` endpoint. There's also a synonym endpoint, `/index`.

<!-- example bulk_replace -->

You can replace multiple documents at once. Check [bulk adding documents](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-adding-documents) for more information.

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
<!-- end -->

<!-- proofread -->
