# 分组搜索结果

<!-- example general -->
分组搜索结果通常有助于获得每个组的匹配计数或其他聚合。例如，它对于创建显示每月匹配博客文章数量的图表，或者按网站对网络搜索结果分组，或按作者对论坛帖子分组等非常有用。

Manticore 支持按单列或多列以及计算表达式对搜索结果进行分组。结果可以：

* 在组内排序
* 每组返回多行
* 过滤组
* 对组排序
* 使用[聚合函数](../Searching/Grouping.md#Aggregation-functions)进行聚合

<!-- intro -->
一般语法是：

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
JSON 查询格式当前支持基本分组，可以检索聚合值及其 count(*)。

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

标准查询输出返回未分组的结果集，可以使用 `limit`（或 `size`）将其隐藏。
聚合需要设置组结果集大小的 `size`。

<!-- end -->

<!-- example group1 -->
### 仅分组
分组非常简单——只需在 `SELECT` 查询末尾添加 "GROUP BY smth"。这里的某物可以是：

* 表中的任何非全文字段：整数、浮点数、字符串、多值属性（MVA）
* 或者，如果你在 `SELECT` 列表中使用了别名，也可以按该别名分组

你可以在 `SELECT` 列表中省略任何[聚合函数](../Searching/Grouping.md#Aggregation-functions)，它仍然可以工作：

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
不过在大多数情况下，你会想获得每个组的一些聚合数据，比如：

* `COUNT(*)` 仅获取每组元素数量
* 或 `AVG(field)` 计算组内字段的平均值

对于 HTTP JSON 请求，主查询层使用单个带 `limit=0` 的 `aggs` bucket，效果类似于 SQL 查询中的 `GROUP BY` 和 `COUNT(*)`，提供等效的行为和性能。

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
##### 分组排序
默认情况下，组不排序，通常接下来你会想按某个字段排序，比如你分组的字段：

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
或者，你可以按聚合排序：

* 按 `count(*)` 显示元素最多的组优先
* 按 `avg(rental_rate)` 显示评分最高的电影优先。注意在示例中是通过别名完成的：`avg(rental_rate)` 首先映射为 `avg`，然后直接使用 `ORDER BY avg`


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
##### 同时按多个字段 GROUP BY
有时，你可能想不仅按单个字段分组，而是同时按多个字段，比如电影的类别和年份：

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
##### 每组返回 N 行
有时不仅想查看每组一个元素，而是多个，这可以通过 `GROUP N BY` 轻松实现。例如，下面的情况我们每年获取两部电影，而不是简单的 `GROUP BY release_year` 只返回一部。

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
另一个关键的分析需求是对组内元素排序。为此，使用 `WITHIN GROUP ORDER BY ... {ASC|DESC}` 子句。例如，我们想获取每年评分最高的电影。注意它与简单的 `ORDER BY` 并行工作：

* `WITHIN GROUP ORDER BY` 用于排序**组内结果**
* 而简单的 `GROUP BY` **排序的是组本身**

两者彼此完全独立。


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
##### 过滤组
`HAVING expression` 是一个用于过滤分组的有用子句。虽然 `WHERE` 在分组之前应用，`HAVING` 则作用于分组。例如，我们只保留那些该年份电影的平均租赁费率高于 3 的年份。结果只剩下四个年份：

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

<!-- example group7 -->
##### GROUPBY()
有一个函数 `GROUPBY()`，它返回当前分组的键。在很多情况下非常有用，特别是当你[对 MVA 进行 GROUP BY](../Searching/Grouping.md#Grouping-by-MVA-%28multi-value-attributes%29)或对[JSON 值进行 GROUP BY](../Searching/Grouping.md#Grouping-by-a-JSON-node)时。

它也可以在 `HAVING` 中使用，例如，只保留年份 2000 和 2002。

注意，当你同时对多个字段进行 GROUP BY 时，不推荐使用 `GROUPBY()`。它仍然可以工作，但由于此时分组键是字段值的复合，可能不会按你预期的方式显示。

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
Manticore 支持按[MVA](../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29)分组。为演示其工作方式，我们创建一个带有 MVA “sizes” 的表 "shoes"，并插入几条文档：
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
如果我们现在对 "sizes" 进行 GROUP BY，它会处理所有的多值属性，并为每个值返回一个聚合，在此例中只是计数：

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
除了返回每组元素数量的 `COUNT(*)` 外，你还可以使用各种其他聚合函数：
<!-- example distinct -->
##### COUNT(DISTINCT 字段)
虽然 `COUNT(*)` 返回组中所有元素的数量，但 `COUNT(DISTINCT 字段)` 返回该字段在组中唯一值的数量，这可能与总数完全不同。例如，你可以有 100 个元素在组中，但它们某个字段的值都是相同的。`COUNT(DISTINCT 字段)` 有助于判断这一点。为演示此功能，创建一个包含学生姓名、年龄和专业的表 "students"：
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

在示例中，你可以看到如果我们按专业进行 GROUP BY 并同时显示 `COUNT(*)` 和 `COUNT(DISTINCT age)`，很明显专业为 "cs" 的有两名学生，他们的年龄各不相同，而专业为 "arts" 的同样有两名学生，但是只有一个唯一年龄。

每个查询最多只能有一个 `COUNT(DISTINCT)`。

** 默认情况下，计数是近似的 **

实际上，有些计数是精确的，有些是近似的。下面会详细说明。

Manticore 支持两种计算不同值计数的算法。一种是传统算法，使用大量内存且通常速度较慢。它收集 `{group; value}` 对，排序并定期剔除重复项。这种方法的好处是能保证在纯表中计数的精确。你可以通过将 [distinct_precision_threshold](../Searching/Options.md#distinct_precision_threshold) 选项设置为 `0` 来启用它。

另一种算法（默认启用）将计数加载到哈希表中并返回其大小。如果哈希表变得过大，其内容会被移动至 `HyperLogLog`。这时计数成为近似值，因为 `HyperLogLog` 是概率算法。优点是每组最大内存使用固定，且依赖于 `HyperLogLog` 的精度。整体内存消耗也受 [max_matches](../Searching/Options.md#max_matches) 设置影响，该设置反映了组的数量。

[distinct_precision_threshold](../Searching/Options.md#distinct_precision_threshold) 选项设置了计数保证精确的阈值。`HyperLogLog` 的精度设置以及从哈希表转换到 `HyperLogLog` 的阈值都来源于该设置。使用此选项需谨慎，因为阈值加倍将使计算计数所需的最大内存加倍。最大内存使用可粗略估算为：`64 * max_matches * distinct_precision_threshold`。请注意这是最坏情况，大多数情况下计数计算会使用明显更少的内存。

** 对分布式表或包含多个磁盘块的实时表执行 `COUNT(DISTINCT)` 可能返回不准确结果，**但对于由本地纯表或实时表（结构相同）组成的分布式表，结果应当准确（字段集合/顺序一致，但可能有不同的分词设置）。

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
##### GROUP_CONCAT(字段)

通常，你想更好地了解每个分组的内容。你可以使用 [GROUP N BY](../Searching/Grouping.md#Give-me-N-rows) 来实现，但它会返回你可能不想要的额外行。`GROUP_CONCAT()` 通过将组中特定字段的值连接起来丰富你的分组。让我们用之前的例子来改进，通过显示每个组中所有年龄。

`GROUP_CONCAT(字段)` 返回逗号分隔的值列表。

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
当然，你也可以获得组内的总和、平均值、最小值和最大值。

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
## 分组精度

分组是在固定内存中完成的，内存大小取决于 [max_matches](../Searching/Options.md#max_matches) 设置。如果 `max_matches` 允许存储所有找到的组，结果将是 100% 精确的。然而，如果 `max_matches` 的值较小，则结果会降低准确性。

当涉及并行处理时，情况会更复杂。启用 `pseudo_sharding` 和/或使用带有多个磁盘块的 RT 表时，每个块或伪分片获得的结果集大小不超过 `max_matches`。这可能在不同线程的结果集合并时导致聚合和分组计数不准确。为解决此问题，可以使用更大的 `max_matches` 值或禁用并行处理。

Manticore 将尝试将 `max_matches` 增加到 [max_matches_increase_threshold](../Searching/Options.md#max_matches_increase_threshold)，如果它检测到 groupby 可能返回不准确的结果。检测基于 groupby 属性的唯一值数量，这些值是从二级索引（如果存在）中检索的。

为了确保在使用 RT 表或 `pseudo_sharding` 时聚合和/或组计数的准确性，可以启用 `accurate_aggregation`。这将尝试将 `max_matches` 增加到阈值，如果阈值不够高，Manticore 将禁用该查询的并行处理。

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

