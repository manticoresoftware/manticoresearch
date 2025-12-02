# 分组搜索结果

<!-- example general -->
分组搜索结果通常有助于获取每个分组的匹配计数或其他聚合。例如，它对于创建显示每月匹配博客文章数量的图表，或按网站分组网页搜索结果，或按作者分组论坛帖子等非常有用。

Manticore 支持按单列、多列和计算表达式对搜索结果进行分组。结果可以：

* 在组内排序
* 每个组返回多行
* 对组进行过滤
* 对组进行排序
* 使用[聚合函数](../Searching/Grouping.md#Aggregation-functions)进行聚合

<!-- intro -->
通用语法为：

<!-- request SQL -->
通用语法
```sql
SELECT {* | SELECT_expr [, SELECT_expr ...]}
...
GROUP BY {field_name | alias } [, ...]
[HAVING where_condition]
[WITHIN GROUP ORDER BY field_name {ASC | DESC} [, ...]]
...

SELECT_expr: { field_name | function_name(...) }
where_condition: {aggregation expression alias | COUNT(*)}
```

<!-- request JSON -->
JSON 查询格式目前支持基本的分组操作，可以检索聚合值及其 count(*)。

```json
{
  "table": "<table_name>",
  "limit": 0,
  "aggs": {
    "<aggr_name>": {
      "terms": {
        "field": "<attribute>",
        "size": <int value>
      }
    }
  }
}
```

标准查询输出返回未分组的结果集，可以通过 `limit`（或 `size`）隐藏它。
聚合需要设置组结果集大小的 `size`。

<!-- end -->

<!-- example group1 -->
### 只做分组
分组非常简单 - 只需在你的 `SELECT` 查询末尾添加 "GROUP BY smth"。这里的某物可以是：

* 表中的任何非全文字段：整数、浮点数、字符串、MVA（多值属性）
* 或者，如果你在 `SELECT` 列表中使用了别名，也可以按它分组

你可以省略 `SELECT` 列表中的任何[聚合函数](../Searching/Grouping.md#Aggregation-functions)，它仍然会生效：

<!-- intro -->
##### 示例：

<!-- request SQL -->
```sql
SELECT release_year FROM films GROUP BY release_year LIMIT 5;
```
<!-- response SQL -->
```sql
+--------------+
| release_year |
+--------------+
|         2004 |
|         2002 |
|         2001 |
|         2005 |
|         2000 |
+--------------+
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SELECT release_year FROM films GROUP BY release_year LIMIT 5;"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "release_year": {
          "type": "long"
        }
      }
    ],
    "data": [
      {
        "release_year": 2004
      },
            {
        "release_year": 2002
      },
            {
        "release_year": 2001
      },
            {
        "release_year": 2005
      },
            {
        "release_year": 2000
      }
    ],
    "total": 5,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->
<!-- example group2 -->
在大多数情况下，你可能希望为每个分组获取一些聚合数据，例如：

* `COUNT(*)` 用于简单地获取每个组中的元素数量
* 或者 `AVG(field)` 用于计算组内字段的平均值

对于 HTTP JSON 请求，使用单一的 `aggs` 桶并在主查询层设置 `limit=0`，其效果类似于带有 `GROUP BY` 和 `COUNT(*)` 的 SQL 查询，提供相同的行为和性能表现。

<!-- intro -->
##### 示例：

<!-- request SQL1 -->
```sql
SELECT release_year, count(*) FROM films GROUP BY release_year LIMIT 5;
```
<!-- response SQL1 -->
```sql
+--------------+----------+
| release_year | count(*) |
+--------------+----------+
|         2004 |      108 |
|         2002 |      108 |
|         2001 |       91 |
|         2005 |       93 |
|         2000 |       97 |
+--------------+----------+
```
<!-- request SQL2 -->
```sql
SELECT release_year, AVG(rental_rate) FROM films GROUP BY release_year LIMIT 5;
```
<!-- response SQL2 -->
```sql
+--------------+------------------+
| release_year | avg(rental_rate) |
+--------------+------------------+
|         2004 |       2.78629661 |
|         2002 |       3.08259249 |
|         2001 |       3.09989142 |
|         2005 |       2.90397978 |
|         2000 |       3.17556739 |
+--------------+------------------+
```

<!-- request JSON -->
``` json
POST /search -d '
    {
     "table" : "films",
     "limit": 0,
     "aggs" :
     {
        "release_year" :
         {
            "terms" :
             {
              "field":"release_year",
              "size":100
             }
         }
     }
    }
'
```
<!-- response JSON -->
``` json
{
  "took": 2,
  "timed_out": false,
  "hits": {
    "total": 10000,
    "hits": [

    ]
  },
  "release_year": {
    "group_brand_id": {
      "buckets": [
        {
          "key": 2004,
          "doc_count": 108
        },
        {
          "key": 2002,
          "doc_count": 108
        },
        {
          "key": 2000,
          "doc_count": 97
        },
        {
          "key": 2005,
          "doc_count": 93
        },
        {
          "key": 2001,
          "doc_count": 91
        }
      ]
    }
  }
}
```
<!-- request PHP -->
``` php
$index->setName('films');
$search = $index->search('');
$search->limit(0);
$search->facet('release_year','release_year',100);
$results = $search->get();
print_r($results->getFacets());
```
<!-- response PHP -->
``` php
Array
(
    [release_year] => Array
        (
            [buckets] => Array
                (
                    [0] => Array
                        (
                            [key] => 2009
                            [doc_count] => 99
                        )
                    [1] => Array
                        (
                            [key] => 2008
                            [doc_count] => 102
                        )
                    [2] => Array
                        (
                            [key] => 2007
                            [doc_count] => 93
                        )
                    [3] => Array
                        (
                            [key] => 2006
                            [doc_count] => 103
                        )
                    [4] => Array
                        (
                            [key] => 2005
                            [doc_count] => 93
                        )
                    [5] => Array
                        (
                            [key] => 2004
                            [doc_count] => 108
                        )
                    [6] => Array
                        (
                            [key] => 2003
                            [doc_count] => 106
                        )
                    [7] => Array
                        (
                            [key] => 2002
                            [doc_count] => 108
                        )
                    [8] => Array
                        (
                            [key] => 2001
                            [doc_count] => 91
                        )
                    [9] => Array
                        (
                            [key] => 2000
                            [doc_count] => 97
                        )
                )
        )
)
```
<!-- request Python -->
``` python
res =searchApi.search({"table":"films","limit":0,"aggs":{"release_year":{"terms":{"field":"release_year","size":100}}}})
```
<!-- response Python -->
``` python
{'aggregations': {u'release_year': {u'buckets': [{u'doc_count': 99,
                                                  u'key': 2009},
                                                 {u'doc_count': 102,
                                                  u'key': 2008},
                                                 {u'doc_count': 93,
                                                  u'key': 2007},
                                                 {u'doc_count': 103,
                                                  u'key': 2006},
                                                 {u'doc_count': 93,
                                                  u'key': 2005},
                                                 {u'doc_count': 108,
                                                  u'key': 2004},
                                                 {u'doc_count': 106,
                                                  u'key': 2003},
                                                 {u'doc_count': 108,
                                                  u'key': 2002},
                                                 {u'doc_count': 91,
                                                  u'key': 2001},
                                                 {u'doc_count': 97,
                                                  u'key': 2000}]}},
 'hits': {'hits': [], 'max_score': None, 'total': 1000},
 'profile': None,
 'timed_out': False,
 'took': 0}

```

<!-- request Python-asyncio -->
``` python
res = await searchApi.search({"table":"films","limit":0,"aggs":{"release_year":{"terms":{"field":"release_year","size":100}}}})
```
<!-- response Python-asyncio -->
``` python
{'aggregations': {u'release_year': {u'buckets': [{u'doc_count': 99,
                                                  u'key': 2009},
                                                 {u'doc_count': 102,
                                                  u'key': 2008},
                                                 {u'doc_count': 93,
                                                  u'key': 2007},
                                                 {u'doc_count': 103,
                                                  u'key': 2006},
                                                 {u'doc_count': 93,
                                                  u'key': 2005},
                                                 {u'doc_count': 108,
                                                  u'key': 2004},
                                                 {u'doc_count': 106,
                                                  u'key': 2003},
                                                 {u'doc_count': 108,
                                                  u'key': 2002},
                                                 {u'doc_count': 91,
                                                  u'key': 2001},
                                                 {u'doc_count': 97,
                                                  u'key': 2000}]}},
 'hits': {'hits': [], 'max_score': None, 'total': 1000},
 'profile': None,
 'timed_out': False,
 'took': 0}

```

<!-- request Javascript -->
``` javascript
res = await searchApi.search({"table":"films","limit":0,"aggs":{"release_year":{"terms":{"field":"release_year","size":100}}}});
```
<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"aggregations":{"release_year":{"buckets":[{"key":2009,"doc_count":99},{"key":2008,"doc_count":102},{"key":2007,"doc_count":93},{"key":2006,"doc_count":103},{"key":2005,"doc_count":93},{"key":2004,"doc_count":108},{"key":2003,"doc_count":106},{"key":2002,"doc_count":108},{"key":2001,"doc_count":91},{"key":2000,"doc_count":97}]}},"hits":{"total":1000,"hits":[]}}
```
<!-- request Java -->
``` java
HashMap<String,Object> aggs = new HashMap<String,Object>(){{
    put("release_year", new HashMap<String,Object>(){{
        put("terms", new HashMap<String,Object>(){{
            put("field","release_year");
            put("size",100);
        }});
    }});
}};

searchRequest = new SearchRequest();
searchRequest.setIndex("films");
searchRequest.setLimit(0);
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
searchRequest.setAggs(aggs);
searchResponse = searchApi.search(searchRequest);
```
<!-- response Java -->
``` java
class SearchResponse {
    took: 0
    timedOut: false
    aggregations: {release_year={buckets=[{key=2009, doc_count=99}, {key=2008, doc_count=102}, {key=2007, doc_count=93}, {key=2006, doc_count=103}, {key=2005, doc_count=93}, {key=2004, doc_count=108}, {key=2003, doc_count=106}, {key=2002, doc_count=108}, {key=2001, doc_count=91}, {key=2000, doc_count=97}]}}
    hits: class SearchResponseHits {
        maxScore: null
        total: 1000
        hits: []
    }
    profile: null
}
```

<!-- request C# -->
``` clike
var agg = new Aggregation("release_year", "release_year");
agg.Size = 100;
object query = new { match_all=null };
var searchRequest = new SearchRequest("films", query);
searchRequest.Aggs = new List<Aggregation> {agg};
var searchResponse = searchApi.Search(searchRequest);
```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 0
    timedOut: false
    aggregations: {release_year={buckets=[{key=2009, doc_count=99}, {key=2008, doc_count=102}, {key=2007, doc_count=93}, {key=2006, doc_count=103}, {key=2005, doc_count=93}, {key=2004, doc_count=108}, {key=2003, doc_count=106}, {key=2002, doc_count=108}, {key=2001, doc_count=91}, {key=2000, doc_count=97}]}}
    hits: class SearchResponseHits {
        maxScore: null
        total: 1000
        hits: []
    }
    profile: null
}
```

<!-- request Rust -->
``` rust
let query = SearchQuery::new();
let aggTerms1 = AggTerms::new {
    fields: "release_year".to_string(),
    size: Some(100),
};
let agg1 = Aggregation {
    terms: Some(Box::new(aggTerms1)),
    ..Default::default(),
};
let mut aggs = HashMap::new();
aggs.insert("release_year".to_string(), agg1);

let search_req = SearchRequest {
    table: "films".to_string(),
    query: Some(Box::new(query)),
    aggs: serde_json::json!(aggs),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```
<!-- response Rust -->
``` rust
class SearchResponse {
    took: 0
    timedOut: false
    aggregations: {release_year={buckets=[{key=2009, doc_count=99}, {key=2008, doc_count=102}, {key=2007, doc_count=93}, {key=2006, doc_count=103}, {key=2005, doc_count=93}, {key=2004, doc_count=108}, {key=2003, doc_count=106}, {key=2002, doc_count=108}, {key=2001, doc_count=91}, {key=2000, doc_count=97}]}}
    hits: class SearchResponseHits {
        maxScore: null
        total: 1000
        hits: []
    }
    profile: null
}
```

<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  limit: 0,
  aggs: {
    cat_id: {
      terms: { field: "cat", size: 1 }
    }
  }
});
```

<!-- response TypeScript -->
``` typescript
{
	"took":0,
	"timed_out":false,
	"aggregations":
	{
		"cat_id":
		{
			"buckets":
			[{
				"key":1,
				"doc_count":1
			}]
		}
	},
	"hits":
	{
		"total":5,
		"hits":[]
	}
}
```

<!-- request Go -->
``` go
query := map[string]interface{} {};
searchRequest.SetQuery(query);
aggTerms := manticoreclient.NewAggregationTerms()
aggTerms.SetField("cat")
aggTerms.SetSize(1)
aggregation := manticoreclient.NewAggregation()
aggregation.setTerms(aggTerms)
searchRequest.SetAggregation(aggregation)
res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```

<!-- response Go -->
``` go
{
	"took":0,
	"timed_out":false,
	"aggregations":
	{
		"cat_id":
		{
			"buckets":
			[{
				"key":1,
				"doc_count":1
			}]
		}
	},
	"hits":
	{
		"total":5,
		"hits":[]
	}
}
```

<!-- end -->

<!-- example sort1 -->
##### 对分组排序
默认情况下，分组不排序，接下来你通常会想根据某个字段对它们排序，比如你用来分组的字段：

<!-- intro -->
##### 示例：

<!-- request SQL -->
```sql
SELECT release_year, count(*) from films GROUP BY release_year ORDER BY release_year asc limit 5;
```
<!-- response SQL -->
```sql
+--------------+----------+
| release_year | count(*) |
+--------------+----------+
|         2000 |       97 |
|         2001 |       91 |
|         2002 |      108 |
|         2003 |      106 |
|         2004 |      108 |
+--------------+----------+
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SELECT release_year, count(*) from films GROUP BY release_year ORDER BY release_year asc limit 5;"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "release_year": {
          "type": "long"
        }
      },
      {
        "count(*)": {
          "type": "long long"
        }
      }
    ],
    "data": [
      {
        "release_year": 2000,
        "count(*)": 97
      },
      {
        "release_year": 2001,
        "count(*)": 91
      },
      {
        "release_year": 2002,
        "count(*)": 108
      },
      {
        "release_year": 2003,
        "count(*)": 106
      },
      {
        "release_year": 2004,
        "count(*)": 108
      }
    ],
    "total": 5,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->
<!-- example sort2 -->
另外，你也可以按聚合结果排序：

* 按 `count(*)` 排序以首先展示元素最多的组
* 按 `avg(rental_rate)` 排序以显示评分最高的电影。需要注意的是，示例中通过别名实现：`avg(rental_rate)` 首先在 `SELECT` 列表中映射到 `avg`，然后我们简单地执行 `ORDER BY avg`


<!-- intro -->
##### 示例：

<!-- request SQL1 -->
```sql
SELECT release_year, count(*) FROM films GROUP BY release_year ORDER BY count(*) desc LIMIT 5;
```
<!-- response SQL1 -->
```sql
+--------------+----------+
| release_year | count(*) |
+--------------+----------+
|         2004 |      108 |
|         2002 |      108 |
|         2003 |      106 |
|         2006 |      103 |
|         2008 |      102 |
+--------------+----------+
```

<!-- request SQL2 -->
```sql
SELECT release_year, AVG(rental_rate) avg FROM films GROUP BY release_year ORDER BY avg desc LIMIT 5;
```
<!-- response SQL2 -->
```sql
+--------------+------------+
| release_year | avg        |
+--------------+------------+
|         2006 | 3.26184368 |
|         2000 | 3.17556739 |
|         2001 | 3.09989142 |
|         2002 | 3.08259249 |
|         2008 | 2.99000049 |
+--------------+------------+
```


<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SELECT release_year, count(*) FROM films GROUP BY release_year ORDER BY count(*) desc LIMIT 5;"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "release_year": {
          "type": "long"
        }
      },
      {
        "count(*)": {
          "type": "long long"
        }
      }
    ],
    "data": [
      {
        "release_year": 2004,
        "count(*)": 108
      },
      {
        "release_year": 2004,
        "count(*)": 108
      },
      {
        "release_year": 2003,
        "count(*)": 106
      },
      {
        "release_year": 2006,
        "count(*)": 103
      },
      {
        "release_year": 2008,
        "count(*)": 102
      }
    ],
    "total": 5,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

<!-- example group3 -->
##### 同时按多个字段分组
有时你可能希望不仅按单个字段分组，还按多个字段同时分组，比如电影的类别和年份：

<!-- intro -->
##### 示例：

<!-- request SQL -->
```sql
SELECT category_id, release_year, count(*) FROM films GROUP BY category_id, release_year ORDER BY category_id ASC, release_year ASC;
```
<!-- response SQL -->
```sql
+-------------+--------------+----------+
| category_id | release_year | count(*) |
+-------------+--------------+----------+
|           1 |         2000 |        5 |
|           1 |         2001 |        2 |
|           1 |         2002 |        6 |
|           1 |         2003 |        6 |
|           1 |         2004 |        5 |
|           1 |         2005 |       10 |
|           1 |         2006 |        4 |
|           1 |         2007 |        5 |
|           1 |         2008 |        7 |
|           1 |         2009 |       14 |
|           2 |         2000 |       10 |
|           2 |         2001 |        5 |
|           2 |         2002 |        6 |
|           2 |         2003 |        6 |
|           2 |         2004 |       10 |
|           2 |         2005 |        4 |
|           2 |         2006 |        5 |
|           2 |         2007 |        8 |
|           2 |         2008 |        8 |
|           2 |         2009 |        4 |
+-------------+--------------+----------+
```
<!-- request JSON -->
``` json
POST /search -d '
    {
    "size": 0,
    "table": "films",
    "aggs": {
        "cat_release": {
            "composite": {
                "size":5,
                "sources": [
                    { "category": { "terms": { "field": "category_id" } } },
                    { "release year": { "terms": { "field": "release_year" } } }
                ]
            }
        }
    }
    }
'
```
<!-- response JSON -->
``` json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1000,
    "total_relation": "eq",
    "hits": []
  },
  "aggregations": {
    "cat_release": {
      "after_key": {
        "category": 1,
        "release year": 2007
      },
      "buckets": [
        {
          "key": {
            "category": 1,
            "release year": 2008
          },
          "doc_count": 7
        },
        {
          "key": {
            "category": 1,
            "release year": 2009
          },
          "doc_count": 14
        },
        {
          "key": {
            "category": 1,
            "release year": 2005
          },
          "doc_count": 10
        },
        {
          "key": {
            "category": 1,
            "release year": 2004
          },
          "doc_count": 5
        },
        {
          "key": {
            "category": 1,
            "release year": 2007
          },
          "doc_count": 5
        }
      ]
    }
  }
}
```
<!-- end -->

<!-- example group4 -->
##### 获取 N 行
有时候查看每个组不止一个元素非常有用。这可以通过 `GROUP N BY` 轻松实现。例如，以下示例中，我们为每个年份获取两部电影，而不是简单的 `GROUP BY release_year` 返回的只有一部。

<!-- intro -->
##### 示例：

<!-- request SQL -->
```sql
SELECT release_year, title FROM films GROUP 2 BY release_year ORDER BY release_year DESC LIMIT 6;
```
<!-- response SQL -->
```sql
+--------------+-----------------------------+
| release_year | title                       |
+--------------+-----------------------------+
|         2009 | ALICE FANTASIA              |
|         2009 | ALIEN CENTER                |
|         2008 | AMADEUS HOLY                |
|         2008 | ANACONDA CONFESSIONS        |
|         2007 | ANGELS LIFE                 |
|         2007 | ARACHNOPHOBIA ROLLERCOASTER |
+--------------+-----------------------------+
```


<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SELECT release_year, title FROM films GROUP 2 BY release_year ORDER BY release_year DESC LIMIT 6;"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "release_year": {
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
        "release_year": 2009,
        "title": "ALICE FANTASIA"
      },
      {
        "release_year": 2009,
        "title": "ALIEN CENTER"
      },
      {
        "release_year": 2008,
        "title": "AMADEUS HOLY"
      },
      {
        "release_year": 2008,
        "title": "ANACONDA CONFESSIONS"
      },
      {
        "release_year": 2007,
        "title": "ANGELS LIFE"
      },
      {
        "release_year": 2007,
        "title": "ARACHNOPHOBIA ROLLERCOASTER"
      }
    ],
    "total": 6,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

<!-- example group5 -->
##### 组内排序
另一个关键的分析需求是对组内元素排序。为此，使用 `WITHIN GROUP ORDER BY ... {ASC|DESC}` 子句。例如，让我们为每个年份获取评分最高的电影。需要注意它与普通的 `ORDER BY` 并行执行：

* `WITHIN GROUP ORDER BY` 用于排序**组内结果**
* 而仅使用 `GROUP BY` 则是用于排序**分组本身**

这两者完全独立。


<!-- intro -->
##### 示例：

<!-- request SQL -->
```sql
SELECT release_year, title, rental_rate FROM films GROUP BY release_year WITHIN GROUP ORDER BY rental_rate DESC ORDER BY release_year DESC LIMIT 5;
```
<!-- response SQL -->
```sql
+--------------+------------------+-------------+
| release_year | title            | rental_rate |
+--------------+------------------+-------------+
|         2009 | AMERICAN CIRCUS  |    4.990000 |
|         2008 | ANTHEM LUKE      |    4.990000 |
|         2007 | ATTACKS HATE     |    4.990000 |
|         2006 | ALADDIN CALENDAR |    4.990000 |
|         2005 | AIRPLANE SIERRA  |    4.990000 |
+--------------+------------------+-------------+
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SELECT release_year, title, rental_rate FROM films GROUP BY release_year WITHIN GROUP ORDER BY rental_rate DESC ORDER BY release_year DESC LIMIT 5;"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "release_year": {
          "type": "long"
        }
      },
      {
        "title": {
          "type": "string"
        }
      },
      {
        "rental_rate": {
          "type": "long"
        }
      }
    ],
    "data": [
      {
        "release_year": 2009,
        "title": "AMERICAN CIRCUS",
        "rental_rate": 4.990000
      },
      {
        "release_year": 2009,
        "title": "ANTHEM LUKE",
        "rental_rate": 4.990000
      },
      {
        "release_year": 2008,
        "title": "ATTACKS HATE",
        "rental_rate": 4.990000
      },
      {
        "release_year": 2008,
        "title": "ALADDIN CALENDAR",
        "rental_rate": 4.990000
      },
      {
        "release_year": 2007,
        "title": "AIRPLANE SIERRA",
        "rental_rate": 4.990000
      }
    ],
    "total": 5,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

<!-- example group6 -->
##### 过滤分组
`HAVING expression` 是一个用于过滤分组的有用子句。虽然 `WHERE` 在分组之前应用，但 `HAVING` 与分组后的结果一起工作。例如，我们只保留那些年份，该年份的电影平均出租率高于 3。结果只得到四个年份：

<!-- intro -->
##### 示例：

<!-- request SQL -->
```sql
SELECT release_year, avg(rental_rate) avg FROM films GROUP BY release_year HAVING avg > 3;
```
<!-- response SQL -->
```sql
+--------------+------------+
| release_year | avg        |
+--------------+------------+
|         2002 | 3.08259249 |
|         2001 | 3.09989142 |
|         2000 | 3.17556739 |
|         2006 | 3.26184368 |
+--------------+------------+
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SELECT release_year, avg(rental_rate) avg FROM films GROUP BY release_year HAVING avg > 3;"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "release_year": {
          "type": "long"
        }
      },
      {
        "avg": {
          "type": "long long"
        }
      }
    ],
    "data": [
      {
        "release_year": 2002,
        "avg": 3.08259249
      },
      {
        "release_year": 2001,
        "avg": 3.09989142
      },
      {
        "release_year": 2000,
        "avg": 3.17556739
      },
      {
        "release_year": 2006,
        "avg": 3.26184368
      }
    ],
    "total": 4,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

**注意：** 在 [搜索查询元信息](../Node_info_and_management/SHOW_META.md#SHOW-META) 中，`total_found` 的值反映了符合 `HAVING` 条件的分组数量。当与 `GROUP BY` 一起使用 `HAVING` 子句时，这使得分页能够正确工作。

<!-- example group7 -->
##### GROUPBY()
有一个函数 `GROUPBY()`，它返回当前分组的键。它在许多情况下很有用，尤其是当你[按 MVA 分组](../Searching/Grouping.md#Grouping-by-MVA-%28multi-value-attributes%29)或者[按 JSON 值分组](../Searching/Grouping.md#Grouping-by-a-JSON-node)时。

它也可以用于 `HAVING`，例如，只保留年份 2000 和 2002。

注意，不建议在同时按多个字段分组时使用 `GROUPBY()`。它仍然会工作，但由于此时分组键是字段值的复合，可能不会按你预期的方式显示。

<!-- intro -->
##### 示例：

<!-- request SQL -->
```sql
SELECT release_year, count(*) FROM films GROUP BY release_year HAVING GROUPBY() IN (2000, 2002);
```
<!-- response SQL -->
```sql
+--------------+----------+
| release_year | count(*) |
+--------------+----------+
|         2002 |      108 |
|         2000 |       97 |
+--------------+----------+
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SELECT release_year, count(*) FROM films GROUP BY release_year HAVING GROUPBY() IN (2000, 2002);"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "release_year": {
          "type": "long"
        }
      },
      {
        "count": {
          "type": "long long"
        }
      }
    ],
    "data": [
      {
        "release_year": 2002,
        "count": 108
      },
      {
        "release_year": 2000,
        "count": 97
      }
    ],
    "total": 2,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->
<!-- example mva -->
##### 按 MVA（多值属性）分组
Manticore 支持按 [MVA](../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) 分组。为了演示其工作方式，我们创建一个包含 MVA “sizes” 的“shoes”表，并插入一些文档：
```sql
create table shoes(title text, sizes multi);
insert into shoes values(0,'nike',(40,41,42)),(0,'adidas',(41,43)),(0,'reebook',(42,43));
```
所以我们有：
```sql
SELECT * FROM shoes;
+---------------------+----------+---------+
| id                  | sizes    | title   |
+---------------------+----------+---------+
| 1657851069130080265 | 40,41,42 | nike    |
| 1657851069130080266 | 41,43    | adidas  |
| 1657851069130080267 | 42,43    | reebook |
+---------------------+----------+---------+
```
如果现在按 “sizes” 分组，它会处理所有我们的多值属性，并为每个返回聚合结果，这里只是计数：

<!-- intro -->
##### 示例：

<!-- request SQL -->
```sql
SELECT groupby() gb, count(*) FROM shoes GROUP BY sizes ORDER BY gb asc;
```
<!-- response SQL -->
```sql
+------+----------+
| gb   | count(*) |
+------+----------+
|   40 |        1 |
|   41 |        2 |
|   42 |        2 |
|   43 |        2 |
+------+----------+
```

<!-- request JSON -->
``` json
POST /search -d '
    {
     "table" : "shoes",
     "limit": 0,
     "aggs" :
     {
        "sizes" :
         {
            "terms" :
             {
              "field":"sizes",
              "size":100
             }
         }
     }
    }
'
```
<!-- response JSON -->
``` json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 3,
    "hits": [

    ]
  },
  "aggregations": {
    "sizes": {
      "buckets": [
        {
          "key": 43,
          "doc_count": 2
        },
        {
          "key": 42,
          "doc_count": 2
        },
        {
          "key": 41,
          "doc_count": 2
        },
        {
          "key": 40,
          "doc_count": 1
        }
      ]
    }
  }
}
```
<!-- request PHP -->
``` php
$index->setName('shoes');
$search = $index->search('');
$search->limit(0);
$search->facet('sizes','sizes',100);
$results = $search->get();
print_r($results->getFacets());
```
<!-- response PHP -->
``` php
Array
(
    [sizes] => Array
        (
            [buckets] => Array
                (
                    [0] => Array
                        (
                            [key] => 43
                            [doc_count] => 2
                        )
                    [1] => Array
                        (
                            [key] => 42
                            [doc_count] => 2
                        )
                    [2] => Array
                        (
                            [key] => 41
                            [doc_count] => 2
                        )
                    [3] => Array
                        (
                            [key] => 40
                            [doc_count] => 1
                        )
                )
        )
)
```
<!-- request Python -->
``` python
res =searchApi.search({"table":"shoes","limit":0,"aggs":{"sizes":{"terms":{"field":"sizes","size":100}}}})
```
<!-- response Python -->
``` python
{'aggregations': {u'sizes': {u'buckets': [{u'doc_count': 2, u'key': 43},
                                          {u'doc_count': 2, u'key': 42},
                                          {u'doc_count': 2, u'key': 41},
                                          {u'doc_count': 1, u'key': 40}]}},
 'hits': {'hits': [], 'max_score': None, 'total': 3},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- request Javascript -->
``` javascript
res = await searchApi.search({"table":"shoes","limit":0,"aggs":{"sizes":{"terms":{"field":"sizes","size":100}}}});
```

<!-- request Python-asyncio -->
``` python
res = await searchApi.search({"table":"shoes","limit":0,"aggs":{"sizes":{"terms":{"field":"sizes","size":100}}}})
```
<!-- response Python-asyncio -->
``` python
{'aggregations': {u'sizes': {u'buckets': [{u'doc_count': 2, u'key': 43},
                                          {u'doc_count': 2, u'key': 42},
                                          {u'doc_count': 2, u'key': 41},
                                          {u'doc_count': 1, u'key': 40}]}},
 'hits': {'hits': [], 'max_score': None, 'total': 3},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Javascript -->
``` javascript
res = await searchApi.search({"table":"shoes","limit":0,"aggs":{"sizes":{"terms":{"field":"sizes","size":100}}}});
```

<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"aggregations":{"sizes":{"buckets":[{"key":43,"doc_count":2},{"key":42,"doc_count":2},{"key":41,"doc_count":2},{"key":40,"doc_count":1}]}},"hits":{"total":3,"hits":[]}}
```
<!-- request Java -->
``` java
HashMap<String,Object> aggs = new HashMap<String,Object>(){{
    put("release_year", new HashMap<String,Object>(){{
        put("terms", new HashMap<String,Object>(){{
            put("field","release_year");
            put("size",100);
        }});
    }});
}};

searchRequest = new SearchRequest();
searchRequest.setIndex("films");
searchRequest.setLimit(0);
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
searchRequest.setAggs(aggs);
searchResponse = searchApi.search(searchRequest);
```
<!-- response Java -->
``` java
class SearchResponse {
    took: 0
    timedOut: false
    aggregations: {release_year={buckets=[{key=43, doc_count=2}, {key=42, doc_count=2}, {key=41, doc_count=2}, {key=40, doc_count=1}]}}
    hits: class SearchResponseHits {
        maxScore: null
        total: 3
        hits: []
    }
    profile: null
}

```

<!-- request C# -->
``` clike
var agg = new Aggregation("release_year", "release_year");
agg.Size = 100;
object query = new { match_all=null };
var searchRequest = new SearchRequest("films", query);
searchRequest.Limit = 0;
searchRequest.Aggs = new List<Aggregation> {agg};
var searchResponse = searchApi.Search(searchRequest);
```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 0
    timedOut: false
    aggregations: {release_year={buckets=[{key=43, doc_count=2}, {key=42, doc_count=2}, {key=41, doc_count=2}, {key=40, doc_count=1}]}}
    hits: class SearchResponseHits {
        maxScore: null
        total: 3
        hits: []
    }
    profile: null
}

```

<!-- request Rust -->
``` rust
let query = SearchQuery::new();
let aggTerms1 = AggTerms::new {
    fields: "release_year".to_string(),
    size: Some(100),
};
let agg1 = Aggregation {
    terms: Some(Box::new(aggTerms1)),
    ..Default::default(),
};
let mut aggs = HashMap::new();
aggs.insert("release_year".to_string(), agg1);

let search_req = SearchRequest {
    table: "films".to_string(),
    query: Some(Box::new(query)),
    aggs: serde_json::json!(aggs),
    limit: serde_json::json!(0),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```
<!-- response Rust -->
``` rust
class SearchResponse {
    took: 0
    timedOut: false
    aggregations: {release_year={buckets=[{key=43, doc_count=2}, {key=42, doc_count=2}, {key=41, doc_count=2}, {key=40, doc_count=1}]}}
    hits: class SearchResponseHits {
        maxScore: null
        total: 3
        hits: []
    }
    profile: null
}

<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  aggs: {
    mva_agg: {
      terms: { field: "mva_field", size: 2 }
    }
  }
});
```

<!-- response TypeScript -->
``` typescript
{
	"took":0,
	"timed_out":false,
	"aggregations":
	{
		"mva_agg":
		{
			"buckets":
			[{
				"key":1,
				"doc_count":4
			},
			{
				"key":2,
				"doc_count":2
			}]
		}
	},
	"hits":
	{
		"total":4,
		"hits":[]
	}
}
```

<!-- request Go -->
``` go
query := map[string]interface{} {};
searchRequest.SetQuery(query);
aggTerms := manticoreclient.NewAggregationTerms()
aggTerms.SetField("mva_field")
aggTerms.SetSize(2)
aggregation := manticoreclient.NewAggregation()
aggregation.setTerms(aggTerms)
searchRequest.SetAggregation(aggregation)
res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```

<!-- response Go -->
``` go
{
	"took":0,
	"timed_out":false,
	"aggregations":
	{
		"mva_agg":
		{
			"buckets":
			[{
				"key":1,
				"doc_count":4
			},
			{
				"key":2,
				"doc_count":2
			}]
		}
	},
	"hits":
	{
		"total":5,
		"hits":[]
	}
}
```

<!-- end -->

<!-- example json -->
##### Grouping by a JSON node
If you have a field of type [JSON](../Creating_a_table/Data_types.md#JSON), you can GROUP BY any node from it. To demonstrate this, let's create a table "products" with a few documents, each having a color in the "meta" JSON field:
```sql
create table products(title text, meta json);
insert into products values(0,'nike','{"color":"red"}'),(0,'adidas','{"color":"red"}'),(0,'puma','{"color":"green"}');
```
This gives us:
```sql
SELECT * FROM products;
+---------------------+-------------------+--------+
| id                  | meta              | title  |
+---------------------+-------------------+--------+
| 1657851069130080268 | {"color":"red"}   | nike   |
| 1657851069130080269 | {"color":"red"}   | adidas |
| 1657851069130080270 | {"color":"green"} | puma   |
+---------------------+-------------------+--------+
```
To group the products by color, we can simply use `GROUP BY meta.color`, and to display the corresponding group key in the `SELECT` list, we can use `GROUPBY()`:

<!-- intro -->
##### Example:

<!-- request SQL -->
```sql
SELECT groupby() color, count(*) from products GROUP BY meta.color;
```
<!-- response SQL -->
```sql
+-------+----------+
| color | count(*) |
+-------+----------+
| red   |        2 |
| green |        1 |
+-------+----------+
```
<!-- request JSON -->
``` json
POST /search -d '
    {
     "table" : "products",
     "limit": 0,
     "aggs" :
     {
        "color" :
         {
            "terms" :
             {
              "field":"meta.color",
              "size":100
             }
         }
     }
    }
'
```
<!-- response JSON -->
``` json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 3,
    "hits": [

    ]
  },
  "aggregations": {
    "color": {
      "buckets": [
        {
          "key": "green",
          "doc_count": 1
        },
        {
          "key": "red",
          "doc_count": 2
        }
      ]
    }
  }
}
```
<!-- request PHP -->
``` php
$index->setName('products');
$search = $index->search('');
$search->limit(0);
$search->facet('meta.color','color',100);
$results = $search->get();
print_r($results->getFacets());
```
<!-- response PHP -->
``` php
Array
(
    [color] => Array
        (
            [buckets] => Array
                (
                    [0] => Array
                        (
                            [key] => green
                            [doc_count] => 1
                        )
                    [1] => Array
                        (
                            [key] => red
                            [doc_count] => 2
                        )
                )
        )
)

```
<!-- request Python -->
``` python
res =searchApi.search({"table":"products","limit":0,"aggs":{"color":{"terms":{"field":"meta.color","size":100}}}})
```
<!-- response Python -->
``` python
{'aggregations': {u'color': {u'buckets': [{u'doc_count': 1,
                                           u'key': u'green'},
                                          {u'doc_count': 2, u'key': u'red'}]}},
 'hits': {'hits': [], 'max_score': None, 'total': 3},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

```
<!-- request Python-asyncio -->
``` python
res = await searchApi.search({"table":"products","limit":0,"aggs":{"color":{"terms":{"field":"meta.color","size":100}}}})
```
<!-- response Python-asyncio -->
``` python
{'aggregations': {u'color': {u'buckets': [{u'doc_count': 1,
                                           u'key': u'green'},
                                          {u'doc_count': 2, u'key': u'red'}]}},
 'hits': {'hits': [], 'max_score': None, 'total': 3},
 'profile': None,
 'timed_out': False,
 'took': 0}
```

<!-- request Javascript -->
``` javascript
res = await searchApi.search({"table":"products","limit":0,"aggs":{"color":{"terms":{"field":"meta.color","size":100}}}});
```
<!-- response Javascript -->
``` javascript
{"took":0,"timed_out":false,"aggregations":{"color":{"buckets":[{"key":"green","doc_count":1},{"key":"red","doc_count":2}]}},"hits":{"total":3,"hits":[]}}
```
<!-- request Java -->
``` java
HashMap<String,Object> aggs = new HashMap<String,Object>(){{
    put("color", new HashMap<String,Object>(){{
        put("terms", new HashMap<String,Object>(){{
            put("field","meta.color");
            put("size",100);
        }});
    }});
}};

searchRequest = new SearchRequest();
searchRequest.setIndex("products");
searchRequest.setLimit(0);
query = new HashMap<String,Object>();
query.put("match_all",null);
searchRequest.setQuery(query);
searchRequest.setAggs(aggs);
searchResponse = searchApi.search(searchRequest);
```
<!-- response Java -->
``` java
class SearchResponse {
    took: 0
    timedOut: false
    aggregations: {color={buckets=[{key=green, doc_count=1}, {key=red, doc_count=2}]}}
    hits: class SearchResponseHits {
        maxScore: null
        total: 3
        hits: []
    }
    profile: null
}

```

<!-- request C# -->
``` clike
var agg = new Aggregation("color", "meta.color");
agg.Size = 100;
object query = new { match_all=null };
var searchRequest = new SearchRequest("products", query);
searchRequest.Limit = 0;
searchRequest.Aggs = new List<Aggregation> {agg};
var searchResponse = searchApi.Search(searchRequest);
```
<!-- response C# -->
``` clike
class SearchResponse {
    took: 0
    timedOut: false
    aggregations: {color={buckets=[{key=green, doc_count=1}, {key=red, doc_count=2}]}}
    hits: class SearchResponseHits {
        maxScore: null
        total: 3
        hits: []
    }
    profile: null
}

```

<!-- request Rust -->
``` rust
let query = SearchQuery::new();
let aggTerms1 = AggTerms::new {
    fields: "meta.color".to_string(),
    size: Some(100),
};
let agg1 = Aggregation {
    terms: Some(Box::new(aggTerms1)),
    ..Default::default(),
};
let mut aggs = HashMap::new();
aggs.insert("color".to_string(), agg1);

let search_req = SearchRequest {
    table: "products".to_string(),
    query: Some(Box::new(query)),
    aggs: serde_json::json!(aggs),
    limit: serde_json::json!(0),
    ..Default::default(),
};
let search_res = search_api.search(search_req).await;
```
<!-- response Rust -->
``` rust
class SearchResponse {
    took: 0
    timedOut: false
    aggregations: {color={buckets=[{key=green, doc_count=1}, {key=red, doc_count=2}]}}
    hits: class SearchResponseHits {
        maxScore: null
        total: 3
        hits: []
    }
    profile: null
}

```

<!-- request TypeScript -->
``` typescript
res = await searchApi.search({
  index: 'test',
  aggs: {
    json_agg: {
      terms: { field: "json_field.year", size: 1 }
    }
  }
});
```

<!-- response TypeScript -->
``` typescript
{
	"took":0,
	"timed_out":false,
	"aggregations":
	{
		"json_agg":
		{
			"buckets":
			[{
				"key":2000,
				"doc_count":2
			},
			{
				"key":2001,
				"doc_count":2
			}]
		}
	},
	"hits":
	{
		"total":4,
		"hits":[]
	}
}
```

<!-- request Go -->
``` go
query := map[string]interface{} {};
searchRequest.SetQuery(query);
aggTerms := manticoreclient.NewAggregationTerms()
aggTerms.SetField("json_field.year")
aggTerms.SetSize(2)
aggregation := manticoreclient.NewAggregation()
aggregation.setTerms(aggTerms)
searchRequest.SetAggregation(aggregation)
res, _, _ := apiClient.SearchAPI.Search(context.Background()).SearchRequest(*searchRequest).Execute()
```

<!-- response Go -->
``` go
{
	"took":0,
	"timed_out":false,
	"aggregations":
	{
		"json_agg":
		{
			"buckets":
			[{
				"key":2000,
				"doc_count":2
			},
			{
				"key":2001,
				"doc_count":2
			}]
		}
	},
	"hits":
	{
		"total":4,
		"hits":[]
	}
}
```

<!-- end -->

## 聚合函数
除了返回每个组中元素数量的 `COUNT(*)` 之外，你还可以使用各种其他聚合函数：
<!-- example distinct -->
##### COUNT(DISTINCT field)
虽然 `COUNT(*)` 返回组内所有元素的数量，`COUNT(DISTINCT field)` 返回该组中某字段的唯一值个数，这可能与总数完全不同。例如，你可能在组中有 100 个元素，但该字段的值都相同。`COUNT(DISTINCT field)` 帮助确定这一点。为演示此功能，创建一个包含学生姓名、年龄和专业的 "students" 表：
```sql
CREATE TABLE students(name text, age int, major string);
INSERT INTO students values(0,'John',21,'arts'),(0,'William',22,'business'),(0,'Richard',21,'cs'),(0,'Rebecca',22,'cs'),(0,'Monica',21,'arts');
```

所以我们有：

```sql
MySQL [(none)]> SELECT * from students;
+---------------------+------+----------+---------+
| id                  | age  | major    | name    |
+---------------------+------+----------+---------+
| 1657851069130080271 |   21 | arts     | John    |
| 1657851069130080272 |   22 | business | William |
| 1657851069130080273 |   21 | cs       | Richard |
| 1657851069130080274 |   22 | cs       | Rebecca |
| 1657851069130080275 |   21 | arts     | Monica  |
+---------------------+------+----------+---------+
```

在示例中，你可以看到如果对专业进行 GROUP BY 并同时显示 `COUNT(*)` 和 `COUNT(DISTINCT age)`，很明显选择专业为 "cs" 的有两名学生，且年龄各不相同，而专业为 "arts" 的虽然也有两名学生，却只有一个唯一年龄。

每个查询最多只能有一个 `COUNT(DISTINCT)`。

** 默认情况下，统计数为近似值 **

实际上，其中一些是精确的，另一些则是近似的。下面会详细说明。

Manticore 支持两种计算唯一值数量的算法。一种是使用大量内存且通常较慢的旧算法。它收集 `{group; value}` 对，进行排序，并定期去重。该方法的优点是在普通表内保证统计的精确性。你可以通过将 [distinct_precision_threshold](../Searching/Options.md#distinct_precision_threshold) 选项设置为 `0` 来启用它。

另一种算法（默认启用）将计数加载到哈希表中，并返回其大小。如果哈希表变得过大，其内容会转移到 `HyperLogLog`。这时计数结果变为近似，因为 `HyperLogLog` 是一种概率算法。优点是每组最大内存使用固定，取决于 `HyperLogLog` 的准确度。总体内存使用还取决于反映组数量的 [max_matches](../Searching/Options.md#max_matches) 设置。

[distinct_precision_threshold](../Searching/Options.md#distinct_precision_threshold) 选项设置了统计保证精确的阈值。`HyperLogLog` 的准确性设置及“从哈希表到 HyperLogLog”的转换阈值均由此选项派生。使用时应谨慎，因为该参数翻倍会使计数计算所需最大内存翻倍。最大内存使用可以粗略估算为 `64 * max_matches * distinct_precision_threshold`。请注意这是最坏情况，大多数情况下计数计算所用 RAM 会远低于此值。

**针对包含多个磁盘分区的分布式表或实时表，`COUNT(DISTINCT)` 可能返回不准确的结果**，但对于由本地普通表或实时表（具有相同模式，即字段集合/顺序相同，但标记化设置可能不同）组成的分布式表，结果应该是准确的。

<!-- intro -->
##### 示例：

<!-- request SQL -->
```sql
SELECT major, count(*), count(distinct age) FROM students GROUP BY major;
```
<!-- response SQL -->
```sql
+----------+----------+---------------------+
| major    | count(*) | count(distinct age) |
+----------+----------+---------------------+
| arts     |        2 |                   1 |
| business |        1 |                   1 |
| cs       |        2 |                   2 |
+----------+----------+---------------------+
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw - d "SELECT major, count(*), count(distinct age) FROM students GROUP BY major;"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "major": {
          "type": "string"
        }
      },
      {
        "count(*)": {
          "type": "long long"
        }
      },
      {
        "count(distinct age)": {
          "type": "long long"
        }
      }
    ],
    "data": [
      {
        "major": "arts",
        "count(*)": 2,
        "count(distinct age)": 1
      },
      {
        "major": "business",
        "count(*)": 1,
        "count(distinct age)": 1
      },
      {
        "major": "cs",
        "count(*)": 2,
        "count(distinct age)": 2
      }
    ],
    "total": 3,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

<!-- example concat -->
##### GROUP_CONCAT(field)

有时，你希望更好地了解每个组的内容。可以使用 [GROUP N BY](../Searching/Grouping.md#Give-me-N-rows) 实现，但那会返回你可能不希望看到的额外行。`GROUP_CONCAT()` 通过将组中特定字段的值连接起来丰富你的分组结果。让我们改进前面的例子，显示每个组中所有的年龄。

`GROUP_CONCAT(field)` 返回以逗号分隔的值列表。

<!-- intro -->
##### 示例：

<!-- request SQL -->
```sql
SELECT major, count(*), count(distinct age), group_concat(age) FROM students GROUP BY major
```
<!-- response SQL -->
```sql
+----------+----------+---------------------+-------------------+
| major    | count(*) | count(distinct age) | group_concat(age) |
+----------+----------+---------------------+-------------------+
| arts     |        2 |                   1 | 21,21             |
| business |        1 |                   1 | 22                |
| cs       |        2 |                   2 | 21,22             |
+----------+----------+---------------------+-------------------+
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SELECT major, count(*), count(distinct age), group_concat(age) FROM students GROUP BY major"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "major": {
          "type": "string"
        }
      },
      {
        "count(*)": {
          "type": "long long"
        }
      },
      {
        "count(distinct age)": {
          "type": "long long"
        }
      },
      {
        "group_concat(age)": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "major": "arts",
        "count(*)": 2,
        "count(distinct age)": 1,
        "group_concat(age)": 21,21
      },
      {
        "major": "business",
        "count(*)": 1,
        "count(distinct age)": 1,
        "group_concat(age)": 22
      },
      {
        "major": "cs",
        "count(*)": 2,
        "count(distinct age)": 2,
        "group_concat(age)": 21,22
      }
    ],
    "total": 3,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->
<!-- example sum -->
##### SUM(), MIN(), MAX(), AVG()
当然，你还可以获取组内的总和、平均值、最小值和最大值。

<!-- intro -->
##### 示例：

<!-- request SQL -->
```sql
SELECT release_year year, sum(rental_rate) sum, min(rental_rate) min, max(rental_rate) max, avg(rental_rate) avg FROM films GROUP BY release_year ORDER BY year asc LIMIT 5;
```
<!-- response SQL -->
```sql
+------+------------+----------+----------+------------+
| year | sum        | min      | max      | avg        |
+------+------------+----------+----------+------------+
| 2000 | 308.030029 | 0.990000 | 4.990000 | 3.17556739 |
| 2001 | 282.090118 | 0.990000 | 4.990000 | 3.09989142 |
| 2002 | 332.919983 | 0.990000 | 4.990000 | 3.08259249 |
| 2003 | 310.940063 | 0.990000 | 4.990000 | 2.93339682 |
| 2004 | 300.920044 | 0.990000 | 4.990000 | 2.78629661 |
+------+------------+----------+----------+------------+
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SELECT release_year year, sum(rental_rate) sum, min(rental_rate) min, max(rental_rate) max, avg(rental_rate) avg FROM films GROUP BY release_year ORDER BY year asc LIMIT 5;"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "year": {
          "type": "long"
        }
      },
      {
        "sum": {
          "type": "long long"
        }
      },
      {
        "min": {
          "type": "long long"
        }
      },
      {
        "max": {
          "type": "long long"
        }
      },
      {
        "avg": {
          "type": "long long"
        }
      }
    ],
    "data": [
      {
        "year": 2000,
        "sum": 308.030029,
        "min": 0.990000,
        "max": 4.990000,
        "avg": 3.17556739
      },
      {
        "year": 2001,
        "sum": 282.090118,
        "min": 0.990000,
        "max": 4.990000,
        "avg": 3.09989142
      },
      {
        "year": 2002,
        "sum": 332.919983,
        "min": 0.99,
        "max": 4.990000,
        "avg": 3.08259249
      },
      {
        "year": 2003,
        "sum": 310.940063,
        "min": 0.990000,
        "max": 4.990000,
        "avg": 2.93339682
      },
      {
        "year": 2004,
        "sum": 300.920044,
        "min": 0.990000,
        "max": 4.990000,
        "avg": 2.78629661
      }
    ],
    "total": 5,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

<!-- example accuracy -->
## 分组准确性

分组在固定内存中进行，该内存大小取决于 [max_matches](../Searching/Options.md#max_matches) 设置。如果 `max_matches` 允许存储所有找到的分组，结果将是 100% 精确的。然而，如果 `max_matches` 取值较低，则结果的准确度会降低。

当涉及并行处理时，情况可能变得更复杂。当启用 `pseudo_sharding` 和/或使用具有多个磁盘分块的 RT 表时，每个分块或伪分片获得的结果集大小不超过 `max_matches`。当来自不同线程的结果集合并时，这可能导致聚合和分组计数的不准确。为了解决此问题，可以使用更大的 `max_matches` 值或禁用并行处理。

如果 Manticore 检测到 groupby 可能返回不准确的结果，它将尝试将 `max_matches` 增加到 [max_matches_increase_threshold](../Searching/Options.md#max_matches_increase_threshold)。检测是基于从辅助索引（如果存在）中检索到的 groupby 属性的唯一值数量。

为了确保在使用 RT 表或 `pseudo_sharding` 时聚合和/或分组计数的准确性，可以启用 `accurate_aggregation`。这将尝试将 `max_matches` 增加到阈值，如果阈值仍不够高，Manticore 将禁用该查询的并行处理。

<!-- intro -->
##### 示例：

<!-- request SQL -->
```sql
MySQL [(none)]> SELECT release_year year, count(*) FROM films GROUP BY year limit 5;
+------+----------+
| year | count(*) |
+------+----------+
| 2004 |      108 |
| 2002 |      108 |
| 2001 |       91 |
| 2005 |       93 |
| 2000 |       97 |
+------+----------+

MySQL [(none)]> SELECT release_year year, count(*) FROM films GROUP BY year limit 5 option max_matches=1;
+------+----------+
| year | count(*) |
+------+----------+
| 2004 |       76 |
+------+----------+

MySQL [(none)]> SELECT release_year year, count(*) FROM films GROUP BY year limit 5 option max_matches=2;
+------+----------+
| year | count(*) |
+------+----------+
| 2004 |       76 |
| 2002 |       74 |
+------+----------+

MySQL [(none)]> SELECT release_year year, count(*) FROM films GROUP BY year limit 5 option max_matches=3;
+------+----------+
| year | count(*) |
+------+----------+
| 2004 |      108 |
| 2002 |      108 |
| 2001 |       91 |
+------+----------+
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SELECT release_year year, count(*) FROM films GROUP BY year limit 5;"
[
  {
    "columns": [
      {
        "year": {
          "type": "long"
        }
      },
      {
        "count(*)": {
          "type": "long long"
        }
      }
    ],
    "data": [
      {
        "year": 2004,
        "count(*)": 108
      },
      {
        "year": 2002,
        "count(*)": 108
      },
      {
        "year": 2001,
        "count(*)": 91
      },
      {
        "year": 2005,
        "count(*)": 93
      },
      {
        "year": 2000,
        "count(*)": 97
      },
    ],
    "total": 5,
    "error": "",
    "warning": ""
  }
]
POST /sql?mode=raw -d "SELECT release_year year, count(*) FROM films GROUP BY year limit 5 option max_matches=1;"
[
  {
    "columns": [
      {
        "year": {
          "type": "long"
        }
      },
      {
        "count(*)": {
          "type": "long long"
        }
      }
    ],
    "data": [
      {
        "year": 2004,
        "count(*)": 76
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
POST /sql?mode=raw -d "SELECT release_year year, count(*) FROM films GROUP BY year limit 5 option max_matches=2;"
[
  {
    "columns": [
      {
        "year": {
          "type": "long"
        }
      },
      {
        "count(*)": {
          "type": "long long"
        }
      }
    ],
    "data": [
      {
        "year": 2004,
        "count(*)": 76
      },
      {
        "year": 2002,
        "count(*)": 74
      }
    ],
    "total": 2,
    "error": "",
    "warning": ""
  }
]
POST /sql?mode=raw -d "SELECT release_year year, count(*) FROM films GROUP BY year limit 5 option max_matches=3;"
[
  {
    "columns": [
      {
        "year": {
          "type": "long"
        }
      },
      {
        "count(*)": {
          "type": "long long"
        }
      }
    ],
    "data": [
      {
        "year": 2004,
        "count(*)": 108
      },
      {
        "year": 2002,
        "count(*)": 108
      },
      {
        "year": 2001,
        "count(*)": 91
      }
    ],
    "total": 3,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->
<!-- proofread -->

