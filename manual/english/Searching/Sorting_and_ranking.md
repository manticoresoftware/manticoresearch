# Sorting and ranking

Query results can be sorted by full-text ranking weight, one or more attributes or expressions.

**Full-text** queries return matches sorted by default. If nothing is specified, they are sorted by relevance, which is equivalent to `ORDER BY weight() DESC` in SQL format.

**Non-full-text** queries do not perform any sorting by default.

## Advanced sorting

Extended mode is automatically enabled when you explicitly provide sorting rules by adding the `ORDER BY` clause in SQL format or using the `sort` option via HTTP JSON.

### Sorting via SQL

General syntax:
```sql
SELECT ... ORDER BY
{attribute_name | expr_alias | weight() | random() } [ASC | DESC],
...
{attribute_name | expr_alias | weight() | random() } [ASC | DESC]
```

<!-- example alias -->

In the sort clause, you can use any combination of up to 5 columns, each followed by `asc` or `desc`. Functions and expressions are not allowed as arguments for the sort clause, except for the `weight()` and `random()` functions (the latter can only be used via SQL in the form of `ORDER BY random()`). However, you can use any expression in the SELECT list and sort by its alias.

<!-- request SQL -->
```sql
select *, a + b alias from test order by alias desc;
```

<!-- response SQL -->
```
+------+------+------+----------+-------+
| id   | a    | b    | f        | alias |
+------+------+------+----------+-------+
|    1 |    2 |    3 | document |     5 |
+------+------+------+----------+-------+
```

<!-- end -->

## Sorting via JSON

<!-- example sorting 1 -->
`"sort"` specifies an array where each element can be an attribute name or `_score` if you want to sort by match weights. In that case, the sort order defaults to ascending for attributes and descending for `_score`.

<!-- intro -->

<!-- request JSON -->

```json
{
  "table":"test",
  "query":
  {
    "match": { "title": "Test document" }
  },
  "sort": [ "_score", "id" ],
  "_source": "title",
  "limit": 3
}
```

<!-- response JSON -->

``` json
    {
	  "took": 0,
	  "timed_out": false,
	  "hits": {
	    "total": 5,
	    "total_relation": "eq",
	    "hits": [
	      {
	        "_id": 5406864699109146628,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 1"
	        }
	      },
	      {
	        "_id": 5406864699109146629,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 2"
	        }
	      },
	      {
	        "_id": 5406864699109146630,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 3"
	        }
	      }
	    ]
	  }
	}
```    

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$search->setIndex("test")->match('Test document')->sort('_score')->sort('id');
```

<!-- intro -->

##### Python:

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
search_request.sort = ['_score', 'id']
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Test document');
searchRequest.sort = ['_score', 'id'];
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
searchRequest.setIndex("test");
QueryFilter queryFilter = new QueryFilter();
queryFilter.setQueryString("Test document");
searchRequest.setFulltextFilter(queryFilter);
List<Object> sort = new ArrayList<Object>( Arrays.asList("_score", "id") );
searchRequest.setSort(sort);

```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
var searchRequest = new SearchRequest("test");
searchRequest.FulltextFilter = new QueryFilter("Test document");
searchRequest.Sort = new List<Object> {"_score", "id"};

```

<!-- intro -->

##### Typescript:

<!-- request typescript -->
``` typescript
searchRequest = {
  index: 'test',
  query: {
    query_string: {'Test document'},
  },
  sort: ['_score', 'id'],
}
```

<!-- intro -->

##### Go:

<!-- request go -->
```go
searchRequest.SetIndex("test")
query := map[string]interface{} {"query_string": "Test document"}
searchRequest.SetQuery(query)
sort := map[string]interface{} {"_score": "asc", "id": "asc"}
searchRequest.SetSort(sort)
```

<!-- end -->

<!-- example sorting 2 -->
You can also specify the sort order explicitly:

* `asc`: sort in ascending order
* `desc`: sort in descending order


<!-- intro -->

<!-- request JSON -->

```json
{
  "table":"test",
  "query":
  {
    "match": { "title": "Test document" }
  },
  "sort":
  [
    { "id": "desc" },
    "_score"
  ],
  "_source": "title",
  "limit": 3
}
```

<!-- response JSON -->

``` json
	{
	  "took": 0,
	  "timed_out": false,
	  "hits": {
	    "total": 5,
	    "total_relation": "eq",
	    "hits": [
	      {
	        "_id": 5406864699109146632,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 5"
	        }
	      },
	      {
	        "_id": 5406864699109146631,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 4"
	        }
	      },
	      {
	        "_id": 5406864699109146630,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 3"
	        }
	      }
	    ]
	  }
	}
```    

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$search->setIndex("test")->match('Test document')->sort('id', 'desc')->sort('_score');
```

<!-- intro -->

##### Python:

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
sort_by_id = manticoresearch.model.SortOrder('id', 'desc')
search_request.sort = [sort_by_id, '_score']
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Test document');
sortById = new Manticoresearch.SortOrder('id', 'desc');
searchRequest.sort = [sortById, 'id'];
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
searchRequest.setIndex("test");
QueryFilter queryFilter = new QueryFilter();
queryFilter.setQueryString("Test document");
searchRequest.setFulltextFilter(queryFilter);
List<Object> sort = new ArrayList<Object>();
SortOrder sortById = new SortOrder();
sortById.setAttr("id");
sortById.setOrder(SortOrder.OrderEnum.DESC);
sort.add(sortById);
sort.add("_score");
searchRequest.setSort(sort);

```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
var searchRequest = new SearchRequest("test");
searchRequest.FulltextFilter = new QueryFilter("Test document");
searchRequest.Sort = new List<Object>();
var sortById = new SortOrder("id", SortOrder.OrderEnum.Desc);
searchRequest.Sort.Add(sortById);
searchRequest.Sort.Add("_score");

```

<!-- intro -->

##### Typescript:

<!-- request typescript -->
``` typescript
searchRequest = {
  index: 'test',
  query: {
    query_string: {'Test document'},
  },
  sort: [{'id': 'desc'}, '_score'],
}
```

<!-- intro -->

##### Go:

<!-- request go -->
```go
searchRequest.SetIndex("test")
query := map[string]interface{} {"query_string": "Test document"}
searchRequest.SetQuery(query)
sortById := map[string]interface{} {"id": "desc"}
sort := map[string]interface{} {"id": "desc", "_score": "asc"}
searchRequest.SetSort(sort)
```

<!-- end -->

<!-- example sorting 3 -->
You can also use another syntax and specify the sort order via the `order` property:


<!-- intro -->

<!-- request JSON -->

```json
{
  "table":"test",
  "query":
  {
    "match": { "title": "Test document" }
  },
  "sort":
  [
    { "id": { "order":"desc" } }
  ],
  "_source": "title",
  "limit": 3
}
```

<!-- response JSON -->

``` json
	{
	  "took": 0,
	  "timed_out": false,
	  "hits": {
	    "total": 5,
	    "total_relation": "eq",
	    "hits": [
	      {
	        "_id": 5406864699109146632,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 5"
	        }
	      },
	      {
	        "_id": 5406864699109146631,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 4"
	        }
	      },
	      {
	        "_id": 5406864699109146630,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 3"
	        }
	      }
	    ]
	  }
	}
```    

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$search->setIndex("test")->match('Test document')->sort('id', 'desc');
```

<!-- intro -->

##### Python:

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
sort_by_id = manticoresearch.model.SortOrder('id', 'desc')
search_request.sort = [sort_by_id]
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Test document');
sortById = new Manticoresearch.SortOrder('id', 'desc');
searchRequest.sort = [sortById];
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
searchRequest.setIndex("test");
QueryFilter queryFilter = new QueryFilter();
queryFilter.setQueryString("Test document");
searchRequest.setFulltextFilter(queryFilter);
List<Object> sort = new ArrayList<Object>();
SortOrder sortById = new SortOrder();
sortById.setAttr("id");
sortById.setOrder(SortOrder.OrderEnum.DESC);
sort.add(sortById);
searchRequest.setSort(sort);

```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
var searchRequest = new SearchRequest("test");
searchRequest.FulltextFilter = new QueryFilter("Test document");
searchRequest.Sort = new List<Object>();
var sortById = new SortOrder("id", SortOrder.OrderEnum.Desc);
searchRequest.Sort.Add(sortById);

```

<!-- intro -->

##### Typescript:

<!-- request typescript -->
``` typescript
searchRequest = {
  index: 'test',
  query: {
    query_string: {'Test document'},
  },
  sort: { {'id': {'order':'desc'} },
}
```

<!-- intro -->

##### Go:

<!-- request go -->
```go
searchRequest.SetIndex("test")
query := map[string]interface{} {"query_string": "Test document"}
searchRequest.SetQuery(query)
sort := map[string]interface{} { "id": {"order":"desc"} }
searchRequest.SetSort(sort)
```

<!-- end -->

<!-- example sorting 4 -->
Sorting by MVA attributes is also supported in JSON queries. Sorting mode can be set via the `mode` property. The following modes are supported:

* `min`: sort by minimum value
* `max`: sort by maximum value

<!-- intro -->

<!-- request JSON -->

```json
{
  "table":"test",
  "query":
  {
    "match": { "title": "Test document" }
  },
  "sort":
  [
    { "attr_mva": { "order":"desc", "mode":"max" } }
  ],
  "_source": "title",
  "limit": 3
}
```

<!-- response JSON -->

``` json
	{
	  "took": 0,
	  "timed_out": false,
	  "hits": {
	    "total": 5,
	    "total_relation": "eq",
	    "hits": [
	      {
	        "_id": 5406864699109146631,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 4"
	        }
	      },
	      {
	        "_id": 5406864699109146629,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 2"
	        }
	      },
	      {
	        "_id": 5406864699109146628,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 1"
	        }
	      }
	    ]
	  }
	}
```    

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$search->setIndex("test")->match('Test document')->sort('id','desc','max');
```

<!-- intro -->

##### Python:

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
sort = manticoresearch.model.SortMVA('attr_mva', 'desc', 'max')
search_request.sort = [sort]
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Test document');
sort = new Manticoresearch.SortMVA('attr_mva', 'desc', 'max');
searchRequest.sort = [sort];
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
searchRequest.setIndex("test");
QueryFilter queryFilter = new QueryFilter();
queryFilter.setQueryString("Test document");
searchRequest.setFulltextFilter(queryFilter);
SortMVA sort = new SortMVA();
sort.setAttr("attr_mva");
sort.setOrder(SortMVA.OrderEnum.DESC);
sort.setMode(SortMVA.ModeEnum.MAX);
searchRequest.setSort(sort);

```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
var searchRequest = new SearchRequest("test");
searchRequest.FulltextFilter = new QueryFilter("Test document");
var sort = new SortMVA("attr_mva", SortMVA.OrderEnum.Desc, SortMVA.ModeEnum.Max);
searchRequest.Sort.Add(sort);
```

<!-- intro -->

##### Typescript:

<!-- request typescript -->
``` typescript
searchRequest = {
  index: 'test',
  query: {
    query_string: {'Test document'},
  },
  sort: { "attr_mva": { "order":"desc", "mode":"max" } },
}
```

<!-- intro -->

##### Go:

<!-- request go -->
```go
searchRequest.SetIndex("test")
query := map[string]interface{} {"query_string": "Test document"}
searchRequest.SetQuery(query)
sort := map[string]interface{} { "attr_mva": { "order":"desc", "mode":"max" } }
searchRequest.SetSort(sort)
```

<!-- end -->

<!-- example sorting 5 -->
When sorting on an attribute, match weight (score) calculation is disabled by default (no ranker is used). You can enable weight calculation by setting the `track_scores` property to `true`:

<!-- intro -->

<!-- request JSON -->

```json
{
  "table":"test",
  "track_scores": true,
  "query":
  {
    "match": { "title": "Test document" }
  },
  "sort":
  [
    { "attr_mva": { "order":"desc", "mode":"max" } }
  ],
  "_source": "title",
  "limit": 3
}
```

<!-- response JSON -->

``` json
	{
	  "took": 0,
	  "timed_out": false,
	  "hits": {
	    "total": 5,
	    "total_relation": "eq",
	    "hits": [
	      {
	        "_id": 5406864699109146631,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 4"
	        }
	      },
	      {
	        "_id": 5406864699109146629,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 2"
	        }
	      },
	      {
	        "_id": 5406864699109146628,
	        "_score": 2319,
	        "_source": {
	          "title": "Test document 1"
	        }
	      }
	    ]
	  }
	}
```    

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$search->setIndex("test")->match('Test document')->sort('id','desc','max')->trackScores(true);
```

<!-- intro -->

##### Python:

<!-- request Python -->
``` python
search_request.index = 'test'
search_request.track_scores = true
search_request.fulltext_filter = manticoresearch.model.QueryFilter('Test document')
sort = manticoresearch.model.SortMVA('attr_mva', 'desc', 'max')
search_request.sort = [sort]
```

<!-- intro -->

##### Javascript:

<!-- request javascript -->
``` javascript
searchRequest.index = "test";
searchRequest.trackScores = true;
searchRequest.fulltext_filter = new Manticoresearch.QueryFilter('Test document');
sort = new Manticoresearch.SortMVA('attr_mva', 'desc', 'max');
searchRequest.sort = [sort];
```

<!-- intro -->

##### java:

<!-- request Java -->
``` java
searchRequest.setIndex("test");
searchRequest.setTrackScores(true);
QueryFilter queryFilter = new QueryFilter();
queryFilter.setQueryString("Test document");
searchRequest.setFulltextFilter(queryFilter);
SortMVA sort = new SortMVA();
sort.setAttr("attr_mva");
sort.setOrder(SortMVA.OrderEnum.DESC);
sort.setMode(SortMVA.ModeEnum.MAX);
searchRequest.setSort(sort);

```

<!-- intro -->

##### C#:

<!-- request C# -->
``` clike
var searchRequest = new SearchRequest("test");
searchRequest.SetTrackScores(true);
searchRequest.FulltextFilter = new QueryFilter("Test document");
var sort = new SortMVA("attr_mva", SortMVA.OrderEnum.Desc, SortMVA.ModeEnum.Max);
searchRequest.Sort.Add(sort);
```

<!-- intro -->

##### Typescript:

<!-- request typescript -->
``` typescript
searchRequest = {
  index: 'test',
  track_scores: true,
  query: {
    query_string: {'Test document'},
  },
  sort: { "attr_mva": { "order":"desc", "mode":"max" } },
}
```

<!-- intro -->

##### Go:

<!-- request go -->
```go
searchRequest.SetIndex("test")
searchRequest.SetTrackScores(true)
query := map[string]interface{} {"query_string": "Test document"}
searchRequest.SetQuery(query)
sort := map[string]interface{} { "attr_mva": { "order":"desc", "mode":"max" } }
searchRequest.SetSort(sort)
```

<!-- end -->


## Ranking overview

Ranking (also known as weighting) of search results can be defined as a process of computing a so-called relevance (weight) for every given matched document regards to a given query that matched it. So relevance is, in the end, just a number attached to every document that estimates how relevant the document is to the query. Search results can then be sorted based on this number and/or some additional parameters, so that the most sought-after results would appear higher on the results page.

There is no single standard one-size-fits-all way to rank any document in any scenario. Moreover, there can never be such a way, because relevance is *subjective*. As in, what seems relevant to you might not seem relevant to me. Hence, in general cases, it's not just hard to compute; it's theoretically impossible.

So ranking in Manticore is configurable. It has a notion of a so-called **ranker**. A ranker can formally be defined as a function that takes a document and a query as its input and produces a relevance value as output. In layman's terms, a ranker controls exactly how (using which specific algorithm) Manticore will assign weights to the documents.

## Available built-in rankers

Manticore ships with several built-in rankers suited for different purposes. Many of them use two factors: phrase proximity (also known as LCS) and BM25. Phrase proximity works on keyword positions, while BM25 works on keyword frequencies. Essentially, the better the degree of phrase match between the document body and the query, the higher the phrase proximity (it maxes out when the document contains the entire query as a verbatim quote). And BM25 is higher when the document contains more rare words. We'll save the detailed discussion for later.

The currently implemented rankers are:

* `proximity_bm25`, the default ranking mode that uses and combines both phrase proximity and BM25 ranking.
* `bm25`, a statistical ranking mode that uses BM25 ranking only (similar to most other full-text engines). This mode is faster but may result in worse quality for queries containing more than one keyword.
* `none`, a no-ranking mode. This mode is obviously the fastest. A weight of 1 is assigned to all matches. This is sometimes called boolean searching, which just matches the documents but does not rank them.
* `wordcount`, ranking by the keyword occurrences count. This ranker computes the per-field keyword occurrence counts, then multiplies them by field weights, and sums the resulting values.
* `proximity` returns the raw phrase proximity value as a result. This mode is internally used to emulate `SPH_MATCH_ALL` queries.
* `matchany` returns rank as it was computed in `SPH_MATCH_ANY` mode earlier and is internally used to emulate `SPH_MATCH_ANY` queries.
* `fieldmask` returns a 32-bit mask with the N-th bit corresponding to the N-th full-text field, numbering from 0. The bit will only be set when the respective field has any keyword occurrences satisfying the query.
* `sph04` is generally based on the default 'proximity_bm25' ranker, but additionally boosts matches when they occur at the very beginning or the very end of a text field. Thus, if a field equals the exact query, `sph04` should rank it higher than a field that contains the exact query but is not equal to it. (For instance, when the query is "Hyde Park", a document titled "Hyde Park" should be ranked higher than one titled "Hyde Park, London" or "The Hyde Park Cafe".)
* `expr` allows you to specify the ranking formula at runtime. It exposes several internal text factors and lets you define how the final weight should be computed from those factors. You can find more details about its syntax and a reference of available factors in a [subsection below](../Searching/Sorting_and_ranking.md#Quick-summary-of-the-ranking-factors).

The ranker name is case-insensitive. Example:

```sql
SELECT ... OPTION ranker=sph04;
```

## Quick summary of the ranking factors

| Name                    | Level     | Type  | Summary                                                                                                            |
| ----------------------- | --------- | ----- | ------------------------------------------------------------------------------------------------------------------ |
| max_lcs                 | query     | int   | maximum possible LCS value for the current query                                                                   |
| bm25                    | document  | int   | quick estimate of BM25(1.2, 0)                                                                                     |
| bm25a(k1, b)            | document  | int   | precise BM25() value with configurable K1, B constants and syntax support                                         |
| bm25f(k1, b, {field=weight, ...}) | document | int   | precise BM25F() value with extra configurable field weights                                                       |
| field_mask              | document  | int   | bit mask of matched fields                                                                                         |
| query_word_count        | document  | int   | number of unique inclusive keywords in a query                                                                     |
| doc_word_count          | document  | int   | number of unique keywords matched in the document                                                                  |
| lcs                     | field     | int   | Longest Common Subsequence between query and document, in words                                                    |
| user_weight             | field     | int   | user field weight                                                                                                  |
| hit_count               | field     | int   | total number of keyword occurrences                                                                                |
| word_count              | field     | int   | number of unique matched keywords                                                                                  |
| tf_idf                  | field     | float | sum(tf*idf) over matched keywords == sum(idf) over occurrences                                                     |
| min_hit_pos             | field     | int   | first matched occurrence position, in words, 1-based                                                               |
| min_best_span_pos       | field     | int   | first maximum LCS span position, in words, 1-based                                                                 |
| exact_hit               | field     | bool  | whether query == field                                                                                             |
| min_idf                 | field     | float | min(idf) over matched keywords                                                                                     |
| max_idf                 | field     | float | max(idf) over matched keywords                                                                                     |
| sum_idf                 | field     | float | sum(idf) over matched keywords                                                                                     |
| exact_order             | field     | bool  | whether all query keywords were a) matched and b) in query order                                                   |
| min_gaps                | field     | int   | minimum number of gaps between the matched keywords over the matching spans                                        |
| lccs                    | field     | int   | Longest Common Contiguous Subsequence between query and document, in words                                         |
| wlccs                   | field     | float | Weighted Longest Common Contiguous Subsequence, sum(idf) over contiguous keyword spans                            |
| atc                     | field     | float | Aggregate Term Closeness, log(1+sum(idf1*idf2*pow(distance, -1.75)) over the best pairs of keywords               |

### Document-level Ranking Factors

A **document-level factor** is a numeric value computed by the ranking engine for every matched document with regards to the current query. So it differs from a plain document attribute in that the attribute does not depend on the full text query, while factors might. These factors can be used anywhere in the ranking expression. Currently implemented document-level factors are:

* `bm25` (integer), a document-level BM25 estimate (computed without keyword occurrence filtering).
* `max_lcs` (integer), a query-level maximum possible value that the `sum(lcs*user_weight)` expression can ever take. This can be useful for weight boost scaling. For instance, `MATCHANY` ranker formula uses this to guarantee that a full phrase match in any field ranks higher than any combination of partial matches in all fields.
* `field_mask` (integer), a document-level 32-bit mask of matched fields.
* `query_word_count` (integer), the number of unique keywords in a query, adjusted for the number of excluded keywords. For instance, both `(one one one one)` and `(one !two)` queries should assign a value of 1 to this factor, because there is just one unique non-excluded keyword.
* `doc_word_count` (integer), the number of unique keywords matched in the entire document.

### Field-level Ranking Factors

A **field-level factor** is a numeric value computed by the ranking engine for every matched in-document text field regards to the current query. As more than one field can be matched by a query, but the final weight needs to be a single integer value, these values need to be folded into a single one. To achieve that, field-level factors can only be used within a field aggregation function, they can **not** be used anywhere in the expression. For example, you cannot use `(lcs+bm25)` as your ranking expression, as `lcs` takes multiple values (one in every matched field). You should use `(sum(lcs)+bm25)` instead, that expression sums `lcs` over all matching fields, and then adds `bm25` to that per-field sum. Currently implemented field-level factors are:

* `lcs` (integer), the length of a maximum verbatim match between the document and the query, counted in words. LCS stands for Longest Common Subsequence (or Subset). Takes a minimum value of 1 when only stray keywords were matched in a field, and a maximum value of query keywords count when the entire query was matched in a field verbatim (in the exact query keywords order). For example, if the query is 'hello world' and the field contains these two words quoted from the query (that is, adjacent to each other, and exactly in the query order), `lcs` will be 2. For example, if the query is 'hello world program' and the field contains 'hello world', `lcs` will be 2. Note that any subset of the query keyword works, not just a subset of adjacent keywords. For example, if the query is 'hello world program' and the field contains 'hello (test program)', `lcs` will be 2 just as well, because both 'hello' and 'program' matched in the same respective positions as they were in the query. Finally, if the query is 'hello world program' and the field contains 'hello world program', `lcs` will be 3. (Hopefully that is unsurprising at this point.)
* `user_weight` (integer), the user specified per-field weight (refer to [OPTION field_weights](../Searching/Options.md#field_weights) in SQL). The weights default to 1 if not specified explicitly.
* `hit_count` (integer), the number of keyword occurrences that matched in the field. Note that a single keyword may occur multiple times. For example, if 'hello' occurs 3 times in a field and 'world' occurs 5 times, `hit_count` will be 8.
* `word_count` (integer), the number of unique keywords matched in the field. For example, if 'hello' and 'world' occur anywhere in a field, `word_count` will be 2, regardless of how many times both keywords occur.
* `tf_idf` (float), the sum of TF/IDF over all the keywords matched in the field. IDF is the Inverse Document Frequency, a floating point value between 0 and 1 that describes how frequent the keyword is (basically, 0 for a keyword that occurs in every document indexed, and 1 for a unique keyword that occurs in just a single document). TF is the Term Frequency, the number of matched keyword occurrences in the field. As a side note, `tf_idf` is actually computed by summing IDF over all matched occurrences. That's by construction equivalent to summing TF*IDF over all matched keywords.
* `min_hit_pos` (integer), the position of the first matched keyword occurrence, counted in words

   Therefore, this is a relatively low-level, "raw" factor that you'll likely want to *adjust* before using it for ranking. The specific adjustments depend heavily on your data and the resulting formula, but here are a few ideas to start with: (a) any min_gaps-based boosts could be simply ignored when word_count<2;

    (b) non-trivial min_gaps values (i.e., when word_count>=2) could be clamped with a certain "worst-case" constant, while trivial values (i.e., when min_gaps=0 and word_count<2) could be replaced by that constant;

    (c) a transfer function like 1/(1+min_gaps) could be applied (so that better, smaller min_gaps values would maximize it, and worse, larger min_gaps values would fall off slowly); and so on.
* `lccs` (integer). Longest Common Contiguous Subsequence. The length of the longest subphrase common between the query and the document, computed in keywords.

    The LCCS factor is somewhat similar to LCS but more restrictive. While LCS can be greater than 1 even if no two query words are matched next to each other, LCCS will only be greater than 1 if there are *exact*, contiguous query subphrases in the document. For example, (one two three four five) query vs (one hundred three hundred five hundred) document would yield lcs=3, but lccs=1, because although the mutual dispositions of 3 keywords (one, three, five) match between the query and the document, no 2 matching positions are actually adjacent.

    Note that LCCS still doesn't differentiate between frequent and rare keywords; for that, see WLCCS.
* `wlccs` (float). Weighted Longest Common Contiguous Subsequence. The sum of IDFs of the keywords of the longest subphrase common between the query and the document.

    WLCCS is calculated similarly to LCCS, but every "suitable" keyword occurrence increases it by the keyword IDF instead of just by 1 (as with LCS and LCCS). This allows ranking sequences of rarer and more important keywords higher than sequences of frequent keywords, even if the latter are longer. For example, a query `(Zanzibar bed and breakfast)` would yield lccs=1 for a `(hotels of Zanzibar)` document, but lccs=3 against `(London bed and breakfast)`, even though "Zanzibar" is actually somewhat rarer than the entire "bed and breakfast" phrase. The WLCCS factor addresses this issue by using keyword frequencies.
* `atc` (float). Aggregate Term Closeness. A proximity-based measure that increases when the document contains more groups of more closely located and more important (rare) query keywords.

    **WARNING:** you should use ATC with OPTION idf='plain,tfidf_unnormalized' (see [below](../Searching/Sorting_and_ranking.md#Configuration-of-IDF-formula)); otherwise, you may get unexpected results.

    ATC essentially operates as follows. For each keyword *occurrence* in the document, we compute the so-called *term closeness*. To do this, we examine all the other closest occurrences of all the query keywords (including the keyword itself) to the left and right of the subject occurrence, calculate a distance dampening coefficient as k = pow(distance, -1.75) for these occurrences, and sum the dampened IDFs. As a result, for every occurrence of each keyword, we obtain a "closeness" value that describes the "neighbors" of that occurrence. We then multiply these per-occurrence closenesses by their respective subject keyword IDF, sum them all, and finally compute a logarithm of that sum.

In other words, we process the best (closest) matched keyword pairs in the document, and compute pairwise "closenesses" as the product of their IDFs scaled by the distance coefficient:

```
pair_tc = idf(pair_word1) * idf(pair_word2) * pow(pair_distance, -1.75)
```

We then sum such closenesses, and compute the final, log-dampened ATC value:

```
atc = log(1+sum(pair_tc))
```

Note that this final dampening logarithm is precisely the reason you should use OPTION idf=plain because, without it, the expression inside the log() could be negative.

Having closer keyword occurrences contributes *much* more to ATC than having more frequent keywords. Indeed, when the keywords are right next to each other, distance=1 and k=1; when there's just one word in between them, distance=2 and k=0.297, with two words between, distance=3 and k=0.146, and so on. At the same time, IDF attenuates somewhat slower. For example, in a 1 million document collection, the IDF values for keywords that match in 10, 100, and 1000 documents would be respectively 0.833, 0.667, and 0.500. So a keyword pair with two rather rare keywords that occur in just 10 documents each but with 2 other words in between would yield pair_tc = 0.101 and thus barely outweigh a pair with a 100-doc and a 1000-doc keyword with 1 other word between them and pair_tc = 0.099. Moreover, a pair of two *unique*, 1-doc keywords with 3 words between them would get a pair_tc = 0.088 and lose to a pair of two 1000-doc keywords located right next to each other and yielding a pair_tc = 0.25. So, basically, while ATC does combine both keyword frequency and proximity, it still somewhat favors proximity.

### Ranking factor aggregation functions

A **field aggregation function** is a single-argument function that accepts an expression with field-level factors, iterates over all matched fields, and computes the final results. The currently implemented field aggregation functions include:

* `sum`, which adds the argument expression over all matched fields. For example `sum(1)` should return the number of matched fields.
* `top`, which returns the highest value of the argument across all matched fields.
* `max_window_hits`, manages a sliding window of hit positions to track the maximum number of hits within a specified window size. It removes outdated hits that fall outside the window and adds the latest hit, updating the maximum number of hits found within that window.

### Formula expressions for all the built-in rankers

Most other rankers can actually be emulated using the expression-based ranker. You just need to provide an appropriate expression. While this emulation will likely be slower than using the built-in, compiled ranker, it may still be interesting if you want to fine-tune your ranking formula starting with one of the existing ones. Additionally, the formulas describe the ranker details in a clear, readable manner.

* proximity_bm25 (default ranker) = `sum(lcs*user_weight)*1000+bm25`
* bm25 = `sum(user_weight)*1000+bm25`
* none = `1`
* wordcount = `sum(hit_count*user_weight)`
* proximity = `sum(lcs*user_weight)`
* matchany = `sum((word_count+(lcs-1)*max_lcs)*user_weight)`
* fieldmask = `field_mask`
* sph04 = `sum((4*lcs+2*(min_hit_pos==1)+exact_hit)*user_weight)*1000+bm25`


### Configuration of IDF formula

The historically default IDF (Inverse Document Frequency) in Manticore is equivalent to `OPTION idf='normalized,tfidf_normalized'`, and those normalizations may cause several undesired effects.

First, `idf=normalized` causes keyword penalization. For instance, if you search for `the | something` and `the` occurs in more than 50% of the documents, then documents with both keywords `the` and`[something` will get less weight than documents with just one keyword  `something`. Using `OPTION idf=plain` avoids this.

Plain IDF varies in `[0, log(N)]` range, and keywords are never penalized; while the normalized IDF varies in `[-log(N), log(N)]` range, and too frequent keywords are penalized.

Second, `idf=tfidf_normalized` causes IDF drift over queries. Historically, we additionally divided IDF by query keyword count, so that the entire `sum(tf*idf)` over all keywords would still fit into `[0,1]` range. However, that means that queries `word1` and `word1 | nonmatchingword2` would assign different weights to the exactly same result set, because the IDFs for both `word1` and `nonmatchingword2` would be divided by 2. `OPTION idf='tfidf_unnormalized'` fixes that. Note that BM25, BM25A, BM25F() ranking factors will be scale accordingly  once you disable this normalization.

IDF flags can be mixed; `plain` and `normalized` are mutually exclusive;`tfidf_unnormalized` and `tfidf_normalized` are mutually exclusive; and unspecified flags in such a mutually exclusive group take their defaults. That means that `OPTION idf=plain` is equivalent to a complete `OPTION idf='plain,tfidf_normalized'` specification.    

<!-- proofread -->
