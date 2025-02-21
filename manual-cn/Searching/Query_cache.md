# 查询缓存

查询缓存将压缩的结果集存储在内存中，并在可能的情况下为后续查询重复使用。你可以使用以下指令来配置它：

- [qcache_max_bytes](../Server_settings/Searchd.md#qcache_max_bytes)：限制用于缓存查询存储的内存大小，默认为16MB。将 `qcache_max_bytes` 设置为0会完全禁用查询缓存。
- [qcache_thresh_msec](../Server_settings/Searchd.md#qcache_thresh_msec)：缓存的最低查询执行时间，低于该时间的查询*不会*被缓存。默认值为3000毫秒，即3秒。
- [qcache_ttl_sec](../Server_settings/Searchd.md#qcache_ttl_sec)：缓存条目的TTL（存活时间）。查询结果将在这个时间内保持缓存，默认为60秒，即1分钟。

这些设置可以通过 `SET GLOBAL` 语句动态更改：

```sql
mysql> SET GLOBAL qcache_max_bytes=128000000;
```

这些更改会立即生效，且不再满足限制条件的缓存结果集会立即被丢弃。当动态减少缓存大小时，最近最常使用（MRU）的结果集会优先保留。

查询缓存的工作原理如下：启用后，每个全文搜索结果会完全存储在内存中。这是在全文匹配、过滤和排名之后进行的，因此我们基本上存储的是 `total_found` {docid, weight} 对。压缩后的匹配数据平均每个匹配会占用2到12个字节，具体取决于后续文档id之间的差异。当查询完成后，我们会检查时间和大小阈值，并决定是保存压缩结果集以供重用还是丢弃它。

请注意，查询缓存对内存的影响不只是由 `qcache_max_bytes` 限制的！例如，如果你同时运行10个查询，每个查询匹配多达1百万个匹配项（经过过滤），那么即使这些查询足够快且不会被缓存，其内存使用峰值范围也将在40MB到240MB之间。

查询可以在表、全文查询（即 `MATCH()` 的内容）和排名器都匹配且过滤条件兼容时使用缓存。这意味着：

- `MATCH()` 中的全文部分必须是逐字节匹配的。即使多加一个空格，也会被视为不同的查询。
- 排名器（以及任何用户定义的排名器参数）必须逐字节匹配。
- 过滤器必须是原始过滤器的超集。你可以添加额外的过滤器并仍然命中缓存（在这种情况下，额外的过滤器会应用于缓存的结果）。但如果你删除一个过滤器，这将被视为新的查询。

缓存条目会因TTL过期、表轮换、`TRUNCATE` 或 `ATTACH` 操作失效。注意，目前缓存条目不会因任意RT表写操作失效！因此，缓存的查询在其TTL持续期间可能返回旧的结果。

你可以通过 [SHOW STATUS](../Node_info_and_management/Node_status.md#SHOW-STATUS) 命令查看当前的缓存状态，通过 `qcache_XXX` 变量：

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
