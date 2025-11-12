# 多查询

多查询，或称查询批处理，允许您在一次网络请求中向 Manticore 发送多个搜索查询。

👍 为什么使用多查询？

主要原因是性能。通过批量发送请求给 Manticore，而不是一条条发送，您可以通过减少网络往返时间来节省时间。此外，批量发送查询还允许 Manticore 执行某些内部优化。如果无法应用批处理优化，查询将被单独处理。

⛔ 什么时候不使用多查询？

多查询要求批处理中所有搜索查询相互独立，但情况并非总是如此。有时查询 B 依赖于查询 A 的结果，也就是说查询 B 只能在执行查询 A 后才能设置。例如，您可能想在主表中没有找到结果时，显示来自辅助索引的结果，或者您可能想基于第一个结果集中的匹配数，指定第二个结果集的偏移量。在这些情况下，您需要使用单独的查询（或单独的批处理）。

<!-- example multi-query 1 -->
您可以通过用分号分隔多个搜索查询来使用 SQL 运行多查询。当 Manticore 从客户端接收到这样格式的查询时，所有语句间的优化都将被应用。

多查询不支持带 `FACET` 的查询。单个批处理中的多查询数量不应超过 [max_batch_queries](../Server_settings/Searchd.md#max_batch_queries)。


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price DESC; SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price ASC
```
<!-- end -->

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price DESC; SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price ASC"
```
<!-- end -->

## 多查询优化

主要有两种需要注意的优化：公共查询优化和公共子树优化。

**公共查询优化** 意味着 `searchd` 会识别批处理中所有仅排序和分组设置不同的查询，并*只执行一次搜索*。例如，如果批处理中有 3 个查询，它们都是针对 "ipod nano" 的，但第一个查询请求按价格排序的前 10 条结果，第二个查询按供应商 ID 分组并请求按评分排序的前 5 个供应商，第三个查询请求最大价格，则全文搜索 "ipod nano" 只会执行一次，其结果将被重用以构建这 3 个不同的结果集。

[分面搜索](../Searching/Faceted_search.md) 是特别受益于此优化的一个重要案例。事实上，分面搜索可以通过运行多个查询来实现，一个查询检索搜索结果本身，几个其他查询使用相同的全文查询但不同的分组设置，以检索所有所需的结果组（前 3 名作者，前 5 名供应商等）。只要全文查询和过滤设置保持相同，公共查询优化便会被触发，大幅提升性能。

**公共子树优化** 则更加有趣。它允许 `searchd` 利用批量全文查询之间的相似性。它识别所有查询中的公共全文查询部分（子树），并在查询间缓存它们。例如，考虑以下查询批处理：

```bash
donald trump president
donald trump barack obama john mccain
donald trump speech
```

这里存在一个共同的两词部分 `donald trump`，只需计算一次，然后缓存并在查询间共享。公共子树优化就是这样工作的。每个查询的缓存大小由 [subtree_docs_cache](../Server_settings/Searchd.md#subtree_docs_cache) 和 [subtree_hits_cache](../Server_settings/Searchd.md#subtree_hits_cache) 指令严格控制（以防缓存 *所有* 匹配 “i am” 的十六亿兆文档耗尽内存并立即使服务器崩溃）。

<!-- example multi-query 2 -->
怎么判断批处理中的查询是否确实被优化了？如果是，相关查询日志将包含一个 "multiplier" 字段，指定一起处理了多少个查询：

注意 "x3" 字段。这意味着此查询被优化，并在一个包含 3 个查询的子批处理中处理。


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

注意多查询情况下每个查询的时间提升了 1.5 到 2.3 倍，具体取决于排序模式。

<!-- proofread -->

