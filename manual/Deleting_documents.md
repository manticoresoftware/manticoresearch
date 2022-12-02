# Deleting documents

Deleting is only supported for:
* [real-time](Creating_a_table/Local_tables/Real-time_table.md) tables,
* [percolate](Creating_a_table/Local_tables/Percolate_table.md) tables
* distributed tables that contain only RT tables as agents

You can delete existing rows (documents) from an existing table based on ID or conditions.

<!-- example delete 1 -->
Deleting documents is supported via SQL and HTTP interfaces.

SQL response for successful operation will show the number of rows deleted.

`json/delete` is an HTTP endpoint for deleting. The server will respond with a JSON object stating if the operation was successful or not and the number of rows deleted.

To delete all documents from a table it's recommended to use instead the [table truncation](Emptying_a_table.md) as it's a much faster operation.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
DELETE FROM table WHERE where_condition
```

* `table` is a name of the table from which the row should be deleted.
* `where_condition` for SQL has the same syntax as in the [SELECT](Searching/Full_text_matching/Basic_usage.md#SQL) statement.

<!-- request HTTP -->
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
* `query` for JSON is the full-text condition and has the same syntax as in the [JSON/update](Updating_documents/UPDATE.md#Updates-via-HTTP-JSON).
* `cluster` for JSON is cluster name property and should be set along with `table` property to delete a row from a table which is inside a [replication cluster](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster).

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

<!-- request HTTP -->

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

<!-- response json -->

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

<!-- request HTTP -->

``` json
POST /delete -d '
    {
        "index":"test",
        "id": 100
    }'
```

<!-- response json -->

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
Here is an example of deleting documents in cluster `nodes4`'s table `test`:


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
delete from nodes4:test where id=100;
```

<!-- request HTTP -->
``` json
POST /delete -d '
    {
      "cluster":"nodes4",
      "index":"test",
      "id": 100
    }'
```
<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->setCluster('nodes4');
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
indexApi.delete({"cluster":"nodes4","index" : "products", "id" : 100})
```

<!-- response Python -->
```python
{'deleted': None, 'id': 100, 'index': 'products', 'result': 'deleted'}
```
<!-- intro -->

##### javascript:

<!-- request javascript -->
``` javascript
indexApi.delete({"cluster":"nodes4","index" : "products", "id" : 100})
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
deleteRequest.cluster("nodes4").index("products").setId(100L); 
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
<!-- end -->
