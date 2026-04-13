# 删除文档

删除文档仅在[实时模式](../Read_this_first.md#Real-time-mode-vs-plain-mode)中支持以下表格类型：
* [实时](../Creating_a_table/Local_tables/Real-time_table.md) 表格
* [Percolate](../Creating_a_table/Local_tables/Percolate_table.md) 表格
* 只包含实时表作为本地或远程代理的分布式表格。

您可以根据其ID或某些条件从表格中删除现有文档。

此外，[批量删除](../Data_creation_and_modification/Deleting_documents.md#Bulk-deletion)可用于删除多个文档。

通过SQL和JSON接口均可实现文档的删除。

对于SQL，成功操作的响应将指示删除的行数。

对于JSON，使用`json/delete`端点。服务器将以JSON对象形式响应，指示操作是否成功以及删除的行数。

建议使用[表清空](../Emptying_a_table.md)而不是删除来从表中删除所有文档，因为这是一个更快的操作。


<!-- example delete 2 -->
在这个示例中，我们从名为`test`的表中删除匹配全文查询`test document`的所有文档：


<!-- intro -->
##### SQL：

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
        "table":"test",
        "query":
        {
            "match": { "*": "test document" }
        }
    }'
```

* `query`中的JSON包含一个用于全文搜索的子句；它与[JSON/update](../Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-HTTP-JSON)中的语法相同。

<!-- response JSON -->

``` json
{
    "table":"test",
    "deleted":2,
}
```
<!-- intro -->
##### PHP：

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

##### Python：

<!-- request Python -->
``` python
indexApi.delete({"table" : "test", "query": { "match": { "*": "test document" }}})
```

<!-- response Python -->
```python
{'deleted': 5, 'id': None, 'table': 'test', 'result': None}
```

<!-- intro -->

##### Python-asyncio：

<!-- request Python-asyncio -->
``` python
indexApi.delete({"table" : "test", "query": { "match": { "*": "test document" }}})
```

<!-- response Python-asyncio -->
```python
{'deleted': 5, 'id': None, 'table': 'test', 'result': None}
```

<!-- intro -->

##### Javascript：

<!-- request javascript -->
``` javascript
res = await indexApi.delete({"table" : "test", "query": { "match": { "*": "test document" }}});
```

<!-- response javascript -->
```javascript
{"table":"test","deleted":5}
```

<!-- intro -->

##### java：

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

##### C#：

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

##### Rust：

<!-- request Rust -->
``` rust
let match_expr = HashMap::new();
match_expr.insert("*".to_string(), serde_json::json!("test document"));
let query = SearchQuery {
     match: Some(serde_json::json!(match_expr).into()),
    ..Default::default()
};

let delete_req = DeleteDocumentRequest::new("test".to_string());
index_api.delete(delete_req).await;
```

<!-- response Rust -->
```rust
class DeleteResponse {
    index: test
    deleted: 5
    id: null
    result: null
}
```

<!-- intro -->

##### TypeScript：

<!-- request TypeScript -->
``` typescript
res = await indexApi.delete({
  index: 'test',
  query: { match: { '*': 'test document' } },
});
```

<!-- response TypeScript -->
```json
{"table":"test","deleted":5}
```

<!-- intro -->

##### Go：

<!-- request Go -->
```go
deleteRequest := manticoresearch.NewDeleteDocumentRequest("test")
matchExpr := map[string]interface{} {"*": "test document"}
deleteQuery := map[string]interface{} {"match": matchExpr }
deleteRequest.SetQuery(deleteQuery)
```

<!-- response Go -->
```json
{"table":"test","deleted":5}
```

<!-- end -->

<!-- example delete 3 -->
这里 - 从名为`test`的表中删除具有`id`等于1的文档：


<!-- intro -->
##### SQL：

<!-- request SQL -->

```sql
mysql> DELETE FROM TEST WHERE id=1;
Query OK, 1 rows affected (0.00 sec)
```

<!-- request JSON -->

``` json
POST /delete -d '
    {
        "table": "test",
        "id": 1
    }'
```

* `id`中的JSON是需要删除的行`id`。

<!-- response JSON -->

``` json
{
    "table": "test",
    "_id": 1,
    "found": true,
    "result": "deleted"
}
```
<!-- intro -->
##### PHP：

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

##### Python：

<!-- request Python -->
``` python
indexApi.delete({"table" : "test", "id" : 1})
```

<!-- response Python -->
```python
{'deleted': None, 'id': 1, 'table': 'test', 'result': 'deleted'}
```

<!-- intro -->

##### Python-asyncio：

<!-- request Python-asyncio -->
``` python
indexApi.delete({"table" : "test", "id" : 1})
```

<!-- response Python-asyncio -->
```python
{'deleted': None, 'id': 1, 'table': 'test', 'result': 'deleted'}
```

<!-- intro -->

##### Javascript：

<!-- request javascript -->
``` javascript
res = await indexApi.delete({"table" : "test", "id" : 1});
```

<!-- response javascript -->
```javascript
{"table":"test","_id":1,"result":"deleted"}
```

<!-- intro -->

##### java：

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

##### C#：

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

##### Rust：

<!-- request Rust -->
``` rust
let delete_req = DeleteDocumentRequest {
    table: "test".to_string(),
    id: serde_json::json!(1),
    ..Default::default(),
};

index_api.delete(delete_req).await;

```

<!-- response Rust -->
```rust
class DeleteResponse {
    index: test
    _id: 1
    result: deleted
}
```

<!-- intro -->

##### TypeScript：

<!-- request TypeScript -->
``` typescript
res = await indexApi.delete({ index: 'test', id: 1 });
```

<!-- response TypeScript -->
```json
{"table":"test","_id":1,"result":"deleted"}
```

<!-- intro -->

##### Go：

<!-- request Go -->
```go
deleteRequest := manticoresearch.NewDeleteDocumentRequest("test")
deleteRequest.SetId(1)
```

<!-- response Go -->
```json
{"table":"test","_id":1,"result":"deleted"}
```

<!-- end -->

<!-- example delete 4 -->
这里，从名为`test`的表中删除具有`id`匹配值的文档：

注意，带有`id=N`或`id IN (X,Y)`的形式的删除操作最快，因为它们在不执行搜索的情况下删除文档。
还请注意，响应中仅包含对应于`_id`字段的第一个被删除文档的id。

<!-- intro -->
##### SQL：

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
        "table":"test",
        "id": [1,2]
    }'
```

<!-- response JSON -->

``` json
    {
        "table":"test",
        "_id":1,
        "found":true,
        "result":"deleted"
    }
```

<!-- intro -->
##### PHP：

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
Manticore SQL 允许为`DELETE`语句使用复杂的条件。

例如，这里我们正在从名为`test`的表中删除匹配全文查询`test document`且具有属性`mva1`的值大于206或`mva1`值为100或103的文档：


<!-- intro -->
##### SQL：

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
这是从集群`cluster`的表`test`中删除文档的一个示例。请注意，我们必须提供集群名称属性以及表属性以从复制集群内的表中删除行：


<!-- intro -->
##### SQL：

<!-- request SQL -->

```sql
delete from cluster:test where id=100;
```

<!-- request JSON -->
``` json
POST /delete -d '
    {
      "cluster": "cluster",
      "table": "test",
      "id": 100
    }'
```
* `cluster`中的JSON是包含所需表的[复制集群](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster)的名称

<!-- intro -->
##### PHP：

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

##### Python：

<!-- request Python -->
``` python
indexApi.delete({"cluster":"cluster","table" : "test", "id" : 1})
```

<!-- response Python -->
```python
{'deleted': None, 'id': 1, 'table': 'test', 'result': 'deleted'}
```

<!-- intro -->

##### Python-asyncio：

<!-- request Python-asyncio -->
``` python
indexApi.delete({"cluster":"cluster","table" : "test", "id" : 1})
```

<!-- response Python-asyncio -->
```python
{'deleted': None, 'id': 1, 'table': 'test', 'result': 'deleted'}
```

<!-- intro -->

##### Javascript：

<!-- request javascript -->
``` javascript
indexApi.delete({"cluster":"cluster_1","table" : "test", "id" : 1})
```

<!-- response javascript -->
```javascript
{"table":"test","_id":1,"result":"deleted"}
```

<!-- intro -->

##### java：

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

##### C#：

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

##### Rust:

<!-- request Rust -->
``` rust
let delete_req = DeleteDocumentRequest {
    table: "test".to_string(),
    cluster: "cluster".to_string(),
    id: serde_json::json!(1),
    ..Default::default(),
};

index_api.delete(delete_req).await;

```

<!-- response Rust -->
```rust
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
{"table":"test","_id":1,"result":"deleted"}
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
{"table":"test","_id":1,"result":"deleted"}
```

<!-- end -->


## 批量删除

<!-- example bulk delete -->

你也可以使用 `/bulk` 端点，在一次调用中执行多个删除操作。此端点仅适用于 `Content-Type` 设置为 `application/x-ndjson` 的数据。数据应格式化为换行符分隔的 JSON（NDJSON）。本质上，这意味着每一行应恰好包含一个 JSON 语句，并以换行符 `\n` 和可能的 `\r` 结尾。


<!-- intro -->
##### JSON:

<!-- request JSON --Content-type=application/x-ndjson -->

```json
POST /bulk

{ "delete" : { "table" : "test", "id" : 1 } }
{ "delete" : { "table" : "test", "query": { "equals": { "int_data" : 20 } } } }
```

<!-- response JSON -->

```json
{
   "items":
   [
      {
         "bulk":
         {
            "table":"test",
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
            'table' => 'test',
            'id' => 1
        ]
    ],
    ['delete'=>[
            'table' => 'test',
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
            { "delete" : { "table" : "test", "id": 1 } }, \
            { "delete" : { "table" : "test", "query": { "equals": { "int_data": 20 } } } } ]
indexApi.bulk('\n'.join(map(json.dumps,docs)))
```

<!-- response Python -->
```python
{
    'error': None,
    'items': [{u'delete': {u'table': test', u'deleted': 2}}]
}

```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->
``` python
docs = [ \
            { "delete" : { "table" : "test", "id": 1 } }, \
            { "delete" : { "table" : "test", "query": { "equals": { "int_data": 20 } } } } ]
indexApi.bulk('\n'.join(map(json.dumps,docs)))
```

<!-- response Python-asyncio -->
```python
{
    'error': None,
    'items': [{u'delete': {u'table': test', u'deleted': 2}}]
}

```

<!-- intro -->
##### Javascript:

<!-- request javascript -->
``` javascript
docs = [
            { "delete" : { "table" : "test", "id": 1 } },
            { "delete" : { "table" : "test", "query": { "equals": { "int_data": 20 } } } } ];
res =  await indexApi.bulk(docs.map(e=>JSON.stringify(e)).join('\n'));
```

<!-- response javascript -->
```javascript
{"items":[{"delete":{"table":"test","deleted":2}}],"errors":false}

```

<!-- intro -->
##### java:

<!-- request Java -->
``` java
String   body = "{ "delete" : { "table" : "test", "id": 1 } } "+"\n"+
    "{ "delete" : { "table" : "test", "query": { "equals": { "int_data": 20 } } } }"+"\n";
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
string   body = "{ "delete" : { "table" : "test", "id": 1 } } "+"\n"+
    "{ "delete" : { "table" : "test", "query": { "equals": { "int_data": 20 } } } }"+"\n";
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
##### Rust:

<!-- request Rust -->
``` rust
let bulk_body = r#"{ "delete" : { "table" : "test", "id": 1 } }
    { "delete" : { "table" : "test", "query": { "equals": { "int_data": 20 } } } }"
"#;
index_api.bulk(bulk_body).await;
```

<!-- response Rust -->
```rust
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
  { "delete" : { "table" : "test", "id": 1 } },
  { "delete" : { "table" : "test", "query": { "equals": { "int_data": 20 } } } }
];
body = await indexApi.bulk(
  docs.map((e) => JSON.stringify(e)).join("\n")
);
res = await indexApi.bulk(body);
```

<!-- response TypeScript -->
```json
{"items":[{"delete":{"table":"test","deleted":2}}],"errors":false}

```

<!-- intro -->

##### Go:

<!-- request Go -->
```go
docs = []string {
  `{ "delete" : { "table" : "test", "id": 1 } }`,
  `{ "delete" : { "table" : "test", "query": { "equals": { "int_data": 20 } } } }`
]
body = strings.Join(docs, "\n")
resp, httpRes, err := manticoreclient.IndexAPI.Bulk(context.Background()).Body(body).Execute()
```

<!-- response Go -->
```json
{"items":[{"delete":{"table":"test","deleted":2}}],"errors":false}
```

<!-- end -->


<!-- end -->
<!-- proofread -->

