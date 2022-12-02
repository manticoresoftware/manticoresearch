# MATCH

The `MATCH` clause allows to perform  full-text searches in text fields. The query string at input is [tokenized](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md) using same settings applied to the text during indexing. In addition to tokenization of input text, the query string supports a number of [full-text operators](../../Searching/Full_text_matching/Operators.md) that allow enforcing different rules on how keywords should provide a valid match.

The full-text match clauses can be combined with attribute [filters](../../Searching/Filters.md) as an AND boolean. **OR relation between full-text matches and attribute filters are not supported**.

The match query is always executed first in the filtering process, followed by the [attribute filters](../../Searching/Filters.md). The attribute filters are applied on the result set of the match query. A query without a match clause is called a fullscan.

There must be at most one `MATCH()` in the `SELECT` clause.

Using the [full-text query syntax](../../Searching/Full_text_matching/Operators.md) matching  is performed over all indexed text fields of a document, unless the expression requires to be match within a field (like phrase search) or limited by field operators.

## SQL
<!-- example Example_1 -->

```sql
SELECT * FROM index WHERE MATCH('cats|birds');
```

[SELECT](../../Searching/Full_text_matching/Basic_usage.md#SQL) statement uses [MATCH](../../Searching/Full_text_matching/Basic_usage.md) clause for performing full-text searches. It accepts an input string in which all [full-text operators](../../Searching/Full_text_matching/Operators.md) are available.


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

<!-- end -->

## HTTP JSON

<!-- example Example_11 -->

Full-text matching is available in the `/search` endpoint and in HTTP-based clients. The following clauses can be used for performing full-text matches:

### match

"match" is a simple query that matches the specified keywords in the specified fields

```json
"query":
{
  "match": { "field": "keyword" }
}
```

You can specify a list of fields:

```json
"match":
{
  "field1,field2": "keyword"
}
```
Or you can use `_all` or `*` to search all fields.

You can search all fields except one using "!field":

```json
"match":
{
  "!field1": "keyword"
}
```
By default keywords are combined using the OR operator. However, you can change that behaviour using the "operator" clause:

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

"operator" can be set to "or" or "and".

### match_phrase

"match_phrase" is a query that matches the entire phrase. It is similar to a phrase operator in SQL. Here's an example:

```json
"query":
{
  "match_phrase": { "_all" : "had grown quite" }
}
```

### query_string
"query_string" accepts an input string as a full-text query in `MATCH()` syntax

```json
"query":
{
  "query_string": "Church NOTNEAR/3 street"
}
```


### Combining full-text filtering with other filters

All full-text match clauses can be combined with [must](../../Searching/Filters.md#must), [must_not](../../Searching/Filters.md#must_not) and [should](../../Searching/Filters.md#should) operators of an [HTTP `bool` query](../../Searching/Filters.md#bool-query).

<!-- intro -->
Examples:

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
            "_id" : "668018",
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
            "_id" : "807160",
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
            "_id" : "807160",
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
<!-- end -->
