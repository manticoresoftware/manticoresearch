# 配置中的 "Searchd" 部分

以下设置用于 Manticore Search 配置文件的 `searchd` 部分，以控制服务器的行为。以下是每个设置的摘要：

### access_plain_attrs

此设置为 [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例级默认值。此项为可选，默认值为 `mmap_preread`。

`access_plain_attrs` 指令允许您为该 searchd 实例管理的所有表定义 [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每表的指令优先级更高，会覆盖该实例级默认值，从而提供更细粒度的控制。

### access_blob_attrs

此设置为 [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例级默认值。此项为可选，默认值为 `mmap_preread`。

`access_blob_attrs` 指令允许您为该 searchd 实例管理的所有表定义 [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每表的指令优先级更高，会覆盖该实例级默认值，从而提供更细粒度的控制。

### access_doclists

此设置为 [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例级默认值。此项为可选，默认值为 `file`。

`access_doclists` 指令允许您为该 searchd 实例管理的所有表定义 [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每表的指令优先级更高，会覆盖该实例级默认值，从而提供更细粒度的控制。

### access_hitlists

此设置为 [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例级默认值。此项为可选，默认值为 `file`。

`access_hitlists` 指令允许您为该 searchd 实例管理的所有表定义 [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每表的指令优先级更高，会覆盖该实例级默认值，从而提供更细粒度的控制。

### access_dict

此设置为 [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例级默认值。此项为可选，默认值为 `mmap_preread`。

`access_dict` 指令允许您为该 searchd 实例管理的所有表定义 [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每表的指令优先级更高，会覆盖该实例级默认值，从而提供更细粒度的控制。

### agent_connect_timeout

此设置为 [agent_connect_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) 参数设置实例级默认值。


### agent_query_timeout

此设置为 [agent_query_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) 参数设置实例级默认值。可通过在查询中使用 `OPTION agent_query_timeout=XXX` 子句进行覆盖。


### agent_retry_count

此设置为整数，指定 Manticore 在报告致命查询错误之前，尝试通过分布式表连接和查询远程代理的次数。默认值为 0（即不重试）。您也可以通过查询中的 `OPTION retry_count=XXX` 子句设置此值。若提供了每查询选项，它将覆盖配置中指定的值。

请注意，如果您在分布式表定义中使用 [agent mirrors](../Creating_a_cluster/Remote_nodes/Mirroring.md#Agent-mirrors)，服务器将在每次连接尝试时根据所选的 [ha_strategy](../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) 选择不同的镜像。在此情况下，`agent_retry_count` 将针对集合中的所有镜像进行汇总。

例如，假设您有 10 个镜像且设置了 `agent_retry_count=5`，服务器将最多重试 50 次，假设每个镜像平均尝试 5 次（在使用 `ha_strategy = roundrobin` 选项时即为如此）。

然而，代理定义中提供的作为 `retry_count` 选项的值为绝对限制。换言之，代理定义中的 `[retry_count=2]` 选项始终表示最多尝试 2 次，无论您为该代理指定了 1 个还是 10 个镜像。

### agent_retry_delay

此设置为以毫秒为单位的整数（或使用 [special_suffixes](../Server_settings/Special_suffixes.md)） ，指定在查询远程代理失败后 Manticore 重试之前的延迟时间。仅当指定了非零 [agent_retry_count](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 或非零的每查询 `retry_count` 时此值才生效。默认值为 500。您也可以通过查询中的 `OPTION retry_delay=XXX` 子句设置此值。若提供了每查询选项，它将覆盖配置中指定的值。


### attr_flush_period

<!-- example conf attr_flush_period -->
当使用 [Update](../Data_creation_and_modification/Updating_documents/UPDATE.md) 实时修改文档属性时，变更首先写入属性的内存副本。这些更新发生在内存映射文件中，意味着操作系统决定何时将更改写入磁盘。在 `searchd` 正常关闭时（由 `SIGTERM` 信号触发），所有更改都会被强制写入磁盘。

您也可以指示 `searchd` 周期性地将这些更改写回磁盘以防止数据丢失。刷新间隔由 `attr_flush_period` 决定，单位为秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。

默认情况下，该值为 0，表示禁用周期性刷新。但在正常关闭时仍会进行刷新。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
attr_flush_period = 900 # persist updates to disk every 15 minutes
```
<!-- end -->

### auto_optimize

<!-- example conf auto_optimize -->
此设置控制表自动 [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 过程以进行表压缩。

默认情况下，表压缩是自动进行的。你可以用 `auto_optimize` 设置修改此行为:
* 0 禁用自动表压缩（你仍然可以手动调用 `OPTIMIZE`）
* 1 显式启用它
* 启用它同时将优化阈值乘以 2。

默认情况下，OPTIMIZE 会运行直到磁盘块数量小于或等于逻辑 CPU 核心数乘以 2。

但是，如果表中有带有 KNN 索引的属性，此阈值会不同。此时阈值设置为物理 CPU 核心数除以 2，以提高 KNN 搜索性能。

请注意，切换 `auto_optimize` 的开关不会阻止你手动运行 [OPTIMIZE TABLE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE)。

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
Manticore 支持自动创建尚不存在但在 INSERT 语句中指定的表。此功能默认启用。要禁用它，请在配置中显式设置 `auto_schema = 0`。要重新启用，设置 `auto_schema = 1` 或从配置中移除 `auto_schema` 设置。

请注意，`/bulk` HTTP 端点不支持自动创建表。

> 注意：[自动模式功能](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保 Buddy 已安装。

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
此设置控制二进制日志事务刷新/同步模式。此项为可选，默认值为 2（每次事务刷新，每秒同步）。

该指令决定二进制日志多久刷新到操作系统并同步到磁盘。支持三种模式：

*  0，每秒刷新和同步一次。性能最好，但在服务器崩溃或操作系统/硬件崩溃时，最多可丢失 1 秒内提交的事务。
*  1，每个事务都刷新并同步。性能最差，但保证每个提交事务的数据都会保存。
*  2，每个事务刷新，每秒同步一次。性能较好，保证服务器崩溃时保存所有提交事务数据。但如果发生操作系统/硬件崩溃，最多可丢失 1 秒内提交的事务。

对于熟悉 MySQL 和 InnoDB 的用户，此指令类似于 `innodb_flush_log_at_trx_commit`。大多数情况下，默认的混合模式 2 在速度和安全性之间取得良好平衡，完全保护实时表数据免遭服务器崩溃，并在一定程度上防范硬件崩溃。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
binlog_flush = 1 # ultimate safety, low speed
```
<!-- end -->

### binlog_common

<!-- example conf binlog_common -->
此设置控制二进制日志文件的管理方式。此项为可选，默认值为 0（为每个表使用独立文件）。

你可以选择两种二进制日志文件管理方式：

* 为每个表使用独立文件（默认，`0`）：每个表将更改保存到自己的日志文件。此方案适合你有许多表且它们更新时间不一的场景。它允许各表独立更新，不需等待其他表。此外，一个表的日志文件出现问题不会影响其他表。
* 所有表共用一个文件（`1`）：所有表使用同一个二进制日志文件。此方法便于管理文件，因为文件数量更少。但如果某个表仍需保存更新，可能导致文件存留时间延长。若大量表同时需要更新，也会因所有变化需排队写入同一文件而拖慢速度。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
binlog_common = 1 # use a single binary log file for all tables
```
<!-- end -->

### binlog_max_log_size

<!-- example conf binlog_max_log_size -->
此设置控制二进制日志文件的最大大小。此项为可选，默认值为 256 MB。

当前二进制日志文件达到此大小限制后，会强制打开新的 binlog 文件。这带来更细粒度的日志，并且在某些边界工作负载下能够更有效地使用 binlog 磁盘空间。值为 0 表示 binlog 文件不因大小而被重新打开。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
binlog_max_log_size = 16M
```
<!-- end -->


### binlog_path

<!-- example conf binlog_path -->
此设置确定二进制日志（也称事务日志）文件的路径。此项为可选，默认值为构建时配置的数据目录（例如 Linux 中的 `/var/lib/manticore/data/binlog.*`）。

二进制日志用于RT表数据的崩溃恢复，以及普通磁盘索引的属性更新，否则这些属性更新只会存在于RAM中，直到刷新。启用日志记录时，每个提交（COMMIT）到RT表的事务都会写入日志文件。在非正常关闭后，启动时会自动重放日志，恢复已记录的更改。

`binlog_path`指令指定二进制日志文件的位置。它只应包含路径；`searchd`将根据需要在该目录中创建和删除多个`binlog.*`文件（包括binlog数据、元数据和锁文件等）。

空值将禁用二进制日志，这将提升性能，但会使RT表数据处于风险中。


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
此设置确定Manticore Buddy二进制文件的路径。该设置是可选的，默认值为构建时配置的路径，不同操作系统间会有所不同。通常情况下，您无需修改此设置。不过，如果您希望以调试模式运行Manticore Buddy、修改Manticore Buddy或实现新插件，这将很有用。在后一种情况下，您可以从https://github.com/manticoresoftware/manticoresearch-buddy克隆Buddy，向`./plugins/`目录添加新插件，然后切换到Buddy源代码目录后运行`composer install --prefer-source`以方便开发。

为了确保您可以运行`composer`，您的机器必须安装PHP 8.2或更高版本，并带有以下扩展：

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

您也可以选择发布中的Linux amd64专用版本 `manticore-executor-dev`，例如：https://github.com/manticoresoftware/executor/releases/tag/v1.0.13

若选择此方式，记得将dev版本的manticore执行器链接到`/usr/bin/php`。

要禁用Manticore Buddy，请将值设置为空，如示例所示。

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
此设置确定在使用持久连接时请求之间的最大等待时间（以秒或[special_suffixes](../Server_settings/Special_suffixes.md)计）。可选，默认值为五分钟。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
client_timeout = 1h
```
<!-- end -->


### collation_libc_locale

<!-- example conf collation_libc_locale -->
服务器libc区域设置。可选，默认为C。

指定libc区域设置，影响基于libc的排序规则。详情请参阅[排序规则](../Searching/Collations.md)部分。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
collation_libc_locale = fr_FR
```
<!-- end -->


### collation_server

<!-- example conf collation_server -->
默认服务器排序规则。可选，默认为libc_ci。

指定用于传入请求的默认排序规则。排序规则可以按查询覆盖。请参阅[排序规则](../Searching/Collations.md)部分以获取可用排序规则列表和其他详情。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
collation_server = utf8_ci
```
<!-- end -->


### data_dir

<!-- example conf data_dir -->
指定此项时，启用[实时模式](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)，这是一种命令式的数据模式管理方式。其值应为您希望存储所有表、二进制日志及运行此模式下Manticore Search所需一切内容的目录路径。
指定`data_dir`时，不允许对[普通表](../Creating_a_table/Local_tables/Plain_table.md)进行索引。更多关于RT模式与普通模式区别，请参阅[此部分](../Read_this_first.md#Real-time-table-vs-plain-table)。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
data_dir = /var/lib/manticore
```
<!-- end -->

### diskchunk_flush_search_timeout

<!-- example conf diskchunk_flush_search_timeout -->
防止当表中无搜索操作时自动刷新RAM数据块的超时时间。可选，默认30秒。

在决定是否自动刷新前，检查有无搜索操作的时间。
仅当表在过去`diskchunk_flush_search_timeout`秒内至少进行过一次搜索时才会自动刷新。与 [diskchunk_flush_write_timeout](../Server_settings/Searchd.md#diskchunk_flush_write_timeout) 配合使用。对应的[每表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_search_timeout)优先级更高，会覆盖该实例默认值，实现更细粒度的控制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
diskchunk_flush_search_timeout = 120s
```
<!-- end -->

### diskchunk_flush_write_timeout

<!-- example conf diskchunk_flush_write_timeout -->
在无写入操作后等待自动刷新RAM数据块到磁盘的时间（秒）。可选，默认1秒。

如果在`diskchunk_flush_write_timeout`秒内RAM数据块没有写入，数据块将被刷新到磁盘。与 [diskchunk_flush_search_timeout](../Server_settings/Searchd.md#diskchunk_flush_search_timeout) 配合使用。若要禁用自动刷新，请在配置中显式设置`diskchunk_flush_write_timeout = -1`。对应的[每表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_write_timeout)优先级更高，会覆盖此实例默认值，实现更细粒度的控制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
diskchunk_flush_write_timeout = 60s
```
<!-- end -->

### docstore_cache_size

<!-- example conf docstore_cache_size -->
此设置指定从文档存储中保持在内存中的文档块的最大大小。可选，默认值为16m（16兆字节）。

当使用 `stored_fields` 时，文档块从磁盘读取并解压。由于每个块通常包含多个文档，因此在处理下一个文档时可以重用该块。为此，块被保存在服务器范围的缓存中。缓存中保存的是未压缩的块。


<!-- intro -->
##### 例子:

<!-- request Example -->

```ini
docstore_cache_size = 8m
```
<!-- end -->

### engine

<!-- example conf engine -->
RT 模式下创建表时使用的默认属性存储引擎。可以是 `rowwise`（默认）或 `columnar`。

<!-- intro -->
##### 例子:

<!-- request Example -->

```ini
engine = columnar
```
<!-- end -->


### expansion_limit

<!-- example conf expansion_limit -->
此设置确定单个通配符的最大扩展关键字数量。该设置是可选的，默认值为 0（无限制）。

当对启用了 `dict = keywords` 的表执行子字符串搜索时，单个通配符可能会导致成千上万甚至数百万个匹配的关键字（比如对整个牛津词典匹配 `a*`）。此指令允许限制此类扩展的影响。设置 `expansion_limit = N` 限制每个通配符在查询中最多扩展为 N 个最常见的匹配关键字。

<!-- intro -->
##### 例子:

<!-- request Example -->

```ini
expansion_limit = 16
```
<!-- end -->

### expansion_merge_threshold_docs

<!-- example conf expansion_merge_threshold_docs -->
此设置确定允许合并所有扩展关键字的最大文档数。该设置是可选的，默认值为 32。

当对启用了 `dict = keywords` 的表执行子字符串搜索时，单个通配符可能会导致成千上万甚至数百万个匹配的关键字。此指令允许增加合并关键字的限制，以加速匹配，但会使用更多搜索内存。

<!-- intro -->
##### 例子:

<!-- request Example -->

```ini
expansion_merge_threshold_docs = 1024
```
<!-- end -->

### expansion_merge_threshold_hits

<!-- example conf expansion_merge_threshold_hits -->
此设置确定允许合并所有扩展关键字的最大命中数。该设置是可选的，默认值为 256。

当对启用了 `dict = keywords` 的表执行子字符串搜索时，单个通配符可能会导致成千上万甚至数百万个匹配的关键字。此指令允许增加合并关键字的限制，以加速匹配，但会使用更多搜索内存。

<!-- intro -->
##### 例子:

<!-- request Example -->

```ini
expansion_merge_threshold_hits = 512
```
<!-- end -->

### expansion_phrase_limit

<!-- example conf expansion_phrase_limit -->
此设置控制由于 `PHRASE`、`PROXIMITY` 和 `QUORUM` 操作符内的 `OR` 操作符产生的替代短语变体的最大数量。该设置是可选的，默认值为 1024。

当在短语类操作符内使用 `|`（OR）操作符时，根据指定的替代项数量，扩展组合的总数可能以指数方式增长。该设置通过限制查询处理中考虑的排列数来防止过度的查询扩展。如果生成的变体数量超过此限制，查询将失败并返回错误。

<!-- intro -->
##### 例子:

<!-- request Example -->

```ini
expansion_phrase_limit = 4096
```
<!-- end -->

### grouping_in_utc

该设置指定 API 和 SQL 中的时间分组是基于本地时区还是 UTC 进行计算。该设置是可选的，默认值为 0（即“本地时区”）。

默认情况下，所有“按时间分组”表达式（如 API 中的按日、周、月、年分组，以及 SQL 中的按日、月、年、年月、年月日分组）均使用本地时间。例如，如果您有属性时间为 `13:00 utc` 和 `15:00 utc` 的文档，在分组时，两者都将根据本地时区设置被归入相应的分组。如果您的时区是 `utc`，它们将算作同一天；但如果您的时区是 `utc+10`，那么这两份文档将被归入不同的按日分组（因为 UTC+10 时区的 13:00 utc 是本地时间的 23:00，15:00 utc 是次日 01:00）。有时这种行为是不被接受的，希望时间分组不依赖时区。您可以通过设置全局 TZ 环境变量运行服务器，但这会影响到日志时间戳，不一定理想。打开此选项（在配置文件中或通过 SQL 中的 [SET global](../Server_settings/Setting_variables_online.md#SET) 语句）将使所有时间分组表达式采用 UTC 来计算，而其他与时间相关的函数（如服务器日志记录）依然使用本地时区。


### timezone

该设置指定日期/时间相关函数使用的时区。默认使用本地时区，但您可以指定 IANA 格式（例如 `Europe/Amsterdam`）的其他时区。

请注意，此设置不会影响日志记录，日志始终以本地时区进行。

另外，注意如果使用了 `grouping_in_utc`，按时间分组功能将使用 UTC，而其他日期/时间相关函数将使用指定时区。总体而言，不建议同时混用 `grouping_in_utc` 和 `timezone`。

您可以在配置文件中设置该选项，或通过 SQL 中的 [SET global](../Server_settings/Setting_variables_online.md#SET) 语句设置。


### ha_period_karma

<!-- example conf ha_period_karma -->
该设置指定代理镜像统计窗口大小，单位为秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。该设置是可选的，默认值为 60 秒。

对于包含代理镜像的分布式表（详见[agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)），主节点跟踪几个不同的每镜像计数器。这些计数器随后用于故障切换和负载均衡（主节点基于计数器选择最佳镜像）。计数器以 `ha_period_karma` 秒为单位累计。

开始一个新周期后，主节点仍可能使用前一个周期累计的值，直到新周期使用了一半为止。因此，任何之前的历史数据最多在 1.5 倍 ha_period_karma 秒后不再影响镜像选择。

尽管镜像选择最多使用两个周期的数据，但为了监控目的，会存储最多15个最近的周期。这些周期可以通过 [SHOW AGENT STATUS](../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS) 语句进行查看。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_period_karma = 2m
```
<!-- end -->


### ha_ping_interval

<!-- example conf ha_ping_interval -->
此设置配置代理镜像间 Ping 的间隔时间，单位为毫秒（或[特殊后缀](../Server_settings/Special_suffixes.md)）。可选，默认值为1000毫秒。

对于包含代理镜像的分布式表（详见[agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)），主节点会在空闲期间向所有镜像发送 Ping 命令，以追踪当前代理状态（存活或失效、网络往返时间等）。Ping 间隔由此指令定义。若要禁用 Ping，将 ha_ping_interval 设置为0。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_ping_interval = 3s
```
<!-- end -->


### hostname_lookup

`hostname_lookup` 选项定义了主机名刷新策略。默认情况下，服务器启动时会缓存代理主机名的IP地址，以避免频繁访问DNS。但是在某些情况下，IP会动态变化（例如云托管），这时可能不希望缓存IP。将此选项设置为 `request` 会禁用缓存，并在每次查询时访问DNS。IP地址也可通过 `FLUSH HOSTNAMES` 命令手动刷新。

### jobs_queue_size

jobs_queue_size 设置定义队列中可同时存在的“作业”数量。默认无限制。

在大多数情况下，“作业”指对单个本地表（普通表或实时表的数据块）的一次查询。例如，如果你的分布式表由两个本地表组成，或一个实时表有两个磁盘数据块，则对任何一个进行搜索查询时通常会在队列中放置两个作业。然后，由线程池（大小由 [threads](../Server_settings/Searchd.md#threads) 定义）处理它们。但在某些情况下，如果查询非常复杂，可能会产生更多作业。当 [max_connections](../Server_settings/Searchd.md#max_connections) 和 [threads](../Server_settings/Searchd.md#threads) 不足以达到性能平衡时，建议调整此设置。

### join_batch_size

表连接通过积累一个匹配批次来工作，该批次是对左表执行查询的结果。随后，这个批次作为一个整体查询右表。

此选项允许你调整批次大小。默认值为 `1000`，设置为 `0` 则禁用批处理。

较大的批次大小可能提高性能；但对某些查询，可能导致过高的内存消耗。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
join_batch_size = 2000
```
<!-- end -->

### join_cache_size

对右表执行的每个查询都依据特定的 JOIN ON 条件，这决定了从右表检索的结果集。

如果唯一的 JOIN ON 条件较少，重复使用结果比多次执行右表查询更高效。为启用此功能，结果集会被存储在缓存中。

此选项用于配置该缓存的大小。默认值为 `20 MB`，设置为0则禁用缓存。

注意每个线程维护自己的缓存，因此估算总内存使用时需考虑执行查询的线程数。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
join_cache_size = 10M
```
<!-- end -->

### listen_backlog

<!-- example conf listen_backlog -->
listen_backlog 设置确定TCP监听队列的长度，以接收传入连接。这在处理请求逐个处理的Windows版本中尤为重要。当连接队列达到上限时，新的连接将被拒绝。
对于非Windows版本，默认值通常足够，一般无需调整此设置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
listen_backlog = 20
```
<!-- end -->

### kibana_version_string

<!-- example conf kibana_version_string -->
返回给 Kibana 或 OpenSearch Dashboards 的服务器版本字符串。可选，默认值为 `7.6.0`。

某些版本的 Kibana 和 OpenSearch Dashboards 期望服务器报告特定版本号，且可能根据版本号表现不同。为解决此类问题，可以使用此设置，使 Manticore 向 Kibana 或 OpenSearch Dashboards 报告自定义版本号。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
kibana_version_string = 1.2.3
```
<!-- end -->

### listen

<!-- example conf listen -->
此设置允许你指定 Manticore 接受连接的 IP 地址和端口，或 Unix 域套接字路径。

`listen` 的通用语法为：

```ini
listen = ( address ":" port | port | path | address ":" port start - port end ) [ ":" protocol [ "_vip" ] [ "_readonly" ] ]
```

你可以指定：
* IP 地址（或主机名）和端口号
* 或仅端口号
* 或 Unix 套接字路径（Windows不支持）
* 或 IP 地址和端口范围

如果只指定端口号而未指定地址，`searchd` 将监听所有网络接口。Unix 路径以斜杠开头。端口范围仅能用于复制协议。

您还可以为此套接字上的连接指定一个协议处理器（监听器）。监听器包括：

* **未指定** - Manticore 将接受此端口上的连接，来源包括：
  - 其他 Manticore 代理（即远程分布式表）
  - 通过 HTTP 和 HTTPS 的客户端
  - [Manticore Buddy](https://manticoresearch.com/blog/manticoresearch-buddy-intro/)。**确保您配置了此类监听器（或下面提到的 `http` 监听器），以避免 Manticore 功能受限。**
* `mysql` - 来自 MySQL 客户端的 MySQL 协议连接。注意：
  - 也支持压缩协议。
  - 如果启用了 [SSL](../Security/SSL.md#SSL)，可以建立加密连接。
* `replication` - 用于节点通信的复制协议。更多详情见 [复制](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md) 部分。您可以指定多个复制监听器，但它们必须全部监听同一个 IP，只能端口不同。当您用端口范围定义复制监听器（例如，`listen = 192.168.0.1:9320-9328:replication`）时，Manticore 不会立即开始监听这些端口。只有当您开始使用复制时，才会从指定范围中随机使用空闲端口。复制正常工作至少需要范围内有 2 个端口。
* `http` - 与 **未指定** 相同。Manticore 将接受该端口来自远程代理和 HTTP/HTTPS 客户端的连接。
* `https` - HTTPS 协议。Manticore 仅接受该端口的 HTTPS 连接。更多详情见 [SSL](../Security/SSL.md) 部分。
* `sphinx` - 传统二进制协议。用于服务来自远程 [SphinxSE](../Extensions/SphinxSE.md) 客户端的连接。一些 Sphinx API 客户端实现（如 Java 版）需要显式声明监听器。

在客户端协议后缀添加 `_vip`（即除 `replication` 外的所有协议，例如 `mysql_vip` 或 `http_vip` 或仅 `_vip`）会强制为连接创建专用线程，以绕过不同限制。这对节点维护有用，在严重过载时服务器否则会挂起或不允许通过普通端口连接。

后缀 `_readonly` 会为监听器设置[只读模式](../Security/Read_only.md)，仅允许接受读查询。

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

可以有多个 `listen` 指令。`searchd` 将监听所有指定端口和套接字上的客户端连接。Manticore 包中默认配置定义了监听以下端口：
* `9308` 和 `9312` 用于远程代理和非 MySQL 客户端连接
* 端口 `9306` 用于 MySQL 连接。

如果完全未在配置中指定任何 `listen`，Manticore 将等待连接于：
* `127.0.0.1:9306` 用于 MySQL 客户端
* `127.0.0.1:9312` 用于 HTTP/HTTPS 及来自其他 Manticore 节点和基于 Manticore 二进制 API 的客户端的连接。

#### 监听特权端口

默认情况下，Linux 不允许 Manticore 监听 1024 以下的端口（如 `listen = 127.0.0.1:80:http` 或 `listen = 127.0.0.1:443:https`），除非您以 root 权限运行 searchd。如果您仍希望在非 root 用户下启动 Manticore 监听 < 1024 端口，可考虑以下方法之一（任选其一均可）：
* 运行命令 `setcap CAP_NET_BIND_SERVICE=+eip /usr/bin/searchd`
* 将 `AmbientCapabilities=CAP_NET_BIND_SERVICE` 添加至 Manticore 的 systemd 单元并重新加载守护进程（`systemctl daemon-reload`）。

#### 关于 Sphinx API 协议和 TFO 的技术细节
<details>
传统 Sphinx 协议分为两个阶段：握手交换和数据流。握手阶段由客户端发送的 4 字节数据包和守护进程返回的 4 字节数据包组成，唯一作用是客户端确认远程是正真的 Sphinx 守护进程，守护进程确认远程是正真的 Sphinx 客户端。主要数据流很简单：双方都声明握手，另一方验证。这个短包交换使用特殊的 `TCP_NODELAY` 标志，禁用 Nagle TCP 算法，声明 TCP 连接将作为小包对话进行。
然而，这个协商中并未严格规定谁先发言。历史上，所有使用二进制 API 的客户端均先行：发送握手，再从守护进程读 4 字节，接着发送请求，并读取守护进程响应。
在提升 Sphinx 协议兼容性时，我们考虑了以下几点：

1. 通常，主-代理通信由已知客户端连接到已知主机的已知端口。因此，端点发送错误握手的可能性很小。基于此，我们可以隐式假设双方都有效且真正使用 Sphinx 协议。
2. 基于此假设，我们可以将握手“粘”到真实请求中，一起以一个数据包发送。如果后端是传统 Sphinx 守护进程，它会先读作 4 字节握手包，再读取请求体。因为它们都在一个包内，后端套接字减少了一次往返时间（RTT），前端缓冲也能正常工作。
3. 进一步假设：由于“查询”包较小，握手更小，我们可以利用现代 TFO（tcp-fast-open）技术，将两者在初始 TCP “SYN” 包中一起发送。即连接远程节点时，将握手与正文融为一个包发送。守护进程接受连接时即可在套接字缓冲区里同时获得握手和数据，因为都来自第一个 TCP “SYN” 包，从而省去又一次 RTT。
4. 最后，教守护进程接受此改进。实际上，从应用程序的角度来看，这意味着不使用 `TCP_NODELAY`。而从系统方面来看，这意味着确保守护进程端激活接受 TFO，客户端也激活发送 TFO。现代系统默认情况下，客户端的 TFO 已经默认激活，所以你只需调整服务器的 TFO，所有功能即可正常运行。

所有这些改进在未实际更改协议本身的情况下，使我们能够消除连接中的 1.5 RTT 的 TCP 协议时间。如果查询和响应能够放在一个单一的 TCP 包中，将会使整个二进制 API 会话从 3.5 RTT 降低到 2 RTT —— 这使得网络协商速度提高了大约两倍。

因此，我们所有的改进都围绕一个最初未定义的声明展开：“谁先说话”。如果客户端先说话，我们可以应用所有这些优化，有效地在一个 TFO 包中处理连接 + 握手 + 查询。此外，我们可以查看接收包的开头以确定真实的协议。这就是为什么你可以通过 API/http/https 连接到同一个端口的原因。如果守护进程必须先说话，所有这些优化是不可能的，多协议也不可能。这就是为什么我们为 MySQL 设置了专用端口，而没有将其与其他所有协议合并为同一个端口。突然地，在所有客户端中，有一个客户端被编写为假定守护进程应先发送握手包。也就是说，没有可能应用上述所有改进。那就是 mysql/mariadb 的 SphinxSE 插件。因此，我们专门为这个单一的客户端专门定义了 `sphinx` 协议定义，以实现最遗留的工作方式。具体来说：双方都激活 `TCP_NODELAY` 并交换小包。守护进程在连接时发送握手包，然后客户端发送自己的，随后一切以常规方式工作。这不是最优的，但至少能正常工作。如果你用 SphinxSE 连接到 Manticore —— 你必须为此专门配置一个明确声明了 `sphinx` 协议的监听器。对于其他客户端 —— 避免使用此监听器，因为它较慢。如果你使用其他遗留 Sphinx API 客户端 —— 先检查它们是否能够与非专用的多协议端口正常工作。对于使用非专用（多协议）端口且启用客户端和服务器端 TFO 的主从链接，它工作良好，且肯定会加快网络后台的运行速度，特别是如果你有非常轻量且迅速的查询。
</details>

### listen_tfo

此设置允许为所有监听器启用 TCP_FASTOPEN 标志。默认情况下，由系统管理，但可以通过设置为“0”明确关闭。

关于 TCP Fast Open 扩展的通用知识，请参阅 [Wikipedia](https://en.wikipedia.org/wiki/TCP_Fast_Open)。简而言之，它允许在建立连接时消除一次 TCP 往返。

在实际应用中，在许多情况下使用 TFO 可优化客户端与代理之间的网络效率，就像使用了 [持久代理](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) ，但没有保持活动连接，也没有最大连接数的限制。

在现代操作系统上，TFO 支持通常默认已在系统级别开启，但这只是“能力”而非规则。Linux（作为最先进系统）自2011年开始支持，从内核版本3.7起支持服务器端。Windows 从 Windows 10 部分版本开始支持此功能。其他操作系统（FreeBSD，MacOS）也已加入支持行列。

在 Linux 系统中，服务器检查变量 `/proc/sys/net/ipv4/tcp_fastopen` 并根据其行为操作。位 0 控制客户端，位 1 控制监听器。默认情况下，系统设置为 1，即客户端启用，监听器禁用。

### log

<!-- example conf log -->
log 设置指定了所有 `searchd` 运行时事件记录的日志文件名。如果未指定，默认名称为 'searchd.log'。

或者，可以使用 'syslog' 作为文件名。在这种情况下，事件将发送到 syslog 守护进程。要使用 syslog 选项，构建时需要使用 `-–with-syslog` 选项配置 Manticore。


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

使 searchd 对使用 [多查询](../Searching/Multi-queries.md) 时单批提交的查询数量进行合理性检查。设置为 0 则跳过检查。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_batch_queries = 256
```
<!-- end -->

### max_connections

<!-- example max_connections -->
最大同时客户端连接数。默认无限制。通常仅在使用任何形式的持久连接时显著，例如 cli mysql 会话或远程分布式表的持久远程连接。超过限制后，仍可使用 [VIP 连接](../Connecting_to_the_server/MySQL_protocol.md#VIP-connection) 连接服务器。VIP 连接不计入限制。

<!-- request Example -->
```ini
max_connections = 10
```

<!-- end -->

### max_threads_per_query

<!-- example max_threads_per_query -->
实例级别一个操作可使用的线程数上限。默认情况下，相关操作可占用所有 CPU 核心，导致其他操作无资源。例如，对相当大的回溯表执行 `call pq` 可能会占用所有线程数十秒。将 `max_threads_per_query` 设置为，例如 [threads](../Server_settings/Searchd.md#threads) 的一半，将保证可以并行运行几个这样的 `call pq` 操作。

你也可以在运行时将此设置设为会话或全局变量。

此外，可以通过 [threads OPTION](../Searching/Options.md#threads) 基于每个查询控制行为。

<!-- intro -->
##### 示例：
<!-- request Example -->

```ini
max_threads_per_query = 4
```

<!-- end -->

### max_filters

<!-- example conf max_filters -->
每个查询允许的最大过滤器数量。此设置仅用于内部健全性检查，不直接影响内存使用或性能。可选，默认值为256。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_filters = 1024
```
<!-- end -->


### max_filter_values

<!-- example conf max_filter_values -->
每个过滤器允许的最大值数量。此设置仅用于内部健全性检查，不直接影响内存使用或性能。可选，默认值为4096。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_filter_values = 16384
```
<!-- end -->


### max_open_files

<!-- example conf max_open_files -->
服务器允许打开的最大文件数称为“软限制”。请注意，提供大型分段的实时表可能需要将该限制设置得较高，因为每个磁盘块可能占用十几个或更多文件。例如，一个拥有1000个块的实时表可能需要同时打开数千个文件。如果日志中出现“打开的文件过多”错误，请尝试调整此选项，这可能有助于解决问题。

还有一个“硬限制”，选项无法超过此限制。该限制由系统定义，可以在Linux上的 `/etc/security/limits.conf` 文件中更改。其他操作系统可能有不同的方法，请参考相应手册获取更多信息。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = 10000
```
<!-- end -->

<!-- example conf max_open_files max -->
除了直接的数值，还可以使用魔法词“max”将限制设置为当前可用的硬限制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = max
```
<!-- end -->


### max_packet_size

<!-- example conf max_packet_size -->
允许的最大网络数据包大小。此设置限制来自客户端的查询数据包以及分布式环境中远程代理的响应数据包。仅用于内部健全性检查，不直接影响内存使用或性能。可选，默认值为128M。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_packet_size = 32M
```
<!-- end -->


### mysql_version_string

<!-- example conf mysql_version_string -->
通过MySQL协议返回的服务器版本字符串。可选，默认值为空（返回Manticore版本）。

一些严格的MySQL客户端库依赖于MySQL使用的特定版本号格式，且有时根据报告的版本号（而非显示的功能标志）选择不同的执行路径。例如，Python MySQLdb 1.2.2 在版本号不是X.Y.ZZ格式时会抛出异常；MySQL .NET连接器6.3.x在版本号为1.x且带有某组合标志时内部失败等。为了解决这个问题，你可以使用 `mysql_version_string` 指令，让 `searchd` 向通过MySQL协议连接的客户端报告不同的版本号。（默认是报告其自身版本。）


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
mysql_version_string = 5.0.37
```
<!-- end -->


### net_workers

网络线程数量，默认值为1。

当查询速率极高时，单线程可能不足以管理所有传入的查询，此设置非常有用。


### net_wait_tm

控制网络线程的忙循环间隔。默认值为-1，可设置为-1、0或正整数。

当服务器配置为纯主节点并仅将请求路由给代理时，重要的是无延迟处理请求，不允许网络线程休眠。为此设置了忙循环。对于传入请求，若`net_wait_tm`为正数，网络线程使用CPU轮询`10 * net_wait_tm`毫秒；若为0，则仅使用CPU轮询。忙循环也可以通过设置`net_wait_tm = -1`禁用——这时，轮询器在系统轮询调用中将超时设置为实际代理的超时。

> **警告：** CPU忙循环实际上会加载CPU核心，因此将此值设置为非默认值即使服务器空闲，也会导致明显的CPU使用率。


### net_throttle_accept

定义在网络循环的每次迭代中接受的客户端数量。默认值为0（无限制），对于大多数用户足够。此为高负载场景下调节网络循环吞吐量的微调选项。


### net_throttle_action

定义在网络循环的每次迭代中处理的请求数量。默认值为0（无限制），对于大多数用户足够。此为高负载场景下调节网络循环吞吐量的微调选项。

### network_timeout

<!-- example conf network_timeout -->
网络客户端请求读写超时，单位为秒（或[special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认值为5秒。`searchd`将强制关闭在此超时内未发送查询或未读到结果的客户端连接。

另请注意[reset_network_timeout_on_packet](../Server_settings/Searchd.md#reset_network_timeout_on_packet)参数。该参数将`network_timeout`的行为从应用于整个`query`或`result`改为单个数据包。通常，查询/结果可以包含在一两个数据包内，但需要大量数据时，此参数对于维持活动操作非常有用。

<!-- request Example -->

```ini
network_timeout = 10s
```
<!-- end -->

### node_address

<!-- example conf node_address -->
此设置允许指定节点的网络地址。默认情况下，它设置为复制[listen](../Server_settings/Searchd.md#listen)地址。在大多数情况下这都是正确的；但是，有些情况下需要手动指定：

* 节点处于防火墙后面
* 启用了网络地址转换（NAT）
* 容器部署，如Docker或云部署
* 节点分布在多个区域的集群


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
node_address = 10.101.0.10
```
<!-- end -->

### not_terms_only_allowed

<!-- example conf not_terms_only_allowed -->
此设置决定是否允许仅使用[否定](../Searching/Full_text_matching/Operators.md#Negation-operator)全文运算符的查询。可选，默认值为0（仅包含NOT运算符的查询失败）。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
not_terms_only_allowed = 1
```
<!-- end -->

### optimize_cutoff

<!-- example conf optimize_cutoff -->
设置默认的表压缩阈值。详情请参见 - [优化磁盘块的数量](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks)。此设置可以通过每查询选项 [cutoff](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 重写。也可以通过 [SET GLOBAL](../Server_settings/Setting_variables_online.md#SET) 动态更改。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
optimize_cutoff = 4
```
<!-- end -->

### persistent_connections_limit

<!-- example conf persistent_connections_limit -->
此设置决定到远程[持久代理](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)的最大同时持久连接数。每当连接`agent_persistent`下定义的代理时，我们尝试重用已有连接（如果有），或者连接并保存该连接以备将来使用。然而，有时限制此类持久连接数是有意义的。该指令定义该限制。它影响所有分布式表中到每个代理主机的连接数。

合理的做法是将该值设置为不大于代理配置中的[max_connections](../Server_settings/Searchd.md#max_connections)选项。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
persistent_connections_limit = 29 # assume that each host of agents has max_connections = 30 (or 29).
```
<!-- end -->


### pid_file

<!-- example conf pid_file -->
pid_file 是 Manticore 搜索中的一个必选配置选项，用于指定存储 `searchd` 服务器进程 ID 的文件路径。

searchd 进程 ID 文件在启动时重新创建并加锁，服务器运行期间包含主服务器进程 ID，服务器关闭时文件将被删除。
该文件的目的是让 Manticore 执行各种内部任务，如检查是否已有正在运行的 `searchd` 实例，停止 `searchd`，以及通知其应旋转表。该文件也可用于外部自动化脚本。


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
基于最大查询时间设置，在查询完成前终止查询是一种良好的安全保障，但存在缺陷：结果不确定（不稳定）。也就是说，如果多次运行同一（复杂）搜索查询并设时间限制，时间限制可能在不同阶段被触发，结果集也会不同。

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

有一个选项，[SELECT … OPTION max_predicted_time](../Searching/Options.md#max_predicted_time)，它允许限制查询时间并获得稳定、可重复的结果。它不是定期检查实际的当前时间（这会带来不确定性），而是使用一个简单的线性模型预测当前运行时间：

```ini
predicted_time =
    doc_cost * processed_documents +
    hit_cost * processed_hits +
    skip_cost * skiplist_jumps +
    match_cost * found_matches
```

当 `predicted_time` 达到指定限额时，查询即提前终止。

当然，这不是实际时间的硬限制（但确实是*处理*工作量的硬限制），且简单的线性模型并不完美精准。因此实际墙钟时间可能低于或超过目标限额。但误差幅度相当可接受：比如我们针对100毫秒限额的实验中，大多数测试查询在95到105毫秒范围内，*所有*查询范围为80到120毫秒。此外，使用模型预测查询时间而非测量实际运行时间，调用 gettimeofday() 的次数稍有减少。

由于不同服务器品牌和型号各异，`predicted_time_costs` 指令允许配置上述模型的成本值。为方便起见，它们是以纳秒为单位的整数。（max_predicted_time 的限制以毫秒计，若要指定成本值为0.000128毫秒而非128纳秒，容易出错。）不必同时指定所有四个成本值，缺失的将使用默认值。但强烈建议为可读性指定全部四项。


### preopen_tables

<!-- example conf preopen_tables -->
preopen_tables 配置指令指定是否在启动时强制预打开所有表。默认值为1，表示所有表都会被预打开，无论单表的 [preopen](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Other-performance-related-settings) 设置如何。如果设为0，则单表设置生效，默认值为0。

预打开表可以防止查询与表旋转间的竞争条件，避免查询偶尔失败。但这会使用更多文件句柄。在大多数场景下，推荐预打开表。

示例如下：

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
preopen_tables = 1
```
<!-- end -->

### pseudo_sharding

<!-- example conf pseudo_sharding -->
pseudo_sharding 配置选项启用对本地普通表和实时表的搜索查询的并行化，无论是直接查询还是通过分布式表查询。此功能将自动将查询并行化，最多可达到 `searchd.threads` 中指定的线程数。

请注意，如果您的工作线程已经很忙，可能是因为：
* 高查询并发
* 任何形式的物理分片：
  - 多个普通/实时表的分布式表
  - 包含太多磁盘块的实时表

那么启用 pseudo_sharding 可能不会带来任何好处，甚至可能导致吞吐量略有下降。如果您优先考虑更高的吞吐量而非更低的延迟，建议禁用此选项。

默认启用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
pseudo_sharding = 0
```
<!-- end -->


### replication_connect_timeout

`replication_connect_timeout` 指令定义了连接远程节点的超时。默认情况下，值被认为是毫秒，但也可以带有[其他后缀](../Server_settings/Special_suffixes.md)。默认值为 1000（1 秒）。

连接远程节点时，Manticore 最多等待该时间以成功完成连接。如果达到超时但连接尚未建立，并且启用了 `retries`，则将启动重试。


### replication_query_timeout

`replication_query_timeout` 设置 searchd 等待远程节点完成查询的时间。默认值为 3000 毫秒（3 秒），但可以使用后缀来指示不同的时间单位。

建立连接后，Manticore 将等待远程节点完成查询的最大时间为 `replication_query_timeout`。请注意，此超时与 `replication_connect_timeout` 分开计算，远程节点可能造成的总延迟是这两个值之和。


### replication_retry_count

此设置为整数，指定在报告致命查询错误之前，Manticore 在复制过程中尝试连接和查询远程节点的次数。默认值为 3。


### replication_retry_delay

此设置为整数，单位为毫秒（或[特殊后缀](../Server_settings/Special_suffixes.md)），指定在复制过程中查询远程节点失败后重试前的延迟时间。该值仅在非零值时有效。默认值为 500。

### qcache_max_bytes

<!-- example conf qcache_max_bytes -->
此配置设置用于缓存结果集的最大 RAM（字节）。默认值为 16777216，相当于 16 兆字节。如果值设置为 0，则禁用查询缓存。有关查询缓存的详细信息，请参见[查询缓存](../Searching/Query_cache.md)。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
qcache_max_bytes = 16777216
```
<!-- end -->


### qcache_thresh_msec

整数，单位为毫秒。查询结果被缓存的最小壁钟时间阈值。默认值为 3000，即 3 秒。0 表示缓存所有结果。详情请参阅[查询缓存](../Searching/Query_cache.md)。此值也可用时间[特殊后缀](../Server_settings/Special_suffixes.md)表示，但请谨慎使用，并不要与值名称本身包含的 '_msec' 混淆。


### qcache_ttl_sec

整数，单位为秒。缓存结果集的过期时间。默认值为 60，即 1 分钟。最小值为 1 秒。详情请参阅[查询缓存](../Searching/Query_cache.md)。此值也可用时间[特殊后缀](../Server_settings/Special_suffixes.md)表示，但请谨慎使用，并不要与值名称本身包含的 '_sec' 混淆。


### query_log_format

<!-- example conf query_log_format -->
查询日志格式。可选值为 `plain` 和 `sphinxql`，默认是 `sphinxql`。

`sphinxql` 模式记录有效的 SQL 语句。`plain` 模式以纯文本格式记录查询（主要适用于纯全文搜索用例）。此指令允许您在搜索服务器启动时切换两种格式。日志格式也可以通过 `SET GLOBAL query_log_format=sphinxql` 语法动态更改。详情请参阅[查询日志](../Logging/Query_logging.md)。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_format = sphinxql
```
<!-- end -->

### query_log_min_msec

限制（毫秒），防止查询被写入查询日志。可选，默认值为 0（所有查询均写入查询日志）。此指令指定只有执行时间超过该限制的查询才会被记录（此值也可用时间[特殊后缀](../Server_settings/Special_suffixes.md)表示，但请谨慎使用，并不要与值名称本身包含的 `_msec` 混淆）。

### query_log

<!-- example conf query_log -->
查询日志文件名。可选，默认为空（不记录查询）。所有搜索查询（例如 SELECT，但不包括 INSERT/REPLACE/UPDATE 查询）都会记录在该文件中。格式详见[查询日志](../Logging/Query_logging.md)。如果采用 'plain' 格式，路径可以使用 'syslog'，此时所有查询将被发送到 syslog 守护进程，优先级为 `LOG_INFO`，并用 '[query]' 前缀替代时间戳。使用 syslog 选项时，Manticore 必须在编译时配置 `-–with-syslog`。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log = /var/log/query.log
```
<!-- end -->


### query_log_mode

<!-- example conf query_log_mode -->
`query_log_mode` 指令允许您为 searchd 和查询日志文件设置不同的权限。默认情况下，这些日志文件的权限为 600，意味着只有运行服务器的用户和 root 用户可以读取日志文件。
该指令在您希望允许其他用户读取日志文件时非常有用，例如，运行在非root用户上的监控解决方案。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_mode  = 666
```
<!-- end -->

### read_buffer_docs

<!-- example conf read_buffer_docs -->
read_buffer_docs 指令控制文档列表的每关键词读取缓冲区大小。对于每个搜索查询中的每个关键词出现，存在两个相关的读取缓冲区：一个用于文档列表，一个用于命中列表。此设置允许您控制文档列表缓冲区大小。

较大的缓冲区大小可能会增加每次查询的内存使用，但有可能减少I/O时间。对于较慢的存储设备，设置较大的值是合理的，但对于能够实现高IOPS的存储设备，应在较小值区域进行实验。

默认值为256K，最小值为8K。您还可以为每个表设置[read_buffer_docs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs)，这将覆盖服务器配置级别的设置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_docs = 128K
```
<!-- end -->


### read_buffer_hits

<!-- example conf read_buffer_hits -->
read_buffer_hits 指令指定搜索查询中命中列表的每关键词读取缓冲区大小。默认大小为256K，最小值为8K。对于搜索查询中的每个关键词出现，存在两个相关的读取缓冲区，一个用于文档列表，一个用于命中列表。增加缓冲区大小可以增加每次查询的内存使用，但减少I/O时间。对于较慢存储设备，较大的缓冲区大小是合理的，而对于能够实现高IOPS的存储设备，应在较小值区域进行实验。

此设置也可以使用[read_buffer_hits](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_hits)中的选项按表设置，这将覆盖服务器级别的设置。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_hits = 128K
```
<!-- end -->

### read_unhinted

<!-- example conf read_unhinted -->
未经提示的读取大小。可选，默认值为32K，最小为1K。

在查询时，有些读取操作提前知道确切的数据大小，但有些目前不知道。最典型的是，命中列表大小目前无法提前知道。此设置让您控制在这类情况下读取多少数据。它影响命中列表的I/O时间，对于大于未经提示读取大小的列表可减少时间，但会增加较小列表的时间。它**不会**影响内存使用，因为读取缓冲区已经分配好了。因此，它不应大于read_buffer。


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

设置为0时，超时限制整个请求/查询的最大发送时间。
设置为1（默认）时，超时限制网络活动之间的最大时间。

在复制中，节点可能需要向另一个节点发送大型文件（例如100GB）。假设网络传输速度为1GB/s，分多个4-5MB数据包传送。传输整个文件需要100秒。默认5秒超时只允许传输5GB数据，超过则断开连接。增加超时虽可行但不具扩展性（如下一个文件是150GB又会失败）。但默认`reset_network_timeout_on_packet`设置为1时，超时应用于单个数据包，只要传输持续（且超时期间网络实际接收数据），连接保持存活。若传输停滞发生超时，则断开连接。

注意，如果您设置了分布式表，主节点和代理节点都应进行调优。主端影响`agent_query_timeout`，代理端影响`network_timeout`。

<!-- intro -->

##### 示例：

<!-- request Example -->

```ini
reset_network_timeout_on_packet = 0
```

<!-- end -->


### rt_flush_period

<!-- example conf rt_flush_period -->
RT表RAM块刷新检查周期，单位为秒（或[特殊后缀](../Server_settings/Special_suffixes.md)）。可选，默认是10小时。

完全适合RAM块的主动更新RT表仍可能导致不断增长的二进制日志，影响磁盘使用和崩溃恢复时间。通过此指令，搜索服务器定期执行刷新检查，符合条件的RAM块可被保存，从而允许后续的二进制日志清理。详见[二进制日志](../Logging/Binary_logging.md)。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_flush_period = 3600 # 1 hour
```
<!-- end -->


### rt_merge_iops

<!-- example conf rt_merge_iops -->
RT块合并线程允许启动的最大I/O操作次数（每秒）。可选，默认值为0（无限制）。

该指令允许您限制由`OPTIMIZE`语句引起的I/O影响。保证所有RT优化操作不会生成超过配置限制的磁盘IOPS。限制rt_merge_iops可以减少合并时引起的搜索性能下降。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_iops = 40
```
<!-- end -->

### rt_merge_maxiosize

<!-- example conf rt_merge_maxiosize -->
RT块合并线程允许启动的最大I/O操作大小。可选，默认值为0（无限制）。

此指令允许您限制由 `OPTIMIZE` 语句产生的 I/O 影响。大于此限制的 I/O 将被拆分成两个或多个 I/O，对于 [rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops) 限制将分别计算。这确保所有优化活动每秒产生的不超过 (rt_merge_iops * rt_merge_maxiosize) 字节的磁盘 I/O。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_maxiosize = 1M
```
<!-- end -->


### seamless_rotate

<!-- example conf seamless_rotate -->
防止在旋转包含大量数据的表以进行预缓存时导致 `searchd` 停滞。可选，默认值为 1（启用无缝旋转）。在 Windows 系统上，默认禁用无缝旋转。

表可能包含需要预缓存到 RAM 的一些数据。目前 `.spa`、`.spb`、`.spi` 和 `.spm` 文件被完全预缓存（它们分别包含属性数据、blob 属性数据、关键字表和已删除行映射）。如果不启用无缝旋转，旋转表时会尽可能少用 RAM，操作步骤如下：

1. 暂时拒绝新的查询（返回“重试”错误代码）；
2. `searchd` 等待所有当前运行的查询完成；
3. 旧表被释放，其文件被重命名；
4. 新表文件被重命名，并分配所需的 RAM；
5. 新表的属性和字典数据被预加载到 RAM；
6. `searchd` 恢复处理新表的查询。

然而，如果属性或字典数据量很大，那么预加载步骤可能耗时显著——预加载 1-5+ GB 文件的情况可能持续几分钟。

启用无缝旋转后，旋转操作如下：

1. 分配新表的 RAM 存储；
2. 异步预加载新表的属性和字典数据到 RAM；
3. 预加载成功后，释放旧表，同时重命名两个表的文件；
4. 预加载失败时，释放新表；
5. 在任一时刻，查询要么由旧表要么由新表副本提供服务。

无缝旋转的代价是旋转期间峰值内存使用增加（因为预加载新副本时需要同时在 RAM 中存在旧副本和新副本的 `.spa/.spb/.spi/.spm` 数据）。平均使用量保持不变。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
seamless_rotate = 1
```
<!-- end -->

### secondary_indexes
<!-- example conf secondary_indexes -->

此选项用于启用/禁用搜索查询中二级索引的使用。可选，默认值为 1（启用）。请注意，索引建立时无需启用此选项，只要安装了 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)，其始终启用。该库也是进行搜索时使用索引的前提。提供三种模式：

* `0`：禁用搜索中二级索引。可以针对单个查询使用[分析器提示](../Searching/Options.md#Query-optimizer-hints)启用；
* `1`：启用搜索中二级索引。可以针对单个查询使用[分析器提示](../Searching/Options.md#Query-optimizer-hints)禁用；
* `force`：同 `1`，但加载二级索引时出现错误会被报告，并且整个索引不会加载到守护进程。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
secondary_indexes = 1
```

<!-- end -->

### server_id

<!-- example conf server_id -->
整数，作为服务器标识符，用于生成复制集群节点的唯一短 UUID 的种子。server_id 在集群节点间必须唯一，且范围为 0 到 127。如果未设置 server_id，则计算为 MAC 地址和 PID 文件路径的哈希值，或使用随机数作为短 UUID 的种子。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
server_id = 1
```
<!-- end -->


### shutdown_timeout

<!-- example conf shutdown_timeout -->
`searchd --stopwait` 等待时间，单位为秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认是 60 秒。

当您运行 `searchd --stopwait` 时，服务器需要在停止前执行一些操作，如完成查询、刷新 RT RAM 块、刷新属性和更新 binlog。这些任务需要一定时间。`searchd --stopwait` 将最多等待 `shutdown_time` 秒，以便服务器完成任务。合适的时间取决于您的表的大小和负载。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
shutdown_timeout = 3m # wait for up to 3 minutes
```
<!-- end -->


### shutdown_token

调用 VIP Manticore SQL 连接的 'shutdown' 命令所需的密码 SHA1 哈希。没有它，[debug](../Reporting_bugs.md#DEBUG) 'shutdown' 子命令永远不会导致服务器停止。注意，这样的简单哈希不应被视为强保护，因为我们未使用加盐哈希或任何现代哈希函数。它旨在作为本地网络内综合管理守护进程的防呆措施。

### snippets_file_prefix

<!-- example conf snippets_file_prefix -->
生成片段时，前置于本地文件名的前缀。可选，默认是当前工作目录。

此前缀可与 `load_files` 或 `load_files_scattered` 选项结合用于分布式片段生成。

请注意，这只是前缀，**不是**路径！这意味着如果前缀设置为 "server1"，请求引用 "file23" 时，`searchd` 会尝试打开 "server1file23"（不含引号）。因此，如果您需要它作为路径，必须包含末尾斜杠。

构建最终文件路径后，服务器会展开所有相对目录，并将最终结果与 `snippet_file_prefix` 的值比对。如果结果未以该前缀开始，则此文件会被拒绝，并显示错误信息。

例如，如果设置为 `/mnt/data`，而有人用文件 `../../../etc/passwd` 作为来源调用片段生成，则会收到错误信息：

`文件 '/mnt/data/../../../etc/passwd' 超出了 '/mnt/data/' 范围`

而不是文件的内容。

另外，对于未设置参数且读取 `/etc/passwd` 的情况，它实际上会读取 /daemon/working/folder/etc/passwd，因为该参数的默认值是服务器的工作文件夹。

另请注意，这是一个本地选项；它不会以任何方式影响代理。因此，您可以安全地在主服务器上设置前缀。路由到代理的请求将不受主服务器设置的影响。但它们会受到代理自身设置的影响。

例如，当文档存储位置（无论是本地存储还是 NAS 挂载点）在服务器之间不一致时，这可能很有用。


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
当前 SQL 状态将被序列化到的文件路径。

服务器启动时，该文件会被重新执行。在符合条件的状态更改时（例如，SET GLOBAL），该文件会自动重写。这可以防止一个难以诊断的问题：如果加载了 UDF 函数但 Manticore 崩溃，自动重启时，您的 UDF 和全局变量将不再可用。使用持久状态有助于确保顺利恢复，避免此类意外。

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
使用 SQL 接口时，两次请求之间的最大等待时间（以秒为单位，或使用 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认是 15 分钟。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
sphinxql_timeout = 15m
```
<!-- end -->


### ssl_ca

<!-- example conf ssl_ca -->
SSL 证书颁发机构 (CA) 证书文件路径（也称为根证书）。可选，默认为空。当不为空时，`ssl_cert` 中的证书应由此根证书签名。

服务器使用 CA 文件验证证书上的签名。文件必须为 PEM 格式。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ssl_ca = keys/ca-cert.pem
```
<!-- end -->


### ssl_cert

<!-- example conf ssl_cert -->
服务器的 SSL 证书路径。可选，默认为空。

服务器使用此证书作为自签名公钥，对通过 SSL 的 HTTP 流量进行加密。文件必须为 PEM 格式。


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

服务器使用此私钥对通过 SSL 的 HTTP 流量进行加密。文件必须为 PEM 格式。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ssl_key = keys/server-key.pem
```
<!-- end -->


### subtree_docs_cache

<!-- example conf subtree_docs_cache -->
每个查询的最大公共子树文档缓存大小。可选，默认是 0（禁用）。

此设置限制公共子树优化器（参见 [multi-queries](../Searching/Multi-queries.md)）的内存使用量。最多为每个查询缓存这么多文档条目。将限制设置为 0 禁用此优化器。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
subtree_docs_cache = 8M
```
<!-- end -->


### subtree_hits_cache

<!-- example conf subtree_hits_cache -->
每个查询的最大公共子树匹配缓存大小。可选，默认是 0（禁用）。

此设置限制公共子树优化器（参见 [multi-queries](../Searching/Multi-queries.md)）的内存使用量。最多为每个查询缓存这么多关键词出现（匹配项）。将限制设置为 0 禁用此优化器。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
subtree_hits_cache = 16M
```
<!-- end -->

### threads

<!-- example threads -->
Manticore 守护进程的工作线程数（或线程池大小）。Manticore 启动时创建此数量的操作系统线程，它们执行守护进程内部的所有工作，如执行查询、创建片段等。一些操作可能被拆分为子任务并行执行，例如：

* 在实时表中搜索
* 在由本地表构成的分布式表中搜索
* 触发查询调用
* 以及其他

默认情况下，它设置为服务器的 CPU 核数。Manticore 启动时创建线程，直到停止时一直保持。每个子任务需要时可使用其中一个线程。子任务完成后释放线程，供其他子任务使用。

在密集 I/O 类型负载情况下，设置值高于 CPU 核数可能更合理。

<!-- request Example -->
```ini
threads = 10
```

<!-- end -->

### thread_stack

<!-- example conf thread_stack -->
每个作业的最大堆栈大小（协程，一个搜索查询可能导致多个作业/协程）。可选，默认是 128K。

每个作业拥有自己的 128K 堆栈。运行查询时，会检查所需堆栈大小。如果默认的 128K 足够，则直接处理；如果需要更多，则安排另一个具有更大堆栈的作业继续处理。此高级堆栈的最大大小由此设置限制。

将此值设置为合理较高的值，有助于处理非常深层的查询，同时不意味着整体内存消耗会过高。例如，设置为 1G 并不意味着每个新作业都会占用 1G 内存，但如果发现它需要约 100M 堆栈，则仅为该作业分配 100M。其他作业仍以其默认的 128K 堆栈运行。同理，我们可以运行需要 500M 堆栈的更复杂查询。只有当内部**检测**到作业需要超过 1G 堆栈时，才会失败并报告线程堆栈不足。

然而，实际上，即使是需要16M堆栈的查询，通常也过于复杂，难以解析，并且消耗过多时间和资源处理。因此，守护进程会处理它，但通过`thread_stack`设置限制此类查询看起来相当合理。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
thread_stack = 8M
```
<!-- end -->


### unlink_old

<!-- example conf unlink_old -->
决定在成功轮换时是否取消链接`.old`表副本。可选，默认值为1（执行取消链接）。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
unlink_old = 0
```
<!-- end -->


### watchdog

<!-- example conf watchdog -->
线程服务器看门狗。可选，默认值为1（启用看门狗）。

当Manticore查询崩溃时，可能会导致整个服务器崩溃。启用看门狗功能后，`searchd`还会维护一个独立的轻量级进程，监控主服务器进程，并在异常终止时自动重启它。看门狗默认启用。

<!-- request Example -->

```ini
watchdog = 0 # disable watchdog
```
<!-- end -->
<!-- proofread -->

