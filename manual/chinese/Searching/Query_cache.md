# 查询缓存

查询缓存将压缩的结果集存储在内存中，并在可能的情况下重用它们以响应后续查询。您可以使用以下指令进行配置：

* [qcache_max_bytes](../Server_settings/Searchd.md#qcache_max_bytes)，缓存查询存储的 RAM 使用限制。默认值为 16 MB。将 `qcache_max_bytes` 设置为 0 会完全禁用查询缓存。
* [qcache_thresh_msec](../Server_settings/Searchd.md#qcache_thresh_msec)，缓存的最小查询耗时。完成时间快于此值的查询将*不会*被缓存。默认值为 3000 毫秒，即 3 秒。
* [qcache_ttl_sec](../Server_settings/Searchd.md#qcache_ttl_sec)，缓存条目的 TTL（存活时间）。查询结果将缓存此时长。默认值为 60 秒，即 1 分钟。

这些设置可以通过 `SET GLOBAL` 语句动态更改：

```sql
mysql> SET GLOBAL qcache_max_bytes=128000000;
```

这些更改会立即生效，不再满足约束条件的缓存结果集会立即被丢弃。在动态减少缓存大小时，最近最常使用（MRU）的结果集优先保留。

查询缓存的工作原理如下。启用时，每个全文搜索结果都会完整存储在内存中。这发生在全文匹配、过滤和排序之后，基本上我们存储了 `total_found` 个 {docid,weight} 对。压缩后的匹配项平均每个占用 2 到 12 字节，主要取决于相邻 docid 之间的差值。查询完成后，我们检查耗时和大小阈值，决定是保存压缩结果集以供重用，还是丢弃它。

请注意，查询缓存对 RAM 的影响不限于 `qcache_max_bytes`！例如，如果您同时运行 10 个并发查询，每个查询匹配最多 100 万条结果（过滤后），即使查询足够快且未被缓存，峰值临时 RAM 使用量也会在 40 MB 到 240 MB 之间。

当表、全文查询（即 `MATCH()` 内容）和排序器完全匹配且过滤条件兼容时，查询可以使用缓存。这意味着：

*   `MATCH()` 中的全文部分必须逐字节匹配。添加一个额外空格，查询缓存就会认为这是一个不同的查询。
*   排序器（及其参数，如果是用户定义的排序器）必须逐字节匹配。
*   过滤条件必须是原始过滤条件的超集。您可以添加额外的过滤条件仍然命中缓存。（在这种情况下，额外的过滤条件会应用于缓存结果。）但如果移除其中一个过滤条件，则会被视为新查询。

缓存条目会根据 TTL 过期，并且在表轮换、`TRUNCATE` 或 `ATTACH` 时失效。请注意，目前条目不会因任意 RT 表写入而失效！因此，缓存查询可能在其 TTL 期间返回较旧的结果。

您可以通过 [SHOW STATUS](../Node_info_and_management/Node_status.md#SHOW-STATUS) 查看当前缓存状态，相关变量以 `qcache_XXX` 命名：

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

