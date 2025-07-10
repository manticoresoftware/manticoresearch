# 配置中的 "Searchd" 部分

以下设置用于 Manticore Search 配置文件中的 `searchd` 部分，以控制服务器的行为。下面是每个设置的总结：

### access_plain_attrs

此设置为 [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。它是可选的，默认值为 `mmap_preread`。

`access_plain_attrs` 指令允许您为此 searchd 实例管理的所有表定义 [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每表指令具有更高优先级，会覆盖此实例的默认值，从而提供更细粒度的控制。

### access_blob_attrs

此设置为 [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。它是可选的，默认值为 `mmap_preread`。

`access_blob_attrs` 指令允许您为此 searchd 实例管理的所有表定义 [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每表指令具有更高优先级，会覆盖此实例的默认值，从而提供更细粒度的控制。

### access_doclists

此设置为 [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。它是可选的，默认值为 `file`。

`access_doclists` 指令允许您为此 searchd 实例管理的所有表定义 [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每表指令具有更高优先级，会覆盖此实例的默认值，从而提供更细粒度的控制。

### access_hitlists

此设置为 [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。它是可选的，默认值为 `file`。

`access_hitlists` 指令允许您为此 searchd 实例管理的所有表定义 [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每表指令具有更高优先级，会覆盖此实例的默认值，从而提供更细粒度的控制。

### access_dict

此设置为 [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。它是可选的，默认值为 `mmap_preread`。

`access_dict` 指令允许您为此 searchd 实例管理的所有表定义 [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每表指令具有更高优先级，会覆盖此实例的默认值，从而提供更细粒度的控制。

### agent_connect_timeout

此设置为 [agent_connect_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) 参数设置实例范围的默认值。


### agent_query_timeout

此设置为 [agent_query_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) 参数设置实例范围的默认值。可以通过使用 `OPTION agent_query_timeout=XXX` 条款在每个查询中覆盖此设置。


### agent_retry_count

此设置为整数，指定 Manticore 在报告致命查询错误之前尝试通过分布式表连接和查询远程代理的次数。默认值为 0（即不重试）。您也可以通过使用 `OPTION retry_count=XXX` 条款在每个查询中设置此值。如果提供了每查询选项，它将覆盖配置中指定的值。

请注意，如果您在分布式表的定义中使用了 [agent mirrors](../Creating_a_cluster/Remote_nodes/Mirroring.md#Agent-mirrors)，服务器将根据所选的 [ha_strategy](../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) 为每次连接尝试选择不同的镜像。在这种情况下，`agent_retry_count` 会针对一组所有镜像进行累计。

例如，如果您有 10 个镜像，并设置 `agent_retry_count=5`，服务器将最多重试 50 次，假设每个镜像平均尝试 5 次（在使用 `ha_strategy = roundrobin` 选项时，如此）。

但是，作为 [agent](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) 的 `retry_count` 选项所提供的值是绝对限制。换句话说，代理定义中的 `[retry_count=2]` 选项始终意味着最多尝试 2 次，无论您为代理指定了 1 个还是 10 个镜像。

### agent_retry_delay

此设置为整数，单位为毫秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)），指定在查询远程代理失败时，Manticore 重试前的等待延迟。仅当指定非零的 [agent_retry_count](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 或非零的每查询 `retry_count` 时，此值才有意义。默认值为 500。您也可以通过使用 `OPTION retry_delay=XXX` 条款在每个查询中设置此值。如果提供了每查询选项，它将覆盖配置中指定的值。


### attr_flush_period

<!-- example conf attr_flush_period -->
当使用 [Update](../Data_creation_and_modification/Updating_documents/UPDATE.md) 实时修改文档属性时，更改首先写入属性的内存副本。这些更新发生在内存映射文件中，这意味着操作系统决定何时将更改写入磁盘。在 `searchd` 正常关闭时（由 `SIGTERM` 信号触发），所有更改都会被强制写入磁盘。

你还可以指示 `searchd` 定期将这些更改写回磁盘，以防止数据丢失。刷新间隔由 `attr_flush_period` 决定，单位为秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。

默认情况下，该值为 0，表示禁用定期刷新。但是，刷新仍将在正常关闭时发生。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
attr_flush_period = 900 # persist updates to disk every 15 minutes
```
<!-- end -->

### auto_optimize

<!-- example conf auto_optimize -->
此设置控制表压缩的自动 [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 进程。

默认情况下，表压缩会自动进行。你可以通过 `auto_optimize` 设置修改此行为：
* 0 禁用自动表压缩（你仍然可以手动调用 `OPTIMIZE`）
* 1 明确启用它
* 启用它同时将优化阈值乘以 2。

默认情况下，OPTIMIZE 会运行直到磁盘块数量少于或等于逻辑 CPU 内核数乘以 2。

但是，如果表具有带 KNN 索引的属性，则该阈值不同。在这种情况下，它被设置为物理 CPU 内核数除以 2，以提高 KNN 搜索性能。

请注意，切换 `auto_optimize` 的开启或关闭并不会阻止你手动运行 [OPTIMIZE TABLE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE)。

<!-- intro -->
##### 示例：

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
Manticore 支持自动创建尚不存在但在 INSERT 语句中指定的表。此功能默认启用。要禁用它，请在配置中显式设置 `auto_schema = 0`。要重新启用，设置 `auto_schema = 1` 或从配置中删除 `auto_schema` 设置。

请记住，`/bulk` HTTP 端点不支持自动创建表。

> 注意：[自动模式功能](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果不工作，请确保 Buddy 已安装。

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
此设置控制二进制日志事务的刷新/同步模式。它是可选的，默认值为 2（每个事务刷新，每秒同步一次）。

该指令决定二进制日志刷新到操作系统和同步到磁盘的频率。支持三种模式：

*  0，每秒刷新并同步一次。这提供最佳性能，但在服务器崩溃或操作系统/硬件崩溃时，最多可能丢失 1 秒内已提交的事务。
*  1，每个事务刷新并同步一次。此模式性能最差，但保证每个已提交事务的数据都被保存。
*  2，每个事务刷新，每秒同步一次。此模式提供良好性能，并保证在服务器崩溃时每个已提交的事务都被保存。但在操作系统/硬件崩溃时，最多可能丢失 1 秒内已提交的事务。

对于熟悉 MySQL 和 InnoDB 的用户，此指令类似于 `innodb_flush_log_at_trx_commit`。在大多数情况下，默认的混合模式 2 实现了速度和安全的良好平衡，能够完全保护 RT 表数据免受服务器崩溃的影响，并对硬件崩溃提供一定的保护。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
binlog_flush = 1 # ultimate safety, low speed
```
<!-- end -->

### binlog_common

<!-- example conf binlog_common -->
此设置控制二进制日志文件的管理方式。它是可选的，默认值为 0（每个表单独文件）。

你可以选择两种管理二进制日志文件的方式：

* 每个表一个单独文件（默认，`0`）：每个表将其更改保存到自己的日志文件中。如果你有许多在不同时间更新的表，这种设置很好。它允许表独立更新，无需等待其他表。同时，如果一个表的日志文件出现问题，不会影响其他表。
* 所有表使用同一个文件（`1`）：所有表共享同一个二进制日志文件。此方式便于文件管理，因为文件数量较少。但如果一个表仍需保存更新，文件可能会被保留时间较长。此外，如果许多表需要同时更新，所有更改都必须等待写入同一个文件，可能导致性能下降。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
binlog_common = 1 # use a single binary log file for all tables
```
<!-- end -->

### binlog_max_log_size

<!-- example conf binlog_max_log_size -->
此设置控制最大二进制日志文件大小。它是可选的，默认值为 256 MB。

当当前二进制日志文件达到此大小限制时，会强制打开一个新的日志文件。这使日志更细粒度，在某些边界工作负载下可以实现更高效的二进制日志磁盘使用。值为 0 表示不基于大小重新打开日志文件。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
binlog_max_log_size = 16M
```
<!-- end -->


### binlog_path

<!-- example conf binlog_path -->
此设置确定二进制日志（也称为事务日志）文件的路径。它是可选的，默认值为构建时配置的数据目录（例如，Linux 下的 `/var/lib/manticore/data/binlog.*`）。

二进制日志用于 RT 表数据的崩溃恢复以及平面磁盘索引的属性更新，否则这些属性更新只会存储在 RAM 中，直到刷新。启用日志记录时，每个提交到 RT 表的事务都会被写入日志文件。日志将在非正常关闭后启动时自动重放，恢复已记录的更改。

`binlog_path` 指令指定二进制日志文件的位置。它应仅包含路径；`searchd` 将根据需要在该目录中创建和解绑多个 `binlog.*` 文件（包括二进制日志数据、元数据和锁文件等）。

空值会禁用二进制日志记录，这会提升性能，但会使 RT 表数据处于风险之中。


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
此设置确定 Manticore Buddy 二进制文件的路径。此项为可选，默认值为构建时配置的路径，该路径因操作系统不同而异。通常，你无需修改此设置。但如果你希望在调试模式下运行 Manticore Buddy、修改 Manticore Buddy 或实现新插件时，此设置可能有用。在后一种情况下，你可以从 https://github.com/manticoresoftware/manticoresearch-buddy `git clone` Buddy，向目录 `./plugins/` 添加新插件，并在切换到 Buddy 源代码目录后运行 `composer install --prefer-source` 以便于开发。

为了确保可以运行 `composer`，你的机器必须安装 PHP 8.2 或更高版本，并包含以下扩展：

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

你也可以选择发布中可用的适用于 Linux amd64 的特殊版本 `manticore-executor-dev`，例如：https://github.com/manticoresoftware/executor/releases/tag/v1.0.13

如果走这条路，记得将开发版的 manticore 执行器链接到 `/usr/bin/php`。

要禁用 Manticore Buddy，请将值设置为空，如示例所示。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
buddy_path = manticore-executor -n /usr/share/manticore/modules/manticore-buddy/src/main.php # use the default Manticore Buddy in Linux
buddy_path = manticore-executor -n /opt/homebrew/share/manticore/modules/manticore-buddy/bin/manticore-buddy/src/main.php # use the default Manticore Buddy in MacOS arm64
buddy_path = manticore-executor -n /Users/username/manticoresearch-buddy/src/main.php # use Manticore Buddy from a non-default location
buddy_path = # disables Manticore Buddy
buddy_path = manticore-executor -n /Users/username/manticoresearch-buddy/src/main.php --skip=manticoresoftware/buddy-plugin-replace # --skip - skips plugins
```
<!-- end -->

### client_timeout

<!-- example conf client_timeout -->
此设置确定使用持久连接时请求之间等待的最大时间（单位为秒或 [special_suffixes](../Server_settings/Special_suffixes.md)）。此项为可选，默认值为五分钟。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
client_timeout = 1h
```
<!-- end -->


### collation_libc_locale

<!-- example conf collation_libc_locale -->
服务器 libc 地区设置。可选，默认是 C。

指定 libc 地区，影响基于 libc 的排序规则。详情请参阅 [排序规则](../Searching/Collations.md) 部分。


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

指定传入请求使用的默认排序规则。排序规则可以在每个查询中覆盖。请参阅 [排序规则](../Searching/Collations.md) 部分以获取可用排序规则列表及其他详细信息。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
collation_server = utf8_ci
```
<!-- end -->


### data_dir

<!-- example conf data_dir -->
指定该项时，将启用 [实时模式](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)，这是一种管理数据模式的命令式方式。该值应为存储所有表、二进制日志及 Manticore Search 在此模式下正常运行所需所有内容的目录路径。
指定 `data_dir` 时，不允许对 [平面表](../Creating_a_table/Local_tables/Plain_table.md) 进行索引。关于实时模式与平面模式的区别，请参阅 [本节](../Read_this_first.md#Real-time-table-vs-plain-table)。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
data_dir = /var/lib/manticore
```
<!-- end -->

### diskchunk_flush_search_timeout

<!-- example conf diskchunk_flush_search_timeout -->
当表中无搜索时，防止自动刷新 RAM 数据块的超时时间。可选，默认值为 30 秒。

检查搜索的时间，以确定是否自动刷新。
仅当在过去 `diskchunk_flush_search_timeout` 秒内表中至少发生过一次搜索时，才会触发自动刷新。该设置与 [diskchunk_flush_write_timeout](../Server_settings/Searchd.md#diskchunk_flush_write_timeout) 配合使用。对应的 [单表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_search_timeout) 优先级更高，会覆盖此实例级默认值，实现更细粒度的控制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
diskchunk_flush_search_timeout = 120s
```
<!-- end -->

### diskchunk_flush_write_timeout

<!-- example conf diskchunk_flush_write_timeout -->
等待写入超时时间（秒），在无写操作的情况下自动刷新 RAM 数据块到磁盘。可选，默认值为 1 秒。

如果在 `diskchunk_flush_write_timeout` 秒内 RAM 数据块无写入，将会刷新该数据块到磁盘。该设置与 [diskchunk_flush_search_timeout](../Server_settings/Searchd.md#diskchunk_flush_search_timeout) 配合使用。要禁用自动刷新，请在配置中显式设置 `diskchunk_flush_write_timeout = -1`。对应的 [单表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_write_timeout) 优先级更高，会覆盖此实例级默认值，实现更细粒度的控制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
diskchunk_flush_write_timeout = 60s
```
<!-- end -->

### docstore_cache_size

<!-- example conf docstore_cache_size -->
此设置指定文档存储中的文档块在内存中缓存的最大大小。该设置为可选，默认值为 16m（16 兆字节）。

当使用 `stored_fields` 时，文档块将从磁盘读取并解压缩。由于每个块通常包含多个文档，因此在处理下一个文档时可以重复使用它。为此，块会保存在服务器范围的缓存中。缓存保存的是未压缩的块。


<!-- intro -->
##### 示例:

<!-- request Example -->

```ini
docstore_cache_size = 8m
```
<!-- end -->

### engine

<!-- example conf engine -->
创建 RT 模式表时默认使用的属性存储引擎。可以是 `rowwise`（默认）或 `columnar`。

<!-- intro -->
##### 示例:

<!-- request Example -->

```ini
engine = columnar
```
<!-- end -->


### expansion_limit

<!-- example conf expansion_limit -->
此设置决定单个通配符展开的最大关键字数。可选，默认值为 0（无限制）。

当对启用了 `dict = keywords` 的表执行子串搜索时，单个通配符可能导致数千甚至数百万个匹配关键字（例如，将 `a*` 与整个牛津词典匹配）。该指令允许您限制此类展开的影响。设置 `expansion_limit = N` 将扩展限制为每个查询中的每个通配符最多 N 个最频繁匹配的关键字。

<!-- intro -->
##### 示例:

<!-- request Example -->

```ini
expansion_limit = 16
```
<!-- end -->

### expansion_merge_threshold_docs

<!-- example conf expansion_merge_threshold_docs -->
此设置决定允许合并所有此类关键字的展开关键字中的最大文档数。可选，默认值为 32。

当对启用了 `dict = keywords` 的表执行子串搜索时，单个通配符可能导致数千甚至数百万个匹配关键字。该指令允许您增加合并关键字的限制，以加快匹配速度，但会在搜索时使用更多内存。

<!-- intro -->
##### 示例:

<!-- request Example -->

```ini
expansion_merge_threshold_docs = 1024
```
<!-- end -->

### expansion_merge_threshold_hits

<!-- example conf expansion_merge_threshold_hits -->
此设置决定允许合并所有此类关键字的展开关键字中的最大匹配数量。可选，默认值为 256。

当对启用了 `dict = keywords` 的表执行子串搜索时，单个通配符可能导致数千甚至数百万个匹配关键字。该指令允许您增加合并关键字的限制，以加快匹配速度，但会在搜索时使用更多内存。

<!-- intro -->
##### 示例:

<!-- request Example -->

```ini
expansion_merge_threshold_hits = 512
```
<!-- end -->

### grouping_in_utc

此设置指定 API 和 SQL 中的时间分组是按本地时区还是 UTC 计算。可选，默认值为 0（表示“本地时区”）。

默认情况下，所有“按时间分组”表达式（如 API 中的按日、周、月、年分组，以及 SQL 中的按日、月、年、年月、年月日分组）均使用本地时间。例如，如果有属性时间分别为 `13:00 utc` 和 `15:00 utc` 的文档，分组时它们都会根据本地时区设置划分到相应的分组。如果您位于 `utc`，这将是同一天，但如果您位于 `utc+10`，这些文档将匹配到不同的“按日分组”分组（因为 13:00 utc 在 UTC+10 时区是本地时间 23:00，而 15:00 是次日 01:00）。有时这种行为不可接受，希望使时间分组不依赖时区。您可以运行服务器时定义全局的 TZ 环境变量，但它不仅影响分组，还会影响日志中的时间戳，这可能也是不希望的。启用此选项（可在配置中或使用 SQL 中的 [SET global](../Server_settings/Setting_variables_online.md#SET) 语句）后，所有时间分组表达式将以 UTC 计算，而服务器的其他与时间相关的功能（如日志记录）仍使用本地时区。


### timezone

此设置指定日期/时间相关函数使用的时区。默认为本地时区，但您可以指定 IANA 格式的其他时区（例如，`Europe/Amsterdam`）。

请注意，此设置不影响日志记录，日志始终使用本地时区。

另外，如果使用了 `grouping_in_utc`，则“按时间分组”函数仍使用 UTC，而其他日期/时间相关函数将使用指定的时区。总体而言，不推荐同时混用 `grouping_in_utc` 和 `timezone`。

您可以在配置中设置此选项，或通过 SQL 中的 [SET global](../Server_settings/Setting_variables_online.md#SET) 语句配置。


### ha_period_karma

<!-- example conf ha_period_karma -->
此设置指定代理镜像统计窗口大小，单位为秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认值为 60 秒。

对于包含代理镜像的分布式表（详见 [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)），主节点跟踪多个每镜像计数器。这些计数器用于故障转移和平衡（主节点根据计数器选择最佳镜像）。计数器按 `ha_period_karma` 秒为块进行累积。

开始新块后，主节点仍可能使用前一个块的累计值，直到新块填满一半。因此，任何之前的历史最多在 1.5 倍的 ha_period_karma 秒后不再影响镜像选择。

尽管最多使用两个块进行镜像选择，但为监控目的会存储最多最近的 15 个块。这些块可通过 [SHOW AGENT STATUS](../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS) 语句查看。


<!-- intro -->
##### 示例:

<!-- request Example -->

```ini
ha_period_karma = 2m
```
<!-- end -->


### ha_ping_interval

<!-- example conf ha_ping_interval -->
此设置配置代理镜像 ping 的间隔，单位为毫秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。该设置为可选，默认值为 1000 毫秒。

对于包含代理镜像的分布式表（更多内容见 [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)），主节点会在空闲期间向所有镜像发送 ping 命令。目的是追踪当前代理状态（存活或死亡，网络往返时间等）。此类 ping 之间的间隔由该指令定义。要禁用 ping，请将 ha_ping_interval 设置为 0。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_ping_interval = 3s
```
<!-- end -->


### hostname_lookup

`hostname_lookup` 选项定义更新主机名的策略。默认情况下，服务器启动时会缓存代理主机名的 IP 地址，以避免频繁访问 DNS。但是，在某些情况下，IP 可能会动态更改（例如云主机），此时可能希望不缓存 IP。将此选项设置为 `request` 将禁用缓存，并在每次查询时访问 DNS。IP 地址也可以通过 `FLUSH HOSTNAMES` 命令手动更新。

### jobs_queue_size

jobs_queue_size 设置定义队列中可同时存在的“作业”数。默认情况下无限制。

在大多数情况下，一个“作业”意味着对单个本地表（普通表或实时表的磁盘分块）的一次查询。例如，如果你有一个包含 2 个本地表的分布式表，或一个有 2 个磁盘分块的实时表，对它们的搜索查询通常会在队列中放入 2 个作业。然后，由线程池（其大小由 [threads](../Server_settings/Searchd.md#threads) 定义）处理它们。但是，若查询过于复杂，可能会创建更多作业。当 [max_connections](../Server_settings/Searchd.md#max_connections) 和 [threads](../Server_settings/Searchd.md#threads) 不足以平衡性能需求时，建议调整此设置。

### join_batch_size

表连接工作方式是积累一批匹配项，这些匹配项是左表执行查询的结果。然后将该批次作为单个查询在右表上处理。

此选项允许调整批次大小。默认值为 `1000`，设置为 `0` 则禁用批处理。

较大的批次可能提升性能；然而，对于某些查询，这可能导致过高的内存消耗。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
join_batch_size = 2000
```
<!-- end -->

### join_cache_size

右表执行的每个查询由特定的 JOIN ON 条件定义，这些条件决定了从右表检索的结果集。

如果只有少数唯一的 JOIN ON 条件，重复使用结果比一遍遍在右表执行查询更有效。启用此功能会将结果集存储在缓存中。

此选项允许配置此缓存的大小。默认值为 `20 MB`，设置为 0 关闭缓存。

注意每个线程维护自己的缓存，因此在估计总内存使用时应考虑执行查询的线程数。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
join_cache_size = 10M
```
<!-- end -->

### listen_backlog

<!-- example conf listen_backlog -->
listen_backlog 设置决定传入连接的 TCP 监听队列长度。该设置在 Windows 版本中尤为重要，因其逐一处理请求。当连接队列达到限制时，新传入连接将被拒绝。
对于非 Windows 版本，默认值通常适用，通常无需调整此设置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
listen_backlog = 20
```
<!-- end -->

### kibana_version_string

<!-- example conf kibana_version_string -->
返回给 Kibana 或 OpenSearch Dashboards 的服务器版本字符串。可选——默认设置为 `7.6.0`。

部分版本的 Kibana 和 OpenSearch Dashboards 期望服务器报告特定版本号，并可能据此表现不同。为解决此类问题，可以使用此设置，让 Manticore 向 Kibana 或 OpenSearch Dashboards 报告自定义版本。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
kibana_version_string = 1.2.3
```
<!-- end -->

### listen

<!-- example conf listen -->
此设置允许指定 Manticore 接受连接时所用的 IP 地址和端口，或 Unix 域套接字路径。

`listen` 的一般语法为：

```ini
listen = ( address ":" port | port | path | address ":" port start - port end ) [ ":" protocol [ "_vip" ] [ "_readonly" ] ]
```

你可以指定：
* IP 地址（或主机名）和端口号
* 仅端口号
* Unix 套接字路径（Windows 不支持）
* IP 地址和端口范围

若指定端口号而未指定地址，`searchd` 会监听所有网络接口。Unix 路径以斜杠开头。端口范围只能为复制协议设置。

你还可以为此套接字上的连接指定协议处理器（监听器）。监听器包括：

* **未指定** — Manticore 会从以下来源在该端口接受连接：
  - 其他 Manticore 代理（即远程分布式表）
  - 通过 HTTP 和 HTTPS 的客户端
  - [Manticore Buddy](https://manticoresearch.com/blog/manticoresearch-buddy-intro/)。**确保你有此类监听器（或如下面提到的 `http` 监听器），以避免 Manticore 功能受限。**
* `mysql` MySQL 协议，供 MySQL 客户端连接。注意：
  - 也支持压缩协议。
  - 若启用 [SSL](../Security/SSL.md#SSL)，则支持加密连接。
* `replication` - 用于节点通信的复制协议。更多细节请参见[复制](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md)章节。您可以指定多个复制监听器，但它们必须全部监听相同的IP；端口可以不同。当您使用端口范围定义复制监听器时（例如，`listen = 192.168.0.1:9320-9328:replication`），Manticore不会立即开始监听这些端口。相反，只有在您开始使用复制时，才会从指定范围中随机选择空闲端口。复制正常工作至少需要2个端口范围。
* `http` - 与**未指定**相同。Manticore 将通过 HTTP 和 HTTPS 接受来自远程代理和客户端的连接。
* `https` - HTTPS 协议。Manticore 仅在此端口接受 HTTPS 连接。详情请参阅[SSL](../Security/SSL.md)章节。
* `sphinx` - 传统二进制协议。用于服务来自远程[SphinxSE](../Extensions/SphinxSE.md)客户端的连接。一些 Sphinx API 客户端实现（例如 Java 客户端）需要显式声明监听器。

在客户端协议名称后添加后缀 `_vip`（即除`replication`外的所有协议，例如 `mysql_vip`、`http_vip` 或仅 `_vip`）会强制为该连接创建专用线程，以绕过不同的限制。当服务器严重过载导致停滞或无法通过常规端口连接时，这对于节点维护非常有用。

后缀 `_readonly` 为监听器设置[只读模式](../Security/Read_only.md)，仅允许接收读查询。

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

可以有多个 `listen` 指令。`searchd` 将在所有指定的端口和套接字上监听客户端连接。Manticore 包中提供的默认配置定义了以下监听端口：
* `9308` 和 `9312`，接受来自远程代理和非基于 MySQL 的客户端的连接
* 端口 `9306`，用于 MySQL 连接。

如果配置中完全没有指定 `listen`，Manticore 将在以下地址等待连接：
* 对于 MySQL 客户端，`127.0.0.1:9306`
* 对于 HTTP/HTTPS 及来自其他 Manticore 节点和基于 Manticore 二进制 API 的客户端连接，`127.0.0.1:9312`

#### 监听特权端口

默认情况下，Linux 不允许 Manticore 监听 1024 以下的端口（例如 `listen = 127.0.0.1:80:http` 或 `listen = 127.0.0.1:443:https`），除非以 root 用户身份运行 searchd。如果您仍希望让 Manticore 在非 root 用户下监听小于 1024 的端口，可以考虑以下任一方法（均可生效）：
* 运行命令 `setcap CAP_NET_BIND_SERVICE=+eip /usr/bin/searchd`
* 在 Manticore 的 systemd 单元文件中添加 `AmbientCapabilities=CAP_NET_BIND_SERVICE`，然后重新加载守护进程（`systemctl daemon-reload`）。

#### 关于 Sphinx API 协议和 TFO 的技术细节
<details>
传统 Sphinx 协议有两个阶段：握手交换和数据传输。握手阶段由客户端发送的4字节包和守护进程发送的4字节包组成，其唯一目的在于客户端确认远程为真实的 Sphinx 守护进程，守护进程确认远程为真实的 Sphinx 客户端。主要数据流很简单：双方声明各自的握手信息，对方进行验证。该短包交换需要使用特殊的 `TCP_NODELAY` 标志，该标志关闭了 Nagle TCP 算法，表示 TCP 连接将作为多个小包的对话进行。
然而，握手中谁先发言没有明确规定。历史上，所有使用二进制 API 的客户端都会先发言：先发送握手包，接着读取守护进程返回的4字节数据，然后发送请求，再读取守护进程的响应。
在改进 Sphinx 协议兼容性时，我们考虑了以下几点：

1. 通常，主代理之间的通信是从已知客户端到已知主机和端口，因此不太可能出现错误的握手。所以我们可以隐式假定双方都有效且真实使用 Sphinx 协议。
2. 基于这个假设，我们可以将握手包“黏合”到真实请求中，一并发送。如果后端是传统的 Sphinx 守护进程，它会先读取这4字节握手，再读取请求体。由于两者在同一个包中，后端套接字减少一次往返延迟(RTT)，且前端缓冲仍能正常处理。
3. 继续该假设，鉴于“查询”包较小且握手更小，我们使用现代 TFO（tcp-fast-open）技术，在初始 TCP 同步包（SYN）中同时发送握手和请求体。守护进程接受连接时，套接字缓冲区已经包含了握手包和请求体，这样进一步消除了一次 RTT。
4. 最后，需要教守护进程接受这一改进。实际上，应用层不再使用 `TCP_NODELAY`。系统层面则确保服务器开启 TFO，客户端也开启 TFO。现代系统默认启用客户端 TFO，因此只需在服务器端启用即可正常工作。

这些改进没有改变协议本身，却使连接的 TCP 往返时间减少了 1.5 次。如果查询和应答都可以放入单个 TCP 包，则二进制 API 会话从 3.5 RTT 降到 2 RTT，使网络协商速度提升约两倍。

所以，我们所有的改进都是围绕最初未定义的声明：‘谁先发言’。如果客户端先发言，我们可以应用所有这些优化，并有效地在单个 TFO 包中处理连接 + 握手 + 查询。此外，我们可以查看接收包的开头并确定真实协议。这就是为什么你可以通过 API/http/https 连接到同一个端口的原因。如果守护进程必须先发言，所有这些优化都是不可能的，多协议也不可能。这就是我们为 MySQL 专门设置了一个端口，并没有将其与所有其他协议统一到同一个端口的原因。突然之间，在所有客户端中，有一个被写成暗示守护进程应该先发送握手的客户端。那就是所有上述改进都无法实现的 SphinxSE 插件，用于 mysql/mariadb。因此，专门针对这个单一客户端，我们专门为 `sphinx` 协议定义了以最传统的方式工作。也就是说：双方都启用 `TCP_NODELAY`，并用小包交换。守护进程在连接时发送握手，然后客户端发送其握手，然后一切照常工作。这不是很优化，但能工作。如果你使用 SphinxSE 连接到 Manticore——你必须专门为此设置一个明确声明了 `sphinx` 协议的监听器。对于其他客户端——避免使用此监听器，因为它更慢。如果你使用其他传统的 Sphinx API 客户端——首先检查它们是否能使用非专用的多协议端口工作。对于主-代理连接，使用非专用（多协议）端口并启用客户端和服务器端的 TFO 工作良好，绝对能使网络后端工作更快，尤其是在你有非常轻量和快速的查询时。
</details>

### listen_tfo

此设置允许所有监听器使用 TCP_FASTOPEN 标志。默认情况下，它由系统管理，但可以通过设置为 '0' 显式关闭。

有关 TCP Fast Open 扩展的一般知识，请参考 [维基百科](https://en.wikipedia.org/wiki/TCP_Fast_Open)。简而言之，它允许在建立连接时消除一次 TCP 往返。

在实践中，在许多情况下使用 TFO 可以优化客户端-代理之间的网络效率，就像使用[持久代理](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)一样，但不需要保持活动连接，也不限制最大连接数。

在现代操作系统中，TFO 支持通常在系统级别默认开启，但这只是一个“能力”，而非规则。Linux（作为最先进的之一）自 2011 年起支持它，在内核 3.7 及更高版本支持服务端。Windows 从部分 Windows 10 版本开始支持它。其他操作系统（FreeBSD、MacOS）也在参与支持。

对于 Linux 系统，服务器检查变量 `/proc/sys/net/ipv4/tcp_fastopen` 并根据其值行动。第 0 位管理客户端，第 1 位控制监听器。默认情况下，系统设置为 1，即客户端启用，监听器禁用。

### log

<!-- example conf log -->
log 设置指定了所有 `searchd` 运行时事件的日志文件名。如果未指定，默认名称为 'searchd.log'。

或者，你可以使用 'syslog' 作为文件名。在这种情况下，事件将发送到 syslog 守护进程。使用 syslog 选项时，你需要在构建 Manticore 时使用 `-–with-syslog` 选项进行配置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
log = /var/log/searchd.log
```
<!-- end -->


### max_batch_queries

<!-- example conf max_batch_queries -->
限制每批查询的数量。可选，默认值为 32。

使 searchd 在使用[多查询](../Searching/Multi-queries.md)时对单个批次提交的查询数量进行合理性检查。设置为 0 则跳过此检查。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_batch_queries = 256
```
<!-- end -->

### max_connections

<!-- example max_connections -->
最大同时客户端连接数。默认为无限制。通常只有在使用任何类型的持久连接时才会明显，比如CLI mysql 会话或远程分布式表的持久远程连接。当超过限制时，您仍然可以通过 [VIP 连接](../Connecting_to_the_server/MySQL_protocol.md#VIP-connection) 连接到服务器。VIP 连接不计入限制。

<!-- request Example -->
```ini
max_connections = 10
```

<!-- end -->

### max_threads_per_query

<!-- example max_threads_per_query -->
实例级别的操作线程数限制。默认情况下，适当的操作可以占用所有 CPU 核心，使其他操作无运行空间。例如，针对较大的 percolate 表执行 `call pq` 可能会使用所有线程持续数十秒。将 `max_threads_per_query` 设置为例如 [threads](../Server_settings/Searchd.md#threads) 的一半，可以保证你能并行运行几个此类 `call pq` 操作。

你也可以在运行时将此设置作为会话或全局变量设置。

另外，你可以借助 [threads 选项](../Searching/Options.md#threads) 对单个查询的行为进行控制。

<!-- intro -->
##### 示例：
<!-- request Example -->

```ini
max_threads_per_query = 4
```

<!-- end -->

### max_filters

<!-- example conf max_filters -->
单查询最大允许的过滤器数量。此设置仅用于内部合理性检查，不直接影响内存使用或性能。可选，默认值为 256。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_filters = 1024
```
<!-- end -->


### max_filter_values

<!-- example conf max_filter_values -->
单个过滤器允许的最大值数量。此设置仅用于内部合理性检查，不直接影响内存使用或性能。可选，默认值为 4096。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_filter_values = 16384
```
<!-- end -->


### max_open_files

<!-- example conf max_open_files -->
服务器允许打开的最大文件数称为“软限制”。请注意，服务大型分散的实时表可能需要将此限制设置得较高，因为每个磁盘块可能占用十几个或更多文件。例如，一个拥有1000个块的实时表可能需要同时打开数千个文件。如果在日志中遇到“Too many open files”错误，请尝试调整此选项，可能有助于解决问题。

还有一个不能被选项超过的“硬限制”。该限制由系统定义，可在Linux的 `/etc/security/limits.conf` 文件中更改。其他操作系统可能有不同的方法，详情请参阅相应的手册。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = 10000
```
<!-- end -->

<!-- example conf max_open_files max -->
除了直接的数值，您还可以使用魔法词“max”将限制设置为当前可用的硬限制值。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = max
```
<!-- end -->


### max_packet_size

<!-- example conf max_packet_size -->
允许的最大网络数据包大小。此设置限制来自客户端的查询数据包和分布式环境中远程代理的响应数据包。仅用于内部合理性检查，不直接影响内存使用或性能。可选，默认值为128M。


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

某些挑剔的MySQL客户端库依赖MySQL使用的特定版本号格式，且有时根据报告的版本号（而非指示的功能标志）选择不同的执行路径。例如，Python MySQLdb 1.2.2当版本号不是X.Y.ZZ格式时会抛出异常；MySQL .NET连接器6.3.x在版本号为1.x且某些标志组合时内部失败等。为了解决这个问题，可以使用 `mysql_version_string` 指令，让 `searchd` 向通过MySQL协议连接的客户端报告不同的版本。（默认情况下，报告自身版本。）


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
mysql_version_string = 5.0.37
```
<!-- end -->


### net_workers

网络线程数，默认值为1。

当查询率极高，单线程无法处理所有传入查询时，此设置非常有用。


### net_wait_tm

控制网络线程的忙循环间隔。默认值为-1，可设置为-1、0或正整数。

如果服务器被配置为纯主节点，只负责将请求路由给代理，则需要无延迟地处理请求，不允许网络线程休眠。为此有忙循环。当收到请求后，如果 `net_wait_tm` 为正数，网络线程会使用CPU轮询 `10 * net_wait_tm` 毫秒；如果为0，则仅用CPU轮询。忙循环可通过设置 `net_wait_tm = -1` 关闭，这种情况下，轮询器在系统轮询调用时将超时设置为实际代理的超时时间。

> **警告：**CPU忙循环会实际占用CPU核心，因此将此值设置为非默认值即使在空闲服务器上也会导致明显的CPU使用率。


### net_throttle_accept

定义网络循环每次迭代接收的客户端数量。默认0（无限制），对大多数用户来说都合适。此为高负载场景下微调网络循环吞吐量的选项。


### net_throttle_action

定义网络循环每次迭代处理的请求数量。默认0（无限制），对大多数用户来说都合适。此为高负载场景下微调网络循环吞吐量的选项。

### network_timeout

<!-- example conf network_timeout -->
网络客户端请求读写超时时间，单位为秒（或[special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认5秒。如果客户端在此超时时间内未发送查询或读取结果，`searchd`将强制关闭连接。

另请注意参数 [reset_network_timeout_on_packet](../Server_settings/Searchd.md#reset_network_timeout_on_packet)。该参数将 `network_timeout` 的应用范围从整个`query`或`result`改为单个数据包。通常，查询/结果在一到两个数据包内完成。然而，在需要大量数据的情况下，该参数对保持活跃操作非常有用。

<!-- request Example -->

```ini
network_timeout = 10s
```
<!-- end -->

### node_address

<!-- example conf node_address -->
此设置允许指定节点的网络地址。默认设置为复制的[listen](../Server_settings/Searchd.md#listen)地址。在大多数情况下这是正确的，但在以下情况下需要手动指定：

* 节点位于防火墙之后
* 启用了网络地址转换（NAT）
* 容器部署，例如Docker或云部署
* 多个区域内的集群节点


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
node_address = 10.101.0.10
```
<!-- end -->

### not_terms_only_allowed

<!-- example conf not_terms_only_allowed -->
此设置决定是否允许仅包含[取反](../Searching/Full_text_matching/Operators.md#Negation-operator)全文操作符的查询。可选，默认值为0（不允许仅含NOT操作符的查询）。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
not_terms_only_allowed = 1
```
<!-- end -->

### optimize_cutoff

<!-- example conf optimize_cutoff -->
设置默认的表压缩阈值。详细信息请参见此处 - [优化的磁盘块数量](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks)。此设置可以通过每查询选项 [cutoff](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 覆盖。它也可以通过 [SET GLOBAL](../Server_settings/Setting_variables_online.md#SET) 动态更改。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
optimize_cutoff = 4
```
<!-- end -->

### persistent_connections_limit

<!-- example conf persistent_connections_limit -->
此设置决定连接到远程 [持久代理](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 的最大持久连接数。每当连接定义在 `agent_persistent` 下的代理时，我们会尝试重用现有连接（如果有），或者连接并保存该连接以便将来使用。然而，在某些情况下，限制此类持久连接数是有意义的。该指令定义了限制。它影响所有分布式表中对每个代理主机的连接数。

合理将其设置为代理配置中的 [max_connections](../Server_settings/Searchd.md#max_connections) 选项的值或更小。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
persistent_connections_limit = 29 # assume that each host of agents has max_connections = 30 (or 29).
```
<!-- end -->


### pid_file

<!-- example conf pid_file -->
pid_file 是 Manticore search 中的必填配置选项，指定存储 `searchd` 服务器进程 ID 的文件路径。

searchd 进程 ID 文件在启动时被重新创建和锁定，在服务器运行时包含主服务器的进程 ID。服务器关闭时会删除该文件。
该文件的目的是使 Manticore 执行各种内部任务，例如检查是否已有运行中的 `searchd` 实例，停止 `searchd`，并通知它应旋转表。该文件也可供外部自动化脚本使用。


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
基于执行时间终止查询（使用最大查询时间设置）是一种不错的安全网，但它有一个固有缺点：不确定性（不稳定）结果。也就是说，如果你重复多次运行相同的（复杂的）带时间限制的搜索查询，时间限制将在不同阶段触发，你将获得*不同*的结果集。

<!-- intro -->
##### SQL：

<!-- request SQL -->

```sql
SELECT … OPTION max_query_time
```
<!-- request API -->

```api
SetMaxQueryTime()
```
<!-- end -->

有一个新选项，[SELECT … OPTION max_predicted_time](../Searching/Options.md#max_predicted_time)，允许你限制查询时间*同时*获得稳定、可重复的结果。它不是定期检查实际当前时间，因为这存在不确定性，而是使用一个简单的线性模型来预测当前运行时间：

```ini
predicted_time =
    doc_cost * processed_documents +
    hit_cost * processed_hits +
    skip_cost * skiplist_jumps +
    match_cost * found_matches
```

当 `predicted_time` 达到设定限额时，查询将提前终止。

当然，这不是对实际花费时间的严格限制（但它对处理工作量是严格限制），而且简单线性模型绝非理想精准模型。因此，实际时钟时间*可能*低于或超过目标限制。然而，误差范围相当可接受：例如，在我们的实验中，对于100毫秒的目标限制，大部分测试查询在95到105毫秒范围内，*所有*查询都在80到120毫秒范围内。此外，作为一个副作用，使用模型估算的查询时间而非测量实际运行时间，也减少了部分 gettimeofday() 调用。

不同服务器品牌和型号不同，因此 `predicted_time_costs` 指令允许你配置上面模型的成本。为了方便起见，它们是整数，单位为纳秒。（max_predicted_time 中的限制以毫秒计，而如果成本值要以0.000128毫秒而非128纳秒指定，则更容易出错。）不需要同时指定所有四个成本，遗漏的将使用默认值。不过，强烈建议为可读性指定全部。


### preopen_tables

<!-- example conf preopen_tables -->
preopen_tables 配置指令指定是否在启动时强制预打开所有表。默认值是1，这意味着无论每个表的 [preopen](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Other-performance-related-settings) 设置如何，都会预打开所有表。如果设置为0，则可以生效每个表的设置，默认是0。

预打开表可以防止搜索查询和轮换间的竞争，避免查询偶尔失败。然而，这也会使用更多文件句柄。大多数情况下，推荐预打开表。

示例如下配置：

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
preopen_tables = 1
```
<!-- end -->

### pseudo_sharding

<!-- example conf pseudo_sharding -->
pseudo_sharding 配置选项启用对本地普通表和实时表的搜索查询并行化，无论它们是直接查询还是通过分布式表查询。该功能会自动并行化查询，最多达到 `searchd.threads` 指定的线程数。

请注意，如果你的工作线程已经很忙，因为你有：
* 高查询并发
* 任何形式的物理分片：
  - 多个普通/实时表的分布式表
  - 由过多磁盘块组成的实时表

那么启用 pseudo_sharding 可能不会带来任何好处，甚至可能导致吞吐量略有下降。如果您更看重更高的吞吐量而不是更低的延迟，建议禁用此选项。

默认启用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
pseudo_sharding = 0
```
<!-- end -->


### replication_connect_timeout

`replication_connect_timeout` 指令定义了连接远程节点的超时时间。默认情况下，值被认为是毫秒，但也可以带有[其他后缀](../Server_settings/Special_suffixes.md)。默认值是 1000（1 秒）。

连接到远程节点时，Manticore 最多等待该时间以成功完成连接。如果达到超时但尚未建立连接，并且启用了 `retries`，则会发起重试。


### replication_query_timeout

`replication_query_timeout` 设置 searchd 等待远程节点完成查询的时间。默认值是 3000 毫秒（3 秒），但可以加`suffix`表示不同的时间单位。

建立连接后，Manticore 将等待远程节点最多 `replication_query_timeout` 时间完成。注意此超时与 `replication_connect_timeout` 是独立的，远程节点可能造成的总延迟是两个值的和。


### replication_retry_count

该设置为整数，指定 Manticore 在复制过程中尝试连接并查询远程节点的次数，超出后报告致命查询错误。默认值是 3。


### replication_retry_delay

该设置为以毫秒为单位的整数（或[特殊后缀](../Server_settings/Special_suffixes.md)），指定在复制期间失败时，Manticore 重试查询远程节点前的延迟时间。仅当指定非零值时有效。默认值是 500。

### qcache_max_bytes

<!-- example conf qcache_max_bytes -->
该配置设置用于缓存结果集的最大内存（以字节为单位）。默认值是 16777216，相当于 16 兆字节。如果设置为 0，则禁用查询缓存。有关查询缓存的更多信息，请参阅 [查询缓存](../Searching/Query_cache.md)。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
qcache_max_bytes = 16777216
```
<!-- end -->


### qcache_thresh_msec

整数，单位为毫秒。查询结果被缓存的最小实际执行时间阈值。默认值为 3000，即 3 秒。0 表示缓存所有结果。详情请参见 [查询缓存](../Searching/Query_cache.md)。此值也可以用时间[特殊后缀](../Server_settings/Special_suffixes.md)表示，但请谨慎使用，不要与含有“_msec”名称的值混淆。


### qcache_ttl_sec

整数，单位为秒。缓存结果集的过期时间。默认值为 60，即 1 分钟。最小值为 1 秒。详情请参见 [查询缓存](../Searching/Query_cache.md)。此值也可以用时间[特殊后缀](../Server_settings/Special_suffixes.md)表示，但请谨慎使用，不要与含有“_sec”名称的值混淆。


### query_log_format

<!-- example conf query_log_format -->
查询日志格式。可选值为 `plain` 和 `sphinxql`，默认是 `sphinxql`。

`sphinxql` 模式记录有效的 SQL 语句。`plain` 模式以纯文本格式记录查询（主要适用于纯全文搜索用例）。此指令允许您在搜索服务器启动时切换两种格式。日志格式也可以动态更改，使用 `SET GLOBAL query_log_format=sphinxql` 语法。详情请参见 [查询日志](../Logging/Query_logging.md)。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_format = sphinxql
```
<!-- end -->

### query_log_min_msec

限制（毫秒）防止查询被写入查询日志。可选，默认值为 0（所有查询都会写入日志）。该指令指定只有执行时间超过该限制的查询才会被记录（该值也可用时间[特殊后缀](../Server_settings/Special_suffixes.md)表示，但请谨慎使用，不要与名称中含 `_msec` 的值混淆）。

### query_log

<!-- example conf query_log -->
查询日志文件名。可选，默认为空（不记录查询）。所有搜索查询（如 SELECT ...，但不包括 INSERT/REPLACE/UPDATE 查询）会被记录到此文件。格式详见 [查询日志](../Logging/Query_logging.md)。若使用“plain”格式，可以使用“syslog”作为日志文件路径。在此情况下，所有搜索查询将作为 `LOG_INFO` 优先级发送到 syslog 守护进程，前缀为 “[query]” 替代时间戳。要使用 syslog 选项，Manticore 必须在构建时配置 `-–with-syslog`。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log = /var/log/query.log
```
<!-- end -->


### query_log_mode

<!-- example conf query_log_mode -->
`query_log_mode` 指令允许您为 searchd 和查询日志文件设置不同的权限。默认情况下，这些日志文件创建时权限为 600，意味着只有运行服务器的用户和 root 用户可以读取日志文件。
如果您希望允许其他用户读取日志文件（例如运行于非 root 用户的监控方案），这个指令非常有用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_mode  = 666
```
<!-- end -->

### read_buffer_docs

<!-- example conf read_buffer_docs -->
`read_buffer_docs` 指令控制文档列表中每个关键字的读取缓冲区大小。每个搜索查询中的每个关键字出现都有两个相关的读取缓冲区：一个用于文档列表，一个用于命中列表。此设置让您控制文档列表缓冲区的大小。

较大的缓冲区大小可能会增加每个查询的 RAM 使用量，但可能减少 I/O 时间。对于慢速存储，设置较大的值是合理的，但对于具备高 IOPS 能力的存储，应在较低值区域进行实验。

默认值是 256K，最小值是 8K。您也可以对单个表设置 [read_buffer_docs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs)，这将覆盖服务器配置级别的设置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_docs = 128K
```
<!-- end -->


### read_buffer_hits

<!-- example conf read_buffer_hits -->
read_buffer_hits 指令指定搜索查询中命中列表每个关键字的读取缓冲区大小。默认大小为 256K，最小值为 8K。对于搜索查询中的每个关键字出现，存在两个相关的读取缓冲区，一个用于文档列表，一个用于命中列表。增加缓冲区大小可以增加每查询使用的 RAM 但减少 I/O 时间。对于慢速存储，较大的缓冲区大小是合理的，而对于具备高 IOPS 能力的存储，应在较低值区域进行实验。

该设置也可以通过表级别的 read_buffer_hits 选项进行指定，见 [read_buffer_hits](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_hits)，这将覆盖服务器级设置。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_hits = 128K
```
<!-- end -->

### read_unhinted

<!-- example conf read_unhinted -->
未预提示读取大小。可选，默认是 32K，最小值是 1K。

在查询时，某些读取操作提前知道要读取的数据量，但有些则不知道。最典型的是，命中列表大小目前不能提前得知。此设置让您控制在这种情况下读取的数据量。它影响命中列表的 I/O 时间，对大小超过未预提示读取大小的列表降低 I/O 时间，但对较小列表时间会增加。它**不**影响 RAM 使用，因为读取缓冲区已经分配了。因此它的值不应大于 read_buffer。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_unhinted = 32K
```
<!-- end -->

### reset_network_timeout_on_packet

<!-- example conf reset_network_timeout_on_packet -->
细化网络超时行为（如 `network_timeout`、`read_timeout` 和 `agent_query_timeout`）。

当设置为 0 时，超时限制的是发送整个请求/查询的最长时间。
当设置为 1（默认）时，超时限制的是网络活动之间的最长时间。

在复制过程中，节点可能需要将一个大文件（例如 100GB）发送到另一个节点。假设网络传输速率为 1GB/s，分为每个 4-5MB 的数据包。传输整个文件需要 100 秒。默认的 5 秒超时只允许传输 5GB 数据后连接就会断开。增加超时可能是一个解决方案，但不可扩展（比如下一个文件是 150GB，同样会失败）。但是，默认的 `reset_network_timeout_on_packet` 设为 1，超时应用于单个数据包而不是整个传输。只要传输正在进行（且超时期间网络实际接收到数据），连接就保持活跃。如果传输卡住，出现包间超时则连接会被断开。

注意如果设置了分布式表，主节点和各代理节点都应调优。主节点方面，受影响的是 `agent_query_timeout`；代理节点方面，相关的是 `network_timeout`。

<!-- intro -->

##### 示例：

<!-- request Example -->

```ini
reset_network_timeout_on_packet = 0
```

<!-- end -->


### rt_flush_period

<!-- example conf rt_flush_period -->
实时 (RT) 表 RAM 块刷新检查周期，单位秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认是 10 小时。

完全放入 RAM 块的主动更新 RT 表仍可能导致不断增长的二进制日志，影响磁盘使用和崩溃恢复时间。通过此指令，搜索服务器执行定期的刷新检查，符合条件的 RAM 块可被保存，从而使后续二进制日志能够清理。详情见 [二进制日志](../Logging/Binary_logging.md)。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_flush_period = 3600 # 1 hour
```
<!-- end -->


### rt_merge_iops

<!-- example conf rt_merge_iops -->
RT 块合并线程允许启动的最大 I/O 操作次数（每秒）。可选，默认是 0（无限制）。

此指令让您限制因执行 `OPTIMIZE` 语句引起的 I/O 负载。保证所有 RT 优化活动产生的磁盘 IOPS（每秒输入输出次数）不会超过配置的限制。限制 rt_merge_iops 可以减少因合并导致的搜索性能下降。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_iops = 40
```
<!-- end -->

### rt_merge_maxiosize

<!-- example conf rt_merge_maxiosize -->
RT 块合并线程允许启动的最大单次 I/O 操作大小。可选，默认是 0（无限制）。

此指令让您限制因执行 `OPTIMIZE` 语句引起的 I/O 负载。超过此限制的 I/O 将被拆分为两个或多个 I/O，且这些拆分的 I/O 会被分别计入 [rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops) 的限制。这样保证所有优化操作每秒产生的磁盘 I/O 不会超过 (rt_merge_iops * rt_merge_maxiosize) 字节。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_maxiosize = 1M
```
<!-- end -->


### seamless_rotate

<!-- example conf seamless_rotate -->
防止在旋转包含大量数据以预缓存的表时 `searchd` 卡住。可选，默认启用无缝旋转（值为 1）。在 Windows 系统上，无缝旋转默认是关闭的。

表可能包含一些需要预缓存到RAM中的数据。目前，`.spa`、`.spb`、`.spi` 和 `.spm` 文件会被完全预缓存（它们分别包含属性数据、二进制属性数据、关键词表和已删除行映射）。在未启用无缝旋转的情况下，旋转表时尽量少用RAM，其工作流程如下：

1. 临时拒绝新查询（返回“重试”错误码）；
2. `searchd` 等待所有当前运行的查询完成；
3. 旧表被释放，其文件被重命名；
4. 新表文件被重命名，并分配所需的RAM；
5. 新表的属性和词典数据被预加载到RAM；
6. `searchd` 恢复从新表提供查询服务。

但是，如果属性或词典数据量很大，预加载步骤可能需要显著时间——预加载1-5GB以上的文件时可能长达几分钟。

启用无缝旋转后，旋转流程如下：

1. 分配新表的RAM存储；
2. 异步地将新表的属性和词典数据预加载到RAM；
3. 成功后，旧表被释放，两个表的文件都被重命名；
4. 失败时，新表被释放；
5. 在任意时刻，查询要么从旧表副本提供，要么从新表副本提供。

无缝旋转的代价是旋转期间峰值内存使用更高（因为在预加载新副本时，旧的和新的 `.spa/.spb/.spi/.spm` 数据都必须存在于RAM中）。平均使用量保持不变。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
seamless_rotate = 1
```
<!-- end -->

### secondary_indexes
<!-- example conf secondary_indexes -->

此选项启用/禁用搜索查询中二级索引的使用。该选项为可选，默认值为1（启用）。注意，索引时无需启用此选项，因为只要安装了[Manticore Columnar Library](https://github.com/manticoresoftware/columnar)，它始终启用。在搜索时使用索引也需要此库。共有三种模式：

* `0`：禁用在搜索时使用二级索引。可以通过[分析器提示](../Searching/Options.md#Query-optimizer-hints)为单个查询启用。
* `1`：启用在搜索时使用二级索引。可以通过[分析器提示](../Searching/Options.md#Query-optimizer-hints)为单个查询禁用。
* `force`：与启用相同，但加载二级索引时的任何错误都会被报告，且整个索引将不会被加载到守护进程。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
secondary_indexes = 1
```

<!-- end -->

### server_id

<!-- example conf server_id -->
用于作为服务器标识符的整数，用作生成集群内唯一短UUID的种子。`server_id` 必须在集群节点中唯一，且范围为0到127。如果未设置`server_id`，则通过MAC地址和PID文件路径的哈希计算，或者使用随机数作为短UUID的种子。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
server_id = 1
```
<!-- end -->


### shutdown_timeout

<!-- example conf shutdown_timeout -->
`searchd --stopwait` 的等待时间（单位：秒，支持[special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认值为60秒。

运行 `searchd --stopwait` 时，服务器需在停止前完成一些任务，如完成查询、刷新RT内存块、刷新属性和更新binlog。这些任务需要一定时间。`searchd --stopwait` 会等待最多 `shutdown_timeout` 秒，以完成这些任务。合适时间取决于表的大小和负载。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
shutdown_timeout = 3m # wait for up to 3 minutes
```
<!-- end -->


### shutdown_token

调用VIP Manticore SQL连接的“shutdown”命令所需的密码SHA1哈希。没有它，[debug](../Reporting_bugs.md#DEBUG) 的“shutdown”子命令永远不会导致服务器停止。请注意，这种简单的哈希不应被视为强保护，因为我们未使用盐哈希或任何现代哈希函数。此设计旨在作为局域网内管理守护进程的防误措施。

### snippets_file_prefix

<!-- example conf snippets_file_prefix -->
生成片段时，用于本地文件名前缀。可选，默认是当前工作目录。

该前缀可与 `load_files` 或 `load_files_scattered` 选项结合，用于分布式生成片段。

请注意，这只是一个前缀，**不是路径！** 这意味着如果前缀设置为“server1”，请求引用“file23”，`searchd` 会尝试打开“server1file23”（不含引号）。如果需要路径，则必须包含尾部斜杠。

构造最终文件路径后，服务器会展开所有相对目录，并将最终结果与 `snippets_file_prefix` 的值比较。如果结果没有以该前缀开头，则该文件会被拒绝并报错。

例如，如果将其设置为 `/mnt/data`，有人以 `../../../etc/passwd` 作为源调用片段生成，返回的错误信息将是：

`File '/mnt/data/../../../etc/passwd' escapes '/mnt/data/' scope`

而不是文件内容。

另外，当参数未设置且读取 `/etc/passwd` 时，实际上会读取 `/daemon/working/folder/etc/passwd`，因为参数默认值是服务器工作目录。

还需注意，此为本地选项；不会影响代理。因此，可以安全地在主服务器上设置前缀。路由到代理的请求不会受主服务器设置影响，但会受代理自身设置影响。

当文档存储位置（无论本地存储或NAS挂载点）在服务器间不一致时，此设置非常有用。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
snippets_file_prefix = /mnt/common/server1/
```
<!-- end -->

> **警告：** 如果您仍然想从文件系统根目录访问文件，必须显式将 `snippets_file_prefix` 设置为空值（通过 `snippets_file_prefix=` 行），或设置为根目录（通过 `snippets_file_prefix=/`）。


### sphinxql_state

<!-- example conf sphinxql_state -->
用于序列化当前 SQL 状态的文件路径。

服务器启动时，会重放此文件。在符合条件的状态更改（例如，SET GLOBAL）时，该文件会自动重写。这可以防止一个难以诊断的问题：如果您加载了 UDF 函数但 Manticore 崩溃，当它（自动）重启时，您的 UDF 和全局变量将不再可用。使用持久状态有助于确保平滑恢复，避免此类意外。

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
使用 SQL 接口时，两个请求之间最大等待时间（以秒为单位，或使用 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认是 15 分钟。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
sphinxql_timeout = 15m
```
<!-- end -->


### ssl_ca

<!-- example conf ssl_ca -->
SSL 证书授权机构 (CA) 证书文件的路径（也称为根证书）。可选，默认为空。当不为空时，`ssl_cert` 中的证书应由此根证书签署。

服务器使用该 CA 文件验证证书签名。文件必须为 PEM 格式。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ssl_ca = keys/ca-cert.pem
```
<!-- end -->


### ssl_cert

<!-- example conf ssl_cert -->
服务器 SSL 证书的路径。可选，默认为空。

服务器使用此证书作为自签名公钥，通过 SSL 加密 HTTP 流量。文件必须为 PEM 格式。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ssl_cert = keys/server-cert.pem
```
<!-- end -->


### ssl_key

<!-- example conf ssl_key -->
SSL 证书密钥的路径。可选，默认为空。

服务器使用此私钥通过 SSL 加密 HTTP 流量。文件必须为 PEM 格式。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ssl_key = keys/server-key.pem
```
<!-- end -->


### subtree_docs_cache

<!-- example conf subtree_docs_cache -->
每个查询最大公共子树文档缓存大小。可选，默认是 0（禁用）。

此设置限制公共子树优化器的内存使用（参见 [multi-queries](../Searching/Multi-queries.md)）。对于每个查询，最多使用这么多内存缓存文档条目。设置为 0 禁用优化器。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
subtree_docs_cache = 8M
```
<!-- end -->


### subtree_hits_cache

<!-- example conf subtree_hits_cache -->
每个查询最大公共子树命中缓存大小。可选，默认是 0（禁用）。

此设置限制公共子树优化器的内存使用（参见 [multi-queries](../Searching/Multi-queries.md)）。对于每个查询，最多使用这么多内存缓存关键词出现（命中）信息。设置为 0 禁用优化器。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
subtree_hits_cache = 16M
```
<!-- end -->

### threads

<!-- example threads -->
Manticore 守护进程的工作线程数（或线程池大小）。Manticore 启动时创建这么多操作系统线程，这些线程执行守护进程中的所有任务，例如执行查询、创建摘要等。有些操作可以分解为子任务并并行执行，例如：

* 在实时表中搜索
* 在由本地表组成的分布式表中搜索
* 轮询查询调用
* 以及其它情况

默认情况下，设置为服务器的 CPU 核心数。Manticore 启动时创建线程，并保持它们直到停止。每个子任务需要时可以使用一个线程。子任务完成后释放线程供其他子任务使用。

在 I/O 密集型负载情况下，设置高于 CPU 核心数可能更合理。

<!-- request Example -->
```ini
threads = 10
```

<!-- end -->

### thread_stack

<!-- example conf thread_stack -->
每个作业的最大栈大小（协程，一个搜索查询可能引发多个作业/协程）。可选，默认是 128K。

每个作业拥有自己的 128K 栈。运行查询时，会检查所需栈大小。如果默认 128K 足够，直接处理；如需更大栈，将调度另一个带更大栈的作业继续处理。最大栈大小受此设置限制。

将此值设置得比较高，有助于处理非常深的查询，同时不会导致整体内存消耗过高。例如，设置为 1G 并不意味着每个新作业都会占用 1G 内存，而是根据实际需要分配，比如需要 100M 就分配 100M，其他作业仍使用默认 128K 栈。以此类推，可处理更复杂需求为 500M 的查询。只有当作业内部真正需要超过 1G 栈时，才会失败并报告 `thread_stack` 过低。

不过实际上，需要16M栈的查询通常过于复杂，解析费时费资源。因此守护进程会处理它，但通过 `thread_stack` 限制此类查询是合理的。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
thread_stack = 8M
```
<!-- end -->


### unlink_old

<!-- example conf unlink_old -->
确定在成功重命名（轮换）后是否删除 `.old` 表副本。可选，默认是 1（执行删除）。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
unlink_old = 0
```
<!-- end -->


### watchdog

<!-- example conf watchdog -->
线程化服务器监控守护进程。可选，默认是 1（启用监控）。

当 Manticore 查询崩溃时，可能会导致整个服务器宕机。启用 watchdog 功能后，`searchd` 还会维护一个单独的轻量级进程，该进程监控主服务器进程，并在异常终止时自动重启它。watchdog 默认启用。

<!-- request Example -->

```ini
watchdog = 0 # disable watchdog
```
<!-- end -->
<!-- proofread -->

