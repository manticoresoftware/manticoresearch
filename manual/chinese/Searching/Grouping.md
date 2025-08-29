# 分组搜索结果

<!-- example general -->
分组搜索结果通常有助于获取每组匹配计数或其他聚合结果。例如，创建图表展示每月匹配博客文章数量，或按网站分组网页搜索结果，或按作者分组论坛帖子等非常有用。

Manticore 支持按单列或多列及计算表达式对搜索结果进行分组。结果可以：

* 在组内排序
* 每组返回多行
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
JSON 查询格式目前支持基本分组，可以检索聚合值及其 count(*)。

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

标准查询输出返回未分组的结果集，可以使用 `limit`（或 `size`）隐藏。
聚合需要设置 `size` 作为组结果集大小。

<!-- end -->

<!-- example group1 -->
### 仅分组
分组非常简单 —— 只需在 `SELECT` 查询末尾添加 "GROUP BY smth"。其中的某物可以是：

* 表中的任何非全文字段：整数、浮点数、字符串、多值属性（MVA）
* 或者，如果你在 `SELECT` 列表中用了别名，也可以按别名分组

你可以省略 `SELECT` 列表中的任何[聚合函数](../Searching/Grouping.md#Aggregation-functions)，分组仍然有效：

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
<!-- end -->
<!-- example group2 -->
但在大多数情况下，你会想获得每组的一些聚合数据，比如：

* `COUNT(*)` 简单获取每组的元素数量
* 或用 `AVG(field)` 计算组内字段的平均值

对于 HTTP JSON 请求，主查询级别使用单个带 `limit=0` 的 `aggs` 桶，效果类似于 SQL 查询中带 `GROUP BY` 和 `COUNT(*)`，性能和行为相当。

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
默认情况下，组不排序，通常你想按某个维度对它们排序，比如分组字段：

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
<!-- end -->
<!-- example sort2 -->
或者，你也可以根据聚合结果排序：

* 按 `count(*)` 让元素最多的组显示在最前面
* 按 `avg(rental_rate)` 显示评分最高的电影优先。注意示例中通过别名实现：先在 `SELECT` 列表中把 `avg(rental_rate)` 映射为 `avg`，然后直接用 `ORDER BY avg`


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
<!-- end -->

<!-- example group3 -->
##### 一次 GROUP BY 多个字段
有时你不只想按单个字段分组，而是同时按多个字段分组，比如电影的类别和年份：

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
有时你希望每组不只看一个元素，而是多个。使用 `GROUP N BY` 可以轻松实现。例如，下面示例对每个年份获取两部电影，而不是单个 `GROUP BY release_year` 那样只返回一个。

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
<!-- end -->

<!-- example group5 -->
##### 组内排序
另一个重要的分析需求是在组内对元素排序。可以用 `WITHIN GROUP ORDER BY ... {ASC|DESC}` 实现。例如，获取每年评分最高的电影。注意它与 `ORDER BY` 并行工作：

* `WITHIN GROUP ORDER BY` 排序**组内的结果**
* 而单独的 `GROUP BY` 排序**组本身**

两者完全独立。


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
<!-- end -->

<!-- example group6 -->
##### 过滤分组
`HAVING expression` 是过滤分组的有用子句。`WHERE` 在分组前应用，`HAVING` 针对分组应用。例如，只保留那些年份中影片平均租金高于 3 的年份。结果只有四个年份：

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
<!-- end -->

注意，`HAVING` 不会影响[搜索查询元信息](../Node_info_and_management/SHOW_META.md#SHOW-META)中的 `total_found`。

<!-- example group7 -->
##### GROUPBY()
有一个函数`GROUPBY()`，它返回当前分组的键。在许多情况下非常有用，特别是当你[按MVA分组](../Searching/Grouping.md#Grouping-by-MVA-%28multi-value-attributes%29)或按[JSON值分组](../Searching/Grouping.md#Grouping-by-a-JSON-node)时。

它也可以在`HAVING`中使用，例如，只保留年份为2000和2002的结果。

请注意，当你同时按多个字段进行GROUP BY时，不推荐使用`GROUPBY()`。它仍然可以工作，但因为此时分组键是字段值的复合，它可能不会出现你期望的样子。

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
<!-- end -->
<!-- example mva -->
##### 按MVA（多值属性）分组
Manticore支持按[MVA](../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29)分组。为了演示其工作原理，我们创建一个包含MVA字段"sizes"的表"shoes"并插入一些文档：
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
如果我们现在按"sizes"分组，它将处理所有我们的多值属性并返回每个的聚合值，在这里仅为计数：

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
除了返回每组元素数量的 `COUNT(*)`，你还可以使用其他各种聚合函数：
<!-- example distinct -->
##### COUNT(DISTINCT field)
虽然 `COUNT(*)` 返回组内所有元素的数量，但 `COUNT(DISTINCT field)` 返回组内字段唯一值的数量，这个数量可能和总数完全不同。例如，你可以有 100 个元素，但某个字段的值全部相同。`COUNT(DISTINCT field)` 可以帮助确定这一点。为了演示这一点，让我们创建一个包含学生姓名、年龄和专业的表 "students"：
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

在示例中，你可以看到如果按 major 分组并同时显示 `COUNT(*)` 和 `COUNT(DISTINCT age)`，很明显选择 "cs" 专业的学生有两名且年龄各不相同，而 "arts" 专业同样有两名学生，但年龄只有一个唯一值。

每个查询最多只能有一个 `COUNT(DISTINCT)`。

** 默认情况下，计数是近似值 **

实际上，有些计数是精确的，而另一些是近似的。详细说明见下文。

Manticore 支持两种计算不同值计数的算法。一种是使用大量内存且通常较慢的传统算法。它收集 `{group; value}` 对，排序，并定期去除重复值。这种算法的优点是在普通表中可以保证计数的精确。你可以通过将 [distinct_precision_threshold](../Searching/Options.md#distinct_precision_threshold) 选项设置为 `0` 来启用它。

另一种算法（默认启用）将计数加载到哈希表中并返回其大小。如果哈希表变得过大，其内容会被转换成 `HyperLogLog`。此时计数成为近似值，因为 `HyperLogLog` 是一种概率算法。优点是每组最大内存使用量是固定的，且取决于 `HyperLogLog` 的准确度。总体内存使用量还取决于反映组数量的 [max_matches](../Searching/Options.md#max_matches) 设置。

[distinct_precision_threshold](../Searching/Options.md#distinct_precision_threshold) 选项设置保证计数精确的阈值。`HyperLogLog` 的准确度设置和从“哈希表到 HyperLogLog”转换的阈值均由该设置推导。使用此选项时需小心，因为将其数值翻倍会使计数计算所需内存最大值翻倍。最大内存使用量大致可用此公式估测：`64 * max_matches * distinct_precision_threshold`。请注意这是最坏情况，实际计数计算大多使用的内存远少于此。

**针对由多个磁盘块组成的分布式表或实时表，`COUNT(DISTINCT)` 可能返回不准确结果**，但对于由具有相同模式（字段集/顺序相同，可能有不同分词设置）的本地普通或实时表组成的分布式表，结果应当是准确的。

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
<!-- end -->

<!-- example concat -->
##### GROUP_CONCAT(field)

通常，你希望更好地了解每个分组的内容。你可以使用 [GROUP N BY](../Searching/Grouping.md#Give-me-N-rows) 实现，但它会返回你可能不想要的额外行。`GROUP_CONCAT()` 通过连接组内指定字段的值来丰富分组展示。我们用前面的例子改进，显示每组内所有年龄。

`GROUP_CONCAT(field)` 返回逗号分隔的列表。

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
<!-- end -->

<!-- example accuracy -->
## 分组准确度

分组在固定内存中完成，该内存依赖于 [max_matches](../Searching/Options.md#max_matches) 设置。如果 `max_matches` 足够存储所有找到的分组，结果将是 100% 准确的。但如果 `max_matches` 较小，结果的准确度会降低。

当涉及并行处理时，情况会更复杂。当启用 `pseudo_sharding` 和/或使用包含多个磁盘块的 RT 表时，每个块或伪分片得到的结果集最大不超过 `max_matches`。这在不同线程合并结果集时可能导致聚合和分组计数不准确。解决方法是增大 `max_matches` 选项或禁用并行处理。

若检测到 groupby 可能返回不准确结果，Manticore 会尝试将 `max_matches` 增加到 [max_matches_increase_threshold](../Searching/Options.md#max_matches_increase_threshold)。检测基于从二级索引（若存在）获取的分组属性唯一值数量。

使用 RT 表或 `pseudo_sharding` 时，为确保聚合和/或分组计数准确，可以启用 `accurate_aggregation`。该选项将尝试将 `max_matches` 增加至阈值，若阈值不足，Manticore 会禁用该查询的并行处理。

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
<!-- end -->
<!-- proofread -->

