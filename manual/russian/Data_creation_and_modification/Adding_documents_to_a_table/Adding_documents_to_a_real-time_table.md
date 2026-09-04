# Добавление документов в таблицу реального времени

> Если вам нужна информация о добавлении документов в обычную таблицу, см. раздел [добавление данных из внешних хранилищ](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md).

<!-- example insert -->
Добавление документов в реальном времени поддерживается только для таблиц [Real-Time](../../Creating_a_table/Local_tables/Real-time_table.md) и [percolate](../../Creating_a_table/Local_tables/Percolate_table.md). Соответствующая команда SQL, HTTP-эндпоинт или функции клиента вставляют новые строки (документы) в таблицу с указанными значениями полей. Таблица не обязана существовать до добавления в нее документов. Если таблицы нет, Manticore попытается создать ее автоматически. Подробнее см. [Auto schema](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema).

Можно вставить один или [несколько документов](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-adding-documents) со значениями для всех полей таблицы или только для их части. В этом случае остальные поля будут заполнены значениями по умолчанию (0 для скалярных типов, пустая строка для текстовых типов).

Выражения в `INSERT` сейчас не поддерживаются, поэтому значения нужно задавать явно.

Поле/значение `ID` можно опустить, поскольку таблицы RT и PQ поддерживают функцию [auto-id](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID). Для таблиц с числовым ID можно также использовать значение `0` для автоматической генерации ID. Строки с дублирующимися ID не будут перезаписаны через `INSERT`. Вместо этого для такой задачи используйте [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md).

Для таблиц, созданных с [`id uuid`](../../Creating_a_table/Data_types.md#UUID-document-IDs), передавайте явный UUID в кавычках или опускайте `id`, чтобы сгенерировать его автоматически. Явные значения должны соответствовать шаблону `xxxxxxxx-xxxx-Vxxx-Wxxx-xxxxxxxxxxxx`, где каждый `x` - шестнадцатеричная цифра, `V` - версия (`1` - `8`), а `W` - вариант (`8`, `9`, `a` или `b`). Заглавные шестнадцатеричные буквы допускаются и приводятся к нижнему регистру. В отличие от числовых ID, значение `0` не запрашивает автоматическую генерацию UUID.

При использовании HTTP JSON-протокола можно выбрать один из двух форматов запросов: обычный формат Manticore и формат, похожий на Elasticsearch. Оба формата показаны в примерах ниже.

Кроме того, при использовании формата запроса Manticore JSON помните, что узел `doc` обязателен, и все значения должны передаваться внутри него.


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

Для таблиц, созданных с `id uuid`, передавайте JSON `id` как строку UUID или опускайте его, чтобы сгенерировать значение автоматически:

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

> ПРИМЕЧАНИЕ: для `_create` требуется [Manticore Buddy](Installation/Manticore_Buddy.md). Если это не работает, убедитесь, что Buddy установлен.

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

Для реплицируемых таблиц включайте свойство cluster в каждый документ:

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

Для реплицируемых таблиц включайте свойство cluster в каждый документ:

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

Для реплицируемых таблиц включайте свойство cluster в каждый документ:

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

Для реплицируемых таблиц включайте свойство cluster в каждый документ:

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

Для реплицируемых таблиц включайте свойство cluster в каждый документ:

``` clike
string body = "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 1, \"doc\" : {\"title\" : \"Crossbody Bag with Tassel\", \"price\" : 19.85}}}"+"\n"+
    "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 4, \"doc\" : {\"title\" : \"microfiber sheet set\", \"price\" : 19.99}}}"+"\n"+
    "{\"insert\": {\"cluster\" : \"posts\", \"table\" : \"weekly_table\", \"id\" : 5, \"doc\" : {\"title\" : \"Pet Hair Remover Glove\", \"price\" : 7.99}}}"+"\n";
BulkResponse bulkresult = indexApi.Bulk(string.Join("\n", docs));
```

<!-- end -->

## Добавление документов в реплицируемые таблицы
<!-- example replicated_insert -->
При работе с [реплицируемыми таблицами](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md) нужно использовать специальный синтаксис, чтобы операции записи корректно распространялись на все узлы кластера.

Для всех операций записи (INSERT, REPLACE, DELETE, TRUNCATE, UPDATE) в реплицируемые таблицы нужно:
* В SQL: использовать формат `cluster_name:table_name` вместо одного только имени таблицы
* В JSON: указывать свойство `cluster` вместе со свойством `table`

Если использовать неверный синтаксис, операция завершится ошибкой.

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

> ПРИМЕЧАНИЕ: Auto schema требует [Manticore Buddy](Installation/Manticore_Buddy.md). Если это не работает, убедитесь, что Buddy установлен.

В Manticore есть механизм автоматического создания таблиц, который срабатывает, когда указанная в запросе insert или replace таблица еще не существует. По умолчанию этот механизм включен. Чтобы отключить его, задайте `auto_schema = 0` в разделе [Searchd](../../Server_settings/Searchd.md#auto_schema) файла конфигурации Manticore.

<!-- example auto-schema -->

По умолчанию все текстовые значения в предложении `VALUES` считаются значениями типа `text`, за исключением значений, представляющих корректные адреса электронной почты, которые обрабатываются как тип `string`.

Если при `INSERT`/`REPLACE` нескольких строк для одного и того же поля указаны разные, несовместимые типы значений, автоматическое создание таблицы будет отменено и вернется сообщение об ошибке. Однако если типы совместимы, итоговый тип поля будет выбран так, чтобы он подходил для всех значений. Возможные автоматические преобразования типов данных включают:
* mva -> mva64
* uint -> bigint -> float (это может привести к некоторой потере точности)
* string -> text

Механизм auto schema не поддерживает создание таблиц с векторными полями (полями типа `float_vector`), которые используются для [KNN](../../Searching/KNN.md#Configuring-a-table-for-KNN-search) поиска сходства (K-Nearest Neighbors). Чтобы использовать векторные поля в таблице, нужно явно создать таблицу со схемой, в которой эти поля определены. Если нужно хранить векторные данные в обычной таблице без поддержки KNN-поиска, их можно сохранить как JSON-массив с использованием стандартного JSON-синтаксиса, например: `INSERT INTO table_name (vector_field) VALUES ('[1.0, 2.0, 3.0]')`.

Кроме того, будут распознаны и преобразованы в метки времени следующие форматы дат, а все остальные форматы дат будут обработаны как строки:
- `%Y-%m-%dT%H:%M:%E*S%Z`
- `%Y-%m-%d'T'%H:%M:%S%Z`
- `%Y-%m-%dT%H:%M:%E*S`
- `%Y-%m-%dT%H:%M:%s`
- `%Y-%m-%dT%H:%M`
- `%Y-%m-%dT%H`


Имейте в виду, что HTTP-эндпоинт `/bulk` не поддерживает автоматическое создание таблиц (auto schema). Эту возможность поддерживают только эндпоинт `/_bulk` (в стиле Elasticsearch) и SQL-интерфейс.

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
Manticore автоматически генерирует ID для документов, вставляемых или заменяемых в таблице реального времени или [таблице Percolate](../../Creating_a_table/Local_tables/Percolate_table.md). Генератор создает уникальное числовое значение с указанными ниже гарантиями, но его не следует считать автоинкрементной последовательностью.

Сгенерированное значение ID гарантированно уникально при соблюдении следующих условий:
* Значение [server_id](../../Server_settings/Searchd.md#server_id) текущего сервера находится в диапазоне от 0 до 127 и уникально среди узлов кластера, либо используется значение по умолчанию, сгенерированное на основе MAC-адреса как начального значения
* Системное время на узле Manticore не изменяется между перезапусками сервера
* Средняя скорость генерации auto-ID между двумя запусками сервера остается ниже примерно 16 миллионов ID в секунду

Генератор auto ID создает 64-битное целое число со следующим расположением битов:
* Биты 0 - 23 - это счетчик, который увеличивается при каждом вызове генератора auto ID
* Биты 24 - 55 хранят время запуска сервера в секундах, закодированное как `(unix_timestamp_at_start - 2019-05-01 00:00:00 UTC)`
* Биты 56 - 62 хранят `server_id` (значение маскируется в диапазон 0..127)

Такое расположение гарантирует уникальность сгенерированных ID между узлами кластера и исключает коллизии данных, вставленных в разные узлы кластера. Это особенно важно при работе с реплицируемыми таблицами, поскольку оно гарантирует уникальность автоматически сгенерированных ID на всех узлах кластера репликации.

Важно: 24-битный счетчик не является жестким пределом общего числа документов, которые можно вставить за один запуск сервера. После запуска можно вставить более 16 777 216 документов; ID при этом будут продолжать расти и останутся уникальными для этого процесса. Правило про `~16 million IDs per second` важно для уникальности между перезапусками: после перезапуска временная часть должна сдвинуться достаточно далеко, чтобы вновь сгенерированные ID не пересекались с ID, созданными до перезапуска.

Из-за этого первый ID, который генерируется для auto ID, - НЕ 1, а большее число. Кроме того, поток документов, вставляемых в таблицу, может содержать не последовательные значения ID, если между вызовами выполняются вставки в другие таблицы, поскольку генератор ID в сервере единственный и общий для всех его таблиц.

Для таблиц с числовым ID это целочисленный публичный ID документа. Для таблиц с UUID-ID Manticore кодирует его в каноническую строку UUIDv8; клиент видит только UUID.

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
### Генерация нескольких ID через UUID_SHORT

```sql
CALL UUID_SHORT(N)
```

Оператор `CALL UUID_SHORT(N)` позволяет сгенерировать N уникальных 64-битных ID за один вызов без вставки каких-либо документов. Это особенно полезно, когда нужно заранее сгенерировать ID в Manticore для использования в других системах или хранилищах. Например, можно сгенерировать auto-ID в Manticore, а затем использовать их в другой базе данных, приложении или рабочем процессе, обеспечив согласованные и уникальные идентификаторы в разных средах.

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
В таблицу реального времени можно вставлять не только один документ, но и сколько угодно. Вполне нормально загружать в такую таблицу партии по десятки тысяч документов. Однако важно учитывать следующее:
* Чем больше партия, тем выше задержка каждой операции вставки
* Чем больше партия, тем выше ожидаемая скорость индексации
* Возможно, стоит увеличить значение [max_packet_size](../../Server_settings/Searchd.md#max_packet_size), чтобы разрешить более крупные партии
* Обычно каждая операция пакетной вставки считается одной [транзакцией](../../Data_creation_and_modification/Transactions.md) с гарантией атомарности, поэтому в таблице либо сразу окажутся все новые документы, либо в случае ошибки не будет добавлен ни один. Подробнее об пустой строке или переключении на другую таблицу см. в примере "JSON".

Обратите внимание, что HTTP-эндпоинт `/bulk` не поддерживает автоматическое создание таблиц (auto schema). Эту возможность поддерживают только эндпоинт `/_bulk` (в стиле Elasticsearch) и SQL-интерфейс. HTTP-эндпоинт `/_bulk` (в стиле Elasticsearch) позволяет включать имя кластера в имя таблицы в формате `cluster_name:table_name`.

Эндпоинт `/_bulk` принимает ID документов в том же формате, что и Elasticsearch, и вы также можете указать `id` внутри самого документа:
```json
{ "index": { "table": "products", "_id": "1" } }
{ "title": "Crossbody Bag with Tassel", "price": 19.85 }
```

или

```json
{ "index": { "table": "products" } }
{ "title": "Crossbody Bag with Tassel", "price": 19.85, "id": "1" }
```

Для RT-таблицы, объявленной с `id uuid`, `/bulk` читает UUID из `id`. `/_bulk` читает их из метаданных `_id` или из `id` документа. Оба эндпоинта могут обойтись без ID, чтобы сгенерировать UUID автоматически.

#### Потоковая передача по частям в /bulk
Эндпоинт `/bulk` (режим Manticore) поддерживает [потоковую передачу с разбиением на части](https://en.wikipedia.org/wiki/Chunked_transfer_encoding). Ее можно использовать для передачи больших пакетов. Это:
* уменьшает пиковое потребление RAM, снижая риск OOM
* сокращает время ответа
* позволяет обойти [max_packet_size](../../Server_settings/Searchd.md#max_packet_size) и передавать партии значительно больше максимального разрешенного значения `max_packet_size` (128MB), например по 1GB за раз.

### Пакетный импорт

Для более быстрой загрузки больших пакетов в локальную таблицу реального времени Manticore Search может записывать строки `INSERT` напрямую в дисковый chunk и публиковать этот chunk при фиксации транзакции. Это позволяет не создавать сначала пакет в RAM chunk. Строки остаются невидимыми, пока chunk не опубликован, а при сбое операции или `ROLLBACK` таблица не изменяется.

Поддерживаются как строчные, так и колонночные таблицы, включая полнотекстовые поля, числовые атрибуты, строки, JSON, MVA/MVA64 и float-векторы с индексами KNN.

Имена таблиц здесь не зависят от регистра. Например, `SET bulk_import=Products` и `bulk_import=products` обе выбирают таблицу, хранящуюся как `products`; таблицы, имена которых различаются только регистром букв, неотличимы.

#### SQL

Включите режим для текущей SQL-сессии, выполните один или несколько операторов `INSERT` для привязанной таблицы и зафиксируйте изменения:

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

Выполните `SET bulk_import=<table>` перед `BEGIN` и до любой незавершенной операции записи в этом соединении. Это закрепляет выбранную таблицу за сессией: поиск по ней остается доступным, а другой пакетный импорт или обычная запись в ту же таблицу будут отклонены, пока резерв не будет снят. `BEGIN` и `START TRANSACTION` разрешены, но не обязательны и в этом режиме не влияют на поведение.

`COMMIT` публикует строки, собранные с момента предыдущего `COMMIT` или `ROLLBACK`, как один disk chunk. `ROLLBACK` отбрасывает эти строки. В обоих случаях bulk import остается включенным, так что можно начать новую партию. Выполните `SET bulk_import=0`, чтобы отбросить все незавершенные строки, отключить bulk import и освободить резерв этого соединения. После снятия резерва записи в таблицу возобновляются. Закрытие соединения делает то же самое. При включенном bulk import изменить autocommit нельзя.

#### HTTP `/bulk`

Добавьте `bulk_import=<table>` в запрос Manticore `/bulk`. Тело запроса по-прежнему остается обычным NDJSON (JSON, разделенный переводами строк), и каждая операция должна быть `insert` или `create` для привязанной таблицы:

```bash
printf '%s\n' \
  '{"insert":{"table":"products","id":101,"doc":{"title":"Crossbody Bag with Tassel","price":19.85}}}' \
  '{"insert":{"table":"products","id":102,"doc":{"title":"Microfiber Sheet Set","price":19.99}}}' |
curl -sS -X POST \
  -H 'Content-Type: application/x-ndjson' \
  --data-binary @- \
  'http://localhost:9308/bulk?bulk_import=products'
```

В теле запроса пустая строка завершает и публикует текущую партию; конец запроса публикует последнюю партию. Manticore отправляет HTTP-ответ после обработки всех партий в этом запросе. Если более поздняя партия завершается ошибкой, партии, опубликованные раньше в том же запросе, остаются доступными для поиска. Чтобы опубликовать весь запрос атомарно как один disk chunk, не добавляйте пустые строки. В ответе содержится один агрегированный результат `bulk` для каждой опубликованной партии, а не по одному результату на каждый документ.

Большинству клиентов следует отправлять один запрос, как в примере выше. Если приложение намеренно отправляет несколько запросов по одному и тому же [постоянному HTTP-соединению](../../Connecting_to_the_server/HTTP.md#Persistent-connections), первый запрос выбирает таблицу. Последующие запросы `/bulk` или `/json/bulk` в этом соединении могут не указывать `bulk_import`, но они должны продолжать запись в ту же таблицу. После завершения работы закройте соединение или отправьте пустой запрос `/bulk?bulk_import=0`, чтобы отключить bulk import и освободить резерв этого соединения. После снятия резерва записи в таблицу возобновляются.

Эндпоинт поддерживает chunked transfer encoding, поэтому может обрабатывать тела больше `max_packet_size` без буферизации всего запроса.

#### Elasticsearch `/_bulk`

Совместимый с Elasticsearch эндпоинт `/_bulk` не поддерживает direct-to-disk `bulk_import`; используйте SQL или Manticore `/bulk`. В чистой сессии `?pipeline=bulk_import` и устаревший `?bulk_import=1` отклоняются. Другие неизвестные значения `bulk_import` игнорируются, и обычная обработка bulk в стиле Elasticsearch продолжается. Соединение с активным bulk import не может переключиться на `/_bulk`.

#### Дублирующиеся ID документов

Внутри одной партии direct-to-disk первая строка для числового ID документа остается видимой, а последующие строки с тем же ID логически удаляются. Для SQL партией считаются строки, подготовленные до `COMMIT` или `ROLLBACK`. Для Manticore `/bulk` отдельной партией считается каждая группа, опубликованная на пустой строке, при смене таблицы или в конце запроса.

Когда Manticore публикует disk chunk в целевую таблицу, строка в этом chunk заменяет любую существующую строку с тем же ID. Это также относится к ID, опубликованным более ранней HTTP-партией. В результате повторная отправка уже опубликованной партии заменяет ее строки, а дубликаты внутри повторно отправленной партии по-прежнему сохраняют первую строку. В запросе Manticore `/bulk` операция `create`, например:

```json
{"create":{"table":"products",...}}
```

ведет себя как `insert`; она не завершается ошибкой только потому, что ID уже существует.

#### Временные файлы и очистка

Manticore удаляет текущий каталог staging после обычного `COMMIT`, `ROLLBACK`, отключения режима или закрытия сессии. Сбой демона или хоста может оставить после себя брошенный каталог staging. Более поздняя загрузка direct-to-disk создает новый каталог с уникальным именем и не переиспользует файлы, оставшиеся после сбоя.

Используйте следующий запрос, чтобы просмотреть все записи staging direct-to-disk для таблицы:

```sql
SELECT file, normalized, size
FROM products.@files
OPTION format='bulk_import';
```

Результат рекурсивно выводит файлы и каталоги под корнем staging direct-to-disk для таблицы. Для каталогов и других не-обычных записей размер указывается как `0`.

После того как вы убедитесь, что для таблицы не выполняется загрузка direct-to-disk, удалите весь ее корень staging с помощью [`PURGE BULK_IMPORT`](../../Node_info_and_management/PURGE.md#PURGE-BULK_IMPORT):

```sql
PURGE BULK_IMPORT FROM TABLE products;
```

`PURGE` требует существующей локальной таблицы реального времени, которая не входит в кластер репликации. Она удаляет только состояние staging direct-to-disk и не меняет схему таблицы или индексированные строки. Если корень staging отсутствует, операция завершается успешно без действий. В режиме без конфигурации `DROP TABLE` также удаляет корень staging direct-to-disk таблицы.

#### Текущие ограничения

* Целью должна быть одна существующая, не замороженная локальная таблица реального времени, не являющаяся участником кластера репликации. Распределенные, шардинговые, реплицируемые, percolate и обычные таблицы не поддерживаются.
* SQL поддерживает только `INSERT`. Manticore `/bulk` принимает `insert` и `create`; `index`, `replace`, `update` и `delete` отклоняются.
* Каждая строка должна содержать явный числовой ненулевой ID документа. Автоматически сгенерированные ID и UUID-ID не поддерживаются.
* Статические сборки не поддерживаются.
* Исполняемый файл, зависящий от платформы (`indexer` в Linux, `indexer.exe` в Windows), должен находиться в той же директории, что и запущенный `searchd`. Manticore Search ищет только соседний путь и не просматривает `PATH`. Используйте исполняемый файл из той же установки, что и `searchd`, чтобы обеспечить совместимость. Если его нет, он недоступен для чтения или не запускается, не работает только bulk import; обычный запуск и стандартная вставка остаются доступны.

<!-- intro -->
### Примеры пакетной вставки
##### SQL:
<!-- request SQL -->
Для пакетной вставки просто укажите больше документов в скобках после `VALUES()`. Синтаксис такой:

```sql
INSERT INTO <table name>[(column1, column2, ...)] VALUES(value1[, value2 , ...]), (...)
```

Необязательный список имен столбцов позволяет явно указать значения для некоторых столбцов, присутствующих в таблице. Все остальные столбцы будут заполнены значениями по умолчанию (0 для скалярных типов, пустая строка для строковых типов).

Например:

```sql
INSERT INTO products(title,price) VALUES ('Crossbody Bag with Tassel', 19.85), ('microfiber sheet set', 19.99), ('Pet Hair Remover Glove', 7.99);
```
<!-- response SQL -->

```sql
Query OK, 3 rows affected (0.01 sec)
```

Выражения в `INSERT` сейчас не поддерживаются, и значения нужно задавать явно.

<!-- intro -->
##### JSON:
<!-- request JSON -->
Синтаксис в целом такой же, как при [вставке одного документа](../../Quick_start_guide.md#Add-documents). Просто укажите больше строк, по одной на каждый документ, и используйте эндпоинт `/bulk` вместо `/insert`. Заключайте каждый документ в узел "insert". Обратите внимание, что также требуется:
* `Content-Type: application/x-ndjson`
* Данные должны быть оформлены как JSON, разделенный переводами строк (NDJSON). По сути это означает, что каждая строка должна содержать ровно один JSON-оператор и заканчиваться переводом строки `\n` и, возможно, `\r`.

Эндпоинт `/bulk` поддерживает запросы 'insert', 'replace', 'delete' и 'update'. Имейте в виду, что можно направлять операции в несколько таблиц, но транзакции возможны только для одной таблицы. Если указать больше одной, Manticore соберет операции, направленные в одну таблицу, в одну транзакцию. При смене таблицы он зафиксирует накопленные операции и начнет новую транзакцию. Пустая строка, разделяющая партии, также приводит к фиксации предыдущей партии и началу новой транзакции. Запрос без операций, будь то пустое тело или только пустые строки либо строки из пробелов, успешно завершается как no-op и возвращает пустой массив `items`.

В ответе на запрос `/bulk` можно найти следующие поля:
* "errors": показывает, были ли ошибки (true/false)
* "error": описывает возникшую ошибку
* "current_line": номер строки, на которой выполнение остановилось (или завершилось ошибкой); пустые строки, включая первую пустую строку, тоже учитываются
* "skipped_lines": число не зафиксированных строк, начиная с `current_line` и двигаясь назад

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

Для реплицируемых таблиц включайте свойство `cluster` в каждую операцию:

```json
POST /bulk
-H "Content-Type: application/x-ndjson" -d '
{"insert": {"cluster":"posts", "table":"weekly_table", "id":1, "doc":  {"title":"Crossbody Bag with Tassel","price":19.85}}}
{"insert": {"cluster":"posts", "table":"weekly_table", "id":2, "doc":  {"title":"microfiber sheet set","price":19.99}}}
{"insert": {"cluster":"posts", "table":"weekly_table", "id":3, "doc":  {"title":"Pet Hair Remover Glove","price":7.99}}}
'
```

<!-- request Elasticsearch -->

> ПРИМЕЧАНИЕ: если таблица еще не существует, для `_bulk` требуется [Manticore Buddy](Installation/Manticore_Buddy.md). Если это не работает, убедитесь, что Buddy установлен.

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

Для реплицируемых таблиц включайте имя кластера в имя таблицы в формате `cluster_name:table_name`:

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

Для реплицируемых таблиц включайте свойство cluster в каждый документ:

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

Для реплицируемых таблиц включайте свойство cluster в каждый документ:

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

Для реплицируемых таблиц включайте свойство cluster в каждый документ:

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

Для реплицируемых таблиц включайте свойство cluster в каждый документ:

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

Для реплицируемых таблиц включайте свойство cluster в каждый документ:

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
## Вставка значений многоэлементных атрибутов (MVA)

Многоэлементные атрибуты (MVA) вставляются как массивы чисел.
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

Или, как вариант
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
Значение JSON можно вставить как [экранированную](../../Searching/Full_text_matching/Escaping.md) строку (через SQL или JSON) или как JSON-объект (через JSON-интерфейс).

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

Значение JSON можно также вставить как строку с экранированным JSON:
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

Или, как вариант
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
Рассматривайте JSON просто как строку:
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
