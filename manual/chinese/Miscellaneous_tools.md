# 杂项工具

## indextool

`indextool` 是一个有用的实用程序，可提取有关物理表的各种信息，但不包括 `template` 或 `distributed` 表。以下是使用 `indextool` 的一般语法：

```sql
indextool <command> [options]
```

### 选项

这些选项适用于所有命令：

* `--config <file>`（简写为 `-c <file>`）允许您覆盖默认的配置文件名称。
* `--quiet`（简写为 `-q`）抑制 `indextool` 输出的横幅等信息。
* `--help`（简写为 `-h`）显示您特定版本的 `indextool` 中可用的所有参数。
* `-v` 显示您特定 `indextool` 版本的信息。

### 命令

以下是可用命令：

* `--checkconfig` 加载并验证配置文件，检查其有效性及任何语法错误。
* `--buildidf DICTFILE1 [DICTFILE2 ...] --out IDFILE` 从一个或多个字典转储（参见 `--dumpdict`）构建IDF文件。附加参数 `--skip-uniq` 会忽略唯一词（df=1）。
* `--build-infixes TABLENAME` 为已存在的 dict=keywords 表生成中缀索引（就地更新 .sph, .spi 文件）。此选项适用于已经使用 dict=keywords 的传统表文件，但现需支持中缀搜索；使用 indextool 更新表文件可能比用 indexer 重新创建要简单或快速。
* `--dumpheader FILENAME.sph` 立即转储指定的表头文件，而不影响任何其他表文件或配置文件。报告详细展示所有表设置，特别是完整的属性和字段列表。
* `--dumpconfig FILENAME.sph` 从指定的表头文件中提取表定义，格式近似 manticore.conf 文件规范。
* `--dumpheader TABLENAME` 通过表名转储表头，同时在配置文件中查找表头路径。
* `--dumpdict TABLENAME` 转储字典。附加 `-stats` 开关将添加总文档数到字典转储中。这对于用于IDF文件创建的字典文件是必要的。
* `--dumpdocids TABLENAME` 按表名转储文档ID。
* `--dumphitlist TABLENAME KEYWORD` 转储指定表中指定关键词（文本定义）的所有实例（出现位置）。
* `--dumphitlist TABLENAME --wordid ID` 转储指定表中指定关键词（内部数值ID表示）的所有实例。
* `--dumpkilllist TABLENAME` 转储指定表的kill列表内容，包括目标和标志。它也可以用作 `--dumpkilllist FILENAME.spk`。
* `--docextract TBL DOCID` 执行对整个字典/文档/命中数据的标准表检查，并收集所请求文档关联的所有词和命中。随后，所有词按照字段和位置排序，并以字段分组打印结果。
* `--fold TABLENAME OPTFILE` 此选项有助于理解分词器的输入处理。您可以向 indextool 提供来自文件的文本（如果指定文件），否则来自标准输入。输出会基于您的 `charset_table` 设置，将分隔符替换为空格，并将词中字母转换为小写。
* `--htmlstrip TABLENAME` 应用指定表的HTML剥离设置过滤标准输入，并将过滤结果输出至标准输出。请注意，设置来自 manticore.conf，而非表头。
* `--mergeidf NODE1.idf [NODE2.idf ...] --out GLOBAL.idf` 合并多个 .idf 文件为单一文件。附加参数 `--skip-uniq` 将忽略唯一词（df=1）。
* `--morph TABLENAME` 对给定的标准输入应用形态学处理，结果输出到标准输出。
* `--check TABLENAME` 检查表数据文件的一致性错误，这些错误可能由 `indexer` 的缺陷或硬件故障引起。`--check` 也适用于RT表、RAM和磁盘块。附加选项：
    - `--check-id-dups` 检查普通表及RT表所有磁盘块中的重复文档ID。
    - `--check-disk-chunk CHUNK_NAME` 仅检查RT表的指定磁盘块。参数为RT表磁盘块的数字扩展名。
* `--strip-path` 删除表引用的所有文件名中的路径名（停用词、词形变化、例外等）。这在验证不同机器构建且路径布局可能不同的表时很有用。
* `--rotate` 仅与 `--check` 兼容，用于确定是否检查等待旋转的表，即带有 .new 扩展名的表。这在您希望验证表而不立即使用时非常有用。
* `--apply-killlists` 加载并应用配置文件中列出的所有表的停用词列表。更改保存至 .SPM 文件。停用词文件 (.SPK) 被移除。如果您想将表的应用从服务器启动阶段转移到索引阶段，这个选项很方便。

### 关于RT表检查的重要注意事项

`indextool` 不能完全检查当前由守护进程服务的RT表。尝试检查活动RT表时，可能会出现以下警告：

```
WARNING: failed to load RAM chunks, checking only N disk chunks
```

为避免这些警告并确保正确检查RT表，请考虑以下做法：

- 在运行 `indextool --check` 之前停止守护进程。
- 确保RT表未被守护进程活动服务。
- 检查RT表的一个副本，而不是实时表。

如果无法停止守护进程，您可以在运行 `indextool --check` 前执行以下MySQL语句，以防止对RT表的非预期修改：

```sql
SET GLOBAL AUTO_OPTIMIZE=0;
```

该命令阻止守护进程执行自动优化，确保RT表文件保持不变。执行该语句后，等待优化线程完全停止，然后再进行 `indextool --check`。这确保检查过程中不会无意中修改或删除磁盘块。
如果先前启用了自动优化，检查完成后应手动通过运行以下命令重新启用：

```sql
SET GLOBAL AUTO_OPTIMIZE=1;
```

## spelldump

`spelldump` 命令用于从采用 `ispell` 或 `MySpell` 格式的字典文件中提取内容。当你需要为词形生成词表时，这非常有用，因为它会为你生成所有可能的形式。

下面是一般语法：

```bash
spelldump [options] <dictionary> <affix> [result] [locale-name]
```

主要参数是字典的主文件和词缀文件。通常，它们分别命名为 `[language-prefix].dict` 和 `[language-prefix].aff`。你可以在大多数标准 Linux 发行版或许多在线资源中找到这些文件。

`[result]` 参数是存储提取的字典数据的位置，`[locale-name]` 是用来指定你选择的地区设置的参数。

还有一个可选的 `-c [file]` 选项。该选项允许你指定一个文件，用于大小写转换的详细信息。

以下是一些使用示例：

```bash
spelldump en.dict en.aff
spelldump ru.dict ru.aff ru.txt ru_RU.CP1251
spelldump ru.dict ru.aff ru.txt .1251
```

生成的文件将列出字典中的所有单词，按字母顺序排列，并格式化为类似词形文件。你可以根据具体需要修改该文件。下面是输出文件可能的示例：

```bash
zone > zone
zoned > zoned
zoning > zoning
```

## wordbreaker

`wordbreaker` 工具设计用于将复合词（URL 中常见的特征）拆解为各个组成部分。例如，它可以把 "lordoftherings" 分解为四个单词，或者将 `http://manofsteel.warnerbros.com` 拆分成 "man of steel warner bros"。这种能力提升了搜索功能，避免了使用前缀或中缀。例如，搜索 "sphinx" 不会返回 "sphinxsearch" 。但是，如果你应用 `wordbreaker` 来拆解复合词并对各部分建立索引，搜索将成功，无需为全文索引中的前缀或中缀使用增加文件大小。

以下是一些使用 `wordbreaker` 的示例：

```bash
echo manofsteel | bin/wordbreaker -dict dict.txt split
man of steel
```

`-dict` 字典文件用于将输入流拆分成单个单词。如果未指定字典文件，Wordbreaker 将在当前工作目录寻找名为 `wordbreaker-dict.txt` 的文件。（确保字典文件与所处理复合词的语言相匹配。）`split` 命令从标准输入拆分单词，并将结果发送到标准输出。`test` 和 `bench` 命令分别用于评估拆分质量和衡量拆分函数性能。

Wordbreaker 使用字典来识别给定字符串中的单独子串。为了区分多个潜在的拆分，它会考虑字典中每个词的相对频率。频率越高，拆分成该词的可能性越大。若要生成这种文件，你可以使用 `indexer` 工具：


```bash
indexer --buildstops dict.txt 100000 --buildfreqs myindex -c /path/to/manticore.conf
```

该命令会生成一个名为 `dict.txt` 的文本文件，其中包含 `myindex` 中出现频率最高的 100,000 个单词及其对应的计数。由于这个输出文件是一个简单的文本文件，你可以随时手动编辑它。根据需要，随意添加或删除单词。

<!-- proofread -->

