# 配置中的 "Searchd" 部分

以下设置用于 Manticore Search 配置文件的 `searchd` 部分，以控制服务器的行为。下面是每个设置的摘要：

### access_plain_attrs

此设置为 [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。此项为可选，默认值为 `mmap_preread`。

`access_plain_attrs` 指令允许你为此 searchd 实例管理的所有表定义 [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令优先级更高，会覆盖此实例范围的默认设置，从而提供更细粒度的控制。

### access_blob_attrs

此设置为 [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。此项为可选，默认值为 `mmap_preread`。

`access_blob_attrs` 指令允许你为此 searchd 实例管理的所有表定义 [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令优先级更高，会覆盖此实例范围的默认设置，从而提供更细粒度的控制。

### access_doclists

此设置为 [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。此项为可选，默认值为 `file`。

`access_doclists` 指令允许你为此 searchd 实例管理的所有表定义 [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令优先级更高，会覆盖此实例范围的默认设置，从而提供更细粒度的控制。

### access_hitlists

此设置为 [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。此项为可选，默认值为 `file`。

`access_hitlists` 指令允许你为此 searchd 实例管理的所有表定义 [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令优先级更高，会覆盖此实例范围的默认设置，从而提供更细粒度的控制。

### access_dict

此设置为 [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。此项为可选，默认值为 `mmap_preread`。

`access_dict` 指令允许你为此 searchd 实例管理的所有表定义 [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令优先级更高，会覆盖此实例范围的默认设置，从而提供更细粒度的控制。

### agent_connect_timeout

此设置用于为 [agent_connect_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) 参数设置实例范围的默认值。


### agent_query_timeout

此设置用于为 [agent_query_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) 参数设置实例范围的默认值。可以通过 `OPTION agent_query_timeout=XXX` 子句在每个查询级别进行覆盖。


### agent_retry_count

此设置为整数，指定 Manticore 在报告严重查询错误之前，通过分布式表连接和查询远程代理的重试次数。默认值为 0（即不重试）。你也可以通过 `OPTION retry_count=XXX` 子句在每个查询级别设置此值。如果提供了每个查询的选项，它将覆盖配置中指定的值。

请注意，如果你在分布式表定义中使用了 [agent mirrors](../Creating_a_cluster/Remote_nodes/Mirroring.md#Agent-mirrors)，服务器将根据所选的 [ha_strategy](../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) 为每次连接尝试选择不同的镜像。在这种情况下，`agent_retry_count` 将对集合中的所有镜像进行累加。

例如，如果你有 10 个镜像并设置了 `agent_retry_count=5`，服务器最多会重试 50 次，假设每个镜像平均尝试 5 次（如果使用 `ha_strategy = roundrobin` 选项，则会是这种情况）。

然而，作为 [agent](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) 定义的 `retry_count` 选项提供的值则作为绝对限制。换句话说，代理定义中的 `[retry_count=2]` 选项始终表示最多尝试 2 次，无论代理是否指定了 1 个或 10 个镜像。

### agent_retry_delay

此设置为以毫秒为单位的整数（或使用 [special_suffixes](../Server_settings/Special_suffixes.md)），指定当远程代理查询失败时，Manticore 重试查询的延迟时间。此值仅在指定非零的 [agent_retry_count](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 或非零每查询的 `retry_count` 时才有意义。默认值为 500。你也可以通过 `OPTION retry_delay=XXX` 子句在每个查询级别设置此值。如果提供了每个查询的选项，它将覆盖配置中指定的值。


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

二进制日志用于RT表数据的崩溃恢复以及普通磁盘索引的属性更新，这些更新否则只会存储在RAM中直到刷新。当启用日志记录时，提交到RT表的每个事务都会写入日志文件。在非正常关闭后启动时，日志会自动重放，恢复记录的更改。

`binlog_path` 指令指定二进制日志文件的位置。它应只包含路径；`searchd` 将根据需要在该目录中创建和取消链接多个 `binlog.*` 文件（包括binlog数据、元数据和锁文件等）。

空值禁用二进制日志记录，这会提高性能，但会使RT表数据面临风险。


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
此设置控制 [boolean_simplify](../Searching/Options.md#boolean_simplify) 搜索选项的默认值。该设置是可选的，默认值为1（启用）。

设置为1时，服务器将自动应用 [布尔查询优化](../Searching/Full_text_matching/Boolean_optimization.md) 以提升查询性能。设置为0时，默认情况下查询将不使用优化执行。此默认设置可通过相应的搜索选项 `boolean_simplify` 在单个查询中覆盖。

<!-- request Example -->
```ini
searchd {
    boolean_simplify = 0  # disable boolean optimization by default
}
```
<!-- end -->

### buddy_path

<!-- example conf buddy_path -->
此设置确定Manticore Buddy二进制文件的路径。它是可选的，默认值为构建时配置的路径，不同操作系统会有所不同。通常，你无需修改此设置。但如果你希望以调试模式运行Manticore Buddy、对其进行更改或实现新插件，则此设置可能有用。在后一种情况下，你可以从 https://github.com/manticoresoftware/manticoresearch-buddy 克隆Buddy，向目录 `./plugins/` 添加新插件，并在切换到Buddy源代码目录后运行 `composer install --prefer-source`，以便更方便地开发。

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

你也可以选择在发布版本中提供的Linux amd64特殊版本 `manticore-executor-dev`，例如：https://github.com/manticoresoftware/executor/releases/tag/v1.0.13

若采用此方法，记得将manticore执行器的开发版本链接到 `/usr/bin/php`。

若要禁用Manticore Buddy，将值设为空，如示例所示。

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
此设置决定使用持久连接时请求之间的最大等待时间（单位为秒或 [特殊后缀](../Server_settings/Special_suffixes.md)）。该设置是可选的，默认值为五分钟。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
client_timeout = 1h
```
<!-- end -->


### collation_libc_locale

<!-- example conf collation_libc_locale -->
服务器 libc 区域设置。可选，默认是 C。

指定 libc 区域，影响基于 libc 的排序规则。详情请参见 [排序规则](../Searching/Collations.md) 部分。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
collation_libc_locale = fr_FR
```
<!-- end -->


### collation_server

<!-- example conf collation_server -->
默认服务器排序规则。可选，默认是 libc_ci。

指定传入请求使用的默认排序规则。排序规则可以在单个查询中覆盖。可参见 [排序规则](../Searching/Collations.md) 部分，了解可用排序规则列表及其他详情。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
collation_server = utf8_ci
```
<!-- end -->


### data_dir

<!-- example conf data_dir -->
当指定此设置时，会启用 [实时模式](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)，这是一种管理数据模式的命令式方式。值应为一个目录路径，存放你希望保存所有表、二进制日志及Manticore Search 在此模式下正常运行所需的其他所有内容。
当指定了 `data_dir` 时，不允许对 [普通表](../Creating_a_table/Local_tables/Plain_table.md) 进行索引。关于RT模式和普通模式的区别，请阅读 [本节](../Read_this_first.md#Real-time-table-vs-plain-table)。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
data_dir = /var/lib/manticore
```
<!-- end -->

### diskchunk_flush_search_timeout

<!-- example conf diskchunk_flush_search_timeout -->
若表中无搜索，防止自动刷新RAM块的超时时间。可选，默认是30秒。

在决定是否自动刷写之前用于检查搜索的时间。
仅当在过去的 `diskchunk_flush_search_timeout` 秒内表中至少有一次搜索时，才会进行自动刷新。此设置与 [diskchunk_flush_write_timeout](../Server_settings/Searchd.md#diskchunk_flush_write_timeout) 配合使用。对应的 [每表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_search_timeout) 优先级更高，会覆盖此实例级默认值，提供更细粒度的控制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
diskchunk_flush_search_timeout = 120s
```
<!-- end -->

### diskchunk_flush_write_timeout

<!-- example conf diskchunk_flush_write_timeout -->
等待无写操作的时间（秒），超过后自动将RAM块刷新到磁盘。可选，默认是1秒。

如果在 `diskchunk_flush_write_timeout` 秒内没有对 RAM 块进行写入，该块将被刷新到磁盘。此设置与 [diskchunk_flush_search_timeout](../Server_settings/Searchd.md#diskchunk_flush_search_timeout) 配合使用。要禁用自动刷新，请在配置中显式设置 `diskchunk_flush_write_timeout = -1`。对应的 [单表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_write_timeout) 优先级更高，会覆盖此实例范围内的默认值，从而提供更细粒度的控制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
diskchunk_flush_write_timeout = 60s
```
<!-- end -->

### docstore_cache_size

<!-- example conf docstore_cache_size -->
此设置指定存储文档块最大占用的内存大小。该设置为可选，默认值为 16m（16 兆字节）。

当使用 `stored_fields` 时，文档块会从磁盘读取并解压。由于每个块通常包含多个文档，因此处理下一个文档时可能会重用该块。为此，块会保存在服务器范围的缓存中，缓存中保存的是解压后的块。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
docstore_cache_size = 8m
```
<!-- end -->

### engine

<!-- example conf engine -->
创建 RT 模式表时使用的默认属性存储引擎。可以是 `rowwise`（默认）或 `columnar`。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
engine = columnar
```
<!-- end -->


### expansion_limit

<!-- example conf expansion_limit -->
此设置确定单个通配符展开关键词的最大数量。该设置为可选，默认值为 0（无限制）。

在对启用了 `dict = keywords` 构建的表执行子串搜索时，单个通配符可能会匹配成千上万乃至百万的关键词（比如将 `a*` 匹配整个牛津词典）。该指令允许限制这类扩展的影响。设置 `expansion_limit = N` 将限制每个通配符的扩展最多不超过 N 个最常见的匹配关键词。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_limit = 16
```
<!-- end -->

### expansion_merge_threshold_docs

<!-- example conf expansion_merge_threshold_docs -->
此设置确定允许合并在扩展关键词中的最大文档数。该设置为可选，默认值为 32。

在对启用了 `dict = keywords` 构建的表执行子串搜索时，单个通配符可能匹配成千上万个关键词。该指令允许提高允许合并的关键词数量限制，以加快匹配速度，但会在搜索时使用更多内存。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_merge_threshold_docs = 1024
```
<!-- end -->

### expansion_merge_threshold_hits

<!-- example conf expansion_merge_threshold_hits -->
此设置确定允许合并在扩展关键词中的最大命中数。该设置为可选，默认值为 256。

在对启用了 `dict = keywords` 构建的表执行子串搜索时，单个通配符可能匹配成千上万个关键词。该指令允许提高允许合并的关键词数量限制，以加快匹配速度，但会在搜索时使用更多内存。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_merge_threshold_hits = 512
```
<!-- end -->

### expansion_phrase_limit

<!-- example conf expansion_phrase_limit -->
该设置控制因 `PHRASE`、`PROXIMITY` 和 `QUORUM` 操作符内的 `OR` 运算符产生的替代短语变体的最大数量。该设置为可选，默认值为 1024。

当在短语类操作符内使用 `|`（OR）运算符时，根据指定的替代项数量，扩展组合可能会呈指数增长。此设置通过限制查询处理中考虑的排列组合数量，帮助防止查询扩展过度。如果生成的变体数超过此限制，查询将失败并报错。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_phrase_limit = 4096
```
<!-- end -->

### grouping_in_utc

该设置指定 API 和 SQL 中基于时间的分组是按本地时区计算还是按 UTC 计算。该设置为可选，默认值为 0（表示“本地时区”）。

默认情况下，所有“按时间分组”的表达式（如 API 中按天、周、月、年分组，SQL 中的按日、月、年、年月、年月日分组）均使用本地时间。例如，如果您有两个带时间属性的文档，分别为 `13:00 utc` 和 `15:00 utc`，在分组时它们均会根据本地时区归入相应分组。如果您处于 UTC 时区，这两个时间点对应同一天，但如果您处于 UTC+10 时区，则这两个文档在“按日分组”时会被归入不同的组（因为 13:00 UTC 在 UTC+10 时区是本地时间 23:00，而 15:00 UTC 是第二天的 01:00）。有时这种行为不可接受，需要使时间分组不依赖于时区。您可以通过定义全局环境变量 TZ 运行服务器，但这不仅影响分组，也会影响日志的时间戳，可能不符合需求。启用该选项（在配置文件中设置或使用 SQL 中的 [SET global](../Server_settings/Setting_variables_online.md#SET) 语句）后，所有时间分组表达式将以 UTC 计算，其他依赖时间的功能（如服务器日志）仍使用本地时区。


### timezone

该设置指定日期/时间相关函数所用的时区。默认使用本地时区，但您可以指定其他 IANA 格式的时区（如 `Europe/Amsterdam`）。

请注意，此设置不影响日志记录，日志始终使用本地时区。

另外，请注意如果使用了 `grouping_in_utc`，则“按时间分组”功能仍将使用 UTC，而其他日期/时间相关函数将使用指定的时区。总体上，不建议混用 `grouping_in_utc` 和 `timezone`。

您可以在配置中设置此选项，也可以通过 SQL 中的 [SET global](../Server_settings/Setting_variables_online.md#SET) 语句来配置。


### ha_period_karma

<!-- example conf ha_period_karma -->
此设置指定代理镜像统计窗口大小，单位为秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。这是一个可选项，默认值为 60 秒。

对于包含代理镜像的分布式表（详见 [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)），主节点会跟踪多个不同的每个镜像的计数器。这些计数器随后被用于故障转移和平衡（主节点根据计数器选择最佳镜像使用）。计数器按 `ha_period_karma` 秒为一块进行累积。

开始一个新的块后，主节点可能会继续使用前一个块累积的值，直到新块填满一半。因此，之前的历史数据最多在 1.5 倍 ha_period_karma 秒后不再影响镜像选择。

尽管最多只有两个块用于镜像选择，但最多会存储最近的 15 个块以供检测使用。这些块可以通过 [SHOW AGENT STATUS](../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS) 语句进行检查。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_period_karma = 2m
```
<!-- end -->


### ha_ping_interval

<!-- example conf ha_ping_interval -->
此设置配置代理镜像 Ping 的间隔，单位为毫秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。这是一个可选项，默认值为 1000 毫秒。

对于包含代理镜像的分布式表（详见 [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)），主节点会在空闲期间向所有镜像发送 Ping 命令。这用于跟踪当前代理状态（存活或死亡、网络往返时间等）。Ping 之间的间隔由此指令定义。要禁用 Ping，将 ha_ping_interval 设置为 0。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_ping_interval = 3s
```
<!-- end -->


### hostname_lookup

`hostname_lookup` 选项定义了刷新主机名的策略。默认情况下，代理主机名的 IP 地址在服务器启动时被缓存，以避免过度访问 DNS。然而，在某些情况下，IP 可能动态变化（例如云托管），这时可能希望不缓存 IP。将此选项设置为 `request` 将禁用缓存，并在每次查询时请求 DNS。IP 地址也可以通过 `FLUSH HOSTNAMES` 命令手动刷新。

### jobs_queue_size

jobs_queue_size 设置定义了队列中可以同时存在多少个“作业”。默认情况下没有限制。

在大多数情况下，“作业”意味着针对单个本地表（普通表或实时表的磁盘块）的一次查询。例如，如果您有一个由 2 个本地表组成的分布式表，或者一个带有 2 个磁盘块的实时表，针对任一表的搜索查询通常会将 2 个作业放入队列。然后，由线程池（其大小由 [threads](../Server_settings/Searchd.md#threads) 定义）处理它们。然而，在某些情况下，如果查询过于复杂，可能会产生更多作业。当 [max_connections](../Server_settings/Searchd.md#max_connections) 和 [threads](../Server_settings/Searchd.md#threads) 不足以平衡所需性能时，建议调整此设置。

### join_batch_size

表连接的工作方式是积累一批匹配项，这些匹配项是对左边表执行查询的结果。这批结果随后作为单个查询在右边表上处理。

此选项允许您调整批量大小。默认值为 `1000`，设置此选项为 `0` 可禁用批处理。

较大的批处理大小可能提升性能；但对于某些查询，可能导致内存消耗过大。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
join_batch_size = 2000
```
<!-- end -->

### join_cache_size

右表上执行的每个查询都是由特定的 JOIN ON 条件定义的，这些条件决定了从右表检索的结果集。

如果唯一的 JOIN ON 条件较少，重用结果会比重复执行右表查询更有效。为此，结果集会被存储在缓存中。

此选项允许您配置缓存大小。默认值为 `20 MB`，设置为 0 则禁用缓存。

注意每个线程维护自己的缓存，因此在估计总内存使用时应考虑正在执行查询的线程数量。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
join_cache_size = 10M
```
<!-- end -->

### listen_backlog

<!-- example conf listen_backlog -->
listen_backlog 设置确定了 TCP 监听队列的长度，用于传入连接。这对于按顺序处理请求的 Windows 版本尤为重要。当连接队列达到限制时，新连接将被拒绝。
对于非 Windows 版本，默认值一般工作良好，通常无需调整此设置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
listen_backlog = 20
```
<!-- end -->

### kibana_version_string

<!-- example conf kibana_version_string -->
返回给 Kibana 或 OpenSearch Dashboards 的服务器版本字符串。可选 —— 默认设置为 `7.6.0`。

某些版本的 Kibana 和 OpenSearch Dashboards 期望服务器报告特定版本号，并可能根据版本号表现不同。为解决此类问题，您可以使用此设置，让 Manticore 向 Kibana 或 OpenSearch Dashboards 报告自定义版本。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
kibana_version_string = 1.2.3
```
<!-- end -->

### listen

<!-- example conf listen -->
此设置允许您指定 Manticore 将接受连接的 IP 地址和端口，或者 Unix 域套接字路径。

`listen` 的通用语法为：

```ini
listen = ( address ":" port | port | path | address ":" port start - port end ) [ ":" protocol [ "_vip" ] [ "_readonly" ] ]
```

您可以指定：
* IP 地址（或主机名）和端口号
* 或者仅端口号
* 或者 Unix 套接字路径（Windows 不支持）
* 或者 IP 地址和端口范围

如果指定了端口号但未指定地址，`searchd` 将监听所有网络接口。Unix 路径以斜杠开头。端口范围只能用于复制协议。

您还可以指定用于该套接字连接的协议处理程序（监听器）。监听器包括：

* **未指定** - Manticore 将在此端口接受来自：
  - 其他 Manticore 代理（例如远程分布式表）
  - 通过 HTTP 和 HTTPS 的客户端
  - [Manticore Buddy](https://manticoresearch.com/blog/manticoresearch-buddy-intro/)。**确保有此类监听器（或下面提到的 `http` 监听器），以避免 Manticore 功能受限。**
* `mysql` MySQL 协议，供 MySQL 客户端连接。注意：
  - 也支持压缩协议。
  - 如果启用了 [SSL](../Security/SSL.md#SSL)，可以建立加密连接。
* `replication` - 复制协议，用于节点间通信。更多细节见[复制](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md)部分。可以指定多个复制监听器，但它们必须监听相同的 IP，端口可不同。定义带端口范围的复制监听器（如 `listen = 192.168.0.1:9320-9328:replication`）时，Manticore 不会立即开始监听这些端口，而是在开始使用复制时随机从指定范围内选择空闲端口进行监听。复制正常工作至少需要范围内包含 2 个端口。
* `http` - 与**未指定**相同。Manticore 在此端口接受来自远程代理和通过 HTTP、HTTPS 的客户端连接。
* `https` - HTTPS 协议。Manticore **仅**在此端口接受 HTTPS 连接。详情见[SSL](../Security/SSL.md)部分。
* `sphinx` - 旧版二进制协议。用于服务远程 [SphinxSE](../Extensions/SphinxSE.md) 客户端连接。一些 Sphinx API 客户端实现（例如 Java 版）需要显式声明监听器。

为客户端协议添加后缀 `_vip`（即除了 `replication` 之外的所有协议，如 `mysql_vip`、`http_vip` 或仅 `_vip`）会强制创建专用线程，以绕过各种限制。对于当服务器严重过载可能卡顿或无法通过常规端口连接的节点维护非常有用。

后缀 `_readonly` 为监听器设置[只读模式](../Security/Read_only.md)，仅接受读取查询。

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

`listen` 指令可以多条。`searchd` 将在所有指定的端口和套接字上监听客户端连接。Manticore 包中提供的默认配置定义监听端口：
* `9308` 和 `9312`，接收远程代理和非 MySQL 客户端的连接
* 以及 `9306` 端口，接收 MySQL 连接。

如果配置中完全未指定 `listen`，Manticore 会监听：
* `127.0.0.1:9306`，供 MySQL 客户端连接
* `127.0.0.1:9312`，供 HTTP/HTTPS 以及其他 Manticore 节点和基于 Manticore 二进制 API 的客户端连接。

#### 监听特权端口

默认情况下，Linux 不允许 Manticore 监听 1024 以下端口（如 `listen = 127.0.0.1:80:http` 或 `listen = 127.0.0.1:443:https`），除非以 root 身份运行 searchd。如果仍希望在非 root 用户下启动 Manticore 并监听 1024 以下端口，可考虑以下方法之一（任意一项均可生效）：
* 运行命令 `setcap CAP_NET_BIND_SERVICE=+eip /usr/bin/searchd`
* 在 Manticore 的 systemd 单元中添加 `AmbientCapabilities=CAP_NET_BIND_SERVICE`，并重新加载 daemon（`systemctl daemon-reload`）。

#### 关于 Sphinx API 协议和 TFO 的技术细节
<details>
旧版 Sphinx 协议有两个阶段：握手交换和数据流。握手由客户端发送的 4 字节包和守护进程发送的 4 字节包组成，唯一目的是客户端确认远端是真正的 Sphinx 守护进程，守护进程确认远端是合法的 Sphinx 客户端。主数据流非常简单：双方各自声明握手，对方验证。这种短包交换意味着使用特殊的 `TCP_NODELAY` 标志，禁用 Nagle 的 TCP 算法，并声明 TCP 连接将以小包对话方式执行。
然而，协商中谁先发送并无严格定义。历史上，所有使用二进制 API 的客户端均先发起：发送握手，读取守护进程的 4 字节响应，然后发送请求并读取守护进程回答。
在改进 Sphinx 协议兼容性时，我们考虑了以下几点：

1. 通常主节点与代理间通信由已知客户端发起到已知主机及端口，因此极不可能收到错误的握手包。所以可以隐式假设双方都是合法的 Sphinx 协议通信方。
2. 基于此假设，我们可以“粘连”握手和实际请求，一起发送。如果后端是传统 Sphinx 守护进程，它将读取前 4 字节为握手，然后读取请求体。由于它们在同一数据包内，后端套接字节省了一个 RTT，而前端缓存仍旧能正常工作。
3. 继续假设：由于“query”数据包相当小，握手数据包更小，我们使用现代的 TFO（tcp-fast-open）技术，在初始的“SYN” TCP包中发送握手和查询包。也就是说：我们用连接的握手+主体包连接到远程节点。守护进程接受连接并立即拥有套接字缓冲区中的握手和主体，因为它们是在第一个TCP“SYN”包中一起到达的。这样就减少了一个往返时间（RTT）。
4. 最后，教守护进程接受此改进。实际上，从应用程序来看，这意味着不要使用 `TCP_NODELAY`。从系统角度来看，这意味着确保守护进程端启用了 TFO 接收，客户端端启用了 TFO 发送。默认情况下，现代系统客户端 TFO 已默认启用，所以只需调整服务器端的 TFO 即可使所有功能正常工作。

所有这些改进都是在不实际更改协议本身的情况下实现的，允许我们从连接中消除 TCP 协议的 1.5 RTT。如果查询和应答能够放入单个 TCP 包，将整个二进制 API 会话从 3.5 RTT 减少到 2 RTT——这使网络协商速度提升约两倍。

所以，所有改进都围绕一个最初未定义的说法：“谁先说话”。如果客户端先说，我们可以应用所有这些优化，有效地在单个 TFO 包中处理连接 + 握手 + 查询。此外，我们可以查看接收到的包的开始部分，确定实际的协议。这就是为什么你可以通过 API/http/https 连接到同一端口。如果守护进程必须先说话，这些优化都不可能，实现多协议也不可能。这就是为什么我们为 MySQL 保留了专用端口，而没有将其与其他所有协议合并到同一端口中。突然之间，在所有客户端中，有一个是编写成守护进程应先发送握手的客户端。也就是说，没有上述所有改进的可能性。那就是 mysql/mariadb 的 SphinxSE 插件。因此，专门为这个单客户端，我们为 `sphinx` 协议定义预留，使用最传统的方式工作。即双方都开启 `TCP_NODELAY` 并使用小包交换。守护进程连接时发送握手，客户端随后发送握手，之后一切照常工作。虽然这样不够最优，但能够正常运行。如果你使用 SphinxSE 连接到 Manticore，你必须专门配置一个带有明确标注 `sphinx` 协议的监听器。对于其他客户端——避免使用此监听器，因为它更慢。若你使用其他老式的 Sphinx API 客户端——首先检查它们是否能够在非专用多协议端口上工作。对于主从代理链接，使用非专用（多协议）端口并启用客户端和服务器端的 TFO 能正常工作，且肯定会使网络后端运行更快，尤其是在查询非常轻便快速时。
</details>

### listen_tfo

此设置允许所有监听器使用 TCP_FASTOPEN 标志。默认情况下由系统管理，但可以通过设置为 '0' 显式关闭。

有关 TCP Fast Open 扩展的通用知识，请参阅[维基百科](https://en.wikipedia.org/wiki/TCP_Fast_Open)。简而言之，它允许在建立连接时消除一次 TCP 往返时间。

实际上，在许多情况下使用 TFO 可能优化客户端和代理之间的网络效率，就像[持久化代理](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)那样，但无需保持活动连接，同时也不受最大连接数的限制。

在现代操作系统中，TFO 支持通常在系统级别开启，但这只是“功能”，并非强制。Linux（作为最具进步性的系统）自 2011 年起支持，服务器端内核版本从 3.7 开始支持。Windows 从 Windows 10 的某些版本开始支持。其他操作系统（FreeBSD、MacOS）也支持。

对于 Linux 系统，服务器会检查变量 `/proc/sys/net/ipv4/tcp_fastopen` 并根据其设置工作。第 0 位控制客户端，第 1 位控制监听器。默认系统设置为 1，即客户端启用，监听器禁用。

### log

<!-- example conf log -->
log 设置指定了记录所有 `searchd` 运行时事件的日志文件名。如果未指定，默认名称为 'searchd.log'。

或者，你可以使用 'syslog' 作为文件名。在这种情况下，事件将发送到 syslog 守护进程。若要使用 syslog 选项，需要在构建 Manticore 时使用 `-–with-syslog` 选项进行配置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
log = /var/log/searchd.log
```
<!-- end -->


### max_batch_queries

<!-- example conf max_batch_queries -->
限制每批次查询的数量。可选，默认值为 32。

使 searchd 对使用[多查询](../Searching/Multi-queries.md)时单批提交的查询数量进行合理性检查。设为0可跳过检查。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_batch_queries = 256
```
<!-- end -->

### max_connections

<!-- example max_connections -->
最大同时客户端连接数。默认无限制。通常只有在使用任何类型的持久连接时才明显，比如 CLI mysql 会话或来自远程分布式表的持久远程连接。当超出限制时，仍可使用[VIP 连接](../Connecting_to_the_server/MySQL_protocol.md#VIP-connection)连接服务器。VIP 连接不计入限制。

<!-- request Example -->
```ini
max_connections = 10
```

<!-- end -->

### max_threads_per_query

<!-- example max_threads_per_query -->
实例范围内一个操作可以使用的线程数限制。默认情况下，适当的操作可以占用所有 CPU 核心，留不给其他操作任何空间。例如，针对大型 percolate 表的 `call pq` 可能会使用所有线程运行数十秒。将 `max_threads_per_query` 设置为比如 [threads](../Server_settings/Searchd.md#threads) 的一半，将确保你可以并行运行几个这样的 `call pq` 操作。

你也可以在运行时将此设置作为会话或全局变量进行设置。

此外，你可以借助 [threads OPTION](../Searching/Options.md#threads) 以每查询为单位控制行为。

<!-- intro -->
##### 示例：
<!-- request Example -->

```ini
max_threads_per_query = 4
```

<!-- end -->

### max_filters

<!-- example conf max_filters -->
每个查询允许的最大过滤器数量。此设置仅用于内部健全性检查，不直接影响内存使用或性能。可选，默认值是 256。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_filters = 1024
```
<!-- end -->


### max_filter_values

<!-- example conf max_filter_values -->
每个过滤器允许的最大值数量。此设置仅用于内部健全性检查，不直接影响内存使用或性能。可选，默认值是 4096。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_filter_values = 16384
```
<!-- end -->


### max_open_files

<!-- example conf max_open_files -->
服务器允许打开的最大文件数称为“软限制”。请注意，服务庞大且碎片化的实时表可能需要将此限制设置得很高，因为每个磁盘块可能占用十几个或更多文件。例如，一个具有1000个块的实时表可能需要同时打开数千个文件。如果你在日志中遇到“Too many open files”错误，可以尝试调整此选项，可能有助于解决问题。

此外还有一个不能被选项超过的“硬限制”。此限制由系统定义，可以在 Linux 的 `/etc/security/limits.conf` 文件中更改。其他操作系统可能有不同的方法，请参阅你的手册获取更多信息。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = 10000
```
<!-- end -->

<!-- example conf max_open_files max -->
除了直接的数值，您可以使用魔术字 'max' 将限制设置为当前可用的硬限制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = max
```
<!-- end -->


### max_packet_size

<!-- example conf max_packet_size -->
允许的最大网络数据包大小。此设置限制来自客户端的查询包和分布式环境中远程代理的响应包。仅用于内部健全性检查，不直接影响内存使用或性能。可选，默认是128M。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_packet_size = 32M
```
<!-- end -->


### mysql_version_string

<!-- example conf mysql_version_string -->
通过 MySQL 协议返回的服务器版本字符串。可选，默认为空（返回 Manticore 版本）。

一些挑剔的 MySQL 客户端库依赖于 MySQL 使用的特定版本号格式，而且有时会根据报告的版本号（而非标示的能力标志）选择不同的执行路径。例如，Python MySQLdb 1.2.2 当版本号不是 X.Y.ZZ 格式时会抛出异常；MySQL .NET 连接器 6.3.x 在 1.x 版本号及特定标志组合下会内部失败等。为了解决这个问题，你可以使用 `mysql_version_string` 指令，让 `searchd` 向通过 MySQL 协议连接的客户端报告不同的版本。（默认情况下，它报告自身版本。）


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
mysql_version_string = 5.0.37
```
<!-- end -->


### net_workers

网络线程数，默认是 1。

当查询请求极高，单线程无法管理所有传入查询时，此设置很有用。


### net_wait_tm

控制网络线程的忙循环间隔。默认值为 -1，可以设置为 -1、0 或正整数。

当服务器配置为纯主节点，仅将请求路由到代理时，重要的是无延迟地处理请求，不允许网络线程休眠。为此使用忙循环。接收请求后，如果 `net_wait_tm` 是正数，网络线程使用 CPU 轮询 `10 * net_wait_tm` 毫秒；如果是 `0`，只用 CPU 轮询。忙循环可以通过设置 `net_wait_tm = -1` 禁用——此时轮询器通过系统轮询调用设置超时为实际代理超时时间。

> **警告：** CPU 忙循环实际上会占用 CPU 核心，因此将此值设置为非默认值会导致即使服务器空闲也有明显的 CPU 使用率。


### net_throttle_accept

定义网络循环每次迭代接受的客户端数量。默认是 0（无限制），对大多数用户来说已足够。此为微调选项，用于高负载场景下控制网络循环吞吐量。


### net_throttle_action

定义网络循环每次迭代处理的请求数。默认是 0（无限制），对大多数用户来说已足够。此为微调选项，用于高负载场景下控制网络循环吞吐量。

### network_timeout

<!-- example conf network_timeout -->
网络客户端请求读写超时时间，单位为秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认是5秒。`searchd` 将强制关闭在此超时内未发送查询或未读取结果的客户端连接。

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

没有两台服务器的厂商和型号是完全相同的，因此`predicted_time_costs`指令允许您配置上述模型的成本。为了方便起见，这些值是整数，以纳秒为单位计数。（max_predicted_time的限制以毫秒为单位，如果要指定成本值为0.000128毫秒而不是128纳秒，稍微容易出错一些。）不需要一次指定所有四个成本，遗漏的将采用默认值。但是，我们强烈建议为了可读性而指定全部。


### preopen_tables

<!-- example conf preopen_tables -->
preopen_tables配置指令指定是否在启动时强制预打开所有表。默认值为1，这意味着无论每个表的[preopen](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Other-performance-related-settings)设置如何，都会预打开所有表。如果设置为0，则每个表的设置可以生效，且默认值为0。

预打开表可以防止搜索查询与轮换之间的竞争，避免查询偶尔失败。但它也会使用更多的文件句柄。在大多数场景中，建议预打开表。

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
pseudo_sharding配置选项启用对本地普通表和实时表的搜索查询并行化，无论它们是通过直接查询还是通过分布式表查询。该功能会自动并行化查询，最多达到`searchd.threads`指定的线程数。

请注意，如果您的工作线程已经忙碌，因为您有：
* 高查询并发
* 任何形式的物理分片：
  - 多个普通表/实时表组成的分布式表
  - 由过多磁盘分片组成的实时表

那么启用pseudo_sharding可能无法带来任何益处，甚至可能导致吞吐量略微下降。如果您优先考虑更高的吞吐量而非更低的延迟，建议禁用此选项。

默认启用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
pseudo_sharding = 0
```
<!-- end -->


### replication_connect_timeout

`replication_connect_timeout`指令定义连接远程节点的超时时间。默认认为该值是毫秒，但可以使用[其他后缀](../Server_settings/Special_suffixes.md)。默认值是1000（1秒）。

连接远程节点时，Manticore最多等待该时间完成连接成功。如果达到超时时间但连接尚未建立，并且启用了`retries`，则会启动重试。


### replication_query_timeout

`replication_query_timeout`设置searchd等待远程节点完成查询的时间。默认值为3000毫秒（3秒），但可以使用后缀指定不同的时间单位。

连接建立后，Manticore最大等待`replication_query_timeout`时间以完成查询。请注意，此超时不同于`replication_connect_timeout`，远程节点可能造成的总延迟是两者之和。


### replication_retry_count

此设置为整数，指定Manticore在报告严重查询错误前尝试连接和查询远程节点的次数。默认值是3。


### replication_retry_delay

此设置为以毫秒为单位的整数（或[特殊后缀](../Server_settings/Special_suffixes.md)），指定复制过程中查询远程节点失败时，Manticore重试前的延迟时间。仅当指定非零值时此设置才相关。默认值是500。

### qcache_max_bytes

<!-- example conf qcache_max_bytes -->
该配置设置用于缓存结果集的最大内存量，单位为字节。默认值是16777216，相当于16兆字节。如果值设为0，则禁用查询缓存。有关查询缓存的更多信息，请参阅[查询缓存](../Searching/Query_cache.md)。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
qcache_max_bytes = 16777216
```
<!-- end -->


### qcache_thresh_msec

整数，单位为毫秒。查询结果被缓存的最小墙钟时间阈值。默认是3000，即3秒。0表示缓存所有结果。详见[查询缓存](../Searching/Query_cache.md)。该值也可以使用时间[特殊后缀](../Server_settings/Special_suffixes.md)表示，但请谨慎使用，不要和本身包含‘_msec’的名字混淆。


### qcache_ttl_sec

整数，单位为秒。缓存结果集的过期时间。默认值是60，即1分钟。最小可设置值为1秒。详见[查询缓存](../Searching/Query_cache.md)。该值也可以使用时间[特殊后缀](../Server_settings/Special_suffixes.md)表示，但请谨慎使用，不要和本身包含‘_sec’的名字混淆。


### query_log_format

<!-- example conf query_log_format -->
查询日志格式。可选值为`plain`和`sphinxql`，默认是`sphinxql`。

`sphinxql`模式记录有效的SQL语句。`plain`模式以纯文本格式记录查询（主要适用于纯全文用例）。此指令允许您在搜索服务器启动时切换两种格式。日志格式也可以通过`SET GLOBAL query_log_format=sphinxql`语法动态更改。有关详细信息，请参阅[查询日志](../Logging/Query_logging.md)。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_format = sphinxql
```
<!-- end -->

### query_log_min_msec

限制查询被写入查询日志的时间（以毫秒为单位）。可选，默认值为0（所有查询都写入查询日志）。该指令指定只有执行时间超过指定限制的查询才会被记录（该值也可以用时间[special_suffixes](../Server_settings/Special_suffixes.md)表示，但请谨慎使用，不要与自身的名称（包含 `_msec`）混淆）。

### query_log

<!-- example conf query_log -->
查询日志文件名。可选，默认为空（不记录查询）。所有搜索查询（例如SELECT …，但不包括INSERT/REPLACE/UPDATE查询）将被记录在该文件中。格式描述见[查询日志记录](../Logging/Query_logging.md)。在'plain'格式下，可以使用'syslog'作为日志文件路径。这种情况下，所有搜索查询都会以`LOG_INFO`优先级发送到syslog守护进程，前缀为'[query]'，而非时间戳。要使用syslog选项，Manticore必须使用`-–with-syslog`配置编译。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log = /var/log/query.log
```
<!-- end -->


### query_log_mode

<!-- example conf query_log_mode -->
query_log_mode指令允许为searchd和查询日志文件设置不同的权限。默认情况下，这些日志文件的权限为600，即只有运行服务器的用户和root用户可以读取日志文件。
如果你希望允许其他用户读取日志文件，例如以非root用户运行的监控解决方案，这个指令非常有用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_mode  = 666
```
<!-- end -->

### read_buffer_docs

<!-- example conf read_buffer_docs -->
read_buffer_docs指令控制文档列表的每关键字读取缓冲区大小。对于每个搜索查询中的每个关键字出现，分别对应两个读取缓冲区：一个用于文档列表，一个用于命中列表。该设置控制文档列表缓冲区大小。

较大的缓冲区大小可能会增加每个查询的RAM使用，但可能减少I/O时间。对速度较慢的存储介质设置较大值更有意义，而对于具有高IOPS能力的存储，应在较低数值区域进行尝试。

默认值为256K，最小值为8K。你也可以在每个表的基础上设置[read_buffer_docs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs)，这将覆盖服务器配置层面的设置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_docs = 128K
```
<!-- end -->


### read_buffer_hits

<!-- example conf read_buffer_hits -->
read_buffer_hits指令指定搜索查询中每关键字命中列表的读取缓冲区大小。默认值为256K，最小值为8K。对于搜索查询中的每个关键字出现，有两个相关的读取缓冲区，一个用于文档列表，另一个用于命中列表。增加缓冲区大小可能会增加每个查询的RAM使用，但能减少I/O时间。对速度较慢的存储适合设置较大的缓冲区，而对于高IOPS能力的存储，应在较低数值区域进行尝试。

该设置也可以通过[read_buffer_hits](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_hits)中的每表选项单独指定，覆盖服务器层的设置。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_hits = 128K
```
<!-- end -->

### read_unhinted

<!-- example conf read_unhinted -->
未提示读取大小。可选，默认32K，最小1K。

查询时，有些读取可以提前知道需要读取的数据量，但有些则不能。目前最突出的是命中列表大小是未知的。此设置让你控制在这种情况下读取的数据大小。它影响命中列表的I/O时间，对于大于未提示读取大小的列表能减少I/O时间，但对较小列表则增加时间。它**不**影响RAM使用，因为读取缓冲区已分配。因此不应大于read_buffer。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_unhinted = 32K
```
<!-- end -->

### reset_network_timeout_on_packet

<!-- example conf reset_network_timeout_on_packet -->
细化网络超时行为（如`network_timeout`和`agent_query_timeout`）。

设置为0时，超时限制发送整个请求/查询的最大时间。
设置为1（默认）时，超时限制网络活动之间的最大时间。

在复制时，节点可能需要向另一个节点发送大文件（例如100GB）。假设网络传输速率为1GB/s，数据包大小为4-5MB。传输整个文件需要100秒。默认5秒的超时只允许传输5GB数据，然后连接会被断开。增加超时可以作为解决办法，但不可扩展（例如，下一个文件可能是150GB，又会失败）。但是，有了默认`reset_network_timeout_on_packet`设置为1，超时应用于单个数据包之间的时间。只要传输进行中（且超时期间有数据接收），连接保持活动状态。如果传输卡住，包之间超时则连接断开。

注意，如果设置分布式表，主节点和代理节点都应调优。主节点影响`agent_query_timeout`；代理节点影响`network_timeout`。

<!-- intro -->

##### 示例：

<!-- request Example -->

```ini
reset_network_timeout_on_packet = 0
```

<!-- end -->


### rt_flush_period

<!-- example conf rt_flush_period -->
RT表RAM块刷新检查周期，单位秒（或[special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认为10小时。

主动更新的完全适合RAM块的RT表，仍然可能导致binlog不断增长，从而影响磁盘使用和崩溃恢复时间。通过此指令，搜索服务器会定期执行刷新检查，符合条件的RAM块可以被保存，从而实现binlog的相应清理。更多详情请参见 [二进制日志记录](../Logging/Binary_logging.md)。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_flush_period = 3600 # 1 hour
```
<!-- end -->


### rt_merge_iops

<!-- example conf rt_merge_iops -->
RT块合并线程每秒允许启动的最大I/O操作数。可选，默认为0（无限制）。

该指令允许你限制由 `OPTIMIZE` 语句引起的I/O影响。可以确保所有RT优化活动产生的磁盘IOPS（每秒I/O次数）不会超过配置的上限。限制rt_merge_iops可以减少合并操作导致的搜索性能下降。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_iops = 40
```
<!-- end -->

### rt_merge_maxiosize

<!-- example conf rt_merge_maxiosize -->
RT块合并线程每次I/O操作允许的最大大小。可选，默认为0（无限制）。

该指令允许你限制由 `OPTIMIZE` 语句引起的I/O影响。大于此限制的I/O将被分为两个或多个I/O，然后在应用 [rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops) 限制时分别计数。因此，可以确保所有优化活动产生的磁盘I/O不会超过（rt_merge_iops * rt_merge_maxiosize）字节/秒。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_maxiosize = 1M
```
<!-- end -->


### seamless_rotate

<!-- example conf seamless_rotate -->
防止`searchd`在轮换需要大量数据预缓存的表时发生卡顿。可选，默认为1（启用无缝轮换）。在Windows系统中，默认禁用无缝轮换。

表可能包含需要预缓存到RAM中的一些数据。目前，`.spa`、`.spb`、`.spi` 和 `.spm` 文件会被完全预缓存（它们分别包含属性数据、blob属性数据、关键字表和已删除行映射）。未启用无缝轮换时，轮换表会尽量少用RAM，流程如下：

1. 临时拒绝新查询（返回“重试”错误码）；
2. `searchd`等所有正在运行的查询结束；
3. 旧表释放，相关文件重命名；
4. 新表文件重命名，所需RAM分配；
5. 新表属性和字典数据预加载到RAM；
6. `searchd`开始使用新表继续服务查询。

但如果属性或字典数据量很大，预加载步骤可能需要较长时间——预加载1-5+ GB文件时可能长达几分钟。

启用无缝轮换后，轮换流程如下：

1. 分配新表的RAM存储空间；
2. 新表属性和字典数据异步预加载到RAM；
3. 预加载成功后，释放旧表，并同时重命名两组文件；
4. 失败则释放新表；
5. 任何时刻，查询都只会从旧表或新表的一个副本获得服务。

无缝轮换的代价是在轮换期间会有更高的内存峰值（因为在预加载新副本时，旧的和新的 `.spa/.spb/.spi/.spm` 数据都需要驻留在RAM），但平均使用量保持不变。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
seamless_rotate = 1
```
<!-- end -->

### secondary_index_block_cache
<!-- example conf secondary_index_block_cache -->

该选项用于指定二级索引使用的块缓存大小。可选，默认值为8 MB。当二级索引与包含多值的过滤器（如IN()过滤器）一起工作时，会读取和处理这些值的元数据块。
在连接（join）查询中，该过程会对左表的每一批数据重复一次，每批可能在同一个连接查询内重复读取同一元数据。这会严重影响性能。元数据块缓存可将这些块保留在内存中，
以便随后的批次可以重复利用。

该缓存仅在连接查询中使用，对非连接查询无影响。注意，缓存大小限制针对每个属性和每个二级索引单独生效。每个磁盘块中的每个属性都在这个限制内工作。在最坏情况下，内存总用量
可通过将该限制乘以磁盘块数和连接查询中用到的属性数来估算。

设置 `secondary_index_block_cache = 0` 可禁用缓存。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
secondary_index_block_cache = 16M
```

<!-- end -->

### secondary_indexes
<!-- example conf secondary_indexes -->

该选项用于启用/禁用搜索查询时二级索引的使用。可选，默认值为1（启用）。请注意，建索引时不需打开该选项，只要已安装 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) 即自动启用。若要在搜索时使用二级索引，也必须安装该库。可用的三种模式如下：

* `0`：禁用搜索时使用二级索引。可通过 [分析器提示](../Searching/Options.md#Query-optimizer-hints) 为单个查询启用
* `1`：启用搜索时使用二级索引。可通过 [分析器提示](../Searching/Options.md#Query-optimizer-hints) 为单个查询禁用
* `force`：与启用类似，但如加载二级索引期间发生任何错误，则会报告错误，整个索引不会加载进守护进程。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
secondary_indexes = 1
```

<!-- end -->

### server_id

<!-- example conf server_id -->
整数，用作服务器标识符，用作种子来生成属于复制集群节点的唯一短 UUID。server_id 在集群节点间必须唯一，且范围为 0 到 127。如果未设置 server_id，则将根据 MAC 地址和 PID 文件路径的哈希计算，或者使用随机数作为短 UUID 的种子。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
server_id = 1
```
<!-- end -->


### shutdown_timeout

<!-- example conf shutdown_timeout -->
`searchd --stopwait` 的等待时间，单位为秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认值为 60 秒。

当您运行 `searchd --stopwait` 时，服务器需要在停止之前执行一些操作，例如完成查询、刷新 RT RAM 块、刷新属性以及更新 binlog。这些任务需要一定时间。`searchd --stopwait` 会等待最多 `shutdown_time` 秒，以让服务器完成其任务。适当的时间取决于您的表大小和负载。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
shutdown_timeout = 3m # wait for up to 3 minutes
```
<!-- end -->


### shutdown_token

需要用来通过 VIP Manticore SQL 连接调用 'shutdown' 命令的密码的 SHA1 哈希值。如果没有该密码，[debug](../Reporting_bugs.md#DEBUG) 模式下的 'shutdown' 子命令永远不会使服务器停止。请注意，这种简单的哈希不应被视为强保护措施，因为我们未使用加盐哈希或任何现代哈希函数。它仅作为局域网内的管理守护进程的防呆措施。

### snippets_file_prefix

<!-- example conf snippets_file_prefix -->
在生成 snippets 时用于本地文件名前缀。可选，默认是当前工作文件夹。

此前缀可与 `load_files` 或 `load_files_scattered` 选项一起用于分布式 snippets 生成。

注意，这是一个前缀，**不是路径！** 这意味着如果前缀设置为 "server1"，而请求引用的是 "file23"，`searchd` 会尝试打开 "server1file23"（以上均不含引号）。所以，如果您需要它作为路径，必须包括末尾斜杠。

构造出最终文件路径后，服务器将展开所有相对目录，并将最终结果与 `snippet_file_prefix` 的值进行比较。如果结果未以该前缀开头，则该文件会被拒绝，并显示错误信息。

例如，如果设置为 `/mnt/data`，某人调用 snippet 生成时，文件源为 `../../../etc/passwd`，他们将收到错误信息：

`File '/mnt/data/../../../etc/passwd' escapes '/mnt/data/' scope`

而不是文件内容。

如果该参数未设置并且读取 `/etc/passwd`，实际上会读取 /daemon/working/folder/etc/passwd，因为该参数的默认值是服务器的工作文件夹。

另请注意，这是本地选项；不会影响任何代理。因此，您可以安全地在主服务器上设置前缀。路由到代理的请求不会受主服务器设定的影响，但会受到代理自身设置的影响。

这在文档存储位置（无论是本地存储还是 NAS 挂载点）在各服务器间不一致时特别有用。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
snippets_file_prefix = /mnt/common/server1/
```
<!-- end -->

> **警告：** 如果您仍希望访问文件系统根目录下的文件，必须显式将 `snippets_file_prefix` 设置为空值（通过 `snippets_file_prefix=` 行），或设置为根目录（通过 `snippets_file_prefix=/`）。


### sphinxql_state

<!-- example conf sphinxql_state -->
当前 SQL 状态序列化保存的文件路径。

服务器启动时将重放此文件。在符合条件的状态更改时（例如，SET GLOBAL），此文件会自动重写。此功能可以防止一个难以诊断的问题：如果加载了 UDF 函数但 Manticore 崩溃，当它（自动）重启时，您的 UDF 和全局变量将不再可用。使用持久化状态有助于确保优雅恢复，避免意外。

`sphinxql_state` 不能用于执行任意命令，如 `CREATE TABLE`。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
sphinxql_state = uservars.sql
```
<!-- end -->


### sphinxql_timeout

<!-- example conf sphinxql_timeout -->
在使用 SQL 接口时，两个请求间的最长等待时间（单位为秒，或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认是 15 分钟。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
sphinxql_timeout = 15m
```
<!-- end -->


### ssl_ca

<!-- example conf ssl_ca -->
SSL 证书颁发机构 (CA) 证书文件的路径（也称根证书）。可选，默认值为空。不为空时，`ssl_cert` 中的证书应由此根证书签名。

服务器使用 CA 文件验证证书签名。该文件必须是 PEM 格式。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ssl_ca = keys/ca-cert.pem
```
<!-- end -->


### ssl_cert

<!-- example conf ssl_cert -->
服务器 SSL 证书文件路径。可选，默认值为空。

服务器使用此证书作为自签名公钥，用于通过 SSL 加密 HTTP 流量。该文件必须是 PEM 格式。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ssl_cert = keys/server-cert.pem
```
<!-- end -->


### ssl_key

<!-- example conf ssl_key -->
SSL 证书密钥文件路径。可选，默认值为空。

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
每查询最大公共子树文档缓存大小。可选，默认是 0（禁用）。

此设置限制公共子树优化器的 RAM 使用（参见 [多查询](../Searching/Multi-queries.md)）。每个查询最多消耗指定的内存用于缓存文档条目。设置为 0 禁用优化器。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
subtree_docs_cache = 8M
```
<!-- end -->


### subtree_hits_cache

<!-- example conf subtree_hits_cache -->
每查询的最大公共子树命中缓存大小。可选，默认值为 0（禁用）。

该设置限制公共子树优化器的内存使用（参见 [multi-queries](../Searching/Multi-queries.md)）。最多可用这么多内存缓存每个查询的关键词出现次数（命中）。将限制设置为 0 禁用优化器。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
subtree_hits_cache = 16M
```
<!-- end -->

### threads

<!-- example threads -->
Manticore 守护进程的工作线程数（或线程池大小）。Manticore 启动时创建这么多个操作系统线程，这些线程执行守护进程内的所有任务，如执行查询、创建摘要等。有些操作可能被拆分为子任务并行执行，比如：

* 在实时表中搜索
* 在由本地表组成的分布式表中搜索
* 逐条查询调用
* 以及其他

默认情况下，线程数设置为服务器 CPU 核心数。Manticore 启动时创建这些线程，直到停止时才释放。每个子任务需要时可使用线程，子任务完成后释放线程供另一个子任务使用。

在高强度 I/O 负载时，设置大于 CPU 核心数的值可能更合理。

<!-- request Example -->
```ini
threads = 10
```

<!-- end -->

### thread_stack

<!-- example conf thread_stack -->
每个作业的最大堆栈大小（协程，一个搜索查询可能产生多个作业/协程）。可选，默认值为 128K。

每个作业有自己的 128K 堆栈。执行查询时，会检查需要多少堆栈空间。如果默认的 128K 足够，则直接处理。若需要更多，会调度另一个分配了更大堆栈的作业继续处理。此高级堆栈的最大大小受此设置限制。

将值设置得相当高有助于处理非常深的查询，同时不会导致整体内存消耗过大。比如，设置为 1G 并不意味着每个新作业都占用 1G 内存，而是如果作业需要 100M 堆栈，就会分配 100M。其他作业仍使用默认的 128K 堆栈。类似地，能运行需要 500M 堆栈的更复杂查询。只有当作业实际需要超过 1G 堆栈时，才会失败并报告 thread_stack 过低。

但实际上，即使是需要 16M 堆栈的查询通常也太复杂，解析耗时耗资源过多。守护进程仍会处理，但用 `thread_stack` 限制这类查询是合理的。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
thread_stack = 8M
```
<!-- end -->


### unlink_old

<!-- example conf unlink_old -->
确定在成功轮换时是否解绑 `.old` 表副本。可选，默认值为 1（解绑）。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
unlink_old = 0
```
<!-- end -->


### watchdog

<!-- example conf watchdog -->
线程服务器看门狗。可选，默认值为 1（启用看门狗）。

当 Manticore 查询崩溃时，可能导致整个服务器崩溃。启用看门狗功能后，`searchd` 会维护一个轻量级的独立进程，监控主服务器进程，异常终止时自动重启。看门狗默认启用。

<!-- request Example -->

```ini
watchdog = 0 # disable watchdog
```
<!-- end -->
<!-- proofread -->

