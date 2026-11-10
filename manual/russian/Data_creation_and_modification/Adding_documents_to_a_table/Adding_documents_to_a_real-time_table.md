# Добавление документов в таблицу реального времени

> Если вам нужна информация о добавлении документов в обычную таблицу, см. раздел [добавления данных из внешних хранилищ](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md).

<!-- example insert -->
Добавление документов в реальном времени поддерживается только для [таблиц реального времени](../../Creating_a_table/Local_tables/Real-time_table.md) и [таблиц percolate](../../Creating_a_table/Local_tables/Percolate_table.md). Соответствующая SQL-команда, HTTP-эндпоинт или функции клиента вставляют в таблицу новые строки (документы) с указанными значениями полей. Создавать таблицу перед добавлением документов необязательно. Если таблица не существует, Manticore попытается создать её автоматически. Подробнее см. в разделе [Автоматическая схема](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema).

Можно вставить один или [несколько документов](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-adding-documents), указав значения всех полей таблицы или только части из них. В последнем случае остальные поля получат значения по умолчанию (0 для скалярных типов, пустая строка для текстовых типов).

Выражения в `INSERT` пока не поддерживаются, поэтому значения необходимо указывать явно.

Поле ID и его значение можно опустить: таблицы RT и PQ поддерживают [автоматическую генерацию ID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID). В таблицах с числовыми ID для автоматической генерации также можно указать `0` в качестве ID. Команда `INSERT` не перезаписывает строки с уже существующими ID. Для этого используйте [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md).

Для таблиц, созданных с [`id uuid`](../../Creating_a_table/Data_types.md#UUID-document-IDs), передайте UUID явно в виде строки в кавычках или опустите `id`, чтобы сгенерировать его автоматически. Явные значения должны соответствовать формату `xxxxxxxx-xxxx-Vxxx-Wxxx-xxxxxxxxxxxx`, где каждый `x` — шестнадцатеричная цифра, `V` — версия (от `1` до `8`), а `W` — вариант (`8`, `9`, `a` или `b`). Шестнадцатеричные буквы в верхнем регистре допускаются и преобразуются в нижний регистр. В отличие от числовых ID, значение `0` не запускает автоматическую генерацию UUID.

При работе по протоколу HTTP JSON можно выбрать один из двух форматов запросов: стандартный формат Manticore или формат, похожий на Elasticsearch. Примеры обоих форматов приведены ниже.

При использовании формата JSON-запросов Manticore учитывайте, что узел `doc` обязателен и все значения должны находиться внутри него.


<!-- intro -->
##### SQL:
<!-- request SQL -->
Общий синтаксис:

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

Для таблиц, созданных с `id uuid`, передайте `id` в JSON как строку UUID или опустите его для автоматической генерации:

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

> ПРИМЕЧАНИЕ: для `_create` требуется [Manticore Buddy](Installation/Manticore_Buddy.md). Если он не работает, убедитесь, что Buddy установлен.

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

Для реплицируемых таблиц задайте имя кластера перед добавлением документов:

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

Для реплицируемых таблиц добавьте свойство cluster в каждый документ:

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

Для реплицируемых таблиц добавьте свойство cluster в каждый документ:

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

Для реплицируемых таблиц добавьте свойство cluster в каждый документ:

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

Для реплицируемых таблиц добавьте свойство cluster в каждый документ:

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

Для реплицируемых таблиц добавьте свойство cluster в каждый документ:

``` clike
string body = "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 1, \"doc\" : {\"title\" : \"Crossbody Bag with Tassel\", \"price\" : 19.85}}}"+"\n"+
    "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 4, \"doc\" : {\"title\" : \"microfiber sheet set\", \"price\" : 19.99}}}"+"\n"+
    "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 5, \"doc\" : {\"title\" : \"Pet Hair Remover Glove\", \"price\" : 7.99}}}"+"\n";
BulkResponse bulkresult = indexApi.Bulk(string.Join("\n", docs));
```

<!-- end -->

## Добавление документов в реплицируемые таблицы
<!-- example replicated_insert -->
При работе с [реплицируемыми таблицами](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md) необходимо использовать специальный синтаксис, чтобы операции записи корректно распространялись на все узлы кластера.

Для всех операций записи (INSERT, REPLACE, DELETE, TRUNCATE, UPDATE) в реплицируемые таблицы необходимо:
* В SQL: использовать формат `cluster_name:table_name` вместо одного имени таблицы
* В JSON: указывать свойство `cluster` вместе со свойством `table`

При неправильном синтаксисе операция завершится ошибкой.

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

## Автоматическая схема

> ПРИМЕЧАНИЕ: для автоматической схемы требуется [Manticore Buddy](Installation/Manticore_Buddy.md). Если она не работает, убедитесь, что Buddy установлен.

В Manticore есть механизм автоматического создания таблиц, который срабатывает, если таблица, указанная в запросе вставки или замены, ещё не существует. По умолчанию этот механизм включён. Чтобы отключить его, задайте `auto_schema = 0` в разделе [Searchd](../../Server_settings/Searchd.md#auto_schema) конфигурационного файла Manticore.

<!-- example auto-schema -->

По умолчанию все текстовые значения в предложении `VALUES` считаются значениями типа `text`, кроме корректных адресов электронной почты, которым назначается тип `string`.

Если при INSERT/REPLACE нескольких строк одно и то же поле содержит значения разных несовместимых типов, автоматическое создание таблицы будет отменено и вернётся сообщение об ошибке. Если же типы совместимы, для поля будет выбран тип, вмещающий все значения. Возможны, в частности, следующие автоматические преобразования типов:
* mva -> mva64
* uint -> bigint -> float (возможна некоторая потеря точности)
* string -> text

Механизм автоматической схемы не поддерживает создание таблиц с векторными полями (типа `float_vector`) для поиска по сходству [KNN](../../Searching/KNN.md#Configuring-a-table-for-KNN-search) (методом ближайших соседей). Чтобы использовать векторные поля, необходимо явно создать таблицу со схемой, в которой они определены. Если нужно хранить векторные данные в обычной таблице без KNN-поиска, их можно сохранить как JSON-массив со стандартным синтаксисом JSON, например: `INSERT INTO table_name (vector_field) VALUES ('[1.0, 2.0, 3.0]')`.

Также распознаются и преобразуются в метки времени следующие форматы дат; даты во всех остальных форматах считаются строками:
- `%Y-%m-%dT%H:%M:%E*S%Z`
- `%Y-%m-%d'T'%H:%M:%S%Z`
- `%Y-%m-%dT%H:%M:%E*S`
- `%Y-%m-%dT%H:%M:%s`
- `%Y-%m-%dT%H:%M`
- `%Y-%m-%dT%H`


Учтите, что HTTP-эндпоинт `/bulk` не поддерживает автоматическое создание таблиц (автоматическую схему). Эта возможность доступна только через эндпоинт `/_bulk` (аналогичный Elasticsearch) и SQL-интерфейс.

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

## Автоматическая генерация ID
<!-- example autoid -->
Manticore автоматически генерирует ID для документов, вставляемых или заменяемых в таблице реального времени или [таблице percolate](../../Creating_a_table/Local_tables/Percolate_table.md). Генератор создаёт уникальное числовое значение с описанными ниже гарантиями, однако его не следует считать автоинкрементной последовательностью.

Уникальность сгенерированного ID гарантируется при следующих условиях:
* Значение [server_id](../../Server_settings/Searchd.md#server_id) текущего сервера находится в диапазоне от 0 до 127 и уникально среди узлов кластера либо используется значение по умолчанию, сгенерированное на основе MAC-адреса
* Системное время на узле Manticore не меняется между перезапусками сервера
* Средняя скорость автоматической генерации ID между двумя запусками сервера не превышает примерно 16 миллионов ID в секунду

Генератор автоматических ID создаёт 64-битное целое число со следующей структурой:
* Биты с 0 по 23 — счётчик, который увеличивается при каждом вызове генератора автоматических ID
* Биты с 24 по 55 хранят время запуска сервера в секундах, закодированное как `(unix_timestamp_at_start - 2019-05-01 00:00:00 UTC)`
* Биты с 56 по 62 хранят `server_id` (значение ограничивается битовой маской до диапазона 0..127)

Такая структура обеспечивает уникальность генерируемых ID на узлах кластера и предотвращает коллизии при вставке данных на разные узлы. Это особенно важно для реплицируемых таблиц: автоматически сгенерированные ID остаются уникальными на всех узлах кластера репликации.

Важно: 24-битный счётчик не ограничивает общее число документов, которые можно вставить за один запуск сервера. После запуска можно вставить более 16 777 216 документов: ID продолжат увеличиваться и останутся уникальными в пределах работающего процесса. Правило `~16 million IDs per second` важно для уникальности между перезапусками: после перезапуска временная часть должна увеличиться достаточно, чтобы новые ID не пересекались с ID, созданными до перезапуска.

Поэтому первый ID, выданный генератором автоматических ID, равен НЕ 1, а большему числу. Кроме того, ID документов, вставляемых в таблицу, могут идти с пропусками, если между вызовами происходят вставки в другие таблицы: на сервере используется один общий генератор ID для всех таблиц.

В таблицах с числовыми ID это целое число служит внешним ID документа. В таблицах с UUID в качестве ID Manticore кодирует его как строку UUIDv8 в каноническом формате; клиенты видят только UUID.

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
### Генерация нескольких ID с UUID_SHORT

```sql
CALL UUID_SHORT(N)
```

Команда `CALL UUID_SHORT(N)` позволяет сгенерировать N уникальных 64-битных ID за один вызов без вставки документов. Это особенно удобно, когда нужно заранее получить ID в Manticore для использования в других системах или хранилищах. Например, можно сгенерировать автоматические ID в Manticore, а затем использовать их в другой базе данных, приложении или рабочем процессе, сохраняя единые и уникальные идентификаторы в разных средах.

<!-- intro -->
##### Пример:
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
## Массовое добавление документов
В таблицу реального времени можно вставить не только один документ, но и любое их количество. Вставлять пакеты из десятков тысяч документов вполне допустимо. При этом учитывайте следующее:
* Чем больше пакет, тем дольше выполняется каждая операция вставки
* Чем больше пакет, тем выше ожидаемая скорость индексации
* Для работы с более крупными пакетами может потребоваться увеличить [max_packet_size](../../Server_settings/Searchd.md#max_packet_size)
* Обычно вставка каждого пакета считается одной [транзакцией](../../Data_creation_and_modification/Transactions.md) с гарантией атомарности: либо все новые документы появятся в таблице одновременно, либо при сбое не будет добавлен ни один. Подробнее о пустых строках и переключении на другую таблицу см. в примере «JSON».

Учтите, что HTTP-эндпоинт `/bulk` не поддерживает автоматическое создание таблиц (автоматическую схему). Эта возможность доступна только через эндпоинт `/_bulk` (аналогичный Elasticsearch) и SQL-интерфейс. HTTP-эндпоинт `/_bulk` (аналогичный Elasticsearch) позволяет включать имя кластера в имя таблицы в формате `cluster_name:table_name`.

Эндпоинт `/_bulk` принимает ID документов в том же формате, что и Elasticsearch. Также можно включить `id` в сам документ:
```json
{ "index": { "table": "products", "_id": "1" } }
{ "title": "Crossbody Bag with Tassel", "price": 19.85 }
```

или

```json
{ "index": { "table": "products" } }
{ "title": "Crossbody Bag with Tassel", "price": 19.85, "id": "1" }
```

Для RT-таблицы, объявленной с `id uuid`, `/bulk` считывает UUID из `id`. Эндпоинт `/_bulk` считывает их из метаданных `_id` или поля `id` документа. В обоих эндпоинтах ID можно опустить, чтобы UUID был сгенерирован автоматически.

#### Передача по частям в /bulk
Эндпоинт `/bulk` (режим Manticore) поддерживает [передачу по частям](https://en.wikipedia.org/wiki/Chunked_transfer_encoding). Её можно использовать для передачи крупных пакетов. Она:
* снижает пиковое потребление оперативной памяти и риск её нехватки (OOM)
* сокращает время ответа
* позволяет обойти [max_packet_size](../../Server_settings/Searchd.md#max_packet_size) и передавать пакеты значительно больше максимально допустимого значения `max_packet_size` (128 МБ), например по 1 ГБ.

### Массовый импорт

Для ускорения загрузки крупных пакетов в локальную таблицу реального времени Manticore Search может записывать строки `INSERT` непосредственно в дисковый чанк и делать его доступным при фиксации транзакции. Это позволяет обойтись без предварительного формирования пакета в чанке оперативной памяти. Строки остаются невидимыми до публикации чанка, а сбой операции или `ROLLBACK` оставляет таблицу без изменений.

Поддерживаются таблицы как с построчным, так и с колоночным хранением, включая полнотекстовые поля, числовые атрибуты, строки, JSON, MVA/MVA64 и векторы чисел с плавающей точкой с KNN-индексами.

Имена таблиц здесь нечувствительны к регистру. Например, `SET bulk_import=Products` и `bulk_import=products` выбирают таблицу, сохранённую как `products`; различить таблицы, имена которых отличаются только регистром букв, невозможно.

#### SQL

Включите режим для текущего SQL-сеанса, выполните одну или несколько команд `INSERT` для выбранной таблицы и зафиксируйте изменения:

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

Выполните `SET bulk_import=<table>` до `BEGIN` и до любой незафиксированной записи в этом соединении. Команда резервирует выбранную таблицу за сеансом: поиск остаётся доступен, а другой массовый импорт или обычная запись в ту же таблицу отклоняются до снятия резервирования. `BEGIN` и `START TRANSACTION` допускаются, но не обязательны и в этом режиме ни на что не влияют.

`COMMIT` публикует строки, накопленные после предыдущего `COMMIT` или `ROLLBACK`, как один дисковый чанк. `ROLLBACK` отбрасывает эти строки. В обоих случаях режим массового импорта остаётся включённым, и можно начать следующий пакет. Выполните `SET bulk_import=0`, чтобы отбросить незафиксированные строки, отключить массовый импорт и снять резервирование таблицы этим соединением. После снятия резервирования запись в таблицу возобновляется. Закрытие соединения даёт тот же результат. Пока массовый импорт включён, изменить режим автоматической фиксации нельзя.

#### HTTP `/bulk`

Добавьте `bulk_import=<table>` в запрос к `/bulk` Manticore. Тело запроса остаётся стандартным JSON с разделением строками (NDJSON), а каждая операция должна быть `insert` или `create` для выбранной таблицы:

```bash
printf '%s\n' \
  '{"insert":{"table":"products","id":101,"doc":{"title":"Crossbody Bag with Tassel","price":19.85}}}' \
  '{"insert":{"table":"products","id":102,"doc":{"title":"Microfiber Sheet Set","price":19.99}}}' |
curl -sS -X POST \
  -H 'Content-Type: application/x-ndjson' \
  --data-binary @- \
  'http://localhost:9308/bulk?bulk_import=products'
```

Строка также может содержать документ без обёртки операции: таблица берётся из `bulk_import=<table>`, а `id` является одним из полей:

```bash
printf '%s\n' \
  '{"id":101,"title":"Crossbody Bag with Tassel","price":19.85}' \
  '{"id":102,"title":"Microfiber Sheet Set","price":19.99}' |
curl -sS -X POST \
  -H 'Content-Type: application/x-ndjson' \
  --data-binary @- \
  'http://localhost:9308/bulk?bulk_import=products'
```

Каждое поле документа без обёртки считается столбцом, включая поле с именем `doc`; документ без `id` получает [автоматически сгенерированный ID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID). Строка считается операцией, если её единственное поле названо по имени операции `/bulk` (`insert`, `create`, `index`, `replace`, `update` или `delete`) и содержит объект с `table` или `index`; любая другая строка считается документом без обёртки. Оба формата можно смешивать в одном запросе. Запрос `/bulk` без `bulk_import` по-прежнему требует формат с обёрткой операции.

В теле запроса пустая строка завершает и публикует текущий пакет; конец запроса публикует последний пакет. Manticore отправляет HTTP-ответ после обработки всех пакетов запроса. Если один из последующих пакетов завершится ошибкой, пакеты, опубликованные ранее в том же запросе, останутся доступны для поиска. Чтобы опубликовать весь запрос атомарно как один дисковый чанк, не добавляйте пустые строки. Ответ содержит один сводный результат `bulk` для каждого опубликованного пакета, а не отдельный результат для каждого документа.

При обычном импорте отправляйте всё тело NDJSON одним запросом, как показано выше. После завершения импорта закройте HTTP-соединение, чтобы разрешить другие операции записи в таблицу.

Эндпоинт поддерживает передачу по частям, поэтому может обрабатывать тела запросов размером больше `max_packet_size`, не буферизуя запрос целиком.

#### Совместный массовый импорт по HTTP

Добавьте `shared` рядом с `bulk_import=<table>`, если в одну таблицу поступает множество параллельных запросов, например от сборщика журналов, который отправляет каждую порцию данных через новое соединение. Все такие запросы направляют данные в общий для таблицы пул индексаторов. Пул не привязан к соединению: он сохраняется между запросами, соединениями и переподключениями, пока не будет зафиксирован, отменён через `PURGE`, пока таблица не будет удалена или демон не остановится.

```bash
curl -sS -X POST -H 'Content-Type: application/x-ndjson' --data-binary @part1.ndjson \
  'http://localhost:9308/bulk?bulk_import=products&shared=4'
```

Каждая строка должна быть операцией `insert` или `create` для целевой таблицы либо документом без обёртки, как описано выше. Первый документ создаёт пул и запускает его индексаторы. Успешный ответ на запрос с данными (`"shared_state":"staged"`) означает, что документы подготовлены, но ещё недоступны для поиска: ни конец запроса, ни закрытие соединения, ни границы частей HTTP не публикуют данные.

С первого документа и до фиксации, сбоя или отмены пул резервирует таблицу так же, как отдельный массовый импорт: поиск продолжает работать, а обычные операции записи (`INSERT`, `REPLACE`, `UPDATE`, `DELETE`) и другой массовый импорт в ту же таблицу завершаются ошибкой о блокировке таблицы. Закрытие соединений не освобождает таблицу. [`SHOW LOCKS`](../../Securing_and_compacting_a_table/Freezing_and_locking_a_table.md#SHOW-LOCKS) показывает зарезервированную таблицу с типом блокировки `write_allow_readers`. Первый документ нового пула отклоняется с `shared_precondition` и той же ошибкой, если таблица занята другим массовым импортом, заблокирована через `LOCK TABLES … READ` или в неё выполняется обычная запись.

Запрос без документов, содержащий пустую строку, фиксирует пул:

```bash
printf '\n' | curl -sS -X POST -H 'Content-Type: application/x-ndjson' --data-binary @- \
  'http://localhost:9308/bulk?bulk_import=products&shared'
```

При фиксации пул закрывается для новых данных, индексаторы, не получившие данных, останавливаются, а остальные завершают построение своих частей. Затем присоединяется по одному дисковому чанку от каждого такого индексатора, пул удаляется и возвращается `"shared_state":"published"`. Если другие запросы с данными ещё выполняются, фиксация возвращает HTTP `409` с `shared_busy`, а пул остаётся открытым; повторите фиксацию после завершения записи. Фиксация без пула возвращает `409` с `shared_no_pool`, поскольку сама фиксация никогда не создаёт пул; пул, не получивший данных, не публикует ни одного чанка. Пустые строки в запросе с документами, включая строки из одних пробелов, игнорируются, поэтому запрос с данными никогда не публикует их. Строку завершают как LF, так и CRLF.

Число индексаторов, а значит, и новых дисковых чанков задаётся параметром `shared=N` в запросе, создающем пул. Если указан только `shared`, демон использует переменную окружения `INDEXER_RT_BULK_SHARED_POOL_SIZE` процесса `searchd`, а если она не задана — один индексатор. N ограничено диапазоном от 1 до числа рабочих [потоков](../../Server_settings/Searchd.md#threads) минус 2 (не менее 1): меньшее или нечисловое значение даёт один индексатор, большее — максимально допустимое число. N считывает только запрос, создающий пул; запросы, присоединяющиеся к открытому пулу, и фиксация его игнорируют. Данные распределяются между индексаторами по очереди пакетами примерно по 32 КиБ. Увеличение числа индексаторов ускоряет построение после фиксации, но повышает нагрузку на процессор и память при загрузке. Для начала подойдёт около двух третей числа рабочих потоков, например 8 индексаторов при 12 потоках; при большем числе индексаторы конкурируют с входящими запросами за процессор и диск, замедляя загрузку. Каждому индексатору при загрузке требуется примерно 200 МБ оперативной памяти.

Каждый ответ представляет собой один JSON-объект с полями `errors`, `error`, `error_code` и `shared_state`:

| `shared_state` | HTTP | `error_code` | Значение |
| --- | --- | --- | --- |
| `staged` | 200 | | данные находятся в пуле |
| `published` | 200 | | пул зафиксирован |
| `busy` | 409 | `shared_busy` | фиксация во время выполнения других запросов с данными |
| `closing` | 409 | `shared_closing` | данные пула публикуются |
| `no_pool` | 409 | `shared_no_pool` | фиксация или запрос с данными и `shared_join` при отсутствии пула |
| `rejected` | 400 или 403 при отсутствии права на запись | `shared_precondition` | некорректный запрос, например нет права на запись или таблица заблокирована при создании пула |
| `failed` | 400, 409 или 500 | `shared_failed` | загрузка завершилась ошибкой, см. ниже |

`current_line` подсчитывает документы запроса; при фиксации возвращается `1`. При ошибке `skipped_lines` равно `current_line`, хотя пакеты запроса, уже переданные индексаторам, остаются в пуле.

Если соединение обрывается или истекает время ожидания посреди передачи тела запроса либо запрос завершается командой `KILL`, сбой затрагивает только этот запрос: его последняя незаписанная часть (менее 32 КиБ) отбрасывается, уже переданные индексаторам данные сохраняются, а пул остаётся открытым, поэтому клиент может отправить запрос повторно. Некорректный документ (неверный JSON, неизвестный столбец, другая таблица, операция, отличная от `insert`/`create`) или ошибка записи в индексатор приводит к сбою всей загрузки: индексаторы останавливаются, таблица сразу освобождается, этот запрос получает `400` при некорректном документе или `500` при ошибке записи, все последующие запросы получают `409` с первой ошибкой, а следующая фиксация возвращает эту ошибку с `500` и удаляет сбойный пул вместе с временными файлами; `PURGE … OPTION clear_sessions=1` также удаляет его. Только после этого можно начать новую загрузку.

Сбой отдельного запроса и сбой всей загрузки дают одинаковый ответ: `failed` с `shared_failed` (`400` или `500` для принудительно завершённого запроса), поэтому по одному ответу их не различить. Разницу покажет следующий запрос: `staged` означает, что пул всё ещё открыт, а `409` с `shared_failed` — что загрузка завершилась ошибкой.

Пул существует только в памяти `searchd`. Если демон остановится или аварийно завершится во время загрузки либо пул будет отменён, подготовленные документы будут потеряны, а следующий запрос с данными создаст новый пул, содержащий только последующие документы. Чтобы обнаружить такую ситуацию, добавляйте `shared_join` во все запросы с данными после первого ответа `staged`: такой запрос только присоединяется к открытому пулу и никогда не создаёт новый, поэтому при отсутствии пула он вернёт `409` с `shared_no_pool`, и загрузку можно будет повторить с начала.

Завершение работы клиентов не завершает работу пула. Если клиенты исчезли до фиксации, например сборщик журналов был остановлен или аварийно завершился, пул остаётся открытым и удерживает таблицу, поэтому обычные операции записи продолжают завершаться ошибкой. Освободить таблицу можно двумя способами: зафиксировать пул, как показано выше, опубликовав уже подготовленные документы, или отменить его, отбросив их:

```sql
PURGE BULK_IMPORT FROM TABLE products OPTION clear_sessions=1;
```

Отмена останавливает индексаторы, освобождает таблицу и удаляет временные файлы; ранее опубликованные чанки остаются в таблице. Пока запрос с данными от исчезнувшего клиента всё ещё считается выполняющимся, фиксация возвращает `409` с `shared_busy` до истечения времени ожидания этого запроса; отмена его не ждёт. После освобождения таблицы `SHOW LOCKS` больше её не показывает, и можно начать новую совместную загрузку. Подробнее см. в разделе [`PURGE BULK_IMPORT`](../../Node_info_and_management/PURGE.md#PURGE-BULK_IMPORT).

#### Elasticsearch `/_bulk`

Совместимый с Elasticsearch эндпоинт `/_bulk` не поддерживает `bulk_import` с записью напрямую на диск; используйте SQL или `/bulk` Manticore.

#### Повторяющиеся ID документов

В пределах одного пакета с записью напрямую на диск первая строка с данным числовым ID документа остаётся видимой, а последующие строки с тем же ID логически удаляются. В SQL пакет состоит из строк, подготовленных до `COMMIT` или `ROLLBACK`. В `/bulk` Manticore каждая группа, опубликованная при пустой строке, смене таблицы или достижении конца запроса, является отдельным пакетом. При совместной загрузке данные каждого индексатора составляют отдельный пакет, а их чанки присоединяются последовательно при фиксации пула. Если присоединить один из них не удалось, ранее присоединённые чанки остаются в таблице, а фиксация возвращает `500`; повторная загрузка тех же документов безопасна при явно заданных ID, поскольку опубликованная строка заменяет любую строку с тем же ID, а документы с автоматически сгенерированными ID добавляются повторно.

Когда Manticore публикует дисковый чанк в целевой таблице, строка из этого чанка заменяет любую существующую строку с тем же ID. Это касается и ID, опубликованных предыдущим HTTP-пакетом. Поэтому повторная отправка ранее опубликованного пакета заменяет его строки, а при повторяющихся ID внутри самого пакета по-прежнему сохраняется первая строка. В запросе `/bulk` Manticore операция `create`, например:

```json
{"create":{"table":"products",...}}
```

работает как `insert`; само по себе наличие такого ID не приводит к ошибке.

#### Использование памяти и диска

Загрузка с записью напрямую на диск не накапливает документы в памяти `searchd`. Каждая команда `INSERT` или каждая часть тела запроса `/bulk` (около 32 КиБ при наличии параметра `shared`) преобразуется в CSV и сразу передаётся процессу `indexer` через канал. `indexer` строит чанк так же, как обычную таблицу: накапливает данные в памяти до [mem_limit](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#mem_limit), фиксированного на значении по умолчанию 128 МБ, сбрасывает их во временные файлы и после последней строки объединяет их в дисковый чанк. Поэтому потребление памяти не растёт с объёмом загрузки: каждому процессу `indexer` требуется память под `mem_limit`, [буферы записи](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#write_buffer) и сам процесс — всего примерно 200 МБ. Сеанс `SET bulk_import` или запрос `/bulk` запускает один `indexer`, загрузка с `shared=N` — N процессов; дополнительно учитывайте обрабатываемую команду или часть запроса.

Как и для обычной таблицы (см. [inplace_enable](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#inplace_enable)), промежуточные данные во временных файлах сопоставимы по размеру с готовым чанком. И те и другие записываются в подготовительный каталог таблицы рядом с её файлами, поэтому на пике диску с таблицей требуется свободное место примерно под удвоенный размер нового чанка. При публикации файлы чанка перемещаются в таблицу переименованием, без копирования. См. раздел [Временные файлы и очистка](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Staging-files-and-cleanup).

#### Временные файлы и очистка

Manticore удаляет текущий подготовительный каталог после штатного `COMMIT`, `ROLLBACK`, отключения режима или закрытия сеанса. Общий пул сохраняет свои подготовительные каталоги до фиксации или отмены; закрытие соединений их не удаляет. После аварийного завершения демона или сбоя хоста может остаться неиспользуемый подготовительный каталог. Следующая загрузка напрямую на диск создаёт новый каталог с уникальным именем и не использует и не присоединяет файлы, оставшиеся от аварийно прерванной загрузки.

Чтобы просмотреть все подготовительные файлы и каталоги загрузки напрямую на диск для таблицы, выполните следующий запрос:

```sql
SELECT file, normalized, size
FROM products.@files
OPTION format='bulk_import';
```

Результат рекурсивно перечисляет файлы и каталоги внутри корневого подготовительного каталога таблицы для загрузки напрямую на диск. Для каталогов и других объектов, не являющихся обычными файлами, указывается размер `0`.

Убедившись, что для таблицы не выполняется загрузка напрямую на диск, удалите весь её корневой подготовительный каталог с помощью [`PURGE BULK_IMPORT`](../../Node_info_and_management/PURGE.md#PURGE-BULK_IMPORT):

```sql
PURGE BULK_IMPORT FROM TABLE products;
```

Пока у таблицы есть открытый общий пул, `PURGE` отклоняется; команда `PURGE BULK_IMPORT FROM TABLE products OPTION clear_sessions=1` сначала отменяет пул, останавливает его индексаторы, освобождает таблицу, а затем удаляет корневой подготовительный каталог. Пока пул присоединяет чанки, `PURGE` отклоняется; повторите команду после завершения фиксации. В Windows команда `PURGE … OPTION clear_sessions=1` или `DROP TABLE`, выполненная во время записи запроса с данными в пул, может вернуть `failed to remove bulk staging root`: пул уже отменён, но его остановленные индексаторы ещё некоторое время держат файлы открытыми; повторите команду.

Для `PURGE` требуется существующая локальная таблица реального времени, не входящая в кластер репликации. Команда удаляет только подготовительные данные загрузки напрямую на диск и не меняет схему таблицы или проиндексированные строки. Если корневого подготовительного каталога нет, команда успешно завершается без изменений. В режиме без конфигурационного файла `DROP TABLE` также удаляет корневой подготовительный каталог таблицы и отменяет её общий пул; завершение работы демона отменяет все общие пулы. Ранее опубликованные чанки остаются в таблице.

#### Текущие ограничения

* Целью должна быть одна существующая незамороженная локальная таблица реального времени, не входящая в кластер репликации. Распределённые, шардированные, реплицируемые, обычные таблицы и таблицы percolate не поддерживаются.
* В SQL поддерживается только `INSERT`. Эндпоинт `/bulk` Manticore принимает `insert`, `create` и документы без обёртки; `index`, `replace`, `update` и `delete` отклоняются.
* ID документа преобразуется так же, как при обычном `INSERT`, с учётом [attr_autoconv_strict](../../Server_settings/Searchd.md#attr_autoconv_strict). Строка без ID или с ID, преобразующимся в 0, получает [автоматически сгенерированный ID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID); ответ не содержит сгенерированные ID, а повторная загрузка добавляет такие строки снова. UUID в качестве ID документов не поддерживаются.
* Статические сборки не поддерживаются.
* Исполняемый файл для соответствующей платформы (`indexer` в Linux, `indexer.exe` в Windows) должен находиться в том же каталоге, что и запущенный исполняемый файл `searchd`. Manticore Search проверяет только этот путь и не ищет файл в `PATH`. Для совместимости используйте исполняемый файл из той же установки, что и `searchd`. Если файл отсутствует, недоступен для чтения или не запускается, не работает только массовый импорт; обычный запуск и стандартная вставка остаются доступны.

Загрузчик с вспомогательным процессом использует источник `csvpipe`. По умолчанию его команда — `/bin/cat` в Linux и `-` в Windows, где `-` означает чтение из стандартного ввода индексатора. Задайте `INDEXER_RT_BULK_CSV_PIPE_COMMAND` в окружении `searchd`, чтобы переопределить только эту команду. Переопределение не меняет источник или способ подачи данных; команда должна работать с механизмом ввода данной платформы и передавать строки CSV индексатору. Неподдерживаемая команда приводит к сбою загрузки с вспомогательным процессом.

<!-- intro -->
### Примеры массовой вставки
##### SQL:
<!-- request SQL -->
Для массовой вставки укажите дополнительные документы в скобках после `VALUES()`. Синтаксис:

```sql
INSERT INTO <table name>[(column1, column2, ...)] VALUES(value1[, value2 , ...]), (...)
```

Необязательный список имён столбцов позволяет явно задать значения для части столбцов таблицы. Остальные столбцы получат значения по умолчанию (0 для скалярных типов, пустая строка для строковых типов).

Например:

```sql
INSERT INTO products(title,price) VALUES ('Crossbody Bag with Tassel', 19.85), ('microfiber sheet set', 19.99), ('Pet Hair Remover Glove', 7.99);
```
<!-- response SQL -->

```sql
Query OK, 3 rows affected (0.01 sec)
```

Выражения в `INSERT` пока не поддерживаются, значения необходимо указывать явно.

<!-- intro -->
##### JSON:
<!-- request JSON -->
Синтаксис в целом такой же, как при [вставке одного документа](../../Quick_start_guide.md#Add-documents). Добавьте строки — по одной на документ — и используйте эндпоинт `/bulk` вместо `/insert`. Поместите каждый документ в узел "insert". Также требуется следующее:
* `Content-Type: application/x-ndjson`
* Данные должны быть в формате JSON с разделением строками (NDJSON). Это означает, что каждая строка должна содержать ровно одно JSON-выражение и заканчиваться символом перевода строки `\n`, возможно, вместе с `\r`.

Эндпоинт `/bulk` поддерживает запросы 'insert', 'replace', 'delete' и 'update'. Операции можно направлять в разные таблицы, но одна транзакция может охватывать только одну таблицу. Если указано несколько таблиц, Manticore объединяет операции для одной таблицы в одну транзакцию. При смене таблицы он фиксирует накопленные операции и начинает новую транзакцию. Пустая строка между пакетами также фиксирует предыдущий пакет и начинает новую транзакцию. Запрос без операций, с пустым телом или только с пустыми строками либо строками из пробельных символов, успешно завершается без изменений и возвращает пустой массив `items`.

Ответ на запрос `/bulk` содержит следующие поля:
* "errors": показывает, возникли ли ошибки (true/false)
* "error": описывает возникшую ошибку
* "current_line": номер строки, на которой выполнение остановилось или завершилось ошибкой; пустые строки, включая первую, тоже учитываются
* "skipped_lines": число незафиксированных строк, считая от `current_line` назад

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

Для реплицируемых таблиц добавьте свойство `cluster` в каждую операцию:

```json
POST /bulk
-H "Content-Type: application/x-ndjson" -d '
{"insert": {"cluster":"posts", "table":"weekly_table", "id":1, "doc":  {"title":"Crossbody Bag with Tassel","price":19.85}}}
{"insert": {"cluster":"posts", "table":"weekly_table", "id":2, "doc":  {"title":"microfiber sheet set","price":19.99}}}
{"insert": {"cluster":"posts", "table":"weekly_table", "id":3, "doc":  {"title":"Pet Hair Remover Glove","price":7.99}}}
'
```

<!-- request Elasticsearch -->

> ПРИМЕЧАНИЕ: для `_bulk` требуется [Manticore Buddy](Installation/Manticore_Buddy.md), если таблица ещё не существует. Если он не работает, убедитесь, что Buddy установлен.

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

Для реплицируемых таблиц включите имя кластера в имя таблицы в формате `cluster_name:table_name`:

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
Используйте метод addDocuments():

```php
$index->addDocuments([
        ['id' => 1, 'title' => 'Crossbody Bag with Tassel', 'price' => 19.85],
        ['id' => 2, 'title' => 'microfiber sheet set', 'price' => 19.99],
        ['id' => 3, 'title' => 'Pet Hair Remover Glove', 'price' => 7.99]
]);
```

Для реплицируемых таблиц задайте имя кластера перед добавлением документов:

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

Для реплицируемых таблиц добавьте свойство cluster в каждый документ:

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

Для реплицируемых таблиц добавьте свойство cluster в каждый документ:

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

Для реплицируемых таблиц добавьте свойство cluster в каждый документ:

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

Для реплицируемых таблиц добавьте свойство cluster в каждый документ:

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

Для реплицируемых таблиц добавьте свойство cluster в каждый документ:

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
## Вставка значений многозначных атрибутов (MVA)

Значения многозначных атрибутов (MVA) вставляются как массивы чисел.
<!-- intro -->
### Примеры
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

Или другой вариант
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
## Вставка JSON
Значение JSON можно вставить как [экранированную](../../Searching/Full_text_matching/Escaping.md) строку (через SQL или JSON) либо как JSON-объект (через JSON-интерфейс).

<!-- intro -->
### Примеры
##### SQL
<!-- request SQL -->
```

INSERT INTO products VALUES (1, 'shoes', '{"size": 41, "color": "red"}');
```
<!-- intro -->
##### JSON
<!-- request JSON -->
Значение JSON можно вставить как JSON-объект
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

Значение JSON также можно вставить как строку, содержащую экранированный JSON:
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

Или другой вариант
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
Передайте JSON как обычную строку:
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
