# 普通表创建

普通表是在创建时从一个或多个数据源一次性拉取数据后生成的表。普通表是不可变的，因为在其生命周期内不能添加或删除文档。只能更新数值属性（包括 MVA）的值。要刷新数据，只能通过重新创建整张表来实现。

普通表仅可在 [Plain 模式](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29) 下使用，其定义由一个表声明和一个或多个数据源声明组成。数据抓取和表创建不是由 `searchd` 服务器完成的，而是由辅助工具 `indexer` 完成的。

**Indexer** 是一个命令行工具，可以直接从命令行或 shell 脚本中调用。

它在调用时可以接受多个参数，同时在 Manticore 配置文件中也有若干自身设置。

在典型场景中，indexer 会执行以下操作：
* 从数据源获取数据
* 构建普通表
* 写入表文件
* （可选）通知搜索服务器有了新表，从而触发表轮转

## Indexer 工具
`indexer` 工具用于在 Manticore Search 中创建普通表。其通用语法如下：

```shell
indexer [OPTIONS] [table_name1 [table_name2 [...]]]
```

使用 `indexer` 创建表时，生成的表文件必须具有允许 `searchd` 读取、写入和删除的权限。在官方 Linux 软件包中，`searchd` 以 `manticore` 用户运行。因此，`indexer` 也必须以 `manticore` 用户运行：

```shell
sudo -u manticore indexer ...
```

如果你的 `searchd` 以其他方式运行，可能就不需要省略 `sudo -u manticore`。只要确保运行 `searchd` 实例的用户对使用 `indexer` 生成的表具有读写权限即可。

要创建普通表，你需要列出要处理的表。例如，如果你的 `manticore.conf` 文件中包含两张表 `mybigindex` 和 `mysmallindex` 的信息，你可以运行：

```shell
sudo -u manticore indexer mysmallindex mybigindex
```

你也可以使用通配符来匹配表名：

* `?` 匹配任意单个字符
* `*` 匹配任意数量的任意字符
* `%` 匹配零个或一个任意字符

```shell
sudo -u manticore indexer indexpart*main --rotate
```

indexer 的退出码如下：

* 0：一切正常
* 1：索引过程中出现问题（如果指定了 `--rotate`，则会跳过）或者某个操作发出了警告
* 2：索引正常完成，但 `--rotate` 尝试失败

### Indexer systemd 服务

你也可以使用下面的 systemctl 单元文件启动 `indexer`：

```shell
systemctl start --no-block manticore-indexer
```

或者，如果你只想构建某一张特定的表：

```shell
systemctl start --no-block manticore-indexer@specific-table-name
```

使用 `systemctl set-environment INDEXER_CONFIG` 命令以自定义配置运行 Indexer，这会替换默认设置。

`systemctl set-environment INDEXER_ARGS` 命令可让你为 Indexer 添加自定义启动选项。完整的命令行选项列表请见 [这里](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments)。

例如，要以静默模式启动 Indexer，请运行：
```bash
systemctl set-environment INDEXER_ARGS='--quiet'
systemctl restart manticore-indexer
```

要恢复这些更改，请运行：
```bash
systemctl set-environment INDEXER_ARGS=''
systemctl restart manticore-indexer
```

### Indexer 命令行参数
* `--config <file>`（简写为 `-c <file>`）告诉 `indexer` 使用指定文件作为配置。通常，它会先在安装目录中查找 `manticore.conf`（例如 `/etc/manticoresearch/manticore.conf`），然后再查找你从 shell 调用 `indexer` 时所在的当前目录。这在共享环境中最有用，因为二进制文件安装在全局目录中，例如 `/usr/bin/`，但你又希望允许用户拥有各自定制的 Manticore 配置，或者你想在单台服务器上运行多个实例。在这类情况下，你可以让他们创建自己的 `manticore.conf` 文件，并通过此选项传给 `indexer`。例如：

  ```shell
  sudo -u manticore indexer --config /home/myuser/manticore.conf mytable
  ```

* `--all` 告诉 `indexer` 更新 `manticore.conf` 中列出的所有表，而不是逐个列出表名。这在小型配置或类似 cron 的维护任务中很有用，因为整套表会在每天、每周或按其他合适周期全部重建。请注意，由于 `--all` 会尝试更新配置中找到的所有表，如果遇到 RealTime 表，它会发出警告，并且即使普通表都成功完成，命令的退出码也会是 `1` 而不是 `0`。示例用法：

  ```shell
  sudo -u manticore indexer --config /home/myuser/manticore.conf --all
  ```

* `--rotate` 用于轮转表。除非你可以把搜索功能下线而不会影响用户，否则在索引新文档时几乎肯定需要保持搜索持续运行。`--rotate` 会创建第二张表，与第一张并行存在（位置相同，只是在文件名中加入 `.new`）。完成后，indexer 会通过发送 `SIGHUP` 信号通知 `searchd`，随后 `searchd` 会尝试重命名这些表（把现有表改名为带 `.old`，再把 `.new` 的文件重命名为替代它们），然后开始使用较新的文件提供服务。根据 [seamless_rotate](../../Server_settings/Searchd.md#seamless_rotate) 的设置，新表可被搜索可能会有轻微延迟。若一次轮转多个通过 [killlist_target](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target) 关系串联的表，轮转会从不是目标的表开始，最后结束于目标链末端的表。示例用法：

  ```shell
  sudo -u manticore indexer --rotate --all
  ```
* `--quiet` 告诉 `indexer` 除非出错，否则不要输出任何内容。这主要用于 cron 类型或其他脚本化任务，在这些场景下输出通常无关紧要或不需要，除非发生某种错误。示例用法：

  ```shell
  sudo -u manticore indexer --rotate --all --quiet
  ```
* `--noprogress` 不显示执行过程中的进度详情。相反，最终状态信息（例如已索引文档数、索引速度等）只会在索引完成时报告。在脚本不是在控制台（或 `tty`）上运行时，该选项默认启用。示例用法：

  ```shell
  sudo -u manticore indexer --rotate --all --noprogress
  ```
* `--remove_dups` 在构建普通表时逻辑上移除重复的数值文档 ID。每个 ID 首次索引到的行会保持可见；后续具有相同 ID 的行会被标记为失效。如果不使用此选项，`indexer` 会保留重复 ID。示例用法：

  ```shell
  sudo -u manticore indexer --config /home/myuser/manticore.conf --remove_dups mytable
  ```
* `--buildstops <outputfile.text> <N>` 会检查表源，就像正在索引数据一样，并生成正在被索引的词项列表。换句话说，它会生成所有将成为表一部分的可搜索词项列表。注意，它不会更新相应的表，只是像索引一样处理数据，包括执行由 [sql_query_pre](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Execution_of_fetch_queries.md#sql_query_pre) 或 [sql_query_post](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Execution_of_fetch_queries.md#sql_query_post) 定义的查询。`outputfile.txt` 将包含词语列表，每行一个，并按频率排序，最常见的词排在最前面，`N` 指定要列出的最大词数。如果它足够大，足以包含表中的每个词，那么只会返回那么多词。这样的词典列表可用于客户端应用中的 “Did you mean…” 功能，通常会与下面的 `--buildfreqs` 结合使用。示例：

  ```shell
  sudo -u manticore indexer mytable --buildstops word_freq.txt 1000
  ```

  这会在当前目录下生成一个文档 `word_freq.txt`，其中包含 `mytable` 中最常见的 1,000 个词，按出现频率从高到低排列。请注意，当与多个表或 `--all` 一起指定时，该文件对应的是最后一个被索引的表（即配置文件中列出的最后一个表）

* `--buildfreqs` 与 `--buildstops` 配合使用（如果未指定 `--buildstops`，则会被忽略）。由于 `--buildstops` 提供的是表中使用的词列表，`--buildfreqs` 会再加入每个词在表中的数量，这对于判断某些词是否因为过于常见而应被视为停用词很有帮助。它还可以帮助开发 “Did you mean…” 功能，因为你需要知道某个词相对于另一个相似词到底常见多少。比如：

  ```shell
  sudo -u manticore indexer mytable --buildstops word_freq.txt 1000 --buildfreqs
  ```

  这会生成上面的 `word_freq.txt`，不过每个词后面都会附上它在该表中出现的次数。

* `--merge <dst-table> <src-table>` 用于将表进行物理合并，例如你有一个 [main+delta 方案](../../Creating_a_table/Local_tables/Plain_table.md#Main+delta-scenario)，其中主表很少变动，而 delta 表则频繁重建，这时就可以使用 `--merge` 将两者合并。该操作是从右向左进行的 - `src-table` 的内容会被检查并与 `dst-table` 的内容物理合并，结果保留在 `dst-table` 中。用伪代码表示，可以写成：`dst-table += src-table` 示例：

  ```shell
  sudo -u manticore indexer --merge main delta --rotate
  ```

  在上面的示例中，主表是 master，即很少修改的表，而 delta 是修改更频繁的表，你可以用上面的命令调用 `indexer`，将 delta 的内容合并到主表中并轮转这些表。

* `--merge-dst-range <attr> <min> <max>` 在合并时对给定的范围过滤器进行处理。具体来说，由于合并会应用到目标表上（作为 `--merge` 的一部分，若未指定 `--merge` 则会被忽略），`indexer` 还会过滤最终进入目标表的文档，只有通过所给过滤条件的文档才会进入最终表。比如，这可用于带有 `deleted` 属性的表，其中 0 表示“未删除”。这样的表可以这样合并：

  ```shell
  sudo -u manticore indexer --merge main delta --merge-dst-range deleted 0 0
  ```

  任何标记为已删除的文档（值为 1）都会从新合并的目标表中移除。该选项可以在命令行中多次添加，以便为合并加入连续的过滤条件，文档必须同时满足所有条件才能成为最终表的一部分。

* --`merge-killlists`（以及较短的别名 `--merge-klists`）会改变合并表时处理 kill list 的方式。默认情况下，合并后两边的 kill list 都会被丢弃。这支持最常见的 main+delta 合并场景。启用此选项后，两个表中的 kill list 会被拼接并存储到目标表中。请注意，源表（delta 表）的 kill list 会始终用于抑制目标表（main 表）中的行。
* `--keep-attrs` 允许在重新索引时复用现有属性。每次重建表时，都会检查每个新文档 ID 在“旧”表中是否存在；如果已存在，则其属性会转移到“新”表中；如果不存在，则使用新表中的属性。如果用户已经在表中更新了属性，但没有同步更新作为表来源的实际数据源，那么重新索引时所有更新都会丢失；使用 `--keep-attrs` 可以保存上一张表中的已更新属性值。还可以指定一个用于表文件的路径，而不是配置中的参考路径：

  ```shell
  sudo -u manticore indexer mytable --keep-attrs=/path/to/index/files
  ```

* `--keep-attrs-names=<attributes list>` 允许你指定在重新索引时从现有表复用哪些属性。默认情况下，现有表中的所有属性都会在新表中复用：

  ```shell
  sudo -u manticore indexer mytable --keep-attrs=/path/to/table/files --keep-attrs-names=update,state
  ```

* `--dump-rows <FILE>` 将从 SQL 数据源获取的行以 MySQL 兼容语法转储到指定文件中。生成的转储是 `indexer` 接收到的数据的精确表示，可帮助复现索引阶段的问题。该命令会从源中抓取数据，并同时创建表文件和转储文件。
* `--print-rt <rt_index> <table>` 会将从数据源获取的数据以适用于实时表的 INSERT 语句形式输出。转储的前几行会包含实时表字段和属性（作为普通表字段和属性的映射）。该命令会从源中抓取数据，并同时创建表文件和转储输出。命令可写成 `sudo -u manticore indexer -c manticore.conf --print-rt indexrt indexplain > dump.sql`。仅支持基于 SQL 的数据源。不支持 MVA。
* `--sighup-each` 在你重建许多大型表，并希望每张表尽快轮转进 `searchd` 时非常有用。使用 `--sighup-each` 后，`indexer` 会在成功完成每张表的工作后向 `searchd` 发送 SIGHUP 信号。（默认行为是在所有表构建完成后只发送一次 SIGHUP）。
* `--nohup` 在你想先用 indextool 检查表、然后再真正轮转时很有用。启用此选项后，indexer 不会发送 SIGHUP。表文件会被重命名为 `.tmp`。请使用 indextool 将表文件重命名为 `.new` 并完成轮转。示例用法：

  ```shell
  sudo -u manticore indexer --rotate --nohup mytable
  sudo -u manticore indextool --rotate --check mytable
  ```

* `--print-queries` 会打印 `indexer` 向数据库发送的 SQL 查询，以及 SQL 连接和断开事件。这对于诊断和修复 SQL 数据源问题很有帮助。
* `--help`（简写为 `-h`）列出 `indexer` 可调用的所有参数。
* `-v` 显示 `indexer` 版本。

### Indexer 配置设置
你也可以在 Manticore 配置文件的 `indexer` 段中配置 indexer 的行为：

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

我们的 [lemmatizer](../../Server_settings/Common.md#lemmatizer_base) 实现使用压缩词典格式，以实现空间与速度之间的权衡。它既可以直接基于压缩数据进行词形还原，消耗更多 CPU 但占用更少 RAM；也可以将词典部分或全部解压并预缓存，从而占用更少 CPU 但更多 RAM。`lemmatizer_cache` 指令可让你精确控制这部分未压缩词典缓存可使用多少 RAM。

目前可用的词典只有 [ru.pak、en.pak 和 de.pak](https://manticoresearch.com/install/)。它们分别是俄语、英语和德语词典。压缩词典的大小大约在 2 到 10 MB 之间。请注意，词典本身也会始终驻留在内存中。默认缓存大小为 256 KB。可接受的缓存大小范围是 0 到 2047 MB。即使把缓存大小设得很高也不会有问题；lemmatizer 只会使用实际需要的内存。例如，整个俄语词典解压后大约为 110 MB，因此将 `lemmatizer_cache` 设得高于这个值不会影响内存使用。即便允许缓存达到 1024 MB，如果只需要 110 MB，它也只会使用这 110 MB。

#### max_file_field_buffer

```ini
max_file_field_buffer = 128M
```

文件字段自适应缓冲区的最大大小，单位为字节。可选，默认值为 8MB，最小值为 1MB。

文件字段缓冲区用于加载来自 [sql_file_field](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_file_field) 列所引用的文件。这个缓冲区是自适应的，首次分配时从 1 MB 开始，并以 2 倍步长增长，直到文件内容可以加载完成或达到 `max_file_field_buffer` 指令指定的最大缓冲区大小为止。

因此，如果没有指定任何文件字段，就根本不会分配缓冲区。如果索引期间加载的所有文件都小于（例如）2 MB，但 `max_file_field_buffer` 的值为 128 MB，那么峰值缓冲区使用量仍然只有 2 MB。不过，超过 128 MB 的文件会被完全跳过。

#### max_iops

```ini
max_iops = 40
```

每秒最大 I/O 操作数，用于 I/O 限流。可选，默认值为 0（无限制）。

这是一个与 I/O 限流相关的选项。它限制任意给定秒内 I/O 操作（读或写）的最大次数。值为 0 表示不施加限制。

`indexer` 在构建表时可能会导致突发性的密集磁盘 I/O，因此有时希望限制它的磁盘活动（并为同一台机器上运行的其他程序保留一些资源，例如 `searchd`）。I/O 限流可以帮助实现这一点。它通过强制 `indexer` 执行相邻磁盘 I/O 操作之间的最小保证延迟来工作。对 I/O 进行限流有助于降低因构建而导致的搜索性能下降。此设置对其他类型的数据导入无效，例如向实时表插入数据。

#### max_iosize

```ini
max_iosize = 1048576
```

用于 I/O 限流的最大允许 I/O 操作大小，单位为字节。可选，默认值为 0（无限制）。

这是一个与 I/O 限流相关的选项。它限制 `indexer` 执行的所有操作中，单次文件 I/O 操作（读或写）的最大大小。值为 0 表示不施加限制。大于该限制的读写会被拆分为多个更小的操作，并由 [max_iops](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#max_iops) 设置计为多次操作。就本文撰写时而言，所有 I/O 调用本来也都应小于 256 KB（默认内部缓冲区大小），因此高于 256 KB 的 `max_iosize` 值不应产生任何影响。

#### max_xmlpipe2_field

```ini
max_xmlpipe2_field = 8M
```

XMLpipe2 数据源类型允许的最大字段大小，单位为字节。可选，默认值为 2 MB。

#### mem_limit

```ini
mem_limit = 256M
# mem_limit = 262144K # same, but in KB
# mem_limit = 268435456 # same, but in bytes
```

普通表构建时的 RAM 使用上限。可选，默认值为 128 MB。强制执行的内存使用限制，`indexer` 不会超过这个值。可按字节、千字节（使用 K 后缀）或兆字节（使用 M 后缀）指定；请参见示例。如果该值设置得极低，导致 I/O 缓冲区小于 8 KB，则会自动提高该限制；这一精确下限取决于已构建数据的大小。如果缓冲区小于 256 KB，则会发出警告。

最大可能限制为 2047M。值太低会影响普通表构建速度，但对于大多数数据集来说，256M 到 1024M 应该已经足够，甚至包括所有数据集。将该值设得过高可能导致 SQL 服务器超时。在文档收集阶段，内存缓冲区会有部分排序且不会与数据库通信的时间段；这时数据库服务器可能超时。你可以通过提高 SQL 服务器侧的超时时间，或者降低 `mem_limit` 来解决。

#### on_file_field_error

```ini
on_file_field_error = skip_document
```

如何处理文件字段中的 IO 错误。可选，默认值为 `ignore_field`。
当索引引用自文件字段（[sql_file_field](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_file_field)）的文件时出现问题，`indexer` 可以选择按该字段内容为空继续处理文档，或者跳过文档，或者直接让索引失败。`on_file_field_error` 指令控制这种行为。其可取值如下：
* `ignore_field`，在没有该字段内容的情况下处理当前文档；
* `skip_document`，跳过当前文档但继续索引；
* `fail_index`，以错误信息使索引失败。

可能出现的问题包括：打开错误、大小错误（文件过大）以及数据读取错误。无论处于哪个阶段，也无论 `on_file_field_error` 如何设置，任何问题都会始终给出警告信息。

请注意，当 `on_file_field_error = skip_document` 时，只有在早期检查阶段检测到问题时，文档才会被忽略，而**不会**在实际文件解析阶段被忽略。`indexer` 会先打开每个引用文件并在做任何工作之前检查其大小，然后在实际解析时再打开一次。因此，如果文件在这两次打开尝试之间消失，文档仍然会被索引。

#### write_buffer

```ini
write_buffer = 4M
```

写缓冲区大小，单位为字节。可选，默认值为 1MB。索引时会使用写缓冲区来写入临时表文件和最终表文件。更大的缓冲区可以减少所需的磁盘写入次数。缓冲区内存是在 [mem_limit](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#mem_limit) 之外额外分配的。请注意，不同文件会分配多个缓冲区（当前最多 4 个），从而按比例增加 RAM 使用量。

#### ignore_non_plain

```ini
ignore_non_plain = 1
```

`ignore_non_plain` 允许你完全忽略跳过非普通表时产生的警告。默认值为 0（不忽略）。
<!-- proofread -->


### 通过 systemd 安排 indexer 计划任务

安排 indexer 运行有两种方式。第一种是使用 crontab 的传统方法。第二种是使用带有自定义计划的 systemd 定时器。要创建定时器单元文件，应将其放在 systemd 查找此类单元文件的相应目录中。在大多数 Linux 发行版上，这个目录通常是 `/etc/systemd/system`。具体做法如下：

1. 为你的自定义计划创建一个定时器单元文件：
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
   `OnCalendar` 语法及示例可在 [这里](https://www.freedesktop.org/software/systemd/man/latest/systemd.time.html#Calendar%20Events) 查看。

2. 按你的具体需求编辑该定时器单元。
3. 启用该定时器：
   ```shell
   systemctl enable manticore-indexer@idx1.timer
   ```
4. 启动该定时器：
   ```shell
   systemctl start manticore-indexer@idx1.timer
   ```
5. 对任何额外的定时器重复步骤 2-4。
