# 多查询

多查询，或查询批处理，允许您在一次网络请求中向 Manticore 发送多个搜索查询。

👍 为什么使用多查询？

主要原因是性能。通过批量发送请求给 Manticore，而不是逐个发送，您可以通过减少网络往返次数来节省时间。此外，批量发送查询允许 Manticore 执行某些内部优化。如果无法应用批处理优化，查询将单独处理。

⛔ 什么时候不使用多查询？

多查询要求批处理中的所有搜索查询相互独立，但情况并非总是如此。有时查询 B 依赖于查询 A 的结果，这意味着只能在执行查询 A 后设置查询 B。例如，您可能只想在主表中未找到结果时显示来自辅助索引的结果，或者您可能想根据第一个结果集中的匹配数量指定第二个结果集中的偏移量。在这些情况下，您需要使用单独的查询（或单独的批处理）。

使用像 php 中的 `mysqli` 这样的连接库时，您可以添加多个查询，然后将它们作为单个批处理运行。这将作为单个多查询批处理工作。

注意：如果您使用控制台 mysql 客户端，默认情况下它会自行将分号解释为分隔符，然后逐条发送查询到服务器；这不是多查询批处理。要覆盖此行为，请在客户端使用内部命令 `delimiter` 将分隔符重新定义为另一个字符。完成此操作后，客户端将发送带有未更改分号的整个字符串，因此，多查询的功能才能生效。

这种控制台客户端的特殊行为经常令人困惑，因为您可能会注意到同一串命令在 mysql 控制台客户端中的表现与在其它协议如 sql-over-http 中不同。这正是因为 mysql 控制台客户端本身通过分号分割查询，而其他协议可能将整个序列作为单个批处理发送。

<!-- example multi-query 1 -->
您可以通过用分号分隔的方式使用 SQL 运行多个搜索查询。当 Manticore 从客户端收到这样格式化的查询时，将应用所有语句间的优化。

多查询不支持带有 `FACET` 的查询。单个批处理中的多查询数量不应超过 [max_batch_queries](../Server_settings/Searchd.md#max_batch_queries)。


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price DESC; SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price ASC
```
<!-- end -->

在控制台 mysql/mariadb 客户端中：
<!-- request SQL -->

```sql
DELIMITER _
SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price DESC; SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price ASC_
```
<!-- end -->

## 多查询优化

有两种主要的优化需要注意：公共查询优化和公共子树优化。

**公共查询优化** 意味着 `searchd` 会识别批处理中那些仅排序和分组设置不同的查询，并且*只执行一次搜索*。例如，如果批处理中有3个查询，全部针对“ipod nano”，第一个查询请求按价格排序的前10名结果，第二个查询按供应商 ID 分组并请求按评分排序的前5名供应商，第三个查询请求最大价格，则全文搜索“ipod nano”只执行一次，其结果将被重用来构建三个不同的结果集。

[分面搜索](../Searching/Faceted_search.md) 是特别受益于此优化的一个重要场景。实际上，分面搜索可以通过运行几个查询来实现，一个用于获取搜索结果本身，其他几个带有相同全文查询但不同分组设置的查询用于获取所有需要的结果组（如前3名作者，前5名供应商等）。只要全文查询和过滤设置保持不变，公共查询优化将被触发，显著提升性能。

**公共子树优化** 更加有趣。它允许 `searchd` 利用批量全文查询中的相似性。它识别所有查询中共有的全文查询部分（子树），在查询间缓存它们。例如，考虑如下查询批处理：

```bash
donald trump president
donald trump barack obama john mccain
donald trump speech
```

存在一段公共的两词部分 `donald trump`，只需计算一次，然后缓存并在所有查询间共享。公共子树优化就是这样做的。每个查询的缓存大小严格受 [subtree_docs_cache](../Server_settings/Searchd.md#subtree_docs_cache) 和 [subtree_hits_cache](../Server_settings/Searchd.md#subtree_hits_cache) 指令控制（以避免缓存 *所有* 数十亿个匹配“i am”的文档而耗尽内存并导致服务器崩溃）。

<!-- example multi-query 2 -->
如何判断批处理中的查询是否实际进行了优化？如果优化了，相应的查询日志会有一个“multiplier”字段，指定一起处理的查询数量：

注意 "x3" 字段。它表示此查询被优化并在一个包含3个查询的子批处理中处理。


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
作为参考，如果查询没有批处理，常规日志看起来会是这样：


<!-- intro -->
##### 日志:

<!-- request log -->
```bash
[Sun Jul 12 15:18:17.062 2009] 0.059 sec [ext/0/rel 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.156 2009] 0.091 sec [ext/0/ext 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.250 2009] 0.092 sec [ext/0/ext 747541 (0,20)] [lj] the
```
<!-- end -->

注意在多查询情况下，每个查询的执行时间提升了约 1.5 倍到 2.3 倍，具体取决于排序模式。

## 多查询的限制和流程

多查询主要支持批处理查询和在批次结束后接收元信息。因此，批处理中只允许使用少量语句。在一个批处理里，您只能组合使用 `select`、`show` 和 `set` 语句。

您可以像往常一样使用 `select`，但请注意，它们将同时在一次执行中运行。如果查询之间不相关，多查询无益。守护进程会检测这一点，并逐个运行查询。

您可以用 `show` 查看警告、状态、代理状态、元信息、性能分析及执行计划。所有其他 `show` 语句在批处理中
将被静默忽略且无输出。例如，您不能在批处理中执行 `show tables`，或者 `show threads`，或者
`show variables`，或者其他未提及的 `show` 命令。

你只能对 `set profiling` 使用 `set`。所有其他的 `set` 将被静默忽略。

执行顺序也有所不同。守护进程分两遍批处理。

首先，它收集所有的 `select` 语句，同时运行它见到的所有 `set` 语句。作为副作用，只有最后的 `set profiling` 生效。如果你以 `set profiling=1; select...; show meta; show profile; set profiling=0` 运行多查询，你将看不到任何性能分析，因为在第一遍中守护进程执行了 `set profiling=1`，然后立刻执行了最后的 `set profiling=0`。

其次，守护进程尝试执行包含所有收集的 `select` 语句的单批查询。如果语句不相关；它将逐一执行它们。
最后，它迭代初始批处理序列，并为每个 `select` 和 `show` 返回其子结果数据和来自结果集的元数据。所有 `set` 语句已经在第一遍执行过了，因此在第二遍中被跳过。