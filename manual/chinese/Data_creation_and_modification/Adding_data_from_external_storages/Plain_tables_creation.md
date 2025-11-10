# 纯表创建

纯表是通过在创建时从一个或多个源获取数据一次性创建的表。纯表是不可变的，因为在其生命周期内无法添加或删除文档。只能更新数值属性（包括 MVA）的值。刷新数据只能通过重新创建整个表来实现。

纯表仅在[纯模式](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29)中可用，其定义由表声明和一个或多个源声明组成。数据收集和表创建不是由 `searchd` 服务器完成，而是由辅助工具 `indexer` 完成。

**Indexer** 是一个命令行工具，可以直接从命令行或 shell 脚本调用。

调用时它可以接受多个参数，但在 Manticore 配置文件中也有一些自身的设置。

在典型场景中，indexer 执行以下操作：
* 从源获取数据
* 构建纯表
* 写入表文件
* （可选）通知搜索服务器新表，触发表轮换

## Indexer 工具
`indexer` 工具用于在 Manticore Search 中创建纯表。其通用语法为：

```shell
indexer [OPTIONS] [table_name1 [table_name2 [...]]]
```

使用 `indexer` 创建表时，生成的表文件必须具有允许 `searchd` 读取、写入和删除的权限。在官方 Linux 包中，`searchd` 以 `manticore` 用户身份运行。因此，`indexer` 也必须以 `manticore` 用户身份运行：

```shell
sudo -u manticore indexer ...
```

如果你以不同方式运行 `searchd`，可能需要省略 `sudo -u manticore`。只需确保运行 `searchd` 实例的用户对使用 `indexer` 生成的表具有读写权限。

要创建纯表，需要列出要处理的表。例如，如果你的 `manticore.conf` 文件包含两个表的详细信息，`mybigindex` 和 `mysmallindex`，你可以运行：

```shell
sudo -u manticore indexer mysmallindex mybigindex
```

你也可以使用通配符匹配表名：

* `?` 匹配任意单个字符
* `*` 匹配任意数量的任意字符
* `%` 匹配无或任意单个字符

```shell
sudo -u manticore indexer indexpart*main --rotate
```

indexer 的退出代码如下：

* 0：一切正常
* 1：索引时出现问题（如果指定了 `--rotate`，则跳过）或操作发出警告
* 2：索引正常，但 `--rotate` 尝试失败

### Indexer systemd 服务

你也可以使用以下 systemctl 单元文件启动 `indexer`：

```shell
systemctl start --no-block manticore-indexer
```

或者，如果你想构建特定表：

```shell
systemctl start --no-block manticore-indexer@specific-table-name
```

使用 `systemctl set-environment INDEXER_CONFIG` 命令以自定义配置运行 Indexer，替代默认设置。

`systemctl set-environment INDEXER_ARGS` 命令允许你为 Indexer 添加自定义启动选项。完整的命令行选项列表，请参见[这里](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments)。

例如，要以静默模式启动 Indexer，运行：
```bash
systemctl set-environment INDEXER_ARGS='--quiet'
systemctl restart manticore-indexer
```

要恢复更改，运行：
```bash
systemctl set-environment INDEXER_ARGS=''
systemctl restart manticore-indexer
```

### Indexer 命令行参数
* `--config <file>`（简写为 `-c <file>`）告诉 `indexer` 使用指定文件作为配置。通常，它会在安装目录（例如 `/etc/manticoresearch/manticore.conf`）中查找 `manticore.conf`，然后是调用 `indexer` 时所在的当前目录。这在共享环境中非常有用，例如二进制文件安装在全局文件夹（如 `/usr/bin/`），但你希望用户能够创建自己的自定义 Manticore 设置，或者你想在单台服务器上运行多个实例。在这种情况下，你可以允许他们创建自己的 `manticore.conf` 文件，并通过此选项传递给 `indexer`。例如：

  ```shell
  sudo -u manticore indexer --config /home/myuser/manticore.conf mytable
  ```

* `--all` 告诉 `indexer` 更新 `manticore.conf` 中列出的所有表，而不是列出单个表。这在小型配置或定时任务或维护作业中很有用，其中整个表集每天、每周或其他最佳周期重建。请注意，由于 `--all` 尝试更新配置中找到的所有表，如果遇到实时表会发出警告，即使纯表完成没有问题，命令的退出代码也会是 `1` 而非 `0`。示例用法：

  ```shell
  sudo -u manticore indexer --config /home/myuser/manticore.conf --all
  ```

* `--rotate` 用于轮换表。除非你能让搜索功能离线而不影响用户，否则几乎肯定需要在索引新文档时保持搜索运行。`--rotate` 创建第二个表，与第一个表并行（在同一位置，只是在文件名中包含 `.new`）。完成后，`indexer` 通过发送 `SIGHUP` 信号通知 `searchd`，`searchd` 会尝试重命名表（将现有表重命名为包含 `.old`，并将 `.new` 重命名替换它们），然后开始从新文件提供服务。根据 [seamless_rotate](../../Server_settings/Searchd.md#seamless_rotate) 的设置，搜索新表可能会有轻微延迟。如果一次轮换多个通过 [killlist_target](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target) 关联的表，轮换将从非目标表开始，最后完成目标链末端的表。示例用法：

  ```shell
  sudo -u manticore indexer --rotate --all
  ```
* `--quiet` 告诉 `indexer` 除非发生错误，否则不输出任何内容。这主要用于定时任务或其他脚本作业，其中输出无关紧要或不必要，除非发生错误。示例用法：

  ```shell
  sudo -u manticore indexer --rotate --all --quiet
  ```
* `--noprogress` 不会显示进度详情。相反，只有在索引完成时才报告最终状态详情（例如索引的文档数、索引速度等）。在脚本未在控制台（或 'tty'）上运行的情况下，默认启用此选项。示例用法：

  ```shell
  sudo -u manticore indexer --rotate --all --noprogress
  ```
* `--buildstops <outputfile.text> <N>` 会审查表的源数据，就像正在索引数据一样，并生成正在被索引的词汇列表。换句话说，它生成所有成为表一部分的可搜索词汇列表。注意，它不会更新相关表，只是像索引一样处理数据，包括运行用 [sql_query_pre](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Execution_of_fetch_queries.md#sql_query_pre) 或 [sql_query_post](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Execution_of_fetch_queries.md#sql_query_post) 定义的查询。`outputfile.txt` 将包含词汇列表，每行一个，按频率排序，最频繁的词在前，`N` 指定列出的最大词数。如果足够大以涵盖表中的所有词，则只返回那么多词。这样的词典列表可用于客户端应用的“你是想说……”功能，通常与下面的 `--buildfreqs` 一起使用。示例：

  ```shell
  sudo -u manticore indexer mytable --buildstops word_freq.txt 1000
  ```

  这将在当前目录生成一个文档 `word_freq.txt`，包含 'mytable' 中最常见的 1000 个词，按最常见排序。注意，如果指定多个表或 `--all`，该文件将对应最后一个被索引的表（即配置文件中最后列出的表）。

* `--buildfreqs` 与 `--buildstops` 配合使用（如果未指定 `--buildstops`，则忽略此选项）。由于 `--buildstops` 提供了表中使用的词列表，`--buildfreqs` 会添加这些词在表中出现的数量，这对于确定某些词是否应被视为停用词（如果它们过于常见）很有用。它还帮助开发“你是想说……”功能，了解某词相较于另一个相似词的出现频率。例如：

  ```shell
  sudo -u manticore indexer mytable --buildstops word_freq.txt 1000 --buildfreqs
  ```

  这将生成上述的 `word_freq.txt`，但每个词后面会跟着它在相关表中出现的次数。

* `--merge <dst-table> <src-table>` 用于物理合并表，例如如果你有一个 [主+增量方案](../../Creating_a_table/Local_tables/Plain_table.md#Main+delta-scenario)，主表很少更改，但增量表经常重建，`--merge` 用于合并两者。操作从右向左进行——检查 `src-table` 的内容并与 `dst-table` 的内容物理合并，结果保留在 `dst-table` 中。伪代码表达为：`dst-table += src-table` 示例：

  ```shell
  sudo -u manticore indexer --merge main delta --rotate
  ```

  在上述示例中，主表是主控且很少修改的表，增量表更频繁修改，你可以用上述命令调用 `indexer` 将增量表内容合并到主表并旋转表。

* `--merge-dst-range <attr> <min> <max>` 在合并时应用给定的过滤范围。具体来说，由于合并应用于目标表（作为 `--merge` 的一部分，如果未指定 `--merge` 则忽略），`indexer` 也会过滤进入目标表的文档，只有通过给定过滤器的文档才会出现在最终表中。例如，在一个有 'deleted' 属性的表中，0 表示“未删除”，可以用如下命令合并：

  ```shell
  sudo -u manticore indexer --merge main delta --merge-dst-range deleted 0 0
  ```

  标记为已删除（值为 1）的文档将从新合并的目标表中移除。此选项可以多次添加到命令行，添加多个连续过滤器，所有过滤条件都必须满足，文档才会成为最终表的一部分。

* --`merge-killlists`（及其简写别名 `--merge-klists`）改变合并表时杀死列表的处理方式。默认情况下，合并后两个杀死列表都会被丢弃。这支持最典型的主+增量合并场景。但启用此选项后，两个表的杀死列表会被连接并存储到目标表中。注意，源（增量）表的杀死列表始终用于抑制目标（主）表中的行。
* `--keep-attrs` 允许在重新索引时重用现有属性。每当表重建时，会检查每个新文档 ID 是否存在于“旧”表中，如果存在，其属性会被转移到“新”表；如果未找到，则使用新表的属性。如果用户更新了表中的属性，但未更新实际用于表的源数据，重新索引时所有更新都会丢失；使用 `--keep-attrs` 可以保存之前表中更新的属性值。可以指定表文件路径以替代配置中的参考路径：

  ```shell
  sudo -u manticore indexer mytable --keep-attrs=/path/to/index/files
  ```

* `--keep-attrs-names=<attributes list>` 允许指定在重新索引时从现有表重用的属性。默认情况下，所有属性都会从现有表重用到新表：

  ```shell
  sudo -u manticore indexer mytable --keep-attrs=/path/to/table/files --keep-attrs-names=update,state
  ```

* `--dump-rows <FILE>` 将由 SQL 源获取的行以 MySQL 兼容的语法转储到指定文件中。生成的转储是 `indexer` 接收到的数据的精确表示，有助于重复索引时出现的问题。该命令执行从源的获取，并创建表文件和转储文件。
* `--print-rt <rt_index> <table>` 以 INSERT 语句的形式输出从源获取的实时表数据。转储的前几行将包含实时字段和属性（作为普通表字段和属性的反映）。该命令执行从源的获取，并创建表文件和转储输出。该命令可用作 `sudo -u manticore indexer -c manticore.conf --print-rt indexrt indexplain > dump.sql`。仅支持基于 SQL 的源。不支持 MVA。
* `--sighup-each` 在重建许多大型表时非常有用，您希望每个表尽快旋转到 `searchd`。使用 `--sighup-each`，`indexer` 会在成功完成每个表的工作后向 searchd 发送 SIGHUP 信号。（默认行为是在所有表构建完成后发送单个 SIGHUP）。
* `--nohup` 在您想在实际旋转表之前使用 indextool 检查表时非常有用。如果启用此选项，indexer 不会发送 SIGHUP。表文件将重命名为 .tmp。使用 indextool 将表文件重命名为 .new 并旋转它。示例用法：

  ```shell
  sudo -u manticore indexer --rotate --nohup mytable
  sudo -u manticore indextool --rotate --check mytable
  ```

* `--print-queries` 打印 `indexer` 发送到数据库的 SQL 查询，以及 SQL 连接和断开事件。这对于诊断和修复 SQL 源的问题很有用。
* `--help`（简写为 `-h`）列出 `indexer` 中可调用的所有参数。
* `-v` 显示 `indexer` 版本。

### Indexer 配置设置
您还可以在 Manticore 配置文件的 `indexer` 部分配置 indexer 行为：

```ini
indexer {
...
}
```

#### lemmatizer_cache

```ini
lemmatizer_cache = 256M
```
词形还原缓存大小。可选，默认值为 256K。

我们的[词形还原器](../../Server_settings/Common.md#lemmatizer_base)实现使用压缩字典格式，支持空间与速度的权衡。它可以直接在压缩数据上执行词形还原，使用更多 CPU 但更少 RAM，或者可以部分或完全解压并预缓存字典，从而使用更少 CPU 但更多 RAM。lemmatizer_cache 指令让您控制用于未压缩字典缓存的 RAM 大小。

目前，唯一可用的字典是[ru.pak、en.pak 和 de.pak](https://manticoresearch.com/install/)，分别是俄语、英语和德语字典。压缩字典大小约为 2 到 10 MB。请注意，字典始终驻留在内存中。默认缓存大小为 256 KB。接受的缓存大小范围是 0 到 2047 MB。将缓存大小设置得过高是安全的；词形还原器只会使用所需的内存。例如，整个俄语字典解压后约为 110 MB；因此将 `lemmatizer_cache` 设置高于此值不会影响内存使用。即使允许缓存为 1024 MB，如果只需 110 MB，它也只会使用这 110 MB。

#### max_file_field_buffer

```ini
max_file_field_buffer = 128M
```

文件字段自适应缓冲区的最大大小（字节）。可选，默认值为 8MB，最小值为 1MB。

文件字段缓冲区用于加载来自 [sql_file_field](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_file_field) 列引用的文件。该缓冲区是自适应的，首次分配时为 1 MB，并以 2 倍的步长增长，直到文件内容可以加载或达到 `max_file_field_buffer` 指令指定的最大缓冲区大小。

因此，如果未指定文件字段，则不会分配缓冲区。如果索引期间加载的所有文件大小均小于（例如）2 MB，但 `max_file_field_buffer` 值为 128 MB，则峰值缓冲区使用量仍仅为 2 MB。然而，超过 128 MB 的文件将被完全跳过。

#### max_iops

```ini
max_iops = 40
```

最大每秒 I/O 操作数，用于 I/O 限速。可选，默认值为 0（无限制）。

与 I/O 限速相关的选项。它限制任意给定秒内的最大 I/O 操作数（读或写）。值为 0 表示不施加限制。

`indexer` 在构建表时可能会引起磁盘 I/O 的突发高峰，可能希望限制其磁盘活动（并为同一机器上运行的其他程序，如 `searchd`，保留资源）。I/O 限速有助于实现这一点。它通过强制 `indexer` 执行的连续磁盘 I/O 操作之间的最小保证延迟来工作。限制 I/O 可以帮助减少构建期间导致的搜索性能下降。此设置对其他类型的数据摄取无效，例如向实时表插入数据。

#### max_iosize

```ini
max_iosize = 1048576
```

最大允许的 I/O 操作大小（字节），用于 I/O 限速。可选，默认值为 0（无限制）。

与 I/O 限速相关的选项。它限制 `indexer` 执行的所有文件 I/O 操作（读或写）的最大大小。值为 0 表示不施加限制。大于限制的读写操作将被拆分为多个较小的操作，并由 [max_iops](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#max_iops) 设置计为多个操作。撰写本文时，所有 I/O 调用应均小于 256 KB（默认内部缓冲区大小），因此 max_iosize 设置高于 256 KB 不应有任何影响。

#### max_xmlpipe2_field

```ini
max_xmlpipe2_field = 8M
```

XMLpipe2 源类型允许的最大字段大小（字节）。可选，默认值为 2 MB。

#### mem_limit

```ini
mem_limit = 256M
# mem_limit = 262144K # same, but in KB
# mem_limit = 268435456 # same, but in bytes
```

纯表构建内存使用限制。可选，默认值为128 MB。强制执行的内存使用限制，`indexer` 不会超过该限制。可以以字节、千字节（使用K后缀）或兆字节（使用M后缀）指定；参见示例。如果设置为极低值导致I/O缓冲区小于8 KB，该限制将自动提高；具体下限取决于构建数据的大小。如果缓冲区小于256 KB，将发出警告。

最大可能的限制是2047M。过低的值会影响纯表构建速度，但256M到1024M对于大多数（如果不是全部）数据集来说应该足够。设置此值过高可能导致SQL服务器超时。在文档收集阶段，会有内存缓冲区部分排序且不与数据库通信的时间段；数据库服务器可能会超时。您可以通过提高SQL服务器端的超时设置或降低`mem_limit`来解决此问题。

#### on_file_field_error

```ini
on_file_field_error = skip_document
```

如何处理文件字段中的IO错误。可选，默认值为`ignore_field`。
当索引由文件字段引用的文件（[sql_file_field](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_file_field)）时出现问题，`indexer` 可以处理文档，假设该字段内容为空，或者跳过文档，或者完全失败索引。`on_file_field_error` 指令控制该行为。它可以取的值有：
* `ignore_field`，处理当前文档但不包含该字段；
* `skip_document`，跳过当前文档但继续索引；
* `fail_index`，索引失败并显示错误信息。

可能出现的问题包括：打开错误、大小错误（文件过大）和数据读取错误。无论阶段和`on_file_field_error`设置如何，任何问题都会发出警告信息。

注意，使用`on_file_field_error = skip_document`时，只有在早期检查阶段检测到问题时文档才会被忽略，**而不是**在实际文件解析阶段。`indexer` 会在进行任何工作之前打开每个引用的文件并检查其大小，然后在实际解析时再次打开它。因此，如果文件在这两次打开之间消失，文档仍将被索引。

#### write_buffer

```ini
write_buffer = 4M
```

写缓冲区大小，单位字节。可选，默认值为1MB。写缓冲区用于在索引时写入临时和最终的表文件。较大的缓冲区减少所需的磁盘写入次数。缓冲区的内存分配是额外的，独立于[mem_limit](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#mem_limit)。注意，将为不同文件分配多个（目前最多4个）缓冲区，比例增加RAM使用量。

#### ignore_non_plain

```ini
ignore_non_plain = 1
```

`ignore_non_plain` 允许您完全忽略关于跳过非纯表的警告。默认值为0（不忽略）。
<!-- proofread -->


### 通过 systemd 调度 indexer

有两种调度 indexer 运行的方法。第一种是使用 crontab 的经典方法。第二种是使用带有用户定义计划的 systemd 计时器。要创建计时器单元文件，应将它们放置在 systemd 查找此类单元文件的适当目录中。在大多数 Linux 发行版中，该目录通常是 `/etc/systemd/system`。操作步骤如下：

1. 为您的自定义计划创建计时器单元文件：
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
   关于 `OnCalendar` 语法和示例的更多信息，请参见[这里](https://www.freedesktop.org/software/systemd/man/latest/systemd.time.html#Calendar%20Events)。

2. 根据您的具体需求编辑计时器单元。
3. 启用计时器：
   ```shell
   systemctl enable manticore-indexer@idx1.timer
   ```
4. 启动计时器：
   ```shell
   systemctl start manticore-indexer@idx1.timer
   ```
5. 对任何额外的计时器重复步骤2-4。

