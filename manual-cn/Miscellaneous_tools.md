# 杂项工具

## indextool

`indextool` 是一个有用的工具，它可以提取物理表（不包括 template 或 distributed 表）的各种信息。以下是使用 `indextool` 的通用语法：

```sql
indextool <command> [options]
```

### 选项

这些选项适用于所有命令：

* `--config <file>` (简写为`-c <file>` ) 允许覆盖默认的配置文件名。
* `--quiet` (简写为`-q` ) 抑制 `indextool` 的横幅和其他输出信息。
* `--help` (简写为`-h` ) 显示您当前构建的 `indextool` 可用的所有参数。
* `-v` 显示您当前 indextool 构建的版本信息。

### 命令

以下是可用的命令：

* `--checkconfig` 加载并验证配置文件，检查其有效性和语法错误。
* `--buildidf DICTFILE1 [DICTFILE2 ...] --out IDFILE` 从一个或多个字典转储文件构建 IDF 文件（参见 --dumpdict）。附加参数 --skip-uniq 会忽略唯一词（df=1）。
* `--build-infixes TABLENAME` 为现有的 dict=keywords 表生成前缀（更新 .sph, .spi 文件）。对于已经使用 dict=keywords 的旧表文件但需要前缀搜索支持的情况，使用 indextool 更新表文件比从头开始重新创建它们更简单或更快速。
* `--dumpheader FILENAME.sph` 快速转储给定表头文件而不影响其他表文件或配置文件。该报告提供了所有表设置的详细视图，特别是完整的属性和字段列表。
* `--dumpconfig FILENAME.sph` 从指定的表头文件中提取表定义，几乎与 manticore.conf 文件兼容。
* `--dumpheader TABLENAME` 按表名转储表头信息，并在配置文件中查找表头路径。
* `--dumpdict TABLENAME` 转储字典。附加的 -stats 开关会将总文档数添加到字典转储中。这对于 IDF 文件创建中使用的字典文件是必要的。
* `--dumpdocids TABLENAME` 按表名转储文档 ID。
* `--dumphitlist TABLENAME KEYWORD` 转储给定表中指定关键字的所有实例（出现次数），关键字是以文本形式定义的。
* `--dumphitlist TABLENAME --wordid ID` 以内部数字 ID 形式转储给定表中指定关键字的所有实例（出现次数）。
* `--docextract TBL DOCID` 执行标准的表检查遍历整个字典/文档/命中列表，并收集与请求的文档相关的所有单词和命中列表。然后，根据字段和位置对所有单词进行排序，结果按字段分组输出。
* `--fold TABLENAME OPTFILE` 该选项有助于理解分词器如何处理输入。您可以向 indextool 提供文本文件（如果指定），否则从标准输入读取。输出将用空格替换分隔符（根据您的 charset_table 设置），并将单词中的字母转换为小写。
* `--htmlstrip TABLENAME` 应用指定表的 HTML 剥离设置以过滤标准输入，并将过滤结果输出到标准输出。请注意，设置将从 manticore.conf 中获取，而不是从表头文件中获取。
* `--mergeidf NODE1.idf [NODE2.idf ...] --out GLOBAL.idf` 将多个 .idf 文件合并为一个。附加参数 --skip-uniq 会忽略唯一词（df=1）。
* `--morph TABLENAME` 将词法应用于给定的标准输入，并将结果输出到标准输出。
* `--check TABLENAME` 检查表数据文件的一致性错误，这些错误可能是由 `indexer` 中的错误或硬件故障引起的。--check 也适用于 RT 表、RAM 和磁盘块。附加选项包括：
    - `--check-id-dups` 检查是否有重复的 ID。
    - `--check-disk-chunk CHUNK_NAME` 仅检查 RT 表的特定磁盘块。参数是要检查的 RT 表磁盘块的数字扩展名。
* `--strip-path` 从表中引用的所有文件名（停用词、词形变化、例外等）中移除路径名。这在验证在不同机器上构建的表时很有用，因为这些机器的路径布局可能不同。
* `--rotate` 仅与 `--check` 兼容，并决定是否检查等待轮换的表，即带有 .new 扩展名的表。当您希望在实际使用表之前验证表时，此功能非常有用。
* `--apply-killlists` 加载并应用配置文件中列出的所有表的删除列表。更改会保存到 .SPM 文件中。删除列表文件 (.SPK) 会被移除。如果您希望将表的应用从服务器启动阶段移到索引阶段，这个功能非常有用。

## spelldump

`spelldump` 命令用于从 `ispell` 或 `MySpell` 格式的字典文件中提取内容。当您需要为词形变化编译词表时，这个工具非常有用，因为它会为您生成所有可能的形式。



通用语法如下：

```bash
spelldump [options] <dictionary> <affix> [result] [locale-name]
```

主要参数是字典的主文件和后缀文件。通常，它们命名为`[language-prefix].dict` 和 `[language-prefix].aff`。您可以在大多数标准 Linux 发行版中找到这些文件，也可以从许多在线资源中下载。

`[result]` 参数是存储提取的字典数据的位置，而 [locale-name] 参数用于指定您选择的区域设置。

可选的 `-c [file]` 选项允许您指定一个文件来进行大小写转换。

使用示例如下：

```bash
spelldump en.dict en.aff
spelldump ru.dict ru.aff ru.txt ru_RU.CP1251
spelldump ru.dict ru.aff ru.txt .1251
```

结果文件将按字母顺序列出字典中的所有单词，并以词形变化文件的格式排列。然后，您可以根据具体需求修改此文件。以下是输出文件的示例：

```bash
zone > zone
zoned > zoned
zoning > zoning
```

## wordbreaker

`wordbreaker` 工具用于将复合词（如 URL 中的词）分解为它们的单独组成部分。例如，它可以将 “lordoftherings” 分解为四个单词，或者将 `http://manofsteel.warnerbros.com` 分解为 “man of steel warner bros”。这提高了搜索功能的精度，消除了使用前缀或后缀的必要性。举例来说，搜索 “sphinx” 可能无法得到 “sphinxsearch” 作为结果，但如果使用 `wordbreaker` 分解复合词并对分解后的元素进行索引，那么即使不使用全文本索引的前缀或后缀，搜索也会成功。

以下是 `wordbreaker` 的一些使用示例：

```bash
echo manofsteel | bin/wordbreaker -dict dict.txt split
man of steel
```

Wordbreaker 使用字典识别给定字符串中的单独子字符串。为了区分多种可能的拆分，它会根据字典中每个单词的相对频率进行判断。频率越高，拆分为该单词的可能性越大。要生成这种文件，您可以使用 `indexer` 工具：


```bash
indexer --buildstops dict.txt 100000 --buildfreqs myindex -c /path/to/manticore.conf
```

这将生成一个名为 `dict.txt` 的文本文件，其中包含 `myindex` 中最常出现的 100,000 个单词及其相应的计数。由于这个输出文件是一个简单的文本文档，您可以根据需要手动编辑它。随时添加或删除单词。

<!-- proofread -->
