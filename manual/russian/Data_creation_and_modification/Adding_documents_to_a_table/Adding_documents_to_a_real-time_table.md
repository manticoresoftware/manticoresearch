# Добавление документов в таблицу в реальном времени

> Если вы ищете информацию о добавлении документов в обычную таблицу, пожалуйста, обратитесь к разделу о [добавлении данных из внешних хранилищ](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md).

<!-- example insert -->
Добавление документов в реальном времени поддерживается только для таблиц [Real-Time](../../Creating_a_table/Local_tables/Real-time_table.md) и [percolate](../../Creating_a_table/Local_tables/Percolate_table.md). Соответствующая SQL-команда, HTTP-эндпоинт или клиентские функции вставляют новые строки (документы) в таблицу с заданными значениями полей. Таблица не обязательно должна существовать до добавления документов. Если таблица не существует, Manticore попытается создать её автоматически. Для дополнительной информации смотрите [Auto schema](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema).

Можно вставлять один или [несколько документов](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-adding-documents) с значениями для всех полей таблицы или лишь для части из них. В этом случае остальные поля будут заполнены значениями по умолчанию (0 для скалярных типов, пустая строка для текстовых).

Выражения в `INSERT` ныне не поддерживаются, поэтому значения должны быть указаны явно.

Поле/значение ID может быть опущено, так как таблицы RT и PQ поддерживают функционал [auto-id](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID). Также можно использовать `0` в качестве значения id для принудительной автоматической генерации ID. Строки с дублирующимися ID не будут перезаписаны при помощи `INSERT`. Вместо этого для таких целей следует использовать [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md).

При использовании HTTP JSON протокола можно выбрать один из двух форматов запроса: общий формат Manticore и формат, похожий на Elasticsearch. Оба формата продемонстрированы в приведённых ниже примерах.

Кроме того, при использовании формата JSON-запроса Manticore, обратите внимание, что узел `doc` обязателен, и все значения должны быть указаны внутри него.


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

> ПРИМЕЧАНИЕ: `_create` требует [Manticore Buddy](Installation/Manticore_Buddy.md). Если не работает, убедитесь, что Buddy установлен.

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

## Auto schema

> ПРИМЕЧАНИЕ: Auto schema требует [Manticore Buddy](Installation/Manticore_Buddy.md). Если не работает, убедитесь, что Buddy установлен.

В Manticore имеется механизм автоматического создания таблиц, который запускается, когда указанная в запросе insert таблица ещё не существует. Этот механизм включён по умолчанию. Для его отключения установите `auto_schema = 0` в секции [Searchd](../../Server_settings/Searchd.md#auto_schema) вашего конфигурационного файла Manticore.

<!-- example auto-schema -->

По умолчанию все текстовые значения в клаузе `VALUES` считаются типа `text`, за исключением значений, представляющих корректные email-адреса, которые трактуются как тип `string`.

Если вы попытаетесь вставить несколько строк с разными, несовместимыми типами значений для одного и того же поля, авто-создание таблицы будет отменено, и будет возвращена ошибка. Однако, если различные типы совместимы, результирующий тип поля будет таким, который может вместить все значения. Некоторые возможные автоматические преобразования типов данных включают:
* mva -> mva64
* uint -> bigint -> float (что может привести к потере точности)
* string -> text

Также будут распознаны и преобразованы в timestamp следующие форматы дат, все остальные форматы будут считаться строками:
- `%Y-%m-%dT%H:%M:%E*S%Z`
- `%Y-%m-%d'T'%H:%M:%S%Z`
- `%Y-%m-%dT%H:%M:%E*S`
- `%Y-%m-%dT%H:%M:%s`
- `%Y-%m-%dT%H:%M`
- `%Y-%m-%dT%H`


Имейте в виду, что HTTP-эндпоинт `/bulk` не поддерживает автоматическое создание таблиц (auto schema). Эту функцию поддерживает только HTTP-эндпоинт `/_bulk` (формат, похожий на Elasticsearch) и SQL-интерфейс.

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

## Auto ID
<!-- example autoid -->
Manticore предоставляет функционал автоматической генерации ID для колонки ID документов, которые вставляются или заменяются в таблицах реального времени или [Percolate table](../../Creating_a_table/Local_tables/Percolate_table.md). Генератор выдаёт уникальный ID с некоторыми гарантиями, но его не следует считать автоинкрементным ID.

Сгенерированное значение ID гарантированно уникально при соблюдении следующих условий:
* Значение [server_id](../../Server_settings/Searchd.md#server_id) текущего сервера находится в диапазоне от 0 до 127 и уникально среди узлов в кластере, либо используется значение по умолчанию, сформированное из MAC-адреса как seed
* Системное время не меняется для узла Manticore между перезапусками сервера
* Авто ID генерируется менее 16 миллионов раз в секунду между перезапусками поискового сервера

Генератор авто ID создаёт 64-битное целое число для ID документа и использует следующую схему:
* Биты с 0 по 23 образуют счётчик, который увеличивается при каждом вызове генератора авто ID
* Биты с 24 по 55 представляют Unix timestamp времени запуска сервера
* Биты с 56 по 63 соответствуют server_id сервера

Эта схема гарантирует, что сгенерированный ID уникален среди всех узлов в кластере и что данные, вставленные в разные узлы кластера, не создают коллизий между узлами.

В результате первый ID от генератора, используемого для авто ID, НЕ равен 1, а больше. Кроме того, поток документов, вставляемых в таблицу, может иметь не последовательные значения ID, если между вызовами происходят вставки в другие таблицы, так как генератор ID един для сервера и общий для всех его таблиц.

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
### UUID_SHORT множественная генерация ID

```sql
CALL UUID_SHORT(N)
```

Оператор `CALL UUID_SHORT(N)` позволяет сгенерировать N уникальных 64-битных ID за один вызов без вставки каких-либо документов. Это особенно полезно, когда нужно предварительно сгенерировать ID в Manticore для использования в других системах или решениях хранения данных. Например, вы можете генерировать авто-ID в Manticore и затем использовать их в другой базе данных, приложении или рабочем процессе, обеспечивая согласованные и уникальные идентификаторы в разных средах.

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
Вы можете вставлять не только один документ в таблицу реального времени, но и столько, сколько захотите. Полностью приемлемо вставлять партии из десятков тысяч документов в таблицу реального времени. Однако важно учесть следующие моменты:
* Чем больше партия, тем выше задержка для каждой операции вставки
* Чем больше партия, тем выше ожидаемая скорость индексирования
* Возможно, вам захочется увеличить значение [max_packet_size](../../Server_settings/Searchd.md#max_packet_size), чтобы разрешить большие партии
* Обычно каждая операция пакетной вставки считается одной [транзакцией](../../Data_creation_and_modification/Transactions.md) с гарантией атомарности, поэтому либо все новые документы будут добавлены в таблицу одновременно, либо в случае сбоя ни один из них добавлен не будет. Подробнее об пустой строке или переключении на другую таблицу смотрите в примере "JSON".

Обратите внимание, что HTTP endpoint `/bulk` не поддерживает автоматическое создание таблиц (auto schema). Только HTTP endpoint `/_bulk` (в стиле Elasticsearch) и SQL интерфейс поддерживают эту функцию. HTTP endpoint `/_bulk` (в стиле Elasticsearch) позволяет указать имя таблицы с включением имени кластера в формате `cluster_name:table_name`.

`/_bulk` endpoint принимает ID документов в том же формате, что и Elasticsearch, и вы также можете включать `id` внутри самого документа:
```json
{ "index": { "table" : "products", "_id" : "1" } }
{ "title" : "Crossbody Bag with Tassel", "price": 19.85 }
```

или

```json
{ "index": { "table" : "products" } }
{ "title" : "Crossbody Bag with Tassel", "price": 19.85, "id": "1" }
```

#### Передача чанками в /bulk
`/bulk` endpoint (режим Manticore) поддерживает [Chunked transfer encoding](https://en.wikipedia.org/wiki/Chunked_transfer_encoding). Вы можете использовать его для передачи больших партий. Это:
* снижает пиковое использование оперативной памяти, уменьшая риск OOM
* уменьшает время отклика
* позволяет обойти ограничение [max_packet_size](../../Server_settings/Searchd.md#max_packet_size) и передавать партии гораздо большего размера, чем максимально разрешённое значение `max_packet_size` (128MB), например, по 1GB за раз.

<!-- intro -->
### Примеры массовой вставки
##### SQL:
<!-- request SQL -->
Для массовой вставки просто укажите больше документов в скобках после `VALUES()`. Синтаксис:

```sql
INSERT INTO <table name>[(column1, column2, ...)] VALUES(value1[, value2 , ...]), (...)
```

Необязательный список имён столбцов позволяет явно указать значения для некоторых из столбцов таблицы. Все остальные столбцы будут заполнены значениями по умолчанию (0 для скалярных типов, пустая строка для строковых типов).

Например:

```sql
INSERT INTO products(title,price) VALUES ('Crossbody Bag with Tassel', 19.85), ('microfiber sheet set', 19.99), ('Pet Hair Remover Glove', 7.99);
```
<!-- response SQL -->

```sql
Query OK, 3 rows affected (0.01 sec)
```

В выражениях типа `INSERT` сейчас не поддерживаются выражения, значения должны быть указаны явно.

<!-- intro -->
##### JSON:
<!-- request JSON -->
Синтаксис в целом такой же, как и при [вставке одного документа](../../Quick_start_guide.md#Add-documents). Просто укажите больше строк, по одной на каждый документ, и используйте эндпоинт `/bulk` вместо `/insert`. Оберните каждый документ в узел "insert". Обратите внимание, что также требуется:
* `Content-Type: application/x-ndjson`
* Данные должны быть в формате newline-delimited JSON (NDJSON). По сути, это означает, что каждая строка должна содержать ровно одно JSON-выражение и завершаться переводом строки `\n` и возможно `\r`.

`/bulk` endpoint поддерживает запросы 'insert', 'replace', 'delete' и 'update'. Учтите, что вы можете направлять операции сразу в несколько таблиц, но транзакции возможны только для одной таблицы. Если вы укажете несколько, Manticore соберёт операции для одной таблицы в одну транзакцию. При смене таблицы он зафиксирует собранные операции и начнёт новую транзакцию с новой таблицей. Пустая строка между партиями также приводит к фиксации предыдущей партии и началу новой транзакции.

В ответе на запрос `/bulk` можно найти следующие поля:
* "errors": показывает, были ли ошибки (true/false)
* "error": описание произошедшей ошибки
* "current_line": номер строки, на которой остановилось (или произошёл сбой) выполнение; пустые строки, включая первую пустую строку, также учитываются
* "skipped_lines": количество незафиксированных строк, считая от `current_line` и идя назад

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

> ПРИМЕЧАНИЕ: `_bulk` требует [Manticore Buddy](Installation/Manticore_Buddy.md), если таблица ещё не существует. Если не работает, убедитесь, что Buddy установлен.

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
Используйте метод addDocuments():

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
## Вставка значений многозначных атрибутов (MVA)

Многозначные атрибуты (MVA) вставляются как массивы чисел.
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

Или, альтернативно
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
JSON значение можно вставить как [экранированную](../../Searching/Full_text_matching/Escaping.md) строку (через SQL или JSON) или как JSON объект (через JSON интерфейс).

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
JSON значение можно вставлять как JSON объект
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

JSON значение также можно вставить как строку, содержащую экранированный JSON:
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

Или, альтернативно
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

