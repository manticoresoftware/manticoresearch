# 分面搜索

分面搜索对于现代搜索应用来说，与[自动补全](../Searching/Autocomplete.md)、[拼写纠正](../Searching/Spell_correction.md)和搜索关键词[高亮](../Searching/Highlighting.md)同样重要，尤其是在电子商务产品中。

![分面搜索](faceted.png)

当处理大量数据和各种相互关联的属性时，例如尺寸、颜色、制造商或其他因素，分面搜索就派上用场了。在查询海量数据时，搜索结果常常包含许多不符合用户预期的条目。分面搜索使最终用户能够明确定义他们希望搜索结果满足的条件。

在 Manticore Search 中，有一项优化功能，它会保留原始查询的结果集，并在每个分面计算中重复使用。由于聚合操作应用于已计算好的文档子集，因此速度很快，总执行时间通常仅比初始查询稍长一些。分面可以添加到任何查询中，分面可以是任何属性或表达式。分面结果包括分面值和分面计数。可以通过在查询末尾声明分面，使用 SQL `SELECT` 语句访问分面。

## 聚合

<!-- example Aggregations -->
### SQL
分面值可以来自属性、JSON 属性内的 JSON 属性或表达式。分面值也可以使用别名，但**别名在所有结果集（主查询结果集和其他分面结果集）中必须是唯一的**。分面值源自聚合的属性/表达式，但也可以来自另一个属性/表达式。

```sql
FACET {expr_list} [BY {expr_list} ] [DISTINCT {field_name}] [ORDER BY {expr | FACET()} {ASC | DESC}] [LIMIT [offset,] count]
```

多个分面声明必须用空格分隔。

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
* `field` 值必须包含要进行分面的属性或表达式的名称
* 可选的 `size` 指定结果中包含的最大桶数。未指定时，继承主查询的限制。更多详细信息可以在[分面结果大小](../Searching/Faceted_search.md#Size-of-facet-result)部分找到。
* 可选的 `sort` 指定一个属性数组和/或附加属性，使用与[主查询中的"sort"参数](../Searching/Sorting_and_ranking.md#Sorting-via-JSON)相同的语法。

结果集将包含一个 `aggregations` 节点，其中包含返回的分面，`key` 是聚合值，`doc_count` 是聚合计数。

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
          "brand_name": "Brand Ten",
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

<!-- intro -->
##### Javascript:

<!-- request Javascript -->
```javascript
res =  await searchApi.search({"table":"facetdemo","query":{"match_all":{}},"limit":5,"aggs":{"group_property":{"terms":{"field":"price",}},"group_brand_id":{"terms":{"field":"brand_id"}}}});
```
<!-- response Javascript -->
```javascript
{"took":0,"timed_out":false,"hits":{"total":10000,"hits":[{"_id": 1,"_score":1,"_source":{"price":197,"brand_id":10,"brand_name":"Brand Ten","categories":[10],"title":"Product Eight One","property":"Six"}},{"_id": 2,"_score":1,"_source":{"price":671,"brand_id":6,"brand_name":"Brand Six","categories":[12,13,14],"title":"Product Nine Seven","property":"Four"}},{"_id": 3,"_score":1,"_source":{"price":92,"brand_id":3,"brand_name":"Brand Three","categories":[13,14,15],"title":"Product Five Four","property":"Six"}},{"_id": 4,"_score":1,"_source":{"price":713,"brand_id":10,"brand_name":"Brand Ten","categories":[11],"title":"Product Eight Nine","property":"Five"}},{"_id": 5,"_score":1,"_source":{"price":805,"brand_id":7,"brand_name":"Brand Seven","categories":[11,12,13],"title":"Product Ten Three","property":"Two"}}]}}

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
        hits: [{_id=1, _score=1, _source={price=197, brand_id=10, brand_name=Brand Ten, categories=[10], title=Product Eight One, property=Six}}, {_id=2, _score=1, _source={price=671, brand_id=6, brand_name=Brand Six, categories=[12, 13, 14], title=Product Nine Seven, property=Four}}, {_id=3, _score=1, _source={price=92, brand_id=3, brand_name=Brand Three, categories=[13, 14, 15], title=Product Five Four, property=Six}}, {_id=4, _score=1, _source={price=713, brand_id=10, brand_name=Brand Ten, categories=[11], title=Product Eight Nine, property=Five}}, {_id=5, _score=1, _source={price=805, brand_id=7, brand_name=Brand Seven, categories=[11, 12, 13], title=Product Ten Three, property=Two}}]
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
        hits: [{_id=1, _score=1, _source={price=197, brand_id=10, brand_name=Brand Ten, categories=[10], title=Product Eight One, property=Six}}, {_id=2, _score=1, _source={price=671, brand_id=6, brand_name=Brand Six, categories=[12, 13, 14], title=Product Nine Seven, property=Four}}, {_id=3, _score=1, _source={price=92, brand_id=3, brand_name=Brand Three, categories=[13, 14, 15], title=Product Five Four, property=Six}}, {_id=4, _score=1, _source={price=713, brand_id=10, brand_name=Brand Ten, categories=[11], title=Product Eight Nine, property=Five}}, {_id=5, _score=1, _source={price=805, brand_id=7, brand_name=Brand Seven, categories=[11, 12, 13], title=Product Ten Three, property=Two}}]
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
        hits: [{_id=1, _score=1, _source={price=197, brand_id=10, brand_name=Brand Ten, categories=[10], title=Product Eight One, property=Six}}, {_id=2, _score=1, _source={price=671, brand_id=6, brand_name=Brand Six, categories=[12, 13, 14], title=Product Nine Seven, property=Four}}, {_id=3, _score=1, _source={price=92, brand_id=3, brand_name=Brand Three, categories=[13, 14, 15], title=Product Five Four, property=Six}}, {_id=4, _score=1, _source={price=713, brand_id=10, brand_name=Brand Ten, categories=[11], title=Product Eight Nine, property=Five}}, {_id=5, _score=1, _source={price=805, brand_id=7, brand_name=Brand Seven, categories=[11, 12, 13], title=Product Ten Three, property=Two}}]
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
          "content": "Text 1",
          "name": "Doc 1",
          "cat": 1
        }
      },
 ...
      {
        "_id": 5,
        "_score": 1,
        "_source": {
          "content": "Text 5",
          "name": "Doc 5",
          "cat": 4
        }
      }
    ]
  },
  "aggregations": {
    "name_group": {
      "buckets": [
        {
          "key": "Doc 1",
          "doc_count": 1
        },
...
        {
          "key": "Doc 5",
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
          "content": "Text 1",
          "name": "Doc 1",
          "cat": 1
        }
      },
 ...
      {
        "_id": 5,
        "_score": 1,
        "_source": {
          "content": "Text 5",
          "name": "Doc 5",
          "cat": 4
        }
      }
    ]
  },
  "aggregations": {
    "name_group": {
      "buckets": [
        {
          "key": "Doc 1",
          "doc_count": 1
        },
...
        {
          "key": "Doc 5",
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

### 通过聚合另一个属性进行分面

可以通过聚合另一个属性或表达式对数据进行分面。例如，如果文档同时包含品牌ID和名称，我们可以在分面中返回品牌名称，但聚合品牌ID。这可以通过使用 `FACET {expr1} BY {expr2}` 来实现。


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

### 去重分面

如果需要从 FACET 返回的桶中移除重复项，可以使用 `DISTINCT field_name`，其中 `field_name` 是您希望用于去重的字段。如果您对分布式表进行 FACET 查询，并且不确定表中是否有唯一的ID（表应该是本地的且具有相同的模式），也可以是 `id`（这是默认值）。

如果查询中有多个 FACET 声明，`field_name` 在所有声明中应该相同。

`DISTINCT` 会在 `count(*)` 列之前返回一个额外的列 `count(distinct ...)`，使您无需进行另一个查询即可获得两个结果。

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
          "brand_name": "Brand Nine",
          "property": "Four"
        }
      },
      {
        "_score": 1,
        "_source": {
          "brand_name": "Brand Ten",
          "property": "Four"
        }
      },
 ...
      {
        "_score": 1,
        "_source": {
          "brand_name": "Brand Four",
          "property": "Eight"
        }
      }
    ]
  },
  "aggregations": {
    "brand_name": {
      "buckets": [
        {
          "key": "Brand Nine",
          "doc_count": 3,
          "count(distinct property)": 3
        },
        {
          "key": "Brand Ten",
          "doc_count": 3,
          "count(distinct property)": 2
        },
...
        {
          "key": "Brand Two",
          "doc_count": 1,
          "count(distinct property)": 1
        },
        {
          "key": "Brand Six",
          "doc_count": 1,
          "count(distinct property)": 1
        },
        {
          "key": "Brand Four",
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
### 基于表达式的分面

分面可以基于表达式进行聚合。一个经典的例子是按特定范围对价格进行分段：

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
          "brand_name": "Brand Ten",
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

<!-- request Python-asyncio -->
```python
res = await searchApi.search({"table":"facetdemo","query":{"match_all":{}},"expressions":{"price_range":"INTERVAL(price,200,400,600,800)"},"aggs":{"group_property":{"terms":{"field":"price_range"}}}})
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
res =  await searchApi.search({"table":"facetdemo","query":{"match_all":{}},"expressions":{"price_range":"INTERVAL(price,200,400,600,800)"},"aggs":{"group_property":{"terms":{"field":"price_range"}}}});
```
<!-- response Javascript -->
```javascript
{"took":0,"timed_out":false,"hits":{"total":10000,"hits":[{"_id": 1,"_score":1,"_source":{"price":197,"brand_id":10,"brand_name":"Brand Ten","categories":[10],"title":"Product Eight One","property":"Six","price_range":0}},{"_id": 2,"_score":1,"_source":{"price":671,"brand_id":6,"brand_name":"Brand Six","categories":[12,13,14],"title":"Product Nine Seven","property":"Four","price_range":3}},{"_id": 3,"_score":1,"_source":{"price":92,"brand_id":3,"brand_name":"Brand Three","categories":[13,14,15],"title":"Product Five Four","property":"Six","price_range":0}},{"_id": 4,"_score":1,"_source":{"price":713,"brand_id":10,"brand_name":"Brand Ten","categories":[11],"title":"Product Eight Nine","property":"Five","price_range":3}},{"_id": 5,"_score":1,"_source":{"price":805,"brand_id":7,"brand_name":"Brand Seven","categories":[11,12,13],"title":"Product Ten Three","property":"Two","price_range":4}},{"_id": 6,"_score":1,"_source":{"price":420,"brand_id":2,"brand_name":"Brand Two","categories":[10,11],"title":"Product Two One","property":"Six","price_range":2}},{"_id": 7,"_score":1,"_source":{"price":412,"brand_id":9,"brand_name":"Brand Nine","categories":[10],"title":"Product Four Nine","property":"Eight","price_range":2}},{"_id": 8,"_score":1,"_source":{"price":300,"brand_id":9,"brand_name":"Brand Nine","categories":[13,14,15],"title":"Product Eight Four","property":"Five","price_range":1}},{"_id": 9,"_score":1,"_source":{"price":728,"brand_id":1,"brand_name":"Brand One","categories":[11],"title":"Product Nine Six","property":"Four","price_range":3}},{"_id": 10,"_score":1,"_source":{"price":622,"brand_id":3,"brand_name":"Brand Three","categories":[10,11],"title":"Product Six Seven","property":"Two","price_range":3}},{"_id": 11,"_score":1,"_source":{"price":462,"brand_id":5,"brand_name":"Brand Five","categories":[10,11],"title":"Product Ten Two","property":"Eight","price_range":2}},{"_id": 12,"_score":1,"_source":{"price":939,"brand_id":7,"brand_name":"Brand Seven","categories":[12,13],"title":"Product Nine Seven","property":"Six","price_range":4}},{"_id": 13,"_score":1,"_source":{"price":948,"brand_id":8,"brand_name":"Brand Eight","categories":[12],"title":"Product Ten One","property":"Six","price_range":4}},{"_id": 14,"_score":1,"_source":{"price":900,"brand_id":9,"brand_name":"Brand Nine","categories":[12,13,14],"title":"Product Ten Nine","property":"Three","price_range":4}},{"_id": 15,"_score":1,"_source":{"price":224,"brand_id":3,"brand_name":"Brand Three","categories":[13],"title":"Product Two Six","property":"Four","price_range":1}},{"_id": 16,"_score":1,"_source":{"price":713,"brand_id":10,"brand_name":"Brand Ten","categories":[12],"title":"Product Two Four","property":"Six","price_range":3}},{"_id": 17,"_score":1,"_source":{"price":510,"brand_id":2,"brand_name":"Brand Two","categories":[10],"title":"Product Ten Two","property":"Seven","price_range":2}},{"_id": 18,"_score":1,"_source":{"price":702,"brand_id":10,"brand_name":"Brand Ten","categories":[12,13],"title":"Product Nine One","property":"Three","price_range":3}},{"_id": 19,"_score":1,"_source":{"price":836,"brand_id":4,"brand_name":"Brand Four","categories":[10,11,12],"title":"Product Four Five","property":"Two","price_range":4}},{"_id": 20,"_score":1,"_source":{"price":227,"brand_id":3,"brand_name":"Brand Three","categories":[12,13],"title":"Product Three Four","property":"Ten","price_range":1}}]}}
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
        hits: [{_id=1, _score=1, _source={price=197, brand_id=10, brand_name=Brand Ten, categories=[10], title=Product Eight One, property=Six, price_range=0}}, {_id=2, _score=1, _source={price=671, brand_id=6, brand_name=Brand Six, categories=[12, 13, 14], title=Product Nine Seven, property=Four, price_range=3}}, {_id=3, _score=1, _source={price=92, brand_id=3, brand_name=Brand Three, categories=[13, 14, 15], title=Product Five Four, property=Six, price_range=0}}, {_id=4, _score=1, _source={price=713, brand_id=10, brand_name=Brand Ten, categories=[11], title=Product Eight Nine, property=Five, price_range=3}}, {_id=5, _score=1, _source={price=805, brand_id=7, brand_name=Brand Seven, categories=[11, 12, 13], title=Product Ten Three, property=Two, price_range=4}}]
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
        hits: [{_id=1, _score=1, _source={price=197, brand_id=10, brand_name=Brand Ten, categories=[10], title=Product Eight One, property=Six, price_range=0}}, {_id=2, _score=1, _source={price=671, brand_id=6, brand_name=Brand Six, categories=[12, 13, 14], title=Product Nine Seven, property=Four, price_range=3}}, {_id=3, _score=1, _source={price=92, brand_id=3, brand_name=Brand Three, categories=[13, 14, 15], title=Product Five Four, property=Six, price_range=0}}, {_id=4, _score=1, _source={price=713, brand_id=10, brand_name=Brand Ten, categories=[11], title=Product Eight Nine, property=Five, price_range=3}}, {_id=5, _score=1, _source={price=805, brand_id=7, brand_name=Brand Seven, categories=[11, 12, 13], title=Product Ten Three, property=Two, price_range=4}}]
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
        hits: [{_id=1, _score=1, _source={price=197, brand_id=10, brand_name=Brand Ten, categories=[10], title=Product Eight One, property=Six, price_range=0}}, {_id=2, _score=1, _source={price=671, brand_id=6, brand_name=Brand Six, categories=[12, 13, 14], title=Product Nine Seven, property=Four, price_range=3}}, {_id=3, _score=1, _source={price=92, brand_id=3, brand_name=Brand Three, categories=[13, 14, 15], title=Product Five Four, property=Six, price_range=0}}, {_id=4, _score=1, _source={price=713, brand_id=10, brand_name=Brand Ten, categories=[11], title=Product Eight Nine, property=Five, price_range=3}}, {_id=5, _score=1, _source={price=805, brand_id=7, brand_name=Brand Seven, categories=[11, 12, 13], title=Product Ten Three, property=Two, price_range=4}}]
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
          "content": "Text 1",
          "name": "Doc 1",
          "cat": 1,
          "cat_range": 1
        }
      },
 ...
      {
        "_id": 5,
        "_score": 1,
        "_source": {
          "content": "Text 5",
          "name": "Doc 5",
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
          "content": "Text 1",
          "name": "Doc 1",
          "cat": 1,
          "cat_range": 1
        }
      },
 ...
      {
        "_id": 5,
        "_score": 1,
        "_source": {
          "content": "Text 5",
          "name": "Doc 5",
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

### 多级分组的Facet

Facets可以对多级分组进行聚合，结果集与查询执行多级分组时的结果相同：

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

### 基于直方图值的Facet

Facets可以通过构造固定大小的桶来对值进行直方图聚合。
键函数是：

```sql
key_of_the_bucket = interval + offset * floor ( ( value - offset ) / interval )
```

直方图参数`interval`必须为正数，直方图参数`offset`必须为正数且小于`interval`。默认情况下，桶以数组形式返回。直方图参数`keyed`使得响应以字典形式返回桶键。

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

### 基于日期直方图值的Facet

Facets可以对日期直方图值进行聚合，这与普通直方图类似。不同之处在于，区间由日期或时间表达式指定。此类表达式需要特殊支持，因为区间长度不总是固定的。值会根据以下键函数四舍五入到最近的桶：

```sql
key_of_the_bucket = interval * floor ( value / interval )
```

直方图参数`calendar_interval`理解月份具有不同的天数。
与`calendar_interval`不同，`fixed_interval`参数使用固定数量的单位，无论其在日历中的位置如何，都不会偏离。但是`fixed_interval`无法处理如周或月这样的单位，因为月不是一个固定数量。尝试为`fixed_interval`指定如周或月这样的单位将导致错误。
接受的区间在[日期直方图](../Functions/Date_and_time_functions.md#DATE_HISTOGRAM%28%29)表达式中描述。默认情况下，桶以数组形式返回。直方图参数`keyed`使得响应以字典形式返回桶键。

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

### 基于范围的Facet

Facets可以对一组范围进行聚合。值会与桶范围进行检查，其中每个桶包括`from`值并排除`to`值。
将`keyed`属性设置为`true`使得响应以字典形式返回桶键，而不是数组。

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

### 基于日期范围的Facet

Facets可以对一组日期范围进行聚合，这与普通范围类似。不同之处在于，`from`和`to`值可以使用[日期数学](../Functions/Date_and_time_functions.md#Date-math)表达式表示。此聚合包括`from`值并排除每个范围的`to`值。将`keyed`属性设置为`true`使得响应以字典形式返回桶键，而不是数组。

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

Facets支持`ORDER BY`子句，就像标准查询一样。每个Facet可以有自己的排序方式，Facet的排序不会影响主结果集的排序，这由主查询的`ORDER BY`决定。排序可以基于属性名、计数（使用`COUNT(*)`、`COUNT(DISTINCT attribute_name)`）或特殊的`FACET()`函数，该函数提供聚合数据值。默认情况下，带有`ORDER BY COUNT(*)`的查询将按降序排序。


<!-- intro -->
##### SQL：

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
##### JSON：

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
### Facet结果的大小

默认情况下，每个Facet结果集仅限于20个值。可以通过`LIMIT`子句单独为每个Facet控制Facet值的数量，提供返回值的数量格式`LIMIT count`或使用偏移量`LIMIT offset, count`。

返回的最大Facet值数量受查询的`max_matches`设置限制。如果您想实现动态`max_matches`（限制`max_matches`为偏移量+每页以提高性能），必须考虑到过低的`max_matches`值可能会影响Facet值的数量。在这种情况下，应使用足以覆盖Facet值数量的最小`max_matches`值。

<!-- intro -->
##### SQL：

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
          "brand_name": "Brand Ten",
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


        }});
    }});
    put("group_brand_id", new HashMap<String,Object>(){{
        put("terms", new HashMap<String,Object>(){{
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
let aggTerms1 = AggTerms {
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
        hits: [{_id=1, _score=1, _source={price=197, brand_id=10, brand_name=Brand Ten, categories=[10], title=Product Eight One, property=Six}}, {_id=2, _score=1, _source={price=671, brand_id=6, brand_name=Brand Six, categories=[12, 13, 14], title=Product Nine Seven, property=Four}}, {_id=3, _score=1, _source={price=92, brand_id=3, brand_name=Brand Three, categories=[13, 14, 15], title=Product Five Four, property=Six}}, {_id=4, _score=1, _source={price=713, brand_id=10, brand_name=Brand Ten, categories=[11], title=Product Eight Nine, property=Five}}, {_id=5, _score=1, _source={price=805, brand_id=7, brand_name=Brand Seven, categories=[11, 12, 13], title=Product Ten Three, property=Two}}]
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
          "content": "Text 1",
          "name": "Doc 1",
          "cat": 1
        }
      },
 ...
      {
        "_id": 5,
        "_score": 1,
        "_source": {
          "content": "Text 5",
          "name": "Doc 5",
          "cat": 4
        }
      }
    ]
  },
  "aggregations": {
    "name_group": {
      "buckets": [
        {
          "key": "Doc 1",
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
          "content": "Text 1",
          "name": "Doc 1",
          "cat": 1
        }
      },
 ...
      {
        "_id": 5,
        "_score": 1,
        "_source": {
          "content": "Text 5",
          "name": "Doc 5",
          "cat": 4
        }
      }
    ]
  },
  "aggregations": {
    "name_group": {
      "buckets": [
        {
          "key": "Doc 1",
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
### 返回的结果集

当使用SQL时，带有 facets 的搜索会返回多个结果集。MySQL客户端/库/连接器所使用的 **必须** 支持多个结果集，以便访问 facets 结果集。

<!-- example Performance -->
### 性能

内部，`FACET` 是执行多查询的一个快捷方式，其中第一个查询包含主要搜索查询，批次中的其余查询各自包含聚类。正如多查询的情况一样，对于带有 facets 的搜索，常见的查询优化可以启动，这意味着搜索查询只执行一次，而 facets 则在搜索查询结果上操作，每个 facet 只增加总查询时间的一小部分。


要检查 facets 搜索是否以优化模式运行，可以在 [查询日志](../Logging/Query_logging.md) 中查找，其中所有记录的查询将包含一个 `xN` 字符串，`N` 是优化组中运行的查询数量。或者，可以检查 [SHOW META](../Node_info_and_management/SHOW_META.md) 语句的输出，该语句将显示一个 `multiplier` 指标：

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
<!-- proofread -->

