# 匹配

`MATCH` 子句允许在文本字段中进行全文搜索。输入查询字符串使用与文本在索引过程中应用的相同设置进行 [标记化](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md)。除了输入文本的标记化之外，查询字符串支持多种 [全文操作符](../../Searching/Full_text_matching/Operators.md)，以强制执行关键字应提供有效匹配的各种规则。

全文匹配子句可以与属性 [过滤器](../../Searching/Filters.md) 结合使用，作为 AND 布尔值。**全文匹配与属性过滤器之间的 OR 关系不被支持**。

匹配查询始终在过滤过程中首先执行，然后是 [属性过滤器](../../Searching/Filters.md)。属性过滤器应用于匹配查询的结果集。没有匹配子句的查询称为全扫描。

`SELECT` 子句中最多只能有一个 `MATCH()`。

使用 [全文查询语法](../../Searching/Full_text_matching/Operators.md)，匹配将在文档的所有索引文本字段中执行，除非该表达式要求在某个字段内匹配（如短语搜索）或受到字段操作符的限制。

在使用 [JOIN](../../Searching/Joining.md) 查询时，`MATCH()` 可以接受一个可选的第二个参数，该参数指定全文搜索应应用于哪个表。默认情况下，全文查询应用于 `JOIN` 操作中的左表：

```sql
SELECT * FROM table1 LEFT JOIN table2 ON table1.id = table2.id WHERE MATCH('search query', table2);
```

这允许您在连接操作中对特定表执行全文搜索。有关在 JOIN 中使用 MATCH 的更多详细信息，请参阅 [连接表](../../Searching/Joining.md) 部分。

## SQL

<!-- example Example_1 -->

```sql
MATCH('search query' [, table_name])
```
- `'search query'`: 全文搜索查询字符串，可以包含各种 [全文操作符](../../Searching/Full_text_matching/Operators.md)。
- `table_name`: （可选）要应用全文搜索的表的名称，用于 `JOIN` 查询，以指定不同于默认左表的表。


[SELECT](../../Searching/Full_text_matching/Basic_usage.md#SQL) 语句使用了 [MATCH](../../Searching/Full_text_matching/Basic_usage.md) 子句，该子句必须置于 WHERE 之后，用于执行全文搜索。`MATCH()` 接受一个输入字符串，其中可用所有 [全文操作符](../../Searching/Full_text_matching/Operators.md)。


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

<!-- request MATCH with filters -->
使用 MATCH 和 WHERE 过滤器的更复杂查询示例。

```sql
SELECT * FROM myindex WHERE MATCH('cats|birds') AND (`title`='some title' AND `id`=123);
```

<!-- end -->

## HTTP JSON

<!-- example Example_11 -->

全文匹配可在 `/search` 端点和基于 HTTP 的客户端中使用。可以使用以下子句来执行全文匹配：

### 匹配

"match" 是一个简单的查询，会匹配指定字段中的指定关键字。

```json
"query":
{
  "match": { "field": "keyword" }
}
```

您可以指定字段列表：

```json
"match":
{
  "field1,field2": "keyword"
}
```
或者您可以使用 `_all` 或 `*` 来搜索所有字段。

您可以使用 "!field" 搜索所有字段，但排除一个字段：

```json
"match":
{
  "!field1": "keyword"
}
```
默认情况下，关键字使用 OR 操作符组合。但是，您可以使用 "operator" 子句更改该行为：

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

"operator" 可以设置为 "or" 或 "and"。

`boost` 修饰符也可以应用。它通过所指示的因子在包含 IDF 的排名分数中提高词 [IDF](../../Searching/Options.md#idf)_score。它不以任何方式影响匹配过程。
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

### 匹配短语

"match_phrase" 是一个匹配整个短语的查询。它类似于 SQL 中的短语操作符。以下是一个示例：

```json
"query":
{
  "match_phrase": { "_all" : "had grown quite" }
}
```

### 查询字符串
"query_string" 接受输入字符串作为 `MATCH()` 语法的全文查询。

```json
"query":
{
  "query_string": "Church NOTNEAR/3 street"
}
```

### 匹配所有

"match_all" 接受一个空对象，并返回表中的文档，而不执行任何属性过滤或全文匹配。或者，您可以直接在请求中省略 `query` 子句，其效果相同。

```json
"query":
{
  "match_all": {}
}
```


### 将全文过滤与其他过滤结合

所有全文匹配子句可以与 [must](../../Searching/Filters.md#must)、[must_not](../../Searching/Filters.md#must_not) 和 [should](../../Searching/Filters.md#should) 操作符结合使用，在 [JSON `bool` 查询](../../Searching/Filters.md#bool-query) 中使用。

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
            "*" : "查找乔"
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
            "*" : "查找乔"
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
        "query_string": "@comment_text "查找乔 快 "/2"
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
$result = $search->('@title 查找我快');
foreach($result as $doc)
{
    echo '文档: '.$doc->getId();
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.': '.$value;
    }
}
```
<!-- response PHP -->
```php
Document: 1
title: 首次查找我快
gid: 11
Document: 2
title: 第二次查找我快
gid: 12

```

<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.search({"table":"hn_small","query":{"query_string":"@comment_text "查找乔 快 "/2"}, "_source": ["story_author","comment_author"], "limit":1})
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
await searchApi.search({"table":"hn_small","query":{"query_string":"@comment_text "查找乔 快 "/2"}, "_source": ["story_author","comment_author"], "limit":1})
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
res = await searchApi.search({"table":"hn_small","query":{"query_string":"@comment_text "查找乔 快 "/2"}, "_source": ["story_author","comment_author"], "limit":1});
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
query.put("query_string", "@comment_text "查找乔 快 "/2");
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
object query =  new { query_string="@comment_text "查找乔 快 "/2" };
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
     query_string: Some(serde_json::json!("@comment_text "find joe fast "/2").into()),
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
# 匹配

`MATCH` 子句允许在文本字段中进行全文搜索。输入查询字符串使用与文本在索引时应用的相同设置进行 [分词](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md)。除了输入文本的分词之外，查询字符串支持多种 [全文运算符](../../Searching/Full_text_matching/Operators.md)，这些运算符强制执行关键字应提供有效匹配的不同规则。

全文匹配子句可以与属性 [过滤器](../../Searching/Filters.md) 组合，作为 AND 布尔运算。**不支持全文匹配和属性过滤器之间的 OR 关系**。

在过滤过程中，匹配查询始终优先执行，接着是 [属性过滤器](../../Searching/Filters.md)。属性过滤器应用于匹配查询的结果集。没有匹配子句的查询称为全扫描。

`SELECT` 子句中最多只能有一个 `MATCH()`。

使用 [全文查询语法](../../Searching/Full_text_matching/Operators.md)，匹配会在文档的所有索引文本字段中进行，除非表达式需要在某个字段内进行匹配（例如短语搜索）或受到字段运算符的限制。

当使用 [JOIN](../../Searching/Joining.md) 查询时，`MATCH()` 可以接受一个可选的第二个参数，指定全文搜索应应用于哪个表。默认情况下，全文查询应用于 `JOIN` 操作中的左表：

```sql
SELECT * FROM table1 LEFT JOIN table2 ON table1.id = table2.id WHERE MATCH('搜索查询', table2);
```

这允许您在连接操作中对特定表执行全文搜索。有关使用 MATCH 与 JOIN 的更多细节，请参阅 [连接表](../../Searching/Joining.md) 部分。

## SQL

<!-- example Example_1 -->

```sql
MATCH('搜索查询' [, 表名])
```
- `'搜索查询'`：全文搜索查询字符串，可以包含各种 [全文运算符](../../Searching/Full_text_matching/Operators.md)。
- `表名`： （可选）要应用全文搜索的表名，用于 `JOIN` 查询以指定不同于默认左表的表。


[SELECT](../../Searching/Full_text_matching/Basic_usage.md#SQL) 语句使用 [MATCH](../../Searching/Full_text_matching/Basic_usage.md) 子句，必须在 WHERE 之后，用于执行全文搜索。`MATCH()` 接受一个输入字符串，其中可用所有 [全文运算符](../../Searching/Full_text_matching/Operators.md)。


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT * FROM myindex WHERE MATCH('"迅速找到我"/2');
```
<!-- response SQL -->

```sql
+------+------+----------------+
| id   | gid  | title          |
+------+------+----------------+
|    1 |   11 | 首次找到我     |
|    2 |   12 | 再次找到我     |
+------+------+----------------+
2 rows in set (0.00 sec)
```

<!-- request MATCH with filters -->
一个使用 MATCH 和 WHERE 过滤器的更复杂查询示例。

```sql
SELECT * FROM myindex WHERE MATCH('猫|鸟') AND (`title`='某个标题' AND `id`=123);
```

<!-- end -->

## HTTP JSON

<!-- example Example_11 -->

全文匹配可在 `/search` 端点和基于 HTTP 的客户端中使用。以下子句可用于执行全文匹配：

### match

"match" 是一个简单的查询，在指定的字段中匹配指定的关键字。

```json
"query":
{
  "match": { "field": "keyword" }
}
```

您可以指定字段列表：

```json
"match":
{
  "field1,field2": "keyword"
}
```
或者您可以使用 `_all` 或 `*` 来搜索所有字段。

您可以使用 "!field" 搜索除一个字段外的所有字段：

```json
"match":
{
  "!field1": "keyword"
}
```
默认情况下，关键字使用 OR 运算符组合。不过，您可以使用 "operator" 子句更改该行为：

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

"operator" 可以设置为 "or" 或 "and"。

`boost` 修饰符也可以应用。它提高了在其计算中包含 IDF 的排名分数中，单词 [IDF](../../Searching/Options.md#idf)_score 的倍数。它不会以任何方式影响匹配过程。
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

"match_phrase" 是一个匹配整个短语的查询。它类似于 SQL 中的短语运算符。以下是一个示例：

```json
"query":
{
  "match_phrase": { "_all" : "已经增长相当" }
}
```

### query_string
"query_string" 接受输入字符串作为 `MATCH()` 语法中的全文查询。

```json
"query":
{
  "query_string": "教堂 NOTNEAR/3 街道"
}
```

### match_all

"match_all" 接受一个空对象，并返回来自表的文档，而不执行任何属性过滤或全文匹配。或者，您可以在请求中省略 `query` 子句，这将产生相同的效果。

```json
"query":
{
  "match_all": {}
}
```


### 将全文过滤与其他过滤器结合

所有全文匹配子句可以与 [must](../../Searching/Filters.md#must)、[must_not](../../Searching/Filters.md#must_not) 和 [should](../../Searching/Filters.md#should) 运算符结合使用，这些运算符是 [JSON `bool` 查询](../../Searching/Filters.md#bool-query) 的一部分。

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
            "*" : "寻找乔"
        }
    },
    "_source": ["故事作者","评论作者"],
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
               "故事作者" : "IgorPartola",
               "评论作者" : "joe_the_user"
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
            "*" : "寻找乔"
        }
    },
    "_source": ["故事作者","评论作者"],
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
               "故事作者" : "rbanffy",
               "评论作者" : "runjake"
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
        "query_string": "@comment_text "寻找乔 快"/2"
    },
    "_source": ["故事作者","评论作者"],
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
               "故事作者" : "rbanffy",
               "评论作者" : "runjake"
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
$result = $search->('@title 寻找我快');
foreach($result as $doc)
{
    echo '文档: '.$doc->getId();
    foreach($doc->getData() as $field=>$value)
    {
        echo $field.': '.$value;
    }
}
```
<!-- response PHP -->
```php
文档: 1
title: 第一个寻找我快
gid: 11
文档: 2
title: 第二个寻找我快
gid: 12

```

<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.search({"table":"hn_small","query":{"query_string":"@comment_text "寻找乔 快"/2"}, "_source": ["故事作者","评论作者"], "limit":1})
```
<!-- response Python -->
``` python
{'aggregations': None,
 'hits': {'hits': [{'_id': '807160',
                    '_score': 2566,
                    '_source': {'评论作者': 'runjake',
                                '故事作者': 'rbanffy'}}],
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
res = await searchApi.search({"table":"hn_small","query":{"query_string":"@comment_text "寻找乔 快"/2"}, "_source": ["故事作者","评论作者"], "limit":1});
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
query.put("query_string", "@comment_text "寻找乔 快"/2");
searchRequest = new SearchRequest();
searchRequest.setIndex("hn_small");
searchRequest.setQuery(query);
searchRequest.addSourceItem("故事作者");
searchRequest.addSourceItem("评论作者");
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
        hits: [{_id=807160, _score=2566, _source={故事作者=rbanffy, 评论作者=runjake}}]
    }
    profile: null
    warning: null
}
```

<!-- intro -->
C#
<!-- request C# -->

```clike
object query =  new { query_string="@comment_text "寻找乔 快"/2" };
var searchRequest = new SearchRequest("hn_small", query);
searchRequest.Source = new List<string> {"故事作者", "评论作者"};
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
        hits: [{_id=807160, _score=2566, _source={故事作者=rbanffy, 评论作者=runjake}}]
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
  query: { query_string: "测试文档 1" },
  "_source": ["内容", "标题"],
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
          _source: { 内容: '这是测试文档 1', 标题: '文档 1' }
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
          "content": "这是一个测试文档 1",
          "title": "文档 1"
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
# 匹配

`MATCH` 子句允许对文本字段进行全文搜索。输入的查询字符串使用与索引期间应用于文本相同的设置进行 [分词](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md)。除了输入文本的分词外，查询字符串支持多个 [全文操作符](../../Searching/Full_text_matching/Operators.md)，这些操作符强制执行不同的规则，以确保关键字提供有效的匹配。

全文匹配子句可以与属性 [过滤器](../../Searching/Filters.md) 结合使用，作为 AND 布尔。**不支持全文匹配和属性过滤器之间的 OR 关系**。

匹配查询始终在过滤过程中的首位执行，随后是 [属性过滤器](../../Searching/Filters.md)。属性过滤器应用于匹配查询的结果集。没有匹配子句的查询称为全扫描。

在 `SELECT` 子句中最多只能有一个 `MATCH()`。

使用 [全文查询语法](../../Searching/Full_text_matching/Operators.md)，匹配在文档的所有索引文本字段中执行，除非表达式需要在字段内进行匹配（如短语搜索）或受到字段操作符的限制。

使用 [JOIN](../../Searching/Joining.md) 查询时，`MATCH()` 可以接受一个可选的第二个参数，用于指定全文搜索应应用于哪个表。默认情况下，全文查询应用于 `JOIN` 操作中的左表：

```sql
SELECT * FROM table1 LEFT JOIN table2 ON table1.id = table2.id WHERE MATCH('search query', table2);
```

这使您可以在连接操作中对特定表进行全文搜索。有关使用 MATCH 与 JOIN 的更多详细信息，请参见 [连接表](../../Searching/Joining.md) 部分。

## SQL

<!-- example Example_1 -->

```sql
MATCH('search query' [, table_name])
```
- `'search query'`：全文搜索查询字符串，可以包含各种 [全文操作符](../../Searching/Full_text_matching/Operators.md)。
- `table_name`：（可选）要应用全文搜索的表的名称，用于 `JOIN` 查询中指定不同于默认左表的表。


[SELECT](../../Searching/Full_text_matching/Basic_usage.md#SQL) 语句使用了 [MATCH](../../Searching/Full_text_matching/Basic_usage.md) 子句，该子句必须放在 WHERE 之后，以执行全文搜索。`MATCH()` 接受输入字符串，其中所有 [全文操作符](../../Searching/Full_text_matching/Operators.md) 都是可用的。


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

<!-- request MATCH with filters -->
使用 MATCH 结合 WHERE 过滤器的更复杂查询示例。

```sql
SELECT * FROM myindex WHERE MATCH('cats|birds') AND (`title`='some title' AND `id`=123);
```

<!-- end -->

## HTTP JSON

<!-- example Example_11 -->

全文匹配在 `/search` 端点和基于 HTTP 的客户端中可用。以下子句可用于执行全文匹配：

### 匹配

"match" 是一个简单查询，它在指定字段中匹配指定的关键字。

```json
"query":
{
  "match": { "field": "keyword" }
}
```

您可以指定字段列表：

```json
"match":
{
  "field1,field2": "keyword"
}
```
或者您可以使用 `_all` 或 `*` 来搜索所有字段。

您可以使用 "!field" 搜索除一个以外的所有字段：

```json
"match":
{
  "!field1": "keyword"
}
```
默认情况下，关键字通过 OR 操作符组合。然而，您可以使用 "operator" 子句更改该行为：

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

"operator" 可以设置为 "or" 或 "and"。

`boost` 修饰符也可以应用。它通过在结合 IDF 计算评分中提高指定因子的单词 [IDF](../../Searching/Options.md#idf)_score 来提高排名分数。这不会以任何方式影响匹配过程。
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

### 匹配短语

"match_phrase" 是一个匹配整个短语的查询。它类似于 SQL 中的短语操作符。以下是一个示例：

```json
"query":
{
  "match_phrase": { "_all" : "had grown quite" }
}
```

### 查询字符串
"query_string" 接受一个输入字符串作为 `MATCH()` 语法的全文查询。

```json
"query":
{
  "query_string": "Church NOTNEAR/3 street"
}
```

### 匹配所有

"match_all" 接受一个空对象，并从表中返回文档，而不执行任何属性过滤或全文匹配。您也可以在请求中简单地省略 `query` 子句，这将产生相同的效果。

```json
"query":
{
  "match_all": {}
}
```


### 将全文过滤与其他过滤结合
所有全文匹配子句都可以与 [must](../../Searching/Filters.md#must)、[must_not](../../Searching/Filters.md#must_not) 和 [should](../../Searching/Filters.md#should) 操作符结合使用，以形成 [JSON `bool` 查询](../../Searching/Filters.md#bool-query)。

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
        "query_string": "@comment_text "find joe fast "/2"
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
searchApi.search({"table":"hn_small","query":{"query_string":"@comment_text "find joe fast "/2"}, "_source": ["story_author","comment_author"], "limit":1})
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
javascript
<!-- request javascript -->

```javascript
res = await searchApi.search({"table":"hn_small","query":{"query_string":"@comment_text "find joe fast "/2"}, "_source": ["story_author","comment_author"], "limit":1});
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
query.put("query_string", "@comment_text "find joe fast "/2");
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
object query =  new { query_string="@comment_text "find joe fast "/2" };
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
          _source: { content: '这是测试文档1', title: '文档1' }
        }
      ]
   }
}
```

<!-- intro -->
进入
<!-- request Go -->

```go
searchRequest := manticoresearch.NewSearchRequest("test")
query := map[string]interface{} {"query_string": "测试文件 1"}
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
          "content": "这是一份测试文件 1",
          "title": "文件 1"
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


# 匹配

`MATCH` 子句允许在文本字段中进行全文搜索。输入查询字符串被 [标记化](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md)，使用与索引期间应用于文本的相同设置。除了输入文本的标记化外，查询字符串支持许多 [全文操作符](../../Searching/Full_text_matching/Operators.md)，这些操作符强制执行各种规则，规定关键字应如何提供有效匹配。

全文匹配子句可以与属性 [过滤器](../../Searching/Filters.md) 结合使用，作为AND布尔值。**不支持全文匹配和属性过滤器之间的OR关系**。

匹配查询始终在过滤过程中首先执行，然后是 [属性过滤器](../../Searching/Filters.md)。属性过滤器应用于匹配查询的结果集。没有匹配子句的查询称为全扫描。

在 `SELECT` 子句中至多只能有一个 `MATCH()`。

使用 [全文查询语法](../../Searching/Full_text_matching/Operators.md)，匹配在文档的所有索引文本字段中执行，除非表达式需要在某个字段内进行匹配（如短语搜索）或受到字段操作符的限制。

在使用 [JOIN](../../Searching/Joining.md) 查询时，`MATCH()`可以接受一个可选的第二个参数，指定全文搜索应应用于哪个表。默认情况下，全文查询应用于 `JOIN` 操作中的左侧表：

```sql
SELECT * FROM table1 LEFT JOIN table2 ON table1.id = table2.id WHERE MATCH('search query', table2);
```

这允许在联接操作中的特定表上执行全文搜索。有关在JOIN中使用MATCH的更多详细信息，请参阅 [Joining tables](../../Searching/Joining.md) 部分。

## SQL

<!-- example Example_1 -->

```sql
MATCH('search query' [, table_name])
```
- `'search query'`：包含各种 [全文操作符](../../Searching/Full_text_matching/Operators.md) 的全文搜索查询字符串。
- `table_name`：（可选）要应用全文搜索的表名，在 `JOIN` 查询中用于指定不同于默认左表的表。


[SELECT](../../Searching/Full_text_matching/Basic_usage.md#SQL) 语句使用 [MATCH](../../Searching/Full_text_matching/Basic_usage.md) 子句，必须在 WHERE 之后，用于执行全文搜索。 `MATCH()` 接受包含所有 [全文操作符](../../Searching/Full_text_matching/Operators.md) 的输入字符串。


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

<!-- request MATCH with filters -->
这是一个使用MATCH和WHERE过滤器的更复杂查询的示例。

```sql
SELECT * FROM myindex WHERE MATCH('cats|birds') AND (`title`='some title' AND `id`=123);
```

<!-- end -->

## HTTP JSON

<!-- example Example_11 -->

全文匹配在 `/search` 端点和基于HTTP的客户端中可用。可以使用以下子句执行全文匹配：

### match

"match" 是一个简单的查询，用于在指定字段中匹配指定的关键字。

```json
"query":
{
  "match": { "field": "keyword" }
}
```

您可以指定字段的列表：

```json
"match":
{
  "field1,field2": "keyword"
}
```
或者您可以使用 `_all` 或 `*` 来搜索所有字段。

您可以使用 "!field" 搜索除一个字段之外的所有字段：

```json
"match":
{
  "!field1": "keyword"
}
```
默认情况下，关键字使用 OR 操作符组合。但是，您可以使用 "operator" 子句更改该行为：

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

"operator" 可以设置为 "or" 或 "and"。

`boost` 修饰符也可以应用。它通过在计算其排名分数时结合IDF，按指示的因子提升单词 [IDF](../../Searching/Options.md#idf)_score。它不会以任何方式影响匹配过程。
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

"match_phrase" 是一个匹配整个短语的查询。它类似于SQL中的短语操作符。以下是一个示例：

```json
"query":
{
  "match_phrase": { "_all" : "had grown quite" }
}
```

### query_string
"query_string" 接受一个输入字符串作为 `MATCH()` 语法中的全文查询。

```json
"query":
{
  "query_string": "Church NOTNEAR/3 street"
}
```

### match_all

"match_all" 接受一个空对象并返回表中的文档，而不执行任何属性过滤或全文匹配。或者，您可以在请求中省略 `query` 子句，这样也会产生相同的效果。

```json
"query":
{
  "match_all": {}
}
```


### 将全文过滤与其他过滤结合

所有全文匹配子句可以与 [must](../../Searching/Filters.md#must)、[must_not](../../Searching/Filters.md#must_not) 和 [should](../../Searching/Filters.md#should) 操作符结合使用，以构成 [JSON `bool` 查询](../../Searching/Filters.md#bool-query)。

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
          _source: { content: '这是测试文档1', title: '文档1' }
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
          "content": "这是一个测试文档 1",
          "title": "文档 1"
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
