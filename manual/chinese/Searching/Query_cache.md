# 查询缓存

查询缓存将压缩的结果集存储在内存中，并在可能时对后续查询重用它们。您可以使用以下指令进行配置：

* [qcache_max_bytes](../Server_settings/Searchd.md#qcache_max_bytes)，缓存查询存储的内存使用限制。默认值为16 MB。将`qcache_max_bytes`设置为0会完全禁用查询缓存。
* [qcache_thresh_msec](../Server_settings/Searchd.md#qcache_thresh_msec)，缓存的最小查询耗时阈值。查询完成时间快于该值的将*不*被缓存。默认值为3000毫秒，即3秒。
* [qcache_ttl_sec](../Server_settings/Searchd.md#qcache_ttl_sec)，缓存条目的TTL，即存活时间。查询将在此时间内保持缓存。默认值为60秒，即1分钟。

这些设置可以使用`SET GLOBAL`语句动态更改：

```sql
mysql> SET GLOBAL qcache_max_bytes=128000000;
```

这些更改会立即生效，不再满足约束的缓存结果集会被立即丢弃。当动态缩减缓存大小时，最近最常用（MRU）的结果集优先保留。

查询缓存的工作方式如下。启用时，每个全文搜索结果将完整存储于内存中。这发生在全文匹配、过滤和排名之后，因此本质上我们存储的是`total_found`个{docid,weight}对。压缩匹配平均每个匹配耗用2到12字节不等，主要取决于连续docid之间的差异。查询完成后，我们检查耗时和大小阈值，要么保存压缩后的结果集以供重用，要么丢弃它。

请注意，查询缓存对内存的影响不限于`qcache_max_bytes`！例如，如果您运行10个并发查询，每个查询匹配最多100万条结果（经过过滤后），那么峰值临时内存使用可能在40 MB到240 MB之间，即使查询够快且未被缓存。

当表、全文查询（即`MATCH()`内容）和排序器都匹配，且过滤条件兼容时，查询可以使用缓存。具体而言：

*   `MATCH()`中的全文部分必须逐字节匹配。添加一个额外的空格，在查询缓存看来就是一个不同的查询。
*   排序器（及其参数，如自定义排序器的参数）必须逐字节匹配。
*   过滤条件必须是原过滤条件的超集。您可以添加额外过滤条件仍然命中缓存。（在这种情况下，额外过滤条件将在缓存结果上应用。）但如果移除一个过滤条件，则视为新的查询。

缓存条目会按TTL过期，并在表轮换、`TRUNCATE`或`ATTACH`时失效。请注意，目前条目不会因任意RT表写入而失效！因此缓存的查询可能在TTL期间返回较旧的结果。

您可以通过[SHOW STATUS](../Node_info_and_management/Node_status.md#SHOW-STATUS) 使用`qcache_XXX`变量查看当前缓存状态：

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

