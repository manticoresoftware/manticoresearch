# Удаление документов

Удаление документов поддерживается только в [RT режиме](../../Read_this_first.md#Real-time-mode-vs-plain-mode) для следующих типов таблиц:
* [Реальное время](../Creating_a_table/Local_tables/Real-time_table.md) таблицы
* [Перколировать](../Creating_a_table/Local_tables/Percolate_table.md) таблицы
* Распределенные таблицы, которые содержат только RT таблицы в качестве локальных или удаленных агентов.

Вы можете удалить существующие документы из таблицы на основе их ID или определенных условий.

Также доступно [массовое удаление](../Data_creation_and_modification/Deleting_documents.md#Bulk-deletion) для удаления нескольких документов.

Удаление документов может быть выполнено как через SQL, так и через JSON интерфейсы.

Для SQL ответ на успешную операцию укажет количество удаленных строк.

Для JSON используется конечная точка `json/delete`. Сервер ответит JSON объектом, указывающим, была ли операция успешной и количество удаленных строк.

Рекомендуется использовать [очистку таблицы](../Emptying_a_table.md) вместо удаления для удаления всех документов из таблицы, так как это гораздо более быстрая операция.


<!-- example delete 2 -->
В этом примере мы удаляем все документы, которые соответствуют полнотекстовому запросу `test document` из таблицы с именем `test`:


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
        "table":"test",
        "query":
        {
            "match": { "*": "test document" }
        }
    }'
```

* `query` для JSON содержит условие для полнотекстового поиска; он имеет ту же синтаксическую структуру, что и в [JSON/update](../Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-HTTP-JSON).

<!-- response JSON -->

``` json
{
    "table":"test",
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
indexApi.delete({"table" : "test", "query": { "match": { "*": "test document" }}})
```

<!-- response Python -->
```python
{'deleted': 5, 'id': None, 'table': 'test', 'result': None}
```
<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.delete({"table" : "test", "query": { "match": { "*": "test document" }}});
```

<!-- response javascript -->
```javascript
{"table":"test","deleted":5}
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
{"table":"test","deleted":5}
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
{"table":"test","deleted":5}
```

<!-- end -->

<!-- example delete 3 -->
Здесь - удаление документа с `id`, равным 1, из таблицы с именем `test`:


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
        "table": "test",
        "id": 1
    }'
```
* `id` для JSON - это идентификатор строки `id`, которая должна быть удалена.

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
indexApi.delete({"table" : "test", "id" : 1})
```

<!-- response Python -->
```python
{'deleted': None, 'id': 1, 'table': 'test', 'result': 'deleted'}
```
<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
res = await indexApi.delete({"table" : "test", "id" : 1});
```

<!-- response javascript -->
```javascript
{"table":"test","_id":1,"result":"deleted"}
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
{"table":"test","_id":1,"result":"deleted"}
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
{"table":"test","_id":1,"result":"deleted"}
```

<!-- end -->

<!-- example delete 4 -->
Здесь удаляются документы с `id`, соответствующими значениям из таблицы с именем `test`:

Обратите внимание, что формы удаления с `id=N` или `id IN (X,Y)` являются самыми быстрыми, так как они удаляют документы без выполнения поиска.
Также обратите внимание, что ответ содержит только id первого удаленного документа в соответствующем поле `_id`.

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
Manticore SQL позволяет использовать сложные условия для оператора `DELETE`.

Например, здесь мы удаляем документы, которые соответствуют полнотекстовому запросу `test document` и имеют атрибут `mva1` со значением больше 206 или значения `mva1` 100 или 103 из таблицы с именем `test`:


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
Вот пример удаления документов в таблице `test` кластера `cluster`. Обратите внимание, что мы должны указать свойство имени кластера вместе со свойством таблицы для удаления строки из таблицы в кластере репликации:


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
      "table": "test",
      "id": 100
    }'
```
* `cluster` для JSON - это имя [кластера репликации](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster), который содержит нужную таблицу

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
indexApi.delete({"cluster":"cluster","table" : "test", "id" : 1})
```

<!-- response Python -->
```python
{'deleted': None, 'id': 1, 'table': 'test', 'result': 'deleted'}
```
<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
indexApi.delete({"cluster":"cluster_1","table" : "test", "id" : 1})
```

<!-- response javascript -->
```javascript
{"table":"test","_id":1,"result":"deleted"}
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


## Bulk deletion

<!-- example bulk delete -->

You can also perform multiple delete operations in a single call using the `/bulk` endpoint. This endpoint only works with data that has `Content-Type` set to `application/x-ndjson`. The data should be formatted as newline-delimited JSON (NDJSON). Essentially, this means that each line should contain exactly one JSON statement and end with a newline `\n` and, possibly, a `\r`.


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

