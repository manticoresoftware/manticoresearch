# 配置中的 "Searchd" 部分

以下设置用于 Manticore Search 配置文件的 `searchd` 部分，以控制服务器的行为。以下是每个设置的摘要：

### access_plain_attrs

此设置为 [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。此项为可选，默认值为 `mmap_preread`。

`access_plain_attrs` 指令允许您为此 searchd 实例管理的所有表定义 [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令优先级更高，会覆盖此实例范围的默认值，从而提供更细粒度的控制。

### access_blob_attrs

此设置为 [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。此项为可选，默认值为 `mmap_preread`。

`access_blob_attrs` 指令允许您为此 searchd 实例管理的所有表定义 [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令优先级更高，会覆盖此实例范围的默认值，从而提供更细粒度的控制。

### access_doclists

此设置为 [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。此项为可选，默认值为 `file`。

`access_doclists` 指令允许您为此 searchd 实例管理的所有表定义 [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令优先级更高，会覆盖此实例范围的默认值，从而提供更细粒度的控制。

### access_hitlists

此设置为 [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。此项为可选，默认值为 `file`。

`access_hitlists` 指令允许您为此 searchd 实例管理的所有表定义 [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令优先级更高，会覆盖此实例范围的默认值，从而提供更细粒度的控制。

### access_dict

此设置为 [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。此项为可选，默认值为 `mmap_preread`。

`access_dict` 指令允许您为此 searchd 实例管理的所有表定义 [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令优先级更高，会覆盖此实例范围的默认值，从而提供更细粒度的控制。

### agent_connect_timeout

此设置为 [agent_connect_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) 参数设置实例范围的默认值。


### agent_query_timeout

此设置为 [agent_query_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) 参数设置实例范围的默认值。可以通过 `OPTION agent_query_timeout=XXX` 子句在每个查询级别覆盖。


### agent_retry_count

此设置为整数，指定 Manticore 在报告致命查询错误之前，通过分布式表尝试连接和查询远程代理的次数。默认值为 0（即不重试）。您也可以通过 `OPTION retry_count=XXX` 子句在每个查询级别设置此值。如果提供了每查询选项，它将覆盖配置中指定的值。

请注意，如果您在分布式表定义中使用了 [agent mirrors](../Creating_a_cluster/Remote_nodes/Mirroring.md#Agent-mirrors)，服务器将根据所选的 [ha_strategy](../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) 为每次连接尝试选择不同的镜像。在这种情况下，`agent_retry_count` 将对集合中的所有镜像进行汇总。

例如，如果您有 10 个镜像并设置 `agent_retry_count=5`，服务器将最多重试 50 次，假设每个镜像平均尝试 5 次（使用 `ha_strategy = roundrobin` 选项时即为如此）。

然而，作为 [agent](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) 的 `retry_count` 选项提供的值是绝对限制。换句话说，代理定义中的 `[retry_count=2]` 选项始终意味着最多尝试 2 次，无论您为代理指定了 1 个还是 10 个镜像。

### agent_retry_delay

此设置为以毫秒为单位的整数（或 [special_suffixes](../Server_settings/Special_suffixes.md)），指定 Manticore 在失败时重试查询远程代理之前的延迟时间。仅当指定非零的 [agent_retry_count](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 或非零的每查询 `retry_count` 时，此值才有意义。默认值为 500。您也可以通过 `OPTION retry_delay=XXX` 子句在每个查询级别设置此值。如果提供了每查询选项，它将覆盖配置中指定的值。


### attr_flush_period

<!-- example conf attr_flush_period -->
当使用 [Update](../Data_creation_and_modification/Updating_documents/UPDATE.md) 实时修改文档属性时，变更首先写入属性的内存副本。这些更新发生在内存映射文件中，意味着操作系统决定何时将更改写入磁盘。在 `searchd` 正常关闭时（由 `SIGTERM` 信号触发），所有更改都会被强制写入磁盘。

您也可以指示 `searchd` 定期将这些更改写回磁盘以防止数据丢失。刷新间隔由 `attr_flush_period` 决定，单位为秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。

默认值为 0，表示禁用定期刷新。但在正常关闭时仍会进行刷新。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
attr_flush_period = 900 # persist updates to disk every 15 minutes
```
<!-- end -->

### auto_optimize

<!-- example conf auto_optimize -->
此设置控制表压缩的自动 [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 过程。

默认情况下，表压缩会自动进行。您可以通过 `auto_optimize` 设置修改此行为：
* 0 禁用自动表压缩（您仍然可以手动调用 `OPTIMIZE`）
* 1 显式启用自动表压缩
* 启用自动表压缩并将优化阈值乘以 2。

默认情况下，OPTIMIZE 会运行直到磁盘块数小于或等于逻辑 CPU 核心数乘以 2。

但是，如果表具有带 KNN 索引的属性，则阈值不同。在这种情况下，阈值设置为物理 CPU 核心数除以 2，以提升 KNN 搜索性能。

请注意，切换 `auto_optimize` 开关不会阻止您手动运行 [OPTIMIZE TABLE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE)。

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
Manticore 支持自动创建尚不存在但在 INSERT 语句中指定的表。此功能默认启用。要禁用它，请在配置中显式设置 `auto_schema = 0`。要重新启用，请设置 `auto_schema = 1` 或从配置中移除 `auto_schema` 设置。

请注意，`/bulk` HTTP 端点不支持自动创建表。

> 注意：[自动 schema 功能](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果功能无法使用，请确保 Buddy 已安装。

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
此设置控制二进制日志事务的刷新/同步模式。为可选项，默认值为 2（每个事务刷新，每秒同步一次）。

该指令决定二进制日志刷新到操作系统和同步到磁盘的频率。支持三种模式：

*  0，每秒刷新和同步一次。性能最佳，但在服务器崩溃或操作系统/硬件崩溃时，最多可能丢失 1 秒内的已提交事务。
*  1，每个事务刷新和同步一次。性能最差，但保证每个已提交事务的数据都被保存。
*  2，每个事务刷新，每秒同步一次。性能良好，确保服务器崩溃时每个已提交事务都被保存。但在操作系统/硬件崩溃时，最多可能丢失 1 秒内的已提交事务。

对于熟悉 MySQL 和 InnoDB 的用户，此指令类似于 `innodb_flush_log_at_trx_commit`。在大多数情况下，默认的混合模式 2 在速度和安全性之间提供了良好平衡，完全保护 RT 表数据免受服务器崩溃影响，并对硬件崩溃提供一定保护。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
binlog_flush = 1 # ultimate safety, low speed
```
<!-- end -->

### binlog_common

<!-- example conf binlog_common -->
此设置控制二进制日志文件的管理方式。为可选项，默认值为 0（每个表单独文件）。

您可以选择两种管理二进制日志文件的方式：

* 每个表单独文件（默认，`0`）：每个表将其更改保存到自己的日志文件中。此设置适合有许多表且更新时间不同的情况。它允许表独立更新，无需等待其他表。如果某个表的日志文件出现问题，也不会影响其他表。
* 所有表共用一个文件（`1`）：所有表使用同一个二进制日志文件。此方法便于文件管理，因为文件数量较少。但如果某个表仍需保存更新，文件可能会被保留时间较长。如果许多表同时需要更新，可能会导致性能下降，因为所有更改必须等待写入同一个文件。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
binlog_common = 1 # use a single binary log file for all tables
```
<!-- end -->

### binlog_max_log_size

<!-- example conf binlog_max_log_size -->
此设置控制二进制日志文件的最大大小。为可选项，默认值为 256 MB。

当当前二进制日志文件达到此大小限制时，将强制打开一个新的日志文件。这带来了更细粒度的日志划分，并且在某些边界工作负载下可以更高效地使用二进制日志磁盘空间。值为 0 表示不基于大小重新打开日志文件。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
binlog_max_log_size = 16M
```
<!-- end -->


### binlog_path

<!-- example conf binlog_path -->
此设置确定二进制日志（也称为事务日志）文件的路径。为可选项，默认值为构建时配置的数据目录（例如 Linux 下的 `/var/lib/manticore/data/binlog.*`）。

二进制日志用于RT表数据的崩溃恢复以及普通磁盘索引的属性更新，否则这些属性更新仅存储在RAM中直到刷新。启用日志记录时，每个提交到RT表的事务都会写入日志文件。在非正常关闭后启动时，日志会自动重放，恢复已记录的更改。

`binlog_path` 指令指定二进制日志文件的位置。它应仅包含路径；`searchd` 会根据需要在该目录中创建和删除多个 `binlog.*` 文件（包括binlog数据、元数据和锁文件等）。

空值禁用二进制日志记录，这会提升性能，但会使RT表数据面临风险。


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
此设置控制 [boolean_simplify](../Searching/Options.md#boolean_simplify) 搜索选项的默认值。该设置为可选，默认值为1（启用）。

设置为1时，服务器将自动应用 [布尔查询优化](../Searching/Full_text_matching/Boolean_optimization.md) 以提升查询性能。设置为0时，查询默认不进行优化。此默认值可通过对应的搜索选项 `boolean_simplify` 在每个查询中覆盖。

<!-- request Example -->
```ini
searchd {
    boolean_simplify = 0  # disable boolean optimization by default
}
```
<!-- end -->

### buddy_path

<!-- example conf buddy_path -->
此设置确定Manticore Buddy二进制文件的路径。该设置为可选，默认值为构建时配置的路径，不同操作系统有所不同。通常不需要修改此设置。但如果您希望以调试模式运行Manticore Buddy、对其进行修改或实现新插件，则可能需要修改。后者情况下，您可以从 https://github.com/manticoresoftware/manticoresearch-buddy 克隆Buddy，向 `./plugins/` 目录添加新插件，并在切换到Buddy源码目录后运行 `composer install --prefer-source` 以便于开发。

为确保能运行 `composer`，您的机器必须安装PHP 8.2或更高版本，并带有以下扩展：

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

如果采用此方式，记得将manticore执行器的开发版本链接到 `/usr/bin/php`。

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
此设置确定使用持久连接时请求之间的最大等待时间（以秒或 [special_suffixes](../Server_settings/Special_suffixes.md) 表示）。该设置为可选，默认值为五分钟。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
client_timeout = 1h
```
<!-- end -->


### collation_libc_locale

<!-- example conf collation_libc_locale -->
服务器 libc 区域设置。可选，默认值为 C。

指定 libc 区域，影响基于 libc 的排序规则。详情请参阅 [collations](../Searching/Collations.md) 部分。


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

指定用于传入请求的默认排序规则。排序规则可在每个查询中覆盖。可参阅 [collations](../Searching/Collations.md) 部分了解可用排序规则列表及其他详情。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
collation_server = utf8_ci
```
<!-- end -->


### data_dir

<!-- example conf data_dir -->
指定此设置时，启用 [实时模式](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)，这是一种命令式的数据模式管理方式。该值应为目录路径，用于存储所有表、二进制日志及运行Manticore Search所需的其他内容。
指定 `data_dir` 时，不允许对 [普通表](../Creating_a_table/Local_tables/Plain_table.md) 进行索引。关于RT模式与普通模式的区别，请参阅 [本节](../Read_this_first.md#Real-time-table-vs-plain-table)。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
data_dir = /var/lib/manticore
```
<!-- end -->

### diskchunk_flush_search_timeout

<!-- example conf diskchunk_flush_search_timeout -->
防止在表中无搜索时自动刷新RAM块的超时时间。可选，默认值为30秒。

检查搜索的时间窗口，用于决定是否自动刷新。
仅当在过去 `diskchunk_flush_search_timeout` 秒内表中至少有一次搜索时，才会自动刷新。此设置与 [diskchunk_flush_write_timeout](../Server_settings/Searchd.md#diskchunk_flush_write_timeout) 配合使用。对应的 [每表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_search_timeout) 优先级更高，会覆盖此实例级默认值，实现更细粒度控制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
diskchunk_flush_search_timeout = 120s
```
<!-- end -->

### diskchunk_flush_write_timeout

<!-- example conf diskchunk_flush_write_timeout -->
等待无写入操作的秒数，超过该时间后自动将RAM块刷新到磁盘。可选，默认值为1秒。

如果在 `diskchunk_flush_write_timeout` 秒内没有对 RAM 块进行写操作，该块将被刷新到磁盘。此设置与 [diskchunk_flush_search_timeout](../Server_settings/Searchd.md#diskchunk_flush_search_timeout) 配合使用。要禁用自动刷新，请在配置中显式设置 `diskchunk_flush_write_timeout = -1`。对应的[每表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_write_timeout)优先级更高，会覆盖此实例范围的默认值，提供更细粒度的控制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
diskchunk_flush_write_timeout = 60s
```
<!-- end -->

### docstore_cache_size

<!-- example conf docstore_cache_size -->
此设置指定文档存储中保存在内存中的文档块的最大大小。此项为可选，默认值为 16m（16 兆字节）。

当使用 `stored_fields` 时，文档块会从磁盘读取并解压缩。由于每个块通常包含多个文档，因此在处理下一个文档时可能会重用该块。为此，块会保存在服务器范围的缓存中。缓存保存的是未压缩的块。


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
此设置确定单个通配符展开的最大关键字数量。此项为可选，默认值为 0（无限制）。

在对启用了 `dict = keywords` 的表执行子串搜索时，单个通配符可能会导致成千上万甚至数百万个匹配关键字（例如匹配整个牛津词典中的 `a*`）。此指令允许您限制此类展开的影响。设置 `expansion_limit = N` 限制每个查询中每个通配符的展开不超过 N 个最频繁匹配的关键字。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_limit = 16
```
<!-- end -->

### expansion_merge_threshold_docs

<!-- example conf expansion_merge_threshold_docs -->
此设置确定允许合并所有此类关键字的展开关键字中的最大文档数。此项为可选，默认值为 32。

在对启用了 `dict = keywords` 的表执行子串搜索时，单个通配符可能会导致成千上万甚至数百万个匹配关键字。此指令允许您增加合并关键字的数量限制，以加快匹配速度，但会在搜索时使用更多内存。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_merge_threshold_docs = 1024
```
<!-- end -->

### expansion_merge_threshold_hits

<!-- example conf expansion_merge_threshold_hits -->
此设置确定允许合并所有此类关键字的展开关键字中的最大命中数。此项为可选，默认值为 256。

在对启用了 `dict = keywords` 的表执行子串搜索时，单个通配符可能会导致成千上万甚至数百万个匹配关键字。此指令允许您增加合并关键字的数量限制，以加快匹配速度，但会在搜索时使用更多内存。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_merge_threshold_hits = 512
```
<!-- end -->

### expansion_phrase_limit

<!-- example conf expansion_phrase_limit -->
此设置控制由于 `PHRASE`、`PROXIMITY` 和 `QUORUM` 操作符内的 `OR` 操作符生成的替代短语变体的最大数量。此项为可选，默认值为 1024。

当在类似短语的操作符内使用 `|`（OR）操作符时，展开的组合总数可能会根据指定的备选项数量呈指数增长。此设置通过限制查询处理期间考虑的排列数，帮助防止过度的查询展开。

如果生成的变体数量超过此限制，查询将：

- 失败并返回错误（默认行为）
- 如果启用了 `expansion_phrase_warning`，则返回带有警告的部分结果

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_phrase_limit = 4096
```
<!-- end -->

### expansion_phrase_warning

<!-- example conf expansion_phrase_warning -->
此设置控制当查询展开限制 `expansion_phrase_limit` 被超出时的行为。

默认情况下，查询将失败并返回错误信息。当 `expansion_phrase_warning` 设置为 1 时，搜索将继续使用短语的部分转换（最多到配置的限制），服务器会向用户返回带有警告信息的结果集。这允许对于过于复杂无法完全展开的查询，仍能返回部分结果而不会完全失败。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_phrase_warning = 1
```
<!-- end -->

### grouping_in_utc

此设置指定 API 和 SQL 中的时间分组是按本地时区还是 UTC 计算。此项为可选，默认值为 0（表示“本地时区”）。

默认情况下，所有的“按时间分组”表达式（如 API 中的按天、周、月和年分组，以及 SQL 中的按天、月、年、年月、年月日分组）都是使用时间函数配置的时区（参见 `timezone` 设置或 `TZ` 环境变量）来完成的。例如，如果您有属性时间为 `13:00 utc` 和 `15:00 utc` 的文档，在分组时，它们都会根据您的时区设置分配到对应的设施组。如果您所在时区是 `utc`，则会属于同一天，但如果您位于 `utc+10`，那么这些文档将匹配到不同的“按天分组”设施组（因为 13:00 utc 在 UTC+10 时区是本地时间 23:00，而 15:00 是次日的 01:00）。有时这种行为不可接受，希望使时间分组不依赖于时区。将此选项“开启”（可以在配置中或使用 SQL 中的 [SET global](../Server_settings/Setting_variables_online.md#SET) 语句）会使所有时间分组表达式都以 UTC 计算，而不管时区设置如何。

**注意：** 日志始终使用实际系统本地时区（在类 Unix 系统中从 `/etc/localtime` 获取），无视 `TZ` 环境变量或 `timezone` 配置设置。这确保日志时间戳反映真实系统时间，有助于调试和监控。


### timezone

此设置指定日期/时间相关函数（例如 `NOW()`、`CURTIME()`、`FROM_UNIXTIME()` 等）使用的时区。默认使用系统本地时区，但您可以指定一个不同的 IANA 格式时区（例如 `Europe/Amsterdam` 或 `Asia/Singapore`）。

**时区解析顺序：**
1. 如果明确设置了 `timezone` 配置，则优先使用。
2. 否则，如果设置了 `TZ` 环境变量，则使用该变量。
3. 若二者均未设置，则使用系统本地时区（在类 Unix 系统中从 `/etc/localtime` 获取）。

**重要说明：**
- **日志记录**：日志始终使用实际系统本地时区（从 `/etc/localtime` 获取），无视 `TZ` 环境变量或 `timezone` 配置。这确保日志时间戳反映真实系统时间，有利于调试和监控。
- **时间函数**：日期/时间相关函数遵循 `timezone` 配置或 `TZ` 环境变量设置，如无设置，则使用系统本地时区。
- **分组**：如果启用 `grouping_in_utc`，则“按时间分组”函数使用 UTC，而其他日期/时间函数使用此设置指定的时区。总体来看，不建议混用 `grouping_in_utc` 和 `timezone`。

您可以在配置文件中设置此选项，或使用 SQL 中的 [SET global](../Server_settings/Setting_variables_online.md#SET) 语句进行设置。


### ha_period_karma

<!-- example conf ha_period_karma -->
此设置指定代理镜像统计时间窗口大小，单位为秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。该设置为可选，默认值为 60 秒。

对于包含代理镜像的分布式表（详见 [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)），主节点会跟踪多个不同镜像的计数器。这些计数器用于故障切换和平衡（主节点基于计数器选择最佳镜像）。计数器以 `ha_period_karma` 秒为块累积。

开始新块后，主节点仍可能使用前一块累积的值，直到新块填充达到一半。因此，任何前期历史数据最多在 1.5 倍 ha_period_karma 秒时间内不再影响镜像选择。

尽管最多使用两个块进行镜像选择，但最多保存最近 15 个块以供监控。这些块可通过 [SHOW AGENT STATUS](../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS) 语句查看。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_period_karma = 2m
```
<!-- end -->


### ha_ping_interval

<!-- example conf ha_ping_interval -->
此设置配置代理镜像 ping 的时间间隔，单位为毫秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。该设置为可选，默认值为 1000 毫秒。

对于包含代理镜像的分布式表（详见 [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)），主节点在空闲期间向所有镜像发送 ping 命令，跟踪当前代理状态（存活或死机、网络往返时间等）。ping 间隔由此指令定义。若要禁用 ping，设置 `ha_ping_interval` 为 0。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_ping_interval = 3s
```
<!-- end -->


### hostname_lookup

`hostname_lookup` 选项定义了主机名更新策略。默认情况下，服务器启动时会缓存代理主机名的 IP 地址，以避免频繁访问 DNS。但在某些情况下，IP 会动态变化（如云托管），可能希望不缓存这些 IP。将该选项设为 `request` 会禁用缓存，每次查询都访问 DNS。IP 地址也可以使用 `FLUSH HOSTNAMES` 命令手动更新。

### jobs_queue_size

`jobs_queue_size` 设置定义队列中同时可存在的“任务”数量。默认无限制。

在大多数情况下，“作业”指的是对单个本地表（一张普通表或实时表的磁盘分片）的一次查询。例如，如果你有一个由两个本地表组成的分布式表，或者一个包含两个磁盘分片的实时表，对其中任一表的搜索查询通常会将两个作业放入队列。然后，线程池（大小由[threads](../Server_settings/Searchd.md#threads)定义）将处理它们。然而，在某些情况下，如果查询过于复杂，则可能会创建更多作业。当[max_connections](../Server_settings/Searchd.md#max_connections)和[threads](../Server_settings/Searchd.md#threads)不足以在所需性能之间找到平衡时，建议更改此设置。

### join_batch_size

表连接的工作原理是累积一个匹配批次，该批次是对左表执行查询的结果。然后，该批次作为单个查询在右表上处理。

此选项允许你调整批次大小。默认值为`1000`，设置此选项为`0`则禁用批处理。

较大的批量大小可能提升性能；然而，对于某些查询，可能导致过度的内存消耗。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
join_batch_size = 2000
```
<!-- end -->

### join_cache_size

对右表执行的每个查询都是由特定的JOIN ON条件定义的，这些条件决定了从右表检索的结果集。

如果只有少数独特的JOIN ON条件，重复使用结果比反复执行右表的查询更有效。为了启用这一点，结果集会被存储在缓存中。

此选项允许你配置缓存的大小。默认值为`20 MB`，设置此选项为0则禁用缓存。

请注意，每个线程维护自己的缓存，因此在估算总内存使用时应考虑执行查询的线程数。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
join_cache_size = 10M
```
<!-- end -->

### listen_backlog

<!-- example conf listen_backlog -->
listen_backlog 设置决定了TCP监听队列的长度，用于接受传入连接。这在逐个处理请求的Windows版本中特别重要。当连接队列达到上限时，新到的连接将被拒绝。
对于非Windows版本，默认值一般适用，通常无需调整此设置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
listen_backlog = 20
```
<!-- end -->

### kibana_version_string

<!-- example conf kibana_version_string -->
向Kibana或OpenSearch Dashboards返回的服务器版本字符串。可选——默认设置为`7.6.0`。

某些版本的Kibana和OpenSearch Dashboards期望服务器报告特定的版本号，且可能因此表现不同。为解决此类问题，你可以使用此设置，使Manticore向Kibana或OpenSearch Dashboards报告自定义版本。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
kibana_version_string = 1.2.3
```
<!-- end -->

### listen

<!-- example conf listen -->
此设置让你指定Manticore接受连接的IP地址和端口，或者Unix域套接字路径。

`listen`的一般语法为：

```ini
listen = ( address ":" port | port | path | address ":" port start - port end ) [ ":" protocol [ "_vip" ] [ "_readonly" ] ]
```

你可以指定：
* IP地址（或主机名）和端口号
* 仅端口号
* Unix套接字路径（Windows不支持）
* IP地址和端口范围

如果你指定了端口号但未指定地址，`searchd`将监听所有网络接口。Unix路径以斜杠开头。端口范围仅可为复制协议设置。

你还可以为此套接字上的连接指定协议处理器（监听器）。监听器有：

* **未指定** - Manticore将在此端口接受来自：
  - 其他Manticore代理（即远程分布式表）的连接
  - 通过HTTP和HTTPS的客户端连接
  - [Manticore Buddy](https://manticoresearch.com/blog/manticoresearch-buddy-intro/)。**请确保你有此类监听器（或如下所述的`http`监听器），以避免Manticore功能受限。**
* `mysql` MySQL协议，用于MySQL客户端连接。注意：
  - 也支持压缩协议。
  - 若启用[SSL](../Security/SSL.md#SSL)，可建立加密连接。
* `replication` - 用于节点通信的复制协议。更多细节见[复制](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md)部分。你可以指定多个复制监听器，但它们必须监听同一IP，只能端口不同。当定义带端口范围的复制监听器（如`listen = 192.168.0.1:9320-9328:replication`）时，Manticore不会立即监听这些端口，而是在开始使用复制时从指定范围随机选择空闲端口。范围内最少需2个端口以保证复制正常工作。
* `http` - 同**未指定**。Manticore将在此端口接受来自远程代理和通过HTTP/HTTPS的客户端连接。
* `https` - HTTPS协议。Manticore将在此端口仅接受HTTPS连接。更多细节见[SSL](../Security/SSL.md)。
* `sphinx` - 传统二进制协议。用于服务来自远程[SphinxSE](../Extensions/SphinxSE.md)客户端的连接。某些Sphinx API客户端实现（例如Java客户端）需明确声明该监听器。

给客户端协议添加后缀`_vip`（即除`replication`外的所有协议，例如`mysql_vip`或`http_vip`或仅`_vip`）会强制为连接创建专用线程以绕过各种限制。这在服务器压力过大时对节点维护很有用，否则服务器可能会停滞或无法通过常规端口连接。

后缀`_readonly`为监听器设置[只读模式](../Security/Read_only.md)，限制其仅接受读查询。

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

可以有多个 `listen` 指令。`searchd` 会监听所有指定的端口和套接字上的客户端连接。Manticore 软件包中提供的默认配置定义了以下端口的监听：
* 远程代理和非 MySQL 客户端连接的端口为 `9308` 和 `9312`
* MySQL 连接的端口为 `9306`。

如果在配置中完全没有指定任何 `listen`，Manticore 会在以下地址等待连接：
* 用于 MySQL 客户端的 `127.0.0.1:9306`
* 用于 HTTP/HTTPS 以及来自其他 Manticore 节点和基于 Manticore 二进制 API 客户端的连接的 `127.0.0.1:9312`。

#### 监听特权端口

默认情况下，Linux 不允许你让 Manticore 监听低于 1024 的端口（例如 `listen = 127.0.0.1:80:http` 或 `listen = 127.0.0.1:443:https`），除非你以 root 用户身份运行 searchd。如果你仍然希望能够启动 Manticore，使其在非 root 用户下监听 <1024 端口，请考虑执行以下任一操作（以下任意一项均可生效）：
* 运行命令 `setcap CAP_NET_BIND_SERVICE=+eip /usr/bin/searchd`
* 将 `AmbientCapabilities=CAP_NET_BIND_SERVICE` 添加到 Manticore 的 systemd 单元文件，并重新加载守护进程（`systemctl daemon-reload`）。

#### 关于 Sphinx API 协议和 TFO 的技术细节
<details>
传统的 Sphinx 协议有两个阶段：握手交换和数据流。握手阶段由客户端发送的 4 字节包和守护进程返回的 4 字节包组成，其唯一目的是客户端确认远端是一个真正的 Sphinx 守护进程，守护进程确认远端是一个真正的 Sphinx 客户端。主要数据流非常简单：双方都声明自己的握手，且对方会检查。由于此交换包较短，故会使用特殊的 `TCP_NODELAY` 标志，该标志关闭了 Nagle 的 TCP 算法，并声明 TCP 连接将作为小包对话进行。
然而，谁先发言在这个协商中并没有严格规定。历史上，所有使用二进制 API 的客户端都是先发送：先发送握手，然后读取守护进程的 4 字节响应，再发送请求并读取守护进程的回答。
当我们改进 Sphinx 协议兼容性时，考虑到以下几点：

1. 通常，主代理通信是从已知客户端连接到已知主机的已知端口，因此端点提供错误握手的可能性较低。所以我们可以隐式假定双方都是有效的，并且确实在使用 Sphinx 协议。
2. 在此假定下，我们可以将握手与真实请求“粘合”在一起，并作为一个数据包发送。如果后端是传统的 Sphinx 守护进程，则会将此粘合包读取为前 4 字节握手包，然后是请求体。由于它们在一个包中，后端套接字节省了 1 个 RTT，前端缓冲区也仍能正常工作。
3. 继续假定：既然“查询”包很小，握手更小，不如在初始 TCP “SYN” 包中使用现代 TFO（tcp-fast-open）技术发送两者。这就是说：我们用粘合的握手+请求体包连接远程节点。守护进程接受连接后，握手和请求体即存在套接字缓冲区，因为它们作为首个 TCP “SYN” 包到达。这省去了另一个 RTT。
4. 最后，教会守护进程支持此改进。实际上，从应用程序层面，这意味着不使用 `TCP_NODELAY`。从系统层面，这意味着确保守护进程端激活了 TFO 接收，客户端也激活了 TFO 发送。现代系统中，客户端 TFO 默认已启用，故你只需调优服务器端的 TFO 即可。

所有这些改进在未更改协议本身的情况下，帮助我们消除了 TCP 连接中的 1.5 RTT。若查询和响应可以合并到一个 TCP 包中，则将整个二进制 API 会话从 3.5 RTT 降至 2 RTT——网络协商速度提升约两倍。

因此，所有改进基于一个最初未定义的前提：“谁先发言”。若客户端先发言，我们可应用所有这些优化，有效地在一个 TFO 包中处理连接 + 握手 + 查询。此外，我们可查看接收包的开头以确定真实协议，这就是为何你可以通过 API/http/https 连接到同一端口。如果守护进程必须先发言，则所有这些优化都不可行，且多协议也不可能。这就是为什么我们给 MySQL 设立了专用端口，且未将其与所有其他协议统一至同一端口。突然之间，有一个客户为了 SphinxSE 插件（用于 mysql/mariadb）被写成守护进程需先发送握手，因此无法使用上述改进。为此单一客户端，我们专门设定了 `sphinx` 协议定义，以最传统的方式工作。即双方激活 `TCP_NODELAY` 并通过小包交换握手，守护进程连接时先发送握手，随后客户端发送，然后一切按常规工作。性能不太理想，但能正常运转。如果你使用 SphinxSE 连接 Manticore，则必须专门指定一个监听器显式声明 `sphinx` 协议。对于其他客户端，则避免使用该监听器，因为它较慢。如果你使用其他传统 Sphinx API 客户端，首先检查它们是否能在非专用多协议端口上工作。对于主代理链路，使用非专用（多协议）端口并启用客户端和服务器的 TFO 能很好运行，且会显著加速网络后端，尤其是你的查询非常轻量且快速时。
</details>

### listen_tfo

此设置允许为所有监听器启用 TCP_FASTOPEN 标志。默认情况下由系统管理，但可通过将其设置为 '0' 明确关闭。

有关TCP Fast Open扩展的一般知识，请参阅[维基百科](https://en.wikipedia.org/wiki/TCP_Fast_Open)。简而言之，它允许在建立连接时消除一次TCP往返。

在实践中，在许多情况下使用TFO可以优化客户端代理的网络效率，就好像使用了[持久代理](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)，但无需保持活动连接，并且对最大连接数没有限制。

在现代操作系统上，TFO支持通常在系统级别被切换为“开启”，但这只是一个“能力”，而非规则。Linux（作为最进步的）从2011年起支持它，内核版本从3.7开始（仅服务器端）。Windows从某些Windows 10版本开始支持。其他操作系统（FreeBSD、MacOS）也支持该功能。

对于Linux系统，服务器检查变量`/proc/sys/net/ipv4/tcp_fastopen`并根据其值做出相应行为。第0位管理客户端，第1位管理监听器。默认情况下，系统将该参数设置为1，即客户端启用，监听器禁用。

### log

<!-- example conf log -->
日志设置指定所有`searchd`运行时事件将被记录的日志文件名。如果未指定，默认名称为`searchd.log`。

或者，你可以使用`syslog`作为文件名。在这种情况下，事件将被发送到syslog守护进程。要使用syslog选项，你需要在构建时使用`-–with-syslog`选项配置Manticore。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
log = /var/log/searchd.log
```
<!-- end -->


### max_batch_queries

<!-- example conf max_batch_queries -->
限制每批查询的数量。可选，默认值为32。

当使用[multi-queries](../Searching/Multi-queries.md)时，使searchd对单批提交的查询数量进行合理性检查。设置为0可跳过该检查。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_batch_queries = 256
```
<!-- end -->

### max_connections

<!-- example max_connections -->
最大同时客户端连接数。默认无限制。通常只有在使用诸如cli mysql会话或分布式远程表的持久连接时才会明显影响。当超过限制时，你仍然可以使用[VIP连接](../Connecting_to_the_server/MySQL_protocol.md#VIP-connection)连接服务器。VIP连接不计入限制。

<!-- request Example -->
```ini
max_connections = 10
```

<!-- end -->

### max_threads_per_query

<!-- example max_threads_per_query -->
单个操作可使用的线程实例范围限制。默认情况下，适当的操作可以占用所有CPU核心，不留给其他操作。例如，针对相当大的percolate表执行`call pq`可能会持续数十秒占用所有线程。将`max_threads_per_query`设置为，例如，[threads](../Server_settings/Searchd.md#threads)的一半将确保你可以并行运行几个这样的`call pq`操作。

你也可以在运行时将此设置作为会话或全局变量设置。

此外，你可以使用[threads选项](../Searching/Options.md#threads)按查询控制此行为。

<!-- intro -->
##### 示例：
<!-- request Example -->

```ini
max_threads_per_query = 4
```

<!-- end -->

### max_filters

<!-- example conf max_filters -->
每查询允许的最大过滤器数量。该设置仅用于内部合理性检查，不直接影响内存使用或性能。可选，默认值为256。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_filters = 1024
```
<!-- end -->


### max_filter_values

<!-- example conf max_filter_values -->
每过滤器允许的最大值数量。该设置仅用于内部合理性检查，不直接影响内存使用或性能。可选，默认值为4096。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_filter_values = 16384
```
<!-- end -->


### max_open_files

<!-- example conf max_open_files -->
服务器允许打开的最大文件数称为“软限制”。请注意，服务大型碎片化实时表可能需要将此限制设置得很高，因为每个磁盘块可能占用十几个或更多文件。例如，一个具有1000个块的实时表可能需要同时打开数千个文件。如果日志中出现“Too many open files”错误，请尝试调整此选项，可能有助于解决问题。

还有一个“硬限制”，此选项无法超过该限制。该限制由系统定义，可在Linux的`/etc/security/limits.conf`文件中更改。其他操作系统可能有不同的方法，请参考你的手册了解更多信息。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = 10000
```
<!-- end -->

<!-- example conf max_open_files max -->
除了直接使用数值外，你还可以使用魔法词`max`将限制设置为当前可用的最大硬限制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = max
```
<!-- end -->


### max_packet_size

<!-- example conf max_packet_size -->
允许的最大网络数据包大小。此设置限制客户端的查询包和分布式环境中远程代理的响应包。仅用于内部合理性检查，不直接影响内存使用或性能。可选，默认值为128M。


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

一些挑剔的 MySQL 客户端库依赖于 MySQL 使用的特定版本号格式，而且有时会根据报告的版本号（而非指示的功能标志）选择不同的执行路径。例如，Python MySQLdb 1.2.2 在版本号不是 X.Y.ZZ 格式时会抛出异常；MySQL .NET 连接器 6.3.x 在版本号为 1.x 且某种标志组合下会内部失败，等等。为了解决这个问题，你可以使用 `mysql_version_string` 指令，让 `searchd` 对通过 MySQL 协议连接的客户端报告不同的版本号。（默认情况下，它报告自己的版本。）


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
mysql_version_string = 5.0.37
```
<!-- end -->


### net_workers

网络线程数，默认值为 1。

此设置对于极高查询速率非常有用，当只有一个线程不足以管理所有传入查询时。


### net_wait_tm

控制网络线程的繁忙循环间隔。默认值为 -1，可以设置为 -1、0 或正整数。

当服务器配置为纯主服务器并且只是将请求路由到代理时，重要的是无延迟处理请求，并且不让网络线程休眠。为此，有一个繁忙循环。收到传入请求后，如果 `net_wait_tm` 是正数，网络线程会使用 CPU 轮询运行 `10 * net_wait_tm` 毫秒；如果 `net_wait_tm` 是 `0`，则仅使用 CPU 轮询。此外，可以通过设置 `net_wait_tm = -1` 来禁用繁忙循环——在这种情况下，轮询器将超时设置为系统轮询调用上代理的实际超时。

> **警告：** CPU 繁忙循环实际上会加载 CPU 核心，因此将此值设置为任何非默认值即使在服务器空闲时也会导致明显的 CPU 使用量。


### net_throttle_accept

定义在网络循环的每次迭代中接受多少客户端。默认值为 0（无限制），对大多数用户应当没问题。这是一个微调选项，用于在高负载场景中控制网络循环的吞吐量。


### net_throttle_action

定义在网络循环的每次迭代中处理多少请求。默认值为 0（无限制），对大多数用户应当没问题。这是一个微调选项，用于在高负载场景中控制网络循环的吞吐量。

### network_timeout

<!-- example conf network_timeout -->
网络客户端请求读写超时，单位为秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认值为 5 秒。`searchd` 会强制关闭在此超时时间内未能发送查询或读取结果的客户端连接。

另请注意 [reset_network_timeout_on_packet](../Server_settings/Searchd.md#reset_network_timeout_on_packet) 参数。该参数改变 `network_timeout` 的行为，使其不再应用于整个查询或结果，而是应用于单个数据包。通常，查询/结果会包含在一到两个数据包中。然而，在需要大量数据的情况下，该参数对保持活动操作非常宝贵。

<!-- request Example -->

```ini
network_timeout = 10s
```
<!-- end -->

### node_address

<!-- example conf node_address -->
此设置允许你指定节点的网络地址。默认设置为复制的 [listen](../Server_settings/Searchd.md#listen) 地址。大多数情况下这很正确；但在以下情况下需要手动指定：

* 节点位于防火墙后面
* 启用网络地址转换（NAT）
* 容器部署，如 Docker 或云端部署
* 有多个区域节点的集群


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
node_address = 10.101.0.10
```
<!-- end -->

### not_terms_only_allowed

<!-- example conf not_terms_only_allowed -->
此设置决定是否允许仅带有[非](../Searching/Full_text_matching/Operators.md#Negation-operator)全文操作符的查询。可选，默认值为 0（对于仅含 NOT 操作符的查询失败）。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
not_terms_only_allowed = 1
```
<!-- end -->

### optimize_cutoff

<!-- example conf optimize_cutoff -->
设置默认表压缩阈值。详情请参阅 - [优化的磁盘块数量](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks)。此设置可被每查询选项 [cutoff](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 覆盖。也可以通过 [SET GLOBAL](../Server_settings/Setting_variables_online.md#SET) 动态更改。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
optimize_cutoff = 4
```
<!-- end -->

### persistent_connections_limit

<!-- example conf persistent_connections_limit -->
此设置决定到远程 [持久代理](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 的最大同时持久连接数。每次连接在 `agent_persistent` 下定义的代理时，我们尝试重用现有连接（如果有），否则建立新连接并保存用于后续使用。然而，在某些情况下，限制此类持久连接数是合理的。此指令定义了该限制。它影响所有分布式表中对每个代理主机的连接数。

合理的做法是将该值设置为代理配置中 [max_connections](../Server_settings/Searchd.md#max_connections) 选项的值或更低。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
persistent_connections_limit = 29 # assume that each host of agents has max_connections = 30 (or 29).
```
<!-- end -->


### pid_file

<!-- example conf pid_file -->
pid_file 是 Manticore Search 中的必需配置选项，指定存放 `searchd` 服务器进程 ID 的文件路径。

searchd 进程 ID 文件在启动时被重新创建并锁定，服务器运行时包含主服务器进程 ID。服务关闭时会删除该文件。
该文件的目的是使 Manticore 执行各种内部任务，例如检查是否已经有运行中的 `searchd` 实例，停止 `searchd`，以及通知它应该旋转表。该文件也可以用于外部自动化脚本。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
pid_file = /run/manticore/searchd.pid
```
<!-- end -->


### predicted_time_costs

<!-- example conf predicted_time_costs -->
查询时间预测模型的成本，以纳秒为单位。可选，默认值为 `doc=64, hit=48, skip=2048, match=64`。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
predicted_time_costs = doc=128, hit=96, skip=4096, match=128
```
<!-- end -->

<!-- example conf predicted_time_costs 1 -->
基于查询执行时间（使用最大查询时间设置）在完成前终止查询是一个不错的安全网，但也存在一个固有缺点：结果不确定（不稳定）。也就是说，如果你多次重复执行完全相同（复杂）的带时间限制的搜索查询，时间限制会在不同阶段被触发，你将得到*不同*的结果集。

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

有一个选项，[SELECT … OPTION max_predicted_time](../Searching/Options.md#max_predicted_time)，允许你限制查询时间*并且*获得稳定、可重复的结果。它不是定期检查实际当前时间（这是不确定的），而是使用一个简单的线性模型来预测当前运行时间：

```ini
predicted_time =
    doc_cost * processed_documents +
    hit_cost * processed_hits +
    skip_cost * skiplist_jumps +
    match_cost * found_matches
```

当 `predicted_time` 达到给定限制时，查询将提前终止。

当然，这并不是对实际使用时间的硬限制（但它确实是对所做*处理*工作的硬限制），而且简单的线性模型绝不是理想的精确模型。因此，墙钟时间*可能*低于或超过目标限制。然而，误差范围相当可接受：例如，在我们的实验中，目标限制为 100 毫秒，大多数测试查询落在 95 到 105 毫秒范围内，*所有*查询都在 80 到 120 毫秒范围内。此外，一个额外的好处是，使用建模的查询时间而不是测量实际运行时间，会稍微减少 gettimeofday() 的调用次数。

没有两台服务器的型号是完全相同的，所以 `predicted_time_costs` 指令允许你配置上述模型的成本。为方便起见，它们是整数，单位是纳秒。（`max_predicted_time` 中的限制以毫秒计，若要以毫秒指定成本值为 0.000128 则比指定为 128 纳秒更容易出错。）不必一次指定所有四项成本，缺失的会采用默认值。但我们强烈建议为了可读性全部指定。


### preopen_tables

<!-- example conf preopen_tables -->
`preopen_tables` 配置指令指定是否在启动时强制预先打开所有表。默认值为 1，表示所有表都将被预先打开，而不管每个表的 [preopen](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Other-performance-related-settings) 设置。如果设置为 0，则允许每个表的设置生效，它们默认也是 0。

预先打开表可以防止搜索查询和表旋转之间的竞争，避免查询偶尔失败。但它也会使用更多的文件句柄。在大多数场景中，建议预先打开表。

示例配置如下：

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
preopen_tables = 1
```
<!-- end -->

### pseudo_sharding

<!-- example conf pseudo_sharding -->
`pseudo_sharding` 配置选项启用对本地普通和实时表的搜索查询的并行化，无论是直接查询还是通过分布式表查询。该功能将自动并行化查询，最多达到 `searchd.threads` 指定的线程数。

注意，如果你的工作线程已经繁忙，因为你拥有：
* 高查询并发
* 任何形式的物理分片：
  - 多个普通/实时表的分布式表
  - 包含过多磁盘分片的实时表

那么启用 `pseudo_sharding` 可能不会带来任何好处，甚至可能导致吞吐量略微下降。如果你更重视较高的吞吐量而非较低的延迟，建议禁用此选项。

默认启用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
pseudo_sharding = 0
```
<!-- end -->


### replication_connect_timeout

`replication_connect_timeout` 指令定义连接远程节点的超时时间。默认值被认为是毫秒单位，但也可以使用[其他后缀](../Server_settings/Special_suffixes.md)。默认值为 1000（1 秒）。

当连接远程节点时，Manticore 会等待最多该时间以成功建立连接。如果达到超时但尚未建立连接，且启用了 `retries`，将重新尝试连接。


### replication_query_timeout

`replication_query_timeout` 设置 searchd 等待远程节点完成查询的时间。默认值为 3000 毫秒（3 秒），但可以带后缀以表示不同的时间单位。

建立连接后，Manticore 会等待最多 `replication_query_timeout` 时间以完成查询。请注意，该超时与 `replication_connect_timeout` 分开计算，远程节点导致的总可能延迟是两者之和。


### replication_retry_count

该设置是一个整数，指定 Manticore 在报告致命查询错误之前尝试连接和查询远程节点的次数。默认值为 3。


### replication_retry_delay

此设置为以毫秒为单位的整数（或 [special_suffixes](../Server_settings/Special_suffixes.md)），指定在复制过程中查询远程节点失败时，Manticore 重试查询的延迟时间。仅当指定了非零值时，此值才有效。默认值为 500。

### qcache_max_bytes

<!-- example conf qcache_max_bytes -->
此配置设置缓存结果集分配的最大内存（以字节为单位）。默认值为 16777216，相当于 16 兆字节。如果值设为 0，则禁用查询缓存。有关查询缓存的更多信息，请参阅 [query cache](../Searching/Query_cache.md)。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
qcache_max_bytes = 16777216
```
<!-- end -->


### qcache_thresh_msec

整数，以毫秒为单位。查询结果被缓存的最小墙时阈值。默认值为 3000，即 3 秒。0 表示缓存所有结果。详情请参阅 [query cache](../Searching/Query_cache.md)。此值也可以用时间 [special_suffixes](../Server_settings/Special_suffixes.md) 表示，但请谨慎使用，避免与值名称中包含的 '_msec' 混淆。


### qcache_ttl_sec

整数，以秒为单位。缓存结果集的过期时间。默认值为 60，即 1 分钟。最小可设值为 1 秒。详情请参阅 [query cache](../Searching/Query_cache.md)。此值也可以用时间 [special_suffixes](../Server_settings/Special_suffixes.md) 表示，但请谨慎使用，避免与值名称中包含的 '_sec' 混淆。


### query_log_format

<!-- example conf query_log_format -->
查询日志格式。可选，允许的值为 `plain` 和 `sphinxql`，默认是 `sphinxql`。

`sphinxql` 模式记录有效的 SQL 语句。`plain` 模式以纯文本格式记录查询（主要适合纯全文本使用场景）。此指令允许你在搜索服务器启动时切换两种格式。日志格式也可以动态更改，使用 `SET GLOBAL query_log_format=sphinxql` 语法。详情请参阅 [Query logging](../Logging/Query_logging.md)。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_format = sphinxql
```
<!-- end -->

### query_log_min_msec

限制（以毫秒为单位），防止执行时间低于该限制的查询被写入查询日志。可选，默认值为 0（所有查询均写入查询日志）。此指令指定只有执行时间超过该限定值的查询才会被记录（此值也可用时间 [special_suffixes](../Server_settings/Special_suffixes.md) 表示，但请谨慎使用，避免与值名称中包含的 `_msec` 混淆）。

### query_log

<!-- example conf query_log -->
查询日志文件名。可选，默认为空（不记录查询）。所有搜索查询（如 SELECT ...，但不包括 INSERT/REPLACE/UPDATE 查询）将记录在该文件中。格式见 [Query logging](../Logging/Query_logging.md)。对于 'plain' 格式，你可以使用 'syslog' 作为日志文件路径。在这种情况下，所有搜索查询将通过 `LOG_INFO` 优先级发送到 syslog 守护进程，前缀为 '[query]'，替代时间戳。要使用 syslog 选项，Manticore 编译时必须配置 `-–with-syslog`。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log = /var/log/query.log
```
<!-- end -->


### query_log_mode

<!-- example conf query_log_mode -->
query_log_mode 指令允许你为 searchd 和查询日志文件设置不同的权限。默认这些日志文件创建权限为 600，即只有服务器运行的用户和 root 用户可以读取日志文件。
如果你想允许其他用户读取日志文件，例如运行在非 root 用户下的监控解决方案，此指令非常有用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_mode  = 666
```
<!-- end -->

### read_buffer_docs

<!-- example conf read_buffer_docs -->
read_buffer_docs 指令控制每个关键字的文档列表读取缓冲区大小。对于每个搜索查询中的关键字出现，存在两个相关的读取缓冲区：一个用于文档列表，一个用于命中列表。该设置控制文档列表缓冲区大小。

较大的缓冲区大小可能增加每查询的 RAM 使用量，但可能减少 I/O 时间。对于慢速存储设较大值合理，而对于高 IOPS 存储，建议在较低值范围进行试验。

默认值为 256K，最小值为 8K。你也可以针对每个表设置 [read_buffer_docs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs)，该设置会覆盖服务器配置层级设置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_docs = 128K
```
<!-- end -->


### read_buffer_hits

<!-- example conf read_buffer_hits -->
read_buffer_hits 指令指定每个关键字在搜索查询中命中列表的读取缓冲区大小。默认大小为 256K，最小值为 8K。对于每个关键字出现，存在两个读取缓冲区，一个用于文档列表，一个用于命中列表。增加缓冲区大小可能增加每查询的 RAM 使用量，但降低 I/O 时间。慢速存储下设置较大缓冲区合理，而高 IOPS 存储建议在较低值范围试验。

该设置也可以通过每表级别的 read_buffer_hits 选项在 [read_buffer_hits](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_hits) 中指定，覆盖服务器级设置。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_hits = 128K
```
<!-- end -->

### read_unhinted

<!-- example conf read_unhinted -->
未经提示的读取大小。可选，默认值为 32K，最小为 1K

在查询时，有些读取操作事先确切知道要读取的数据量，但有些当前还不知道。最显著的是，命中列表的大小当前还无法预知。此设置允许您在这种情况下控制读取多少数据。它影响命中列表的I/O时间，对于大于未提示读取大小的列表可以减少时间，但对于较小的列表则会增加时间。它**不会**影响RAM使用，因为读取缓冲区已经被分配。因此，它不应大于read_buffer。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_unhinted = 32K
```
<!-- end -->

### reset_network_timeout_on_packet

<!-- example conf reset_network_timeout_on_packet -->
细化网络超时（如`network_timeout`和`agent_query_timeout`）的行为。

当设置为0时，超时限制整个请求/查询的最长发送时间。
当设置为1（默认）时，超时限制网络活动之间的最长时间。

在复制时，一个节点可能需要向另一个节点发送一个大文件（例如100GB）。假设网络传输速度为1GB/s，分为4-5MB一系列数据包。发送整个文件需要100秒。默认5秒的超时只允许传输5GB数据，之后连接会被断开。增加超时可能是一个变通办法，但不可扩展（例如，下一文件可能是150GB，又会导致失败）。然而，默认情况下`reset_network_timeout_on_packet`为1，超时应用于单个数据包，而不是整个传输。只要传输正在进行（并且在超时期间实际上接收到了数据），连接将保持活跃。如果传输卡住，数据包之间发生超时，就会断开连接。

注意，如果您配置了分布式表，每个节点——无论是主节点还是代理——都应调整。在主节点上，受到`agent_query_timeout`影响；在代理上，受到`network_timeout`影响。

<!-- intro -->

##### 示例：

<!-- request Example -->

```ini
reset_network_timeout_on_packet = 0
```

<!-- end -->


### rt_flush_period

<!-- example conf rt_flush_period -->
RT表RAM块刷新检查周期，单位为秒（或[special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认值为10小时。

完全适合在RAM块中的主动更新RT表仍可能导致二进制日志持续增长，影响磁盘使用和崩溃恢复时间。使用此指令，搜索服务器执行定期刷新检查，符合条件的RAM块可以保存，从而实现后续的二进制日志清理。详情请参见[二进制日志](../Logging/Binary_logging.md)。

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

此指令允许您控制`OPTIMIZE`语句产生的I/O影响。确保所有RT优化活动产生的磁盘IOPS（每秒I/O次数）不会超过配置的限制。限制rt_merge_iops可以减少合并操作导致的搜索性能下降。

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

此指令允许您控制`OPTIMIZE`语句产生的I/O影响。超过此限制的I/O操作将被拆分成两个或多个I/O操作，然后根据[rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops)限制分别计数。因此，保证所有优化活动每秒磁盘I/O不会超过 (rt_merge_iops * rt_merge_maxiosize) 字节。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_maxiosize = 1M
```
<!-- end -->


### seamless_rotate

<!-- example conf seamless_rotate -->
防止在旋转包含大量数据的表以进行预缓存时`searchd`阻塞。可选，默认启用无缝旋转（值为1）。Windows系统默认禁用无缝旋转。

表可能包含一些需要预缓存到RAM的数据。目前，`.spa`、`.spb`、`.spi`和`.spm` 文件会被完整预缓存（分别包含属性数据、二进制大对象属性数据、关键词表和“已删除”行映射）。没有无缝旋转时，旋转表尝试尽量减少RAM使用，过程如下：

1. 临时拒绝新查询（返回“重试”错误码）；
2. `searchd`等待所有正在执行的查询完成；
3. 旧表释放，文件重命名；
4. 新表文件重命名，分配所需RAM；
5. 预加载新表属性和字典数据到RAM；
6. `searchd`恢复从新表服务查询。

但是，如果属性或字典数据较多，预加载步骤可能耗时显著——例如预加载1-5+ GB文件时可能需数分钟。

启用无缝旋转后，旋转过程如下：

1. 分配新表RAM存储；
2. 异步预加载新表属性和字典数据到RAM；
3. 成功时，释放旧表，重命名两个表文件；
4. 失败时，释放新表；
5. 任何时刻，查询从旧表或新表其中一份提供服务。

无缝旋转的代价是旋转期间峰值内存使用更高（因为在预加载新副本时旧副本和新副本的`.spa/.spb/.spi/.spm`数据都需驻留RAM）。平均使用保持不变。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
seamless_rotate = 1
```
<!-- end -->

### secondary_index_block_cache
<!-- example conf secondary_index_block_cache -->

此选项指定次级索引使用的块缓存大小。它是可选的，默认值为 8 MB。当次级索引与包含许多值的过滤器（例如，IN() 过滤器）一起使用时，它们会读取并处理这些值的元数据块。
在联接查询中，此过程会针对左表的每个行批次重复进行，并且每个批次可能在单个联接查询内重新读取相同的元数据。这会严重影响性能。元数据块缓存将这些块保存在内存中，以便
后续批次重复使用。

该缓存仅用于联接查询，对非联接查询无影响。请注意，缓存大小限制适用于每个属性和每个次级索引。每个磁盘块中的每个属性都在此限制范围内运行。在最坏情况下，总内存
使用量可以通过将限制乘以磁盘块数量和联接查询中使用的属性数量来估算。

设置 `secondary_index_block_cache = 0` 会禁用缓存。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
secondary_index_block_cache = 16M
```

<!-- end -->

### secondary_indexes
<!-- example conf secondary_indexes -->

此选项启用/禁用在搜索查询中使用次级索引。它是可选的，默认值为 1（启用）。请注意，索引创建时无需启用该选项，只要安装了 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)，该功能始终启用。搜索时使用索引也需要后者。有三种可用模式：

* `0`：禁用搜索中次级索引的使用。可以通过使用[分析器提示](../Searching/Options.md#Query-optimizer-hints)为单个查询启用。
* `1`：启用搜索中次级索引的使用。可以通过使用[分析器提示](../Searching/Options.md#Query-optimizer-hints)为单个查询禁用。
* `force`：同启用，但在加载次级索引时任何错误都会被报告，并且整个索引不会加载到守护进程中。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
secondary_indexes = 1
```

<!-- end -->

### server_id

<!-- example conf server_id -->
整数编号，作为服务器标识符，用作在复制集群节点之间生成唯一短 UUID 的种子。server_id 必须在集群节点间唯一，且范围为 0 到 127。如果未设置 server_id，则会作为 MAC 地址和 PID 文件路径的哈希计算，或以随机数作为短 UUID 的种子。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
server_id = 1
```
<!-- end -->


### shutdown_timeout

<!-- example conf shutdown_timeout -->
`searchd --stopwait` 等待时间，单位为秒（或使用 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认值为 60 秒。

当你运行 `searchd --stopwait` 时，服务器需要在停止前执行一些操作，例如完成查询、刷新 RT RAM 块、刷新属性以及更新二进制日志。这些任务需要一定时间。`searchd --stopwait` 会等待最多 `shutdown_time` 秒，以等待服务器完成其任务。合适的等待时间取决于你的表大小和负载。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
shutdown_timeout = 3m # wait for up to 3 minutes
```
<!-- end -->


### shutdown_token

需要的密码的 SHA1 哈希，用于从 VIP Manticore SQL 连接调用 'shutdown' 命令。没有该密码，运行 [debug](../Reporting_bugs.md#DEBUG) 'shutdown' 子命令永远不会导致服务器停止。请注意，这种简单的哈希不应被视为强保护措施，因为我们不使用盐哈希或任何现代哈希函数。它旨在作为本地网络中的维护守护进程的一种防呆措施。

### snippets_file_prefix

<!-- example conf snippets_file_prefix -->
生成片段时用于前置的本地文件名前缀。可选，默认是当前工作文件夹。

该前缀可与 `load_files` 或 `load_files_scattered` 选项一起用于分布式片段生成。

请注意，这是前缀，**不是**路径！这意味着如果前缀设置为 "server1" 并且请求涉及 "file23"，`searchd` 会尝试打开 "server1file23"（全部不带引号）。所以，如果你需要它是路径，必须包括结尾的斜杠。

在构造最终文件路径后，服务器会解析所有相对目录并将最终结果与 `snippet_file_prefix` 的值比较。如果结果不以该前缀开始，则该文件会被拒绝，并显示错误消息。

例如，如果你设置为 `/mnt/data`，有人使用文件 `../../../etc/passwd` 作为源调用片段生成时，会收到错误信息：

`File '/mnt/data/../../../etc/passwd' escapes '/mnt/data/' scope`

而不是文件内容。

另外，如果未设置此参数而读取 `/etc/passwd`，实际上会读取 /daemon/working/folder/etc/passwd，因为参数的默认值是服务器的工作文件夹。

还请注意，这是本地选项；不会以任何方式影响代理。因此你可以安全地在主服务器上设置前缀。路由到代理的请求不会受主服务器设置的影响。但它们会受到代理自身设置的影响。

这在文档存储位置（无论是本地存储还是 NAS 挂载点）在服务器间不一致时可能很有用。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
snippets_file_prefix = /mnt/common/server1/
```
<!-- end -->

> **警告：** 如果你仍想访问文件系统根目录的文件，必须显式将 `snippets_file_prefix` 设置为空值（通过 `snippets_file_prefix=` 行）或设置为根目录（通过 `snippets_file_prefix=/`）。


### sphinxql_state

<!-- example conf sphinxql_state -->
当前 SQL 状态将被序列化的文件路径。

在服务器启动时，此文件将被重放。在符合条件的状态更改（例如，SET GLOBAL）时，此文件会自动重写。这可以防止一个难以诊断的问题：如果您加载了UDF函数但Manticore崩溃，当它（自动）重启时，您的UDF和全局变量将不再可用。使用持久化状态有助于确保平稳恢复，避免此类意外。

`sphinxql_state` 不能用来执行任意命令，例如 `CREATE TABLE`。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
sphinxql_state = uservars.sql
```
<!-- end -->


### sphinxql_timeout

<!-- example conf sphinxql_timeout -->
使用SQL接口时，两个请求之间等待的最长时间（秒，或者使用 [special_suffixes](../Server_settings/Special_suffixes.md))。可选，默认是15分钟。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
sphinxql_timeout = 15m
```
<!-- end -->


### ssl_ca

<!-- example conf ssl_ca -->
SSL证书颁发机构（CA）证书文件的路径（也称为根证书）。可选，默认为空。当不为空时，`ssl_cert` 中的证书应由此根证书签署。

服务器使用CA文件来验证证书的签名。该文件必须是PEM格式。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ssl_ca = keys/ca-cert.pem
```
<!-- end -->


### ssl_cert

<!-- example conf ssl_cert -->
服务器SSL证书的路径。可选，默认为空。

服务器使用此证书作为自签名公钥，通过SSL加密HTTP流量。该文件必须是PEM格式。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ssl_cert = keys/server-cert.pem
```
<!-- end -->


### ssl_key

<!-- example conf ssl_key -->
SSL证书密钥的路径。可选，默认为空。

服务器使用此私钥通过SSL加密HTTP流量。该文件必须是PEM格式。


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

此设置限制公共子树优化器（参见 [multi-queries](../Searching/Multi-queries.md)）的内存使用。最多为每个查询缓存这么多文档条目所用的RAM。设置为0禁用优化器。


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

此设置限制公共子树优化器（参见 [multi-queries](../Searching/Multi-queries.md)）的内存使用。最多为每个查询缓存这么多关键词出现（命中）的RAM。设置为0禁用优化器。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
subtree_hits_cache = 16M
```
<!-- end -->

### threads

<!-- example threads -->
Manticore守护进程的工作线程数（或线程池大小）。Manticore在启动时创建这些操作系统线程，它们执行守护进程内部的所有任务，如执行查询、创建摘要等。有些操作可拆分成子任务并行执行，例如：

* 在实时表中搜索
* 在由本地表组成的分布式表中搜索
* 通知查询调用（Percolate query call）
* 以及其他

默认情况下，该值设为服务器CPU核心数。Manticore启动时创建线程并保持它们直到停止。每个子任务在需要时可以使用一个线程。子任务完成后释放线程，供其他子任务使用。

对于密集的I/O负载，设置高于CPU核心数可能更合适。

<!-- request Example -->
```ini
threads = 10
```

<!-- end -->

### thread_stack

<!-- example conf thread_stack -->
作业的最大栈大小（协程，一个搜索查询可能引起多个作业/协程）。可选，默认是128K。

每个作业都有自己的128K栈。当运行一个查询时，会检查它需要多少栈空间。如果默认128K足够，直接处理；如果需要更多，则调度另一个栈更大的作业继续处理。栈大小的最大值受此设置限制。

合理提高此值可以处理非常深层查询，同时不会导致整体RAM消耗过高。例如，设为1G并不意味着每个新作业都会占用1G内存，而是如果作业需要，比如100M栈空间，则只给它分配100M。与此同时，其他作业则用默认的128K栈。同理，可以运行更复杂需要500M的查询。仅当内部检测到作业需要超过1G栈时，才会失败并报告 thread_stack 过小。

不过实际中，使用16M栈甚至是过于复杂的查询，解析起来耗时且资源巨大。所以守护进程会处理它，但用 `thread_stack` 限制此类查询是合理的。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
thread_stack = 8M
```
<!-- end -->


### unlink_old

<!-- example conf unlink_old -->
确定在成功轮转时是否解除链接 `.old` 表副本。可选，默认是1（执行解除链接）。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
unlink_old = 0
```
<!-- end -->


### watchdog

<!-- example conf watchdog -->
多线程服务器看门狗。可选，默认是1（启用看门狗）。

当Manticore查询崩溃时，可能导致整个服务器崩溃。启用看门狗功能后，`searchd` 会维护一个单独的轻量级进程，监视主服务器进程并在异常终止时自动重启。看门狗默认启用。

<!-- request Example -->

```ini
watchdog = 0 # disable watchdog
```
<!-- end -->
<!-- proofread -->

