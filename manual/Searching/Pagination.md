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
  "index": "<index_name>",
  "query": ...
  ...  
  "limit": 20,
  "offset": 0
}
{
  "index": "<index_name>",
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
  "index": "<index_name>",
  "query": ...
  ...
  "max_matches":<value>
  }
}

```

<!-- end -->
<!-- proofread -->