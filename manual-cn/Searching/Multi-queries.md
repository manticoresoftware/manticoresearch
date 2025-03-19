# 多查询

多查询（也称为查询批处理）允许您在一次网络请求中向 Manticore 发送多个搜索查询。

👍 为什么使用多查询？

主要原因是性能。通过批量发送请求而不是逐个发送，可以减少网络往返时间，从而节省时间。此外，批量发送查询可以让 Manticore 执行某些内部优化。如果无法应用批量优化，查询将被单独处理。

⛔ 何时不应使用多查询？

多查询要求批处理中的所有搜索查询彼此独立，而这并不总是适用。有时，查询 B 依赖于查询 A 的结果，意味着必须在执行完查询 A 后才能设置查询 B。例如，您可能希望仅在主表中未找到结果时才显示次级索引中的结果，或者基于第一个结果集中的匹配数量指定第二个结果集的偏移量。在这些情况下，您需要使用单独的查询（或单独的批处理）。

<!-- example multi-query 1 -->

您可以通过使用分号分隔多个 SQL 查询来运行多次搜索查询。当 Manticore 从客户端收到这种格式的查询时，将应用所有语句间的优化。

多查询不支持带有 `FACET` 的查询。一次批处理中多查询的数量不应超过 [max_batch_queries](../Server_settings/Searchd.md#max_batch_queries)。

<!-- intro -->

##### SQL:

<!-- request SQL -->

```sql
SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price DESC; SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price ASC
```
<!-- end -->

## 多查询优化

主要有两种优化需要注意：**公共查询优化**和**公共子树优化**。

**公共查询优化**指的是 `searchd` 会识别出批处理中仅排序和分组设置不同的查询，并*只执行一次搜索*。例如，如果批处理中有 3 个查询，所有查询的关键词都是 "ipod nano"，但第一个查询请求按价格排序的前 10 个结果，第二个查询按供应商 ID 分组并请求前 5 个供应商按评分排序，第三个查询请求最高价格。此时，针对 "ipod nano" 的全文搜索只会执行一次，然后搜索结果会被复用来构建 3 个不同的结果集。

[分面搜索](../Searching/Faceted_search.md) 是从此优化中受益的重要案例之一。分面搜索可以通过运行多个查询来实现，一个查询检索搜索结果本身，其他查询使用相同的全文搜索，但具有不同的分组设置，以获取所需的结果组（如前 3 名作者、前 5 名供应商等）。只要全文搜索查询和过滤设置保持不变，公共查询优化将会触发，从而显著提升性能。

**公共子树优化**更为有趣。它允许 `searchd` 利用批量全文查询中的相似性，识别所有查询中的公共全文查询部分（子树），并在查询之间缓存这些部分。例如，考虑以下查询批次：

```bash
donald trump president
donald trump barack obama john mccain
donald trump speech
```

在这些查询中，`donald trump` 是一个共同的两词部分，只需计算一次，然后缓存并在查询中共享。公共子树优化就是这样做的。每个查询的缓存大小由 [subtree_docs_cache](../Server_settings/Searchd.md#subtree_docs_cache) 和 [subtree_hits_cache](../Server_settings/Searchd.md#subtree_hits_cache) 指令严格控制（这样缓存匹配 "i am" 的所有数十亿文档不会耗尽内存并导致服务器宕机）。

<!-- example multi-query 2 -->
如果查询被优化处理，相应的查询日志中会有一个 "multiplier" 字段，说明有多少查询被一起处理：

注意 "x3" 字段，这意味着此查询已优化并作为 3 个查询的子批次一起处理。


<!-- intro -->
##### log:

<!-- request log -->
```bash
[Sun Jul 12 15:18:17.000 2009] 0.040 sec x3 [ext/0/rel 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.000 2009] 0.040 sec x3 [ext/0/ext 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.000 2009] 0.040 sec x3 [ext/0/ext 747541 (0,20)] [lj] the
```
<!-- end -->

<!-- example multi-query 3 -->
作为参考，以下是查询未进行批处理时的常规日志：


<!-- intro -->
##### log:

<!-- request log -->
```bash
[Sun Jul 12 15:18:17.062 2009] 0.059 sec [ext/0/rel 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.156 2009] 0.091 sec [ext/0/ext 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.250 2009] 0.092 sec [ext/0/ext 747541 (0,20)] [lj] the
```
<!-- end -->

注意，在多查询情况下，每个查询的处理时间提升了 1.5 倍至 2.3 倍，具体提升取决于排序模式。

<!-- proofread -->