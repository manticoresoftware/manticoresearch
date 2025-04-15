# 查询缓存

查询缓存将压缩的结果集存储在内存中，并在可能的情况下重复使用它们以应对后续查询。您可以使用以下指令进行配置：

* [qcache_max_bytes](../Server_settings/Searchd.md#qcache_max_bytes)，缓存查询存储的内存使用限制。默认值为16 MB。将`qcache_max_bytes`设置为0将完全禁用查询缓存。
* [qcache_thresh_msec](../Server_settings/Searchd.md#qcache_thresh_msec)，缓存的最小墙查询时间。完成速度快于此时间的查询将*不*被缓存。默认值为3000毫秒，或3秒。
* [qcache_ttl_sec](../Server_settings/Searchd.md#qcache_ttl_sec)，缓存条目TTL或生存时间。查询将在此持续时间内保持缓存。默认值为60秒，或1分钟。

这些设置可以使用`SET GLOBAL`语句动态更改：

```sql
mysql> SET GLOBAL qcache_max_bytes=128000000;
```

这些更改会立即生效，且不再满足约束的缓存结果集将立即被丢弃。在动态减少缓存大小时，MRU（最近最常使用）结果集优先。

查询缓存的工作方式如下。当启用时，每个全文搜索结果会完全存储在内存中。这发生在全文匹配、过滤和排名之后，因此实际上我们存储的是`total_found` {docid,weight}对。压缩的匹配项平均每个匹配项可能消耗2字节到12字节，主要取决于后续docid之间的增量。一旦查询完成，我们检查时间和大小的阈值，并保存压缩的结果集以便重用或丢弃。

请注意，查询缓存对RAM的影响并不受`qcache_max_bytes`的限制！例如，如果您并行运行10个查询，每个查询最多匹配1M个匹配项（过滤后），那么峰值临时RAM使用量将在40 MB到240 MB之间，即使查询的速度足够快并且没有被缓存。

查询可以使用缓存，当表、全文查询（即`MATCH()`内容）和排名器完全匹配且过滤器兼容时。这意味着：

*  `MATCH()`中的全文部分必须是字节级匹配。添加一个额外的空格，就会就查询缓存而言这是一个不同的查询。
* 排名器（及其参数，如果有的话，对于用户定义的排名器）必须是字节级匹配。
* 过滤器必须是原始过滤器的超集。您可以添加额外的过滤器，仍然命中缓存。（在这种情况下，额外的过滤器将应用于缓存结果。）但如果您删除一个，这将再次成为一个新查询。

缓存条目随TTL过期，并且在表轮换、`TRUNCATE`或`ATTACH`时被失效。请注意，目前，条目不会因任意RT表写入而失效！因此，缓存查询在其TTL持续期间可能返回较旧的结果。

您可以通过`qcache_XXX`变量使用[SHOW STATUS](../Node_info_and_management/Node_status.md#SHOW-STATUS)检查当前缓存状态：

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
