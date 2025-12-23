# 多查询

多查询，或查询批处理，允许您在一个网络请求中向 Manticore 发送多个搜索查询。

👍 为什么使用多查询？

主要原因是性能。通过批量发送请求给 Manticore，而不是一个一个发送，您可以通过减少网络往返节省时间。此外，批量发送查询允许 Manticore 执行某些内部优化。如果无法应用批处理优化，查询将被单独处理。

⛔ 什么时候不使用多查询？

多查询要求批处理中的所有搜索查询都是独立的，但情况并非总是如此。有时查询 B 依赖于查询 A 的结果，这意味着查询 B 只能在执行查询 A 后设置。例如，您可能希望仅在主表未找到结果时才显示来自辅助索引的结果，或者您可能想根据第一结果集中的匹配数量来指定第二结果集的偏移量。在这些情况下，您需要使用单独的查询（或单独的批次）。

使用连接器库时，例如 PHP 中的 `mysqli`，您可以添加多个查询，然后将它们作为一个批处理一起运行。这将作为一个多查询批次工作。

注意：如果您使用命令行 MySQL 客户端，默认情况下它将分号（；）解释为分隔符，并将每个查询单独发送到服务器；这不是多查询批处理。要覆盖此行为，请使用内部命令 `delimiter` 在客户端重新定义分隔符为另一个字符。更改后，客户端将不更改分号地发送整个字符串，从而允许“多查询魔法”发挥作用。

命令行客户端的这种特殊行为常常令人困惑，因为您可能会注意到，同一序列的命令在 MySQL 客户端控制台中表现不同于像 SQL-over-HTTP 等其他协议。这正是因为 MySQL 控制台客户端使用分号划分查询，而其他协议可能将整个序列作为单个批处理发送。

<!-- example multi-query 1 -->
您可以通过用分号分隔多个搜索查询来运行它们。当 Manticore 从客户端接收到这样格式的查询时，所有语句间优化都会被应用。

多查询不支持带有 `FACET` 的查询。一个批处理中的多查询数量不应超过 [max_batch_queries](../Server_settings/Searchd.md#max_batch_queries)。


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price DESC; SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price ASC
```
<!-- end -->

从命令行 MySQL/MariaDB 客户端：
<!-- request SQL -->

```sql
DELIMITER _
SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price DESC; SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price ASC_
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price DESC; SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price ASC"
```

<!-- end -->

## 多查询优化

需要注意的两大主要优化是：公共查询优化和公共子树优化。

**公共查询优化**意味着 `searchd` 会识别批处理内所有仅在排序和分组设置上不同的查询，并*只执行一次搜索*。例如，如果一个批处理包含3个查询，三者都是针对“ipod nano”，但第一个查询请求按价格排序的前10个结果，第二个查询按供应商 ID 分组并请求评分排序的前5个供应商，第三个查询请求最大价格，全文搜索“ipod nano”只会执行一次，其结果将被重用来构建3个不同的结果集。

[分面搜索](../Searching/Faceted_search.md) 是特别重要的一个案例，受益于此优化。确实，分面搜索可以通过运行多个查询来实现：一个用来检索搜索结果本身，还有几个用相同全文查询但不同分组设置的查询来检索所有需要的结果组（前3作者，前5供应商等）。只要全文查询和过滤设置保持不变，公共查询优化就会生效，大大提升性能。

**公共子树优化**更为有趣。它允许 `searchd` 利用批量全文查询间的相似之处。它识别所有查询中共有的全文查询部分（子树），并在查询间缓存它们。例如，考虑以下查询批处理：

```bash
donald trump president
donald trump barack obama john mccain
donald trump speech
```

有一个公共的两词部分 `donald trump`，可以只计算一次，然后缓存并在查询间共享。公共子树优化就是这样工作的。每个查询的缓存大小严格受 [subtree_docs_cache](../Server_settings/Searchd.md#subtree_docs_cache) 和 [subtree_hits_cache](../Server_settings/Searchd.md#subtree_hits_cache) 指令控制（以防缓存所有匹配“i am”的无数文件耗尽内存并立刻导致服务器崩溃）。

<!-- example multi-query 2 -->
如何判断批处理中的查询是否真正被优化？如果是，相关查询日志将包含“multiplier”字段，指定一起处理了多少查询：

注意“x3”字段。它表示该查询经过优化并在一个3个查询的子批处理中处理。


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
作为参考，如果查询未批处理，常规日志如下所示：


<!-- intro -->
##### 日志:

<!-- request log -->
```bash
[Sun Jul 12 15:18:17.062 2009] 0.059 sec [ext/0/rel 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.156 2009] 0.091 sec [ext/0/ext 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.250 2009] 0.092 sec [ext/0/ext 747541 (0,20)] [lj] the
```
<!-- end -->

注意在多查询情况下，每个查询的时间提升了1.5到2.3倍，具体取决于具体的排序模式。

## 多查询的限制与流程

多查询主要支持批量查询并接收批次后的元信息。由于此限制，批处理仅允许使用少部分语句。在一个批处理内，只能组合 `SELECT`、`SHOW` 和 `SET` 语句。

您可以照常使用 `SELECT`；但请注意，所有查询将一起在单次处理过程中执行。如果查询不相关，多查询不会带来好处。守护进程将检测到这一点，并一一运行这些查询。

您可以使用 `SHOW` 来处理 *warnings*、*status*、*agent status*、*meta*、*profile* 和 *plan*。所有其他批处理中的 `SHOW` 语句将被静默忽略且无输出。例如，当批处理时，您不能执行 `SHOW TABLES`、`SHOW THREADS` 或 `SHOW VARIABLES`，或任何未在上述提及的语句。

您只能对 `SET` 使用 `SET PROFILING`。所有其他 `SET ...` 命令将被静默忽略。

执行顺序也不同。守护进程分两遍处理批处理。

首先，它收集所有 `SELECT` 语句并同时执行它看到的所有 `SET PROFILING` 语句。作为副作用，只有最后一个 `SET PROFILING` 语句有效。如果您执行一个多查询如：`SET PROFILING=1; SELECT...; SHOW META; SHOW PROFILE; SET PROFILING=0`，您将看不到任何配置文件，因为在第一遍中，守护进程执行了 `SET PROFILING=1`，然后立即执行了 `SET PROFILING=0`。

其次，守护进程尝试执行包含所有收集到的 `SELECT` 语句的单批查询。如果语句不相关，它将逐一执行。

最后，它遍历初始批处理序列，并从结果集中返回每个 `SELECT` 和 `SHOW` 的子结果数据和元信息。由于所有 `SET PROFILING` 语句已在第一遍执行，它们在第二遍中被跳过。
each `SELECT` and `SHOW`. Since all `SET PROFILING` statements were executed in the first pass, they are skipped on this second pass.