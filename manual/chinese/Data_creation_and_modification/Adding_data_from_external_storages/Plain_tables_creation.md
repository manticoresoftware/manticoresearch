# 普通表创建

普通表是通过在创建时从一个或多个源获取数据一次性创建的表。普通表是不可变的，因为在其生命周期内无法添加或删除文档。只能更新数值属性（包括多值属性 MVA）的值。刷新数据只能通过重新创建整个表来完成。

普通表仅在[普通模式](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29)下可用，其定义由一个表声明和一个或多个源声明组成。数据收集和表创建不是由 `searchd` 服务器完成，而是由辅助工具 `indexer` 完成。

**Indexer** 是一个命令行工具，可以直接从命令行调用，也可以从 shell 脚本调用。

调用时它可以接受许多参数，但它在 Manticore 配置文件中也有几个自己的设置。

在典型场景下，indexer 执行以下操作：
* 从源获取数据
* 构建普通表
* 写入表文件
* （可选）通知搜索服务器新表，触发表旋转

## Indexer 工具
`indexer` 工具用于在 Manticore Search 中创建普通表。其通用语法为：

```shell
indexer [OPTIONS] [table_name1 [table_name2 [...]]]
```

使用 `indexer` 创建表时，生成的表文件必须设置权限，使 `searchd` 能够读取、写入和删除它们。在官方 Linux 软件包中，`searchd` 以 `manticore` 用户身份运行。因此，`indexer` 也必须以 `manticore` 用户身份运行：

```shell
sudo -u manticore indexer ...
```

如果您以不同方式运行 `searchd`，可能需要省略 `sudo -u manticore`。只需确保运行 `searchd` 实例的用户对使用 `indexer` 生成的表具有读写权限。

要创建普通表，您需要列出要处理的表。例如，如果您的 `manticore.conf` 文件包含两个表 `mybigindex` 和 `mysmallindex` 的详细信息，您可以运行：

```shell
sudo -u manticore indexer mysmallindex mybigindex
```

您也可以使用通配符来匹配表名：

* `?` 匹配任意单个字符
* `*` 匹配任意数量的任意字符
* `%` 匹配零个或任意单个字符

```shell
sudo -u manticore indexer indexpart*main --rotate
```

indexer 的退出代码如下：

* 0：一切正常
* 1：索引过程中发生问题（如果指定了 `--rotate`，则被跳过）或操作发出了警告
* 2：索引成功，但 `--rotate` 尝试失败

### Indexer systemd 服务

您也可以使用以下 systemctl 单元文件启动 `indexer`：

```shell
systemctl start --no-block manticore-indexer
```

或者，在您想构建特定表的情况下：

```shell
systemctl start --no-block manticore-indexer@specific-table-name
```

使用 `systemctl set-environment INDEXER_CONFIG` 命令可使用自定义配置运行 Indexer，以替代默认设置。

`systemctl set-environment INDEXER_ARGS` 命令让您添加自定义的启动选项。完整的命令行选项列表，请见[这里](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments)。

例如，要以静默模式启动 Indexer，请运行：
```bash
systemctl set-environment INDEXER_ARGS='--quiet'
systemctl restart manticore-indexer
```

要还原更改，请运行：
```bash
systemctl set-environment INDEXER_ARGS=''
systemctl restart manticore-indexer
```

### Indexer 命令行参数
* `--config <file>`（简写为 `-c <file>`）告诉 `indexer` 使用指定的配置文件。通常，它会在安装目录下查找 `manticore.conf`（例如 `/etc/manticoresearch/manticore.conf`），然后是您调用 `indexer` 时所在的当前目录。这在共享环境中特别有用，例如二进制文件安装在全局文件夹（如 `/usr/bin/`），但您希望用户能够创建自己的自定义 Manticore 设置，或在单台服务器上运行多个实例。在这种情况下，您可以允许他们创建自己的 `manticore.conf` 文件，并使用此选项传递给 `indexer`。例如：

  ```shell
  sudo -u manticore indexer --config /home/myuser/manticore.conf mytable
  ```

* `--all` 告诉 `indexer` 更新 `manticore.conf` 中列出的所有表，而不是单独列出表名。这在小型配置、定时任务或维护作业中很有用，在这些情况下，整个表集会在每天、每周或其他合适的周期内重建。请注意，由于 `--all` 尝试更新配置中找到的所有表，如果遇到实时表（RealTime tables），它会发出警告，且命令的退出代码为 `1`，即使普通表没有问题。示例如下：

  ```shell
  sudo -u manticore indexer --config /home/myuser/manticore.conf --all
  ```

* `--rotate` 用于表的旋转。除非您能在不影响用户的情况下关闭搜索功能，否则几乎肯定需要在索引新文档时保持搜索运行。`--rotate` 创建第二个表，与第一个并行（位于同一位置，只是在文件名中包含 `.new`）。完成后，`indexer` 通过发送 `SIGHUP` 信号通知 `searchd`，然后 `searchd` 会尝试重命名表（将现有表重命名为包括 `.old`，再将 `.new` 替代其位置），然后开始使用较新版本的文件提供服务。根据 [seamless_rotate](../../Server_settings/Searchd.md#seamless_rotate) 的设置，查询新表可能会有轻微延迟。如果多个通过 [killlist_target](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target) 链接的表同时旋转，旋转将从不是目标的表开始，最后完成目标链末端的表。示例如下：

  ```shell
  sudo -u manticore indexer --rotate --all
  ```
* `--quiet` 告诉 `indexer` 除非发生错误，否则不输出任何内容。此选项主要用于定时任务或其他脚本作业，除了出现错误外，输出内容无关紧要或不必要。示例如下：

  ```shell
  sudo -u manticore indexer --rotate --all --quiet
  ```
* `--noprogress` 不显示进行中的进度详情。相反，只有在索引完成时才报告最终状态详情（例如已索引的文档数、索引速度等等）。当脚本不是在控制台（或“tty”）上运行时，默认启用此选项。示例用法：

  ```shell
  sudo -u manticore indexer --rotate --all --noprogress
  ```
* `--buildstops <outputfile.text> <N>` 会检查表的源数据，就像正在对该数据进行索引一样，并生成一个被索引术语的列表。换句话说，它生成所有将成为表一部分的可搜索词列表。注意，它不会更新相应的表，只是像索引数据一样处理数据，包括运行使用 [sql_query_pre](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Execution_of_fetch_queries.md#sql_query_pre) 或 [sql_query_post](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Execution_of_fetch_queries.md#sql_query_post) 定义的查询。`outputfile.txt` 将包含词列表，每行一个，按词频从高到低排序，`N` 指定将列出的最大词数。如果该数字足够大以包含表中的每个词，只会返回这么多词。这样生成的词典列表可用于客户端应用中“你是指…”功能，通常配合下面的 `--buildfreqs` 使用。示例：

  ```shell
  sudo -u manticore indexer mytable --buildstops word_freq.txt 1000
  ```

  这将在当前目录下生成文档 `word_freq.txt`，包含 'mytable' 中最常见的 1000 个词，按出现频率由高到低排序。注意，当指定多个表或使用 `--all` 时，文件仅与最后一个索引的表相关（即配置文件中列出的最后一个表）。

* `--buildfreqs` 与 `--buildstops` 一起使用（如果未指定 `--buildstops`，则忽略该选项）。因为 `--buildstops` 提供表中使用的词列表，`--buildfreqs` 会添加表中该词出现的频率，这对于判断某些词是否应被视为停用词（stopwords）非常有用（如果它们出现得过于频繁的话）。它还辅助开发“你是指…”功能，使你能够知道某个词比类似词更常见多少。示例：

  ```shell
  sudo -u manticore indexer mytable --buildstops word_freq.txt 1000 --buildfreqs
  ```

  这会产生上述的 `word_freq.txt`，但在每个词之后会显示它在相应表中出现的次数。

* `--merge <dst-table> <src-table>` 用于物理合并两个表，例如在采用 [main+delta 方案](../../Creating_a_table/Local_tables/Plain_table.md#Main+delta-scenario) 时，主表很少修改，而 delta 表频繁重建，使用 `--merge` 来合并这两个表。合并操作从右向左进行 — 会检查 `src-table` 的内容并将其物理合并到 `dst-table` 中，结果保留在 `dst-table`。伪代码形式表达为：`dst-table += src-table` 一个示例：

  ```shell
  sudo -u manticore indexer --merge main delta --rotate
  ```

  在上述示例中，`main` 是主表，几乎不修改，`delta` 是更频繁修改的表，使用以上命令调用 `indexer` 将 `delta` 表的内容合并进 `main` 表并旋转索引。

* `--merge-dst-range <attr> <min> <max>` 在合并时应用过滤范围。具体来说，由于合并是应用在目标表上（配合 `--merge` 使用，如果未指定 `--merge` 则忽略此选项），`indexer` 还会对进入目标表的文档进行筛选，只有通过给定过滤条件的文档才会保留在最终表中。比如有个表带有名为 'deleted' 的属性，其中 0 表示“未删除”。这样的表可以用如下命令合并：

  ```shell
  sudo -u manticore indexer --merge main delta --merge-dst-range deleted 0 0
  ```

  标记为已删除的文档（值为1）将从合并后的目标表中移除。该选项可在命令行中多次添加，以增加连续的过滤条件，所有条件都需满足文档才能成为最终表的一部分。

* --`merge-killlists`（及其简写别名 `--merge-klists`）改变合并表时 kill 列表的处理方式。默认情况下，合并后两个表的 kill 列表都会丢弃，这支持典型的 main+delta 合并场景。然而启用此选项后，两个表的 kill 列表会被拼接并存储到目标表中。注意，源（delta）表的 kill 列表始终用于抑制目标（main）表中的行。
* `--keep-attrs` 允许在重新索引时重用已有属性。每当表被重新构建时，会检测每个新文档id是否在“旧”表中存在，若存在，其属性会被转移到“新”表；若不存在，则使用新表中的属性。如果用户更新了表中的属性，但没有更新用于构建表的实际数据源，重新索引时所有更新都会丢失；使用 `--keep-attrs` 可以保存之前版本表中的更新属性值。可以指定路径以使用该路径下的表文件，而不是配置文件中的参考路径：

  ```shell
  sudo -u manticore indexer mytable --keep-attrs=/path/to/index/files
  ```

* `--keep-attrs-names=<attributes list>` 允许指定需要从已有表重用的属性列表。默认情况下，所有原表属性都会被重用：

  ```shell
  sudo -u manticore indexer mytable --keep-attrs=/path/to/table/files --keep-attrs-names=update,state
  ```

* `--dump-rows <FILE>` 将 SQL 源获取到的行数据以 MySQL 兼容的语法导出到指定文件。生成的转储是 `indexer` 接收到的数据的精确表示，有助于重复索引时的问题排查。该命令执行从源获取数据，并创建表文件和转储文件。
* `--print-rt <rt_index> <table>` 将从源获取的数据以对实时表的 INSERT 语句形式输出。转储的前几行会包含实时字段和属性（作为普通表字段和属性的反映）。该命令执行从源获取数据，并创建表文件和转储输出。该命令可用如下方式使用：`sudo -u manticore indexer -c manticore.conf --print-rt indexrt indexplain > dump.sql`。仅支持基于 SQL 的源。不支持 MVA。
* `--sighup-each` 当你要重建多个大型表时非常有用，能让每个表尽快旋转到 `searchd`。使用 `--sighup-each` 时，`indexer` 在成功完成每个表的工作后会给 `searchd` 发送 SIGHUP 信号。（默认行为是在所有表构建完成后只发送一个 SIGHUP）。
* `--nohup` 当你想在实际旋转表之前用 indextool 检查表时非常有用。如果开启了此选项，indexer 不会发送 SIGHUP。表文件会被重命名为 .tmp。使用 indextool 将表文件重命名为 .new 并旋转它。示例用法：

  ```shell
  sudo -u manticore indexer --rotate --nohup mytable
  sudo -u manticore indextool --rotate --check mytable
  ```

* `--print-queries` 打印出 `indexer` 发送到数据库的 SQL 查询，以及 SQL 连接和断开事件。这有助于诊断和修复 SQL 源的问题。
* `--help`（简写为 `-h`）列出 `indexer` 可以调用的所有参数。
* `-v` 显示 `indexer` 版本。

### Indexer 配置设置
你也可以在 Manticore 配置文件的 `indexer` 部分配置 indexer 行为：

```ini
indexer {
...
}
```

#### lemmatizer_cache

```ini
lemmatizer_cache = 256M
```
形态还原缓存大小。可选，默认值为 256K。

我们的 [形态还原器](../../Server_settings/Common.md#lemmatizer_base) 实现使用压缩词典格式，支持空间与速度的权衡。它可以直接对压缩数据进行形态还原，使用更多 CPU 但更少 RAM，或者可以部分或全部解压并预缓存词典，使用更少 CPU 但更多 RAM。lemmatizer_cache 指令让你控制用于未压缩词典缓存的确切 RAM 大小。

当前可用的词典只有 [ru.pak、en.pak 和 de.pak](https://manticoresearch.com/install/)，分别是俄语、英语和德语词典。压缩词典大小大约为 2 到 10 MB。请注意词典会一直驻留内存中。默认缓存大小为 256 KB。可接受的缓存大小范围是 0 到 2047 MB。增大缓存大小是安全的；形态还原器只会使用所需的内存。例如整个俄语词典解压后大约是 110 MB；因此设置`lemmatizer_cache` 超过此值不会影响内存使用。即使允许缓存为 1024 MB，但实际只需 110 MB，它也只会使用这 110 MB。

#### max_file_field_buffer

```ini
max_file_field_buffer = 128M
```

文件字段自适应缓冲区最大大小，单位字节。可选，默认值为 8MB，最小值为 1MB。

文件字段缓冲区用于加载从 [sql_file_field](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_file_field) 列引用的文件。该缓冲区是自适应的，初次分配时为 1 MB，每步按 2 倍增长，直到能加载完整文件内容或达到 `max_file_field_buffer` 指定的最大缓冲大小。

因此，如果未指定文件字段，则不会分配缓冲区。如果索引期间加载的所有文件大小都小于（例如）2 MB，但 max_file_field_buffer 值为 128 MB，则峰值缓冲区使用仍为 2 MB。然而，超过 128 MB 的文件则会被完全跳过。

#### max_iops

```ini
max_iops = 40
```

每秒最大 I/O 操作数，用于 I/O 限速。可选，默认值为 0（无限制）。

I/O 限速相关选项。限制任意一秒内最多允许的 I/O 操作数（读或写）。值为 0 表示无限制。

`indexer` 可能在构建表时引发磁盘 I/O 突发高负载，可能希望限制其磁盘活动（并为同机运行的其他程序，如 `searchd` 留出资源）。I/O 限速帮助实现这一点。其通过强制控制相继磁盘 I/O 操作之间的最小保证延迟来工作。对 I/O 进行限速有助于减少构建过程导致的搜索性能下降。该设置对其他类型的数据注入无效，如插入实时表数据。

#### max_iosize

```ini
max_iosize = 1048576
```

用于 I/O 限速的单次最大允许 I/O 操作大小，单位字节。可选，默认值为 0（无限制）。

I/O 限速相关选项。限制 `indexer` 执行的所有文件 I/O 操作（读或写）的最大大小。值为 0 表示无限制。大于限制的读写操作将被拆分为几个更小的操作，并在 [max_iops](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#max_iops) 设置中被计为多个操作。在撰写时，所有 I/O 调用应当都小于 256 KB（默认内部缓冲区大小），因此 max_iosize 设置大于 256 KB 不应有影响。

#### max_xmlpipe2_field

```ini
max_xmlpipe2_field = 8M
```

XMLpipe2 源类型允许的最大字段大小，单位字节。可选，默认值为 2 MB。

#### mem_limit

```ini
mem_limit = 256M
# mem_limit = 262144K # same, but in KB
# mem_limit = 268435456 # same, but in bytes
```

Plain table building RAM usage limit. Optional, default is 128 MB. 强制性的内存使用限制， `indexer` 不会超过该限制。可以以字节、千字节（使用 K 后缀）或兆字节（使用 M 后缀）指定；见示例。如果设置为极低值导致 I/O 缓冲区小于 8 KB，该限制将被自动提高；具体下限取决于构建数据的大小。如果缓冲区小于 256 KB，将发出警告。

最大可能的限制是 2047M。过低的值会影响纯表构建速度，但 256M 到 1024M 应该足够大多数（如果不是全部）数据集。设置过高可能导致 SQL 服务器超时。在文档收集阶段，会有内存缓冲区部分排序且没有与数据库通信的时期；数据库服务器可能会超时。您可以通过提高 SQL 服务器端的超时设置或降低 `mem_limit` 来解决。

#### on_file_field_error

```ini
on_file_field_error = skip_document
```

如何处理文件字段中的 IO 错误。可选，默认是 `ignore_field`。
当索引一个由文件字段引用的文件出现问题时（参见 [sql_file_field](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_file_field)），`indexer` 可以处理文档，假设这个字段为空内容，也可以跳过该文档，或者完全失败索引。`on_file_field_error` 指令控制该行为。它可取的值有：
* `ignore_field`，处理当前文档但不包含该字段；
* `skip_document`，跳过当前文档但继续索引；
* `fail_index`，索引失败并显示错误信息。

可能出现的问题有：打开错误、大小错误（文件过大）和数据读取错误。无论阶段和 `on_file_field_error` 设置如何，任何问题都会发出警告信息。

注意，使用 `on_file_field_error = skip_document` 时，只有在早期检查阶段检测到问题时，文档才会被忽略，**而不是**在实际文件解析阶段。`indexer` 会在进行任何工作前先打开每个引用的文件并检查其大小，然后在实际解析时再次打开。因此，如果文件在两次打开之间消失，文档仍然会被索引。

#### write_buffer

```ini
write_buffer = 4M
```

写缓冲区大小，单位字节。可选，默认是 1MB。写缓冲用于索引时写入临时表文件和最终表文件。较大的缓冲减少所需的磁盘写入次数。缓冲区内存是在 [mem_limit](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#mem_limit) 之外分配的。注意会为不同文件分别分配若干（目前最多 4 个）缓冲，成比例增加 RAM 使用。

#### ignore_non_plain

```ini
ignore_non_plain = 1
```

`ignore_non_plain` 允许您完全忽略关于跳过非纯表的警告。默认值为 0（不忽略）。
<!-- proofread -->


### 通过 systemd 调度 indexer

有两种方式调度 indexer 运行。第一种是使用 crontab 的经典方法。第二种是使用带有用户定义计划的 systemd 计时器。要创建计时器单元文件，您应将它们放置在 systemd 查找此类单元文件的合适目录中。在大多数 Linux 发行版中，该目录通常是 `/etc/systemd/system`。操作步骤如下：

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
   有关 `OnCalendar` 语法及示例的更多信息，请参见 [这里](https://www.freedesktop.org/software/systemd/man/latest/systemd.time.html#Calendar%20Events)。

2. 根据您的特定需求编辑计时器单元文件。
3. 启用计时器：
   ```shell
   systemctl enable manticore-indexer@idx1.timer
   ```
4. 启动计时器：
   ```shell
   systemctl start manticore-indexer@idx1.timer
   ```
5. 对任何其他计时器重复步骤 2-4。

