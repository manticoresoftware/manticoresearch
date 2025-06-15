# 向实时表添加文档

> 如果您想了解如何向普通表添加文档，请参阅[从外部存储添加数据](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)一节。

<!-- example insert -->
实时添加文档仅支持[实时表](../../Creating_a_table/Local_tables/Real-time_table.md)和[percolate 表](../../Creating_a_table/Local_tables/Percolate_table.md)。对应的 SQL 命令、HTTP 端点或客户端函数会将具有指定字段值的新行（文档）插入表中。添加文档时不要求表必须事先存在。如果表不存在，Manticore 会尝试自动创建。更多信息请参阅[自动模式](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。

您可以插入单个或[多个文档](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-adding-documents)，为表的所有字段或部分字段提供值。此时，其他字段将填充为默认值（标量类型为 0，文本类型为空字符串）。

`INSERT` 当前不支持表达式，因此必须显式指定值。

ID 字段/值可省略，因为 RT 和 PQ 表支持[自动 ID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID)功能。您也可以使用 `0` 作为 id 值以强制自动生成 ID。对于重复 ID 的行，`INSERT` 不会覆盖。欲覆盖请使用[REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md)。

使用 HTTP JSON 协议时，您可选择两种不同的请求格式：通用 Manticore 格式和类似 Elasticsearch 的格式。以下示例展示了两种格式。

此外，使用 Manticore JSON 请求格式时，请注意需要 `doc` 节点，所有值应包含在该节点中。


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

> 注意：`_create` 需要[Manticore Buddy](Installation/Manticore_Buddy.md)。如果无法使用，请确保 Buddy 已安装。

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

<!-- intro -->
##### Rust:

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

> 注意：自动模式需要[Manticore Buddy](Installation/Manticore_Buddy.md)。如果无法使用，请确保 Buddy 已安装。

Manticore 提供自动表创建机制，当插入查询中指定的表尚不存在时会自动触发。该机制默认启用。若需禁用，请在 Manticore 配置文件的 [Searchd](../../Server_settings/Searchd.md#auto_schema) 部分设置 `auto_schema = 0`。

<!-- example auto-schema -->

默认情况下，`VALUES` 子句中的所有文本值均视为 `text` 类型，除非为有效的邮件地址，这种情况下视为 `string` 类型。

若 INSERT 多行时同一字段出现不同且不兼容的值类型，则自动创建表操作将取消，并返回错误信息。但如果不同的值类型兼容，则结果字段类型为能够包含所有值的类型。可能发生的一些自动数据类型转换包括：
* mva -> mva64
* uint -> bigint -> float（可能导致精度丢失）
* string -> text

此外，以下日期格式将被识别并转为时间戳，所有其他日期格式将作为字符串处理：
- `%Y-%m-%dT%H:%M:%E*S%Z`
- `%Y-%m-%d'T'%H:%M:%S%Z`
- `%Y-%m-%dT%H:%M:%E*S`
- `%Y-%m-%dT%H:%M:%s`
- `%Y-%m-%dT%H:%M`
- `%Y-%m-%dT%H`


注意，`/bulk` HTTP 端点不支持自动建表（自动模式）。仅 `/_bulk`（类似 Elasticsearch）HTTP 端点和 SQL 接口支持此功能。

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
Manticore 为插入或替换到实时或[Percolate 表](../../Creating_a_table/Local_tables/Percolate_table.md)中的文档的 ID 列提供自动 ID 生成。该生成器为文档生成唯一 ID，具备一定保证，但不应被视为自增 ID。

生成的 ID 值在以下条件下保证唯一：
* 当前服务器的 [server_id](../../Server_settings/Searchd.md#server_id) 值位于 0 到 127 范围内，且在集群节点中唯一，或使用由 MAC 地址生成的默认值作为种子
* Manticore 节点重启间系统时间不发生变动
* 在搜索服务器重启间，自动 ID 生成次数少于每秒 1600 万次

自动 ID 生成器为文档 ID 创建 64 位整数，采用以下方案：
* 第 0 到 23 位构成计数器，每次调用自动 ID 生成器时自增
* 第24至55位表示服务器启动的Unix时间戳
* 第56至63位对应server_id

此架构确保生成的ID在集群的所有节点中是唯一的，并且插入到不同集群节点的数据不会在节点之间产生冲突。

因此，用于自动ID的生成器生成的第一个ID不是1，而是一个较大的数字。此外，如果在调用之间对其他表进行插入，则插入到表中的文档流的ID值可能不是连续的，因为ID生成器在服务器中是单一的，并在其所有表之间共享。

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

`CALL UUID_SHORT(N)` 语句允许在一次调用中生成N个唯一的64位ID，而无需插入任何文档。当您需要在Manticore中预生成ID以供其他系统或存储方案使用时，这尤其有用。例如，您可以在Manticore中生成自动ID，然后在其他数据库、应用程序或工作流程中使用它们，确保不同环境中的标识符一致且唯一。

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
您不仅可以向实时表插入单个文档，还可以插入任意数量的文档。完全可以向实时表批量插入成千上万的文档。但需要注意以下几点：
* 批量越大，每次插入操作的延迟越高
* 批量越大，您可以期待更高的索引速度
* 您可能需要增加 [max_packet_size](../../Server_settings/Searchd.md#max_packet_size) 的值，以允许插入更大的批量
* 通常，每次批量插入操作被视为单个具有原子性保证的 [事务](../../Data_creation_and_modification/Transactions.md)，因此您要么一次性在表中获得所有新文档，要么如果失败，则不会添加任何文档。有关空行或切换到另一表的更多详细信息，请参见“JSON”示例。

请注意，`/bulk` HTTP端点不支持自动创建表（自动架构）。只有`/_bulk`（类似Elasticsearch）HTTP端点和SQL接口支持此功能。`/_bulk`（类似Elasticsearch）HTTP端点允许表名中包含集群名称，格式为`cluster_name:table_name`。

`/_bulk`端点接受与Elasticsearch相同格式的文档ID，您也可以在文档本身中包含`id`：
```json
{ "index": { "table" : "products", "_id" : "1" } }
{ "title" : "Crossbody Bag with Tassel", "price": 19.85 }
```

或者

```json
{ "index": { "table" : "products" } }
{ "title" : "Crossbody Bag with Tassel", "price": 19.85, "id": "1" }
```

#### /bulk中的分块传输
`/bulk`（Manticore模式）端点支持[分块传输编码](https://en.wikipedia.org/wiki/Chunked_transfer_encoding)。您可以使用它传输大型批量。它：
* 减少峰值内存使用，降低OOM风险
* 缩短响应时间
* 允许您绕过 [max_packet_size](../../Server_settings/Searchd.md#max_packet_size) 限制，传输远大于 `max_packet_size`（128MB）最大允许值的批量，例如一次传输1GB。

<!-- intro -->
### 批量插入示例
##### SQL:
<!-- request SQL -->
对于批量插入，只需在`VALUES()`后提供更多的文档。语法是：

```sql
INSERT INTO <table name>[(column1, column2, ...)] VALUES(value1[, value2 , ...]), (...)
```

可选的列名列表允许显式指定表中某些列的值。所有其它列将使用默认值（标量类型为0，字符串类型为空字符串）。

例如：

```sql
INSERT INTO products(title,price) VALUES ('Crossbody Bag with Tassel', 19.85), ('microfiber sheet set', 19.99), ('Pet Hair Remover Glove', 7.99);
```
<!-- response SQL -->

```sql
Query OK, 3 rows affected (0.01 sec)
```

当前`INSERT`不支持表达式，值应明确指定。

<!-- intro -->
##### JSON:
<!-- request JSON -->
语法通常与[插入单个文档](../../Quick_start_guide.md#Add-documents)相同。只需提供更多行，每行一个文档，使用`/bulk`端点替代`/insert`。将每个文档放在“insert”节点中。注意它还需要：
* `Content-Type: application/x-ndjson`
* 数据应格式为换行分隔JSON（NDJSON）。基本上这意味着每行应包含正好一个JSON语句，并以换行符`\n`和可能的`\r`结束。

`/bulk`端点支持“insert”、“replace”、“delete”和“update”查询。请记住，您可以对多个表执行操作，但事务只支持单个表。如果指定多个，Manticore会将指向同一表的操作聚集为一个事务。当表名更换时，它会提交已收集的操作并在新表启动新的事务。批次之间的空行也会导致提交前一批次并启动新事务。

在`/bulk`请求的响应中，您可以找到以下字段：
* "errors"：显示是否发生任何错误（true/false）
* "error"：描述发生的错误
* "current_line"：执行停止（或失败）的行号；空行包括第一个空行也计数
* "skipped_lines"：未提交行的计数，从`current_line`开始向前计算

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

或者，另选：
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
JSON 值可以作为[转义](../../Searching/Full_text_matching/Escaping.md)字符串插入（通过 SQL 或 JSON），也可以作为 JSON 对象插入（通过 JSON 接口）。

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

或者，另选：
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

