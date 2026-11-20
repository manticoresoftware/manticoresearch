# Adding documents to a real-time table

> If you're looking for information on adding documents to a plain table, please refer to the section on [adding data from external storages](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md).

<!-- example insert -->
Adding documents in real-time is supported only for [Real-Time](../../Creating_a_table/Local_tables/Real-time_table.md) and [percolate](../../Creating_a_table/Local_tables/Percolate_table.md) tables. The corresponding SQL command, HTTP endpoint, or client functions insert new rows (documents) into a table with the provided field values. It's not necessary for a table to exist before adding documents to it. If the table doesn't exist, Manticore will attempt to create it automatically. For more information, see [Auto schema](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema).

You can insert a single or [multiple documents](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-adding-documents) with values for all fields of the table or just a portion of them. In this case, the other fields will be filled with their default values (0 for scalar types, an empty string for text types).

Expressions are not currently supported in `INSERT`, so values must be explicitly specified.

The ID field/value can be omitted, as RT and PQ tables support [auto-id](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) functionality. For numeric-ID tables, you can also use `0` as the id value to force automatic ID generation. Rows with duplicate IDs will not be overwritten by `INSERT`. Instead, you can use [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md) for that purpose.

For tables created with [`id uuid`](../../Creating_a_table/Data_types.md#UUID-document-IDs), pass an explicit UUID as a quoted string or omit `id` to generate one automatically. Explicit values must match `xxxxxxxx-xxxx-Vxxx-Wxxx-xxxxxxxxxxxx`, where each `x` is a hexadecimal digit, `V` is the version (`1` through `8`), and `W` is the variant (`8`, `9`, `a`, or `b`). Uppercase hexadecimal letters are accepted and normalized to lowercase. Unlike numeric IDs, `0` does not request automatic UUID generation.

When using the HTTP JSON protocol, you have two different request formats to choose from: a common Manticore format and an Elasticsearch-like format. Both formats are demonstrated in the examples below.

Additionally, when using the Manticore JSON request format, keep in mind that the `doc` node is required, and all the values should be provided within it.


<!-- intro -->
##### SQL:
<!-- request SQL -->
General syntax:

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

For tables created with `id uuid`, pass the JSON `id` as a UUID string, or omit it to generate one automatically:

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

> NOTE: `_create` requires [Manticore Buddy](Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

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

For replicated tables, set the cluster name before adding documents:

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

For replicated tables, include the cluster property in each document:

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

For replicated tables, include the cluster property in each document:

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

For replicated tables, include the cluster property in each document:

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

For replicated tables, include the cluster property in each document:

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

For replicated tables, include the cluster property in each document:

``` clike
string body = "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 1, \"doc\" : {\"title\" : \"Crossbody Bag with Tassel\", \"price\" : 19.85}}}"+"\n"+
    "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 4, \"doc\" : {\"title\" : \"microfiber sheet set\", \"price\" : 19.99}}}"+"\n"+
    "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 5, \"doc\" : {\"title\" : \"Pet Hair Remover Glove\", \"price\" : 7.99}}}"+"\n";
BulkResponse bulkresult = indexApi.Bulk(string.Join("\n", docs));
```

<!-- end -->

## Adding documents to replicated tables
<!-- example replicated_insert -->
When working with [replicated tables](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md), you must use a special syntax to ensure that write operations are properly propagated to all nodes in the cluster.

For all write operations (INSERT, REPLACE, DELETE, TRUNCATE, UPDATE) on replicated tables, you must:
* In SQL: Use the `cluster_name:table_name` format instead of just the table name
* In JSON: Include the `cluster` property along with the `table` property

If you don't use the correct syntax, the operation will fail with an error.

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

## Auto schema

> NOTE: Auto schema requires [Manticore Buddy](Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

Manticore features an automatic table creation mechanism, which activates when a specified table in the insert or replace query doesn't yet exist. This mechanism is enabled by default. To disable it, set `auto_schema = 0` in the [Searchd](../../Server_settings/Searchd.md#auto_schema) section of your Manticore config file.

<!-- example auto-schema -->

By default, all text values in the `VALUES` clause are considered to be of the `text` type, except for values representing valid email addresses, which are treated as the `string` type.

If you attempt to INSERT/REPLACE multiple rows with different, incompatible value types for the same field, auto table creation will be canceled, and an error message will be returned. However, if the different value types are compatible, the resulting field type will be the one that accommodates all the values. Some automatic data type conversions that may occur include:
* mva -> mva64
* uint -> bigint -> float (this may cause some precision loss)
* string -> text

The auto schema mechanism does not support creating tables with vector fields (fields of type `float_vector`) used for [KNN](../../Searching/KNN.md#Configuring-a-table-for-KNN-search) (K-Nearest Neighbors) similarity search. To use vector fields in your table, you must explicitly create the table with a schema that defines these fields. If you need to store vector data in a regular table without KNN search capability, you can store it as a JSON array using the standard JSON syntax, for example: `INSERT INTO table_name (vector_field) VALUES ('[1.0, 2.0, 3.0]')`.

Also, the following formats of dates will be recognized and converted to timestamps while all other date formats will be treated as strings:
- `%Y-%m-%dT%H:%M:%E*S%Z`
- `%Y-%m-%d'T'%H:%M:%S%Z`
- `%Y-%m-%dT%H:%M:%E*S`
- `%Y-%m-%dT%H:%M:%s`
- `%Y-%m-%dT%H:%M`
- `%Y-%m-%dT%H`


Keep in mind that the `/bulk` HTTP endpoint does not support automatic table creation (auto schema). Only the `/_bulk` (Elasticsearch-like) endpoint and the SQL interface support this feature.

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

## Auto ID
<!-- example autoid -->
Manticore provides automatic ID generation for documents inserted or replaced into a real-time or [Percolate table](../../Creating_a_table/Local_tables/Percolate_table.md). The generator produces a unique numeric value with the guarantees below, but it should not be considered an auto-incrementing sequence.

The generated ID value is guaranteed to be unique under the following conditions:
* The [server_id](../../Server_settings/Searchd.md#server_id) value of the current server is in the range of 0 to 127 and is unique among nodes in the cluster, or it uses the default value generated from the MAC address as a seed
* The system time does not change for the Manticore node between server restarts
* The average auto-ID generation rate between two server starts stays below about 16 million IDs per second

The auto ID generator creates a 64-bit integer with the following layout:
* Bits 0 to 23 are a counter that is incremented on every call to the auto ID generator
* Bits 24 to 55 store the server start time in seconds, encoded as `(unix_timestamp_at_start - 2019-05-01 00:00:00 UTC)`
* Bits 56 to 62 store the `server_id` (the value is masked to the range 0..127)

This layout ensures that generated IDs are unique among cluster nodes and that data inserted into different cluster nodes does not create collisions. This is particularly important when working with replicated tables, as it guarantees that auto-generated IDs are unique across all nodes in the replication cluster.

Important: the 24-bit counter is not a hard limit on the total number of documents you can insert during one server run. You can insert more than 16,777,216 documents after startup; the IDs will still keep increasing and remain unique for that running process. The `~16 million IDs per second` rule matters for uniqueness across restarts: after a restart, the time-based part must advance far enough so that newly generated IDs do not overlap with IDs created before the restart.

As a result, the first ID from the generator used for auto ID is NOT 1 but a larger number. Additionally, the document stream inserted into a table might have non-sequential ID values if inserts into other tables occur between calls, as the ID generator is singular in the server and shared between all its tables.

For numeric-ID tables, this integer is the public document ID. For UUID-ID tables, Manticore encodes it as a canonical UUIDv8 string; clients see only the UUID.

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
### UUID_SHORT multi-ID generation

```sql
CALL UUID_SHORT(N)
```

The `CALL UUID_SHORT(N)` statement allows for generating N unique 64-bit IDs in a single call without inserting any documents. It is particularly useful when you need to pre-generate IDs in Manticore for use in other systems or storage solutions. For example, you can generate auto-IDs in Manticore and then use them in another database, application, or workflow, ensuring consistent and unique identifiers across different environments.

<!-- intro -->
##### Example:
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
## Bulk adding documents
You can insert not just a single document into a real-time table, but as many as you'd like. It's perfectly fine to insert batches of tens of thousands of documents into a real-time table. However, it's important to keep the following points in mind:
* The larger the batch, the higher the latency of each insert operation
* The larger the batch, the higher the indexation speed you can expect
* You might want to increase the [max_packet_size](../../Server_settings/Searchd.md#max_packet_size) value to allow for larger batches
* Normally, each batch insert operation is considered a single [transaction](../../Data_creation_and_modification/Transactions.md) with atomicity guarantee, so you will either have all the new documents in the table at once or, in case of failure, none of them will be added. See more details about an empty line or switching to another table in the "JSON" example.

Note that the `/bulk` HTTP endpoint does not support automatic creation of tables (auto schema). Only the `/_bulk` (Elasticsearch-like) endpoint and the SQL interface support this feature. The `/_bulk` (Elasticsearch-like) HTTP endpoint allows the table name to include the cluster name in the format `cluster_name:table_name`.

`/_bulk` endpoint accepts document IDs in the same format as Elasticsearch, and you can also include the `id` within the document itself:
```json
{ "index": { "table": "products", "_id": "1" } }
{ "title": "Crossbody Bag with Tassel", "price": 19.85 }
```

or

```json
{ "index": { "table": "products" } }
{ "title": "Crossbody Bag with Tassel", "price": 19.85, "id": "1" }
```

For an RT table declared with `id uuid`, `/bulk` reads UUIDs from `id`. `/_bulk` reads them from metadata `_id` or the document `id`. Both endpoints can omit the ID to generate a UUID automatically.

#### Chunked transfer in /bulk
The `/bulk` (Manticore mode) endpoint supports [Chunked transfer encoding](https://en.wikipedia.org/wiki/Chunked_transfer_encoding). You can use it to transmit large batches. It:
* reduces peak RAM usage, lowering the risk of OOM
* decreases response time
* allows you to bypass [max_packet_size](../../Server_settings/Searchd.md#max_packet_size) and transfer batches much larger than the maximum allowed value of `max_packet_size` (128MB), for example, 1GB at a time.

### Bulk import

For faster large batch loads into a local real-time table, Manticore Search can write `INSERT` rows directly to a disk chunk and publish that chunk when the transaction commits. This avoids building the batch in a RAM chunk first. Rows remain invisible until the chunk is published, and a failed operation or `ROLLBACK` leaves the table unchanged.

It supports both row-wise and columnar tables, including full-text fields, numeric attributes, strings, JSON, MVA/MVA64, and float vectors with KNN indexes.

Table names are case-insensitive here. For example, `SET bulk_import=Products` and `bulk_import=products` both select the table stored as `products`; tables whose names differ only by letter case cannot be distinguished.

#### SQL

Enable the mode for the current SQL session, run one or more `INSERT` statements against the bound table, and commit:

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

Run `SET bulk_import=<table>` before `BEGIN` and before any uncommitted write on that connection. It reserves the selected table for that session: searches remain available, while another bulk import or an ordinary write to the same table is rejected until the reservation is released. `BEGIN` and `START TRANSACTION` are allowed but not required and have no effect in this mode.

`COMMIT` publishes the rows collected since the previous `COMMIT` or `ROLLBACK` as one disk chunk. `ROLLBACK` discards those rows. In both cases, bulk import remains enabled so you can start another batch. Run `SET bulk_import=0` to discard any uncommitted rows, disable bulk import, and release this connection's reservation. Writes to the table resume after the reservation is released. Closing the connection does the same. Autocommit cannot be changed while bulk import is enabled.

For a sharded table, SQL works differently: see [Shared bulk import into a sharded table](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Shared-bulk-import-into-a-sharded-table).

#### HTTP `/bulk`

Add `bulk_import=<table>` to a Manticore `/bulk` request. The request body remains standard newline-delimited JSON (NDJSON), and each operation must be `insert` or `create` for the bound table:

```bash
printf '%s\n' \
  '{"insert":{"table":"products","id":101,"doc":{"title":"Crossbody Bag with Tassel","price":19.85}}}' \
  '{"insert":{"table":"products","id":102,"doc":{"title":"Microfiber Sheet Set","price":19.99}}}' |
curl -sS -X POST \
  -H 'Content-Type: application/x-ndjson' \
  --data-binary @- \
  'http://localhost:9308/bulk?bulk_import=products'
```

A line can also be a bare document: the table comes from `bulk_import=<table>`, and `id` is one of the fields:

```bash
printf '%s\n' \
  '{"id":101,"title":"Crossbody Bag with Tassel","price":19.85}' \
  '{"id":102,"title":"Microfiber Sheet Set","price":19.99}' |
curl -sS -X POST \
  -H 'Content-Type: application/x-ndjson' \
  --data-binary @- \
  'http://localhost:9308/bulk?bulk_import=products'
```

Every field of a bare document is a column, including a field named `doc`; a document without `id` gets an [auto-generated ID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID). A line is an operation when its only field is named as a `/bulk` operation (`insert`, `create`, `index`, `replace`, `update` or `delete`) and holds an object with `table` or `index`; any other line is a bare document. Both forms can be mixed in one request. A `/bulk` request without `bulk_import` still requires the operation form.

In a request body, an empty line ends and publishes the current batch; the end of the request publishes the final batch. Manticore sends the HTTP response after processing all batches in that request. If a later batch fails, batches published earlier in the same request remain searchable. To publish the entire request atomically as one disk chunk, do not include empty lines. The response contains one aggregate `bulk` result for each published batch rather than one result per document.

For a typical import, send the complete NDJSON body in one request as shown above. Close the HTTP connection when the import is complete to release the table for other writes.

The endpoint supports chunked transfer encoding, so it can process bodies larger than `max_packet_size` without buffering the whole request.

For a sharded table, `/bulk` works differently: see [Shared bulk import into a sharded table](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Shared-bulk-import-into-a-sharded-table).

#### Shared HTTP bulk import

Add `shared` next to `bulk_import=<table>` when many concurrent requests, for example a log shipper that sends every flush over a new connection, load one table. All such requests feed one table-wide pool of indexers. The pool does not belong to a connection: it lives across requests, connections and reconnects until it is committed, cancelled with `PURGE`, the table is dropped, or the daemon stops.

```bash
curl -sS -X POST -H 'Content-Type: application/x-ndjson' --data-binary @part1.ndjson \
  'http://localhost:9308/bulk?bulk_import=products&shared=4'
```

Each line must be an `insert` or `create` for the target table, or a bare document as described above. The first document creates the pool and starts its indexers. A successful data response (`"shared_state":"staged"`) means the documents are staged, not searchable: neither the end of a request, nor a closed connection, nor HTTP chunk boundaries publish anything.

From the first document until the pool is committed, fails or is cancelled, the pool reserves the table, as a private bulk import does: searches keep working, while ordinary writes (`INSERT`, `REPLACE`, `UPDATE`, `DELETE`) and another bulk import of the same table fail with an error saying that the table is locked. Closing connections does not release the table. [`SHOW LOCKS`](../../Securing_and_compacting_a_table/Freezing_and_locking_a_table.md#SHOW-LOCKS) lists a reserved table with the lock type `write_allow_readers`. The first document of a new pool is rejected with `shared_precondition` and the same error while another bulk import holds the table, while the table is locked with `LOCK TABLES … READ`, or while an ordinary write to it is running.

A request without documents that contains an empty line commits the pool:

```bash
printf '\n' | curl -sS -X POST -H 'Content-Type: application/x-ndjson' --data-binary @- \
  'http://localhost:9308/bulk?bulk_import=products&shared'
```

The commit closes the pool to new data, stops the indexers that received no data, waits until every other indexer has built its part, attaches one disk chunk per such indexer, removes the pool and answers `"shared_state":"published"`. If other data requests are still running, the commit answers HTTP `409` with `shared_busy` and the pool stays open; repeat the commit after the writers finish. A commit without a pool answers `409` with `shared_no_pool`, because a commit never creates a pool; a pool that received no data publishes no chunk. Empty lines in a request with documents, including lines of only spaces, are ignored, so a data request never publishes. LF and CRLF both end a line.

The number of indexers, and so of new disk chunks, is set by `shared=N` in the request that creates the pool. With a bare `shared`, the daemon uses the `INDEXER_RT_BULK_SHARED_POOL_SIZE` environment variable of `searchd`, otherwise one indexer. N is limited to the range from 1 to the number of worker [threads](../../Server_settings/Searchd.md#threads) minus 2 (at least 1): a smaller value, or one that is not a number, gives one indexer, and a larger value gives the maximum. Only the request that creates the pool reads N; requests that join the open pool, and the commit, ignore it. Data is spread over the indexers in batches of about 32 KiB, in turn. More indexers shorten the build after the commit but use more CPU and memory while loading. A good starting point is about two thirds of the worker threads, for example 8 indexers with 12 threads; with more, the indexers compete with the incoming requests for CPU and disk, and the load gets slower. Each indexer needs roughly 200 MB of RAM while loading with the default [mem_limit](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Memory-and-disk-usage).

Every response is one JSON object with `errors`, `error`, `error_code` and `shared_state`:

| `shared_state` | HTTP | `error_code` | Meaning |
| --- | --- | --- | --- |
| `staged` | 200 | | data is in the pool |
| `published` | 200 | | the pool was committed |
| `busy` | 409 | `shared_busy` | commit while other data requests are active |
| `closing` | 409 | `shared_closing` | the pool is being published |
| `no_pool` | 409 | `shared_no_pool` | commit, or a data request with `shared_join`, without a pool |
| `rejected` | 400, or 403 without write permission | `shared_precondition` | invalid request, e.g. no write permission or a table that is locked when the pool is created |
| `failed` | 400, 409 or 500 | `shared_failed` | the load failed, see below |

`current_line` counts the documents of the request; a commit reports `1`. On an error `skipped_lines` equals `current_line`, although the batches of the request already handed to the indexers stay in the pool.

If a connection breaks or times out in the middle of a request body, or the request is killed with `KILL`, only that request fails: its last unwritten part (less than 32 KiB) is dropped, data already handed to the indexers stays, and the pool remains open, so the client can resend the request. An invalid document (malformed JSON, unknown column, another table, an operation other than `insert`/`create`) or a failed write to an indexer fails the whole load: its indexers are stopped and the table is released at once, that request gets `400` for an invalid document or `500` for a failed write, every later request gets `409` with the first error, and the next commit returns this error with `500` and removes the failed pool and its staging files; `PURGE … OPTION clear_sessions=1` removes it as well. Only then can a new load start.

A failed request and a failed load get the same answer, `failed` with `shared_failed` (`400`, or `500` for a killed request), so the response alone does not tell them apart. The next request does: `staged` means the pool is still open, while `409` with `shared_failed` means the load failed.

A pool lives only in the memory of `searchd`. If the daemon stops or crashes during a load, or the pool is cancelled, its staged documents are gone, and the next data request would start a new pool that holds only the later documents. To notice this, add `shared_join` to every data request after the first `staged` reply: such a request only joins the open pool and never creates one, so without a pool it answers `409` with `shared_no_pool`, and the load can be repeated from the start.

The pool does not end when its clients do. If they are gone before the commit, for example because a log shipper was stopped or crashed, the pool stays open and keeps the table reserved, so ordinary writes keep failing. Release the table in one of two ways: commit the pool as shown above, which publishes the documents staged so far, or cancel it, which discards them:

```sql
PURGE BULK_IMPORT FROM TABLE products OPTION clear_sessions=1;
```

The cancel stops the indexers, releases the table and removes the staging files; chunks published earlier stay in the table. A commit answers `409` with `shared_busy` while a data request of a vanished client still counts as running, until that request times out; the cancel does not wait for it. Once the table is released, `SHOW LOCKS` no longer lists it, and a new Shared load can start. See [`PURGE BULK_IMPORT`](../../Node_info_and_management/PURGE.md#PURGE-BULK_IMPORT) for details.

#### Shared bulk import into a sharded table

A [sharded table](../../Creating_a_table/Creating_a_sharded_table/Creating_a_sharded_table.md) also takes a Shared HTTP bulk import, and SQL `INSERT` statements can feed the same load. Name the sharded table in the request:

```bash
curl -sS -X POST -H 'Content-Type: application/x-ndjson' --data-binary @part1.ndjson \
  'http://localhost:9308/bulk?bulk_import=sharded_products'
```

Send all requests of a load and all ordinary writes to the sharded table through one node, the node that coordinates the load. With replication factor `1`, a load started on another node is refused at its first document. With a higher replication factor, it can pick other replicas and is not refused.

The coordinating node routes every document to the shard that a regular `INSERT` into the sharded table would choose, after the same ID conversion and auto-ID assignment. A document with a cluster prefix is checked as in a regular `INSERT` into the sharded table. A bare document takes the cluster of the sharded table.

The first document of the load starts a Shared pool with one indexer on every shard; for the memory an indexer needs, see [Shared HTTP bulk import](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Shared-HTTP-bulk-import). A pool runs on the node of its shard, so every node with shards of the table needs:

* `indexer` next to `searchd`;
* a version of Manticore Search that supports this load. An older node fails the load at its first document.

Every shard that received documents gets one new disk chunk. A request needs no `shared`; `shared=N` and `INDEXER_RT_BULK_SHARED_POOL_SIZE` are ignored. The responses, `shared_join` and the empty-line commit work as described above.

##### Replication factor above 1

With a replication factor above `1`, every shard is a table in a [replication cluster](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md) with a replica on several nodes. When the load starts, the coordinating node picks one replica of every shard, as a regular write does, and the pool of the shard runs on that replica. At the commit, that replica publishes its chunk as a [bulk import into a replicated table](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-import-into-a-replicated-table) does. The load does not move to another replica: if the picked replica fails, the load fails as with one replica.

SQL is one more client of the same load. On a connection to the coordinating node, run `SET bulk_import=<sharded table>`. Then every `INSERT` works as a data request and `COMMIT` as the commit request:

```sql
SET bulk_import=sharded_products;
INSERT INTO sharded_products(id,title,price) VALUES
  (101,'Crossbody Bag with Tassel',19.85),
  (102,'Microfiber Sheet Set',19.99);
COMMIT;
```

Unlike SQL bulk import into a real-time table, the load belongs to the sharded table, not to the connection:

* `SET bulk_import=<sharded table>` reserves nothing; the shards are reserved from the first document.
* Other SQL connections and HTTP requests can feed the same load.
* `SET bulk_import=0` or a closed connection leaves the load open.
* An invalid `INSERT` fails the whole load, as an invalid document does.
* `COMMIT` fails while another request is writing to the load, and when there is no load to commit.
* `ROLLBACK` is not supported. Cancel the load with `PURGE BULK_IMPORT FROM TABLE <sharded table> OPTION clear_sessions=1`.

##### Writes during a sharded load

The coordinating node refuses an ordinary `INSERT`, `REPLACE`, `DELETE` or `UPDATE` of the sharded table with `table '<sharded table>' is locked` until the commit ends, with any replication factor. Nothing is applied to any shard. In a transaction, `INSERT`, `REPLACE` and `DELETE` are accepted, and `COMMIT` is refused. An `UPDATE` of a sharded table is not part of a transaction and is refused at once.

A write that bypasses the coordinating node, through another node or straight into a shard:

* with one replica, fails with `table is locked` on every shard that runs a pool of the load;
* with a replication factor above `1`, can reach a replica without the pool and is applied there. A write whose commit had passed the check of the coordinating node before the load started can also finish this way. A published row replaces a row with the same ID in the order the cluster applies them.

A direct Shared request into a shard of the load is refused. `SHOW LOCKS` and `@files` show the shards by their own names.

##### Failures before the commit

Until the commit, any of these fails the whole load and cancels its pools on all shards:

* an invalid document;
* a shard that cannot start a pool: it is locked by another bulk import or by a running ordinary write, it is a local shard changed after the sharded table was loaded, or its node has no `indexer`;
* a failed write to a shard;
* any failed call to a remote shard, including a lost connection or a timeout.

The next commit returns the error and removes the load, or `PURGE` cancels it. Then repeat the whole load. A client that breaks its own request, or a `KILL` of a data request, fails only that request.

The cancel is best effort. After a load has failed, the coordinating node no longer refuses writes, but a shard whose node the cancel did not reach keeps its pool. Such a shard refuses ordinary writes with `table is locked`, so an ordinary commit that spans several shards can apply its changes to some of them before it stops there. The next load is refused on that shard too. Remove such pools as described below.

##### Commit and partial failure

The commit publishes all the shards at once. Every indexer builds its chunk at the same time, so the commit takes about as long as the largest shard. A shard that fails does not stop the others, and the shards that succeeded keep their chunks. The commit then answers `500` with `shared_failed` and an error that names the first failed shard and its error, followed by `published <N> of <M> tables` and the published shards.

A failed shard can still have published its rows:

* the reply of a remote shard to the commit was lost. The error says that its commit is not confirmed.
* a replica of a replicated shard failed to attach its chunk while the other nodes of its cluster attached it. The rows reach every node, and that replica gets them when it rejoins the cluster.

After a failed commit, repeating the load with explicit IDs is safe. A `KILL` of the commit request fails the local shards that are still building. Remote shards have already received the commit and finish it.

##### Leftover shard pools

A shard pool stays on its node until the load is committed or cancelled, also when the coordinating node stops or crashes. Remove leftover pools with `PURGE BULK_IMPORT FROM TABLE <sharded table> OPTION clear_sessions=1` on any node that has the sharded table, or by the shard name on the node of the shard. [`PURGE BULK_IMPORT`](../../Node_info_and_management/PURGE.md#Sharded-and-distributed-tables) describes its errors. Run it:

* after the coordinating node stopped or crashed. A daemon shutdown cancels the load it coordinates and the pools of its own shards, but not the pools on other nodes.
* with a replication factor above `1`, after the coordinating node restarts, before you write to the sharded table or start a new load. Until then, a write or a new load that picks the replica with the old pool is refused, and one that picks another replica is not.
* when a node that the cancel did not reach is back.

`DROP TABLE` of the sharded table on the coordinating node cancels the load and the pools of its shards with the same best effort. A commit that is already running finishes. A pool left on an unreachable node can then be removed only by the shard name on that node.

Duplicate document IDs follow the rules below for each shard: a document ID always goes to the same shard, so the first row with an ID in the load stays, and a published row replaces a row with the same ID.

#### Bulk import into a replicated table

A real-time table in a [replication cluster](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md) takes SQL, Manticore `/bulk` and Shared bulk import on any node of the cluster. As in any [write to a cluster table](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Write-statements), every row must name the cluster: `INSERT INTO <cluster>:<table>` in SQL and `"cluster":"<cluster>"` in JSON. A row without it is rejected with the same error as a regular `INSERT`. So is a row with a cluster for a table that is not in a cluster. `SET bulk_import=<table>` and the `bulk_import=<table>` parameter take the table name without the cluster. A bare document belongs to the table of the request and goes to its cluster without a cluster name.

```sql
SET bulk_import=products;
INSERT INTO posts:products(id,title,price) VALUES
  (101,'Crossbody Bag with Tassel',19.85),
  (102,'Microfiber Sheet Set',19.99);
COMMIT;
SET bulk_import=0;
```

```bash
printf '%s\n' \
  '{"insert":{"cluster":"posts","table":"products","id":101,"doc":{"title":"Crossbody Bag with Tassel","price":19.85}}}' |
curl -sS -X POST -H 'Content-Type: application/x-ndjson' --data-binary @- \
  'http://localhost:9308/bulk?bulk_import=products'
```

##### Replicated commit

The node that runs the load builds the disk chunk as described above. At commit it:

1. Waits until no node of the cluster is a donor or is joining. It checks up to twice [replication_retry_count](../../Server_settings/Searchd.md#replication_retry_count) times, twice [replication_retry_delay](../../Server_settings/Searchd.md#replication_retry_delay) apart, and then fails the commit.
2. Sends the chunk files to every other node of the cluster. Each node checks the chunk when it has received it.
3. Attaches the chunk on all nodes, its own included, with one replicated command.

The commit answers once the node that runs the load has attached the chunk. The other nodes attach it as they apply the command, as with any replicated write. A Shared commit does this for every chunk it publishes, one after another.

Sending the chunk uses the same file transfer as [adding a table to a cluster](../../Creating_a_cluster/Setting_up_replication/Adding_and_removing_a_table_from_a_replication_cluster.md), to all nodes at once. So a commit takes longer than the same commit into a local table, by about the time that `ALTER CLUSTER ... ADD` of a table of the chunk's size would take. The command itself only attaches the files that the nodes already have. If the table has rows in RAM that are not saved to disk yet, for example from ordinary writes through other nodes, the command first saves them to disk on every node, and replication of the cluster waits until it finishes.

Each node keeps a received chunk in `<table-filebase>.indexer-rt-bulk/repl-<id>.<ext>`, with a `.new` suffix while it is being received, and keeps the prepared chunk open in memory until the attach command takes it or cleanup discards it.

##### Writes during a replicated load

While a load runs, only the node that runs it reserves the table: on that node, ordinary writes to the table fail with `table is locked`, as described above. Writes through other nodes keep working and are replicated as usual. Another node can run its own bulk import into the same table at the same time; every commit attaches its own chunk. A published row replaces a row with the same ID on every node, also a row written through another node while the load ran.

##### Node failures

* A node of the cluster cannot receive the chunk, for example it does not answer or cannot create the staging directory of the table. The commit fails with an error that names the node, every node that answers removes the chunk, and no node changes the table. A Shared commit publishes its chunks one by one, so the chunks attached before the failed one stay on all nodes, as described in [Duplicate document IDs](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Duplicate-document-IDs).
* A node is not in the cluster at the moment of the commit, for example because it stopped or lost its network. It does not get the chunk. When it comes back, it rejoins the cluster with a full state transfer (SST) and gets the table with the rows.
* A node has left the cluster but is still in its [node list](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) and accepts connections without answering, for example a hung process. It delays the commit: the check for donor and joining nodes waits for its answer, which can take minutes. Run `ALTER CLUSTER <cluster> UPDATE nodes` to drop such a node from the list. A node that does not accept connections delays the commit by seconds only.
* A node cannot attach a chunk that the other nodes attach, for example because its copy was removed. It leaves the cluster: the replication provider stops the daemon, and after a restart (at once, if the watchdog is enabled) the node rejoins with a full state transfer.
* The node that runs the load cannot attach the chunk that the other nodes attach. The client gets an error saying that the other nodes attached the chunk, and the node rejoins the cluster with a full state transfer without a restart. If this rejoin fails twice, the cluster stays `closed` on the node, with the reason in `cluster_<name>_last_error` (see [Replication cluster status](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md)). Restart the daemon, and the node rejoins with a full state transfer.

Until a node that did not attach a chunk has left the cluster, it does not send its tables to a joining node. A `JOIN CLUSTER` that it serves fails with `the donor did not attach a bulk_import chunk and leaves the cluster, join again`; repeat the join.

##### Leftover chunks

Between the delivery of a chunk and its attach command, `PURGE BULK_IMPORT` of the table is refused on every node that keeps the chunk. A node can also keep a delivered chunk after the load has ended, when:

* the node that runs the load stopped before the command;
* the node itself was out of the cluster at the moment of the command;
* the node did not answer while the delivery was cancelled.

The chunk stays in the staging root of the table, which `@files` shows. [`PURGE BULK_IMPORT`](../../Node_info_and_management/PURGE.md#Tables-in-a-replication-cluster) describes when to wait for the commit and when to remove the chunk.

##### Limitations of a replicated table

* A table with external files, such as stopwords, wordforms or exceptions, is not supported: the chunk is built on one node and attached on all nodes as it is. This is not checked, so such a load is not refused.
* Every node of the cluster needs a version of Manticore Search that supports it. When upgrading from an earlier version, stop all nodes of the cluster and start it again, see [Upgrading a cluster](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Upgrading-a-cluster).

#### Elasticsearch `/_bulk`

The Elasticsearch-compatible `/_bulk` endpoint does not support direct-to-disk `bulk_import`; use SQL or Manticore `/bulk`.

#### Duplicate document IDs

Within one direct-to-disk batch, the first row for a numeric document ID remains visible and later rows with the same ID are logically removed. For SQL, a batch consists of the rows staged before `COMMIT` or `ROLLBACK`. For Manticore `/bulk`, each group published at an empty line, table change, or request EOF is a separate batch. In a Shared load, the data of each indexer is a separate batch, and its chunks are attached one after another when the pool is committed. If attaching one of them fails, the chunks attached before it stay in the table and the commit answers `500`; loading the same documents again is safe for explicit IDs, because a published row replaces any row with the same ID, while documents with auto-generated IDs are added again.

When Manticore publishes a disk chunk to the target table, a row in that chunk replaces any existing row with the same ID. This also applies to IDs published by an earlier HTTP batch. As a result, retrying a previously published batch replaces its rows, while duplicates within the retried batch still keep the first row. In a Manticore `/bulk` request, the `create` operation, for example:

```json
{"create":{"table":"products",...}}
```

behaves like `insert`; it does not fail merely because the ID already exists.

#### Memory and disk usage

A direct-to-disk load does not collect its documents in the memory of `searchd`. Each `INSERT` statement, or each part of a `/bulk` request body (about 32 KiB when the request has the `shared` parameter), is converted to CSV and passed to an `indexer` process over a pipe right away. `indexer` builds the chunk the same way as a plain table: it collects data in memory up to its [mem_limit](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#mem_limit), flushes it to temporary files and, after the last row, merges them into the disk chunk. So the memory a load needs does not grow with its size: each `indexer` process takes its `mem_limit` plus its [write buffers](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#write_buffer) and the process itself, roughly 200 MB in total with the default 128 MB `mem_limit`. A `SET bulk_import` session or a `/bulk` request runs one `indexer`, a load with `shared=N` runs N; add the statement or request part being processed.

The `indexer` processes take these options of the `searchd` configuration file: `mem_limit`, `write_buffer`, `max_iops`, `max_iosize` and `lemmatizer_cache` of the [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-configuration-settings) section, and `lemmatizer_base`, `plugin_dir`, `on_json_attr_error`, `json_autoconv_numbers` and `json_autoconv_keynames` of the [common](../../Server_settings/Common.md) section. Other options of these sections are not used by bulk import. `searchd` reads them when it starts, as it reads the `common` section; a relative path is resolved as `searchd` resolves it. Without `mem_limit` and `write_buffer` a load uses 128 MB and 1 MB. JSON values are parsed with the options of the `common` section, as for `INSERT`; the deprecated copies of these options in the `indexer` section are not used.

`mem_limit` applies to every `indexer` process, and many can run at the same time: one for every SQL session with uncommitted bulk import rows and for every running `/bulk` request, N for every Shared load, and, on every node, one for every shard of a [sharded table](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Shared-bulk-import-into-a-sharded-table) that a load writes. A large `mem_limit` set for plain tables multiplies by that number, and it does not make a load faster: in our tests a chunk was built fastest with the default 128 MB.

As with a plain table (see [inplace_enable](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#inplace_enable)), the intermediate data in the temporary files is comparable in size to the finished chunk. Both are written to the table's staging directory, next to the table files, so at the peak the disk that holds the table needs free space for about twice the size of the new chunk. When the chunk is published, its files are renamed into the table without copying. See [Staging files and cleanup](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Staging-files-and-cleanup).

#### Staging files and cleanup

Manticore removes the current staging directory after a normal `COMMIT`, `ROLLBACK`, mode disable, or session close. A Shared pool keeps its staging directories until it is committed or cancelled; closing connections does not remove them. A daemon or host crash can leave an abandoned staging directory behind. A later direct-to-disk load creates a new uniquely named directory and does not reuse or attach files left by the crashed load.

Use the following query to inspect all direct-to-disk staging entries for a table:

```sql
SELECT file, normalized, size
FROM products.@files
OPTION format='bulk_import';
```

The result recursively lists files and directories below the table's direct-to-disk staging root. Directories and other non-regular entries have a reported size of `0`.

After confirming that no direct-to-disk load is active for the table, remove its entire staging root with [`PURGE BULK_IMPORT`](../../Node_info_and_management/PURGE.md#PURGE-BULK_IMPORT):

```sql
PURGE BULK_IMPORT FROM TABLE products;
```

While the table has an open Shared pool, `PURGE` is refused; `PURGE BULK_IMPORT FROM TABLE products OPTION clear_sessions=1` cancels the pool first, stops its indexers, releases the table and then removes the staging root. While the pool is attaching its chunks, `PURGE` is refused; repeat it after the commit finishes. On Windows, `PURGE … OPTION clear_sessions=1` or `DROP TABLE` while a data request is still writing to the pool can answer `failed to remove bulk staging root`: the pool is already cancelled, but its stopped indexers keep their files open for a moment; repeat the statement.

`PURGE` requires an existing local real-time table, or a sharded or distributed table, see [`PURGE BULK_IMPORT`](../../Node_info_and_management/PURGE.md#Sharded-and-distributed-tables). For a table in a replication cluster, it purges only the node where it runs, see [Bulk import into a replicated table](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-import-into-a-replicated-table). It removes only direct-to-disk staging state and does not change the table schema or indexed rows. It succeeds as a no-op when the staging root is absent. In configless mode, `DROP TABLE` also removes the table's direct-to-disk staging root and cancels its Shared pool; a daemon shutdown cancels every Shared pool. Chunks published earlier stay in the table.

#### Current limitations

* The target is one existing, unfrozen local real-time table, also one in a replication cluster, see [Bulk import into a replicated table](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-import-into-a-replicated-table).
* For Manticore `/bulk` and SQL, the target can also be a sharded table, see [Shared bulk import into a sharded table](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Shared-bulk-import-into-a-sharded-table).
* Distributed, percolate, and plain tables are not supported.
* SQL supports only `INSERT`. Manticore `/bulk` accepts `insert`, `create` and bare documents; `index`, `replace`, `update`, and `delete` are rejected.
* A document ID is converted as in a regular `INSERT`, including [attr_autoconv_strict](../../Server_settings/Searchd.md#attr_autoconv_strict). A row without an ID, or with an ID that converts to 0, gets an [auto-generated ID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID); the response does not return generated IDs, and a repeated load adds such rows again. UUID document IDs are not supported.
* Static builds are not supported.
* The platform-specific executable (`indexer` on Linux, `indexer.exe` on Windows) must be in the same directory as the running `searchd` executable. Manticore Search resolves only that sibling path and does not search `PATH`. Use the executable from the same installation as `searchd` to ensure compatibility. If it is absent, unreadable, or cannot be started, only bulk import fails; normal startup and regular insertion remain available.

The assisted loader uses the `csvpipe` source. Its command defaults to `/bin/cat` on Linux and `-` on Windows, where `-` reads from the indexer's standard input. Set `INDEXER_RT_BULK_CSV_PIPE_COMMAND` in the `searchd` environment to override only this command. The override does not select another source or feed; the command must work with the platform's input feed and pass CSV rows to the indexer. An unsupported command causes the assisted load to fail.

<!-- intro -->
### Bulk insert examples
##### SQL:
<!-- request SQL -->
For bulk insert, simply provide more documents in brackets after `VALUES()`. The syntax is:

```sql
INSERT INTO <table name>[(column1, column2, ...)] VALUES(value1[, value2 , ...]), (...)
```

The optional column name list allows you to explicitly specify values for some of the columns present in the table. All other columns will be filled with their default values (0 for scalar types, empty string for string types).

For example:

```sql
INSERT INTO products(title,price) VALUES ('Crossbody Bag with Tassel', 19.85), ('microfiber sheet set', 19.99), ('Pet Hair Remover Glove', 7.99);
```
<!-- response SQL -->

```sql
Query OK, 3 rows affected (0.01 sec)
```

Expressions are currently not supported in `INSERT`, and values should be explicitly specified.

<!-- intro -->
##### JSON:
<!-- request JSON -->
The syntax is generally the same as for [inserting a single document](../../Quick_start_guide.md#Add-documents). Just provide more lines, one for each document, and use the `/bulk` endpoint instead of `/insert`. Enclose each document in the "insert" node. Note that it also requires:
* `Content-Type: application/x-ndjson`
* The data should be formatted as newline-delimited JSON (NDJSON). Essentially, this means that each line should contain exactly one JSON statement and end with a newline `\n` and possibly `\r`.

The `/bulk` endpoint supports 'insert', 'replace', 'delete', and 'update' queries. Keep in mind that you can direct operations to multiple tables, but transactions are only possible for a single table. If you specify more, Manticore will gather operations directed to one table into a single transaction. When the table changes, it will commit the collected operations and initiate a new transaction. An empty line separating batches also leads to committing the previous batch and starting a new transaction. A request containing no operations, whether its body is empty or contains only empty or whitespace-only lines, is a successful no-op and returns an empty `items` array.

In the response for a `/bulk` request, you can find the following fields:
* "errors": shows whether any errors occurred (true/false)
* "error": describes the error that took place
* "current_line": the line number where execution stopped (or failed); empty lines, including the first empty line, are also counted
* "skipped_lines": the count of non-committed lines, beginning from the `current_line` and moving backward

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

For replicated tables, include the `cluster` property in each operation:

```json
POST /bulk
-H "Content-Type: application/x-ndjson" -d '
{"insert": {"cluster":"posts", "table":"weekly_table", "id":1, "doc":  {"title":"Crossbody Bag with Tassel","price":19.85}}}
{"insert": {"cluster":"posts", "table":"weekly_table", "id":2, "doc":  {"title":"microfiber sheet set","price":19.99}}}
{"insert": {"cluster":"posts", "table":"weekly_table", "id":3, "doc":  {"title":"Pet Hair Remover Glove","price":7.99}}}
'
```

<!-- request Elasticsearch -->

> NOTE: `_bulk` requires [Manticore Buddy](Installation/Manticore_Buddy.md) if the table doesn't exist yet. If it doesn't work, make sure Buddy is installed.

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

For replicated tables, include the cluster name in the table name using the format `cluster_name:table_name`:

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
Use method addDocuments():

```php
$index->addDocuments([
        ['id' => 1, 'title' => 'Crossbody Bag with Tassel', 'price' => 19.85],
        ['id' => 2, 'title' => 'microfiber sheet set', 'price' => 19.99],
        ['id' => 3, 'title' => 'Pet Hair Remover Glove', 'price' => 7.99]
]);
```

For replicated tables, set the cluster name before adding documents:

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

For replicated tables, include the cluster property in each document:

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

For replicated tables, include the cluster property in each document:

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

For replicated tables, include the cluster property in each document:

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

For replicated tables, include the cluster property in each document:

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

For replicated tables, include the cluster property in each document:

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
## Inserting multi-value attributes (MVA) values

Multi-value attributes (MVA) are inserted as arrays of numbers.
<!-- intro -->
### Examples
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

Or, alternatively
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
## Inserting JSON
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

JSON value can be also inserted as a string containing escaped JSON:
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

Or, alternatively
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
Consider JSON just as string:
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
