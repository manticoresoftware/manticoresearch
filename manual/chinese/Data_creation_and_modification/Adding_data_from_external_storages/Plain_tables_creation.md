# 纯表创建

纯表是通过在创建时从一个或多个源获取数据一次性创建的表。纯表是不可变的，在其生命周期内无法添加或删除文档。只能更新数值属性（包括多值属性 MVA）的值。刷新数据只能通过重新创建整个表来实现。

纯表仅在[纯模式](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29)中可用，其定义由表声明和一个或多个源声明组成。数据收集和表创建不是由`searchd`服务器完成，而是由辅助工具`indexer`完成。

**Indexer**是一个命令行工具，可以直接从命令行或shell脚本中调用。

调用时可以接受多个参数，此外，它在Manticore配置文件中还有自己的若干设置。

在典型场景中，indexer执行以下操作：
* 从源获取数据
* 构建纯表
* 写入表文件
* （可选）通知搜索服务器有关新表的信息，从而触发表轮换

## Indexer工具
`indexer`工具用于在Manticore Search中创建纯表。其通用语法为：

```shell
indexer [OPTIONS] [table_name1 [table_name2 [...]]]
```

使用`indexer`创建表时，生成的表文件必须具有允许`searchd`读取、写入和删除的权限。在官方Linux软件包中，`searchd`以`manticore`用户身份运行。因此，`indexer`也必须以`manticore`用户身份运行：

```shell
sudo -u manticore indexer ...
```

如果您以不同方式运行`searchd`，则可能需要省略`sudo -u manticore`。只需确保运行`searchd`实例的用户对使用`indexer`生成的表具有读写权限。

要创建纯表，需要列出要处理的表。例如，如果您的`manticore.conf`文件包含两个表`mybigindex`和`mysmallindex`的详细信息，您可以运行：

```shell
sudo -u manticore indexer mysmallindex mybigindex
```

您还可以使用通配符来匹配表名：

* `?`匹配任何单个字符
* `*`匹配任意数量的任意字符
* `%`匹配无字符或任意单个字符

```shell
sudo -u manticore indexer indexpart*main --rotate
```

indexer的退出代码如下：

* 0：一切正常
* 1：索引过程中出现问题（如果指定了`--rotate`，则跳过），或者操作发出警告
* 2：索引成功，但`--rotate`尝试失败

### Indexer systemd服务

您还可以使用以下systemctl单元文件启动`indexer`：

```shell
systemctl start --no-block manticore-indexer
```

或者，如果您想构建特定表：

```shell
systemctl start --no-block manticore-indexer@specific-table-name
```

使用`systemctl set-environment INDEXER_CONFIG`命令可以使用自定义配置运行Indexer，替代默认设置。

`systemctl set-environment INDEXER_ARGS`命令允许您添加Indexer的自定义启动选项。有关命令行选项的完整列表，请参见[这里](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments)。

例如，要以静默模式启动Indexer，运行：
```bash
systemctl set-environment INDEXER_ARGS='--quiet'
systemctl restart manticore-indexer
```

要恢复更改，运行：
```bash
systemctl set-environment INDEXER_ARGS=''
systemctl restart manticore-indexer
```

### Indexer命令行参数
* `--config <file>`（简写为`-c <file>`）告诉`indexer`使用指定的文件作为其配置。通常，它会查找安装目录中的`manticore.conf`（例如`/etc/manticoresearch/manticore.conf`），然后是调用`indexer`时所在的当前目录。此选项在共享环境中特别有用，例如二进制文件安装在全局文件夹（如`/usr/bin/`）中，但您希望用户能够进行自定义Manticore配置，或者希望在单台服务器上运行多个实例。在这种情况下，您可以允许他们创建自己的`manticore.conf`文件，并使用此选项传递给`indexer`。例如：

  ```shell
  sudo -u manticore indexer --config /home/myuser/manticore.conf mytable
  ```

* `--all`告诉`indexer`更新`manticore.conf`中列出的所有表，而非指定单个表。这在小型配置或周期性维护任务（如每天或每周重建整个表集）中非常实用。请注意，由于`--all`尝试更新配置中的所有表，如果遇到实时表（RealTime tables），会发出警告，且命令的退出码为`1`，即使纯表没有问题。用法示例：

  ```shell
  sudo -u manticore indexer --config /home/myuser/manticore.conf --all
  ```

* `--rotate`用于表轮换。除非您能将搜索功能下线而不影响用户，否则几乎肯定需要在索引新文档时保持搜索服务运行。`--rotate`创建一个与原表平行的第二个表（在同一路径下，只是文件名中包含`.new`）。完成后，`indexer`通过发送`SIGHUP`信号通知`searchd`，`searchd`将尝试重命名表（将现有表重命名为带`.old`后缀，将`.new`表重命名替换它们），然后开始从新文件提供服务。根据[seamless_rotate](../../Server_settings/Searchd.md#seamless_rotate)的设置，可能会有短暂的延迟才能搜索更新的表。如果一次轮换多个表，而它们通过[killlist_target](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target)链式关联，则轮换将从未被指定为目标的表开始，最后轮换链条末端的表。用法示例：

  ```shell
  sudo -u manticore indexer --rotate --all
  ```
* `--quiet`告诉`indexer`除非发生错误，否则不输出任何内容。此选项多用于类似cron的定时任务或其他脚本任务，其中输出无关紧要或不必要，除非出现错误。用法示例：

  ```shell
  sudo -u manticore indexer --rotate --all --quiet
  ```
* `--noprogress` 不显示进度细节。相反，只有在索引完成时才会报告最终状态详情（例如，已索引的文档数量、索引速度等）。在脚本未在控制台（或“tty”）上运行的情况下，此选项将默认启用。示例用法：

  ```shell
  sudo -u manticore indexer --rotate --all --noprogress
  ```
* `--buildstops <outputfile.text> <N>` 会审查表源，就像正在对数据进行索引一样，并生成正在被索引的术语列表。换句话说，它生成所有成为表一部分的可搜索术语列表。注意，它不会更新表本身，只是像索引那样处理数据，包括运行通过[sql_query_pre](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Execution_of_fetch_queries.md#sql_query_pre)或[sql_query_post](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Execution_of_fetch_queries.md#sql_query_post)定义的查询。`outputfile.txt` 将包含按频率排序的一系列单词列表，最频繁出现的单词排在前面，`N` 指定要列出的最大单词数。如果足够大以涵盖表中的每个单词，则只会返回这么多单词。这样的词典列表可以用于客户端应用程序中的“您是否是指…”功能，通常与 `--buildfreqs` 结合使用。示例：

  ```shell
  sudo -u manticore indexer mytable --buildstops word_freq.txt 1000
  ```

  这将生成当前目录中的一个名为 `word_freq.txt` 的文件，其中包含 `mytable` 中最常见的 1,000 个单词，按最常见顺序排列。请注意，该文件将针对指定多个表或 `--all` 时最后索引的表相关（即配置文件中列出的最后一个表）

* `--buildfreqs` 与 `--buildstops` 共同使用（如果没有指定 `--buildstops`，则会被忽略）。`--buildstops` 提供了表中使用的单词列表，而 `--buildfreqs` 添加了这些单词在表中的数量，这对于确定某些单词是否应被视为停用词（如果它们过于普遍）非常有用。它还将帮助开发“您是否是指…”功能，其中需要知道一个给定单词比另一个相似单词更常见的程度。例如：

  ```shell
  sudo -u manticore indexer mytable --buildstops word_freq.txt 1000 --buildfreqs
  ```

  这将生成上述的 `word_freq.txt` 文件，但在每个单词后面还会列出它在问题表中出现的次数。

* `--merge <dst-table> <src-table>` 用于物理合并表，例如，如果您有一个 [主+增量方案](../../Creating_a_table/Local_tables/Plain_table.md#Main+delta-scenario)，其中主表很少更改，但增量表经常重建，那么 `--merge` 将用于结合两者。操作从右到左进行 - `src-table` 的内容将被检查并与 `dst-table` 的内容物理合并，结果将保留在 `dst-table` 中。伪代码表示为：`dst-table += src-table` 示例：

  ```shell
  sudo -u manticore indexer --merge main delta --rotate
  ```

  在上述示例中，其中主表是很少修改的主表，而增量表是更常修改的一个，您可以使用上述命令调用 `indexer` 来将增量表的内容合并到主表中并旋转表。

* `--merge-dst-range <attr> <min> <max>` 在合并时应用给定的过滤范围。具体来说，当合并应用于目标表（作为 `--merge` 的一部分，如果未指定 `--merge` 则忽略），`indexer` 也会过滤最终进入目标表的文档，只有符合给定过滤条件的文档才会进入最终表。例如，在一个具有 'deleted' 属性的表中，值为 0 表示“未删除”。此类表可以与以下内容合并：

  ```shell
  sudo -u manticore indexer --merge main delta --merge-dst-range deleted 0 0
  ```

  标记为已删除（值为 1）的任何文档都将从新合并的目标表中移除。可以通过多次添加到命令行来添加多个过滤器，所有这些过滤器都必须同时满足，文档才能成为最终表的一部分。

* --`merge-killlists`（及其较短别名 `--merge-klists`）改变了在合并表时处理杀列表的方式。默认情况下，合并后会丢弃两个杀列表。然而，启用此选项后，两个表的杀列表将被连接并存储到目标表中。请注意，源（增量）表的杀列表将始终用于抑制来自目标（主）表的行。
* `--keep-attrs` 允许在重新索引时重用现有属性。每当表被重建时，每个新文档 ID 都会检查是否存在旧表中，如果存在，则其属性将转移到新表；如果不存在，则使用新表中的属性。如果用户更新了表中的属性，但未更新实际用于表的数据源，则在重新索引时所有更新都会丢失；使用 `--keep-attrs` 可以保存之前表中的更新属性值。还可以指定用于表文件的路径，而不是从配置文件中引用的路径：

  ```shell
  sudo -u manticore indexer mytable --keep-attrs=/path/to/index/files
  ```

* `--keep-attrs-names=<attributes list>` 允许您指定在重新索引时要重用的现有表中的属性列表。默认情况下，所有现有表中的属性都会在新表中重用：

  ```shell
  sudo -u manticore indexer mytable --keep-attrs=/path/to/table/files --keep-attrs-names=update,state
  ```

* `--dump-rows <FILE>` 将由 SQL 源获取的行以 MySQL 兼容的语法转储到指定文件中。生成的转储是 `indexer` 接收的数据的精确表示，有助于重现索引时间的问题。该命令从源执行获取操作，并创建表文件和转储文件。
* `--print-rt <rt_index> <table>` 以 INSERT 语句的形式输出从源获取的实时表数据。转储的首几行将包含实时字段和属性（作为普通表字段和属性的反映）。该命令从源执行获取操作，并创建表文件和转储输出。命令示例如 `sudo -u manticore indexer -c manticore.conf --print-rt indexrt indexplain > dump.sql`。仅支持基于 SQL 的源。不支持 MVA。
* `--sighup-each` 在重建多个大型表且希望尽快将每个表轮转至 `searchd` 时非常有用。启用 `--sighup-each` 时，`indexer` 在成功完成每个表的工作后会发送 SIGHUP 信号给 searchd。（默认行为是在所有表构建完毕后发送一个 SIGHUP）。
* `--nohup` 在你想在实际轮转前先用 indextool 检查表时很有用。如果启用此选项，indexer 不会发送 SIGHUP。表文件被重命名为 .tmp。使用 indextool 将表文件重命名为 .new 并进行轮转。示例用法：

  ```shell
  sudo -u manticore indexer --rotate --nohup mytable
  sudo -u manticore indextool --rotate --check mytable
  ```

* `--print-queries` 打印 `indexer` 发送给数据库的 SQL 查询及 SQL 连接和断开事件。这对诊断和修复 SQL 源的相关问题很有帮助。
* `--help`（简写为 `-h`）列出所有可以由 `indexer` 调用的参数。
* `-v` 显示 `indexer` 版本。

### Indexer 配置设置
你也可以在 Manticore 配置文件的 `indexer` 部分配置 indexer 的行为：

```ini
indexer {
...
}
```

#### lemmatizer_cache

```ini
lemmatizer_cache = 256M
```
词形还原缓存大小。可选，默认是 256K。

我们的[词形还原器](../../Server_settings/Common.md#lemmatizer_base)实现使用了压缩字典格式，支持空间和速度之间的权衡。它可以直接在压缩数据上执行词形还原，用更多的 CPU 但更少的内存，或者可以解压字典并部分或全部预缓存，从而使用更少的 CPU 但更多的内存。lemmatizer_cache 指令让你控制可以为该未压缩字典缓存使用多少内存。

目前，唯一可用的字典是[ru.pak, en.pak 和 de.pak](https://manticoresearch.com/install/)，分别是俄语、英语和德语字典。压缩字典大小约为 2 到 10 MB。注意，字典始终驻留在内存中。默认缓存大小是 256 KB。可接受的缓存大小是 0 到 2047 MB。将缓存大小设置得很高是安全的；词形还原器只会使用所需的内存。例如，整个俄语字典解压后约为 110 MB；因此设置 `lemmatizer_cache` 高于该值不会影响内存使用。即使允许 1024 MB 缓存，如果仅需 110 MB，也只会使用这 110 MB。

#### max_file_field_buffer

```ini
max_file_field_buffer = 128M
```

最大文件字段自适应缓冲区大小，单位字节。可选，默认是 8MB，最小值为 1MB。

文件字段缓冲区用于加载从 [sql_file_field](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_file_field)列引用的文件。该缓冲区是自适应的，首次分配时为 1 MB，并以 2 倍递增，直到能够加载文件内容或达到 `max_file_field_buffer` 指定的最大缓冲区大小。

因此，如果没有指定文件字段，则不会分配缓冲区。如果在索引期间加载的所有文件大小均低于（例如）2 MB，而 `max_file_field_buffer` 值为 128 MB，则峰值缓冲区使用仍为 2 MB。但是，超过 128 MB 的文件将被完全跳过。

#### max_iops

```ini
max_iops = 40
```

最大每秒 I/O 操作次数，用于 I/O 限流。可选，默认是 0（无限制）。

这是一个与 I/O 限流相关的选项。它限制每秒最大的 I/O 操作数（读或写）。值为 0 表示无限制。

`indexer` 在构建表时可能会导致磁盘 I/O 突发高峰，限制其磁盘活动（以为同一台机器上的其他程序如 `searchd` 留出资源）可能会很有必要。I/O 限流通过强制在 `indexer` 执行的连续磁盘 I/O 操作之间存在最小保证延迟来实现。限制 I/O 可以帮助减轻构建期间对搜索性能的影响。此设置对其他类型的数据摄取（例如插入实时表）无效。

#### max_iosize

```ini
max_iosize = 1048576
```

最大允许的 I/O 操作大小，单位字节，用于 I/O 限流。可选，默认是 0（无限制）。

这是一个与 I/O 限流相关的选项。它限制 `indexer` 所有操作的最大文件 I/O 操作（读或写）大小。值为 0 表示无限制。大于限制的读写操作将被拆分成多个较小的操作，并由 [max_iops](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#max_iops) 计数。当前所有 I/O 调用应小于 256 KB（默认内部缓冲区大小），因此设置大于 256 KB 的 max_iosize 不会生效。

#### max_xmlpipe2_field

```ini
max_xmlpipe2_field = 8M
```

XMLpipe2 源类型允许的最大字段大小，单位字节。可选，默认是 2 MB。

#### mem_limit

```ini
mem_limit = 256M
# mem_limit = 262144K # same, but in KB
# mem_limit = 268435456 # same, but in bytes
```

Plain table building RAM 使用限制。可选，默认值为128 MB。强制执行的内存使用限制，`indexer` 不会超过此限制。可以以字节、千字节（使用K后缀）或兆字节（使用M后缀）的形式指定；请参见示例。如果设置的值极低导致I/O缓冲区小于8 KB，则此限制将自动提高；具体下限取决于构建数据的大小。如果缓冲区小于256 KB，则会产生警告。

可能的最大限制是2047M。太低的值会影响plain table构建速度，但256M到1024M应该足以满足大多数甚至所有数据集的需求。将此值设置得太高可能会导致SQL服务器超时。在文档收集阶段，会有时期间内存缓冲区部分排序且不与数据库进行通信；数据库服务器可能会超时。可以通过提高SQL服务器端的超时时间或降低`mem_limit`来解决这个问题。

#### on_file_field_error

```ini
on_file_field_error = skip_document
```

如何处理文件字段中的IO错误。可选，默认值为`ignore_field`。
当索引通过文件字段引用的文件时（[sql_file_field](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_file_field)），`indexer` 可以选择忽略该字段并继续处理当前文档，或者跳过当前文档，或者完全失败。`on_file_field_error` 指令控制这种行为。它可以接受以下值：
* `ignore_field`，忽略该字段并继续处理当前文档；
* `skip_document`，跳过当前文档但继续索引；
* `fail_index`，因错误消息而失败索引。

可能出现的问题包括：打开错误、大小错误（文件太大）和数据读取错误。无论处于哪个阶段或`on_file_field_error`设置如何，任何问题都会给出警告信息。

请注意，当`on_file_field_error = skip_document`时，只有在早期检查阶段检测到问题时才会忽略文档，而不是在实际解析文件阶段。`indexer`会在开始工作之前打开每个引用的文件并检查其大小，然后在实际解析工作时再次打开。因此，在这两个打开尝试之间，如果文件丢失，文档仍然会被索引。

#### write_buffer

```ini
write_buffer = 4M
```

写缓冲区大小，字节。可选，默认值为1MB。写缓冲区用于索引时写入临时和最终表文件。较大的缓冲区可以减少所需的磁盘写入次数。缓冲区的内存分配在[mem_limit](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#mem_limit)之外。请注意，将为不同的文件分配多个（目前最多4个）缓冲区，这会相应地增加RAM使用量。

#### ignore_non_plain

```ini
ignore_non_plain = 1
```

`ignore_non_plain` 允许您完全忽略关于跳过非plain表的警告。默认值为0（不忽略）。
<!-- proofread -->


### 通过systemd调度索引器

有两种方法可以调度索引器运行。第一种方法是传统的crontab方法。第二种方法是使用systemd定时器和用户定义的时间表。要创建定时器单元文件，您需要将它们放在systemd查找此类单元文件的适当目录中。在大多数Linux发行版中，这个目录通常是`/etc/systemd/system`。以下是操作步骤：

1. 创建一个定时器单元文件以适应您的自定义时间表：
   ```shell
   cat << EOF > /etc/systemd/system/manticore-indexer@.timer
   [Unit]
   Description=Run Manticore Search's indexer on schedule
   [Timer]
   OnCalendar=minutely
   RandomizedDelaySec=5m
   Unit=manticore-indexer@%i.service
   [Install]
   WantedBy=timers.target
   EOF
   ```
   更多关于`OnCalendar`语法和示例的信息可以在[这里](https://www.freedesktop.org/software/systemd/man/latest/systemd.time.html#Calendar%20Events)找到。

2. 编辑定时器单元以满足您的特定需求。
3. 启用定时器：
   ```shell
   systemctl enable manticore-indexer@idx1.timer
   ```
4. 启动定时器：
   ```shell
   systemctl start manticore-indexer@idx1.timer
   ```
5. 对于任何其他定时器重复步骤2-4。

