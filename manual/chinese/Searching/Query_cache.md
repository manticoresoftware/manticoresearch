# 查询缓存

查询缓存将压缩的结果集存储在内存中，并在可能的情况下重用它们以响应后续查询。您可以使用以下指令进行配置：

* [qcache_max_bytes](../Server_settings/Searchd.md#qcache_max_bytes)，缓存查询存储的RAM使用限制。默认值为16 MB。将 `qcache_max_bytes` 设置为0会完全禁用查询缓存。
* [qcache_thresh_msec](../Server_settings/Searchd.md#qcache_thresh_msec)，缓存的最小查询实际执行时间。执行时间短于此值的查询不会被缓存。默认值为3000毫秒，即3秒。
* [qcache_ttl_sec](../Server_settings/Searchd.md#qcache_ttl_sec)，缓存条目的TTL（存活时间）。查询结果将缓存此时长。默认值为60秒，即1分钟。

这些设置可以通过 `SET GLOBAL` 语句动态更改：

```sql
mysql> SET GLOBAL qcache_max_bytes=128000000;
```

这些更改会立即生效，不再满足约束条件的缓存结果集会被立即丢弃。当动态缩小缓存大小时，最常用（MRU）的结果集将被保留。

查询缓存的工作原理如下。启用时，每个全文搜索结果都会完整存储在内存中。这发生在全文匹配、过滤和排序之后，因此本质上我们存储的是 `total_found` 个 {docid,weight} 对。压缩后的匹配结果平均每个匹配消耗2到12字节，主要取决于后续docid之间的差值。查询完成后，我们会检查实际执行时间和大小阈值，然后保存压缩结果集以供重用，或丢弃它。

注意，查询缓存对RAM的影响不限于 `qcache_max_bytes`！例如，如果您运行10个并发查询，每个查询匹配最多100万条记录（过滤后），那么峰值临时RAM使用量将在40 MB到240 MB之间，即使查询足够快未被缓存。

查询可以使用缓存的条件是表、全文查询（即 `MATCH()` 内容）和排名算法均一致，且过滤条件兼容。这意味着：

*   `MATCH()` 内的全文部分必须字节完全匹配。多一个空格，查询缓存就会认为这是不同的查询。
*   评分器（包括用户定义评分器的参数）必须字节完全匹配。
*   过滤条件必须是原过滤条件的超集。您可以添加额外过滤器，依然命中缓存。（在这种情况下，会对缓存结果应用额外的过滤器。）但如果移除过滤器，则会被视为新查询。

缓存条目在TTL过期后失效，并在表轮换、执行 `TRUNCATE` 或 `ATTACH` 时失效。注意，目前条目不会因任意RT表写操作而失效！因此缓存查询在其TTL期间可能返回较旧的结果。

您可以通过 [SHOW STATUS](../Node_info_and_management/Node_status.md#SHOW-STATUS) 以及 `qcache_XXX` 变量查看当前缓存状态：

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

