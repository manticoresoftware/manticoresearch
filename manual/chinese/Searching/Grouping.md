# 分组搜索结果

<!-- example general -->
分组搜索结果通常有助于获取每个组的匹配计数或其他聚合。例如，它对于创建按月匹配博客帖子数量的图表，或按站点分组网页搜索结果，或按作者分组论坛帖子等非常有用。

Manticore 支持按单列、多列和计算表达式对搜索结果进行分组。结果可以：

* 在组内排序
* 每组返回多行
* 对组进行过滤
* 对组进行排序
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
JSON 查询格式目前支持基本分组，可检索聚合值及其 count(*)。

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

标准查询输出返回未分组的结果集，可以通过 `limit`（或 `size`）将其隐藏。
聚合需要设置组结果集大小 `size`。

<!-- end -->

<!-- example group1 -->
### 仅分组
分组相当简单——只需在 `SELECT` 查询末尾添加 "GROUP BY smth"。该某物可以是：

* 表中的任何非全文字段：整数、浮点、字符串、多值属性(MVA)
* 或者，如果你在 `SELECT` 列表中使用了别名，也可以按它分组

你可以省略 `SELECT` 列表中的任何[聚合函数](../Searching/Grouping.md#Aggregation-functions)，仍然有效：

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
然而，在大多数情况下，你会想为每个组获得一些聚合数据，例如：

* `COUNT(*)` 仅仅获取每组的元素数量
* 或 `AVG(field)` 计算组内字段的平均值

对于 HTTP JSON 请求，使用单个 `aggs` 桶，并在主查询级别设置 `limit=0`，效果类似于 SQL 查询中的 `GROUP BY` 和 `COUNT(*)`，提供等效的行为和性能。

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
##### 组内排序
默认情况下，组不排序，接下来你通常想做的是按某个字段排序，比如你分组用的字段：

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
另外，你也可以按聚合排序：

* 按 `count(*)` 排序，优先显示包含最多元素的组
* 按 `avg(rental_rate)` 排序，优先显示评分最高的电影。注意，在示例中，这是通过别名完成的：先在 `SELECT` 列表中将 `avg(rental_rate)` 映射为 `avg`，然后简单地执行 `ORDER BY avg`


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
##### 一次按多个字段 GROUP BY
有时你可能想按多个字段分组，而不仅仅是单个字段，比如电影的类别和年份：

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
##### 给我 N 行
有时看到每组不止一个元素也很有用。这可以通过 `GROUP N BY` 很容易实现。例如，下面的例子中，我们为每个年份获取两部电影，而不是简单 `GROUP BY release_year` 只返回一部。

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
另一个关键的分析需求是对组内元素排序。为此，使用 `WITHIN GROUP ORDER BY ... {ASC|DESC}` 子句。例如，获取每年评分最高的电影。注意它与普通的 `ORDER BY` 并行工作：

* `WITHIN GROUP ORDER BY` 对**组内**结果排序
* 而仅用 `GROUP BY` 对**组本身**排序

这两者完全独立工作。


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
##### 过滤组
`HAVING expression` 是过滤组的有用子句。`WHERE` 用于分组前过滤，而 `HAVING` 用于分组后过滤。例如，我们只保留那些该年电影平均租赁率高于3的年份。结果只有四个年份：

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

<!-- example group7 -->
##### GROUPBY()
有一个函数 `GROUPBY()`，它返回当前分组的键。在许多情况下非常有用，特别是当你[按 MVA 分组](../Searching/Grouping.md#Grouping-by-MVA-%28multi-value-attributes%29)或按[JSON 值分组](../Searching/Grouping.md#Grouping-by-a-JSON-node)时。

它也可以用于 `HAVING`，例如，只保留年份 2000 和 2002。

注意，当你一次按多个字段进行 GROUP BY 时，不推荐使用 `GROUPBY()`。它仍然可以工作，但由于此时的分组键是字段值的复合，可能不会以你期望的方式出现。

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
##### 按 MVA（多值属性）分组
Manticore 支持按[MVA](../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29)分组。为了演示其工作原理，我们创建一个包含 MVA 字段 "sizes" 的表 "shoes"，并插入一些文档：
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
如果现在按 "sizes" 进行 GROUP BY，它将处理所有多值属性，并为每个返回一个聚合，在此例中仅是计数：

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

## Aggregation functions
Besides `COUNT(*)`, which returns the number of elements in each group, you can use various other aggregation functions:
<!-- example distinct -->
##### COUNT(DISTINCT field)
While `COUNT(*)` returns the number of all elements in the group, `COUNT(DISTINCT field)` returns the number of unique values of the field in the group, which may be completely different from the total count. For instance, you can have 100 elements in the group, but all with the same value for a certain field. `COUNT(DISTINCT field)` helps to determine that. To demonstrate this, let's create a table "students" with the student's name, age, and major:
```sql
CREATE TABLE students(name text, age int, major string);
INSERT INTO students values(0,'John',21,'arts'),(0,'William',22,'business'),(0,'Richard',21,'cs'),(0,'Rebecca',22,'cs'),(0,'Monica',21,'arts');
```

so we have:

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

In the example, you can see that if we GROUP BY major and display both `COUNT(*)` and `COUNT(DISTINCT age)`, it becomes clear that there are two students who chose the major "cs" with two unique ages, but for the major "arts", there are also two students, yet only one unique age.

There can be at most one `COUNT(DISTINCT)` per query.

** By default, counts are approximate **

Actually, some of them are exact, while others are approximate. More on that below.

Manticore supports two algorithms for computing counts of distinct values. One is a legacy algorithm that uses a lot of memory and is usually slow. It collects `{group; value}` pairs, sorts them, and periodically discards duplicates. The benefit of this approach is that it guarantees exact counts within a plain table. You can enable it by setting the [distinct_precision_threshold](../Searching/Options.md#distinct_precision_threshold) option to `0`.

The other algorithm (enabled by default) loads counts into a hash table and returns its size. If the hash table becomes too large, its contents are moved into a `HyperLogLog`. This is where the counts become approximate since `HyperLogLog` is a probabilistic algorithm. The advantage is that the maximum memory usage per group is fixed and depends on the accuracy of the `HyperLogLog`. The overall memory usage also depends on the [max_matches](../Searching/Options.md#max_matches) setting, which reflects the number of groups.

The [distinct_precision_threshold](../Searching/Options.md#distinct_precision_threshold) option sets the threshold below which counts are guaranteed to be exact. The `HyperLogLog` accuracy setting and the threshold for the "hash table to HyperLogLog" conversion are derived from this setting. It's important to use this option with caution because doubling it will double the maximum memory required for count calculations. The maximum memory usage can be roughly estimated using this formula: `64 * max_matches * distinct_precision_threshold`. Note that this is the worst-case scenario, and in most cases, count calculations will use significantly less RAM.

**`COUNT(DISTINCT)` against a distributed table or a real-time table consisting of multiple disk chunks may return inaccurate results**, but the result should be accurate for a distributed table consisting of local plain or real-time tables with the same schema (identical set/order of fields, but may have different tokenization settings).

<!-- intro -->
##### Example:

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

Often, you want to better understand the contents of each group. You can use [GROUP N BY](../Searching/Grouping.md#Give-me-N-rows) for that, but it would return additional rows you might not want in the output. `GROUP_CONCAT()` enriches your grouping by concatenating values of a specific field in the group. Let's take the previous example and improve it by displaying all the ages in each group.

`GROUP_CONCAT(field)` returns the list as comma-separated values.

<!-- intro -->
##### Example:

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
Of course, you can also obtain the sum, average, minimum, and maximum values within a group.

<!-- intro -->
##### Example:

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
## Grouping accuracy

Grouping is done in fixed memory, which depends on the [max_matches](../Searching/Options.md#max_matches) setting. If `max_matches` allows for storage of all found groups, the results will be 100% accurate. However, if the value of `max_matches` is lower, the results will be less accurate.

When parallel processing is involved, it can become more complicated. When `pseudo_sharding` is enabled and/or when using an RT table with several disk chunks, each chunk or pseudo shard gets a result set that is no larger than `max_matches`. This can lead to inaccuracies in aggregates and group counts when the result sets from different threads are merged. To fix this, either a larger `max_matches` value or disabling parallel processing can be used.

Manticore will try to increase `max_matches` up to [max_matches_increase_threshold](../Searching/Options.md#max_matches_increase_threshold) if it detects that groupby may return inaccurate results. Detection is based on the number of unique values of the groupby attribute, which is retrieved from secondary indexes (if present).

To ensure accurate aggregates and/or group counts when using RT tables or `pseudo_sharding`, `accurate_aggregation` can be enabled. This will try to increase `max_matches` up to the threshold, and if the threshold is not high enough, Manticore will disable parallel processing for the query.

<!-- intro -->
##### Example:

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

