# 合并表

合并两个现有的**普通表**在某些情况下比从头开始索引数据更高效，可能是所需要的操作（例如，在"主+增量"分区方案中，合并“主”表和“增量”表，而不是简单地重建“主”表）。因此，`indexer` 提供了一个选项来实现这一点。合并表通常比重建速度更快，但对于巨大的表来说仍然**不是**瞬间完成的。它本质上需要一次读取两个表的内容并写入结果。例如，合并一个 100 GB 和 1 GB 的表将产生 202 GB 的 I/O（但这仍然比从头开始索引所需的要少）。

基本的命令语法如下：

```bash
sudo -u manticore indexer --merge DSTINDEX SRCINDEX [--rotate] [--drop-src]
```

除非指定了 `--drop-src`，否则只有目标表 DSTINDEX 会受到影响：SRCINDEX 的内容将被合并到其中。

如果 DSTINDEX 已由 `searchd` 服务，必须使用 `--rotate` 选项。

典型的用法模式是将 SRCINDEX 中较小的更新合并到 DSTINDEX 中。因此，在合并属性时，如果遇到重复的文档 ID，则优先使用 SRCINDEX 中的值。但请注意，在这种情况下，“旧”的关键词**不会**被自动移除。例如，如果在 DSTINDEX 中文档 123 关联了关键词“旧”，而在 SRCINDEX 中该文档关联了关键词“新”，合并后文档 123 将同时通过这两个关键词被找到。可以通过提供显式条件来删除 DSTINDEX 中的文档，以减轻这一问题；相关的选项是 `--merge-dst-range`：

```bash
sudo -u manticore indexer --merge main delta --merge-dst-range deleted 0 0
```

该选项允许在合并时对目标表应用过滤条件。可以使用多个过滤器，所有条件都必须满足才能将文档包含在合并后的结果表中。在上面的示例中，过滤器仅通过 'deleted' 为 0 的记录，从而删除了标记为删除的所有记录。

`--drop-src` 选项在合并后、旋转表之前删除 SRCINDEX，这在将 DSTINDEX 指定为 DSTINDEX 的 `killlist_target` 时非常重要。否则，在旋转表时，已经合并到 DSTINDEX 的文档可能会被 SRCINDEX 抑制。

<!-- proofread -->