# 向渗透表添加规则

<!-- example -->

在 [渗透表](../../Creating_a_table/Local_tables/Percolate_table.md) 中，存储的是渗透查询规则文档，且这些文档必须遵循以下四个字段的精确模式：

| 字段    | 类型   | 描述                                                         |
| ------- | ------ | ------------------------------------------------------------ |
| id      | bigint | PQ 规则标识符（如果省略，将自动分配）                        |
| query   | string | 全文查询（可以为空），与 [渗透表](../../Creating_a_table/Local_tables/Percolate_table.md) 兼容 |
| filters | string | 非全文字段的附加过滤条件（可以为空），与 [渗透表](../../Creating_a_table/Local_tables/Percolate_table.md) 兼容 |
| tags    | string | 一个或多个逗号分隔的标签字符串，可以用于有选择地显示/删除已保存的查询 |

不支持其他字段名称，否则会触发错误。

**警告：** 通过 SQL 插入/替换 JSON 格式的 PQ 规则是无效的。换句话说，JSON 特定操作符（如 `match` 等）将被视为规则文本的一部分，应该与文档匹配。如果您更喜欢使用 JSON 语法，请使用 HTTP 端点，而不是 `INSERT`/`REPLACE`。

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
有两种方式可以将渗透查询添加到渗透表中：

* 兼容 JSON /search 格式的查询，详见 [json/search](../../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON)
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

* SQL 格式的查询，详见 [search query syntax](../../Searching/Filters.md#SQL格式的查询)
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
    'index' => 'test_pq',
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
newstoredquery ={"index" : "test_pq", "id" : 2, "doc" : {"query": {"ql": "@title shoes"},"filters": "price > 5","tags": ["Loius Vuitton"]}}
indexApi.insert(newstoredquery)
```

<!-- intro -->
##### Javascript
<!-- request Javascript -->
```javascript
newstoredquery ={"index" : "test_pq", "id" : 2, "doc" : {"query": {"ql": "@title shoes"},"filters": "price > 5","tags": ["Loius Vuitton"]}};
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

<!-- end -->

<!-- example noid -->
## 自动 ID 分配

如果您没有指定 ID，则会自动分配。您可以在[这里](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#自动-ID)了解更多关于自动 ID 的信息。

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
  "index": "pq_table",
  "type": "doc",
  "_id": 1657843905795719196,
  "result": "created"
}

{
  "index": "pq_table",
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
    'index' => 'pq_table',
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
newstoredquery ={"index" : "test_pq",   "doc" : {"query": {"ql": "@title shoes"},"filters": "price > 5","tags": ["Loius Vuitton"]}}
indexApi.insert(store_query)
```
<!-- response Python -->
```python
{'created': True,
 'found': None,
 'id': 1657843905795719198,
 'index': 'test_pq',
 'result': 'created'}
```
<!-- intro -->
##### Javascript
<!-- request Javascript -->
```javascript
newstoredquery ={"index" : "test_pq",  "doc" : {"query": {"ql": "@title shoes"},"filters": "price > 5","tags": ["Loius Vuitton"]}};
res =  await indexApi.insert(store_query);
```
<!-- response Javascript -->
```javascript
{"_index":"test_pq","_id":1657843905795719198,"created":true,"result":"created"}

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

<!-- end -->

<!-- example noschema -->
## SQL 中无模式

在 SQL `INSERT` 命令中省略模式时，期望的参数如下：

1. ID。您可以使用 `0` 作为 ID 以触发自动 ID 生成。
2. Query - 全文查询。
3. Tags - PQ 规则标签字符串。
4. Filters - 根据属性的附加过滤条件。

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
## 替换渗透表中的规则

要在 SQL 中用新规则替换现有的 PQ 规则，只需使用常规的 [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md) 命令。对于通过 HTTP JSON 接口以 **JSON 模式** 定义的 PQ 规则，使用特殊语法 `?refresh=1` 来替换规则。


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