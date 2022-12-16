# Query cache

Query cache stores a compressed result set in memory, and then reuses it for subsequent queries where possible. You can configure it using the following directives:

*   [qcache_max_bytes](../Server_settings/Searchd.md#qcache_max_bytes), a limit on the RAM use for cached queries storage. Defaults to 16 MB. Setting `qcache_max_bytes` to 0 completely disables the query  cache.
*   [qcache_thresh_msec](../Server_settings/Searchd.md#qcache_thresh_msec), the minimum wall query time to cache. Queries that completed faster than this will *not* be cached. Defaults to 3000 msec, or 3 seconds.
*   [qcache_ttl_sec](../Server_settings/Searchd.md#qcache_ttl_sec), cached entry TTL, or time to live. Queries will stay cached for this much. Defaults to 60 seconds, or 1 minute.

These settings can be changed on the fly using the `SET GLOBAL` statement:

```sql
mysql> SET GLOBAL qcache_max_bytes=128000000;
```

These changes are applied immediately, and the cached result sets that no longer satisfy the constraints are immediately discarded. When reducing the cache size on the fly, MRU (most recently used) result sets win.

Query cache works as follows. When it's enabled, *every* full-text search result gets *completely* stored in memory. That happens after full-text matching, filtering, and ranking, so basically we store `total_found` {docid,weight} pairs. Compressed matches can consume anywhere from 2 bytes to 12 bytes per match on average, mostly depending on the deltas between the subsequent docids. Once the query completes, we check the wall time and size thresholds, and either save that compressed result set for reuse, or discard it.

Note how the query cache impact on RAM is thus *not* limited by `qcache_max_bytes`! If you run, say, 10 concurrent queries, each of them matching upto 1M matches (after filters), then the peak temporary RAM use will be in the 40 MB to 240 MB range, even if in the end the queries are quick enough and do not get cached.

Queries can then use cache when the table, the full-text query (ie.`MATCH()` contents), and the ranker are all a match, and filters are compatible. Meaning:

*   The full-text part within `MATCH()` must be a bytewise match. Add a single extra space, and that is now a different query where the query cache is concerned.
*   The ranker (and its parameters if any, for user-defined rankers) must be a bytewise match.
*   The filters must be a superset of the original filters. That is, you can add extra filters and still hit the cache. (In this case, the extra filters will be applied to the cached result.) But if you remove one, that will be a new query again.

Cache entries expire with TTL, and also get invalidated on table rotation, or on `TRUNCATE`, or on `ATTACH`. Note that at the moment entries are **not** invalidated on arbitrary RT table writes! So a cached query might be returning older results for the duration of its TTL.

Current cache status can be inspected with in [SHOW STATUS](../Node_info_and_management/Node_status.md#SHOW-STATUS) through the `qcache_XXX` variables:

```sql
mysql> SHOW STATUS LIKE 'qcache%';
+-----------------------+----------+
| Counter               | Value    |
+-----------------------+----------+
| qcache_max_bytes      | 16777216 |
| qcache_thresh_msec    | 3000     |
| qcache_ttl_sec        | 60       |
| qcache_cached_queries | 0        |
| qcache_used_bytes     | 0        |
| qcache_hits           | 0        |
+-----------------------+----------+
6 rows in set (0.00 sec)
```
