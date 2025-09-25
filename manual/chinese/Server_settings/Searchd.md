# Section "Searchd" in configuration

以下设置用于 Manticore Search 配置文件中 `searchd` 部分，以控制服务器的行为。以下是每个设置的摘要：

### access_plain_attrs

此设置为 [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。此设置是可选的，默认值为 `mmap_preread`。

`access_plain_attrs` 指令允许你为该 searchd 实例管理的所有表定义 [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每表指令具有更高的优先级，会覆盖此实例范围的默认值，从而提供更细粒度的控制。

### access_blob_attrs

此设置为 [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。此设置是可选的，默认值为 `mmap_preread`。

`access_blob_attrs` 指令允许你为该 searchd 实例管理的所有表定义 [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每表指令具有更高的优先级，会覆盖此实例范围的默认值，从而提供更细粒度的控制。

### access_doclists

此设置为 [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。此设置是可选的，默认值为 `file`。

`access_doclists` 指令允许你为该 searchd 实例管理的所有表定义 [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每表指令具有更高的优先级，会覆盖此实例范围的默认值，从而提供更细粒度的控制。

### access_hitlists

此设置为 [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。此设置是可选的，默认值为 `file`。

`access_hitlists` 指令允许你为该 searchd 实例管理的所有表定义 [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每表指令具有更高的优先级，会覆盖此实例范围的默认值，从而提供更细粒度的控制。

### access_dict

此设置为 [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。此设置是可选的，默认值为 `mmap_preread`。

`access_dict` 指令允许你为该 searchd 实例管理的所有表定义 [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每表指令具有更高的优先级，会覆盖此实例范围的默认值，从而提供更细粒度的控制。

### agent_connect_timeout

此设置为 [agent_connect_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) 参数设置实例范围的默认值。


### agent_query_timeout

此设置为 [agent_query_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) 参数设置实例范围的默认值。可以通过在查询中使用 `OPTION agent_query_timeout=XXX` 子句按查询覆盖该值。


### agent_retry_count

此设置是一个整数，指定 Manticore 在报告致命查询错误之前，通过分布式表连接并查询远程代理的尝试次数。默认值是 0（即不重试）。你也可以通过在查询中使用 `OPTION retry_count=XXX` 子句按查询设置此值。如果提供了按查询选项，将覆盖配置中指定的值。

注意，如果你在分布式表定义中使用了 [agent mirrors](../Creating_a_cluster/Remote_nodes/Mirroring.md#Agent-mirrors)，服务器将根据所选的 [ha_strategy](../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) 为每次连接尝试选择不同的镜像。在这种情况下，`agent_retry_count` 会针对集合中的所有镜像汇总。

例如，如果你有 10 个镜像并设置了 `agent_retry_count=5`，服务器将最多重试 50 次，假设每个镜像平均尝试 5 次（使用 `ha_strategy = roundrobin` 选项时即为如此）。

然而，为 [agent](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) 提供的 `retry_count` 选项作为一个绝对限制。换句话说，代理定义中的 `[retry_count=2]` 选项总是意味着最多尝试 2 次，无论你为代理指定了 1 个还是 10 个镜像。

### agent_retry_delay

此设置是一个以毫秒为单位的整数（或 [special_suffixes](../Server_settings/Special_suffixes.md)），指定在失败时 Manticore 在重试查询远程代理之前的等待时间。此值仅在指定了非零的[agent_retry_count](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)或非零的按查询 `retry_count` 时才相关。默认值为 500。你也可以通过在查询中使用 `OPTION retry_delay=XXX` 子句按查询设置此值。如果提供了按查询选项，将覆盖配置中指定的值。


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

二进制日志用于 RT 表数据的崩溃恢复以及纯磁盘索引的属性更新，否则这些更新只会存储在 RAM 中直到刷新。当启用日志记录时，每个提交到 RT 表的事务都会写入日志文件。在非正常关闭后启动时，日志会自动重放，从而恢复已记录的更改。

`binlog_path` 指令指定二进制日志文件的位置。它只应包含路径；`searchd` 会根据需要在该目录中创建和删除多个 `binlog.*` 文件（包括 binlog 数据、元数据和锁文件等）。

空值表示禁用二进制日志，这会提升性能但会使 RT 表数据面临风险。


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
此设置控制 [boolean_simplify](../Searching/Options.md#boolean_simplify) 搜索选项的默认值。该设置为可选，默认值为 1（启用）。

当设置为 1 时，服务器将自动应用[布尔查询优化](../Searching/Full_text_matching/Boolean_optimization.md)以提升查询性能。设置为 0 时，默认情况下查询将不进行优化。可以通过相应的搜索选项 `boolean_simplify` 按查询覆盖此默认值。

<!-- request Example -->
```ini
searchd {
    boolean_simplify = 0  # disable boolean optimization by default
}
```
<!-- end -->

### buddy_path

<!-- example conf buddy_path -->
此设置决定 Manticore Buddy 二进制文件的路径。该设置为可选，默认值是构建时配置的路径，不同操作系统可能不同。通常不需要修改此设置。但如果你想在调试模式下运行 Manticore Buddy、修改 Manticore Buddy 或实现新的插件，这会很有用。在后者情况下，你可以从 https://github.com/manticoresoftware/manticoresearch-buddy 克隆 Buddy，向 `./plugins/` 目录添加新插件，切换到 Buddy 源代码目录后运行 `composer install --prefer-source` 以方便开发。

为了保证能运行 `composer`，你的机器必须安装 PHP 8.2 或更高版本以及以下扩展：

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

你也可以选择发布版中的适用于 Linux amd64 的特殊 `manticore-executor-dev` 版本，例如：https://github.com/manticoresoftware/executor/releases/tag/v1.0.13

如果采用此方案，记得将开发版的 manticore executor 软链接到 `/usr/bin/php`。

要禁用 Manticore Buddy，将值设置为空，如示例所示。

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
此设置决定在使用持久连接时，两个请求间最大等待时间（单位为秒或[special_suffixes](../Server_settings/Special_suffixes.md)）。为可选，默认值为五分钟。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
client_timeout = 1h
```
<!-- end -->


### collation_libc_locale

<!-- example conf collation_libc_locale -->
服务器 libc 区域设置。可选，默认为 C。

指定 libc 区域，影响基于 libc 的排序规则。详情参见[排序规则](../Searching/Collations.md)部分。


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

指定传入请求使用的默认排序规则。排序规则可在每个查询中覆盖。有关可用排序规则和详细信息，请参见[排序规则](../Searching/Collations.md)部分。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
collation_server = utf8_ci
```
<!-- end -->


### data_dir

<!-- example conf data_dir -->
如果指定此设置，则启用[实时模式](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)，这是一种强制方式管理数据模式。该值应为目录路径，用于存储所有表格、二进制日志及 Manticore Search 在此模式下正常运行所需的所有内容。
当指定 `data_dir` 时，不允许索引[纯表](../Creating_a_table/Local_tables/Plain_table.md)。有关 RT 模式与纯模式之间区别的更多信息，请参见[本节](../Read_this_first.md#Real-time-table-vs-plain-table)。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
data_dir = /var/lib/manticore
```
<!-- end -->

### diskchunk_flush_search_timeout

<!-- example conf diskchunk_flush_search_timeout -->
用于防止自动刷新 RAM 块时表中无搜索的超时时间。可选，默认 30 秒。

检查搜索情况的时间，用于判定是否执行自动刷新。
只有当表在最近 `diskchunk_flush_search_timeout` 秒内至少执行过一次搜索，才会发生自动刷新。此设置与 [diskchunk_flush_write_timeout](../Server_settings/Searchd.md#diskchunk_flush_write_timeout) 配合工作。对应的[单表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_search_timeout)优先级更高，能覆盖该实例范围的默认值，提供更细粒度的控制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
diskchunk_flush_search_timeout = 120s
```
<!-- end -->

### diskchunk_flush_write_timeout

<!-- example conf diskchunk_flush_write_timeout -->
在无写操作后等待多少秒自动将 RAM 块刷新到磁盘。可选，默认值为 1 秒。

如果在 `diskchunk_flush_write_timeout` 秒内未对 RAM 块进行写操作，该块将被刷新到磁盘。此设置与 [diskchunk_flush_search_timeout](../Server_settings/Searchd.md#diskchunk_flush_search_timeout) 配合使用。要禁用自动刷新，请在配置中显式设置 `diskchunk_flush_write_timeout = -1`。对应的 [每表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_write_timeout) 优先级更高，会覆盖此实例范围的默认设置，从而提供更细粒度的控制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
diskchunk_flush_write_timeout = 60s
```
<!-- end -->

### docstore_cache_size

<!-- example conf docstore_cache_size -->
此设置指定文档存储中文档块在内存中保留的最大大小。此设置为可选，默认值为 16m（16 兆字节）。

当使用 `stored_fields` 时，文档块将从磁盘读取并解压。由于每个块通常包含多个文档，因此在处理下一个文档时，可能会重复使用该块。为此，块被存放在服务器范围的缓存中。缓存中保存的是未压缩的块。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
docstore_cache_size = 8m
```
<!-- end -->

### engine

<!-- example conf engine -->
创建 RT 模式表时默认使用的属性存储引擎。可以是 `rowwise`（默认）或 `columnar`。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
engine = columnar
```
<!-- end -->


### expansion_limit

<!-- example conf expansion_limit -->
此设置确定单个通配符的最大扩展关键词数。此设置为可选，默认值为 0（无限制）。

在针对启用了 `dict = keywords` 的表进行子串搜索时，单个通配符可能会匹配成千上万甚至数百万个关键词（比如匹配整个牛津词典中的 `a*`）。此指令允许您限制此类扩展的影响。设置 `expansion_limit = N` 会限制扩展至每个通配符查询中匹配频率最高的最多 N 个关键词。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_limit = 16
```
<!-- end -->

### expansion_merge_threshold_docs

<!-- example conf expansion_merge_threshold_docs -->
此设置确定展开关键词中允许合并的最大文档数。此设置为可选，默认值为 32。

在针对启用了 `dict = keywords` 的表进行子串搜索时，单个通配符可能匹配成千上万甚至数百万关键词。此指令允许您提高合并关键词的数量限制，以加快匹配速度，但搜索过程中会使用更多内存。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_merge_threshold_docs = 1024
```
<!-- end -->

### expansion_merge_threshold_hits

<!-- example conf expansion_merge_threshold_hits -->
此设置确定展开关键词中允许合并的最大命中数。此设置为可选，默认值为 256。

在针对启用了 `dict = keywords` 的表进行子串搜索时，单个通配符可能匹配成千上万甚至数百万关键词。此指令允许您提高合并关键词的数量限制，以加快匹配速度，但搜索过程中会使用更多内存。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_merge_threshold_hits = 512
```
<!-- end -->

### expansion_phrase_limit

<!-- example conf expansion_phrase_limit -->
此设置控制由于 `PHRASE`、`PROXIMITY` 和 `QUORUM` 操作符内的 `OR` 运算符生成的最大备选短语变体数。此设置为可选，默认值为 1024。

当在短语类操作符内使用 `|`（OR）运算符时，根据指定的备选项数，扩展组合总数可能呈指数级增长。该设置通过限制查询处理时考虑的排列组合数量来防止过度查询扩展。如果生成的变体数超过此限制，查询将失败并返回错误。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_phrase_limit = 4096
```
<!-- end -->

### grouping_in_utc

此设置指定 API 和 SQL 中的时间分组计算是使用本地时区还是 UTC。此设置为可选，默认值为 0（表示“本地时区”）。

默认情况下，所有“按时间分组”的表达式（如 API 中的按天、周、月、年分组，SQL 中的按天、月、年、年月、年月日分组）均基于本地时间。例如，如果您有时间为 `13:00 utc` 和 `15:00 utc` 的文档，分组时它们都会按照您本地时区划分到相应的分组中。如果您处于 `utc` 时区，则为同一天，但如果您处于 `utc+10`，则这两个文档将落入不同的按天分组（因为在 UTC+10 时区，13:00 utc 是当地时间 23:00，但 15:00 utc 是第二天的 01:00）。有时这种行为不可接受，希望时间分组不受时区影响。您可以通过设置全局 TZ 环境变量来运行服务器，但这不仅影响分组，还会影响日志时间戳，这可能也不理想。启用此选项（在配置中或使用 SQL 中的 [SET global](../Server_settings/Setting_variables_online.md#SET) 语句）会使所有时间分组表达式以 UTC 计算，而其他依赖时间的函数（如服务器日志）仍使用本地时区。


### timezone

此设置指定日期/时间相关函数使用的时区。默认使用本地时区，但您可以指定 IANA 格式的其他时区（例如 `Europe/Amsterdam`）。

请注意，该设置不影响日志记录，日志始终采用本地时区。

另外，请注意，如果使用了`grouping_in_utc`，则“按时间分组”功能仍将使用UTC，而其他与日期/时间相关的功能将使用指定的时区。总体来说，不建议混合使用`grouping_in_utc`和`timezone`。

您可以在配置文件中配置此选项，也可以通过SQL中的[SET global](../Server_settings/Setting_variables_online.md#SET)语句进行设置。


### ha_period_karma

<!-- example conf ha_period_karma -->
此设置指定代理镜像统计窗口大小，单位为秒（或[special_suffixes](../Server_settings/Special_suffixes.md)）。该选项为可选，默认值为60秒。

对于包含代理镜像的分布式表（详见[agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)），主节点会跟踪多个不同的每镜像计数器。这些计数器用于故障转移和平衡（主节点根据计数器选择最佳镜像）。计数器以`ha_period_karma`秒为单位累计。

开始新块后，主节点可能仍会使用前一个块的累积值，直到新块填充到一半。因此，任何之前的历史记录最多在1.5倍ha_period_karma秒后停止影响镜像选择。

尽管最多使用两个块进行镜像选择，但最多存储最近15个块以供监控用途。这些块可通过[SHOW AGENT STATUS](../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS)语句进行查看。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_period_karma = 2m
```
<!-- end -->


### ha_ping_interval

<!-- example conf ha_ping_interval -->
此设置配置代理镜像ping命令之间的间隔，单位为毫秒（或[special_suffixes](../Server_settings/Special_suffixes.md)）。该选项为可选，默认值为1000毫秒。

对于包含代理镜像的分布式表（详见[agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)），主节点会在空闲期间向所有镜像发送ping命令，用以跟踪当前代理状态（存活或死活、网络往返等）。此ping间隔由该指令定义。若要禁用ping，将ha_ping_interval设置为0。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_ping_interval = 3s
```
<!-- end -->


### hostname_lookup

`hostname_lookup`选项定义了刷新主机名的策略。默认情况下，代理主机名的IP地址在服务器启动时缓存，以避免频繁访问DNS。然而，在某些情况下，IP可能动态变化（如云主机），可能希望不缓存IP。将此选项设为`request`可禁用缓存，针对每个查询访问DNS。也可以使用`FLUSH HOSTNAMES`命令手动刷新IP地址。

### jobs_queue_size

jobs_queue_size设置定义了队列中允许同时存在的“作业”数量。默认无限制。

大多数情况下，“作业”指的是对单个本地表（普通表或实时表的磁盘块）的一次查询。例如，若分布式表包含2个本地表或实时表有2个磁盘块，则搜索查询大多会将2个作业放入队列。然后，线程池（其大小由[threads](../Server_settings/Searchd.md#threads)定义）进行处理。但在某些情况下，查询过于复杂时，可能会生成更多作业。当[max_connections](../Server_settings/Searchd.md#max_connections)和[threads](../Server_settings/Searchd.md#threads)不足以平衡性能时，建议调整此设置。

### join_batch_size

表连接通过积累匹配批次来工作，这些匹配是对左表执行查询的结果。该批次随后作为单个查询在右表上处理。

此选项允许调整批次大小。默认值为`1000`，设置为`0`将禁用批处理。

较大批次大小可能提升性能；但对于某些查询，可能导致内存消耗过大。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
join_batch_size = 2000
```
<!-- end -->

### join_cache_size

对右表执行的每个查询由特定的JOIN ON条件定义，这些条件决定从右表检索的结果集。

如果只有少数唯一的JOIN ON条件，重复利用结果比重复执行右表查询更高效。为此，结果集被存储在缓存中。

此选项允许配置该缓存的大小。默认值为`20 MB`，设置为0将禁用缓存。

注意，每个线程维护自己的缓存，因此在估计总内存使用时应考虑执行查询的线程数。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
join_cache_size = 10M
```
<!-- end -->

### listen_backlog

<!-- example conf listen_backlog -->
listen_backlog设置决定TCP监听等待队列的长度，适用于传入连接。此设置对于一次处理一个请求的Windows版本特别重要。当连接队列达到限制时，新连接将被拒绝。
对于非Windows版本，默认值通常已足够，无需调整此设置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
listen_backlog = 20
```
<!-- end -->

### kibana_version_string

<!-- example conf kibana_version_string -->
一个服务器版本字符串，用于返回给Kibana或OpenSearch Dashboards。可选，默认值为`7.6.0`。

某些版本的Kibana和OpenSearch Dashboards期望服务器报告特定版本号，且行为可能因此不同。为解决此类问题，可以使用此设置，使Manticore向Kibana或OpenSearch Dashboards报告自定义版本。

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

`listen` 的通用语法是：

```ini
listen = ( address ":" port | port | path | address ":" port start - port end ) [ ":" protocol [ "_vip" ] [ "_readonly" ] ]
```

您可以指定：
* IP 地址（或主机名）和端口号
* 或仅端口号
* 或 Unix 套接字路径（Windows 不支持）
* 或 IP 地址和端口范围

如果您只指定端口号而未指定地址，`searchd` 会监听所有网络接口。Unix 路径以斜杠开头表示。端口范围只能用于复制协议。

您还可以为该套接字上的连接指定协议处理器（监听器）。监听器包括：

* **未指定** - Manticore 将在此端口接受来自：
  - 其他 Manticore 节点（例如，远程分布式表）
  - 通过 HTTP 和 HTTPS 的客户端
  - [Manticore Buddy](https://manticoresearch.com/blog/manticoresearch-buddy-intro/)。**请确保您有此类型的监听器（或如下所述的 `http` 监听器），以免影响 Manticore 功能的限制。**
* `mysql` MySQL 协议，用于来自 MySQL 客户端的连接。注意：
  - 也支持压缩协议。
  - 如果启用[SSL](../Security/SSL.md#SSL)，则可以建立加密连接。
* `replication` - 用于节点间通信的复制协议。更多细节可见[复制](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md)部分。您可以指定多个复制监听器，但它们必须监听同一 IP；仅端口可以不同。当您使用端口范围定义复制监听器时（例如 `listen = 192.168.0.1:9320-9328:replication`），Manticore 不会立即开始监听这些端口。而是在开始使用复制时从指定范围内随机选择空闲端口。复制正常工作需要端口范围内至少有 2 个端口。
* `http` - 与**未指定**相同。Manticore 会在该端口接受来自远程节点和通过 HTTP、HTTPS 的客户端连接。
* `https` - HTTPS 协议。Manticore 仅接受此端口上的 HTTPS 连接。更多信息见[SSL](../Security/SSL.md)章节。
* `sphinx` - 传统二进制协议。用于服务来自远程 [SphinxSE](../Extensions/SphinxSE.md) 客户端的连接。部分 Sphinx API 客户端实现（如 Java 版本）需要显式声明监听器。

在客户端协议后缀中添加 `_vip`（即除 `replication` 外，如 `mysql_vip`、`http_vip` 或单独的 `_vip`）会强制为连接创建专用线程，以绕过各种限制。这对于节点维护时服务器极度负载、否则可能挂起或不允许通过常规端口连接的情况非常有用。

后缀 `_readonly` 为监听器设置[只读模式](../Security/Read_only.md)，限制只接受读取查询。

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

可以有多个 `listen` 指令。`searchd` 会在所有指定的端口和套接字上监听客户端连接。Manticore 包中提供的默认配置定义了监听端口：
* `9308` 和 `9312`，用于远程节点和非 MySQL 客户端连接
* 以及 `9306` 端口，用于 MySQL 连接。

如果配置中根本未指定任何 `listen`，Manticore 将等待以下连接：
* `127.0.0.1:9306`，用于 MySQL 客户端
* `127.0.0.1:9312`，用于 HTTP/HTTPS 以及来自其他 Manticore 节点和基于 Manticore 二进制 API 的客户端连接。

#### 监听特权端口

默认情况下，Linux 不允许您让 Manticore 监听 1024 以下的端口（例如 `listen = 127.0.0.1:80:http` 或 `listen = 127.0.0.1:443:https`），除非您以 root 身份运行 searchd。如果您仍想在非 root 用户下启动 Manticore，并监听低于 1024 的端口，请考虑执行以下任一操作（任意一种方法均可）：
* 执行命令 `setcap CAP_NET_BIND_SERVICE=+eip /usr/bin/searchd`
* 在 Manticore 的 systemd 单元中添加 `AmbientCapabilities=CAP_NET_BIND_SERVICE`，并重载守护进程（`systemctl daemon-reload`）。

#### 关于 Sphinx API 协议和 TFO 的技术细节
<details>
传统 Sphinx 协议包含两个阶段：握手交换和数据流。握手由客户端发送的 4 字节数据包和守护进程发送的 4 字节数据包组成，唯一目的是让客户端确认远端是实际的 Sphinx 守护进程，守护进程确认远端是实际的 Sphinx 客户端。主要数据流非常简单：双方都声明各自的握手，另一方进行验证。此种短包交换隐含使用特殊的 `TCP_NODELAY` 标志，该标志关闭 Nagle 的 TCP 算法，声明 TCP 连接将作为小包的对话进行。
然而，此协商中谁先发言并未严格定义。历史上，所有使用二进制 API 的客户端都先发言：发送握手，接着读取守护进程返回的 4 字节，再发送请求并读取守护进程的响应。
我们提升 Sphinx 协议兼容性时考虑了以下事项：

1. 通常，主节点与代理的通信是在已知客户端和已知主机、端口间建立。因此，端点提供错误握手的可能性极低。故可默认双方都是有效的，并确实使用 Sphinx 协议通信。
2. 在此假设下，我们可以将握手“粘合”到真实请求中，一起发送一个包。如果后台是传统 Sphinx 守护进程，它会先读取这粘合包中的前 4 字节作为握手，再读取请求体。因为它们都在一个包中，所以后台套接字有 -1 个往返时延，但前端缓冲仍然正常工作。
3. 继续假设：由于“query”数据包很小，握手包更小，我们使用现代TFO（tcp-fast-open）技术，在初始的“SYN” TCP包中同时发送握手和数据包。也就是说：我们使用拼接的握手+主体包连接到远程节点。守护进程接受连接时，立即在套接字缓冲区中获得握手和主体，因为它们都是在第一个TCP“SYN”包中到达的。这样就消除了另外一个RTT。
4. 最后，教守护进程接受这种改进。实际上，从应用程序角度来说，这意味着不使用`TCP_NODELAY`。从系统角度看，这意味着确保守护进程端启用了TFO接收，客户端端启用了TFO发送。在现代系统中，默认客户端TFO是启用的，所以你只需调整服务器端TFO即可使所有功能正常工作。

所有这些改进而没有实际改变协议本身，使我们能够消除连接中TCP协议的1.5个RTT。如果查询和响应能够放入单个TCP包，则整个二进制API会话从3.5 RTT减少到2 RTT——这使得网络协商速度提升了大约2倍。

因此，所有改进基于一个最初未定义的命题：“谁先说话”。如果客户端先发言，我们可以应用所有这些优化，有效地在单个TFO包中处理连接+握手+查询。此外，我们可以查看接收到包的开头，确定真实协议。这就是为什么你可以通过API/http/https连接到同一个端口。如果守护进程必须先发言，所有这些优化都不可能，且多协议也无法实现。这就是为什么我们为MySQL划分了专用端口，而没有把它和其他协议统一到同一端口。突然，在所有客户端中，有一个客户端的设计是要求守护进程先发送握手包。这意味着所有上述改进都无法实现——那就是mysql/mariadb的SphinxSE插件。因此，专门为这个单一客户端我们定义了`sphinx`协议，用最传统的方式工作。也就是说：双方都启用`TCP_NODELAY`并交换小包。连接时守护进程先发送握手，然后客户端发送，之后一切照常运行。这不是非常优化，但可用。如果你使用SphinxSE连接Manticore——必须针对该协议显式声明`sphinx`监听器。其他客户端避免使用该监听器，因为它较慢。如果你使用其他老旧的Sphinx API客户端——先检查它们是否能在非专用多协议端口工作。对于主-代理链接，使用非专用（多协议）端口且启用客户端和服务器TFO效果良好，肯定会加快网络后端的工作，尤其是当你有非常轻量和快速的查询时。
</details>

### listen_tfo

此设置允许所有监听器开启TCP_FASTOPEN标志。默认情况下由系统管理，但可通过设置为“0”显式关闭。

有关TCP Fast Open扩展的一般知识，请参阅[维基百科](https://en.wikipedia.org/wiki/TCP_Fast_Open)。简而言之，它允许在建立连接时消除一个TCP往返时间。

实际上，在许多情况下使用TFO可能会优化客户端-代理的网络效率，仿佛有[持久代理](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)，但不保持活动连接，也不限制最大连接数。

在现代操作系统上，TFO支持通常在系统级别启用，但这只是“能力”，而非规定。Linux（作为最先进系统）自2011年起在3.7以上内核支持（服务器端）。Windows从Windows 10某些版本开始支持。其他操作系统（FreeBSD，MacOS）也支持。

对Linux系统服务器来说，检查变量`/proc/sys/net/ipv4/tcp_fastopen`并据此行为。位0控制客户端，位1控制监听器。默认系统参数为1，即客户端启用，监听器禁用。

### log

<!-- example conf log -->
log 设置指明了所有`searchd`运行时事件的日志文件名称。如未指定，默认名称为`searchd.log`。

或者，你可以使用`syslog`作为文件名。在这种情况下，事件将发送到syslog守护进程。要使用syslog选项，构建时需使用`-–with-syslog`选项配置Manticore。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
log = /var/log/searchd.log
```
<!-- end -->


### max_batch_queries

<!-- example conf max_batch_queries -->
限制每个批次的查询数量。可选，默认值为32。

使searchd在使用[多查询](../Searching/Multi-queries.md)时对单个批次提交的查询数进行合理性检查。设置为0则跳过检查。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_batch_queries = 256
```
<!-- end -->

### max_connections

<!-- example max_connections -->
最大同时客户端连接数。默认无限制。通常仅在使用持久连接（如mysql命令行会话或远程分布式表的持久远程连接）时明显。当超出限制时，仍可使用[VIP连接](../Connecting_to_the_server/MySQL_protocol.md#VIP-connection)连接服务器。VIP连接不计入限制。

<!-- request Example -->
```ini
max_connections = 10
```

<!-- end -->

### max_threads_per_query

<!-- example max_threads_per_query -->
实例级别的线程限制，即一个操作可以使用的线程数。默认情况下，合适的操作可以占用所有CPU核心，导致没有余地给其他操作。例如，对一个相当大的percolate表执行`call pq`可能会占用所有线程数十秒。将`max_threads_per_query`设置为例如[threads](../Server_settings/Searchd.md#threads)的一半，将确保你可以并行运行几个这样的`call pq`操作。

你也可以在运行时将此设置作为会话或全局变量进行设置。

此外，你可以借助[threads OPTION](../Searching/Options.md#threads)按查询来控制行为。

<!-- intro -->
##### 示例：
<!-- request Example -->

```ini
max_threads_per_query = 4
```

<!-- end -->

### max_filters

<!-- example conf max_filters -->
每个查询允许使用的最大过滤器数量。此设置仅用于内部合理性检查，不会直接影响RAM使用或性能。可选，默认值为256。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_filters = 1024
```
<!-- end -->


### max_filter_values

<!-- example conf max_filter_values -->
每个过滤器允许的最大值数量。此设置仅用于内部合理性检查，不会直接影响RAM使用或性能。可选，默认值为4096。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_filter_values = 16384
```
<!-- end -->


### max_open_files

<!-- example conf max_open_files -->
服务器允许打开的最大文件数称为“软限制”。注意，服务大型碎片化实时表可能需要将此限制设置较高，因为每个磁盘块可能占用十几个或更多文件。例如，一个含有1000个块的实时表可能需要同时打开成千上万个文件。如果你在日志中遇到“Too many open files（打开的文件太多）”错误，尝试调整此选项，可能有助于解决问题。

还有一个不能被此选项超过的“硬限制”。此限制由系统定义，可以在Linux的`/etc/security/limits.conf`文件中修改。其他操作系统可能采用不同的方法，请查阅相关手册了解详情。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = 10000
```
<!-- end -->

<!-- example conf max_open_files max -->
除了直接使用数值外，你还可以使用魔术词“max”来设置限制与当前可用的系统硬限制相等。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = max
```
<!-- end -->


### max_packet_size

<!-- example conf max_packet_size -->
允许的最大网络数据包大小。此设置限制客户端的查询包以及分布式环境中远程代理的响应包。仅用于内部合理性检查，不直接影响RAM使用或性能。可选，默认值为128M。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_packet_size = 32M
```
<!-- end -->


### mysql_version_string

<!-- example conf mysql_version_string -->
通过MySQL协议返回的服务器版本字符串。可选，默认是空（返回Manticore版本）。

一些挑剔的MySQL客户端库依赖于MySQL使用的特定版本号格式，此外，有时会根据报告的版本号（而不是指示的能力标志）选择不同的执行路径。例如，Python MySQLdb 1.2.2在版本号不符合X.Y.ZZ格式时会抛出异常；MySQL .NET connector 6.3.x在版本号为1.x并且标志组合特定时内部失败等等。为了解决这个问题，你可以使用`mysql_version_string`指令，让`searchd`向通过MySQL协议连接的客户端报告不同的版本号。（默认情况下，它报告自己的版本）


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
mysql_version_string = 5.0.37
```
<!-- end -->


### net_workers

网络线程数，默认值为1。

当查询速率极高且单线程无法管理所有传入查询时，此设置非常有用。


### net_wait_tm

控制网络线程的忙等待循环间隔。默认值为-1，可以设置为-1、0或正整数。

在服务器被配置为纯主节点且仅路由请求给代理的情况下，重要的是无延迟地处理请求，并且不允许网络线程休眠。为此存在忙等待循环。收到请求后，如果`net_wait_tm`为正数，则网络线程使用CPU轮询`10 * net_wait_tm`毫秒；如果`net_wait_tm`为`0`，则仅使用CPU轮询。同时，忙等待循环可通过`net_wait_tm = -1`禁用——此时轮询器将根据系统轮询调用中代理的实际超时设置超时。

> **警告：** CPU忙等待循环实际上会占用CPU核心资源，因此将此值设置为非默认值即使服务器空闲也会导致明显的CPU使用。


### net_throttle_accept

定义每轮网络循环接受的客户端数量。默认是0（无限制），对于大多数用户来说已足够。此项是针对高负载场景下细调网络循环吞吐量的选项。


### net_throttle_action

定义每轮网络循环处理的请求数量。默认是0（无限制），对于大多数用户来说已足够。此项是针对高负载场景下细调网络循环吞吐量的选项。

### network_timeout

<!-- example conf network_timeout -->
网络客户端请求读写超时时间，单位是秒（或[特殊后缀](../Server_settings/Special_suffixes.md)）。可选，默认值为5秒。`searchd`会强制关闭未能在此超时时间内发送查询或读取结果的客户端连接。

Note also the [reset_network_timeout_on_packet](../Server_settings/Searchd.md#reset_network_timeout_on_packet) parameter. This parameter alters the behavior of `network_timeout` from applying to the entire `query` or `result` to individual packets instead. Typically, a query/result fits within one or two packets. However, in cases where a large amount of data is required, this parameter can be invaluable in maintaining active operations.

<!-- request Example -->

```ini
network_timeout = 10s
```
<!-- end -->

### node_address

<!-- example conf node_address -->
此设置允许您指定节点的网络地址。默认情况下，它设置为复制 [listen](../Server_settings/Searchd.md#listen) 地址。在大多数情况下这是正确的；然而，在某些情况下，您必须手动指定：

* 节点位于防火墙后面
* 启用了网络地址转换（NAT）
* 容器部署，如 Docker 或云部署
* 集群中节点分布在多个区域


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
node_address = 10.101.0.10
```
<!-- end -->

### not_terms_only_allowed

<!-- example conf not_terms_only_allowed -->
此设置决定是否允许仅包含[非](../Searching/Full_text_matching/Operators.md#Negation-operator)全文搜索操作符的查询。可选，默认值为 0（不允许仅含 NOT 操作符的查询）。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
not_terms_only_allowed = 1
```
<!-- end -->

### optimize_cutoff

<!-- example conf optimize_cutoff -->
设置默认的表压缩阈值。详细信息请见 - [Number of optimized disk chunks](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks)。此设置可以被每个查询选项 [cutoff](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 覆盖。也可以通过 [SET GLOBAL](../Server_settings/Setting_variables_online.md#SET) 动态修改。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
optimize_cutoff = 4
```
<!-- end -->

### persistent_connections_limit

<!-- example conf persistent_connections_limit -->
此设置决定连接到远程 [persistent agents](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 的最大同时持久连接数。每次连接 `agent_persistent` 下定义的代理时，我们尝试重用现有连接（如果有），或连接并保存该连接以供将来使用。然而，在某些情况下，限制持久连接的数量是合理的。该指令定义了此限制。它影响所有分布式表中每个代理主机的连接数量。

合理的设置值应小于或等于代理配置中的 [max_connections](../Server_settings/Searchd.md#max_connections) 选项。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
persistent_connections_limit = 29 # assume that each host of agents has max_connections = 30 (or 29).
```
<!-- end -->


### pid_file

<!-- example conf pid_file -->
pid_file 是 Manticore 搜索中的一个必需配置选项，指定存储 `searchd` 服务器进程 ID 的文件路径。

searchd 进程 ID 文件在启动时重新创建并锁定，运行时包含主服务器进程 ID。服务器关闭时会取消链接该文件。
该文件的目的是让 Manticore 执行各种内部任务，例如检查是否已有运行中的 `searchd` 实例、停止 `searchd` 以及通知其应当轮换表。该文件也可用于外部自动化脚本。


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
基于最大查询时间设置在完成前终止查询是个良好的安全保障，但存在固有缺陷：结果不确定（不稳定）。也就是说，如果你多次以时间限制重复执行相同（复杂）查询，时间限制会在不同阶段被触发，且结果集会*不同*。

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

有一个选项，[SELECT … OPTION max_predicted_time](../Searching/Options.md#max_predicted_time)，让你既能限制查询时间，又能获得稳定、可重复的结果。它不再在查询执行时定期检查实际时间（不确定），而是使用简单的线性模型预测当前运行时间：

```ini
predicted_time =
    doc_cost * processed_documents +
    hit_cost * processed_hits +
    skip_cost * skiplist_jumps +
    match_cost * found_matches
```

当 `predicted_time` 达到指定限制时，就提前终止查询。

当然，这并不是对实际耗时的硬性限制（但确实是对所做*处理*工作的硬限制），而且简单的线性模型并非最准确模型。因此，实际墙钟时间*可能*低于或超过目标限制。但是误差范围相当可接受：例如，在我们以 100 毫秒为目标限制的实验中，大多数测试查询的时间在 95 到 105 毫秒范围内，*所有*查询时间都在 80 到 120 毫秒范围内。此外，使用模型时间替代实际运行时间还能减少些许 gettimeofday() 调用。

没有两台服务器的型号和规格是完全相同的，因此 `predicted_time_costs` 指令允许您为上述模型配置成本。出于便利考虑，这些值是整数，以纳秒为单位计数。（max_predicted_time 限制值以毫秒计数，必须以 0.000128 毫秒而不是 128 纳秒来指定成本值，这样更容易出错。）不必同时指定所有四个成本值，未指定的将采用默认值。不过，我们强烈建议为了可读性指定所有成本值。


### preopen_tables

<!-- example conf preopen_tables -->
preopen_tables 配置指令指定是否在启动时强制预先打开所有表。默认值为 1，意味着所有表将被预先打开，而不考虑每个表的 [preopen](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Other-performance-related-settings) 设置。如果设置为 0，则允许每个表的设置生效，其默认值为 0。

预先打开表可以防止搜索查询和轮换操作之间的竞争，避免查询偶尔失败。但这也会使用更多的文件句柄。在大多数场景中，建议预先打开表。

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
pseudo_sharding 配置选项启用对本地普通表和平实时表的搜索查询进行并行处理，无论它们是直接查询还是通过分布式表查询。此功能会自动将查询并行化，最高并行线程数为 `searchd.threads` 中指定的线程数量。

请注意，如果您的工作线程已经忙碌，比如因为：
* 高查询并发
* 任意形式的物理分片：
  - 多个普通/实时表的分布式表
  - 包含过多磁盘块的实时表

那么启用 pseudo_sharding 可能不会带来任何好处，甚至可能导致吞吐量略微下降。如果您优先考虑更高吞吐量而非更低延迟，建议禁用此选项。

默认启用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
pseudo_sharding = 0
```
<!-- end -->


### replication_connect_timeout

`replication_connect_timeout` 指令定义连接远程节点的超时时间。默认值假设为毫秒，但可以使用[其他后缀](../Server_settings/Special_suffixes.md)。默认值为 1000（1 秒）。

连接远程节点时，Manticore 将最多等待此时间完成连接。如果超时但连接未建立，且启用了 `retries`，将启动重试。


### replication_query_timeout

`replication_query_timeout` 设置 searchd 等待远程节点完成查询的时间。默认值为 3000 毫秒（3 秒），但可以通过后缀表示不同时间单位。

建立连接后，Manticore 将最长等待 `replication_query_timeout` 以完成远程节点查询。注意此超时与 `replication_connect_timeout` 分开，远程节点导致的总延迟为二者之和。


### replication_retry_count

此设置为整数，指定 Manticore 在报告致命查询错误前尝试连接和查询远程节点的次数。默认值为 3。


### replication_retry_delay

此设置为毫秒整数（或[特殊后缀](../Server_settings/Special_suffixes.md)），指定在复制失败时重试查询远程节点前的延迟。仅当非零值时生效。默认值为 500。

### qcache_max_bytes

<!-- example conf qcache_max_bytes -->
此配置设置用于缓存结果集的最大内存（以字节计）。默认值为 16777216，等同于 16 兆字节。如果值为 0，则禁用查询缓存。有关查询缓存的详细信息，请参阅 [查询缓存](../Searching/Query_cache.md)。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
qcache_max_bytes = 16777216
```
<!-- end -->


### qcache_thresh_msec

整数，单位为毫秒。查询结果被缓存的最小实际耗时阈值。默认值为 3000，即 3 秒。0 表示全部缓存。详见 [查询缓存](../Searching/Query_cache.md)。此值也可以用时间[特殊后缀](../Server_settings/Special_suffixes.md)表示，但请小心使用，不要与本身包含 '_msec' 的名称混淆。


### qcache_ttl_sec

整数，单位为秒。缓存结果集的过期时间。默认值为 60，即 1 分钟。最小值为 1 秒。详见 [查询缓存](../Searching/Query_cache.md)。此值也可以用时间[特殊后缀](../Server_settings/Special_suffixes.md)表示，但请小心使用，不要与本身包含 '_sec' 的名称混淆。


### query_log_format

<!-- example conf query_log_format -->
查询日志格式。可选，允许值有 `plain` 和 `sphinxql`，默认值为 `sphinxql`。

`sphinxql` 模式记录有效的 SQL 语句。`plain` 模式以纯文本格式记录查询（主要适用于纯全文本用例）。该指令允许您在搜索服务器启动时切换两种格式。日志格式也可以动态更改，使用 `SET GLOBAL query_log_format=sphinxql` 语法。详见 [查询日志](../Logging/Query_logging.md)。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_format = sphinxql
```
<!-- end -->

### query_log_min_msec

限制（以毫秒为单位）防止查询被写入查询日志。可选，默认值为0（所有查询都会被写入查询日志）。该指令指定只有执行时间超过指定限制的查询才会被记录（此值也可以用时间[特殊后缀](../Server_settings/Special_suffixes.md)表示，但请谨慎使用，不要被值本身名称中包含的 `_msec` 混淆）。

### query_log

<!-- example conf query_log -->
查询日志文件名。可选，默认为空（不记录查询）。所有搜索查询（例如 SELECT ...，但不包括 INSERT/REPLACE/UPDATE 查询）都会被记录到此文件中。格式在[查询日志](../Logging/Query_logging.md)中有描述。如果格式为 'plain'，可以使用 'syslog' 作为日志文件路径。此时，所有搜索查询将以 `LOG_INFO` 优先级发送到 syslog 守护进程，并以 '[query]' 作为前缀代替时间戳。要使用 syslog 选项，Manticore 必须在构建时通过 `-–with-syslog` 配置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log = /var/log/query.log
```
<!-- end -->


### query_log_mode

<!-- example conf query_log_mode -->
query_log_mode 指令允许您为 searchd 和查询日志文件设置不同的权限。默认情况下，这些日志文件的权限为600，意味着只有运行服务器的用户和 root 用户可以读取日志文件。
如果您想允许其他用户读取日志文件（例如，运行在非 root 用户上的监控解决方案），该指令非常有用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_mode  = 666
```
<!-- end -->

### read_buffer_docs

<!-- example conf read_buffer_docs -->
read_buffer_docs 指令控制文档列表的每关键字读取缓冲区大小。对于每个搜索查询中的每个关键字出现，有两个相关联的读取缓冲区：一个用于文档列表，一个用于命中列表。此设置让您控制文档列表缓冲区大小。

较大的缓冲区大小可能会增加每次查询的 RAM 使用量，但可能会减少 I/O 时间。对于慢速存储，设置较大值是合理的，但对于高 IOPS 存储，应在较低值区域进行试验。

默认值为256K，最小值为8K。您也可以在每个表级别设置 [read_buffer_docs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs)，该设置将覆盖服务器配置级别的设置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_docs = 128K
```
<!-- end -->


### read_buffer_hits

<!-- example conf read_buffer_hits -->
read_buffer_hits 指令指定搜索查询中每关键字命中列表的读取缓冲区大小。默认大小为256K，最小值为8K。对于每个关键字出现，有两个相关联的读取缓冲区：一个用于文档列表，一个用于命中列表。增加缓冲区大小可以增加每次查询的RAM使用，但减少I/O时间。对于慢速存储，设置较大缓冲区大小是合理的，而对于高IOPS存储，应在较低值区域进行试验。

此设置也可以在每个表级别使用 [read_buffer_hits](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_hits) 设置，覆盖服务器级别的设置。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_hits = 128K
```
<!-- end -->

### read_unhinted

<!-- example conf read_unhinted -->
无预知读取大小。可选，默认值为32K，最小值为1K

在查询时，有些读取能够提前确定读取数据的确切大小，但有些无法。最明显的是，命中列表大小目前无法提前知道。此设置允许您控制在这种情况下读取多少数据。它影响命中列表的 I/O 时间，对于大于无预知读取大小的列表来说可以减少I/O时间，但对于较小的列表会增加I/O时间。它**不会**影响RAM使用，因为读取缓冲区已经分配好。因此，它不应大于 read_buffer。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_unhinted = 32K
```
<!-- end -->

### reset_network_timeout_on_packet

<!-- example conf reset_network_timeout_on_packet -->
细化网络超时的行为（例如 `network_timeout`、`read_timeout` 和 `agent_query_timeout`）。

当设置为0时，超时限制整个请求/查询发送的最大时间。
当设置为1（默认）时，超时限制网络活动之间的最大时间。

在复制过程中，一个节点可能需要向另一个节点发送大文件（例如，100GB）。假设网络传输速度为1GB/s，每个数据包大小为4-5MB。传输整个文件需要100秒。默认5秒的超时只允许传输5GB，然后连接就会断开。增加超时可以作为解决办法，但这不可扩展（例如，下一个文件可能是150GB，仍然会失败）。但是默认情况下 `reset_network_timeout_on_packet` 设置为1，超时不是应用于整个传输，而是应用于单个数据包。只要传输正在进行（且在超时期间数据确实通过网络接收），连接就会保持活跃。如果传输卡住，在数据包之间发生超时，则连接会被断开。

注意，如果设置了分布式表，主节点和各代理节点都应进行调整。主节点受 `agent_query_timeout` 影响；代理节点则相关于 `network_timeout`。

<!-- intro -->

##### 示例：

<!-- request Example -->

```ini
reset_network_timeout_on_packet = 0
```

<!-- end -->


### rt_flush_period

<!-- example conf rt_flush_period -->
RT 表RAM块刷新检测周期，单位为秒（或[特殊后缀](../Server_settings/Special_suffixes.md)）。可选，默认是10小时。

积极更新且完全适合RAM块的RT表仍可能导致不断增长的binlog，影响磁盘使用和崩溃恢复时间。通过此指令，搜索服务器执行定期刷新检查，并可以保存符合条件的RAM块，从而实现后续的binlog清理。更多细节请参见[二进制日志](../Logging/Binary_logging.md)。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_flush_period = 3600 # 1 hour
```
<!-- end -->


### rt_merge_iops

<!-- example conf rt_merge_iops -->
RT块合并线程允许启动的最大I/O操作数（每秒）。可选，默认值为0（无上限）。

此指令允许您限制由`OPTIMIZE`语句产生的I/O影响。保证所有RT优化活动不会产生超过配置限制的磁盘IOPS（每秒I/O数）。限制rt_merge_iops可以减少由合并导致的搜索性能下降。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_iops = 40
```
<!-- end -->

### rt_merge_maxiosize

<!-- example conf rt_merge_maxiosize -->
RT块合并线程允许启动的最大I/O操作大小。可选，默认0（无上限）。

此指令允许您限制由`OPTIMIZE`语句产生的I/O影响。大于此限制的I/O将被拆分成两个或多个I/O，并在[rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops)限制中作为多个单独的I/O计算。因此，保证所有优化活动每秒生成的磁盘I/O不超过(rt_merge_iops * rt_merge_maxiosize)字节。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_maxiosize = 1M
```
<!-- end -->


### seamless_rotate

<!-- example conf seamless_rotate -->
防止在旋转包含大量数据的表以进行预缓存时`searchd`卡顿。可选，默认值为1（启用无缝旋转）。在Windows系统中，默认禁用无缝旋转。

表可能包含需要预缓存到RAM的数据。目前，`.spa`、`.spb`、`.spi` 和 `.spm` 文件会被完全预缓存（它们分别包含属性数据、二进制属性数据、关键字表及已杀死的行映射）。未启用无缝旋转时，旋转表尽量少用RAM，过程如下：

1. 临时拒绝新查询（返回“重试”错误码）；
2. `searchd`等待所有当前查询完成；
3. 旧表释放，重命名其文件；
4. 新表文件重命名，并分配所需RAM；
5. 新表的属性和字典数据预加载到RAM；
6. `searchd`恢复从新表服务查询。

然而，如果属性或字典数据很多，预加载步骤可能需要显著时间——在预加载1-5GB以上文件时可达数分钟。

启用无缝旋转时，旋转过程如下：

1. 分配新表的RAM存储；
2. 异步预加载新表的属性和字典数据到RAM；
3. 成功后，释放旧表，重命名两个表的文件；
4. 失败时，释放新表；
5. 任何时刻，查询均从旧表或新表副本服务。

无缝旋转带来旋转时峰值内存使用增加（因预加载新副本时旧副本和新副本的 `.spa/.spb/.spi/.spm` 数据均需在RAM），但平均使用量相同。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
seamless_rotate = 1
```
<!-- end -->

### secondary_indexes
<!-- example conf secondary_indexes -->

此选项启用/禁用搜索查询使用辅助索引。可选，默认启用（1）。注意，索引建立时无需启用，只要安装了[Manticore Columnar Library](https://github.com/manticoresoftware/columnar) 即始终启用。使用辅助索引进行搜索时也需此库。支持三种模式：

* `0`：禁用搜索使用辅助索引。可通过[分析器提示](../Searching/Options.md#Query-optimizer-hints)为单个查询启用。
* `1`：启用搜索使用辅助索引。可通过[分析器提示](../Searching/Options.md#Query-optimizer-hints)为单个查询禁用。
* `force`：同启用，但加载辅助索引时任何错误都会报告，且整个索引不会被加载到守护进程。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
secondary_indexes = 1
```

<!-- end -->

### server_id

<!-- example conf server_id -->
作为服务器标识符的整数，用作生成复制集群中节点唯一短UUID的种子。server_id在集群节点中必须唯一，且范围为0至127。若未设置server_id，则通过MAC地址与PID文件路径哈希计算，或使用随机数作为短UUID种子。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
server_id = 1
```
<!-- end -->


### shutdown_timeout

<!-- example conf shutdown_timeout -->
`searchd --stopwait` 的等待时间，单位为秒（或[特殊后缀](../Server_settings/Special_suffixes.md)）。可选，默认60秒。

运行`searchd --stopwait`时，服务器需进行一些操作后停止，如完成查询、刷新RT RAM块、刷新属性、更新binlog等。这些任务需时间。`searchd --stopwait`将最多等待`shutdown_timeout`秒，直到服务器完成任务。合适时间取决于表大小和负载。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
shutdown_timeout = 3m # wait for up to 3 minutes
```
<!-- end -->


### shutdown_token

SHA1哈希值，用于从VIP Manticore SQL连接调用'shutdown'命令。没有它，[debug](../Reporting_bugs.md#DEBUG) 'shutdown'子命令将永远不会导致服务器停止。请注意，这种简单的哈希不应被视为强保护，因为我们没有使用加盐哈希或任何现代哈希函数。它旨在作为本地网络中的维护守护进程的万无一失的措施。

### snippets_file_prefix

<!-- example conf snippets_file_prefix -->
生成片段时要添加到本地文件名的前缀。可选，默认是当前工作目录。

此前缀可与`load_files`或`load_files_scattered`选项一起用于分布式片段生成。

请注意，这只是一个前缀，而**不是**路径！这意味着如果设置的前缀为"server1"，而请求引用"file23"，`searchd`将尝试打开"server1file23"（全部不含引号）。因此，如果你需要它是一个路径，必须包括尾部斜杠。

构造最终文件路径后，服务器会展开所有相对目录并与`snippet_file_prefix`的值进行比较。如果结果不以该前缀开头，则此类文件将被拒绝并显示错误信息。

例如，如果设置为`/mnt/data`，而有人调用片段生成且源文件为`../../../etc/passwd`，他们会收到错误信息：

`File '/mnt/data/../../../etc/passwd' escapes '/mnt/data/' scope`

而不是文件内容。

另外，如果该参数未设置且读取`/etc/passwd`，实际上会读取/daemon/working/folder/etc/passwd，因为该参数默认是服务器的工作目录。

还请注意这是一个本地选项；它不会以任何方式影响代理。因此，你可以安全地在主服务器上设置前缀。路由到代理的请求不会受主服务器设置影响。但会受到代理自身设置的影响。

例如，当文档存储位置（无论是本地存储还是NAS挂载点）在服务器间不一致时，这会很有用。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
snippets_file_prefix = /mnt/common/server1/
```
<!-- end -->

> **警告：** 如果你仍想访问文件系统根目录的文件，必须显式将`snippets_file_prefix`设置为空值（使用`snippets_file_prefix=`行），或者设置为根（使用`snippets_file_prefix=/`）。


### sphinxql_state

<!-- example conf sphinxql_state -->
当前SQL状态序列化保存的文件路径。

服务器启动时会重放此文件。在合适的状态更改时（例如SET GLOBAL），该文件会自动重写。这可以防止难以诊断的问题：如果加载了UDF函数但Manticore崩溃，当它（自动）重启时，你的UDF和全局变量将不再可用。使用持久状态有助于确保优雅恢复，没有这种意外。

`sphinxql_state`不能用于执行任意命令，例如`CREATE TABLE`。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
sphinxql_state = uservars.sql
```
<!-- end -->


### sphinxql_timeout

<!-- example conf sphinxql_timeout -->
使用SQL接口时，两个请求之间最大等待时间（秒，或[特殊后缀](../Server_settings/Special_suffixes.md)）。可选，默认是15分钟。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
sphinxql_timeout = 15m
```
<!-- end -->


### ssl_ca

<!-- example conf ssl_ca -->
SSL证书颁发机构(CA)证书文件路径（也称根证书）。可选，默认为空。不为空时，`ssl_cert`中的证书应由此根证书签发。

服务器使用CA文件验证证书上的签名。该文件必须是PEM格式。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ssl_ca = keys/ca-cert.pem
```
<!-- end -->


### ssl_cert

<!-- example conf ssl_cert -->
服务器的SSL证书路径。可选，默认为空。

服务器使用此证书作为自签名公钥来加密SSL上的HTTP流量。文件必须是PEM格式。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ssl_cert = keys/server-cert.pem
```
<!-- end -->


### ssl_key

<!-- example conf ssl_key -->
SSL证书密钥路径。可选，默认为空。

服务器使用此私钥加密SSL上的HTTP流量。文件必须是PEM格式。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ssl_key = keys/server-key.pem
```
<!-- end -->


### subtree_docs_cache

<!-- example conf subtree_docs_cache -->
每个查询的最大公共子树文档缓存大小。可选，默认是0（禁用）。

此设置限制公共子树优化器的内存使用(见 [多查询](../Searching/Multi-queries.md))。最多使用这么多内存缓存每个查询的文档条目。设置为0禁用该优化器。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
subtree_docs_cache = 8M
```
<!-- end -->


### subtree_hits_cache

<!-- example conf subtree_hits_cache -->
每个查询的最大公共子树命中缓存大小。可选，默认是0（禁用）。

此设置限制公共子树优化器的内存使用(见 [多查询](../Searching/Multi-queries.md))。最多使用这么多内存缓存每个查询的关键词出现（命中）。设置为0禁用该优化器。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
subtree_hits_cache = 16M
```
<!-- end -->

### threads

<!-- example threads -->
Manticore守护进程的工作线程数（或线程池大小）。Manticore启动时创建这些数量的操作系统线程，它们负责守护进程内所有任务，如执行查询，生成片段等。有些操作可能被拆分为子任务并并行执行，例如：

* 在实时表中搜索
* 在由本地表组成的分布式表中搜索
* 查询感知调用
* 以及其他

默认情况下，它设置为服务器上的 CPU 内核数量。Manticore 在启动时创建线程，并保持这些线程直到停止。每个子任务在需要时可以使用线程。子任务完成后释放线程，其他子任务可以使用它。

在密集的 I/O 类型负载情况下，设置的值高于 CPU 内核数可能更合理。

<!-- request Example -->
```ini
threads = 10
```

<!-- end -->

### thread_stack

<!-- example conf thread_stack -->
作业的最大栈大小（协程，一个搜索查询可能导致多个作业/协程）。可选，默认值为 128K。

每个作业都有自己的 128K 栈。当您运行查询时，会检查其需要多少栈空间。如果默认的 128K 足够，查询将被处理。如果需要更多，则调度一个具有更大栈的另一个作业继续处理。此高级栈的最大大小受此设置限制。

将该值设置为合理的大值将有助于处理非常深的查询，同时不意味着整体 RAM 消耗会过高。例如，设置为 1G 并不意味着每个新作业都会占用 1G RAM，但如果我们看到它需要 例如 100M 栈，我们就为该作业分配 100M 。同时其他作业将使用默认的 128K 栈。我们也可以运行需要 500M 的更复杂查询，只有当内部检测到作业需要超过 1G 栈时，才会失败并报告 thread_stack 过低。

然而，实际中，即使需要 16M 栈的查询通常也过于复杂，难以解析并消耗过多时间和资源。因此，守护进程会处理它，但通过 `thread_stack` 设置限制此类查询是合理的。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
thread_stack = 8M
```
<!-- end -->


### unlink_old

<!-- example conf unlink_old -->
确定在成功轮换时是否解除链接 `.old` 表副本。可选，默认值为 1（执行解除链接）。


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

当 Manticore 查询崩溃时，可能导致整个服务器崩溃。启用看门狗功能后，`searchd` 会维护一个单独的轻量级进程，监控主服务器进程并在异常终止时自动重启。看门狗默认启用。

<!-- request Example -->

```ini
watchdog = 0 # disable watchdog
```
<!-- end -->
<!-- proofread -->

