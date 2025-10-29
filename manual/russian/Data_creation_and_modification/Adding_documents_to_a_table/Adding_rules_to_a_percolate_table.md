# Добавление правил в таблицу перколации

<!-- example -->
В [таблице перколации](../../Creating_a_table/Local_tables/Percolate_table.md) хранятся документы с правилами запросов перколации и должны строго соответствовать схеме из четырёх полей:

| поле | тип | описание |
| - | - | - |
| id | bigint | идентификатор правила PQ (если отсутствует, будет назначен автоматически) |
| query | string | полнотекстовый запрос (может быть пустым), совместимый с [таблицей перколации](../../Creating_a_table/Local_tables/Percolate_table.md) |
| filters | string | дополнительные фильтры по не полнотекстовым полям (может быть пустым), совместимые с [таблицей перколации](../../Creating_a_table/Local_tables/Percolate_table.md) |
| tags   | string | строка с одной или несколькими тегами, разделёнными запятыми, может использоваться для выборочного отображения/удаления сохранённых запросов |

Любые другие имена полей не поддерживаются и вызовут ошибку.

**Внимание:** Вставка/замена правил PQ в формате JSON через SQL не сработает. Другими словами, специфичные для JSON операторы (`match` и т. п.) будут рассматриваться просто как части текста правила, которые должны совпадать с документами. Если вы предпочитаете синтаксис JSON, используйте HTTP интерфейс вместо `INSERT`/`REPLACE`.

<!-- intro -->
##### SQL
<!-- request SQL -->

```sql
INSERT INTO pq(id, query, filters) VALUES (1, '@title shoes', 'price > 5');
INSERT INTO pq(id, query, tags) VALUES (2, '@title bag', 'Louis Vuitton');
SELECT * FROM pq;
```
<!-- response SQL -->

```sql
+------+--------------+---------------+---------+
| id   | query        | tags          | filters |
+------+--------------+---------------+---------+
|    1 | @title shoes |               | price>5 |
|    2 | @title bag   | Louis Vuitton |         |
+------+--------------+---------------+---------+
```
<!-- intro -->
##### JSON
<!-- request JSON -->
Есть два способа добавить запрос перколации в таблицу перколации:
* Запрос в формате JSON, совместимом с /search, описанном в [json/search](../../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON)
```json
PUT /pq/pq_table/doc/1
{
  "query": {
    "match": {
      "title": "shoes"
    },
    "range": {
      "price": {
        "gt": 5
      }
    }
  },
  "tags": ["Loius Vuitton"]
}
```

* Запрос в формате SQL, описанном в [синтаксисе запросов поиска](../../Searching/Filters.md#Queries-in-SQL-format)
```json
PUT /pq/pq_table/doc/2
{
  "query": {
    "ql": "@title shoes"
  },
  "filters": "price > 5",
  "tags": ["Loius Vuitton"]
}
```
<!-- intro -->
##### PHP
<!-- request PHP -->
```php
$newstoredquery = [
    'table' => 'test_pq',
    'body' => [
        'query' => [
                       'match' => [
                               'title' => 'shoes'
                       ]
               ],
               'range' => [
                       'price' => [
                               'gt' => 5
                       ]
               ]
       ],
    'tags' => ['Loius Vuitton']
];
$client->pq()->doc($newstoredquery);
```

<!-- intro -->
##### Python
<!-- request Python -->
```python
newstoredquery ={"table" : "test_pq", "id" : 2, "doc" : {"query": {"ql": "@title shoes"},"filters": "price > 5","tags": ["Loius Vuitton"]}}
indexApi.insert(newstoredquery)
```

<!-- intro -->
##### Python-asyncio
<!-- request Python-asyncio -->
```python
newstoredquery ={"table" : "test_pq", "id" : 2, "doc" : {"query": {"ql": "@title shoes"},"filters": "price > 5","tags": ["Loius Vuitton"]}}
await indexApi.insert(newstoredquery)
```

<!-- intro -->
##### Javascript
<!-- request Javascript -->
```javascript
newstoredquery ={"table" : "test_pq", "id" : 2, "doc" : {"query": {"ql": "@title shoes"},"filters": "price > 5","tags": ["Loius Vuitton"]}};
indexApi.insert(newstoredquery);
```
<!-- intro -->
##### java
<!-- request Java -->
```java
newstoredquery = new HashMap<String,Object>(){{
    put("query",new HashMap<String,Object >(){{
        put("q1","@title shoes");
        put("filters","price>5");
        put("tags",new String[] {"Loius Vuitton"});
    }});
}};
newdoc.index("test_pq").id(2L).setDoc(doc);
indexApi.insert(newdoc);
```

<!-- intro -->
##### C#
<!-- request C# -->
```clike
Dictionary<string, Object> query = new Dictionary<string, Object>();
query.Add("q1", "@title shoes");
query.Add("filters", "price>5");
query.Add("tags", new List<string> {"Loius Vuitton"});
Dictionary<string, Object> newstoredquery = new Dictionary<string, Object>();
newstoredquery.Add("query", query);
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "test_pq", id: 2, doc: doc);
indexApi.Insert(newdoc);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let mut pq_doc = HashMap::new();
pq_doc.insert("q1".to_string(), serde_json::json!("@title shoes"));
pq_doc.insert("filters".to_string(), serde_json::json!("price>5"));
pq_doc.insert("tags".to_string(), serde_json::json!(["Louis Vitton"]));

let mut doc = HashMap::new();
pq_doc.insert("query".to_string(), serde_json::json!(pq_doc));

let insert_req = InsertDocumentRequest::new("test_pq".to_string(), serde_json::json!(doc));
let insert_res = index_api.insert(insert_req).await;
```
<!-- end -->

<!-- example noid -->
## Автоматическое назначение идентификатора

Если вы не указываете ID, он будет назначен автоматически. Подробнее об авто-ID читайте [здесь](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID).

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
INSERT INTO pq(query, filters) VALUES ('wristband', 'price > 5');
SELECT * FROM pq;
```
<!-- response SQL -->

```sql
+---------------------+-----------+------+---------+
| id                  | query     | tags | filters |
+---------------------+-----------+------+---------+
| 1657843905795719192 | wristband |      | price>5 |
+---------------------+-----------+------+---------+
```
<!-- intro -->
##### JSON
<!-- request JSON -->

```json
PUT /pq/pq_table/doc
{
"query": {
  "match": {
    "title": "shoes"
  },
  "range": {
    "price": {
      "gt": 5
    }
  }
},
"tags": ["Loius Vuitton"]
}

PUT /pq/pq_table/doc
{
"query": {
  "ql": "@title shoes"
},
"filters": "price > 5",
"tags": ["Loius Vuitton"]
}
```
<!-- response JSON -->

```json
{
  "table": "pq_table",
  "type": "doc",
  "_id": 1657843905795719196,
  "result": "created"
}

{
  "table": "pq_table",
  "type": "doc",
  "_id": 1657843905795719198,
  "result": "created"
}
```
<!-- intro -->
##### PHP
<!-- request PHP -->
```php
$newstoredquery = [
    'table' => 'pq_table',
    'body' => [
        'query' => [
                       'match' => [
                               'title' => 'shoes'
                       ]
               ],
               'range' => [
                       'price' => [
                               'gt' => 5
                       ]
               ]
       ],
    'tags' => ['Loius Vuitton']
];
$client->pq()->doc($newstoredquery);
```
<!-- response PHP -->
```php
Array(
       [index] => pq_table
       [type] => doc
       [_id] => 1657843905795719198
       [result] => created
)
```

<!-- intro -->
##### Python
<!-- request Python -->
```python
indexApi = api = manticoresearch.IndexApi(client)
newstoredquery ={"table" : "test_pq",   "doc" : {"query": {"ql": "@title shoes"},"filters": "price > 5","tags": ["Loius Vuitton"]}}
indexApi.insert(store_query)
```
<!-- response Python -->
```python
{'created': True,
 'found': None,
 'id': 1657843905795719198,
 'table': 'test_pq',
 'result': 'created'}
```

<!-- intro -->
##### Python-asyncio
<!-- request Python-asyncio -->
```python
indexApi = api = manticoresearch.IndexApi(client)
newstoredquery ={"table" : "test_pq",   "doc" : {"query": {"ql": "@title shoes"},"filters": "price > 5","tags": ["Loius Vuitton"]}}
await indexApi.insert(store_query)
```
<!-- response Python-asyncio -->
```python
{'created': True,
 'found': None,
 'id': 1657843905795719198,
 'table': 'test_pq',
 'result': 'created'}
```

<!-- intro -->
##### Javascript
<!-- request Javascript -->
```javascript
newstoredquery ={"table" : "test_pq",  "doc" : {"query": {"ql": "@title shoes"},"filters": "price > 5","tags": ["Loius Vuitton"]}};
res =  await indexApi.insert(store_query);
```
<!-- response Javascript -->
```javascript
{"table":"test_pq","_id":1657843905795719198,"created":true,"result":"created"}

```

<!-- intro -->
##### java
<!-- request Java -->
```java
newstoredquery = new HashMap<String,Object>(){{
    put("query",new HashMap<String,Object >(){{
        put("q1","@title shoes");
        put("filters","price>5");
        put("tags",new String[] {"Loius Vuitton"});
    }});
}};
newdoc.index("test_pq").setDoc(doc);
indexApi.insert(newdoc);
```

<!-- intro -->
##### C#
<!-- request C# -->
```clike
Dictionary<string, Object> query = new Dictionary<string, Object>();
query.Add("q1", "@title shoes");
query.Add("filters", "price>5");
query.Add("tags", new List<string> {"Loius Vuitton"});
Dictionary<string, Object> newstoredquery = new Dictionary<string, Object>();
newstoredquery.Add("query", query);
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "test_pq", doc: doc);
indexApi.Insert(newdoc);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let mut pq_doc = HashMap::new();
pq_doc.insert("q1".to_string(), serde_json::json!("@title shoes"));
pq_doc.insert("filters".to_string(), serde_json::json!("price>5"));
pq_doc.insert("tags".to_string(), serde_json::json!(["Louis Vitton"]));

let mut doc = HashMap::new();
pq_doc.insert("query".to_string(), serde_json::json!(pq_doc));

let insert_req = InsertDocumentRequest::new("test_pq".to_string(), serde_json::json!(doc));
let insert_res = index_api.insert(insert_req).await;
```
<!-- end -->

<!-- example noschema -->
## Отсутствие схемы в SQL
Если в SQL-команде `INSERT` схема опущена, ожидаются следующие параметры:
1. ID. Можно использовать `0` как ID, чтобы вызвать авто-генерацию идентификатора.
2. Query - полнотекстовый запрос.
3. Tags - строка тегов правила PQ.
4. Filters - дополнительные фильтры по атрибутам.

<!-- request SQL -->

```sql
INSERT INTO pq VALUES (0, '@title shoes', '', '');
INSERT INTO pq VALUES (0, '@title shoes', 'Louis Vuitton', '');
SELECT * FROM pq;
```
<!-- response SQL -->

```sql
+---------------------+--------------+---------------+---------+
| id                  | query        | tags          | filters |
+---------------------+--------------+---------------+---------+
| 2810855531667783688 | @title shoes |               |         |
| 2810855531667783689 | @title shoes | Louis Vuitton |         |
+---------------------+--------------+---------------+---------+
```
<!-- end -->

<!-- example replace -->
## Замена правил в таблице PQ

Чтобы заменить существующее правило PQ новым в SQL, просто используйте обычную команду [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md). Существует специальный синтаксис `?refresh=1` для замены правила PQ **определённого в JSON-режиме** через HTTP JSON интерфейс.


<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
mysql> select * from pq;
+---------------------+--------------+------+---------+
| id                  | query        | tags | filters |
+---------------------+--------------+------+---------+
| 2810823411335430148 | @title shoes |      |         |
+---------------------+--------------+------+---------+
1 row in set (0.00 sec)

mysql> replace into pq(id,query) values(2810823411335430148,'@title boots');
Query OK, 1 row affected (0.00 sec)

mysql> select * from pq;
+---------------------+--------------+------+---------+
| id                  | query        | tags | filters |
+---------------------+--------------+------+---------+
| 2810823411335430148 | @title boots |      |         |
+---------------------+--------------+------+---------+
1 row in set (0.00 sec)
```

<!-- request JSON -->
```json
GET /pq/pq/doc/2810823411335430149
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "_id": 2810823411335430149,
        "_score": 1,
        "_source": {
          "query": {
            "match": {
              "title": "shoes"
            }
          },
          "tags": "",
          "filters": ""
        }
      }
    ]
  }
}

PUT /pq/pq/doc/2810823411335430149?refresh=1 -d '{
  "query": {
    "match": {
      "title": "boots"
    }
  }
}'

GET /pq/pq/doc/2810823411335430149
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "_id": 2810823411335430149,
        "_score": 1,
        "_source": {
          "query": {
            "match": {
              "title": "boots"
            }
          },
          "tags": "",
          "filters": ""
        }
      }
    ]
  }
}
```

<!-- end -->
<!-- proofread -->

