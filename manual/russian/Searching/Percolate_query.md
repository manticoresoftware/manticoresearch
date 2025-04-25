# Перколировать запрос

Запросы на перколирование также известны как постоянные запросы, перспективный поиск, маршрутизация документов, поиск наоборот и обратный поиск.

Традиционный способ проведения поисков включает в себя хранение документов и выполнение поисковых запросов по ним. Однако есть случаи, когда мы хотим применить запрос к вновь поступившему документу, чтобы сигнализировать о совпадении. Некоторые сценарии, где это необходимо, включают в себя системы мониторинга, которые собирают данные и уведомляют пользователей о конкретных событиях, таких как достижение определенного порога для метрики или появление конкретного значения в отслеживаемых данных. Другой пример - агрегирование новостей, где пользователи могут хотеть получать уведомления только о определенных категориях или темах, или даже конкретных «ключевых словах».

В этих ситуациях традиционный поиск не является наилучшим вариантом, так как предполагает, что желаемый поиск выполняется по всей коллекции. Этот процесс умножается на количество пользователей, что приводит к множеству запросов по всей коллекции, что может вызвать значительную дополнительную нагрузку. Альтернативный подход, описанный в этом разделе, заключается в хранении запросов и их проверке на новом входящем документе или пакете документов.

Google Alerts, AlertHN, Bloomberg Terminal и другие системы, которые позволяют пользователям подписываться на конкретный контент, используют аналогичную технологию.

> * См. [перколировать](../Creating_a_table/Local_tables/Percolate_table.md) для получения информации о создании таблицы PQ.
> * См. [Добавление правил в таблицу перколирования](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md), чтобы узнать, как добавлять правила перколирования (также известные как правила PQ). Вот быстрый пример:


### Выполнение запроса на перколирование с CALL PQ

Главное, что нужно помнить о запросах на перколирование, это то, что ваши поисковые запросы уже в таблице. Вам нужно предоставить документы **для проверки, совпадает ли какой-либо из них с любым из хранимых правил**.

Вы можете выполнить запрос на перколирование через интерфейсы SQL или JSON, а также с использованием клиентов языков программирования. Подход на основе SQL предоставляет больше гибкости, в то время как метод HTTP проще и обеспечивает большинство ваших потребностей. Таблица ниже может помочь вам понять различия.

| Желаемое поведение         | SQL                                     | HTTP                                 |
| --------------------------- | --------------------------------------- | ------------------------------------ |
| Предоставить один документ  | `CALL PQ('tbl', '{doc1}')`              | `query.percolate.document{doc1}`     |
| Предоставить один документ (альтернатива) | `CALL PQ('tbl', 'doc1', 0 as docs_json)` | - |
| Предоставить несколько документов | `CALL PQ('tbl', ('doc1', 'doc2'), 0 as docs_json)` | - |
| Предоставить несколько документов (альтернатива) | `CALL PQ('tbl', ('{doc1}', '{doc2}'))` | - |
| Предоставить несколько документов (альтернатива) | `CALL PQ('tbl', '[{doc1}, {doc2}]')` | - |
| Вернуть идентификаторы совпадающих документов | 0/1 как docs (отключено по умолчанию) | Включено по умолчанию                   |
| Использовать собственный идентификатор документа для отображения в результате | 'id field' как docs_id (отключено по умолчанию) | Не доступно |
| Рассматривать входные документы как JSON | 1 как docs_json (1 по умолчанию) | Включено по умолчанию |
| Рассматривать входные документы как простой текст | 0 как docs_json (1 по умолчанию) | Не доступно |
| [Разреженный режим распределения](../Searching/Percolate_query.md#I-want-higher-performance-of-a-percolate-query) | по умолчанию | по умолчанию |
| [Шардированный режим распределения](../Searching/Percolate_query.md#I-want-higher-performance-of-a-percolate-query) | шардированный как режим | Не доступно |
| Вернуть всю информацию о совпадающем запросе | 1 как query (0 по умолчанию) | Включено по умолчанию |
| Пропустить недействительный JSON | 1 как skip_bad_json (0 по умолчанию) | Не доступно |
| Расширенная информация в [SHOW META](../Node_info_and_management/SHOW_META.md) | 1 как verbose (0 по умолчанию) | Не доступно |
| Определить номер, который будет добавлен к идентификаторам документов, если поля docs_id не предоставлены (в основном актуально в [распределенных режимах PQ](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#Distributed-percolate-tables-%28DPQ-tables%29)) | 1 как shift (0 по умолчанию) | Не доступно |

<!-- example create percolate -->
Для демонстрации того, как это работает, вот несколько примеров. Давайте создадим таблицу PQ с двумя полями:

* заголовок (текст)
* цвет (строка)

и тремя правилами в ней:

* Просто полный текст. Запрос: `@title bag`
* Полный текст и фильтрация. Запрос: `@title shoes`. Фильтры: `color='red'`
* Полный текст и более сложная фильтрация. Запрос: `@title shoes`. Фильтры: `color IN('blue', 'green')`

<!-- intro -->
#### SQL
<!-- request SQL -->

```sql
CREATE TABLE products(title text, color string) type='pq';
INSERT INTO products(query) values('@title bag');
INSERT INTO products(query,filters) values('@title shoes', 'color='red'');
INSERT INTO products(query,filters) values('@title shoes', 'color in ('blue', 'green')');
select * from products;
```

<!-- response SQL -->

```sql
+---------------------+--------------+------+---------------------------+
| id                  | query        | tags | filters                   |
+---------------------+--------------+------+---------------------------+
| 1657852401006149635 | @title shoes |      | color IN ('blue, 'green') |
| 1657852401006149636 | @title shoes |      | color='red'               |
| 1657852401006149637 | @title bag   |      |                           |
+---------------------+--------------+------+---------------------------+
```

<!-- request JSON -->

```json
PUT /pq/products/doc/
{
  "query": {
    "match": {
      "title": "bag"
    }
  },
  "filters": ""
}

PUT /pq/products/doc/
{
  "query": {
    "match": {
      "title": "shoes"
    }
  },
  "filters": "color='red'"
}

PUT /pq/products/doc/
{
  "query": {
    "match": {
      "title": "shoes"
    }
  },
  "filters": "цвет IN ('синий', 'зеленый')"
}
```

<!-- intro -->
#### JSON
<!-- response JSON -->

```json
{
  "table": "продукты",
  "type": "doc",
  "_id": 1657852401006149661,
  "result": "создано"
}
{
  "table": "продукты",
  "type": "doc",
  "_id": 1657852401006149662,
  "result": "создано"
}
{
  "table": "продукты",
  "type": "doc",
  "_id": 1657852401006149663,
  "result": "создано"
}
```

<!-- intro -->
#### PHP
<!-- request PHP -->

```php

$index = [
    'table' => 'продукты',
    'body' => [
        'columns' => [
            'title' => ['type' => 'text'],
            'color' => ['type' => 'string']
        ],
        'settings' => [
            'type' => 'pq'
        ]
    ]
];
$client->indices()->create($index);

$query = [
    'table' => 'продукты',
    'body' => [ 'query'=>['match'=>['title'=>'сумка']]]
];
$client->pq()->doc($query);
$query = [
    'table' => 'продукты',
    'body' => [ 'query'=>['match'=>['title'=>'обувь']],'filters'=>"цвет='красный'"]
];
$client->pq()->doc($query);


$query = [
    'table' => 'продукты',
    'body' => [ 'query'=>['match'=>['title'=>'обувь']],'filters'=>"цвет IN ('синий', 'зеленый')"]
];
$client->pq()->doc($query);
```
<!-- response PHP -->
``` php
Array(
  [table] => продукты
  [type] => doc
  [_id] => 1657852401006149661
  [result] => создано
)
Array(
  [table] => продукты
  [type] => doc
  [_id] => 1657852401006149662
  [result] => создано
)
Array(
  [table] => продукты
  [type] => doc
  [_id] => 1657852401006149663
  [result] => создано
)
```
<!-- intro -->
Python
<!-- request Python -->

```python
utilsApi.sql('create table продукты(title text, color string) type='pq'')
indexApi.insert({"table" : "продукты", "doc" : {"query" : "@title сумка" }})
indexApi.insert({"table" : "продукты",  "doc" : {"query" : "@title обувь", "filters": "цвет='красный'" }})
indexApi.insert({"table" : "продукты",  "doc" : {"query" : "@title обувь","filters": "цвет IN ('синий', 'зеленый')" }})
```
<!-- response Python -->
``` python
{'created': True,
 'found': None,
 'id': 0,
 'table': 'продукты',
 'result': 'создано'}
{'created': True,
 'found': None,
 'id': 0,
 'table': 'продукты',
 'result': 'создано'}
{'created': True,
 'found': None,
 'id': 0,
 'table': 'продукты',
 'result': 'создано'}
```

<!-- intro -->
Python-asyncio
<!-- request Python-asyncio -->

```python
await utilsApi.sql('create table продукты(title text, color string) type='pq'')
await indexApi.insert({"table" : "продукты", "doc" : {"query" : "@title сумка" }})
await indexApi.insert({"table" : "продукты",  "doc" : {"query" : "@title обувь", "filters": "цвет='красный'" }})
await indexApi.insert({"table" : "продукты",  "doc" : {"query" : "@title обувь","filters": "цвет IN ('синий', 'зеленый')" }})
```
<!-- response Python-asyncio -->
``` python
{'created': True,
 'found': None,
 'id': 0,
 'table': 'продукты',
 'result': 'создано'}
{'created': True,
 'found': None,
 'id': 0,
 'table': 'продукты',
 'result': 'создано'}
{'created': True,
 'found': None,
 'id': 0,
 'table': 'продукты',
 'result': 'создано'}
```

<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await utilsApi.sql('create table продукты(title text, color string) type='pq'');
res = indexApi.insert({"table" : "продукты", "doc" : {"query" : "@title сумка" }});
res = indexApi.insert({"table" : "продукты",  "doc" : {"query" : "@title обувь", "filters": "цвет='красный'" }});
res = indexApi.insert({"table" : "продукты",  "doc" : {"query" : "@title обувь","filters": "цвет IN ('синий', 'зеленый')" }});
```
<!-- response javascript -->
``` javascript
"table":"продукты","_id":0,"created":true,"result":"создано"}
{"table":"продукты","_id":0,"created":true,"result":"создано"}
{"table":"продукты","_id":0,"created":true,"result":"создано"}
```
<!-- intro -->
java
<!-- request Java -->

```java
utilsApi.sql("create table продукты(title text, color string) type='pq'", true);
doc = new HashMap<String,Object>(){{
    put("query", "@title сумка");
}};
newdoc = new InsertDocumentRequest();
newdoc.index("продукты").setDoc(doc);
indexApi.insert(newdoc);

doc = new HashMap<String,Object>(){{
    put("query", "@title обувь");
    put("filters", "цвет='красный'");
}};
newdoc = new InsertDocumentRequest();
newdoc.index("продукты").setDoc(doc);
indexApi.insert(newdoc);

doc = new HashMap<String,Object>(){{
    put("query", "@title обувь");
    put("filters", "цвет IN ('синий', 'зеленый')");
}};
newdoc = new InsertDocumentRequest();
newdoc.index("продукты").setDoc(doc);
indexApi.insert(newdoc);
```
<!-- response Java -->
``` java
{total=0, error=, warning=}
class SuccessResponse {
    index: продукты
    id: 0
    created: true
    result: создано
    found: null
}
class SuccessResponse {
    index: продукты
    id: 0
    created: true
    result: создано
    found: null
}
class SuccessResponse {
    index: продукты
    id: 0
    created: true
    result: создано
    found: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
utilsApi.Sql("create table продукты(title text, color string) type='pq'", true);

Dictionary<string, Object> doc = new Dictionary<string, Object>(); 
doc.Add("query", "@title сумка");
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "продукты", doc: doc);
indexApi.Insert(newdoc);

doc = new Dictionary<string, Object>(); 
doc.Add("query", "@title обувь");
doc.Add("filters", "цвет='красный'");
newdoc = new InsertDocumentRequest(index: "продукты", doc: doc);
indexApi.Insert(newdoc);

doc = new Dictionary<string, Object>(); 
doc.Add("query", "@title bag");
doc.Add("filters", "color IN ('blue', 'green')");
newdoc = new InsertDocumentRequest(index: "products", doc: doc);
indexApi.Insert(newdoc);
```
<!-- response C# -->
``` clike
{total=0, error="", warning=""}

class SuccessResponse {
    index: products
    id: 0
    created: true
    result: created
    found: null
}
class SuccessResponse {
    index: products
    id: 0
    created: true
    result: created
    found: null
}
class SuccessResponse {
    index: products
    id: 0
    created: true
    result: created
    found: null
}

```

<!-- intro -->
Rust
<!-- request Rust -->

```rust
utils_api.sql("create table products(title text, color string) type='pq'", Some(true)).await;

let mut doc1 = HashMap::new();
doc1.insert("query".to_string(), serde_json::json!("@title bag"));
let insert_req1 = InsertDocumentRequest::new("products".to_string(), serde_json::json!(doc1));
index_api.insert(insert_req1).await;

let mut doc2 = HashMap::new();
doc2.insert("query".to_string(), serde_json::json!("@title shoes"));
doc2.insert("filters".to_string(), serde_json::json!("color='red'"));
let insert_req2 = InsertDocumentRequest::new("products".to_string(), serde_json::json!(doc2));
index_api.insert(insert_req2).await;

let mut doc3 = HashMap::new();
doc3.insert("query".to_string(), serde_json::json!("@title bag"));
doc3.insert("filters".to_string(), serde_json::json!("color IN ('blue', 'green')"));
let insert_req3 = InsertDocumentRequest::new("products".to_string(), serde_json::json!(doc3));
index_api.insert(insert_req3).await;
```
<!-- response Rust -->
``` rust
{total=0, error="", warning=""}

class SuccessResponse {
    index: products
    id: 0
    created: true
    result: created
    found: null
}
class SuccessResponse {
    index: products
    id: 0
    created: true
    result: created
    found: null
}
class SuccessResponse {
    index: products
    id: 0
    created: true
    result: created
    found: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->
```typescript
res = await utilsApi.sql("create table test_pq(title text, color string) type='pq'");
res = indexApi.insert({
  index: 'test_pq',
  doc: { query : '@title bag' }
});
res = indexApi.insert(
  index: 'test_pq',
  doc: { query: '@title shoes', filters: "color='red'" }
});
res = indexApi.insert({
  index: 'test_pq',
  doc: { query : '@title shoes', filters: "color IN ('blue', 'green')" }
});
```
<!-- response TypeScript -->
``` typescript
{

"table":"test_pq",

"_id":1657852401006149661,

"created":true,

"result":"created"
}
{

"table":"test_pq",

"_id":1657852401006149662,

"created":true,

"result":"created"
}
{

"table":"test_pq",

"_id":1657852401006149663,

"created":true,

"result":"created"
}
```

<!-- intro -->
Go
<!-- request Go -->
```go
apiClient.UtilsAPI.Sql(context.Background()).Body("create table test_pq(title text, color string) type='pq'").Execute()

indexDoc := map[string]interface{} {"query": "@title bag"}
indexReq := manticoreclient.NewInsertDocumentRequest("test_pq", indexDoc)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute();

indexDoc = map[string]interface{} {"query": "@title shoes", "filters": "color='red'"}
indexReq = manticoreclient.NewInsertDocumentRequest("test_pq", indexDoc)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute();

indexDoc = map[string]interface{} {"query": "@title shoes", "filters": "color IN ('blue', 'green')"}
indexReq = manticoreclient.NewInsertDocumentRequest("test_pq", indexDoc)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute();
```
<!-- response Go -->
``` go
{

"table":"test_pq",

"_id":1657852401006149661,

"created":true,

"result":"created"
}
{

"table":"test_pq",

"_id":1657852401006149662,

"created":true,

"result":"created"
}
{

"table":"test_pq",

"_id":1657852401006149663,

"created":true,

"result":"created"
}
```

<!-- end -->

<!-- example single -->
##### Just tell me what PQ rules match my single document

The first document doesn't match any rules. It could match the first two, but they require additional filters.

The second document matches one rule. Note that CALL PQ by default expects a document to be a JSON, but if you use `0 as docs_json`, you can pass a plain string instead.

<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products', 'Beautiful shoes', 0 as docs_json);

CALL PQ('products', 'What a nice bag', 0 as docs_json);
CALL PQ('products', '{"title": "What a nice bag"}');
```
<!-- response SQL -->

```sql
+---------------------+
| id                  |
+---------------------+
| 1657852401006149637 |
+---------------------+

+---------------------+
| id                  |
+---------------------+
| 1657852401006149637 |
+---------------------+
```
<!-- intro -->
JSON:
<!-- request JSON -->

```json
POST /pq/products/_search
{
  "query": {
    "percolate": {
      "document": {
        "title": "What a nice bag"
      }
    }
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
    "max_score": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149644,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
PHP:
<!-- request PHP -->

```php
$percolate = [
    'table' => 'products',
    'body' => [
        'query' => [
            'percolate' => [
                'document' => [
                    'title' => 'Какой красивый сумка'
                ]
            ]
        ]
    ]
];
$client->pq()->search($percolate);

```
<!-- response PHP -->

```php
Array
(
    [took] => 0
    [timed_out] =>
    [hits] => Array
        (
            [total] => 1
            [max_score] => 1
            [hits] => Array
                (
                    [0] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 1657852401006149644
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => bag
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 1
                                        )
                                )
                        )
                )
        )
)
```
<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"Какой красивый сумка"}}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title bag'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- intro -->
Python-asyncio
<!-- request Python-asyncio -->

```python
await searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"Какой красивый сумка"}}}})
```
<!-- response Python-asyncio -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title bag'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"Какой красивый сумка"}}}});
```
<!-- response javascript -->
``` javascript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233808,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
java
<!-- request Java -->

```java
PercolateRequest percolateRequest = new PercolateRequest();
query = new HashMap<String,Object>(){{
    put("percolate",new HashMap<String,Object >(){{
        put("document", new HashMap<String,Object >(){{
            put("title","Какой красивый сумка");
        }});
    }});
}};
percolateRequest.query(query);
searchApi.percolate("test_pq",percolateRequest);
```
<!-- response Javs -->
``` java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234109, _score=1, _source={query={ql=@title bag}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
Dictionary<string, Object> percolateDoc = new Dictionary<string, Object>(); 
percolateDoc.Add("document", new Dictionary<string, Object> {{ "title", "Какой красивый сумка" }});
Dictionary<string, Object> query = new Dictionary<string, Object> {{ "percolate", percolateDoc }}; 
PercolateRequest percolateRequest = new PercolateRequest(query=query);
searchApi.Percolate("test_pq",percolateRequest);

```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234109, _score=1, _source={query={ql=@title bag}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
Rust
<!-- request Rust -->

```rust
let mut percolate_doc_fields = HashMap::new();
percolate_doc_fileds.insert("title".to_string(), "Какой красивый сумка");
let mut percolate_doc = HashMap::new();
percolate_doc.insert("document".to_string(), percolate_doc_fields); 
let percolate_query = PercolateRequestQuery::new(serde_json::json!(percolate_doc));
let percolate_req = PercolateRequest::new(percolate_query); 
search_api.percolate("test_pq", percolate_req).await;
```
<!-- response Rust -->
``` rust
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234109, _score=1, _source={query={ql=@title bag}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
res = await searchApi.percolate('test_pq', { query: { percolate: { document : { title : 'Что за прекрасная сумка' } } } } );
```
<!-- response TypeScript -->
``` typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
query := map[string]interface{} {"title": "что за прекрасная сумка"}
percolateRequestQuery := manticoreclient.NewPercolateQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery)
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()

```
<!-- response Go -->
``` go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- end -->

<!-- example pq_rules -->
##### I want to know complete PQ rules matching my document
<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products', '{"title": "Что за прекрасная сумка"}', 1 as query);
```
<!-- response SQL -->

```sql
+---------------------+------------+------+---------+
| id                  | query      | tags | filters |
+---------------------+------------+------+---------+
| 1657852401006149637 | @title bag |      |         |
+---------------------+------------+------+---------+
```
<!-- intro -->
JSON:
<!-- request JSON -->

```json
POST /pq/products/_search
{
  "query": {
    "percolate": {
      "document": {
        "title": "Что за прекрасная сумка"
      }
    }
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
    "max_score": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149644,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
PHP:
<!-- request PHP -->

```php
$percolate = [
    'table' => 'products',
    'body' => [
        'query' => [
            'percolate' => [
                'document' => [
                    'title' => 'Что за прекрасная сумка'
                ]
            ]
        ]
    ]
];
$client->pq()->search($percolate);

```
<!-- response PHP -->

```php
Array
(
    [took] => 0
    [timed_out] =>
    [hits] => Array
        (
            [total] => 1
            [max_score] => 1
            [hits] => Array
                (
                    [0] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 1657852401006149644
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => bag
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 1
                                        )
                                )
                        )
                )
        )
)
```

<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"Что за прекрасная сумка"}}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title bag'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- intro -->
Python-asyncio
<!-- request Python-asyncio -->

```python
await searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"Какой красивый рюкзак"}}}})
```
<!-- response Python-asyncio -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title рюкзак'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"Какой красивый рюкзак"}}}});
```
<!-- response javascript -->
``` javascript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233808,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title рюкзак"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
java
<!-- request Java -->

```java
PercolateRequest percolateRequest = new PercolateRequest();
query = new HashMap<String,Object>(){{
    put("percolate",new HashMap<String,Object >(){{
        put("document", new HashMap<String,Object >(){{
            put("title","какой красивый рюкзак");
        }});
    }});
}};
percolateRequest.query(query);
searchApi.percolate("test_pq",percolateRequest);
```
<!-- response Java -->
``` java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234109, _score=1, _source={query={ql=@title рюкзак}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
Dictionary<string, Object> percolateDoc = new Dictionary<string, Object>(); 
percolateDoc.Add("document", new Dictionary<string, Object> {{ "title", "какой красивый рюкзак" }});
Dictionary<string, Object> query = new Dictionary<string, Object> {{ "percolate", percolateDoc }}; 
PercolateRequest percolateRequest = new PercolateRequest(query=query);
searchApi.Percolate("test_pq",percolateRequest);

```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234109, _score=1, _source={query={ql=@title рюкзак}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
Rust
<!-- request Rust -->

```rust
let mut percolate_doc_fields = HashMap::new();
percolate_doc_fileds.insert("title".to_string(), "какой красивый рюкзак");
let mut percolate_doc = HashMap::new();
percolate_doc.insert("document".to_string(), percolate_doc_fields); 
let percolate_query = PercolateRequestQuery::new(serde_json::json!(percolate_doc));
let percolate_req = PercolateRequest::new(percolate_query); 
search_api.percolate("test_pq", percolate_req).await;

```
<!-- response Rust -->
``` rust
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234109, _score=1, _source={query={ql=@title рюкзак}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
res = await searchApi.percolate('test_pq', { query: { percolate: { document : { title : 'Какой красивый рюкзак' } } } } );
```
<!-- response TypeScript -->
``` typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title рюкзак"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
query := map[string]interface{} {"title": "какой красивый рюкзак"}
percolateRequestQuery := manticoreclient.NewPercolateQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery)
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()

```
<!-- response Go -->
``` go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title рюкзак"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- end -->

<!-- example multiple -->

##### Как насчет нескольких документов?
Обратите внимание, что с помощью `CALL PQ` вы можете предоставить несколько документов различными способами:

* в виде массива текстовых документов в круглых скобках `('doc1', 'doc2')`. Это требует `0 как docs_json`

* в виде массива JSON в круглых скобках `('{doc1}', '{doc2}')`

* или как стандартный JSON массив `'[{doc1}, {doc2}]'`

<!-- intro -->

SQL:

<!-- request SQL -->

```sql

CALL PQ('products', ('nice pair of shoes', 'beautiful bag'), 1 as query, 0 as docs_json);

CALL PQ('products', ('{"title": "nice pair of shoes", "color": "red"}', '{"title": "beautiful bag"}'), 1 as query);

CALL PQ('products', '[{"title": "nice pair of shoes", "color": "blue"}, {"title": "beautiful bag"}]', 1 as query);

```

<!-- response SQL -->

```sql

+---------------------+------------+------+---------+
| id                  | query      | tags | filters |
+---------------------+------------+------+---------+
| 1657852401006149637 | @title bag |      |         |
+---------------------+------------+------+---------+

+---------------------+--------------+------+-------------+
| id                  | query        | tags | filters     |
+---------------------+--------------+------+-------------+
| 1657852401006149636 | @title shoes |      | color='red' |
| 1657852401006149637 | @title bag   |      |             |
+---------------------+--------------+------+-------------+

+---------------------+--------------+------+---------------------------+
| id                  | query        | tags | filters                   |
+---------------------+--------------+------+---------------------------+
| 1657852401006149635 | @title shoes |      | color IN ('blue, 'green') |
| 1657852401006149637 | @title bag   |      |                           |
+---------------------+--------------+------+---------------------------+

```

<!-- intro -->

JSON:

<!-- request JSON -->

```json

POST /pq/products/_search

{
  "query": {
    "percolate": {
      "documents": [
        {"title": "nice pair of shoes", "color": "blue"},
        {"title": "beautiful bag"}
      ]
    }
  }
}

```

<!-- response JSON -->

```json

{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "max_score": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149644,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            2
          ]
        }
      },
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149646,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}

```

<!-- intro -->

PHP:

<!-- request PHP -->

```php

$percolate = [
    'table' => 'products',
    'body' => [
        'query' => [
            'percolate' => [
                'documents' => [
                    ['title' => 'nice pair of shoes','color'=>'blue'],
                    ['title' => 'beautiful bag']
                ]
            ]
        ]
    ]
];
$client->pq()->search($percolate);

```

<!-- response PHP -->

```php

Array
(
    [took] => 23
    [timed_out] =>
    [hits] => Array
        (
            [total] => 2
            [max_score] => 1
            [hits] => Array
                (
                    [0] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 2810781492890828819
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => bag
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 2
                                        )
                                )
                        )
                    [1] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 2810781492890828821
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => shoes
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 1
                                        )
                                )
                        )
                )
        )
)
```
<!-- intro -->
Питон
<!-- request Python -->

```python
searchApi.percolate('products',{"query":{"percolate":{"documents":[{"title":"nice pair of shoes","color":"blue"},{"title":"beautiful bag"}]}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381494',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title bag'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [2]}},
                   {u'_id': u'2811025403043381496',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title shoes'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}

```

<!-- intro -->
Python-asyncio
<!-- request Python-asyncio -->

```python
await searchApi.percolate('products',{"query":{"percolate":{"documents":[{"title":"nice pair of shoes","color":"blue"},{"title":"beautiful bag"}]}}})
```
<!-- response Python-asyncio -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381494',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title bag'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [2]}},
                   {u'_id': u'2811025403043381496',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title shoes'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}

```

<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate('products',{"query":{"percolate":{"documents":[{"title":"nice pair of shoes","color":"blue"},{"title":"beautiful bag"}]}}});
```
<!-- response javascript -->
``` javascript
{
  "took": 6,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233808,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            2
          ]
        }
      },
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233810,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}

```

<!-- intro -->
java
<!-- request Java -->

```java
percolateRequest = new PercolateRequest();
query = new HashMap<String,Object>(){{
        put("percolate",new HashMap<String,Object >(){{
            put("documents", new ArrayList<Object>(){{
                    add(new HashMap<String,Object >(){{
                        put("title","nice pair of shoes");
                        put("color","blue");
                    }});
                    add(new HashMap<String,Object >(){{
                        put("title","beautiful bag");

                    }});

                     }});
        }});
    }};
percolateRequest.query(query);
searchApi.percolate("products",percolateRequest);

```
<!-- response Java -->
``` java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234133, _score=1, _source={query={ql=@title bag}}, fields={_percolator_document_slot=[2]}}, {_index=products, _type=doc, _id=2811045522851234135, _score=1, _source={query={ql=@title shoes}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
var doc1 = new Dictionary<string, Object>();
doc1.Add("title","nice pair of shoes");
doc1.Add("color","blue");
var doc2 = new Dictionary<string, Object>();
doc2.Add("title","beautiful bag");
var docs = new List<Object> {doc1, doc2};
Dictionary<string, Object> percolateDoc = new Dictionary<string, Object> {{ "documents", docs }}; 
Dictionary<string, Object> query = new Dictionary<string, Object> {{ "percolate", percolateDoc }}; 
PercolateRequest percolateRequest = new PercolateRequest(query=query);
searchApi.Percolate("products",percolateRequest);
```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234133, _score=1, _source={query={ql=@title bag}}, fields={_percolator_document_slot=[2]}}, {_index=products, _type=doc, _id=2811045522851234135, _score=1, _source={query={ql=@title shoes}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
Rust
<!-- request Rust -->

```rust
let mut percolate_doc_fields1 = HashMap::new();
percolate_doc_fields1.insert("title".to_string(), "nice pair of shoes");
percolate_doc_fields1.insert("color".to_string(), "blue");
let mut percolate_doc_fields2 = HashMap::new();
percolate_doc_fields2.insert("title".to_string(), "красивый пакет");
let mut percolate_doc_fields_list: [HashMap; 2] = [percolate_doc_fields1, percolate_doc_fields2];
let mut percolate_doc = HashMap::new();
percolate_doc.insert("documents".to_string(), percolate_doc_fields_list); 
let percolate_query = PercolateRequestQuery::new(serde_json::json!(percolate_doc));
let percolate_req = PercolateRequest::new(percolate_query); 
search_api.percolate("products", percolate_req).await;
```
<!-- response Rust -->
``` rust
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234133, _score=1, _source={query={ql=@title пакет}}, fields={_percolator_document_slot=[2]}}, {_index=products, _type=doc, _id=2811045522851234135, _score=1, _source={query={ql=@title обувь}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
docs = [ {title : 'Какой красивый пакет'}, {title : 'Действительно красивые туфли'} ]; 
res = await searchApi.percolate('test_pq', { query: { percolate: { documents : docs } } } );
```
<!-- response TypeScript -->
``` typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title пакет"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title обувь"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
doc1 := map[string]interface{} {"title": "Какой красивый пакет"}
doc2 := map[string]interface{} {"title": "Действительно красивые туфли"}
query := []interface{} {doc1, doc2}
percolateRequestQuery := manticoreclient.NewPercolateQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery)
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()

```
<!-- response Go -->
``` go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title пакет"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title обувь"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- end -->

<!-- example docs_1 -->
##### Я хочу знать, какие документы соответствуют каким правилам

Использование опции `1 as docs` позволяет вам увидеть, какие из предоставленных документов соответствуют каким правилам.
<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products', '[{"title": "красивые туфли", "color": "синий"}, {"title": "красивый пакет"}]', 1 as query, 1 as docs);
```

<!-- response SQL -->

```sql
+---------------------+-----------+--------------+------+---------------------------+
| id                  | documents | query        | tags | filters                   |
+---------------------+-----------+--------------+------+---------------------------+
| 1657852401006149635 | 1         | @title обувь  |      | color IN ('синий, 'зеленый') |
| 1657852401006149637 | 2         | @title пакет   |      |                           |
+---------------------+-----------+--------------+------+---------------------------+
```
<!-- intro -->
JSON:
<!-- request JSON -->

```json
POST /pq/products/_search
{
  "query": {
    "percolate": {
      "documents": [
        {"title": "красивые туфли", "color": "синий"},
        {"title": "красивый пакет"}
      ]
    }
  }
}
```
<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "max_score": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149644,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title пакет"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            2
          ]
        }
      },
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149646,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title обувь"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
PHP:
<!-- request PHP -->

```php
$percolate = [
    'table' => 'products',
    'body' => [
        'query' => [
            'percolate' => [
                'документы' => [
                    ['название' => 'красивые туфли','цвет'=>'синий'],
                    ['название' => 'красивый сумка']
                ]
            ]
        ]
    ]
];
$client->pq()->search($percolate);

```
<!-- response PHP -->

```php
Array
(
    [took] => 23
    [timed_out] =>
    [hits] => Array
        (
            [total] => 2
            [max_score] => 1
            [hits] => Array
                (
                    [0] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 2810781492890828819
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => bag
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 2
                                        )
                                )
                        )
                    [1] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 2810781492890828821
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => shoes
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 1
                                        )
                                )
                        )
                )
        )
)
```
<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.percolate('products',{"query":{"percolate":{"documents":[{"title":"nice pair of shoes","color":"blue"},{"title":"beautiful bag"}]}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381494',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title bag'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [2]}},
                   {u'_id': u'2811025403043381496',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title shoes'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}

```

<!-- intro -->
Python-asyncio
<!-- request Python-asyncio -->

```python
await searchApi.percolate('products',{"query":{"percolate":{"documents":[{"title":"nice pair of shoes","color":"blue"},{"title":"beautiful bag"}]}}})
```
<!-- response Python-asyncio -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381494',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title bag'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [2]}},
                   {u'_id': u'2811025403043381496',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title shoes'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}

```

<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate('products',{"query":{"percolate":{"documents":[{"title":"nice pair of shoes","color":"blue"},{"title":"beautiful bag"}]}}});
```
<!-- response javascript -->
``` javascript
{
  "took": 6,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233808,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            2
          ]
        }
      },
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233810,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
java
<!-- request Java -->

```java
percolateRequest = new PercolateRequest();
query = new HashMap<String,Object>(){{
        put("percolate",new HashMap<String,Object >(){{
            put("documents", new ArrayList<Object>(){{
                    add(new HashMap<String,Object >(){{
                        put("title","прекрасная пара обуви");
                        put("color","синий");
                    }});
                    add(new HashMap<String,Object >(){{
                        put("title","красивый кошелек");

                    }});

                     }});
        }});
    }};
percolateRequest.query(query);
searchApi.percolate("products",percolateRequest);

```
<!-- response Java -->
``` java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234133, _score=1, _source={query={ql=@title кошелек}}, fields={_percolator_document_slot=[2]}}, {_index=products, _type=doc, _id=2811045522851234135, _score=1, _source={query={ql=@title обувь}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
var doc1 = new Dictionary<string, Object>();
doc1.Add("title","прекрасная пара обуви");
doc1.Add("color","синий");
var doc2 = new Dictionary<string, Object>();
doc2.Add("title","красивый кошелек");
var docs = new List<Object> {doc1, doc2};
Dictionary<string, Object> percolateDoc = new Dictionary<string, Object> {{ "documents", docs }}; 
Dictionary<string, Object> query = new Dictionary<string, Object> {{ "percolate", percolateDoc }}; 
PercolateRequest percolateRequest = new PercolateRequest(query=query);
searchApi.Percolate("products",percolateRequest);
```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234133, _score=1, _source={query={ql=@title кошелек}}, fields={_percolator_document_slot=[2]}}, {_index=products, _type=doc, _id=2811045522851234135, _score=1, _source={query={ql=@title обувь}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
Rust
<!-- request Rust -->

```rust
let mut percolate_doc_fields1 = HashMap::new();
percolate_doc_fields1.insert("title".to_string(), "прекрасная пара обуви");
percolate_doc_fields1.insert("color".to_string(), "синий");
let mut percolate_doc_fields2 = HashMap::new();
percolate_doc_fields2.insert("title".to_string(), "красивый кошелек");
let mut percolate_doc_fields_list: [HashMap; 2] = [percolate_doc_fields1, percolate_doc_fields2];
let mut percolate_doc = HashMap::new();
percolate_doc.insert("documents".to_string(), percolate_doc_fields_list); 
let percolate_query = PercolateRequestQuery::new(serde_json::json!(percolate_doc));
let percolate_req = PercolateRequest::new(percolate_query); 
search_api.percolate("products", percolate_req).await;
```
<!-- response Rust -->
``` rust
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234133, _score=1, _source={query={ql=@title кошелек}}, fields={_percolator_document_slot=[2]}}, {_index=products, _type=doc, _id=2811045522851234135, _score=1, _source={query={ql=@title обувь}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
docs = [ {title : 'Какой красивый кошелек'}, {title : 'Действительно красивые туфли'} ]; 
res = await searchApi.percolate('test_pq', { query: { percolate: { documents : docs } } } );
```
<!-- response TypeScript -->
``` typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title кошелек"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title туфли"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
doc1 := map[string]interface{} {"title": "Какой красивый кошелек"}
doc2 := map[string]interface{} {"title": "Действительно красивые туфли"}
query := []interface{} {doc1, doc2}
percolateRequestQuery := manticoreclient.NewPercolateQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery)
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()

```
<!-- response Go -->
``` go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title кошелек"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- end -->

<!-- пример docs_id -->
#### Статические идентификаторы
По умолчанию идентификаторы соответствующих документов соответствуют их относительным номерам в списке, который вы предоставляете. Однако в некоторых случаях каждый документ уже имеет свой собственный идентификатор. В этом случае имеется опция `'id field name' как docs_id` для `CALL PQ`.

Обратите внимание, что если идентификатор не может быть найден по предоставленному имени поля, правило PQ не будет показано в результатах.

Эта опция доступна только для `CALL PQ` через SQL.

<!-- intro -->
##### SQL:

<!-- запрос SQL -->

```sql
CALL PQ('products', '[{"id": 123, "title": "nice pair of shoes", "color": "blue"}, {"id": 456, "title": "beautiful bag"}]', 1 as query, 'id' as docs_id, 1 as docs);
```
<!-- ответ SQL -->

```sql
+---------------------+-----------+--------------+------+---------------------------+
| id                  | documents | query        | tags | filters                   |
+---------------------+-----------+--------------+------+---------------------------+
| 1657852401006149664 | 456       | @title bag   |      |                           |
| 1657852401006149666 | 123       | @title shoes |      | color IN ('blue, 'green') |
+---------------------+-----------+--------------+------+---------------------------+
```
<!-- end -->

<!-- пример invalid_json -->
##### У меня могут быть недопустимые JSON, пожалуйста, пропустите их

При использовании CALL PQ с отдельными JSON вы можете использовать опцию 1 как skip_bad_json, чтобы пропускать любые недопустимые JSON во входных данных. В следующем примере второй запрос завершается ошибкой из-за недопустимого JSON, но третий запрос избегает ошибки, используя 1 как skip_bad_json. Имейте в виду, что эта опция недоступна при отправке JSON-запросов по HTTP, так как весь JSON-запрос должен быть допустимым в этом случае.

<!-- intro -->
SQL:
<!-- запрос SQL -->

```sql
CALL PQ('products', ('{"title": "nice pair of shoes", "color": "blue"}', '{"title": "beautiful bag"}'));

CALL PQ('products', ('{"title": "nice pair of shoes", "color": "blue"}', '{"title": "beautiful bag}'));

CALL PQ('products', ('{"title": "nice pair of shoes", "color": "blue"}', '{"title": "beautiful bag}'), 1 as skip_bad_json);
```
<!-- ответ SQL -->

```sql
+---------------------+
| id                  |
+---------------------+
| 1657852401006149635 |
| 1657852401006149637 |
+---------------------+

ERROR 1064 (42000): Плохие JSON объекты в строках: 2

+---------------------+
| id                  |
+---------------------+
| 1657852401006149635 |
+---------------------+
```

<!-- end -->

##### Я хочу увеличить производительность запроса перколяции
Запросы перколяции разработаны с учетом высокой пропускной способности и больших объемов данных. Для оптимизации производительности для более низкой задержки и более высокой пропускной способности рассмотрите следующее.

Существует два режима распределения для таблицы перколяции и того, как запрос перколяции может работать с ней:

* **Разреженный (по умолчанию).** Идеально подходит для: многих документов, зеркалируемых PQ таблиц. Когда ваш набор документов велик, но набор хранимых запросов в таблице PQ мал, разреженный режим полезен. В этом режиме пакет документов, который вы передаете, будет разделен между количеством агентов, так что каждый узел обрабатывает только часть документов из вашего запроса. Manticore разбивает ваш набор документов и распределяет куски между зеркалами. После завершения обработки запросов агенты возвращают и объединяют результаты, возвращая окончательный набор запросов, как если бы он пришел из одной таблицы. Используйте [репликацию](../References.md#Replication), чтобы помочь в этом процессе.
* **Шардированный.** Идеально подходит для: многих правил PQ, правил, разделенных между PQ таблицами. В этом режиме весь набор документов транслируется всем таблицам распределенной таблицы PQ без первоначального разделения документов. Это полезно, когда вы отправляете относительно небольшой набор документов, но количество хранимых запросов велико. В этом случае более целесообразно хранить только часть правил PQ на каждом узле и затем объединить результаты, возвращенные узлами, которые обрабатывают один и тот же набор документов по различным наборам правил PQ. Этот режим должен быть явно установлен, так как он подразумевает увеличение сетевой нагрузки и ожидает таблицы с различными PQ, что [репликация](../References.md#Replication) не может сделать из коробки.

Предположим, у вас есть таблица `pq_d2`, определенная как:

``` ini
table pq_d2
{
    type = distributed
    agent = 127.0.0.1:6712:pq
    agent = 127.0.0.1:6712:ptitle
}
```

<!-- пример распределенных pq режимов 1 -->
Каждый из 'pq' и 'ptitle' содержит:


<!-- intro -->
##### SQL:

<!-- запрос SQL -->

```sql
SELECT * FROM pq;
```

<!-- ответ sql -->

```sql
+------+-------------+------+-------------------+
| id   | query       | tags | filters           |
+------+-------------+------+-------------------+
|    1 | filter test |      | gid>=10           |
|    2 | angry       |      | gid>=10 OR gid<=3 |
+------+-------------+------+-------------------+
2 rows in set (0.01 sec)
```

<!-- запрос JSON -->

```json
POST /pq/pq/_search
```

<!-- ответ JSON -->

```json
{
    "took":0,
    "timed_out":false,
    "hits":{
        "total":2,
        "hits":[
            {
                "_id": 1,
                "_score":1,
                "_source":{
                    "query":{ "ql":"filter test" },
                    "tags":"",
                    "filters":"gid>=10"
                }
            },
            {
                "_id": 2,
                "_score":1,
                "_source":{
                    "query":{"ql":"злой"},
                    "tags":"",
                    "filters":"gid>=10 OR gid<=3"
                }
            }            
        ]
    }
}
```

<!-- request PHP -->

```php
$params = [
    'table' => 'pq',
    'body' => [
    ]
];
$response = $client->pq()->search($params);
```

<!-- response PHP -->

```php
(
    [took] => 0
    [timed_out] =>
    [hits] =>
        (
            [total] => 2
            [hits] =>
                (
                    [0] =>
                        (
                            [_id] => 1
                            [_score] => 1
                            [_source] =>
                                (
                                    [query] =>
                                        (
                                            [ql] => тест фильтра
                                        )
                                    [tags] =>
                                    [filters] => gid>=10
                                )
                        ),
                    [1] =>
                        (
                            [_id] => 1
                            [_score] => 1
                            [_source] =>
                                (
                                    [query] =>
                                        (
                                            [ql] => злой
                                        )
                                    [tags] =>
                                    [filters] => gid>=10 OR gid<=3
                                )
                        )
                )
        )
)
```
<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.search({"table":"pq","query":{"match_all":{}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381501',
                    u'_score': 1,
                    u'_source': {u'filters': u"gid>=10",
                                 u'query': u'тест фильтра',
                                 u'tags': u''}},
                   {u'_id': u'2811025403043381502',
                    u'_score': 1,
                    u'_source': {u'filters': u"gid>=10 OR gid<=3",
                                 u'query': u'злой',
                                 u'tags': u''}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- intro -->
Python-asyncio
<!-- request Python-asyncio -->

```python
await searchApi.search({"table":"pq","query":{"match_all":{}}})
```
<!-- response Python-asyncio -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381501',
                    u'_score': 1,
                    u'_source': {u'filters': u"gid>=10",
                                 u'query': u'тест фильтра',
                                 u'tags': u''}},
                   {u'_id': u'2811025403043381502',
                    u'_score': 1,
                    u'_source': {u'filters': u"gid>=10 OR gid<=3",
                                 u'query': u'злой',
                                 u'tags': u''}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"pq","query":{"match_all":{}}});
```
<!-- response javascript -->
``` javascript
{'hits': {'hits': [{u'_id': u'2811025403043381501',
                    u'_score': 1,
                    u'_source': {u'filters': u"gid>=10",
                                 u'query': u'тест фильтра',
                                 u'tags': u''}},
                   {u'_id': u'2811025403043381502',
                    u'_score': 1,
                    u'_source': {u'filters': u"gid>=10 OR gid<=3",
                                 u'query': u'злой',
                                 u'tags': u''}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"pq","query":{"match_all":{}}});
```
<!-- response javascript -->
``` javascript
{"hits": {"hits": [{"_id": 2811025403043381501,
                    "_score": 1,
                    "_source": {"filters": u"gid>=10",
                                 "query": "тест фильтра",
                                 "tags": ""}},
                   {"_id": 2811025403043381502,
                    "_score": 1,
                    "_source": {"filters": u"gid>=10 OR gid<=3",
                                 "query": "злой",
                                 "tags": ""}}],
          "total": 2},
  "timed_out": false,
 "took": 0}
```

<!-- intro -->
java
<!-- request Java -->

```java
Map<String,Object> query = new HashMap<String,Object>();
query.put("match_all",null);
SearchRequest searchRequest = new SearchRequest();
searchRequest.setIndex("pq");
searchRequest.setQuery(query);
SearchResponse searchResponse = searchApi.search(searchRequest);
```
<!-- response Java -->
``` java

class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: null
        hits: [{_id=2811045522851233962, _score=1, _source={filters=gid>=10, query=тест фильтра, tags=}}, {_id=2811045522851233951, _score=1, _source={filters=gid>=10 OR gid<=3, query=злой,tags=}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
object query =  new { match_all=null };
SearchRequest searchRequest = new SearchRequest("pq", query);
SearchResponse searchResponse = searchApi.Search(searchRequest);
```
<!-- response C# -->
``` clike

class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: null
        hits: [{_id=2811045522851233962, _score=1, _source={filters=gid>=10, query=filter test, tags=}}, {_id=2811045522851233951, _score=1, _source={filters=gid>=10 OR gid<=3, query=angry,tags=}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
Rust
<!-- request Rust -->

```rust
let query = SearchQuery::new();
let search_req = SearchRequest {
    table: "pq".to_string(),
    query: Some(Box::new(query)),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```
<!-- response Rust -->
``` rust

class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: null
        hits: [{_id=2811045522851233962, _score=1, _source={filters=gid>=10, query=filter test, tags=}}, {_id=2811045522851233951, _score=1, _source={filters=gid>=10 OR gid<=3, query=angry,tags=}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
res = await searchApi.search({"table":"test_pq","query":{"match_all":{}}});
```
<!-- response TypeScript -->
``` typescript
{

'hits':

{

'hits': 

[{

'_id': '2811025403043381501',
            '_score': 1,
            '_source': 
            {
            
'filters': "gid>=10",
                'query': 'filter test',
                'tags': ''
            }
        },
        {
         
'_id': 
         
'2811025403043381502',
            '_score': 1,
            '_source': 
            {
            
'filters': "gid>=10 OR gid<=3",
                 'query': 'angry',
                 'tags': ''
            }
        }],
    
'total': 2

},

'profile': None,

'timed_out': False,

'took': 0
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
query := map[string]interface{} {}
percolateRequestQuery := manticoreclient.NewPercolateRequestQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery) 
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()
```
<!-- response Go -->
``` go
{

'hits':

{

'hits': 

[{

'_id': '2811025403043381501',
            '_score': 1,
            '_source': 
            {
            
'filters': "gid>=10",
                'query': 'filter test',
                'tags': ''
            }
        },
        {
         
'_id': 
         
'2811025403043381502',
            '_score': 1,
            '_source': 
            {
            
'filters': "gid>=10 OR gid<=3",
                 'query': 'angry',
                 'tags': ''
            }
        }],
    
'total': 2

},

'profile': None,

'timed_out': False,

'took': 0
}
```

<!-- end -->


<!-- example call_pq_example -->

And you execute `CALL PQ` on the distributed table with a couple of documents.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CALL PQ ('pq_d2', ('{"title":"angry test", "gid":3 }', '{"title":"filter test doc2", "gid":13}'), 1 AS docs);
```

<!-- response sql -->

```sql
+------+-----------+
| id   | documents |
+------+-----------+
|    1 | 2         |
|    2 | 1         |
+------+-----------+
```

<!-- request JSON -->

```json
POST /pq/pq/_search -d '
"query":
{
        "percolate":
        {
                "documents" : [
                    { "title": "angry test", "gid": 3 },
                    { "title": "filter test doc2", "gid": 13 }
                ]
        }
}
'
```

<!-- response JSON -->

```json
{
    "took":0,
    "timed_out":false,
    "hits":{
    "total":2,"hits":[
        {
            "_id": 2,
            "_score":1,
            "_source":{
                "query":{"title":"angry"},
                "tags":"",
                "filters":"gid>=10 OR gid<=3"
            }
        }
        {
            "_id": 1,
            "_score":1,
            "_source":{
                "query":{"ql":"filter test"},
                "tags":"",
                "filters":"gid>=10"
            }
        },
        ]
    }
}
```

<!-- request PHP -->

```php
$params = [
    'table' => 'pq',
    'body' => [
        'query' => [
            'percolate' => [
                'documents' => [
                    [
                        'title'=>'angry test',
                        'gid' => 3
                    ],
                    [
                        'title'=>'filter test doc2',
                        'gid' => 13
                    ],
                ]
            ]
        ]
    ]
];
$response = $client->pq()->search($params);
```

<!-- response PHP -->

```php
(
    [took] => 0
    [timed_out] =>
    [hits] =>
        (
            [total] => 2
            [hits] =>
                (
                    [0] =>
                        (
                            [_index] => pq  
                            [_type] => doc                            
                            [_id] => 2
                            [_score] => 1
                            [_source] =>
                                (
                                    [query] =>
                                        (
                                            [ql] => сердитый
                                        )
                                    [tags] =>
                                    [filters] => gid>=10 OR gid<=3
                                ),
                           [fields] =>
                                (
                                    [_percolator_document_slot] =>
                                        (
                                            [0] => 1
                                        )

                                )
                        ),
                    [1] =>
                        (
                            [_index] => pq                            
                            [_id] => 1
                            [_score] => 1
                            [_source] =>
                                (
                                    [query] =>
                                        (
                                            [ql] => фильтр тест
                                        )
                                    [tags] =>
                                    [filters] => gid>=10
                                )
                           [fields] =>
                                (
                                    [_percolator_document_slot] =>
                                        (
                                            [0] => 0
                                        )

                                )
                        )
                )
        )
)
```
<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.percolate('pq',{"percolate":{"documents":[{"title":"сердитый тест","gid":3},{"title":"фильтр тест документ2","gid":13}]}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'table': u'pq',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'сердитый'},u'tags':u'',u'filters':u"gid>=10 OR gid<=3"},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}},
                    {u'_id': u'2811025403043381501',
                    u'table': u'pq',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'фильтр тест'},u'tags':u'',u'filters':u"gid>=10"},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- intro -->
Python-asyncio
<!-- request Python-asyncio -->

```python
await searchApi.percolate('pq',{"percolate":{"documents":[{"title":"сердитый тест","gid":3},{"title":"фильтр тест документ2","gid":13}]}})
```
<!-- response Python-asyncio -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'table': u'pq',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'сердитый'},u'tags':u'',u'filters':u"gid>=10 OR gid<=3"},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}},
                    {u'_id': u'2811025403043381501',
                    u'table': u'pq',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'фильтр тест'},u'tags':u'',u'filters':u"gid>=10"},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate('pq',{"percolate":{"documents":[{"title":"сердитый тест","gid":3},{"title":"фильтр тест документ2","gid":13}]}});
```
<!-- response javascript -->
``` javascript
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'table': u'pq',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'сердитый'},u'tags':u'',u'filters':u"gid>=10 OR gid<=3"},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}},
                    {u'_id': u'2811025403043381501',
                    u'table': u'pq',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'фильтр тест'},u'tags':u'',u'filters':u"gid>=10"},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
java
<!-- request Java -->

```java
percolateRequest = new PercolateRequest();
query = new HashMap<String,Object>(){{
    put("percolate",new HashMap<String,Object >(){{
        put("documents", new ArrayList<Object>(){{
            add(new HashMap<String,Object >(){{
                put("title","сердитый тест");
                put("gid",3);
            }});
            add(new HashMap<String,Object >(){{
                put("title","фильтр тест документ2");
                put("gid",13);
            }});
        }});
    }});
}};
percolateRequest.query(query);
searchApi.percolate("pq",percolateRequest);
```
<!-- response java -->
``` java
class SearchResponse {
    took: 10
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=pq, _type=doc, _id=2811045522851234165, _score=1, _source={query={ql=@title angry}}, fields={_percolator_document_slot=[1]}}, {_index=pq, _type=doc, _id=2811045522851234166, _score=1, _source={query={ql=@title filter test doc2}}, fields={_percolator_document_slot=[2]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
var doc1 = new Dictionary<string, Object>();
doc1.Add("title","гневный тест");
doc1.Add("gid",3);
var doc2 = new Dictionary<string, Object>();
doc2.Add("title","тест фильтра doc2");
doc2.Add("gid",13);
var docs = new List<Object> {doc1, doc2};
Dictionary<string, Object> percolateDoc = new Dictionary<string, Object> {{ "documents", docs }}; 
Dictionary<string, Object> query = new Dictionary<string, Object> {{ "percolate", percolateDoc }}; 
PercolateRequest percolateRequest = new PercolateRequest(query=query);
searchApi.Percolate("pq",percolateRequest);
```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 10
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=pq, _type=doc, _id=2811045522851234165, _score=1, _source={query={ql=@title angry}}, fields={_percolator_document_slot=[1]}}, {_index=pq, _type=doc, _id=2811045522851234166, _score=1, _source={query={ql=@title filter test doc2}}, fields={_percolator_document_slot=[2]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
Rust
<!-- request Rust -->

```rust
let mut percolate_doc_fields1 = HashMap::new();
percolate_doc_fields1.insert("title".to_string(), "гневный тест");
percolate_doc_fields1.insert("gid".to_string(), 3);
let mut percolate_doc_fields2 = HashMap::new();
percolate_doc_fields2.insert("title".to_string(), "тест фильтра doc2");
percolate_doc_fields2.insert("gid".to_string(), 13);
let mut percolate_doc_fields_list: [HashMap; 2] = [percolate_doc_fields1, percolate_doc_fields2];
let mut percolate_doc = HashMap::new();
percolate_doc.insert("documents".to_string(), percolate_doc_fields_list); 
let percolate_query = PercolateRequestQuery::new(serde_json::json!(percolate_doc));
let percolate_req = PercolateRequest::new(percolate_query); 
search_api.percolate("pq", percolate_req).await;
```
<!-- response Rust -->
``` rust
class SearchResponse {
    took: 10
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=pq, _type=doc, _id=2811045522851234165, _score=1, _source={query={ql=@title angry}}, fields={_percolator_document_slot=[1]}}, {_index=pq, _type=doc, _id=2811045522851234166, _score=1, _source={query={ql=@title filter test doc2}}, fields={_percolator_document_slot=[2]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
docs = [ {title : 'Какой красивый мешок'}, {title : 'Действительно красивые туфли'} ]; 
res = await searchApi.percolate('test_pq', { query: { percolate: { documents : docs } } } );
```
<!-- response TypeScript -->
``` typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
doc1 := map[string]interface{} {"title": "Какой красивый мешок"}
doc2 := map[string]interface{} {"title": "Действительно красивые туфли"}
query := []interface{} {doc1, doc2}
percolateRequestQuery := manticoreclient.NewPercolateQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery)
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()

```
<!-- response Go -->
``` go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- end -->

В предыдущем примере мы использовали режим **разреженного** по умолчанию. Чтобы продемонстрировать режим **распределенного**, давайте создадим распределённую таблицу PQ, состоящую из 2 локальных таблиц PQ и добавим 2 документа в "products1" и 1 документ в "products2":
```sql
create table products1(title text, color string) type='pq';
create table products2(title text, color string) type='pq';
create table products_distributed type='distributed' local='products1' local='products2';

ВСТАВИТЬ В products1(query) значения('@title bag');
ВСТАВИТЬ В products1(query,filters) значения('@title shoes', 'color='red'');
ВСТАВИТЬ В products2(query,filters) значения('@title shoes', 'color in ('blue', 'green')');
```

<!-- example sharded -->
Теперь, если вы добавите `'sharded' как режим` к `CALL PQ`, это отправит документы во все таблицы агента (в данном случае, только локальные таблицы, но они могут быть удаленными для использования внешнего оборудования). Этот режим недоступен через JSON интерфейс.

<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products_distributed', ('{"title": "nice pair of shoes", "color": "blue"}', '{"title": "beautiful bag"}'), 'sharded' как режим, 1 как запрос);
```
<!-- response SQL -->

```sql
+---------------------+--------------+------+---------------------------+
| id                  | query        | tags | filters                   |
# Перколяторный запрос

Перколяторные запросы также известны как постоянные запросы, перспективный поиск, маршрутизация документов, обратный поиск и инвертированный поиск.

Традиционный способ проведения поисков включает в себя хранение документов и выполнение запросов к ним. Однако бывают случаи, когда мы хотим применить запрос к вновь поступившему документу, чтобы сигнализировать о совпадении. Некоторые сценарии, когда это желательно, включают системы мониторинга, которые собирают данные и уведомляют пользователей о конкретных событиях, таких как достижение определенного порога для метрики или появление конкретного значения в отслеживаемых данных. Другим примером является агрегирование новостей, когда пользователи могут хотеть получать уведомления только о некоторых категориях или темах, или даже о конкретных "ключевых словах".

В таких ситуациях традиционный поиск не является лучшим вариантом, поскольку предполагает, что желаемый поиск выполняется по всей коллекции. Этот процесс умножается на количество пользователей, что приводит к множеству запросов, выполняемых по всей коллекции, что может вызвать значительную дополнительную нагрузку. Альтернативный подход, описанный в этом разделе, заключается в хранении запросов и их тестировании на новом входящем документе или партии документов.

Google Alerts, AlertHN, Bloomberg Terminal и другие системы, которые позволяют пользователям подписываться на конкретный контент, используют аналогичную технологию.

> * См. [перколятор](../Creating_a_table/Local_tables/Percolate_table.md) для получения информации о создании таблицы PQ.
> * См. [Добавление правил в таблицу перколяторов](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md), чтобы узнать, как добавить правила перколяторов (также известные как правила PQ). Вот быстрый пример:


### Выполнение перколяторного запроса с CALL PQ

Главное, что нужно помнить о перколяторных запросах, это то, что ваши поисковые запросы уже находятся в таблице. То, что вам нужно предоставить, это документы **для проверки на совпадение с любыми из сохраненных правил**.

Вы можете выполнять перколяторный запрос через SQL или JSON интерфейсы, а также с помощью клиентов языков программирования. Подход SQL предлагает больше гибкости, тогда как HTTP-метод проще и предоставляет большинство того, что вам нужно. Таблица ниже может помочь вам понять различия.

| Желаемое поведение         | SQL                                     | HTTP                                 |
| -------------------------- | --------------------------------------- | ------------------------------------ |
| Предоставить один документ | `CALL PQ('tbl', '{doc1}')`              | `query.percolate.document{doc1}`     |
| Предоставить один документ (альтернатива) | `CALL PQ('tbl', 'doc1', 0 as docs_json)` | - |
| Предоставить несколько документов | `CALL PQ('tbl', ('doc1', 'doc2'), 0 as docs_json)` | - |
| Предоставить несколько документов (альтернатива) | `CALL PQ('tbl', ('{doc1}', '{doc2}'))` | - |
| Предоставить несколько документов (альтернатива) | `CALL PQ('tbl', '[{doc1}, {doc2}]')` | - |
| Вернуть совпадающие идентификаторы документов | 0/1 as docs (отключено по умолчанию)       | Включено по умолчанию                   |
| Использовать собственный идентификатор документа для отображения в результате | 'id field' as docs_id (отключено по умолчанию) | Не доступно |
| Рассматривать входящие документы как JSON | 1 as docs_json (1 по умолчанию) | Включено по умолчанию |
| Рассматривать входящие документы как обычный текст | 0 as docs_json (1 по умолчанию) | Не доступно |
| [Разреженный режим дистрибуции](../Searching/Percolate_query.md#I-want-higher-performance-of-a-percolate-query) | по умолчанию | по умолчанию |
| [Шардированный режим дистрибуции](../Searching/Percolate_query.md#I-want-higher-performance-of-a-percolate-query) | шардированный как режим | Не доступно |
| Вернуть всю информацию о совпадающем запросе | 1 as query (0 по умолчанию) | Включено по умолчанию |
| Пропустить недействительный JSON | 1 as skip_bad_json (0 по умолчанию) | Не доступно |
| Расширенная информация в [SHOW META](../Node_info_and_management/SHOW_META.md) | 1 as verbose (0 по умолчанию) | Не доступно |
| Определить число, которое будет добавлено к идентификаторам документов, если не указаны поля docs_id (в основном актуально в [распределенных режимах PQ](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#Distributed-percolate-tables-%28DPQ-tables%29)) | 1 as shift (0 по умолчанию) | Не доступно |

<!-- пример создания перколятора -->
Чтобы продемонстрировать, как это работает, вот несколько примеров. Давайте создадим таблицу PQ с двумя полями:

* заголовок (текст)
* цвет (строка)

и тремя правилами в ней:

* Просто полный текст. Запрос: `@title bag`
* Полный текст и фильтрация. Запрос: `@title shoes`. Фильтры: `color='red'`
* Полный текст и более сложная фильтрация. Запрос: `@title shoes`. Фильтры: `color IN('blue', 'green')`

<!-- ввод -->
#### SQL
<!-- запрос SQL -->

```sql
CREATE TABLE products(title text, color string) type='pq';
INSERT INTO products(query) values('@title bag');
INSERT INTO products(query,filters) values('@title shoes', 'color='red'');
INSERT INTO products(query,filters) values('@title shoes', 'color in ('blue', 'green')');
select * from products;
```

<!-- ответ SQL -->

```sql
+---------------------+--------------+------+---------------------------+
| id                  | query        | tags | filters                   |
+---------------------+--------------+------+---------------------------+
| 1657852401006149635 | @title shoes |      | color IN ('blue, 'green') |
| 1657852401006149636 | @title shoes |      | color='red'               |
| 1657852401006149637 | @title bag   |      |                           |
+---------------------+--------------+------+---------------------------+
```

<!-- запрос JSON -->

```json
PUT /pq/products/doc/
{
  "query": {
    "match": {
      "title": "bag"
    }
  },
  "filters": ""
}

PUT /pq/products/doc/
{
  "query": {
    "match": {
      "title": "shoes"
    }
  },
  "filters": "color='red'"
}

PUT /pq/products/doc/
{
  "query": {
    "match": {
      "title": "shoes"
    }
  },
  "filters": "цвет IN ('синий', 'зеленый')"
}
```

<!-- intro -->
#### JSON
<!-- response JSON -->

```json
{
  "table": "продукты",
  "type": "doc",
  "_id": 1657852401006149661,
  "result": "создано"
}
{
  "table": "продукты",
  "type": "doc",
  "_id": 1657852401006149662,
  "result": "создано"
}
{
  "table": "продукты",
  "type": "doc",
  "_id": 1657852401006149663,
  "result": "создано"
}
```

<!-- intro -->
#### PHP
<!-- request PHP -->

```php

$index = [
    'table' => 'продукты',
    'body' => [
        'columns' => [
            'title' => ['type' => 'text'],
            'color' => ['type' => 'string']
        ],
        'settings' => [
            'type' => 'pq'
        ]
    ]
];
$client->indices()->create($index);

$query = [
    'table' => 'продукты',
    'body' => [ 'query'=>['match'=>['title'=>'сумка']]]
];
$client->pq()->doc($query);
$query = [
    'table' => 'продукты',
    'body' => [ 'query'=>['match'=>['title'=>'обувь']],'filters'=>"цвет='красный'"]
];
$client->pq()->doc($query);


$query = [
    'table' => 'продукты',
    'body' => [ 'query'=>['match'=>['title'=>'обувь']],'filters'=>"цвет IN ('синий', 'зеленый')"]
];
$client->pq()->doc($query);
```
<!-- response PHP -->
``` php
Array(
  [table] => продукты
  [type] => doc
  [_id] => 1657852401006149661
  [result] => создано
)
Array(
  [table] => продукты
  [type] => doc
  [_id] => 1657852401006149662
  [result] => создано
)
Array(
  [table] => продукты
  [type] => doc
  [_id] => 1657852401006149663
  [result] => создано
)
```
<!-- intro -->
Python
<!-- request Python -->

```python
utilsApi.sql('create table продукты(title text, color string) type='pq'')
indexApi.insert({"table" : "продукты", "doc" : {"query" : "@title сумка" }})
indexApi.insert({"table" : "продукты",  "doc" : {"query" : "@title обувь", "filters": "цвет='красный'" }})
indexApi.insert({"table" : "продукты",  "doc" : {"query" : "@title обувь","filters": "цвет IN ('синий', 'зеленый')" }})
```
<!-- response Python -->
``` python
{'created': True,
 'found': None,
 'id': 0,
 'table': 'продукты',
 'result': 'создано'}
{'created': True,
 'found': None,
 'id': 0,
 'table': 'продукты',
 'result': 'создано'}
{'created': True,
 'found': None,
 'id': 0,
 'table': 'продукты',
 'result': 'создано'}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await utilsApi.sql('create table продукты(title text, color string) type='pq'');
res = indexApi.insert({"table" : "продукты", "doc" : {"query" : "@title сумка" }});
res = indexApi.insert({"table" : "продукты",  "doc" : {"query" : "@title обувь", "filters": "цвет='красный'" }});
res = indexApi.insert({"table" : "продукты",  "doc" : {"query" : "@title обувь","filters": "цвет IN ('синий', 'зеленый')" }});
```
<!-- response javascript -->
``` javascript
"table":"продукты","_id":0,"created":true,"result":"создано"}
{"table":"продукты","_id":0,"created":true,"result":"создано"}
{"table":"продукты","_id":0,"created":true,"result":"создано"}
```
<!-- intro -->
java
<!-- request Java -->

```java
utilsApi.sql("create table продукты(title text, color string) type='pq'");
doc = new HashMap<String,Object>(){{
    put("query", "@title сумка");
}};
newdoc = new InsertDocumentRequest();
newdoc.index("продукты").setDoc(doc);
indexApi.insert(newdoc);

doc = new HashMap<String,Object>(){{
    put("query", "@title обувь");
    put("filters", "цвет='красный'");
}};
newdoc = new InsertDocumentRequest();
newdoc.index("продукты").setDoc(doc);
indexApi.insert(newdoc);

doc = new HashMap<String,Object>(){{
    put("query", "@title обувь");
    put("filters", "цвет IN ('синий', 'зеленый')");
}};
newdoc = new InsertDocumentRequest();
newdoc.index("продукты").setDoc(doc);
indexApi.insert(newdoc);
```
<!-- response Java -->
``` java
{total=0, error=, warning=}
class SuccessResponse {
    index: продукты
    id: 0
    created: true
    result: создано
    found: null
}
class SuccessResponse {
    index: продукты
    id: 0
    created: true
    result: создано
    found: null
}
class SuccessResponse {
    index: продукты
    id: 0
    created: true
    result: создано
    found: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
utilsApi.Sql("create table продукты(title text, color string) type='pq'");

Dictionary<string, Object> doc = new Dictionary<string, Object>(); 
doc.Add("query", "@title сумка");
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "продукты", doc: doc);
indexApi.Insert(newdoc);

doc = new Dictionary<string, Object>(); 
doc.Add("query", "@title обувь");
doc.Add("filters", "цвет='красный'");
newdoc = new InsertDocumentRequest(index: "продукты", doc: doc);
indexApi.Insert(newdoc);

doc = new Dictionary<string, Object>(); 
doc.Add("query", "@title сумка");
doc.Add("filters", "цвет IN ('синий', 'зеленый')");
newdoc = new InsertDocumentRequest(index: "продукты", doc: doc);
indexApi.Insert(newdoc);
```
<!-- response C# -->
``` clike
{total=0, error="", warning=""}

class SuccessResponse {
    index: продукты
    id: 0
    created: true
    result: создано
    found: null
}
class SuccessResponse {
    index: продукты
    id: 0
    created: true
    result: создано
    found: null
}
class SuccessResponse {
    index: продукты
    id: 0
    created: true
    result: создано
    found: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->
```typescript
res = await utilsApi.sql("создать таблицу test_pq(title text, color string) type='pq'");
res = indexApi.insert({
  index: 'test_pq',
  doc: { query : '@title bag' }
});
res = indexApi.insert(
  index: 'test_pq',
  doc: { query: '@title shoes', filters: "color='red'" }
});
res = indexApi.insert({
  index: 'test_pq',
  doc: { query : '@title shoes', filters: "color IN ('blue', 'green')" }
});
```
<!-- response TypeScript -->
``` typescript
{
	"table":"test_pq",
	"_id":1657852401006149661,
	"created":true,
	"result":"создана"
}
{
	"table":"test_pq",
	"_id":1657852401006149662,
	"created":true,
	"result":"создана"
}
{
	"table":"test_pq",
	"_id":1657852401006149663,
	"created":true,
	"result":"создана"
}
```

<!-- intro -->
Идти
<!-- request Go -->
```go
apiClient.UtilsAPI.Sql(context.Background()).Body("создать таблицу test_pq(title text, color string) type='pq'").Execute()

indexDoc := map[string]interface{} {"query": "@title bag"}
indexReq := manticoreclient.NewInsertDocumentRequest("test_pq", indexDoc)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute();

indexDoc = map[string]interface{} {"query": "@title shoes", "filters": "color='red'"}
indexReq = manticoreclient.NewInsertDocumentRequest("test_pq", indexDoc)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute();

indexDoc = map[string]interface{} {"query": "@title shoes", "filters": "color IN ('blue', 'green')"}
indexReq = manticoreclient.NewInsertDocumentRequest("test_pq", indexDoc)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute();
```
<!-- response Go -->
``` go
{
	"table":"test_pq",
	"_id":1657852401006149661,
	"created":true,
	"result":"создана"
}
{
	"table":"test_pq",
	"_id":1657852401006149662,
	"created":true,
	"result":"создана"
}
{
	"table":"test_pq",
	"_id":1657852401006149663,
	"created":true,
	"result":"создана"
}
```

<!-- end -->

<!-- example single -->
##### Просто скажите мне, какие правила PQ соответствуют моему отдельному документу

Первый документ не соответствует никаким правилам. Он мог бы соответствовать первым двум, но они требуют дополнительных фильтров.

Второй документ соответствует одному правилу. Обратите внимание, что CALL PQ по умолчанию ожидает, что документ будет в формате JSON, но если вы используете `0 as docs_json`, вы можете передать обычную строку вместо этого.

<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products', 'Красивые туфли', 0 as docs_json);

CALL PQ('products', 'Какой красивый мешок', 0 as docs_json);
CALL PQ('products', '{"title": "Какой красивый мешок"}');
```
<!-- response SQL -->

```sql
+---------------------+
| id                  |
+---------------------+
| 1657852401006149637 |
+---------------------+

+---------------------+
| id                  |
+---------------------+
| 1657852401006149637 |
+---------------------+
```
<!-- intro -->
JSON:
<!-- request JSON -->

```json
POST /pq/products/_search
{
  "query": {
    "percolate": {
      "document": {
        "title": "Какой красивый мешок"
      }
    }
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
    "max_score": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149644,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
PHP:
<!-- request PHP -->

```php
$percolate = [
    'table' => 'products',
    'body' => [
        'query' => [
            'percolate' => [
                'document' => [
                    'title' => 'Какой красивый мешок'
                ]
            ]
        ]
    ]
];
$client->pq()->search($percolate);

```
<!-- response PHP -->

```php
Array
(
    [took] => 0
    [timed_out] =>
    [hits] => Array
        (
            [total] => 1
            [max_score] => 1
            [hits] => Array
                (
                    [0] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 1657852401006149644
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => bag
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 1
                                        )
                                )
                        )
                )
        )
)
```
<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"Какой красивый мешок"}}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title bag'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"Что за красивая сумка"}}}});
```
<!-- response javascript -->
``` javascript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233808,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
java
<!-- request Java -->

```java
PercolateRequest percolateRequest = new PercolateRequest();
query = new HashMap<String,Object>(){{
    put("percolate",new HashMap<String,Object >(){{
        put("document", new HashMap<String,Object >(){{
            put("title","что за красивая сумка");
        }});
    }});
}};
percolateRequest.query(query);
searchApi.percolate("test_pq",percolateRequest);
```
<!-- response Javs -->
``` java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234109, _score=1, _source={query={ql=@title bag}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
Dictionary<string, Object> percolateDoc = new Dictionary<string, Object>(); 
percolateDoc.Add("document", new Dictionary<string, Object> {{ "title", "что за красивая сумка" }});
Dictionary<string, Object> query = new Dictionary<string, Object> {{ "percolate", percolateDoc }}; 
PercolateRequest percolateRequest = new PercolateRequest(query=query);
searchApi.Percolate("test_pq",percolateRequest);

```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234109, _score=1, _source={query={ql=@title bag}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
res = await searchApi.percolate('test_pq', { query: { percolate: { document : { title : 'Что за красивая сумка' } } } } );
```
<!-- response TypeScript -->
``` typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
query := map[string]interface{} {"title": "что за красивая сумка"}
percolateRequestQuery := manticoreclient.NewPercolateQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery)
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()

```
<!-- response Go -->
``` go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- end -->

<!-- example pq_rules -->
##### I want to know complete PQ rules matching my document
<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products', '{"title": "Что за красивая сумка"}', 1 as query);
```
<!-- response SQL -->

```sql
+---------------------+------------+------+---------+
| id                  | query      | tags | filters |
+---------------------+------------+------+---------+
| 1657852401006149637 | @title bag |      |         |
+---------------------+------------+------+---------+
```
<!-- intro -->
JSON:
<!-- request JSON -->

```json
POST /pq/products/_search
{
  "query": {
    "percolate": {
      "document": {
        "title": "Что за красивая сумка"
      }
    }
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
    "max_score": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149644,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
PHP:
<!-- request PHP -->

```php
$percolate = [
    'table' => 'products',
    'body' => [
        'query' => [
            'percolate' => [
                'document' => [
                    'title' => 'Какой приятный мешок'
                ]
            ]
        ]
    ]
];
$client->pq()->search($percolate);

```
<!-- response PHP -->

```php
Array
(
    [took] => 0
    [timed_out] =>
    [hits] => Array
        (
            [total] => 1
            [max_score] => 1
            [hits] => Array
                (
                    [0] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 1657852401006149644
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => мешок
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 1
                                        )
                                )
                        )
                )
        )
)
```

<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"Какой приятный мешок"}}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title мешок'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"Какой приятный мешок"}}}});
```
<!-- response javascript -->
``` javascript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233808,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title мешок"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
java
<!-- request Java -->

```java
PercolateRequest percolateRequest = new PercolateRequest();
query = new HashMap<String,Object>(){{
    put("percolate",new HashMap<String,Object >(){{
        put("document", new HashMap<String,Object >(){{
            put("title","какой приятный мешок");
        }});
    }});
}};
percolateRequest.query(query);
searchApi.percolate("test_pq",percolateRequest);
```
<!-- response Java -->
``` java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234109, _score=1, _source={query={ql=@title мешок}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
Dictionary<string, Object> percolateDoc = new Dictionary<string, Object>(); 
percolateDoc.Add("document", new Dictionary<string, Object> {{ "title", "какой приятный мешок" }});
Dictionary<string, Object> query = new Dictionary<string, Object> {{ "percolate", percolateDoc }}; 
PercolateRequest percolateRequest = new PercolateRequest(query=query);
searchApi.Percolate("test_pq",percolateRequest);

```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234109, _score=1, _source={query={ql=@title мешок}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
res = await searchApi.percolate('test_pq', { query: { percolate: { document : { title : 'Какой приятный мешок' } } } } );
```
<!-- response TypeScript -->
``` typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title мешок"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
query := map[string]interface{} {"title": "какой приятный мешок"}
percolateRequestQuery := manticoreclient.NewPercolateQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery)
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()

```
<!-- response Go -->
``` go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- end -->

<!-- example multiple -->

##### Как насчет нескольких документов?

Заметьте, что с помощью `CALL PQ` вы можете предоставить несколько документов различными способами:

* в виде массива обычных документов в круглых скобках `('doc1', 'doc2')`. Это требует `0 as docs_json`
* в виде массива JSON в круглых скобках `('{doc1}', '{doc2}')`
* или как стандартный JSON массив `'[{doc1}, {doc2}]'`

<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products', ('nice pair of shoes', 'beautiful bag'), 1 as query, 0 as docs_json);

CALL PQ('products', ('{"title": "nice pair of shoes", "color": "red"}', '{"title": "beautiful bag"}'), 1 as query);

CALL PQ('products', '[{"title": "nice pair of shoes", "color": "blue"}, {"title": "beautiful bag"}]', 1 as query);
```
<!-- response SQL -->

```sql
+---------------------+------------+------+---------+
| id                  | query      | tags | filters |
+---------------------+------------+------+---------+
| 1657852401006149637 | @title bag |      |         |
+---------------------+------------+------+---------+

+---------------------+--------------+------+-------------+
| id                  | query        | tags | filters     |
+---------------------+--------------+------+-------------+
| 1657852401006149636 | @title shoes |      | color='red' |
| 1657852401006149637 | @title bag   |      |             |
+---------------------+--------------+------+-------------+

+---------------------+--------------+------+---------------------------+
| id                  | query        | tags | filters                   |
+---------------------+--------------+------+---------------------------+
| 1657852401006149635 | @title shoes |      | color IN ('blue, 'green') |
| 1657852401006149637 | @title bag   |      |                           |
+---------------------+--------------+------+---------------------------+
```
<!-- intro -->
JSON:
<!-- request JSON -->

```json
POST /pq/products/_search
{
  "query": {
    "percolate": {
      "documents": [
        {"title": "nice pair of shoes", "color": "blue"},
        {"title": "beautiful bag"}
      ]
    }
  }
}
```
<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "max_score": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149644,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            2
          ]
        }
      },
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149646,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
PHP:
<!-- request PHP -->

```php
$percolate = [
    'table' => 'products',
    'body' => [
        'query' => [
            'percolate' => [
                'documents' => [
                    ['title' => 'nice pair of shoes','color'=>'blue'],
                    ['title' => 'beautiful bag']
                ]
            ]
        ]
    ]
];
$client->pq()->search($percolate);

```
<!-- response PHP -->

```php
Array
(
    [took] => 23
    [timed_out] =>
    [hits] => Array
        (
            [total] => 2
            [max_score] => 1
            [hits] => Array
                (
                    [0] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 2810781492890828819
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => bag
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 2
                                        )
                                )
                        )
                    [1] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 2810781492890828821
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => shoes
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 1
                                        )
                                )
                        )
                )
        )
)
```
<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.percolate('products',{"query":{"percolate":{"documents":[{"title":"nice pair of shoes","color":"blue"},{"title":"beautiful bag"}]}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381494',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title bag'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [2]}},
                   {u'_id': u'2811025403043381496',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title shoes'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}

```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate('products',{"query":{"percolate":{"documents":[{"title":"nice pair of shoes","color":"blue"},{"title":"beautiful bag"}]}}});
```
<!-- response javascript -->
``` javascript
{
  "took": 6,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233808,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            2
          ]
        }
      },
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233810,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}

```

<!-- intro -->
java
<!-- request Java -->

```java
percolateRequest = new PercolateRequest();
query = new HashMap<String,Object>(){{
        put("percolate",new HashMap<String,Object >(){{
            put("documents", new ArrayList<Object>(){{
                    add(new HashMap<String,Object >(){{
                        put("title","nice pair of shoes");
                        put("color","blue");
                    }});
                    add(new HashMap<String,Object >(){{
                        put("title","beautiful bag");

                    }});

                     }});
        }});
    }};
percolateRequest.query(query);
searchApi.percolate("products",percolateRequest);

```
<!-- response Java -->
``` java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234133, _score=1, _source={query={ql=@title bag}}, fields={_percolator_document_slot=[2]}}, {_index=products, _type=doc, _id=2811045522851234135, _score=1, _source={query={ql=@title shoes}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
var doc1 = new Dictionary<string, Object>();
doc1.Add("title","nice pair of shoes");
doc1.Add("color","blue");
var doc2 = new Dictionary<string, Object>();
doc2.Add("title","beautiful bag");
var docs = new List<Object> {doc1, doc2};
Dictionary<string, Object> percolateDoc = new Dictionary<string, Object> {{ "documents", docs }}; 
Dictionary<string, Object> query = new Dictionary<string, Object> {{ "percolate", percolateDoc }}; 
PercolateRequest percolateRequest = new PercolateRequest(query=query);
searchApi.Percolate("products",percolateRequest);
```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234133, _score=1, _source={query={ql=@title bag}}, fields={_percolator_document_slot=[2]}}, {_index=products, _type=doc, _id=2811045522851234135, _score=1, _source={query={ql=@title shoes}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
docs = [ {title : 'What a nice bag'}, {title : 'Really nice shoes'} ]; 
res = await searchApi.percolate('test_pq', { query: { percolate: { documents : docs } } } );
```
<!-- response TypeScript -->
``` typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
Идти
<!-- request Go -->

```go
doc1 := map[string]interface{} {"title": "Какой красивый пакет"}
doc2 := map[string]interface{} {"title": "Действительно красивые туфли"}
query := []interface{} {doc1, doc2}
percolateRequestQuery := manticoreclient.NewPercolateQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery)
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()

```
<!-- response Go -->
``` go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- end -->

<!-- example docs_1 -->
##### Я хочу знать, какие документы соответствуют каким правилам

Использование опции `1 as docs` позволяет вам увидеть, какие из предоставленных документов соответствуют каким правилам.
<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products', '[{"title": "nice pair of shoes", "color": "blue"}, {"title": "beautiful bag"}]', 1 as query, 1 as docs);
```

<!-- response SQL -->

```sql
+---------------------+-----------+--------------+------+---------------------------+
| id                  | documents | query        | tags | filters                   |
+---------------------+-----------+--------------+------+---------------------------+
| 1657852401006149635 | 1         | @title shoes |      | color IN ('blue, 'green') |
| 1657852401006149637 | 2         | @title bag   |      |                           |
+---------------------+-----------+--------------+------+---------------------------+
```
<!-- intro -->
JSON:
<!-- request JSON -->

```json
POST /pq/products/_search
{
  "query": {
    "percolate": {
      "documents": [
        {"title": "nice pair of shoes", "color": "blue"},
        {"title": "beautiful bag"}
      ]
    }
  }
}
```
<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "max_score": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149644,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            2
          ]
        }
      },
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149646,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
PHP:
<!-- request PHP -->

```php
$percolate = [
    'table' => 'products',
    'body' => [
        'query' => [
            'percolate' => [
                'documents' => [
                    ['title' => 'nice pair of shoes','color'=>'blue'],
                    ['title' => 'beautiful bag']
                ]
            ]
        ]
    ]
];
$client->pq()->search($percolate);

```
<!-- response PHP -->

```php
Array
(
    [took] => 23
    [timed_out] =>
    [hits] => Array
        (
            [total] => 2
            [max_score] => 1
            [hits] => Array
                (
                    [0] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 2810781492890828819
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => bag
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 2
                                        )
                                )
                        )
                    [1] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 2810781492890828821
                            [_score] => 1
                            [_source] => Array
                            (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => обувь
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 1
                                        )
                                )
                        )
                )
        )
)
```
<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.percolate('products',{"query":{"percolate":{"documents":[{"title":"нормальная пара обуви","color":"синий"},{"title":"красивый пакет"}]}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381494',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title пакет'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [2]}},
                   {u'_id': u'2811025403043381496',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title обувь'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}

```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate('products',{"query":{"percolate":{"documents":[{"title":"нормальная пара обуви","color":"синий"},{"title":"красивый пакет"}]}}});
```
<!-- response javascript -->
``` javascript
{
  "took": 6,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233808,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title пакет"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            2
          ]
        }
      },
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233810,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title обувь"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
java
<!-- request Java -->

```java
percolateRequest = new PercolateRequest();
query = new HashMap<String,Object>(){{
        put("percolate",new HashMap<String,Object >(){{
            put("documents", new ArrayList<Object>(){{
                    add(new HashMap<String,Object >(){{
                        put("title","нормальная пара обуви");
                        put("color","синий");
                    }});
                    add(new HashMap<String,Object >(){{
                        put("title","красивый пакет");

                    }});

                     }});
        }});
    }};
percolateRequest.query(query);
searchApi.percolate("products",percolateRequest);

```
<!-- response Java -->
``` java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234133, _score=1, _source={query={ql=@title пакет}}, fields={_percolator_document_slot=[2]}}, {_index=products, _type=doc, _id=2811045522851234135, _score=1, _source={query={ql=@title обувь}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
var doc1 = new Dictionary<string, Object>();
doc1.Add("title","нормальная пара обуви");
doc1.Add("color","синий");
var doc2 = new Dictionary<string, Object>();
doc2.Add("title","красивый пакет");
var docs = new List<Object> {doc1, doc2};
Dictionary<string, Object> percolateDoc = new Dictionary<string, Object> {{ "documents", docs }}; 
Dictionary<string, Object> query = new Dictionary<string, Object> {{ "percolate", percolateDoc }}; 
PercolateRequest percolateRequest = new PercolateRequest(query=query);
searchApi.Percolate("products",percolateRequest);
```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234133, _score=1, _source={query={ql=@title пакет}}, fields={_percolator_document_slot=[2]}}, {_index=products, _type=doc, _id=2811045522851234135, _score=1, _source={query={ql=@title обувь}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
docs = [ {title : 'Какой красивый пакет'}, {title : 'Действительно красивые обуви'} ]; 
res = await searchApi.percolate('test_pq', { query: { percolate: { documents : docs } } } );
```
<!-- response TypeScript -->
``` typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title сумка"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title обувь"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
Идти
<!-- request Go -->

```go
doc1 := map[string]interface{} {"title": "Какой красивый сумка"}
doc2 := map[string]interface{} {"title": "Действительно красивые туфли"}
query := []interface{} {doc1, doc2}
percolateRequestQuery := manticoreclient.NewPercolateQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery)
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()

```
<!-- response Go -->
``` go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title сумка"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title обувь"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- end -->

<!-- example docs_id -->
#### Статические идентификаторы
По умолчанию, идентификаторы совпадающих документов соответствуют их относительным номерам в списке, который вы предоставляете. Однако в некоторых случаях каждый документ уже имеет свой собственный идентификатор. Для этого случая существует опция `'id field name' как docs_id` для `CALL PQ`.

Обратите внимание, что если идентификатор не может быть найден по предоставленному имени поля, правило PQ не будет показано в результатах.

Эта опция доступна только для `CALL PQ` через SQL.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CALL PQ('products', '[{"id": 123, "title": "красивые туфли", "color": "синий"}, {"id": 456, "title": "прекрасная сумка"}]', 1 as query, 'id' as docs_id, 1 as docs);
```
<!-- response SQL -->

```sql
+---------------------+-----------+--------------+------+---------------------------+
| id                  | documents | query        | tags | filters                   |
+---------------------+-----------+--------------+------+---------------------------+
| 1657852401006149664 | 456       | @title сумка |      |                           |
| 1657852401006149666 | 123       | @title обувь |      | color IN ('blue, 'green') |
+---------------------+-----------+--------------+------+---------------------------+
```
<!-- end -->

<!-- example invalid_json -->
##### У меня могут быть недопустимые JSON, пожалуйста, пропустите их

При использовании CALL PQ с отдельными JSON, вы можете использовать опцию 1 как skip_bad_json, чтобы пропустить любые недопустимые JSON в вводе. В примере ниже второй запрос не проходит из-за недопустимого JSON, но третий запрос избегает ошибки, используя 1 как skip_bad_json. Имейте в виду, что эта опция недоступна при отправке JSON-запросов через HTTP, так как весь JSON-запрос должен быть допустимым в этом случае.

<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products', ('{"title": "красивые туфли", "color": "синий"}', '{"title": "прекрасная сумка"}'));

CALL PQ('products', ('{"title": "красивые туфли", "color": "синий"}', '{"title": "прекрасная сумка}'));

CALL PQ('products', ('{"title": "красивые туфли", "color": "синий"}', '{"title": "прекрасная сумка}'), 1 as skip_bad_json);
```
<!-- response SQL -->

```sql
+---------------------+
| id                  |
+---------------------+
| 1657852401006149635 |
| 1657852401006149637 |
+---------------------+

ERROR 1064 (42000): Плохие JSON-объекты в строках: 2

+---------------------+
| id                  |
+---------------------+
| 1657852401006149635 |
+---------------------+
```

<!-- end -->

##### Я хочу более высокую производительность перколяционного запроса
Перколяционные запросы разработаны с учетом высокой пропускной способности и больших объемов данных. Чтобы оптимизировать производительность для более низкой задержки и большей пропускной способности, рассмотрите следующее.

Существуют два режима распределения для таблицы перколяции и того, как перколяционный запрос может работать с ней:

* **Разреженный (по умолчанию).** Идеально подходит для: множество документов, зеркалируемые таблицы PQ. Когда ваш набор документов большой, но набор запросов, хранящихся в таблице PQ, мал, разреженный режим полезен. В этом режиме пакет документов, который вы передаете, будет разделен между количеством агентов, поэтому каждый узел обрабатывает только часть документов из вашего запроса. Manticore разделяет ваш набор документов и распределяет части между зеркалами. Как только агенты завершают обработку запросов, Manticore собирает и объединяет результаты, возвращая окончательный набор запросов, как если бы он пришел из одной таблицы. Используйте [репликацию](../References.md#Replication), чтобы помочь процессу.
* **Разделенный.** Идеально подходит для: многих правил PQ, правил, распределенных между таблицами PQ. В этом режиме весь набор документов транслируется во все таблицы распределенной таблицы PQ без первоначального разделения документов. Это полезно при отправке относительно небольшого набора документов, но количество сохраненных запросов велико. В этом случае более целесообразно хранить только часть правил PQ на каждом узле, а затем объединить результаты, возвращенные узлами, которые обрабатывают один и тот же набор документов с разными наборами правил PQ. Этот режим должен быть явно установлен, так как это подразумевает увеличение сетевой нагрузки и ожидает таблицы с разными PQ, что [репликация](../References.md#Replication) не может сделать "из коробки".

Предположим, у вас есть таблица `pq_d2`, определенная как:

``` ini
table pq_d2
{
    type = distributed
    agent = 127.0.0.1:6712:pq
    agent = 127.0.0.1:6712:ptitle
}
```

<!-- example distributed pq modes 1 -->
Каждая из 'pq' и 'ptitle' содержит:


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT * FROM pq;
```

<!-- response sql -->

```sql
+------+-------------+------+-------------------+
| id   | query       | tags | filters           |
+------+-------------+------+-------------------+
|    1 | filter test |      | gid>=10           |
|    2 | angry       |      | gid>=10 OR gid<=3 |
+------+-------------+------+-------------------+
2 rows in set (0.01 sec)
```

<!-- request JSON -->

```json
POST /pq/pq/_search
```

<!-- response JSON -->

```json
{
    "took":0,
    "timed_out":false,
    "hits":{
        "total":2,
        "hits":[
            {
                "_id": 1,
                "_score":1,
                "_source":{
                    "query":{ "ql":"filter test" },
                    "tags":"",
                    "filters":"gid>=10"
                }
            },
            {
                "_id": 2,
                "_score":1,
                "_source":{
                    "query":{"ql":"angry"},
                    "tags":"",
                    "filters":"gid>=10 OR gid<=3"
                }
            }            
        ]
    }
}
```

<!-- request PHP -->

```php
$params = [
    'table' => 'pq',
    'body' => [
    ]
];
$response = $client->pq()->search($params);
```

<!-- response PHP -->

```php
(
    [took] => 0
    [timed_out] =>
    [hits] =>
        (
            [total] => 2
            [hits] =>
                (
                    [0] =>
                        (
                            [_id] => 1
                            [_score] => 1
                            [_source] =>
                                (
                                    [query] =>
                                        (
                                            [ql] => filter test
                                        )
                                    [tags] =>
                                    [filters] => gid>=10
                                )
                        ),
                    [1] =>
                        (
                            [_id] => 1
                            [_score] => 1
                            [_source] =>
                                (
                                    [query] =>
                                        (
                                            [ql] => angry
                                        )
                                    [tags] =>
                                    [filters] => gid>=10 OR gid<=3
                                )
                        )
                )
        )
)
```
<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.search({"table":"pq","query":{"match_all":{}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381501',
                    u'_score': 1,
                    u'_source': {u'filters': u"gid>=10",
                                 u'query': u'filter test',
                                 u'tags': u''}},
                   {u'_id': u'2811025403043381502',
                    u'_score': 1,
                    u'_source': {u'filters': u"gid>=10 OR gid<=3",
                                 u'query': u'angry',
                                 u'tags': u''}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"pq","query":{"match_all":{}}});
```
<!-- response javascript -->
``` javascript
{'hits': {'hits': [{u'_id': u'2811025403043381501',
                    u'_score': 1,
                    u'_source': {u'filters': u"gid>=10",
                                 u'query': u'filter test',
                                 u'tags': u''}},
                   {u'_id': u'2811025403043381502',
                    u'_score': 1,
                    u'_source': {u'filters': u"gid>=10 OR gid<=3",
                                 u'query': u'angry',
                                 u'tags': u''}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"pq","query":{"match_all":{}}});
```
<!-- response javascript -->
``` javascript
{"hits": {"hits": [{"_id": 2811025403043381501,
                    "_score": 1,
                    "_source": {"filters": u"gid>=10",
                                 "query": "filter test",
                                 "tags": ""}},
                   {"_id": 2811025403043381502,
                    "_score": 1,
                    "_source": {"filters": u"gid>=10 OR gid<=3",
                                 "query": "angry",
                                 "tags": ""}}],
          "total": 2},
  "timed_out": false,
 "took": 0}
```

<!-- intro -->
java
<!-- request Java -->

```java
Map<String,Object> query = new HashMap<String,Object>();
query.put("match_all",null);
SearchRequest searchRequest = new SearchRequest();
searchRequest.setIndex("pq");
searchRequest.setQuery(query);
SearchResponse searchResponse = searchApi.search(searchRequest);
```
<!-- response Java -->
``` java

class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: null
        hits: [{_id=2811045522851233962, _score=1, _source={filters=gid>=10, query=filter test, tags=}}, {_id=2811045522851233951, _score=1, _source={filters=gid>=10 OR gid<=3, query=angry,tags=}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
object query =  new { match_all=null };
SearchRequest searchRequest = new SearchRequest("pq", query);
SearchResponse searchResponse = searchApi.Search(searchRequest);
```
<!-- response C# -->
``` clike

class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: null
        hits: [{_id=2811045522851233962, _score=1, _source={filters=gid>=10, query=filter test, tags=}}, {_id=2811045522851233951, _score=1, _source={filters=gid>=10 OR gid<=3, query=angry,tags=}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
res = await searchApi.search({"table":"test_pq","query":{"match_all":{}}});
```
<!-- response TypeScript -->
``` typescript
{
	'hits':
	{
		'hits': 
		[{
			'_id': '2811025403043381501',
            '_score': 1,
            '_source': 
            {
            	'filters': "gid>=10",
                'query': 'filter test',
                'tags': ''
            }
        },
        {
         	'_id': 
         	'2811025403043381502',
            '_score': 1,
            '_source': 
            {
            	'filters': "gid>=10 OR gid<=3",
                 'query': 'angry',
                 'tags': ''
            }
        }],
    	'total': 2
	},
	'profile': None,
	'timed_out': False,
	'took': 0
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
query := map[string]interface{} {}
percolateRequestQuery := manticoreclient.NewPercolateRequestQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery) 
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()
```
<!-- response Go -->
``` go
{
	'hits':
	{
		'hits': 
		[{
			'_id': '2811025403043381501',
            '_score': 1,
            '_source': 
            {
            	'filters': "gid>=10",
                'query': 'filter test',
                'tags': ''
            }
        },
        {
         	'_id': 
         	'2811025403043381502',
            '_score': 1,
            '_source': 
            {
            	'filters': "gid>=10 OR gid<=3",
                 'query': 'angry',
                 'tags': ''
            }
        }],
    	'total': 2
	},
	'profile': None,
	'timed_out': False,
	'took': 0
}
```

<!-- end -->


<!-- example call_pq_example -->

And you execute `CALL PQ` on the distributed table with a couple of documents.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CALL PQ ('pq_d2', ('{"title":"angry test", "gid":3 }', '{"title":"filter test doc2", "gid":13}'), 1 AS docs);
```

<!-- response sql -->

```sql
+------+-----------+
| id   | documents |
+------+-----------+
|    1 | 2         |
|    2 | 1         |
+------+-----------+
```

<!-- request JSON -->

```json
POST /pq/pq/_search -d '
"query":
{
        "percolate":
        {
                "documents" : [
                    { "title": "angry test", "gid": 3 },
                    { "title": "filter test doc2", "gid": 13 }
                ]
        }
}
'
```

<!-- response JSON -->

```json
{
    "took":0,
    "timed_out":false,
    "hits":{
    "total":2,"hits":[
        {
            "_id": 2,
            "_score":1,
            "_source":{
                "query":{"title":"angry"},
                "tags":"",
                "filters":"gid>=10 OR gid<=3"
            }
        }
        {
            "_id": 1,
            "_score":1,
            "_source":{
                "query":{"ql":"filter test"},
                "tags":"",
                "filters":"gid>=10"
            }
        },
        ]
    }
}
```

<!-- request PHP -->

```php
$params = [
    'table' => 'pq',
    'body' => [
        'query' => [
            'percolate' => [
                'documents' => [
                    [
                        'title'=>'angry test',
                        'gid' => 3
                    ],
                    [
                        'title'=>'filter test doc2',
                        'gid' => 13
                    ],
                ]
            ]
        ]
    ]
];
$response = $client->pq()->search($params);
```

<!-- response PHP -->

```php
(
    [took] => 0
    [timed_out] =>
    [hits] =>
        (
            [total] => 2
            [hits] =>
                (
                    [0] =>
                        (
                            [_index] => pq  
                            [_type] => doc                            
                            [_id] => 2
                            [_score] => 1
                            [_source] =>
                                (
                                    [query] =>
                                        (
                                            [ql] => angry
                                        )
                                    [tags] =>
                                    [filters] => gid>=10 OR gid<=3
                                ),
                           [fields] =>
                                (
                                    [_percolator_document_slot] =>
                                        (
                                            [0] => 1
                                        )

                                )
                        ),
                    [1] =>
                        (
                            [_index] => pq                            
                            [_id] => 1
                            [_score] => 1
                            [_source] =>
                                (
                                    [query] =>
                                        (
                                            [ql] => filter test
                                        )
                                    [tags] =>
                                    [filters] => gid>=10
                                )
                           [fields] =>
                                (
                                    [_percolator_document_slot] =>
                                        (
                                            [0] => 0
                                        )

                                )
                        )
                )
        )
)
```
<!-- intro -->
Питон
<!-- request Python -->

```python
searchApi.percolate('pq',{"percolate":{"documents":[{"title":"angry test","gid":3},{"title":"filter test doc2","gid":13}]}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'table': u'pq',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'angry'},u'tags':u'',u'filters':u"gid>=10 OR gid<=3"},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}},
                    {u'_id': u'2811025403043381501',
                    u'table': u'pq',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'filter test'},u'tags':u'',u'filters':u"gid>=10"},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate('pq',{"percolate":{"documents":[{"title":"angry test","gid":3},{"title":"filter test doc2","gid":13}]}});
```
<!-- response javascript -->
``` javascript
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'table': u'pq',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'angry'},u'tags':u'',u'filters':u"gid>=10 OR gid<=3"},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}},
                    {u'_id': u'2811025403043381501',
                    u'table': u'pq',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'filter test'},u'tags':u'',u'filters':u"gid>=10"},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
java
<!-- request Java -->

```java
percolateRequest = new PercolateRequest();
query = new HashMap<String,Object>(){{
    put("percolate",new HashMap<String,Object >(){{
        put("documents", new ArrayList<Object>(){{
            add(new HashMap<String,Object >(){{
                put("title","angry test");
                put("gid",3);
            }});
            add(new HashMap<String,Object >(){{
                put("title","filter test doc2");
                put("gid",13);
            }});
        }});
    }});
}};
percolateRequest.query(query);
searchApi.percolate("pq",percolateRequest);
```
<!-- response java -->
``` java
class SearchResponse {
    took: 10
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=pq, _type=doc, _id=2811045522851234165, _score=1, _source={query={ql=@title angry}}, fields={_percolator_document_slot=[1]}}, {_index=pq, _type=doc, _id=2811045522851234166, _score=1, _source={query={ql=@title filter test doc2}}, fields={_percolator_document_slot=[2]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
var doc1 = new Dictionary<string, Object>();
doc1.Add("title","angry test");
doc1.Add("gid",3);
var doc2 = new Dictionary<string, Object>();
doc2.Add("title","filter test doc2");
doc2.Add("gid",13);
var docs = new List<Object> {doc1, doc2};
Dictionary<string, Object> percolateDoc = new Dictionary<string, Object> {{ "documents", docs }}; 
Dictionary<string, Object> query = new Dictionary<string, Object> {{ "percolate", percolateDoc }}; 
PercolateRequest percolateRequest = new PercolateRequest(query=query);
searchApi.Percolate("pq",percolateRequest);
```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 10
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=pq, _type=doc, _id=2811045522851234165, _score=1, _source={query={ql=@title angry}}, fields={_percolator_document_slot=[1]}}, {_index=pq, _type=doc, _id=2811045522851234166, _score=1, _source={query={ql=@title filter test doc2}}, fields={_percolator_document_slot=[2]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
docs = [ {title : 'Что за красивая сумка'}, {title : 'Действительно красивые ботинки'} ]; 
res = await searchApi.percolate('test_pq', { query: { percolate: { documents : docs } } } );
```
<!-- response TypeScript -->
``` typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title сумка"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title обувь"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
doc1 := map[string]interface{} {"title": "Что за красивая сумка"}
doc2 := map[string]interface{} {"title": "Действительно красивые ботинки"}
query := []interface{} {doc1, doc2}
percolateRequestQuery := manticoreclient.NewPercolateQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery)
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()

```
<!-- response Go -->
``` go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title сумка"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title обувь"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- end -->

In the previous example, we used the default **sparse** mode. To demonstrate the **sharded** mode, let's create a distributed PQ table consisting of 2 local PQ tables and add 2 documents to "products1" and 1 document to "products2":
```sql
create table products1(title text, color string) type='pq';
create table products2(title text, color string) type='pq';
create table products_distributed type='distributed' local='products1' local='products2';

INSERT INTO products1(query) values('@title сумка');
INSERT INTO products1(query,filters) values('@title обувь', 'color='red'');
INSERT INTO products2(query,filters) values('@title обувь', 'color in ('blue', 'green')');
```

<!-- example sharded -->
Now, if you add `'sharded' as mode` to `CALL PQ`, it will send the documents to all the agent's tables (in this case, just local tables, but they can be remote to utilize external hardware). This mode is not available via the JSON interface.

<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products_distributed', ('{"title": "красивые ботинки", "color": "blue"}', '{"title": "красивую сумку"}'), 'sharded' as mode, 1 as query);
```
<!-- response SQL -->

```sql
+---------------------+--------------+------+---------------------------+
| id                  | query        | tags | filters                   |
+---------------------+--------------+------+---------------------------+
| 1657852401006149639 | @title сумка |      |                           |
| 1657852401006149643 | @title обувь |      | color IN ('blue, 'green') |
+---------------------+--------------+------+---------------------------+
```

<!-- end -->

Note that the syntax of agent mirrors in the configuration (when several hosts are assigned to one `agent` line, separated with `|`) has nothing to do with the `CALL PQ` query mode. Each `agent` always represents **one** node, regardless of the number of HA mirrors specified for that agent.

<!-- example verbose -->
##### How can I learn more about performance?
In some cases, you might want to get more details about the performance of a percolate query. For that purpose, there is the option `1 as verbose`, which is only available via SQL and allows you to save more performance metrics. You can see them using the `SHOW META` query, which you can run after `CALL PQ`. See [SHOW META](../Node_info_and_management/SHOW_META.md) for more info.

<!-- intro -->
1 as verbose:
<!-- request 1 as verbose -->

```sql
CALL PQ('products', ('{"title": "красивые ботинки", "color": "blue"}', '{"title": "красивую сумку"}'), 1 as verbose); show meta;
```
<!-- response 1 as verbose -->

```sql
+---------------------+
| id                  |
+---------------------+
| 1657852401006149644 |
| 1657852401006149646 |
+---------------------+
+-------------------------+-----------+
| Название переменной     | Значение  |
+-------------------------+-----------+
| всего                   | 0.000 сек |
| настройка               | 0.000 сек |
| совпадения_запросов     | 2         |
| неудачные_запросы      | 0         |
| совпадения_документов   | 2         |
| всего_хранимых_запросов | 3         |
| только_терминальные_запросы | 3    |
| быстро_отклоненные_запросы | 0     |
| время_на_запрос        | 27, 10    |
| время_совпавших_запросов | 37      |
+-------------------------+-----------+
```
<!-- intro -->
0 as verbose (default):
<!-- request 0 as verbose -->

```sql
CALL PQ('products', ('{"title": "nice pair of shoes", "color": "blue"}', '{"title": "beautiful bag"}'), 0 as verbose); show meta;
```
<!-- response 0 as verbose -->

```sql
+---------------------+
| id                  |
+---------------------+
| 1657852401006149644 |
| 1657852401006149646 |
+---------------------+
+-----------------------+-----------+
| Название переменной   | Значение  |
+-----------------------+-----------+
| всего                 | 0.000 сек |
| совпадения_запросов   | 2         |
| неудачные_запросы    | 0         |
| совпадения_документов | 2         |
| всего_хранимых_запросов | 3       |
| только_терминальные_запросы | 3   |
| быстро_отклоненные_запросы | 0     |
+-----------------------+-----------+
```
<!-- end -->
<!-- proofread -->
# Перколятор Запрос

Перколятор запросы также известны как Постоянные запросы, Перспективный поиск, маршрутизация документов, поиск в обратном направлении и обратный поиск.

Традиционный способ проведения поисков включает в себя хранение документов и выполнение запросов на поиск против них. Тем не менее, есть ситуации, когда мы хотим применить запрос к вновь поступившему документу, чтобы сигнализировать о совпадении. Некоторые сценарии, где это необходимо, включают в себя мониторинговые системы, которые собирают данные и уведомляют пользователей о конкретных событиях, таких как достижение определенного порога для метрики или появление конкретного значения в контролируемых данных. Другой пример — это агрегация новостей, где пользователи могут захотеть получать уведомления только о некоторых категориях или темах, или даже о конкретных «ключевых словах».

В этих ситуациях традиционный поиск не является наилучшим решением, поскольку предполагается, что желаемый поиск выполняется по всей коллекции. Этот процесс умножается на количество пользователей, что приводит к многим запросам, работающим по всей коллекции, что может вызвать значительную дополнительную нагрузку. Альтернативный подход, описанный в этом разделе, заключается в том, чтобы вместо этого хранить запросы и тестировать их против вновь поступившего документа или партии документов.

Google Alerts, AlertHN, Bloomberg Terminal и другие системы, которые позволяют пользователям подписываться на определенный контент, используют аналогичную технологию.

> * См. [перколятор](../Creating_a_table/Local_tables/Percolate_table.md) для получения информации о создании таблицы PQ.
> * См. [Добавление правил в таблицу перколяторов](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md), чтобы узнать, как добавить правила перколяторов (также известные как правила PQ). Вот быстрый пример:


### Выполнение запроса перколятора с CALL PQ

Ключевое, что нужно помнить о перколятор запросах, это то, что ваши поисковые запросы уже находятся в таблице. То, что вам нужно предоставить, это документы **для проверки, совпадают ли какие-либо из них с любыми из сохраненных правил**.

Вы можете выполнить запрос перколятора через SQL или JSON интерфейсы, а также с использованием клиентов языков программирования. Подход SQL предлагает больше гибкости, в то время как метод HTTP проще и предоставляет большинство необходимого. Таблица ниже может помочь вам понять различия.

| Желаемое поведение          | SQL                                    | HTTP                                 |
| --------------------------- | -------------------------------------- | ------------------------------------ |
| Предоставить один документ  | `CALL PQ('tbl', '{doc1}')`              | `query.percolate.document{doc1}`     |
| Предоставить один документ (альтернатива) | `CALL PQ('tbl', 'doc1', 0 as docs_json)` | - |
| Предоставить несколько документов | `CALL PQ('tbl', ('doc1', 'doc2'), 0 as docs_json)` | - |
| Предоставить несколько документов (альтернатива) | `CALL PQ('tbl', ('{doc1}', '{doc2}'))` | - |
| Предоставить несколько документов (альтернатива) | `CALL PQ('tbl', '[{doc1}, {doc2}]')` | - |
| Вернуть соответствующие идентификаторы документов | 0/1 as docs (отключено по умолчанию)       | Включено по умолчанию                   |
| Использовать собственный идентификатор документа для отображения в результате | 'id field' as docs_id (отключено по умолчанию) | Не доступно |
| Предположить, что входные документы являются JSON | 1 as docs_json (1 по умолчанию) | Включено по умолчанию |
| Предположить, что входные документы являются обычным текстом | 0 as docs_json (1 по умолчанию) | Не доступно |
| [Режим разреженного распределения](../Searching/Percolate_query.md#I-want-higher-performance-of-a-percolate-query) | по умолчанию | по умолчанию |
| [Режим шардированного распределения](../Searching/Percolate_query.md#I-want-higher-performance-of-a-percolate-query) | шардированный как режим | Не доступно |
| Вернуть всю информацию о совпадающем запросе | 1 as query (0 по умолчанию) | Включено по умолчанию |
| Пропустить недействительный JSON | 1 as skip_bad_json (0 по умолчанию) | Не доступно |
| Расширенная информация в [SHOW META](../Node_info_and_management/SHOW_META.md) | 1 as verbose (0 по умолчанию) | Не доступно |
| Определить число, которое будет добавлено к идентификаторам документов, если поля docs_id не предоставлены (в основном актуально в [распределенных режимах PQ](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#Distributed-percolate-tables-%28DPQ-tables%29)) | 1 as shift (0 по умолчанию) | Не доступно |

<!-- example create percolate -->
Чтобы продемонстрировать, как это работает, вот несколько примеров. Давайте создадим таблицу PQ с двумя полями:

* title (текст)
* color (строка)

и тремя правилами в ней:

* Просто полный текст. Запрос: `@title bag`
* Полный текст и фильтрация. Запрос: `@title shoes`. Фильтры: `color='red'`
* Полный текст и более сложная фильтрация. Запрос: `@title shoes`. Фильтры: `color IN('blue', 'green')`

<!-- intro -->
#### SQL
<!-- request SQL -->

```sql
CREATE TABLE products(title text, color string) type='pq';
INSERT INTO products(query) values('@title bag');
INSERT INTO products(query,filters) values('@title shoes', 'color=\'red\'');
INSERT INTO products(query,filters) values('@title shoes', 'color in (\'blue\', \'green\')');
select * from products;
```

<!-- response SQL -->

```sql
+---------------------+--------------+------+---------------------------+
| id                  | query        | tags | filters                   |
+---------------------+--------------+------+---------------------------+
| 1657852401006149635 | @title shoes |      | color IN ('blue, 'green') |
| 1657852401006149636 | @title shoes |      | color='red'               |
| 1657852401006149637 | @title bag   |      |                           |
+---------------------+--------------+------+---------------------------+
```

<!-- request JSON -->

```json
PUT /pq/products/doc/
{
  "query": {
    "match": {
      "title": "bag"
    }
  },
  "filters": ""
}

PUT /pq/products/doc/
{
  "query": {
    "match": {
      "title": "shoes"
    }
  },
  "filters": "color='red'"
}

PUT /pq/products/doc/
{
  "query": {
    "match": {
      "title": "shoes"
    }
  },
  "filters": "color IN ('blue', 'green')"
}
```

<!-- intro -->
#### JSON
<!-- response JSON -->

```json
{
  "table": "products",
  "type": "doc",
  "_id": 1657852401006149661,
  "result": "created"
}
{
  "table": "products",
  "type": "doc",
  "_id": 1657852401006149662,
  "result": "created"
}
{
  "table": "products",
  "type": "doc",
  "_id": 1657852401006149663,
  "result": "created"
}
```

<!-- intro -->
#### PHP
<!-- request PHP -->

```php

$index = [
    'table' => 'products',
    'body' => [
        'columns' => [
            'title' => ['type' => 'text'],
            'color' => ['type' => 'string']
        ],
        'settings' => [
            'type' => 'pq'
        ]
    ]
];
$client->indices()->create($index);

$query = [
    'table' => 'products',
    'body' => [ 'query'=>['match'=>['title'=>'bag']]]
];
$client->pq()->doc($query);
$query = [
    'table' => 'products',
    'body' => [ 'query'=>['match'=>['title'=>'shoes']],'filters'=>"color='red'"]
];
$client->pq()->doc($query);


$query = [
    'table' => 'products',
    'body' => [ 'query'=>['match'=>['title'=>'shoes']],'filters'=>"color IN ('blue', 'green')"]
];
$client->pq()->doc($query);
```
<!-- response PHP -->
``` php
Array(
  [table] => products
  [type] => doc
  [_id] => 1657852401006149661
  [result] => created
)
Array(
  [table] => products
  [type] => doc
  [_id] => 1657852401006149662
  [result] => created
)
Array(
  [table] => products
  [type] => doc
  [_id] => 1657852401006149663
  [result] => created
)
```
<!-- intro -->
Python
<!-- request Python -->

```python
utilsApi.sql('create table products(title text, color string) type=\'pq\'')
indexApi.insert({"table" : "products", "doc" : {"query" : "@title bag" }})
indexApi.insert({"table" : "products",  "doc" : {"query" : "@title shoes", "filters": "color='red'" }})
indexApi.insert({"table" : "products",  "doc" : {"query" : "@title shoes","filters": "color IN ('blue', 'green')" }})
```
<!-- response Python -->
``` python
{'created': True,
 'found': None,
 'id': 0,
 'table': 'products',
 'result': 'created'}
{'created': True,
 'found': None,
 'id': 0,
 'table': 'products',
 'result': 'created'}
{'created': True,
 'found': None,
 'id': 0,
 'table': 'products',
 'result': 'created'}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await utilsApi.sql('create table products(title text, color string) type=\'pq\'');
res = indexApi.insert({"table" : "products", "doc" : {"query" : "@title bag" }});
res = indexApi.insert({"table" : "products",  "doc" : {"query" : "@title shoes", "filters": "color='red'" }});
res = indexApi.insert({"table" : "products",  "doc" : {"query" : "@title shoes","filters": "color IN ('blue', 'green')" }});
```
<!-- response javascript -->
``` javascript
"table":"products","_id":0,"created":true,"result":"created"}
{"table":"products","_id":0,"created":true,"result":"created"}
{"table":"products","_id":0,"created":true,"result":"created"}
```
<!-- intro -->
java
<!-- request Java -->

```java
utilsApi.sql("create table products(title text, color string) type='pq'");
doc = new HashMap<String,Object>(){{
    put("query", "@title bag");
}};
newdoc = new InsertDocumentRequest();
newdoc.index("products").setDoc(doc);
indexApi.insert(newdoc);

doc = new HashMap<String,Object>(){{
    put("query", "@title shoes");
    put("filters", "color='red'");
}};
newdoc = new InsertDocumentRequest();
newdoc.index("products").setDoc(doc);
indexApi.insert(newdoc);

doc = new HashMap<String,Object>(){{
    put("query", "@title shoes");
    put("filters", "color IN ('blue', 'green')");
}};
newdoc = new InsertDocumentRequest();
newdoc.index("products").setDoc(doc);
indexApi.insert(newdoc);
```
<!-- response Java -->
``` java
{total=0, error=, warning=}
class SuccessResponse {
    index: products
    id: 0
    created: true
    result: created
    found: null
}
class SuccessResponse {
    index: products
    id: 0
    created: true
    result: created
    found: null
}
class SuccessResponse {
    index: products
    id: 0
    created: true
    result: created
    found: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
utilsApi.Sql("create table products(title text, color string) type='pq'");

Dictionary<string, Object> doc = new Dictionary<string, Object>(); 
doc.Add("query", "@title bag");
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", doc: doc);
indexApi.Insert(newdoc);

doc = new Dictionary<string, Object>(); 
doc.Add("query", "@title shoes");
doc.Add("filters", "color='red'");
newdoc = new InsertDocumentRequest(index: "products", doc: doc);
indexApi.Insert(newdoc);

doc = new Dictionary<string, Object>(); 
doc.Add("query", "@title bag");
doc.Add("filters", "color IN ('blue', 'green')");
newdoc = new InsertDocumentRequest(index: "products", doc: doc);
indexApi.Insert(newdoc);
```
<!-- response C# -->
``` clike
{total=0, error="", warning=""}

class SuccessResponse {
    index: products
    id: 0
    created: true
    result: created
    found: null
}
class SuccessResponse {
    index: products
    id: 0
    created: true
    result: created
    found: null
}
class SuccessResponse {
    index: products
    id: 0
    created: true
    result: created
    found: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->
```typescript
res = await utilsApi.sql("создать таблицу test_pq(title text, color string) type='pq'");
res = indexApi.insert({
  index: 'test_pq',
  doc: { query : '@title bag' }
});
res = indexApi.insert(
  index: 'test_pq',
  doc: { query: '@title shoes', filters: "color='red'" }
});
res = indexApi.insert({
  index: 'test_pq',
  doc: { query : '@title shoes', filters: "color IN ('blue', 'green')" }
});
```
<!-- response TypeScript -->
``` typescript
{
	"table":"test_pq",
	"_id":1657852401006149661,
	"created":true,
	"result":"создано"
}
{
	"table":"test_pq",
	"_id":1657852401006149662,
	"created":true,
	"result":"создано"
}
{
	"table":"test_pq",
	"_id":1657852401006149663,
	"created":true,
	"result":"создано"
}
```

<!-- intro -->
Go
<!-- request Go -->
```go
apiClient.UtilsAPI.Sql(context.Background()).Body("создать таблицу test_pq(title text, color string) type='pq'").Execute()

indexDoc := map[string]interface{} {"query": "@title bag"}
indexReq := manticoreclient.NewInsertDocumentRequest("test_pq", indexDoc)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute();

indexDoc = map[string]interface{} {"query": "@title shoes", "filters": "color='red'"}
indexReq = manticoreclient.NewInsertDocumentRequest("test_pq", indexDoc)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute();

indexDoc = map[string]interface{} {"query": "@title shoes", "filters": "color IN ('blue', 'green')"}
indexReq = manticoreclient.NewInsertDocumentRequest("test_pq", indexDoc)
apiClient.IndexAPI.Insert(context.Background()).InsertDocumentRequest(*indexReq).Execute();
```
<!-- response Go -->
``` go
{
	"table":"test_pq",
	"_id":1657852401006149661,
	"created":true,
	"result":"создано"
}
{
	"table":"test_pq",
	"_id":1657852401006149662,
	"created":true,
	"result":"создано"
}
{
	"table":"test_pq",
	"_id":1657852401006149663,
	"created":true,
	"result":"создано"
}
```

<!-- end -->

<!-- example single -->
##### Просто скажите мне, какие правила PQ соответствуют моему единственному документу

Первый документ не соответствует никаким правилам. Он мог бы соответствовать первым двум, но для этого требуются дополнительные фильтры.

Второй документ соответствует одному правилу. Обратите внимание, что CALL PQ по умолчанию ожидает, что документ будет в формате JSON, но если вы используете `0 as docs_json`, можете передать обычную строку вместо этого.

<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products', 'Красивые туфли', 0 as docs_json);

CALL PQ('products', 'Какой хороший мешок', 0 as docs_json);
CALL PQ('products', '{"title": "Какой хороший мешок"}');
```
<!-- response SQL -->

```sql
+---------------------+
| id                  |
+---------------------+
| 1657852401006149637 |
+---------------------+

+---------------------+
| id                  |
+---------------------+
| 1657852401006149637 |
+---------------------+
```
<!-- intro -->
JSON:
<!-- request JSON -->

```json
POST /pq/products/_search
{
  "query": {
    "percolate": {
      "document": {
        "title": "Какой хороший мешок"
      }
    }
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
    "max_score": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149644,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
PHP:
<!-- request PHP -->

```php
$percolate = [
    'table' => 'products',
    'body' => [
        'query' => [
            'percolate' => [
                'document' => [
                    'title' => 'Какой хороший мешок'
                ]
            ]
        ]
    ]
];
$client->pq()->search($percolate);

```
<!-- response PHP -->

```php
Array
(
    [took] => 0
    [timed_out] =>
    [hits] => Array
        (
            [total] => 1
            [max_score] => 1
            [hits] => Array
                (
                    [0] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 1657852401006149644
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => bag
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 1
                                        )
                                )
                        )
                )
        )
)
```
<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"Какой хороший мешок"}}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title bag'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"Какой милый сумка"}}}});
```
<!-- response javascript -->
``` javascript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233808,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
java
<!-- request Java -->

```java
PercolateRequest percolateRequest = new PercolateRequest();
query = new HashMap<String,Object>(){{
    put("percolate",new HashMap<String,Object >(){{
        put("document", new HashMap<String,Object >(){{
            put("title","какая милая сумка");
        }});
    }});
}};
percolateRequest.query(query);
searchApi.percolate("test_pq",percolateRequest);
```
<!-- response Javs -->
``` java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234109, _score=1, _source={query={ql=@title bag}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
Dictionary<string, Object> percolateDoc = new Dictionary<string, Object>(); 
percolateDoc.Add("document", new Dictionary<string, Object> {{ "title", "какая милая сумка" }});
Dictionary<string, Object> query = new Dictionary<string, Object> {{ "percolate", percolateDoc }}; 
PercolateRequest percolateRequest = new PercolateRequest(query=query);
searchApi.Percolate("test_pq",percolateRequest);

```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234109, _score=1, _source={query={ql=@title bag}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
res = await searchApi.percolate('test_pq', { query: { percolate: { document : { title : 'Какой милый сумка' } } } } );
```
<!-- response TypeScript -->
``` typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
query := map[string]interface{} {"title": "какая милая сумка"}
percolateRequestQuery := manticoreclient.NewPercolateQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery)
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()

```
<!-- response Go -->
``` go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- end -->

<!-- example pq_rules -->
##### Я хочу знать полные правила PQ, соответствующие моему документу
<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products', '{"title": "Какой милый сумка"}', 1 as query);
```
<!-- response SQL -->

```sql
+---------------------+------------+------+---------+
| id                  | query      | tags | filters |
+---------------------+------------+------+---------+
| 1657852401006149637 | @title bag |      |         |
+---------------------+------------+------+---------+
```
<!-- intro -->
JSON:
<!-- request JSON -->

```json
POST /pq/products/_search
{
  "query": {
    "percolate": {
      "document": {
        "title": "Какой милый сумка"
      }
    }
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
    "max_score": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149644,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
PHP:
<!-- request PHP -->

```php
$percolate = [
    'table' => 'products',
    'body' => [
        'query' => [
            'percolate' => [
                'document' => [
                    'title' => 'Какой красивый сумка'
                ]
            ]
        ]
    ]
];
$client->pq()->search($percolate);

```
<!-- response PHP -->

```php
Array
(
    [took] => 0
    [timed_out] =>
    [hits] => Array
        (
            [total] => 1
            [max_score] => 1
            [hits] => Array
                (
                    [0] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 1657852401006149644
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => сумка
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 1
                                        )
                                )
                        )
                )
        )
)
```

<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"Какой красивый сумка"}}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title сумка'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"Какой красивый сумка"}}}});
```
<!-- response javascript -->
``` javascript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233808,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title сумка"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
java
<!-- request Java -->

```java
PercolateRequest percolateRequest = new PercolateRequest();
query = new HashMap<String,Object>(){{
    put("percolate",new HashMap<String,Object >(){{
        put("document", new HashMap<String,Object >(){{
            put("title","как хороший сумка");
        }});
    }});
}};
percolateRequest.query(query);
searchApi.percolate("test_pq",percolateRequest);
```
<!-- response Java -->
``` java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234109, _score=1, _source={query={ql=@title сумка}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
Dictionary<string, Object> percolateDoc = new Dictionary<string, Object>(); 
percolateDoc.Add("document", new Dictionary<string, Object> {{ "title", "как хороший сумка" }});
Dictionary<string, Object> query = new Dictionary<string, Object> {{ "percolate", percolateDoc }}; 
PercolateRequest percolateRequest = new PercolateRequest(query=query);
searchApi.Percolate("test_pq",percolateRequest);

```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 1
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234109, _score=1, _source={query={ql=@title сумка}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
res = await searchApi.percolate('test_pq', { query: { percolate: { document : { title : 'Какой красивый сумка' } } } } );
```
<!-- response TypeScript -->
``` typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title сумка"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
query := map[string]interface{} {"title": "как хороший сумка"}
percolateRequestQuery := manticoreclient.NewPercolateQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery)
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()

```
<!-- response Go -->
``` go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- end -->

<!-- пример несколько -->

##### Как насчет нескольких документов?

Обратите внимание, что с помощью `CALL PQ` вы можете предоставить несколько документов разными способами:

* в виде массива простых документов в круглых скобках `('doc1', 'doc2')`. Это требует `0 as docs_json`
* в виде массива JSON в круглых скобках `('{doc1}', '{doc2}')`
* или в виде стандартного JSON массива `'[{doc1}, {doc2}]'`

<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products', ('nice pair of shoes', 'beautiful bag'), 1 as query, 0 as docs_json);

CALL PQ('products', ('{"title": "nice pair of shoes", "color": "red"}', '{"title": "beautiful bag"}'), 1 as query);

CALL PQ('products', '[{"title": "nice pair of shoes", "color": "blue"}, {"title": "beautiful bag"}]', 1 as query);
```
<!-- response SQL -->

```sql
+---------------------+------------+------+---------+
| id                  | query      | tags | filters |
+---------------------+------------+------+---------+
| 1657852401006149637 | @title bag |      |         |
+---------------------+------------+------+---------+

+---------------------+--------------+------+-------------+
| id                  | query        | tags | filters     |
+---------------------+--------------+------+-------------+
| 1657852401006149636 | @title shoes |      | color='red' |
| 1657852401006149637 | @title bag   |      |             |
+---------------------+--------------+------+-------------+

+---------------------+--------------+------+---------------------------+
| id                  | query        | tags | filters                   |
+---------------------+--------------+------+---------------------------+
| 1657852401006149635 | @title shoes |      | color IN ('blue, 'green') |
| 1657852401006149637 | @title bag   |      |                           |
+---------------------+--------------+------+---------------------------+
```
<!-- intro -->
JSON:
<!-- request JSON -->

```json
POST /pq/products/_search
{
  "query": {
    "percolate": {
      "documents": [
        {"title": "nice pair of shoes", "color": "blue"},
        {"title": "beautiful bag"}
      ]
    }
  }
}
```
<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "max_score": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149644,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            2
          ]
        }
      },
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149646,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
PHP:
<!-- request PHP -->

```php
$percolate = [
    'table' => 'products',
    'body' => [
        'query' => [
            'percolate' => [
                'documents' => [
                    ['title' => 'nice pair of shoes','color'=>'blue'],
                    ['title' => 'beautiful bag']
                ]
            ]
        ]
    ]
];
$client->pq()->search($percolate);

```
<!-- response PHP -->

```php
Array
(
    [took] => 23
    [timed_out] =>
    [hits] => Array
        (
            [total] => 2
            [max_score] => 1
            [hits] => Array
                (
                    [0] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 2810781492890828819
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => bag
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 2
                                        )
                                )
                        )
                    [1] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 2810781492890828821
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => shoes
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 1
                                        )
                                )
                        )
                )
        )
)
```
<!-- intro -->
Питон
<!-- request Python -->

```python
searchApi.percolate('products',{"query":{"percolate":{"documents":[{"title":"nice pair of shoes","color":"blue"},{"title":"beautiful bag"}]}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381494',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title bag'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [2]}},
                   {u'_id': u'2811025403043381496',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title shoes'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}

```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate('products',{"query":{"percolate":{"documents":[{"title":"nice pair of shoes","color":"blue"},{"title":"beautiful bag"}]}}});
```
<!-- response javascript -->
``` javascript
{
  "took": 6,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233808,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            2
          ]
        }
      },
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233810,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}

```

<!-- intro -->
java
<!-- request Java -->

```java
percolateRequest = new PercolateRequest();
query = new HashMap<String,Object>(){{
        put("percolate",new HashMap<String,Object >(){{
            put("documents", new ArrayList<Object>(){{
                    add(new HashMap<String,Object >(){{
                        put("title","nice pair of shoes");
                        put("color","blue");
                    }});
                    add(new HashMap<String,Object >(){{
                        put("title","beautiful bag");

                    }});

                     }});
        }});
    }};
percolateRequest.query(query);
searchApi.percolate("products",percolateRequest);

```
<!-- response Java -->
``` java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234133, _score=1, _source={query={ql=@title bag}}, fields={_percolator_document_slot=[2]}}, {_index=products, _type=doc, _id=2811045522851234135, _score=1, _source={query={ql=@title shoes}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
var doc1 = new Dictionary<string, Object>();
doc1.Add("title","nice pair of shoes");
doc1.Add("color","blue");
var doc2 = new Dictionary<string, Object>();
doc2.Add("title","beautiful bag");
var docs = new List<Object> {doc1, doc2};
Dictionary<string, Object> percolateDoc = new Dictionary<string, Object> {{ "documents", docs }}; 
Dictionary<string, Object> query = new Dictionary<string, Object> {{ "percolate", percolateDoc }}; 
PercolateRequest percolateRequest = new PercolateRequest(query=query);
searchApi.Percolate("products",percolateRequest);
```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234133, _score=1, _source={query={ql=@title bag}}, fields={_percolator_document_slot=[2]}}, {_index=products, _type=doc, _id=2811045522851234135, _score=1, _source={query={ql=@title shoes}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
docs = [ {title : 'What a nice bag'}, {title : 'Really nice shoes'} ]; 
res = await searchApi.percolate('test_pq', { query: { percolate: { documents : docs } } } );
```
<!-- response TypeScript -->
``` typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
Идти
<!-- request Go -->

```go
doc1 := map[string]interface{} {"title": "Какой милый мешок"}
doc2 := map[string]interface{} {"title": "Действительно красивые туфли"}
query := []interface{} {doc1, doc2}
percolateRequestQuery := manticoreclient.NewPercolateQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery)
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()

```
<!-- response Go -->
``` go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- end -->

<!-- example docs_1 -->
##### Я хочу знать, какие документы соответствуют каким правилам

Использование опции `1 as docs` позволяет вам увидеть, какие документы из предоставленных соответствуют каким правилам.
<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products', '[{"title": "nice pair of shoes", "color": "blue"}, {"title": "beautiful bag"}]', 1 as query, 1 as docs);
```

<!-- response SQL -->

```sql
+---------------------+-----------+--------------+------+---------------------------+
| id                  | documents | query        | tags | filters                   |
+---------------------+-----------+--------------+------+---------------------------+
| 1657852401006149635 | 1         | @title shoes |      | color IN ('blue, 'green') |
| 1657852401006149637 | 2         | @title bag   |      |                           |
+---------------------+-----------+--------------+------+---------------------------+
```
<!-- intro -->
JSON:
<!-- request JSON -->

```json
POST /pq/products/_search
{
  "query": {
    "percolate": {
      "documents": [
        {"title": "nice pair of shoes", "color": "blue"},
        {"title": "beautiful bag"}
      ]
    }
  }
}
```
<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "max_score": 1,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149644,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            2
          ]
        }
      },
      {
        "table": "products",
        "_type": "doc",
        "_id": 1657852401006149646,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
PHP:
<!-- request PHP -->

```php
$percolate = [
    'table' => 'products',
    'body' => [
        'query' => [
            'percolate' => [
                'documents' => [
                    ['title' => 'nice pair of shoes','color'=>'blue'],
                    ['title' => 'beautiful bag']
                ]
            ]
        ]
    ]
];
$client->pq()->search($percolate);

```
<!-- response PHP -->

```php
Array
(
    [took] => 23
    [timed_out] =>
    [hits] => Array
        (
            [total] => 2
            [max_score] => 1
            [hits] => Array
                (
                    [0] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 2810781492890828819
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => bag
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 2
                                        )
                                )
                        )
                    [1] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 2810781492890828821
                            [_score] => 1
                            [_source] => Array
                            (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => shoes
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 1
                                        )
                                )
                        )
                )
        )
)
```
<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.percolate('products',{"query":{"percolate":{"documents":[{"title":"nice pair of shoes","color":"blue"},{"title":"beautiful bag"}]}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381494',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title bag'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [2]}},
                   {u'_id': u'2811025403043381496',
                    u'table': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title shoes'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}

```

<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate('products',{"query":{"percolate":{"documents":[{"title":"nice pair of shoes","color":"blue"},{"title":"beautiful bag"}]}}});
```

<!-- response javascript -->
```javascript
{
  "took": 6,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233808,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            2
          ]
        }
      },
      {
        "table": "products",
        "_type": "doc",
        "_id": 2811045522851233810,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
java
<!-- request Java -->

```java
percolateRequest = new PercolateRequest();
query = new HashMap<String,Object>(){{
        put("percolate",new HashMap<String,Object >(){{
            put("documents", new ArrayList<Object>(){{
                    add(new HashMap<String,Object >(){{
                        put("title","nice pair of shoes");
                        put("color","blue");
                    }});
                    add(new HashMap<String,Object >(){{
                        put("title","beautiful bag");

                    }});

                     }});
        }});
    }};
percolateRequest.query(query);
searchApi.percolate("products",percolateRequest);

```

<!-- response Java -->
```java
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234133, _score=1, _source={query={ql=@title bag}}, fields={_percolator_document_slot=[2]}}, {_index=products, _type=doc, _id=2811045522851234135, _score=1, _source={query={ql=@title shoes}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
var doc1 = new Dictionary<string, Object>();
doc1.Add("title","nice pair of shoes");
doc1.Add("color","blue");
var doc2 = new Dictionary<string, Object>();
doc2.Add("title","beautiful bag");
var docs = new List<Object> {doc1, doc2};
Dictionary<string, Object> percolateDoc = new Dictionary<string, Object> {{ "documents", docs }}; 
Dictionary<string, Object> query = new Dictionary<string, Object> {{ "percolate", percolateDoc }}; 
PercolateRequest percolateRequest = new PercolateRequest(query=query);
searchApi.Percolate("products",percolateRequest);
```

<!-- response C# -->
```clike
class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=products, _type=doc, _id=2811045522851234133, _score=1, _source={query={ql=@title bag}}, fields={_percolator_document_slot=[2]}}, {_index=products, _type=doc, _id=2811045522851234135, _score=1, _source={query={ql=@title shoes}}, fields={_percolator_document_slot=[1]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
docs = [ {title : 'What a nice bag'}, {title : 'Really nice shoes'} ]; 
res = await searchApi.percolate('test_pq', { query: { percolate: { documents : docs } } } );
```

<!-- response TypeScript -->
```typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title сумка"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title обувь"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
Идти
<!-- request Go -->

```go
doc1 := map[string]interface{} {"title": "Какой прекрасный сумка"}
doc2 := map[string]interface{} {"title": "Действительно красивые обувь"}
query := []interface{} {doc1, doc2}
percolateRequestQuery := manticoreclient.NewPercolateQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery)
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()

```
<!-- response Go -->
``` go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title сумка"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title обувь"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- end -->

<!-- example docs_id -->
#### Статические идентификаторы
По умолчанию соответствующие идентификаторы документов соответствуют их относительным номерам в списке, который вы предоставляете. Однако в некоторых случаях у каждого документа уже есть свой собственный идентификатор. В этом случае есть опция `'id field name' как docs_id` для `CALL PQ`.

Обратите внимание, что если идентификатор не может быть найден по предоставленному имени поля, правило PQ не будет отображено в результатах.

Эта опция доступна только для `CALL PQ` через SQL.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CALL PQ('products', '[{"id": 123, "title": "красивые пары обуви", "color": "синий"}, {"id": 456, "title": "красивый сумка"}]', 1 as query, 'id' as docs_id, 1 as docs);
```
<!-- response SQL -->

```sql
+---------------------+-----------+--------------+------+---------------------------+
| id                  | documents | query        | tags | filters                   |
+---------------------+-----------+--------------+------+---------------------------+
| 1657852401006149664 | 456       | @title сумка   |      |                           |
| 1657852401006149666 | 123       | @title обувь |      | color IN ('синий, 'зеленый') |
+---------------------+-----------+--------------+------+---------------------------+
```
<!-- end -->

<!-- example invalid_json -->
##### У меня могут быть недействительные JSON-данные, пожалуйста, пропустите их

При использовании CALL PQ с отдельными JSON-данными вы можете использовать опцию 1 как skip_bad_json, чтобы пропустить любые недействительные JSON-данные во входных данных. В приведенном ниже примере второй запрос не выполняется из-за недействительного JSON, но третий запрос избегает ошибки, используя 1 как skip_bad_json. Имейте в виду, что эта опция недоступна при отправке JSON-запросов через HTTP, так как весь JSON-запрос должен быть действительным в этом случае.

<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products', ('{"title": "красивые пары обуви", "color": "синий"}', '{"title": "красивый сумка"}'));

CALL PQ('products', ('{"title": "красивые пары обуви", "color": "синий"}', '{"title": "красивый сумка}'));

CALL PQ('products', ('{"title": "красивые пары обуви", "color": "синий"}', '{"title": "красивый сумка}'), 1 as skip_bad_json);
```
<!-- response SQL -->

```sql
+---------------------+
| id                  |
+---------------------+
| 1657852401006149635 |
| 1657852401006149637 |
+---------------------+

ERROR 1064 (42000): Плохие JSON-объекты в строках: 2

+---------------------+
| id                  |
+---------------------+
| 1657852401006149635 |
+---------------------+
```

<!-- end -->

##### Я хочу более высокую производительность перколяционного запроса
Перколяционные запросы разработаны с учетом высокой пропускной способности и больших объемов данных. Чтобы оптимизировать производительность для более низкой задержки и более высокой пропускной способности, рассмотрите следующее.

Существует два режима распределения для таблицы перколяций и того, как перколяционный запрос может работать против нее:

* **Разреженный (по умолчанию).** Идеально для: множества документов, отраженные PQ таблицы. Когда ваш набор документов велик, но набор запросов, хранящихся в таблице PQ, мал, разреженный режим будет полезен. В этом режиме пакет документов, который вы передаете, будет разделен между количеством агентов, поэтому каждый узел обрабатывает только часть документов из вашего запроса. Manticore разделяет ваш набор документов и распределяет куски между зеркалами. Как только агенты завершили обработку запросов, Manticore собирает и объединяет результаты, возвращая окончательный набор запросов, как если бы он пришел из одной таблицы. Используйте [репликацию](../References.md#Replication), чтобы помочь этому процессу.
* **Шарированное.** Идеально для: многих правил PQ, правил, распределённых по таблицам PQ. В этом режиме весь набор документов транслируется во все таблицы распределённой таблицы PQ без первоначального разделения документов. Это полезно при отправке относительно небольшого набора документов, но количество хранимых запросов велико. В этом случае более целесообразно хранить только часть правил PQ на каждом узле, а затем объединять результаты, возвращённые от узлов, которые обрабатывают один и тот же набор документов против разных наборов правил PQ. Этот режим должен быть явно установлен, так как он подразумевает увеличение сетевой нагрузки и ожидает таблицы с разными PQ, что [репликация](../References.md#Replication) не может сделать из коробки.

Предположим, у вас есть таблица `pq_d2`, определенная как:

``` ini
table pq_d2
{
    type = distributed
    agent = 127.0.0.1:6712:pq
    agent = 127.0.0.1:6712:ptitle
}
```

<!-- example distributed pq modes 1 -->
Каждая из 'pq' и 'ptitle' содержит:


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT * FROM pq;
```

<!-- response sql -->

```sql
+------+-------------+------+-------------------+
| id   | query       | tags | filters           |
+------+-------------+------+-------------------+
|    1 | filter test |      | gid>=10           |
|    2 | angry       |      | gid>=10 OR gid<=3 |
+------+-------------+------+-------------------+
2 rows in set (0.01 sec)
```

<!-- request JSON -->

```json
POST /pq/pq/_search
```

<!-- response JSON -->

```json
{
    "took":0,
    "timed_out":false,
    "hits":{
        "total":2,
        "hits":[
            {
                "_id": 1,
                "_score":1,
                "_source":{
                    "query":{ "ql":"filter test" },
                    "tags":"",
                    "filters":"gid>=10"
                }
            },
            {
                "_id": 2,
                "_score":1,
                "_source":{
                    "query":{"ql":"angry"},
                    "tags":"",
                    "filters":"gid>=10 OR gid<=3"
                }
            }            
        ]
    }
}
```

<!-- request PHP -->

```php
$params = [
    'table' => 'pq',
    'body' => [
    ]
];
$response = $client->pq()->search($params);
```

<!-- response PHP -->

```php
(
    [took] => 0
    [timed_out] =>
    [hits] =>
        (
            [total] => 2
            [hits] =>
                (
                    [0] =>
                        (
                            [_id] => 1
                            [_score] => 1
                            [_source] =>
                                (
                                    [query] =>
                                        (
                                            [ql] => filter test
                                        )
                                    [tags] =>
                                    [filters] => gid>=10
                                )
                        ),
                    [1] =>
                        (
                            [_id] => 1
                            [_score] => 1
                            [_source] =>
                                (
                                    [query] =>
                                        (
                                            [ql] => angry
                                        )
                                    [tags] =>
                                    [filters] => gid>=10 OR gid<=3
                                )
                        )
                )
        )
)
```
<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.search({"table":"pq","query":{"match_all":{}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381501',
                    u'_score': 1,
                    u'_source': {u'filters': u"gid>=10",
                                 u'query': u'filter test',
                                 u'tags': u''}},
                   {u'_id': u'2811025403043381502',
                    u'_score': 1,
                    u'_source': {u'filters': u"gid>=10 OR gid<=3",
                                 u'query': u'angry',
                                 u'tags': u''}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"pq","query":{"match_all":{}}});
```
<!-- response javascript -->
``` javascript
{'hits': {'hits': [{u'_id': u'2811025403043381501',
                    u'_score': 1,
                    u'_source': {u'filters': u"gid>=10",
                                 u'query': u'filter test',
                                 u'tags': u''}},
                   {u'_id': u'2811025403043381502',
                    u'_score': 1,
                    u'_source': {u'filters': u"gid>=10 OR gid<=3",
                                 u'query': u'angry',
                                 u'tags': u''}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"pq","query":{"match_all":{}}});
```
<!-- response javascript -->
``` javascript
{"hits": {"hits": [{"_id": 2811025403043381501,
                    "_score": 1,
                    "_source": {"filters": u"gid>=10",
                                 "query": "filter test",
                                 "tags": ""}},
                   {"_id": 2811025403043381502,
                    "_score": 1,
                    "_source": {"filters": u"gid>=10 OR gid<=3",
                                 "query": "angry",
                                 "tags": ""}}],
          "total": 2},
  "timed_out": false,
 "took": 0}
```

<!-- intro -->
java
<!-- request Java -->

```java
Map<String,Object> query = new HashMap<String,Object>();
query.put("match_all",null);
SearchRequest searchRequest = new SearchRequest();
searchRequest.setIndex("pq");
searchRequest.setQuery(query);
SearchResponse searchResponse = searchApi.search(searchRequest);
```
<!-- response Java -->
``` java

class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: null
        hits: [{_id=2811045522851233962, _score=1, _source={filters=gid>=10, query=filter test, tags=}}, {_id=2811045522851233951, _score=1, _source={filters=gid>=10 OR gid<=3, query=angry,tags=}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
object query =  new { match_all=null };
SearchRequest searchRequest = new SearchRequest("pq", query);
SearchResponse searchResponse = searchApi.Search(searchRequest);
```
<!-- response C# -->
``` clike

class SearchResponse {
    took: 0
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: null
        hits: [{_id=2811045522851233962, _score=1, _source={filters=gid>=10, query=filter test, tags=}}, {_id=2811045522851233951, _score=1, _source={filters=gid>=10 OR gid<=3, query=angry,tags=}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
res = await searchApi.search({"table":"test_pq","query":{"match_all":{}}});
```
<!-- response TypeScript -->
``` typescript
{
	'hits':
	{
		'hits': 
		[{
			'_id': '2811025403043381501',
            '_score': 1,
            '_source': 
            {
            	'filters': "gid>=10",
                'query': 'filter test',
                'tags': ''
            }
        },
        {
         	'_id': 
         	'2811025403043381502',
            '_score': 1,
            '_source': 
            {
            	'filters': "gid>=10 OR gid<=3",
                 'query': 'angry',
                 'tags': ''
            }
        }],
    	'total': 2
	},
	'profile': None,
	'timed_out': False,
	'took': 0
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
query := map[string]interface{} {}
percolateRequestQuery := manticoreclient.NewPercolateRequestQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery) 
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()
```
<!-- response Go -->
``` go
{
	'hits':
	{
		'hits': 
		[{
			'_id': '2811025403043381501',
            '_score': 1,
            '_source': 
            {
            	'filters': "gid>=10",
                'query': 'filter test',
                'tags': ''
            }
        },
        {
         	'_id': 
         	'2811025403043381502',
            '_score': 1,
            '_source': 
            {
            	'filters': "gid>=10 OR gid<=3",
                 'query': 'angry',
                 'tags': ''
            }
        }],
    	'total': 2
	},
	'profile': None,
	'timed_out': False,
	'took': 0
}
```

<!-- end -->


<!-- example call_pq_example -->

And you execute `CALL PQ` on the distributed table with a couple of documents.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CALL PQ ('pq_d2', ('{"title":"angry test", "gid":3 }', '{"title":"filter test doc2", "gid":13}'), 1 AS docs);
```

<!-- response sql -->

```sql
+------+-----------+
| id   | documents |
+------+-----------+
|    1 | 2         |
|    2 | 1         |
+------+-----------+
```

<!-- request JSON -->

```json
POST /pq/pq/_search -d '
"query":
{
        "percolate":
        {
                "documents" : [
                    { "title": "angry test", "gid": 3 },
                    { "title": "filter test doc2", "gid": 13 }
                ]
        }
}
'
```

<!-- response JSON -->

```json
{
    "took":0,
    "timed_out":false,
    "hits":{
    "total":2,"hits":[
        {
            "_id": 2,
            "_score":1,
            "_source":{
                "query":{"title":"angry"},
                "tags":"",
                "filters":"gid>=10 OR gid<=3"
            }
        }
        {
            "_id": 1,
            "_score":1,
            "_source":{
                "query":{"ql":"filter test"},
                "tags":"",
                "filters":"gid>=10"
            }
        },
        ]
    }
}
```

<!-- request PHP -->

```php
$params = [
    'table' => 'pq',
    'body' => [
        'query' => [
            'percolate' => [
                'documents' => [
                    [
                        'title'=>'angry test',
                        'gid' => 3
                    ],
                    [
                        'title'=>'filter test doc2',
                        'gid' => 13
                    ],
                ]
            ]
        ]
    ]
];
$response = $client->pq()->search($params);
```

<!-- response PHP -->

```php
(
    [took] => 0
    [timed_out] =>
    [hits] =>
        (
            [total] => 2
            [hits] =>
                (
                    [0] =>
                        (
                            [_index] => pq  
                            [_type] => doc                            
                            [_id] => 2
                            [_score] => 1
                            [_source] =>
                                (
                                    [query] =>
                                        (
                                            [ql] => angry
                                        )
                                    [tags] =>
                                    [filters] => gid>=10 OR gid<=3
                                ),
                           [fields] =>
                                (
                                    [_percolator_document_slot] =>
                                        (
                                            [0] => 1
                                        )

                                )
                        ),
                    [1] =>
                        (
                            [_index] => pq                            
                            [_id] => 1
                            [_score] => 1
                            [_source] =>
                                (
                                    [query] =>
                                        (
                                            [ql] => filter test
                                        )
                                    [tags] =>
                                    [filters] => gid>=10
                                )
                           [fields] =>
                                (
                                    [_percolator_document_slot] =>
                                        (
                                            [0] => 0
                                        )

                                )
                        )
                )
        )
)
```
<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.percolate('pq',{"percolate":{"documents":[{"title":"angry test","gid":3},{"title":"filter test doc2","gid":13}]}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'table': u'pq',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'angry'},u'tags':u'',u'filters':u"gid>=10 OR gid<=3"},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}},
                    {u'_id': u'2811025403043381501',
                    u'table': u'pq',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'filter test'},u'tags':u'',u'filters':u"gid>=10"},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate('pq',{"percolate":{"documents":[{"title":"angry test","gid":3},{"title":"filter test doc2","gid":13}]}});
```
<!-- response javascript -->
``` javascript
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'table': u'pq',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'angry'},u'tags':u'',u'filters':u"gid>=10 OR gid<=3"},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}},
                    {u'_id': u'2811025403043381501',
                    u'table': u'pq',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'filter test'},u'tags':u'',u'filters':u"gid>=10"},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
java
<!-- request Java -->

```java
percolateRequest = new PercolateRequest();
query = new HashMap<String,Object>(){{
    put("percolate",new HashMap<String,Object >(){{
        put("documents", new ArrayList<Object>(){{
            add(new HashMap<String,Object >(){{
                put("title","angry test");
                put("gid",3);
            }});
            add(new HashMap<String,Object >(){{
                put("title","filter test doc2");
                put("gid",13);
            }});
        }});
    }});
}};
percolateRequest.query(query);
searchApi.percolate("pq",percolateRequest);
```
<!-- response java -->
``` java
class SearchResponse {
    took: 10
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=pq, _type=doc, _id=2811045522851234165, _score=1, _source={query={ql=@title angry}}, fields={_percolator_document_slot=[1]}}, {_index=pq, _type=doc, _id=2811045522851234166, _score=1, _source={query={ql=@title filter test doc2}}, fields={_percolator_document_slot=[2]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
C#
<!-- request C# -->

```clike
var doc1 = new Dictionary<string, Object>();
doc1.Add("title","angry test");
doc1.Add("gid",3);
var doc2 = new Dictionary<string, Object>();
doc2.Add("title","filter test doc2");
doc2.Add("gid",13);
var docs = new List<Object> {doc1, doc2};
Dictionary<string, Object> percolateDoc = new Dictionary<string, Object> {{ "documents", docs }}; 
Dictionary<string, Object> query = new Dictionary<string, Object> {{ "percolate", percolateDoc }}; 
PercolateRequest percolateRequest = new PercolateRequest(query=query);
searchApi.Percolate("pq",percolateRequest);
```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 10
    timedOut: false
    hits: class SearchResponseHits {
        total: 2
        maxScore: 1
        hits: [{_index=pq, _type=doc, _id=2811045522851234165, _score=1, _source={query={ql=@title angry}}, fields={_percolator_document_slot=[1]}}, {_index=pq, _type=doc, _id=2811045522851234166, _score=1, _source={query={ql=@title filter test doc2}}, fields={_percolator_document_slot=[2]}}]
        aggregations: null
    }
    profile: null
}

```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
docs = [ {title : 'Что за хорошая сумка'}, {title : 'Действительно красивые туфли'} ]; 
res = await searchApi.percolate('test_pq', { query: { percolate: { documents : docs } } } );
```
<!-- response TypeScript -->
``` typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
doc1 := map[string]interface{} {"title": "Что за хорошая сумка"}
doc2 := map[string]interface{} {"title": "Действительно красивые туфли"}
query := []interface{} {doc1, doc2}
percolateRequestQuery := manticoreclient.NewPercolateQuery(query)
percolateRequest := manticoreclient.NewPercolateRequest(percolateRequestQuery)
res, _, _ := apiClient.SearchAPI.Percolate(context.Background(), "test_pq").PercolateRequest(*percolateRequest).Execute()

```
<!-- response Go -->
``` go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149661,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      },
      {
        "table": "test_pq",
        "_type": "doc",
        "_id": 1657852401006149662,
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- end -->

In the previous example, we used the default **sparse** mode. To demonstrate the **sharded** mode, let's create a distributed PQ table consisting of 2 local PQ tables and add 2 documents to "products1" and 1 document to "products2":
```sql
create table products1(title text, color string) type='pq';
create table products2(title text, color string) type='pq';
create table products_distributed type='distributed' local='products1' local='products2';

INSERT INTO products1(query) values('@title bag');
INSERT INTO products1(query,filters) values('@title shoes', 'color=\'red\'');
INSERT INTO products2(query,filters) values('@title shoes', 'color in (\'blue\', \'green\')');
```

<!-- example sharded -->
Now, if you add `'sharded' as mode` to `CALL PQ`, it will send the documents to all the agent's tables (in this case, just local tables, but they can be remote to utilize external hardware). This mode is not available via the JSON interface.

<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products_distributed', ('{"title": "nice pair of shoes", "color": "blue"}', '{"title": "beautiful bag"}'), 'sharded' as mode, 1 as query);
```
<!-- response SQL -->

```sql
+---------------------+--------------+------+---------------------------+
| id                  | query        | tags | filters                   |
+---------------------+--------------+------+---------------------------+
| 1657852401006149639 | @title bag   |      |                           |
| 1657852401006149643 | @title shoes |      | color IN ('blue, 'green') |
+---------------------+--------------+------+---------------------------+
```

<!-- end -->

Note that the syntax of agent mirrors in the configuration (when several hosts are assigned to one `agent` line, separated with `|`) has nothing to do with the `CALL PQ` query mode. Each `agent` always represents **one** node, regardless of the number of HA mirrors specified for that agent.

<!-- example verbose -->
##### How can I learn more about performance?
In some cases, you might want to get more details about the performance of a percolate query. For that purpose, there is the option `1 as verbose`, which is only available via SQL and allows you to save more performance metrics. You can see them using the `SHOW META` query, which you can run after `CALL PQ`. See [SHOW META](../Node_info_and_management/SHOW_META.md) for more info.

<!-- intro -->
1 as verbose:
<!-- request 1 as verbose -->

```sql
CALL PQ('products', ('{"title": "nice pair of shoes", "color": "blue"}', '{"title": "beautiful bag"}'), 1 as verbose); show meta;
```
<!-- response 1 as verbose -->

```sql
+---------------------+
| id                  |
+---------------------+
| 1657852401006149644 |
| 1657852401006149646 |
+---------------------+
+-------------------------+-----------+
| Название переменной      | Значение   |
+-------------------------+-----------+
| total                   | 0.000 sec |
| setup                   | 0.000 sec |
| queries_matched         | 2         |
| queries_failed          | 0         |
| document_matched        | 2         |
| total_queries_stored    | 3         |
| term_only_queries       | 3         |
| fast_rejected_queries   | 0         |
| time_per_query          | 27, 10    |
| time_of_matched_queries | 37        |
+-------------------------+-----------+
```
<!-- intro -->
0 как verbose (по умолчанию):
<!-- request 0 as verbose -->

```sql
CALL PQ('products', ('{"title": "nice pair of shoes", "color": "blue"}', '{"title": "beautiful bag"}'), 0 как verbose); show meta;
```
<!-- response 0 as verbose -->

```sql
+---------------------+
| id                  |
+---------------------+
| 1657852401006149644 |
| 1657852401006149646 |
+---------------------+
+-----------------------+-----------+
| Название переменной    | Значение   |
+-----------------------+-----------+
| total                 | 0.000 sec |
| queries_matched       | 2         |
| queries_failed        | 0         |
| document_matched      | 2         |
| total_queries_stored  | 3         |
