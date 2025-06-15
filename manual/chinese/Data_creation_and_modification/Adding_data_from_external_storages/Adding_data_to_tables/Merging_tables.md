# 合并表格

合并两个现有的**普通**表格可能比从头索引数据更高效，并且在某些情况下是需要的（例如，在“main+delta”分区方案中合并“main”和“delta”表，而不是简单地重建“main”）。因此，`indexer` 提供了一个选项来执行此操作。合并表格通常比重建快，但对于超大表仍然**不是**瞬时的。实际上，它需要读取两个表的内容一次，写入结果一次。例如，合并一个100 GB和一个1 GB的表将导致202 GB的I/O（但这仍可能比从头索引所需的少）。

基本命令语法如下：

```bash
sudo -u manticore indexer --merge DSTINDEX SRCINDEX [--rotate] [--drop-src]
```

除非指定了 `--drop-src`，否则只有 DSTINDEX 表会受到影响：SRCINDEX 的内容将被合并到它里面。

如果 DSTINDEX 已经由 `searchd` 提供服务，则必须使用 `--rotate` 参数。

典型用法是将来自 SRCINDEX 的较小更新合并到 DSTINDEX 中。因此，当合并属性时，如果遇到重复的文档ID，来自 SRCINDEX 的值将优先。然而，请注意，在这种情况下，“旧”关键字**不会**被自动移除。例如，如果 DSTINDEX 中文档123关联了关键字“old”，而SRCINDEX中关联了关键字“new”，那么文档123在合并后将能通过这两个关键字被找到。你可以提供一个显式条件来从 DSTINDEX 中移除文档以缓解此问题；相关参数是 `--merge-dst-range`：

```bash
sudo -u manticore indexer --merge main delta --merge-dst-range deleted 0 0
```

此参数允许你在合并的同时对目标表应用过滤器。可以有多个过滤器；所有条件都必须满足，文档才会包含在合并结果表中。在上例中，过滤器只通过那些‘deleted’字段为0的记录，剔除了所有被标记为删除的记录。

`--drop-src` 允许在合并之后、旋转表之前删除 SRCINDEX，这在你将 DSTINDEX 指定为 DSTINDEX 的 `killlist_target` 时非常重要。否则，在旋转表时，已合并到 DSTINDEX 的文档可能会被 SRCINDEX 抑制。
<!-- proofread -->

