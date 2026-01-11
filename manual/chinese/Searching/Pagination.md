# 搜索结果的分页

<!-- example general -->

Manticore Search 默认返回结果集中匹配度最高的前20个文档。

#### SQL
在 SQL 中，可以使用 `LIMIT` 子句来遍历结果集。

`LIMIT` 可以接受一个数字作为返回集的大小，或者一个偏移量和大小的配对值。

#### HTTP JSON
使用 HTTP JSON 时，节点 `offset` 和 `limit` 控制结果集的偏移量和返回集的大小。或者，可以使用配对 `size` 和 `from`。

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

Manticore Search 默认使用一个结果集窗口，最多返回1000个最佳排名的文档。如果结果集分页超过这个值，查询将出错。

这个限制可以通过查询选项 [max_matches](../Searching/Options.md#max_matches) 进行调整。

将 `max_matches` 增加到非常高值，只有在导航到这些点时才需要。高 `max_matches` 值需要更多的内存，并且会增加查询响应时间。处理深层结果集的一种方法是将 `max_matches` 设置为偏移量和限制之和。

将 `max_matches` 降低到1000以下的好处是可以减少查询使用的内存。这也可以减少查询时间，但在大多数情况下，这可能不会带来明显的改进。

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

## 滚动搜索选项

滚动搜索选项提供了一种高效且可靠的方式来分页遍历大量结果集。与传统的基于偏移量的分页不同，滚动搜索为深度分页提供了更好的性能，并提供了一种更简单的分页实现方式。
虽然它使用与基于偏移量的分页相同的 `max_matches` 窗口，但滚动搜索可以通过使用滚动令牌在多次请求中检索结果，**可以返回比 `max_matches` 值更多的文档**。
使用滚动分页时，不需要一起使用 `offset` 和 `limit` — 这是多余的，通常被认为是过度工程。相反，只需指定 `limit` 以及 `scroll` 令牌来获取每一页。

#### 通过 SQL 滚动

<!-- example scroll_sql_init -->
**初始查询带有排序条件**

首先执行一个带有您所需排序条件的初始查询。唯一的要求是 `id` 必须包含在 ORDER BY 子句中，以确保分页的一致性。查询将返回您的结果和用于后续页面的滚动令牌。

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
**获取滚动令牌**

执行初始查询后，通过执行 `SHOW SCROLL` 命令获取滚动令牌。
在滚动序列中的每次查询后都必须调用 `SHOW SCROLL` 以获取用于下一页的更新滚动令牌。
每次查询都会生成一个新的令牌，反映最新的滚动位置。

```sql
SHOW SCROLL;
```

响应：
```sql
| scroll_token                       |
|------------------------------------|
| <base64 encoded scroll token>      |
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
**使用 scroll 进行分页查询**

要检索下一页的结果，请在后续查询中包含滚动令牌作为选项。当提供 `scroll` 选项时，指定排序条件是可选的。
记住，在此查询后再次调用 `SHOW SCROLL` 以获取用于下一页的新令牌。

```sql
SELECT ... [ORDER BY [... ,] id {ASC|DESC}] OPTION scroll='<base64 encoded scroll token>'[, ...];
```

这确保了分页可以无缝继续，保持初始查询建立的排序上下文。

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

在初始请求中，在选项中指定 `"scroll": true` 并包含所需的排序条件。注意 `id` 必须在 `sort` 数组中。响应将包括一个滚动令牌，该令牌可以在后续请求中用于分页。

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
    "scroll": "<base64 encoded scroll token>"
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
**使用 scroll 进行分页请求**

要继续分页，请在下一个请求的选项对象中包含从上一个响应中获得的滚动令牌。指定排序条件是可选的。

```json
POST /search
{
  "table": "<table_names>",
  "options": {
    "scroll": "<base64 encoded scroll token>"
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

