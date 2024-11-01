# 向实时表添加文档

> 如果您正在查找向普通表中添加文档的信息，请参阅[从外部存储中添加数据](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)的部分。

<!-- example insert -->

实时添加文档仅支持[实时表](../../Creating_a_table/Local_tables/Real-time_table.md)和[预过滤表](../../Creating_a_table/Local_tables/Percolate_table.md)。相应的 SQL 命令、HTTP 端点或客户端函数将新的行（文档）插入到提供了字段值的表中。添加文档之前不需要该表已存在。如果表不存在，Manticore 将尝试自动创建它。有关更多信息，请参阅[自动模式](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。

您可以插入单个文档或[多个文档](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-adding-documents)，为表中的所有字段或仅部分字段提供值。在这种情况下，其他字段将使用其默认值（标量类型为 0，文本类型为空字符串）填充。

`INSERT` 中当前不支持表达式，因此必须明确指定值。

可以省略 ID 字段/值，因为 RT 和 PQ 表支持[自动 ID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) 功能。您还可以使用 `0` 作为 id 值来强制自动生成 ID。具有重复 ID 的行不会被 `INSERT` 覆盖，您可以使用 [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md) 来实现该目的。

使用 HTTP JSON 协议时，您可以选择两种不同的请求格式：通用的 Manticore 格式和类似 Elasticsearch 的格式。两种格式在下面的示例中都有展示。

此外，使用 Manticore JSON 请求格式时，请注意 `doc` 节点是必需的，所有值都应在其中提供。


<!-- intro -->
##### SQL:
<!-- request SQL -->
通用语法：

```sql
INSERT INTO <table name> [(column, ...)]
VALUES (value, ...)
[, (...)]
```

```sql
INSERT INTO products(title,price) VALUES ('Crossbody Bag with Tassel', 19.85);
INSERT INTO products(title) VALUES ('Crossbody Bag with Tassel');
INSERT INTO products VALUES (0,'Yellow bag', 4.95);
```
<!-- response SQL -->

```sql
Query OK, 1 rows affected (0.00 sec)
Query OK, 1 rows affected (0.00 sec)
Query OK, 1 rows affected (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /insert
{
  "index":"products",
  "id":1,
  "doc":
  {
    "title" : "Crossbody Bag with Tassel",
    "price" : 19.85
  }
}

POST /insert
{
  "index":"products",
  "id":2,
  "doc":
  {
    "title" : "Crossbody Bag with Tassel"
  }
}

POST /insert
{
  "index":"products",
  "id":0,
  "doc":
  {
    "title" : "Yellow bag"
  }
}

```

<!-- response JSON -->

```json
{
  "_index": "products",
  "_id": 1,
  "created": true,
  "result": "created",
  "status": 201
}
{
  "_index": "products",
  "_id": 2,
  "created": true,
  "result": "created",
  "status": 201
}
{
  "_index": "products",
  "_id": 0,
  "created": true,
  "result": "created",
  "status": 201
}

```

<!-- intro -->
##### Elasticsearch:

<!-- request Elasticsearch -->

> 注意：`_create` 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果不起作用，请确保已安装 Buddy。

```json
POST /products/_create/3
{
  "title": "Yellow Bag with Tassel",
  "price": 19.85
}

POST /products/_create/
{
  "title": "Red Bag with Tassel",
  "price": 19.85
}

```
<!-- response Elasticsearch -->

```json
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
"result":"updated"
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->addDocuments([
        ['id' => 1, 'title' => 'Crossbody Bag with Tassel', 'price' => 19.85]
]);
$index->addDocuments([
        ['id' => 2, 'title' => 'Crossbody Bag with Tassel']
]);
$index->addDocuments([
        ['id' => 0, 'title' => 'Yellow bag']
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

``` python
indexApi.insert({"index" : "test", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}})
indexApi.insert({"index" : "test", "id" : 2, "doc" : {"title" : "Crossbody Bag with Tassel"}})
indexApi.insert({"index" : "test", "id" : 0, "doc" : {{"title" : "Yellow bag"}})
```
<!-- intro -->
##### Javascript:

<!-- request Javascript -->

``` javascript
res = await indexApi.insert({"index" : "test", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}});
res = await indexApi.insert({"index" : "test", "id" : 2, "doc" : {"title" : "Crossbody Bag with Tassel"}});
res = await indexApi.insert({"index" : "test", "id" : 0, "doc" : {{"title" : "Yellow bag"}});
```

<!-- intro -->
##### java:

<!-- request Java -->

``` java
InsertDocumentRequest newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","Crossbody Bag with Tassel");
    put("price",19.85);
}};
newdoc.index("products").id(1L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);

newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","Crossbody Bag with Tassel");
}};
newdoc.index("products").id(2L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);

newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","Yellow bag");
 }};
newdoc.index("products").id(0L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);

```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "Crossbody Bag with Tassel");
doc.Add("price", 19.85);
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", id: 1, doc: doc);
var sqlresult = indexApi.Insert(newdoc);

doc = new Dictionary<string, Object>();
doc.Add("title", "Crossbody Bag with Tassel");
newdoc = new InsertDocumentRequest(index: "products", id: 2, doc: doc);
sqlresult = indexApi.Insert(newdoc);

doc = new Dictionary<string, Object>();
doc.Add("title", "Yellow bag");
newdoc = new InsertDocumentRequest(index: "products", id: 0, doc: doc);
sqlresult = indexApi.Insert(newdoc);

```

<!-- end -->

## 自动模式

> 注意：自动模式需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果不起作用，请确保已安装 Buddy。

Manticore 具有自动创建表的机制，当插入查询中指定的表尚不存在时，此机制将自动激活。该机制默认启用。要禁用它，请在 Manticore 配置文件的 [Searchd](../../Server_settings/Searchd.md#auto_schema) 部分中将 `auto_schema = 0` 设置为禁用。

<!-- example auto-schema -->

默认情况下，`VALUES` 子句中的所有文本值都将视为 `text` 类型，除非值为有效的电子邮件地址，这时将视为 `string` 类型。

如果尝试插入具有不同且不兼容值类型的多个行，自动表创建将被取消，并返回错误消息。但是，如果这些值类型兼容，最终的字段类型将是能够容纳所有值的类型。可能发生的一些自动数据类型转换包括：

- mva -> mva64
- uint -> bigint -> float
- string -> text

此外，以下日期格式将被识别并转换为时间戳，而所有其他日期格式将被视为字符串：

- `%Y-%m-%dT%H:%M:%E*S%Z`
- `%Y-%m-%d'T'%H:%M:%S%Z`
- `%Y-%m-%dT%H:%M:%E*S`
- `%Y-%m-%dT%H:%M:%s`
- `%Y-%m-%dT%H:%M`
- `%Y-%m-%dT%H`

请注意，`/bulk` HTTP 端点不支持自动创建表（自动模式）。只有 `/_bulk`（类似 Elasticsearch 的 HTTP 端点）和 SQL 接口支持此功能。

<!-- intro -->

##### SQL:

<!-- request SQL -->

```sql
MySQL [(none)]> drop table if exists t; insert into t(i,f,t,s,j,b,m,mb) values(123,1.2,'text here','test@mail.com','{"a": 123}',1099511627776,(1,2),(1099511627776,1099511627777)); desc t; select * from t;
```

<!-- response SQL -->

```sql
--------------
drop table if exists t
--------------

Query OK, 0 rows affected (0.42 sec)

--------------
insert into t(i,f,t,j,b,m,mb) values(123,1.2,'text here','{"a": 123}',1099511627776,(1,2),(1099511627776,1099511627777))
--------------

Query OK, 1 row affected (0.00 sec)

--------------
desc t
--------------

+-------+--------+----------------+
| Field | Type   | Properties     |
+-------+--------+----------------+
| id    | bigint |                |
| t     | text   | indexed stored |
| s     | string |                |
| j     | json   |                |
| i     | uint   |                |
| b     | bigint |                |
| f     | float  |                |
| m     | mva    |                |
| mb    | mva64  |                |
+-------+--------+----------------+
8 rows in set (0.00 sec)

--------------
select * from t
--------------

+---------------------+------+---------------+----------+------+-----------------------------+-----------+---------------+------------+
| id                  | i    | b             | f        | m    | mb                          | t         | s             | j          |
+---------------------+------+---------------+----------+------+-----------------------------+-----------+---------------+------------+
| 5045949922868723723 |  123 | 1099511627776 | 1.200000 | 1,2  | 1099511627776,1099511627777 | text here | test@mail.com | {"a": 123} |
+---------------------+------+---------------+----------+------+-----------------------------+-----------+---------------+------------+
1 row in set (0.00 sec)
```

<!-- request JSON -->

```json
POST /insert  -d
{
 "index":"t",
 "id": 2,
 "doc":
 {
   "i" : 123,
   "f" : 1.23,
   "t": "text here",
   "s": "test@mail.com",
   "j": {"a": 123},
   "b": 1099511627776,
   "m": [1,2],
   "mb": [1099511627776,1099511627777]
 }
}
```

<!-- response JSON -->

```json
{"_index":"t","_id":2,"created":true,"result":"created","status":201}
```

<!-- end -->

## 自动 ID
<!-- example autoid -->

Manticore 为插入或替换到实时表或 [Percolate 表](../../Creating_a_table/Local_tables/Percolate_table.md) 的文档提供自动 ID 生成功能。此生成器为文档生成唯一 ID，并提供一定的保证，但它不应被视为自增 ID。

在以下条件下，生成的 ID 值可以确保唯一性：

- 当前服务器的 [server_id](../../Server_settings/Searchd.md#server_id) 值在 0 到 127 的范围内，并且在集群中的节点之间是唯一的，或者它使用从 MAC 地址生成的默认值作为种子
- 在 Manticore 节点的服务器重启之间，系统时间没有变化
- 在搜索服务器重启之间，自动 ID 每秒生成次数少于 1600 万次

自动 ID 生成器为文档 ID 生成 64 位整数，并使用以下方案：

- 第 0 到 23 位形成一个计数器，每次调用自动 ID 生成器时计数器递增
- 第 24 到 55 位表示服务器启动的 Unix 时间戳
- 第 56 到 63 位对应于 server_id

该方案确保生成的 ID 在集群中的所有节点之间都是唯一的，并且插入到不同集群节点中的数据不会在节点之间产生冲突。

因此，自动 ID 生成器生成的第一个 ID 不是 1，而是一个较大的数字。此外，如果在调用之间对其他表进行了插入操作，由于 ID 生成器是服务器的唯一生成器并在所有表之间共享，插入到表中的文档流可能会有非连续的 ID 值。

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
INSERT INTO products(title,price) VALUES ('Crossbody Bag with Tassel', 19.85);
INSERT INTO products VALUES (0,'Yello bag', 4.95);
select * from products;
```
<!-- response SQL -->
```
+---------------------+-----------+---------------------------+
| id                  | price     | title                     |
+---------------------+-----------+---------------------------+
| 1657860156022587404 | 19.850000 | Crossbody Bag with Tassel |
| 1657860156022587405 |  4.950000 | Yello bag                 |
+---------------------+-----------+---------------------------+
```
<!-- intro -->
##### JSON:
<!-- request JSON -->

```json
POST /insert
{
  "index":"products",
  "id":0,
  "doc":
  {
    "title" : "Yellow bag"
  }
}

GET /search
{
  "index":"products",
  "query":{
    "query_string":""
  }
}
```
<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "_id": 1657860156022587406,
        "_score": 1,
        "_source": {
          "price": 0,
          "title": "Yellow bag"
        }
      }
    ]
  }
}
```
<!-- intro -->
##### PHP:
<!-- request PHP -->

```php
$index->addDocuments([
        ['id' => 0, 'title' => 'Yellow bag']
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
indexApi.insert({"index" : "products", "id" : 0, "doc" : {"title" : "Yellow bag"}})
```
<!-- intro -->

##### Javascript:

<!-- request Javascript -->

```javascript
res = await indexApi.insert({"index" : "products", "id" : 0, "doc" : {"title" : "Yellow bag"}});
```

<!-- intro -->
##### java:

<!-- request Java -->

``` java
newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","Yellow bag");
 }};
newdoc.index("products").id(0L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);
```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "Yellow bag");
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", id: 0, doc: doc);
var sqlresult = indexApi.Insert(newdoc);
```
<!-- end -->

<!-- example bulk_insert -->
## 批量添加文档
你不仅可以向实时表插入单个文档，还可以根据需要插入任意数量的文档。批量插入成千上万条文档是完全可以的。然而，需要注意以下几点：

- 批量越大，每次插入操作的延迟越高
- 批量越大，预期的索引速度越快
- 你可能需要增加 [max_packet_size](../../Server_settings/Searchd.md#max_packet_size) 的值以支持更大的批量插入
- 通常情况下，每次批量插入操作被视为单个 [事务](../../Data_creation_and_modification/Transactions.md)，具有原子性保证，因此所有新文档要么一次性添加到表中，要么在失败时全部不添加。关于空行或切换到其他表的更多详情，请参见 "JSON" 示例。

需要注意的是，`/bulk` HTTP 端点不支持表的自动创建（自动模式）。只有 `/_bulk`（类似 Elasticsearch 的 HTTP 端点）和 SQL 接口支持此功能。`/_bulk`（类似 Elasticsearch 的 HTTP 端点）允许表名包括集群名称，格式为 `cluster_name:table_name`。

#### /bulk 中的分块传输

`/bulk`（Manticore 模式）端点支持 [分块传输编码](https://en.wikipedia.org/wiki/Chunked_transfer_encoding)。你可以使用它来传输大批量数据。它的优势包括：

- 降低峰值内存使用，减少 OOM（内存不足）风险
- 减少响应时间
- 允许你绕过 [max_packet_size](../Server_settings/Searchd.md#max_packet_size) 的限制，并传输远大于 `max_packet_size`（128MB）的批量数据，例如一次传输 1GB 的数据。

<!-- intro -->
### 批量插入示例

##### SQL:
<!-- request SQL -->
对于批量插入，只需在 `VALUES()` 后提供更多的文档。其语法为：

```sql
INSERT INTO <table name>[(column1, column2, ...)] VALUES ()[,(value1,[value2, ...])]
```

可选的列名列表允许你显式地为表中的某些列指定值。所有其他列将使用其默认值填充（标量类型的默认值为 0，字符串类型的默认值为空字符串）。

例如：

```sql
INSERT INTO products(title,price) VALUES ('Crossbody Bag with Tassel', 19.85), ('microfiber sheet set', 19.99), ('Pet Hair Remover Glove', 7.99);
```
<!-- response SQL -->

```sql
Query OK, 3 rows affected (0.01 sec)
```

当前在 `INSERT` 中不支持表达式，所有值应明确指定。

<!-- intro -->
##### JSON:
<!-- request JSON -->

语法与[插入单个文档](../../Quick_start_guide.md#Add-documents)基本相同。只需为每个文档提供多行，并使用 `/bulk` 端点而不是 `/insert`。将每个文档封装在 "insert" 节点中。注意，它还需要以下条件：

- `Content-Type: application/x-ndjson`
- 数据应格式化为换行分隔的 JSON (NDJSON)。本质上，这意味着每行应包含一个 JSON 语句，并以换行符 `\n`（可能还包括 `\r`）结束。

`/bulk` 端点支持 'insert'、'replace'、'delete' 和 'update' 查询。请记住，您可以将操作指向多个表，但事务仅支持单个表。如果指定多个表，Manticore 将把指向一个表的操作收集到一个事务中。当表发生变化时，它会提交收集的操作并在新表上启动一个新事务。空行分隔批次也会导致提交前一个批次并启动新事务。

在 `/bulk` 请求的响应中，您可以找到以下字段：

- "errors"：显示是否发生任何错误（true/false）
- "error"：描述发生的错误
- "current_line"：执行停止（或失败）的行号；空行，包括第一个空行，也会被计入
- "skipped_lines"：从 `current_line` 开始向后移动的未提交行数

```json
POST /bulk
-H "Content-Type: application/x-ndjson" -d '
{"insert": {"index":"products", "id":1, "doc":  {"title":"Crossbody Bag with Tassel","price" : 19.85}}}
{"insert":{"index":"products", "id":2, "doc":  {"title":"microfiber sheet set","price" : 19.99}}}
'

POST /bulk
-H "Content-Type: application/x-ndjson" -d '
{"insert":{"index":"test1","id":21,"doc":{"int_col":1,"price":1.1,"title":"bulk doc one"}}}
{"insert":{"index":"test1","id":22,"doc":{"int_col":2,"price":2.2,"title":"bulk doc two"}}}

{"insert":{"index":"test1","id":23,"doc":{"int_col":3,"price":3.3,"title":"bulk doc three"}}}
{"insert":{"index":"test2","id":24,"doc":{"int_col":4,"price":4.4,"title":"bulk doc four"}}}
{"insert":{"index":"test2","id":25,"doc":{"int_col":5,"price":5.5,"title":"bulk doc five"}}}
'
```

<!-- response JSON -->
```json
{
  "items": [
    {
      "bulk": {
        "_index": "products",
        "_id": 2,
        "created": 2,
        "deleted": 0,
        "updated": 0,
        "result": "created",
        "status": 201
      }
    }
  ],
  "current_line": 4,
  "skipped_lines": 0,
  "errors": false,
  "error": ""
}

{
  "items": [
    {
      "bulk": {
        "_index": "test1",
        "_id": 22,
        "created": 2,
        "deleted": 0,
        "updated": 0,
        "result": "created",
        "status": 201
      }
    },
    {
      "bulk": {
        "_index": "test1",
        "_id": 23,
        "created": 1,
        "deleted": 0,
        "updated": 0,
        "result": "created",
        "status": 201
      }
    },
    {
      "bulk": {
        "_index": "test2",
        "_id": 25,
        "created": 2,
        "deleted": 0,
        "updated": 0,
        "result": "created",
        "status": 201
      }
    }
  ],
  "current_line": 8,
  "skipped_lines": 0,
  "errors": false,
  "error": ""
}
```

<!-- request Elasticsearch -->

> 注意：自动模式需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果不起作用，请确保已安装 Buddy。

```json
POST /_bulk
-H "Content-Type: application/x-ndjson" -d '
{ "index" : { "_index" : "products" } }
{ "title" : "Yellow Bag", "price": 12 }
{ "create" : { "_index" : "products" } }
{ "title" : "Red Bag", "price": 12.5, "id": 3 }
'
```
<!-- response Elasticsearch -->
```json
{
  "items": [
    {
      "index": {
        "_index": "products",
        "_type": "doc",
        "_id": 0,
        "_version": 1,
        "result": "created",
        "_shards": {
          "total": 1,
          "successful": 1,
          "failed": 0
        },
        "_seq_no": 0,
        "_primary_term": 1,
        "status": 201
      }
    },
    {
      "create": {
        "_index": "products",
        "_type": "doc",
        "_id": 3,
        "_version": 1,
        "result": "created",
        "_shards": {
          "total": 1,
          "successful": 1,
          "failed": 0
        },
        "_seq_no": 0,
        "_primary_term": 1,
        "status": 201
      }
    }
  ],
  "errors": false,
  "took": 1
}
```

<!-- intro -->
##### PHP:
<!-- request PHP -->
使用方法 addDocuments():

```php
$index->addDocuments([
        ['id' => 1, 'title' => 'Crossbody Bag with Tassel', 'price' => 19.85],
        ['id' => 2, 'title' => 'microfiber sheet set', 'price' => 19.99],
        ['id' => 3, 'title' => 'Pet Hair Remover Glove', 'price' => 7.99]
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
docs = [ \
    {"insert": {"index" : "products", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}}, \
    {"insert": {"index" : "products", "id" : 2, "doc" : {"title" : "microfiber sheet set", "price" : 19.99}}}, \
    {"insert": {"index" : "products", "id" : 3, "doc" : {"title" : "CPet Hair Remover Glove", "price" : 7.99}}}
]
res = indexApi.bulk('\n'.join(map(json.dumps,docs)))
```


<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
let docs = [
    {"insert": {"index" : "products", "id" : 3, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}},
    {"insert": {"index" : "products", "id" : 4, "doc" : {"title" : "microfiber sheet set", "price" : 19.99}}},
    {"insert": {"index" : "products", "id" : 5, "doc" : {"title" : "CPet Hair Remover Glove", "price" : 7.99}}}
];
res =  await indexApi.bulk(docs.map(e=>JSON.stringify(e)).join('\n'));
```
<!-- intro -->
##### java:

<!-- request Java -->

``` java
String body = "{\"insert\": {\"index\" : \"products\", \"id\" : 1, \"doc\" : {\"title\" : \"Crossbody Bag with Tassel\", \"price\" : 19.85}}}"+"\n"+
    "{\"insert\": {\"index\" : \"products\", \"id\" : 4, \"doc\" : {\"title\" : \"microfiber sheet set\", \"price\" : 19.99}}}"+"\n"+
    "{\"insert\": {\"index\" : \"products\", \"id\" : 5, \"doc\" : {\"title\" : \"CPet Hair Remover Glove\", \"price\" : 7.99}}}"+"\n";         
BulkResponse bulkresult = indexApi.bulk(body);
```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
string body = "{\"insert\": {\"index\" : \"products\", \"id\" : 1, \"doc\" : {\"title\" : \"Crossbody Bag with Tassel\", \"price\" : 19.85}}}"+"\n"+
    "{\"insert\": {\"index\" : \"products\", \"id\" : 4, \"doc\" : {\"title\" : \"microfiber sheet set\", \"price\" : 19.99}}}"+"\n"+
    "{\"insert\": {\"index\" : \"products\", \"id\" : 5, \"doc\" : {\"title\" : \"CPet Hair Remover Glove\", \"price\" : 7.99}}}"+"\n";                                 
BulkResponse bulkresult = indexApi.Bulk(string.Join("\n", docs));
```

<!-- end -->
<!-- example MVA_insert -->
## 插入多值属性 (MVA) 值

多值属性 (MVA) 以数字数组的形式插入。
<!-- intro -->

### 示例
##### SQL
<!-- request SQL -->
```

INSERT INTO products(title, sizes) VALUES('shoes', (40,41,42,43));
```
<!-- intro -->
##### JSON
<!-- request JSON -->
```json

POST /insert
{
  "index":"products",
  "id":1,
  "doc":
  {
    "title" : "shoes",
    "sizes" : [40, 41, 42, 43]
  }
}
```

<!-- intro -->
##### Elasticsearch:
<!-- request Elasticsearch -->

```json
POST /products/_create/1
{
  "title": "shoes",
  "sizes" : [40, 41, 42, 43]
}
```

或者，作为替代方案
```json
POST /products/_doc/
{
  "title": "shoes",
  "sizes" : [40, 41, 42, 43]
}
```


<!-- intro -->
##### PHP
<!-- request PHP -->

```php
$index->addDocument(
  ['title' => 'shoes', 'sizes' => [40,41,42,43]],
  1
);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
indexApi.insert({"index" : "products", "id" : 0, "doc" : {"title" : "Yellow bag","sizes":[40,41,42,43]}})
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await indexApi.insert({"index" : "products", "id" : 0, "doc" : {"title" : "Yellow bag","sizes":[40,41,42,43]}});
```


<!-- intro -->
##### java:

<!-- request Java -->

``` java
newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","Yellow bag");
    put("sizes",new int[]{40,41,42,43});
 }};
newdoc.index("products").id(0L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);
```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "Yellow bag");
doc.Add("sizes", new List<Object> {40,41,42,43});
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", id: 0, doc: doc);
var sqlresult = indexApi.Insert(newdoc);
```
<!-- end -->

<!-- example JSON_insert -->
## 插入 JSON
JSON value can be inserted as an [escaped](../../Searching/Full_text_matching/Escaping.md) string (via SQL or JSON) or as a JSON object (via the JSON interface).

<!-- intro -->
### Examples
##### SQL
<!-- request SQL -->
```

INSERT INTO products VALUES (1, 'shoes', '{"size": 41, "color": "red"}');
```
<!-- intro -->
##### JSON
<!-- request JSON -->
JSON value can be inserted as a JSON object
```json
POST /insert
{
  "index":"products",
  "id":1,
  "doc":
  {
    "title" : "shoes",
    "meta" : {
      "size": 41,
      "color": "red"
    }
  }
}
```

JSON value can be also inserted as a string containing escaped JSON:
```json
POST /insert
{
  "index":"products",
  "id":1,
  "doc":
  {
    "title" : "shoes",
    "meta" : "{\"size\": 41, \"color\": \"red\"}"
  }
}
```

<!-- intro -->
##### Elasticsearch:
<!-- request Elasticsearch -->

```json
POST /products/_create/1
{
  "title": "shoes",
  "meta" : {
    "size": 41,
    "color": "red"
  }
}
```

或者，作为替代方案
```json
POST /products/_doc/
{
  "title": "shoes",
  "meta" : {
    "size": 41,
    "color": "red"
  }
}
```


<!-- intro -->
##### PHP
将 JSON 视为字符串：
<!-- request PHP -->

```php
$index->addDocument(
  ['title' => 'shoes', 'meta' => '{"size": 41, "color": "red"}'],
  1
);
```
<!-- intro -->
##### Python:

<!-- request Python -->
``` python
indexApi = api = manticoresearch.IndexApi(client)
indexApi.insert({"index" : "products", "id" : 0, "doc" : {"title" : "Yellow bag","meta":'{"size": 41, "color": "red"}'}})
```
<!-- intro -->
##### Javascript:

<!-- request Javascript -->
```javascript

res = await indexApi.insert({"index" : "products", "id" : 0, "doc" : {"title" : "Yellow bag","meta":'{"size": 41, "color": "red"}'}});
```
<!-- intro -->
##### java:

<!-- request Java -->

``` java
newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","Yellow bag");
    put("meta",
        new HashMap<String,Object>(){{
            put("size",41);
            put("color","red");
        }});
 }};
newdoc.index("products").id(0L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);
```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
Dictionary<string, Object> meta = new Dictionary<string, Object>();
meta.Add("size", 41);
meta.Add("color", "red");
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "Yellow bag");
doc.Add("meta", meta);
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", id: 0, doc: doc);
var sqlresult = indexApi.Insert(newdoc);
```

<!-- end -->

<!-- proofread -->
