# 向 percolate 表添加规则

<!-- example -->
在 [percolate 表](../../Creating_a_table/Local_tables/Percolate_table.md) 中，存储了 percolate 查询规则，必须遵循包含四个字段的精确模式：

| 字段 | 类型 | 描述 |
| - | - | - |
| id | bigint | PQ 规则标识符（如果省略，将自动分配） |
| query | string | 全文查询（可为空），兼容 [percolate 表](../../Creating_a_table/Local_tables/Percolate_table.md) |
| filters | string | 通过非全文字段的附加过滤器（可为空），兼容 [percolate 表](../../Creating_a_table/Local_tables/Percolate_table.md) |
| tags   | string | 一个包含一个或多个逗号分隔标签的字符串，可用于有选择地显示/删除已保存的查询 |

任何其他字段名不被支持，且会触发错误。

**警告：** 通过 SQL 插入/替换 JSON 格式的 PQ 规则将无法正常工作。换句话说，JSON 特定操作符（如 `match` 等）将被视为规则文本的一部分，应与文档匹配。如果您希望使用 JSON 语法，请使用 HTTP 端点而不是 `INSERT`/`REPLACE`。

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
添加 percolate 查询到 percolate 表有两种方式：
* 使用 JSON /search 兼容格式的查询，详见 [json/search](../../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON)
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

* 使用 SQL 格式的查询，详见 [search query syntax](../../Searching/Filters.md#Queries-in-SQL-format)
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
## 自动 ID 分配

如果未指定 ID，将自动分配。您可以在[此处](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID)阅读有关自动 ID 的更多信息。

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
## SQL 中无模式
如果 SQL `INSERT` 命令中省略了模式，则期望如下参数：
1. ID。您可以使用 `0` 作为 ID 来触发自动生成 ID。
2. Query - 全文查询。
3. Tags - PQ 规则标签字符串。
4. Filters - 通过属性的附加过滤器。

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

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "INSERT INTO pq VALUES (0, '@title shoes', '', '');"
POST /sql?mode=raw -d "INSERT INTO pq VALUES (0, '@title shoes', 'Louis Vuitton', '');"
POST /sql?mode=raw -d "SELECT * FROM pq;"
```

<!-- response JSON -->
```JSON
[
  {
    "total": 1,
    "error": "",
    "warning": ""
  }
]
[
  {
    "total": 1,
    "error": "",
    "warning": ""
  }
]
[
  {
    "columns": [
      {
        "id": {
          "type": "long long"
        }
      },
      {
        "query": {
          "type": "string"
        }
      },
      {
        "tags": {
          "type": "string"
        }
      },
      {
        "filters": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "id": 724024784404348900,
        "query": "@title shoes",
        "tags": "Louis Vuitton",
        "filters": ""
      },
      {
        "id": 724024784404348900,
        "query": "@title shoes",
        "tags": "",
        "filters": ""
      }
    ],
    "total": 2,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

<!-- example replace -->
## 替换 PQ 表中的规则

要在 SQL 中用新规则替换现有的 PQ 规则，只需使用常规的 [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md) 命令。通过 HTTP JSON 接口替换**以 JSON 模式定义的** PQ 规则，有一个特殊语法 `?refresh=1`。


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

