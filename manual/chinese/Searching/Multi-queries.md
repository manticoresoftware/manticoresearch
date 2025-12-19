# 多查询

多查询，或称查询批量，允许您在单个网络请求中向 Manticore 发送多个搜索查询。

👍 为什么使用多查询？

主要原因是性能。通过批量发送请求给 Manticore，而不是一个一个发送，您可以通过减少网络往返次数来节省时间。此外，批量发送查询允许 Manticore 执行某些内部优化。如果无法应用批量优化，查询将被单独处理。

⛔ 何时不使用多查询？

多查询要求批量中的所有搜索查询相互独立，但情况并非总是如此。有时查询 B 依赖查询 A 的结果，这意味着查询 B 只能在执行查询 A 之后设置。例如，您可能想要仅当主要表中未找到结果时才显示辅助索引的结果，或者您可能想基于第一结果集中的匹配数来指定第二结果集的偏移量。在这些情况下，您需要使用单独的查询（或单独的批量）。

使用连接库时，例如 PHP 中的 `mysqli`，您可以添加多个查询，然后将它们作为单个批次一起运行。这将被视为单个多查询批次。

注意：如果您使用命令行 MySQL 客户端，默认情况下它会将分号（;）解释为分隔符，并单独发送每个查询到服务器；这不是多查询批次。要覆盖此行为，请在客户端使用内部命令 `delimiter` 将分隔符重新定义为其他字符。更改后，客户端将发送整个字符串，保留分号不变，从而使“多查询魔法”生效。

命令行客户端的此行为通常令人困惑，因为您可能注意到，同一系列命令在 MySQL 客户端控制台和其他协议（如 SQL-over-HTTP）中表现不同。这正是因为 MySQL 控制台客户端本身使用分号划分查询，但其他协议可能将整个序列作为单一批量发送。

<!-- example multi-query 1 -->
您可以通过用分号分隔多个搜索查询，并用 SQL 运行它们。当 Manticore 从客户端接收到这样的格式的查询时，会应用所有语句间的优化。

多查询不支持带 `FACET` 的查询。单个批次中的多查询数量不应超过 [max_batch_queries](../Server_settings/Searchd.md#max_batch_queries)。


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price DESC; SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price ASC
```
<!-- end -->

来自命令行 MySQL/MariaDB 客户端：
<!-- request SQL -->

```sql
DELIMITER _
SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price DESC; SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price ASC_
```
<!-- end -->

## 多查询优化

有两个主要的优化需要了解：公共查询优化和公共子树优化。

**公共查询优化** 意味着 `searchd` 会识别批次中仅排序和分组设置不同的所有查询，并且*只执行一次搜索*。例如，如果批次由 3 个查询组成，它们全部针对 “ipod nano”，但第一个查询请求按价格排序的前10个结果，第二个查询按供应商 ID 分组并请求按评级排序的前5个供应商，第三个查询请求最大价格，则只进行一次全文搜索 “ipod nano”，其结果将被重用以构建 3 个不同的结果集。

[分面搜索](../Searching/Faceted_search.md) 是特别受益于此优化的一个应用。确实，分面搜索可以通过运行多个查询实现，一个用于检索搜索结果本身，其他几个使用相同的全文查询但不同的分组设置以检索所有需要的结果组（前三名作者，前五名供应商等）。只要全文查询和过滤设置保持不变，就会触发公共查询优化，从而大幅提升性能。

**公共子树优化** 更为有趣。它允许 `searchd` 利用批量全文查询间的相似性。它识别所有查询中的公共全文查询部分（子树）并在查询间缓存它们。例如，考虑以下查询批次：

```bash
donald trump president
donald trump barack obama john mccain
donald trump speech
```

有一个共同的双词部分 `donald trump`，它只需计算一次，然后缓存并在查询间共享。公共子树优化正是这么做的。每个查询的缓存大小由 [subtree_docs_cache](../Server_settings/Searchd.md#subtree_docs_cache) 和 [subtree_hits_cache](../Server_settings/Searchd.md#subtree_hits_cache) 指令严格控制（以防缓存 *所有* 十六万亿条匹配 “i am” 的文档导致 RAM 耗尽并立即崩溃服务器）。

<!-- example multi-query 2 -->
如何判断批次中的查询是否被真正优化？如果是，相关的查询日志将包含一个“multiplier”字段，指定一起处理的查询数：

注意 "x3" 字段。这意味着该查询被优化并在一个包含3个查询的子批次中处理。


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
作为参照，如果查询未批量处理，常规日志将如以下所示：


<!-- intro -->
##### 日志:

<!-- request log -->
```bash
[Sun Jul 12 15:18:17.062 2009] 0.059 sec [ext/0/rel 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.156 2009] 0.091 sec [ext/0/ext 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.250 2009] 0.092 sec [ext/0/ext 747541 (0,20)] [lj] the
```
<!-- end -->

请注意，根据具体的排序模式，多查询情况下每个查询的执行时间提升了 1.5 倍到 2.3 倍。

## 多查询的限制与流程

多查询主要支持将查询批量处理并在此类批次后接收元信息。由于此限制，批次中仅允许一小部分语句。在单个批次中，您只能组合使用 `SELECT`、`SHOW` 和 `SET` 语句。

您可以像往常一样使用 `SELECT`；但请注意所有查询将一起在单个遍历中运行。如果查询彼此无关，多查询不会有任何优势。守护进程会检测到这一点，并逐个运行查询。

您可以使用 `SHOW` 来处理 *warnings*、*status*、*agent status*、*meta*、*profile* 和 *plan*。所有其他批处理中的 `SHOW` 语句将被静默忽略且无输出。例如，当批处理时，您不能执行 `SHOW TABLES`、`SHOW THREADS` 或 `SHOW VARIABLES`，或任何未在上述提及的语句。

您只能对 `SET` 使用 `SET PROFILING`。所有其他 `SET ...` 命令将被静默忽略。

执行顺序也不同。守护进程分两遍处理批处理。

首先，它收集所有 `SELECT` 语句并同时执行它看到的所有 `SET PROFILING` 语句。作为副作用，只有最后一个 `SET PROFILING` 语句有效。如果您执行一个多查询如：`SET PROFILING=1; SELECT...; SHOW META; SHOW PROFILE; SET PROFILING=0`，您将看不到任何配置文件，因为在第一遍中，守护进程执行了 `SET PROFILING=1`，然后立即执行了 `SET PROFILING=0`。

其次，守护进程尝试执行包含所有收集到的 `SELECT` 语句的单批查询。如果语句不相关，它将逐一执行。

最后，它遍历初始批处理序列，并从结果集中返回每个 `SELECT` 和 `SHOW` 的子结果数据和元信息。由于所有 `SET PROFILING` 语句已在第一遍执行，它们在第二遍中被跳过。
each `SELECT` and `SHOW`. Since all `SET PROFILING` statements were executed in the first pass, they are skipped on this second pass.