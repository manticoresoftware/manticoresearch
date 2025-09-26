# 配置中的 "searchd" 部分

以下设置用于 Manticore Search 配置文件的 `searchd` 部分，以控制服务器的行为。下面是每个设置的摘要：

### access_plain_attrs

此设置为 [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。此项为可选，默认值为 `mmap_preread`。

`access_plain_attrs` 指令允许您为此 searchd 实例管理的所有表定义 [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令优先级更高，将覆盖此实例范围的默认值，从而提供更精细的控制。

### access_blob_attrs

此设置为 [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。此项为可选，默认值为 `mmap_preread`。

`access_blob_attrs` 指令允许您为此 searchd 实例管理的所有表定义 [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令优先级更高，将覆盖此实例范围的默认值，从而提供更精细的控制。

### access_doclists

此设置为 [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。此项为可选，默认值为 `file`。

`access_doclists` 指令允许您为此 searchd 实例管理的所有表定义 [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令优先级更高，将覆盖此实例范围的默认值，从而提供更精细的控制。

### access_hitlists

此设置为 [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。此项为可选，默认值为 `file`。

`access_hitlists` 指令允许您为此 searchd 实例管理的所有表定义 [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令优先级更高，将覆盖此实例范围的默认值，从而提供更精细的控制。

### access_dict

此设置为 [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。此项为可选，默认值为 `mmap_preread`。

`access_dict` 指令允许您为此 searchd 实例管理的所有表定义 [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令优先级更高，将覆盖此实例范围的默认值，从而提供更精细的控制。

### agent_connect_timeout

此设置为 [agent_connect_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) 参数设置实例范围的默认值。


### agent_query_timeout

此设置为 [agent_query_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) 参数设置实例范围的默认值。可以通过 `OPTION agent_query_timeout=XXX` 子句在每次查询时覆盖。


### agent_retry_count

此设置为整数，指定 Manticore 在通过分布式表连接和查询远程代理时尝试的次数，超过后将报告致命查询错误。默认值为 0（即不重试）。您还可以通过 `OPTION retry_count=XXX` 子句在每次查询时设置此值。如果提供了每次查询的选项，它将覆盖配置中指定的值。

请注意，如果您在定义分布式表时使用了[代理镜像](../Creating_a_cluster/Remote_nodes/Mirroring.md#Agent-mirrors)，服务器将根据所选的 [ha_strategy](../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) 为每次连接尝试选择不同的镜像。在这种情况下，`agent_retry_count` 将针对一组内的所有镜像进行汇总。

例如，如果您有 10 个镜像并设置了 `agent_retry_count=5`，则服务器将最多重试 50 次，假设每个镜像平均尝试 5 次（如果选择了 `ha_strategy = roundrobin` 选项，则情况如此）。

然而，作为 [agent](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) 定义中 `retry_count` 选项提供的值是绝对限制。换言之，agent 定义中的 `[retry_count=2]` 选项始终意味着最多尝试 2 次，无论您为该代理指定了 1 个还是 10 个镜像。

### agent_retry_delay

此设置为以毫秒为单位的整数（或 [special_suffixes](../Server_settings/Special_suffixes.md)），指定 Manticore 在失败后重试查询远程代理前的延迟时间。仅在指定非零的 [agent_retry_count](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 或非零的每次查询 `retry_count` 时此值才有效。默认值为 500。您还可以通过 `OPTION retry_delay=XXX` 子句在每次查询时设置此值。如果提供了每次查询的选项，它将覆盖配置中指定的值。


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

二进制日志用于RT表数据的崩溃恢复以及平面磁盘索引属性的更新，否则这些更新只会存储在RAM中直到刷新。当启用日志记录时，RT表中每个提交的事务都会写入日志文件。在非正常关闭后启动时，日志将自动重放，以恢复记录的更改。

`binlog_path`指令指定二进制日志文件的位置。它应只包含路径；`searchd`将在必要时在该目录中创建和删除多个`binlog.*`文件（包括binlog数据、元数据和锁文件等）。

空值禁用二进制日志，这会提升性能，但会使RT表数据处于风险中。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
binlog_path = # disable logging
binlog_path = /var/lib/manticore/data # /var/lib/manticore/data/binlog.001 etc will be created
```
<!-- end -->

### boolean_simplify

<!-- example conf boolean_simplify -->
此设置控制[boolean_simplify](../Searching/Options.md#boolean_simplify)搜索选项的默认值。此项为可选，默认值为1（启用）。

设置为1时，服务器将自动应用[布尔查询优化](../Searching/Full_text_matching/Boolean_optimization.md)以提升查询性能。设置为0时，查询默认将不使用优化执行。该默认值可以通过对应的搜索选项`boolean_simplify`在每次查询时覆盖。

<!-- request Example -->
```ini
searchd {
    boolean_simplify = 0  # disable boolean optimization by default
}
```
<!-- end -->

### buddy_path

<!-- example conf buddy_path -->
此设置确定Manticore Buddy二进制文件的路径。此项为可选，默认值为构建时配置的路径，不同操作系统有所差异。通常不需要修改此设置。不过，如果您希望以调试模式运行Manticore Buddy、修改Manticore Buddy或实现新插件时，这个设置可能会有用。在后一种情况下，您可以从https://github.com/manticoresoftware/manticoresearch-buddy `git clone` Buddy，向`./plugins/`目录添加新插件，并在切换到Buddy源代码目录后运行`composer install --prefer-source`以便于开发。

为确保您能运行`composer`，您的机器必须安装PHP 8.2或更高版本，并具备以下扩展：

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

您也可以选择发布中提供的Linux amd64专用版本`manticore-executor-dev`，例如：https://github.com/manticoresoftware/executor/releases/tag/v1.0.13

若采用此方案，请记得将开发版本的manticore执行器链接到`/usr/bin/php`。

要禁用Manticore Buddy，请将值设为空，如示例所示。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
buddy_path = manticore-executor -n /usr/share/manticore/modules/manticore-buddy/src/main.php # use the default Manticore Buddy in Linux
buddy_path = manticore-executor -n /usr/share/manticore/modules/manticore-buddy/src/main.php --threads=1 # runs Buddy with a single worker
buddy_path = manticore-executor -n /opt/homebrew/share/manticore/modules/manticore-buddy/bin/manticore-buddy/src/main.php # use the default Manticore Buddy in MacOS arm64
buddy_path = manticore-executor -n /Users/username/manticoresearch-buddy/src/main.php # use Manticore Buddy from a non-default location
buddy_path = # disables Manticore Buddy
buddy_path = manticore-executor -n /Users/username/manticoresearch-buddy/src/main.php --skip=manticoresoftware/buddy-plugin-replace # --skip - skips plugins
buddy_path = manticore-executor -n /usr/share/manticore/modules/manticore-buddy/src/main.php --enable-plugin=manticoresoftware/buddy-plugin-show # runs Buddy with only the SHOW plugin
```
<!-- end -->

### client_timeout

<!-- example conf client_timeout -->
此设置决定在使用持久连接时请求之间的最大等待时间（以秒或[special_suffixes](../Server_settings/Special_suffixes.md)表示）。此项为可选，默认值为五分钟。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
client_timeout = 1h
```
<!-- end -->


### collation_libc_locale

<!-- example conf collation_libc_locale -->
服务器libc区域设置。可选，默认值为C。

指定libc区域设置，影响基于libc的排序规则。详情请参考[排序规则](../Searching/Collations.md)章节。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
collation_libc_locale = fr_FR
```
<!-- end -->


### collation_server

<!-- example conf collation_server -->
默认服务器排序规则。可选，默认值为libc_ci。

指定被用于传入请求的默认排序规则。排序规则可以在每次查询时覆盖。可参考[排序规则](../Searching/Collations.md)章节以查看可用排序规则及其他细节。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
collation_server = utf8_ci
```
<!-- end -->


### data_dir

<!-- example conf data_dir -->
指定此项启用[实时模式](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)，这是一种动态管理数据模式的方式。其值应为存储所有表、二进制日志及运行Manticore Search所需所有内容的目录路径。
当指定`data_dir`时，不允许向[平面表](../Creating_a_table/Local_tables/Plain_table.md)建立索引。有关RT模式和平面模式的区别，请阅读[本节](../Read_this_first.md#Real-time-table-vs-plain-table)。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
data_dir = /var/lib/manticore
```
<!-- end -->

### diskchunk_flush_search_timeout

<!-- example conf diskchunk_flush_search_timeout -->
为防止在表中无搜索时自动刷新RAM块而设置的超时时间。可选，默认值为30秒。

检查表中搜索活动的时长，以判断是否进行自动刷新。
仅当在最近的`diskchunk_flush_search_timeout`秒内表内至少有一次搜索时，才会自动刷新。此参数与[diskchunk_flush_write_timeout](../Server_settings/Searchd.md#diskchunk_flush_write_timeout)配合使用。对应的[每表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_search_timeout)优先级更高，将覆盖该实例的默认值，从而实现更细粒度的控制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
diskchunk_flush_search_timeout = 120s
```
<!-- end -->

### diskchunk_flush_write_timeout

<!-- example conf diskchunk_flush_write_timeout -->
在无写操作后等待多少秒才自动将RAM块刷写到磁盘。可选，默认值为1秒。

If no write occurs in the RAM chunk within `diskchunk_flush_write_timeout` seconds, the chunk will be flushed to disk. Works in conjunction with [diskchunk_flush_search_timeout](../Server_settings/Searchd.md#diskchunk_flush_search_timeout). To disable auto-flush, set `diskchunk_flush_write_timeout = -1` explicitly in your configuration. The corresponding [per-table setting](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_write_timeout) has a higher priority and will override this instance-wide default, providing more fine-grained control.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
diskchunk_flush_write_timeout = 60s
```
<!-- end -->

### docstore_cache_size

<!-- example conf docstore_cache_size -->
This setting specifies the maximum size of document blocks from document storage that are held in memory. It is optional, with a default value of 16m (16 megabytes).

When `stored_fields` is used, document blocks are read from disk and uncompressed. Since every block typically holds several documents, it may be reused when processing the next document. For this purpose, the block is held in a server-wide cache. The cache holds uncompressed blocks.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
docstore_cache_size = 8m
```
<!-- end -->

### engine

<!-- example conf engine -->
Default attribute storage engine used when creating tables in RT mode. Can be `rowwise` (default) or `columnar`.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
engine = columnar
```
<!-- end -->


### expansion_limit

<!-- example conf expansion_limit -->
This setting determines the maximum number of expanded keywords for a single wildcard. It is optional, with a default value of 0 (no limit).

When performing substring searches against tables built with `dict = keywords` enabled, a single wildcard may potentially result in thousands or even millions of matched keywords (think of matching `a*` against the entire Oxford dictionary). This directive allows you to limit the impact of such expansions. Setting `expansion_limit = N` restricts expansions to no more than N of the most frequent matching keywords (per each wildcard in the query).

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
expansion_limit = 16
```
<!-- end -->

### expansion_merge_threshold_docs

<!-- example conf expansion_merge_threshold_docs -->
This setting determines the maximum number of documents in the expanded keyword that allows merging all such keywords together. It is optional, with a default value of 32.

When performing substring searches against tables built with `dict = keywords` enabled, a single wildcard may potentially result in thousands or even millions of matched keywords. This directive allows you to increase the limit of how many keywords will merge together to speed up matching but uses more memory in the search.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
expansion_merge_threshold_docs = 1024
```
<!-- end -->

### expansion_merge_threshold_hits

<!-- example conf expansion_merge_threshold_hits -->
This setting determines the maximum number of hits in the expanded keyword that allows merging all such keywords together. It is optional, with a default value of 256.

When performing substring searches against tables built with `dict = keywords` enabled, a single wildcard may potentially result in thousands or even millions of matched keywords. This directive allows you to increase the limit of how many keywords will merge together to speed up matching but uses more memory in the search.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
expansion_merge_threshold_hits = 512
```
<!-- end -->

### expansion_phrase_limit

<!-- example conf expansion_phrase_limit -->
This setting controls the maximum number of alternative phrase variants generated due to `OR` operators inside `PHRASE`, `PROXIMITY`, and `QUORUM` operators. It is optional, with a default value of 1024.

When using the `|` (OR) operator inside phrase-like operator, the total number of expanded combinations may grow exponentially depending on the number of alternatives specified. This setting helps prevent excessive query expansion by capping the number of permutations considered during query processing. If the number of generated variants exceeds this limit, the query will fail with an error.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
expansion_phrase_limit = 4096
```
<!-- end -->

### grouping_in_utc

This setting specifies whether timed grouping in API and SQL will be calculated in the local timezone or in UTC. It is optional, with a default value of 0 (meaning 'local timezone').

By default, all 'group by time' expressions (like group by day, week, month, and year in API, also group by day, month, year, yearmonth, yearmonthday in SQL) are done using local time. For example, if you have documents with attributes timed `13:00 utc` and `15:00 utc`, in the case of grouping, they both will fall into facility groups according to your local timezone setting. If you live in `utc`, it will be one day, but if you live in `utc+10`, then these documents will be matched into different `group by day` facility groups (since 13:00 utc in UTC+10 timezone is 23:00 local time, but 15:00 is 01:00 of the next day). Sometimes such behavior is unacceptable, and it is desirable to make time grouping not dependent on timezone. You can run the server with a defined global TZ environment variable, but it will affect not only grouping but also timestamping in the logs, which may be undesirable as well. Switching 'on' this option (either in config or using [SET global](../Server_settings/Setting_variables_online.md#SET) statement in SQL) will cause all time grouping expressions to be calculated in UTC, leaving the rest of time-depentend functions (i.e. logging of the server) in local TZ.


### timezone

This setting specifies the timezone to be used by date/time-related functions. By default, the local timezone is used, but you can specify a different timezone in IANA format (e.g., `Europe/Amsterdam`).

Note that this setting has no impact on logging, which always operates in the local timezone.

另外，请注意如果使用 `grouping_in_utc`，‘按时间分组’功能仍将使用 UTC，而其他与日期/时间相关的功能将使用指定的时区。总体而言，不建议混合使用 `grouping_in_utc` 和 `timezone`。

您可以通过配置文件中配置此选项，或使用 SQL 中的 [SET global](../Server_settings/Setting_variables_online.md#SET) 语句来设置。


### ha_period_karma

<!-- example conf ha_period_karma -->
此设置指定代理镜像统计窗口大小，单位为秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。该项为可选，默认值为 60 秒。

对于包含代理镜像的分布式表（详见 [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)），主节点跟踪几个不同的每镜像计数器。这些计数器随后用于故障转移和平衡（主节点根据计数器选择最佳镜像使用）。计数器在 `ha_period_karma` 秒的时间块中累计。

开始新时间块后，主节点仍可能使用前一块累计的值，直到新块填充到一半。因此，任何之前的历史最多在 1.5 倍 ha_period_karma 秒后不再影响镜像选择。

虽然最多使用两个时间块进行镜像选择，但最多存储最近 15 个时间块以供监控使用。这些时间块可通过 [SHOW AGENT STATUS](../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS) 语句检查。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_period_karma = 2m
```
<!-- end -->


### ha_ping_interval

<!-- example conf ha_ping_interval -->
此设置配置代理镜像 ping 的间隔时间，单位为毫秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。该项为可选，默认值为 1000 毫秒。

对于包含代理镜像的分布式表（详见 [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)），主节点在空闲期间向所有镜像发送 ping 命令。目的是跟踪当前代理状态（存活或死亡、网络往返时间等）。此指令定义了 ping 之间的间隔。若要禁用 ping，请将 ha_ping_interval 设置为 0。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_ping_interval = 3s
```
<!-- end -->


### hostname_lookup

`hostname_lookup` 选项定义主机名更新策略。默认情况下，代理主机名的 IP 地址在服务器启动时缓存，以避免过度访问 DNS。然而，在某些情况下，IP 可能动态变化（如云托管），这时可能希望不缓存 IP。将此选项设置为 `request` 可禁用缓存，每次查询时都访问 DNS。IP 地址也可以通过 `FLUSH HOSTNAMES` 命令手动更新。

### jobs_queue_size

jobs_queue_size 设置定义队列中可以同时存在的“作业”数量。默认情况下不限制。

在大多数情况下，“作业”意味着对单个本地表（普通表或实时表的磁盘分块）的一次查询。例如，如果您有一个由 2 个本地表组成的分布式表，或者一个拥有两个磁盘分块的实时表，针对其中任一个的查询大多会在队列中产生 2 个作业。然后，线程池（其大小由 [threads](../Server_settings/Searchd.md#threads) 定义）处理这些作业。但在某些情况下，如果查询过于复杂，可能会产生更多作业。当 [max_connections](../Server_settings/Searchd.md#max_connections) 与 [threads](../Server_settings/Searchd.md#threads) 无法满足性能平衡需求时，建议调整此设置。

### join_batch_size

表连接通过累积匹配批次实现，匹配是对左表执行查询的结果。该批次随后作为单个查询在右表上执行。

此选项允许调整批次大小。默认值为 `1000`，设置为 `0` 则禁用批处理。

较大的批次大小可能提升性能，但对于某些查询，可能导致内存消耗过大。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
join_batch_size = 2000
```
<!-- end -->

### join_cache_size

针对右表执行的每个查询由特定的 JOIN ON 条件定义，这决定了从右表检索的结果集。

如果只有少量唯一的 JOIN ON 条件，重复使用结果比重复执行右表查询更高效。为此，结果集会被存储在缓存中。

此选项允许配置该缓存大小。默认值为 `20 MB`，设置为 0 则禁用缓存。

注意每个线程维护自己的缓存，因此估算总内存使用时应考虑执行查询的线程数。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
join_cache_size = 10M
```
<!-- end -->

### listen_backlog

<!-- example conf listen_backlog -->
listen_backlog 设置决定了 TCP 监听队列的长度，用于接收连接。该设置在逐个处理请求的 Windows 版本中尤为重要。当连接队列达到上限时，新来的连接将被拒绝。
对于非 Windows 版本，默认值通常合适，通常无需调整此设置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
listen_backlog = 20
```
<!-- end -->

### kibana_version_string

<!-- example conf kibana_version_string -->
供 Kibana 或 OpenSearch Dashboards 返回的服务器版本字符串。可选，默认设置为 `7.6.0`。

某些版本的 Kibana 和 OpenSearch Dashboards 期望服务器报告特定版本号，且可能根据此表现出不同的行为。为解决此类问题，您可以使用此设置，使 Manticore 向 Kibana 或 OpenSearch Dashboards 报告自定义版本。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
kibana_version_string = 1.2.3
```
<!-- end -->

### listen

<!-- example conf listen -->
此设置允许您指定 Manticore 将接受连接的 IP 地址和端口，或 Unix 域套接字路径。

`listen` 的通用语法为：

```ini
listen = ( address ":" port | port | path | address ":" port start - port end ) [ ":" protocol [ "_vip" ] [ "_readonly" ] ]
```

您可以指定：
* IP 地址（或主机名）及端口号
* 或仅端口号
* 或 Unix 套接字路径（Windows 不支持）
* 或 IP 地址和端口范围

如果您指定了端口号但未指定地址，`searchd` 将监听所有网络接口。Unix 路径以斜杠开头标识。端口范围只能为复制协议设置。

您还可以为该套接字上的连接指定协议处理器（监听器）。监听器有：

* **未指定** - Manticore 将在此端口接受以下连接：
  - 来自其他 Manticore 代理（例如远程分布式表）
  - 通过 HTTP 和 HTTPS 的客户端
  - [Manticore Buddy](https://manticoresearch.com/blog/manticoresearch-buddy-intro/)。**确保您已经有此类监听器（或下文提到的 `http` 监听器），以避免 Manticore 功能受限。**
* `mysql` MySQL 协议，适用于 MySQL 客户端连接。注意：
  - 也支持压缩协议。
  - 若启用了 [SSL](../Security/SSL.md#SSL)，可以建立加密连接。
* `replication` - 用于节点通信的复制协议。详细信息请参见 [replication](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md) 部分。您可以指定多个复制监听器，但它们必须监听相同的 IP，端口可以不同。当您定义带端口范围的复制监听器（例如：`listen = 192.168.0.1:9320-9328:replication`）时，Manticore 不会立即在这些端口开始监听。而是在开始使用复制时，从指定范围内随机选择可用端口。复制需要范围内至少有 2 个端口才能正常工作。
* `http` - 与 **未指定** 相同。Manticore 将在此端口接受来自远程代理和客户端的 HTTP 和 HTTPS 连接。
* `https` - HTTPS 协议。Manticore 在此端口只接收 **HTTPS** 连接。详情参见 [SSL](../Security/SSL.md) 部分。
* `sphinx` - 旧版二进制协议。用于服务来自远程 [SphinxSE](../Extensions/SphinxSE.md) 客户端的连接。一些 Sphinx API 客户端实现（例如 Java 版本）需要显式声明监听器。

为客户端协议添加后缀 `_vip`（即除 `replication` 之外所有协议，如 `mysql_vip`、`http_vip` 或仅 `_vip`）会强制为连接创建专用线程，以绕过各种限制。在服务器严重负载时，此举有助于节点维护，避免服务器卡死或无法通过常规端口连接。

后缀 `_readonly` 设置监听器为 [只读模式](../Security/Read_only.md)，仅允许接受只读查询。

<!-- intro -->
##### 例子：

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

可设置多个 `listen` 指令。`searchd` 将在所有指定的端口和套接字上监听客户端连接。Manticore 包中提供的默认配置定义了监听端口：
* `9308` 和 `9312` 用于来自远程代理和非 MySQL 客户端的连接
* 以及端口 `9306` 用于 MySQL 连接。

如果配置中完全未指定 `listen`，Manticore 将监听：
* `127.0.0.1:9306` 用于 MySQL 客户端
* `127.0.0.1:9312` 用于 HTTP/HTTPS 以及来自其他 Manticore 节点和基于 Manticore 二进制 API 客户端的连接。

#### 监听特权端口

默认情况下，Linux 不允许 Manticore 监听 1024 以下的端口（例如 `listen = 127.0.0.1:80:http` 或 `listen = 127.0.0.1:443:https`），除非以 root 用户身份运行 searchd。如果您希望在非 root 用户下让 Manticore 监听 < 1024 端口，请考虑以下操作之一（任选其一均可）：
* 运行命令 `setcap CAP_NET_BIND_SERVICE=+eip /usr/bin/searchd`
* 在 Manticore 的 systemd 单元中添加 `AmbientCapabilities=CAP_NET_BIND_SERVICE` 并重新加载守护进程 (`systemctl daemon-reload`)。

#### 关于 Sphinx API 协议及 TFO 的技术细节
<details>
传统的 Sphinx 协议有两个阶段：握手交换和数据流。握手包括客户端发送的 4 字节包，以及守护进程发送的 4 字节包，唯一目的——客户端确认远程是真实的 Sphinx 守护进程，守护进程确认远程是真实的 Sphinx 客户端。主要数据流很简单：双方都声明各自的握手，另一方校验。此短包交换使用特殊的 `TCP_NODELAY` 标志，禁用 Nagle 的 TCP 算法，指示 TCP 连接将作为小包对话执行。
但握手协商中谁先发言并无严格定义。历史上，所有使用二进制 API 的客户端先发送握手，然后读取守护进程的 4 字节包，再发送请求并读取守护进程响应。
我们改进协议兼容性时考虑：

1. 通常，主服务器与代理通信是已知客户端连接到已知主机的已知端口，几乎不可能出现错误握手。因此，可以隐式假定双方均有效且使用 Sphinx 协议。
2. 基于此假设，我们可将握手“粘合”到真实请求，一起包发送。若后端为传统 Sphinx 守护进程，它会先读取这 4 字节握手，再读取请求体。因两者在一个包里，后端 socket 减少一个往返时间，前端缓存照常工作。
3. 继续假设：由于“query”包非常小，而握手包更小，让我们使用现代的TFO（tcp-fast-open）技术将两者都发送在初始的“SYN” TCP包中。也就是说：我们用绑定的握手+主体包连接到远程节点。守护进程接受连接，并立即在套接字缓冲区中获得握手和主体，因为它们是在第一个TCP“SYN”包中同时到达的。这样就消除了一个往返时间（RTT）。
4. 最后，教守护进程接受此改进。实际上，从应用程序来看，这意味着不要使用`TCP_NODELAY`。从系统层面来看，这意味着确保守护进程端启用了TFO接受，客户端启用了TFO发送。在现代系统中，默认客户端TFO已经启用，因此只需调整服务器端的TFO即可使所有功能正常工作。

所有这些改进都没有实际改变协议本身，却允许我们消除连接中TCP协议的1.5个RTT。如果查询和回答能够放入一个TCP包中，则将整个二进制API会话从3.5 RTT减少到2 RTT —— 这使得网络协商速度大约提升了2倍。

因此，我们所有的改进都围绕一个最初未定义的声明：“谁先发送”。如果客户端先发送，我们可以应用所有这些优化，有效地在一个TFO包中完成连接+握手+查询的处理。此外，我们可以查看收到包的开头并确定真实协议。这就是为什么你可以通过API/http/https连接到同一个端口的原因。如果守护进程必须先发送，所有这些优化都不可能实现，多协议也不可能实现。这就是为什么我们为MySQL设置了专用端口，而未将其与其他协议合并到同一个端口。突然地，在所有客户端中，有一个客户端的设计假设守护进程应先发送握手，即不可能进行上述所有改进。那就是mysql/mariadb的SphinxSE插件。因此，专为这个客户端，我们定义了`sphinx`协议以最传统的方式工作。即：双方均启用`TCP_NODELAY`并交换小包。守护进程在连接时发送握手，随后客户端发送握手，然后一切按常规运行。这不是很优化，但确实可行。如果你使用SphinxSE连接到Manticore —— 你必须专门设置一个明确指定为`sphinx`协议的监听器。对于其他客户端，避免使用这个监听器，因为它较慢。如果你使用其他传统的Sphinx API客户端，首先检查它们能否与非专用多协议端口一起工作。对于主从代理连接，使用非专用（多协议）端口并启用客户端和服务器TFO效果良好，并且肯定会使网络后端更快，尤其是当你有非常轻量且快速的查询时。
</details>

### listen_tfo

此设置允许所有监听器使用TCP_FASTOPEN标志。默认由系统管理，但可以通过设置为“0”显式关闭。

关于TCP快速打开扩展的通用知识，请参阅[维基百科](https://en.wikipedia.org/wiki/TCP_Fast_Open)。简言之，它减少一次TCP往返时间来建立连接。

在实际应用中，使用TFO在许多场景下可以优化客户端-代理的网络效率，就像[持久代理](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)那样，但无需保持活跃连接，也没有最大连接数的限制。

在现代操作系统中，通常系统层面默认开启TFO支持，但这只是“能力”而非必然。Linux（作为最先进的系统）从2011年的3.7内核开始支持服务器端。Windows从某些Windows 10版本开始支持。其他操作系统（FreeBSD、MacOS）也支持。

Linux系统服务器检查变量`/proc/sys/net/ipv4/tcp_fastopen`并根据其值行为。第0位控制客户端，第1位控制监听器。默认系统设置为1，即客户端启用，监听器禁用。

### log

<!-- example conf log -->
log设置指定记录所有`searchd`运行事件的日志文件名。如果未指定，默认名为“searchd.log”。

另外，你也可以使用“syslog”作为文件名。此时，事件将发送至syslog守护进程。要使用syslog选项，构建Manticore时需使用`--with-syslog`选项。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
log = /var/log/searchd.log
```
<!-- end -->


### max_batch_queries

<!-- example conf max_batch_queries -->
限制每批次的查询数量。可选，默认值为32。

使searchd在使用[多查询](../Searching/Multi-queries.md)时对单批提交的查询数量进行合理检查。设置为0可跳过检查。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_batch_queries = 256
```
<!-- end -->

### max_connections

<!-- example max_connections -->
最大同时客户端连接数。默认无限制。通常只有在使用某种持久连接（如cli mysql会话或远程分布式表的持久远程连接）时才会显著。当超过限制时，你仍然可以使用[VIP连接](../Connecting_to_the_server/MySQL_protocol.md#VIP-connection)连接服务器。VIP连接不计入限制内。

<!-- request Example -->
```ini
max_connections = 10
```

<!-- end -->

### max_threads_per_query

<!-- example max_threads_per_query -->
实例范围内单个操作可使用的线程数限制。默认情况下，适当的操作可以占用所有CPU核心，导致其他操作无法使用。例如，对一个相当大的percolate表执行`call pq`操作可以利用所有线程，持续数十秒。将`max_threads_per_query`设置为比如说[threads](../Server_settings/Searchd.md#threads)的一半，可以确保你能够并行运行几个这样的`call pq`操作。

你也可以在运行时将此设置作为会话变量或全局变量设置。

此外，你可以借助[threads OPTION](../Searching/Options.md#threads)按查询控制其行为。

<!-- intro -->
##### 示例：
<!-- request Example -->

```ini
max_threads_per_query = 4
```

<!-- end -->

### max_filters

<!-- example conf max_filters -->
每查询允许的最大过滤器数量。此设置仅用于内部合理性检查，并不会直接影响RAM使用或性能。可选，默认值为256。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_filters = 1024
```
<!-- end -->


### max_filter_values

<!-- example conf max_filter_values -->
每过滤器允许的最大值数量。此设置仅用于内部合理性检查，并不会直接影响RAM使用或性能。可选，默认值为4096。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_filter_values = 16384
```
<!-- end -->


### max_open_files

<!-- example conf max_open_files -->
服务器允许打开的最大文件数称为“软限制”。请注意，服务大型碎片化实时表时可能需要将此限制设置得较高，因为每个磁盘块可能占用十几个或更多文件。例如，一个拥有1000个块的实时表可能需要同时打开数千个文件。如果你在日志中遇到“打开的文件过多”错误，尝试调整此选项，可能有助于解决问题。

还有一个“硬限制”，该限制不能被此选项超越。这个限制由系统定义，可以在Linux系统的`/etc/security/limits.conf`文件中更改。其他操作系统可能有不同的方法，详情请查阅相关手册。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = 10000
```
<!-- end -->

<!-- example conf max_open_files max -->
除了直接的数值，你还可以使用关键字“max”将限制设置为当前可用的硬限制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = max
```
<!-- end -->


### max_packet_size

<!-- example conf max_packet_size -->
允许的最大网络数据包大小。此设置限制来自客户端的查询包和分布式环境中远程代理的响应包。仅用于内部合理性检查，且不会直接影响RAM使用或性能。可选，默认值为128M。


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

一些挑剔的MySQL客户端库依赖MySQL所使用的特定版本号格式，而且有时基于报告的版本号（而非标明的能力标志）选择不同的执行路径。例如，Python MySQLdb 1.2.2当版本号不是X.Y.ZZ格式时会抛出异常；MySQL .NET 连接器6.3.x在版本号为1.x且配合某些标志组合时内部失败等。为解决这些问题，你可以使用`mysql_version_string`指令，让`searchd`向通过MySQL协议连接的客户端报告不同的版本号。（默认情况下，它报告自身版本。）


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
mysql_version_string = 5.0.37
```
<!-- end -->


### net_workers

网络线程数量，默认值是1。

当查询率非常高且单个线程不足以管理所有传入查询时，此设置非常有用。


### net_wait_tm

控制网络线程的忙等待循环间隔。默认值为-1，可设为-1、0或正整数。

若服务器配置为纯主节点，仅负责将请求路由至代理，必须无延迟地处理请求，并且网络线程不得休眠。有一个忙循环实现该功能。在收到请求后，若`net_wait_tm`为正数，网络线程会用CPU轮询`10 * net_wait_tm`毫秒；若`net_wait_tm`为0，则只用CPU轮询。此外，可以用`net_wait_tm = -1`禁用忙循环——在此情况下，轮询器在系统调用中将超时设置为实际代理的超时。

> **警告：** CPU忙循环会实际占用CPU核心资源，所以将此值设置为非默认值将在服务器空闲时也产生明显的CPU使用率。


### net_throttle_accept

定义网络循环每次迭代接收的客户端数量。默认是0（无限制），对大多数用户足够用。此为调优选项，用于在高负载场景下控制网络循环的吞吐量。


### net_throttle_action

定义网络循环每次迭代处理的请求数。默认是0（无限制），对大多数用户足够用。此为调优选项，用于在高负载场景下控制网络循环的吞吐量。

### network_timeout

<!-- example conf network_timeout -->
网络客户端请求读写超时时间，单位为秒（或[special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认值为5秒。`searchd`将在客户端未能在此超时时间内发送查询或读取结果时强制关闭连接。

Note also the [reset_network_timeout_on_packet](../Server_settings/Searchd.md#reset_network_timeout_on_packet) parameter. This parameter alters the behavior of `network_timeout` from applying to the entire `query` or `result` to individual packets instead. Typically, a query/result fits within one or two packets. However, in cases where a large amount of data is required, this parameter can be invaluable in maintaining active operations.

<!-- request Example -->

```ini
network_timeout = 10s
```
<!-- end -->

### node_address

<!-- example conf node_address -->
This setting allows you to specify the network address of the node. By default, it is set to the replication [listen](../Server_settings/Searchd.md#listen) address. This is correct in most cases; however, there are situations where you have to specify it manually:

* Node behind a firewall
* Network address translation enabled (NAT)
* Container deployments, such as Docker or cloud deployments
* Clusters with nodes in more than one region


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
node_address = 10.101.0.10
```
<!-- end -->

### not_terms_only_allowed

<!-- example conf not_terms_only_allowed -->
This setting determines whether to allow queries with only the [negation](../Searching/Full_text_matching/Operators.md#Negation-operator) full-text operator. Optional, the default is 0 (fail queries with only the NOT operator).


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
not_terms_only_allowed = 1
```
<!-- end -->

### optimize_cutoff

<!-- example conf optimize_cutoff -->
Sets the default table compaction threshold. Read more here - [Number of optimized disk chunks](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks). This setting can be overridden with the per-query option [cutoff](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks). It can also be changed dynamically via [SET GLOBAL](../Server_settings/Setting_variables_online.md#SET).

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
optimize_cutoff = 4
```
<!-- end -->

### persistent_connections_limit

<!-- example conf persistent_connections_limit -->
This setting determines the maximum number of simultaneous persistent connections to remote [persistent agents](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md). Each time an agent defined under `agent_persistent` is connected, we try to reuse an existing connection (if any), or connect and save the connection for future use. However, in some cases, it makes sense to limit the number of such persistent connections. This directive defines the limit. It affects the number of connections to each agent's host across all distributed tables.

It is reasonable to set the value equal to or less than the [max_connections](../Server_settings/Searchd.md#max_connections) option in the agent's config.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
persistent_connections_limit = 29 # assume that each host of agents has max_connections = 30 (or 29).
```
<!-- end -->


### pid_file

<!-- example conf pid_file -->
pid_file is a mandatory configuration option in Manticore search that specifies the path of the file where the process ID of the `searchd` server is stored.

The searchd process ID file is re-created and locked on startup, and contains the head server process ID while the server is running. It is unlinked on server shutdown.
The purpose of this file is to enable Manticore to perform various internal tasks, such as checking whether there is already a running instance of `searchd`, stopping `searchd`, and notifying it that it should rotate the tables. The file can also be used for external automation scripts.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
pid_file = /run/manticore/searchd.pid
```
<!-- end -->


### predicted_time_costs

<!-- example conf predicted_time_costs -->
Costs for the query time prediction model, in nanoseconds. Optional, the default is `doc=64, hit=48, skip=2048, match=64`.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
predicted_time_costs = doc=128, hit=96, skip=4096, match=128
```
<!-- end -->

<!-- example conf predicted_time_costs 1 -->
Terminating queries before completion based on their execution time (with the max query time setting) is a nice safety net, but it comes with an inherent drawback: indeterministic (unstable) results. That is, if you repeat the very same (complex) search query with a time limit several times, the time limit will be hit at different stages, and you will get *different* result sets.

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

There is an option, [SELECT … OPTION max_predicted_time](../Searching/Options.md#max_predicted_time), that lets you limit the query time *and* get stable, repeatable results. Instead of regularly checking the actual current time while evaluating the query, which is indeterministic, it predicts the current running time using a simple linear model instead:

```ini
predicted_time =
    doc_cost * processed_documents +
    hit_cost * processed_hits +
    skip_cost * skiplist_jumps +
    match_cost * found_matches
```

The query is then terminated early when the `predicted_time` reaches a given limit.

Of course, this is not a hard limit on the actual time spent (it is, however, a hard limit on the amount of *processing* work done), and a simple linear model is in no way an ideally precise one. So the wall clock time *may* be either below or over the target limit. However, the error margins are quite acceptable: for instance, in our experiments with a 100 msec target limit, the majority of the test queries fell into a 95 to 105 msec range, and *all* the queries were in an 80 to 120 msec range. Also, as a nice side effect, using the modeled query time instead of measuring the actual run time results in somewhat fewer gettimeofday() calls, too.

没有两个服务器的品牌和型号完全相同，因此 `predicted_time_costs` 指令允许您为上述模型配置成本。为了方便起见，它们是整数，以纳秒计数。（`max_predicted_time` 中的限制以毫秒计数，且必须将成本值指定为 0.000128 毫秒而不是 128 纳秒，这样更容易出错。）不必一次指定所有四个成本，因为缺失的成本将采用默认值。然而，我们强烈建议为了可读性指定全部成本。


### preopen_tables

<!-- example conf preopen_tables -->
`preopen_tables` 配置指令指定是否在启动时强制预先打开所有表。默认值为 1，表示无论每个表的 [preopen](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Other-performance-related-settings) 设置如何，所有表都将被预先打开。如果设置为 0，则可以使用每个表的设置，默认值为 0。

预先打开表可以防止搜索查询和轮换之间的竞争，这种竞争有时会导致查询失败。然而，它也会使用更多的文件句柄。在大多数情况下，建议预先打开表。

下面是一个配置示例：

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
preopen_tables = 1
```
<!-- end -->

### pseudo_sharding

<!-- example conf pseudo_sharding -->
`pseudo_sharding` 配置选项启用对本地普通表和平实时表的搜索查询并行化，无论它们是直接查询还是通过分布式表查询。此功能将自动将查询并行化到 `searchd.threads` # 个线程。

请注意，如果您的工作线程已经忙碌，例如因为：
* 高查询并发
* 任何形式的物理分片：
  - 多个普通/实时表的分布式表
  - 由太多磁盘块组成的实时表

则启用 pseudo_sharding 可能不会带来任何好处，甚至可能导致吞吐量略微下降。如果您优先考虑更高的吞吐量而非更低的延迟，建议禁用此选项。

默认启用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
pseudo_sharding = 0
```
<!-- end -->


### replication_connect_timeout

`replication_connect_timeout` 指令定义连接远程节点的超时时间。默认值假定为毫秒，但可使用[其他后缀](../Server_settings/Special_suffixes.md)。默认值为 1000（1 秒）。

连接远程节点时，Manticore 最多等待此时间以成功完成连接。如果达到超时但未建立连接，并且启用了 `retries`，则将重新尝试连接。


### replication_query_timeout

`replication_query_timeout` 设置 searchd 等待远程节点完成查询的时间。默认值为 3000 毫秒（3 秒），但可以带有 `后缀` 以表示不同的时间单位。

建立连接后，Manticore 将最多等待远程节点完成查询的时间为 `replication_query_timeout`。请注意，该超时与 `replication_connect_timeout` 分开计算，远程节点导致的总可能延迟是两者之和。


### replication_retry_count

该设置是一个整数，指定 Manticore 在报告严重查询错误之前尝试连接和查询远程节点的次数。默认值为 3。


### replication_retry_delay

该设置是一个以毫秒为单位的整数（或 [special_suffixes](../Server_settings/Special_suffixes.md)），指定 Manticore 在复制失败时重试查询远程节点之前的延迟时间。仅当设置非零值时此值才有效。默认值为 500。

### qcache_max_bytes

<!-- example conf qcache_max_bytes -->
此配置设置用于缓存结果集的最大 RAM 大小，单位为字节。默认值为 16777216，相当于 16 兆字节。如果设置为 0，则禁用查询缓存。有关查询缓存的更多信息，请参见 [query cache](../Searching/Query_cache.md) 了解详情。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
qcache_max_bytes = 16777216
```
<!-- end -->


### qcache_thresh_msec

整数，单位毫秒。查询结果被缓存的最小墙钟时间阈值。默认值为 3000，或 3 秒。0 表示缓存所有内容。详情请参阅 [query cache](../Searching/Query_cache.md)。该值也可以使用时间 [special_suffixes](../Server_settings/Special_suffixes.md) 表示，但请谨慎使用，且不要被值名称中的 '_msec' 混淆。


### qcache_ttl_sec

整数，单位秒。缓存结果集的过期时间。默认值为 60，或 1 分钟。最小可能值为 1 秒。详情请参阅 [query cache](../Searching/Query_cache.md)。该值也可以使用时间 [special_suffixes](../Server_settings/Special_suffixes.md) 表示，但请谨慎使用，且不要被值名称中的 '_sec' 混淆。


### query_log_format

<!-- example conf query_log_format -->
查询日志格式。可选，允许值为 `plain` 和 `sphinxql`，默认是 `sphinxql`。

`sphinxql` 模式记录有效的 SQL 语句。`plain` 模式以纯文本格式记录查询（大多适用于纯全文本用例）。此指令允许您在搜索服务器启动时切换两种格式。日志格式也可以动态改变，使用 `SET GLOBAL query_log_format=sphinxql` 语法。详情请参阅 [Query logging](../Logging/Query_logging.md)。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_format = sphinxql
```
<!-- end -->

### query_log_min_msec

限制（以毫秒为单位），防止查询被写入查询日志。可选，默认值为0（所有查询都写入查询日志）。该指令指定只有执行时间超过指定限制的查询才会被记录（该值也可以用带[特殊后缀](../Server_settings/Special_suffixes.md)的时间表示，但请谨慎使用，不要与值名称本身包含的 `_msec` 混淆）。

### query_log

<!-- example conf query_log -->
查询日志文件名。可选，默认值为空（不记录查询）。所有搜索查询（如 SELECT ... 但不包括 INSERT/REPLACE/UPDATE 查询）都会记录在此文件中。格式详见[查询日志](../Logging/Query_logging.md)。如果使用 'plain' 格式，可以将日志文件路径设置为 'syslog'。在这种情况下，所有搜索查询将以 `LOG_INFO` 优先级发送给 syslog 守护进程，前缀为 '[query]'，而不是时间戳。要使用 syslog 选项，构建 Manticore 时必须配置 `-–with-syslog`。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log = /var/log/query.log
```
<!-- end -->


### query_log_mode

<!-- example conf query_log_mode -->
query_log_mode 指令允许为 searchd 和查询日志文件设置不同的权限。默认情况下，这些日志文件的权限为 600，意味着只有运行服务器的用户和 root 用户可以读取日志文件。
如果您想允许其他用户读取日志文件，例如在非 root 用户下运行的监控解决方案，此指令非常有用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_mode  = 666
```
<!-- end -->

### read_buffer_docs

<!-- example conf read_buffer_docs -->
read_buffer_docs 指令控制文档列表的每关键字读取缓冲区大小。每个搜索查询中的每个关键字出现都有两个关联的读取缓冲区：一个用于文档列表，一个用于命中列表。此设置允许您控制文档列表缓冲区大小。

较大的缓冲区大小可能增加每个查询的 RAM 使用量，但可能减少 I/O 时间。对于较慢的存储设备，设置较大值是合理的，但对于能支持高 IOPS 的存储，应试验较小的值。

默认值是 256K，最小值为 8K。您也可以在每张表级别上设置 [read_buffer_docs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs)，该设置将覆盖服务器配置级别的设置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_docs = 128K
```
<!-- end -->


### read_buffer_hits

<!-- example conf read_buffer_hits -->
read_buffer_hits 指令指定搜索查询中命中列表的每关键字读取缓冲区大小。默认大小为 256K，最小值为 8K。每个关键字出现都关联两个缓冲区，一个用于文档列表，一个用于命中列表。增加缓冲区大小会增加每个查询的 RAM 使用量，但可以减少 I/O 时间。对于慢速存储，大缓冲区值合理；而对于能够支持高 IOPS 的存储，应试验较小的值。

该设置也可以在每表级别通过 [read_buffer_hits](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_hits) 选项指定，此选项将覆盖服务器级设置。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_hits = 128K
```
<!-- end -->

### read_unhinted

<!-- example conf read_unhinted -->
未提示的读取大小。可选，默认值为 32K，最小值为 1K。

在查询时，某些读取提前知道需要读取多少数据，但有些目前不知道。最明显的是命中列表大小当前未知。此设置允许您控制在这种情况下读取的数据量。它影响命中列表的 I/O 时间，对于大于未提示读取大小的列表，能减少 I/O 时间，但对于较小的列表，则增加。它**不**影响 RAM 使用，因为读取缓冲区已分配。所以它不应大于 read_buffer。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_unhinted = 32K
```
<!-- end -->

### reset_network_timeout_on_packet

<!-- example conf reset_network_timeout_on_packet -->
细化网络超时行为（例如 `network_timeout`、`read_timeout` 和 `agent_query_timeout`）。

设置为 0 时，超时限制发送整个请求/查询的最长时间。
设置为 1（默认）时，超时限制网络活动之间的最长时间。

在复制场景下，一个节点可能需要向另一个节点发送大型文件（例如 100GB）。假设网络传输速率为 1GB/s，每次传送的数据包大小为 4-5MB。传输整个文件需要 100 秒。默认的 5 秒超时只允许传输 5GB 后连接被断开。增加超时可以作为变通方法，但不具备可扩展性（例如，下一文件可能是 150GB，会再次失败）。但是，默认的 `reset_network_timeout_on_packet` 设为 1 时，超时应用于单个数据包，而非整个传输。只要传输正常（且在超时期间实际上接收到数据），连接保持活跃。如果传输卡住且在包间超时，连接将被断开。

请注意，如果设置分布式表，每个节点——主节点和代理节点——都应进行调优。主节点影响 `agent_query_timeout`，代理节点影响 `network_timeout`。

<!-- intro -->

##### 示例：

<!-- request Example -->

```ini
reset_network_timeout_on_packet = 0
```

<!-- end -->


### rt_flush_period

<!-- example conf rt_flush_period -->
实时表 RAM 块刷新检查周期，单位为秒（或[特殊后缀](../Server_settings/Special_suffixes.md)）。可选，默认是 10 小时。

积极更新且完全适合内存块的实时表仍可能导致不断增长的二进制日志，影响磁盘使用和崩溃恢复时间。使用此指令，搜索服务器执行周期性刷新检查，并可保存符合条件的内存块，从而实现相应的二进制日志清理。有关详细信息，请参阅[二进制日志](../Logging/Binary_logging.md)。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_flush_period = 3600 # 1 hour
```
<!-- end -->


### rt_merge_iops

<!-- example conf rt_merge_iops -->
允许RT块合并线程启动的最大I/O操作数（每秒）。可选，默认值为0（无限制）。

此指令让您限制因`OPTIMIZE`语句产生的I/O影响。保证所有实时优化活动产生的磁盘IOPS（每秒I/O次数）不会超过配置的限制。限制rt_merge_iops可以减少由合并引起的搜索性能下降。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_iops = 40
```
<!-- end -->

### rt_merge_maxiosize

<!-- example conf rt_merge_maxiosize -->
允许RT块合并线程启动的最大I/O操作大小。可选，默认值为0（无限制）。

此指令让您限制因`OPTIMIZE`语句产生的I/O影响。大于该限制的I/O将被拆分成两个或多个I/O，随后根据[rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops)限制分别计数。因此，保证所有优化活动每秒产生的磁盘I/O不会超过 (rt_merge_iops * rt_merge_maxiosize) 字节。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_maxiosize = 1M
```
<!-- end -->


### seamless_rotate

<!-- example conf seamless_rotate -->
防止`searchd`在旋转包含大量数据以预缓存的表时停顿。可选，默认值为1（启用无缝旋转）。在Windows系统上，默认关闭无缝旋转。

表可能包含需要在内存中预缓存的数据。目前，`.spa`、`.spb`、`.spi`和`.spm`文件被完全预缓存（它们分别包含属性数据、二进制属性数据、关键字表和已删除行映射）。没有无缝旋转时，旋转表尽可能少用内存，操作过程如下：

1. 临时拒绝新的查询（返回“重试”错误码）；
2. `searchd`等待所有当前运行的查询结束；
3. 旧表释放，文件重命名；
4. 新表文件重命名，分配所需内存；
5. 将新表属性和字典数据预加载到内存；
6. `searchd`恢复从新表查询。

然而，如果属性或字典数据较大，预加载步骤可能耗费显著时间——在预加载1-5+ GB文件时，甚至可能持续几分钟。

启用无缝旋转时，旋转过程如下：

1. 分配新表内存存储；
2. 异步预加载新表属性和字典数据；
3. 成功时，释放旧表，重命名两张表文件；
4. 失败时，释放新表；
5. 任意时刻，查询总是从旧表或新表其中之一提供服务。

无缝旋转的代价是旋转期间内存峰值使用更高（因为在预加载新副本时，旧副本和新副本的`.spa/.spb/.spi/.spm`数据需同时驻留内存）。平均使用量保持不变。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
seamless_rotate = 1
```
<!-- end -->

### secondary_indexes
<!-- example conf secondary_indexes -->

此选项用于启用/禁用搜索查询使用二级索引。可选，默认值为1（启用）。注意，对索引构建不需启用，只要安装了[Manticore Columnar Library](https://github.com/manticoresoftware/columnar)，索引始终启用。搜索时使用这些索引也需该库支持。有三种模式：

* `0`：禁用搜索时使用二级索引。可通过[分析器提示](../Searching/Options.md#Query-optimizer-hints)为单个查询启用
* `1`：启用搜索时使用二级索引。可通过[分析器提示](../Searching/Options.md#Query-optimizer-hints)为单个查询禁用
* `force`：同启用，但加载二级索引时发生错误会上报，并导致整个索引不加载进守护进程。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
secondary_indexes = 1
```

<!-- end -->

### server_id

<!-- example conf server_id -->
整数，用作服务器标识符，作为生成复制集群节点唯一短UUID的种子。server_id须在集群节点间唯一，且范围为0到127。若未设置server_id，则以MAC地址与PID文件路径的哈希值计算，或使用随机数作为短UUID的种子。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
server_id = 1
```
<!-- end -->


### shutdown_timeout

<!-- example conf shutdown_timeout -->
`searchd --stopwait`等待时间，单位秒（或[特殊后缀](../Server_settings/Special_suffixes.md)）。可选，默认60秒。

执行`searchd --stopwait`时，服务器须先完成若干操作，如完成查询、刷新RT内存块、刷新属性及更新二进制日志等，这些任务需要时间。`searchd --stopwait`将最多等待`shutdown_time`秒以完成这些操作。合适等待时间取决于表大小和负载。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
shutdown_timeout = 3m # wait for up to 3 minutes
```
<!-- end -->


### shutdown_token

调用 VIP Manticore SQL 连接的 'shutdown' 命令所需的密码的 SHA1 哈希值。没有它，[debug](../Reporting_bugs.md#DEBUG) 'shutdown' 子命令将永远不会导致服务器停止。请注意，这种简单的哈希不应被视为强保护，因为我们没有使用加盐哈希或任何现代哈希函数。它旨在作为本地网络中值守守护进程的万无一失的措施。

### snippets_file_prefix

<!-- example conf snippets_file_prefix -->
在生成片段时，要添加到本地文件名之前的前缀。可选，默认是当前工作目录。

当与 `load_files` 或 `load_files_scattered` 选项一起使用时，此前缀可用于分布式片段生成。

请注意，这是一个前缀**而非**路径！这意味着如果前缀设置为 "server1"，请求引用 "file23"，`searchd` 将尝试打开 "server1file23"（全部不带引号）。因此，如果需要它作为路径，必须包含尾部斜杠。

构造最终文件路径后，服务器会展开所有相对目录并将最终结果与 `snippet_file_prefix` 的值进行比较。如果结果不以该前缀开头，则该文件将被拒绝并显示错误消息。

例如，如果设置为 `/mnt/data`，有人调用片段生成时使用文件 `../../../etc/passwd` 作为源，他们将收到错误消息：

`File '/mnt/data/../../../etc/passwd' escapes '/mnt/data/' scope`

而不是文件内容。

另外，如果参数未设置且读取 `/etc/passwd`，实际上会读取 /daemon/working/folder/etc/passwd，因为该参数的默认值是服务器的工作文件夹。

另请注意，这是本地选项；不会影响代理。所以你可以放心地在主服务器上设置前缀。路由到代理的请求不会受主服务器设置的影响，但会受代理自身设置的影响。

这在例如文档存储位置（无论是本地存储还是 NAS 挂载点）在服务器之间不一致时非常有用。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
snippets_file_prefix = /mnt/common/server1/
```
<!-- end -->

> **警告：** 如果仍想访问文件系统根目录中的文件，必须显式将 `snippets_file_prefix` 设置为空值（通过 `snippets_file_prefix=` 行），或者设置为根目录（通过 `snippets_file_prefix=/`）。


### sphinxql_state

<!-- example conf sphinxql_state -->
当前 SQL 状态序列化到的文件路径。

服务器启动时，会重放此文件。在合适的状态变更（例如 SET GLOBAL）时，这个文件会自动重写。这可以防止一个难以诊断的问题：如果加载了 UDF 函数但 Manticore 崩溃，自动重启后，UDF 和全局变量将不再可用。使用持久状态有助于确保优雅恢复，避免此类意外。

`sphinxql_state` 不能用来执行任意命令，如 `CREATE TABLE`。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
sphinxql_state = uservars.sql
```
<!-- end -->


### sphinxql_timeout

<!-- example conf sphinxql_timeout -->
使用 SQL 接口时，两个请求之间最大等待时间（秒，或使用[特殊后缀](../Server_settings/Special_suffixes.md)）。可选，默认15分钟。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
sphinxql_timeout = 15m
```
<!-- end -->


### ssl_ca

<!-- example conf ssl_ca -->
SSL 证书颁发机构 (CA) 证书文件路径（也称为根证书）。可选，默认为空。不为空时，`ssl_cert` 中的证书应由此根证书签名。

服务器使用 CA 文件验证证书上的签名。文件必须是 PEM 格式。

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

服务器使用此证书作为自签名公钥，通过 SSL 加密 HTTP 流量。文件必须是 PEM 格式。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ssl_cert = keys/server-cert.pem
```
<!-- end -->


### ssl_key

<!-- example conf ssl_key -->
SSL 证书密钥路径。可选，默认为空。

服务器使用此私钥通过 SSL 加密 HTTP 流量。文件必须是 PEM 格式。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ssl_key = keys/server-key.pem
```
<!-- end -->


### subtree_docs_cache

<!-- example conf subtree_docs_cache -->
每个查询最大公共子树文档缓存大小。可选，默认0（禁用）。

此设置限制公共子树优化器的内存使用（参见[多查询](../Searching/Multi-queries.md)）。最多消耗这么多内存缓存每个查询的文档条目。设为0则禁用优化器。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
subtree_docs_cache = 8M
```
<!-- end -->


### subtree_hits_cache

<!-- example conf subtree_hits_cache -->
每个查询最大公共子树命中缓存大小。可选，默认0（禁用）。

此设置限制公共子树优化器的内存使用（参见[多查询](../Searching/Multi-queries.md)）。最多消耗这么多内存缓存每个查询的关键词出现（命中）。设为0则禁用优化器。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
subtree_hits_cache = 16M
```
<!-- end -->

### threads

<!-- example threads -->
Manticore 守护进程的工作线程数（或线程池大小）。Manticore 启动时创建这么多操作系统线程，它们执行守护进程内的所有任务，如执行查询、创建片段等。一些操作可能拆分成子任务并行执行，例如：

* 实时表中的搜索
* 由本地表组成的分布式表中的搜索
* 传播查询调用
* 以及其他

默认情况下，它被设置为服务器上的 CPU 核心数。Manticore 在启动时创建线程并保持它们直到停止。每个子任务在需要时可以使用其中一个线程。当子任务完成时，它会释放线程，以便其他子任务使用。

在密集的 I/O 类型负载情况下，设置的值高于 CPU 核心数可能更合理。

<!-- request Example -->
```ini
threads = 10
```

<!-- end -->

### thread_stack

<!-- example conf thread_stack -->
作业的最大堆栈大小（协程，一个搜索查询可能会导致多个作业/协程）。可选，默认是 128K。

每个作业都有自己的 128K 堆栈。运行查询时，会检查其所需的堆栈大小。如果默认的 128K 足够，就直接处理。如果需要更多，调度另一个堆栈增大的作业继续处理。此类高级堆栈的最大大小受此设置限制。

将值设置为合理较高的比率有助于处理非常深度的查询，同时不意味着整体内存消耗会过高。例如，将其设置为 1G 并不表示每个新作业都会占用 1G 内存，但如果我们看到它需要 100M 堆栈，我们就会为该作业分配 100M。其他作业同时仍使用默认的 128K 堆栈。以此类推，我们还可以运行需要 500M 的更复杂查询。只有当我们**内部**看到作业需要超过 1G 堆栈时，才会失败并报告 thread_stack 过低。

然而，实际上，甚至需要 16M 堆栈的查询通常对解析来说过于复杂，且消耗过多时间和资源去处理。因此，守护进程会处理它，但通过 `thread_stack` 设置限制此类查询是相当合理的。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
thread_stack = 8M
```
<!-- end -->


### unlink_old

<!-- example conf unlink_old -->
确定在成功轮换时是否取消链接 `.old` 表副本。可选，默认是 1（执行取消链接）。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
unlink_old = 0
```
<!-- end -->


### watchdog

<!-- example conf watchdog -->
线程化服务器 watchdog。可选，默认是 1（启用 watchdog）。

当 Manticore 查询崩溃时，可能导致整个服务器宕机。启用 watchdog 功能时，`searchd` 还维护一个独立的轻量级进程，监控主服务器进程并在异常终止时自动重启。watchdog 默认启用。

<!-- request Example -->

```ini
watchdog = 0 # disable watchdog
```
<!-- end -->
<!-- proofread -->

