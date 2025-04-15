# 多查询

多查询或查询批处理允许您在单个网络请求中向Manticore发送多个搜索查询。

👍 为什么使用多查询？

主要原因是性能。通过批量发送请求到Manticore，而不是逐个发送，您可以通过减少网络往返时间来节省时间。此外，批量发送查询还允许Manticore执行某些内部优化。如果无法应用批处理优化，查询将单独处理。

⛔ 何时不使用多查询？

多查询要求批处理中的所有搜索查询都是独立的，这并不总是情况。有时查询B依赖于查询A的结果，这意味着查询B只能在执行查询A之后设置。例如，您可能希望仅在主表没有找到结果的情况下显示来自辅助索引的结果，或者您可能希望根据第一个结果集中的匹配数量指定第二个结果集中的偏移量。在这些情况下，您需要使用单独的查询（或单独的批处理）。

<!-- example multi-query 1 -->
您可以通过用分号分隔来使用SQL运行多个搜索查询。当Manticore从客户端接收到像这样的格式化查询时，将应用所有语句之间的优化。

多查询不支持带有`FACET`的查询。一个批处理中的多查询数量不应超过[max_batch_queries](../Server_settings/Searchd.md#max_batch_queries)。


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price DESC; SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price ASC
```
<!-- end -->

## 多查询优化

有两个主要的优化需要注意：常见查询优化和常见子树优化。

**常见查询优化**意味着`searchd`将识别批处理中的所有查询，这些查询仅排序和分组设置不同，并*只执行一次搜索*。例如，如果一个批处理由3个查询组成，所有查询都是针对“ipod nano”，但第一个查询请求按价格排序的前10个结果，第二个查询按供应商ID分组并请求按评级排序的前5个供应商，第三个查询请求最大价格，针对“ipod nano”的全文搜索将只执行一次，其结果将被重用以构建3个不同的结果集。

[分面搜索](../Searching/Faceted_search.md)是一个特别重要的案例，它受益于此优化。实际上，可以通过运行多个查询来实现分面搜索，一个查询用于检索搜索结果本身，其他几个查询使用相同的全文查询但具有不同的分组设置来检索所有所需的结果组（前3位作者、前5位供应商等）。只要全文查询和过滤设置保持不变，常见查询优化将被触发，并极大地提高性能。

**常见子树优化**更有趣。它允许`searchd`在批量的全文查询之间利用相似性。它识别所有查询中的共同全文查询部分（子树），并在查询之间缓存它们。例如，考虑以下查询批处理：

```bash
donald trump president
donald trump barack obama john mccain
donald trump speech
```

有一个共同的两词部分`donald trump`，可以只计算一次，然后缓存并在查询之间共享。而常见子树优化正是这样做的。每个查询的缓存大小由[subtree_docs_cache](../Server_settings/Searchd.md#subtree_docs_cache)和[subtree_hits_cache](../Server_settings/Searchd.md#subtree_hits_cache)指令严格控制（以免缓存*所有*匹配“我在”的十六亿个文档而耗尽内存并瞬间杀死您的服务器）。

<!-- example multi-query 2 -->
如何判断批处理中的查询是否真正得到了优化？如果得到了，相关的查询日志将有一个“乘数”字段，指定一起处理了多少个查询：

注意“x3”字段。这意味着该查询经过优化并在一个3个查询的子批处理中处理。


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
供参考，如果查询没有被批处理，常规日志将如下所示：


<!-- intro -->
##### log:

<!-- request log -->
```bash
[Sun Jul 12 15:18:17.062 2009] 0.059 sec [ext/0/rel 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.156 2009] 0.091 sec [ext/0/ext 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.250 2009] 0.092 sec [ext/0/ext 747541 (0,20)] [lj] the
```
<!-- end -->

请注意，多查询情况下的每个查询时间提高了1.5到2.3倍，具体取决于具体的排序模式。

<!-- proofread -->
