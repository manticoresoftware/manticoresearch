# 合并表格

合并两个现有的 **普通** 表可能比从头开始索引数据更高效，并且在某些情况下可能是所需的（例如，在 'main+delta' 分区方案中合并 'main' 和 'delta' 表，而不是简单地重建 'main'）。因此，`indexer` 提供了一个选项来做到这一点。合并表格通常比重建更快，但对于大型表格来说依然 **不是** 立即完成的。基本上，它需要读取两个表的内容一次，并写入结果一次。例如，合并一个 100 GB 和一个 1 GB 的表，将导致 202 GB 的 I/O（但这仍然可能比从头开始索引所需的少）。

基本命令语法如下：

```bash
sudo -u manticore indexer --merge DSTINDEX SRCINDEX [--rotate] [--drop-src]
```

除非指定 `--drop-src`，否则只有 DSTINDEX 表会受到影响：SRCINDEX 的内容将合并到其中。

如果 DSTINDEX 已经被 `searchd` 提供服务，则需要 `--rotate` 开关。

典型的使用模式是将来自 SRCINDEX 的较小更新合并到 DSTINDEX。因此，当合并属性时，如果遇到重复的文档 ID，SRCINDEX 中的值将优先考虑。但是，请注意，在这种情况下，“旧”关键词将 **不会** 被自动移除。例如，如果在 DSTINDEX 中有一个与文档 123 相关联的关键词“旧”，以及在 SRCINDEX 中与其相关联的关键词“新”，则在合并后，可以通过这两个关键词找到文档 123。您可以提供一个明确的条件以从 DSTINDEX 中移除文档，以缓解这种情况；相关的开关是 `--merge-dst-range`：

```bash
sudo -u manticore indexer --merge main delta --merge-dst-range deleted 0 0
```

此开关允许您在合并的同时对目标表应用过滤器。可以有多个过滤器；必须满足所有条件才能将文档包含在结果合并表中。在上面的示例中，过滤器仅通过那些'已删除'为 0 的记录，消除了所有被标记为已删除的记录。

`--drop-src` 在合并和旋转表之前启用删除 SRCINDEX，这在您在 DSTINDEX 的 `killlist_target` 中指定 DSTINDEX 时非常重要。否则，在旋转表时，已合并到 DSTINDEX 的文档可能会被 SRCINDEX 抑制。
<!-- proofread -->
