# 多查询

多查询，或查询批处理，允许您在一个网络请求中向 Manticore 发送多个搜索查询。

👍 为什么使用多查询？

主要原因在于性能。通过将请求以批处理的形式发送到 Manticore 而不是逐个发送，可以节省时间并减少网络往返次数。此外，以批处理的形式发送查询可以让 Manticore 执行某些内部优化。如果无法应用批处理优化，查询将逐个处理。

⛔ 何时不使用多查询？

多查询要求批处理中的所有搜索查询都是独立的，但这并不总是成立。有时查询 B 依赖于查询 A 的结果，这意味着查询 B 只能在执行查询 A 之后才能设置。例如，您可能只想在主表中未找到结果时显示次级索引的结果，或者您可能需要根据第一个结果集中的匹配数量指定第二个结果集中的偏移量。在这种情况下，您将需要使用单独的查询（或单独的批处理）。

当使用连接器库时，例如 PHP 中的 `mysqli`，您可以添加多个查询，然后将它们作为一个批处理运行。这将作为一个单个多查询批处理工作。

注意：如果您使用控制台 MySQL 客户端，默认情况下它会将分号（;）解释为分隔符本身，并逐个将每个查询发送到服务器；这不是一个多查询批处理。要覆盖此行为，可以在客户端侧使用内部命令 `delimiter` 重新定义分隔符。在做出此更改后，客户端将发送整个包含分号的字符串，允许“多查询魔法”生效。

控制台客户端的这种行为有时会令人困惑，因为您可能会注意到相同的命令序列在 MySQL 客户端控制台中与 SQL-over-HTTP 等其他协议的行为有所不同。这是因为 MySQL 控制台客户端本身使用分号来划分查询，但其他协议可能会将整个序列作为一个批处理发送。

<!-- example multi-query 1 -->
您可以使用 SQL 通过分号分隔多个搜索查询。当 Manticore 从客户端接收到格式为这样的查询时，所有语句间的优化都将被应用。

多查询不支持带有 `FACET` 的查询。一个批处理中的多查询数量不应超过 [max_batch_queries](../Server_settings/Searchd.md#max_batch_queries)。


<!-- intro -->
##### SQL：

<!-- request SQL -->

```sql
SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price DESC; SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price ASC
```
<!-- end -->

从控制台 MySQL/MariaDB 客户端：
<!-- request SQL -->

```sql
DELIMITER _
SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price DESC; SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price ASC_
```
<!-- end -->

## 多查询优化

有两种主要的优化需要了解：公共查询优化和公共子树优化。

**公共查询优化**意味着 `searchd` 将识别批处理中所有仅排序和分组设置不同的查询，并且 *只执行一次搜索*。例如，如果批处理包含 3 个查询，它们都是针对“ipod nano”的，但第一个查询请求按价格排序的前 10 个结果，第二个查询按供应商 ID 分组并请求按评分排序的前 5 个供应商，第三个查询请求最高价格，全文搜索“ipod nano”将只执行一次，并且其结果将被重用以构建 3 个不同的结果集。

[分面搜索](../Searching/Faceted_search.md) 是特别重要的一种情况，可以从这种优化中受益。确实，分面搜索可以通过运行几个查询来实现，一个查询用于检索搜索结果本身，几个其他查询带有相同的全文查询但不同的分组设置，以检索所有所需的分组结果（前 3 位作者，前 5 位供应商等）。只要全文查询和过滤设置保持不变，公共查询优化就会触发，并大大提高性能。

**公共子树优化**更加有趣。它允许 `searchd` 利用批处理中的全文查询之间的相似性。它会识别所有查询中的公共全文查询部分（子树），并在查询之间缓存它们。例如，考虑以下查询批处理：

```bash
donald trump president
donald trump barack obama john mccain
donald trump speech
```

有一个公共的两词部分 `donald trump`，它只需要计算一次，然后缓存并共享给所有查询。公共子树优化正是这样做的。每个查询的缓存大小由 [subtree_docs_cache](../Server_settings/Searchd.md#subtree_docs_cache) 和 [subtree_hits_cache](../Server_settings/Searchd.md#subtree_hits_cache) 指令严格控制（以确保缓存所有 160 亿个匹配“i am”的文档不会耗尽内存并立即杀死您的服务器）。

<!-- example multi-query 2 -->
如何知道批处理中的查询是否实际进行了优化？如果进行了优化，相应的查询日志将有一个“倍数”字段，指定了一起处理了多少个查询：

注意“x3”字段。这意味着此查询被优化并作为一个子批处理中的 3 个查询一起处理。


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
作为参考，如果查询未批处理，常规日志将如下所示：


<!-- intro -->
##### 日志：

<!-- request log -->
```bash
[Sun Jul 12 15:18:17.062 2009] 0.059 sec [ext/0/rel 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.156 2009] 0.091 sec [ext/0/ext 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.250 2009] 0.092 sec [ext/0/ext 747541 (0,20)] [lj] the
```
<!-- end -->

注意，在多查询情况下，每个查询的时间提高了 1.5 到 2.3 倍，具体取决于特定的排序模式。

## 多查询限制和流程

多查询主要用于批处理查询并接收此类批处理的元数据。由于这一限制，批处理中只允许一小部分语句。在一个批处理中，您可以组合 `SELECT`、`SHOW` 和 `SET` 语句。

您可以像平常一样使用 `SELECT`；然而，请注意，所有查询将在一次通过中运行。如果查询之间没有关联，多查询就没有好处。守护进程会检测到这一点，并逐个运行查询。

你可以使用 `SHOW` 来处理 *警告*, *状态*, *代理状态*, *元数据*, *配置文件* 和 *计划*。所有其他在批次中的 `SHOW` 语句将被静默忽略，没有任何输出。例如，你不能执行 `SHOW TABLES`, `SHOW THREADS`, 或 `SHOW VARIABLES`，或任何其他未提及的语句进行批次处理。

你可以仅使用 `SET` 来设置 `SET PROFILING`。所有其他 `SET ...` 命令将被静默忽略。

执行的顺序也不同。守护进程在两轮中处理批次。

首先，它收集所有 `SELECT` 语句，并同时运行它看到的所有 `SET PROFILING` 语句。作为副作用，只有最后一个 `SET PROFILING` 语句有效。如果你执行一个类似的多查询语句，如 `SET PROFILING=1; SELECT...; SHOW META; SHOW PROFILE; SET PROFILING=0`，你将看不到任何配置文件，因为在第一轮中，守护进程执行了 `SET PROFILING=1`，然后立即执行了 `SET PROFILING=0`。

第二轮，守护进程尝试使用收集的所有 `SELECT` 语句执行单个批次查询。如果语句不相关，它将依次执行它们。

最后，它遍历初始批次序列，并返回结果集中的每个 `SELECT` 和 `SHOW` 的子结果数据和元数据。由于所有 `SET PROFILING` 语句在第一轮中已执行，因此在第二轮中将被跳过。
each `SELECT` and `SHOW`. Since all `SET PROFILING` statements were executed in the first pass, they are skipped on this second pass.