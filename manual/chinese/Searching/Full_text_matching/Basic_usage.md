# MATCH

`MATCH` 子句允许对文本字段进行全文搜索。输入的查询字符串使用与索引时应用于文本的相同设置进行[分词](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md)。除了输入文本的分词外，查询字符串还支持多种[全文运算符](../../Searching/Full_text_matching/Operators.md)，用于强制执行关键字如何提供有效匹配的各种规则。

全文匹配子句可以与属性[过滤器](../../Searching/Filters.md)结合使用，作为 AND 布尔关系。**不支持全文匹配与属性过滤器之间的 OR 关系**。

匹配查询总是在过滤过程中的第一步执行，随后是[属性过滤器](../../Searching/Filters.md)。属性过滤器应用于匹配查询的结果集。没有匹配子句的查询称为全表扫描。

在 `SELECT` 子句中，最多只能有一个 `MATCH()`。

使用[全文查询语法](../../Searching/Full_text_matching/Operators.md)时，匹配会跨文档的所有已索引文本字段进行，除非表达式要求在某个字段内匹配（如短语搜索），或被字段运算符限制。

使用[JOIN](../../Searching/Joining.md) 查询时，`MATCH()`可以接受一个可选的第二个参数，指定全文搜索应用到哪张表。默认情况下，全文查询应用于 `JOIN` 操作中的左表：

```sql
SELECT * FROM table1 LEFT JOIN table2 ON table1.id = table2.id WHERE MATCH('search query', table2);
```

这允许你对连接操作中的特定表执行全文搜索。有关使用 MATCH 和 JOIN 的更多细节，请参见[连接表](../../Searching/Joining.md)部分。

## SQL

<!-- example Example_1 -->

```sql
MATCH('search query' [, table_name])
```
- `'search query'`：全文搜索查询字符串，可以包含各种[全文运算符](../../Searching/Full_text_matching/Operators.md)。
- `table_name`：（可选）应用全文搜索的表名，用于 `JOIN` 查询中指定非默认左表的其他表。


[SELECT](../../Searching/Full_text_matching/Basic_usage.md#SQL) 语句使用 [MATCH](../../Searching/Full_text_matching/Basic_usage.md) 子句，必须位于 WHERE 后，用于执行全文搜索。`MATCH()` 接受一个输入字符串，其中所有[全文运算符](../../Searching/Full_text_matching/Operators.md)均可用。


<!-- intro -->
##### SQL:


<!--
data for the following example:

DROP TABLE IF EXISTS myindex;
CREATE TABLE myindex(gid int, title text);
INSERT INTO myindex(gid, title) VALUES
(11, 'first find me'),
(12, 'second find me');
-->

<!-- request SQL -->

```sql
SELECT * FROM myindex WHERE MATCH('"find me fast"/2');
```
<!-- response SQL -->

```sql
+------+------+----------------+
| id   | gid  | title          |
+------+------+----------------+
|    1 |   11 | first find me  |
|    2 |   12 | second find me |
+------+------+----------------+
2 rows in set (0.00 sec)
```

<!-- request JSON -->

```JSON
POST /search
{
  "table": "myindex",
  "query": {
    "match" : { "_all" : "\"find me fast\"/2" }
  }
}
```
<!-- response JSON -->

```JSON
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "total_relation": "eq",
    "hits": [
      {
        "_id": 1,
        "_score": 1500,
        "_source": {
          "gid": 11,
          "title": "first find me"
        }
      },
      {
        "_id": 2,
        "_score": 1500,
        "_source": {
          "gid": 12,
          "title": "second find me"
        }
      }
    ]
  }
}
```

<!-- request MATCH with filters -->
使用 WHERE 过滤器结合 MATCH 的更复杂查询示例。

```sql
SELECT * FROM myindex WHERE MATCH('cats|birds') AND (`title`='some title' AND `id`=123);
```

<!-- end -->

## HTTP JSON

<!-- example Example_11 -->

全文匹配可通过 `/search` 端点和基于 HTTP 的客户端使用。以下子句可用于执行全文匹配：

### match

“match” 是一个简单查询，用于在指定字段中匹配指定关键字。

```json
"query":
{
  "match": { "field": "keyword" }
}
```

你可以指定字段列表：

```json
"match":
{
  "field1,field2": "keyword"
}
```
或者使用 `_all` 或 `*` 搜索所有字段。

你也可以使用 "!field" 排除一个字段搜索所有其他字段：

```json
"match":
{
  "!field1": "keyword"
}
```
默认情况下，关键字通过 OR 运算符合并。但你可以使用 "operator" 子句更改此行为：

```json
"query":
{
  "match":
  {
    "content,title":
    {
      "query":"keyword",
      "operator":"or"
    }
  }
}
```

“operator” 可以设置为 "or" 或 "and"。

还可以应用 `boost` 修饰符。它通过指定的因子提升单词的[IDF](../../Searching/Options.md#idf)评分，影响包含 IDF 的排名评分，但不会以任何方式影响匹配过程。
```json
"query":
{
  "match":
  {
    "field1":
    {
      "query": "keyword",
      "boost": 2.0
    }
  }
}
```

### match_phrase

“match_phrase” 是匹配整个短语的查询。它类似于 SQL 中的短语运算符。示例如下：

```json
"query":
{
  "match_phrase": { "_all" : "had grown quite" }
}
```

### query_string
“query_string” 接受一个输入字符串，作为 `MATCH()` 语法的全文查询。

```json
"query":
{
  "query_string": "Church NOTNEAR/3 street"
}
```

### match_all

“match_all” 接受一个空对象，并返回表中的文档，不执行任何属性过滤或全文匹配。或者，你也可以省略请求中的 `query` 子句，效果相同。

```json
"query":
{
  "match_all": {}
}
```


### 将全文过滤与其他过滤器结合使用

所有全文匹配子句都可以与 [must](../../Searching/Filters.md#must)、[must_not](../../Searching/Filters.md#must_not) 和 [should](../../Searching/Filters.md#should) 运算符结合使用，它们属于 [JSON `bool` 查询](../../Searching/Filters.md#bool-query)。

<!-- intro -->
示例：

<!-- request match -->

```json
POST /search
-d
'{
    "table" : "hn_small",
    "query":
    {
        "match":
        {
            "*" : "find joe"
        }
    },
    "_source": ["story_author","comment_author"],
    "limit": 1
}'
```

<!-- response match -->
```json
{
   "took" : 3,
   "timed_out" : false,
   "hits" : {
      "hits" : [
         {
            "_id": 668018,
            "_score" : 3579,
            "_source" : {
               "story_author" : "IgorPartola",
               "comment_author" : "joe_the_user"
            }
         }
      ],
      "total" : 88063,
      "total_relation" : "eq"
   }
}
```
<!-- request match_phrase -->
```json
POST /search
-d
'{
    "table" : "hn_small",
    "query":
    {
        "match_phrase":
        {
            "*" : "find joe"
        }
    },
    "_source": ["story_author","comment_author"],
    "limit": 1
}'
```
<!-- response match_phrase -->
```json
{
   "took" : 3,
   "timed_out" : false,
   "hits" : {
      "hits" : [
         {
            "_id": 807160,
            "_score" : 2599,
            "_source" : {
               "story_author" : "rbanffy",
               "comment_author" : "runjake"
            }
         }
      ],
      "total" : 2,
      "total_relation" : "eq"
   }
}
```

<!-- request query_string -->
```json
POST /search
-d
'{   "table" : "hn_small",
    "query":
    {
        "query_string": "@comment_text \"find joe fast \"/2"
    },
    "_source": ["story_author","comment_author"],
    "limit": 1
}'
```
<!-- response query_string -->
```json
{
  "took" : 3,
  "timed_out" : false,
  "hits" : {
      "hits" : [
         {
            "_id": 807160,
            "_score" : 2566,
            "_source" : {
               "story_author" : "rbanffy",
               "comment_author" : "runjake"
            }
         }
      ],
      "total" : 1864,
      "total_relation" : "eq"
   }
}
```
<!-- request PHP -->
```php
$search = new Search(new Client());
$result = $search->('@title find me fast');
foreach($result as $doc)
{
    echo 'Document: '.$doc->getId();
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.': '.$value;
    }
}
```
<!-- response PHP -->
```php
Document: 1
title: first find me fast
gid: 11
Document: 2
title: second find me fast
gid: 12

```

<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.search({"table":"hn_small","query":{"query_string":"@comment_text \"find joe fast \"/2"}, "_source": ["story_author","comment_author"], "limit":1})
```
<!-- response Python -->
``` python
{'aggregations': None,
 'hits': {'hits': [{'_id': '807160',
                    '_score': 2566,
                    '_source': {'comment_author': 'runjake',
                                'story_author': 'rbanffy'}}],
          'max_score': None,
          'total': 1864,
          'total_relation': 'eq'},
 'profile': None,
 'timed_out': False,
 'took': 2,
 'warning': None}
```

<!-- intro -->
Python-asyncio
<!-- request Python-asyncio -->

```python
await searchApi.search({"table":"hn_small","query":{"query_string":"@comment_text \"find joe fast \"/2"}, "_source": ["story_author","comment_author"], "limit":1})
```
<!-- response Python-asyncio -->
``` python
{'aggregations': None,
 'hits': {'hits': [{'_id': '807160',
                    '_score': 2566,
                    '_source': {'comment_author': 'runjake',
                                'story_author': 'rbanffy'}}],
          'max_score': None,
          'total': 1864,
          'total_relation': 'eq'},
 'profile': None,
 'timed_out': False,
 'took': 2,
 'warning': None}
```

<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"hn_small","query":{"query_string":"@comment_text \"find joe fast \"/2"}, "_source": ["story_author","comment_author"], "limit":1});
```
<!-- response javascript -->
```javascript
{
  took: 1,
  timed_out: false,
  hits:
   exports {
     total: 1864,
     total_relation: 'eq',
     hits:
      [ { _id: '807160',
          _score: 2566,
          _source: { story_author: 'rbanffy', comment_author: 'runjake' }
        }
      ]
   }
}
```

<!-- intro -->
java
<!-- request Java -->

```java

query = new HashMap<String,Object>();
query.put("query_string", "@comment_text \"find joe fast \"/2");
searchRequest = new SearchRequest();
searchRequest.setIndex("hn_small");
searchRequest.setQuery(query);
searchRequest.addSourceItem("story_author");
searchRequest.addSourceItem("comment_author");
searchRequest.limit(1);
searchResponse = searchApi.search(searchRequest);
```
<!-- response Java -->
```java
class SearchResponse {
    took: 1
    timedOut: false
    aggregations: null
    hits: class SearchResponseHits {
        maxScore: null
        total: 1864
        totalRelation: eq
        hits: [{_id=807160, _score=2566, _source={story_author=rbanffy, comment_author=runjake}}]
    }
    profile: null
    warning: null
}
```

<!-- intro -->
C#
<!-- request C# -->

```clike
object query =  new { query_string="@comment_text \"find joe fast \"/2" };
var searchRequest = new SearchRequest("hn_small", query);
searchRequest.Source = new List<string> {"story_author", "comment_author"};
searchRequest.Limit = 1;
SearchResponse searchResponse = searchApi.Search(searchRequest);
```
<!-- response C# -->
```clike
class SearchResponse {
    took: 1
    timedOut: false
    aggregations: null
    hits: class SearchResponseHits {
        maxScore: null
        total: 1864
        totalRelation: eq
        hits: [{_id=807160, _score=2566, _source={story_author=rbanffy, comment_author=runjake}}]
    }
    profile: null
    warning: null
}
```

<!-- intro -->
Rust
<!-- request Rust -->

```rust
let query = SearchQuery {
     query_string: Some(serde_json::json!("@comment_text \"find joe fast \"/2").into()),
    ..Default::default()
};
let search_req = SearchRequest {
    table: "hn_small".to_string(),
    query: Some(Box::new(query)),
    source: serde_json::json!(["story_author", "comment_author"]),
    limit: serde_json::json!(1),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```
<!-- response Rust -->
```rust
class SearchResponse {
    took: 1
    timedOut: false
    aggregations: null
    hits: class SearchResponseHits {
        maxScore: null
        total: 1864
        totalRelation: eq
        hits: [{_id=807160, _score=2566, _source={story_author=rbanffy, comment_author=runjake}}]
    }
    profile: null
    warning: null
}
```

<!-- intro -->
TypeScript
<!-- request TypeScript -->

```typescript
res = await searchApi.search({
  index: 'test',
  query: { query_string: "test document 1" },
  "_source": ["content", "title"],
  limit: 1
});
```
<!-- response TypeScript -->
```json
{
  took: 1,
  timed_out: false,
  hits:
   exports {
     total: 5,
     total_relation: 'eq',
     hits:
      [ { _id: '1',
          _score: 2566,
          _source: { content: 'This is a test document 1', title: 'Doc 1' }
        }
      ]
   }
}
```

<!-- intro -->
Go
<!-- request Go -->

```go
searchRequest := manticoresearch.NewSearchRequest("test")
query := map[string]interface{} {"query_string": "test document 1"}
searchReq.SetSource([]string{"content", "title"})
searchReq.SetLimit(1)
resp, httpRes, err := search.SearchRequest(*searchRequest).Execute()
```
<!-- response Go -->
```json
{
  "hits": {
    "hits": [
      {
        "_id": 1,
        "_score": 2566,
        "_source": {
          "content": "This is a test document 1",
          "title": "Doc 1"
        }
      }
    ],
    "total": 5,
    "total_relation": "eq"
  },
  "timed_out": false,
  "took": 0
}
```
<!-- end -->
<!-- proofread -->

