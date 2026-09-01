# 将文档添加到实时表

> 如果您正在寻找有关将文档添加到普通表的信息，请参阅[从外部存储添加数据](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)部分。

<!-- example insert -->
实时添加文档仅支持[实时](../../Creating_a_table/Local_tables/Real-time_table.md)和[Percolate](../../Creating_a_table/Local_tables/Percolate_table.md)表。相应的SQL命令、HTTP端点或客户端函数将新行（文档）插入到表中，提供字段值。在添加文档之前，表不一定存在。如果表不存在，Manticore 将尝试自动创建它。有关更多信息，请参阅[自动模式](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)。

您可以插入单个或[多个文档](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-adding-documents)，这些文档包含表的所有字段值或仅部分字段值。在这种情况下，其他字段将使用其默认值填充（标量类型为0，文本类型为空字符串）。

`INSERT` 中目前不支持表达式，因此必须显式指定值。

ID 字段/值可以省略，因为 RT 和 PQ 表支持 [auto-id](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) 功能。对于 numeric-ID 表，你也可以将 `0` 作为 id 值，以强制自动生成 ID。具有重复 ID 的行不会被 `INSERT` 覆盖。要实现这一目的，请改用 [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md)。

对于使用 [`id uuid`](../../Creating_a_table/Data_types.md#UUID-document-IDs) 创建的表，请传入一个带引号的显式 UUID 字符串，或者省略 `id` 以自动生成一个。显式值必须匹配 `xxxxxxxx-xxxx-Vxxx-Wxxx-xxxxxxxxxxxx`，其中每个 `x` 都是十六进制数字，`V` 是版本（`1` 到 `8`），`W` 是变体（`8`、`9`、`a` 或 `b`）。系统接受大写十六进制字母，并会将其规范化为小写。不同于 numeric ID，`0` 不会触发自动生成 UUID。

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

对于使用 `id uuid` 创建的表，请将 JSON `id` 作为 UUID 字符串传入，或者省略它以自动生成一个：

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
        ['id' => 1, 'title' => 'Crossbody Bag with Tassel', 'price' => 19.85],
        ['id' => 2, 'title' => 'microfiber sheet set', 'price' => 19.99],
        ['id' => 3, 'title' => 'Pet Hair Remover Glove', 'price' => 7.99]
]);
```

对于复制表，在添加文档之前先设置集群名称：

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
##### Python：

<!-- request Python -->

```python
docs = [ \
    {"insert": {"table" : "products", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}}, \
    {"insert": {"table" : "products", "id" : 2, "doc" : {"title" : "microfiber sheet set", "price" : 19.99}}}, \
    {"insert": {"table" : "products", "id" : 3, "doc" : {"title" : "Pet Hair Remover Glove", "price" : 7.99}}}
]
res = indexApi.bulk('\n'.join(map(json.dumps,docs)))
```

对于复制表，在每个文档中包含 `cluster` 属性：

```python
docs = [ \
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}}, \
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 2, "doc" : {"title" : "microfiber sheet set", "price" : 19.99}}}, \
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 3, "doc" : {"title" : "Pet Hair Remover Glove", "price" : 7.99}}}
]
res = indexApi.bulk('\n'.join(map(json.dumps,docs)))
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
docs = [ \
    {"insert": {"table" : "products", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}}, \
    {"insert": {"table" : "products", "id" : 2, "doc" : {"title" : "microfiber sheet set", "price" : 19.99}}}, \
    {"insert": {"table" : "products", "id" : 3, "doc" : {"title" : "Pet Hair Remover Glove", "price" : 7.99}}}
]
res = await indexApi.bulk('\n'.join(map(json.dumps,docs)))
```

对于复制表，在每个文档中包含 `cluster` 属性：

```python
docs = [ \
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}}, \
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 2, "doc" : {"title" : "microfiber sheet set", "price" : 19.99}}}, \
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 3, "doc" : {"title" : "Pet Hair Remover Glove", "price" : 7.99}}}
]
res = await indexApi.bulk('\n'.join(map(json.dumps,docs)))
```

<!-- intro -->
##### Javascript：

<!-- request Javascript -->

```javascript
let docs = [
    {"insert": {"table" : "products", "id" : 3, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}},
    {"insert": {"table" : "products", "id" : 4, "doc" : {"title" : "microfiber sheet set", "price" : 19.99}}},
    {"insert": {"table" : "products", "id" : 5, "doc" : {"title" : "Pet Hair Remover Glove", "price" : 7.99}}}
];
res =  await indexApi.bulk(docs.map(e=>JSON.stringify(e)).join('\n'));
```

对于复制表，在每个文档中包含 `cluster` 属性：

```javascript
let docs = [
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 3, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}},
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 4, "doc" : {"title" : "microfiber sheet set", "price" : 19.99}}},
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 5, "doc" : {"title" : "Pet Hair Remover Glove", "price" : 7.99}}}
];
res =  await indexApi.bulk(docs.map(e=>JSON.stringify(e)).join('\n'));
```

<!-- intro -->
##### java：

<!-- request Java -->

``` java
String body = "{\"insert\": {\"index\" : \"products\", \"id\" : 1, \"doc\" : {\"title\" : \"Crossbody Bag with Tassel\", \"price\" : 19.85}}}"+"\n"+
    "{\"insert\": {\"index\" : \"products\", \"id\" : 4, \"doc\" : {\"title\" : \"microfiber sheet set\", \"price\" : 19.99}}}"+"\n"+
    "{\"insert\": {\"index\" : \"products\", \"id\" : 5, \"doc\" : {\"title\" : \"Pet Hair Remover Glove\", \"price\" : 7.99}}}"+"\n";
BulkResponse bulkresult = indexApi.bulk(body);
```

对于复制表，在每个文档中包含 `cluster` 属性：

``` java
String body = "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 1, \"doc\" : {\"title\" : \"Crossbody Bag with Tassel\", \"price\" : 19.85}}}"+"\n"+
    "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 4, \"doc\" : {\"title\" : \"microfiber sheet set\", \"price\" : 19.99}}}"+"\n"+
    "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 5, \"doc\" : {\"title\" : \"Pet Hair Remover Glove\", \"price\" : 7.99}}}"+"\n";
BulkResponse bulkresult = indexApi.bulk(body);
```

<!-- intro -->
##### C#：

<!-- request C# -->

``` clike
string body = "{\"insert\": {\"index\" : \"products\", \"id\" : 1, \"doc\" : {\"title\" : \"Crossbody Bag with Tassel\", \"price\" : 19.85}}}"+"\n"+
    "{\"insert\": {\"index\" : \"products\", \"id\" : 4, \"doc\" : {\"title\" : \"microfiber sheet set\", \"price\" : 19.99}}}"+"\n"+
    "{\"insert\": {\"index\" : \"products\", \"id\" : 5, \"doc\" : {\"title\" : \"Pet Hair Remover Glove\", \"price\" : 7.99}}}"+"\n";
BulkResponse bulkresult = indexApi.Bulk(string.Join("\n", docs));
```

对于复制表，在每个文档中包含 `cluster` 属性：

``` clike
string body = "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 1, \"doc\" : {\"title\" : \"Crossbody Bag with Tassel\", \"price\" : 19.85}}}"+"\n"+
    "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 4, \"doc\" : {\"title\" : \"microfiber sheet set\", \"price\" : 19.99}}}"+"\n"+
    "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 5, \"doc\" : {\"title\" : \"Pet Hair Remover Glove\", \"price\" : 7.99}}}"+"\n";
BulkResponse bulkresult = indexApi.Bulk(string.Join("\n", docs));
```

<!-- end -->

## 向复制表添加文档
<!-- example replicated_insert -->
在使用[复制表](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md)时，必须使用特殊语法，以确保写入操作能够正确传播到集群中的所有节点。

对复制表执行所有写入操作（INSERT、REPLACE、DELETE、TRUNCATE、UPDATE）时，必须：
* 在 SQL 中：使用 `cluster_name:table_name` 格式，而不是只写表名
* 在 JSON 中：将 `cluster` 属性与 `table` 属性一起包含

如果不使用正确的语法，操作将失败并报错。

<!-- intro -->
##### SQL：
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
##### JSON：

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
##### PHP：

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
##### Python：

<!-- request Python -->

```python
indexApi.insert({"cluster":"posts", "table":"weekly_table", "id":1, "doc":{"title":"Crossbody Bag with Tassel", "price":19.85}})
indexApi.insert({"cluster":"posts", "table":"weekly_table", "id":0, "doc":{"title":"Yellow bag", "price":4.95}})
```

<!-- intro -->
##### Javascript：

<!-- request Javascript -->

```javascript
res = await indexApi.insert({"cluster":"posts", "table":"weekly_table", "id":1, "doc":{"title":"Crossbody Bag with Tassel", "price":19.85}});
res = await indexApi.insert({"cluster":"posts", "table":"weekly_table", "id":0, "doc":{"title":"Yellow bag", "price":4.95}});
```

<!-- intro -->
##### Java：

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
##### C#：

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
##### Rust：

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


请记住，`/bulk` HTTP 端点不支持自动创建表（auto schema）。只有 `/_bulk`（类似 Elasticsearch）端点和 SQL 接口支持此功能。

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
{"table":"t","id":2,"created":true,"result":"created","status":201}
```

<!-- end -->

## 自动ID
<!-- example autoid -->
Manticore 会为插入到实时表或 [Percolate table](../../Creating_a_table/Local_tables/Percolate_table.md) 中、或写回其中的文档提供自动 ID 生成。生成器会产生一个具有以下保证的唯一数值，但它不应被视为自增序列。

生成的 ID 值在以下条件下保证唯一：
* 当前服务器的 [server_id](../../Server_settings/Searchd.md#server_id) 值在 0 到 127 范围内，并且在集群节点中唯一，或者它使用从 MAC 地址生成的默认值作为种子
* 系统时间在 Manticore 节点服务器重启间不发生变化
* 两次服务器启动之间的自动生成ID速率平均保持在每秒约1600万ID以下

自动 ID 生成器会创建一个 64 位整数，其布局如下：
* 第0到23位是一个计数器，每次调用自动生成器时递增
* 第24到55位存储服务器启动时间（以秒为单位），编码为`(unix_timestamp_at_start - 2019-05-01 00:00:00 UTC)`
* 第56到62位存储`server_id`（该值被限制在0..127范围内）

这种布局可确保在集群节点之间生成的 ID 是唯一的，并且插入到不同集群节点的数据不会发生冲突。这在使用复制表时尤其重要，因为它能保证自动生成的 ID 在复制集群的所有节点上都是唯一的。

重要的是：24位计数器并不是服务器单次运行期间可以插入文档总数的硬性限制。在启动后，您可以插入超过16,777,216个文档，ID仍会继续增加并在该运行过程中保持唯一。`~16 million IDs per second`规则对于跨重启的唯一性很重要：重启后，基于时间的部分必须足够前进，以确保新生成的ID不会与重启前生成的ID重叠。

因此，自动 ID 生成器生成的第一个 ID 不是 1，而是一个较大的数字。此外，插入表中的文档流可能具有非连续的 ID 值，如果在调用之间向其他表插入数据，因为 ID 生成器在服务器中是唯一的并且在其所有表之间共享。

对于 numeric-ID 表，这个整数就是对外可见的文档 ID。对于 UUID-ID 表，Manticore 会将其编码为标准的 UUIDv8 字符串；客户端只能看到 UUID。

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

## 批量添加文档
你不仅可以向实时表插入单个文档，还可以插入任意数量的文档。向实时表一次插入数万个文档是完全可以的。然而，需要注意以下几点：
* 批量越大，每次插入操作的延迟越高
* 批量越大，索引速度越快
* 你可能需要增加 [max_packet_size](../../Server_settings/Searchd.md#max_packet_size) 的值以允许更大的批量
* 通常，每次批量插入操作被视为具有原子性保证的单个 [事务](../../Data_creation_and_modification/Transactions.md)，因此你要么一次性将所有新文档放入表中，要么在失败情况下一个都不添加。关于空行或切换到另一张表的更多细节见“JSON”示例。

请注意，`/bulk` HTTP 端点不支持自动创建表（auto schema）。只有 `/_bulk`（类似 Elasticsearch）端点和 SQL 接口支持此功能。`/_bulk`（类似 Elasticsearch）HTTP 端点允许在表名中使用 `cluster_name:table_name` 格式包含集群名称。

`/_bulk` 端点接受与 Elasticsearch 相同格式的文档 ID，你也可以在文档内部包含 `id`：
```json
{ "index": { "table": "products", "_id": "1" } }
{ "title": "Crossbody Bag with Tassel", "price": 19.85 }
```

或者

```json
{ "index": { "table": "products" } }
{ "title": "Crossbody Bag with Tassel", "price": 19.85, "id": "1" }
```

对于声明为 `id uuid` 的 RT 表，`/bulk` 从 `id` 读取 UUID。`/_bulk` 从元数据 `_id` 或文档 `id` 中读取 UUID。两个端点都可以省略 ID，以便自动生成 UUID。

#### /bulk 的分块传输
`/bulk`（Manticore 模式）端点支持[分块传输编码](https://en.wikipedia.org/wiki/Chunked_transfer_encoding)。你可以用它来传输大批量数据。它能够：
* 降低峰值内存使用，减少内存溢出风险
* 缩短响应时间
* 允许绕过 [max_packet_size](../../Server_settings/Searchd.md#max_packet_size) 限制，传输远大于最大允许值（128MB）的批量，例如一次 1GB。

### 批量导入

对于向本地实时表进行的大批量加载，Manticore Search 可以将 `INSERT` 行直接写入磁盘块，并在事务提交时发布该块。这样无需先在 RAM 块中构建批次。行在块发布之前保持不可见，失败的操作或 `ROLLBACK` 会让表保持不变。

当你要加载一个大批次，并且直接生成磁盘块比先构建 RAM 块更合适时，使用此模式。它同时支持行式表和列式表，包括全文字段、数值属性、字符串、JSON、MVA/MVA64，以及带 KNN 索引的浮点向量。

此模式下的表名使用规范的小写形式。SQL 设置和 Manticore `/bulk` 查询参数在授权和激活前都会把目标规范化为小写。

#### SQL

为当前 SQL 会话启用该模式，针对绑定的表执行一个或多个 `INSERT` 语句，然后提交：

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

`SET bulk_import=<table>` 要求会话是干净的，并为该表获取写入保留。搜索仍可用，但其他会话不能在该受保留的表上运行 `INSERT`、`REPLACE`、`UPDATE` 或 `DELETE`。`BEGIN` 和 `START TRANSACTION` 在直接写入磁盘模式下是可选的兼容性空操作。`COMMIT` 会构建当前磁盘块并将其原子性地附加；`ROLLBACK` 会丢弃当前批次。两者都会让模式和保留状态继续保持，以便处理下一批。此模式激活期间不能更改自动提交。`SET bulk_import=0` 或关闭连接会丢弃任何待处理批次并释放保留。

#### HTTP `/bulk`

在 Manticore `/bulk` 或 `/json/bulk` 请求中添加 `bulk_import=<table>`。请求体仍然使用标准的换行分隔 JSON（NDJSON），且每个操作都必须是绑定表的 `insert` 或 `create`：

```bash
curl --http1.1 \
  -H 'Content-Type: application/x-ndjson' \
  --data-binary $'{"insert":{"table":"products","id":101,"doc":{"title":"Crossbody Bag with Tassel","price":19.85}}}\n{"insert":{"table":"products","id":102,"doc":{"title":"Microfiber Sheet Set","price":19.99}}}\n' \
  'http://localhost:9308/bulk?bulk_import=products' \
  --next --http1.1 \
  -H 'Content-Type: application/x-ndjson' \
  --data-binary $'{"create":{"table":"products","id":103,"doc":{"title":"Pet Hair Remover Glove","price":7.99}}}\n' \
  'http://localhost:9308/bulk' \
  --next --http1.1 \
  -H 'Content-Type: application/x-ndjson' \
  --data-binary '' \
  'http://localhost:9308/bulk?bulk_import=0'
```

激活请求不需要保持连接打开。一次性请求会在回复前发布其已完成的分组，并在连接关闭时释放该模式和保留。只有后续请求需要继承当前目标，或者需要显式发送 `bulk_import=0` 时，才保持连接持久。HTTP/1.1 默认是持久连接，而 HTTP/1.0 需要 `Connection: keep-alive`。

成功的数据请求会在回复前发布其所有已完成的分组。空行会发布前一个分组，而请求 EOF 会发布最后一个分组。未发布的批次不会跨越响应边界；在持久连接上，只有目标及其保留可以保持激活。后续在该连接上的无参数 `/bulk` 或 `/json/bulk` 请求会继承该目标。一旦激活，会话目标就是权威的；其他带目标值的查询参数不会切换它，而普通 bulk/DML 路径仍会拒绝针对另一张表的操作。激活状态下的 `bulk_import=0` 请求会沿用同样的直接写入磁盘处理，然后禁用该模式并释放其保留。它可以携带最后的数据分组；空内容或仅包含空白字符的正文是成功的空操作，只会禁用该模式。

示例使用一个带 `--next` 的 `curl` 进程，因此这三次传输可以复用同一个连接。不同的 `curl` 进程会创建不同会话，彼此不能继承或关闭对方的直接写入磁盘模式。关于通用连接规则，请参见 [Persistent connections and HTTP state](../../Connecting_to_the_server/HTTP.md#Persistent-connections)。关闭连接也会释放保留。已经处于激活状态的最后一个数据请求可以使用 `Connection: close`；Manticore 会在拆除前先发布该请求。

此模式的响应使用普通的 Manticore bulk 信封，并为每个已发布分组包含一个汇总的 `bulk` 项。该端点支持分块传输编码，因此可以处理大于 `max_packet_size` 的正文，而无需缓存整个请求。失败的请求不会发布当前分组，并会关闭该 HTTP 会话；在更早的普通 bulk 边界已经发布的分组，例如空行或表切换，仍然保持可见。

与 Elasticsearch 兼容的 `/_bulk` 不支持直接写入磁盘模式。干净的 `/_bulk` 会话中的 `bulk_import` 查询值不会激活该功能，而且当该连接上已经处于此模式时，`/_bulk` 会被拒绝。Fluent Bit 的 Elasticsearch 输出使用的是 `/_bulk`，因此它仍然会使用常规实时插入。

#### 重复文档 ID

对于一个直接写入磁盘的批次，数值型文档 ID 的第一行会保持可见，而后续具有相同 ID 的行会被逻辑移除。对于 SQL，一个批次由 `COMMIT` 或 `ROLLBACK` 之前暂存的行组成。对于 Manticore `/bulk`，在空行、表切换或请求 EOF 时发布的每个分组都是一个单独的批次。

如果目标表中已经可见的 ID 在后续的直接写入磁盘块中再次出现，那么在该块附加时会用它替换原有内容。这同样适用于先前 HTTP 分组已经发布的 ID。因此，重试一个先前已发布的批次会替换其中的行，而重试批次内部的重复项仍然会保留第一行。`create` 操作的行为与 `insert` 相同；仅仅因为目标表中已经存在该 ID，它不会失败。

#### 暂存文件与清理

在正常的 `COMMIT`、`ROLLBACK`、禁用模式或会话关闭后，Manticore 会移除当前的暂存目录。守护进程或主机崩溃可能会留下一个被遗弃的暂存目录。之后的直接写入磁盘加载会创建一个新的唯一命名目录，不会重用或附加崩溃加载遗留的文件。

使用以下查询查看某个表的所有直接写入磁盘暂存条目：

```sql
SELECT file, normalized, size
FROM products.@files
OPTION format='bulk_import';
```

结果会递归列出该表直接写入磁盘暂存根目录下的文件和目录。目录及其他非普通条目的报告大小为 `0`。

在确认该表没有活动中的直接写入磁盘加载后，使用 [`PURGE BULK_IMPORT`](../../Node_info_and_management/PURGE.md#PURGE-BULK_IMPORT) 删除其整个暂存根目录：

```sql
PURGE BULK_IMPORT FROM TABLE products;
```

`PURGE` 要求目标是一个已存在的本地实时表，且它不属于复制集群。它只会移除直接写入磁盘的暂存状态，不会改变表结构或已索引的行。如果暂存根目录不存在，它会作为空操作成功返回。在无配置模式下，`DROP TABLE` 也会移除该表的直接写入磁盘暂存根目录。

#### 当前限制

* 目标必须是一个已存在、未冻结的本地实时表，并且不能是复制集群成员。分布式表、分片表、复制表、percolate 表和普通表都不受支持。
* SQL 只支持 `INSERT`。Manticore `/bulk` 接受 `insert` 和 `create`；`index`、`replace`、`update` 和 `delete` 会被拒绝。
* 每一行都必须提供显式的数值型、非零文档 ID。不支持自动生成的和 UUID 文档 ID。
* 当前不支持静态构建和 macOS。

<!-- example bulk_insert -->
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

`/bulk` 端点支持 `'insert'`、`'replace'`、`'delete'` 和 `'update'` 查询。请注意，你可以把操作定向到多张表，但事务只能针对单张表。如果你指定了多张，Manticore 会把指向同一张表的操作收集到一个事务中。当表发生变化时，它会提交已收集的操作并启动新的事务。分隔批次的空行也会导致提交前一个批次并启动新的事务。没有任何操作的请求，无论正文是空的还是只包含空行或仅空白字符行，都会作为成功的空操作返回一个空的 `items` 数组。

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

对于复制表，在每个操作中包含 `cluster` 属性：

```json
POST /bulk
-H "Content-Type: application/x-ndjson" -d '
{"insert": {"cluster":"posts", "table":"weekly_table", "id":1, "doc":  {"title":"Crossbody Bag with Tassel","price":19.85}}}
{"insert": {"cluster":"posts", "table":"weekly_table", "id":2, "doc":  {"title":"microfiber sheet set","price":19.99}}}
{"insert": {"cluster":"posts", "table":"weekly_table", "id":3, "doc":  {"title":"Pet Hair Remover Glove","price":7.99}}}
'
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

对于复制表，使用 `cluster_name:table_name` 格式在表名中包含集群名称：

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

对于复制表，在添加文档之前先设置集群名称：

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

对于复制表，在每个文档中包含 `cluster` 属性：

```python
docs = [ \
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}}, \
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 2, "doc" : {"title" : "microfiber sheet set", "price" : 19.99}}}, \
    {"insert": {"cluster" : "posts", "table" : "weekly_table", "id" : 3, "doc" : {"title" : "Pet Hair Remover Glove", "price" : 7.99}}}
]
res = indexApi.bulk('\n'.join(map(json.dumps,docs)))
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
docs = [ \
    {"insert": {"table" : "products", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}}, \
    {"insert": {"table" : "products", "id" : 2, "doc" : {"title" : "microfiber sheet set", "price" : 19.99}}}, \
    {"insert": {"table" : "products", "id" : 3, "doc" : {"title" : "Pet Hair Remover Glove", "price" : 7.99}}}
]
res = await indexApi.bulk('\n'.join(map(json.dumps,docs)))
```

对于复制表，在每个文档中包含 `cluster` 属性：

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

对于复制表，在每个文档中包含 `cluster` 属性：

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

对于复制表，在每个文档中包含 `cluster` 属性：

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

对于复制表，在每个文档中包含 `cluster` 属性：

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
