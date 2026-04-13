# 杂项工具

## indextool

`indextool` 是一个有用的工具，用于提取物理表的各种信息，不包括 `template` 或 `distributed` 表。以下是使用 `indextool` 的通用语法：

```sql
indextool <command> [options]
```

### 选项

这些选项适用于所有命令：

* `--config <file>`（简写为 `-c <file>`）允许覆盖默认的配置文件名。
* `--quiet`（简写为 `-q`）抑制 `indextool` 的横幅输出等信息。
* `--help`（简写为 `-h`）显示你的 `indextool` 特定构建版本中可用的所有参数。
* `-v` 显示你的 `indextool` 特定构建版本的版本信息。

### 命令

以下是可用的命令：

* `--checkconfig` 加载并验证配置文件，检查其有效性及语法错误。
* `--buildidf DICTFILE1 [DICTFILE2 ...] --out IDFILE` 从一个或多个字典转储文件（参见 `--dumpdict`）构建 IDF 文件。额外参数 `--skip-uniq` 会省略唯一词（df=1）。
* `--build-infixes TABLENAME` 为已存在的 dict=keywords 表生成中缀（原地更新 .sph, .spi 文件）。适用于已使用 dict=keywords 的传统表文件，但现在需要支持中缀搜索；使用 indextool 更新表文件可能比用 indexer 重新创建更简单或更快。
* `--dumpheader FILENAME.sph` 立即转储指定的表头文件，而不会干扰任何其他表文件或配置文件。报告详细显示所有表设置，尤其是完整的属性和字段列表。
* `--dumpconfig FILENAME.sph` 以几乎符合 manticore.conf 文件格式的格式，提取指定表头文件的表定义。
* `--dumpheader TABLENAME` 按表名转储表头，同时在配置文件中查找表头路径。
* `--dumpdict TABLENAME` 转储字典。额外的 `-stats` 开关会向字典转储中添加文档总数。这对于用于 IDF 文件创建的字典文件是必需的。
* `--dumpdocids TABLENAME` 按表名转储文档 ID。
* `--dumphitlist TABLENAME KEYWORD` 转储指定表中某关键词的所有实例（出现次数），关键词以文本定义。
* `--dumphitlist TABLENAME --wordid ID` 转储指定表中某关键词的所有实例（出现次数），关键词以内部数字 ID 表示。
* `--dumpkilllist TABLENAME` 转储指定表的 kill list 内容，包括目标和标记。也可用作 `--dumpkilllist FILENAME.spk`。
* `--docextract TBL DOCID` 执行整个字典/文档/命中标准表检查，收集请求文档相关的所有词和命中，然后将所有词按字段和位置排序，最终按字段分组打印结果。
* `--fold TABLENAME OPTFILE` 该选项帮助理解分词器如何处理输入。你可以向 indextool 提供来自指定文件的文本（若指定）或默认从 stdin 读取。输出将根据 `charset_table` 设置，将分隔符替换为空格，并将单词中字母转换为小写。
* `--htmlstrip TABLENAME` 使用指定表的 HTML 去除设置过滤 stdin，并将结果输出到 stdout。注意设置将从 manticore.conf 中获取，而非表头。
* `--mergeidf NODE1.idf [NODE2.idf ...] --out GLOBAL.idf` 将多个 .idf 文件合并成一个。额外参数 `--skip-uniq` 会忽略唯一词（df=1）。
* `--morph TABLENAME` 对给定的 stdin 应用形态学处理，并将结果输出到 stdout。
* `--check TABLENAME` 检查表数据文件是否存在由 `indexer` 错误或硬件故障导致的一致性错误。`--check` 也适用于 RT 表、RAM 和磁盘块。附加选项：
    - `--check-id-dups` 检查普通表和 RT 表所有磁盘块中的重复文档 ID。
    - `--check-disk-chunk CHUNK_NAME` 仅检查 RT 表的指定磁盘块。参数是 RT 表磁盘块的数字扩展名。
* `--strip-path` 移除表中引用的所有文件名（停词、词形变化、例外等）中的路径名。当验证不同机器上可能路径结构不同的表时非常有用。
* `--rotate` 仅兼容 `--check`，用于决定是否检查等待旋转的表，即带有 .new 扩展名的表。这在你希望在实际使用之前验证表时非常有用。
* `--apply-killlists` 加载并应用配置文件中列出的所有表的 kill-lists。更改保存到 .SPM 文件，kill-list 文件 (.SPK) 被删除。如果你想将 kill-list 的应用从服务器启动阶段转移到索引阶段，这项功能会很方便。

### RT 表检查的重要注意事项

`indextool` 无法完全检查当前由守护进程服务的 RT 表。尝试检查活跃的 RT 表时，可能会遇到以下警告：

```
WARNING: failed to load RAM chunks, checking only N disk chunks
```

为避免这些警告并确保正确检查 RT 表，请考虑以下方法：

- 在运行 `indextool --check` 前停止守护进程。
- 确保 RT 表当前未被守护进程服务。
- 检查 RT 表的独立副本，而非在线表。

如果无法停止守护进程，可以在运行 `indextool --check` 之前执行以下 MySQL 语句，防止对 RT 表发生意外修改：

```sql
SET GLOBAL AUTO_OPTIMIZE=0;
```

此命令防止守护进程执行自动优化，确保RT表文件保持不变。执行此语句后，请等待优化线程完全停止，然后再运行 `indextool --check`。这确保了在检查过程中不会意外修改或删除任何磁盘块。
如果之前启用了自动优化，在检查完成后应手动重新启用它，运行：

```sql
SET GLOBAL AUTO_OPTIMIZE=1;
```

## spelldump

`spelldump` 命令用于从使用 `ispell` 或 `MySpell` 格式的字典文件中检索内容。当您需要为词形变化编译单词列表时，这非常有用，因为它会为您生成所有可能的形式。

以下是通用语法：

```bash
spelldump [options] <dictionary> <affix> [result] [locale-name]
```

主要参数是字典的主要文件和附录文件。通常，这些文件分别命名为 `[language-prefix].dict` 和 `[language-prefix].aff`。您可以在大多数标准 Linux 发行版或许多在线资源中找到这些文件。

`[result]` 参数是提取的字典数据将被存储的位置，`[locale-name]` 参数用于指定您选择的区域详细信息。

还有一个可选的 `-c [file]` 选项。此选项允许您指定用于大小写转换的文件。

这里有一些使用示例：

```bash
spelldump en.dict en.aff
spelldump ru.dict ru.aff ru.txt ru_RU.CP1251
spelldump ru.dict ru.aff ru.txt .1251
```

生成的文件将列出字典中的所有单词，按字母顺序排列并格式化为词形变化文件。然后您可以根据具体需求修改此文件。以下是一个输出文件可能的示例：

```bash
zone > zone
zoned > zoned
zoning > zoning
```

## wordbreaker

`wordbreaker` 工具旨在分解复合词，这是 URL 中的常见特征，将其分解为单独的组件。例如，它可以将 "lordoftherings" 分解为四个单独的词，或将 `http://manofsteel.warnerbros.com` 分解为 "man of steel warner bros"。这种能力通过消除前缀或中缀的需求来增强搜索功能。例如，搜索 "sphinx" 将不会返回 "sphinxsearch" 的结果。然而，如果应用 `wordbreaker` 来拆分复合词并索引分离的元素，搜索将成功，而不会像全文索引中使用前缀或中缀时那样增加文件大小。

这里有一些如何使用 `wordbreaker` 的示例：

```bash
echo manofsteel | bin/wordbreaker -dict dict.txt split
man of steel
```

`-dict` 字典文件用于将输入流分解为单独的词。如果没有指定字典文件，Wordbreaker 将在当前工作目录中查找名为 `wordbreaker-dict.txt` 的文件。（请确保字典文件与您正在处理的复合词的语言匹配。）`split` 命令从标准输入中拆分单词并将结果发送到标准输出。`test` 和 `bench` 命令也用于评估拆分质量并测量拆分功能的性能。

Wordbreaker 使用字典来识别给定字符串中的各个子字符串。为了区分多个潜在的拆分，它会考虑每个词在字典中的相对频率。频率越高，表示该词被拆分的可能性越大。要生成此类文件，可以使用 `indexer` 工具：


```bash
indexer --buildstops dict.txt 100000 --buildfreqs myindex -c /path/to/manticore.conf
```

这将生成一个名为 `dict.txt` 的文本文件，其中包含 `myindex` 中最常见的 100,000 个词及其各自的计数。由于此输出文件是一个简单的文本文件，您可以根据需要对其进行手动编辑。可以添加或删除词。

<!-- proofread -->

