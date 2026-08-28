# 合并表格

合并两个已有的**普通**表格可能比从头开始索引数据更高效，并且在在某些情况下可能更理想（例如合并 '主' 和 '增量' 表格，而不是简单地重建 '主' 在 '主+增量' 分区方案）。因此，`indexer` 提供了一个选项来实现这一点。合并表格通常比重建更快，但对于非常大的表格仍然**不是**瞬间完成。本质上，它需要读取两个表格的内容一次并写入结果一次。例如，合并一个 100 GB 和 1 GB 的表格，将导致 202 GB 的 I/O（但这仍然可能比从头开始索引所需的数据量更少）。

基本命令语法如下：

```bash
sudo -u manticore indexer --merge DSTINDEX SRCINDEX [--rotate] [--drop-src]
```

除非指定 `--drop-src`，否则只有 DSTINDEX 表格会受到影响：SRCINDEX 的内容将被合并到它。

如果 DSTINDEX 已经由 `searchd` 提供服务，则 `--rotate` 开关是必需的。

典型使用模式是将较小的更新从 SRCINDEX 合并到 DSTINDEX。因此，在合并属性时，如果遇到重复文档 ID，SRCINDEX 的值将优先。然而，请注意在这种情况下“旧”关键词**不会**自动被移除。例如，如果在 DSTINDEX 中有关文档 123 有关关键词“旧”，而 SRCINDEX 中有关文档 123 有关关键词“新”，合并后文档 123 将能被两个关键词找到。你可以提供一个明确条件从 DSTINDEX 移除文档来缓解这个问题；相关开关是 `--merge-dst-range`：

```bash
sudo -u manticore indexer --merge main delta --merge-dst-range deleted 0 0
```

这个开关允许你在在合并的同时对目标表格应用过滤器。可以有多个过滤器；所有条件都必须满足才能将文档包含在最终合并表格。在上例子中，过滤器只通过那些记录 'deleted' 为 0，从而消除所有被标记为删除的记录。

`--drop-src` 允许在合并后删除 SRCINDEX 并在在旋转表格之前，这在如果你在 DSTINDEX 的 `killlist_target` 指定 DSTINDEX 很重要。否则，在旋转表格时，已经合并到 DSTINDEX 的文档可能被 SRCINDEX 抑制。
<!-- proofread -->

