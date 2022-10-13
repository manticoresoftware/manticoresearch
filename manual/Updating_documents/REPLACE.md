# REPLACE

<!-- example replace -->

`REPLACE` works similar to [INSERT](../Adding_documents_to_an_index/Adding_documents_to_a_real-time_index.md), but it marks the old document with the same ID as a new document as deleted before inserting a new document.

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
##### HTTP JSON

<!-- request HTTP -->

```http
POST /replace
-H "Content-Type: application/x-ndjson" -d '
{
  "index":"products",
  "id":1,
  "doc":
  {
	"title":"document one",
    "price":10
  }
}
'
```

<!-- response HTTP -->
```http
{
  "_index":"products",
  "_id":1,
  "created":false,
  "result":"updated",
  "status":200
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

`REPLACE` is supported for RT and PQ indexes.

The old document is not removed from the index, it is only marked as deleted. Because of this the index size grows until index chunks are merged and documents marked as deleted in these chunks are not included in the chunk created as a result of merge. You can force chunk merge by using [OPTIMIZE statement](../Securing_and_compacting_an_index/Compacting_an_index.md).

The syntax of the `REPLACE` statement is identical to [INSERT syntax](../Adding_documents_to_an_index/Adding_documents_to_a_real-time_index.md):

```sql
REPLACE INTO index [(column1, column2, ...)]
    VALUES (value1, value2, ...)
    [, (...)]
```

`REPLACE` using HTTP protocol is performed via the `/replace` endpoint. There's also a synonym endpoint, `/index`.

<!-- example bulk_replace -->

Multiple documents can be replaced at once. See [bulk adding documents](../Adding_documents_to_an_index/Adding_documents_to_a_real-time_index.md#Bulk-adding-documents) for more details.

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

<!-- request HTTP -->

```json
POST /bulk
-H "Content-Type: application/x-ndjson" -d '
{ "replace" : { "index" : "products", "id":1, "doc": { "title": "doc one", "tag" : 10 } } }
{ "replace" : { "index" : "products", "id":2, "doc": { "title": "doc two", "tag" : 20 } } }
'
```

<!-- response HTTP -->

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
