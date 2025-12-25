# Percolate Query

Percolate запросы также известны как Persistent queries, Prospective search, document routing, search in reverse и inverse search.

Традиционный способ проведения поиска предполагает хранение документов и выполнение поисковых запросов по ним. Однако бывают случаи, когда мы хотим применить запрос к вновь поступающему документу, чтобы сигнализировать о совпадении. Некоторые сценарии, где это желательно, включают системы мониторинга, которые собирают данные и уведомляют пользователей о конкретных событиях, таких как достижение определенного порога для метрики или появление конкретного значения в отслеживаемых данных. Другой пример — агрегация новостей, где пользователи могут захотеть получать уведомления только об определенных категориях или темах, или даже конкретных "ключевых словах".

В этих ситуациях традиционный поиск не является лучшим решением, так как он предполагает выполнение желаемого поиска по всей коллекции. Этот процесс умножается на количество пользователей, что приводит к выполнению множества запросов по всей коллекции, что может вызвать значительную дополнительную нагрузку. Альтернативный подход, описанный в этом разделе, предполагает хранение самих запросов и их проверку на соответствие входящему новому документу или пакету документов.

Google Alerts, AlertHN, Bloomberg Terminal и другие системы, позволяющие пользователям подписываться на определенный контент, используют аналогичную технологию.

> * См. [percolate](../Creating_a_table/Local_tables/Percolate_table.md) для получения информации о создании PQ таблицы.
> * См. [Adding rules to a percolate table](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md), чтобы узнать, как добавлять percolate правила (также известные как PQ правила). Вот краткий пример:


### Выполнение percolate запроса с помощью CALL PQ

Ключевой момент, который нужно помнить о percolate запросах, это то, что ваши поисковые запросы уже находятся в таблице. Что вам нужно предоставить — это документы **для проверки, соответствуют ли какие-либо из них каким-либо сохраненным правилам**.

Вы можете выполнить percolate запрос через SQL или JSON интерфейсы, а также с помощью клиентов на языках программирования. SQL подход предлагает большую гибкость, в то время как HTTP метод проще и предоставляет большую часть необходимого. Следующая таблица может помочь вам понять различия.

| Желаемое поведение              | SQL                                     | HTTP                                 |
| ----------------------------- | --------------------------------------- | ------------------------------------ |
| Предоставить один документ     | `CALL PQ('tbl', '{doc1}')`              | `query.percolate.document{doc1}`     |
| Предоставить один документ (альтернатива) | `CALL PQ('tbl', 'doc1', 0 as docs_json)` | - |
| Предоставить несколько документов    | `CALL PQ('tbl', ('doc1', 'doc2'), 0 as docs_json)` | - |
| Предоставить несколько документов (альтернатива) | `CALL PQ('tbl', ('{doc1}', '{doc2}'))` | - |
| Предоставить несколько документов (альтернатива) | `CALL PQ('tbl', '[{doc1}, {doc2}]')` | - |
| Возвращать идентификаторы совпадающих документов  | 0/1 as docs (отключено по умолчанию)       | Включено по умолчанию                   |
| Использовать собственный id документа для отображения в результате | 'id field' as docs_id (отключено по умолчанию) | Недоступно |
| Считать входные документы JSON | 1 as docs_json (1 по умолчанию) | Включено по умолчанию |
| Считать входные документы простым текстом | 0 as docs_json (1 по умолчанию) | Недоступно |
| [Режим разреженного распределения](../Searching/Percolate_query.md#I-want-higher-performance-of-a-percolate-query) | default | default |
| [Режим шардированного распределения](../Searching/Percolate_query.md#I-want-higher-performance-of-a-percolate-query) | sharded as mode | Недоступно |
| Возвращать всю информацию о совпадающем запросе | 1 as query (0 по умолчанию) | Включено по умолчанию |
| Пропускать невалидный JSON | 1 as skip_bad_json (0 по умолчанию) | Недоступно |
| Расширенная информация в [SHOW META](../Node_info_and_management/SHOW_META.md) | 1 as verbose (0 по умолчанию) | Недоступно |
| Определить число, которое будет добавлено к идентификаторам документов, если не предоставлены поля docs_id (в основном актуально в [режимах распределенной PQ](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#Distributed-percolate-tables-%28DPQ-tables%29)) | 1 as shift (0 по умолчанию) | Недоступно |

<!-- example create percolate -->
Чтобы продемонстрировать, как это работает, вот несколько примеров. Давайте создадим PQ таблицу с двумя полями:

* title (текст)
* color (строка)

и тремя правилами в ней:

* Просто полнотекстовый поиск. Запрос: `@title bag`
* Полнотекстовый поиск и фильтрация. Запрос: `@title shoes`. Фильтры: `color='red'`
* Полнотекстовый поиск и более сложная фильтрация. Запрос: `@title shoes`. Фильтры: `color IN('blue', 'green')`

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
Python-asyncio
<!-- request Python-asyncio -->

```python
await utilsApi.sql('create table products(title text, color string) type=\'pq\'')
await indexApi.insert({"table" : "products", "doc" : {"query" : "@title bag" }})
await indexApi.insert({"table" : "products",  "doc" : {"query" : "@title shoes", "filters": "color='red'" }})
await indexApi.insert({"table" : "products",  "doc" : {"query" : "@title shoes","filters": "color IN ('blue', 'green')" }})
```
<!-- response Python-asyncio -->
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
utilsApi.sql("create table products(title text, color string) type='pq'", true);
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
utilsApi.Sql("create table products(title text, color string) type='pq'", true);

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
##### Просто скажите мне, какие PQ правила соответствуют моему единственному документу

Первый документ не соответствует ни одному правилу. Он мог бы соответствовать первым двум, но они требуют дополнительных фильтров.

Второй документ соответствует одному правилу. Обратите внимание, что CALL PQ по умолчанию ожидает, что документ будет JSON, но если вы используете `0 as docs_json`, вы можете передать простую строку.

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
POST /pq/products/search
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
                    'title' => 'What a nice bag'
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
searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"What a nice bag"}}}})
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
await searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"What a nice bag"}}}})
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
res = await searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"What a nice bag"}}}});
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
            put("title","what a nice bag");
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
percolateDoc.Add("document", new Dictionary<string, Object> {{ "title", "what a nice bag" }});
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
percolate_doc_fileds.insert("title".to_string(), "what a nice bag");
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
res = await searchApi.percolate('test_pq', { query: { percolate: { document : { title : 'What a nice bag' } } } } );
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
query := map[string]interface{} {"title": "what a nice bag"}
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
##### Я хочу узнать полные правила PQ, соответствующие моему документу
<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products', '{"title": "What a nice bag"}', 1 as query);
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
POST /pq/products/search
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
                    'title' => 'What a nice bag'
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
searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"What a nice bag"}}}})
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
await searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"What a nice bag"}}}})
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
res = await searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"What a nice bag"}}}});
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
            put("title","what a nice bag");
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
percolateDoc.Add("document", new Dictionary<string, Object> {{ "title", "what a nice bag" }});
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
percolate_doc_fileds.insert("title".to_string(), "what a nice bag");
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
res = await searchApi.percolate('test_pq', { query: { percolate: { document : { title : 'What a nice bag' } } } } );
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
query := map[string]interface{} {"title": "what a nice bag"}
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

Обратите внимание, что с помощью `CALL PQ` вы можете предоставить несколько документов разными способами:

* как массив простых документов в круглых скобках `('doc1', 'doc2')`. Это требует `0 as docs_json`
* как массив JSON в круглых скобках `('{doc1}', '{doc2}')`
* или как стандартный JSON-массив `'[{doc1}, {doc2}]'`

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
POST /pq/products/search
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
percolate_doc_fields2.insert("title".to_string(), "beautiful bag");
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
Go
<!-- request Go -->

```go
doc1 := map[string]interface{} {"title": "What a nice bag"}
doc2 := map[string]interface{} {"title": "Really nice shoes"}
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
##### Я хочу узнать, какие документы соответствуют каким правилам

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
POST /pq/products/search
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
percolate_doc_fields2.insert("title".to_string(), "beautiful bag");
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
Go
<!-- request Go -->

```go
doc1 := map[string]interface{} {"title": "What a nice bag"}
doc2 := map[string]interface{} {"title": "Really nice shoes"}
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

<!-- example docs_id -->
#### Статические идентификаторы
По умолчанию идентификаторы совпадающих документов соответствуют их относительным номерам в предоставленном вами списке. Однако в некоторых случаях у каждого документа уже есть свой собственный идентификатор. Для этого случая существует опция `'id field name' as docs_id` для `CALL PQ`.

Обратите внимание, что если идентификатор не может быть найден по указанному имени поля, правило PQ не будет отображено в результатах.

Эта опция доступна только для `CALL PQ` через SQL.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CALL PQ('products', '[{"id": 123, "title": "nice pair of shoes", "color": "blue"}, {"id": 456, "title": "beautiful bag"}]', 1 as query, 'id' as docs_id, 1 as docs);
```
<!-- response SQL -->

```sql
+---------------------+-----------+--------------+------+---------------------------+
| id                  | documents | query        | tags | filters                   |
+---------------------+-----------+--------------+------+---------------------------+
| 1657852401006149664 | 456       | @title bag   |      |                           |
| 1657852401006149666 | 123       | @title shoes |      | color IN ('blue, 'green') |
+---------------------+-----------+--------------+------+---------------------------+
```
<!-- end -->

<!-- example invalid_json -->
##### У меня могут быть неверные JSON, пожалуйста, пропускайте их

При использовании CALL PQ с отдельными JSON можно использовать опцию 1 as skip_bad_json, чтобы пропускать любые неверные JSON в вводе. В приведенном ниже примере 2-й запрос не выполняется из-за неверного JSON, но 3-й запрос избегает ошибки, используя 1 as skip_bad_json. Учтите, что эта опция недоступна при отправке JSON-запросов через HTTP, так как в этом случае весь JSON-запрос должен быть валиден.

<!-- intro -->
SQL:
<!-- request SQL -->

```sql
CALL PQ('products', ('{"title": "nice pair of shoes", "color": "blue"}', '{"title": "beautiful bag"}'));

CALL PQ('products', ('{"title": "nice pair of shoes", "color": "blue"}', '{"title": "beautiful bag}'));

CALL PQ('products', ('{"title": "nice pair of shoes", "color": "blue"}', '{"title": "beautiful bag}'), 1 as skip_bad_json);
```
<!-- response SQL -->

```sql
+---------------------+
| id                  |
+---------------------+
| 1657852401006149635 |
| 1657852401006149637 |
+---------------------+

ERROR 1064 (42000): Bad JSON objects in strings: 2

+---------------------+
| id                  |
+---------------------+
| 1657852401006149635 |
+---------------------+
```

<!-- end -->

##### Я хочу повысить производительность percolate-запроса
Percolate-запросы разработаны с учетом высокой пропускной способности и больших объемов данных. Для оптимизации производительности с целью снижения задержки и увеличения пропускной способности рассмотрите следующее.

Существует два режима распределения для percolate-таблицы и того, как percolate-запрос может работать с ней:

* **Разреженный (по умолчанию).** Идеален для: множества документов, зеркальных PQ-таблиц. Когда ваш набор документов велик, но набор запросов, хранящихся в PQ-таблице, мал, разреженный режим выгоден. В этом режиме пакет документов, который вы передаете, будет разделен между количеством агентов, так что каждый узел обрабатывает только часть документов из вашего запроса. Manticore разделяет ваш набор документов и распределяет фрагменты между зеркалами. Как только агенты завершат обработку запросов, Manticore собирает и объединяет результаты, возвращая итоговый набор запросов, как если бы он поступил из одной таблицы. Используйте [репликацию](../References.md#Replication), чтобы помочь процессу.
* **Сегментированный.** Идеален для: множества PQ-правил, правил, разделенных между PQ-таблицами. В этом режиме весь набор документов рассылается на все таблицы распределенной PQ-таблицы без первоначального разделения документов. Это полезно, когда передается относительно небольшой набор документов, но количество хранимых запросов велико. В этом случае более уместно хранить только часть PQ-правил на каждом узле, а затем объединять результаты, возвращенные от узлов, которые обрабатывают один и тот же набор документов с разными наборами PQ-правил. Этот режим должен быть установлен явно, так как он подразумевает увеличение сетевой нагрузки и ожидает таблицы с разными PQ, что [репликация](../References.md#Replication) не может сделать из коробки.

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
POST /pq/pq/search
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

И вы выполняете `CALL PQ` на распределенной таблице с парой документов.


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
POST /pq/pq/search -d '
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
Python-asyncio
<!-- request Python-asyncio -->

```python
await searchApi.percolate('pq',{"percolate":{"documents":[{"title":"angry test","gid":3},{"title":"filter test doc2","gid":13}]}})
```
<!-- response Python-asyncio -->
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
Rust
<!-- request Rust -->

```rust
let mut percolate_doc_fields1 = HashMap::new();
percolate_doc_fields1.insert("title".to_string(), "angry test");
percolate_doc_fields1.insert("gid".to_string(), 3);
let mut percolate_doc_fields2 = HashMap::new();
percolate_doc_fields2.insert("title".to_string(), "filter test doc2");
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
Go
<!-- request Go -->

```go
doc1 := map[string]interface{} {"title": "What a nice bag"}
doc2 := map[string]interface{} {"title": "Really nice shoes"}
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

В предыдущем примере мы использовали режим по умолчанию **разреженный**. Чтобы продемонстрировать **сегментированный** режим, давайте создадим распределенную PQ-таблицу, состоящую из 2 локальных PQ-таблиц, и добавим 2 документа в "products1" и 1 документ в "products2":
```sql
create table products1(title text, color string) type='pq';
create table products2(title text, color string) type='pq';
create table products_distributed type='distributed' local='products1' local='products2';

INSERT INTO products1(query) values('@title bag');
INSERT INTO products1(query,filters) values('@title shoes', 'color=\'red\'');
INSERT INTO products2(query,filters) values('@title shoes', 'color in (\'blue\', \'green\')');
```

<!-- example sharded -->
Теперь, если вы добавите `'sharded' as mode` в `CALL PQ`, он отправит документы во все таблицы агентов (в данном случае, просто локальные таблицы, но они могут быть удаленными, чтобы использовать внешнее оборудование). Этот режим недоступен через JSON-интерфейс.

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

Обратите внимание, что синтаксис зеркал агентов в конфигурации (когда несколько хостов назначаются одной строке `agent`, разделенных `|`) не имеет ничего общего с режимом запроса `CALL PQ`. Каждый `agent` всегда представляет **один** узел, независимо от количества зеркал высокой доступности, указанных для этого агента.

<!-- example verbose -->
##### Как я могу узнать больше о производительности?
В некоторых случаях вам может понадобиться получить более подробную информацию о производительности percolate-запроса. Для этой цели существует опция `1 as verbose`, которая доступна только через SQL и позволяет сохранить больше метрик производительности. Вы можете увидеть их с помощью запроса `SHOW META`, который можно выполнить после `CALL PQ`. Подробнее см. в [SHOW META](../Node_info_and_management/SHOW_META.md).

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
| Variable name           | Value     |
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
0 as verbose (по умолчанию):
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
| Variable name         | Value     |
+-----------------------+-----------+
| total                 | 0.000 sec |
| queries_matched       | 2         |
| queries_failed        | 0         |
| document_matched      | 2         |
| total_queries_stored  | 3         |
| term_only_queries     | 3         |
| fast_rejected_queries | 0         |
+-----------------------+-----------+
```
<!-- end -->
<!-- proofread -->

