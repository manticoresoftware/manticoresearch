# 杂项工具

## indextool

`indextool` 是一个有用的工具，用于提取有关物理表的各种信息，但不包括 `template` 或 `distributed` 表。以下是使用 `indextool` 的一般语法：

```sql
indextool <command> [options]
```

### 选项

这些选项适用于所有命令：

* `--config <file>`（简写为 `-c <file>`）允许您覆盖默认的配置文件名。
* `--quiet`（简写为 `-q`）抑制 `indextool` 输出的横幅等信息。
* `--help`（简写为 `-h`）显示您特定版本的 `indextool` 中可用的所有参数。
* `-v` 显示您特定版本的 `indextool` 的版本信息。

### 命令

以下是可用的命令：

* `--checkconfig` 加载并验证配置文件，检查其有效性及语法错误。
* `--buildidf DICTFILE1 [DICTFILE2 ...] --out IDFILE` 从一个或多个字典转储文件（参见 `--dumpdict`）构建 IDF 文件。额外参数 `--skip-uniq` 会省略唯一词（df=1）。
* `--build-infixes TABLENAME` 为已有的 dict=keywords 表生成中缀（infixes）（就地更新 .sph、.spi 文件）。此选项适用于已经使用 dict=keywords 的旧表文件，但现在需要支持中缀搜索；使用 indextool 更新表文件可能比用 indexer 重新创建更简单或更快。
* `--dumpheader FILENAME.sph` 立即转储指定的表头文件，而不影响其他表文件或配置文件。报告详细显示所有表设置，特别是完整的属性和字段列表。
* `--dumpconfig FILENAME.sph` 以（几乎）符合 manticore.conf 文件格式的方式提取指定表头文件中的表定义。
* `--dumpheader TABLENAME` 通过表名转储表头，同时在配置文件中搜索表头路径。
* `--dumpdict TABLENAME` 转储字典。额外的 `-stats` 开关会在字典转储中添加总文档数。这对于用于 IDF 文件创建的字典文件是必要的。
* `--dumpdocids TABLENAME` 按表名转储文档 ID。
* `--dumphitlist TABLENAME KEYWORD` 转储指定表中指定关键词的所有实例（出现位置），关键词以文本形式定义。
* `--dumphitlist TABLENAME --wordid ID` 转储指定表中指定关键词的所有实例（出现位置），关键词以内部数字 ID 表示。
* `--dumpkilllist TABLENAME` 转储指定表的 kill list 内容，包括目标和标志。也可用作 `--dumpkilllist FILENAME.spk`。
* `--docextract TBL DOCID` 执行整个字典/文档/命中标准表检查，并收集请求文档相关的所有词和命中。随后，所有词按字段和位置排列，结果按字段分组打印。
* `--fold TABLENAME OPTFILE` 此选项帮助理解分词器如何处理输入。您可以向 indextool 提供来自文件的文本（如果指定了文件），否则从标准输入读取。输出将根据您的 `charset_table` 设置将分隔符替换为空格，并将词中的字母转换为小写。
* `--htmlstrip TABLENAME` 对指定表应用 HTML 去除器设置以过滤标准输入，并将过滤结果发送到标准输出。请注意，设置将从 manticore.conf 中获取，而非表头。
* `--mergeidf NODE1.idf [NODE2.idf ...] --out GLOBAL.idf` 将多个 .idf 文件合并为一个。额外参数 `--skip-uniq` 会忽略唯一词（df=1）。
* `--morph TABLENAME` 对给定的标准输入应用形态学处理，并将结果输出到标准输出。
* `--check TABLENAME` 检查表数据文件的一致性错误，这些错误可能由 `indexer` 的缺陷或硬件故障引起。`--check` 也适用于 RT 表、RAM 和磁盘块。附加选项：
    - `--check-id-dups` 检查普通表和 RT 表所有磁盘块中的重复文档 ID
    - `--check-disk-chunk CHUNK_NAME` 仅检查 RT 表的特定磁盘块。参数是要检查的 RT 表磁盘块的数字扩展名。
* `--strip-path` 从表引用的所有文件名中移除路径名（如停用词、词形变化、例外等）。这在验证在不同机器上构建且路径布局可能不同的表时很有用。
* `--rotate` 仅与 `--check` 兼容，决定是否检查等待旋转的表，即带有 .new 扩展名的表。这在您希望在实际使用表之前验证表时很有用。
* `--apply-killlists` 加载并应用配置文件中列出的所有表的 kill-list。更改保存到 .SPM 文件。kill-list 文件 (.SPK) 被删除。如果您想将表的应用从服务器启动阶段转移到索引阶段，这很方便。

### RT 表检查的重要注意事项

`indextool` 无法完全检查当前由守护进程服务的 RT 表。当尝试检查活动的 RT 表时，您可能会遇到以下警告：

```
WARNING: failed to load RAM chunks, checking only N disk chunks
```

为避免这些警告并确保正确检查 RT 表，请考虑以下方法：

- 在运行 `indextool --check` 之前停止守护进程。
- 确保 RT 表未被守护进程主动服务。
- 检查 RT 表的单独副本，而非实时表。

如果无法停止守护进程，您可以在运行 `indextool --check` 之前执行以下 MySQL 语句，以防止对 RT 表的意外修改：

```sql
SET GLOBAL AUTO_OPTIMIZE=0;
```

此命令阻止守护进程执行自动优化，确保 RT 表文件保持不变。执行此语句后，等待优化线程完全停止，然后再进行 `indextool --check`。这确保在检查过程中不会无意中修改或删除磁盘块。
如果之前启用了自动优化，检查完成后应手动重新启用它，方法是运行：

```sql
SET GLOBAL AUTO_OPTIMIZE=1;
```

## spelldump

`spelldump` 命令用于从采用 `ispell` 或 `MySpell` 格式的字典文件中提取内容。当您需要为词形编译单词列表时，这非常有用，因为它会为您生成所有可能的词形。

其一般语法如下：

```bash
spelldump [options] <dictionary> <affix> [result] [locale-name]
```

主要参数是字典的主文件和词缀文件。通常，这些文件分别命名为 `[language-prefix].dict` 和 `[language-prefix].aff`。您可以在大多数标准 Linux 发行版或许多在线资源中找到这些文件。

`[result]` 参数是提取的字典数据将被存储的位置，`[locale-name]` 是用于指定您选择的区域设置的参数。

还有一个可选的 `-c [file]` 选项。此选项允许您指定一个用于大小写转换细节的文件。

以下是一些使用示例：

```bash
spelldump en.dict en.aff
spelldump ru.dict ru.aff ru.txt ru_RU.CP1251
spelldump ru.dict ru.aff ru.txt .1251
```

生成的文件将列出字典中的所有单词，按字母顺序排列，并格式化为类似词形文件的形式。然后，您可以根据具体需求修改此文件。以下是输出文件可能的示例：

```bash
zone > zone
zoned > zoned
zoning > zoning
```

## wordbreaker

`wordbreaker` 工具用于拆解复合词，这在 URL 中很常见，将其分解为单独的组成部分。例如，它可以将 "lordoftherings" 拆分为四个单词，或将 `http://manofsteel.warnerbros.com` 拆分为 "man of steel warner bros"。这种能力增强了搜索功能，无需使用前缀或中缀。例如，搜索 "sphinx" 不会在结果中出现 "sphinxsearch"。但是，如果您应用 `wordbreaker` 拆解复合词并索引拆分后的元素，搜索将成功，且不会像全文索引中使用前缀或中缀那样导致文件大小膨胀。

以下是一些使用 `wordbreaker` 的示例：

```bash
echo manofsteel | bin/wordbreaker -dict dict.txt split
man of steel
```

`-dict` 字典文件用于将输入流分割成单个单词。如果未指定字典文件，Wordbreaker 会在当前工作目录中查找名为 `wordbreaker-dict.txt` 的文件。（确保字典文件与您处理的复合词的语言匹配。）`split` 命令从标准输入拆分单词并将结果发送到标准输出。`test` 和 `bench` 命令分别用于评估拆分质量和测量拆分函数的性能。

Wordbreaker 使用字典来识别给定字符串中的单个子串。为了区分多种可能的拆分，它会考虑字典中每个单词的相对频率。频率越高，单词拆分的可能性越大。要生成此类文件，您可以使用 `indexer` 工具：


```bash
indexer --buildstops dict.txt 100000 --buildfreqs myindex -c /path/to/manticore.conf
```

它将生成一个名为 `dict.txt` 的文本文件，包含 `myindex` 中出现频率最高的 100,000 个单词及其各自的计数。由于此输出文件是简单的文本文件，您可以随时手动编辑它。根据需要随意添加或删除单词。

<!-- proofread -->

