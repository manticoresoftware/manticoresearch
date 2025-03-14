# MATCH

`MATCH` 子句允许在文本字段中进行全文搜索。输入的查询字符串使用与索引时相同的设置进行[分词](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md)。除了对输入文本进行分词外，查询字符串还支持许多[全文操作符](../../Searching/Full_text_matching/Operators.md)，这些操作符对关键词的匹配方式施加了各种规则。

全文匹配子句可以与属性[过滤器](../../Searching/Filters.md)结合使用，作为布尔值 AND 关系。**不支持全文匹配与属性过滤器之间的 OR 关系**。

在过滤过程中，匹配查询始终首先执行，然后应用[属性过滤器](../../Searching/Filters.md)。属性过滤器应用于匹配查询的结果集。没有匹配子句的查询称为全扫描查询。

在 `SELECT` 子句中最多只能有一个 `MATCH()`。

使用[全文查询语法](../../Searching/Full_text_matching/Operators.md)时，匹配会在文档的所有已索引文本字段中执行，除非表达式要求在字段内匹配（如短语搜索）或被字段操作符限制。

## SQL
<!-- example Example_1 -->

```sql
SELECT * FROM myindex WHERE MATCH('cats|birds');
```

SELECT 语句使用 [MATCH](../../Searching/Full_text_matching/Basic_usage.md) 子句来执行全文搜索，该子句必须位于 WHERE 之后。MATCH() 接受一个输入字符串，所有的 [全文操作符](../../Searching/Full_text_matching/Operators.md) 都可用。

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
一个使用 MATCH 与 WHERE 过滤器的更复杂查询示例。

```sql
SELECT * FROM myindex WHERE MATCH('cats|birds') AND (`title`='some title' AND `id`=123);
```

<!-- end -->

## HTTP JSON

<!-- example Example_11 -->

全文匹配可以在 /search 端点和基于 HTTP 的客户端中使用。以下子句可用于执行全文匹配：

### match

“match” 是一个简单的查询，用于在指定字段中匹配指定的关键字。

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
或者您可以使用 `_all` 或 `*` 搜索所有字段。



您还可以通过使用 !field 排除某个字段来进行搜索：

```json
"match":
{
  "!field1": "keyword"
}
```
默认情况下，关键字通过 OR 操作符组合。但是，您可以使用 operator 子句更改此行为：

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

operator 可以设置为 “or” 或 “and”。

还可以应用 boost 修饰符。它通过提高在排名分数中包含 IDF 计算的词语 [IDF](../../Searching/Options.md#idf)_score 来提升得分。这不会影响匹配过程。

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

"match_phrase"是一个查询，它匹配整个短语。它类似于 SQL 中的短语操作符。以下是一个示例：

```json
"query":
{
  "match_phrase": { "_all" : "had grown quite" }
}
```

### query_string
"query_string" 接受一个输入字符串作为 `MATCH()` 语法的全文查询。

```json
"query":
{
  "query_string": "Church NOTNEAR/3 street"
}
```

### match_all

"match_all" 接受一个空对象，并从表中返回文档，而不进行任何属性过滤或全文匹配。或者，您可以在请求中省略 `query` 子句，这将产生相同的效果。

```json
"query":
{
  "match_all": {}
}
```


### 将全文过滤与其他过滤器组合

所有全文匹配子句都可以与 [must](../../Searching/Filters.md#must)，[must_not](../../Searching/Filters.md#must_not) 和 [should](../../Searching/Filters.md#should) 操作符组合在 [JSON bool 查询](../../Searching/Filters.md#bool-查询) 中。

<!-- intro -->
示例：

<!-- request match -->

```json
POST /search
-d
'{
    "index" : "hn_small",
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
    "index" : "hn_small",
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
'{   "index" : "hn_small",
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
searchApi.search({"index":"hn_small","query":{"query_string":"@comment_text \"find joe fast \"/2"}, "_source": ["story_author","comment_author"], "limit":1})
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
res = await searchApi.search({"index":"hn_small","query":{"query_string":"@comment_text \"find joe fast \"/2"}, "_source": ["story_author","comment_author"], "limit":1});
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
