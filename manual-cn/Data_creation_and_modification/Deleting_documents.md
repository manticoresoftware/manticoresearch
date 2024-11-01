# 删除文档

删除文档仅在 [RT 模式](../../Read_this_first.md#Real-time-mode-vs-plain-mode) 下支持以下表类型：
* [实时表](../Creating_a_table/Local_tables/Real-time_table.md)
* [渗透表](../Creating_a_table/Local_tables/Percolate_table.md)
* 仅包含 RT 表的分布式表作为本地或远程代理。

你可以根据文档的 ID 或某些条件从表中删除现有文档。

此外，[批量删除](../Data_creation_and_modification/Deleting_documents.md#Bulk-deletion) 可用于删除多个文档。

删除文档可以通过 SQL 和 JSON 接口完成。

对于 SQL，成功操作的响应将指示删除的行数。

对于 JSON，使用 `json/delete` 端点。服务器将响应一个 JSON 对象，指示操作是否成功以及删除的行数。

建议使用 [表截断](../Emptying_a_table.md) 来删除表中的所有文档，因为这是一个更快的操作。

<!-- example delete 2 -->
在此示例中，我们将从名为 `test` 的表中删除所有符合全文查询 `test document` 的文档：


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
mysql> SELECT * FROM TEST;
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

mysql> DELETE FROM TEST WHERE MATCH ('test document');
Query OK, 2 rows affected (0.00 sec)

mysql> SELECT * FROM TEST;
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
            "match": { "*": "test document" }
        }
    }'
```

* `query` 的 JSON 格式包含一个用于全文搜索的子句，语法与  [JSON/update](../Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-HTTP-JSON) 中的相同。

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
$index->deleteDocuments(new MatchPhrase('test document','*'));
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
indexApi.delete({"index" : "test", "query": { "match": { "*": "test document" }}})
```

<!-- response Python -->
```python
{'deleted': 5, 'id': None, 'index': 'test', 'result': None}
```
<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.delete({"index" : "test", "query": { "match": { "*": "test document" }}});
```

<!-- response javascript -->
```javascript
{"_index":"test","deleted":5}
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest();
query = new HashMap<String,Object>();
query.put("match",new HashMap<String,Object>(){{
    put("*","test document");
}});
deleteRequest.index("test").setQuery(query);
indexApi.delete(deleteRequest);

```

<!-- response Java -->
```java
class DeleteResponse {
    index: test
    deleted: 5
    id: null
    result: null
}
```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
Dictionary<string, Object> match = new Dictionary<string, Object>();
match.Add("*", "test document");
Dictionary<string, Object> query = new Dictionary<string, Object>();
query.Add("match", match);
DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest(index: "test", query: query);
indexApi.Delete(deleteRequest);

```

<!-- response C# -->
```clike
class DeleteResponse {
    index: test
    deleted: 5
    id: null
    result: null
}
```

<!-- intro -->

##### TypeScript:

<!-- request TypeScript -->
``` typescript
res = await indexApi.delete({
  index: 'test',
  query: { match: { '*': 'test document' } },
});
```

<!-- response TypeScript -->
```json
{"_index":"test","deleted":5}
```

<!-- intro -->

##### Go:

<!-- request Go -->
```go
deleteRequest := manticoresearch.NewDeleteDocumentRequest("test")
matchExpr := map[string]interface{} {"*": "test document"}
deleteQuery := map[string]interface{} {"match": matchExpr }
deleteRequest.SetQuery(deleteQuery)
```

<!-- response Go -->
```json
{"_index":"test","deleted":5}
```

<!-- end -->

<!-- example delete 3 -->
以下示例展示了如何从名为 `test` 的表中删除 `id` 为 1 的文档：


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
mysql> DELETE FROM TEST WHERE id=1;
Query OK, 1 rows affected (0.00 sec)
```

<!-- request JSON -->

``` json
POST /delete -d '
    {
        "index": "test",
        "id": 1
    }'
```

* 对于 JSON，`id` 表示应删除的行的 `id`。

<!-- response JSON -->

``` json
{
    "_index": "test",
    "_id": 1,
    "found": true,
    "result": "deleted"      
}
```
<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->deleteDocument(1);
```

<!-- response json -->

``` php
Array(
    [_index] => test
    [_id] => 1
    [found] => true
    [result] => deleted
)
```
<!-- intro -->

##### Python:

<!-- request Python -->
``` python
indexApi.delete({"index" : "test", "id" : 1})
```

<!-- response Python -->
```python
{'deleted': None, 'id': 1, 'index': 'test', 'result': 'deleted'}
```
<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.delete({"index" : "test", "id" : 1});
```

<!-- response javascript -->
```javascript
{"_index":"test","_id":1,"result":"deleted"}
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest();
deleteRequest.index("test").setId(1L);
indexApi.delete(deleteRequest);

```

<!-- response Java -->
```java
class DeleteResponse {
    index: test
    _id: 1
    result: deleted
}
```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest(index: "test", id: 1);
indexApi.Delete(deleteRequest);

```

<!-- response C# -->
```java
class DeleteResponse {
    index: test
    _id: 1
    result: deleted
}
```

<!-- intro -->

##### TypeScript:

<!-- request TypeScript -->
``` typescript
res = await indexApi.delete({ index: 'test', id: 1 });
```

<!-- response TypeScript -->
```json
{"_index":"test","_id":1,"result":"deleted"}
```

<!-- intro -->

##### Go:

<!-- request Go -->
```go
deleteRequest := manticoresearch.NewDeleteDocumentRequest("test")
deleteRequest.SetId(1)
```

<!-- response Go -->
```json
{"_index":"test","_id":1,"result":"deleted"}
```

<!-- end -->

<!-- example delete 4 -->
以下示例展示了如何删除 `id` 匹配给定值的文档，删除操作发生在名为 `test` 的表中：

请注意，使用 `id=N` 或 `id IN (X,Y)` 的删除形式是最快的，因为它们在删除文档时无需执行搜索操作。 还要注意，响应中只包含第一个被删除文档的 `id`，并显示在相应的 `_id` 字段中。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
DELETE FROM TEST WHERE id IN (1,2);

```

<!-- response SQL -->

```sql
Query OK, 2 rows affected (0.00 sec)
```

<!-- request JSON -->

``` json
POST /delete -d '
    {
        "index":"test",
        "id": [1,2]
    }'
```

<!-- response JSON -->

``` json
    {
        "_index":"test",
        "_id":1,
        "found":true,
        "result":"deleted"      
    }
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->deleteDocumentsByIds([1,2]);
```

<!-- response json -->

``` php
Array(
    [_index] => test
    [_id] => 1
    [found] => true
    [result] => deleted
)
```

<!-- end -->

<!-- example delete 5 -->

Manticore SQL 允许在 `DELETE` 语句中使用复杂条件。

例如，以下示例展示了如何从名为 `test` 的表中删除满足全文查询 `test document` 且属性 `mva1` 大于 206 或 `mva1` 值为 100 或 103 的文档：


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
DELETE FROM TEST WHERE MATCH ('test document') AND ( mva1>206 or mva1 in (100, 103) );

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

<!-- example delete 6 -->
以下是从集群 `cluster` 中的表 `test` 删除文档的示例。请注意，当从复制集群中的表中删除行时，必须同时提供集群名称和表名称属性：

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
      "cluster": "cluster",
      "index": "test",
      "id": 100
    }'
```
* `cluster` 用于 JSON 中，表示包含所需表的[复制集群](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster)的名称。

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
indexApi.delete({"cluster":"cluster","index" : "test", "id" : 1})
```

<!-- response Python -->
```python
{'deleted': None, 'id': 1, 'index': 'test', 'result': 'deleted'}
```
<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
indexApi.delete({"cluster":"cluster_1","index" : "test", "id" : 1})
```

<!-- response javascript -->
```javascript
{"_index":"test","_id":1,"result":"deleted"}
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest();
deleteRequest.cluster("cluster").index("test").setId(1L);
indexApi.delete(deleteRequest);

```

<!-- response Java -->
```java
class DeleteResponse {
    index: test
    _id: 1
    result: deleted
}
```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest(index: "test", cluster: "cluster", id: 1);
indexApi.Delete(deleteRequest);

```

<!-- response C# -->
```clike
class DeleteResponse {
    index: test
    _id: 1
    result: deleted
}
```

<!-- intro -->

##### TypeScript:

<!-- request TypeScript -->
``` typescript
res = await indexApi.delete({ cluster: 'cluster_1', index: 'test', id: 1 });
```

<!-- response TypeScript -->
```json
{"_index":"test","_id":1,"result":"deleted"}
```

<!-- intro -->

##### Go:

<!-- request Go -->
```go
deleteRequest := manticoresearch.NewDeleteDocumentRequest("test")
deleteRequest.SetCluster("cluster_1")
deleteRequest.SetId(1)
```

<!-- response Go -->
```json
{"_index":"test","_id":1,"result":"deleted"}
```

<!-- end -->

## 批量删除

<!-- example bulk delete -->

您还可以通过 `/bulk` 端点在单个调用中执行多个删除操作。该端点仅适用于 `Content-Type` 设置为 `application/x-ndjson` 的数据。数据应以换行分隔的 JSON (NDJSON) 格式编写。简而言之，这意味着每一行应包含一个 JSON 语句，并以换行符 `\n` 或可能的 `\r` 结尾。


<!-- intro -->
##### JSON:

<!-- request JSON --Content-type=application/x-ndjson -->

```json
POST /bulk

{ "delete" : { "index" : "test", "id" : 1 } }
{ "delete" : { "index" : "test", "query": { "equals": { "int_data" : 20 } } } }
```

<!-- response JSON -->

```json
{
   "items":
   [
      {
         "bulk":
         {
            "_index":"test",
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
            'index' => 'test',
            'id' => 1
        ]
    ],
    ['delete'=>[
            'index' => 'test',
            'query' => [
                'equals' => ['int_data' => 20]
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
            { "delete" : { "index" : "test", "id": 1 } }, \
            { "delete" : { "index" : "test", "query": { "equals": { "int_data": 20 } } } } ]
indexApi.bulk('\n'.join(map(json.dumps,docs)))
```

<!-- response Python -->
```python
{
    'error': None,
    'items': [{u'delete': {u'_index': test', u'deleted': 2}}]
}

```
<!-- intro -->
##### Javascript:

<!-- request javascript -->
``` javascript
docs = [
            { "delete" : { "index" : "test", "id": 1 } },
            { "delete" : { "index" : "test", "query": { "equals": { "int_data": 20 } } } } ];
res =  await indexApi.bulk(docs.map(e=>JSON.stringify(e)).join('\n'));
```

<!-- response javascript -->
```javascript
{"items":[{"delete":{"_index":"test","deleted":2}}],"errors":false}

```

<!-- intro -->
##### java:

<!-- request Java -->
``` java
String   body = "{ "delete" : { "index" : "test", "id": 1 } } "+"\n"+
    "{ "delete" : { "index" : "test", "query": { "equals": { "int_data": 20 } } } }"+"\n";         
indexApi.bulk(body);
```

<!-- response Java -->
```java
class BulkResponse {
    items: [{delete={_index=test, _id=0, created=false, deleted=2, result=created, status=200}}]
    error: null
    additionalProperties: {errors=false}
}
```

<!-- intro -->
##### C#:

<!-- request C# -->
``` clike
string   body = "{ "delete" : { "index" : "test", "id": 1 } } "+"\n"+
    "{ "delete" : { "index" : "test", "query": { "equals": { "int_data": 20 } } } }"+"\n";         
indexApi.Bulk(body);
```

<!-- response C# -->
```clike
class BulkResponse {
    items: [{replace={_index=test, _id=0, created=false, deleted=2, result=created, status=200}}]
    error: null
    additionalProperties: {errors=false}
}
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->
``` typescript
docs = [
  { "delete" : { "index" : "test", "id": 1 } },
  { "delete" : { "index" : "test", "query": { "equals": { "int_data": 20 } } } }
];
body = await indexApi.bulk(
  docs.map((e) => JSON.stringify(e)).join("\n")
);            
res = await indexApi.bulk(body);
```

<!-- response TypeScript -->
```json
{"items":[{"delete":{"_index":"test","deleted":2}}],"errors":false}

```

<!-- intro -->

##### Go:

<!-- request Go -->
```go
docs = []string {
  `{ "delete" : { "index" : "test", "id": 1 } }`,
  `{ "delete" : { "index" : "test", "query": { "equals": { "int_data": 20 } } } }`
]
body = strings.Join(docs, "\n")
resp, httpRes, err := manticoreclient.IndexAPI.Bulk(context.Background()).Body(body).Execute()
```

<!-- response Go -->
```json
{"items":[{"delete":{"_index":"test","deleted":2}}],"errors":false}
```

<!-- end -->


<!-- end -->
<!-- proofread -->
