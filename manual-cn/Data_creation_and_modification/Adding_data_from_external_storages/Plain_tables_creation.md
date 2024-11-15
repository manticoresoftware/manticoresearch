# 创建普通表

普通表是一次性创建的，通过从一个或多个源获取数据构建而成。在其生命周期内，普通表是不可变的，不能添加或删除文档。唯一可以更新的是数值属性（包括多值属性 MVA）。要刷新数据，唯一的方式是重新创建整个表。

普通表仅在[普通模式](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-(Plain-mode))下可用，定义包括一个表声明以及一个或多个源声明。数据的收集与表的创建不是由 `searchd` 服务端完成，而是通过辅助工具 `indexer` 来进行。

**Indexer** 是一个命令行工具，可以直接从命令行或脚本中调用。

在调用时可以接受多个参数，同时在 Manticore 配置文件中也有一些特定的设置。

在典型的场景下，indexer 进行以下操作：

- 从源获取数据
- 构建普通表
- 写入表文件
- （可选）通知搜索服务关于新表的信息，并触发表旋转

## Indexer 工具
`indexer` 工具用于在 Manticore Search 中创建普通表。其基本语法为：

```shell
indexer [OPTIONS] [table_name1 [table_name2 [...]]]
```

使用 `indexer` 创建表时，生成的表文件必须设置适当的权限，以允许 `searchd` 读取、写入和删除文件。在官方的 Linux 包中，`searchd` 以 `manticore` 用户身份运行。因此，`indexer` 也必须以 `manticore` 用户身份运行：

```shell
sudo -u manticore indexer ...
```

如果你以其他方式运行 `searchd`，可能不需要 `sudo -u manticore`。只需确保运行 `searchd` 实例的用户具有读取/写入 `indexer` 生成表的权限即可。

要创建普通表，你需要列出要处理的表。例如，如果你的 `manticore.conf` 文件中包含了两个表 `mybigindex` 和 `mysmallindex` 的详细信息，你可以运行：

```shell
sudo -u manticore indexer mysmallindex mybigindex
```

你还可以使用通配符来匹配表名：

- `?` 匹配任意单个字符
- `*` 匹配任意数量的字符
- `%` 匹配零个或一个字符

```shell
sudo -u manticore indexer indexpart*main --rotate
```

indexer 的退出代码如下：

- 0：一切正常
- 1：索引过程中出现问题（如果指定了 `--rotate`，则跳过旋转）或操作发出警告
- 2：索引成功，但 `--rotate` 尝试失败

### Indexer systemd 服务

你还可以通过以下 systemctl 单元文件启动 `indexer`：

```shell
systemctl start --no-block manticore-indexer
```

或者，如果你想构建特定表：

```shell
systemctl start --no-block manticore-indexer@specific-table-name
```

使用 `systemctl set-environment INDEXER_CONFIG` 命令可以用自定义配置运行 Indexer，替换默认设置。

`systemctl set-environment INDEXER_ARGS` 命令允许添加自定义启动选项。有关完整的命令行选项列表，请参阅[此处](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-命令行参数)。

例如，要以安静模式启动 Indexer，可以运行：

```bash
systemctl set-environment INDEXER_ARGS='--quiet'
systemctl restart manticore-indexer
```

要恢复更改，请运行：
```bash
systemctl set-environment INDEXER_ARGS=''
systemctl restart manticore-indexer
```

### Indexer 命令行参数
* `--config <file>` (`-c <file>` 简写) 告诉 `indexer` 使用给定的配置文件。通常，它会在安装目录（例如 `/etc/manticoresearch/manticore.conf`）中查找 `manticore.conf`，其次是你从 shell 调用 `indexer` 时的当前目录。如果你在共享环境中运行，或希望允许用户创建自己的 Manticore 设置，可以为每个用户提供自己的 `manticore.conf` 文件并通过此选项传递给 `indexer`。

  ```shell
  sudo -u manticore indexer --config /home/myuser/manticore.conf mytable
  ```

* `--all` 命令会更新 `manticore.conf` 中列出的所有表，而不是单独列出表。这在小型配置或定期维护任务中非常有用。请注意，如果配置中包含实时表，它将发出警告并退出代码为 `1`，即使普通表已成功完成也会如此。

  ```shell
  sudo -u manticore indexer --config /home/myuser/manticore.conf --all
  ```

* `--rotate` 用于表旋转。除非你可以在不影响用户的情况下使搜索功能离线，否则你几乎肯定需要在索引新文档时保持搜索运行。`--rotate` 会创建一个与原始表平行的第二个表（在同一位置，文件名中包含 `.new`）。一旦完成，`indexer` 会通过发送 `SIGHUP` 信号通知 `searchd`，`searchd` 会尝试重命名这些表（将现有表重命名为 `.old`，并将 `.new` 文件重命名替换它们），然后开始使用新的文件进行服务。具体取决于 [seamless_rotate](../../Server_settings/Searchd.md#seamless_rotate) 的设置，在搜索新表之前可能会有轻微的延迟。如果多个表通过 [killlist_target](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target) 关系链接在一起，旋转将从非目标表开始，并在目标链的末端完成。使用示例：

  ```shell
  sudo -u manticore indexer --rotate --all
  ```
* `--quiet` 命令告诉 `indexer` 在除非出现错误的情况下不输出任何信息。这主要用于定时任务或其他脚本化的工作，在这些情况下输出信息是多余的，除非出现问题。使用示例：

  ```shell
  sudo -u manticore indexer --rotate --all --quiet
  ```
* `--noprogress` 禁止在索引时显示进度详情，直到索引完成后才显示最终状态（例如已索引文档数量、索引速度等）。在非控制台（或 ‘tty’）环境中运行脚本时，此选项将默认启用。使用示例：

  ```shell
  sudo -u manticore indexer --rotate --all --noprogress
  ```
* `--buildstops <输出文件.txt> <N>` 处理表源，如同它在为数据创建索引，生成即将被索引的术语列表，换句话说，它生成表中成为索引一部分的所有可搜索术语列表。注意，它并不更新表，仅处理数据以生成词频列表。`outputfile.txt` 会包含每行一个词，并按频率排序，最常见的词排在前面。`N` 指定列表中的最大词数。可以与 `--buildfreqs` 一起使用，如下所示。示例：

  ```shell
  sudo -u manticore indexer mytable --buildstops word_freq.txt 1000
  ```

  此命令会在当前目录中生成一个名为 `word_freq.txt` 的文件，列出 `mytable` 中最常见的 1000 个词，按出现频率排序。

* `--buildfreqs` 仅与 `--buildstops` 一起使用，并在结果中添加每个词在表中出现的次数。这有助于判断某个词是否过于频繁，是否应该被视为停用词。示例：

  ```shell
  sudo -u manticore indexer mytable --buildstops word_freq.txt 1000 --buildfreqs
  ```

  此命令会生成 `word_freq.txt` 文件，并在每个词后面列出其在表中出现的次数。

* `--merge <目标表> <源表>` 用于物理合并两个表，例如在 [main+delta 方案](../../Creating_a_table/Local_tables/Plain_table.md#Main+delta-scenario) 中，主表很少更改，而增量表经常被重新构建，`--merge` 用于将两者合并。操作从右到左执行——`src-table` 的内容会与 `dst-table` 的内容合并，结果保存在 `dst-table` 中。伪代码表达如下：`dst-table += src-table`。示例：

  ```shell
  sudo -u manticore indexer --merge main delta --rotate
  ```

  在上述例子中，主表是主要表，较少修改，增量表则较为频繁更新。你可以使用此命令来合并主表和增量表的内容，并旋转表。

* `--merge-dst-range <属性> <最小值> <最大值>` 在合并时运行给定的过滤范围。具体来说，此命令作为 `--merge` 的一部分使用（如果没有指定 `--merge` 则忽略），`indexer` 会过滤目标表中的文档，只有通过筛选条件的文档才会保留在最终表中。该命令可用于带有“删除”属性的表，0 表示未删除。示例：

  ```shell
  sudo -u manticore indexer --merge main delta --merge-dst-range deleted 0 0
  ```

  此命令将移除合并后表中所有被标记为删除的文档（值为 1）。你可以多次在命令行中添加此参数，添加多个筛选条件，所有条件都必须满足，文档才能进入最终表。

* `--merge-killlists` 或其简写 `--merge-klists` 更改合并时处理删除列表的方式。默认情况下，合并后两个删除列表都会被丢弃，以支持最常见的 main+delta 合并场景。然而，如果启用了此选项，两个表的删除列表会被拼接并存储在目标表中。
* `--keep-attrs` 允许在重新索引时重用现有的属性。每当重建表时，`indexer` 会检查每个新文档的 ID 是否存在于“旧”表中，如果存在，则其属性将转移到“新”表中；如果不存在，则使用新表的属性。如果用户已经更新了表中的属性，但没有更新用于该表的实际源数据，那么重新索引时所有更新将会丢失。使用 `--keep-attrs` 可以保存之前表中已更新的属性值。你可以指定用于表文件的路径，以替代配置文件中的参考路径：

  ```shell
  sudo -u manticore indexer mytable --keep-attrs=/path/to/index/files
  ```

* `--keep-attrs-names=<属性列表>` 允许你在重新索引时指定需要重用的属性。默认情况下，所有现有表中的属性都会在新表中重用：

  ```shell
  sudo -u manticore indexer mytable --keep-attrs=/path/to/table/files --keep-attrs-names=update,state
  ```

* `--dump-rows <文件>` 将 SQL 源提取的行数据导出到指定文件中，使用 MySQL 兼容的语法。生成的导出文件是 `indexer` 接收到的数据的精确表示，可以帮助解决索引期间的问题。此命令从源提取数据，并创建表文件及导出文件。
* `--print-rt <rt_index> <table>` 将源数据输出为 INSERT 语句，以便用于实时表。导出的第一行会包含实时表的字段和属性（作为普通表字段和属性的映射）。此命令从源提取数据，并创建表文件及导出输出。可以使用此命令 `sudo -u manticore indexer -c manticore.conf --print-rt indexrt indexplain > dump.sql`。仅支持 SQL 源，MVA 不支持。
* `--sighup-each` 在你重建多个大型表时，每个表完成后立即通知 `searchd` 进行旋转，而不是等待所有表完成后发送单一的 SIGHUP 信号。默认情况下，`indexer` 会在所有表完成构建后发送一次 SIGHUP 信号。
* `--nohup` 在你希望在旋转表之前通过 indextool 检查表时使用。启用此选项后，`indexer` 不会发送 SIGHUP 信号。表文件将被重命名为 `.tmp`。你可以使用 indextool 将表文件重命名为 `.new`，然后进行旋转。示例使用：

  ```shell
  sudo -u manticore indexer --rotate --nohup mytable
  sudo -u manticore indextool --rotate --check mytable
  ```

* `--print-queries` 打印出 `indexer` 发送到数据库的 SQL 查询，以及 SQL 连接和断开的事件。这对于诊断和解决 SQL 源问题非常有用。
* `--help` (`-h` 为简写) 列出 `indexer` 可以调用的所有参数。
* `-v` 显示 `indexer` 的版本。

### indexer 配置设置
你还可以在 Manticore 的配置文件中的 `indexer` 部分中配置 `indexer` 的行为：

```ini
indexer {
...
}
```

#### lemmatizer_cache

```ini
lemmatizer_cache = 256M
```
词干缓存大小。可选，默认值为 256K。

我们的 [词干库](../../Server_settings/Common.md#lemmatizer_base) 实现使用了一种压缩词典格式，这允许在空间和速度之间进行权衡。它可以直接对压缩数据执行词干化（使用更多的 CPU 但占用更少的内存），也可以部分或完全解压并缓存词典，从而减少 CPU 使用，增加内存使用。`lemmatizer_cache` 选项可以让你控制用于缓存解压词典的内存量。

目前可用的词典有 [ru.pak, en.pak 和 de.pak](https://manticoresearch.com/install/)，即俄语、英语和德语词典。压缩词典大约为 2 至 10 MB。注意，词典始终保留在内存中。默认缓存大小为 256 KB。可接受的缓存大小范围为 0 到 2047 MB。即使设置了较高的缓存值，词典也只会使用所需的内存。

#### max_file_field_buffer

```ini
max_file_field_buffer = 128M
```

文件字段自适应缓冲区的最大大小，单位为字节。可选，默认值为 8MB，最小值为 1MB。

文件字段缓冲区用于加载从 [sql_file_field](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_file_field) 列中引用的文件。此缓冲区是自适应的，初始分配为 1MB，并以 2 倍的步长增长，直到文件内容可以加载或达到 `max_file_field_buffer` 指定的最大缓冲区大小。

因此，如果未指定任何文件字段，则不会分配缓冲区。如果索引期间加载的所有文件都小于（例如）2MB，而 `max_file_field_buffer` 值为 128MB，则最大缓冲区使用量仍然只有 2MB。但是，超过 128MB 的文件将被完全跳过。

#### max_iops

```ini
max_iops = 40
```

每秒最大 I/O 操作数，用于 I/O 节流。可选，默认值为 0（无限制）。

这是与 I/O 节流相关的选项。它限制每秒的最大 I/O 操作次数（读取或写入）。当值为 0 时，表示不进行限制。

`indexer` 在构建表时可能会导致磁盘 I/O 的高峰，这时可能需要限制其磁盘活动，以保留资源给系统上运行的其他程序（例如 `searchd`）。I/O 节流可以帮助减少构建过程中对搜索性能的影响。此设置对其他数据摄入方式（例如将数据插入实时表）无效。

#### max_iosize

```ini
max_iosize = 1048576
```

每次 I/O 操作的最大大小（字节），用于 I/O 节流。可选，默认值为 0（无限制）。

这是另一个与 I/O 节流相关的选项。它限制每次文件 I/O 操作（读取或写入）的最大大小。如果值为 0，则不进行限制。对于大于设置的操作，`indexer` 会将其拆分为较小的操作，并将其计为多个操作（依据 [max_iops](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#max_iops) 设置）。当前默认的内部缓冲区大小为 256KB，因此 `max_iosize` 设置高于 256KB 时不会产生任何效果。

#### max_xmlpipe2_field

```ini
max_xmlpipe2_field = 8M
```

XMLpipe2 源类型的字段最大允许大小，单位为字节。可选，默认值为 2MB。

#### mem_limit

```ini
mem_limit = 256M
# mem_limit = 262144K # same, but in KB
# mem_limit = 268435456 # same, but in bytes
```

用于构建普通表时的内存使用限制。可选，默认值为 128 MB。`indexer` 不会超过的内存使用上限。可以用字节、KB（加 K 后缀）或 MB（加 M 后缀）来指定，如例子所示。如果设置的值过低导致 I/O 缓冲区小于 8 KB，`indexer` 会自动提高这个限制，具体的下限取决于生成数据的大小。如果缓冲区小于 256 KB，系统将产生警告。

最大可能的限制是 2047M。设置过低的值会影响普通表的构建速度，但大多数数据集应该可以在 256M 到 1024M 范围内顺利运行。设置过高的值可能导致 SQL 服务器超时。在文档收集阶段，当内存缓冲区部分排序且没有与数据库进行通信时，SQL 服务器可能会超时。可以通过在 SQL 服务器端提高超时时间或通过降低 `mem_limit` 来解决此问题。

#### on_file_field_error

```ini
on_file_field_error = skip_document
```

当文件字段发生 I/O 错误时的处理方式。可选，默认值为 `ignore_field`。

当文件字段索引时发生问题（[sql_file_field](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_file_field)），`indexer` 可以选择以下几种方式进行处理：忽略此特定字段的内容继续处理文档、跳过整个文档或完全中断索引操作。`on_file_field_error` 指令控制此行为。可接受的值包括：

- `ignore_field`，继续处理当前文档，但忽略该字段；
- `skip_document`，跳过当前文档，继续索引其他文档；
- `fail_index`，出现错误时中止索引并给出错误消息。

不管具体行为如何，当出现问题时，系统都会发出警告消息。

#### write_buffer

```ini
write_buffer = 4M
```

写入缓冲区大小，单位为字节。可选，默认值为 1MB。写入缓冲区用于写入临时和最终的表文件。较大的缓冲区可以减少磁盘写入操作的次数。缓冲区内存的分配是在 [mem_limit](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#mem_limit) 之外进行的。注意，最多会分配 4 个缓冲区，用于不同的文件，这会相应增加 RAM 的使用。

#### ignore_non_plain

```ini
ignore_non_plain = 1
```

`ignore_non_plain` 允许你忽略跳过非普通表时发出的警告。默认值为 0（不忽略）。
<!-- proofread -->


### 通过 systemd 调度 indexer

调度 indexer 运行有两种方法。第一种是经典的 crontab 方法。第二种是使用 systemd 计时器并定义自定义的调度时间。要创建计时器单元文件，需要将它们放置在 systemd 查找单元文件的适当目录中。在大多数 Linux 发行版中，该目录通常是 `/etc/systemd/system`。操作步骤如下：

1. 为你的自定义调度创建一个计时器单元文件：
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
   关于 `OnCalendar` 语法和更多示例，请查看 [此处](https://www.freedesktop.org/software/systemd/man/latest/systemd.time.html#Calendar Events)。

2. 编辑计时器单元以满足你的特定需求。
3. 启用计时器：
   ```shell
   systemctl enable manticore-indexer@idx1.timer
   ```
4. 启动计时器：
   ```shell
   systemctl start manticore-indexer@idx1.timer
   ```
5. 对其他计时器重复步骤 2-4。
