# Section "Searchd" in configuration

以下设置用于 Manticore Search 配置文件的 `searchd` 部分，以控制服务器的行为。以下是每个设置的摘要：

### access_plain_attrs

此设置为 [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。此项为可选，默认值为 `mmap_preread`。

`access_plain_attrs` 指令允许您为该 searchd 实例管理的所有表定义 [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令优先级更高，会覆盖该实例范围的默认值，提供更细粒度的控制。

### access_blob_attrs

此设置为 [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。此项为可选，默认值为 `mmap_preread`。

`access_blob_attrs` 指令允许您为该 searchd 实例管理的所有表定义 [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令优先级更高，会覆盖该实例范围的默认值，提供更细粒度的控制。

### access_doclists

此设置为 [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。此项为可选，默认值为 `file`。

`access_doclists` 指令允许您为该 searchd 实例管理的所有表定义 [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令优先级更高，会覆盖该实例范围的默认值，提供更细粒度的控制。

### access_hitlists

此设置为 [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。此项为可选，默认值为 `file`。

`access_hitlists` 指令允许您为该 searchd 实例管理的所有表定义 [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令优先级更高，会覆盖该实例范围的默认值，提供更细粒度的控制。

### access_dict

此设置为 [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。此项为可选，默认值为 `mmap_preread`。

`access_dict` 指令允许您为该 searchd 实例管理的所有表定义 [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令优先级更高，会覆盖该实例范围的默认值，提供更细粒度的控制。

### agent_connect_timeout

此设置为 [agent_connect_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) 参数设置实例范围的默认值。


### agent_query_timeout

此设置为 [agent_query_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) 参数设置实例范围的默认值。可以通过 `OPTION agent_query_timeout=XXX` 子句在每个查询级别覆盖。


### agent_retry_count

此设置为整数，指定 Manticore 在通过分布式表连接和查询远程代理失败前尝试的次数。默认值为 0（即不重试）。您也可以使用 `OPTION retry_count=XXX` 子句在每个查询级别设置此值。如果提供了每个查询的选项，它将覆盖配置中指定的值。

请注意，如果您在分布式表定义中使用了 [agent mirrors](../Creating_a_cluster/Remote_nodes/Mirroring.md#Agent-mirrors)，服务器将根据选定的 [ha_strategy](../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) 为每次连接尝试选择不同的镜像。在这种情况下，`agent_retry_count` 将针对镜像集中的所有镜像进行聚合。

例如，如果您有 10 个镜像并设置 `agent_retry_count=5`，服务器最多将重试 50 次，假设每个镜像平均尝试 5 次（使用 `ha_strategy = roundrobin` 选项时即如此）。

然而，作为绝对限制，代理定义中提供的 `retry_count` 选项生效。换句话说，代理定义中的 `[retry_count=2]` 选项始终代表最多尝试 2 次，不论您为代理指定了 1 个还是 10 个镜像。

### agent_retry_delay

此设置为整数，单位为毫秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)），指定在远程代理查询失败时，Manticore 重试前的延迟时间。仅在指定非零的 [agent_retry_count](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 或非零的每查询 `retry_count` 时生效。默认值为 500。您也可以使用 `OPTION retry_delay=XXX` 子句在每个查询级别设置此值。如果提供了每个查询的选项，它将覆盖配置中指定的值。


### attr_flush_period

<!-- example conf attr_flush_period -->
When using [Update](../Data_creation_and_modification/Updating_documents/UPDATE.md) to modify document attributes in real-time, the changes are first written to an in-memory copy of the attributes. These updates occur in a memory-mapped file, meaning the OS decides when to write the changes to disk. Upon normal shutdown of `searchd` (triggered by a `SIGTERM` signal), all changes are forced to be written to disk.

You can also instruct `searchd` to periodically write these changes back to disk to prevent data loss. The interval between these flushes is determined by `attr_flush_period`, specified in seconds (or [special_suffixes](../Server_settings/Special_suffixes.md)).

By default, the value is 0, which disables periodic flushing. However, flushing will still occur during a normal shutdown.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
attr_flush_period = 900 # persist updates to disk every 15 minutes
```
<!-- end -->

### auto_optimize

<!-- example conf auto_optimize -->
This setting controls the automatic [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) process for table compaction.

By default table compaction occurs automatically. You can modify this behavior with the `auto_optimize` setting:
* 0 to disable automatic table compaction (you can still call `OPTIMIZE` manually)
* 1 to explicitly enable it
* to enable it while multiplying the optimization threshold by 2.

By default, OPTIMIZE runs until the number of disk chunks is less than or equal to the number of logical CPU cores multiplied by 2.

However, if the table has attributes with KNN indexes, this threshold is different. In this case, it is set to the number of physical CPU cores divided by 2 to improve KNN search performance.

Note that toggling `auto_optimize` on or off doesn't prevent you from running [OPTIMIZE TABLE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) manually.

<!-- intro -->
##### Example:

<!-- request Disable -->
```ini
auto_optimize = 0 # disable automatic OPTIMIZE
```

<!-- request Throttle -->
```ini
auto_optimize = 2 # OPTIMIZE starts at 16 chunks (on 4 cpu cores server)
```

<!-- end -->

### auto_schema

<!-- example conf auto_schema -->
Manticore supports the automatic creation of tables that don't yet exist but are specified in INSERT statements. This feature is enabled by default. To disable it, set `auto_schema = 0` explicitly in your configuration. To re-enable it, set `auto_schema = 1` or remove the `auto_schema` setting from the configuration.

Keep in mind that the `/bulk` HTTP endpoint does not support automatic table creation.

> NOTE: The [auto schema functionality](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

<!-- request Disable -->
```ini
auto_schema = 0 # disable automatic table creation
```

<!-- request Enable -->
```ini
auto_schema = 1 # enable automatic table creation
```

<!-- end -->

### binlog_flush

<!-- example conf binlog_flush -->
This setting controls the binary log transaction flush/sync mode. It is optional, with a default value of 2 (flush every transaction, sync every second).

The directive determines how frequently the binary log will be flushed to the OS and synced to disk. There are three supported modes:

*  0, flush and sync every second. This offers the best performance, but up to 1 second worth of committed transactions can be lost in the event of a server crash or an OS/hardware crash.
*  1, flush and sync every transaction. This mode provides the worst performance but guarantees that every committed transaction's data is saved.
*  2, flush every transaction, sync every second. This mode delivers good performance and ensures that every committed transaction is saved in case of a server crash. However, in the event of an OS/hardware crash, up to 1 second worth of committed transactions can be lost.

For those familiar with MySQL and InnoDB, this directive is similar to `innodb_flush_log_at_trx_commit`. In most cases, the default hybrid mode 2 provides a nice balance of speed and safety, with full RT table data protection against server crashes and some protection against hardware ones.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
binlog_flush = 1 # ultimate safety, low speed
```
<!-- end -->

### binlog_common

<!-- example conf binlog_common -->
This setting controls how binary log files are managed. It is optional, with a default value of 0 (separate file for each table).

You can choose between two ways to manage binary log files:

* Separate file for each table (default, `0`): Each table saves its changes in its own log file. This setup is good if you have many tables that get updated at different times. It allows tables to be updated without waiting for others. Also, if there is a problem with one table's log file, it does not affect the others.
* Single file for all tables (`1`): All tables use the same binary log file. This method makes it easier to handle files because there are fewer of them. However, this could keep files longer than needed if one table still needs to save its updates. This setting might also slow things down if many tables need to update at the same time because all changes have to wait to be written to one file.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
binlog_common = 1 # use a single binary log file for all tables
```
<!-- end -->

### binlog_max_log_size

<!-- example conf binlog_max_log_size -->
This setting controls the maximum binary log file size. It is optional, with a default value of 256 MB.

A new binlog file will be forcibly opened once the current binlog file reaches this size limit. This results in a finer granularity of logs and can lead to more efficient binlog disk usage under certain borderline workloads. A value of 0 indicates that the binlog file should not be reopened based on size.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
binlog_max_log_size = 16M
```
<!-- end -->


### binlog_path

<!-- example conf binlog_path -->
This setting determines the path for binary log (also known as transaction log) files. It is optional, with a default value of the build-time configured data directory (e.g., `/var/lib/manticore/data/binlog.*` in Linux).

二进制日志用于RT表数据的崩溃恢复以及普通磁盘索引的属性更新，否则这些属性更新只会存储在RAM中，直到刷新时才写入。当启用日志记录时，每个COMMIT到RT表的事务都会写入日志文件。在非正常关闭后启动时，日志会被自动重放，恢复已记录的更改。

`binlog_path` 指令指定二进制日志文件的位置。它应只包含路径；`searchd` 会根据需要在该目录下创建和取消链接多个 `binlog.*` 文件（包括binlog数据、元数据和锁文件等）。

空值表示禁用二进制日志记录，这能提高性能，但会使RT表数据面临风险。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
binlog_path = # disable logging
binlog_path = /var/lib/manticore/data # /var/lib/manticore/data/binlog.001 etc will be created
```
<!-- end -->


### buddy_path

<!-- example conf buddy_path -->
此设置确定Manticore Buddy二进制文件的路径。此项为可选，默认值是构建时配置的路径，不同操作系统间此路径会有所不同。通常，你无需修改此设置。但如果你想以调试模式运行Manticore Buddy、对Manticore Buddy进行更改或实现新的插件，这项设置会很有用。在后者情况下，你可以从 https://github.com/manticoresoftware/manticoresearch-buddy `git clone` Buddy，向目录 `./plugins/` 添加新插件，并切换到Buddy源码目录后运行 `composer install --prefer-source` 以便更轻松开发。

为了确保你可以运行 `composer`，你的机器必须安装PHP 8.2或更高版本，并带有以下扩展：

```
--enable-dom
--with-libxml
--enable-tokenizer
--enable-xml
--enable-xmlwriter
--enable-xmlreader
--enable-simplexml
--enable-phar
--enable-bcmath
--with-gmp
--enable-debug
--with-mysqli
--enable-mysqlnd
```

你也可以选择适用于Linux amd64的特殊版本 `manticore-executor-dev`，该版本可在发布页面下载，例如：https://github.com/manticoresoftware/executor/releases/tag/v1.0.13

如果选择此方式，记得将manticore执行器的开发版本链接到 `/usr/bin/php`。

要禁用Manticore Buddy，请将该值设置为空，如示例所示。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
buddy_path = manticore-executor -n /usr/share/manticore/modules/manticore-buddy/src/main.php --debug # use the default Manticore Buddy in Linux, but run it in debug mode
buddy_path = manticore-executor -n /opt/homebrew/share/manticore/modules/manticore-buddy/bin/manticore-buddy/src/main.php --debug # use the default Manticore Buddy in MacOS arm64, but run it in debug mode
buddy_path = manticore-executor -n /Users/username/manticoresearch-buddy/src/main.php --debug # use Manticore Buddy from a non-default location
buddy_path = # disables Manticore Buddy
buddy_path = manticore-executor -n /Users/username/manticoresearch-buddy/src/main.php --debugv --skip=manticoresoftware/buddy-plugin-replace # debugv - enables more detailed logging, --skip - skips plugins
```
<!-- end -->

### client_timeout

<!-- example conf client_timeout -->
此设置确定使用持久连接时，两个请求之间的最大等待时间（以秒或[special_suffixes](../Server_settings/Special_suffixes.md)计）。此项为可选，默认值为五分钟。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
client_timeout = 1h
```
<!-- end -->


### collation_libc_locale

<!-- example conf collation_libc_locale -->
服务器 libc 区域设置。可选，默认值为C。

指定libc区域设置，影响基于libc的排序规则。详情参阅[排序规则](../Searching/Collations.md)章节。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
collation_libc_locale = fr_FR
```
<!-- end -->


### collation_server

<!-- example conf collation_server -->
默认服务器排序规则。可选，默认值为 libc_ci。

指定用于传入请求的默认排序规则。每个查询可覆盖该排序规则。详情及可用排序规则列表参见[排序规则](../Searching/Collations.md)章节。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
collation_server = utf8_ci
```
<!-- end -->


### data_dir

<!-- example conf data_dir -->
当指定此项时，该设置启用[实时模式](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)，这是一种命令式管理数据模式。值应为目录路径，指向你想用来存储所有表、二进制日志及其它保证Manticore Search在该模式下正常运行所需文件的目录。
指定 `data_dir` 时，不允许对[普通表](../Creating_a_table/Local_tables/Plain_table.md)进行索引。有关实时模式与普通模式的区别，可以阅读[此部分](../Read_this_first.md#Real-time-table-vs-plain-table)。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
data_dir = /var/lib/manticore
```
<!-- end -->

### diskchunk_flush_search_timeout

<!-- example conf diskchunk_flush_search_timeout -->
用于防止自动刷新RAM块时没有搜索发生的超时时间。可选，默认30秒。

检测搜索时间长度后再决定是否自动刷新。
仅当在最近 `diskchunk_flush_search_timeout` 秒内至少有一次搜索时，才会进行自动刷新。此设置与[diskchunk_flush_write_timeout](../Server_settings/Searchd.md#diskchunk_flush_write_timeout) 配合使用。相应的[每表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_search_timeout)优先级更高，会覆盖此全实例默认设置，提供更细粒度的控制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
diskchunk_flush_search_timeout = 120s
```
<!-- end -->

### diskchunk_flush_write_timeout

<!-- example conf diskchunk_flush_write_timeout -->
在无写入的情况下等待自动将RAM块刷新到磁盘的时间（秒）。可选，默认1秒。

如果`diskchunk_flush_write_timeout`秒内RAM块没有写入，块将自动刷新到磁盘。与[diskchunk_flush_search_timeout](../Server_settings/Searchd.md#diskchunk_flush_search_timeout) 配合使用。若需禁用自动刷新，请在配置中显式设置 `diskchunk_flush_write_timeout = -1`。相应的[每表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_write_timeout)优先级更高，会覆盖此全实例默认设置，从而提供更细粒度控制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
diskchunk_flush_write_timeout = 60s
```
<!-- end -->

### docstore_cache_size

<!-- example conf docstore_cache_size -->
此设置指定文档存储中文档块在内存中保持的最大大小。可选，默认值为16m（16兆字节）。

当使用 `stored_fields` 时，文档块会从磁盘读取并解压。由于每个块通常包含多个文档，因此在处理下一个文档时可以重用该块。为此，块会保存在服务器范围内的缓存中。缓存保存的是未压缩的块。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
docstore_cache_size = 8m
```
<!-- end -->

### engine

<!-- example conf engine -->
RT 模式下创建表时默认使用的属性存储引擎。可以是 `rowwise`（默认）或 `columnar`。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
engine = columnar
```
<!-- end -->


### expansion_limit

<!-- example conf expansion_limit -->
该设置决定单个通配符的最大展开关键字数量。此选项是可选的，默认值为 0（无限制）。

当对启用了 `dict = keywords` 的表执行子字符串搜索时，单个通配符可能会导致成千上万甚至数百万个匹配关键字（比如匹配整个牛津词典中的 `a*`）。此指令允许您限制此类展开的影响。设置 `expansion_limit = N` 会限制每个通配符的展开关键字不超过 N 个最频繁匹配的关键字。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_limit = 16
```
<!-- end -->

### expansion_merge_threshold_docs

<!-- example conf expansion_merge_threshold_docs -->
该设置决定允许合并所有此类关键字的最大文档数。此选项是可选的，默认值为 32。

当对启用了 `dict = keywords` 的表执行子字符串搜索时，单个通配符可能会导致成千上万甚至数百万个匹配关键字。此指令允许您增加合并关键字的限制来加快匹配速度，但会增加搜索时的内存使用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_merge_threshold_docs = 1024
```
<!-- end -->

### expansion_merge_threshold_hits

<!-- example conf expansion_merge_threshold_hits -->
该设置决定允许合并所有此类关键字的最大匹配次数。此选项是可选的，默认值为 256。

当对启用了 `dict = keywords` 的表执行子字符串搜索时，单个通配符可能会导致成千上万甚至数百万个匹配关键字。此指令允许您增加合并关键字的限制来加快匹配速度，但会增加搜索时的内存使用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_merge_threshold_hits = 512
```
<!-- end -->

### grouping_in_utc

该设置指定 API 和 SQL 中基于时间的分组计算是在本地时区还是 UTC。此选项是可选的，默认值为 0（表示“本地时区”）。

默认情况下，所有“按时间分组”表达式（如 API 中按天、周、月、年分组，及 SQL 中按天、月、年、年月、年月日分组）均使用本地时间。例如，如果您有时间属性为 `13:00 utc` 和 `15:00 utc` 的文档，在分组时，它们都会根据您的本地时区设置划入相应的分组。如果您位于 `utc`，则会归为同一天，但如果您位于 `utc+10`，则这两个文档会被划分到不同的“按天分组”中（因为 13:00 utc 在 UTC+10 时区是本地时间 23:00，而 15:00 utc 是下一天的 01:00）。有时这种行为是不被接受的，希望时间分组不依赖于时区。您可以通过设置全局 TZ 环境变量来运行服务器，但这会影响日志中的时间戳，也许不符合期望。开启此选项（通过配置或者在 SQL 中使用 [SET global](../Server_settings/Setting_variables_online.md#SET) 语句）会使所有时间分组表达式按 UTC 计算，而其他依赖时间的函数（如服务器日志）依然使用本地时区。


### timezone

该设置指定日期/时间相关函数使用的时区。默认使用本地时区，但您可以指定 IANA 格式的其他时区（例如 `Europe/Amsterdam`）。

请注意，该设置不会影响日志记录，日志始终使用本地时区。

此外，如果启用了 `grouping_in_utc`，则“按时间分组”功能仍使用 UTC，而其他日期/时间相关函数使用指定的时区。总体来说，不建议同时使用 `grouping_in_utc` 和 `timezone`。

您可以在配置文件中设置此选项，或通过 SQL 使用 [SET global](../Server_settings/Setting_variables_online.md#SET) 语句设置。


### ha_period_karma

<!-- example conf ha_period_karma -->
该设置指定代理镜像统计的窗口大小，单位为秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。此选项是可选的，默认值为 60 秒。

对于包含代理镜像的分布式表（详见 [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)），主节点会跟踪多个不同的每镜像计数器。这些计数器用于故障转移和平衡（主节点基于计数器选择最佳镜像）。计数器按 `ha_period_karma` 秒为单位累积。

开始新计数块后，主节点可能仍使用上一块累积的值，直到新块填充至半满。因此，任何之前的历史最多在 1.5 倍 ha_period_karma 秒后停止影响镜像选择。

虽然最多使用两个块进行镜像选择，但最多储存 15 个最近的块用于监控目的。这些块可以通过 [SHOW AGENT STATUS](../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS) 语句查看。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_period_karma = 2m
```
<!-- end -->


### ha_ping_interval

<!-- example conf ha_ping_interval -->
此设置配置代理镜像ping之间的间隔，单位为毫秒（或[special_suffixes](../Server_settings/Special_suffixes.md)）。该设置是可选的，默认值为1000毫秒。

对于包含代理镜像的分布式表（详情请参见[agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)），主服务器会在空闲期间向所有镜像发送ping命令。这是为了跟踪当前代理状态（存活或死亡，网络往返时间等）。此类ping之间的间隔由此指令定义。要禁用ping，将ha_ping_interval设置为0。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_ping_interval = 3s
```
<!-- end -->


### hostname_lookup

`hostname_lookup`选项定义了刷新主机名的策略。默认情况下，在服务器启动时缓存代理主机名的IP地址，以避免过度访问DNS。但是，在某些情况下，IP可能会动态改变（例如云托管），此时可能希望不缓存IP。将此选项设置为`request`会禁用缓存，并在每次查询时查询DNS。IP地址也可以使用`FLUSH HOSTNAMES`命令手动刷新。

### jobs_queue_size

jobs_queue_size设置定义队列中可以同时存在多少个“任务”。默认情况下无限制。

大多数情况下，“任务”指的是对单个本地表（普通表或实时表的磁盘分片）的一次查询。例如，如果您有一个由2个本地表组成的分布式表，或者一个具有2个磁盘分片的实时表，对任意一个的搜索查询通常会在队列中产生2个任务。然后，线程池（其大小由[threads](../Server_settings/Searchd.md#threads)定义）将处理它们。然而，在某些情况下，如果查询过于复杂，可能会创建更多任务。当[max_connections](../Server_settings/Searchd.md#max_connections)和[threads](../Server_settings/Searchd.md#threads)不足以在期望性能之间取得平衡时，建议调整此设置。

### join_batch_size

表连接通过累积一个匹配批次来工作，该批次是对左表执行查询的结果。然后，将该批次作为一个单一查询在右表上进行处理。

此选项允许您调整批次大小。默认值为`1000`，将此选项设置为`0`将禁用批处理。

较大的批次大小可能提升性能；然而，对于某些查询，它可能导致过高的内存消耗。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
join_batch_size = 2000
```
<!-- end -->

### join_cache_size

在右表上执行的每个查询由特定的JOIN ON条件定义，这些条件决定了从右表检索的结果集。

如果JOIN ON条件的唯一数量较少，重复使用结果比重复执行右表查询更有效。为此，结果集存储在缓存中。

此选项允许您配置此缓存的大小。默认值为`20 MB`，将此选项设置为0将禁用缓存。

注意，每个线程维护自己的缓存，因此在估算总内存使用时，应考虑执行查询的线程数量。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
join_cache_size = 10M
```
<!-- end -->

### listen_backlog

<!-- example conf listen_backlog -->
listen_backlog设置决定TCP监听队列的长度，以接收传入连接。这在逐个处理请求的Windows构建中特别相关。当连接队列达到其限制时，新的传入连接将被拒绝。
对于非Windows构建，默认值通常能正常工作，通常不需要调整此设置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
listen_backlog = 20
```
<!-- end -->

### kibana_version_string

<!-- example conf kibana_version_string -->
返回给Kibana或OpenSearch仪表板的服务器版本字符串。可选 — 默认设置为`7.6.0`。

某些版本的Kibana和OpenSearch仪表板期望服务器报告特定版本号，行为可能依赖版本号。为解决此类问题，可以使用此设置，使Manticore向Kibana或OpenSearch仪表板报告自定义版本。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
kibana_version_string = 1.2.3
```
<!-- end -->

### listen

<!-- example conf listen -->
此设置允许您指定Manticore接受连接的IP地址和端口，或Unix域套接字路径。

`listen`的一般语法为：

```ini
listen = ( address ":" port | port | path | address ":" port start - port end ) [ ":" protocol [ "_vip" ] [ "_readonly" ] ]
```

您可以指定：
* IP地址（或主机名）和端口号
* 或仅端口号
* 或Unix套接字路径（Windows不支持）
* 或IP地址和端口范围

如果您指定端口号但未指定地址，`searchd`将监听所有网络接口。Unix路径以斜杠开头。端口范围仅可用于复制协议。

您还可以指定用于连接此套接字的协议处理程序（监听器）。监听器包括：

* **未指定** - Manticore将在此端口接受来自：
  - 其他Manticore代理（即远程分布式表）
  - 通过HTTP和HTTPS的客户端
  - [Manticore Buddy](https://manticoresearch.com/blog/manticoresearch-buddy-intro/)。**确保您有此类监听器（或下面提到的`http`监听器），以避免Manticore功能限制。**
* `mysql` MySQL协议，用于来自MySQL客户端的连接。注意：
  - 同时支持压缩协议。
  - 如果启用[SSL](../Security/SSL.md#SSL)，可以建立加密连接。
* `replication` - 节点通信中使用的复制协议。更多详细信息请参见[复制](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md)部分。您可以指定多个复制监听器，但它们必须都监听在同一IP上；只有端口可以不同。当您定义一个带有端口范围的复制监听器（例如，`listen = 192.168.0.1:9320-9328:replication`）时，Manticore不会立即开始监听这些端口。相反，它只会在您开始使用复制时从指定范围中随机选择空闲端口。要使复制正常工作，范围内至少需要2个端口。
* `http` - 同 **未指定**。Manticore将在此端口接受来自远程代理和客户端的HTTP和HTTPS连接。
* `https` - HTTPS协议。Manticore将仅在此端口接受HTTPS连接。更多详细信息请参见[SSL](../Security/SSL.md)部分。
* `sphinx` - 传统的二进制协议。用于处理来自远程[SphinxSE](../Extensions/SphinxSE.md)客户端的连接。一些Sphinx API客户端的实现（例如Java实现）需要明确声明监听器。

将后缀 `_vip` 添加到客户端协议（即，所有除了 `replication`，例如 `mysql_vip` 或 `http_vip` 或仅 `_vip`）会强制创建一个专用线程来绕过不同的限制。这在节点维护严重过载时非常有用，当时服务器可能会停止响应或无法通过常规端口连接。

后缀 `_readonly` 为监听器设置[只读模式](../Security/Read_only.md)，并限制其仅接受读取查询。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
listen = localhost
listen = localhost:5000 # listen for remote agents (binary API) and http/https requests on port 5000 at localhost
listen = 192.168.0.1:5000 # listen for remote agents (binary API) and http/https requests on port 5000 at 192.168.0.1
listen = /var/run/manticore/manticore.s # listen for binary API requests on unix socket
listen = /var/run/manticore/manticore.s:mysql # listen for mysql requests on unix socket
listen = 9312 # listen for remote agents (binary API) and http/https requests on port 9312 on any interface
listen = localhost:9306:mysql # listen for mysql requests on port 9306 at localhost
listen = localhost:9307:mysql_readonly # listen for mysql requests on port 9307 at localhost and accept only read queries
listen = 127.0.0.1:9308:http # listen for http requests as well as connections from remote agents (and binary API) on port 9308 at localhost
listen = 192.168.0.1:9320-9328:replication # listen for replication connections on ports 9320-9328 at 192.168.0.1
listen = 127.0.0.1:9443:https # listen for https requests (not http) on port 9443 at 127.0.0.1
listen = 127.0.0.1:9312:sphinx # listen for legacy Sphinx requests (e.g. from SphinxSE) on port 9312 at 127.0.0.1
```
<!-- end -->

可以有多个 `listen` 指令。`searchd` 将在所有指定的端口和套接字上监听客户端连接。Manticore包中提供的默认配置定义了在以下端口上监听：
* `9308` 和 `9312` 用于来自远程代理和非MySQL客户端的连接
* 在端口 `9306` 上用于MySQL连接。

如果您完全没有在配置中指定任何 `listen`，Manticore将等待在以下端口上的连接：
* `127.0.0.1:9306` 用于MySQL客户端
* `127.0.0.1:9312` 用于HTTP/HTTPS以及来自其他Manticore节点和基于Manticore二进制API的客户端的连接。

#### 监听特权端口

默认情况下，Linux不允许您让Manticore监听1024以下的端口（例如 `listen = 127.0.0.1:80:http` 或 `listen = 127.0.0.1:443:https`），除非您以root身份运行searchd。如果您仍希望能够启动Manticore，使其在非root用户下监听端口< 1024，请考虑执行以下操作之一（这两种方法都应该有效）：
* 运行命令 `setcap CAP_NET_BIND_SERVICE=+eip /usr/bin/searchd`
* 将 `AmbientCapabilities=CAP_NET_BIND_SERVICE` 添加到Manticore的systemd单元并重新加载守护程序（`systemctl daemon-reload`）。

#### 关于Sphinx API协议和TFO的技术细节
<details>
传统的Sphinx协议有2个阶段：握手交换和数据流。握手由客户端发送的4字节数据包和守护进程发送的4字节数据包组成，其唯一目的在于 - 客户端确定远程是真实的Sphinx守护进程，守护进程确定远程是真实的Sphinx客户端。主数据流相当简单：让双方面声明握手，并进行相应的检查。使用短数据包进行的交换需要使用特殊的 `TCP_NODELAY` 标志，它关闭Nagle的TCP算法并声明TCP连接将作为小数据包的对话进行。
然而，这种谈判中并没有严格定义谁先发言。历史上，所有使用二进制API的客户端都先发言：发送握手，然后从守护进程读取4字节，然后发送请求并从守护进程读取答案。
当我们改进Sphinx协议兼容性时，我们考虑了这些因素：

1. 通常，主代理通信是从已知客户端到已知主机在已知端口上建立的。因此，终端提供错误握手的可能性很小。因此，我们可以隐含假设双方都是有效的，实际上都在使用Sphinx协议。
2. 基于这一假设，我们可以将握手“粘合”到实际请求上并在一个数据包中发送。如果后端是传统的Sphinx守护进程，它只会将此粘合包读取为4字节的握手，然后是请求主体。由于它们都是在一个数据包中到达的，因此后端套接字具有-1 RTT，并且前端缓冲区仍然正常工作。
3. 继续这个假设：由于“查询”数据包相当小，握手甚至更小，我们可以在初始的“SYN”TCP包中使用现代TFO（tcp-fast-open）技术发送两者。这意味着：我们连接到远程节点，携带粘合握手+主体数据包。守护进程接受连接，并立即在套接字缓冲区中拥有握手和主体，因为它们在第一个TCP“SYN”包中到达。这消除了另一个RTT。
4. 最后，让守护进程接受这一改进。实际上，从应用程序的角度来看，它暗示着不要使用 `TCP_NODELAY`。而从系统角度来看，它意味着需要确保守护进程一侧启用接受TFO，客户端一侧也启用发送TFO。默认情况下，在现代系统中，客户端TFO已默认激活，因此您只需要为服务器调优TFO即可使所有功能正常工作。

所有这些改进在不实际更改协议本身的情况下，使我们消除了连接中TCP协议的1.5 RTT。如果查询和答案能够放置在一个TCP包中，则将整个二进制API会话从3.5 RTT缩减到2 RTT - 这使得网络协商速度快约2倍。

所以，我们所有的改进都是围绕一个最初未定义的声明：“谁先发言”。如果客户端先发言，我们可以应用所有这些优化，有效地在单个 TFO 包中处理连接 + 握手 + 查询。此外，我们可以查看收到的数据包的开头，确定真正的协议。这就是为什么你可以通过 API/http/https 连接到同一个端口。如果守护进程必须先发言，所有这些优化都是不可能的，多协议也不可能。这就是为什么我们为 MySQL 设置了专门的端口，而没有将其与所有其他协议合并到同一个端口中。突然，在所有客户端中，有一个是写成假设守护进程应该先发送握手的。那就是——没有可能实现所有上述的改进。那就是 MySQL/MariaDB 的 SphinxSE 插件。因此，专门为这个单一客户端，我们专门定义了 `sphinx` 协议以采用最传统的方式工作。也就是说：双方都启用 `TCP_NODELAY` 并交换小数据包。守护进程在连接时发送其握手，接着客户端发送其握手，然后一切正常工作。虽然这不是很优化，但就是能用。如果你用 SphinxSE 连接到 Manticore——你必须专门为监听器指定明确的 `sphinx` 协议。对于其他客户端——避免使用这个监听器，因为它更慢。如果你使用其他传统的 Sphinx API 客户端——首先检查它们是否能使用非专用的多协议端口工作。对于主控-代理的链接，使用非专用（多协议）端口并启用客户端和服务器的 TFO 能很好地工作，并且肯定会加快网络后端的运行，特别是如果你有非常轻量和快速的查询。
</details>

### listen_tfo

此设置允许所有监听器使用 TCP_FASTOPEN 标记。默认情况下，它由系统管理，但可以通过将其设置为 '0' 明确关闭。

有关 TCP Fast Open 扩展的一般知识，请参阅 [Wikipedia](https://en.wikipedia.org/wiki/TCP_Fast_Open)。简而言之，它允许在建立连接时消除一次 TCP 往返。

在实际操作中，在许多情况下使用 TFO 可以优化客户端-代理网络效率，就像[持久代理](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)在使用，但又不持有激活的连接，而且没有连接数最大限制。

在现代操作系统上，TFO 支持通常在系统级别开启，但这只是一个“能力”，而非规则。Linux（作为最前沿的系统）自 2011 年起支持，从内核 3.7 开始（服务端）。Windows 在某些 Windows 10 版本起支持。其他操作系统（FreeBSD，MacOS）也在支持范围内。

Linux 系统服务器检查变量 `/proc/sys/net/ipv4/tcp_fastopen` 并根据它作出反应。第 0 位控制客户端侧，第 1 位控制监听器。默认情况下，系统将此参数设置为 1，即启用客户端，禁用监听器。

### log

<!-- example conf log -->
log 设置指定日志文件名称，用于记录所有 `searchd` 运行时事件。如果未指定，默认名称为 'searchd.log'。

或者，你可以将文件名设为 'syslog'。这样事件将发送到 syslog 守护进程。要使用 syslog 选项，你需要在构建时使用 `-–with-syslog` 选项配置 Manticore。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
log = /var/log/searchd.log
```
<!-- end -->


### max_batch_queries

<!-- example conf max_batch_queries -->
限制每批处理的查询数量。可选，默认值为 32。

使 searchd 在使用[多查询](../Searching/Multi-queries.md)时，对单批提交的查询数量进行合理性检查。设置为 0 可跳过检查。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_batch_queries = 256
```
<!-- end -->

### max_connections

<!-- example max_connections -->
最大同时客户端连接数。默认无限制。通常只有在使用任何持久连接类型时（如 cli mysql 会话或远程分布式表的持久远程连接）才明显。当超过限制时，仍可以通过 [VIP 连接](../Connecting_to_the_server/MySQL_protocol.md#VIP-connection) 连接到服务器。VIP 连接不计入限制。

<!-- request Example -->
```ini
max_connections = 10
```

<!-- end -->

### max_threads_per_query

<!-- example max_threads_per_query -->
实例范围内单个操作可以使用的线程上限。默认情况下，适当的操作可占用所有 CPU 核心，不给其他操作留空间。例如，对一个较大 percolate 表执行 `call pq` 可能会占用所有线程达数十秒。将 `max_threads_per_query` 设置为比如 [threads](../Server_settings/Searchd.md#threads) 的一半，将确保你可以并行运行几个这样的 `call pq` 操作。

你还可以在运行时作为会话或全局变量设置该项。

此外，你可以利用 [threads OPTION](../Searching/Options.md#threads) 基于单个查询控制行为。

<!-- intro -->
##### 示例：
<!-- request Example -->

```ini
max_threads_per_query = 4
```

<!-- end -->

### max_filters

<!-- example conf max_filters -->
每查询允许的最大过滤器数量。此设置仅用于内部合理性检查，不直接影响内存使用或性能。可选，默认值为 256。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_filters = 1024
```
<!-- end -->


### max_filter_values

<!-- example conf max_filter_values -->
每个过滤器允许的最大值数量。此设置仅用于内部合理性检查，不直接影响内存使用或性能。可选，默认值为 4096。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_filter_values = 16384
```
<!-- end -->


### max_open_files

<!-- example conf max_open_files -->
服务器允许打开的最大文件数称为“软限制”。请注意，服务大型碎片化实时表可能需要将此限制设置得较高，因为每个磁盘块可能占用十几个甚至更多的文件。例如，一个有1000个块的实时表可能需要同时打开数千个文件。如果在日志中遇到“Too many open files”（打开的文件过多）错误，请尝试调整此选项，这可能有助于解决问题。

还有一个“硬限制”，选项不能超过该限制。此限制由系统定义，可以在Linux的 `/etc/security/limits.conf` 文件中更改。其他操作系统可能有不同的方法，请参阅您的手册以获取更多信息。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = 10000
```
<!-- end -->

<!-- example conf max_open_files max -->
除了直接的数字值外，您还可以使用魔法词“max”将限制设置为当前的硬限制值。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = max
```
<!-- end -->


### max_packet_size

<!-- example conf max_packet_size -->
允许的最大网络数据包大小。此设置限制来自客户端的查询数据包和分布式环境中远程代理的响应数据包。仅用于内部完整性检查，不直接影响RAM使用或性能。可选，默认值为128M。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_packet_size = 32M
```
<!-- end -->


### mysql_version_string

<!-- example conf mysql_version_string -->
通过MySQL协议返回的服务器版本字符串。可选，默认为空（返回Manticore版本）。

一些挑剔的MySQL客户端库依赖于MySQL使用的特定版本号格式，而且有时会根据报告的版本号（而非指示的能力标志）选择不同的执行路径。例如，Python MySQLdb 1.2.2在版本号不是X.Y.ZZ格式时会抛出异常；MySQL .NET连接器6.3.x在版本号为1.x且某种标志组合时内部失败等。为了解决这个问题，您可以使用`mysql_version_string`指令，使`searchd`对通过MySQL协议连接的客户端报告不同的版本。（默认情况下，它报告自身版本。）


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
mysql_version_string = 5.0.37
```
<!-- end -->


### net_workers

网络线程数，默认值为1。

此设置适用于极高查询率的情况，当单个线程不足以处理所有传入查询时使用。


### net_wait_tm

控制网络线程的忙循环间隔。默认值为-1，可以设置为-1、0或正整数。

当服务器配置为纯主节点且只是将请求路由到代理时，重要的是无延迟地处理请求，不允许网络线程休眠。为此，有忙循环。在有请求时，如果`net_wait_tm`为正数，网络线程会使用CPU轮询`10 * net_wait_tm`毫秒；如果`net_wait_tm`为0，则只使用CPU轮询。忙循环可以通过设置`net_wait_tm = -1`禁用——此时，轮询器在系统轮询调用中将超时设置为实际代理的超时。

> **警告：** CPU忙循环实际上会占用CPU核心资源，因此将此值设置为非默认值时，即使服务器空闲，也会产生明显的CPU使用量。


### net_throttle_accept

定义网络循环每次迭代接受的客户端数量。默认值为0（无限制），对大多数用户来说足够。此选项用于高负载场景下微调网络循环的吞吐量。


### net_throttle_action

定义网络循环每次迭代处理的请求数量。默认值为0（无限制），对大多数用户来说足够。此选项用于高负载场景下微调网络循环的吞吐量。

### network_timeout

<!-- example conf network_timeout -->
网络客户端请求读写超时，单位为秒（或 [特殊后缀](../Server_settings/Special_suffixes.md)）。可选，默认值为5秒。`searchd`将在客户端在此超时内未发送查询或读取结果时强制关闭连接。

另请注意 [reset_network_timeout_on_packet](../Server_settings/Searchd.md#reset_network_timeout_on_packet) 参数。该参数将`network_timeout`的作用从应用于整个`query`或`result`变为应用于单个数据包。通常一个查询/结果可以包含一个或两个包；但在需要大量数据的情况下，此参数在保持活动操作方面非常有用。

<!-- request Example -->

```ini
network_timeout = 10s
```
<!-- end -->

### node_address

<!-- example conf node_address -->
此设置允许您指定节点的网络地址。默认情况下，它设置为复制的[listen](../Server_settings/Searchd.md#listen)地址。在大多数情况下这是正确的；但某些情况下您需要手动指定：

* 节点位于防火墙后面
* 启用网络地址转换（NAT）
* 容器部署，如Docker或云部署
* 集群节点分布在多个区域


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
node_address = 10.101.0.10
```
<!-- end -->

### not_terms_only_allowed

<!-- example conf not_terms_only_allowed -->
此设置决定是否允许仅带[否定](../Searching/Full_text_matching/Operators.md#Negation-operator)全文操作符的查询。可选，默认值为0（不允许仅含NOT操作符的查询）。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
not_terms_only_allowed = 1
```
<!-- end -->

### optimize_cutoff

<!-- example conf optimize_cutoff -->
设置默认表压缩阈值。详情请参阅这里 - [优化的磁盘块数量](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks)。此设置可以通过每查询选项 [cutoff](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 重写。也可以通过 [SET GLOBAL](../Server_settings/Setting_variables_online.md#SET) 动态更改。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
optimize_cutoff = 4
```
<!-- end -->

### persistent_connections_limit

<!-- example conf persistent_connections_limit -->
此设置决定与远程 [persistent agents](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 建立的最大同时持久连接数。每当连接到 `agent_persistent` 下定义的代理时，我们会尝试重用已有连接（如果有），或者连接并保存该连接以供将来使用。然而，在某些情况下，限制此类持久连接的数量是合理的。该指令定义此限制。它影响所有分布式表中到每个代理主机的连接数。

合理的做法是将此值设置为不超过代理配置中的 [max_connections](../Server_settings/Searchd.md#max_connections) 选项。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
persistent_connections_limit = 29 # assume that each host of agents has max_connections = 30 (or 29).
```
<!-- end -->


### pid_file

<!-- example conf pid_file -->
pid_file 是 Manticore search 中的必填配置选项，指定 `searchd` 服务器进程 ID 存储文件的路径。

searchd 进程 ID 文件在启动时重新创建并锁定，服务器运行时包含主服务器进程 ID。服务器关闭时，该文件会被删除。
此文件的目的是使 Manticore 能执行各种内部任务，例如检查是否已有运行中的 `searchd` 实例，停止 `searchd`，以及通知其应轮换表。该文件也可用于外部自动化脚本。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
pid_file = /var/run/manticore/searchd.pid
```
<!-- end -->


### predicted_time_costs

<!-- example conf predicted_time_costs -->
查询时间预测模型的成本，单位为纳秒。可选，默认值为 `doc=64, hit=48, skip=2048, match=64`。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
predicted_time_costs = doc=128, hit=96, skip=4096, match=128
```
<!-- end -->

<!-- example conf predicted_time_costs 1 -->
基于最大查询时间设置在查询完成前终止查询是一个良好的安全措施，但它有一个固有的缺点：结果不确定（不稳定）。也就是说，如果你多次重复执行相同的（复杂）搜索查询并设置时间限制，时间限制会在不同阶段被触发，结果会有*不同*的结果集。

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT … OPTION max_query_time
```
<!-- request API -->

```api
SetMaxQueryTime()
```
<!-- end -->

有一个新选项，[SELECT … OPTION max_predicted_time](../Searching/Options.md#max_predicted_time)，它允许你限制查询时间*并*获得稳定的、可重复的结果。它不是在评估查询时定期检查实际当前时间（这种方式是不确定的），而是通过简单线性模型预测当前运行时间：

```ini
predicted_time =
    doc_cost * processed_documents +
    hit_cost * processed_hits +
    skip_cost * skiplist_jumps +
    match_cost * found_matches
```

查询在 `predicted_time` 达到给定限制时提前终止。

当然，这不是实际花费时间的硬限制（但它是*处理*工作量的硬限制），而简单线性模型绝不是理想精准的模型。因此，实际挂钟时间*可能*低于或超过目标限制。然而，误差范围是可以接受的：例如，在我们对100毫秒目标限制的实验中，大多数测试查询的时间在95到105毫秒范围内，*所有*查询均在80到120毫秒范围内。此外，作为一个额外好处，使用模型查询时间而非测量实际运行时间也减少了 gettimeofday() 的调用次数。

不同服务器品牌和型号不尽相同，因此 `predicted_time_costs` 指令允许你配置上述模型的成本。方便起见，这些成本是以整数形式计数的纳秒。（max_predicted_time的限制是以毫秒计，必须以0.000128毫秒而非128纳秒方式指定成本值较含糊且容易出错。）不必一次性指定所有四个成本，未指定者将采用默认值。但我们强烈建议为可读性指定全部值。


### preopen_tables

<!-- example conf preopen_tables -->
preopen_tables 配置指令指定是否在启动时强制预先打开所有表。默认值为1，意味着不论每表 [preopen](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Other-performance-related-settings) 设置如何，都会预先打开所有表。如果设置为0，则每表设置生效且默认值为0。

预先打开表可防止搜索查询与轮换之间的竞争导致查询偶尔失败。但这也会使用更多文件句柄。在大多数场景中，推荐预先打开表。

以下是示例配置：

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
preopen_tables = 1
```
<!-- end -->

### pseudo_sharding

<!-- example conf pseudo_sharding -->
pseudo_sharding 配置选项允许对本地普通表和实时表的搜索查询进行并行化，无论其是直接查询还是通过分布式表查询。此功能会自动将查询并行到 `searchd.threads` 配置的线程数。

请注意，如果你的工作线程已经很忙，因为你有：
* 高查询并发
* 任何形式的物理分片：
  - 多个普通/实时表的分布式表
  - 由过多磁盘块组成的实时表

然后启用 pseudo_sharding 可能不会带来任何好处，甚至可能导致吞吐量略微下降。如果您优先考虑更高的吞吐量而非更低的延迟，建议禁用此选项。

默认启用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
pseudo_sharding = 0
```
<!-- end -->


### replication_connect_timeout

`replication_connect_timeout` 指令定义连接到远程节点的超时时间。默认情况下，值以毫秒为单位，也可以有[其他后缀](../Server_settings/Special_suffixes.md)。默认值为 1000（1 秒）。

连接到远程节点时，Manticore 最多等待此时间以成功完成连接。如果超时被触发但连接尚未建立，并且启用了 `retries`，则将启动重试。


### replication_query_timeout

`replication_query_timeout` 设置 searchd 等待远程节点完成查询的时间。默认值为 3000 毫秒（3秒），但可以通过后缀表示不同的时间单位。

建立连接后，Manticore 会等待远程节点最多 `replication_query_timeout` 时间完成。请注意，这个超时与 `replication_connect_timeout` 是分开的，由远程节点引起的总延迟将是两者之和。


### replication_retry_count

该设置是一个整数，指定 Manticore 在报告致命查询错误之前，复制期间尝试连接和查询远程节点的次数。默认值为 3。


### replication_retry_delay

该设置是以毫秒为单位的整数（或[特殊后缀](../Server_settings/Special_suffixes.md)），用于指定 Manticore 在复制期间查询远程节点失败后重试前的延迟时间。仅当指定非零值时此值才有效。默认值为 500。

### qcache_max_bytes

<!-- example conf qcache_max_bytes -->
此配置设置用于缓存结果集的最大内存（以字节为单位）。默认值为 16777216，相当于16兆字节。如果值设为0，则禁用查询缓存。有关查询缓存的更多信息，请参阅[查询缓存](../Searching/Query_cache.md)。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
qcache_max_bytes = 16777216
```
<!-- end -->


### qcache_thresh_msec

整数，单位为毫秒。查询结果被缓存的最小执行时间阈值。默认值为3000，或3秒。0表示缓存所有查询。详情请参阅[查询缓存](../Searching/Query_cache.md)。此值也可以用时间[特殊后缀](../Server_settings/Special_suffixes.md)表示，但请谨慎使用，避免与自身名称中的 '_msec' 混淆。


### qcache_ttl_sec

整数，单位为秒。缓存结果集的过期时间。默认值为60，或1分钟。最小可能值为1秒。详情请参阅[查询缓存](../Searching/Query_cache.md)。此值也可以用时间[特殊后缀](../Server_settings/Special_suffixes.md)表示，但请谨慎使用，避免与自身名称中的 '_sec' 混淆。


### query_log_format

<!-- example conf query_log_format -->
查询日志格式。可选值为 `plain` 和 `sphinxql`，默认是 `sphinxql`。

`sphinxql` 模式记录有效的 SQL 语句。`plain` 模式以纯文本格式记录查询（主要适用于纯全文搜索场景）。该指令允许你在搜索服务器启动时切换两种格式，也可以通过 `SET GLOBAL query_log_format=sphinxql` 语法动态修改。详情请参阅[查询日志](../Logging/Query_logging.md)。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_format = sphinxql
```
<!-- end -->

### query_log_min_msec

限制（以毫秒计），防止查询被写入查询日志。可选，默认值为0（所有查询都写入查询日志）。此指令指定只有执行时间超过指定限制的查询才会被记录（此值也可用时间[特殊后缀](../Server_settings/Special_suffixes.md) 表示，但请谨慎使用，避免与名称中包含的`_msec` 混淆）。

### query_log

<!-- example conf query_log -->
查询日志文件名。可选，默认为空（不记录查询）。所有搜索查询（如 SELECT …，但不包括 INSERT/REPLACE/UPDATE 查询）将被记录在此文件。格式详见[查询日志](../Logging/Query_logging.md)。如果使用“plain”格式，可以将日志文件路径设置为“syslog”，这样所有查询将以 `LOG_INFO` 优先级发送给 syslog 守护进程，并使用 “[query]” 前缀代替时间戳。使用 syslog 选项时，Manticore 需在构建时配置 `-–with-syslog`。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log = /var/log/query.log
```
<!-- end -->


### query_log_mode

<!-- example conf query_log_mode -->
`query_log_mode` 指令允许为 searchd 和查询日志文件设置不同的权限。默认情况下，这些日志文件权限是 600，意味着只有运行服务器的用户和 root 用户可以读取日志文件。
如果您希望允许其他用户读取日志文件（例如运行在非 root 用户下的监控方案），此指令非常有用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_mode  = 666
```
<!-- end -->

### read_buffer_docs

<!-- example conf read_buffer_docs -->
`read_buffer_docs` 指令控制文档列表的每关键词读取缓冲区大小。每个查询中每个关键词存在两个读取缓冲区：一个用于文档列表，一个用于匹配列表。此设置允许你控制文档列表缓冲区大小。

较大的缓冲区大小可能会增加每个查询的内存使用，但可能会降低 I/O 时间。对于较慢的存储设备，设置较大值是合理的，但对于能够实现高 IOPS 的存储，应在较低值区域进行实验。

默认值为 256K，最小值为 8K。您还可以为每个表单独设置 [read_buffer_docs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs)，这将覆盖服务器配置级别的设置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_docs = 128K
```
<!-- end -->


### read_buffer_hits

<!-- example conf read_buffer_hits -->
read_buffer_hits 指令指定搜索查询中命中的关键词的每个关键词读取缓冲区大小。默认大小为 256K，最小值为 8K。对于搜索查询中的每个关键词出现，有两个相关的读取缓冲区，一个用于文档列表，一个用于命中列表。增加缓冲区大小可以增加每个查询的内存使用，但减少 I/O 时间。对于较慢的存储设备，较大的缓冲区大小是合理的，而对于能够实现高 IOPS 的存储，应在较低值区域进行实验。

该设置还可以通过 [read_buffer_hits](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_hits) 选项在每个表级别指定，这将覆盖服务器级别的设置。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_hits = 128K
```
<!-- end -->

### read_unhinted

<!-- example conf read_unhinted -->
未提示的读取大小。可选，默认是 32K，最小为 1K。

在查询时，有些读取事先知道要读取的数据量，但有些目前不知道。最明显的是，命中列表的大小目前无法预先知道。此设置允许您控制在这种情况下读取多少数据。它影响命中列表的 I/O 时间，对于比未提示读取大小大的列表减少 I/O 时间，但对于较小的列表则增加 I/O 时间。它**不会**影响内存使用，因为读取缓冲区已经分配。因此，它不应大于 read_buffer。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_unhinted = 32K
```
<!-- end -->

### reset_network_timeout_on_packet

<!-- example conf reset_network_timeout_on_packet -->
细化网络超时（例如 `network_timeout`、`read_timeout` 和 `agent_query_timeout`）的行为。

设置为 0 时，超时限制发送整个请求/查询的最长时间。
设置为 1（默认），超时限制网络活动之间的最大时间。

使用复制时，一个节点可能需要向另一个节点发送一个大文件（例如 100GB）。假设网络传输速度为 1GB/s，一系列数据包大小为 4-5MB。传输整个文件需要 100 秒。默认 5 秒的超时只允许传输 5GB 数据，之后连接就会被断开。增加超时可以作为一种解决方法，但这不可扩展（例如，下一个文件可能是 150GB，仍会导致失败）。然而，默认 `reset_network_timeout_on_packet` 设为 1 时，超时不是应用于整个传输，而是应用于单个数据包。只要传输在进行中（且超时期间实际有数据通过网络接收），连接保持活动状态。如果传输卡住，导致数据包间发生超时，则连接会被断开。

请注意，如果您设置分布式表，所有节点——主节点和代理节点——都应调整。在主节点端，影响 `agent_query_timeout`；在代理节点端，影响 `network_timeout`。

<!-- intro -->

##### 示例：

<!-- request Example -->

```ini
reset_network_timeout_on_packet = 0
```

<!-- end -->


### rt_flush_period

<!-- example conf rt_flush_period -->
RT 表 RAM 块刷新检查周期，单位秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认值为 10 小时。

完全适合 RAM 块的活跃更新 RT 表仍可能导致 binlog 不断增长，影响磁盘使用和崩溃恢复时间。使用此指令，搜索服务器会定期执行刷新检查，符合条件的 RAM 块可以被保存，从而实现后续 binlog 清理。详情参见 [二进制日志](../Logging/Binary_logging.md)。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_flush_period = 3600 # 1 hour
```
<!-- end -->


### rt_merge_iops

<!-- example conf rt_merge_iops -->
RT 块合并线程允许启动的最大 I/O 操作数（每秒）。可选，默认值为 0（无限制）。

此指令让您限制因 `OPTIMIZE` 语句引起的 I/O 影响。保证所有 RT 优化活动所产生的磁盘 IOPS（每秒 I/O 操作数）不会超过配置的限制。限制 rt_merge_iops 可以减少因合并导致的搜索性能下降。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_iops = 40
```
<!-- end -->

### rt_merge_maxiosize

<!-- example conf rt_merge_maxiosize -->
RT 块合并线程允许启动的最大 I/O 操作大小。可选，默认值为 0（无限制）。

此指令让您限制因 `OPTIMIZE` 语句引起的 I/O 影响。超过此限制的 I/O 会被拆分成两个或更多 I/O，随后根据 [rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops) 限制分别计入。因此，保证所有优化活动产生的磁盘 I/O 不会超过 (rt_merge_iops * rt_merge_maxiosize) 字节每秒。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_maxiosize = 1M
```
<!-- end -->


### seamless_rotate

<!-- example conf seamless_rotate -->
防止在旋转具有大量数据的表以进行预缓存时导致 `searchd` 阻塞。可选，默认启用（1）。在 Windows 系统上，默认禁用无缝旋转。

表可能包含一些需要预先缓存到内存中的数据。目前，`.spa`、`.spb`、`.spi` 和 `.spm` 文件会被完全预缓存（它们分别包含属性数据、二进制属性数据、关键词表和已删除行映射）。在未启用无缝旋转的情况下，旋转表时尽量少用 RAM，工作流程如下：

1. 临时拒绝新的查询（返回“重试”错误代码）；
2. `searchd` 等待所有正在运行的查询完成；
3. 旧表被释放，其文件被重命名；
4. 新表文件被重命名，并分配所需的内存；
5. 新表的属性和字典数据被预加载到内存；
6. `searchd` 恢复从新表提供查询服务。

然而，如果属性或字典数据量很大，预加载步骤可能需要较长时间——预加载 1-5GB 以上文件时可能耗时几分钟。

启用无缝旋转时，旋转流程如下：

1. 分配新表的内存存储；
2. 异步预加载新表属性和字典数据到内存；
3. 成功后，释放旧表，重命名两个表的文件；
4. 失败时，释放新表；
5. 在任何时间点，查询要么从旧表，要么从新表副本提供服务。

无缝旋转的代价是旋转期间内存峰值更高（因为预加载新副本时，`.spa/.spb/.spi/.spm` 数据的旧副本和新副本都需加载到内存）。平均内存使用保持不变。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
seamless_rotate = 1
```
<!-- end -->

### secondary_indexes
<!-- example conf secondary_indexes -->

此选项启用或禁用搜索查询中的二级索引。可选，默认值为 1（启用）。注意，索引时无需启用它，只要安装了 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) 就始终启用。后者也是搜索时使用索引的必需条件。有三种模式可用：

* `0`：禁用搜索时使用二级索引。可通过[分析器提示](../Searching/Options.md#Query-optimizer-hints)为单个查询启用。
* `1`：启用搜索时使用二级索引。可以通过[分析器提示](../Searching/Options.md#Query-optimizer-hints)为单个查询禁用。
* `force`：与启用相同，但加载二级索引出错时会报告错误，且整个索引不会加载到守护进程中。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
secondary_indexes = 1
```

<!-- end -->

### server_id

<!-- example conf server_id -->
用作服务器标识符的整数，用于生成复制集群中节点唯一短 UUID 的种子。server_id 在集群节点间必须唯一，范围为 0 到 127。如果未设置 server_id，则根据 MAC 地址和 PID 文件路径的哈希计算，或使用随机数作为短 UUID 的种子。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
server_id = 1
```
<!-- end -->


### shutdown_timeout

<!-- example conf shutdown_timeout -->
`searchd --stopwait` 的等待时间，以秒为单位（或使用[特殊后缀](../Server_settings/Special_suffixes.md)）。可选，默认是60秒。

运行 `searchd --stopwait` 时，服务器需要执行一些停止前的活动，比如完成查询、刷新 RT RAM 块、刷新属性和更新 binlog，这些任务需要时间。`searchd --stopwait` 会等待最多 `shutdown_time` 秒让服务器完成这些工作。合适的时间取决于表大小和负载。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
shutdown_timeout = 3m # wait for up to 3 minutes
```
<!-- end -->


### shutdown_token

调用 VIP Manticore SQL 连接的 'shutdown' 命令所需的密码 SHA1 哈希值。若无此值，[debug](../Reporting_bugs.md#DEBUG) 模式下的 'shutdown' 子命令不会停止服务器。注意，这种简单哈希不应视为强保护措施，因为未使用盐或任何现代哈希算法。它仅作为局域网中家务守护进程的防错措施。

### snippets_file_prefix

<!-- example conf snippets_file_prefix -->
生成片段时附加到本地文件名的前缀。可选，默认是当前工作目录。

此前缀可与 `load_files` 或 `load_files_scattered` 选项一起用于分布式片段生成。

注意这是一个前缀，**不是**路径！这意味着如果设置前缀为 "server1" 且请求文件名是 "file23"，`searchd` 将尝试打开 "server1file23"（不含引号）。如果需要作为路径，应包含末尾斜杠。

构成最终文件路径后，服务器会解析所有相对目录，并将结果与 `snippet_file_prefix` 进行比较。如果结果不以该前缀开头，该文件将被拒绝并返回错误信息。

例如，若设置为 `/mnt/data` 并有请求用文件 `../../../etc/passwd` 生成片段，将收到错误：

`File '/mnt/data/../../../etc/passwd' escapes '/mnt/data/' scope`

而非文件内容。

另外，若未设置该参数且读取 `/etc/passwd`，实际路径为 /daemon/working/folder/etc/passwd，因为默认参数是服务器工作目录。

此选项是本地的，不影响代理(agent)的行为。因此，可以在主服务器上安全设置此前缀，路由给代理的请求不会受主服务器设置影响，但会受代理自身设置影响。

此举可能在文档存储位置（无论本地存储还是 NAS 挂载点）不一致的服务器环境中非常有用。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
snippets_file_prefix = /mnt/common/server1/
```
<!-- end -->

> **警告：** 如果您仍然想访问文件系统根目录的文件，必须显式将 `snippets_file_prefix` 设置为空值（通过 `snippets_file_prefix=` 行），或者设置为根目录（通过 `snippets_file_prefix=/`）。


### sphinxql_state

<!-- example conf sphinxql_state -->
当前 SQL 状态将被序列化到的文件路径。

在服务器启动时，会重放此文件。在符合条件的状态更改时（例如，SET GLOBAL），此文件会自动重写。这可以防止一个难以诊断的问题：如果您加载了 UDF 函数但 Manticore 崩溃，当它被（自动）重启时，您的 UDF 和全局变量将不再可用。使用持久状态有助于确保优雅恢复，没有此类意外。

`sphinxql_state` 不能用于执行任意命令，例如 `CREATE TABLE`。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
sphinxql_state = uservars.sql
```
<!-- end -->


### sphinxql_timeout

<!-- example conf sphinxql_timeout -->
使用 SQL 接口时，最大请求间等待时间（以秒为单位，或[特殊后缀](../Server_settings/Special_suffixes.md)）。可选，默认是 15 分钟。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
sphinxql_timeout = 15m
```
<!-- end -->


### ssl_ca

<!-- example conf ssl_ca -->
SSL 证书颁发机构（CA）证书文件路径（也称为根证书）。可选，默认为空。非空时，`ssl_cert` 中的证书应由此根证书签名。

服务器使用 CA 文件来验证证书的签名。该文件必须是 PEM 格式。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ssl_ca = keys/ca-cert.pem
```
<!-- end -->


### ssl_cert

<!-- example conf ssl_cert -->
服务器 SSL 证书路径。可选，默认为空。

服务器使用此证书作为自签名公钥，通过 SSL 加密 HTTP 流量。该文件必须是 PEM 格式。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ssl_cert = keys/server-cert.pem
```
<!-- end -->


### ssl_key

<!-- example conf ssl_key -->
SSL 证书私钥路径。可选，默认为空。

服务器使用此私钥通过 SSL 加密 HTTP 流量。该文件必须是 PEM 格式。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ssl_key = keys/server-key.pem
```
<!-- end -->


### subtree_docs_cache

<!-- example conf subtree_docs_cache -->
每查询最大公共子树文档缓存大小。可选，默认值为 0（禁用）。

此设置限制公共子树优化器的内存使用（参见 [多查询](../Searching/Multi-queries.md)）。最多将使用这么多内存来缓存每个查询的文档条目。设置为 0 禁用优化器。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
subtree_docs_cache = 8M
```
<!-- end -->


### subtree_hits_cache

<!-- example conf subtree_hits_cache -->
每查询最大公共子树命中缓存大小。可选，默认值为 0（禁用）。

此设置限制公共子树优化器的内存使用（参见 [多查询](../Searching/Multi-queries.md)）。最多将使用这么多内存来缓存每个查询的关键词出现（命中）。设置为 0 禁用优化器。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
subtree_hits_cache = 16M
```
<!-- end -->

### threads

<!-- example threads -->
Manticore 守护进程的工作线程数（或线程池大小）。Manticore 启动时创建该数量的操作系统线程，这些线程执行守护进程中的所有任务，如执行查询、创建摘要等。某些操作可能被拆分成子任务并行执行，例如：

* 在实时表中搜索
* 在由本地表组成的分布式表中搜索
* Percolate 查询调用
* 及其他

默认设置为服务器的 CPU 核心数。Manticore 启动时创建线程并保持运行直到停止。每个子任务需要时可以使用一个线程。子任务完成后，释放线程供其他子任务使用。

在密集 I/O 类型负载情况下，设置超过 CPU 核心数可能更合理。

<!-- request Example -->
```ini
threads = 10
```

<!-- end -->

### thread_stack

<!-- example conf thread_stack -->
作业的最大栈大小（协程，一个搜索查询可能产生多个作业/协程）。可选，默认是 128K。

每个作业都有 128K 的栈。当运行查询时，检测所需栈大小。如果默认的 128K 足够，则直接处理；如果需要更多，将调度另一个栈更大的作业继续处理。此设置限制此类高级栈的最大大小。

将值设置为合理较高的数值有助于处理非常深的查询，而不会导致整体内存消耗过高。例如，设置为 1G 并不意味着每个新作业都占用 1G 内存，但如果检测需要 100M，只分配 100M。其他作业仍使用默认 128K 栈。同理，可以运行需要 500M 的复杂查询。仅当作业需要超过 1G 栈时，才会失败并报告 thread_stack 过低。

然而，实际上即使需要 16M 栈的查询通常对解析来说过于复杂，且消耗大量时间和资源。守护进程会处理此类查询，但通过 `thread_stack` 限制这类查询是合理的。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
thread_stack = 8M
```
<!-- end -->


### unlink_old

<!-- example conf unlink_old -->
决定是否在成功轮换后解绑 `.old` 表副本。可选，默认值为 1（执行解绑）。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
unlink_old = 0
```
<!-- end -->


### watchdog

<!-- example conf watchdog -->
线程化服务器看门狗。可选，默认值为 1（启用看门狗）。

当 Manticore 查询崩溃时，可能会导致整个服务器宕机。启用看门狗功能后，`searchd` 会维护一个单独的轻量级进程，该进程监控主服务器进程，并在异常终止时自动重启它。看门狗默认启用。

<!-- request Example -->

```ini
watchdog = 0 # disable watchdog
```
<!-- end -->
<!-- proofread -->

