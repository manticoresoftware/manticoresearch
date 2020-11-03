# MATCH
 
The `MATCH` clause allows to perform  full-text searches in text fields. The query string at input is [tokenized](Creating_an_index/NLP_and_tokenization/Data_tokenization.md) using same settings applied to the text during indexing. In addition to tokenization of input text, the query string supports a number of [full-text operators](Searching/Full_text_matching/Operators.md) that allow enforcing different rules on how keywords should provide a valid match.

The full-text match clauses can be combined with attribute [filters](Searching/Filters.md) as an AND boolean. **OR relation between full-text matches and attribute filters are not supported**.

The match query is always executed first in the filtering process, followed by the [attribute filters](Searching/Filters.md). The attribute filters are applied on the result set of the match query. A query without a match clause is called a fullscan.

There must be at most one `MATCH()` in the `SELECT` clause.

Using the [full-text query syntax](Searching/Full_text_matching/Operators.md) matching  is performed over all indexed text fields of a document, unless the expression requires to be match within a field (like phrase search) or limited by field operators.

## SQL
<!-- example Example_1 -->

```sql
SELECT * FROM index WHERE MATCH('cats|birds');
```

[SELECT](Searching/Full_text_matching/Basic_usage.md#SQL) statement uses [MATCH](Searching/Full_text_matching/Basic_usage.md) clause for performing full-text searches. It accepts an input string in which all [full-text operators](Searching/Full_text_matching/Operators.md) are available.


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

## HTTP

<!-- example Example_11 -->

Full-text matching is available in the `/search` endpoint. The following clauses can be used for performing full-text matches:

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

All full-text match clauses can be combined with [must](Searching/Filters.md#must), [must_not](Searching/Filters.md#must_not) and [should](Searching/Filters.md#should) operators of an [HTTP `bool` query](Searching/Filters.md#bool-query).

<!-- intro -->
Examples:

<!-- request match -->

```json
POST /search 
-d
'{   "index" : "myindex",
    "query":
    {
        "match":
        {
            "*" : "find me"
        }
    }
}'
```

<!-- response match -->
```json
{
  "took":10,
  "timed_out": false,
  "hits":
  {
    "total": 2,
    "hits":
    [
      {
        "_id": "1",
        "_score": 1,
        "_source": {"title":"first find me fast", "gid": 11 }
      },
      {
        "_id": "2",
        "_score": 1,
        "_source": { "title":"second find me fast", "gid": 12 }
      }
    ]
  }
}
```
<!-- request match_phrase -->
```json
POST /search 
-d
'{   "index" : "myindex",
    "query":
    {
        "match_phrase":
        {
            "*" : "find me"
        }
    }
}'
```
<!-- response match_phrase -->
```json
{
  "took":10,
  "timed_out": false,
  "hits":
  {
    "total": 2,
    "hits":
    [
      {
        "_id": "1",
        "_score": 1,
        "_source": {"title":"first find me fast", "gid": 11 }
      },
      {
        "_id": "2",
        "_score": 1,
        "_source": { "title":"second find me fast", "gid": 12 }
      }
    ]
  }
}
```

<!-- request query_string -->
```json
POST /search 
-d
'{   "index" : "myindex",
    "query":
    {
        "query_string": "@title \"find me fast \"/2"
    }
}'
```
<!-- response query_string -->
```json
{
  "took":10,
  "timed_out": false,
  "hits":
  {
    "total": 2,
    "hits":
    [
      {
        "_id": "1",
        "_score": 1,
        "_source": {"title":"first find me fast", "gid": 11 }
      },
      {
        "_id": "2",
        "_score": 1,
        "_source": { "title":"second find me fast", "gid": 12 }
      }
    ]
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
searchApi.search({"index":"myindex","query":{"query_string":"@title \"find me fast \"/2"}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'1',
                    u'_score': 1,
                    u'_source': {u'title': u'first find me fast', u'gid':11}},
                    {u'_id': u'2',
                    u'_score': 1,
                    u'_source': {u'title': u'second find me fast', u'gid':12}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.search({"index":"myindex","query":{"query_string":"@title \"find me fast \"/2"}});
```
<!-- response javascript -->
```javascript
{"hits": {"hits": [{"_id": "1",
                    "_score": 1,
                    "_source": {"title": "first find me fast", "gid":11}},
                    {"_id": "2",
                    "_score": 1,
                    "_source": {"title": "second find me fast", "gid":12}}],
          "total": 2},
 "profile": None,
 "timed_out": False,
 "took": 0}
```
<!-- end -->
