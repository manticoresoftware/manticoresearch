# 普通表的创建

普通表是通过在创建时从一个或多个源提取数据一次性创建的表。普通表是不可变的，因为在其生命周期内无法添加或删除文档。只能更新数值属性的值（包括 MVA）。刷新数据只能通过重新创建整个表来实现。

普通表仅在[普通模式](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29)下可用，其定义由表声明和一个或多个源声明组成。数据收集和表创建不是由`searchd`服务器完成的，而是由辅助工具`indexer`完成的。

**Indexer**是一个命令行工具，可以直接从命令行或脚本中调用。

它在调用时可以接受许多参数，但在 Manticore 配置文件中也有几个自己的设置。

在典型场景中，indexer 执行以下操作：
* 从源获取数据
* 构建普通表
* 写入表文件
* （可选）通知搜索服务器新表，这会触发表轮换

## Indexer 工具
`indexer` 工具用于在 Manticore Search 中创建普通表。它的通用语法为：

```shell
indexer [OPTIONS] [table_name1 [table_name2 [...]]]
```

在使用`indexer`创建表时，生成的表文件必须具有允许`searchd`读取、写入和删除的权限。在官方的 Linux 包情况下，`searchd` 以`manticore`用户身份运行。因此，`indexer` 也必须以`manticore`用户身份运行：

```shell
sudo -u manticore indexer ...
```

如果您以其他方式运行`searchd`，则可能需要省略`sudo -u manticore`。只需确保运行您的`searchd`实例的用户具有对使用`indexer`生成的表的读/写权限。

要创建普通表，您需要列出要处理的表。例如，如果您的`manticore.conf`文件包含两个表的详细信息，`mybigindex`和`mysmallindex`，您可以运行：

```shell
sudo -u manticore indexer mysmallindex mybigindex
```

您还可以使用通配符匹配表名：

* `?` 匹配任何单个字符
* `*` 匹配任意数量的任意字符
* `%` 匹配零个或一个单个字符

```shell
sudo -u manticore indexer indexpart*main --rotate
```

indexer 的退出代码如下：

* 0: 一切正常
* 1: 在索引时出现问题（如果指定了`--rotate`，则会跳过）或某个操作发出了警告
* 2: 索引正常，但`--rotate`尝试失败

### Indexer systemd 服务

您还可以使用以下 systemctl 单元文件启动`indexer`：

```shell
systemctl start --no-block manticore-indexer
```

或者，如果您想构建特定表：

```shell
systemctl start --no-block manticore-indexer@specific-table-name
```

使用`systemctl set-environment INDEXER_CONFIG`命令以自定义配置运行 Indexer，这将替换默认设置。

`systemctl set-environment INDEXER_ARGS`命令允许您为 Indexer 添加自定义启动选项。有关命令行选项的完整列表，请参见[这里](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments)。

例如，要以静默模式启动 Indexer，请运行：
```bash
systemctl set-environment INDEXER_ARGS='--quiet'
systemctl restart manticore-indexer
```

若要恢复更改，请运行：
```bash
systemctl set-environment INDEXER_ARGS=''
systemctl restart manticore-indexer
```

### Indexer 命令行参数
* `--config <file>` （`-c <file>` 简写形式）告诉`indexer`使用给定的文件作为其配置。通常，它将查找安装目录中的`manticore.conf`（例如 `/etc/manticoresearch/manticore.conf`），然后是您在从 shell 调用`indexer`时所在的当前目录。这在共享环境中特别有用，其中二进制文件安装在全局文件夹中，例如`/usr/bin/`，但您希望为用户提供自定义 Manticore 设置的能力，或者如果您想在单个服务器上运行多个实例。在这种情况下，您可以允许他们创建自己的`manticore.conf`文件，并通过此选项将其传递给`indexer`。例如：

  ```shell
  sudo -u manticore indexer --config /home/myuser/manticore.conf mytable
  ```

* `--all`告诉`indexer`更新`manticore.conf`中列出的每个表，而不是列出单个表。这在小配置或Cron式或维护任务中会很有用，其中整个表集每天或每周或者任何最佳时间段都会重建。请注意，由于`--all`尝试更新配置中找到的所有表，如果它遇到实时表，它会发出警告，并且命令的退出代码将为`1`而不是`0`，即使普通表顺利完成。例如用法：

  ```shell
  sudo -u manticore indexer --config /home/myuser/manticore.conf --all
  ```
* `--rotate` 用于旋转表。除非您可以在不打扰用户的情况下将搜索功能下线，否则几乎肯定需要在索引新文档时保持搜索运行。 `--rotate` 会创建一个第二个表，与第一个表平行（位于相同位置，仅在文件名中包含 `.new`）。完成后， `indexer` 通过发送 `SIGHUP` 信号通知 `searchd`，然后 `searchd` 将尝试重命名表（将现有表重命名为包含 `.old` ，并将 `.new` 重命名为替换它们），然后开始从较新的文件提供服务。根据 [seamless_rotate](../../Server_settings/Searchd.md#seamless_rotate) 的设置，可能会有轻微的延迟才能搜索较新的表。如果同时旋转多个通过 [killlist_target](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target) 关系连接的表，则旋转将从非目标表开始，最后以目标链末尾的表结束。示例用法：

  ```shell
  sudo -u manticore indexer --rotate --all
  ```
* `--quiet` 告诉 `indexer` 不输出任何内容，除非出现错误。这主要用于 cron 类型或其他脚本作业，在这些情况下，输出是无关紧要或不必要的，除非发生某种错误。示例用法：

  ```shell
  sudo -u manticore indexer --rotate --all --quiet
  ```
* `--noprogress` 在发生时不显示进度详细信息。相反，最终状态详细信息（例如索引的文档、索引速度等）仅在索引完成时报告。在脚本未在控制台（或 'tty'）上运行的情况下，此选项默认为开启。示例用法：

  ```shell
  sudo -u manticore indexer --rotate --all --noprogress
  ```
* `--buildstops <outputfile.text> <N>` 检查表源，就像索引数据一样，并生成正在索引的术语列表。换句话说，它生成一个包含正在成为表的一部分的所有可搜索术语的列表。注意，它不会更新相关表，它只是处理数据，就像它正在索引一样，包括运行通过 [sql_query_pre](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Execution_of_fetch_queries.md#sql_query_pre) 或 [sql_query_post](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Execution_of_fetch_queries.md#sql_query_post) 定义的查询。 `outputfile.txt` 将包含逐行列出的词汇，按频率排序，频率最高的在前， `N` 指定将列出的最大单词数量。如果这个数量足够大以涵盖表中的每个单词，则只会返回这么多单词。这样的词典列表可用于与 "你是说..." 功能相关的客户端应用程序功能，通常与下面的 `--buildfreqs` 一起使用。示例：

  ```shell
  sudo -u manticore indexer mytable --buildstops word_freq.txt 1000
  ```

  这将在当前目录下生成一个名为 `word_freq.txt` 的文档，其中包含 'mytable' 中 1,000 个最常用的单词，按最常用的在前排序。请注意，当指定多个表或 `--all` 时，该文件将与最后索引的表相关（即在配置文件中列出的最后一个表）

* `--buildfreqs` 与 `--buildstops` 一起使用（如果未指定 `--buildstops`，则将被忽略）。由于 `--buildstops` 提供了在表中使用的单词列表， `--buildfreqs` 添加了在表中出现的数量，这将有助于确定某些单词是否应该被视为停用词，如果它们过于频繁。它还将有助于开发 "你是说..." 功能，其中您需要知道给定单词与另一个相似单词的相对常见程度。例如：

  ```shell
  sudo -u manticore indexer mytable --buildstops word_freq.txt 1000 --buildfreqs
  ```

  这将生成上面的 `word_freq.txt` ，但是在每个单词后面将是该单词在相关表中出现的次数。

* `--merge <dst-table> <src-table>` 用于物理合并表，例如，如果您有一个 [main+delta scheme](../../Creating_a_table/Local_tables/Plain_table.md#Main+delta-scenario)，其中主表很少更改，但增量表频繁重建， `--merge` 将被用于合并这两个表。操作从右到左进行 - `src-table` 的内容会被检查并物理合并到 `dst-table` 的内容中，结果保留在 `dst-table` 中。在伪代码中，可以表示为： `dst-table += src-table` 示例：

  ```shell
  sudo -u manticore indexer --merge main delta --rotate
  ```

  在上面的示例中，主表是主表，很少被修改，增量表是更频繁修改的表，您可能会使用上面的命令调用 `indexer` 将增量表的内容合并到主表并旋转表。

* `--merge-dst-range <attr> <min> <max>` 在合并时运行给定的过滤范围。具体来说，在将合并应用于目标表时（作为 `--merge` 的一部分，如果未指定 `--merge` 则将被忽略）， `indexer` 还将过滤最终进入目标表的文档，只有通过给定过滤的文档才会出现在最终表中。例如，这可以用于表中有 'deleted' 属性，其中 0 表示 '未删除'。这样的表可以与：

  ```shell
  sudo -u manticore indexer --merge main delta --merge-dst-range deleted 0 0
  ```

  任何标记为删除（值为 1）的文档将从新合并的目标表中删除。可以在命令行中多次添加，以在合并中添加连续的过滤器，所有过滤器都必须满足才能使文档成为最终表的一部分。
* --`merge-killlists`（及其较短别名`--merge-klists`）改变了合并表时杀死列表的处理方式。默认情况下，两个杀死列表在合并后都会被丢弃。这支持最典型的主+增量合并场景。然而，启用此选项后，两个表的杀死列表会被连接并存储到目标表中。请注意，源（增量）表的杀死列表将始终用于抑制目标（主）表中的行。
* `--keep-attrs`允许在重新索引时重用现有属性。每当重建表时，会检查每个新文档ID在“旧”表中的存在性，如果已存在，则将其属性转移到“新”表中；如果未找到，则使用新表中的属性。如果用户在表中更新了属性，但未在用于表的实际源中更新，则在重新索引时所有更新将丢失；使用`--keep-attrs`可保存来自先前表的更新属性值。可以指定用于表文件的路径，而不是从配置中引用的路径：

  ```shell
  sudo -u manticore indexer mytable --keep-attrs=/path/to/index/files
  ```

* `--keep-attrs-names=<attributes list>`允许您在重新索引时指定从现有表中重用的属性。默认情况下，所有来自现有表的属性都在新表中重用：

  ```shell
  sudo -u manticore indexer mytable --keep-attrs=/path/to/table/files --keep-attrs-names=update,state
  ```

* `--dump-rows <FILE>`将通过 SQL 源提取的行转储到指定的文件中，采用 MySQL 兼容的语法。生成的转储是`indexer`接收到的数据的确切表示，可以帮助重复索引时的问题。该命令从源提取数据并创建表文件和转储文件。
* `--print-rt <rt_index> <table>`以 INSERT 的形式输出来自源的提取数据，适用于实时表。转储的第一行将包含实时字段和属性（作为普通表字段和属性的反映）。该命令从源提取数据并创建表文件和转储输出。该命令可以用于`sudo -u manticore indexer -c manticore.conf --print-rt indexrt indexplain > dump.sql`。仅支持基于 SQL 的源。MVAs 不受支持。
* `--sighup-each`在重建许多大型表时非常有用，并希望尽快将每个表轮换到`searchd`。使用`--sighup-each`，`indexer`将在成功完成每个表的工作后向 searchd 发送 SIGHUP 信号。（默认行为是在所有表构建完成后发送一个 SIGHUP）。
* `--nohup`在您希望在实际轮换之前通过 indextool 检查表时非常有用。如果启用此选项，indexer 将不会发送 SIGHUP。表文件将重命名为 .tmp。使用 indextool 将表文件重命名为 .new 并进行轮换。示例用法：

  ```shell
  sudo -u manticore indexer --rotate --nohup mytable
  sudo -u manticore indextool --rotate --check mytable
  ```

* `--print-queries`打印出`indexer`发送到数据库的 SQL 查询，以及 SQL 连接和断开连接事件。这对诊断和解决 SQL 源的问题很有用。
* `--help`（短格式为`-h`）列出可以在`indexer`中调用的所有参数。
* `-v`显示`indexer`版本。

### Indexer 配置设置
您还可以在 `indexer` 部分的 Manticore 配置文件中配置 indexer 行为：

```ini
indexer {
...
}
```

#### lemmatizer_cache

```ini
lemmatizer_cache = 256M
```
词形还原器缓存大小。可选，默认值为 256K。

我们的[词形还原器](../../Server_settings/Common.md#lemmatizer_base)实现使用压缩字典格式，可以实现空间/速度的权衡。它可以在压缩数据上执行词形还原，使用更多的 CPU 但更少的 RAM，或者它可以部分或完全解压并预缓存字典，从而使用更少的 CPU 但更多的 RAM。lemmatizer_cache 指令可让您控制可以用于未压缩字典缓存的确切 RAM 数量。

目前，唯一可用的字典是[ru.pak, en.pak 和 de.pak](https://manticoresearch.com/install/)。这些是俄语、英语和德语字典。压缩字典的大约大小为 2 到 10 MB。请注意，字典始终保持在内存中。默认缓存大小为 256 KB。接受的缓存大小为 0 到 2047 MB。提高缓存大小是安全的；词形还原器将只使用所需的内存。例如，整个俄语字典解压后约为 110 MB；因此，设置`lemmatizer_cache`高于此不会影响内存使用。即使允许 1024 MB 用于缓存，如果只需要 110 MB，它也只会使用这 110 MB。

#### max_file_field_buffer

```ini
max_file_field_buffer = 128M
```

最大文件字段自适应缓冲区大小（以字节为单位）。可选，默认值为 8MB，最小值为 1MB。

文件字段缓冲区用于加载由[sql_file_field](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_file_field)列引用的文件。此缓冲区是自适应的，初始分配时为 1 MB，并以 2 倍的步长增长，直到能够加载文件内容或达到由`max_file_field_buffer`指令指定的最大缓冲区大小。

因此，如果未指定任何文件字段，则根本不分配缓冲区。如果在索引期间加载的所有文件的大小均小于（例如）2 MB，但`max_file_field_buffer`值为 128 MB，则峰值缓冲区使用仍将只有 2 MB。但是，大于 128 MB 的文件将完全被跳过。

#### max_iops

```ini
max_iops = 40
```

每秒最大 I/O 操作数，用于 I/O 限流。可选，默认值为 0（无限制）。
I/O 限制相关选项。它限制了每秒最大 I/O 操作（读取或写入）的次数。值为 0 意味着没有限制。

`indexer` 在构建表时可能会导致大量的磁盘 I/O，可能希望限制其磁盘活动（并为同一机器上运行的其他程序保留一些，比如 `searchd`）。I/O 限制有助于实现这一点。它通过强制在 `indexer` 执行的后续磁盘 I/O 操作之间强制最小保证延迟来工作。限制 I/O 可以帮助减少由于构建引起的搜索性能下降。此设置对其他类型的数据摄取无效，例如向实时表中插入数据。

#### max_iosize

```ini
max_iosize = 1048576
```

I/O 限制的最大允许 I/O 操作大小，单位为字节。可选，默认为 0（无限制）。

I/O 限制相关选项。它限制 `indexer` 执行的所有操作的最大文件 I/O 操作（读取或写入）大小。值为 0 意味着没有限制。大于限制的读取或写入将被拆分为几个较小的操作，并被 [max_iops](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#max_iops) 设置计算为多个操作。在撰写本文时，所有 I/O 调用都应低于 256 KB（默认内部缓冲区大小），因此 max_iosize 值高于 256 KB 应该没有任何影响。

#### max_xmlpipe2_field

```ini
max_xmlpipe2_field = 8M
```

XMLpipe2 源类型的最大允许字段大小，单位为字节。可选，默认为 2 MB。

#### mem_limit

```ini
mem_limit = 256M
# mem_limit = 262144K # 同样，但以 KB 为单位
# mem_limit = 268435456 # 同样，但以字节为单位
```

普通表构建的 RAM 使用限制。可选，默认值为 128 MB。强制的内存使用限制，`indexer` 将不会超过该限制。可以用字节、千字节（使用 K 后缀）或兆字节（使用 M 后缀）指定；见例子。如果设置为极低值造成 I/O 缓冲区小于 8 KB，则该限制将自动提高；确切的下限取决于构建数据的大小。如果缓冲区低于 256 KB，将产生警告。

最大可能限制为 2047M。过低的值可能会影响普通表的构建速度，但 256M 到 1024M 应该适合大多数数据集。如果将此值设置得过高，可能会导致 SQL 服务器超时。在文档收集阶段，会有一些时间段内内存缓冲区部分排序且不与数据库进行通信；此时数据库服务器可能会超时。您可以通过提高 SQL 服务器端的超时设置或降低 `mem_limit` 来解决此问题。

#### on_file_field_error

```ini
on_file_field_error = skip_document
```

如何处理文件字段中的 I/O 错误。可选，默认值为 `ignore_field`。
当索引文件字段（[sql_file_field](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_file_field)）引用的文件出现问题时，`indexer` 可以处理文档，假设该特定字段的内容为空，或者跳过文档，或者完全失败索引。`on_file_field_error` 指令控制该行为。它可以取的值有：
* `ignore_field`，处理当前文档而不包含字段；
* `skip_document`，跳过当前文档但继续索引；
* `fail_index`，索引失败并给出错误消息。

可能出现的问题包括：打开错误、大小错误（文件过大）和数据读取错误。无论阶段和 `on_file_field_error` 设置如何，任何问题将始终给出警告消息。

请注意，使用 `on_file_field_error = skip_document` 时，文档将仅在早期检查阶段检测到问题时被忽略，而 **不会** 在实际文件解析阶段被忽略。`indexer` 将在进行任何工作之前打开每个引用的文件并检查其大小，然后在进行实际解析工作时再次打开它。因此，如果在这两次打开尝试之间文件消失，文档仍然会被索引。

#### write_buffer

```ini
write_buffer = 4M
```

写入缓冲区大小，单位为字节。可选，默认值为 1MB。在索引时，写入缓冲区用于写入临时和最终表文件。较大的缓冲区减少所需的磁盘写入次数。缓冲区所需的内存是额外分配的，超过 [mem_limit](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#mem_limit)。请注意，将为不同文件分配多个（当前最多 4 个）缓冲区，按比例增加 RAM 使用量。

#### ignore_non_plain

```ini
ignore_non_plain = 1
```

`ignore_non_plain` 允许您完全忽略关于跳过非普通表的警告。默认值为 0（不忽略）。
<!-- proofread -->


### 通过 systemd 安排索引器

有两种方法可以调度索引器运行。第一种方式是使用 crontab 的经典方法。第二种方式是使用具有用户定义计划的 systemd 定时器。要创建定时器单元文件，您应将其放置在 systemd 查找此类单元文件的适当目录中。在大多数 Linux 发行版中，此目录通常是 `/etc/systemd/system`。以下是如何操作：

1. 为您的自定义计划创建一个定时器单元文件：
   ```shell
   cat << EOF > /etc/systemd/system/manticore-indexer@.timer
   [Unit]
   Description=按计划运行 Manticore Search 的索引器
   [Timer]
   OnCalendar=minutely
   RandomizedDelaySec=5m
   Unit=manticore-indexer@%i.service
   [Install]
   WantedBy=timers.target
   EOF
   ```
   关于 `OnCalendar` 语法和示例，请见 [这里](https://www.freedesktop.org/software/systemd/man/latest/systemd.time.html#Calendar%20Events).
2. 编辑定时器单元以满足您的特定需求。
3. 启用定时器：
   ```shell
   systemctl enable manticore-indexer@idx1.timer
   ```
4. 启动定时器：
   ```shell
   systemctl start manticore-indexer@idx1.timer
   ```
5. 对任何其他定时器重复步骤 2-4。




