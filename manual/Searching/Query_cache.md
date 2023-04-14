# Query cache

Query cache stores compressed result sets in memory and reuses them for subsequent queries when possible. You can configure it using the following directives:

* [qcache_max_bytes](../Server_settings/Searchd.md#qcache_max_bytes), a limit on the RAM usage for cached query storage. Defaults to 16 MB. Setting `qcache_max_bytes` to 0 completely disables the query cache.
* [qcache_thresh_msec](../Server_settings/Searchd.md#qcache_thresh_msec), the minimum wall query time to cache. Queries that complete faster than this will *not* be cached. Defaults to 3000 msec, or 3 seconds.
* [qcache_ttl_sec](../Server_settings/Searchd.md#qcache_ttl_sec), cached entry TTL, or time to live. Queries will stay cached for this duration. Defaults to 60 seconds, or 1 minute.

These settings can be changed on the fly using the `SET GLOBAL` statement:

```sql
mysql> SET GLOBAL qcache_max_bytes=128000000;
```

These changes are applied immediately, and cached result sets that no longer satisfy the constraints are immediately discarded. When reducing the cache size on the fly, MRU (most recently used) result sets win.

Query cache operates as follows. When enabled, every full-text search result is completely stored in memory. This occurs after full-text matching, filtering, and ranking, so essentially we store `total_found` docid,weight} pairs. Compressed matches can consume anywhere from 2 bytes to 12 bytes per match on average, mostly depending on the deltas between subsequent docids. Once the query is complete, we check the wall time and size thresholds, and either save the compressed result set for reuse or discard it.

Note that the query cache's impact on RAM is not limited by`qcache_max_bytes`! If you run, for example, 10 concurrent queries, each matching up to 1M matches (after filters), then the peak temporary RAM usage will be in the range of 40 MB to 240 MB, even if the queries are fast enough and don't get cached.

Queries can use cache when the table, full-text query (i.e.,`MATCH()` contents), and ranker all match, and filters are compatible. This means:

*   The full-text part within `MATCH()` must be a bytewise match. Add a single extra space, and it's now a different query as far as the query cache is concerned.
*   The ranker (and its parameters, if any, for user-defined rankers) must be a bytewise match.
*   The filters must be a superset of the original filters. You can add extra filters and still hit the cache. (In this case, the extra filters will be applied to the cached result.) But if you remove one, that will be a new query again.

Cache entries expire with TTL and also get invalidated on table rotation, or on  `TRUNCATE`, or on `ATTACH`.  Note that currently, entries are not invalidated on arbitrary RT table writes! So a cached query might return older results for the duration of its TTL.

You can inspect the current cache status with [SHOW STATUS](../Node_info_and_management/Node_status.md#SHOW-STATUS) through the `qcache_XXX` variables:

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
<!-- proofread -->