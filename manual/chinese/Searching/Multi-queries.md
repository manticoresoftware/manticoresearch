# 多查询

多查询，或称查询批处理，允许您在一次网络请求中向 Manticore 发送多个搜索查询。

👍 为什么使用多查询？

主要原因是性能。通过批量发送请求给 Manticore，而不是逐个发送，您可以通过减少网络往返时间来节省时间。此外，批量发送查询允许 Manticore 执行某些内部优化。如果无法应用批量优化，查询将被单独处理。

⛔ 何时不使用多查询？

多查询要求批处理中的所有搜索查询相互独立，但情况并非总是如此。有时查询 B 依赖于查询 A 的结果，这意味着查询 B 只能在执行查询 A 后设置。例如，您可能只想在主表中未找到结果时显示来自辅助索引的结果，或者您可能想根据第一个结果集中的匹配数指定第二个结果集的偏移量。在这些情况下，您需要使用单独的查询（或单独的批处理）。

<!-- example multi-query 1 -->
您可以通过用分号分隔多个搜索查询来运行多查询。当 Manticore 从客户端接收到这样格式的查询时，将应用所有语句间的优化。

多查询不支持带有 `FACET` 的查询。单个批处理中的多查询数量不应超过 [max_batch_queries](../Server_settings/Searchd.md#max_batch_queries)。


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price DESC; SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price ASC
```
<!-- end -->

## 多查询优化

需要注意两种主要优化：公共查询优化和公共子树优化。

**公共查询优化** 意味着 `searchd` 会识别批处理中所有仅排序和分组设置不同的查询，并且*只执行一次搜索*。例如，如果一个批处理包含 3 个查询，它们都是针对“ipod nano”的，但第一个查询请求按价格排序的前 10 个结果，第二个查询按供应商 ID 分组并请求按评分排序的前 5 个供应商，第三个查询请求最大价格，则全文搜索“ipod nano”只会执行一次，其结果将被重用以构建 3 个不同的结果集。

[分面搜索](../Searching/Faceted_search.md) 是特别受益于此优化的一个重要案例。实际上，分面搜索可以通过运行多个查询来实现，一个用于检索搜索结果本身，其他几个使用相同的全文查询但不同的分组设置来检索所有所需的结果组（前 3 名作者，前 5 名供应商等）。只要全文查询和过滤设置保持不变，公共查询优化就会触发，并大大提高性能。

**公共子树优化** 更加有趣。它允许 `searchd` 利用批量全文查询之间的相似性。它识别所有查询中的公共全文查询部分（子树）并在查询之间缓存它们。例如，考虑以下查询批处理：

```bash
donald trump president
donald trump barack obama john mccain
donald trump speech
```

有一个公共的两词部分 `donald trump`，只需计算一次，然后缓存并在查询间共享。公共子树优化正是这样做的。每个查询的缓存大小由 [subtree_docs_cache](../Server_settings/Searchd.md#subtree_docs_cache) 和 [subtree_hits_cache](../Server_settings/Searchd.md#subtree_hits_cache) 指令严格控制（以防缓存*所有*匹配“i am”的十六亿亿文档耗尽内存并立即导致服务器崩溃）。

<!-- example multi-query 2 -->
如何判断批处理中的查询是否真的被优化了？如果是，相关的查询日志将有一个“multiplier”字段，指定一起处理了多少个查询：

注意“x3”字段。这意味着该查询被优化并在一个包含 3 个查询的子批处理中处理。


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
作为参考，如果查询未批处理，常规日志将如下所示：


<!-- intro -->
##### 日志:

<!-- request log -->
```bash
[Sun Jul 12 15:18:17.062 2009] 0.059 sec [ext/0/rel 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.156 2009] 0.091 sec [ext/0/ext 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.250 2009] 0.092 sec [ext/0/ext 747541 (0,20)] [lj] the
```
<!-- end -->

注意多查询情况下每个查询的时间提升了 1.5 倍到 2.3 倍，具体取决于排序模式。

<!-- proofread -->

