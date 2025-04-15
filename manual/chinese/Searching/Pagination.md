# 搜索结果的分页

<!-- example general -->

Manticore Search 默认返回结果集中的前 20 个匹配文档。

#### SQL
在 SQL 中，您可以使用 `LIMIT` 子句在结果集中导航。

`LIMIT` 可以接受一个数字作为返回集的大小，前提是偏移量为零，或者一对偏移量和大小值。

#### HTTP JSON
使用 HTTP JSON 时，节点 `offset` 和 `limit` 控制结果集的偏移量和返回集的大小。或者，您可以改用 `size` 和 `from` 一对。

<!-- intro -->

<!-- request SQL -->

```sql
SELECT  ... FROM ...  [LIMIT [offset,] row_count]
SELECT  ... FROM ...  [LIMIT row_count][ OFFSET offset]
```


<!-- request JSON -->

```json
{
  "table": "<table_name>",
  "query": ...
  ...  
  "limit": 20,
  "offset": 0
}
{
  "table": "<table_name>",
  "query": ...
  ...  
  "size": 20,
  "from": 0
}
```

<!-- end -->

<!-- example maxMatches -->
### 结果集窗口

默认情况下，Manticore Search 使用 1000 个最佳排名文档的结果集窗口，可以在结果集中返回。如果结果集超过此值进行分页，查询将以错误结束。

该限制可以通过查询选项 [max_matches](../Searching/Options.md#max_matches) 进行调整。

将 `max_matches` 增加到非常高的值只应在为了达到这样的点的导航必要时进行。高的 `max_matches` 值需要更多的内存，并且可能会增加查询响应时间。处理深结果集的一种方法是将 `max_matches` 设置为偏移量和限制的总和。

将 `max_matches` 降低到 1000 以下的好处是减少查询使用的内存。它也可以减少查询时间，但在大多数情况下，这可能不会有明显的收益。

<!-- intro -->


<!-- request SQL -->

```sql
SELECT  ... FROM ...   OPTION max_matches=<value>
```


<!-- request JSON -->


```json
{
  "table": "<table_name>",
  "query": ...
  ...
  "max_matches":<value>
  }
}

```

<!-- end -->

# 滚动搜索选项

滚动搜索选项提供了一种高效和可靠的方式来在大型结果集中进行分页。与传统的基于偏移量的分页不同，滚动搜索提供了更好的深度分页性能，并提供了更简单的实现分页的方法。

#### 通过 SQL 滚动

<!-- example scroll_sql_init -->
**带排序标准的初始查询**

首先执行带有所需排序标准的初始查询。唯一的要求是 `id` 必须包含在 ORDER BY 子句中，以确保一致的分页。查询将返回您的结果和用于后续页面的滚动令牌。

```sql
SELECT ... ORDER BY [... ,] id {ASC|DESC};
```

<!-- intro -->
示例：
<!-- request Initial Query Example -->
```sql
SELECT weight(), id FROM test WHERE match('hello') ORDER BY weight() desc, id asc limit 2;
```

<!-- response Initial Query Example -->
```sql
+----------+------+
| weight() | id   |
+----------+------+
|     1281 |    1 |
|     1281 |    2 |
+----------+------+
2 rows in set (0.00 sec)
```

<!-- end -->

<!-- example scroll_sql_show -->
**检索滚动令牌**

执行初始查询后，通过执行 `SHOW SCROLL` 命令来检索滚动令牌。

```sql
SHOW SCROLL;
```

响应：
```sql
| scroll_token                       |
|------------------------------------|
| <base64 编码的滚动令牌>      |
```

<!-- intro -->
示例：

<!-- request Scroll Token Example -->
```sql
SHOW SCROLL;
```

<!-- response Scroll Token Example -->
```sql
+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| scroll_token                                                                                                                                                                                                             |
+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| eyJvcmRlcl9ieV9zdHIiOiJ3ZWlnaHQoKSBkZXNjLCBpZCBhc2MiLCJvcmRlcl9ieSI6W3siYXR0ciI6IndlaWdodCgpIiwiZGVzYyI6dHJ1ZSwidmFsdWUiOjEyODEsInR5cGUiOiJpbnQifSx7ImF0dHIiOiJpZCIsImRlc2MiOmZhbHNlLCJ2YWx1ZSI6MiwidHlwZSI6ImludCJ9XX0= |
+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
1 row in set (0.00 sec)
```

<!-- end -->

<!-- example scroll_sql_paginated -->
**使用滚动的分页查询**

 为了检索下一页的结果，在后续查询中包含滚动令牌作为选项。当提供 `scroll` 选项时，指定排序标准是可选的。

```sql
SELECT ... [ORDER BY [... ,] id {ASC|DESC}] OPTION scroll='<base64 编码的滚动令牌>'[, ...];
```

这确保了分页无缝继续，保持在初始查询中建立的排序上下文。

<!-- intro -->
示例：

<!-- request Paginated Query Example -->
```sql
SELECT weight(), id FROM test WHERE match('hello') limit 2
OPTION scroll='eyJvcmRlcl9ieV9zdHIiOiJ3ZWlnaHQoKSBkZXNjLCBpZCBhc2MiLCJvcmRlcl9ieSI6W3siYXR0ciI6IndlaWdodCgpIiwiZGVzYyI6dHJ1ZSwidmFsdWUiOjEyODEsInR5cGUiOiJpbnQifSx7ImF0dHIiOiJpZCIsImRlc2MiOmZhbHNlLCJ2YWx1ZSI6MiwidHlwZSI6ImludCJ9XX0=';
```

<!-- response Paginated Query Example -->
```sql
+----------+------+
| weight() | id   |
+----------+------+
|     1281 |    3 |
|     1281 |    4 |
+----------+------+
2 rows in set (0.00 sec)
```

<!-- end -->

#### 通过 JSON 滚动

<!-- example scroll_json_init -->
**初始请求**

在初始请求中，在选项中指定 `"scroll": true` 和所需的排序标准。请注意，`id` 必须包含在 `sort` 数组中。响应将包括一个滚动令牌，可用于后续请求中的分页。

```json
POST /search
{ 
  "table": "<table_names>",
  "options": {
	  "scroll": true
  }, 
  
  ...
  
  "sort": [
    ...
    { "id":{ "order":"{asc|desc}"} }
  ]
}
```

示例输出：

```json
{
    "timed_out": false,
    "hits": {

		...
    
    },
    "scroll": "<base64 编码的滚动令牌>"
}
```

<!-- request Initial Request Example -->
```json
POST /search
{ 
  "table": "test",
  "options":
  {
	"scroll": true
  },
  "query":
  {  
	"query_string":"hello"
  },
  "sort":
  [
    { "_score":{ "order":"desc"} },
	{ "id":{ "order":"asc"} }
  ],
  "track_scores": true,
  "limit":2
}
```

<!-- response Initial Request Example -->
```json
{
  "took": 0,
  "timed_out": false,
  "hits":
  {
    "total": 10,
    "total_relation": "eq",
    "hits":
	[
      {
        "_id": 1,
        "_score": 1281,
        "_source":
		{
          "title": "hello world1"
        }
      },
      {
        "_id": 2,
        "_score": 1281,
        "_source":
		{
          "title": "hello world2"
        }
      }
    ]
  },
  "scroll": "eyJvcmRlcl9ieV9zdHIiOiJAd2VpZ2h0IGRlc2MsIGlkIGFzYyIsIm9yZGVyX2J5IjpbeyJhdHRyIjoid2VpZ2h0KCkiLCJkZXNjIjp0cnVlLCJ2YWx1ZSI6MTI4MSwidHlwZSI6ImludCJ9LHsiYXR0ciI6ImlkIiwiZGVzYyI6ZmFsc2UsInZhbHVlIjoyLCJ0eXBlIjoiaW50In1dfQ=="
}
```
<!-- end -->

<!-- example scroll_json_paginated -->
**使用滚动的分页请求**

要继续分页，在下一个请求的选项对象中包含从先前响应中获得的滚动令牌。指定排序标准是可选的。

```json
POST /search
{ 
  "table": "<table_names>",
  "options": {
    "scroll": "<base64 编码的滚动令牌>"
  },

  ...

}
```

<!-- request Paginated Request Example -->

```json
POST /search
{ 
  "table": "test",
  "options":
  {
	"scroll": "eyJvcmRlcl9ieV9zdHIiOiJAd2VpZ2h0IGRlc2MsIGlkIGFzYyIsIm9yZGVyX2J5IjpbeyJhdHRyIjoid2VpZ2h0KCkiLCJkZXNjIjp0cnVlLCJ2YWx1ZSI6MTI4MSwidHlwZSI6ImludCJ9LHsiYXR0ciI6ImlkIiwiZGVzYyI6ZmFsc2UsInZhbHVlIjoyLCJ0eXBlIjoiaW50In1dfQ=="
  },
  "query":
  {  
	"query_string":"hello"
  },
  "track_scores": true,
  "limit":2
}
```

<!-- response Paginated Request Example -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits":
  {
    "total": 8,
    "total_relation": "eq",
    "hits":
   [
      {
        "_id": 3,
        "_score": 1281,
        "_source":
        {
          "title": "hello world3"
        }
      },
      {
        "_id": 4,
        "_score": 1281,
        "_source":
        {
          "title": "hello world4"
        }
      }
    ]
  },
  "scroll": "eyJvcmRlcl9ieV9zdHIiOiJAd2VpZ2h0IGRlc2MsIGlkIGFzYyIsIm9yZGVyX2J5IjpbeyJhdHRyIjoid2VpZ2h0KCkiLCJkZXNjIjp0cnVlLCJ2YWx1ZSI6MTI4MSwidHlwZSI6ImludCJ9LHsiYXR0ciI6ImlkIiwiZGVzYyI6ZmFsc2UsInZhbHVlIjo0LCJ0eXBlIjoiaW50In1dfQ=="
}
```
<!-- end -->

<!-- proofread -->
