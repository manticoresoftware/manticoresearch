# MATCH

`MATCH` 子句允许在文本字段中进行全文搜索。输入的查询字符串会使用与索引期间应用于文本的相同设置进行[分词](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md)。除了对输入文本的分词外，查询字符串还支持许多[全文操作符](../../Searching/Full_text_matching/Operators.md)，用于规定关键字应如何提供有效匹配的各种规则。

全文匹配子句可以与属性[过滤器](../../Searching/Filters.md)组合，作为 AND 布尔关系。**全文匹配与属性过滤器之间不支持 OR 关系**。

匹配查询总是在过滤过程中首先执行，随后是[属性过滤器](../../Searching/Filters.md)。属性过滤器应用于匹配查询的结果集。没有 MATCH 子句的查询称为全表扫描（fullscan）。

`SELECT` 子句中最多只能有一个 `MATCH()`。

使用[全文查询语法](../../Searching/Full_text_matching/Operators.md)时，匹配会跨文档的所有已索引文本字段执行，除非表达式要求在某字段内匹配（如短语搜索）或受字段操作符所限制。

在使用[JOIN](../../Searching/Joining.md)查询时，`MATCH()` 可以接受一个可选的第二个参数，指定全文搜索应应用于哪个表。默认情况下，全文查询应用于 `JOIN` 操作中的左表：

```sql
SELECT * FROM table1 LEFT JOIN table2 ON table1.id = table2.id WHERE MATCH('search query', table2);
```

这允许你在连接操作中针对特定表执行全文搜索。有关在 JOIN 中使用 MATCH 的更多详细信息，请参见[连接表](../../Searching/Joining.md)部分。

## SQL

<!-- example Example_1 -->

```sql
MATCH('search query' [, table_name])
```
- `'search query'`：全文搜索查询字符串，可以包含各种[全文操作符](../../Searching/Full_text_matching/Operators.md)。
- `table_name`：（可选）要应用全文搜索的表名，用于 `JOIN` 查询中指定不同于默认左表的表。


[SELECT](../../Searching/Full_text_matching/Basic_usage.md#SQL) 语句使用 [MATCH](../../Searching/Full_text_matching/Basic_usage.md) 子句，必须放在 WHERE 之后，用于执行全文搜索。`MATCH()` 接受输入字符串，其中所有[全文操作符](../../Searching/Full_text_matching/Operators.md)都可用。


<!-- intro -->
##### SQL:

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
POST /sql?mode=raw -d "SELECT * FROM myindex WHERE MATCH('"find me fast"/2');"
```
<!-- response JSON -->

```JSON
[
  {
    "columns": [
      {
        "id": {
          "type": "long long"
        }
      },
      {
        "gid": {
          "type": "long"
        }
      },
      {
        "title": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "id": 1,
        "gid": 11,
        "title": "first find me"
      },
      {
        "id": 1,
        "gid": 12,
        "title": "second find me"
      },
    ],
    "total": 2,
    "error": "",
    "warning": ""
  }
]
```

<!-- request MATCH with filters -->
一个使用 MATCH 和 WHERE 过滤器进行更复杂查询的示例。

```sql
SELECT * FROM myindex WHERE MATCH('cats|birds') AND (`title`='some title' AND `id`=123);
```

<!-- end -->

## HTTP JSON

<!-- example Example_11 -->

全文匹配功能可在 `/search` 端点及 HTTP 客户端中使用。以下子句可用于执行全文匹配：

### match

"match" 是简单查询，在指定字段中匹配指定的关键字。

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
或者使用 `_all` 或 `*` 来搜索所有字段。

你也可以使用 "!field" 搜索除某字段外的所有字段：

```json
"match":
{
  "!field1": "keyword"
}
```
默认情况下，关键字通过 OR 运算符组合。但你可以通过 "operator" 子句更改此行为：

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

"operator" 可设置为 "or" 或 "and"。

也可以应用 `boost` 修改符。它通过指定的因子提升词的 [IDF](../../Searching/Options.md#idf) 分数（在评分中结合 IDF 的排名分数中体现）。它不会以任何方式影响匹配过程。
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

"match_phrase" 是匹配整个短语的查询，类似于 SQL 的短语操作符，示例：

```json
"query":
{
  "match_phrase": { "_all" : "had grown quite" }
}
```

### query_string
"query_string" 接受一个作为 `MATCH()` 语法的全文查询的输入字符串。

```json
"query":
{
  "query_string": "Church NOTNEAR/3 street"
}
```

### match_all

"match_all" 接受一个空对象，返回表中的文档，不执行任何属性过滤或全文匹配。或者，直接省略请求中的 `query` 子句也有同样效果。

```json
"query":
{
  "match_all": {}
}
```


### 结合全文过滤和其他过滤器

所有全文匹配子句都可以结合 [must](../../Searching/Filters.md#must)、[must_not](../../Searching/Filters.md#must_not) 和 [should](../../Searching/Filters.md#should) 操作符使用 [JSON `bool` 查询](../../Searching/Filters.md#bool-query)。

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

