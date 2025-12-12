# 多查询

多查询，或称查询批次，允许你在单个网络请求中向 Manticore 发送多个搜索查询。

👍 为什么使用多查询？

主要原因是性能。通过批量发送请求到 Manticore 而不是一条条发送，你可以通过减少网络往返次数节省时间。此外，批量发送查询还允许 Manticore 执行某些内部优化。如果无法应用批量优化，查询将单独处理。

⛔ 什么时候不使用多查询？

多查询要求批次中的所有搜索查询相互独立，但这并不总是如此。有时查询 B 依赖于查询 A 的结果，也就是说，只有在执行查询 A 之后才能设置查询 B。例如，你可能只想在主表没有找到结果时显示来自辅助索引的结果，或者你想根据第一组结果中的匹配数为第二组结果指定一个偏移。在这些情况下，你需要使用单独的查询（或单独的批次）。

<!-- example multi-query 1 -->
你可以通过用分号分隔多条 SQL 查询来运行多个搜索查询。当 Manticore 从客户端接收到这样的查询格式时，将应用所有语句间的优化。

多查询不支持包含 `FACET` 的查询。单批次中多查询的数量不应超过 [max_batch_queries](../Server_settings/Searchd.md#max_batch_queries)。


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price DESC; SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price ASC
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price DESC; SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price ASC"
```

<!-- end -->

## 多查询优化

需要注意两个主要优化：通用查询优化和通用子树优化。

**通用查询优化** 意味着 `searchd` 将识别批次中所有仅排序和分组设置不同的查询，并*只进行一次搜索*。例如，如果一个批次包含 3 个查询，且全部都是针对 "ipod nano" 的，但第一个查询请求按价格排序的前 10 个结果，第二个查询按供应商 ID 分组并请求按评分排序的前 5 个供应商，第三个查询请求最高价格，全文搜索 “ipod nano” 只会执行一次，其结果将被重用以构建三组不同的结果。

[分面搜索](../Searching/Faceted_search.md) 是一个特别重要的例子，受益于这一优化。实际上，分面搜索可以通过运行多个查询实现，其中一条用于检索搜索结果本身，另外几条使用相同的全文查询但不同的分组设置来检索所有所需的分组结果（前三名作者、前五名供应商等）。只要全文查询和过滤设置保持不变，通用查询优化就会触发，大幅提升性能。

**通用子树优化** 更为有趣。它允许 `searchd` 利用批量全文查询之间的相似性。它在所有查询中识别出公共的全文查询部分（子树），并在查询间缓存它们。例如，考虑以下查询批次：

```bash
donald trump president
donald trump barack obama john mccain
donald trump speech
```

这里有一个公共的两词部分 `donald trump`，它只需计算一次，然后缓存并在各查询间共享。通用子树优化就是这样做的。单个查询的缓存大小由 [subtree_docs_cache](../Server_settings/Searchd.md#subtree_docs_cache) 和 [subtree_hits_cache](../Server_settings/Searchd.md#subtree_hits_cache) 指令严格控制（这样缓存*所有*匹配 “i am” 的十六亿万文件，不会耗尽内存并立即导致服务器崩溃）。

<!-- example multi-query 2 -->
你怎么判断批次中的查询是否真的被优化了？如果是，相关的查询日志将包含一个 "multiplier" 字段，说明多少查询被一起处理：

注意 "x3" 字段。它表示此查询经过优化，作为包含 3 个查询的子批次处理。


<!-- intro -->
##### 日志：

<!-- request log -->
```bash
[Sun Jul 12 15:18:17.000 2009] 0.040 sec x3 [ext/0/rel 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.000 2009] 0.040 sec x3 [ext/0/ext 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.000 2009] 0.040 sec x3 [ext/0/ext 747541 (0,20)] [lj] the
```
<!-- end -->

<!-- example multi-query 3 -->
供参考，如果查询未批量处理，常规日志会是这样：


<!-- intro -->
##### 日志：

<!-- request log -->
```bash
[Sun Jul 12 15:18:17.062 2009] 0.059 sec [ext/0/rel 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.156 2009] 0.091 sec [ext/0/ext 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.250 2009] 0.092 sec [ext/0/ext 747541 (0,20)] [lj] the
```
<!-- end -->

注意多查询情况中单个查询的执行时间提升了 1.5 到 2.3 倍，具体取决于排序模式。

<!-- proofread -->

