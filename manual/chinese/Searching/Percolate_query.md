# Percolate 查询

Percolate 查询也被称为持久查询、前瞻搜索、文档路由、反向搜索和逆向搜索。

传统的搜索方式是存储文档并针对它们执行搜索查询。然而，有些情况下我们希望将查询应用于新到达的文档，以检测是否匹配。一些需要这种方式的场景包括监控系统收集数据并通知用户特定事件，如达到某个指标阈值或监控数据中出现特定值。另一个例子是新闻聚合，用户可能只想被通知某些类别或话题，甚至特定的“关键词”。

在这些情况下，传统搜索并不适合，因为它假定所需的搜索是在整个集合上执行的。这个过程会乘以用户数量，导致许多查询同时在整个集合上运行，从而引起显著的额外负载。本节描述的替代方法是存储查询，然后将它们应用于一个新到达的文档或一批文档。

Google Alerts、AlertHN、彭博终端以及其他允许用户订阅特定内容的系统使用类似的技术。

> * 参见 [percolate](../Creating_a_table/Local_tables/Percolate_table.md) 了解如何创建 PQ 表。
> * 参见 [向 percolate 表添加规则](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md) 学习如何添加 percolate 规则（也称为 PQ 规则）。这里是一个快速示例：


### 使用 CALL PQ 执行 percolate 查询

关于 percolate 查询需要记住的关键是你的搜索查询已经存在表中。你需要提供的是文档，**用以检查它们是否匹配任何存储的规则**。

你可以通过 SQL 或 JSON 接口执行 percolate 查询，也可以使用编程语言客户端。SQL 方法提供更多灵活性，而 HTTP 方法更简单，且提供了大部分必需功能。下表可以帮助你理解它们的差异。

| 期望的行为                     | SQL                                     | HTTP                                 |
| ----------------------------- | --------------------------------------- | ------------------------------------ |
| 提供单个文档                 | `CALL PQ('tbl', '{doc1}')`              | `query.percolate.document{doc1}`     |
| 提供单个文档（备选）         | `CALL PQ('tbl', 'doc1', 0 as docs_json)` | - |
| 提供多个文档                 | `CALL PQ('tbl', ('doc1', 'doc2'), 0 as docs_json)` | - |
| 提供多个文档（备选）         | `CALL PQ('tbl', ('{doc1}', '{doc2}'))` | - |
| 提供多个文档（备选）         | `CALL PQ('tbl', '[{doc1}, {doc2}]')` | - |
| 返回匹配的文档 id           | 0/1 作为 docs（默认禁用）                | 默认启用                           |
| 使用文档自己的 id 在结果中显示 | 'id field' 作为 docs_id（默认禁用）       | 不支持                               |
| 认为输入文档是 JSON          | 1 作为 docs_json（默认是 1）              | 默认启用                           |
| 认为输入文档是纯文本          | 0 作为 docs_json（默认是 1）              | 不支持                               |
| [稀疏分布模式](../Searching/Percolate_query.md#I-want-higher-performance-of-a-percolate-query) | 默认                                   | 默认                               |
| [分片分布模式](../Searching/Percolate_query.md#I-want-higher-performance-of-a-percolate-query) | sharded 作为 mode                        | 不支持                               |
| 返回匹配查询的所有信息       | 1 作为 query（默认是 0）                  | 默认启用                           |
| 跳过无效 JSON               | 1 作为 skip_bad_json（默认是 0）           | 不支持                               |
| 在 [SHOW META](../Node_info_and_management/SHOW_META.md) 中显示扩展信息 | 1 作为 verbose（默认是 0）               | 不支持                               |
| 定义当未提供 docs_id 字段时将添加到文档 id 的数值（主要针对[分布式 PQ 模式](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#Distributed-percolate-tables-%28DPQ-tables%29)） | 1 作为 shift（默认是 0）                | 不支持                               |

<!-- example create percolate -->
为了演示它是如何工作的，以下是几个例子。我们创建一个包含两个字段的 PQ 表：

* title（文本）
* color（字符串）

并在其中添加三个规则：

* 纯全文。查询：`@title bag`
* 全文和过滤。查询：`@title shoes`。过滤器：`color='red'`
* 全文和更复杂的过滤。查询：`@title shoes`。过滤器：`color IN('blue', 'green')`

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
##### 只告诉我哪些 PQ 规则匹配我的单个文档

第一个文档不匹配任何规则。它可能匹配前两个，但它们需要额外的过滤器。

第二个文档匹配一个规则。请注意，CALL PQ 默认期望文档为 JSON，但如果使用 `0 as docs_json`，你可以传递纯字符串。

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
##### 我想知道与我的文档匹配的完整 PQ 规则
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

##### 多个文档怎么样？

请注意，通过 `CALL PQ`，您可以通过不同的方式提供多个文档：

* 作为圆括号中的普通文档数组 `('doc1', 'doc2')`。这要求 `0 as docs_json`
* 作为圆括号中的 JSON 数组 `('{doc1}', '{doc2}')`
* 或作为标准 JSON 数组 `'[{doc1}, {doc2}]'`

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
##### 我想知道哪些文档匹配哪些规则

使用选项 `1 as docs` 可以让您查看所提供的文档中哪些匹配了哪些规则。
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
#### 静态 id
默认情况下，匹配的文档 id 对应于您提供列表中的相对编号。然而，在某些情况下，每个文档已经有自己的 id。对于这种情况，`CALL PQ` 提供了选项 `'id field name' as docs_id`。

请注意，如果通过提供的字段名找不到 id，PQ 规则将不会在结果中显示。

此选项仅适用于通过 SQL 调用 `CALL PQ`。

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
##### 我可能有无效的 JSON，请跳过它们

当使用 CALL PQ 执行分离的 JSON 时，可以使用选项 1 作为 skip_bad_json 来跳过输入中的任何无效 JSON。在下面的示例中，第 2 个查询由于无效的 JSON 而失败，但第 3 个查询通过使用 1 作为 skip_bad_json 避免了错误。请记住，当通过 HTTP 发送 JSON 查询时，不支持此选项，因为此时必须保证整个 JSON 查询的有效性。

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

##### 我想要更高性能的 percolate 查询
Percolate 查询是为高吞吐量和大数据量设计的。为优化性能以降低延迟和提高吞吐量，请考虑以下内容。

percolate 表有两种分布模式，以及 percolate 查询如何针对它们工作：

* **Sparse（默认）。** 适合：大量文档，镜像 PQ 表。当文档集很大，而存储在 PQ 表中的查询集较小时，稀疏模式是有利的。在这种模式下，您传递的文档批次将被划分给多个代理，因此每个节点只处理请求中文档的一部分。Manticore 会将文档集拆分并分发给镜像。代理处理完查询后，Manticore 收集并合并结果，返回一个最终的查询集，就好像它来自单个表一样。使用[复制](../References.md#Replication)来协助此过程。
* **Sharded。** 适合：大量 PQ 规则，将规则拆分到多个 PQ 表中。在此模式中，整个文档集将广播给分布式 PQ 表的所有表，而不是先拆分文档。这在推动相对较小的文档集，而存储的大量查询时很有用。在这种情况下，更合适的是在每个节点上只存储一部分 PQ 规则，然后合并多个节点处理同一文档集但针对不同 PQ 规则集返回的结果。此模式需显式设置，因为它会增加网络负载，并期望表具有不同的 PQ，[复制](../References.md#Replication) 不能开箱即用地处理。

假设您有如下定义的表 `pq_d2`：

``` ini
table pq_d2
{
    type = distributed
    agent = 127.0.0.1:6712:pq
    agent = 127.0.0.1:6712:ptitle
}
```

<!-- example distributed pq modes 1 -->
每个 'pq' 和 'ptitle' 包含：


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

然后在分布式表上执行带有几个文档的 `CALL PQ`。


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

在前面的示例中，我们使用了默认的 **sparse** 模式。为了展示 **sharded** 模式，我们创建一个由 2 个本地 PQ 表组成的分布式 PQ 表，并向 "products1" 添加 2 个文档，向 "products2" 添加 1 个文档：
```sql
create table products1(title text, color string) type='pq';
create table products2(title text, color string) type='pq';
create table products_distributed type='distributed' local='products1' local='products2';

INSERT INTO products1(query) values('@title bag');
INSERT INTO products1(query,filters) values('@title shoes', 'color=\'red\'');
INSERT INTO products2(query,filters) values('@title shoes', 'color in (\'blue\', \'green\')');
```

<!-- example sharded -->
现在，如果将 `'sharded' as mode` 添加到 `CALL PQ`，它将把文档发送到所有代理的表（本例中仅为本地表，但它们也可以是远程的以利用外部硬件）。此模式通过 JSON 接口不可用。

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

请注意，配置中代理镜像的语法（当一行 `agent` 指定多个主机，用 `|` 分隔时）与 `CALL PQ` 的查询模式无关。每个 `agent` 始终代表**一个**节点，无论该代理指定了多少 HA 镜像。

<!-- example verbose -->
##### 我怎样才能了解更多关于性能的信息？
在某些情况下，您可能想要获取关于 percolate 查询性能的更多细节。为此，有一个选项 `1 as verbose`，该选项仅通过 SQL 可用，允许您保存更多的性能指标。您可以使用 `SHOW META` 查询查看它们，该查询可以在 `CALL PQ` 之后运行。更多信息请参见 [SHOW META](../Node_info_and_management/SHOW_META.md)。

<!-- intro -->
1 as verbose：
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
0 as verbose（默认）：
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

