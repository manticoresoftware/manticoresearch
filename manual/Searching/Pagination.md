# Pagination of search results

<!-- example general -->

Manticore Search returns the top 20 matched documents in the result set by default.

#### SQL
In SQL, you can navigate through the result set using the `LIMIT` clause.

`LIMIT` can accept either one number as the size of the returned set with a zero offset, or a pair of offset and size values.

#### HTTP JSON
When using HTTP JSON, the nodes `offset` and `limit` control the offset of the result set and the size of the returned set. Alternatively, you can use the pair `size` and `from` instead.

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
### Result set window

By default, Manticore Search uses a result set window of 1000 best-ranked documents that can be returned in the result set. If the result set is paginated beyond this value, the query will end in error.

This limitation can be adjusted with the query option [max_matches](../Searching/Options.md#max_matches).

Increasing the `max_matches` to very high values should only be done if it's necessary for the navigation to reach such points. A high `max_matches` value requires more memory and can increase the query response time. One way to work with deep result sets is to set `max_matches` as the sum of the offset and limit.

Lowering `max_matches` below 1000 has the benefit of reducing the memory used by the query. It can also reduce the query time, but in most cases, it might not be a noticeable gain.

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

# Scroll Search Option

The scroll search option provides an efficient and reliable way to paginate through large result sets. Unlike traditional offset-based pagination, scroll search offers better performance for deep pagination and provides an easier way to implement pagination.

#### Scrolling via SQL

<!-- example scroll_sql_init -->
**Initial Query with Sorting Criteria**

Start by executing an initial query with your desired sorting criteria. The only requirement is that `id` must be included in the ORDER BY clause to ensure consistent pagination. The query will return both your results and a scroll token for subsequent pages.

```sql
SELECT ... ORDER BY [... ,] id {ASC|DESC};
```

<!-- intro -->
Example:
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
**Retrieving the Scroll Token**

After executing the initial query, retrieve the scroll token by executing the `SHOW SCROLL` command.

```sql
SHOW SCROLL;
```

Response:
```sql
| scroll_token                       |
|------------------------------------|
| <base64 encoded scroll token>      |
```

<!-- intro -->
Example:

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
**Paginated Query Using scroll**

 To retrieve the next page of results, include the scroll token in the subsequent query as an option. When the `scroll` option is provided, specifying the sort criteria is optional.

```sql
SELECT ... [ORDER BY [... ,] id {ASC|DESC}] OPTION scroll='<base64 encoded scroll token>'[, ...];
```

This ensures that pagination continues seamlessly, maintaining the sorting context established in the initial query.

<!-- intro -->
Example:

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

#### Scrolling via JSON

<!-- example scroll_json_init -->
**Initial Request**

In the initial request, specify `"scroll": true` in the options and the desired sorting criteria. Note that `id` must be present in the `sort` array. The response will include a scroll token, which can be used for pagination in subsequent requests.

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

Example output:

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
**Paginated Request Using scroll**

To continue pagination, include the scroll token obtained from the previous response within the options object of the next request. Specifying the sort criteria is optional.

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