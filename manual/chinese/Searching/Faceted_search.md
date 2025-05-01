# 分面搜索

分面搜索对现代搜索应用程序至关重要，如同 [自动完成](../Searching/Autocomplete.md)、[拼写纠正](../Searching/Spell_correction.md) 和搜索关键词 [高亮显示](../Searching/Highlighting.md)，尤其是在电子商务产品中。

![分面搜索](faceted.png)

在处理大量数据和各种相互关联的属性时，如大小、颜色、制造商或其他因素，分面搜索非常有用。当查询大量数据时，搜索结果通常包括许多与用户期望不符的条目。分面搜索使最终用户能够明确定义他们希望搜索结果满足的标准。

在 Manticore Search 中，有一种优化维护原始查询的结果集，并为每个分面计算重用它。由于聚合应用于一个已经计算过的文档子集，因此它们速度很快，总的执行时间通常只比初始查询稍长。可以将分面添加到任何查询中，分面可以是任何属性或表达式。分面结果包括分面值和分面计数。可以通过在查询的最后声明它们，使用 SQL `SELECT` 语句访问分面。

## 聚合

<!-- example Aggregations -->
### SQL
分面值可以来自一个属性、JSON 属性内的 JSON 属性，或一个表达式。分面值也可以被别名，但 **别名在所有结果集中必须唯一**（主查询结果集和其他分面结果集）。分面值源自聚合的属性/表达式，但它也可以来自另一个属性/表达式。

```sql
FACET {expr_list} [BY {expr_list} ] [DISTINCT {field_name}] [ORDER BY {expr | FACET()} {ASC | DESC}] [LIMIT [offset,] count]
```

多个分面声明必须用空白分隔。

### HTTP JSON

分面可以在 `aggs` 节点中定义：

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
             "sort": [ {"attribute name": { "order":"asc" }} ]
         }
     }
```

其中：
* `group name` 是分配给聚合的别名
* `field` 值必须包含被分面的属性或表达式的名称
* 可选的 `size` 指定要包含在结果中的最大桶数。当未指定时，它继承主查询的限制。更多细节可在 [分面结果的大小](../Searching/Faceted_search.md#Size-of-facet-result) 部分找到。
* 可选的 `sort` 指定一个属性和/或附加属性的数组，采用与 [主查询中的 "sort" 参数](../Searching/Sorting_and_ranking.md#Sorting-via-JSON) 相同的语法。

结果集将包含一个 `aggregations` 节点，返回的分面中，`key` 是聚合值，`doc_count` 是聚合计数。

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
<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /search -d '
    {
     "table" : "facetdemo",
     "query" : {"match_all" : {} },
     "limit": 5,
     "aggs" :
     {
        "group_property" :
         {
            "terms" :
             {
              "field":"price"
             }
         },
        "group_brand_id" :
         {
            "terms" :
             {
              "field":"brand_id"
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
        "_id": 1,
        "_score": 1,
        "_source": {
          "price": 197,
          "brand_id": 10,
          "brand_name": "品牌十",
          "categories": [
            10
          ]
        }
      },
 ...
      {
        "_id": 5,
        "_score": 1,
        "_source": {
          "price": 805,
          "brand_id": 7,
          "brand_name": "品牌七",
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

<!-- intro -->
##### PHP:

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

<!-- intro -->
##### Python:

<!-- request Python -->
```python
res =searchApi.search({"table":"facetdemo","query":{"match_all":{}},"limit":5,"aggs":{"group_property":{"terms":{"field":"price",}},"group_brand_id":{"terms":{"field":"brand_id"}}}})
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
                                 u'brand_name': u'品牌十',
                                 u'categories': [10],
                                 u'price': 197,
                                 u'property': u'六',
                                 u'title': u'产品八一'}},
                   {u'_id': u'2',
                    u'_score': 1,
                    u'_source': {u'brand_id': 6,
                                 u'brand_name': u'品牌六',
                                 u'categories': [12, 13, 14],
                                 u'price': 671,
                                 u'property': u'四',
                                 u'title': u'产品九七'}},
                   {u'_id': u'3',
                    u'_score': 1,
                    u'_source': {u'brand_id': 3,
                                 u'brand_name': u'品牌三',
                                 u'categories': [13, 14, 15],
                                 u'price': 92,
                                 u'property': u'六',
                                 u'title': u'产品五四'}},
                   {u'_id': u'4',
                    u'_score': 1,
                    u'_source': {u'brand_id': 10,
                                 u'brand_name': u'品牌十',
                                 u'categories': [11],
                                 u'price': 713,
                                 u'property': u'五',
                                 u'title': u'产品八九'}},
                   {u'_id': u'5',
                    u'_score': 1,
                    u'_source': {u'brand_id': 7,
                                 u'brand_name': u'品牌七',
                                 u'categories': [11, 12, 13],
                                 u'price': 805,
                                 u'property': u'二',
                                 u'title': u'产品十三'}}],
          'max_score': None,
          'total': 10000},
 'profile': None,
 'timed_out': False,
 'took': 4}

```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->
```python
res = await searchApi.search({"table":"facetdemo","query":{"match_all":{}},"limit":5,"aggs":{"group_property":{"terms":{"field":"price",}},"group_brand_id":{"terms":{"field":"brand_id"}}}})
```
<!-- response Python-asyncio -->
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
                                 u'brand_name': u'品牌十',
                                 u'categories': [10],
                                 u'price': 197,
                                 u'property': u'六',
                                 u'title': u'产品八一'}},
                   {u'_id': u'2',
                    u'_score': 1,
                    u'_source': {u'brand_id': 6,
                                 u'brand_name': u'品牌六',
                                 u'categories': [12, 13, 14],
                                 u'price': 671,
                                 u'property': u'四',
                                 u'title': u'产品九七'}},
                   {u'_id': u'3',
                    u'_score': 1,
                    u'_source': {u'brand_id': 3,
                                 u'brand_name': u'品牌三',
                                 u'categories': [13, 14, 15],
                                 u'price': 92,
                                 u'property': u'六',
                                 u'title': u'产品五四'}},
                   {u'_id': u'4',
                    u'_score': 1,
                    u'_source': {u'brand_id': 10,
                                 u'brand_name': u'品牌十',
                                 u'categories': [11],
                                 u'price': 713,
                                 u'property': u'五',
                                 u'title': u'产品八九'}},
                   {u'_id': u'5',
                    u'_score': 1,
                    u'_source': {u'brand_id': 7,
                                 u'brand_name': u'品牌七',
                                 u'categories': [11, 12, 13],
                                 u'price': 805,
                                 u'property': u'二',
                                 u'title': u'产品十三'}}],
          'max_score': None,
          'total': 10000},
 'profile': None,
 'timed_out': False,
 'took': 4}

```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->
```javascript
res =  await searchApi.search({"table":"facetdemo","query":{"match_all":{}},"limit":5,"aggs":{"group_property":{"terms":{"field":"price",}},"group_brand_id":{"terms":{"field":"brand_id"}}}});
```
<!-- response Javascript -->
```javascript
{"took":0,"timed_out":false,"hits":{"total":10000,"hits":[{"_id": 1,"_score":1,"_source":{"price":197,"brand_id":10,"brand_name":"品牌十","categories":[10],"title":"产品八一","property":"六"}},{"_id": 2,"_score":1,"_source":{"price":671,"brand_id":6,"brand_name":"品牌六","categories":[12,13,14],"title":"产品九七","property":"四"}},{"_id": 3,"_score":1,"_source":{"price":92,"brand_id":3,"brand_name":"品牌三","categories":[13,14,15],"title":"产品五四","property":"六"}},{"_id": 4,"_score":1,"_source":{"price":713,"brand_id":10,"brand_name":"品牌十","categories":[11],"title":"产品八九","property":"五"}},{"_id": 5,"_score":1,"_source":{"price":805,"brand_id":7,"brand_name":"品牌七","categories":[11,12,13],"title":"产品十三","property":"二"}}]}}

```

<!-- intro -->
##### Java:

<!-- request Java -->
```java
aggs = new HashMap<String,Object>(){{
    put("group_property", new HashMap<String,Object>(){{
        put("terms", new HashMap<String,Object>(){{
                put("field","price");


        }});
    }});
    put("group_brand_id", new HashMap<String,Object>(){{
        put("terms", new HashMap<String,Object>(){{
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
        hits: [{_id=1, _score=1, _source={price=197, brand_id=10, brand_name=品牌十, categories=[10], title=产品八一, property=六}}, {_id=2, _score=1, _source={price=671, brand_id=6, brand_name=品牌六, categories=[12, 13, 14], title=产品九七, property=四}}, {_id=3, _score=1, _source={price=92, brand_id=3, brand_name=品牌三, categories=[13, 14, 15], title=产品五四, property=六}}, {_id=4, _score=1, _source={price=713, brand_id=10, brand_name=品牌十, categories=[11], title=产品八九, property=五}}, {_id=5, _score=1, _source={price=805, brand_id=7, brand_name=品牌七, categories=[11, 12, 13], title=产品十三, property=二}}]
    }
    profile: null
}
```

<!-- intro -->
##### C#:

<!-- request C# -->
```clike
var agg1 = new Aggregation("group_property", "price");
var agg2 = new Aggregation("group_brand_id", "brand_id");
object query = new { match_all=null };
var searchRequest = new SearchRequest("facetdemo", query);
searchRequest.Limit = 5;
searchRequest.Aggs = new List<Aggregation> {agg1, agg2};
var searchResponse = searchApi.Search(searchRequest);

```
<!-- response C# -->
```clike
class SearchResponse {
    took: 0
    timedOut: false
    aggregations: {group_property={buckets=[{key=1000, doc_count=11}, {key=999, doc_count=12}, {key=998, doc_count=7}, {key=997, doc_count=14}, {key=996, doc_count=8}]}, group_brand_id={buckets=[{key=10, doc_count=1019}, {key=9, doc_count=954}, {key=8, doc_count=1021}, {key=7, doc_count=1011}, {key=6, doc_count=997}]}}
    hits: class SearchResponseHits {
        maxScore: null
        total: 10000
        hits: [{_id=1, _score=1, _source={price=197, brand_id=10, brand_name=品牌十, categories=[10], title=产品八一, property=六}}, {_id=2, _score=1, _source={price=671, brand_id=6, brand_name=品牌六, categories=[12, 13, 14], title=产品九七, property=四}}, {_id=3, _score=1, _source={price=92, brand_id=3, brand_name=品牌三, categories=[13, 14, 15], title=产品五四, property=六}}, {_id=4, _score=1, _source={price=713, brand_id=10, brand_name=品牌十, categories=[11], title=产品八九, property=五}}, {_id=5, _score=1, _source={price=805, brand_id=7, brand_name=品牌七, categories=[11, 12, 13], title=产品十三, property=二}}]
    }
    profile: null
}
```

<!-- intro -->
##### Rust:

<!-- request Rust -->
```rust
let query = SearchQuery::new();
let aggTerms1 = AggTerms::new("price");
let agg1 = Aggregation {
    terms: Some(Box::new(aggTerms1)),
    ..Default::default(),
};
let aggTerms2 = AggTerms::new("brand_id");
let agg2 = Aggregation {
    terms: Some(Box::new(aggTerms2)),
    ..Default::default(),
};
let mut aggs = HashMap::new();
aggs.insert("group_property".to_string(), agg1); 
aggs.insert("group_brand_id".to_string(), agg2);

let search_req = SearchRequest {
    table: "facetdemo".to_string(),
    query: Some(Box::new(query)),
    aggs: serde_json::json!(aggs),
    limit: serde_json::json!(5),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```
<!-- response Rust -->
```rust
class SearchResponse {
    took: 0
    timedOut: false
    aggregations: {group_property={buckets=[{key=1000, doc_count=11}, {key=999, doc_count=12}, {key=998, doc_count=7}, {key=997, doc_count=14}, {key=996, doc_count=8}]}, group_brand_id={buckets=[{key=10, doc_count=1019}, {key=9, doc_count=954}, {key=8, doc_count=1021}, {key=7, doc_count=1011}, {key=6, doc_count=997}]}}
    hits: class SearchResponseHits {
        maxScore: null
        total: 10000
        hits: [{_id=1, _score=1, _source={price=197, brand_id=10, brand_name=品牌十, categories=[10], title=产品八一, property=六}}, {_id=2, _score=1, _source={price=671, brand_id=6, brand_name=品牌六, categories=[12, 13, 14], title=产品九七, property=四}}, {_id=3, _score=1, _source={price=92, brand_id=3, brand_name=品牌三, categories=[13, 14, 15], title=产品五四, property=六}}, {_id=4, _score=1, _source={price=713, brand_id=10, brand_name=品牌十, categories=[11], title=产品八九, property=五}}, {_id=5, _score=1, _source={price=805, brand_id=7, brand_name=品牌七, categories=[11, 12, 13], title=产品十三, property=二}}]
    }
    profile: null
}
```

<!-- request TypeScript -->
```typescript
res =  await searchApi.search({
  index: 'test',
  query: { match_all:{} },
  aggs: {
    name_group: {
      terms: { field : 'name' }
    },
    cat_group: {
      terms: { field: 'cat' }
    }
  }
});
```
<!-- response TypeScript -->
```typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 5,
    "hits": [
      {
        "_id": 1,
        "_score": 1,
        "_source": {
          "content": "文本 1",
          "name": "文档 1",
          "cat": 1
        }
      },
 ...
      {
        "_id": 5,
        "_score": 1,
        "_source": {
          "content": "文本 5",
          "name": "文档 5",
          "cat": 4
        }
      }
    ]
  },
  "aggregations": {
    "name_group": {
      "buckets": [
        {
          "key": "文档 1",
          "doc_count": 1
        },
...
        {
          "key": "文档 5",
          "doc_count": 1
        }
      ]
    },
    "cat_group": {
      "buckets": [
        {
          "key": 1,
          "doc_count": 2
        },
...        
        {
          "key": 4,
          "doc_count": 1
        }
      ]
    }
  }
}
```

<!-- request Go -->
```go
query := map[string]interface{} {}
searchRequest.SetQuery(query)

aggByName := manticoreclient.NewAggregation()
aggTerms := manticoreclient.NewAggregationTerms()
aggTerms.SetField("name")
aggByName.SetTerms(aggTerms)
aggByCat := manticoreclient.NewAggregation()
aggTerms.SetField("cat")
aggByCat.SetTerms(aggTerms)
aggs := map[string]Aggregation{} { "name_group": aggByName, "cat_group": aggByCat }
searchRequest.SetAggs(aggs)

res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```
<!-- response Go -->
```go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 5,
    "hits": [
      {
        "_id": 1,
        "_score": 1,
        "_source": {
          "content": "文本 1",
          "name": "文档 1",
          "cat": 1
        }
      },
 ...
      {
        "_id": 5,
        "_score": 1,
        "_source": {
          "content": "文本 5",
          "name": "文档 5",
          "cat": 4
        }
      }
    ]
  },
  "aggregations": {
    "name_group": {
      "buckets": [
        {
          "key": "文档 1",
          "doc_count": 1
        },
...
        {
          "key": "文档 5",
          "doc_count": 1
        }
      ]
    },
    "cat_group": {
      "buckets": [
        {
          "key": 1,
          "doc_count": 2
        },
...        
        {
          "key": 4,
          "doc_count": 1
        }
      ]
    }
  }
}
```

<!-- end -->

<!-- example Another_attribute -->

### 通过另一个属性的聚合进行分面

数据可以通过聚合另一个属性或表达式进行分面。例如，如果文档同时包含品牌 ID 和名称，我们可以返回品牌名称的分面，但聚合品牌 ID。这可以通过使用 `FACET {expr1} BY {expr2}` 来完成。


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
|    1 |   306 |        1 | 产品十三           | 品牌一      | 六_十       | {"prop1":66,"prop2":91,"prop3":"一"} | 10,11      |
|    2 |   400 |       10 | 产品三一           | 品牌十      | 四_三       | {"prop1":69,"prop2":19,"prop3":"一"} | 13,14      |
....
|   19 |   855 |        1 | 产品七二           | 品牌一      | 八_七       | {"prop1":63,"prop2":78,"prop3":"一"} | 10,11,12   |
|   20 |    31 |        9 | 产品四一           | 品牌九      | 十_四       | {"prop1":79,"prop2":42,"prop3":"一"} | 12,13,14   |
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
20 rows in set (0.00 sec)
+-------------+----------+
| brand_name  | count(*) |
+-------------+----------+
| 品牌一      |     1013 |
| 品牌十      |      998 |
| 品牌五      |     1007 |
| 品牌九      |      944 |
| 品牌二      |      990 |
| 品牌六      |     1039 |
| 品牌三   |     1016 |
| 品牌四   |      994 |
| 品牌八   |     1033 |
| 品牌七   |      965 |
+-------------+----------+
10 rows in set (0.00 sec)
```

<!-- end -->

<!-- example Distinct -->

### 无重复的分面

如果您需要从FACET返回的桶中删除重复项，可以使用 `DISTINCT field_name`，其中 `field_name` 是您想要执行去重的字段。如果您对分布式表进行FACET查询并且不确定表中是否有唯一的id（表应该是本地的并且具有相同的模式），那么它也可以是 `id`（这也是默认值）。

如果在您的查询中有多个FACET声明，`field_name` 应该在它们之间保持一致。

`DISTINCT` 在 `count(*)` 列之前返回一个额外的列 `count(distinct ...)`，允许您在不需要另行查询的情况下获得这两种结果。

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
| 品牌九    | 四       |
| 品牌十    | 四       |
| 品牌一    | 五       |
| 品牌七    | 九       |
| 品牌七    | 七       |
| 品牌三    | 七       |
| 品牌九    | 五       |
| 品牌三    | 八       |
| 品牌二    | 八       |
| 品牌六    | 八       |
| 品牌十    | 四       |
| 品牌十    | 二       |
| 品牌四    | 十       |
| 品牌一    | 九       |
| 品牌四    | 八       |
| 品牌九    | 七       |
| 品牌四    | 五       |
| 品牌三    | 四       |
| 品牌四    | 二       |
| 品牌四    | 八       |
+-------------+----------+
20 rows in set (0.00 sec)

+-------------+--------------------------+----------+
| brand_name  | count(distinct property) | count(*) |
+-------------+--------------------------+----------+
| 品牌九    |                        3 |        3 |
| 品牌十    |                        2 |        3 |
| 品牌一    |                        2 |        2 |
| 品牌七    |                        2 |        2 |
| 品牌三    |                        3 |        3 |
| 品牌二    |                        1 |        1 |
| 品牌六    |                        1 |        1 |
| 品牌四    |                        4 |        5 |
+-------------+--------------------------+----------+
8 rows in set (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /sql -d 'SELECT brand_name, property FROM facetdemo FACET brand_name distinct property'
```

<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 20,
    "total_relation": "eq",
    "hits": [
      {
        "_score": 1,
        "_source": {
          "brand_name": "品牌九",
          "property": "四"
        }
      },
      {
        "_score": 1,
        "_source": {
          "brand_name": "品牌十",
          "property": "四"
        }
      },
 ...
      {
        "_score": 1,
        "_source": {
          "brand_name": "品牌四",
          "property": "八"
        }
      }
    ]
  },
  "aggregations": {
    "brand_name": {
      "buckets": [
        {
          "key": "品牌九",
          "doc_count": 3,
          "count(distinct property)": 3
        },
        {
          "key": "品牌十",
          "doc_count": 3,
          "count(distinct property)": 2
        },
...
        {
          "key": "品牌二",
          "doc_count": 1,
          "count(distinct property)": 1
        },
        {
          "key": "品牌六",
          "doc_count": 1,
          "count(distinct property)": 1
        },
        {
          "key": "品牌四",
          "doc_count": 5,
          "count(distinct property)": 4
        }
      ]
    }
  }
}
```

<!-- end -->

<!-- example Expressions -->
### 根据表达式分面

分面可以在表达式上进行聚合。一个经典的例子是按特定范围对价格进行分段：

<!-- request SQL -->

```sql
SELECT * FROM facetdemo FACET INTERVAL(price,200,400,600,800) AS price_range ;
```

<!-- response SQL -->

```sql
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+-------------+
| id   | price | brand_id | title               | brand_name  | property    | j                                     | categories | price_range |
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+-------------+
|    1 |   306 |        1 | Product Ten Three   | 品牌一     | 六至十     | {"prop1":66,"prop2":91,"prop3":"一"} | 10,11      |           1 |
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
     "table": "facetdemo",
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
        "_id": 1,
        "_score": 1,
        "_source": {
          "price": 197,
          "brand_id": 10,
          "brand_name": "品牌十",
          "categories": [
            10
          ],
          "price_range": 0
        }
      },
 ...
      {
        "_id": 20,
        "_score": 1,
        "_source": {
          "price": 227,
          "brand_id": 3,
          "brand_name": "品牌三",
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
res =searchApi.search({"table":"facetdemo","query":{"match_all":{}},"expressions":{"price_range":"INTERVAL(price,200,400,600,800)"},"aggs":{"group_property":{"terms":{"field":"price_range"}}}})
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
                                 u'brand_name': u'品牌十',
                                 u'categories': [10],
                                 u'price': 197,
                                 u'property': u'六',
                                 u'title': u'产品八一'}},
                   {u'_id': u'2',
                    u'_score': 1,
                    u'_source': {u'brand_id': 6,
                                 u'brand_name': u'品牌六',
                                 u'categories': [12, 13, 14],
                                 u'price': 671,
                                 u'property': u'四',
                                 u'title': u'产品九七'}},
                   {u'_id': u'3',
                    u'_score': 1,
                    u'_source': {u'brand_id': 3,
                                 u'brand_name': u'品牌三',
                                 u'categories': [13, 14, 15],
                                 u'price': 92,
                                 u'property': u'六',
                                 u'title': u'产品五四'}},
                   {u'_id': u'4',
                    u'_score': 1,
                    u'_source': {u'brand_id': 10,
                                 u'brand_name': u'品牌十',
                                 u'categories': [11],
                                 u'price': 713,
                                 u'属性': u'五',
                                 u'标题': u'产品八九'}},
                   {u'_id': u'5',
                    u'_score': 1,
                    u'_source': {u'品牌_id': 7,
                                 u'品牌名称': u'品牌七',
                                 u'类别': [11, 12, 13],
                                 u'价格': 805,
                                 u'属性': u'二',
                                 u'标题': u'产品十三'}}],
          '最大得分': None,
          '总数': 10000},
 '档案': None,
 '超时': False,
 '耗时': 0}
```

<!-- request Python-asyncio -->
```python
res = await searchApi.search({"table":"facetdemo","query":{"match_all":{}},"expressions":{"price_range":"INTERVAL(price,200,400,600,800)"},"aggs":{"group_property":{"terms":{"field":"price_range"}}}})
```
<!-- response Python-asyncio -->
```python
{'聚合': {u'品牌_id分组': {u'桶': [{u'文档数量': 1019,
                                                    u'键': 10},
                                                   {u'文档数量': 954,
                                                    u'键': 9},
                                                   {u'文档数量': 1021,
                                                    u'键': 8},
                                                   {u'文档数量': 1011,
                                                    u'键': 7},
                                                   {u'文档数量': 997,
                                                    u'键': 6}]},
                  u'属性分组': {u'桶': [{u'文档数量': 11,
                                                    u'键': 1000},
                                                   {u'文档数量': 12,
                                                    u'键': 999},
                                                   {u'文档数量': 7,
                                                    u'键': 998},
                                                   {u'文档数量': 14,
                                                    u'键': 997},
                                                   {u'文档数量': 8,
                                                    u'键': 996}]}},
 '命中': {'命中': [{u'_id': u'1',
                    u'_score': 1,
                    u'_source': {u'品牌_id': 10,
                                 u'品牌名称': u'品牌十',
                                 u'类别': [10],
                                 u'价格': 197,
                                 u'属性': u'六',
                                 u'标题': u'产品八一'}},
                   {u'_id': u'2',
                    u'_score': 1,
                    u'_source': {u'品牌_id': 6,
                                 u'品牌名称': u'品牌六',
                                 u'类别': [12, 13, 14],
                                 u'价格': 671,
                                 u'属性': u'四',
                                 u'标题': u'产品九七'}},
                   {u'_id': u'3',
                    u'_score': 1,
                    u'_source': {u'品牌_id': 3,
                                 u'品牌名称': u'品牌三',
                                 u'类别': [13, 14, 15],
                                 u'价格': 92,
                                 u'属性': u'六',
                                 u'标题': u'产品五四'}},
                   {u'_id': u'4',
                    u'_score': 1,
                    u'_source': {u'品牌_id': 10,
                                 u'品牌名称': u'品牌十',
                                 u'类别': [11],
                                 u'价格': 713,
                                 u'属性': u'五',
                                 u'标题': u'产品八九'}},
                   {u'_id': u'5',
                    u'_score': 1,
                    u'_source': {u'品牌_id': 7,
                                 u'品牌名称': u'品牌七',
                                 u'类别': [11, 12, 13],
                                 u'价格': 805,
                                 u'属性': u'二',
                                 u'标题': u'产品十三'}}],
          '最大得分': None,
          '总数': 10000},
 '档案': None,
 '超时': False,
 '耗时': 0}
```

<!-- request Javascript -->
```javascript
res =  await searchApi.search({"table":"facetdemo","query":{"match_all":{}},"expressions":{"price_range":"INTERVAL(price,200,400,600,800)"},"aggs":{"group_property":{"terms":{"field":"price_range"}}}});
```
<!-- response Javascript -->
```javascript
{"took":0,"timed_out":false,"hits":{"total":10000,"hits":[{"_id": 1,"_score":1,"_source":{"price":197,"brand_id":10,"brand_name":"品牌十","categories":[10],"title":"产品八一","property":"六","price_range":0}},{"_id": 2,"_score":1,"_source":{"price":671,"brand_id":6,"brand_name":"品牌六","categories":[12,13,14],"title":"产品九七","property":"四","price_range":3}},{"_id": 3,"_score":1,"_source":{"price":92,"brand_id":3,"brand_name":"品牌三","categories":[13,14,15],"title":"产品五四","property":"六","price_range":0}},{"_id": 4,"_score":1,"_source":{"price":713,"brand_id":10,"brand_name":"品牌十","categories":[11],"title":"产品八九","property":"五","price_range":3}},{"_id": 5,"_score":1,"_source":{"price":805,"brand_id":7,"brand_name":"品牌七","categories":[11,12,13],"title":"产品十三","property":"二","price_range":4}},{"_id": 6,"_score":1,"_source":{"price":420,"brand_id":2,"brand_name":"品牌二","categories":[10,11],"title":"产品二一","property":"六","price_range":2}},{"_id": 7,"_score":1,"_source":{"price":412,"brand_id":9,"brand_name":"品牌九","categories":[10],"title":"产品四九","property":"八","price_range":2}},{"_id": 8,"_score":1,"_source":{"price":300,"brand_id":9,"brand_name":"品牌九","categories":[13,14,15],"title":"产品八四","property":"五","price_range":1}},{"_id": 9,"_score":1,"_source":{"price":728,"brand_id":1,"brand_name":"品牌一","categories":[11],"title":"产品九六","property":"四","price_range":3}},{"_id": 10,"_score":1,"_source":{"price":622,"brand_id":3,"brand_name":"品牌三","categories":[10,11],"title":"产品六七","property":"二","price_range":3}},{"_id": 11,"_score":1,"_source":{"price":462,"brand_id":5,"brand_name":"品牌五","categories":[10,11],"title":"产品十二","property":"八","price_range":2}},{"_id": 12,"_score":1,"_source":{"price":939,"brand_id":7,"brand_name":"品牌七","categories":[12,13],"title":"产品九七","property":"六","price_range":4}},{"_id": 13,"_score":1,"_source":{"price":948,"brand_id":8,"brand_name":"品牌八","categories":[12],"title":"产品十一","property":"六","price_range":4}},{"_id": 14,"_score":1,"_source":{"price":900,"brand_id":9,"brand_name":"品牌九","categories":[12,13,14],"title":"产品十九","property":"三","price_range":4}},{"_id": 15,"_score":1,"_source":{"price":224,"brand_id":3,"brand_name":"品牌三","categories":[13],"title":"产品二六","property":"四","price_range":1}},{"_id": 16,"_score":1,"_source":{"price":713,"brand_id":10,"brand_name":"品牌十","categories":[12],"title":"产品二四","property":"六","price_range":3}},{"_id": 17,"_score":1,"_source":{"price":510,"brand_id":2,"brand_name":"品牌二","categories":[10],"title":"产品十二","property":"七","price_range":2}},{"_id": 18,"_score":1,"_source":{"price":702,"brand_id":10,"brand_name":"品牌十","categories":[12,13],"title":"产品九一","property":"三","price_range":3}},{"_id": 19,"_score":1,"_source":{"price":836,"brand_id":4,"brand_name":"品牌四","categories":[10,11,12],"title":"产品四五","property":"二","price_range":4}},{"_id": 20,"_score":1,"_source":{"price":227,"brand_id":3,"brand_name":"品牌三","categories":[12,13],"title":"产品三四","property":"十","price_range":1}}]}}
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
        put("terms", new HashMap<String,Object>(){{
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
        hits: [{_id=1, _score=1, _source={price=197, brand_id=10, brand_name=品牌十, categories=[10], title=产品八一, property=六, price_range=0}}, {_id=2, _score=1, _source={price=671, brand_id=6, brand_name=品牌六, categories=[12, 13, 14], title=产品九七, property=四, price_range=3}}, {_id=3, _score=1, _source={price=92, brand_id=3, brand_name=品牌三, categories=[13, 14, 15], title=产品五四, property=六, price_range=0}}, {_id=4, _score=1, _source={price=713, brand_id=10, brand_name=品牌十, categories=[11], title=产品八九, property=五, price_range=3}}, {_id=5, _score=1, _source={price=805, brand_id=7, brand_name=品牌七, categories=[11, 12, 13], title=产品十三, property=二, price_range=4}}]
    }
    profile: null
}
```

<!-- request C# -->
```clike
var expr = new Dictionary<string, string> { {"price_range", "INTERVAL(price,200,400,600,800"} } ;
var agg = new Aggregation("group_property", "price_range");
object query = new { match_all=null };
var searchRequest = new SearchRequest("facetdemo", query);
searchRequest.Limit = 5;
searchRequest.Expressions = new List<Object> {expr};
searchRequest.Aggs = new List<Aggregation> {agg};
var searchResponse = searchApi.Search(searchRequest);

```
<!-- response C# -->
```clike
class SearchResponse {
    took: 0
    timedOut: false
    aggregations: {group_property={buckets=[{key=4, doc_count=2100}, {key=3, doc_count=1973}, {key=2, doc_count=1999}, {key=1, doc_count=2043}, {key=0, doc_count=1885}]}}
    hits: class SearchResponseHits {
        maxScore: null
        total: 10000
        hits: [{_id=1, _score=1, _source={price=197, brand_id=10, brand_name=品牌十, categories=[10], title=产品八一, property=六, price_range=0}}, {_id=2, _score=1, _source={price=671, brand_id=6, brand_name=品牌六, categories=[12, 13, 14], title=产品九七, property=四, price_range=3}}, {_id=3, _score=1, _source={price=92, brand_id=3, brand_name=品牌三, categories=[13, 14, 15], title=产品五四, property=六, price_range=0}}, {_id=4, _score=1, _source={price=713, brand_id=10, brand_name=品牌十, categories=[11], title=产品八九, property=五, price_range=3}}, {_id=5, _score=1, _source={price=805, brand_id=7, brand_name=品牌七, categories=[11, 12, 13], title=产品十三, property=二, price_range=4}}]
    }
    profile: null
}
```

<!-- request Rust -->
```rust
let query = SearchQuery::new();
let aggTerms1 = AggTerms::new("price_range");
let agg1 = Aggregation {
    terms: Some(Box::new(aggTerms1)),
    ..Default::default(),
};
let mut aggs = HashMap::new();
aggs.insert("group_property".to_string(), agg1);
let mut expr = HashMap::new(); 
expr.insert("price_range".to_string(), "INTERVAL(price,200,400,600,800");
let expressions: [HashMap; 1] = [expr];

let search_req = SearchRequest {
    table: "facetdemo".to_string(),
    query: Some(Box::new(query)),
    expressions: serde_json::json!(expressions),
    aggs: serde_json::json!(aggs),
    limit: serde_json::json!(5),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```
<!-- response Rust -->
```rust
class SearchResponse {
    took: 0
    timedOut: false
    aggregations: {group_property={buckets=[{key=4, doc_count=2100}, {key=3, doc_count=1973}, {key=2, doc_count=1999}, {key=1, doc_count=2043}, {key=0, doc_count=1885}]}}
    hits: class SearchResponseHits {
        maxScore: null
        total: 10000
        hits: [{_id=1, _score=1, _source={price=197, brand_id=10, brand_name=品牌十, categories=[10], title=产品八一, property=六, price_range=0}}, {_id=2, _score=1, _source={price=671, brand_id=6, brand_name=品牌六, categories=[12, 13, 14], title=产品九七, property=四, price_range=3}}, {_id=3, _score=1, _source={price=92, brand_id=3, brand_name=品牌三, categories=[13, 14, 15], title=产品五四, property=六, price_range=0}}, {_id=4, _score=1, _source={price=713, brand_id=10, brand_name=品牌十, categories=[11], title=产品八九, property=五, price_range=3}}, {_id=5, _score=1, _source={price=805, brand_id=7, brand_name=品牌七, categories=[11, 12, 13], title=产品十三, property=二, price_range=4}}]
    }
    profile: null
}
```

<!-- request TypeScript -->
```typecript
res =  await searchApi.search({
  index: 'test',
  query: { match_all:{} },
  expressions: { cat_range: "INTERVAL(cat,1,3)" }
  aggs: {
    expr_group: {
      terms: { field : 'cat_range' }
    }
  }
});
```

<!-- response TypeScript -->
```typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 5,
    "hits": [
      {
        "_id": 1,
        "_score": 1,
        "_source": {
          "content": "文本 1",
          "name": "文档 1",
          "cat": 1,
          "cat_range": 1
        }
      },
 ...
      {
        "_id": 5,
        "_score": 1,
        "_source": {
          "content": "文本 5",
          "name": "文档 5",
          "cat": 4,
          "cat_range": 2,
        }
      }
    ]
  },
  "aggregations": {
    "expr_group": {
      "buckets": [
        {
          "key": 0,
          "doc_count": 0
        },

{
          "key": 1,
          "doc_count": 3
        },
        {
          "key": 2,
          "doc_count": 2
        }
      ]
    }
  }
}
```

<!-- request Go -->
```go
query := map[string]interface{} {}
searchRequest.SetQuery(query)

exprs := map[string]string{} { "cat_range": "INTERVAL(cat,1,3)" }
searchRequest.SetExpressions(exprs)

aggByExpr := manticoreclient.NewAggregation()
aggTerms := manticoreclient.NewAggregationTerms()
aggTerms.SetField("cat_range")
aggByExpr.SetTerms(aggTerms)
aggs := map[string]Aggregation{} { "expr_group": aggByExpr }
searchRequest.SetAggs(aggs)

res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```

<!-- response Go -->
```go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 5,
    "hits": [
      {
        "_id": 1,
        "_score": 1,
        "_source": {
          "content": "文本 1",
          "name": "文档 1",
          "cat": 1,
          "cat_range": 1
        }
      },
 ...
      {
        "_id": 5,
        "_score": 1,
        "_source": {
          "content": "文本 5",
          "name": "文档 5",
          "cat": 4,
          "cat_range": 2
        }
      }
    ]
  },
  "aggregations": {
    "expr_group": {
      "buckets": [
        {
          "key": 0,
          "doc_count": 0
        },

{
          "key": 1,
          "doc_count": 3
        },
        {
          "key": 2,
          "doc_count": 2
        }
      ]
    }
  }
}
```

<!-- end -->

<!-- example Multi-level -->

### Facet over multi-level grouping

Facets can aggregate over multi-level grouping, with the result set being the same as if the query performed a multi-level grouping:

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

<!-- example histogram -->

### 按直方图值进行分面

分面可以通过在值上构建固定大小的桶来汇总直方图值。
关键函数是：

```sql
key_of_the_bucket = interval + offset * floor ( ( value - offset ) / interval )
```

直方图参数 `interval` 必须为正，直方图参数 `offset` 必须为正且小于 `interval`。默认情况下，桶作为数组返回。直方图参数 `keyed` 使响应成为一个带有桶键的字典。

<!-- request SQL -->

```sql
SELECT COUNT(*), HISTOGRAM(price, {hist_interval=100}) as price_range FROM facets GROUP BY price_range ORDER BY price_range ASC;
```

<!-- response SQL -->

```sql
+----------+-------------+
| count(*) | price_range |
+----------+-------------+
|        5 |           0 |
|        5 |         100 |
|        1 |         300 |
|        4 |         400 |
|        1 |         500 |
|        3 |         700 |
|        1 |         900 |
+----------+-------------+
```
<!-- request JSON -->

``` json
POST /search -d '
{
  "size": 0,
  "table": "facets",
  "aggs": {
    "price_range": {
      "histogram": {
        "field": "price",
        "interval": 300
      }
    }
  }
}'
```

<!-- response JSON -->
``` json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 20,
    "total_relation": "eq",
    "hits": []
  },
  "aggregations": {
    "price_range": {
      "buckets": [
        {
          "key": 0,
          "doc_count": 10
        },
        {
          "key": 300,
          "doc_count": 6
        },
        {
          "key": 600,
          "doc_count": 3
        },
        {
          "key": 900,
          "doc_count": 1
        }
      ]
    }
  }
}
```
<!-- request JSON 2 -->

``` json
POST /search -d '
{
  "size": 0,
  "table": "facets",
  "aggs": {
    "price_range": {
      "histogram": {
        "field": "price",
        "interval": 300,
        "keyed": true
      }
    }
  }
}'
```

<!-- response JSON 2 -->
``` json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 20,
    "total_relation": "eq",
    "hits": []
  },
  "aggregations": {
    "price_range": {
      "buckets": {
        "0": {
          "key": 0,
          "doc_count": 10
        },
        "300": {
          "key": 300,
          "doc_count": 6
        },
        "600": {
          "key": 600,
          "doc_count": 3
        },
        "900": {
          "key": 900,
          "doc_count": 1
        }
      }
    }
  }
}
```
<!-- end -->

<!-- example histogram_date -->

### 按直方图日期值进行分面

分面可以对直方图日期值进行汇总，这与普通直方图类似。不同之处在于间隔使用日期或时间表达式来指定。此类表达式需要特殊支持，因为间隔不一定是固定长度。使用以下关键函数将值舍入到最接近的桶：

```sql
key_of_the_bucket = interval * floor ( value / interval )
```

直方图参数 `calendar_interval` 理解月份有不同的天数。
与 `calendar_interval` 不同，`fixed_interval` 参数使用固定数量的单位，并且不偏离，无论它在日历的哪个位置。然而 `fixed_interval` 不能处理像月份这样的单位，因为一个月不是一个固定量。尝试为 `fixed_interval` 指定像周或月这样的单位将导致错误。
可接受的间隔在 [date_histogram](../Functions/Date_and_time_functions.md#DATE_HISTOGRAM%28%29) 表达式中描述。默认情况下，桶作为数组返回。直方图参数 `keyed` 使响应成为一个带有桶键的字典。

<!-- request SQL -->

```sql
SELECT count(*), DATE_HISTOGRAM(tm, {calendar_interval='month'}) AS months FROM idx_dates GROUP BY months ORDER BY months ASC
```

<!-- response SQL -->

```sql
+----------+------------+
| count(*) | months     |
+----------+------------+
|      442 | 1485907200 |
|      744 | 1488326400 |
|      720 | 1491004800 |
|      230 | 1493596800 |
+----------+------------+
```
<!-- request JSON -->

``` json
POST /search -d '
{
  "table": "idx_dates",
  "size": 0,
  "aggs": {
    "months": {
      "date_histogram": {
        "field": "tm",
        "keyed": true,
        "calendar_interval": "month"
      }
    }
  }
}'
```

<!-- response JSON -->
``` json
{
    "timed_out": false,
    "hits": {
        "total": 2136,
        "total_relation": "eq",
        "hits": []
    },
    "aggregations": {
        "months": {
            "buckets": {
                "2017-02-01T00:00:00": {
                    "key": 1485907200,
                    "key_as_string": "2017-02-01T00:00:00",
                    "doc_count": 442
                },
                "2017-03-01T00:00:00": {
                    "key": 1488326400,
                    "key_as_string": "2017-03-01T00:00:00",
                    "doc_count": 744
                },
                "2017-04-01T00:00:00": {
                    "key": 1491004800,
                    "key_as_string": "2017-04-01T00:00:00",
                    "doc_count": 720
                },
                "2017-05-01T00:00:00": {
                    "key": 1493596800,
                    "key_as_string": "2017-05-01T00:00:00",
                    "doc_count": 230
                }
            }
        }
    }
}
```
<!-- end -->


<!-- example facet range -->

### Facet over set of ranges

Facets can aggregate over a set of ranges. The values are checked against the bucket range, where each bucket includes the `from` value and excludes the `to` value from the range.
Setting the `keyed` property to `true` makes the response a dictionary with the bucket keys rather than an array.

<!-- request SQL -->

```sql
SELECT COUNT(*), RANGE(price, {range_to=150},{range_from=150,range_to=300},{range_from=300}) price_range FROM facets GROUP BY price_range ORDER BY price_range ASC;
```

<!-- response SQL -->

```sql
+----------+-------------+
| count(*) | price_range |
+----------+-------------+
|        8 |           0 |
|        2 |           1 |
|       10 |           2 |
+----------+-------------+
```
<!-- request JSON -->

``` json
POST /search -d '
{
  "size": 0,
  "table": "facets",
  "aggs": {
    "price_range": {
      "range": {
        "field": "price",
        "ranges": [
          {
            "to": 99
          },
          {
            "from": 99,
            "to": 550
          },
          {
            "from": 550
          }
        ]
      }
    }
  }
}'
```

<!-- response JSON -->
``` json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 20,
    "total_relation": "eq",
    "hits": []
  },
  "aggregations": {
    "price_range": {
      "buckets": [
        {
          "key": "*-99",
          "to": 99,
          "doc_count": 5
        },
        {
          "key": "99-550",
          "from": 99,
          "to": 550,
          "doc_count": 11
        },
        {
          "key": "550-*",
          "from": 550,
          "doc_count": 4
        }
      ]
    }
  }
}
```
<!-- request JSON 2 -->

``` json
POST /search -d '
{
  "size":0,
  "table":"facets",
  "aggs":{
    "price_range":{
      "range":{
        "field":"price",
        "keyed":true,
        "ranges":[
          {
            "from":100,
            "to":399
          },
          {
            "from":399
          }
        ]
      }
    }
  }
}'
```

<!-- response JSON 2 -->
``` json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 20,
    "total_relation": "eq",
    "hits": []
  },
  "aggregations": {
    "price_range": {
      "buckets": {
        "100-399": {
          "from": 100,
          "to": 399,
          "doc_count": 6
        },
        "399-*": {
          "from": 399,
          "doc_count": 9
        }
      }
    }
  }
}
```
<!-- end -->

<!-- example facet range_date -->

### Facet over set of date ranges

Facets can aggregate over a set of date ranges, which is similar to the normal range. The difference is that the `from` and `to` values can be expressed in [Date math](../Functions/Date_and_time_functions.md#Date-math) expressions. This aggregation includes the `from` value and excludes the `to` value for each range. Setting the `keyed` property to `true` makes the response a dictionary with the bucket keys rather than an array.

<!-- request SQL -->

```sql
SELECT COUNT(*), DATE_RANGE(tm, {range_to='2017||+2M/M'},{range_from='2017||+2M/M',range_to='2017||+5M/M'},{range_from='2017||+5M/M'}) AS points FROM idx_dates GROUP BY points ORDER BY points ASC;
```

<!-- response SQL -->

```sql
+----------+--------+
| count(*) | points |
+----------+--------+
|      442 |      0 |
|     1464 |      1 |
|      230 |      2 |
+----------+--------+
```
<!-- request JSON -->

``` json
POST /search -d '
{
  "table": "idx_dates",
  "size": 0,
  "aggs": {
    "points": {
      "date_range": {
        "field": "tm",
        "keyed": true,
        "ranges": [
          {
            "to": "2017||+2M/M"
          },
          {
            "from": "2017||+2M/M",
            "to": "2017||+4M/M"
          },
          {
            "from": "2017||+4M/M",
            "to": "2017||+5M/M"
          },
          {
            "from": "2017||+5M/M"
          }
        ]
      }
    }
  }
}'
```

<!-- response JSON -->
``` json
{
    "timed_out": false,
    "hits": {
        "total": 2136,
        "total_relation": "eq",
        "hits": []
    },
    "aggregations": {
        "points": {
            "buckets": {
                "*-2017-03-01T00:00:00": {
                    "to": "2017-03-01T00:00:00",
                    "doc_count": 442
                },
                "2017-03-01T00:00:00-2017-04-01T00:00:00": {
                    "from": "2017-03-01T00:00:00",
                    "to": "2017-04-01T00:00:00",
                    "doc_count": 744
                },
                "2017-04-01T00:00:00-2017-05-01T00:00:00": {
                    "from": "2017-04-01T00:00:00",
                    "to": "2017-05-01T00:00:00",
                    "doc_count": 720
                },
                "2017-05-01T00:00:00-*": {
                    "from": "2017-05-01T00:00:00",
                    "doc_count": 230
                }
            }
        }
    }
}
```
<!-- end -->

<!-- example Ordering -->
### Facet结果中的排序

Facets支持`ORDER BY`子句，就像标准查询一样。每个facet可以有自己的排序方式，facet排序不影响主结果集的排序，主结果集的排序由主查询的`ORDER BY`决定。可以根据属性名称、计数（使用`COUNT(*)`，`COUNT(DISTINCT attribute_name)`）或特殊的`FACET()`函数进行排序，该函数提供聚合数据值。默认情况下，使用`ORDER BY COUNT(*)`的查询将按降序排序。


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT * FROM facetdemo
FACET brand_name BY brand_id ORDER BY FACET() ASC
FACET brand_name BY brand_id ORDER BY brand_name ASC
FACET brand_name BY brand_id order BY COUNT(*) DESC;
FACET brand_name BY brand_id order BY COUNT(*);
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

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /search -d '
{
   "table":"table_name",
   "aggs":{
      "group_property":{
         "terms":{
            "field":"a"
         },
         "sort":[
            {
               "count(*)":{
                  "order":"desc"
               }
            }
         ]
      }
   }
}'

```

<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 6,
    "total_relation": "eq",
    "hits": [
      {
        "_id": 1515697460415037554,
        "_score": 1,
        "_source": {
          "a": 1
        }
      },
      {
        "_id": 1515697460415037555,
        "_score": 1,
        "_source": {
          "a": 2
        }
      },
      {
        "_id": 1515697460415037556,
        "_score": 1,
        "_source": {
          "a": 2
        }
      },
      {
        "_id": 1515697460415037557,
        "_score": 1,
        "_source": {
          "a": 3
        }
      },
      {
        "_id": 1515697460415037558,
        "_score": 1,
        "_source": {
          "a": 3
        }
      },
      {
        "_id": 1515697460415037559,
        "_score": 1,
        "_source": {
          "a": 3
        }
      }
    ]
  },
  "aggregations": {
    "group_property": {
      "buckets": [
        {
          "key": 3,
          "doc_count": 3
        },
        {
          "key": 2,
          "doc_count": 2
        },
        {
          "key": 1,
          "doc_count": 1
        }
      ]
    }
  }
}
```

<!-- end -->


<!-- example Size -->
### Facet 结果大小

默认情况下，每个 facet 结果集限制为 20 个值。可以通过提供返回值的数量，格式为 `LIMIT count`，或通过偏移量 `LIMIT offset, count` 来单独控制每个 facet 的值的数量。

可以返回的最大 facet 值受到查询的 `max_matches` 设置的限制。如果您想实现动态的 `max_matches` （将 `max_matches` 限制为偏移量 + 每页以获得更好的性能），必须考虑到 `max_matches` 值过低会影响 facet 值的数量。在这种情况下，应使用一个足够覆盖 facet 值数量的最小 `max_matches` 值。

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
|    1 |   306 |        1 | 产品十三           | 品牌一      | 六十       | {"prop1":66,"prop2":91,"prop3":"一"} | 10,11      |
...
|   20 |    31 |        9 | 产品四一           | 品牌九      | 十四       | {"prop1":79,"prop2":42,"prop3":"一"} | 12,13,14   |
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
20 行在集合中（0.01秒）

+-------------+----------+
| brand_name  | count(*) |
+-------------+----------+
| 品牌一      |     1013 |
+-------------+----------+
1 行在集合中（0.01秒）

+-------------+----------+
| brand_name  | count(*) |
+-------------+----------+
| 品牌四      |      994 |
| 品牌九      |      944 |
| 品牌一      |     1013 |
| 品牌七      |      965 |
+-------------+----------+
4 行在集合中（0.01秒）

+-------------+----------+
| brand_name  | count(*) |
+-------------+----------+
| 品牌六      |     1039 |
| 品牌八      |     1033 |
| 品牌三      |     1016 |
+-------------+----------+
3 行在集合中（0.01秒）
```
<!-- request JSON -->

``` json
POST /search -d '
    {
     "table" : "facetdemo",
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
        "_id": 1,
        "_score": 1,
        "_source": {
          "price": 197,
          "brand_id": 10,
          "brand_name": "品牌十",
          "categories": [
            10
          ]
        }
      },
 ...
      {
        "_id": 5,
        "_score": 1,
        "_source": {
          "price": 805,
          "brand_id": 7,
          "brand_name": "品牌七",
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
res =searchApi.search({"table":"facetdemo","query":{"match_all":{}},"limit":5,"aggs":{"group_property":{"terms":{"field":"price","size":1,}},"group_brand_id":{"terms":{"field":"brand_id","size":3}}}})
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
                                 u'brand_name': u'品牌十',
                                 u'categories': [10],
                                 u'price': 197,
                                 u'property': u'六',
                                 u'title': u'产品八一'}},
                   {u'_id': u'2',
                    u'_score': 1,
                    u'_source': {u'brand_id': 6,
                                 u'brand_name': u'品牌六',
                                 u'categories': [12, 13, 14],
                                 u'price': 671,
                                 u'property': u'四',
                                 u'title': u'产品九七'}},
                   {u'_id': u'3',
                    u'_score': 1,
                    u'_source': {u'brand_id': 3,
                                 u'brand_name': u'品牌三',
                                 u'categories': [13, 14, 15],
                                 u'price': 92,
                                 u'property': u'六',
                                 u'title': u'产品五四'}},
                   {u'_id': u'4',
                    u'_score': 1,
                    u'_source': {u'brand_id': 10,
                                 u'brand_name': u'品牌十',
                                 u'categories': [11],
                                 u'price': 713,
                                 u'property': u'五',
                                 u'title': u'产品八九'}},
                   {u'_id': u'5',
                    u'_score': 1,
                    u'_source': {u'brand_id': 7,
                                 u'brand_name': u'品牌七',
                                 u'categories': [11, 12, 13],
                                 u'price': 805,
                                 u'property': u'二',
                                 u'title': u'产品十三'}}],
          'max_score': None,
          'total': 10000},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Python-asyncio -->
```python
res = await searchApi.search({"table":"facetdemo","query":{"match_all":{}},"limit":5,"aggs":{"group_property":{"terms":{"field":"price","size":1,}},"group_brand_id":{"terms":{"field":"brand_id","size":3}}}})
```
<!-- response Python-asyncio -->
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
                                 u'brand_name': u'品牌十',
                                 u'categories': [10],
                                 u'price': 197,
                                 u'property': u'六',
                                 u'title': u'产品八一'}},
                   {u'_id': u'2',
                    u'_score': 1,
                    u'_source': {u'brand_id': 6,
                                 u'brand_name': u'品牌六',
                                 u'categories': [12, 13, 14],
                                 u'price': 671,
                                 u'property': u'四',
                                 u'title': u'产品九七'}},
                   {u'_id': u'3',
                    u'_score': 1,
                    u'_source': {u'brand_id': 3,
                                 u'brand_name': u'品牌三',
                                 u'categories': [13, 14, 15],
                                 u'price': 92,
                                 u'property': u'六',
                                 u'title': u'产品五四'}},
                   {u'_id': u'4',
                    u'_score': 1,
                    u'_source': {u'brand_id': 10,
                                 u'brand_name': u'品牌十',
                                 u'categories': [11],
                                 u'price': 713,
                                 u'property': u'五',
                                 u'title': u'产品八九'}},
                   {u'_id': u'5',
                    u'_score': 1,
                    u'_source': {u'brand_id': 7,
                                 u'brand_name': u'品牌七',
                                 u'categories': [11, 12, 13],
                                 u'price': 805,
                                 u'property': u'二',
                                 u'title': u'产品十三'}}],
          'max_score': None,
          'total': 10000},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Javascript -->
```javascript
res =  await searchApi.search({"table":"facetdemo","query":{"match_all":{}},"limit":5,"aggs":{"group_property":{"terms":{"field":"price","size":1,}},"group_brand_id":{"terms":{"field":"brand_id","size":3}}}});

```

<!-- response Javascript -->

```javascript

{"took":0,"timed_out":false,"hits":{"total":10000,"hits":[{"_id": 1,"_score":1,"_source":{"price":197,"brand_id":10,"brand_name":"Brand Ten","categories":[10],"title":"Product Eight One","property":"Six"}},{"_id": 2,"_score":1,"_source":{"price":671,"brand_id":6,"brand_name":"Brand Six","categories":[12,13,14],"title":"Product Nine Seven","property":"Four"}},{"_id": 3,"_score":1,"_source":{"price":92,"brand_id":3,"brand_name":"Brand Three","categories":[13,14,15],"title":"Product Five Four","property":"Six"}},{"_id": 4,"_score":1,"_source":{"price":713,"brand_id":10,"brand_name":"Brand Ten","categories":[11],"title":"Product Eight Nine","property":"Five"}},{"_id": 5,"_score":1,"_source":{"price":805,"brand_id":7,"brand_name":"Brand Seven","categories":[11,12,13],"title":"Product Ten Three","property":"Two"}}]}}

```

<!-- request Java -->

```java

searchRequest = new SearchRequest();

aggs = new HashMap<String,Object>(){{

    put("group_property", new HashMap<String,Object>(){{

        put("terms", new HashMap<String,Object>(){{

                put("field","price");

                put("size",1);

});

    }});

    put("group_brand_id", new HashMap<String,Object>(){{

        put("terms", new HashMap<String,Object>(){{

                put("field","brand_id");

                put("size",3);

});

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

<!-- request C# -->

```clike

var agg1 = new Aggregation("group_property", "price");

agg1.Size = 1;

var agg2 = new Aggregation("group_brand_id", "brand_id");

agg2.Size = 3;

agg2.Size = 100;

object query = new { match_all=null };

var searchRequest = new SearchRequest("facetdemo", query);

searchRequest.Aggs = new List<Aggregation> {agg1, agg2};

var searchResponse = searchApi.Search(searchRequest);

```

<!-- response C# -->

```clike

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

<!-- request Rust -->

```rust

let query = SearchQuery::new();

let aggTerms1 = AggTerms {

    field: "price".to_string(),

    size: Some(1),

};

let agg1 = Aggregation {

    terms: Some(Box::new(aggTerms1)),

    ..Default::default(),

};

let aggTerms2 = AggTerms {

    field: "brand_id".to_string(),

    size: Some(3),

};

let agg2 = Aggregation {

    terms: Some(Box::new(aggTerms2)),

    ..Default::default(),

};

let mut aggs = HashMap::new();

aggs.insert("group_property".to_string(), agg1); 

aggs.insert("group_brand_id".to_string(), agg2);

let search_req = SearchRequest {

    table: "facetdemo".to_string(),

    query: Some(Box::new(query)),

    aggs: serde_json::json!(aggs),

    limit: serde_json::json!(5),

    ..Default::default(),

};

let search_res = search_api.search(search_req).await;

```

<!-- response Rust -->

```rust

class SearchResponse {

    took: 0

    timedOut: false

    aggregations: {group_property={buckets=[{key=1000, doc_count=11}]}, group_brand_id={buckets=[{key=10, doc_count=1019}, {key=9, doc_count=954}, {key=8, doc_count=1021}]}}

    hits: class SearchResponseHits {

        maxScore: null

        total: 10000
        hits: [{_id=1, _score=1, _source={price=197, brand_id=10, brand_name=品牌十, categories=[10], title=产品八一, property=六}}, {_id=2, _score=1, _source={price=671, brand_id=6, brand_name=品牌六, categories=[12, 13, 14], title=产品九七, property=四}}, {_id=3, _score=1, _source={price=92, brand_id=3, brand_name=品牌三, categories=[13, 14, 15], title=产品五四, property=六}}, {_id=4, _score=1, _source={price=713, brand_id=10, brand_name=品牌十, categories=[11], title=产品八九, property=五}}, {_id=5, _score=1, _source={price=805, brand_id=7, brand_name=品牌七, categories=[11, 12, 13], title=产品十三, property=二}}]
    }
    profile: null
}
```

<!-- request TypeScript -->
```typescript
res =  await searchApi.search({
  index: 'test',
  query: { match_all:{} },
  aggs: {
    name_group: {
      terms: { field : 'name', size: 1 }
    },
    cat_group: {
      terms: { field: 'cat' }
    }
  }
});
```
<!-- response TypeScript -->
```typescript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 5,
    "hits": [
      {
        "_id": 1,
        "_score": 1,
        "_source": {
          "content": "文本 1",
          "name": "文档 1",
          "cat": 1
        }
      },
 ...
      {
        "_id": 5,
        "_score": 1,
        "_source": {
          "content": "文本 5",
          "name": "文档 5",
          "cat": 4
        }
      }
    ]
  },
  "aggregations": {
    "name_group": {
      "buckets": [
        {
          "key": "文档 1",
          "doc_count": 1
        }
      ]
    },
    "cat_group": {
      "buckets": [
        {
          "key": 1,
          "doc_count": 2
        },
...        
        {
          "key": 4,
          "doc_count": 1
        }
      ]
    }
  }
}
```

<!-- request Go -->
```go
query := map[string]interface{} {}
searchRequest.SetQuery(query)

aggByName := manticoreclient.NewAggregation()
aggTerms := manticoreclient.NewAggregationTerms()
aggTerms.SetField("name")
aggByName.SetTerms(aggTerms)
aggByName.SetSize(1)
aggByCat := manticoreclient.NewAggregation()
aggTerms.SetField("cat")
aggByCat.SetTerms(aggTerms)
aggs := map[string]Aggregation{} { "name_group": aggByName, "cat_group": aggByCat }
searchRequest.SetAggs(aggs)

res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```
<!-- response Go -->
```go
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 5,
    "hits": [
      {
        "_id": 1,
        "_score": 1,
        "_source": {
          "content": "文本 1",
          "name": "文档 1",
          "cat": 1
        }
      },
# 分面搜索

					分面搜索对现代搜索应用程序来说与 [自动完成](../Searching/Autocomplete.md)、[拼写校正](../Searching/Spell_correction.md) 和搜索关键词 [高亮](../Searching/Highlighting.md) 一样重要，尤其是在电子商务产品中。

					![分面搜索](faceted.png)

					分面搜索在处理大量数据和各种互联属性时非常有用，例如尺寸、颜色、制造商或其他因素。在查询大量数据时，搜索结果往往包含许多与用户期望不符的条目。分面搜索使最终用户能够明确定义他们希望搜索结果满足的标准。

					在 Manticore Search 中，有一种优化会保持原始查询的结果集，并在每个分面计算中重用该结果集。由于聚合是应用于已计算的文档子集，因此速度很快，并且总执行时间通常仅比初始查询稍长。可以将分面添加到任何查询中，并且分面可以是任何属性或表达式。分面结果包括分面值和分面计数。可以通过在查询的最后声明它们来使用 SQL `SELECT` 语句访问分面。

					## 聚合

					<!-- example Aggregations -->
					### SQL
					分面值可以源自属性、JSON 属性中的 JSON 属性，或表达式。分面值也可以被别名，但 **别名必须在所有结果集中是唯一的**（主查询结果集和其他分面结果集）。分面值来源于聚合的属性/表达式，但也可以来自另一个属性/表达式。

					CODE_BLOCK_0
					多个分面声明必须用空格分隔。
					### HTTP JSON

					分面可以在 `aggs` 节点中定义：

					CODE_BLOCK_1

					其中：

					* `group name` 是分配给聚合的别名
					* `field` 值必须包含被分面的属性或表达式的名称
					* 可选的 `size` 指定要包含在结果中的最大桶数。当未指定时，它会继承主查询的限制。更多详细信息请查看 [分面结果的大小](../Searching/Faceted_search.md#Size-of-facet-result) 部分。
					* 可选的 `sort` 使用与 [主查询中的 "sort" 参数](../Searching/Sorting_and_ranking.md#Sorting-via-JSON) 相同的语法指定属性和/或附加属性的数组。
					结果集将包含一个 `aggregations` 节点，其中返回的分面为 `key` 是聚合值，`doc_count` 是聚合计数。
					CODE_BLOCK_2
					<!-- intro -->
					##### SQL:
					<!-- request SQL -->
					CODE_BLOCK_3
					<!-- response SQL -->
					CODE_BLOCK_4
					<!-- intro -->
					##### JSON:

					<!-- request JSON -->
					CODE_BLOCK_5
					<!-- response JSON -->
					CODE_BLOCK_6
					<!-- intro -->

					##### PHP:

					<!-- request PHP -->
					CODE_BLOCK_7
					<!-- response PHP -->
					CODE_BLOCK_8
					<!-- intro -->
					##### Python:
					<!-- request Python -->
					CODE_BLOCK_9
					<!-- response Python -->
					CODE_BLOCK_10
					<!-- intro -->
					##### Javascript:
					<!-- request Javascript -->
					CODE_BLOCK_11
					<!-- response Javascript -->
					CODE_BLOCK_12
					<!-- intro -->
					##### Java:
					<!-- request Java -->
					CODE_BLOCK_13
					<!-- response Java -->
					CODE_BLOCK_14
					<!-- intro -->
					##### C#:
					<!-- request C# -->
					CODE_BLOCK_15
					<!-- response C# -->
					CODE_BLOCK_16

					<!-- request TypeScript -->
					CODE_BLOCK_17

					<!-- response TypeScript -->

					CODE_BLOCK_18
					<!-- request Go -->
					CODE_BLOCK_19

					<!-- response Go -->

					CODE_BLOCK_20
					<!-- end -->
					<!-- example Another_attribute -->
					### 通过聚合另一个属性进行分面
					数据可以通过聚合另一个属性或表达式来分面。例如，如果文档同时包含品牌 ID 和名称，我们可以在分面中返回品牌名称，但聚合品牌 ID。这可以通过使用 `FACET {expr1} BY {expr2}` 来实现。
					<!-- intro -->
					##### SQL:
					<!-- request SQL -->
					CODE_BLOCK_21
					<!-- response SQL -->
					CODE_BLOCK_22
					<!-- end -->
					<!-- example Distinct -->
					### 无重复项的分面
					如果您需要从 FACET 返回的桶中移除重复项，可以使用 `DISTINCT field_name`，其中 `field_name` 是您想要去重的字段。如果您对分布式表进行了 FACET 查询并且不确定表中是否有唯一 ID，这也可以是 `id`（这是默认值）（表应该是本地的并且拥有相同的模式）。
					如果您的查询中有多个 FACET 声明，`field_name` 应在所有声明中保持一致。
					`DISTINCT` 在 `count(*)` 列之前返回附加列 `count(distinct ...)`，允许您在不需要进行另一个查询的情况下获得两者结果。
					<!-- intro -->
					##### SQL:
					<!-- request SQL -->
					CODE_BLOCK_23
					<!-- response SQL -->
					CODE_BLOCK_24
					<!-- intro -->
					##### JSON:
					<!-- request JSON -->
					CODE_BLOCK_25
					<!-- response JSON -->
					CODE_BLOCK_26
					<!-- end -->
					<!-- example Expressions -->
					### 对表达式的分面
					分面可以对表达式进行聚合。一个经典的例子是通过特定范围划分价格：
					<!-- request SQL -->

					CODE_BLOCK_27

					<!-- response SQL -->
					CODE_BLOCK_28
					<!-- request JSON -->
					CODE_BLOCK_29
					<!-- response JSON -->
					CODE_BLOCK_30
					<!-- request PHP -->
					CODE_BLOCK_31
					<!-- response PHP -->
					CODE_BLOCK_32
					<!-- request Python -->
					CODE_BLOCK_33
					<!-- response Python -->
					CODE_BLOCK_34
					<!-- request Javascript -->
					CODE_BLOCK_35
					<!-- response Javascript -->
					CODE_BLOCK_36
					<!-- request Java -->
					CODE_BLOCK_37
					<!-- response Java -->
					CODE_BLOCK_38
					<!-- request C# -->
					CODE_BLOCK_39
					<!-- response C# -->
					CODE_BLOCK_40

					<!-- request TypeScript -->

					CODE_BLOCK_41
					<!-- response TypeScript -->
					CODE_BLOCK_42
					<!-- request Go -->
					CODE_BLOCK_43
					<!-- response Go -->
					CODE_BLOCK_44
					<!-- end -->
					<!-- example Multi-level -->
					### 对多级分组的分面
					分面可以对多级分组进行聚合，结果集与查询执行多级分组时的结果集相同：
					<!-- request SQL -->
					CODE_BLOCK_45
					<!-- response SQL -->
Here's the translation of the document, maintaining the original structure exactly:
					<!-- end -->
					<!-- example histogram -->
					### 直方图值分面
					分面可以通过构建固定大小的桶来聚合直方图值。
					关键函数是：
					CODE_BLOCK_47
					直方图参数 `interval` 必须为正数，直方图参数 `offset` 必须为正数且小于 `interval`。默认情况下，桶作为数组返回。直方图参数 `keyed` 使响应成为带有桶键的字典。
					<!-- request SQL -->
					CODE_BLOCK_48
					<!-- response SQL -->
					CODE_BLOCK_49
					<!-- request JSON -->
					CODE_BLOCK_50
					<!-- response JSON -->
					CODE_BLOCK_51
					<!-- request JSON 2 -->
					CODE_BLOCK_52
					<!-- response JSON 2 -->
					CODE_BLOCK_53
					<!-- end -->
					<!-- example histogram_date -->
					### 日期值直方图分面
					分面可以聚合日期值直方图，这与普通直方图类似。不同之处在于使用日期或时间表达式指定间隔。这种表达式需要特殊支持，因为间隔并非总是固定长度。使用以下关键函数将值舍入到最近的桶：
					CODE_BLOCK_54
					直方图参数 `calendar_interval` 理解月份有不同的天数。
					与 `calendar_interval` 不同，`fixed_interval` 参数使用固定数量的单位，无论落在日历的哪个位置都不会偏离。但是 `fixed_interval` 无法处理月等单位，因为月不是固定数量。尝试为 `fixed_interval` 指定周或月等单位将导致错误。
					接受的间隔在 [date_histogram](../Functions/Date_and_time_functions.md#DATE_HISTOGRAM%28%29) 表达式中描述。默认情况下，桶作为数组返回。直方图参数 `keyed` 使响应成为带有桶键的字典。
					<!-- request SQL -->
					CODE_BLOCK_55
					<!-- response SQL -->
					CODE_BLOCK_56
					<!-- request JSON -->
					CODE_BLOCK_57
					<!-- response JSON -->
					CODE_BLOCK_58
					<!-- end -->
					<!-- example facet range -->
					### 范围集分面
					分面可以聚合范围集。值根据桶范围进行检查，每个桶包含 `from` 值并排除范围的 `to` 值。
					将 `keyed` 属性设置为 `true` 会使响应成为带有桶键的字典，而不是数组。
					<!-- request SQL -->
					CODE_BLOCK_59
					<!-- response SQL -->
					CODE_BLOCK_60
					<!-- request JSON -->
					CODE_BLOCK_61
					<!-- response JSON -->
					CODE_BLOCK_62
					<!-- request JSON 2 -->
					CODE_BLOCK_63
					<!-- response JSON 2 -->
					CODE_BLOCK_64
					<!-- end -->
					<!-- example facet range_date -->
					### 日期范围集分面
					分面可以聚合日期范围集，这与普通范围类似。不同之处在于 `from` 和 `to` 值可以用 [日期数学](../Functions/Date_and_time_functions.md#Date-math) 表达式表示。此聚合包含每个范围的 `from` 值并排除 `to` 值。将 `keyed` 属性设置为 `true` 会使响应成为带有桶键的字典，而不是数组。
					<!-- request SQL -->
					CODE_BLOCK_65
					<!-- response SQL -->
					CODE_BLOCK_66
					<!-- request JSON -->
					CODE_BLOCK_67
					<!-- response JSON -->
					CODE_BLOCK_68
					<!-- end -->
					<!-- example Ordering -->

					### 分面结果排序
					分面支持 `ORDER BY` 子句，就像标准查询一样。每个分面可以有自己的排序，分面排序不影响主结果集的排序，后者由主查询的 `ORDER BY` 决定。排序可以基于属性名称、计数（使用 `COUNT(*)`, `COUNT(DISTINCT attribute_name)`）或特殊的 `FACET()` 函数，该函数提供聚合的数据值。默认情况下，带有 `ORDER BY COUNT(*)` 的查询将以降序排序。

					<!-- intro -->
					##### SQL:
					<!-- request SQL -->
					CODE_BLOCK_69
					<!-- response SQL -->
					CODE_BLOCK_70
					<!-- intro -->
					##### JSON:
					<!-- request JSON -->
					CODE_BLOCK_71
					<!-- response JSON -->
					CODE_BLOCK_72
					<!-- end -->
					<!-- example Size -->
					### 分面结果大小
					默认情况下，每个分面结果集限制为 20 个值。可以使用 `LIMIT` 子句单独控制每个分面的值数量，方法是提供要返回的值数量，格式为 `LIMIT count`，或者使用偏移量 `LIMIT offset, count`。
					返回的最大分面值受查询的 `max_matches` 设置限制。如果要实现动态 `max_matches`（为了更好的性能，将 `max_matches` 限制为 offset + 每页），必须考虑到太低的 `max_matches` 值可能会影响分面值的数量。在这种情况下，应使用足以覆盖分面值数量的最小 `max_matches` 值。
					<!-- intro -->
					##### SQL:
					<!-- request SQL -->
					CODE_BLOCK_73
					<!-- response SQL -->
					CODE_BLOCK_74
					<!-- request JSON -->
					CODE_BLOCK_75
					<!-- response JSON -->
					CODE_BLOCK_76
					<!-- request PHP -->
					CODE_BLOCK_77
					<!-- response PHP -->
					CODE_BLOCK_78
					<!-- request Python -->
					CODE_BLOCK_79
					<!-- response Python -->
					CODE_BLOCK_80
					<!-- request Javascript -->
					CODE_BLOCK_81
					<!-- response Javascript -->
					CODE_BLOCK_82
					<!-- request Java -->
					CODE_BLOCK_83
					<!-- response Java -->
					CODE_BLOCK_84
					<!-- request C# -->
					CODE_BLOCK_85
					<!-- response C# -->
					CODE_BLOCK_86
					<!-- request TypeScript -->
					CODE_BLOCK_87
					<!-- response TypeScript -->
					CODE_BLOCK_88
					<!-- request Go -->
					CODE_BLOCK_89
					<!-- response Go -->
					CODE_BLOCK_90
					<!-- end -->
					### 返回的结果集
					使用 SQL 时，带有分面的搜索返回多个结果集。使用的 MySQL 客户端/库/连接器**必须**支持多结果集才能访问分面结果集。
					<!-- example Performance -->
					### 性能
Internally, the `FACET` 是执行多查询的简写，其中第一个查询包含主要搜索查询，批处理中其余的查询各自具有聚类。与多查询的情况一样，通用查询优化可以在分面搜索中启动，这意味着搜索查询只执行一次，分面在搜索查询结果上操作，每个分面仅为总查询时间添加一小部分时间。
					要检查分面搜索是否以优化模式运行，可以查看 [query log](../Logging/Query_logging.md)，所有记录的查询将包含一个 `xN` 字符串，其中 `N` 是在优化组中运行的查询数量。或者，您可以检查 [SHOW META](../Node_info_and_management/SHOW_META.md) 语句的输出，该输出将显示一个 `multiplier` 指标：
					<!-- request SQL -->
					CODE_BLOCK_91
					<!-- response SQL -->
					CODE_BLOCK_92
					<!-- end -->
					<!-- proofread -->
















































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































