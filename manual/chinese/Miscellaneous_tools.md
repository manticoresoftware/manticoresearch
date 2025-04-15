# 杂项工具

## indextool

`indextool` 是一个有用的工具，它提取有关物理表的各种信息，排除 `template` 或 `distributed` 表。以下是使用 `indextool` 的一般语法：

```sql
indextool <command> [options]
```

### 选项

这些选项适用于所有命令：

* `--config <file>` (`-c <file>` 的简写) 允许您覆盖默认的配置文件名称。
* `--quiet` (`-q` 的简写) 抑制 `indextool` 的横幅等输出。
* `--help` (`-h` 的简写) 显示您特定版本的 `indextool` 中可用的所有参数。
* `-v` 显示您特定 `indextool` 版本的信息。

### 命令

以下是可用的命令：

* `--checkconfig` 加载并验证配置文件，检查其有效性和任何语法错误。
* `--buildidf DICTFILE1 [DICTFILE2 ...] --out IDFILE` 从一个或多个字典转储构建 IDF 文件 (参见 `--dumpdict`)。附加参数 `--skip-uniq` 将省略唯一单词 (df=1)。
* `--build-infixes TABLENAME` 为一个已经存在的 dict=keywords 表生成内部词 (更新 .sph, .spi)。对于已经使用 dict=keywords 的旧表文件，使用此选项，可以更简单或更快速地用 indextool 更新表文件，而不是从头开始使用 indexer 重建它们。
* `--dumpheader FILENAME.sph` 迅速转储给定的表头文件，而不会干扰其他表文件甚至配置文件。报告提供所有表设置的详细视图，特别是完整属性和字段列表。
* `--dumpconfig FILENAME.sph` 从指定的表头文件中提取表定义，以 (几乎) 符合 manticore.conf 文件的格式。
* `--dumpheader TABLENAME` 根据表名转储表头，同时在配置文件中搜索头路径。
* `--dumpdict TABLENAME` 转储字典。额外的 `-stats` 开关将把总文档计数添加到字典转储中。这对于用于 IDF 文件创建的字典文件是必要的。
* `--dumpdocids TABLENAME` 根据表名转储文档 ID。
* `--dumphitlist TABLENAME KEYWORD` 在给定表中转储指定关键字的所有实例 (出现)，其中关键字被定义为文本。
* `--dumphitlist TABLENAME --wordid ID` 在给定表中转储特定关键字的所有实例 (出现)，其中关键字表示为内部数字 ID。
* `--docextract TBL DOCID` 执行整个字典/docs/hits 的标准表检查，通过请求的文档收集所有相关的词和命中。随后，所有词按其字段和位置排列，结果按字段分组打印。
* `--fold TABLENAME OPTFILE` 此选项有助于理解标记器如何处理输入。您可以为 indextool 提供文本，如果指定的话来自文件，否则来自标准输入。输出将用空格替换分隔符 (基于您的 `charset_table` 设置) 并将单词中的字母转换为小写。
* `--htmlstrip TABLENAME` 对指定表应用 HTML 剥离器设置以过滤标准输入，并将过滤结果发送到标准输出。请注意，设置将从 manticore.conf 中获取，而不是从表头中获取。
* `--mergeidf NODE1.idf [NODE2.idf ...] --out GLOBAL.idf` 将多个 .idf 文件合并为一个单一文件。额外参数 `--skip-uniq` 将忽略唯一单词 (df=1)。
* `--morph TABLENAME` 对给定标准输入应用形态学并将结果定向到标准输出。
* `--check TABLENAME` 评估表数据文件的一致性错误，这可能是由 `indexer` 中的错误或硬件故障造成的。`--check` 也适用于实时 (RT) 表、RAM 和磁盘块。附加选项：
    - `--check-id-dups` 评估重复 ID
    - `--check-disk-chunk CHUNK_NAME` 仅检查 RT 表的特定磁盘块。参数是要检查的 RT 表磁盘块的数字扩展。
* `--strip-path` 从表中提到的所有文件名中删除路径名称 (停用词、词形、例外等)。当验证在不同机器上构建的表时，这非常有用，因为可能有不同的路径布局。
* `--rotate` 仅与 `--check` 兼容，并确定是否检查等待轮换的表，即带有 .new 扩展名的表。当您希望在实际使用之前验证表时，这很有用。
* `--apply-killlists` 加载并应用配置文件中列出的所有表的杀死列表。更改保存在 .SPM 文件中。杀死列表文件 (.SPK) 被删除。如果您希望将表的应用从服务器启动阶段转移到索引阶段，则这可能很方便。

### 对于 RT 表检查的重要考虑事项

`indextool` 不能完全检查当前由守护进程提供服务的 RT 表。当尝试检查一个活动的 RT 表时，您可能会遇到以下警告：

```
警告：未能加载 RAM 块，仅检查 N 磁盘块
```

为避免这些警告并确保正确检查 RT 表，请考虑以下方法：

- 在运行 `indextool --check` 之前停止守护进程。  
- 确保 RT 表未由守护进程提供服务。  
- 检查 RT 表的单独副本，而不是实时的表。  

如果停止守护进程不是一个选项，您可以通过在运行 `indextool --check` 之前执行以下 MySQL 语句来防止对 RT 表的意外修改：

```sql
SET GLOBAL AUTO_OPTIMIZE=0;
```

此命令阻止守护进程执行自动优化，确保 RT 表文件保持不变。执行此语句后，等待优化线程完全停止，然后再继续执行 `indextool --check`。这确保在检查过程中不会意外修改或删除任何磁盘块。
如果之前启用了自动优化，检查完成后您应该手动重新启用它，方法是运行：

```sql
SET GLOBAL AUTO_OPTIMIZE=1;
```

## spelldump

`spelldump` 命令旨在从使用 `ispell` 或 `MySpell` 格式的字典文件中提取内容。当您需要编译词形的词表时，这很方便，因为它会为您生成所有可能的形式。

一般语法如下：

```bash
spelldump [options] <dictionary> <affix> [result] [locale-name]
```

主要参数是字典的主文件和附录文件。通常，这些文件命名为 `[language-prefix].dict` 和 `[language-prefix].aff`。您可以在大多数标准 Linux 发行版中找到这些文件，或者从众多在线资源中获取。

`[result]` 参数是提取的字典数据将存储的位置，`[locale-name]` 是用于指定您选择的区域详细信息的参数。

还有一个可选的 `-c [file]` 选项。此选项允许您为大小写转换详细信息指定一个文件。

以下是一些用法示例：

```bash
spelldump en.dict en.aff
spelldump ru.dict ru.aff ru.txt ru_RU.CP1251
spelldump ru.dict ru.aff ru.txt .1251
```

结果文件将列出字典中的所有单词，按字母顺序排列，格式类似于词形文件。然后，您可以根据自己的具体要求修改此文件。以下是输出文件可能的样本：

```bash
zone > zone
zoned > zoned
zoning > zoning
```

## wordbreaker

`wordbreaker` 工具旨在将复合词（在 URL 中常见）拆解为其单独组件。例如，它可以将 "lordoftherings" 拆解为四个单独的单词，或将 `http://manofsteel.warnerbros.com` 拆解为 "man of steel warner bros"。这种能力增强了搜索功能，消除了对前缀或中缀的需求。例如，搜索 "sphinx" 将不会在结果中获得 "sphinxsearch"。但是，如果您应用 `wordbreaker` 来拆解复合词并索引拆分的元素，搜索将会成功，而不会因在全文索引中使用前缀或中缀而导致文件大小的增加。

以下是如何使用 `wordbreaker` 的一些示例：

```bash
echo manofsteel | bin/wordbreaker -dict dict.txt split
man of steel
```

`-dict` 字典文件用于将输入流分隔为单独的单词。如果未指定字典文件，Wordbreaker 将在当前工作目录中查找名为 `wordbreaker-dict.txt` 的文件。（确保字典文件与您正在处理的复合词的语言匹配。）`split` 命令从标准输入分割单词，并将结果发送到标准输出。`test` 和 `bench` 命令也可用于评估拆分质量和测量拆分功能的性能。

Wordbreaker 使用字典来识别给定字符串中的单个子字符串。为了区分多个潜在的拆分，它考虑字典中每个单词的相对频率。频率越高，单词拆分的可能性就越高。要生成这种文件，您可以使用 `indexer` 工具：


```bash
indexer --buildstops dict.txt 100000 --buildfreqs myindex -c /path/to/manticore.conf
```

这将生成一个名为 `dict.txt` 的文本文件，包含 `myindex` 中最常出现的 100,000 个单词及其相应的计数。由于该输出文件是一个简单的文本文档，您可以在需要时灵活地手动编辑。可以根据需要随意添加或删除单词。

<!-- proofread -->
