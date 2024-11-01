# 分组搜索结果

<!-- example general -->
分组搜索结果通常有助于获取每个组的匹配计数或其他聚合信息。例如，这对于创建显示每月匹配博客文章数量的图表，或者按网站对网页搜索结果进行分组、按作者对论坛帖子进行分组等非常有用。

Manticore 支持按单列、多列或计算表达式对搜索结果进行分组。结果可以：

- 在组内排序
- 返回每个组中超过一行的结果
- 对组进行过滤
- 对组进行排序
- 使用 [聚合函数](../Searching/Grouping.md#Aggregation-functions) 进行聚合

<!-- intro -->
通用语法：

<!-- request SQL -->
通用语法：

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
JSON 查询格式目前支持基本的分组功能，能够检索聚合值及其 `count(*)`。

```json
{
  "index": "<index_name>",
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

标准查询输出返回未分组的结果集，可以通过使用 `limit`（或 `size`）来隐藏它。要进行聚合，需要为组的结果集设置一个 `size`。

<!-- end -->

<!-- example group1 -->
### 仅分组
分组非常简单——只需在 `SELECT` 查询末尾添加 "GROUP BY smth"。该分组依据可以是：

- 表中的任何非全文字段：整数、浮点数、字符串、MVA（多值属性）
- 或者，如果在 `SELECT` 列表中使用了别名，也可以按此别名进行分组

您可以省略 `SELECT` 列表中的任何[聚合函数](../Searching/Grouping.md#Aggregation-functions)，查询仍然能够正常运行。

<!-- intro -->

##### 示例:

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

不过，在大多数情况下，您可能希望为每个组获取一些聚合数据，例如：

- `COUNT(*)` 用于获取每个组中的元素数量
- 或者 `AVG(field)` 用于计算组内该字段的平均值


<!-- intro -->
##### 示例:

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
     "index" : "films",
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
res =searchApi.search({"index":"films","limit":0,"aggs":{"release_year":{"terms":{"field":"release_year","size":100}}}})
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
<!-- request Javascript -->
``` javascript
res = await searchApi.search({"index":"films","limit":0,"aggs":{"release_year":{"terms":{"field":"release_year","size":100}}}});
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
##### 对组进行排序
默认情况下，组没有排序，接下来您通常希望根据某些内容对它们进行排序，比如按您分组的字段进行排序：

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

或者，您可以按聚合结果进行排序：

- 按 `count(*)` 排序，以首先显示包含最多元素的组
- 按 `avg(rental_rate)` 排序，以首先显示评分最高的电影。请注意，在示例中，这是通过别名完成的：`avg(rental_rate)` 首先在 `SELECT` 列表中映射为 `avg`，然后我们简单地执行 `ORDER BY avg`


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
##### 同时按多个字段分组
在某些情况下，您可能希望不仅按一个字段分组，还可以同时按多个字段分组，例如按电影的类别和年份：

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
    "index": "films",
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
##### 返回 N 行
有时查看每组中的多个元素而不仅仅是一个是很有用的。可以通过使用 `GROUP N BY` 来轻松实现。例如，在以下情况下，我们为每个年份返回两部电影，而不是仅通过 `GROUP BY release_year` 返回的一部。

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
##### 在组内排序
另一个关键的分析需求是对组内的元素进行排序。要实现此功能，可以使用 `WITHIN GROUP ORDER BY ... {ASC|DESC}` 子句。例如，我们可以获取每年评分最高的电影。需要注意的是，它与 `ORDER BY` 并行工作：

- `WITHIN GROUP ORDER BY` 对 **组内** 的结果进行排序
- 而 `GROUP BY` 则对 **组本身** 进行排序

这两者是完全独立运行的。


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
`HAVING expression` 是一个用于过滤组的有用子句。`WHERE` 在分组之前应用，而 `HAVING` 则用于处理分组后的结果。例如，我们可以保留那些电影年均租赁费率高于 3 的年份。结果只返回了四个年份：

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

请注意，`HAVING` 不会影响 [查询元信息中的 `total_found`](../Node_info_and_management/SHOW_META.md#SHOW-META)。

<!-- example group7 -->

##### GROUPBY()
`GROUPBY()` 是一个返回当前组键的函数，在许多情况下非常有用，尤其是在您[按 MVA（多值属性）分组](../Searching/Grouping.md#Grouping-by-MVA-(multi-value-attributes))或[按JSON 值分组](../Searching/Grouping.md#Grouping-by-a-JSON-node)时。

它也可以在 `HAVING` 中使用，例如，仅保留年份为 2000 和 2002 的组。

请注意，当您同时按多个字段分组时，不建议使用 `GROUPBY()`。尽管它仍然可以工作，但由于此时组键是字段值的复合体，可能不会按您预期的方式显示。

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
Manticore 支持按 [MVA](../Creating_a_table/Data_types.md#Multi-value-integer-(MVA)) 分组。为了演示其工作原理，我们可以创建一个包含 MVA 字段 "sizes" 的表 "shoes"，并向其中插入一些文档：
```sql
create table shoes(title text, sizes multi);
insert into shoes values(0,'nike',(40,41,42)),(0,'adidas',(41,43)),(0,'reebook',(42,43));
```
因此，我们有以下数据：
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
现在如果我们按 "sizes" 进行分组，它将处理所有的多值属性，并为每个值返回一个聚合结果，在这个例子中仅返回计数：

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
     "index" : "shoes",
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
res =searchApi.search({"index":"shoes","limit":0,"aggs":{"sizes":{"terms":{"field":"sizes","size":100}}}})
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
res = await searchApi.search({"index":"shoes","limit":0,"aggs":{"sizes":{"terms":{"field":"sizes","size":100}}}});
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
##### 按 JSON 节点分组
如果您有一个 [JSON](../Creating_a_table/Data_types.md#JSON) 类型的字段，您可以按其中的任何节点进行分组。为了演示这一点，我们可以创建一个表 "products"，并插入几个文档，每个文档在 "meta" JSON 字段中都有一个颜色信息：
```sql
create table products(title text, meta json);
insert into products values(0,'nike','{"color":"red"}'),(0,'adidas','{"color":"red"}'),(0,'puma','{"color":"green"}');
```
这会生成以下结果：
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
要按颜色对产品进行分组，只需使用 `GROUP BY meta.color`，并在 `SELECT` 列表中使用 `GROUPBY()` 显示相应的分组键：

<!-- intro -->
##### 示例：

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
     "index" : "products",
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
res =searchApi.search({"index":"products","limit":0,"aggs":{"color":{"terms":{"field":"meta.color","size":100}}}})
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
<!-- request Javascript -->
``` javascript
res = await searchApi.search({"index":"products","limit":0,"aggs":{"color":{"terms":{"field":"meta.color","size":100}}}});
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
除了 `COUNT(*)` 返回每个组中的元素数量之外，您还可以使用其他各种聚合函数：
<!-- example distinct -->

##### COUNT(DISTINCT field)
`COUNT(*)` 返回组中所有元素的数量，而 `COUNT(DISTINCT field)` 返回组中特定字段的唯一值数量，这可能与总数完全不同。例如，组中可能有 100 个元素，但某个字段的值相同。`COUNT(DISTINCT field)` 有助于确定这一点。为了演示这一点，我们创建一个包含学生姓名、年龄和专业的 "students" 表：

```sql
CREATE TABLE students(name text, age int, major string);
INSERT INTO students values(0,'John',21,'arts'),(0,'William',22,'business'),(0,'Richard',21,'cs'),(0,'Rebecca',22,'cs'),(0,'Monica',21,'arts');
```

我们有以下数据：

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

在这个例子中，您可以看到如果我们按 `major` 分组并显示 `COUNT(*)` 和 `COUNT(DISTINCT age)`，就可以发现专业为 "cs" 的两名学生有两个不同的年龄，而专业为 "arts" 的两名学生只有一个唯一的年龄。

每个查询最多只能有一个 `COUNT(DISTINCT)`。

**默认情况下，计数是近似的**

实际上，某些计数是精确的，而另一些是近似的。下面会详细说明。

Manticore 支持两种算法来计算唯一值的计数。一种是传统算法，使用大量内存，通常较慢。它收集 `{group; value}` 对，将其排序，并定期丢弃重复项。这种方法的好处是可以在普通表中保证精确计数。您可以通过将 [distinct_precision_threshold](../Searching/Options.md#distinct_precision_threshold) 选项设置为 `0` 来启用此算法。

另一种算法（默认启用）将计数加载到哈希表中并返回其大小。如果哈希表变得过大，其内容会转移到 `HyperLogLog` 中。此时，计数变为近似值，因为 `HyperLogLog` 是一种概率算法。它的优势在于每组的最大内存使用是固定的，并且取决于 `HyperLogLog` 的精度设置。

[distinct_precision_threshold](../Searching/Options.md#distinct_precision_threshold) 选项设置了确保计数精确的阈值。`HyperLogLog` 的精度设置以及从哈希表转换为 `HyperLogLog` 的阈值取决于此设置。谨慎使用此选项，因为将其值加倍将使计算计数所需的最大内存加倍。最大内存使用可以使用以下公式大致估算：`64 * max_matches * distinct_precision_threshold`。请注意，这只是最坏情况下的估算，在大多数情况下，计算计数会使用显著更少的内存。

在包含多个磁盘块的实时表或分布式表中，`COUNT(DISTINCT)` 的结果可能不准确，但对于由具有相同架构的本地普通表或实时表组成的分布式表，结果应是准确的。

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

通常，您可能希望更好地了解每个组的内容。可以使用 [GROUP N BY](../Searching/Grouping.md#Give-me-N-rows)，但这会返回您可能不希望在输出中显示的其他行。`GROUP_CONCAT()` 可以通过将特定字段的值连接在一起来丰富您的分组。例如，使用之前的例子，可以显示每个组中的所有年龄。

`GROUP_CONCAT(field)` 以逗号分隔的形式返回字段值列表。

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
当然，您还可以获得组内的总和、平均值、最小值和最大值。

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
## 分组精度

分组在固定内存中进行，内存的使用取决于 [max_matches](../Searching/Options.md#max_matches) 设置。如果 `max_matches` 允许存储所有找到的组，结果将是 100% 准确的。然而，如果 `max_matches` 的值较低，结果的准确性将降低。

当涉及并行处理时，情况可能会变得更加复杂。当启用了 `pseudo_sharding` 和/或使用包含多个磁盘块的实时表时，每个块或伪分片的结果集不会超过 `max_matches`。这可能会导致合并不同线程的结果集时聚合和组计数不准确。为了解决这个问题，可以使用更大的 `max_matches` 值，或者禁用并行处理。

如果 Manticore 检测到 groupby 可能返回不准确的结果，它会尝试将 `max_matches` 增加到 [max_matches_increase_threshold](../Searching/Options.md#max_matches_increase_threshold)。该检测基于从辅助索引中获取的 groupby 属性的唯一值数量（如果存在）。

要确保使用实时表或 `pseudo_sharding` 时的精确聚合和/或组计数，可以启用 `accurate_aggregation`。这将尝试将 `max_matches` 增加到阈值，如果阈值不够高，Manticore 将禁用该查询的并行处理。

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
