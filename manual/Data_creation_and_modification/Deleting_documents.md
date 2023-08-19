# Deleting documents

Deleting documents is only supported for the following table types:
* [Real-time](../Creating_a_table/Local_tables/Real-time_table.md) tables,
* [Percolate](../Creating_a_table/Local_tables/Percolate_table.md) tables,
* Distributed tables that only contain RT tables a local or remote agents.

You can delete existing documents from a table based on either their ID or certain conditions.

Also, [bulk deletion](../Data_creation_and_modification/Deleting_documents.md#Bulk-deletion) is available to delete multiple documents.

<!-- example delete 1 -->
Deletion of documents can be accomplished via both SQL and HTTP interfaces.

For SQL, the response for a successful operation will indicate the number of rows deleted.

For HTTP, the `json/delete` endpoint is used. The server will respond with a JSON object indicating whether the operation was successful and the number of rows deleted.

It is recommended to use [table truncation](../Emptying_a_table.md) instead of deletion to delete all documents from a table, as it is a much faster operation.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
DELETE FROM table WHERE where_condition
```

* `table` is a name of the table from which the row should be deleted.
* `where_condition` for SQL has the same syntax as in the [SELECT](../Searching/Full_text_matching/Basic_usage.md#SQL) statement.

<!-- request JSON -->
``` json
POST /delete -d '
    {
     "index": "test",
     "id": 1
    }'
POST /delete -d '
    {
        "index": "test",
        "query":
        {
            "match": { "*": "apple" }
        }
    }'
```

* `id` for JSON is the row `id` which should be deleted.
* `query` for JSON is the full-text condition and has the same syntax as in the [JSON/update](../Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-HTTP-JSON).
* `cluster` for JSON is cluster name property and should be set along with `table` property to delete a row from a table which is inside a [replication cluster](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster).

<!-- end -->

<!-- example delete 2 -->
In this example we are deleting all documents that match full-text query `dummy` from table named `test`:


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT * FROM TEST;

DELETE FROM TEST WHERE MATCH ('dummy');

SELECT * FROM TEST;
```

<!-- response SQL -->

```sql
+------+------+-------------+------+
| id   | gid  | mva1        | mva2 |
+------+------+-------------+------+
|  100 | 1000 | 100,201     | 100  |
|  101 | 1001 | 101,202     | 101  |
|  102 | 1002 | 102,203     | 102  |
|  103 | 1003 | 103,204     | 103  |
|  104 | 1004 | 104,204,205 | 104  |
|  105 | 1005 | 105,206     | 105  |
|  106 | 1006 | 106,207     | 106  |
|  107 | 1007 | 107,208     | 107  |
+------+------+-------------+------+
8 rows in set (0.00 sec)

Query OK, 2 rows affected (0.00 sec)

+------+------+-------------+------+
| id   | gid  | mva1        | mva2 |
+------+------+-------------+------+
|  100 | 1000 | 100,201     | 100  |
|  101 | 1001 | 101,202     | 101  |
|  102 | 1002 | 102,203     | 102  |
|  103 | 1003 | 103,204     | 103  |
|  104 | 1004 | 104,204,205 | 104  |
|  105 | 1005 | 105,206     | 105  |
+------+------+-------------+------+
6 rows in set (0.00 sec)
```

<!-- request JSON -->

``` json
POST /delete -d '
    {
        "index":"test",
        "query":
        {
            "match": { "*": "dummy" }
        }
    }'
```

<!-- response JSON -->

``` json
    {
        "_index":"test",
        "deleted":2,
    }
```    
<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->deleteDocuments(new Match('dummy','*'));
```

<!-- response json -->

``` php
Array(
    [_index] => test
    [deleted] => 2
)
```
<!-- intro -->

##### Python:

<!-- request Python -->
``` python
indexApi.delete({"index" : "products", "query": { "match": { "*": "dummy" }}})
```

<!-- response Python -->
```python
{'deleted': 2, 'id': None, 'index': 'products', 'result': None}
```
<!-- intro -->

##### javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.delete({"index" : "products", "query": { "match": { "*": "dummy" }}});
```

<!-- response javascript -->
```javascript
{"_index":"products","deleted":2}
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest();
query = new HashMap<String,Object>();
query.put("match",new HashMap<String,Object>(){{
    put("*","dummy");
}});
deleteRequest.index("products").setQuery(query);
indexApi.delete(deleteRequest);

```

<!-- response Java -->
```java
class DeleteResponse {
    index: products
    deleted: 2
    id: null
    result: null
}
```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
Dictionary<string, Object> match = new Dictionary<string, Object>(); 
match.Add("*", "dummy");
Dictionary<string, Object> query = new Dictionary<string, Object>(); 
query.Add("match", match);
DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest(index: "products", query: query);
indexApi.Delete(deleteRequest);

```

<!-- response C# -->
```clike
class DeleteResponse {
    index: products
    deleted: 2
    id: null
    result: null
}
```
<!-- end -->

<!-- example delete 3 -->
Here - deleting a document with `id` 100 from table named `test`:


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
DELETE FROM TEST WHERE id=100;

SELECT * FROM TEST;
```

<!-- response SQL -->

```sql
Query OK, 1 rows affected (0.00 sec)

+------+------+-------------+------+
| id   | gid  | mva1        | mva2 |
+------+------+-------------+------+
|  101 | 1001 | 101,202     | 101  |
|  102 | 1002 | 102,203     | 102  |
|  103 | 1003 | 103,204     | 103  |
|  104 | 1004 | 104,204,205 | 104  |
|  105 | 1005 | 105,206     | 105  |
+------+------+-------------+------+
5 rows in set (0.00 sec)
```

<!-- request JSON -->

``` json
POST /delete -d '
    {
        "index":"test",
        "id": 100
    }'
```

<!-- response JSON -->

``` json
    {
        "_index":"test",
        "_id":100,
        "found":true,
        "result":"deleted"      
    }
```    
<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->deleteDocument(100);
```

<!-- response json -->

``` php
Array(
    [_index] => test
    [_id] => 100
    [found] => true
    [result] => deleted
)
```
<!-- intro -->

##### Python:

<!-- request Python -->
``` python
indexApi.delete({"index" : "products", "id" : 1})
```

<!-- response Python -->
```python
{'deleted': None, 'id': 1, 'index': 'products', 'result': 'deleted'}
```
<!-- intro -->

##### javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.delete({"index" : "products", "id" : 1});
```

<!-- response javascript -->
```javascript
{"_index":"products","_id":1,"result":"deleted"}
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest();
deleteRequest.index("products").setId(1L);
indexApi.delete(deleteRequest);

```

<!-- response Java -->
```java
class DeleteResponse {
    index: products
    _id: 1
    result: deleted
}
```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest(index: "products", id: 1);
indexApi.Delete(deleteRequest);

```

<!-- response C# -->
```java
class DeleteResponse {
    index: products
    _id: 1
    result: deleted
}
```
<!-- end -->

<!-- example delete 4 -->
Manticore SQL allows to use complex conditions for the `DELETE` statement.

For example here we are deleting documents that match full-text query `dummy` and have attribute `mva1` with a value greater than 206 or `mva1` values 100 or 103 from table named `test`:


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
DELETE FROM TEST WHERE MATCH ('dummy') AND ( mva1>206 or mva1 in (100, 103) );

SELECT * FROM TEST;
```

<!-- response SQL -->

```sql
Query OK, 4 rows affected (0.00 sec)

+------+------+-------------+------+
| id   | gid  | mva1        | mva2 |
+------+------+-------------+------+
|  101 | 1001 | 101,202     | 101  |
|  102 | 1002 | 102,203     | 102  |
|  104 | 1004 | 104,204,205 | 104  |
|  105 | 1005 | 105,206     | 105  |
+------+------+-------------+------+
6 rows in set (0.00 sec)
```
<!-- end -->

<!-- example delete 5 -->
Here is an example of deleting documents in cluster `cluster`'s table `test`:


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
delete from cluster:test where id=100;
```

<!-- request JSON -->
``` json
POST /delete -d '
    {
      "cluster":"cluster",
      "index":"test",
      "id": 100
    }'
```
<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->setCluster('cluster');
$index->deleteDocument(100);
```

<!-- response JSON -->

``` php
Array(
    [_index] => test
    [_id] => 100
    [found] => true
    [result] => deleted
)
```
<!-- intro -->

##### Python:

<!-- request Python -->
``` python
indexApi.delete({"cluster":"cluster","index" : "products", "id" : 100})
```

<!-- response Python -->
```python
{'deleted': None, 'id': 100, 'index': 'products', 'result': 'deleted'}
```
<!-- intro -->

##### javascript:

<!-- request javascript -->
``` javascript
indexApi.delete({"cluster":"cluster","index" : "products", "id" : 100})
```

<!-- response javascript -->
```javascript
{"_index":"products","_id":100,"result":"deleted"}
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest();
deleteRequest.cluster("cluster").index("products").setId(100L);
indexApi.delete(deleteRequest);

```

<!-- response Java -->
```java
class DeleteResponse {
    index: products
    _id: 100
    result: deleted
}
```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest(index: "products", cluster: "cluster", id: 100);
indexApi.Delete(deleteRequest);

```

<!-- response C# -->
```clike
class DeleteResponse {
    index: products
    _id: 100
    result: deleted
}
```
<!-- end -->


## Bulk deletion

<!-- example bulk delete -->

You can also perform multiple delete operations in a single call using the `/bulk` endpoint. This endpoint only works with data that has `Content-Type` set to `application/x-ndjson`. The data should be formatted as newline-delimited JSON (NDJSON). Essentially, this means that each line should contain exactly one JSON statement and end with a newline `\n` and, possibly, a `\r`.


<!-- intro -->
##### JSON:

<!-- request JSON --Content-type=application/x-ndjson -->

```json
POST /bulk

{ "delete" : { "index" : "products", "id" : 1 } }
{ "delete" : { "index" : "products", "query": { "equals": { "price" : 20 } } } }
```

<!-- response JSON -->

```json
{
   "items":
   [
      {
         "bulk":
         {
            "_index":"products",
            "_id":0,
            "created":0,
            "deleted":2,
            "updated":0,
            "result":"created",
            "status":201
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
    ['delete' => [
            'index' => 'products',
            'id' => 1
        ]
    ],
    ['delete'=>[
            'index' => 'products',
            'query' => [
                'equals' => ['price' => 20]
            ]
        ]
    ]
]);
```

<!-- response PHP -->

```php
Array(
    [items] => Array
        (
            [0] => Array
                (
                    [bulk] => Array
                        (
                            [_index] => test
                            [_id] => 0
                            [created] => 0
                            [deleted] => 2
                            [updated] => 0
                            [result] => created
                            [status] => 201
                        )

                )

        )

    [current_line] => 3
    [skipped_lines] => 0
    [errors] => 
    [error] => 
)

```


<!-- intro -->
##### Python:

<!-- request Python -->
``` python
docs = [ \
            { "delete" : { "index" : "products", "id": 1 } }, \
            { "delete" : { "index" : "products", "query": { "equals": { "price": 20 } } } } ]
indexApi.bulk('\n'.join(map(json.dumps,docs)))
```

<!-- response Python -->
```python
{
    'error': None,
    'items': [{u'delete': {u'_index': u'products', u'deleted': 2}}]
}

```
<!-- intro -->
##### javascript:

<!-- request javascript -->
``` javascript
docs = [
            { "delete" : { "index" : "products", "id": 1 } },
            { "delete" : { "index" : "products", "query": { "equals": { "price": 20 } } } } ];
res =  await indexApi.bulk(docs.map(e=>JSON.stringify(e)).join('\n'));
```

<!-- response javascript -->
```javascript
{"items":[{"delete":{"_index":"products","deleted":2}}],"errors":false}

```

<!-- intro -->
##### java:

<!-- request Java -->
``` java
String   body = "{ "delete" : { "index" : "products", "id": 1 } } "+"\n"+
    "{ "delete" : { "index" : "products", "query": { "equals": { "price": 20 } } } }"+"\n";         
indexApi.bulk(body);
```

<!-- response Java -->
```java
class BulkResponse {
    items: [{delete={_index=products, _id=0, created=false, deleted=2, result=created, status=200}}]
    error: null
    additionalProperties: {errors=false}
}
```

<!-- intro -->
##### C#:

<!-- request C# -->
``` clike
string   body = "{ "delete" : { "index" : "products", "id": 1 } } "+"\n"+
    "{ "delete" : { "index" : "products", "query": { "equals": { "price": 20 } } } }"+"\n";         
indexApi.Bulk(body);
```

<!-- response C# -->
```clike
class BulkResponse {
    items: [{replace={_index=products, _id=0, created=false, deleted=2, result=created, status=200}}]
    error: null
    additionalProperties: {errors=false}
}
```


<!-- end -->
<!-- proofread -->