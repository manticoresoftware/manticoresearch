# Faceted search

Faceted search is as essential function of a modern search application as [autocomplete](../Searching/Autocomplete.md), [spell correction](../Searching/Spell_correction.md) and search keywords [highlighting](../Searching/Highlighting.md). Especially in E-commerce products.

![Faceted search](faceted.png)

It comes to the rescue when we deal with large amounts of data and various properties related to each other, whether it is size, color, manufacturer or something else. When querying large amounts of data search results often include large swaths of entries which does not fit user’s expectations. Faceted search allows an end-user to explicitly specify the dimensions that they want their search results to meet.

In Manticore Search there is an optimization that retains the result set of the original query and reuses it for each facet calculation. As the aggregations are applied to already calculated subset of documents they are fast and the total execution time can be in many cases only marginally bigger than the initial query. Facets can be added to any query and the facet can be any attribute or expression. A facet result contains the facet values along with the facet counts. Facets are available using SQL `SELECT` statement by declaring them at the very end of the query.

## Aggregations

<!-- example Aggregations -->
### SQL
The facet values can come from an attribute, JSON property from a JSON attribute or expression. Facet values can be also aliased, however the **alias must be unique** across all result sets (main query results set and other facets results sets). The facet value is taken from the aggregated attribute/expression, but it can also come from another attribute/expression.

```sql
FACET {expr_list} [BY {expr_list} ] [DISTINCT {field_name}] [ORDER BY {expr | FACET()} {ASC | DESC}] [LIMIT [offset,] count]
```

Multiple facet declarations need to be separated with a whitespace.

### HTTP JSON

Facets can be defined in the `aggs` node:

``` json
     "aggs" :
     {
        "group name" :
         {
            "terms" :
             {
              "field":"attribute name",
              "size": 1000
             }
         }
     }
```

where:
* `group name` is an alias given to the aggregation
* `field` value must contain the name of the attribute or expression we are faceting
* optional `size` specifies the maximum number of buckets to include into the result. When not specified it inherits the main query's limit. More details can be found in section [Size of facet result](../Searching/Faceted_search.md#Size-of-facet-result).

The result set will contain an `aggregations` node with the returned facets, where `key` is the aggregated value and `doc_count` is the aggregation count.

``` json
    "aggregations": {
        "group name": {
        "buckets": [
            {
                "key": 10,
                "doc_count": 1019
            },
            {
                "key": 9,
                "doc_count": 954
            },
            {
                "key": 8,
                "doc_count": 1021
            },
            {
                "key": 7,
                "doc_count": 1011
            },
            {
                "key": 6,
                "doc_count": 997
            }
            ]
        }
    }   
```

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT *, price AS aprice FROM facetdemo LIMIT 10 FACET price LIMIT 10 FACET brand_id LIMIT 5;
```

<!-- response SQL -->

```sql
+------+-------+----------+---------------------+------------+-------------+---------------------------------------+------------+--------+
| id   | price | brand_id | title               | brand_name | property    | j                                     | categories | aprice |
+------+-------+----------+---------------------+------------+-------------+---------------------------------------+------------+--------+
|    1 |   306 |        1 | Product Ten Three   | Brand One  | Six_Ten     | {"prop1":66,"prop2":91,"prop3":"One"} | 10,11      |    306 |
|    2 |   400 |       10 | Product Three One   | Brand Ten  | Four_Three  | {"prop1":69,"prop2":19,"prop3":"One"} | 13,14      |    400 |
...
|    9 |   560 |        6 | Product Two Five    | Brand Six  | Eight_Two   | {"prop1":90,"prop2":84,"prop3":"One"} | 13,14      |    560 |
|   10 |   229 |        9 | Product Three Eight | Brand Nine | Seven_Three | {"prop1":84,"prop2":39,"prop3":"One"} | 12,13      |    229 |
+------+-------+----------+---------------------+------------+-------------+---------------------------------------+------------+--------+
10 rows in set (0.00 sec)
+-------+----------+
| price | count(*) |
+-------+----------+
|   306 |        7 |
|   400 |       13 |
...
|   229 |        9 |
|   595 |       10 |
+-------+----------+
10 rows in set (0.00 sec)
+----------+----------+
| brand_id | count(*) |
+----------+----------+
|        1 |     1013 |
|       10 |      998 |
|        5 |     1007 |
|        8 |     1033 |
|        7 |      965 |
+----------+----------+
5 rows in set (0.00 sec)
```
<!-- request JSON -->

```json
POST /search -d '
    {
     "index" : "facetdemo",
     "query" : {"match_all" : {} },
     "limit": 5,
     "aggs" :
     {
        "group_property" :
         {
            "terms" :
             {
              "field":"price",
             }
         },
        "group_brand_id" :
         {
            "terms" :
             {
              "field":"brand_id",
             }
         }
     }
    }
'
```

<!-- response JSON -->

```json
{
  "took": 3,
  "timed_out": false,
  "hits": {
    "total": 10000,
    "hits": [
      {
        "_id": "1",
        "_score": 1,
        "_source": {
          "price": 197,
          "brand_id": 10,
          "brand_name": "Brand Ten",
          "categories": [
            10
          ]
        }
      },
 ...
      {
        "_id": "5",
        "_score": 1,
        "_source": {
          "price": 805,
          "brand_id": 7,
          "brand_name": "Brand Seven",
          "categories": [
            11,
            12,
            13
          ]
        }
      }
    ]
  },
  "aggregations": {
    "group_property": {
      "buckets": [
        {
          "key": 1000,
          "doc_count": 11
        },
        {
          "key": 999,
          "doc_count": 12
        },
...
        {
          "key": 991,
          "doc_count": 7
        }
      ]
    },
    "group_brand_id": {
      "buckets": [
        {
          "key": 10,
          "doc_count": 1019
        },
        {
          "key": 9,
          "doc_count": 954
        },
        {
          "key": 8,
          "doc_count": 1021
        },
        {
          "key": 7,
          "doc_count": 1011
        },
        {
          "key": 6,
          "doc_count": 997
        }
      ]
    }
  }
}
```
<!-- request PHP -->
```php
$index->setName('facetdemo');
$search = $index->search('');
$search->limit(5);
$search->facet('price','price');
$search->facet('brand_id','group_brand_id');
$results = $search->get();
```
<!-- response PHP -->
```php
Array
(
    [price] => Array
        (
            [buckets] => Array
                (
                    [0] => Array
                        (
                            [key] => 1000
                            [doc_count] => 11
                        )
                    [1] => Array
                        (
                            [key] => 999
                            [doc_count] => 12
                        )
                    [2] => Array
                        (
                            [key] => 998
                            [doc_count] => 7
                        )
                    [3] => Array
                        (
                            [key] => 997
                            [doc_count] => 14
                        )
                    [4] => Array
                        (
                            [key] => 996
                            [doc_count] => 8
                        )
                )
        )
    [group_brand_id] => Array
        (
            [buckets] => Array
                (
                    [0] => Array
                        (
                            [key] => 10
                            [doc_count] => 1019
                        )
                    [1] => Array
                        (
                            [key] => 9
                            [doc_count] => 954
                        )
                    [2] => Array
                        (
                            [key] => 8
                            [doc_count] => 1021
                        )
                    [3] => Array
                        (
                            [key] => 7
                            [doc_count] => 1011
                        )
                    [4] => Array
                        (
                            [key] => 6
                            [doc_count] => 997
                        )
                )
        )
)
```
<!-- request Python -->
```python
res =searchApi.search({"index":"facetdemo","query":{"match_all":{}},"limit":5,"aggs":{"group_property":{"terms":{"field":"price",}},"group_brand_id":{"terms":{"field":"brand_id"}}}})
```
<!-- response Python -->
```python
{'aggregations': {u'group_brand_id': {u'buckets': [{u'doc_count': 1019,
                                                    u'key': 10},
                                                   {u'doc_count': 954,
                                                    u'key': 9},
                                                   {u'doc_count': 1021,
                                                    u'key': 8},
                                                   {u'doc_count': 1011,
                                                    u'key': 7},
                                                   {u'doc_count': 997,
                                                    u'key': 6}]},
                  u'group_property': {u'buckets': [{u'doc_count': 11,
                                                    u'key': 1000},
                                                   {u'doc_count': 12,
                                                    u'key': 999},
                                                   {u'doc_count': 7,
                                                    u'key': 998},
                                                   {u'doc_count': 14,
                                                    u'key': 997},
                                                   {u'doc_count': 8,
                                                    u'key': 996}]}},
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 1,
                    u'_source': {u'brand_id': 10,
                                 u'brand_name': u'Brand Ten',
                                 u'categories': [10],
                                 u'price': 197,
                                 u'property': u'Six',
                                 u'title': u'Product Eight One'}},
                   {u'_id': u'2',
                    u'_score': 1,
                    u'_source': {u'brand_id': 6,
                                 u'brand_name': u'Brand Six',
                                 u'categories': [12, 13, 14],
                                 u'price': 671,
                                 u'property': u'Four',
                                 u'title': u'Product Nine Seven'}},
                   {u'_id': u'3',
                    u'_score': 1,
                    u'_source': {u'brand_id': 3,
                                 u'brand_name': u'Brand Three',
                                 u'categories': [13, 14, 15],
                                 u'price': 92,
                                 u'property': u'Six',
                                 u'title': u'Product Five Four'}},
                   {u'_id': u'4',
                    u'_score': 1,
                    u'_source': {u'brand_id': 10,
                                 u'brand_name': u'Brand Ten',
                                 u'categories': [11],
                                 u'price': 713,
                                 u'property': u'Five',
                                 u'title': u'Product Eight Nine'}},
                   {u'_id': u'5',
                    u'_score': 1,
                    u'_source': {u'brand_id': 7,
                                 u'brand_name': u'Brand Seven',
                                 u'categories': [11, 12, 13],
                                 u'price': 805,
                                 u'property': u'Two',
                                 u'title': u'Product Ten Three'}}],
          'max_score': None,
          'total': 10000},
 'profile': None,
 'timed_out': False,
 'took': 4}

```
<!-- request Javascript -->
```javascript
res =  await searchApi.search({"index":"facetdemo","query":{"match_all":{}},"limit":5,"aggs":{"group_property":{"terms":{"field":"price",}},"group_brand_id":{"terms":{"field":"brand_id"}}}});
```
<!-- response Javascript -->
```javascript
{"took":0,"timed_out":false,"hits":{"total":10000,"hits":[{"_id":"1","_score":1,"_source":{"price":197,"brand_id":10,"brand_name":"Brand Ten","categories":[10],"title":"Product Eight One","property":"Six"}},{"_id":"2","_score":1,"_source":{"price":671,"brand_id":6,"brand_name":"Brand Six","categories":[12,13,14],"title":"Product Nine Seven","property":"Four"}},{"_id":"3","_score":1,"_source":{"price":92,"brand_id":3,"brand_name":"Brand Three","categories":[13,14,15],"title":"Product Five Four","property":"Six"}},{"_id":"4","_score":1,"_source":{"price":713,"brand_id":10,"brand_name":"Brand Ten","categories":[11],"title":"Product Eight Nine","property":"Five"}},{"_id":"5","_score":1,"_source":{"price":805,"brand_id":7,"brand_name":"Brand Seven","categories":[11,12,13],"title":"Product Ten Three","property":"Two"}}]}}

```
<!-- request Java -->
```java
aggs = new HashMap<String,Object>(){{
    put("group_property", new HashMap<String,Object>(){{
        put("sizes", new HashMap<String,Object>(){{
                put("field","price");


        }});
    }});
    put("group_brand_id", new HashMap<String,Object>(){{
        put("sizes", new HashMap<String,Object>(){{
                put("field","brand_id");


        }});
    }});            
}};

searchRequest = new SearchRequest();
searchRequest.setIndex("facetdemo");        
searchRequest.setLimit(5);
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
searchRequest.setAggs(aggs);
searchResponse = searchApi.search(searchRequest);

```
<!-- response Java -->
```java
class SearchResponse {
    took: 0
    timedOut: false
    aggregations: {group_property={buckets=[{key=1000, doc_count=11}, {key=999, doc_count=12}, {key=998, doc_count=7}, {key=997, doc_count=14}, {key=996, doc_count=8}]}, group_brand_id={buckets=[{key=10, doc_count=1019}, {key=9, doc_count=954}, {key=8, doc_count=1021}, {key=7, doc_count=1011}, {key=6, doc_count=997}]}}
    hits: class SearchResponseHits {
        maxScore: null
        total: 10000
        hits: [{_id=1, _score=1, _source={price=197, brand_id=10, brand_name=Brand Ten, categories=[10], title=Product Eight One, property=Six}}, {_id=2, _score=1, _source={price=671, brand_id=6, brand_name=Brand Six, categories=[12, 13, 14], title=Product Nine Seven, property=Four}}, {_id=3, _score=1, _source={price=92, brand_id=3, brand_name=Brand Three, categories=[13, 14, 15], title=Product Five Four, property=Six}}, {_id=4, _score=1, _source={price=713, brand_id=10, brand_name=Brand Ten, categories=[11], title=Product Eight Nine, property=Five}}, {_id=5, _score=1, _source={price=805, brand_id=7, brand_name=Brand Seven, categories=[11, 12, 13], title=Product Ten Three, property=Two}}]
    }
    profile: null
}
```
<!-- end -->



<!-- example Another_attribute -->

### Faceting by aggregation over another attribute

Data can be faceted by aggregating another attribute or expression. For example if the documents contain both the brand id and name, we can return in facet the brand names, but aggregate the brand ids. This can be done by using `FACET {expr1} BY {expr2}`


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT * FROM facetdemo FACET brand_name by brand_id;
```

<!-- response SQL -->

```sql
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
| id   | price | brand_id | title               | brand_name  | property    | j                                     | categories |
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
|    1 |   306 |        1 | Product Ten Three   | Brand One   | Six_Ten     | {"prop1":66,"prop2":91,"prop3":"One"} | 10,11      |
|    2 |   400 |       10 | Product Three One   | Brand Ten   | Four_Three  | {"prop1":69,"prop2":19,"prop3":"One"} | 13,14      |
....
|   19 |   855 |        1 | Product Seven Two   | Brand One   | Eight_Seven | {"prop1":63,"prop2":78,"prop3":"One"} | 10,11,12   |
|   20 |    31 |        9 | Product Four One    | Brand Nine  | Ten_Four    | {"prop1":79,"prop2":42,"prop3":"One"} | 12,13,14   |
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
20 rows in set (0.00 sec)
+-------------+----------+
| brand_name  | count(*) |
+-------------+----------+
| Brand One   |     1013 |
| Brand Ten   |      998 |
| Brand Five  |     1007 |
| Brand Nine  |      944 |
| Brand Two   |      990 |
| Brand Six   |     1039 |
| Brand Three |     1016 |
| Brand Four  |      994 |
| Brand Eight |     1033 |
| Brand Seven |      965 |
+-------------+----------+
10 rows in set (0.00 sec)
```

<!-- end -->

<!-- example Distinct -->

### Faceting without duplicates

If you need to remove duplicates from the buckets FACET returns you can use `DISTINCT field_name` where `field_name` is the field by which you want to do the deduplication. It can be also `id` (and it is by default) if you make a FACET query against a distributed table and are not sure you have unique ids in the tables (the tables should be local and have the same schema).

If you have multiple FACET declarations in your query `field_name` should be the same in all of them.

`DISTINCT` returns additional column `count(distinct ...)` before column `count(*)`, so you can get the both results without the need to make another query.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT brand_name, property FROM facetdemo FACET brand_name distinct property;
```

<!-- response SQL -->

```sql
+-------------+----------+
| brand_name  | property |
+-------------+----------+
| Brand Nine  | Four     |
| Brand Ten   | Four     |
| Brand One   | Five     |
| Brand Seven | Nine     |
| Brand Seven | Seven    |
| Brand Three | Seven    |
| Brand Nine  | Five     |
| Brand Three | Eight    |
| Brand Two   | Eight    |
| Brand Six   | Eight    |
| Brand Ten   | Four     |
| Brand Ten   | Two      |
| Brand Four  | Ten      |
| Brand One   | Nine     |
| Brand Four  | Eight    |
| Brand Nine  | Seven    |
| Brand Four  | Five     |
| Brand Three | Four     |
| Brand Four  | Two      |
| Brand Four  | Eight    |
+-------------+----------+
20 rows in set (0.00 sec)

+-------------+--------------------------+----------+
| brand_name  | count(distinct property) | count(*) |
+-------------+--------------------------+----------+
| Brand Nine  |                        3 |        3 |
| Brand Ten   |                        2 |        3 |
| Brand One   |                        2 |        2 |
| Brand Seven |                        2 |        2 |
| Brand Three |                        3 |        3 |
| Brand Two   |                        1 |        1 |
| Brand Six   |                        1 |        1 |
| Brand Four  |                        4 |        5 |
+-------------+--------------------------+----------+
8 rows in set (0.00 sec)
```

<!-- end -->

<!-- example Expressions -->
### Facet over expressions

Facets can aggregate over expressions. A classic example is segmentation of price by certain ranges:

<!-- request SQL -->

```sql
SELECT * FROM facetdemo FACET INTERVAL(price,200,400,600,800) AS price_range ;
```

<!-- response SQL -->

```sql
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+-------------+
| id   | price | brand_id | title               | brand_name  | property    | j                                     | categories | price_range |
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+-------------+
|    1 |   306 |        1 | Product Ten Three   | Brand One   | Six_Ten     | {"prop1":66,"prop2":91,"prop3":"One"} | 10,11      |           1 |
...
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+-------------+
20 rows in set (0.00 sec)

+-------------+----------+
| price_range | count(*) |
+-------------+----------+
|           0 |     1885 |
|           3 |     1973 |
|           4 |     2100 |
|           2 |     1999 |
|           1 |     2043 |
+-------------+----------+
5 rows in set (0.01 sec)
```

<!-- request JSON -->

``` json
POST /search -d '
    {
     "index": "facetdemo",
     "query":
     {
        "match_all": {}
     },
     "expressions":
     {
        "price_range": "INTERVAL(price,200,400,600,800)"
     },
    "aggs":
    {
      "group_property":
      {
        "terms":
        {
            "field": "price_range"
        }
      }
    }
}
```

<!-- response JSON -->

``` json
{
  "took": 3,
  "timed_out": false,
  "hits": {
    "total": 10000,
    "hits": [
      {
        "_id": "1",
        "_score": 1,
        "_source": {
          "price": 197,
          "brand_id": 10,
          "brand_name": "Brand Ten",
          "categories": [
            10
          ],
          "price_range": 0
        }
      },
 ...
      {
        "_id": "20",
        "_score": 1,
        "_source": {
          "price": 227,
          "brand_id": 3,
          "brand_name": "Brand Three",
          "categories": [
            12,
            13
          ],
          "price_range": 1
        }
      }
    ]
  },
  "aggregations": {
    "group_property": {
      "buckets": [
        {
          "key": 4,
          "doc_count": 2100
        },
        {
          "key": 3,
          "doc_count": 1973
        },
        {
          "key": 2,
          "doc_count": 1999
        },
        {
          "key": 1,
          "doc_count": 2043
        },
        {
          "key": 0,
          "doc_count": 1885
        }
      ]
    }
  }
}
```
<!-- request PHP -->
```php
$index->setName('facetdemo');
$search = $index->search('');
$search->limit(5);
$search->expression('price_range','INTERVAL(price,200,400,600,800)');
$search->facet('price_range','group_property');
$results = $search->get();
print_r($results->getFacets());
```
<!-- response PHP -->
```php
Array
(
    [group_property] => Array
        (
            [buckets] => Array
                (
                    [0] => Array
                        (
                            [key] => 4
                            [doc_count] => 2100
                        )
                    [1] => Array
                        (
                            [key] => 3
                            [doc_count] => 1973
                        )
                    [2] => Array
                        (
                            [key] => 2
                            [doc_count] => 1999
                        )
                    [3] => Array
                        (
                            [key] => 1
                            [doc_count] => 2043
                        )
                    [4] => Array
                        (
                            [key] => 0
                            [doc_count] => 1885
                        )
                )
        )
)
```
<!-- request Python -->
```python
res =searchApi.search({"index":"facetdemo","query":{"match_all":{}},"expressions":{"price_range":"INTERVAL(price,200,400,600,800)"},"aggs":{"group_property":{"terms":{"field":"price_range"}}}})
```
<!-- response Python -->
```python
{'aggregations': {u'group_brand_id': {u'buckets': [{u'doc_count': 1019,
                                                    u'key': 10},
                                                   {u'doc_count': 954,
                                                    u'key': 9},
                                                   {u'doc_count': 1021,
                                                    u'key': 8},
                                                   {u'doc_count': 1011,
                                                    u'key': 7},
                                                   {u'doc_count': 997,
                                                    u'key': 6}]},
                  u'group_property': {u'buckets': [{u'doc_count': 11,
                                                    u'key': 1000},
                                                   {u'doc_count': 12,
                                                    u'key': 999},
                                                   {u'doc_count': 7,
                                                    u'key': 998},
                                                   {u'doc_count': 14,
                                                    u'key': 997},
                                                   {u'doc_count': 8,
                                                    u'key': 996}]}},
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 1,
                    u'_source': {u'brand_id': 10,
                                 u'brand_name': u'Brand Ten',
                                 u'categories': [10],
                                 u'price': 197,
                                 u'property': u'Six',
                                 u'title': u'Product Eight One'}},
                   {u'_id': u'2',
                    u'_score': 1,
                    u'_source': {u'brand_id': 6,
                                 u'brand_name': u'Brand Six',
                                 u'categories': [12, 13, 14],
                                 u'price': 671,
                                 u'property': u'Four',
                                 u'title': u'Product Nine Seven'}},
                   {u'_id': u'3',
                    u'_score': 1,
                    u'_source': {u'brand_id': 3,
                                 u'brand_name': u'Brand Three',
                                 u'categories': [13, 14, 15],
                                 u'price': 92,
                                 u'property': u'Six',
                                 u'title': u'Product Five Four'}},
                   {u'_id': u'4',
                    u'_score': 1,
                    u'_source': {u'brand_id': 10,
                                 u'brand_name': u'Brand Ten',
                                 u'categories': [11],
                                 u'price': 713,
                                 u'property': u'Five',
                                 u'title': u'Product Eight Nine'}},
                   {u'_id': u'5',
                    u'_score': 1,
                    u'_source': {u'brand_id': 7,
                                 u'brand_name': u'Brand Seven',
                                 u'categories': [11, 12, 13],
                                 u'price': 805,
                                 u'property': u'Two',
                                 u'title': u'Product Ten Three'}}],
          'max_score': None,
          'total': 10000},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Javascript -->
```javascript
res =  await searchApi.search({"index":"facetdemo","query":{"match_all":{}},"expressions":{"price_range":"INTERVAL(price,200,400,600,800)"},"aggs":{"group_property":{"terms":{"field":"price_range"}}}});
```
<!-- response Javascript -->
```javascript
{"took":0,"timed_out":false,"hits":{"total":10000,"hits":[{"_id":"1","_score":1,"_source":{"price":197,"brand_id":10,"brand_name":"Brand Ten","categories":[10],"title":"Product Eight One","property":"Six","price_range":0}},{"_id":"2","_score":1,"_source":{"price":671,"brand_id":6,"brand_name":"Brand Six","categories":[12,13,14],"title":"Product Nine Seven","property":"Four","price_range":3}},{"_id":"3","_score":1,"_source":{"price":92,"brand_id":3,"brand_name":"Brand Three","categories":[13,14,15],"title":"Product Five Four","property":"Six","price_range":0}},{"_id":"4","_score":1,"_source":{"price":713,"brand_id":10,"brand_name":"Brand Ten","categories":[11],"title":"Product Eight Nine","property":"Five","price_range":3}},{"_id":"5","_score":1,"_source":{"price":805,"brand_id":7,"brand_name":"Brand Seven","categories":[11,12,13],"title":"Product Ten Three","property":"Two","price_range":4}},{"_id":"6","_score":1,"_source":{"price":420,"brand_id":2,"brand_name":"Brand Two","categories":[10,11],"title":"Product Two One","property":"Six","price_range":2}},{"_id":"7","_score":1,"_source":{"price":412,"brand_id":9,"brand_name":"Brand Nine","categories":[10],"title":"Product Four Nine","property":"Eight","price_range":2}},{"_id":"8","_score":1,"_source":{"price":300,"brand_id":9,"brand_name":"Brand Nine","categories":[13,14,15],"title":"Product Eight Four","property":"Five","price_range":1}},{"_id":"9","_score":1,"_source":{"price":728,"brand_id":1,"brand_name":"Brand One","categories":[11],"title":"Product Nine Six","property":"Four","price_range":3}},{"_id":"10","_score":1,"_source":{"price":622,"brand_id":3,"brand_name":"Brand Three","categories":[10,11],"title":"Product Six Seven","property":"Two","price_range":3}},{"_id":"11","_score":1,"_source":{"price":462,"brand_id":5,"brand_name":"Brand Five","categories":[10,11],"title":"Product Ten Two","property":"Eight","price_range":2}},{"_id":"12","_score":1,"_source":{"price":939,"brand_id":7,"brand_name":"Brand Seven","categories":[12,13],"title":"Product Nine Seven","property":"Six","price_range":4}},{"_id":"13","_score":1,"_source":{"price":948,"brand_id":8,"brand_name":"Brand Eight","categories":[12],"title":"Product Ten One","property":"Six","price_range":4}},{"_id":"14","_score":1,"_source":{"price":900,"brand_id":9,"brand_name":"Brand Nine","categories":[12,13,14],"title":"Product Ten Nine","property":"Three","price_range":4}},{"_id":"15","_score":1,"_source":{"price":224,"brand_id":3,"brand_name":"Brand Three","categories":[13],"title":"Product Two Six","property":"Four","price_range":1}},{"_id":"16","_score":1,"_source":{"price":713,"brand_id":10,"brand_name":"Brand Ten","categories":[12],"title":"Product Two Four","property":"Six","price_range":3}},{"_id":"17","_score":1,"_source":{"price":510,"brand_id":2,"brand_name":"Brand Two","categories":[10],"title":"Product Ten Two","property":"Seven","price_range":2}},{"_id":"18","_score":1,"_source":{"price":702,"brand_id":10,"brand_name":"Brand Ten","categories":[12,13],"title":"Product Nine One","property":"Three","price_range":3}},{"_id":"19","_score":1,"_source":{"price":836,"brand_id":4,"brand_name":"Brand Four","categories":[10,11,12],"title":"Product Four Five","property":"Two","price_range":4}},{"_id":"20","_score":1,"_source":{"price":227,"brand_id":3,"brand_name":"Brand Three","categories":[12,13],"title":"Product Three Four","property":"Ten","price_range":1}}]}}
```

<!-- request Java -->
```java
searchRequest = new SearchRequest();
expressions = new HashMap<String,Object>(){{
    put("price_range","INTERVAL(price,200,400,600,800)");
}};
searchRequest.setExpressions(expressions);
aggs = new HashMap<String,Object>(){{
    put("group_property", new HashMap<String,Object>(){{
        put("sizes", new HashMap<String,Object>(){{
                put("field","price_range");


        }});
    }});

}};
searchRequest.setIndex("facetdemo");        
searchRequest.setLimit(5);
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
searchRequest.setAggs(aggs);
searchResponse = searchApi.search(searchRequest);

```
<!-- response Java -->
```java
class SearchResponse {
    took: 0
    timedOut: false
    aggregations: {group_property={buckets=[{key=4, doc_count=2100}, {key=3, doc_count=1973}, {key=2, doc_count=1999}, {key=1, doc_count=2043}, {key=0, doc_count=1885}]}}
    hits: class SearchResponseHits {
        maxScore: null
        total: 10000
        hits: [{_id=1, _score=1, _source={price=197, brand_id=10, brand_name=Brand Ten, categories=[10], title=Product Eight One, property=Six, price_range=0}}, {_id=2, _score=1, _source={price=671, brand_id=6, brand_name=Brand Six, categories=[12, 13, 14], title=Product Nine Seven, property=Four, price_range=3}}, {_id=3, _score=1, _source={price=92, brand_id=3, brand_name=Brand Three, categories=[13, 14, 15], title=Product Five Four, property=Six, price_range=0}}, {_id=4, _score=1, _source={price=713, brand_id=10, brand_name=Brand Ten, categories=[11], title=Product Eight Nine, property=Five, price_range=3}}, {_id=5, _score=1, _source={price=805, brand_id=7, brand_name=Brand Seven, categories=[11, 12, 13], title=Product Ten Three, property=Two, price_range=4}}]
    }
    profile: null
}
```
<!-- end -->

<!-- example Multi-level -->

### Facet over multi-level grouping

Facets can aggregate over multi-level grouping, the result set being the same as the query would perform a multi-level grouping:

<!-- request SQL -->

```sql
SELECT *,INTERVAL(price,200,400,600,800) AS price_range FROM facetdemo
FACET price_range AS price_range,brand_name ORDER BY brand_name asc;
```

<!-- response SQL -->

```sql
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+-------------+
| id   | price | brand_id | title               | brand_name  | property    | j                                     | categories | price_range |
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+-------------+
|    1 |   306 |        1 | Product Ten Three   | Brand One   | Six_Ten     | {"prop1":66,"prop2":91,"prop3":"One"} | 10,11      |           1 |
...
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+-------------+
20 rows in set (0.00 sec)

+--------------+-------------+----------+
| fprice_range | brand_name  | count(*) |
+--------------+-------------+----------+
|            1 | Brand Eight |      197 |
|            4 | Brand Eight |      235 |
|            3 | Brand Eight |      203 |
|            2 | Brand Eight |      201 |
|            0 | Brand Eight |      197 |
|            4 | Brand Five  |      230 |
|            2 | Brand Five  |      197 |
|            1 | Brand Five  |      204 |
|            3 | Brand Five  |      193 |
|            0 | Brand Five  |      183 |
|            1 | Brand Four  |      195 |
...
```
<!-- end -->

<!-- example Ordering -->
### Ordering in facet result

Facets support `ORDER BY` clause as same as a standard query. Each facet can have it's or own ordering and the facet ordering doesn't affect in any way the ordering of the main result set, which is ordered by the main query's `ORDER BY`. Sorting can be made on attribute name, count (using `COUNT(*)`) or special `FACET()` function can be used, which provides the aggregated data values.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT * FROM facetdemo
FACET brand_name BY brand_id ORDER BY FACET() ASC
FACET brand_name BY brand_id ORDER BY brand_name ASC
FACET brand_name BY brand_id order BY COUNT(*) DESC;
```

<!-- response SQL -->

```sql
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
| id   | price | brand_id | title               | brand_name  | property    | j                                     | categories |
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
|    1 |   306 |        1 | Product Ten Three   | Brand One   | Six_Ten     | {"prop1":66,"prop2":91,"prop3":"One"} | 10,11      |
...
|   20 |    31 |        9 | Product Four One    | Brand Nine  | Ten_Four    | {"prop1":79,"prop2":42,"prop3":"One"} | 12,13,14   |
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
20 rows in set (0.01 sec)

+-------------+----------+
| brand_name  | count(*) |
+-------------+----------+
| Brand One   |     1013 |
| Brand Two   |      990 |
| Brand Three |     1016 |
| Brand Four  |      994 |
| Brand Five  |     1007 |
| Brand Six   |     1039 |
| Brand Seven |      965 |
| Brand Eight |     1033 |
| Brand Nine  |      944 |
| Brand Ten   |      998 |
+-------------+----------+
10 rows in set (0.01 sec)

+-------------+----------+
| brand_name  | count(*) |
+-------------+----------+
| Brand Eight |     1033 |
| Brand Five  |     1007 |
| Brand Four  |      994 |
| Brand Nine  |      944 |
| Brand One   |     1013 |
| Brand Seven |      965 |
| Brand Six   |     1039 |
| Brand Ten   |      998 |
| Brand Three |     1016 |
| Brand Two   |      990 |
+-------------+----------+
10 rows in set (0.01 sec)

+-------------+----------+
| brand_name  | count(*) |
+-------------+----------+
| Brand Six   |     1039 |
| Brand Eight |     1033 |
| Brand Three |     1016 |
| Brand One   |     1013 |
| Brand Five  |     1007 |
| Brand Ten   |      998 |
| Brand Four  |      994 |
| Brand Two   |      990 |
| Brand Seven |      965 |
| Brand Nine  |      944 |
+-------------+----------+
10 rows in set (0.01 sec)
```

<!-- end -->


<!-- example Size -->
### Size of facet result

By default each facet result set is limited to 20 values. The number of facet values can be controlled with `LIMIT` clause individually for each facet  by providing either a number of values to return in format `LIMIT count` or with an offset as `LIMIT offset, count`.

The maximum facet values that can be returned is limited by the query's `max_matches` setting. In case dynamic max_matches (limiting max_matches to offset+per page for better performance) is wanted to be implemented, it must be taken in account that a too low max_matches value can hurt the number of facet values. In this case, a minimum max_matches value should be used good enough to cover the number of facet values.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT * FROM facetdemo
FACET brand_name BY brand_id ORDER BY FACET() ASC  LIMIT 0,1
FACET brand_name BY brand_id ORDER BY brand_name ASC LIMIT 2,4
FACET brand_name BY brand_id order BY COUNT(*) DESC LIMIT 4;
```

<!-- response SQL -->

```sql
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
| id   | price | brand_id | title               | brand_name  | property    | j                                     | categories |
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
|    1 |   306 |        1 | Product Ten Three   | Brand One   | Six_Ten     | {"prop1":66,"prop2":91,"prop3":"One"} | 10,11      |
...
|   20 |    31 |        9 | Product Four One    | Brand Nine  | Ten_Four    | {"prop1":79,"prop2":42,"prop3":"One"} | 12,13,14   |
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
20 rows in set (0.01 sec)

+-------------+----------+
| brand_name  | count(*) |
+-------------+----------+
| Brand One   |     1013 |
+-------------+----------+
1 rows in set (0.01 sec)

+-------------+----------+
| brand_name  | count(*) |
+-------------+----------+
| Brand Four  |      994 |
| Brand Nine  |      944 |
| Brand One   |     1013 |
| Brand Seven |      965 |
+-------------+----------+
4 rows in set (0.01 sec)

+-------------+----------+
| brand_name  | count(*) |
+-------------+----------+
| Brand Six   |     1039 |
| Brand Eight |     1033 |
| Brand Three |     1016 |
+-------------+----------+
3 rows in set (0.01 sec)
```
<!-- request JSON -->

``` json
POST /search -d '
    {
     "index" : "facetdemo",
     "query" : {"match_all" : {} },
     "limit": 5,
     "aggs" :
     {
        "group_property" :
         {
            "terms" :
             {
              "field":"price",
              "size":1,
             }
         },
        "group_brand_id" :
         {
            "terms" :
             {
              "field":"brand_id",
              "size":3
             }
         }
     }
    }
'
```

<!-- response JSON -->

``` json
{
  "took": 3,
  "timed_out": false,
  "hits": {
    "total": 10000,
    "hits": [
      {
        "_id": "1",
        "_score": 1,
        "_source": {
          "price": 197,
          "brand_id": 10,
          "brand_name": "Brand Ten",
          "categories": [
            10
          ]
        }
      },
 ...
      {
        "_id": "5",
        "_score": 1,
        "_source": {
          "price": 805,
          "brand_id": 7,
          "brand_name": "Brand Seven",
          "categories": [
            11,
            12,
            13
          ]
        }
      }
    ]
  },
  "aggregations": {
    "group_property": {
      "buckets": [
        {
          "key": 1000,
          "doc_count": 11
        }
      ]
    },
    "group_brand_id": {
      "buckets": [
        {
          "key": 10,
          "doc_count": 1019
        },
        {
          "key": 9,
          "doc_count": 954
        },
        {
          "key": 8,
          "doc_count": 1021
        }
      ]
    }
  }
}
```
<!-- request PHP -->
```php
$index->setName('facetdemo');
$search = $index->search('');
$search->limit(5);
$search->facet('price','price',1);
$search->facet('brand_id','group_brand_id',3);
$results = $search->get();
print_r($results->getFacets());
```
<!-- response PHP -->
```php

Array
(
    [price] => Array
        (
            [buckets] => Array
                (
                    [0] => Array
                        (
                            [key] => 1000
                            [doc_count] => 11
                        )
                )
        )
    [group_brand_id] => Array
        (
            [buckets] => Array
                (
                    [0] => Array
                        (
                            [key] => 10
                            [doc_count] => 1019
                        )
                    [1] => Array
                        (
                            [key] => 9
                            [doc_count] => 954
                        )
                    [2] => Array
                        (
                            [key] => 8
                            [doc_count] => 1021
                        )
                )
        )
)
```
<!-- request Python -->
```python
res =searchApi.search({"index":"facetdemo","query":{"match_all":{}},"limit":5,"aggs":{"group_property":{"terms":{"field":"price","size":1,}},"group_brand_id":{"terms":{"field":"brand_id","size":3}}}})
```
<!-- response Python -->
```python
{'aggregations': {u'group_brand_id': {u'buckets': [{u'doc_count': 1019,
                                                    u'key': 10},
                                                   {u'doc_count': 954,
                                                    u'key': 9},
                                                   {u'doc_count': 1021,
                                                    u'key': 8}]},
                  u'group_property': {u'buckets': [{u'doc_count': 11,
                                                    u'key': 1000}]}},
 'hits': {'hits': [{u'_id': u'1',
                    u'_score': 1,
                    u'_source': {u'brand_id': 10,
                                 u'brand_name': u'Brand Ten',
                                 u'categories': [10],
                                 u'price': 197,
                                 u'property': u'Six',
                                 u'title': u'Product Eight One'}},
                   {u'_id': u'2',
                    u'_score': 1,
                    u'_source': {u'brand_id': 6,
                                 u'brand_name': u'Brand Six',
                                 u'categories': [12, 13, 14],
                                 u'price': 671,
                                 u'property': u'Four',
                                 u'title': u'Product Nine Seven'}},
                   {u'_id': u'3',
                    u'_score': 1,
                    u'_source': {u'brand_id': 3,
                                 u'brand_name': u'Brand Three',
                                 u'categories': [13, 14, 15],
                                 u'price': 92,
                                 u'property': u'Six',
                                 u'title': u'Product Five Four'}},
                   {u'_id': u'4',
                    u'_score': 1,
                    u'_source': {u'brand_id': 10,
                                 u'brand_name': u'Brand Ten',
                                 u'categories': [11],
                                 u'price': 713,
                                 u'property': u'Five',
                                 u'title': u'Product Eight Nine'}},
                   {u'_id': u'5',
                    u'_score': 1,
                    u'_source': {u'brand_id': 7,
                                 u'brand_name': u'Brand Seven',
                                 u'categories': [11, 12, 13],
                                 u'price': 805,
                                 u'property': u'Two',
                                 u'title': u'Product Ten Three'}}],
          'max_score': None,
          'total': 10000},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- request Javascript -->
```javascript
res =  await searchApi.search({"index":"facetdemo","query":{"match_all":{}},"limit":5,"aggs":{"group_property":{"terms":{"field":"price","size":1,}},"group_brand_id":{"terms":{"field":"brand_id","size":3}}}});
```
<!-- response Javascript -->
```javascript
{"took":0,"timed_out":false,"hits":{"total":10000,"hits":[{"_id":"1","_score":1,"_source":{"price":197,"brand_id":10,"brand_name":"Brand Ten","categories":[10],"title":"Product Eight One","property":"Six"}},{"_id":"2","_score":1,"_source":{"price":671,"brand_id":6,"brand_name":"Brand Six","categories":[12,13,14],"title":"Product Nine Seven","property":"Four"}},{"_id":"3","_score":1,"_source":{"price":92,"brand_id":3,"brand_name":"Brand Three","categories":[13,14,15],"title":"Product Five Four","property":"Six"}},{"_id":"4","_score":1,"_source":{"price":713,"brand_id":10,"brand_name":"Brand Ten","categories":[11],"title":"Product Eight Nine","property":"Five"}},{"_id":"5","_score":1,"_source":{"price":805,"brand_id":7,"brand_name":"Brand Seven","categories":[11,12,13],"title":"Product Ten Three","property":"Two"}}]}}

```

<!-- request Java -->
```java
searchRequest = new SearchRequest();
aggs = new HashMap<String,Object>(){{
    put("group_property", new HashMap<String,Object>(){{
        put("sizes", new HashMap<String,Object>(){{
                put("field","price");
                put("size",1);


        }});
    }});
    put("group_brand_id", new HashMap<String,Object>(){{
        put("sizes", new HashMap<String,Object>(){{
                put("field","brand_id");
                put("size",3);


        }});
    }});            
}};
searchRequest.setIndex("facetdemo");        
searchRequest.setLimit(5);
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
searchRequest.setAggs(aggs);
searchResponse = searchApi.search(searchRequest);
```
<!-- response Java -->
```java
class SearchResponse {
    took: 0
    timedOut: false
    aggregations: {group_property={buckets=[{key=1000, doc_count=11}]}, group_brand_id={buckets=[{key=10, doc_count=1019}, {key=9, doc_count=954}, {key=8, doc_count=1021}]}}
    hits: class SearchResponseHits {
        maxScore: null
        total: 10000
        hits: [{_id=1, _score=1, _source={price=197, brand_id=10, brand_name=Brand Ten, categories=[10], title=Product Eight One, property=Six}}, {_id=2, _score=1, _source={price=671, brand_id=6, brand_name=Brand Six, categories=[12, 13, 14], title=Product Nine Seven, property=Four}}, {_id=3, _score=1, _source={price=92, brand_id=3, brand_name=Brand Three, categories=[13, 14, 15], title=Product Five Four, property=Six}}, {_id=4, _score=1, _source={price=713, brand_id=10, brand_name=Brand Ten, categories=[11], title=Product Eight Nine, property=Five}}, {_id=5, _score=1, _source={price=805, brand_id=7, brand_name=Brand Seven, categories=[11, 12, 13], title=Product Ten Three, property=Two}}]
    }
    profile: null
}
```
<!-- end -->
### Returned result set

When using SQL, a search with facets returns a multiple result sets response. The MySQL client/library/connector used **must** have support (most do) for multiple result sets in order to be able to access the facet result sets.

<!-- example Performance -->
### Performance

Internally, the `FACET` is a shorthand for executing a multi-query where the first query contains the main search query and the rest of the queries in the batch have each a clustering. As in the case of multi-query, the common query optimization can kick-in for a faceted search, meaning the search query is executed only once and the facets operates on the search query result, each facet adding only a fraction of time to the total query time.


To check if the faceted search ran in an optimized mode can be seen in [query log](../Logging/Query_logging.md), where all the logged queries will contain a `xN` string, where `N` is the number of queries that ran in the optimized group or checking the output of [SHOW META](../Node_info_and_management/SHOW_META.md) statement which will exhibit a `multiplier` metric:

<!-- request SQL -->

```sql
SELECT * FROM facetdemo FACET brand_id FACET price FACET categories;
SHOW META LIKE 'multiplier';
```

<!-- response SQL -->

```sql
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
| id   | price | brand_id | title               | brand_name  | property    | j                                     | categories |
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
|    1 |   306 |        1 | Product Ten Three   | Brand One   | Six_Ten     | {"prop1":66,"prop2":91,"prop3":"One"} | 10,11      |
...

+----------+----------+
| brand_id | count(*) |
+----------+----------+
|        1 |     1013 |
...

+-------+----------+
| price | count(*) |
+-------+----------+
|   306 |        7 |
...

+------------+----------+
| categories | count(*) |
+------------+----------+
|         10 |     2436 |
...

+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| multiplier    | 4     |
+---------------+-------+
1 row in set (0.00 sec)
```

<!-- end -->
