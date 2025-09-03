# 多查询

多查询，或查询批处理，允许你在单个网络请求中向 Manticore 发送多个搜索查询。

👍 为什么使用多查询？

主要原因是性能。通过批量发送请求到 Manticore，而不是逐个发送，可以通过减少网络往返次数节省时间。此外，批量发送查询允许 Manticore 执行某些内部优化。如果无法应用批量优化，查询将被单独处理。

⛔ 什么时候不使用多查询？

多查询要求批处理中的所有搜索查询必须是独立的，而这并不总是如此。有时查询 B 依赖于查询 A 的结果，意味着查询 B 只能在执行查询 A 后设置。例如，你可能只想在主表中没有找到结果时显示来自辅助索引的结果，或者你可能想基于第一组结果的匹配数量指定第二组结果的偏移量。在这些情况下，你需要使用单独的查询（或单独的批处理）。

<!-- example multi-query 1 -->
你可以通过用分号分隔多个搜索查询来使用 SQL 执行多查询。当 Manticore 从客户端接收到这样格式化的查询时，将应用所有语句间的优化。

多查询不支持带 `FACET` 的查询。一个批处理中的多查询数量不应超过 [max_batch_queries](../Server_settings/Searchd.md#max_batch_queries)。


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price DESC; SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price ASC
```
<!-- end -->

## 多查询优化

有两个主要的优化需要注意：公共查询优化和公共子树优化。

**公共查询优化**意味着 `searchd` 会识别批处理中那些除了排序和分组设置不同外其他都相同的查询，并且*只执行一次搜索*。例如，如果一个批处理包含3个查询，它们都是针对“ipod nano”的，但第1个查询请求按价格排序的前10条结果，第2个查询按供应商ID分组并请求按评分排序的前5个供应商，第3个查询请求最大价格，那么全文搜索“ipod nano”只会执行一次，结果将被重用以构建三组不同的结果集。

[分面搜索](../Searching/Faceted_search.md) 是特别重要的一个案例，受益于此优化。实际上，分面搜索可以通过运行多个查询来实现，一个查询用于检索搜索结果本身，另几个带有相同的全文查询但不同分组设置以获取所有所需的结果组（前3名作者，前5名供应商等）。只要全文查询和过滤设置保持不变，公共查询优化就会触发，大幅提升性能。

**公共子树优化**甚至更有趣。它允许 `searchd` 利用批量全文查询间的相似性。它识别所有查询中共有的全文查询部分（子树）并在查询间缓存它们。例如，考虑以下查询批：

```bash
donald trump president
donald trump barack obama john mccain
donald trump speech
```

这里有一个两个词的共通部分 `donald trump`，只需计算一次，之后缓存并在查询间共享。公共子树优化正是这么做的。单个查询的缓存大小由 [subtree_docs_cache](../Server_settings/Searchd.md#subtree_docs_cache) 和 [subtree_hits_cache](../Server_settings/Searchd.md#subtree_hits_cache) 指令严格控制（这样缓存“i am”匹配的*所有*十六个亿万的文档就不会耗尽内存并立即导致服务器崩溃）。

<!-- example multi-query 2 -->
你如何判断批处理中的查询是否真正被优化了？如果是，被查询的日志中会有一个“multiplier”字段，指定了多少查询被一起处理：

注意“x3”字段。它表示该查询被优化并在一个包含3个查询的子批处理中处理。


<!-- intro -->
##### 日志:

<!-- request log -->
```bash
[Sun Jul 12 15:18:17.000 2009] 0.040 sec x3 [ext/0/rel 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.000 2009] 0.040 sec x3 [ext/0/ext 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.000 2009] 0.040 sec x3 [ext/0/ext 747541 (0,20)] [lj] the
```
<!-- end -->

<!-- example multi-query 3 -->
作为参考，如果查询没有批处理，常规日志会是这样：


<!-- intro -->
##### 日志:

<!-- request log -->
```bash
[Sun Jul 12 15:18:17.062 2009] 0.059 sec [ext/0/rel 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.156 2009] 0.091 sec [ext/0/ext 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.250 2009] 0.092 sec [ext/0/ext 747541 (0,20)] [lj] the
```
<!-- end -->

注意多查询情况下单个查询所需时间相比提高了1.5倍到2.3倍，具体取决于排序模式。

<!-- proofread -->

