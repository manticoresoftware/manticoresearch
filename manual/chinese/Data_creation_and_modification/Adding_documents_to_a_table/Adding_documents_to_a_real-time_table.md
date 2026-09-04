# 向实时表添加文档

> 如果你要查找向普通表添加文档的信息，请参阅 [从外部存储添加数据](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md) 一节。

<!-- example insert -->
实时添加文档仅支持 [Real-Time](../../Creating_a_table/Local_tables/Real-time_table.md) 和 [percolate](../../Creating_a_table/Local_tables/Percolate_table.md) 表。相应的 SQL 命令、HTTP 端点或客户端函数会把新行（文档）及其字段值插入到表中。添加文档之前不需要先创建表。如果表不存在，Manticore 会尝试自动创建它。更多信息请参阅 [自动模式](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。

你可以插入单个或 [多个文档](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-adding-documents)，为表的所有字段提供值，也可以只提供其中一部分。在这种情况下，其余字段将使用默认值填充（标量类型为 0，文本类型为空字符串）。

`INSERT` 当前不支持表达式，因此值必须显式指定。

可以省略 ID 字段/值，因为 RT 和 PQ 表支持 [自动 ID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) 功能。对于数字 ID 表，你也可以把 `0` 作为 id 值来强制自动生成 ID。对于重复 ID，`INSERT` 不会覆盖现有行。需要这种行为时，请改用 [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md)。

对于使用 [`id uuid`](../../Creating_a_table/Data_types.md#UUID-document-IDs) 创建的表，请将显式 UUID 作为带引号的字符串传入，或者省略 `id` 以自动生成一个。显式值必须符合 `xxxxxxxx-xxxx-Vxxx-Wxxx-xxxxxxxxxxxx`，其中每个 `x` 是十六进制数字，`V` 是版本（`1` 到 `8`），`W` 是变体（`8`、`9`、`a` 或 `b`）。接受大写十六进制字母，并会规范化为小写。与数字 ID 不同，`0` 不会触发自动生成 UUID。

使用 HTTP JSON 协议时，你可以在两种不同的请求格式之间选择：一种是通用的 Manticore 格式，另一种是类似 Elasticsearch 的格式。下面的示例展示了这两种格式。

另外，在使用 Manticore JSON 请求格式时，请记住必须包含 `doc` 节点，所有值都应放在其中提供。


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
  "id": 1,
  "created": true,
  "result": "created",
  "status": 201
}
{
  "table": "products",
  "id": 2,
  "created": true,
  "result": "created",
  "status": 201
}
{
  "table": "products",
  "id": 1657860156022587406,
  "created": true,
  "result": "created",
  "status": 201
}

```

对于使用 `id uuid` 创建的表，请将 JSON `id` 作为 UUID 字符串传入，或者省略它以自动生成：

<!-- request JSON -->

```json
POST /insert
{
  "table":"products_uuid",
  "id":"550e8400-e29b-41d4-a716-446655440000",
  "doc":
  {
    "title":"Crossbody Bag with Tassel",
    "price":19.85
  }
}

POST /insert
{
  "table":"products_uuid",
  "doc":
  {
    "title":"Generated UUID Bag",
    "price":29
  }
}
```

<!-- response JSON -->

```json
{
  "table": "products_uuid",
  "id": "550e8400-e29b-41d4-a716-446655440000",
  "created": true,
  "result": "created",
  "status": 201
}
{
  "table": "products_uuid",
  "id": "<generated UUID>",
  "created": true,
  "result": "created",
  "status": 201
}
```

<!-- intro -->
##### Elasticsearch:

<!-- request Elasticsearch -->

> 注意：`_create` 需要 [Manticore Buddy](Installation/Manticore_Buddy.md)。如果它不起作用，请确保已安装 Buddy。

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
        ['id' => 1, 'title' => 'Crossbody Bag with Tassel', 'price' => 19.85],
        ['id' => 2, 'title' => 'microfiber sheet set', 'price' => 19.99],
        ['id' => 3, 'title' => 'Pet Hair Remover Glove', 'price' => 7.99]
]);
```

对于复制表，请在添加文档之前设置集群名称：

```php
// Set the cluster name
$index->setName('weekly_table')->setCluster('posts');

// Then add documents in bulk
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
    {"insert": {"table" : "products", "id" : 3, "doc" : {"title" : "Pet Hair Remover Glove", "price" : 7.99}}}
]
res = indexApi.bulk('\n'.join(map(json.dumps,docs)))
```

对于复制表，请在每个文档中包含 `cluster` 属性：

```python
docs = [ \
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}}, \
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 2, "doc" : {"title" : "microfiber sheet set", "price" : 19.99}}}, \
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 3, "doc" : {"title" : "Pet Hair Remover Glove", "price" : 7.99}}}
]
res = indexApi.bulk('\n'.join(map(json.dumps,docs)))
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
docs = [ \
    {"insert": {"table" : "products", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}}, \
    {"insert": {"table" : "products", "id" : 2, "doc" : {"title" : "microfiber sheet set", "price" : 19.99}}}, \
    {"insert": {"table" : "products", "id" : 3, "doc" : {"title" : "Pet Hair Remover Glove", "price" : 7.99}}}
]
res = await indexApi.bulk('\n'.join(map(json.dumps,docs)))
```

对于复制表，请在每个文档中包含 `cluster` 属性：

```python
docs = [ \
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}}, \
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 2, "doc" : {"title" : "microfiber sheet set", "price" : 19.99}}}, \
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 3, "doc" : {"title" : "Pet Hair Remover Glove", "price" : 7.99}}}
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
    {"insert": {"table" : "products", "id" : 5, "doc" : {"title" : "Pet Hair Remover Glove", "price" : 7.99}}}
];
res =  await indexApi.bulk(docs.map(e=>JSON.stringify(e)).join('\n'));
```

对于复制表，请在每个文档中包含 `cluster` 属性：

```javascript
let docs = [
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 3, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}},
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 4, "doc" : {"title" : "microfiber sheet set", "price" : 19.99}}},
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 5, "doc" : {"title" : "Pet Hair Remover Glove", "price" : 7.99}}}
];
res =  await indexApi.bulk(docs.map(e=>JSON.stringify(e)).join('\n'));
```

<!-- intro -->
##### java:

<!-- request Java -->

``` java
String body = "{\"insert\": {\"index\" : \"products\", \"id\" : 1, \"doc\" : {\"title\" : \"Crossbody Bag with Tassel\", \"price\" : 19.85}}}"+"\n"+
    "{\"insert\": {\"index\" : \"products\", \"id\" : 4, \"doc\" : {\"title\" : \"microfiber sheet set\", \"price\" : 19.99}}}"+"\n"+
    "{\"insert\": {\"index\" : \"products\", \"id\" : 5, \"doc\" : {\"title\" : \"Pet Hair Remover Glove\", \"price\" : 7.99}}}"+"\n";
BulkResponse bulkresult = indexApi.bulk(body);
```

对于复制表，请在每个文档中包含 `cluster` 属性：

``` java
String body = "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 1, \"doc\" : {\"title\" : \"Crossbody Bag with Tassel\", \"price\" : 19.85}}}"+"\n"+
    "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 4, \"doc\" : {\"title\" : \"microfiber sheet set\", \"price\" : 19.99}}}"+"\n"+
    "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 5, \"doc\" : {\"title\" : \"Pet Hair Remover Glove\", \"price\" : 7.99}}}"+"\n";
BulkResponse bulkresult = indexApi.bulk(body);
```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
string body = "{\"insert\": {\"index\" : \"products\", \"id\" : 1, \"doc\" : {\"title\" : \"Crossbody Bag with Tassel\", \"price\" : 19.85}}}"+"\n"+
    "{\"insert\": {\"index\" : \"products\", \"id\" : 4, \"doc\" : {\"title\" : \"microfiber sheet set\", \"price\" : 19.99}}}"+"\n"+
    "{\"insert\": {\"index\" : \"products\", \"id\" : 5, \"doc\" : {\"title\" : \"Pet Hair Remover Glove\", \"price\" : 7.99}}}"+"\n";
BulkResponse bulkresult = indexApi.Bulk(string.Join("\n", docs));
```

对于复制表，请在每个文档中包含 `cluster` 属性：

``` clike
string body = "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 1, \"doc\" : {\"title\" : \"Crossbody Bag with Tassel\", \"price\" : 19.85}}}"+"\n"+
    "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 4, \"doc\" : {\"title\" : \"microfiber sheet set\", \"price\" : 19.99}}}"+"\n"+
    "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 5, \"doc\" : {\"title\" : \"Pet Hair Remover Glove\", \"price\" : 7.99}}}"+"\n";
BulkResponse bulkresult = indexApi.Bulk(string.Join("\n", docs));
```

<!-- end -->

## 向复制表添加文档
<!-- example replicated_insert -->
在处理 [复制表](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md) 时，你必须使用特殊语法，以确保写操作能够正确传播到集群中的所有节点。

对于复制表上的所有写操作（INSERT、REPLACE、DELETE、TRUNCATE、UPDATE），你必须：
* 在 SQL 中：使用 `cluster_name:table_name` 格式，而不是只写表名
* 在 JSON 中：在 `table` 属性之外再包含 `cluster` 属性

如果不使用正确的语法，操作将因错误而失败。

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
INSERT INTO posts:weekly_table(title,price) VALUES ('Crossbody Bag with Tassel', 19.85);
INSERT INTO posts:weekly_table VALUES (0,'Yellow bag', 4.95);
```
<!-- response SQL -->

```sql
Query OK, 1 rows affected (0.00 sec)
Query OK, 1 rows affected (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /insert
{
  "cluster":"posts",
  "table":"weekly_table",
  "id":1,
  "doc":
  {
    "title" : "Crossbody Bag with Tassel",
    "price" : 19.85
  }
}

POST /insert
{
  "cluster":"posts",
  "table":"weekly_table",
  "id":0,
  "doc":
  {
    "title" : "Yellow bag",
    "price" : 4.95
  }
}
```

<!-- response JSON -->

```json
{
  "table": "weekly_table",
  "id": 1,
  "created": true,
  "result": "created",
  "status": 201
}
{
  "table": "weekly_table",
  "id": 1657860156022587406,
  "created": true,
  "result": "created",
  "status": 201
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
// Set the cluster name
$index->setName('weekly_table')->setCluster('posts');

// Then add documents
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
indexApi.insert({"cluster":"posts", "table":"weekly_table", "id":1, "doc":{"title":"Crossbody Bag with Tassel", "price":19.85}})
indexApi.insert({"cluster":"posts", "table":"weekly_table", "id":0, "doc":{"title":"Yellow bag", "price":4.95}})
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await indexApi.insert({"cluster":"posts", "table":"weekly_table", "id":1, "doc":{"title":"Crossbody Bag with Tassel", "price":19.85}});
res = await indexApi.insert({"cluster":"posts", "table":"weekly_table", "id":0, "doc":{"title":"Yellow bag", "price":4.95}});
```

<!-- intro -->
##### Java:

<!-- request Java -->

```java
InsertDocumentRequest newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","Crossbody Bag with Tassel");
    put("price",19.85);
}};
newdoc.table("weekly_table").cluster("posts").id(1L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);

newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc2 = new HashMap<String,Object>(){{
    put("title","Yellow bag");
    put("price",4.95);
}};
newdoc.table("weekly_table").cluster("posts").id(0L).setDoc(doc2);
sqlresult = indexApi.insert(newdoc);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "Crossbody Bag with Tassel");
doc.Add("price", 19.85);
InsertDocumentRequest newdoc = new InsertDocumentRequest(table: "weekly_table", cluster: "posts", id: 1, doc: doc);
var sqlresult = indexApi.Insert(newdoc);

doc = new Dictionary<string, Object>();
doc.Add("title", "Yellow bag");
doc.Add("price", 4.95);
newdoc = new InsertDocumentRequest(table: "weekly_table", cluster: "posts", id: 0, doc: doc);
sqlresult = indexApi.Insert(newdoc);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
let mut doc = HashMap::new();
doc.insert("title".to_string(), serde_json::json!("Crossbody Bag with Tassel"));
doc.insert("price".to_string(), serde_json::json!(19.85));
let mut insert_req = InsertDocumentRequest {
    table: serde_json::json!("weekly_table"),
    doc: serde_json::json!(doc),
    cluster: serde_json::json!("posts"),
    id: serde_json::json!(1),
};
let mut insert_res = index_api.insert(insert_req).await;

doc = HashMap::new();
doc.insert("title".to_string(), serde_json::json!("Yellow bag"));
doc.insert("price".to_string(), serde_json::json!(4.95));
insert_req = InsertDocumentRequest {
    table: serde_json::json!("weekly_table"),
    doc: serde_json::json!(doc),
    cluster: serde_json::json!("posts"),
    id: serde_json::json!(0),
};
insert_res = index_api.insert(insert_req).await;
```

<!-- end -->

## 自动模式

> 注意：自动模式需要 [Manticore Buddy](Installation/Manticore_Buddy.md)。如果它不起作用，请确保已安装 Buddy。

Manticore 提供自动创建表的机制，当插入或替换查询中指定的表尚不存在时会自动启用。该机制默认开启。如需关闭，请在 Manticore 配置文件的 [Searchd](../../Server_settings/Searchd.md#auto_schema) 部分将 `auto_schema = 0`。

<!-- example auto-schema -->

默认情况下，`VALUES` 子句中的所有文本值都被视为 `text` 类型，唯独表示有效电子邮件地址的值会被当作 `string` 类型。

如果你尝试对同一字段以不同且不兼容的值类型执行多行 `INSERT/REPLACE`，自动建表将被取消，并返回错误信息。不过，如果不同的值类型彼此兼容，最终生成的字段类型会选择能够容纳所有值的类型。可能发生的自动数据类型转换包括：
* mva -> mva64
* uint -> bigint -> float（这可能会造成一定精度损失）
* string -> text

自动模式机制不支持创建包含用于 [KNN](../../Searching/KNN.md#Configuring-a-table-for-KNN-search)（K-Nearest Neighbors）相似度搜索的向量字段（类型为 `float_vector`）的表。要在表中使用向量字段，你必须显式创建一个定义了这些字段的表结构。如果你需要在不具备 KNN 搜索能力的普通表中存储向量数据，可以使用标准 JSON 语法将其保存为 JSON 数组，例如：`INSERT INTO table_name (vector_field) VALUES ('[1.0, 2.0, 3.0]')`。

另外，以下日期格式会被识别并转换为时间戳，其余所有日期格式都会被当作字符串处理：
- `%Y-%m-%dT%H:%M:%E*S%Z`
- `%Y-%m-%d'T'%H:%M:%S%Z`
- `%Y-%m-%dT%H:%M:%E*S`
- `%Y-%m-%dT%H:%M:%s`
- `%Y-%m-%dT%H:%M`
- `%Y-%m-%dT%H`


请注意，`/bulk` HTTP 端点不支持自动建表（自动模式）。只有 `/_bulk`（类似 Elasticsearch）端点和 SQL 接口支持此功能。

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
{"table":"t","id":2,"created":true,"result":"created","status":201}
```

<!-- end -->

## 自动 ID
<!-- example autoid -->
Manticore 为插入或替换到实时表或 [Percolate 表](../../Creating_a_table/Local_tables/Percolate_table.md) 的文档提供自动 ID 生成。生成器会产生一个唯一的数值，并满足下面的保证，但它不应被视为自增序列。

在以下条件下，生成的 ID 值保证唯一：
* 当前服务器的 [server_id](../../Server_settings/Searchd.md#server_id) 值在 0 到 127 的范围内，并且在集群节点之间唯一，或者它使用由 MAC 地址作为种子生成的默认值
* Manticore 节点在两次服务器重启之间系统时间没有变化
* 两次服务器启动之间的平均自动 ID 生成速率保持在每秒约 1600 万个 ID 以下

自动 ID 生成器会创建一个 64 位整数，其布局如下：
* 第 0 到 23 位是计数器，每次调用自动 ID 生成器时都会递增
* 第 24 到 55 位存储服务器启动时间（秒），编码为 `(unix_timestamp_at_start - 2019-05-01 00:00:00 UTC)`
* 第 56 到 62 位存储 `server_id`（该值会被掩码到 0..127 范围）

这种布局确保在集群节点之间生成的 ID 是唯一的，并且插入到不同集群节点的数据不会发生冲突。这在处理复制表时尤其重要，因为它能保证自动生成的 ID 在复制集群的所有节点上都唯一。

重要：24 位计数器并不是你在一次服务器运行期间可插入文档总数的硬限制。启动后你可以插入超过 16,777,216 个文档；在该进程运行期间，ID 仍会继续递增并保持唯一。`每秒约 1600 万个 ID` 这一规则影响的是重启后的唯一性：重启之后，基于时间的部分必须推进到足够远，确保新生成的 ID 不会与重启前创建的 ID 重叠。

因此，用于自动 ID 的生成器产生的第一个 ID 不是 1，而是一个更大的数字。另外，由于服务器中的 ID 生成器是单例，并且在所有表之间共享，如果在两次调用之间向其他表插入了数据，那么插入到某个表中的文档流可能会出现非连续的 ID 值。

对于数字 ID 表，这个整数就是对外可见的文档 ID。对于 UUID ID 表，Manticore 会把它编码为规范的 UUIDv8 字符串；客户端看到的只有 UUID。

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

`CALL UUID_SHORT(N)` 语句允许在一次调用中生成 N 个唯一的 64 位 ID，而无需插入任何文档。它特别适合在 Manticore 中预先生成 ID，供其他系统或存储方案使用。例如，你可以在 Manticore 中生成自动 ID，然后在另一个数据库、应用程序或工作流中使用它们，从而确保不同环境之间的标识符一致且唯一。

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
你不仅可以向实时表插入单个文档，也可以按任意数量批量插入。向实时表插入成千上万条文档的批次完全没问题。不过，需要注意以下几点：
* 批次越大，每次插入操作的延迟越高
* 批次越大，预期的索引构建速度也越高
* 你可能需要增大 [max_packet_size](../../Server_settings/Searchd.md#max_packet_size) 的值，以便允许更大的批次
* 通常，每次批量插入操作都会被视为一个具有原子性保证的 [事务](../../Data_creation_and_modification/Transactions.md)，因此要么一次性把所有新文档都写入表中，要么在失败时一个都不会添加。关于空行或切换到另一张表的更多细节，请参见下方“JSON”示例。

请注意，`/bulk` HTTP 端点不支持自动创建表（自动模式）。只有 `/_bulk`（类似 Elasticsearch）端点和 SQL 接口支持此功能。`/_bulk`（类似 Elasticsearch）的 HTTP 端点允许在表名中包含集群名称，格式为 `cluster_name:table_name`。

`/_bulk` 端点接受的文档 ID 格式与 Elasticsearch 相同，你也可以把 `id` 放在文档本身中：
```json
{ "index": { "table": "products", "_id": "1" } }
{ "title": "Crossbody Bag with Tassel", "price": 19.85 }
```

或者

```json
{ "index": { "table": "products" } }
{ "title": "Crossbody Bag with Tassel", "price": 19.85, "id": "1" }
```

对于使用 `id uuid` 声明的 RT 表，`/bulk` 从 `id` 读取 UUID。`/_bulk` 从元数据 `_id` 或文档 `id` 中读取 UUID。两个端点都可以省略 ID 以自动生成 UUID。

#### /bulk 中的分块传输
`/bulk`（Manticore 模式）端点支持 [分块传输编码](https://en.wikipedia.org/wiki/Chunked_transfer_encoding)。你可以用它来传输大批量数据。它：
* 降低峰值 RAM 占用，减少 OOM 风险
* 缩短响应时间
* 允许你绕过 [max_packet_size](../../Server_settings/Searchd.md#max_packet_size)，传输远大于 `max_packet_size` 允许上限（128MB）的批次，例如一次 1GB。

### 批量导入

对于向本地实时表进行更快的大批量加载，Manticore Search 可以直接把 `INSERT` 行写入磁盘块，并在事务提交时发布该块。这避免了先在 RAM 块中构建批次。行在块发布之前保持不可见，若操作失败或执行 `ROLLBACK`，表将保持不变。

它同时支持行式表和列式表，包括全文字段、数值属性、字符串、JSON、MVA/MVA64，以及带 KNN 索引的浮点向量。

这里表名不区分大小写。例如，`SET bulk_import=Products` 和 `bulk_import=products` 都会选择存储为 `products` 的表；仅大小写不同的表名无法区分。

#### SQL

为当前 SQL 会话启用该模式，针对绑定的表运行一个或多个 `INSERT` 语句，然后提交：

```sql
SET bulk_import=products;
INSERT INTO products(id,title,price) VALUES
  (101,'Crossbody Bag with Tassel',19.85),
  (102,'Microfiber Sheet Set',19.99);
INSERT INTO products(id,title,price) VALUES
  (103,'Pet Hair Remover Glove',7.99);
COMMIT;
SET bulk_import=0;
```

在 `BEGIN` 之前以及该连接上的任何未提交写入之前运行 `SET bulk_import=<table>`。它会为该会话保留选定的表：搜索仍可正常进行，而对同一表的另一次批量导入或普通写入会被拒绝，直到保留被释放为止。`BEGIN` 和 `START TRANSACTION` 允许使用，但不是必需的，在此模式下也没有作用。

`COMMIT` 会把自上一个 `COMMIT` 或 `ROLLBACK` 以来收集的行作为一个磁盘块发布。`ROLLBACK` 会丢弃这些行。在这两种情况下，批量导入都会保持启用，因此你可以开始下一批。运行 `SET bulk_import=0` 可以丢弃所有未提交的行、禁用批量导入，并释放此连接的保留。保留释放后，对该表的写入将恢复。关闭连接也会执行同样的操作。在批量导入启用时，不能更改 autocommit。

#### HTTP `/bulk`

在 Manticore `/bulk` 请求中添加 `bulk_import=<table>`。请求体仍然是标准的换行分隔 JSON（NDJSON），并且每个操作都必须是绑定表的 `insert` 或 `create`：

```bash
printf '%s\n' \
  '{"insert":{"table":"products","id":101,"doc":{"title":"Crossbody Bag with Tassel","price":19.85}}}' \
  '{"insert":{"table":"products","id":102,"doc":{"title":"Microfiber Sheet Set","price":19.99}}}' |
curl -sS -X POST \
  -H 'Content-Type: application/x-ndjson' \
  --data-binary @- \
  'http://localhost:9308/bulk?bulk_import=products'
```

在请求体中，空行会结束并发布当前批次；请求结束会发布最后一个批次。Manticore 会在处理完该请求中的所有批次后再发送 HTTP 响应。如果后面的批次失败，同一请求中较早已发布的批次仍然可搜索。要把整个请求作为一个磁盘块原子性发布，请不要包含空行。响应会为每个已发布的批次返回一个汇总的 `bulk` 结果，而不是每个文档一个结果。

大多数客户端应该像上面的示例那样发送单个请求。如果应用故意在同一个 [持久 HTTP 连接](../../Connecting_to_the_server/HTTP.md#Persistent-connections) 上发送多个请求，第一个请求会选择表。该连接上的后续 `/bulk` 或 `/json/bulk` 请求可以省略 `bulk_import`，但它们必须继续写入同一张表。完成后关闭连接，或者发送一个空的 `/bulk?bulk_import=0` 请求来禁用批量导入并释放该连接的保留。保留释放后，对该表的写入将恢复。

该端点支持分块传输编码，因此它可以处理大于 `max_packet_size` 的请求体，而无需把整个请求缓存在内存中。

#### Elasticsearch `/_bulk`

兼容 Elasticsearch 的 `/_bulk` 端点不支持直接写入磁盘的 `bulk_import`；请改用 SQL 或 Manticore `/bulk`。在干净会话中，`?pipeline=bulk_import` 和旧的 `?bulk_import=1` 都会被拒绝。其他未知的 `bulk_import` 值会被忽略，普通的 Elasticsearch bulk 处理会继续进行。具有活动批量导入的连接不能切换到 `/_bulk`。

#### 重复文档 ID

在一个直接写入磁盘的批次中，数字文档 ID 的第一行会保持可见，后续相同 ID 的行会在逻辑上被移除。对于 SQL，批次由 `COMMIT` 或 `ROLLBACK` 之前暂存的行组成。对于 Manticore `/bulk`，每个在空行、表切换或请求 EOF 时发布的分组都是一个独立批次。

当 Manticore 将磁盘块发布到目标表时，该块中的一行会替换任何现有的同 ID 行。这同样适用于先前 HTTP 批次发布的 ID。因此，重试之前已发布的批次时，会用其行替换现有内容，而重试批次内部的重复项仍然会保留第一行。在 Manticore `/bulk` 请求中，例如 `create` 操作：

```json
{"create":{"table":"products",...}}
```

其行为与 `insert` 相同；仅仅因为 ID 已存在，它不会失败。

#### 暂存文件与清理

在正常执行 `COMMIT`、`ROLLBACK`、禁用模式或关闭会话后，Manticore 会删除当前暂存目录。守护进程或主机崩溃可能会遗留一个废弃的暂存目录。后续的直接写入磁盘加载会创建一个新的唯一命名目录，不会重用或附加崩溃前加载留下的文件。

使用下面的查询查看某个表的所有直接写入磁盘暂存条目：

```sql
SELECT file, normalized, size
FROM products.@files
OPTION format='bulk_import';
```

结果会递归列出表的直接写入磁盘暂存根目录下的文件和目录。目录及其他非常规条目的报告大小为 `0`。

确认该表当前没有进行直接写入磁盘加载后，可使用 [`PURGE BULK_IMPORT`](../../Node_info_and_management/PURGE.md#PURGE-BULK_IMPORT) 删除其整个暂存根目录：

```sql
PURGE BULK_IMPORT FROM TABLE products;
```

`PURGE` 需要一个存在的本地实时表，且该表不属于复制集群。它只会移除直接写入磁盘的暂存状态，不会改变表结构或已索引行。在暂存根目录不存在时，它会以无操作的方式成功返回。在无配置模式下，`DROP TABLE` 也会移除该表的直接写入磁盘暂存根目录。

#### 当前限制

* 目标必须是一个已存在、未冻结的本地实时表，且不能是复制集群成员。不支持分布式表、分片表、复制表、percolate 表和普通表。
* SQL 仅支持 `INSERT`。Manticore `/bulk` 接受 `insert` 和 `create`；`index`、`replace`、`update` 和 `delete` 会被拒绝。
* 每一行都必须提供显式的非零数字文档 ID。不支持自动生成的 ID 和 UUID 文档 ID。
* 不支持静态构建版本。
* 平台相关的可执行文件（Linux 上的 `indexer`，Windows 上的 `indexer.exe`）必须与正在运行的 `searchd` 可执行文件位于同一目录。Manticore Search 只会解析这个同级路径，不会搜索 `PATH`。请使用与 `searchd` 相同安装中的可执行文件以确保兼容性。如果它缺失、不可读或无法启动，则只有批量导入会失败；正常启动和常规插入仍可使用。

<!-- intro -->
### 批量插入示例
##### SQL:
<!-- request SQL -->
进行批量插入时，只需在 `VALUES()` 后的括号中提供更多文档即可。语法如下：

```sql
INSERT INTO <table name>[(column1, column2, ...)] VALUES(value1[, value2 , ...]), (...)
```

可选的列名列表允许你为表中部分列显式指定值。其他所有列都将使用默认值填充（标量类型为 0，字符串类型为空字符串）。

例如：

```sql
INSERT INTO products(title,price) VALUES ('Crossbody Bag with Tassel', 19.85), ('microfiber sheet set', 19.99), ('Pet Hair Remover Glove', 7.99);
```
<!-- response SQL -->

```sql
Query OK, 3 rows affected (0.01 sec)
```

`INSERT` 当前不支持表达式，值应显式指定。

<!-- intro -->
##### JSON:
<!-- request JSON -->
语法与 [插入单个文档](../../Quick_start_guide.md#Add-documents) 基本相同。只需提供更多行，每个文档一行，并使用 `/bulk` 端点替代 `/insert`。将每个文档包裹在 "insert" 节点中。请注意，它还要求：
* `Content-Type: application/x-ndjson`
* 数据应格式化为换行分隔 JSON（NDJSON）。本质上，这意味着每一行都必须恰好包含一个 JSON 语句，并以换行符 `\n` 结束，可能还包含 `\r`。

`/bulk` 端点支持 'insert'、'replace'、'delete' 和 'update' 查询。请记住，你可以把操作发送到多个表，但事务只能针对单个表。如果你指定了多个表，Manticore 会把发往同一表的操作收集到一个事务中。当表发生变化时，它会提交已收集的操作并启动新的事务。分隔批次的空行也会导致提交前一个批次并启动新的事务。一个不包含任何操作的请求，无论其主体为空还是只包含空行/纯空白行，都会成功作为无操作处理，并返回一个空的 `items` 数组。

在 `/bulk` 请求的响应中，你可以找到以下字段：
* "errors"：表示是否发生错误（true/false）
* "error"：描述发生的错误
* "current_line"：执行停止（或失败）的位置所在行号；空行也会计数，包括第一条空行
* "skipped_lines"：未提交行的数量，从 `current_line` 开始向前回溯统计

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

对于复制表，请在每个操作中包含 `cluster` 属性：

```json
POST /bulk
-H "Content-Type: application/x-ndjson" -d '
{"insert": {"cluster":"posts", "table":"weekly_table", "id":1, "doc":  {"title":"Crossbody Bag with Tassel","price":19.85}}}
{"insert": {"cluster":"posts", "table":"weekly_table", "id":2, "doc":  {"title":"microfiber sheet set","price":19.99}}}
{"insert": {"cluster":"posts", "table":"weekly_table", "id":3, "doc":  {"title":"Pet Hair Remover Glove","price":7.99}}}
'
```

<!-- request Elasticsearch -->

> 注意：如果表尚不存在，`_bulk` 需要 [Manticore Buddy](Installation/Manticore_Buddy.md)。如果它不起作用，请确保已安装 Buddy。

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

对于复制表，请使用 `cluster_name:table_name` 格式把集群名称包含在表名中：

```json
POST /_bulk
-H "Content-Type: application/x-ndjson" -d '
{ "index" : { "table" : "posts:weekly_table" } }
{ "title" : "Yellow Bag", "price": 12 }
{ "create" : { "table" : "posts:weekly_table" } }
{ "title" : "Red Bag", "price": 12.5, "id": 3 }
'
```

<!-- intro -->
##### PHP:
<!-- request PHP -->
使用 `addDocuments()` 方法：

```php
$index->addDocuments([
        ['id' => 1, 'title' => 'Crossbody Bag with Tassel', 'price' => 19.85],
        ['id' => 2, 'title' => 'microfiber sheet set', 'price' => 19.99],
        ['id' => 3, 'title' => 'Pet Hair Remover Glove', 'price' => 7.99]
]);
```

对于复制表，请在添加文档之前设置集群名称：

```php
// Set the cluster name
$index->setName('weekly_table')->setCluster('posts');

// Then add documents in bulk
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
    {"insert": {"table" : "products", "id" : 3, "doc" : {"title" : "Pet Hair Remover Glove", "price" : 7.99}}}
]
res = indexApi.bulk('\n'.join(map(json.dumps,docs)))
```

对于复制表，请在每个文档中包含 `cluster` 属性：

```python
docs = [ \
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}}, \
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 2, "doc" : {"title" : "microfiber sheet set", "price" : 19.99}}}, \
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 3, "doc" : {"title" : "Pet Hair Remover Glove", "price" : 7.99}}}
]
res = indexApi.bulk('\n'.join(map(json.dumps,docs)))
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
docs = [ \
    {"insert": {"table" : "products", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}}, \
    {"insert": {"table" : "products", "id" : 2, "doc" : {"title" : "microfiber sheet set", "price" : 19.99}}}, \
    {"insert": {"table" : "products", "id" : 3, "doc" : {"title" : "Pet Hair Remover Glove", "price" : 7.99}}}
]
res = await indexApi.bulk('\n'.join(map(json.dumps,docs)))
```

对于复制表，请在每个文档中包含 `cluster` 属性：

```python
docs = [ \
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}}, \
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 2, "doc" : {"title" : "microfiber sheet set", "price" : 19.99}}}, \
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 3, "doc" : {"title" : "Pet Hair Remover Glove", "price" : 7.99}}}
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
    {"insert": {"table" : "products", "id" : 5, "doc" : {"title" : "Pet Hair Remover Glove", "price" : 7.99}}}
];
res =  await indexApi.bulk(docs.map(e=>JSON.stringify(e)).join('\n'));
```

对于复制表，请在每个文档中包含 `cluster` 属性：

```javascript
let docs = [
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 3, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}},
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 4, "doc" : {"title" : "microfiber sheet set", "price" : 19.99}}},
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 5, "doc" : {"title" : "Pet Hair Remover Glove", "price" : 7.99}}}
];
res =  await indexApi.bulk(docs.map(e=>JSON.stringify(e)).join('\n'));
```

<!-- intro -->
##### java:

<!-- request Java -->

``` java
String body = "{\"insert\": {\"index\" : \"products\", \"id\" : 1, \"doc\" : {\"title\" : \"Crossbody Bag with Tassel\", \"price\" : 19.85}}}"+"\n"+
    "{\"insert\": {\"index\" : \"products\", \"id\" : 4, \"doc\" : {\"title\" : \"microfiber sheet set\", \"price\" : 19.99}}}"+"\n"+
    "{\"insert\": {\"index\" : \"products\", \"id\" : 5, \"doc\" : {\"title\" : \"Pet Hair Remover Glove\", \"price\" : 7.99}}}"+"\n";
BulkResponse bulkresult = indexApi.bulk(body);
```

对于复制表，请在每个文档中包含 `cluster` 属性：

``` java
String body = "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 1, \"doc\" : {\"title\" : \"Crossbody Bag with Tassel\", \"price\" : 19.85}}}"+"\n"+
    "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 4, \"doc\" : {\"title\" : \"microfiber sheet set\", \"price\" : 19.99}}}"+"\n"+
    "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 5, \"doc\" : {\"title\" : \"Pet Hair Remover Glove\", \"price\" : 7.99}}}"+"\n";
BulkResponse bulkresult = indexApi.bulk(body);
```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
string body = "{\"insert\": {\"index\" : \"products\", \"id\" : 1, \"doc\" : {\"title\" : \"Crossbody Bag with Tassel\", \"price\" : 19.85}}}"+"\n"+
    "{\"insert\": {\"index\" : \"products\", \"id\" : 4, \"doc\" : {\"title\" : \"microfiber sheet set\", \"price\" : 19.99}}}"+"\n"+
    "{\"insert\": {\"index\" : \"products\", \"id\" : 5, \"doc\" : {\"title\" : \"Pet Hair Remover Glove\", \"price\" : 7.99}}}"+"\n";
BulkResponse bulkresult = indexApi.Bulk(string.Join("\n", docs));
```

对于复制表，请在每个文档中包含 `cluster` 属性：

``` clike
string body = "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 1, \"doc\" : {\"title\" : \"Crossbody Bag with Tassel\", \"price\" : 19.85}}}"+"\n"+
    "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 4, \"doc\" : {\"title\" : \"microfiber sheet set\", \"price\" : 19.99}}}"+"\n"+
    "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 5, \"doc\" : {\"title\" : \"Pet Hair Remover Glove\", \"price\" : 7.99}}}"+"\n";
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

或者
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
JSON 值可以作为 [转义](../../Searching/Full_text_matching/Escaping.md) 字符串插入（通过 SQL 或 JSON），也可以作为 JSON 对象插入（通过 JSON 接口）。

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

JSON 值也可以作为包含已转义 JSON 的字符串插入：
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

或者
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
将 JSON 仅视为字符串：
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
