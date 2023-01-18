# Pagination of search results

<!-- example general -->

Manticore Search returns by default the top 20 matched documents in the result set.

#### SQL
For SQL the navigation over the result set can be done with `LIMIT` clause.

`LIMIT` can accept one number as the size of the returned set using a zero offset or a pair of offset and size.

#### HTTP JSON
If HTTP JSON is used, the nodes `offset` and `limit` can control the offset of the result set and the size of the returned set. Alternatively the pair `size` and `from` can be used instead.

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

By default, Manticore Search uses a result set window of 1000 best ranked documents that can be returned back in the result set. If the result set is paginated beyond this value, the query will end in error.

This limitation can be adjusted with the query option [max_matches](../Searching/Options.md#max_matches).

Increasing the `max_matches` to very high values should be made only if it's required for the navigation to reach such points. High `max_matches` value requires more memory used and can increase the query response time. One way to work with deep result sets is to set `max_matches` as a sum of offset and limit.

Lowering `max_matches` under 1000 has a benefit in reducing the memory used by the query. It can also reduce the query time, but in most cases it might not be a noticeable gain.

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
