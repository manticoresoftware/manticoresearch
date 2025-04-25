# Добавление документов в таблицу реального времени

> Если вы ищете информацию о добавлении документов в обычную таблицу, пожалуйста, обратитесь к разделу о [добавлении данных из внешних хранилищ](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md).

<!-- example insert -->
Добавление документов в реальном времени поддерживается только для таблиц [Real-Time](../../Creating_a_table/Local_tables/Real-time_table.md) и [percolate](../../Creating_a_table/Local_tables/Percolate_table.md). Соответствующая SQL-команда, HTTP-эндпоинт или клиентские функции вставляют новые строки (документы) в таблицу с указанными значениями полей. Необязательно, чтобы таблица существовала до добавления в неё документов. Если таблица не существует, Manticore попытается создать её автоматически. Для получения дополнительной информации см. [Авто-схема](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema).

Вы можете вставить один или [несколько документов](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real_time_table.md#Bulk-adding-documents) со значениями для всех полей таблицы или только для части из них. В этом случае остальные поля будут заполнены значениями по умолчанию (0 для скалярных типов, пустая строка для текстовых типов).

Выражения в данный момент не поддерживаются в `INSERT`, поэтому значения должны быть явно указаны.

Поле/значение ID можно опустить, так как таблицы RT и PQ поддерживают функциональность [авто-ID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID). Вы также можете использовать `0` в качестве значения id, чтобы принудительно сгенерировать ID автоматически. Строки с дублирующимися ID не будут перезаписаны командой `INSERT`. Вместо этого вы можете использовать [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md) для этой цели.

При использовании HTTP JSON протокола у вас есть два различных формата запросов на выбор: общий формат Manticore и формат, похожий на Elasticsearch. Оба формата продемонстрированы в примерах ниже.

Кроме того, при использовании формата запроса Manticore JSON имейте в виду, что узел `doc` является обязательным, и все значения должны быть предоставлены внутри него.


<!-- intro -->
##### SQL:
<!-- request SQL -->
Общий синтаксис:

```sql
INSERT INTO <название таблицы> [(колонка, ...)]
VALUES (значение, ...)
[, (...)]
```

```sql
INSERT INTO products(title,price) VALUES ('Сумка через плечо с кисточкой', 19.85);
INSERT INTO products(title) VALUES ('Сумка через плечо с кисточкой');
INSERT INTO products VALUES (0,'Желтая сумка', 4.95);
```
<!-- response SQL -->

```sql
Запрос выполнен, затронуто 1 строк (0.00 сек)
Запрос выполнен, затронуто 1 строк (0.00 сек)
Запрос выполнен, затронуто 1 строк (0.00 сек)
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
    "title" : "Сумка через плечо с кисточкой",
    "price" : 19.85
  }
}

POST /insert
{
  "table":"products",
  "id":2,
  "doc":
  {
    "title" : "Сумка через плечо с кисточкой"
  }
}

POST /insert
{
  "table":"products",
  "id":0,
  "doc":
  {
    "title" : "Желтая сумка"
  }
}

```

<!-- response JSON -->

```json
{
  "table": "products",
  "_id": 1,
  "created": true,
  "result": "создано",
  "status": 201
}
{
  "table": "products",
  "_id": 2,
  "created": true,
  "result": "создано",
  "status": 201
}
{
  "table": "products",
  "_id": 1657860156022587406,
  "created": true,
  "result": "создано",
  "status": 201
}

```

<!-- intro -->
##### Elasticsearch:

<!-- request Elasticsearch -->

> ЗАМЕТКА: `_create` требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если это не работает, убедитесь, что Buddy установлен.

```json
POST /products/_create/3
{
  "title": "Желтая сумка с кисточкой",
  "price": 19.85
}

POST /products/_create/
{
  "title": "Красная сумка с кисточкой",
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
"result":"обновлено"
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
"result":"обновлено"
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->addDocuments([
        ['id' => 1, 'title' => 'Сумка через плечо с кисточкой', 'price' => 19.85]
]);
$index->addDocuments([
        ['id' => 2, 'title' => 'Сумка через плечо с кисточкой']
]);
$index->addDocuments([
        ['id' => 0, 'title' => 'Желтая сумка']
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

``` python
indexApi.insert({"table" : "test", "id" : 1, "doc" : {"title" : "Сумка через плечо с кисточкой", "price" : 19.85}})
indexApi.insert({"table" : "test", "id" : 2, "doc" : {"title" : "Сумка через плечо с кисточкой"}})
indexApi.insert({"table" : "test", "id" : 0, "doc" : {{"title" : "Желтая сумка"}})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

``` python
await indexApi.insert({"table" : "test", "id" : 1, "doc" : {"title" : "Сумка через плечо с кисточкой", "price" : 19.85}})
await indexApi.insert({"table" : "test", "id" : 2, "doc" : {"title" : "Сумка через плечо с кисточкой"}})
await indexApi.insert({"table" : "test", "id" : 0, "doc" : {{"title" : "Желтая сумка"}})
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

``` javascript
res = await indexApi.insert({"table" : "test", "id" : 1, "doc" : {"title" : "Сумка через плечо с кисточкой", "price" : 19.85}});
res = await indexApi.insert({"table" : "test", "id" : 2, "doc" : {"title" : "Сумка через плечо с кисточкой"}});
res = await indexApi.insert({"table" : "test", "id" : 0, "doc" : {{"title" : "Желтая сумка"}});
```

<!-- intro -->
##### java:

<!-- request Java -->

``` java
InsertDocumentRequest newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","Сумка через плечо с кисточкой");
    put("price",19.85);
}};
newdoc.index("products").id(1L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);

newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","Сумка через плечо с кисточкой");
}};
newdoc.index("products").id(2L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);

newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","Желтая сумка");
 }};
newdoc.index("products").id(0L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);

```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "Сумка через плечо с кисточкой");
doc.Add("price", 19.85);
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", id: 1, doc: doc);
var sqlresult = indexApi.Insert(newdoc);

doc = new Dictionary<string, Object>();
doc.Add("title", "Сумка через плечо с кисточкой");
newdoc = new InsertDocumentRequest(index: "products", id: 2, doc: doc);
sqlresult = indexApi.Insert(newdoc);

doc = new Dictionary<string, Object>();
doc.Add("title", "Желтая сумка");
newdoc = new InsertDocumentRequest(index: "products", id: 0, doc: doc);
sqlresult = indexApi.Insert(newdoc);

```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let mut doc = HashMap::new();
doc.insert("title".to_string(), serde_json::json!("Сумка через плечо с кисточкой"));
doc.insert("price".to_string(), serde_json::json!(19.85));
let mut insert_req = InsertDocumentRequest {
    table: serde_json::json!("products"),
    doc: serde_json::json!(doc),
    id: serde_json::json!(1),
    ..Default::default(),
};
let mut insert_res = index_api.insert(insert_req).await;

doc = HashMap::new();
doc.insert("title".to_string(), serde_json::json!("Сумка через плечо с кисточкой"));
insert_req = InsertDocumentRequest {
    table: serde_json::json!("products"),
    doc: serde_json::json!(doc),
    id: serde_json::json!(2),
    ..Default::default(),
};
insert_res = index_api.insert(insert_req).await;

doc = HashMap::new();
doc.insert("title".to_string(), serde_json::json!("Желтая сумка"));
insert_req = InsertDocumentRequest {
    table: serde_json::json!("products"),
    doc: serde_json::json!(doc),
    id: serde_json::json!(0),
    ..Default::default(),
};
insert_res = index_api.insert(insert_req).await;
```

<!-- end -->

## Авто схема

> ЗАМЕТКА: Авто схема требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если это не работает, убедитесь, что Buddy установлен.

Manticore имеет механизм автоматического создания таблиц, который активируется, когда указанная таблица в запросе вставки еще не существует. Этот механизм включен по умолчанию. Чтобы отключить его, установите `auto_schema = 0` в разделе [Searchd](../../Server_settings/Searchd.md#auto_schema) вашего конфигурационного файла Manticore.

<!-- example auto-schema -->

По умолчанию все текстовые значения в предложении `VALUES` считаются типом `text`, за исключением значений, представляющих действительные адреса электронной почты, которые рассматриваются как тип `string`.

Если вы попытаетесь ВСТАВИТЬ несколько строк с разными, несовместимыми типами значений для одного и того же поля, автоматическое создание таблицы будет отменено, и будет возвращено сообщение об ошибке. Однако, если различные типы значений совместимы, конечный тип поля будет тем, который вмещает все значения. Некоторые автоматические преобразования типов данных, которые могут произойти, включают:
* mva -> mva64
* uint -> bigint -> float (это может вызвать некоторую потерю точности)
* string -> text

Также будут распознаны и преобразованы в временные метки следующие форматы дат, в то время как все другие форматы дат будут рассматриваться как строки:
- `%Y-%m-%dT%H:%M:%E*S%Z`
- `%Y-%m-%d'T'%H:%M:%S%Z`
- `%Y-%m-%dT%H:%M:%E*S`
- `%Y-%m-%dT%H:%M:%s`
- `%Y-%m-%dT%H:%M`
- `%Y-%m-%dT%H`


Имейте в виду, что HTTP-эндпоинт `/bulk` не поддерживает автоматическое создание таблиц (авто схема). Только HTTP-эндпоинт `/_bulk` (аналогичный Elasticsearch) и SQL-интерфейс поддерживают эту функцию.

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
8 строк в наборе (0.00 сек)

--------------
select * from t
--------------

+---------------------+------+---------------+----------+------+-----------------------------+-----------+---------------+------------+
| id                  | i    | b             | f        | m    | mb                          | t         | s             | j          |
+---------------------+------+---------------+----------+------+-----------------------------+-----------+---------------+------------+
| 5045949922868723723 |  123 | 1099511627776 | 1.200000 | 1,2  | 1099511627776,1099511627777 | текст здесь | test@mail.com | {"a": 123} |
+---------------------+------+---------------+----------+------+-----------------------------+-----------+---------------+------------+
1 строка в наборе (0.00 сек)
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
   "t": "текст здесь",
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

## Авто ID
<!-- example autoid -->
Manticore предоставляет функцию генерации авто ID для столбца ID документов, вставленных или замененных в реальном времени или [таблице Перколятор](../../Creating_a_table/Local_tables/Percolate_table.md). Генератор создает уникальный ID для документа с некоторыми гарантиями, но его не следует считать автоинкрементируемым ID.

Сгенерированное значение ID гарантированно уникально при следующих условиях:
* Значение [server_id](../../Server_settings/Searchd.md#server_id) текущего сервера находится в диапазоне от 0 до 127 и уникально среди узлов кластера, или использует значение по умолчанию, сгенерированное из MAC-адреса в качестве семени
* Системное время не изменяется для узла Manticore между перезагрузками сервера
* Авто ID генерируется менее чем 16 миллионов раз в секунду между перезагрузками сервера поиска

Генератор авто ID создает 64-битное целое число для ID документа и использует следующую схемы:
* Биты с 0 по 23 формируют счетчик, который увеличивается при каждом вызове генератора авто ID
* Биты с 24 по 55 представляют временную метку Unix старта сервера
* Биты с 56 по 63 соответствуют server_id

Эта схема гарантирует, что сгенерированный ID уникален среди всех узлов в кластере и что данные, вставленные в разные узлы кластера, не создают коллизии между узлами.

В результате первый ID от генератора, используемого для авто ID, НЕ равен 1, а является большим числом. Кроме того, поток документов, вставленных в таблицу, может иметь не последовательные значения ID, если вставки в другие таблицы происходят между вызовами, так как генератор ID является единичным на сервере и разделяется между всеми его таблицами.

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

<!-- пример вызова -->
### UUID_SHORT много-ID генерация

```sql
CALL UUID_SHORT(N)
```

Оператор `CALL UUID_SHORT(N)` позволяет генерировать N уникальных 64-битных идентификаторов за один вызов без вставки каких-либо документов. Это особенно полезно, когда вам нужно предварительно сгенерировать идентификаторы в Manticore для использования в других системах или решениях для хранения. Например, вы можете сгенерировать авто-ID в Manticore, а затем использовать их в другой базе данных, приложении или рабочем процессе, обеспечивая последовательные и уникальные идентификаторы в разных средах.

<!-- ввод -->
##### Пример:
<!-- запрос Пример -->

```sql
CALL UUID_SHORT(3)
```
<!-- ответ SQL -->
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

<!-- пример bulk_insert -->
## Пакетное добавление документов
Вы можете вставить не просто один документ в реальное время, но столько, сколько вам нужно. Ничего страшного в том, чтобы вставлять партии из десятков тысяч документов в таблицу в реальном времени. Тем не менее, важно учитывать следующие моменты:
* Чем больше партия, тем выше задержка каждой операции вставки
* Чем больше партия, тем выше скорость индексации, которую вы можете ожидать
* Возможно, вам захочется увеличить значение [max_packet_size](../../Server_settings/Searchd.md#max_packet_size), чтобы позволить более крупные партии
* Обычно каждая операция пакетной вставки рассматривается как одна [транзакция](../../Data_creation_and_modification/Transactions.md) с гарантией атомарности, поэтому вы либо получите все новые документы в таблице сразу, либо в случае сбоя ни один из них не будет добавлен. Смотрите больше деталей о пустой строке или переходе на другую таблицу в примере "JSON".

Обратите внимание, что HTTP-эндпоинт `/bulk` не поддерживает автоматическое создание таблиц (авто-схема). Только HTTP-эндпоинт `/_bulk` (подобно Elasticsearch) и SQL-интерфейс поддерживают эту функцию. HTTP-эндпоинт `/_bulk` (подобно Elasticsearch) позволяет имени таблицы включать имя кластера в формате `cluster_name:table_name`.

Эндпоинт `/_bulk` принимает идентификаторы документов в том же формате, что и Elasticsearch, и вы также можете включить `id` внутри самого документа:
```json
{ "index": { "table" : "products", "_id" : "1" } }
{ "title" : "Сумка через плечо с кисточкой", "price": 19.85 }
```

или

```json
{ "index": { "table" : "products" } }
{ "title" : "Сумка через плечо с кисточкой", "price": 19.85, "id": "1" }
```

#### Пакетная передача в /bulk
Эндпоинт `/bulk` (Режим Manticore) поддерживает [Кодирование с распределением на фрагменты](https://en.wikipedia.org/wiki/Chunked_transfer_encoding). Вы можете использовать его для передачи крупных партий. Он:
* сокращает пиковое использование ОЗУ, снижая риск OOM
* уменьшает время отклика
* позволяет обойти [max_packet_size](../Server_settings/Searchd.md#max_packet_size) и передавать партии, значительно превышающие максимальное допустимое значение `max_packet_size` (128 МБ), например, 1 ГБ за раз.

<!-- ввод -->
### Примеры пакетной вставки
##### SQL:
<!-- запрос SQL -->
Для пакетной вставки просто предоставьте больше документов в скобках после `VALUES()`. Синтаксис таков:

```sql
INSERT INTO <имя таблицы>[(column1, column2, ...)] VALUES(value1[, value2 , ...]), (...)
```

Список названий столбцов является необязательным и позволяет вам явно указывать значения для некоторых из столбцов, присутствующих в таблице. Все остальные столбцы будут заполнены своими значениями по умолчанию (0 для скалярных типов, пустая строка для строковых типов).

Например:

```sql
INSERT INTO products(title,price) VALUES ('Сумка через плечо с кисточкой', 19.85), ('набор микрофибры', 19.99), ('перчатка для удаления шерсти домашних животных', 7.99);
```
<!-- ответ SQL -->

```sql
Запрос выполнен, 3 строки затронуты (0.01 сек)
```

В выражениях в настоящее время не поддерживается `INSERT`, и значения должны быть явно указаны.

<!-- ввод -->
##### JSON:
<!-- запрос JSON -->
Синтаксис в целом такой же, как для [вставки одного документа](../../Quick_start_guide.md#Add-documents). Просто предоставьте больше строк, по одной для каждого документа, и используйте эндпоинт `/bulk` вместо `/insert`. Заключите каждый документ в узел "insert". Обратите внимание, что он также требует:
* `Content-Type: application/x-ndjson`
* Данные должны быть отформатированы как JSON, разделенный новой строкой (NDJSON). По сути, это означает, что каждая строка должна содержать ровно одно заявление JSON и заканчиваться новой строкой `
` и, возможно, ``.

Эндпоинт `/bulk` поддерживает запросы 'вставить', 'заменить', 'удалить' и 'обновить'. Имейте в виду, что вы можете направить операции на несколько таблиц, но транзакции возможны только для одной таблицы. Если вы укажете больше, Manticore соберет операции, направленные на одну таблицу, в одну транзакцию. Когда таблица изменяется, она зафиксирует собранные операции и инициирует новую транзакцию на новой таблице. Пустая строка, разделяющая партии, также приводит к фиксации предыдущей партии и началу новой транзакции.

В ответе на запрос `/bulk` вы можете найти следующие поля:
* "errors": показывает, произошли ли какие-либо ошибки (true/false)
* "error": описывает произошедшую ошибку
* "current_line": номер строки, на которой выполнение остановилось (или завершилось неудачей); пустые строки, включая первую пустую строку, также учитываются
* "skipped_lines": количество непрофиксированных строк, начиная с `current_line` и движущихся назад

```json
POST /bulk
-H "Content-Type: application/x-ndjson" -d '
{"insert": {"table":"products", "id":1, "doc":  {"title":"Сумка через плечо с кисточкой","price" : 19.85}}}
{"insert":{"table":"products", "id":2, "doc":  {"title":"набор микрофибры","price" : 19.99}}}
'

POST /bulk
-H "Content-Type: application/x-ndjson" -d '
{"insert":{"table":"test1","id":21,"doc":{"int_col":1,"price":1.1,"title":"оптовый документ один"}}}
{"insert":{"table":"test1","id":22,"doc":{"int_col":2,"price":2.2,"title":"оптовый документ два"}}}

{"insert":{"table":"test1","id":23,"doc":{"int_col":3,"price":3.3,"title":"оптовый документ три"}}}
{"insert":{"table":"test2","id":24,"doc":{"int_col":4,"price":4.4,"title":"оптовый документ четыре"}}}
{"insert":{"table":"test2","id":25,"doc":{"int_col":5,"price":5.5,"title":"оптовый документ пять"}}}
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

> NOTE: `_bulk` требует [Manticore Buddy](../Installation/Manticore_Buddy.md), если таблица еще не существует. Если не работает, убедитесь, что Buddy установлен.

```json
POST /_bulk
-H "Content-Type: application/x-ndjson" -d '
{ "index" : { "table" : "products" } }
{ "title" : "Желтая сумка", "price": 12 }
{ "create" : { "table" : "products" } }
{ "title" : "Красная сумка", "price": 12.5, "id": 3 }
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
        ['id' => 1, 'title' => 'Сумка через плечо с кисточкой', 'price' => 19.85],
        ['id' => 2, 'title' => 'комплект простыней из микрофибры', 'price' => 19.99],
        ['id' => 3, 'title' => 'Перчатка для удаления шерсти домашних животных', 'price' => 7.99]
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
docs = [     {"insert": {"table" : "products", "id" : 1, "doc" : {"title" : "Сумка через плечо с кисточкой", "price" : 19.85}}},     {"insert": {"table" : "products", "id" : 2, "doc" : {"title" : "комплект простыней из микрофибры", "price" : 19.99}}},     {"insert": {"table" : "products", "id" : 3, "doc" : {"title" : "Перчатка для удаления шерсти домашних животных", "price" : 7.99}}}
]
res = indexApi.bulk('
'.join(map(json.dumps,docs)))
```

<!-- intro -->
##### Python=asyncio:

<!-- request Python-asyncio -->

```python
docs = [     {"insert": {"table" : "products", "id" : 1, "doc" : {"title" : "Сумка через плечо с кисточкой", "price" : 19.85}}},     {"insert": {"table" : "products", "id" : 2, "doc" : {"title" : "комплект простыней из микрофибры", "price" : 19.99}}},     {"insert": {"table" : "products", "id" : 3, "doc" : {"title" : "Перчатка для удаления шерсти домашних животных", "price" : 7.99}}}
]
res = await indexApi.bulk('
'.join(map(json.dumps,docs)))
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
let docs = [
    {"insert": {"table" : "products", "id" : 3, "doc" : {"title" : "Сумка через плечо с кисточкой", "price" : 19.85}}},
    {"insert": {"table" : "products", "id" : 4, "doc" : {"title" : "комплект простыней из микрофибры", "price" : 19.99}}},
    {"insert": {"table" : "products", "id" : 5, "doc" : {"title" : "Перчатка для удаления шерсти домашних животных", "price" : 7.99}}}
];
res =  await indexApi.bulk(docs.map(e=>JSON.stringify(e)).join('
'));
```
<!-- intro -->
##### java:

<!-- request Java -->

``` java
String body = "{"insert": {"index" : "products", "id" : 1, "doc" : {"title" : "Сумка через плечо с кисточкой", "price" : 19.85}}}"+"
"+
    "{"insert": {"index" : "products", "id" : 4, "doc" : {"title" : "комплект простыней из микрофибры", "price" : 19.99}}}"+"
"+
    "{"insert": {"index" : "products", "id" : 5, "doc" : {"title" : "Перчатка для удаления шерсти домашних животных", "price" : 7.99}}}"+"
";
BulkResponse bulkresult = indexApi.bulk(body);
```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
string body = "{"insert": {"index" : "products", "id" : 1, "doc" : {"title" : "Сумка через плечо с кисточкой", "price" : 19.85}}}"+"
"+
    "{"insert": {"index" : "products", "id" : 4, "doc" : {"title" : "комплект простыней из микрофибры", "price" : 19.99}}}"+"
"+
    "{"insert": {"index" : "products", "id" : 5, "doc" : {"title" : "CPet Hair Remover Glove", "price" : 7.99}}}"+"
";
BulkResponse bulkresult = indexApi.Bulk(string.Join("
", docs));
```

<!-- intro -->
##### Rust:

<!-- request Rust -->
``` rust
let bulk_body = r#"{"insert": "index" : "products", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}}}
    {"insert": {"index" : "products", "id" : 4, "doc" : {"title" : "микрофибровый набор простыней", "price" : 19.99}}}
    {"insert": {"index" : "products", "id" : 5, "doc" : {"title" : "CPet Hair Remover Glove", "price" : 7.99}}}         
"#;
index_api.bulk(bulk_body).await;
```

<!-- end -->
<!-- example MVA_insert -->
## Вставка значений многозначных атрибутов (MVA)

Многозначные атрибуты (MVA) вставляются в виде массивов чисел.
<!-- intro -->
### Примеры
##### SQL
<!-- request SQL -->
```

INSERT INTO products(title, sizes) VALUES('обувь', (40,41,42,43));
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
    "title" : "обувь",
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
  "title": "обувь",
  "sizes" : [40, 41, 42, 43]
}
```

Или, альтернативно
```json
POST /products/_doc/
{
  "title": "обувь",
  "sizes" : [40, 41, 42, 43]
}
```


<!-- intro -->
##### PHP
<!-- request PHP -->

```php
$index->addDocument(
  ['title' => 'обувь', 'sizes' => [40,41,42,43]],
  1
);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "Желтая сумка","sizes":[40,41,42,43]}})
```

<!-- intro -->
##### Python=asyncio:

<!-- request Python-asyncio -->

```python
await indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "Желтая сумка","sizes":[40,41,42,43]}})
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "Желтая сумка","sizes":[40,41,42,43]}});
```


<!-- intro -->
##### java:

<!-- request Java -->

``` java
newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","Желтая сумка");
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
doc.Add("title", "Желтая сумка");
doc.Add("sizes", new List<Object> {40,41,42,43});
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", id: 0, doc: doc);
var sqlresult = indexApi.Insert(newdoc);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let mut doc = HashMap::new();
doc.insert("title".to_string(), serde_json::json!("Желтая сумка"));
doc.insert("sizes".to_string(), serde_json::json!([40,41,42,43]));
let insert_req = InsertDocumentRequest::new("products".to_string(), serde_json::json!(doc));
let insert_res = index_api.insert(insert_req).await;
```

<!-- end -->

<!-- example JSON_insert -->
## Вставка JSON
Значение JSON может быть вставлено как [экранированная](../../Searching/Full_text_matching/Escaping.md) строка (через SQL или JSON) или как объект JSON (через интерфейс JSON).

<!-- intro -->
### Примеры
##### SQL
<!-- request SQL -->
```

INSERT INTO products VALUES (1, 'обувь', '{"size": 41, "color": "красный"}');
```
<!-- intro -->
##### JSON
<!-- request JSON -->
Значение JSON может быть вставлено как объект JSON
```json
POST /insert
{
  "table":"products",
  "id":1,
  "doc":
  {
    "title" : "обувь",
    "meta" : {
      "size": 41,
      "color": "красный"
    }
  }
}
```

Значение JSON также может быть вставлено как строка, содержащая экранированный JSON:
```json
POST /insert
{
  "table":"products",
  "id":1,
  "doc":
  {
    "title" : "обувь",
    "meta" : "{"size": 41, "color": "красный"}"
  }
}
```

<!-- intro -->
##### Elasticsearch:
<!-- request Elasticsearch -->

```json
POST /products/_create/1
{
  "title": "обувь",
  "meta" : {
    "size": 41,
    "color": "красный"
  }
}
```

Или, альтернативно
```json
POST /products/_doc/
{
  "title": "обувь",
  "meta" : {
    "size": 41,
    "color": "красный"
  }
}
```


<!-- intro -->
##### PHP
Рассматривайте JSON просто как строку:
<!-- request PHP -->

```php
$index->addDocument(
  ['title' => 'обувь', 'meta' => '{"size": 41, "color": "красный"}'],
  1
);
```
<!-- intro -->
##### Python:

<!-- request Python -->
``` python
indexApi = api = manticoresearch.IndexApi(client)
indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "Желтая сумка","meta":'{"size": 41, "color": "красный"}'}})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->
``` python
indexApi = api = manticoresearch.IndexApi(client)
await indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "Желтая сумка","meta":'{"size": 41, "color": "красный"}'}})
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->
```javascript

res = await indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "Желтая сумка","meta":'{"size": 41, "color": "красный"}'}});
```
<!-- intro -->
##### java:

<!-- request Java -->

``` java
newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","Желтая сумка");
    put("meta",
        new HashMap<String,Object>(){{
            put("size",41);
            put("color","красный");
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
meta.Add("color", "красный");
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "Желтая сумка");
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
meta.insert("color".to_string(), serde_json::json!("красный"));

let mut doc = HashMap::new();
doc.insert("title".to_string(), serde_json::json!("Желтая сумка"));
doc.insert("meta".to_string(), serde_json::json!(meta));

let insert_req = InsertDocumentRequest::new("products".to_string(), serde_json::json!(doc));
let insert_res = index_api.insert(insert_req).await;
```

<!-- end -->

<!-- proofread -->
# Добавление документов в таблицу реального времени

> Если вы ищете информацию о добавлении документов в обычную таблицу, пожалуйста, обратитесь к разделу о [добавлении данных из внешних хранилищ](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md).

<!-- example insert -->
Добавление документов в реальном времени поддерживается только для [Real-Time](../../Creating_a_table/Local_tables/Real-time_table.md) и [percolate](../../Creating_a_table/Local_tables/Percolate_table.md) таблиц. Соответствующая команда SQL, HTTP-эндпоинт или клиентские функции вставляют новые строки (документы) в таблицу с предоставленными значениями полей. Не обязательно, чтобы таблица существовала до добавления в нее документов. Если таблицы не существует, Manticore попытается создать ее автоматически. Для получения дополнительной информации см. [Авто-схема](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema).

Вы можете вставить один или [несколько документов](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-adding-documents) со значениями для всех полей таблицы или только для части из них. В этом случае остальные поля будут заполнены их значениями по умолчанию (0 для скалярных типов, пустая строка для текстовых типов).

Выражения в данный момент не поддерживаются в `INSERT`, поэтому значения должны быть явно указаны.

Поле/значение ID можно опустить, так как таблицы RT и PQ поддерживают функциональность [auto-id](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID). Вы также можете использовать `0` в качестве значения id, чтобы принудительно сгенерировать автоматический ID. Строки с дублирующимися ID не будут перезаписаны с помощью `INSERT`. Вместо этого вы можете использовать [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md) для этой цели.

При использовании HTTP JSON протокола у вас есть два различных формата запроса на выбор: общий формат Manticore и формат, похожий на Elasticsearch. Оба формата продемонстрированы в примерах ниже.

Кроме того, при использовании формата запроса Manticore JSON имейте в виду, что узел `doc` является обязательным, и все значения должны быть указаны внутри него.


<!-- intro -->
##### SQL:
<!-- request SQL -->
Общий синтаксис:

```sql
INSERT INTO <имя таблицы> [(столбец, ...)]
VALUES (значение, ...)
[, (...)]
```

```sql
INSERT INTO products(title,price) VALUES ('Сумка через плечо с кисточкой', 19.85);
INSERT INTO products(title) VALUES ('Сумка через плечо с кисточкой');
INSERT INTO products VALUES (0,'Желтая сумка', 4.95);
```
<!-- response SQL -->

```sql
Запрос OK, 1 строка затронута (0.00 сек)
Запрос OK, 1 строка затронута (0.00 сек)
Запрос OK, 1 строка затронута (0.00 сек)
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
    "title" : "Сумка через плечо с кисточкой",
    "price" : 19.85
  }
}

POST /insert
{
  "table":"products",
  "id":2,
  "doc":
  {
    "title" : "Сумка через плечо с кисточкой"
  }
}

POST /insert
{
  "table":"products",
  "id":0,
  "doc":
  {
    "title" : "Желтая сумка"
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

> ПРИМЕЧАНИЕ: `_create` требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если это не работает, убедитесь, что Buddy установлен.

```json
POST /products/_create/3
{
  "title": "Желтая сумка с кисточкой",
  "price": 19.85
}

POST /products/_create/
{
  "title": "Красная сумка с кисточкой",
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
        ['id' => 1, 'title' => 'Сумка через плечо с кисточкой', 'price' => 19.85]
]);
$index->addDocuments([
        ['id' => 2, 'title' => 'Сумка через плечо с кисточкой']
]);
$index->addDocuments([
        ['id' => 0, 'title' => 'Желтая сумка']
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

``` python
indexApi.insert({"table" : "test", "id" : 1, "doc" : {"title" : "Сумка через плечо с кисточкой", "price" : 19.85}})
indexApi.insert({"table" : "test", "id" : 2, "doc" : {"title" : "Сумка через плечо с кисточкой"}})
indexApi.insert({"table" : "test", "id" : 0, "doc" : {{"title" : "Желтая сумка"}})
```
<!-- intro -->
##### Javascript:

<!-- request Javascript -->

``` javascript
res = await indexApi.insert({"table" : "test", "id" : 1, "doc" : {"title" : "Сумка через плечо с кисточкой", "price" : 19.85}});
res = await indexApi.insert({"table" : "test", "id" : 2, "doc" : {"title" : "Сумка через плечо с кисточкой"}});
res = await indexApi.insert({"table" : "test", "id" : 0, "doc" : {{"title" : "Желтая сумка"}});
```

<!-- intro -->
##### java:

<!-- request Java -->

``` java
InsertDocumentRequest newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","Сумка через плечо с кисточкой");
    put("price",19.85);
}};
newdoc.index("products").id(1L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);

newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","Сумка через плечо с кисточкой");
}};
newdoc.index("products").id(2L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);

newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","Желтая сумка");
 }};
newdoc.index("products").id(0L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);

```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "Сумка через плечо с кисточкой");
doc.Add("price", 19.85);
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", id: 1, doc: doc);
var sqlresult = indexApi.Insert(newdoc);

doc = new Dictionary<string, Object>();
doc.Add("title", "Сумка через плечо с кисточкой");
newdoc = new InsertDocumentRequest(index: "products", id: 2, doc: doc);
sqlresult = indexApi.Insert(newdoc);

doc = new Dictionary<string, Object>();
doc.Add("title", "Желтая сумка");
newdoc = new InsertDocumentRequest(index: "products", id: 0, doc: doc);
sqlresult = indexApi.Insert(newdoc);

```

<!-- end -->

## Авто-схема

> ЗАМЕТКА: Авто-схема требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если это не работает, убедитесь, что Buddy установлен.

Manticore имеет механизм автоматического создания таблиц, который активируется, когда указанная таблица в запросе на вставку еще не существует. Этот механизм включен по умолчанию. Чтобы отключить его, установите `auto_schema = 0` в разделе [Searchd](../../Server_settings/Searchd.md#auto_schema) вашего конфигурационного файла Manticore.

<!-- example auto-schema -->

По умолчанию все текстовые значения в клаузе `VALUES` рассматриваются как тип `text`, за исключением значений, представляющих действительные адреса электронной почты, которые обрабатываются как тип `string`.

Если вы попытаетесь ВСТАВИТЬ несколько строк с различными, несовместимыми типами значений для одного и того же поля, создание автоматической таблицы будет отменено, и будет возвращено сообщение об ошибке. Однако если различные типы значений совместимы, результирующий тип поля будет тот, который accommodates все значения. Некоторые автоматические преобразования типов данных, которые могут произойти, включают:
* mva -> mva64
* uint -> bigint -> float (это может привести к некоторой потере точности)
* string -> text

Также будут распознаны и преобразованы в метки времени следующие форматы дат, в то время как все другие форматы дат будут рассматриваться как строки:
- `%Y-%m-%dT%H:%M:%E*S%Z`
- `%Y-%m-%d'T'%H:%M:%S%Z`
- `%Y-%m-%dT%H:%M:%E*S`
- `%Y-%m-%dT%H:%M:%s`
- `%Y-%m-%dT%H:%M`
- `%Y-%m-%dT%H`


Имейте в виду, что HTTP-эндпоинт `/bulk` не поддерживает автоматическое создание таблиц (авто-схему). Только HTTP-эндпоинт `/_bulk` (похожий на Elasticsearch) и SQL интерфейс поддерживают эту функцию.

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

## Авто ID
<!-- example autoid -->
Manticore предоставляет функцию автоматической генерации ID для столбца ID документов, вставленных или замененных в реальном времени или [Перколируемой таблице](../../Creating_a_table/Local_tables/Percolate_table.md). Генератор производит уникальный ID для документа с некоторыми гарантиями, но его не следует считать автоинкрементным ID.
Сгенерированное значение ID гарантированно будет уникальным при следующих условиях:
* Значение [server_id](../../Server_settings/Searchd.md#server_id) текущего сервера находится в диапазоне от 0 до 127 и уникально среди узлов в кластере, или оно использует значение по умолчанию, сгенерированное из MAC-адреса в качестве семени
* Системное время не меняется для узла Manticore между перезагрузками сервера
* Авто ID генерируется менее 16 миллионов раз в секунду между перезагрузками поискового сервера

Генератор авто ID создает 64-битное целое число для ID документа и использует следующую схему:
* Биты с 0 по 23 формируют счетчик, который увеличивается при каждом вызове генератора авто ID
* Биты с 24 по 55 представляют временную метку Unix начала сервера
* Биты с 56 по 63 соответствуют server_id

Эта схема обеспечивает уникальность сгенерированного ID среди всех узлов в кластере и гарантирует, что данные, вставленные в разные узлы кластера, не создают коллизий между узлами.

В результате первый ID от генератора, используемого для авто ID, не равен 1, а является большим числом. Кроме того, поток документов, вставленных в таблицу, может иметь не последовательные значения ID, если вставки в другие таблицы происходят между вызовами, так как генератор ID является единым на сервере и общим для всех его таблиц.

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
### UUID_SHORT multi-ID generation

```sql
CALL UUID_SHORT(N)
```

Инструкция `CALL UUID_SHORT(N)` позволяет сгенерировать N уникальных 64-битных ID за один вызов без вставки каких-либо документов. Это особенно полезно, когда необходимо предварительно сгенерировать ID в Manticore для использования в других системах или решениях для хранения. Например, вы можете сгенерировать авто-ID в Manticore, а затем использовать их в другой базе данных, приложении или рабочем процессе, гарантируя согласованные и уникальные идентификаторы в разных окружениях.

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
## Массированное добавление документов
Вы можете вставлять не только один документ в реальное время, но и столько, сколько захотите. Совершенно нормально вставлять партии десятков тысяч документов в реальную таблицу. Однако важно помнить о следующих моментах:
* Чем больше партия, тем выше задержка каждой операции вставки
* Чем больше партия, тем выше ожидаемая скорость индексации
* Возможно, вам захочется увеличить значение [max_packet_size](../../Server_settings/Searchd.md#max_packet_size), чтобы разрешить большие партии
* Обычно каждая операция вставки партии считается одной [транзакцией](../../Data_creation_and_modification/Transactions.md) с гарантией атомарности, так что вы либо получите все новые документы в таблице сразу, либо, в случае сбоя, ни один из них не будет добавлен. Смотрите больше деталей о пустой строке или переходе к другой таблице в примере "JSON".

Обратите внимание, что HTTP-эндпоинт `/bulk` не поддерживает автоматическое создание таблиц (авто-схема). Только HTTP-эндпоинт `/_bulk` (похожий на Elasticsearch) и SQL-интерфейс поддерживают эту функцию. HTTP-эндпоинт `/_bulk` (похожий на Elasticsearch) позволяет названию таблицы включать имя кластера в формате `cluster_name:table_name`.
`/_bulk` конечная точка принимает идентификаторы документов в том же формате, что и Elasticsearch, и вы также можете включить `id` внутри самого документа:
```json
{ "index": { "table" : "products", "_id" : "1" } }
{ "title" : "Сумка через плечо с кисточкой", "price": 19.85 }
```

или

```json
{ "index": { "table" : "products" } }
{ "title" : "Сумка через плечо с кисточкой", "price": 19.85, "id": "1" }
```

#### Пакетная передача в /bulk
Конечная точка `/bulk` (в режиме Manticore) поддерживает [передачу в пакетах](https://en.wikipedia.org/wiki/Chunked_transfer_encoding). Вы можете использовать ее для передачи больших пакетов. Это:
* уменьшает пиковое использование ОЗУ, уменьшая риск OOM
* снижает время ответа
* позволяет вам обойти [max_packet_size](../Server_settings/Searchd.md#max_packet_size) и передавать пакеты, значительно превышающие максимальное допустимое значение `max_packet_size` (128MB), например, 1GB за раз.

<!-- intro -->
### Примеры пакетной вставки
##### SQL:
<!-- request SQL -->
Для пакетной вставки просто предоставьте больше документов в скобках после `VALUES()`. Синтаксис следующий:

```sql
INSERT INTO <table name>[(column1, column2, ...)] VALUES(value1[, value2 , ...]), (...)
```

Необязательный список имен столбцов позволяет вам явно указать значения для некоторых из столбцов, присутствующих в таблице. Все остальные столбцы будут заполнены их значениями по умолчанию (0 для скалярных типов, пустая строка для строковых типов).

Например:

```sql
INSERT INTO products(title,price) VALUES ('Сумка через плечо с кисточкой', 19.85), ('комплект простыней из микрофибры', 19.99), ('Перчатка для удаления шерсти домашних животных', 7.99);
```
<!-- response SQL -->

```sql
Запрос выполнен, 3 строки затронуты (0.01 сек)
```

Выражения в данный момент не поддерживаются в `INSERT`, и значения должны быть явно указаны.

<!-- intro -->
##### JSON:
<!-- request JSON -->
Синтаксис в общем такой же, как для [вставки одного документа](../../Quick_start_guide.md#Add-documents). Просто предоставьте больше строк, по одной на каждый документ, и используйте конечную точку `/bulk` вместо `/insert`. Оберните каждый документ в узел "insert". Обратите внимание, что это также требует:
* `Content-Type: application/x-ndjson`
* Данные должны быть отформатированы как JSON, разделенный новой строкой (NDJSON). По сути, это означает, что каждая строка должна содержать ровно одно JSON-выражение и заканчиваться новой строкой `\n` и, возможно, `\r`.

Конечная точка `/bulk` поддерживает запросы 'insert', 'replace', 'delete' и 'update'. Имейте в виду, что вы можете направлять операции на несколько таблиц, но транзакции возможны только для одной таблицы. Если вы укажете больше, Manticore соберет операции, направленные на одну таблицу, в одну транзакцию. Когда таблица меняется, она зафиксирует собранные операции и начнет новую транзакцию на новой таблице. Пустая строка, разделяющая пакеты, также приводит к фиксации предыдущего пакета и началу новой транзакции.

В ответе на запрос `/bulk` вы можете найти следующие поля:
* "errors": показывает, произошли ли какие-либо ошибки (true/false)
* "error": описывает произошедшую ошибку
* "current_line": номер строки, на которой выполнение остановилось (или завершилось неудачей); пустые строки, включая первую пустую строку, также учитываются
* "skipped_lines": количество незафиксированных строк, начиная с `current_line` и двигаясь назад

```json
POST /bulk
-H "Content-Type: application/x-ndjson" -d '
{"insert": {"table":"products", "id":1, "doc":  {"title":"Сумка через плечо с кисточкой","price" : 19.85}}}
{"insert":{"table":"products", "id":2, "doc":  {"title":"комплект простыней из микрофибры","price" : 19.99}}}
'

POST /bulk
-H "Content-Type: application/x-ndjson" -d '
{"insert":{"table":"test1","id":21,"doc":{"int_col":1,"price":1.1,"title":"пакетный документ один"}}}
{"insert":{"table":"test1","id":22,"doc":{"int_col":2,"price":2.2,"title":"пакетный документ два"}}}

{"insert":{"table":"test1","id":23,"doc":{"int_col":3,"price":3.3,"title":"пакетный документ три"}}}
{"insert":{"table":"test2","id":24,"doc":{"int_col":4,"price":4.4,"title":"пакетный документ четыре"}}}
{"insert":{"table":"test2","id":25,"doc":{"int_col":5,"price":5.5,"title":"пакетный документ пять"}}}
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

> ЗАМЕТКА: `_bulk` требует [Manticore Buddy](../Installation/Manticore_Buddy.md), если таблица еще не существует. Если это не сработает, убедитесь, что Buddy установлен.

```json
POST /_bulk
-H "Content-Type: application/x-ndjson" -d '
{ "index" : { "table" : "products" } }
{ "title" : "Желтая сумка", "price": 12 }
{ "create" : { "table" : "products" } }
{ "title" : "Красная сумка", "price": 12.5, "id": 3 }
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
        "result": "создано",
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
        "result": "создано",
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
        ['id' => 1, 'title' => 'Сумка через плечо с кисточкой', 'price' => 19.85],
        ['id' => 2, 'title' => 'Комплект простыней из микрофибры', 'price' => 19.99],
        ['id' => 3, 'title' => 'Перчатка для удаления шерсти домашних животных', 'price' => 7.99]
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
docs = [ \
    {"insert": {"table" : "products", "id" : 1, "doc" : {"title" : "Сумка через плечо с кисточкой", "price" : 19.85}}}, \
    {"insert": {"table" : "products", "id" : 2, "doc" : {"title" : "Комплект простыней из микрофибры", "price" : 19.99}}}, \
    {"insert": {"table" : "products", "id" : 3, "doc" : {"title" : "Перчатка для удаления шерсти домашних животных", "price" : 7.99}}}
]
res = indexApi.bulk('\n'.join(map(json.dumps,docs)))
```


<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
let docs = [
    {"insert": {"table" : "products", "id" : 3, "doc" : {"title" : "Сумка через плечо с кисточкой", "price" : 19.85}}},
    {"insert": {"table" : "products", "id" : 4, "doc" : {"title" : "Комплект простыней из микрофибры", "price" : 19.99}}},
    {"insert": {"table" : "products", "id" : 5, "doc" : {"title" : "Перчатка для удаления шерсти домашних животных", "price" : 7.99}}}
];
res =  await indexApi.bulk(docs.map(e=>JSON.stringify(e)).join('\n'));
```
<!-- intro -->
##### java:

<!-- request Java -->

``` java
String body = "{\"insert\": {\"index\" : \"products\", \"id\" : 1, \"doc\" : {\"title\" : \"Сумка через плечо с кисточкой\", \"price\" : 19.85}}}"+"\n"+
    "{\"insert\": {\"index\" : \"products\", \"id\" : 4, \"doc\" : {\"title\" : \"Комплект простыней из микрофибры\", \"price\" : 19.99}}}"+"\n"+
    "{\"insert\": {\"index\" : \"products\", \"id\" : 5, \"doc\" : {\"title\" : \"Перчатка для удаления шерсти домашних животных\", \"price\" : 7.99}}}"+"\n";
BulkResponse bulkresult = indexApi.bulk(body);
```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
string body = "{\"insert\": {\"index\" : \"products\", \"id\" : 1, \"doc\" : {\"title\" : \"Сумка через плечо с кисточкой\", \"price\" : 19.85}}}"+"\n"+
    "{\"insert\": {\"index\" : \"products\", \"id\" : 4, \"doc\" : {\"title\" : \"Комплект простыней из микрофибры\", \"price\" : 19.99}}}"+"\n"+
    "{\"insert\": {\"index\" : \"products\", \"id\" : 5, \"doc\" : {\"title\" : \"Перчатка для удаления шерсти домашних животных\", \"price\" : 7.99}}}"+"\n";
BulkResponse bulkresult = indexApi.Bulk(string.Join("\n", docs));
```

<!-- end -->
<!-- example MVA_insert -->
## Вставка значений многоразовых атрибутов (MVA)

Многоразовые атрибуты (MVA) вставляются как массивы чисел.
<!-- intro -->
### Примеры
##### SQL
<!-- request SQL -->
```

INSERT INTO products(title, sizes) VALUES('обувь', (40,41,42,43));
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
    "title" : "обувь",
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
  "title": "обувь",
  "sizes" : [40, 41, 42, 43]
}
```

Or, alternatively
```json
POST /products/_doc/
{
  "title": "обувь",
  "sizes" : [40, 41, 42, 43]
}
```


<!-- intro -->
##### PHP
<!-- request PHP -->

```php
$index->addDocument(
  ['title' => 'обувь', 'sizes' => [40,41,42,43]],
  1
);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "Желтая сумка","sizes":[40,41,42,43]}})
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "Желтая сумка","sizes":[40,41,42,43]}});
```


<!-- intro -->
##### java:

<!-- request Java -->

``` java
newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","Желтая сумка");
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
doc.Add("title", "Желтая сумка");
doc.Add("sizes", new List<Object> {40,41,42,43});
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", id: 0, doc: doc);
var sqlresult = indexApi.Insert(newdoc);
```
<!-- end -->

<!-- example JSON_insert -->
## Вставка JSON
Значение JSON может быть вставлено как [экранированная](../../Searching/Full_text_matching/Escaping.md) строка (через SQL или JSON) или как объект JSON (через интерфейс JSON).

<!-- intro -->
### Примеры
##### SQL
<!-- request SQL -->
```

INSERT INTO products VALUES (1, 'обувь', '{"size": 41, "color": "красный"}');
```
<!-- intro -->
##### JSON
<!-- request JSON -->
Значение JSON может быть вставлено как объект JSON
```json
POST /insert
{
  "table":"products",
  "id":1,
  "doc":
  {
    "title" : "обувь",
    "meta" : {
      "size": 41,
      "color": "красный"
    }
  }
}
```

JSON значение также может быть вставлено как строка, содержащая экранированный JSON:
```json
POST /insert
{
  "table":"products",
  "id":1,
  "doc":
  {
    "title" : "обувь",
    "meta" : "{\"size\": 41, \"color\": \"красный\"}"
  }
}
```

<!-- intro -->
##### Elasticsearch:
<!-- request Elasticsearch -->

```json
POST /products/_create/1
{
  "title": "обувь",
  "meta" : {
    "size": 41,
    "color": "красный"
  }
}
```

Или, альтернативно
```json
POST /products/_doc/
{
  "title": "обувь",
  "meta" : {
    "size": 41,
    "color": "красный"
  }
}
```


<!-- intro -->
##### PHP
Рассматривать JSON просто как строку:
<!-- request PHP -->

```php
$index->addDocument(
  ['title' => 'обувь', 'meta' => '{"size": 41, "color": "красный"}'],
  1
);
```
<!-- intro -->
##### Python:

<!-- request Python -->
``` python
indexApi = api = manticoresearch.IndexApi(client)
indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "Желтая сумка","meta":'{"size": 41, "color": "красный"}'}})
```
<!-- intro -->
##### Javascript:

<!-- request Javascript -->
```javascript

res = await indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "Желтая сумка","meta":'{"size": 41, "color": "красный"}'}});
```
<!-- intro -->
##### java:

<!-- request Java -->

``` java
newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","Желтая сумка");
    put("meta",
        new HashMap<String,Object>(){{
            put("size",41);
            put("color","красный");
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
meta.Add("color", "красный");
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "Желтая сумка");
doc.Add("meta", meta);
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", id: 0, doc: doc);
var sqlresult = indexApi.Insert(newdoc);
```

<!-- end -->

<!-- proofread -->


