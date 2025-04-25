# 将文档添加到实时表中

> 如果您正在寻找有关将文档添加到普通表的信息，请参阅[从外部存储添加数据](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)章节。

<!-- example insert -->
仅支持将文档实时添加到[实时](../../Creating_a_table/Local_tables/Real-time_table.md)和[渗透](../../Creating_a_table/Local_tables/Percolate_table.md)表中。相应的SQL命令、HTTP端点或客户端函数将使用提供的字段值向表中插入新行（文档）。在添加文档之前，不必确保表已经存在。如果表不存在，Manticore将尝试自动创建它。有关更多信息，请参见[自动模式](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。

您可以插入单个或[多个文档](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-adding-documents)，为表的所有字段或仅部分字段提供值。在这种情况下，其他字段将填充为其默认值（标量类型为0，文本类型为空字符串）。

当前不支持在`INSERT`中使用表达式，因此必须明确指定值。

ID字段/值可以省略，因为RT和PQ表支持[自动ID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID)功能。您还可以使用`0`作为ID值来强制自动ID生成。具有重复ID的行将不会被`INSERT`覆盖。相反，您可以使用[REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md)来实现这一目的。

使用HTTP JSON协议时，您可以选择两种不同的请求格式：常见的Manticore格式和类似Elasticsearch的格式。两种格式在下面的示例中进行了演示。

此外，在使用Manticore JSON请求格式时，请记住`doc`节点是必需的，所有值应在其中提供。


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
  "table":"products",
  "id":1,
  "doc":
  {
    "title" : "Crossbody Bag with Tassel",
    "price" : 19.85
  }
}

POST /insert
{
  "table":"products",
  "id":2,
  "doc":
  {
    "title" : "Crossbody Bag with Tassel"
  }
}

POST /insert
{
  "table":"products",
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
  "table": "products",
  "_id": 1,
  "created": true,
  "result": "created",
  "status": 201
}
{
  "table": "products",
  "_id": 2,
  "created": true,
  "result": "created",
  "status": 201
}
{
  "table": "products",
  "_id": 1657860156022587406,
  "created": true,
  "result": "created",
  "status": 201
}

```

<!-- intro -->
##### Elasticsearch:

<!-- request Elasticsearch -->

> 注意：`_create`需要[Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保Buddy已安装。

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
"_id":2235747273424240642,
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
indexApi.insert({"table" : "test", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}})
indexApi.insert({"table" : "test", "id" : 2, "doc" : {"title" : "Crossbody Bag with Tassel"}})
indexApi.insert({"table" : "test", "id" : 0, "doc" : {{"title" : "Yellow bag"}})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

``` python
await indexApi.insert({"table" : "test", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}})
await indexApi.insert({"table" : "test", "id" : 2, "doc" : {"title" : "Crossbody Bag with Tassel"}})
await indexApi.insert({"table" : "test", "id" : 0, "doc" : {{"title" : "Yellow bag"}})
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

``` javascript
res = await indexApi.insert({"table" : "test", "id" : 1, "doc" : {"title" : "斜挎包", "price" : 19.85}});
res = await indexApi.insert({"table" : "test", "id" : 2, "doc" : {"title" : "斜挎包"}});
res = await indexApi.insert({"table" : "test", "id" : 0, "doc" : {{"title" : "黄色包"}});
```

<!-- intro -->
##### java:

<!-- request Java -->

``` java
InsertDocumentRequest newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","斜挎包");
    put("price",19.85);
}};
newdoc.index("products").id(1L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);

newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","斜挎包");
}};
newdoc.index("products").id(2L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);

newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","黄色包");
 }};
newdoc.index("products").id(0L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);

```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "斜挎包");
doc.Add("price", 19.85);
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", id: 1, doc: doc);
var sqlresult = indexApi.Insert(newdoc);

doc = new Dictionary<string, Object>();
doc.Add("title", "斜挎包");
newdoc = new InsertDocumentRequest(index: "products", id: 2, doc: doc);
sqlresult = indexApi.Insert(newdoc);

doc = new Dictionary<string, Object>();
doc.Add("title", "黄色包");
newdoc = new InsertDocumentRequest(index: "products", id: 0, doc: doc);
sqlresult = indexApi.Insert(newdoc);

```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let mut doc = HashMap::new();
doc.insert("title".to_string(), serde_json::json!("斜挎包"));
doc.insert("price".to_string(), serde_json::json!(19.85));
let mut insert_req = InsertDocumentRequest {
    table: serde_json::json!("products"),
    doc: serde_json::json!(doc),
    id: serde_json::json!(1),
    ..Default::default(),
};
let mut insert_res = index_api.insert(insert_req).await;

doc = HashMap::new();
doc.insert("title".to_string(), serde_json::json!("斜挎包"));
insert_req = InsertDocumentRequest {
    table: serde_json::json!("products"),
    doc: serde_json::json!(doc),
    id: serde_json::json!(2),
    ..Default::default(),
};
insert_res = index_api.insert(insert_req).await;

doc = HashMap::new();
doc.insert("title".to_string(), serde_json::json!("黄色包"));
insert_req = InsertDocumentRequest {
    table: serde_json::json!("products"),
    doc: serde_json::json!(doc),
    id: serde_json::json!(0),
    ..Default::default(),
};
insert_res = index_api.insert(insert_req).await;
```

<!-- end -->

## 自动模式

> 注意: 自动模式需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不工作，请确保已经安装Buddy。

Manticore提供了一个自动表创建机制，当插入查询中指定的表尚不存在时会激活。该机制默认启用。要禁用它，请在Manticore配置文件的[Searchd](../../Server_settings/Searchd.md#auto_schema)部分中设置`auto_schema = 0`。

<!-- example auto-schema -->

默认情况下，`VALUES`子句中的所有文本值被视为`text`类型，除了表示有效电子邮件地址的值，它们被视为`string`类型。

如果您尝试在同一字段上插入多个具有不同不兼容值类型的行，则自动表创建将被取消，并且将返回错误消息。但是，如果不同的值类型兼容，则结果字段类型将是可以容纳所有值的类型。一些可能发生的自动数据类型转换包括：
* mva -> mva64
* uint -> bigint -> float（这可能导致一些精度损失）
* string -> text

此外，以下日期格式将被识别并转换为时间戳，而所有其他日期格式将被视为字符串：
- `%Y-%m-%dT%H:%M:%E*S%Z`
- `%Y-%m-%d'T'%H:%M:%S%Z`
- `%Y-%m-%dT%H:%M:%E*S`
- `%Y-%m-%dT%H:%M:%s`
- `%Y-%m-%dT%H:%M`
- `%Y-%m-%dT%H`


请记住，`/bulk` HTTP端点不支持自动表创建（自动模式）。只有`/_bulk`（类似于Elasticsearch）HTTP端点和SQL接口支持此功能。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
MySQL [(none)]> drop table if exists t; insert into t(i,f,t,s,j,b,m,mb) values(123,1.2,'这里的文本','test@mail.com','{"a": 123}',1099511627776,(1,2),(1099511627776,1099511627777)); desc t; select * from t;
```

<!-- response SQL -->

```sql
--------------
drop table if exists t
--------------

Query OK, 0 rows affected (0.42 sec)

--------------
insert into t(i,f,t,j,b,m,mb) values(123,1.2,'这里的文本','{"a": 123}',1099511627776,(1,2),(1099511627776,1099511627777))
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
8 行在集合中 (0.00 秒)

--------------
select * from t
--------------

+---------------------+------+---------------+----------+------+-----------------------------+-----------+---------------+------------+
| id                  | i    | b             | f        | m    | mb                          | t         | s             | j          |
+---------------------+------+---------------+----------+------+-----------------------------+-----------+---------------+------------+
| 5045949922868723723 |  123 | 1099511627776 | 1.200000 | 1,2  | 1099511627776,1099511627777 | text here | test@mail.com | {"a": 123} |
+---------------------+------+---------------+----------+------+-----------------------------+-----------+---------------+------------+
1 行在集合中 (0.00 秒)
```

<!-- request JSON -->

```json
POST /insert  -d
{
 "table":"t",
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
{"table":"t","_id":2,"created":true,"result":"created","status":201}
```

<!-- end -->

## 自动 ID
<!-- example autoid -->
Manticore 为插入或替换到实时或 [Percolate 表](../../Creating_a_table/Local_tables/Percolate_table.md) 的文档的列 ID 提供自动 ID 生成功能。生成器为文档生成唯一 ID 并有一些保证，但不应被视为自增 ID。

生成的 ID 值在以下条件下保证唯一：
* 当前服务器的 [server_id](../../Server_settings/Searchd.md#server_id) 值在 0 到 127 范围内，并在集群中的节点之间是唯一的，或者它使用从 MAC 地址生成的默认值作为种子
* 在服务器重启之间，Manticore 节点的系统时间没有变化
* 在搜索服务器重启之间，自动 ID 的生成次数少于 1600 万次

自动 ID 生成器为文档 ID 创建 64 位整数，并使用以下方案：
* 位 0 到 23 形成一个计数器，在每次调用自动 ID 生成器时递增
* 位 24 到 55 表示服务器启动的 Unix 时间戳
* 位 56 到 63 对应于 server_id

此方案确保生成的 ID 在集群中的所有节点中是唯一的，并且插入到不同集群节点的数据不会在节点之间产生冲突。

因此，生成器用于自动 ID 的第一个 ID 不是 1，而是一个更大的数字。此外，插入到表中的文档流可能具有非顺序的 ID 值，如果在调用之间对其他表进行插入，因为 ID 生成器在服务器中是独一无二且在所有表之间共享的。

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
  "table":"products",
  "id":0,
  "doc":
  {
    "title" : "Yellow bag"
  }
}

GET /search
{
  "table":"products",
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
indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "Yellow bag"}})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "Yellow bag"}})
```

<!-- intro -->

##### Javascript:

<!-- request Javascript -->

```javascript
res = await indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "Yellow bag"}});
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

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let doc = HashMap::new();
doc.insert("title".to_string(), serde_json::json!("Yellow bag"));
let insert_req = InsertDocumentRequest {
    table: serde_json::json!("products"),
    doc: serde_json::json!(doc),
    id: serde_json::json!(0),
    ..Default::default(),
};
let insert_res = index_api.insert(insert_req).await;
```

<!-- end -->

<!-- example call -->
### UUID_SHORT 多ID生成

```sql
CALL UUID_SHORT(N)
```

`CALL UUID_SHORT(N)` 语句允许在一次调用中生成 N 个独特的 64 位 ID，而无需插入任何文档。这在你需要在 Manticore 中预生成 ID 以供其他系统或存储解决方案使用时特别有用。例如，你可以在 Manticore 中生成自动 ID，然后在另一个数据库、应用程序或工作流程中使用它们，确保在不同环境中具有一致且唯一的标识符。

<!-- intro -->
##### 示例：
<!-- request Example -->

```sql
CALL UUID_SHORT(3)
```
<!-- response SQL -->
```
+---------------------+
| uuid_short()        |
+---------------------+
| 1227930988733973183 |
| 1227930988733973184 |
| 1227930988733973185 |
+---------------------+
```
<!-- end -->

<!-- example bulk_insert -->
## 批量添加文档
你可以将不止一个文档插入实时表，也可以插入任意数量的文档。将数万个文档批量插入实时表是完全可以的。然而，重要的是要记住以下几点：
* 批量越大，每次插入操作的延迟越高
* 批量越大，你可以期望的索引速度越高
* 你可能需要增加 [max_packet_size](../../Server_settings/Searchd.md#max_packet_size) 值，以允许更大的批量
* 通常情况下，每个批量插入操作被视为一个具有原子性保证的单个 [事务](../../Data_creation_and_modification/Transactions.md)，因此要么所有新文档同时出现在表中，要么在发生故障时没有文档被添加。有关空行或切换到另一个表的更多细节，请参见“JSON”示例。

请注意，`/bulk` HTTP 端点不支持自动创建表（自动模式）。只有 `/_bulk`（类似 Elasticsearch）HTTP 端点和 SQL 接口支持此功能。`/_bulk`（类似 Elasticsearch）HTTP 端点允许表名以 `cluster_name:table_name` 格式包括集群名称。

`/_bulk` 端点接受与 Elasticsearch 相同格式的文档 IDs，您还可以在文档本身中包含 `id`：
```json
{ "index": { "table" : "products", "_id" : "1" } }
{ "title" : "带流苏的斜挎包", "price": 19.85 }
```

或

```json
{ "index": { "table" : "products" } }
{ "title" : "带流苏的斜挎包", "price": 19.85, "id": "1" }
```

#### 在 /bulk 中使用分块传输
`/bulk` (Manticore 模式) 端点支持 [Chunked transfer encoding](https://en.wikipedia.org/wiki/Chunked_transfer_encoding)。您可以使用它来传输大批量的数据。它：
* 减少峰值内存使用，从而降低 OOM 的风险
* 减少响应时间
* 允许您绕过 [max_packet_size](../Server_settings/Searchd.md#max_packet_size) 并传输比 `max_packet_size` 的最大允许值（128MB）更大的批量数据，例如，一次传输 1GB。

<!-- intro -->
### 批量插入示例
##### SQL：
<!-- request SQL -->
对于批量插入，只需在 `VALUES()` 后用括号提供更多文档。语法如下：

```sql
INSERT INTO <table name>[(column1, column2, ...)] VALUES(value1[, value2 , ...]), (...)
```

可选的列名列表允许您显式指定表中某些列的值。所有其他列将填充默认值（标量类型为 0，字符串类型为空字符串）。

例如：

```sql
INSERT INTO products(title,price) VALUES ('带流苏的斜挎包', 19.85), ('超细纤维床单套装', 19.99), ('宠物毛发清除手套', 7.99);
```
<!-- response SQL -->

```sql
查询 OK，影响 3 行 (0.01 秒)
```

目前在 `INSERT` 中不支持表达式，值应显式指定。

<!-- intro -->
##### JSON：
<!-- request JSON -->
语法通常与 [插入单个文档](../../Quick_start_guide.md#Add-documents) 相同。只需提供更多行，每个文档各一行，并使用 `/bulk` 端点代替 `/insert`。将每个文档放在“insert”节点中。请注意，它还要求：
* `Content-Type: application/x-ndjson`
* 数据应格式化为换行分隔的 JSON (NDJSON)。本质上，这意味着每一行应包含正好一个 JSON 语句，并以换行符 `
` 和可能的 `` 结尾。

`/bulk` 端点支持 'insert'、'replace'、'delete' 和 'update' 查询。请记住，您可以指向多个表进行操作，但事务仅适用于单个表。如果您指定了多个，Manticore 会将定向到一个表的操作聚集为一个事务。当表发生变化时，它会提交收集的操作并在新表上启动新的事务。分隔批次的空行也会导致提交上一个批次并启动新的事务。

在 `/bulk` 请求的响应中，您可以找到以下字段：
* "errors": 显示是否发生了任何错误（true/false）
* "error": 描述发生的错误
* "current_line": 执行停止（或失败）时的行号；空行，包括第一行空行，也会被计算在内
* "skipped_lines": 从 `current_line` 开始并向后移动的未提交行的数量

```json
POST /bulk
-H "Content-Type: application/x-ndjson" -d '
{"insert": {"table":"products", "id":1, "doc":  {"title":"带流苏的斜挎包","price" : 19.85}}}
{"insert":{"table":"products", "id":2, "doc":  {"title":"超细纤维床单套装","price" : 19.99}}}
'

POST /bulk
-H "Content-Type: application/x-ndjson" -d '
{"insert":{"table":"test1","id":21,"doc":{"int_col":1,"price":1.1,"title":"bulk doc one"}}}
{"insert":{"table":"test1","id":22,"doc":{"int_col":2,"price":2.2,"title":"bulk doc two"}}}

{"insert":{"table":"test1","id":23,"doc":{"int_col":3,"price":3.3,"title":"bulk doc three"}}}
{"insert":{"table":"test2","id":24,"doc":{"int_col":4,"price":4.4,"title":"bulk doc four"}}}
{"insert":{"table":"test2","id":25,"doc":{"int_col":5,"price":5.5,"title":"bulk doc five"}}}
'
```

<!-- response JSON -->
```json
{
  "items": [
    {
      "bulk": {
        "table": "products",
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
        "table": "test1",
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
        "table": "test1",
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
        "table": "test2",
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

> NOTE: `_bulk` requires [Manticore Buddy](../Installation/Manticore_Buddy.md) if the table doesn't exist yet. If it doesn't work, make sure Buddy is installed.

```json
POST /_bulk
-H "Content-Type: application/x-ndjson" -d '
{ "index" : { "table" : "products" } }
{ "title" : "Yellow Bag", "price": 12 }
{ "create" : { "table" : "products" } }
{ "title" : "Red Bag", "price": 12.5, "id": 3 }
'
```
<!-- response Elasticsearch -->
```json
{
  "items": [
    {
      "table": {
        "table": "products",
        "_type": "doc",
        "_id": 1657860156022587406,
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
        "table": "products",
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
Use method addDocuments():

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
docs = [     {"insert": {"table" : "products", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}},     {"insert": {"table" : "products", "id" : 2, "doc" : {"title" : "microfiber sheet set", "price" : 19.99}}},     {"insert": {"table" : "products", "id" : 3, "doc" : {"title" : "CPet Hair Remover Glove", "price" : 7.99}}}
]
res = indexApi.bulk('
'.join(map(json.dumps,docs)))
```

<!-- intro -->
##### Python=asyncio:

<!-- request Python-asyncio -->

```python
docs = [     {"insert": {"table" : "products", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}},     {"insert": {"table" : "products", "id" : 2, "doc" : {"title" : "microfiber sheet set", "price" : 19.99}}},     {"insert": {"table" : "products", "id" : 3, "doc" : {"title" : "CPet Hair Remover Glove", "price" : 7.99}}}
]
res = await indexApi.bulk('
'.join(map(json.dumps,docs)))
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
let docs = [
    {"insert": {"table" : "products", "id" : 3, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}},
    {"insert": {"table" : "products", "id" : 4, "doc" : {"title" : "microfiber sheet set", "price" : 19.99}}},
    {"insert": {"table" : "products", "id" : 5, "doc" : {"title" : "CPet Hair Remover Glove", "price" : 7.99}}}
];
res =  await indexApi.bulk(docs.map(e=>JSON.stringify(e)).join('
'));
```
<!-- intro -->
##### java:

<!-- request Java -->

``` java
String body = "{"insert": {"index" : "products", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}}"+"
"+
    "{"insert": {"index" : "products", "id" : 4, "doc" : {"title" : "microfiber sheet set", "price" : 19.99}}}"+"
"+
    "{"insert": {"index" : "products", "id" : 5, "doc" : {"title" : "CPet Hair Remover Glove", "price" : 7.99}}}"+"
";
BulkResponse bulkresult = indexApi.bulk(body);
```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
string body = "{"insert": {"index" : "products", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}}"+"
"+
    "{"insert": {"index" : "products", "id" : 4, "doc" : {"title" : "microfiber sheet set", "price" : 19.99}}}"+"
"+
    "{"insert": {"index" : "products", "id" : 5, "doc" : {"title" : "CPet 毛发去除手套", "price" : 7.99}}}"+"
";
BulkResponse bulkresult = indexApi.Bulk(string.Join("
", docs));
```

<!-- intro -->
##### Rust:

<!-- request Rust -->
``` rust
let bulk_body = r#"{"insert": "index" : "products", "id" : 1, "doc" : {"title" : "流苏斜挎包", "price" : 19.85}}}
    {"insert": {"index" : "products", "id" : 4, "doc" : {"title" : "超细纤维床单套装", "price" : 19.99}}}
    {"insert": {"index" : "products", "id" : 5, "doc" : {"title" : "CPet 毛发去除手套", "price" : 7.99}}}         
"#;
index_api.bulk(bulk_body).await;
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

INSERT INTO products(title, sizes) VALUES('鞋子', (40,41,42,43));
```
<!-- intro -->
##### JSON
<!-- request JSON -->
```json

POST /insert
{
  "table":"products",
  "id":1,
  "doc":
  {
    "title" : "鞋子",
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
  "title": "鞋子",
  "sizes" : [40, 41, 42, 43]
}
```

或者，替代方式
```json
POST /products/_doc/
{
  "title": "鞋子",
  "sizes" : [40, 41, 42, 43]
}
```


<!-- intro -->
##### PHP
<!-- request PHP -->

```php
$index->addDocument(
  ['title' => '鞋子', 'sizes' => [40,41,42,43]],
  1
);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "黄色包","sizes":[40,41,42,43]}})
```

<!-- intro -->
##### Python=asyncio:

<!-- request Python-asyncio -->

```python
await indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "黄色包","sizes":[40,41,42,43]}})
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "黄色包","sizes":[40,41,42,43]}});
```


<!-- intro -->
##### java:

<!-- request Java -->

``` java
newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","黄色包");
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
doc.Add("title", "黄色包");
doc.Add("sizes", new List<Object> {40,41,42,43});
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", id: 0, doc: doc);
var sqlresult = indexApi.Insert(newdoc);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let mut doc = HashMap::new();
doc.insert("title".to_string(), serde_json::json!("黄色包"));
doc.insert("sizes".to_string(), serde_json::json!([40,41,42,43]));
let insert_req = InsertDocumentRequest::new("products".to_string(), serde_json::json!(doc));
let insert_res = index_api.insert(insert_req).await;
```

<!-- end -->

<!-- example JSON_insert -->
## 插入 JSON
JSON 值可以作为 [转义](../../Searching/Full_text_matching/Escaping.md) 字符串插入（通过 SQL 或 JSON），或作为 JSON 对象插入（通过 JSON 接口）。

<!-- intro -->
### 示例
##### SQL
<!-- request SQL -->
```

INSERT INTO products VALUES (1, '鞋子', '{"size": 41, "color": "红色"}');
```
<!-- intro -->
##### JSON
<!-- request JSON -->
JSON 值可以作为 JSON 对象插入
```json
POST /insert
{
  "table":"products",
  "id":1,
  "doc":
  {
    "title" : "鞋子",
    "meta" : {
      "size": 41,
      "color": "红色"
    }
  }
}
```

JSON 值也可以作为包含转义 JSON 的字符串插入：
```json
POST /insert
{
  "table":"products",
  "id":1,
  "doc":
  {
    "title" : "鞋子",
    "meta" : "{"size": 41, "color": "红色"}"
  }
}
```

<!-- intro -->
##### Elasticsearch:
<!-- request Elasticsearch -->

```json
POST /products/_create/1
{
  "title": "鞋子",
  "meta" : {
    "size": 41,
    "color": "红色"
  }
}
```

或者，替代方式
```json
POST /products/_doc/
{
  "title": "鞋子",
  "meta" : {
    "size": 41,
    "color": "红色"
  }
}
```


<!-- intro -->
##### PHP
将 JSON 视为字符串：
<!-- request PHP -->

```php
$index->addDocument(
  ['title' => '鞋子', 'meta' => '{"size": 41, "color": "红色"}'],
  1
);
```
<!-- intro -->
##### Python:

<!-- request Python -->
``` python
indexApi = api = manticoresearch.IndexApi(client)
indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "黄色包","meta":'{"size": 41, "color": "红色"}'}})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->
``` python
indexApi = api = manticoresearch.IndexApi(client)
await indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "黄色包","meta":'{"size": 41, "color": "红色"}'}})
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->
```javascript

res = await indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "黄色包","meta":'{"size": 41, "color": "红色"}'}});
```
<!-- intro -->
##### java:

<!-- request Java -->

``` java
newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","黄色包");
    put("meta",
        new HashMap<String,Object>(){{
            put("size",41);
            put("color","红色");
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
meta.Add("color", "红色");
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "黄色包");
doc.Add("meta", meta);
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", id: 0, doc: doc);
var sqlresult = indexApi.Insert(newdoc);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let mut meta = HashMap::new();
metadoc.insert("size".to_string(), serde_json::json!(41));
meta.insert("color".to_string(), serde_json::json!("红色"));

let mut doc = HashMap::new();
doc.insert("title".to_string(), serde_json::json!("黄色包"));
doc.insert("meta".to_string(), serde_json::json!(meta));

let insert_req = InsertDocumentRequest::new("products".to_string(), serde_json::json!(doc));
let insert_res = index_api.insert(insert_req).await;
```

<!-- end -->

<!-- proofread -->
# 向实时表添加文档

> 如果您正在寻找有关向普通表添加文档的信息，请参阅[从外部存储添加数据](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)部分。

<!-- example insert -->
实时添加文档仅支持[实时](../../Creating_a_table/Local_tables/Real-time_table.md)和[过滤](../../Creating_a_table/Local_tables/Percolate_table.md)表。相应的 SQL 命令、HTTP 端点或客户端函数将新行（文档）插入具有提供的字段值的表中。在向表中添加文档之前，不必先让表存在。如果表不存在，Manticore 将尝试自动创建它。有关更多信息，请参见[自动模式](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。

您可以插入单个或[多个文档](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-adding-documents)，并提供表中所有字段的值或仅提供部分字段的值。在这种情况下，其他字段将填充为它们的默认值（标量类型为 0，文本类型为空字符串）。

当前不支持表达式在 `INSERT` 中，因此必须显式指定值。

ID 字段/值可以省略，因为 RT 和 PQ 表支持[自动 ID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID)功能。您也可以使用 `0` 作为 ID 值以强制自动 ID 生成。具有重复 ID 的行不会被 `INSERT` 覆盖。相反，您可以使用[REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md)来实现此目的。

使用 HTTP JSON 协议时，您可以选择两种不同的请求格式：常见的 Manticore 格式和类似 Elasticsearch 的格式。以下示例演示了这两种格式。

此外，使用 Manticore JSON 请求格式时，请记住需要 `doc` 节点，并且所有值应在其中提供。


<!-- intro -->
##### SQL:
<!-- request SQL -->
一般语法：

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
  "table":"products",
  "id":1,
  "doc":
  {
    "title" : "Crossbody Bag with Tassel",
    "price" : 19.85
  }
}

POST /insert
{
  "table":"products",
  "id":2,
  "doc":
  {
    "title" : "Crossbody Bag with Tassel"
  }
}

POST /insert
{
  "table":"products",
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
  "table": "products",
  "_id": 1,
  "created": true,
  "result": "created",
  "status": 201
}
{
  "table": "products",
  "_id": 2,
  "created": true,
  "result": "created",
  "status": 201
}
{
  "table": "products",
  "_id": 1657860156022587406,
  "created": true,
  "result": "created",
  "status": 201
}

```

<!-- intro -->
##### Elasticsearch:

<!-- request Elasticsearch -->

> 注意：`_create` 需要[Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保已安装 Buddy。

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
"_id":2235747273424240642,
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
indexApi.insert({"table" : "test", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}})
indexApi.insert({"table" : "test", "id" : 2, "doc" : {"title" : "Crossbody Bag with Tassel"}})
indexApi.insert({"table" : "test", "id" : 0, "doc" : {{"title" : "Yellow bag"}})
```
<!-- intro -->
##### Javascript:

<!-- request Javascript -->

``` javascript
res = await indexApi.insert({"table" : "test", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}});
res = await indexApi.insert({"table" : "test", "id" : 2, "doc" : {"title" : "Crossbody Bag with Tassel"}});
res = await indexApi.insert({"table" : "test", "id" : 0, "doc" : {{"title" : "Yellow bag"}});
```

<!-- intro -->
##### java:

<!-- request Java -->

``` java
InsertDocumentRequest newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","跨体包与流苏");
    put("price",19.85);
}};
newdoc.index("products").id(1L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);

newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","跨体包与流苏");
}};
newdoc.index("products").id(2L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);

newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","黄色包");
 }};
newdoc.index("products").id(0L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);

```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "跨体包与流苏");
doc.Add("price", 19.85);
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", id: 1, doc: doc);
var sqlresult = indexApi.Insert(newdoc);

doc = new Dictionary<string, Object>();
doc.Add("title", "跨体包与流苏");
newdoc = new InsertDocumentRequest(index: "products", id: 2, doc: doc);
sqlresult = indexApi.Insert(newdoc);

doc = new Dictionary<string, Object>();
doc.Add("title", "黄色包");
newdoc = new InsertDocumentRequest(index: "products", id: 0, doc: doc);
sqlresult = indexApi.Insert(newdoc);

```

<!-- end -->

## 自动模式

> 注意：自动模式需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它无法工作，请确保安装了 Buddy。

Manticore 具有自动表创建机制，当插入查询中的指定表尚不存在时激活。此机制默认启用。要禁用它，请在 Manticore 配置文件的 [Searchd](../../Server_settings/Searchd.md#auto_schema) 部分设置 `auto_schema = 0`。

<!-- example auto-schema -->

默认情况下，`VALUES` 子句中的所有文本值被视为 `text` 类型，除了表示有效电子邮件地址的值，被视为 `string` 类型。

如果您尝试使用相同字段的不同、不兼容的值类型插入多行，将取消自动表创建，并返回错误消息。然而，如果不同的值类型是兼容的，最终字段类型将是可以容纳所有值的类型。可能发生的一些自动数据类型转换包括：
* mva -> mva64
* uint -> bigint -> float（这可能导致某些精度损失）
* string -> text

此外，以下日期格式将被识别并转换为时间戳，而所有其他日期格式将被视为字符串：
- `%Y-%m-%dT%H:%M:%E*S%Z`
- `%Y-%m-%d'T'%H:%M:%S%Z`
- `%Y-%m-%dT%H:%M:%E*S`
- `%Y-%m-%dT%H:%M:%s`
- `%Y-%m-%dT%H:%M`
- `%Y-%m-%dT%H`


请记住，`/bulk` HTTP 端点不支持自动表创建（自动模式）。仅 `/_bulk`（类似 Elasticsearch）HTTP 端点和 SQL 接口支持此功能。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
MySQL [(none)]> drop table if exists t; insert into t(i,f,t,s,j,b,m,mb) values(123,1.2,'这里的文本','test@mail.com','{"a": 123}',1099511627776,(1,2),(1099511627776,1099511627777)); desc t; select * from t;
```

<!-- response SQL -->

```sql
--------------
drop table if exists t
--------------

Query OK, 0 rows affected (0.42 sec)

--------------
insert into t(i,f,t,j,b,m,mb) values(123,1.2,'这里的文本','{"a": 123}',1099511627776,(1,2),(1099511627776,1099511627777))
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
| 5045949922868723723 |  123 | 1099511627776 | 1.200000 | 1,2  | 1099511627776,1099511627777 | 这里的文本 | test@mail.com | {"a": 123} |
+---------------------+------+---------------+----------+------+-----------------------------+-----------+---------------+------------+
1 row in set (0.00 sec)
```

<!-- request JSON -->

```json
POST /insert  -d
{
 "table":"t",
 "id": 2,
 "doc":
 {
   "i" : 123,
   "f" : 1.23,
   "t": "这里的文本",
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
{"table":"t","_id":2,"created":true,"result":"创建成功","status":201}
```

<!-- end -->

## 自动 ID
<!-- example autoid -->
Manticore 为插入或替换到实时或 [Percolate 表](../../Creating_a_table/Local_tables/Percolate_table.md) 的文档的列 ID 提供自动 ID 生成功能。生成器为文档产生一个唯一的 ID，并附带一些保证，但不应该被视为自动递增 ID。
生成的 ID 值在以下条件下保证唯一：
* 当前服务器的 [server_id](../../Server_settings/Searchd.md#server_id) 值在 0 到 127 之间，并且在集群中的节点之间是唯一的，或者使用从 MAC 地址生成的默认值作为种子
* Manticore 节点在服务器重启之间的系统时间没有变化
* 在搜索服务器重启之间，自动 ID 生成的次数少于 1600 万次

自动 ID 生成器为文档 ID 创建一个 64 位整数，并使用以下方案：
* 位 0 到 23 形成一个计数器，在每次调用自动 ID 生成器时递增
* 位 24 到 55 代表服务器启动的 Unix 时间戳
* 位 56 到 63 对应于 server_id

该方案确保生成的 ID 在集群中的所有节点之间是唯一的，并且插入到不同集群节点中的数据不会导致节点之间的冲突。

因此，用于自动 ID 的生成器的第一个 ID 不是 1，而是一个更大的数字。此外，插入到表中的文档流可能会具有非顺序的 ID 值，因为在调用之间如果对其他表进行插入，ID 生成器在服务器中是唯一的，并且在其所有表之间共享。

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
  "table":"products",
  "id":0,
  "doc":
  {
    "title" : "Yellow bag"
  }
}

GET /search
{
  "table":"products",
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
indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "Yellow bag"}})
```
<!-- intro -->

##### Javascript:

<!-- request Javascript -->

```javascript
res = await indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "Yellow bag"}});
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

<!-- example call -->
### UUID_SHORT 多 ID 生成

```sql
CALL UUID_SHORT(N)
```

`CALL UUID_SHORT(N)` 语句允许在一次调用中生成 N 个唯一的 64 位 ID，而无需插入任何文档。当您需要在 Manticore 中预生成 IDs 以用于其他系统或存储解决方案时，这尤其有用。例如，您可以在 Manticore 中生成自动 ID，然后在另一个数据库、应用程序或工作流中使用它们，确保在不同环境之间具有一致和唯一的标识符。

<!-- intro -->
##### 示例：
<!-- request Example -->

```sql
CALL UUID_SHORT(3)
```
<!-- response SQL -->
```
+---------------------+
| uuid_short()        |
+---------------------+
| 1227930988733973183 |
| 1227930988733973184 |
| 1227930988733973185 |
+---------------------+
```
<!-- end -->

<!-- example bulk_insert -->
## 批量添加文档
您不仅可以将单个文档插入实时表中，还可以插入任意数量的文档。将成千上万的文档批量插入实时表是完全没有问题的。然而，重要的是要记住以下几点：
* 批量越大，每次插入操作的延迟越高
* 批量越大，您可以期待的索引速度越高
* 您可能想要增加 [max_packet_size](../../Server_settings/Searchd.md#max_packet_size) 值以允许更大的批量
* 通常，每个批量插入操作被视为一个具有原子性保证的单个 [transaction](../../Data_creation_and_modification/Transactions.md)，因此，要么所有新文档同时在表中，要么在失败的情况下，全部都不会被添加。有关空行或切换到另一个表的更多详细信息，请参见“JSON”示例。

请注意，`/bulk` HTTP 端点不支持自动创建表（自动架构）。只有 `/_bulk`（类似于 Elasticsearch）HTTP 端点和 SQL 接口支持此功能。`/_bulk`（类似于 Elasticsearch）HTTP 端点允许表名包含以 `cluster_name:table_name` 格式表示的集群名称。
`/_bulk` 端点接受与 Elasticsearch 相同格式的文档 ID，您还可以在文档本身中包含 `id`：
```json
{ "index": { "table" : "products", "_id" : "1" } }
{ "title" : "带流苏的斜挎包", "price": 19.85 }
```

或者

```json
{ "index": { "table" : "products" } }
{ "title" : "带流苏的斜挎包", "price": 19.85, "id": "1" }
```

#### /bulk 中的分块传输
`/bulk`（Manticore 模式）端点支持 [分块传输编码](https://en.wikipedia.org/wiki/Chunked_transfer_encoding)。您可以用它来传输大型批量数据。它：
* 降低峰值 RAM 使用，降低 OOM 风险
* 减少响应时间
* 允许您绕过 [max_packet_size](../Server_settings/Searchd.md#max_packet_size) 并传输远超 `max_packet_size` 的最大允许值（128MB）的大批量数据，例如，一次传输 1GB。

<!-- intro -->
### 批量插入示例
##### SQL:
<!-- request SQL -->
对于批量插入，只需在 `VALUES()` 后提供更多的文档。语法是：

```sql
INSERT INTO <table name>[(column1, column2, ...)] VALUES(value1[, value2 , ...]), (...)
```

可选的列名列表允许您明确指定表中的某些列的值。其他所有列将使用其默认值填充（标量类型为 0，字符串类型为空字符串）。

例如：

```sql
INSERT INTO products(title,price) VALUES ('带流苏的斜挎包', 19.85), ('超细纤维床单套件', 19.99), ('宠物毛发去除手套', 7.99);
```
<!-- response SQL -->

```sql
查询 OK, 受影响的行数 3 (0.01 秒)
```

目前在 `INSERT` 中不支持表达式，值应明确指定。

<!-- intro -->
##### JSON:
<!-- request JSON -->
语法与 [单个文档插入](../../Quick_start_guide.md#Add-documents) 基本相同。只需提供更多行，每个文档一行，并使用 `/bulk` 端点而不是 `/insert`。将每个文档放在 "insert" 节点中。请注意，它还要求：
* `Content-Type: application/x-ndjson`
* 数据应格式化为换行分隔的 JSON（NDJSON）。本质上，这意味着每行应包含恰好一个 JSON 语句，并以换行符 `\n` 和可能的 `\r` 结束。

`/bulk` 端点支持 'insert'、'replace'、'delete' 和 'update' 查询。请记住，您可以将操作定向到多个表，但事务仅适用于单个表。如果您指定多个表，Manticore 将把定向到一个表的操作集合到一个事务中。当表更改时，它将提交收集的操作，并在新表上启动新的事务。分隔批次的空行也会导致提交先前的批次并开始新的事务。

在 `/bulk` 请求的响应中，您可以找到以下字段：
* "errors": 显示是否发生了任何错误（true/false）
* "error": 描述发生的错误
* "current_line": 执行停止（或失败）时的行号；空行，包括第一个空行，也被计入
* "skipped_lines": 从 `current_line` 开始向后计数的未提交行的数量

```json
POST /bulk
-H "Content-Type: application/x-ndjson" -d '
{"insert": {"table":"products", "id":1, "doc":  {"title":"带流苏的斜挎包","price" : 19.85}}}
{"insert":{"table":"products", "id":2, "doc":  {"title":"超细纤维床单套件","price" : 19.99}}}
'

POST /bulk
-H "Content-Type: application/x-ndjson" -d '
{"insert":{"table":"test1","id":21,"doc":{"int_col":1,"price":1.1,"title":"批量文档一"}}}
{"insert":{"table":"test1","id":22,"doc":{"int_col":2,"price":2.2,"title":"批量文档二"}}}

{"insert":{"table":"test1","id":23,"doc":{"int_col":3,"price":3.3,"title":"批量文档三"}}}
{"insert":{"table":"test2","id":24,"doc":{"int_col":4,"price":4.4,"title":"批量文档四"}}}
{"insert":{"table":"test2","id":25,"doc":{"int_col":5,"price":5.5,"title":"批量文档五"}}}
'
```

<!-- response JSON -->
```json
{
  "items": [
    {
      "bulk": {
        "table": "products",
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
        "table": "test1",
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
        "table": "test1",
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
        "table": "test2",
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

> 注意：`_bulk` 如果表尚不存在，则需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保 Buddy 已安装。

```json
POST /_bulk
-H "Content-Type: application/x-ndjson" -d '
{ "index" : { "table" : "products" } }
{ "title" : "黄色包", "price": 12 }
{ "create" : { "table" : "products" } }
{ "title" : "红色包", "price": 12.5, "id": 3 }
'
```
<!-- response Elasticsearch -->
```json
{
  "items": [
    {
      "table": {
        "table": "products",
        "_type": "doc",
        "_id": 1657860156022587406,
        "_version": 1,
        "result": "创建",
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
        "table": "产品",
        "_type": "文档",
        "_id": 3,
        "_version": 1,
        "result": "创建",
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
        ['id' => 1, 'title' => '流行单肩包', 'price' => 19.85],
        ['id' => 2, 'title' => '超细纤维床单套装', 'price' => 19.99],
        ['id' => 3, 'title' => '宠物毛发清理手套', 'price' => 7.99]
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
docs = [ \
    {"insert": {"table" : "产品", "id" : 1, "doc" : {"title" : "流行单肩包", "price" : 19.85}}}, \
    {"insert": {"table" : "产品", "id" : 2, "doc" : {"title" : "超细纤维床单套装", "price" : 19.99}}}, \
    {"insert": {"table" : "产品", "id" : 3, "doc" : {"title" : "宠物毛发清理手套", "price" : 7.99}}}
]
res = indexApi.bulk('\n'.join(map(json.dumps,docs)))
```


<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
let docs = [
    {"insert": {"table" : "产品", "id" : 3, "doc" : {"title" : "流行单肩包", "price" : 19.85}}},
    {"insert": {"table" : "产品", "id" : 4, "doc" : {"title" : "超细纤维床单套装", "price" : 19.99}}},
    {"insert": {"table" : "产品", "id" : 5, "doc" : {"title" : "宠物毛发清理手套", "price" : 7.99}}}
];
res =  await indexApi.bulk(docs.map(e=>JSON.stringify(e)).join('\n'));
```
<!-- intro -->
##### java:

<!-- request Java -->

``` java
String body = "{\"insert\": {\"index\" : \"产品\", \"id\" : 1, \"doc\" : {\"title\" : \"流行单肩包\", \"price\" : 19.85}}}"+"\n"+
    "{\"insert\": {\"index\" : \"产品\", \"id\" : 4, \"doc\" : {\"title\" : \"超细纤维床单套装\", \"price\" : 19.99}}}"+"\n"+
    "{\"insert\": {\"index\" : \"产品\", \"id\" : 5, \"doc\" : {\"title\" : \"宠物毛发清理手套\", \"price\" : 7.99}}}"+"\n";
BulkResponse bulkresult = indexApi.bulk(body);
```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
string body = "{\"insert\": {\"index\" : \"产品\", \"id\" : 1, \"doc\" : {\"title\" : \"流行单肩包\", \"price\" : 19.85}}}"+"\n"+
    "{\"insert\": {\"index\" : \"产品\", \"id\" : 4, \"doc\" : {\"title\" : \"超细纤维床单套装\", \"price\" : 19.99}}}"+"\n"+
    "{\"insert\": {\"index\" : \"产品\", \"id\" : 5, \"doc\" : {\"title\" : \"宠物毛发清理手套\", \"price\" : 7.99}}}"+"\n";
BulkResponse bulkresult = indexApi.Bulk(string.Join("\n", docs));
```

<!-- end -->
<!-- example MVA_insert -->
## 插入多值属性（MVA）值

多值属性（MVA）作为数字数组插入。
<!-- intro -->
### 示例
##### SQL
<!-- request SQL -->
```

INSERT INTO 产品(title, sizes) VALUES('鞋子', (40,41,42,43));
```
<!-- intro -->
##### JSON
<!-- request JSON -->
```json

POST /insert
{
  "table":"产品",
  "id":1,
  "doc":
  {
    "title" : "鞋子",
    "sizes" : [40, 41, 42, 43]
  }
}
```

<!-- intro -->
##### Elasticsearch:
<!-- request Elasticsearch -->

```json
POST /产品/_create/1
{
  "title": "鞋子",
  "sizes" : [40, 41, 42, 43]
}
```

或者，另一种方式
```json
POST /产品/_doc/
{
  "title": "鞋子",
  "sizes" : [40, 41, 42, 43]
}
```


<!-- intro -->
##### PHP
<!-- request PHP -->

```php
$index->addDocument(
  ['title' => '鞋子', 'sizes' => [40,41,42,43]],
  1
);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
indexApi.insert({"table" : "产品", "id" : 0, "doc" : {"title" : "黄色包", "sizes":[40,41,42,43]}})
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await indexApi.insert({"table" : "产品", "id" : 0, "doc" : {"title" : "黄色包", "sizes":[40,41,42,43]}});
```


<!-- intro -->
##### java:

<!-- request Java -->

``` java
newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","黄色包");
    put("sizes",new int[]{40,41,42,43});
 }};
newdoc.index("产品").id(0L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);
```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "黄色包");
doc.Add("sizes", new List<Object> {40,41,42,43});
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "产品", id: 0, doc: doc);
var sqlresult = indexApi.Insert(newdoc);
```
<!-- end -->

<!-- example JSON_insert -->
## 插入JSON
JSON值可以作为[转义](../../Searching/Full_text_matching/Escaping.md)字符串（通过SQL或JSON）或作为JSON对象（通过JSON接口）插入。

<!-- intro -->
### 示例
##### SQL
<!-- request SQL -->
```

INSERT INTO 产品 VALUES (1, '鞋子', '{"size": 41, "color": "红色"}');
```
<!-- intro -->
##### JSON
<!-- request JSON -->
JSON值可以作为JSON对象插入
```json
POST /insert
{
  "table":"products",
  "id":1,
  "doc":
  {
    "title" : "鞋子",
    "meta" : {
      "size": 41,
      "color": "红色"
    }
  }
}
```

JSON值也可以作为包含转义JSON的字符串插入：
```json
POST /insert
{
  "table":"products",
  "id":1,
  "doc":
  {
    "title" : "鞋子",
    "meta" : "{\"size\": 41, \"color\": \"红色\"}"
  }
}
```

<!-- intro -->
##### Elasticsearch:
<!-- request Elasticsearch -->

```json
POST /products/_create/1
{
  "title": "鞋子",
  "meta" : {
    "size": 41,
    "color": "红色"
  }
}
```

或者，替代方案
```json
POST /products/_doc/
{
  "title": "鞋子",
  "meta" : {
    "size": 41,
    "color": "红色"
  }
}
```


<!-- intro -->
##### PHP
将JSON视为字符串：
<!-- request PHP -->

```php
$index->addDocument(
  ['title' => '鞋子', 'meta' => '{"size": 41, "color": "红色"}'],
  1
);
```
<!-- intro -->
##### Python:

<!-- request Python -->
``` python
indexApi = api = manticoresearch.IndexApi(client)
indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "黄色包","meta":'{"size": 41, "color": "红色"}'}})
```
<!-- intro -->
##### Javascript:

<!-- request Javascript -->
```javascript

res = await indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "黄色包","meta":'{"size": 41, "color": "红色"}'}});
```
<!-- intro -->
##### java:

<!-- request Java -->

``` java
newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","黄色包");
    put("meta",
        new HashMap<String,Object>(){{
            put("size",41);
            put("color","红色");
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
meta.Add("color", "红色");
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "黄色包");
doc.Add("meta", meta);
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", id: 0, doc: doc);
var sqlresult = indexApi.Insert(newdoc);
```

<!-- end -->

<!-- proofread -->


