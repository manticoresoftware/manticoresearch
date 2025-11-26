# 合并表格

合并两个现有的**普通**表格可能比从头开始索引数据更高效，并且在某些情况下可能是需要的（例如在“main+delta”分区方案中合并“main”和“delta”表，而不是简单地重建“main”表）。因此，`indexer` 提供了一个选项来实现这一点。合并表格通常比重建更快，但对于巨大的表格来说仍然**不是**瞬间完成的。基本上，它需要读取两个表的内容一次，并写入结果一次。例如，合并一个100 GB和一个1 GB的表，将导致202 GB的I/O（但这仍然可能少于从头索引所需的量）。

基本命令语法如下：

```bash
sudo -u manticore indexer --merge DSTINDEX SRCINDEX [--rotate] [--drop-src]
```

除非指定了 `--drop-src`，否则只有 DSTINDEX 表会受到影响：SRCINDEX 的内容将被合并到 DSTINDEX 中。

如果 DSTINDEX 已经被 `searchd` 服务，则需要使用 `--rotate` 选项。

典型的使用模式是将较小的更新从 SRCINDEX 合并到 DSTINDEX 中。因此，在合并属性时，如果遇到重复的文档ID，SRCINDEX 中的值将优先使用。但是，请注意，在这种情况下，“旧”的关键字**不会**被自动删除。例如，如果 DSTINDEX 中文档123关联了关键字“old”，而 SRCINDEX 中关联了关键字“new”，合并后文档123将同时被这两个关键字找到。你可以提供一个显式条件来从 DSTINDEX 中删除文档以缓解此问题；相关的选项是 `--merge-dst-range`：

```bash
sudo -u manticore indexer --merge main delta --merge-dst-range deleted 0 0
```

此选项允许你在合并的同时对目标表应用过滤器。可以有多个过滤器；所有条件都必须满足，文档才会被包含在合并后的结果表中。在上面的示例中，过滤器只通过那些 'deleted' 为0的记录，排除了所有被标记为已删除的记录。

`--drop-src` 选项允许在合并后且旋转表之前删除 SRCINDEX，这在你将 DSTINDEX 指定为 DSTINDEX 的 `killlist_target` 时非常重要。否则，在旋转表时，已合并到 DSTINDEX 的文档可能会被 SRCINDEX 抑制。
<!-- proofread -->

