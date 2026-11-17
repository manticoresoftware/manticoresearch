# 向实时表添加文档

> 如果您想了解如何向普通表添加文档，请参阅[从外部存储添加数据](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)一节。

<!-- example insert -->
只有[实时表](../../Creating_a_table/Local_tables/Real-time_table.md)和 [percolate 表](../../Creating_a_table/Local_tables/Percolate_table.md)支持实时添加文档。相应的 SQL 命令、HTTP 端点或客户端函数会使用提供的字段值向表中插入新行（文档）。添加文档前不必先创建表。如果表不存在，Manticore 会尝试自动创建。详情请参阅[自动表结构](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。

您可以插入单个或[多个文档](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-adding-documents)，并为表中的全部或部分字段提供值。未指定的字段将填入默认值（标量类型为 0，文本类型为空字符串）。

`INSERT` 目前不支持表达式，因此必须显式指定值。

可以省略 ID 字段及其值，因为 RT 和 PQ 表支持[自动 ID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) 功能。对于使用数值 ID 的表，也可以将 id 值设为 `0`，强制自动生成 ID。`INSERT` 不会覆盖 ID 重复的行。如需覆盖，请使用 [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md)。

对于使用 [`id uuid`](../../Creating_a_table/Data_types.md#UUID-document-IDs) 创建的表，可以用带引号的字符串显式传入 UUID，也可以省略 `id` 以自动生成。显式指定的值必须符合 `xxxxxxxx-xxxx-Vxxx-Wxxx-xxxxxxxxxxxx` 格式，其中每个 `x` 是十六进制数字，`V` 是版本（`1` 至 `8`），`W` 是变体（`8`、`9`、`a` 或 `b`）。允许使用大写十六进制字母，但会统一转换为小写。与数值 ID 不同，`0` 不会触发自动生成 UUID。

使用 HTTP JSON 协议时，可以选择两种请求格式：Manticore 通用格式和类似 Elasticsearch 的格式。下面的示例展示了这两种格式。

另外，使用 Manticore JSON 请求格式时，请注意必须包含 `doc` 节点，并将所有值放在该节点中。


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

对于使用 `id uuid` 创建的表，请将 JSON 中的 `id` 设为 UUID 字符串，或省略它以自动生成：

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

> 注意：`_create` 需要 [Manticore Buddy](Installation/Manticore_Buddy.md)。如果无法使用，请确认已安装 Buddy。

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

对于复制表，请在添加文档前设置集群名称：

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

对于复制表，请在每个文档中包含 cluster 属性：

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

对于复制表，请在每个文档中包含 cluster 属性：

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

对于复制表，请在每个文档中包含 cluster 属性：

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

对于复制表，请在每个文档中包含 cluster 属性：

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

对于复制表，请在每个文档中包含 cluster 属性：

``` clike
string body = "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 1, \"doc\" : {\"title\" : \"Crossbody Bag with Tassel\", \"price\" : 19.85}}}"+"\n"+
    "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 4, \"doc\" : {\"title\" : \"microfiber sheet set\", \"price\" : 19.99}}}"+"\n"+
    "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 5, \"doc\" : {\"title\" : \"Pet Hair Remover Glove\", \"price\" : 7.99}}}"+"\n";
BulkResponse bulkresult = indexApi.Bulk(string.Join("\n", docs));
```

<!-- end -->

## 向复制表添加文档
<!-- example replicated_insert -->
使用[复制表](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md)时，必须采用专用语法，确保写入操作正确传播到集群中的所有节点。

对复制表执行任何写入操作（INSERT、REPLACE、DELETE、TRUNCATE、UPDATE）时，都必须：
* 在 SQL 中：使用 `cluster_name:table_name` 格式，不能只写表名
* 在 JSON 中：同时包含 `cluster` 属性和 `table` 属性

如果未使用正确的语法，操作将失败并报错。

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

## 自动表结构

> 注意：自动表结构功能需要 [Manticore Buddy](Installation/Manticore_Buddy.md)。如果无法使用，请确认已安装 Buddy。

Manticore 提供自动建表机制：当插入或替换查询指定的表尚不存在时，该机制就会触发。此功能默认启用。如需禁用，请在 Manticore 配置文件的 [Searchd](../../Server_settings/Searchd.md#auto_schema) 部分设置 `auto_schema = 0`。

<!-- example auto-schema -->

默认情况下，`VALUES` 子句中的所有文本值都视为 `text` 类型，但有效的电子邮件地址除外，这类值会视为 `string` 类型。

如果尝试通过 INSERT/REPLACE 写入多行数据，而同一字段的值类型不同且互不兼容，自动建表将取消并返回错误消息。如果这些类型兼容，则最终字段类型会选择能够容纳所有值的类型。可能发生的自动类型转换包括：
* mva -> mva64
* uint -> bigint -> float（可能损失部分精度）
* string -> text

自动表结构机制不支持创建包含向量字段（`float_vector` 类型字段）的表，这类字段用于 [KNN](../../Searching/KNN.md#Configuring-a-table-for-KNN-search)（K 近邻）相似度搜索。要在表中使用向量字段，必须显式创建表，并在表结构中定义这些字段。如果只需在常规表中存储向量数据而不需要 KNN 搜索，可以使用标准 JSON 语法将其存储为 JSON 数组，例如：`INSERT INTO table_name (vector_field) VALUES ('[1.0, 2.0, 3.0]')`。

此外，以下日期格式会被识别并转换为时间戳，其他日期格式均视为字符串：
- `%Y-%m-%dT%H:%M:%E*S%Z`
- `%Y-%m-%d'T'%H:%M:%S%Z`
- `%Y-%m-%dT%H:%M:%E*S`
- `%Y-%m-%dT%H:%M:%s`
- `%Y-%m-%dT%H:%M`
- `%Y-%m-%dT%H`


请注意，`/bulk` HTTP 端点不支持自动建表（自动表结构）。只有 `/_bulk`（类似 Elasticsearch）端点和 SQL 接口支持此功能。

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
Manticore 可为插入或替换到实时表或 [percolate 表](../../Creating_a_table/Local_tables/Percolate_table.md)中的文档自动生成 ID。生成器会在满足下述条件时生成唯一的数值，但不应将其视为自增序列。

在以下条件下，可以保证生成的 ID 值唯一：
* 当前服务器的 [server_id](../../Server_settings/Searchd.md#server_id) 值在 0 到 127 之间，且在集群节点中唯一；或者使用以 MAC 地址为种子生成的默认值
* 在两次服务器重启之间，Manticore 节点的系统时间未被调整
* 在两次服务器启动之间，自动 ID 的平均生成速率保持在每秒约 1600 万个以下

自动 ID 生成器创建的 64 位整数采用以下布局：
* 第 0 到 23 位是计数器，每次调用自动 ID 生成器时递增
* 第 24 到 55 位存储以秒为单位的服务器启动时间，编码为 `(unix_timestamp_at_start - 2019-05-01 00:00:00 UTC)`
* 第 56 到 62 位存储 `server_id`（通过掩码将值限定在 0..127 范围内）

此布局确保生成的 ID 在集群节点间唯一，向不同集群节点插入的数据不会产生 ID 冲突。这对复制表尤为重要，因为它保证了自动生成的 ID 在整个复制集群的所有节点中唯一。

重要：24 位计数器并不限制单次服务器运行期间可插入的文档总数。启动后可以插入超过 16,777,216 个文档；对于该运行中的进程，ID 仍会持续递增并保持唯一。“每秒约 1600 万个 ID”的规则影响的是跨重启的唯一性：重启后，基于时间的部分必须增加足够多，才能避免新生成的 ID 与重启前生成的 ID 重叠。

因此，自动 ID 生成器生成的第一个 ID 并不是 1，而是一个较大的数值。此外，如果在两次调用之间向其他表插入了文档，同一表中连续插入的文档也可能具有不连续的 ID，因为服务器只有一个 ID 生成器，由所有表共享。

对于使用数值 ID 的表，这个整数就是对外提供的文档 ID。对于使用 UUID ID 的表，Manticore 会将其编码为标准的 UUIDv8 字符串，客户端只能看到 UUID。

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
### 使用 UUID_SHORT 批量生成 ID

```sql
CALL UUID_SHORT(N)
```

`CALL UUID_SHORT(N)` 语句可在一次调用中生成 N 个唯一的 64 位 ID，无需插入任何文档。当需要在 Manticore 中预先生成 ID，供其他系统或存储方案使用时，这一功能尤其有用。例如，可以在 Manticore 中生成自动 ID，再将其用于其他数据库、应用程序或工作流程，确保不同环境中的标识符一致且唯一。

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
您不仅可以向实时表插入单个文档，也可以插入任意数量的文档。一次向实时表插入数万个文档完全可行，但请注意以下几点：
* 批次越大，每次插入操作的延迟越高
* 批次越大，通常索引构建速度越快
* 如需支持更大的批次，可以增大 [max_packet_size](../../Server_settings/Searchd.md#max_packet_size) 的值
* 通常，每次批量插入都视为一个具有原子性保证的[事务](../../Data_creation_and_modification/Transactions.md)：要么所有新文档一次性出现在表中，要么在失败时一个也不添加。有关空行或切换到其他表的详细行为，请参阅“JSON”示例。

请注意，`/bulk` HTTP 端点不支持自动建表（自动表结构）。只有 `/_bulk`（类似 Elasticsearch）端点和 SQL 接口支持此功能。`/_bulk`（类似 Elasticsearch）HTTP 端点允许在表名中以 `cluster_name:table_name` 格式包含集群名称。

`/_bulk` 端点接受与 Elasticsearch 相同格式的文档 ID，也可以在文档本身中包含 `id`：
```json
{ "index": { "table": "products", "_id": "1" } }
{ "title": "Crossbody Bag with Tassel", "price": 19.85 }
```

或

```json
{ "index": { "table": "products" } }
{ "title": "Crossbody Bag with Tassel", "price": 19.85, "id": "1" }
```

对于声明为 `id uuid` 的 RT 表，`/bulk` 从 `id` 读取 UUID，`/_bulk` 从元数据 `_id` 或文档的 `id` 中读取。两个端点都可以省略 ID，以自动生成 UUID。

#### /bulk 中的分块传输
`/bulk`（Manticore 模式）端点支持[分块传输编码](https://en.wikipedia.org/wiki/Chunked_transfer_encoding)，可用于传输大批量数据。它可以：
* 降低内存峰值用量，减少内存不足（OOM）的风险
* 缩短响应时间
* 绕过 [max_packet_size](../../Server_settings/Searchd.md#max_packet_size) 限制，传输远大于 `max_packet_size` 最大允许值（128MB）的批次，例如一次传输 1GB。

### 批量导入

为加快向本地实时表加载大批量数据，Manticore Search 可以将 `INSERT` 行直接写入磁盘块，并在事务提交时发布该块。这样无需先在内存块中构建整个批次。在磁盘块发布前，这些行不可见；操作失败或执行 `ROLLBACK` 时，表保持不变。

此功能同时支持行式表和列式表，包括全文字段、数值属性、字符串、JSON、MVA/MVA64，以及带 KNN 索引的浮点向量。

此处的表名不区分大小写。例如，`SET bulk_import=Products` 和 `bulk_import=products` 都会选中存储名称为 `products` 的表；无法区分仅字母大小写不同的表名。

#### SQL

为当前 SQL 会话启用此模式，对绑定的表执行一条或多条 `INSERT` 语句，然后提交：

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

请在 `BEGIN` 之前、且该连接尚无任何未提交写入时执行 `SET bulk_import=<table>`。这会为当前会话独占该表的写入权限：搜索仍可使用，但在占用解除前，对同一表执行其他批量导入或常规写入都会被拒绝。允许使用 `BEGIN` 和 `START TRANSACTION`，但并非必需，且在此模式下不起作用。

`COMMIT` 会将自上次 `COMMIT` 或 `ROLLBACK` 以来收集的行作为一个磁盘块发布。`ROLLBACK` 则丢弃这些行。两种操作都不会关闭批量导入模式，因此可以继续下一个批次。执行 `SET bulk_import=0` 可丢弃所有未提交的行、关闭批量导入，并解除此连接对表的占用。占用解除后即可恢复对该表的写入。关闭连接也有相同效果。启用批量导入期间无法更改自动提交设置。

#### HTTP `/bulk`

在 Manticore `/bulk` 请求中添加 `bulk_import=<table>`。请求体仍采用标准的换行分隔 JSON（NDJSON），且每个操作都必须是针对绑定表的 `insert` 或 `create`：

```bash
printf '%s\n' \
  '{"insert":{"table":"products","id":101,"doc":{"title":"Crossbody Bag with Tassel","price":19.85}}}' \
  '{"insert":{"table":"products","id":102,"doc":{"title":"Microfiber Sheet Set","price":19.99}}}' |
curl -sS -X POST \
  -H 'Content-Type: application/x-ndjson' \
  --data-binary @- \
  'http://localhost:9308/bulk?bulk_import=products'
```

每行也可以直接是一个文档：表由 `bulk_import=<table>` 指定，`id` 是文档字段之一：

```bash
printf '%s\n' \
  '{"id":101,"title":"Crossbody Bag with Tassel","price":19.85}' \
  '{"id":102,"title":"Microfiber Sheet Set","price":19.99}' |
curl -sS -X POST \
  -H 'Content-Type: application/x-ndjson' \
  --data-binary @- \
  'http://localhost:9308/bulk?bulk_import=products'
```

直接提供的文档中，每个字段都对应一列，包括名为 `doc` 的字段；没有 `id` 的文档会获得[自动生成的 ID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID)。如果某行只有一个字段，字段名为 `/bulk` 操作名（`insert`、`create`、`index`、`replace`、`update` 或 `delete`），且其值是包含 `table` 或 `index` 的对象，则该行视为操作；其他行均视为直接提供的文档。两种形式可以在同一个请求中混用。不含 `bulk_import` 的 `/bulk` 请求仍必须使用操作形式。

在请求体中，空行会结束并发布当前批次；请求结束时会发布最后一个批次。Manticore 处理完该请求中的所有批次后才发送 HTTP 响应。如果后续批次失败，同一请求中先前已发布的批次仍可搜索。如需将整个请求作为一个磁盘块原子发布，请勿包含空行。响应会为每个已发布批次返回一个汇总的 `bulk` 结果，而非为每个文档返回一个结果。

常规导入时，按上例在一个请求中发送完整的 NDJSON 请求体。导入完成后关闭 HTTP 连接，以解除对表的占用，允许其他写入。

该端点支持分块传输编码，因此无需缓冲整个请求，即可处理大于 `max_packet_size` 的请求体。

#### 共享 HTTP 批量导入

当多个并发请求向同一张表加载数据时，请在 `bulk_import=<table>` 旁添加 `shared`，例如日志采集器每次刷新都通过新连接发送数据的场景。所有此类请求都会将数据送入该表共享的索引构建进程池。此池不属于某个连接：它会跨请求、连接和重连持续存在，直到被提交、通过 `PURGE` 取消、表被删除，或守护进程停止。

```bash
curl -sS -X POST -H 'Content-Type: application/x-ndjson' --data-binary @part1.ndjson \
  'http://localhost:9308/bulk?bulk_import=products&shared=4'
```

每行必须是针对目标表的 `insert` 或 `create` 操作，或上文所述的直接提供的文档。第一个文档会创建进程池并启动索引构建进程。数据请求成功响应（`"shared_state":"staged"`）表示文档已暂存，尚不可搜索：请求结束、连接关闭和 HTTP 分块边界都不会发布任何数据。

从第一个文档到进程池提交、失败或取消为止，进程池会像独占式批量导入一样占用表：搜索继续可用，但常规写入（`INSERT`、`REPLACE`、`UPDATE`、`DELETE`）以及对同一表的其他批量导入都会失败，并报错提示表已锁定。关闭连接不会解除占用。[`SHOW LOCKS`](../../Securing_and_compacting_a_table/Freezing_and_locking_a_table.md#SHOW-LOCKS) 会列出被占用的表，锁类型为 `write_allow_readers`。如果表正被另一个批量导入占用、通过 `LOCK TABLES … READ` 锁定，或正在执行常规写入，则新进程池的第一个文档会被拒绝，返回 `shared_precondition` 和相同的错误。

发送一个不含文档但包含空行的请求，即可提交进程池：

```bash
printf '\n' | curl -sS -X POST -H 'Content-Type: application/x-ndjson' --data-binary @- \
  'http://localhost:9308/bulk?bulk_import=products&shared'
```

提交会停止接收新数据，停止未收到数据的索引构建进程，等待其余各进程完成各自部分的构建，为每个有数据的进程挂载一个磁盘块，然后移除进程池并返回 `"shared_state":"published"`。如果其他数据请求仍在运行，提交会返回 HTTP `409` 和 `shared_busy`，进程池继续开放；请在写入请求完成后重试提交。没有进程池时，提交会返回 `409` 和 `shared_no_pool`，因为提交不会创建进程池；未收到数据的进程池不会发布磁盘块。含文档请求中的空行（包括仅含空格的行）会被忽略，因此数据请求永远不会触发发布。LF 和 CRLF 均可作为行结束符。

索引构建进程数（也就是新增磁盘块数）由创建进程池的请求中的 `shared=N` 设置。只提供 `shared` 而不指定值时，守护进程会使用 `searchd` 的 `INDEXER_RT_BULK_SHARED_POOL_SIZE` 环境变量；若未设置，则使用一个索引构建进程。N 的范围为 1 到工作[线程](../../Server_settings/Searchd.md#threads)数减 2（上限至少为 1）：值过小或不是数字时使用一个进程，值过大时使用上限。只有创建进程池的请求会读取 N；加入已有进程池的请求和提交请求都会忽略它。数据以约 32 KiB 的批次轮流分配给各索引构建进程。增加进程数可缩短提交后的构建时间，但加载期间会占用更多 CPU 和内存。建议从工作线程数的约三分之二开始，例如 12 个线程配 8 个索引构建进程；再增加时，这些进程会与传入请求争用 CPU 和磁盘，反而降低加载速度。每个索引构建进程在加载期间大约需要 200 MB 内存。

每个响应都是一个 JSON 对象，包含 `errors`、`error`、`error_code` 和 `shared_state`：

| `shared_state` | HTTP | `error_code` | 含义 |
| --- | --- | --- | --- |
| `staged` | 200 | | 数据已存入进程池 |
| `published` | 200 | | 进程池已提交 |
| `busy` | 409 | `shared_busy` | 提交时仍有其他数据请求在运行 |
| `closing` | 409 | `shared_closing` | 正在发布进程池中的数据 |
| `no_pool` | 409 | `shared_no_pool` | 没有进程池时执行提交，或发送带 `shared_join` 的数据请求 |
| `rejected` | 400，或在无写入权限时为 403 | `shared_precondition` | 请求无效，例如没有写入权限，或创建进程池时表已锁定 |
| `failed` | 400、409 或 500 | `shared_failed` | 加载失败，详见下文 |

`current_line` 统计请求中的文档数；提交时报告为 `1`。出错时，`skipped_lines` 等于 `current_line`，但该请求中已经交给索引构建进程的批次仍保留在进程池中。

如果连接在请求体传输过程中断开或超时，或请求被 `KILL` 终止，只会导致该请求失败：最后尚未写入的部分（小于 32 KiB）会被丢弃，已经交给索引构建进程的数据会保留，进程池也继续开放，因此客户端可以重发请求。文档无效（JSON 格式错误、未知列、指定了其他表，或使用了 `insert`/`create` 以外的操作）或向索引构建进程写入失败，则会导致整个加载失败：所有索引构建进程立即停止，并解除对表的占用；文档无效时该请求返回 `400`，写入失败时返回 `500`；之后的所有请求均返回 `409` 和首次错误；下一次提交会以 `500` 返回此错误，并移除失败的进程池及其暂存文件；`PURGE … OPTION clear_sessions=1` 也可以移除它们。只有清理完成后，才能开始新的加载。

请求失败和整个加载失败返回的响应相同，均为 `failed` 和 `shared_failed`（状态码为 `400`，请求被终止时为 `500`），因此仅凭响应无法区分。下一次请求可以帮助判断：`staged` 表示进程池仍然开放，而 `409` 和 `shared_failed` 表示整个加载已失败。

进程池仅存在于 `searchd` 的内存中。如果守护进程在加载期间停止或崩溃，或进程池被取消，暂存的文档就会丢失；下一次数据请求会创建新进程池，其中只包含之后的文档。要检测这种情况，请在收到第一次 `staged` 响应后，为后续每个数据请求添加 `shared_join`：这类请求只会加入已有进程池，绝不会创建新池，因此没有进程池时会返回 `409` 和 `shared_no_pool`，此时可以从头重新加载。

客户端退出不会结束进程池。如果客户端在提交前退出，例如日志采集器停止或崩溃，进程池仍会保持开放并继续占用表，导致常规写入持续失败。可以通过两种方式解除占用：按上文所示提交进程池，发布已暂存的文档；或者取消进程池，丢弃这些文档：

```sql
PURGE BULK_IMPORT FROM TABLE products OPTION clear_sessions=1;
```

取消操作会停止索引构建进程、解除对表的占用并删除暂存文件；先前已发布的磁盘块仍保留在表中。如果已退出客户端的数据请求仍被视为正在运行，提交会返回 `409` 和 `shared_busy`，直到该请求超时；取消操作无需等待。表的占用解除后，`SHOW LOCKS` 将不再列出该表，此时即可开始新的共享加载。详情请参阅 [`PURGE BULK_IMPORT`](../../Node_info_and_management/PURGE.md#PURGE-BULK_IMPORT)。

#### Elasticsearch `/_bulk`

兼容 Elasticsearch 的 `/_bulk` 端点不支持直接写入磁盘的 `bulk_import`；请使用 SQL 或 Manticore `/bulk`。

#### 重复的文档 ID

在同一个直接写入磁盘的批次内，对于数值文档 ID，首次出现的行保持可见，后续相同 ID 的行会被逻辑删除。对于 SQL，一个批次包含 `COMMIT` 或 `ROLLBACK` 之前暂存的所有行。对于 Manticore `/bulk`，在空行、切换表或请求 EOF 处发布的每组数据各自构成一个批次。在共享加载中，每个索引构建进程的数据分别构成一个批次，其磁盘块会在提交进程池时依次挂载。如果某个磁盘块挂载失败，此前已挂载的磁盘块仍保留在表中，提交返回 `500`；对于显式指定 ID 的文档，重新加载相同文档是安全的，因为发布的行会替换同 ID 的任何现有行，而自动生成 ID 的文档则会被再次添加。

Manticore 将磁盘块发布到目标表时，块中的行会替换表中具有相同 ID 的任何现有行。这也适用于先前 HTTP 批次发布的 ID。因此，重试已发布的批次会替换其中的行，而重试批次内部的重复 ID 仍只保留第一行。在 Manticore `/bulk` 请求中，`create` 操作，例如：

```json
{"create":{"table":"products",...}}
```

行为与 `insert` 相同，不会仅因为 ID 已存在而失败。

#### 内存和磁盘用量

直接写入磁盘的加载不会将文档集中存放在 `searchd` 的内存中。每条 `INSERT` 语句或 `/bulk` 请求体的每个部分（请求带有 `shared` 参数时约为 32 KiB）都会立即转换为 CSV，并通过管道传给 `indexer` 进程。`indexer` 以与普通表相同的方式构建磁盘块：先在内存中收集数据，达到 [mem_limit](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#mem_limit)（固定为默认值 128 MB）后写入临时文件，并在处理完最后一行后将临时文件合并为磁盘块。因此，加载所需的内存不会随数据量增长：每个 `indexer` 进程的内存用量为 `mem_limit` 加上[写入缓冲区](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#write_buffer)和进程自身开销，总计约 200 MB。一个 `SET bulk_import` 会话或一个 `/bulk` 请求运行一个 `indexer`，带 `shared=N` 的加载运行 N 个；还需加上正在处理的语句或请求部分所占的内存。

与普通表一样（参阅 [inplace_enable](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#inplace_enable)），临时文件中的中间数据与最终磁盘块的大小相近。两者都会写入表文件旁的暂存目录，因此在峰值时，表所在磁盘需要约为新磁盘块大小两倍的可用空间。发布磁盘块时，通过重命名将文件移入表中，无需复制。参阅[暂存文件与清理](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Staging-files-and-cleanup)。

#### 暂存文件与清理

正常执行 `COMMIT`、`ROLLBACK`、关闭模式或关闭会话后，Manticore 会删除当前暂存目录。共享进程池会保留暂存目录，直到提交或取消；关闭连接不会删除它们。守护进程或主机崩溃可能留下废弃的暂存目录。后续直接写入磁盘的加载会创建名称唯一的新目录，不会复用或挂载崩溃加载留下的文件。

使用以下查询查看某个表直接写入磁盘时的所有暂存条目：

```sql
SELECT file, normalized, size
FROM products.@files
OPTION format='bulk_import';
```

结果会递归列出该表直接写入磁盘暂存根目录下的文件和目录。目录及其他非常规文件条目的大小报告为 `0`。

确认该表没有正在进行的直接写入磁盘加载后，使用 [`PURGE BULK_IMPORT`](../../Node_info_and_management/PURGE.md#PURGE-BULK_IMPORT) 删除其整个暂存根目录：

```sql
PURGE BULK_IMPORT FROM TABLE products;
```

当表存在开放的共享进程池时，`PURGE` 会被拒绝；`PURGE BULK_IMPORT FROM TABLE products OPTION clear_sessions=1` 会先取消进程池、停止索引构建进程并解除对表的占用，然后删除暂存根目录。进程池正在挂载磁盘块时，`PURGE` 也会被拒绝；请在提交完成后重试。在 Windows 上，如果数据请求仍在向进程池写入，执行 `PURGE … OPTION clear_sessions=1` 或 `DROP TABLE` 可能返回 `failed to remove bulk staging root`：此时进程池已取消，但已停止的索引构建进程还会短暂保持文件打开；请重试该语句。

`PURGE` 要求目标是已存在且不属于复制集群的本地实时表。它只删除直接写入磁盘的暂存状态，不会更改表结构或已建立索引的行。如果暂存根目录不存在，操作会成功，但不执行任何更改。在无配置模式下，`DROP TABLE` 也会删除该表的直接写入磁盘暂存根目录，并取消其共享进程池；守护进程关闭时会取消所有共享进程池。先前已发布的磁盘块仍保留在表中。

#### 当前限制

* 目标必须是一张已存在、未冻结且不属于复制集群的本地实时表。不支持分布式表、分片表、复制表、percolate 表和普通表。
* SQL 仅支持 `INSERT`。Manticore `/bulk` 接受 `insert`、`create` 和直接提供的文档；`index`、`replace`、`update` 和 `delete` 均会被拒绝。
* 文档 ID 的转换方式与常规 `INSERT` 相同，包括遵循 [attr_autoconv_strict](../../Server_settings/Searchd.md#attr_autoconv_strict)。没有 ID 或 ID 转换后为 0 的行会获得[自动生成的 ID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID)；响应不会返回生成的 ID，重复加载会再次添加这些行。不支持 UUID 文档 ID。
* 不支持静态构建版本。
* 平台对应的可执行文件（Linux 上为 `indexer`，Windows 上为 `indexer.exe`）必须与正在运行的 `searchd` 可执行文件位于同一目录。Manticore Search 只会查找该同级路径，不会搜索 `PATH`。请使用与 `searchd` 来自同一次安装的可执行文件，以确保兼容。如果文件缺失、不可读或无法启动，只会导致批量导入失败；正常启动和常规插入仍可使用。

辅助加载器使用 `csvpipe` 数据源。其命令在 Linux 上默认为 `/bin/cat`，在 Windows 上默认为 `-`，其中 `-` 表示从索引构建进程的标准输入读取。可在 `searchd` 环境中设置 `INDEXER_RT_BULK_CSV_PIPE_COMMAND`，仅覆盖此命令。覆盖设置不会选择其他数据源或输入方式；该命令必须兼容当前平台的输入方式，并将 CSV 行传给索引构建进程。不受支持的命令会导致辅助加载失败。

<!-- intro -->
### 批量插入示例
##### SQL:
<!-- request SQL -->
批量插入时，只需在 `VALUES()` 后用括号提供更多文档。语法如下：

```sql
INSERT INTO <table name>[(column1, column2, ...)] VALUES(value1[, value2 , ...]), (...)
```

可选的列名列表允许您为表中的部分列显式指定值。其他列均填入默认值（标量类型为 0，字符串类型为空字符串）。

例如：

```sql
INSERT INTO products(title,price) VALUES ('Crossbody Bag with Tassel', 19.85), ('microfiber sheet set', 19.99), ('Pet Hair Remover Glove', 7.99);
```
<!-- response SQL -->

```sql
Query OK, 3 rows affected (0.01 sec)
```

`INSERT` 目前不支持表达式，必须显式指定值。

<!-- intro -->
##### JSON:
<!-- request JSON -->
语法与[插入单个文档](../../Quick_start_guide.md#Add-documents)基本相同。只需增加行数，每行一个文档，并将 `/insert` 端点改为 `/bulk`。将每个文档放在 "insert" 节点中。另外还需满足以下要求：
* `Content-Type: application/x-ndjson`
* 数据应采用换行分隔 JSON（NDJSON）格式。也就是说，每行应恰好包含一条 JSON 语句，并以换行符 `\n` 结尾，也可以包含 `\r`。

`/bulk` 端点支持 'insert'、'replace'、'delete' 和 'update' 查询。请注意，操作可以指向多个表，但一个事务只能涉及一张表。如果指定了多个表，Manticore 会将指向同一表的操作归入一个事务。切换表时，会提交已收集的操作并启动新事务。批次之间的空行也会触发提交上一批次并启动新事务。不包含操作的请求，无论请求体为空，还是仅包含空行或纯空白行，都会成功且不执行任何操作，并返回空的 `items` 数组。

`/bulk` 请求的响应包含以下字段：
* "errors"：表示是否发生错误（true/false）
* "error"：描述发生的错误
* "current_line"：执行停止（或失败）处的行号；空行也计数，包括第一个空行
* "skipped_lines"：从 `current_line` 开始向前回溯统计的未提交行数

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

> 注意：如果表尚不存在，`_bulk` 需要 [Manticore Buddy](Installation/Manticore_Buddy.md)。如果无法使用，请确认已安装 Buddy。

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

对于复制表，请使用 `cluster_name:table_name` 格式，在表名中包含集群名称：

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
使用 addDocuments() 方法：

```php
$index->addDocuments([
        ['id' => 1, 'title' => 'Crossbody Bag with Tassel', 'price' => 19.85],
        ['id' => 2, 'title' => 'microfiber sheet set', 'price' => 19.99],
        ['id' => 3, 'title' => 'Pet Hair Remover Glove', 'price' => 7.99]
]);
```

对于复制表，请在添加文档前设置集群名称：

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

对于复制表，请在每个文档中包含 cluster 属性：

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

对于复制表，请在每个文档中包含 cluster 属性：

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

对于复制表，请在每个文档中包含 cluster 属性：

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

对于复制表，请在每个文档中包含 cluster 属性：

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

对于复制表，请在每个文档中包含 cluster 属性：

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

多值属性（MVA）以数值数组的形式插入。
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

也可以使用以下方式
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
JSON 值可以作为[转义](../../Searching/Full_text_matching/Escaping.md)后的字符串插入（通过 SQL 或 JSON），也可以作为 JSON 对象插入（通过 JSON 接口）。

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

也可以使用以下方式
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
将 JSON 作为字符串处理：
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
