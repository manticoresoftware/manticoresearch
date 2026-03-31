# 将文档添加到实时表

> 如果您正在寻找有关将文档添加到普通表的信息，请参阅[从外部存储添加数据](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)部分。

<!-- example insert -->
实时添加文档仅支持[实时](../../Creating_a_table/Local_tables/Real-time_table.md)和[Percolate](../../Creating_a_table/Local_tables/Percolate_table.md)表。相应的SQL命令、HTTP端点或客户端函数将新行（文档）插入到表中，提供字段值。在添加文档之前，表不一定存在。如果表不存在，Manticore 将尝试自动创建它。有关更多信息，请参阅[自动模式](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。

您可以插入单个或[多个文档](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-adding-documents)，这些文档包含表的所有字段值或仅部分字段值。在这种情况下，其他字段将使用其默认值填充（标量类型为0，文本类型为空字符串）。

`INSERT` 中目前不支持表达式，因此必须显式指定值。

ID字段/值可以省略，因为RT和PQ表支持[自动ID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID)功能。您也可以使用0作为ID值以强制生成自动ID。具有重复ID的行不会通过`INSERT` 被覆盖。相反，您可以使用[REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md)来实现这一点。

使用HTTP JSON协议时，您有两种不同的请求格式可供选择：通用Manticore格式和Elasticsearch类似的格式。这两种格式在下面的示例中都有展示。

此外，在使用Manticore JSON请求格式时，请注意`doc`节点是必需的，并且所有值都应在此节点内提供。


<!-- intro -->
##### SQL：
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
##### JSON：

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
##### Elasticsearch：

<!-- request Elasticsearch -->

> 注意：`_create` 需要[Manticore Buddy](Installation/Manticore_Buddy.md)。如果不起作用，请确保已安装Buddy。

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
##### PHP：

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
##### Python：

<!-- request Python -->

``` python
indexApi.insert({"table" : "test", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}})
indexApi.insert({"table" : "test", "id" : 2, "doc" : {"title" : "Crossbody Bag with Tassel"}})
indexApi.insert({"table" : "test", "id" : 0, "doc" : {{"title" : "Yellow bag"}})
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

``` python
await indexApi.insert({"table" : "test", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}})
await indexApi.insert({"table" : "test", "id" : 2, "doc" : {"title" : "Crossbody Bag with Tassel"}})
await indexApi.insert({"table" : "test", "id" : 0, "doc" : {{"title" : "Yellow bag"}})
```

<!-- intro -->
##### Javascript：

<!-- request Javascript -->

``` javascript
res = await indexApi.insert({"table" : "test", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}});
res = await indexApi.insert({"table" : "test", "id" : 2, "doc" : {"title" : "Crossbody Bag with Tassel"}});
res = await indexApi.insert({"table" : "test", "id" : 0, "doc" : {{"title" : "Yellow bag"}});
```

<!-- intro -->
##### java：

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
##### C#：

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

<!-- intro -->
##### Rust：

<!-- request Rust -->

``` rust
let mut doc = HashMap::new();
doc.insert("title".to_string(), serde_json::json!("Crossbody Bag with Tassel"));
doc.insert("price".to_string(), serde_json::json!(19.85));
let mut insert_req = InsertDocumentRequest {
    table: serde_json::json!("products"),
    doc: serde_json::json!(doc),
    id: serde_json::json!(1),
    ..Default::default(),
};
let mut insert_res = index_api.insert(insert_req).await;

doc = HashMap::new();
doc.insert("title".to_string(), serde_json::json!("Crossbody Bag with Tassel"));
insert_req = InsertDocumentRequest {
    table: serde_json::json!("products"),
    doc: serde_json::json!(doc),
    id: serde_json::json!(2),
    ..Default::default(),
};
insert_res = index_api.insert(insert_req).await;

doc = HashMap::new();
doc.insert("title".to_string(), serde_json::json!("Tellow bag"));
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

> 注意：自动模式需要[Manticore Buddy](Installation/Manticore_Buddy.md)。如果不起作用，请确保已安装Buddy。

Manticore具有自动表创建机制，当插入或替换查询中指定的表尚不存在时，该机制会激活。此机制默认启用。要禁用它，请在Manticore配置文件的[Searchd](../../Server_settings/Searchd.md#auto_schema)部分中设置`auto_schema = 0`。

<!-- example auto-schema -->

默认情况下，`VALUES` 子句中的所有文本值被视为`text`类型，除非它们表示有效的电子邮件地址，这些地址将被视为`string`类型。

如果您尝试插入/替换具有相同字段的不同且不兼容值类型的多行，自动表创建将被取消，并返回错误消息。但是，如果不同的值类型兼容，结果字段类型将是能够容纳所有值的类型。可能发生的一些自动数据类型转换包括：
* mva -> mva64
* uint -> bigint -> float（这可能会导致一些精度损失）
* string -> text

自动模式机制不支持创建包含用于[KNN](../../Searching/KNN.md#Configuring-a-table-for-KNN-search)（K-最近邻）相似搜索的向量字段（类型为`float_vector`）的表。要在表中使用向量字段，您必须显式创建具有定义这些字段的模式的表。如果您需要在没有KNN搜索功能的普通表中存储向量数据，可以使用标准JSON语法将其存储为JSON数组，例如：`INSERT INTO table_name (vector_field) VALUES ('[1.0, 2.0, 3.0]')`。

此外，以下日期格式将被识别并转换为时间戳，而其他日期格式将被视为字符串：
- `%Y-%m-%dT%H:%M:%E*S%Z`
- `%Y-%m-%d'T'%H:%M:%S%Z`
- `%Y-%m-%dT%H:%M:%E*S`
- `%Y-%m-%dT%H:%M:%s`
- `%Y-%m-%dT%H:%M`
- `%Y-%m-%dT%H`


请注意，`/bulk` HTTP端点不支持自动表创建（自动模式）。仅`/_bulk`（Elasticsearch类似的）HTTP端点和SQL接口支持此功能。

<!-- intro -->
##### SQL：

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

## 自动ID
<!-- example autoid -->
Manticore 为插入或替换到实时表或[Percolate表](../../Creating_a_table/Local_tables/Percolate_table.md)的文档的ID列提供了自动ID生成功能。生成器为文档生成一个唯一的ID，但不应将其视为自动递增ID。

生成的 ID 值在以下条件下保证唯一：
* 当前服务器的 [server_id](../../Server_settings/Searchd.md#server_id) 值在 0 到 127 范围内，并且在集群节点中唯一，或者它使用从 MAC 地址生成的默认值作为种子
* 系统时间在 Manticore 节点服务器重启间不发生变化
* 自动 ID 在搜索服务器重启间每秒生成次数少于 1600 万次

自动 ID 生成器为文档 ID 创建一个 64 位整数，使用以下架构：
* 位 0 至 23 形成一个计数器，每次调用自动 ID 生成器时递增
* 位 24 至 55 代表服务器启动的 Unix 时间戳
* 位 56 至 63 对应 server_id

该架构确保生成的 ID 在集群所有节点中唯一，且插入不同集群节点的数据不会在节点之间造成冲突。

因此，自动 ID 生成器生成的第一个 ID 不是 1，而是一个较大的数字。此外，插入表中的文档流可能具有非连续的 ID 值，如果在调用之间向其他表插入数据，因为 ID 生成器在服务器中是唯一的并且在其所有表之间共享。

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
### UUID_SHORT 多 ID 生成

```sql
CALL UUID_SHORT(N)
```

`CALL UUID_SHORT(N)` 语句允许在一次调用中生成 N 个唯一的 64 位 ID，无需插入任何文档。它在需要在 Manticore 中预生成 ID 以供其他系统或存储方案使用时尤其有用。比如，你可以在 Manticore 中生成自动 ID，然后将它们用于另一个数据库、应用程序或工作流程，确保不同环境中的标识符一致且唯一。

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
你不仅可以向实时表插入单个文档，还可以插入任意数量的文档。向实时表一次插入数万个文档是完全可以的。然而，需要注意以下几点：
* 批量越大，每次插入操作的延迟越高
* 批量越大，索引速度越快
* 你可能需要增加 [max_packet_size](../../Server_settings/Searchd.md#max_packet_size) 的值以允许更大的批量
* 通常，每次批量插入操作被视为具有原子性保证的单个 [事务](../../Data_creation_and_modification/Transactions.md)，因此你要么一次性将所有新文档放入表中，要么在失败情况下一个都不添加。关于空行或切换到另一张表的更多细节见“JSON”示例。

请注意，`/bulk` HTTP 端点不支持自动创建表（自动模式）。只有 `/_bulk`（类似 Elasticsearch）HTTP 端点和 SQL 接口支持此功能。`/_bulk`（类似 Elasticsearch）HTTP 端点允许表名包含集群名，格式为 `cluster_name:table_name`。

`/_bulk` 端点接受与 Elasticsearch 相同格式的文档 ID，你也可以在文档内部包含 `id`：
```json
{ "index": { "table" : "products", "_id" : "1" } }
{ "title" : "Crossbody Bag with Tassel", "price": 19.85 }
```

或者

```json
{ "index": { "table" : "products" } }
{ "title" : "Crossbody Bag with Tassel", "price": 19.85, "id": "1" }
```

#### /bulk 的分块传输
`/bulk`（Manticore 模式）端点支持[分块传输编码](https://en.wikipedia.org/wiki/Chunked_transfer_encoding)。你可以用它来传输大批量数据。它能够：
* 降低峰值内存使用，减少内存溢出风险
* 缩短响应时间
* 允许绕过 [max_packet_size](../../Server_settings/Searchd.md#max_packet_size) 限制，传输远大于最大允许值（128MB）的批量，例如一次 1GB。

<!-- intro -->
### 批量插入示例
##### SQL:
<!-- request SQL -->
对于批量插入，只需在`VALUES()`后提供更多文档，语法如下：

```sql
INSERT INTO <table name>[(column1, column2, ...)] VALUES(value1[, value2 , ...]), (...)
```

可选的列名列表允许你显式指定表中某些列的值。所有其他列将被填充默认值（标量类型为 0，字符串类型为空字符串）。

例如：

```sql
INSERT INTO products(title,price) VALUES ('Crossbody Bag with Tassel', 19.85), ('microfiber sheet set', 19.99), ('Pet Hair Remover Glove', 7.99);
```
<!-- response SQL -->

```sql
Query OK, 3 rows affected (0.01 sec)
```

当前 `INSERT` 不支持表达式，值应明确指定。

<!-- intro -->
##### JSON:
<!-- request JSON -->
语法基本与 [插入单个文档](../../Quick_start_guide.md#Add-documents) 相同。只需提供更多行，每行一个文档，并使用 `/bulk` 端点代替 `/insert`。将每个文档封装在 "insert" 节点中。注意还需：
* `Content-Type: application/x-ndjson`
* 数据格式应为换行分隔 JSON（NDJSON）。本质上，这意味着每行应仅包含一条 JSON 声明，并以换行符 `\n`（可能还有 `\r`）结尾。

`/bulk` 端点支持 'insert'、'replace'、'delete' 和 'update' 查询。请注意，您可以将操作定向到多个表，但事务仅适用于单个表。如果指定了多个表，Manticore 会将针对一个表的操作收集到单个事务中。当表发生变化时，它将提交已收集的操作，并在新表上启动新的事务。分隔批次的空行也会导致提交前一批次并开始新事务。

在 `/bulk` 请求的响应中，您可以找到以下字段：
* "errors"：显示是否发生了任何错误（true/false）
* "error"：描述发生的错误
* "current_line"：执行停止（或失败）的行号；空行（包括第一个空行）也会被计数
* "skipped_lines"：未提交行的数量，从 `current_line` 开始向后计算

```json
POST /bulk
-H "Content-Type: application/x-ndjson" -d '
{"insert": {"table":"products", "id":1, "doc":  {"title":"Crossbody Bag with Tassel","price" : 19.85}}}
{"insert":{"table":"products", "id":2, "doc":  {"title":"microfiber sheet set","price" : 19.99}}}
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

> 注意：如果表尚不存在，`_bulk` 需要 [Manticore Buddy](Installation/Manticore_Buddy.md)。如果不起作用，请确保已安装 Buddy。

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
使用 addDocuments() 方法：

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
    {"insert": {"table" : "products", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}}, \
    {"insert": {"table" : "products", "id" : 2, "doc" : {"title" : "microfiber sheet set", "price" : 19.99}}}, \
    {"insert": {"table" : "products", "id" : 3, "doc" : {"title" : "CPet Hair Remover Glove", "price" : 7.99}}}
]
res = indexApi.bulk('\n'.join(map(json.dumps,docs)))
```

<!-- intro -->
##### Python=asyncio:

<!-- request Python-asyncio -->

```python
docs = [ \
    {"insert": {"table" : "products", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}}, \
    {"insert": {"table" : "products", "id" : 2, "doc" : {"title" : "microfiber sheet set", "price" : 19.99}}}, \
    {"insert": {"table" : "products", "id" : 3, "doc" : {"title" : "CPet Hair Remover Glove", "price" : 7.99}}}
]
res = await indexApi.bulk('\n'.join(map(json.dumps,docs)))
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

<!-- intro -->
##### Rust:

<!-- request Rust -->
``` rust
let bulk_body = r#"{"insert": "index" : "products", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}}
    {"insert": {"index" : "products", "id" : 4, "doc" : {"title" : "microfiber sheet set", "price" : 19.99}}}
    {"insert": {"index" : "products", "id" : 5, "doc" : {"title" : "CPet Hair Remover Glove", "price" : 7.99}}}
"#;
index_api.bulk(bulk_body).await;
```

<!-- end -->
<!-- example MVA_insert -->
## 插入多值属性（MVA）值

多值属性（MVA）以数字数组的形式插入。
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
  "table":"products",
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

或者，也可以这样
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
indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "Yellow bag","sizes":[40,41,42,43]}})
```

<!-- intro -->
##### Python=asyncio:

<!-- request Python-asyncio -->

```python
await indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "Yellow bag","sizes":[40,41,42,43]}})
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "Yellow bag","sizes":[40,41,42,43]}});
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

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let mut doc = HashMap::new();
doc.insert("title".to_string(), serde_json::json!("Yellow bag"));
doc.insert("sizes".to_string(), serde_json::json!([40,41,42,43]));
let insert_req = InsertDocumentRequest::new("products".to_string(), serde_json::json!(doc));
let insert_res = index_api.insert(insert_req).await;
```

<!-- end -->

<!-- example JSON_insert -->
## 插入 JSON
JSON 值可以作为[转义](../../Searching/Full_text_matching/Escaping.md)字符串（通过 SQL 或 JSON）或作为 JSON 对象（通过 JSON 接口）插入。

<!-- intro -->
### 示例
##### SQL
<!-- request SQL -->
```

INSERT INTO products VALUES (1, 'shoes', '{"size": 41, "color": "red"}');
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
    "title" : "shoes",
    "meta" : {
      "size": 41,
      "color": "red"
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

或者，也可以这样
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
indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "Yellow bag","meta":'{"size": 41, "color": "red"}'}})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->
``` python
indexApi = api = manticoresearch.IndexApi(client)
await indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "Yellow bag","meta":'{"size": 41, "color": "red"}'}})
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->
```javascript

res = await indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "Yellow bag","meta":'{"size": 41, "color": "red"}'}});
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

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let mut meta = HashMap::new();
metadoc.insert("size".to_string(), serde_json::json!(41));
meta.insert("color".to_string(), serde_json::json!("red"));

let mut doc = HashMap::new();
doc.insert("title".to_string(), serde_json::json!("Yellow bag"));
doc.insert("meta".to_string(), serde_json::json!(meta));

let insert_req = InsertDocumentRequest::new("products".to_string(), serde_json::json!(doc));
let insert_res = index_api.insert(insert_req).await;
```

<!-- end -->

<!-- proofread -->

