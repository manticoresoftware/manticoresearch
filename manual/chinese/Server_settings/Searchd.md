# 配置中的 "Searchd" 部分

以下设置用于 Manticore Search 配置文件的 `searchd` 部分，以控制服务器的行为。以下是每个设置的摘要：

### access_plain_attrs

此设置为 [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例级默认值。它是可选的，默认值为 `mmap_preread`。

`access_plain_attrs` 指令允许您为此 searchd 实例管理的所有表定义 [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令具有更高的优先级，将优先于此实例级默认值，从而提供更细粒度的控制。

### access_blob_attrs

此设置为 [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例级默认值。它是可选的，默认值为 `mmap_preread`。

`access_blob_attrs` 指令允许您为此 searchd 实例管理的所有表定义 [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令具有更高的优先级，将优先于此实例级默认值，从而提供更细粒度的控制。

### access_doclists

此设置为 [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例级默认值。它是可选的，默认值为 `file`。

`access_doclists` 指令允许您为此 searchd 实例管理的所有表定义 [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令具有更高的优先级，将优先于此实例级默认值，从而提供更细粒度的控制。

### access_hitlists

此设置为 [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例级默认值。它是可选的，默认值为 `file`。

`access_hitlists` 指令允许您为此 searchd 实例管理的所有表定义 [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令具有更高的优先级，将优先于此实例级默认值，从而提供更细粒度的控制。

### access_dict

此设置为 [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例级默认值。它是可选的，默认值为 `mmap_preread`。

`access_dict` 指令允许您为此 searchd 实例管理的所有表定义 [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令具有更高的优先级，将优先于此实例级默认值，从而提供更细粒度的控制。

### agent_connect_timeout

此设置为 [agent_connect_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) 参数设置实例级默认值。


### agent_query_timeout

此设置为 [agent_query_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) 参数设置实例级默认值。可以使用 `OPTION agent_query_timeout=XXX` 子句按查询覆盖此值。


### agent_retry_count

此设置是一个整数，指定 Manticore 将尝试通过分布式表连接和查询远程代理的次数，在报告致命查询错误之前。默认值为 0（即不重试）。您还可以使用 `OPTION retry_count=XXX` 子句按查询设置此值。如果提供了按查询选项，则它将覆盖配置中指定的值。

请注意，如果您在分布式表的定义中使用了 [agent mirrors](../Creating_a_cluster/Remote_nodes/Mirroring.md#Agent-mirrors)，服务器将根据所选的 [ha_strategy](../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) 为每次连接尝试选择一个不同的镜像。在这种情况下，`agent_retry_count` 将在一组所有镜像中汇总。

例如，如果您有 10 个镜像并设置 `agent_retry_count=5`，则服务器将最多重试 50 次，假设每个 10 个镜像平均尝试 5 次（在 `ha_strategy = roundrobin` 选项下，将是这种情况）。

然而，提供作为 [agent](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) 的 `retry_count` 选项的值则作为绝对限制。换句话说，代理定义中的 `[retry_count=2]` 选项始终意味着最多 2 次尝试，无论您为代理指定了 1 个还是 10 个镜像。

### agent_retry_delay

此设置是一个以毫秒为单位的整数（或 [special_suffixes](../Server_settings/Special_suffixes.md)）指定在 Manticore 重试查询远程代理时的延迟。此值仅在指定了非零的 [agent_retry_count](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 或非零的每查询 `retry_count` 时相关。默认值为 500。您还可以使用 `OPTION retry_delay=XXX` 子句按查询设置此值。如果提供了按查询选项，则它将覆盖配置中指定的值。


### attr_flush_period

<!-- example conf attr_flush_period -->
当使用 [Update](../Data_creation_and_modification/Updating_documents/UPDATE.md) 实时修改文档属性时，更改首先被写入内存中属性的副本。这些更新发生在内存映射文件中，意味着操作系统决定何时将更改写入磁盘。在 `searchd` 正常关闭时（由 `SIGTERM` 信号触发），所有更改将被强制写入磁盘。

您还可以指示 `searchd` 定期将这些更改写回磁盘，以防止数据丢失。这些刷新之间的间隔由 `attr_flush_period` 确定，以秒为单位（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。

默认情况下，值为 0，这会禁用定期刷新。然而，在正常关闭期间仍会进行刷新。

<!-- intro -->

##### 示例：

<!-- request Example -->

```ini

attr_flush_period = 900 # 每 15 分钟将更新持久化到磁盘

```

<!-- end -->

### auto_optimize

<!-- example conf auto_optimize -->

此设置控制表压缩的自动 [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 过程。

默认情况下，表压缩会自动发生。您可以通过 `auto_optimize` 设置修改此行为：

* 0 禁用自动表压缩（您仍然可以手动调用 `OPTIMIZE`）

* 1 明确启用它

* 2 在将优化阈值乘以 2 的同时启用它。

默认情况下，OPTIMIZE 会一直运行，直到磁盘块的数量小于或等于逻辑 CPU 核心数量乘以 2。

然而，如果表具有带有 KNN 索引的属性，则该阈值会有所不同。在这种情况下，它被设置为物理 CPU 核心数量除以 2，以提高 KNN 搜索性能。

请注意，切换 `auto_optimize` 的启用或禁用并不会阻止您手动运行 [OPTIMIZE TABLE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE)。

<!-- intro -->

##### 示例：

<!-- request Disable -->

```ini

auto_optimize = 0 # 禁用自动 OPTIMIZE

```

<!-- request Throttle -->

```ini

auto_optimize = 2 # OPTIMIZE 在 16 个块（在 4 个 CPU 核心的服务器上）开始

```

<!-- end -->

### auto_schema

<!-- example conf auto_schema -->

Manticore 支持自动创建尚不存在但在 INSERT 语句中指定的表。此功能默认启用。要禁用它，请在配置中明确设置 `auto_schema = 0`。要重新启用它，请设置 `auto_schema = 1` 或从配置中删除 `auto_schema` 设置。

请记住， `/bulk` HTTP 端点不支持自动表创建。

> 注意： [自动模式功能](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保 Buddy 已安装。

<!-- request Disable -->

```ini

auto_schema = 0 # 禁用自动表创建

```

<!-- request Enable -->

```ini

auto_schema = 1 # 启用自动表创建

```

<!-- end -->

### binlog_flush

<!-- example conf binlog_flush -->

此设置控制二进制日志事务刷新/同步模式。它是可选的，默认值为 2（每个事务刷新，每秒同步一次）。

此指令决定二进制日志多频繁地显示到操作系统并同步到磁盘。支持三种模式：

*  0，每秒刷新和同步。这提供了最佳性能，但在服务器崩溃或操作系统/硬件崩溃的情况下，最多可能会丢失 1 秒的已提交事务。

*  1，每个事务刷新和同步。此模式提供最差的性能，但保证保存每个已提交事务的数据。

*  2，每个事务刷新，每秒同步。此模式提供良好的性能，并确保在服务器崩溃的情况下保存每个已提交的事务。然而，在操作系统/硬件崩溃的情况下，最多可能会丢失 1 秒的已提交事务。

对于熟悉 MySQL 和 InnoDB 的人来说，此指令类似于 `innodb_flush_log_at_trx_commit`。在大多数情况下，默认的混合模式 2 提供了速度和安全性的良好平衡，完全保护 RT 表数据不受服务器崩溃的影响，并对硬件故障提供一些保护。

<!-- intro -->

##### 示例：

<!-- request Example -->

```ini

binlog_flush = 1 # 终极安全，低速度

```

<!-- end -->

### binlog_max_log_size

<!-- example conf binlog_max_log_size -->

此设置控制最大二进制日志文件大小。它是可选的，默认值为 256 MB。

一旦当前的 binlog 文件达到此大小限制，将强制打开一个新的 binlog 文件。这将导致日志的细粒度，并可能在某些临界负载下导致更有效的 binlog 磁盘使用。值为 0 表示 binlog 文件不应根据大小重新打开。

<!-- intro -->

##### 示例：

<!-- request Example -->

```ini

binlog_max_log_size = 16M

```

<!-- end -->

### binlog_path

<!-- example conf binlog_path -->

此设置决定二进制日志（也称为事务日志）文件的路径。它是可选的，默认值为构建时配置的数据目录（例如，在 Linux 中为 `/var/lib/manticore/data/binlog.*`）。

二进制日志用于 RT 表数据的崩溃恢复和普通磁盘索引的属性更新，这些索引在刷新之前只会保存在 RAM 中。当启用日志记录时，写入 RT 表的每个事务的 COMMIT 将被记录到日志文件中。然后，在不干净关闭后的启动时，日志会被自动重放，从而恢复记录的更改。
`binlog_path` 指令指定二进制日志文件的位置。它应该只包含路径；`searchd` 将根据需要在目录中创建和删除多个 `binlog.*` 文件（包括二进制日志数据、元数据和锁文件等）。

空值将禁用二进制日志记录，这会提高性能，但会使 RT 表数据面临风险。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
binlog_path = # 禁用日志记录
binlog_path = /var/lib/manticore/data # 将创建 /var/lib/manticore/data/binlog.001 等
```
<!-- end -->


### buddy_path

<!-- example conf buddy_path -->
此设置确定 Manticore Buddy 二进制文件的路径。它是可选的，默认值是根据构建时配置的路径，这在不同操作系统中有所不同。通常，您不需要修改此设置。然而，如果您希望以调试模式运行 Manticore Buddy、对其进行更改或实现新插件，则可能会很有用。在后一种情况下，您可以从 https://github.com/manticoresoftware/manticoresearch-buddy `git clone` Buddy，将新插件添加到 `./plugins/` 目录，并在更改目录到 Buddy 源后运行 `composer install --prefer-source` 以便于开发。

为了确保您可以运行 `composer`，您的机器必须安装 PHP 8.2 或更高版本，并具有以下扩展：

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

您还可以选择 Linux amd64 上的特殊 `manticore-executor-dev` 版本，具体可在发布页面找到，例如： https://github.com/manticoresoftware/executor/releases/tag/v1.0.13

如果您选择此路线，请记得将 manticore 执行器的开发版本链接到 `/usr/bin/php`。

要禁用 Manticore Buddy，请将值设置为空，如示例所示。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
buddy_path = manticore-executor -n /usr/share/manticore/modules/manticore-buddy/src/main.php --debug # 在 Linux 中使用默认的 Manticore Buddy，但以调试模式运行
buddy_path = manticore-executor -n /opt/homebrew/share/manticore/modules/manticore-buddy/bin/manticore-buddy/src/main.php --debug # 在 MacOS arm64 中使用默认的 Manticore Buddy，但以调试模式运行
buddy_path = manticore-executor -n /Users/username/manticoresearch-buddy/src/main.php --debug # 从非默认位置使用 Manticore Buddy
buddy_path = # 禁用 Manticore Buddy
buddy_path = manticore-executor -n /Users/username/manticoresearch-buddy/src/main.php --debugv --skip=manticoresoftware/buddy-plugin-replace # debugv - 启用更详细的日志记录，--skip - 跳过插件
```
<!-- end -->

### client_timeout

<!-- example conf client_timeout -->
此设置确定在使用持久连接时，请求之间等待的最长时间（以秒或 [special_suffixes](../Server_settings/Special_suffixes.md) 为单位）。它是可选的，默认值为五分钟。


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

指定 libc 区域设置，影响基于 libc 的排序规则。有关详细信息，请参见 [collations](../Searching/Collations.md) 部分。


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

指定用于传入请求的默认排序规则。排序规则可以在每个查询的基础上被重写。有关可用排序规则和其他详细信息的列表，请参见 [collations](../Searching/Collations.md) 部分。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
collation_server = utf8_ci
```
<!-- end -->


### data_dir

<!-- example conf data_dir -->
指定此设置时，将启用 [实时模式](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)，这是管理数据架构的一种强制方式。值应为您希望存储所有表、二进制日志以及此模式下 Manticore Search 正常运行所需的其他所有内容的目录路径。
在指定 `data_dir` 时，不允许对 [普通表](../Creating_a_table/Local_tables/Plain_table.md) 进行索引。有关 RT 模式与普通模式之间的区别，请在 [本节](../Read_this_first.md#Real-time-table-vs-plain-table) 中阅读更多信息。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
data_dir = /var/lib/manticore
```
<!-- end -->

### diskchunk_flush_search_timeout

<!-- example conf diskchunk_flush_search_timeout -->
防止在表中没有搜索时自动刷新 RAM 块的超时。可选，默认值为 30 秒。

检查搜索的时间，以确定是否自动刷新。
仅当在过去的 `diskchunk_flush_search_timeout` 秒内在表中至少进行过一次搜索时，才会发生自动刷新。此项与 [diskchunk_flush_write_timeout](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout) 一起工作。相应的 [每表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_search_timeout) 优先级更高，将覆盖此实例级的默认值，从而提供更细粒度的控制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
diskchunk_flush_search_timeout = 120s
```
<!-- end -->

### diskchunk_flush_write_timeout

<!-- example conf diskchunk_flush_write_timeout -->
在没有写入的情况下，自动将 RAM 块刷新到磁盘之前等待的时间（以秒为单位）。可选，默认值为 1 秒。

如果在 `diskchunk_flush_write_timeout` 秒内未发生写入，块将被刷新到磁盘。与 [diskchunk_flush_search_timeout](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout) 一起工作。要禁用自动刷新，请在配置中显式设置 `diskchunk_flush_write_timeout = -1`。相应的 [每表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_write_timeout) 优先级更高，将覆盖此实例级的默认值，以提供更细粒度的控制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
diskchunk_flush_write_timeout = 60s
```
<!-- end -->

### docstore_cache_size

<!-- example conf docstore_cache_size -->
此设置指定从文档存储中保持在内存中的文档块的最大大小。可选，默认值为 16m（16 兆字节）。

当使用 `stored_fields` 时，文档块从磁盘读取并解压缩。由于每个块通常包含几个文档，因此在处理下一个文档时可以重用该块。为此，该块被保存在整个服务器的缓存中。缓存中保存未压缩的块。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
docstore_cache_size = 8m
```
<!-- end -->

### engine

<!-- example conf engine -->
在 RT 模式下创建表时使用的默认属性存储引擎。可以是 `rowwise`（默认）或 `columnar`。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
engine = columnar
```
<!-- end -->


### expansion_limit

<!-- example conf expansion_limit -->
此设置确定单个通配符的最大扩展关键词数。可选，默认值为 0（无限制）。

当对启用 `dict = keywords` 的表执行子字符串搜索时，单个通配符可能会导致成千上万甚至数百万个匹配的关键词（想象将 `a*` 与整个牛津词典进行匹配）。该指令允许您限制此类扩展的影响。设置 `expansion_limit = N` 将扩展限制为不超过 N 个最常见的匹配关键词（每个查询中的每个通配符）。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_limit = 16
```
<!-- end -->

### expansion_merge_threshold_docs

<!-- example conf expansion_merge_threshold_docs -->
此设置确定扩展关键词中最大文档数，这允许将所有此类关键词合并在一起。可选，默认值为 32。

当对启用 `dict = keywords` 的表执行子字符串搜索时，单个通配符可能会导致成千上万甚至数百万个匹配的关键词。该指令允许您增加合并在一起的关键词数量限制，以加快匹配速度，但在搜索中会使用更多内存。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_merge_threshold_docs = 1024
```
<!-- end -->

### expansion_merge_threshold_hits

<!-- example conf expansion_merge_threshold_hits -->
此设置确定扩展关键词中最大命中数，这允许将所有此类关键词合并在一起。可选，默认值为 256。

当对启用 `dict = keywords` 的表执行子字符串搜索时，单个通配符可能会导致成千上万甚至数百万个匹配的关键词。该指令允许您增加合并在一起的关键词数量限制，以加快匹配速度，但在搜索中会使用更多内存。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_merge_threshold_hits = 512
```
<!-- end -->

### grouping_in_utc

此设置指定在 API 和 SQL 中的时间分组是否将以本地时区或 UTC 计算。可选，默认值为 0（表示“本地时区”）。

默认情况下，所有“按时间分组”的表达式（如在 API 中按天、周、月和年分组，以及在 SQL 中按天、月、年、年月和日期分组）都是使用本地时间进行的。例如，如果您有属性定时为 `13:00 utc` 和 `15:00 utc` 的文档，在分组的情况下，它们将根据您的本地时区设置落入设施组。如果您生活在 `utc`，它将是一天，但如果您生活在 `utc+10`，那么这些文档将被匹配到不同的“按天分组”设施组（因为 UTC+10 时区的 13:00 utc 是本地时间的 23:00，但 15:00 是第二天的 01:00）。有时这种行为是不可接受的，期望时间分组不依赖于时区。您可以使用定义的全局 TZ 环境变量运行服务器，但这不仅会影响分组，还会影响日志中的时间戳，这也可能是不可取的。开启此选项（在配置中或使用 [SET global](../Server_settings/Setting_variables_online.md#SET) 语句在 SQL 中）将导致所有时间分组表达式在 UTC 中计算，而将其他依赖时间的函数（即服务器的日志记录）保持在本地时区。


### timezone

此设置指定由日期/时间相关函数使用的时区。默认情况下使用本地时区，但您可以在 IANA 格式中指定其他时区（例如，`Europe/Amsterdam`）。

请注意，此设置对日志记录没有影响，日志记录始终在本地时区中运行。
另外，请注意，如果使用`grouping_in_utc`，则“按时间分组”功能仍将使用UTC，而其他与日期/时间相关的功能将使用指定的时区。总体来说，不建议混合使用`grouping_in_utc`和`timezone`。

您可以通过配置文件或在SQL中使用[SET global](../Server_settings/Setting_variables_online.md#SET)语句来配置这个选项。


### ha_period_karma

<!-- example conf ha_period_karma -->
此设置指定代理镜像统计窗口大小，以秒为单位（或[特殊后缀](../Server_settings/Special_suffixes.md)）。这是可选的，默认值为60秒。

对于包含代理镜像的分布式表（在[agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)中了解更多），主节点跟踪多个不同的每个镜像计数器。然后使用这些计数器进行故障转移和平衡（主节点根据计数器选择最佳镜像使用）。计数器以`ha_period_karma`秒的块进行累积。

开始新的块后，主节点仍可以使用前一个块的累积值，直到新的块填充到一半。因此，任何先前的历史在最多1.5倍的ha_period_karma秒后停止影响镜像选择。

尽管在镜像选择中最多使用两个块，但出于仪器目的，最多存储15个最后的块。这些块可以通过[SHOW AGENT STATUS](../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS)语句检查。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_period_karma = 2m
```
<!-- end -->


### ha_ping_interval

<!-- example conf ha_ping_interval -->
此设置配置代理镜像ping之间的间隔，以毫秒为单位（或[特殊后缀](../Server_settings/Special_suffixes.md)）。这是可选的，默认值为1000毫秒。

对于包含代理镜像的分布式表（在[agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)中了解更多），主节点在空闲期间向所有镜像发送ping命令。这是为了跟踪当前代理状态（存活或死亡，网络往返时间等）。这样的ping之间的间隔由此指令定义。要禁用ping，将ha_ping_interval设置为0。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_ping_interval = 3s
```
<!-- end -->


### hostname_lookup

`hostname_lookup`选项定义更新主机名的策略。默认情况下，代理主机名的IP地址在服务器启动时被缓存，以避免过度访问DNS。但是在某些情况下，IP可能会动态变化（例如，云托管），可能希望不缓存IP。将此选项设置为`request`将禁用缓存并在每次查询时查询DNS。IP地址也可以通过`FLUSH HOSTNAMES`命令手动更新。

### jobs_queue_size

jobs_queue_size设置定义同一时间可以在队列中的“作业”数量。默认为无限制。

在大多数情况下，“作业”意味着对单个本地表（普通表或实时表的磁盘块）的一次查询。例如，如果您有一个由2个本地表组成的分布式表或一个具有2个磁盘块的实时表，则对它们中的任一个的搜索查询大多会将2个作业放入队列中。然后，线程池（其大小由[threads](../Server_settings/Searchd.md#threads)定义）将处理它们。但是在某些情况下，如果查询过于复杂，则可能会创建更多作业。当[max_connections](../Server_settings/Searchd.md#max_connections)和[threads](../Server_settings/Searchd.md#threads)不足以在所需性能之间找到平衡时，建议更改此设置。

### join_batch_size

表连接通过累积一批匹配项来工作，这些匹配项是对左侧表执行的查询的结果。然后该批处理作为对右侧表的单个查询进行处理。

此选项允许您调整批处理大小。默认值为`1000`，将此选项设置为`0`将禁用批处理。

较大的批处理大小可能会提高性能；然而，对于某些查询，它可能导致过度的内存消耗。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
join_batch_size = 2000
```
<!-- end -->

### join_cache_size

每个在右侧表上执行的查询由特定的JOIN ON条件定义，这些条件决定了从右侧表检索的结果集。

如果只有少数唯一的JOIN ON条件，复用结果可能比重复执行右侧表上的查询更有效。为此，结果集存储在缓存中。

此选项允许您配置此缓存的大小。默认值为`20 MB`，将此选项设置为0将禁用缓存。

请注意，每个线程维护自己的缓存，因此在估算总内存使用时应考虑执行查询的线程数量。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
join_cache_size = 10M
```
<!-- end -->

### listen_backlog

<!-- example conf listen_backlog -->
listen_backlog设置确定传入连接的TCP监听后备队列的长度。这在按顺序处理请求的Windows构建中特别相关。当连接队列达到极限时，新传入连接将被拒绝。
对于非Windows构建，默认值应该可以正常工作，通常无需调整此设置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
listen_backlog = 20
```
<!-- end -->

### kibana_version_string

<!-- example conf kibana_version_string -->
将返回给Kibana或OpenSearch仪表板的服务器版本字符串。可选—默认设置为`7.6.0`。

某些版本的Kibana和OpenSearch仪表板期望服务器报告特定的版本号，并可能根据版本号表现不同。为了绕过这类问题，您可以使用此设置，使Manticore向Kibana或OpenSearch仪表板报告自定义版本。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
kibana_version_string = 1.2.3
```
<!-- end -->

### listen

<!-- example conf listen -->
此设置允许您指定Manticore将接受连接的IP地址和端口，或Unix域套接字路径。

`listen`的一般语法为：

```ini
listen = ( address ":" port | port | path | address ":" port start - port end ) [ ":" protocol [ "_vip" ] [ "_readonly" ] ]
```

您可以指定：
* 一个IP地址（或主机名）和端口号
* 或者仅仅是一个端口号
* 或者一个Unix套接字路径（在Windows上不支持）
* 或者一个IP地址和端口范围

如果您指定端口号但未指定地址，`searchd`将监听所有网络接口。Unix路径由前导斜杠标识。端口范围只能为复制协议设置。

您还可以指定要用于此套接字连接的协议处理程序（监听器）。监听器包括：

* **未指定** - Manticore将接受来自以下来源的连接：
  - 其他Manticore代理（即，远程分布式表）
  - 通过HTTP和HTTPS的客户端
  - [Manticore Buddy](https://manticoresearch.com/blog/manticoresearch-buddy-intro/)。**确保您有这种类型的监听器（或下面提到的`http`监听器），以避免Manticore功能的限制。**
* `mysql` MySQL协议，用于来自MySQL客户端的连接。注意：
  - 也支持压缩协议。
  - 如果启用了[SSL](../Security/SSL.md#SSL)，您可以建立加密连接。
* `replication` - 用于节点之间通信的复制协议。更多细节可以在[复制](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md)部分找到。您可以指定多个复制监听器，但它们必须都监听同一IP；只能端口不同。当您定义一个具有端口范围的复制监听器（例如，`listen = 192.168.0.1:9320-9328:replication`）时，Manticore不会立即开始监听这些端口。相反，它将仅在您开始使用复制时从指定范围中随机选择空闲端口。复制正常工作至少需要2个端口在范围内。
* `http` - 与**未指定**相同。Manticore将接受来自远程代理和通过HTTP和HTTPS的客户端的连接。
* `https` - HTTPS协议。Manticore仅将在此端口接受HTTPS连接。更多详细信息请参阅[SSL](../Security/SSL.md)部分。
* `sphinx` - 传统二进制协议。用于服务来自远程[SphinxSE](../Extensions/SphinxSE.md)客户端的连接。一些Sphinx API客户端实现（例如Java的实现）要求明确声明监听器。

为客户端协议添加后缀`_vip`（即，所有除`replication`以外的协议，例如`mysql_vip`或`http_vip`或仅`_vip`）将强制为连接创建专用线程，以绕过不同的限制。这对于在严重过载的情况下进行节点维护是有用的，在这种情况下，服务器要么会停滞，要么不会让您通过常规端口连接。

后缀`_readonly`为监听器设置[只读模式](../Security/Read_only.md)，并限制其只接受读取查询。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
listen = localhost
listen = localhost:5000 # 在localhost的5000端口监听远程代理（二进制API）和http/https请求
listen = 192.168.0.1:5000 # 在192.168.0.1的5000端口监听远程代理（二进制API）和http/https请求
listen = /var/run/manticore/manticore.s # 在unix套接字上监听二进制API请求
listen = /var/run/manticore/manticore.s:mysql # 在unix套接字上监听mysql请求
listen = 9312 # 在任何接口的9312端口监听远程代理（二进制API）和http/https请求
listen = localhost:9306:mysql # 在localhost的9306端口监听mysql请求
listen = localhost:9307:mysql_readonly # 在localhost的9307端口监听mysql请求并只接受读取查询
listen = 127.0.0.1:9308:http # 在localhost的9308端口监听http请求以及来自远程代理（和二进制API）的连接
listen = 192.168.0.1:9320-9328:replication # 在192.168.0.1的9320-9328端口监听复制连接
listen = 127.0.0.1:9443:https # 在127.0.0.1的9443端口监听https请求（不包括http）
listen = 127.0.0.1:9312:sphinx # 在127.0.0.1的9312端口监听传统Sphinx请求（例如来自SphinxSE）
```
<!-- end -->

可以有多个`listen`指令。`searchd`将监听所有指定端口和套接字的客户端连接。Manticore软件包中提供的默认配置定义了在以下端口上监听：
* `9308`和`9312`用于来自远程代理和非MySQL基础客户的连接
* 在`9306`端口用于MySQL连接。

如果在配置中根本不指定任何`listen`，Manticore将等待在以下端口的连接：
* `127.0.0.1:9306`用于MySQL客户端
* `127.0.0.1:9312`用于HTTP/HTTPS和来自其他Manticore节点及基于Manticore二进制API的客户端的连接。

#### 在特权端口上监听
默认情况下，Linux 不允许您让 Manticore 监听 1024 以下的端口（例如 `listen = 127.0.0.1:80:http` 或 `listen = 127.0.0.1:443:https`），除非在 root 下运行 searchd。如果您仍然希望能够在非 root 用户下启动 Manticore 并监听 < 1024 的端口，可以考虑以下方法（这两种方法都应该有效）：
* 运行命令 `setcap CAP_NET_BIND_SERVICE=+eip /usr/bin/searchd`
* 在 Manticore 的 systemd 单元中添加 `AmbientCapabilities=CAP_NET_BIND_SERVICE` 并重新加载守护进程（`systemctl daemon-reload`）。

#### 关于 Sphinx API 协议和 TFO 的技术细节
<details>
传统的 Sphinx 协议有 2 个阶段：握手交换和数据流。握手由客户端的 4 字节数据包和守护进程的 4 字节数据包组成，其唯一目的是：客户端确定远程是真正的 Sphinx 守护进程，守护进程确定远程是真正的 Sphinx 客户端。主数据流相当简单：让双方声明自己的握手，然后对方检查。这种使用短数据包的交换意味着使用特殊的 `TCP_NODELAY` 标志，该标志关闭 Nagle 的 TCP 算法，并声明 TCP 连接将作为小数据包的对话进行。
然而，谁先在这个协商中开始并没有严格定义。历史上，所有使用二进制 API 的客户端都先说话：发送握手，然后从守护进程读取 4 字节，然后发送请求并从守护进程读取答案。
当我们改进 Sphinx 协议兼容性时，我们考虑了以下几点：

1. 通常，主从通信是从已知客户端建立到已知主机的已知端口。因此，端点提供错误握手的可能性很小。所以，我们可以隐式地假设双方都是有效的，并且确实在使用 Sphinx 协议。
2. 基于这个假设，我们可以将握手与实际请求"粘合"在一起，并在一个数据包中发送。如果后端是传统的 Sphinx 守护进程，它将简单地将这个粘合的数据包读取为 4 字节的握手，然后是请求体。由于它们都在一个数据包中，后端套接字减少了 1 个 RTT，前端缓冲区仍然可以工作。
3. 继续这个假设：由于"查询"数据包相当小，握手更小，我们可以使用现代 TFO（tcp-fast-open）技术在初始 'SYN' TCP 包中发送两者。也就是说：我们连接到远程节点，并携带粘合的握手 + 请求体数据包。守护进程接受连接，并立即在套接字缓冲区中拥有握手和请求体，因为它们来自第一个 TCP 'SYN' 数据包。这消除了另一个 RTT。
4. 最后，让守护进程接受这种改进。实际上，从应用程序来看，这意味着不使用 `TCP_NODELAY`。从系统角度来看，这意味着确保守护进程端启用接受 TFO，客户端端启用发送 TFO。在现代系统中，默认情况下客户端 TFO 已经激活，所以您只需要调整服务器 TFO 即可使一切正常工作。

所有这些改进都没有实际改变协议本身，但允许我们从 TCP 协议连接中消除 1.5 个 RTT。如果查询和答案能够放在一个单一的 TCP 数据包中，则整个二进制 API 会话从 3.5 个 RTT 减少到 2 个 RTT - 这使得网络协商快了大约 2 倍。

因此，我们所有的改进都围绕着一个最初未定义的陈述：'谁先说话'。如果客户端先说话，我们可以应用所有这些优化，并在单个 TFO 数据包中有效地处理连接 + 握手 + 查询。此外，我们可以查看接收数据包的开头并确定真实协议。这就是为什么您可以通过 API/http/https 连接到同一个端口。如果守护进程必须先说话，所有这些优化都是不可能的，多协议也是不可能的。这就是为什么我们为 MySQL 使用了专用端口，并且没有将其与所有其他协议统一到同一端口。不幸的是，在所有客户端中，有一个假定守护进程应该先发送握手的客户端。这样就无法进行所有所描述的改进。那就是 MySQL/MariaDB 的 SphinxSE 插件。所以，专门为这个单一客户端，我们定义了 `sphinx` 协议以最传统的方式工作。具体来说：双方激活 `TCP_NODELAY` 并交换小数据包。守护进程在连接时发送其握手，然后客户端发送其握手，然后一切按常规方式工作。这不是非常优化，但可以正常工作。如果您使用 SphinxSE 连接到 Manticore - 您必须使用明确指定 `sphinx` 协议的监听器。对于其他客户端 - 避免使用这个监听器，因为它较慢。如果您使用其他传统的 Sphinx API 客户端 - 请先检查它们是否能够使用非专用的多协议端口。对于使用非专用（多协议）端口的主从链接并启用客户端和服务器 TFO，这可以很好地工作，并且肯定会使网络后端更快，特别是对于非常轻量和快速的查询。
</details>

### listen_tfo

此设置允许所有监听器使用 TCP_FASTOPEN 标志。默认情况下由系统管理，但可以通过设置为 '0' 显式关闭。

关于 TCP Fast Open 扩展的一般知识，请查阅 [维基百科](https://en.wikipedia.org/wiki/TCP_Fast_Open)。简而言之，它允许在建立连接时消除一个 TCP 往返。

在实践中，在许多情况下使用 TFO 可以优化客户端-代理网络效率，特别是在使用[持久代理](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)时，但不保持活动连接，并且不限制连接的最大数量。

在现代操作系统中，TFO 支持通常在系统级别开启，但这仅仅是一种“能力”，而非规则。Linux（作为最进步的）自 2011 年起就支持这一功能，内核版本从 3.7 开始（用于服务器端）。Windows 从某些版本的 Windows 10 开始支持该功能。其他操作系统（FreeBSD、MacOS）也参与其中。

对于 Linux 系统，服务器检查变量 `/proc/sys/net/ipv4/tcp_fastopen` 并根据其行为。位 0 管理客户端，位 1 管理监听器。默认情况下，系统将此参数设置为 1，即客户端启用，监听器禁用。

### log

<!-- example conf log -->
日志设置指定了将记录所有 `searchd` 运行时事件的日志文件名称。如果未指定，则默认名称为 'searchd.log'。

另外，您可以使用 'syslog' 作为文件名。在这种情况下，事件将发送到 syslog 守护进程。要使用 syslog 选项，您需要在构建期间将 Manticore 配置为 `-–with-syslog` 选项。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
log = /var/log/searchd.log
```
<!-- end -->


### max_batch_queries

<!-- example conf max_batch_queries -->
每批查询的最大限制。可选，默认值为 32。

使 searchd 对使用 [multi-queries](../Searching/Multi-queries.md) 的单个批次中提交的查询数量执行有效性检查。将其设置为 0 可以跳过检查。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_batch_queries = 256
```
<!-- end -->

### max_connections

<!-- example max_connections -->
同时客户端连接的最大数量。默认情况下不限制。通常只有在使用任何类型的持久连接时才会注意到，例如 cli mysql 会话或来自远程分布式表的持久远程连接。当超出限制时，仍然可以使用 [VIP 连接](../Connecting_to_the_server/MySQL_protocol.md#VIP-connection) 连接到服务器。VIP 连接不计算在限制内。

<!-- request Example -->
```ini
max_connections = 10
```

<!-- end -->

### max_threads_per_query

<!-- example max_threads_per_query -->
单个操作可以使用的线程的实例级限制。默认情况下，适当的操作可以占用所有 CPU 核心，不留空间给其他操作。例如，对相当大的渗透表执行 `call pq` 可能会使用所有线程持续数十秒。将 `max_threads_per_query` 设置为预计的一半 [threads](../Server_settings/Searchd.md#threads) 将确保您可以并行运行几个这样的 `call pq` 操作。

您还可以在运行时将此设置作为会话或全局变量进行设置。

此外，您还可以通过 [threads OPTION](../Searching/Options.md#threads) 在每个查询的基础上控制行为。

<!-- intro -->
##### 示例：
<!-- request Example -->

```ini
max_threads_per_query = 4
```

<!-- end -->

### max_filters

<!-- example conf max_filters -->
每查询允许的最大过滤器数量。此设置仅用于内部有效性检查，不直接影响 RAM 使用或性能。可选，默认值为 256。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_filters = 1024
```
<!-- end -->


### max_filter_values

<!-- example conf max_filter_values -->
每过滤器允许的最大值数量。此设置仅用于内部有效性检查，不直接影响 RAM 使用或性能。可选，默认值为 4096。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_filter_values = 16384
```
<!-- end -->


### max_open_files

<!-- example conf max_open_files -->
服务器允许打开的最大文件数量称为“软限制”。请注意，服务大型分段实时表可能需要将此限制设置得较高，因为每个磁盘块可能占用十个或更多文件。例如，一个具有 1000 个块的实时表可能需要同时打开数千个文件。如果您在日志中遇到“打开文件过多”的错误，尝试调整此选项，这可能有助于解决问题。

还有一个“硬限制”，该限制不能被选项超过。此限制由系统定义，并可以在 Linux 的文件 `/etc/security/limits.conf` 中更改。其他操作系统可能有不同的方法，因此请查阅您的手册以获取更多信息。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = 10000
```
<!-- end -->

<!-- example conf max_open_files max -->
除了直接的数值，您还可以使用“max”这个魔法词将限制设置为当前可用的硬限制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = max
```
<!-- end -->


### max_packet_size

<!-- example conf max_packet_size -->
允许的最大网络数据包大小。此设置限制来自客户端的查询数据包和来自分布式环境中远程代理的响应数据包。仅用于内部有效性检查，不直接影响 RAM 使用或性能。可选，默认值为 128M。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_packet_size = 32M
```
<!-- end -->


### mysql_version_string

<!-- example conf mysql_version_string -->
通过 MySQL 协议返回的服务器版本字符串。可选，默认值为空（返回 Manticore 版本）。
几个挑剔的 MySQL 客户端库依赖于 MySQL 使用的特定版本号格式，而且，有时会根据报告的版本号（而不是指示的能力标志）选择不同的执行路径。例如，Python MySQLdb 1.2.2 在版本号不是 X.Y.ZZ 格式时会抛出异常；MySQL .NET 连接器 6.3.x 在版本号为 1.x 时以及某些标志组合下会内部失败等。为了解决这个问题，可以使用 `mysql_version_string` 指令，让 `searchd` 向通过 MySQL 协议连接的客户端报告不同的版本。（默认情况下，它报告其自己的版本。）


<!-- intro -->
##### 示例:

<!-- request Example -->

```ini
mysql_version_string = 5.0.37
```
<!-- end -->


### net_workers

网络线程数量，默认值为 1。

此设置对于极高查询率的情况非常有用，当仅一个线程无法处理所有传入查询时。


### net_wait_tm

控制网络线程的忙等待间隔。默认值为 -1，可以设置为 -1、0 或正整数。

在服务器配置为纯主服务器并仅将请求路由到代理的情况下，处理请求时必须毫无延迟，并且不允许网络线程休眠。为此有一个忙等待循环。在接收请求后，如果 `net_wait_tm` 是正数，则网络线程会在 `10 * net_wait_tm` 毫秒内使用 CPU 循环，如果 `net_wait_tm` 为 `0`，则仅使用 CPU 轮询。此外，可以通过 `net_wait_tm = -1` 禁用忙等待循环 - 在这种情况下，轮询器将超时设置为系统轮询调用上实际代理的超时。

> **警告:** CPU 忙等待循环实际上会加载 CPU 核心，因此将该值设置为任何非默认值，即使在空闲服务器上也会导致明显的 CPU 使用。


### net_throttle_accept

定义每次网络循环迭代中接受多少个客户端。默认值为 0（无限制），对于大多数用户而言应该是可以的。这是一个微调选项，用于控制高负载场景中网络循环的吞吐量。


### net_throttle_action

定义每次网络循环迭代中处理多少个请求。默认值为 0（无限制），对于大多数用户而言应该是可以的。这是一个微调选项，用于控制高负载场景中网络循环的吞吐量。

### network_timeout

<!-- example conf network_timeout -->
网络客户端请求读/写超时，单位为秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认值为 5 秒。`searchd` 将强制关闭在超时内未能发送查询或读取结果的客户端连接。

还请注意 [reset_network_timeout_on_packet](../Server_settings/Searchd.md#reset_network_timeout_on_packet) 参数。此参数改变了 `network_timeout` 的行为，从应用于整个 `query` 或 `result` 改为应用于单个数据包。通常情况下，一个查询/结果适合在一到两个数据包内。然而，在需要大量数据的情况下，此参数对于维持活跃操作可能是非常宝贵的。

<!-- request Example -->

```ini
network_timeout = 10s
```
<!-- end -->

### node_address

<!-- example conf node_address -->
此设置允许您指定节点的网络地址。默认情况下，它设置为复制 [listen](../Server_settings/Searchd.md#listen) 地址。这在大多数情况下是正确的；但在某些情况下，您必须手动指定它：

* 节点位于防火墙后
* 启用网络地址转换 (NAT)
* 容器部署，如 Docker 或云部署
* 在多个区域内的集群节点


<!-- intro -->
##### 示例:

<!-- request Example -->

```ini
node_address = 10.101.0.10
```
<!-- end -->

### not_terms_only_allowed

<!-- example conf not_terms_only_allowed -->
此设置确定是否允许仅有 [否定](../Searching/Full_text_matching/Operators.md#Negation-operator) 全文操作符的查询。可选，默认值为 0（失败仅包含 NOT 操作符的查询）。


<!-- intro -->
##### 示例:

<!-- request Example -->

```ini
not_terms_only_allowed = 1
```
<!-- end -->

### optimize_cutoff

<!-- example conf optimize_cutoff -->
设置默认的表压缩阈值。有关更多信息，请阅读 - [优化的磁盘块数量](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks)。此设置可以通过每个查询选项 [cutoff](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 被重写。它还可以通过 [SET GLOBAL](../Server_settings/Setting_variables_online.md#SET) 动态更改。

<!-- intro -->
##### 示例:

<!-- request Example -->

```ini
optimize_cutoff = 4
```
<!-- end -->

### persistent_connections_limit

<!-- example conf persistent_connections_limit -->
此设置确定与远程 [持续代理](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 的最大同时持久连接数。每次连接到定义在 `agent_persistent` 下的代理时，我们会尝试重用现有连接（如果有的话），或者连接并保存该连接以备将来使用。然而，在某些情况下，限制此类持久连接的数量是合理的。此指令定义了限制。它影响到所有分布式表中每个代理主机的连接数量。

建议将该值设置为小于或等于代理配置中的 [max_connections](../Server_settings/Searchd.md#max_connections) 选项。

<!-- intro -->
##### 示例:

<!-- request Example -->

```ini
persistent_connections_limit = 29 # assume that each host of agents has max_connections = 30 (or 29).
```
<!-- end -->


### pid_file

<!-- example conf pid_file -->
pid_file 是 Manticore search 中一个强制配置选项，指定了存储 `searchd` 服务器进程 ID 的文件路径。

searchd 进程 ID 文件在启动时重新创建并锁定，包含了服务器运行时的主服务器进程 ID。它在服务器关闭时被取消链接。
此文件的目的是使 Manticore 执行各种内部任务，例如检查是否已经有一个运行中的 `searchd` 实例、停止 `searchd` 并通知它应该轮换表。该文件也可用于外部自动化脚本。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
pid_file = /var/run/manticore/searchd.pid
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
在基于其执行时间（使用最大查询时间设置）完成之前终止查询是一个很好的安全措施，但它也有一个固有的缺点：不确定性（不稳定）结果。也就是说，如果你多次在时间限制内重复同样的（复杂的）搜索查询，时间限制会在不同阶段被触及，你将得到*不同*的结果集。

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

有一个新选项，[SELECT … OPTION max_predicted_time](../Searching/Options.md#max_predicted_time)，可以让你限制查询时间*并*获得稳定、可重复的结果。它使用一个简单的线性模型预测当前运行时间，而不是在评估查询时定期检查实际当前时间（这是不确定的）：

```ini
predicted_time =
    doc_cost * processed_documents +
    hit_cost * processed_hits +
    skip_cost * skiplist_jumps +
    match_cost * found_matches
```

当 `predicted_time` 达到给定限制时，查询将提前终止。

当然，这并不是对实际花费时间的硬限制（不过，它是对*处理*工作量的硬限制），简单的线性模型也未必是一个理想的精确模型。因此，实际钟表时间*可能*会低于或超过目标限制。然而，误差范围是相当可以接受的：例如，在我们以 100 毫秒目标限制进行的实验中，大多数测试查询的耗时在 95 到 105 毫秒范围内，*所有*查询则在 80 到 120 毫秒范围内。此外，使用模型化的查询时间而不是测量实际运行时间也会导致 gettimeofday() 调用略有减少，这是一个不错的附加效果。

没有两个服务器的品牌和型号是完全相同的，因此 `predicted_time_costs` 指令允许你配置上述模型的成本。为了方便起见，它们是整数，以纳秒为单位计算。（max_predicted_time 中的限制以毫秒为单位计算，必须将成本值指定为 0.000128 毫秒而不是 128 纳秒会更容易出错。）不必一次性指定所有四个成本，遗漏的成本将取默认值。然而，我们强烈建议为了可读性指定所有成本。


### preopen_tables

<!-- example conf preopen_tables -->
preopen_tables 配置指令指定是否在启动时强制预打开所有表。默认值为 1，这意味着无论每个表的 [preopen](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Other-performance-related-settings) 设置如何，所有表都将被预打开。如果设置为 0，则每个表的设置可以生效，默认为 0。

预打开表可以防止搜索查询与可能导致查询偶尔失败的轮换之间的竞争条件。然而，它也会使用更多的文件句柄。在大多数场景中，建议预打开表。

这是一个示例配置：

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
preopen_tables = 1
```
<!-- end -->

### pseudo_sharding

<!-- example conf pseudo_sharding -->
pseudo_sharding 配置选项使得搜索查询可以并行化到本地平面和实时表，无论它们是直接查询还是通过分布式表。这项功能将自动将查询并行化到 `searchd.threads` 指定的线程数量。

请注意，如果你的工作线程已经忙碌，因为你有：
* 高查询并发
* 任何形式的物理分片：
  - 由多个平面/实时表组成的分布式表
  - 由太多磁盘分块组成的实时表

那么启用 pseudo_sharding 可能不会提供任何好处，甚至可能导致吞吐量略有下降。如果你更重视更高的吞吐量而不是较低的延迟，建议禁用此选项。

默认情况下启用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
pseudo_sharding = 0
```
<!-- end -->


### replication_connect_timeout

`replication_connect_timeout` 指令定义连接到远程节点的超时时间。默认情况下，值被假定为以毫秒为单位，但它可以具有 [其他后缀](../../Server_settings/Special_suffixes.md)。默认值为 1000（1 秒）。
当连接到远程节点时，Manticore 最多将等待此段时间以成功完成连接。如果超时已到但仍未建立连接，并且启用了 `retries`，则将开始重试。


### replication_query_timeout

`replication_query_timeout` 设置 searchd 等待远程节点完成查询的时间。默认值为 3000 毫秒（3 秒），但可以加上 `后缀` 以指示不同的时间单位。

建立连接后，Manticore 将等待最多 `replication_query_timeout` 以便远程节点完成。请注意，此超时与 `replication_connect_timeout` 是分开的，远程节点可能导致的总延迟将是两个值的总和。


### replication_retry_count

此设置是一个整数，指定 Manticore 在报告致命查询错误之前尝试连接和查询远程节点的次数。默认值为 3。


### replication_retry_delay

此设置是一个以毫秒为单位的整数（或 [special_suffixes](../Server_settings/Special_suffixes.md)），指定在复制期间出现故障时 Manticore 重试查询远程节点之前的延迟。只有在指定非零值时此值才相关。默认值为 500。

### qcache_max_bytes

<!-- example conf qcache_max_bytes -->
此配置设置为缓存结果集分配的最大 RAM 数量（以字节为单位）。默认值为 16777216，相当于 16 兆字节。如果将值设置为 0，则查询缓存将被禁用。有关查询缓存的更多信息，请参阅 [query cache](../Searching/Query_cache.md) 以获取详细信息。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
qcache_max_bytes = 16777216
```
<!-- end -->


### qcache_thresh_msec

整数，以毫秒为单位。查询结果被缓存的最小时间阈值。默认值为 3000，或 3 秒。0 表示缓存所有内容。有关详情，请参阅 [query cache](../Searching/Query_cache.md)。此值也可以用时间 [special_suffixes](../Server_settings/Special_suffixes.md) 表示，但请谨慎使用，不要混淆值名称中包含的 '_msec'。


### qcache_ttl_sec

整数，以秒为单位。缓存结果集的过期时间。默认值为 60，或 1 分钟。最小可能值为 1 秒。有关详情，请参阅 [query cache](../Searching/Query_cache.md)。此值也可以用时间 [special_suffixes](../Server_settings/Special_suffixes.md) 表示，但请谨慎使用，不要混淆值名称中包含的 '_sec'。


### query_log_format

<!-- example conf query_log_format -->
查询日志格式。可选，允许值为 `plain` 和 `sphinxql`，默认值为 `sphinxql`。

`sphinxql` 模式记录有效的 SQL 语句。`plain` 模式以纯文本格式记录查询（主要适用于完全文本使用情况）。该指令允许您在搜索服务器启动时在两种格式之间切换。日志格式也可以实时更改，使用 `SET GLOBAL query_log_format=sphinxql` 语法。有关更多详细信息，请参阅 [Query logging](../Logging/Query_logging.md)。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_format = sphinxql
```
<!-- end -->

### query_log_min_msec

限制（以毫秒为单位），防止查询被写入查询日志。可选，默认值为 0（所有查询都将写入查询日志）。该指令指定只有执行时间超过指定限制的查询将被记录（此值也可以用时间 [special_suffixes](../Server_settings/Special_suffixes.md) 表示，但请谨慎使用，不要混淆值名称中包含的 `_msec`）。

### query_log

<!-- example conf query_log -->
查询日志文件名。可选，默认值为空（不记录查询）。所有搜索查询（例如 SELECT ... 但不包括 INSERT/REPLACE/UPDATE 查询）将记录在此文件中。格式在 [Query logging](../Logging/Query_logging.md) 中描述。在 'plain' 格式的情况下，您可以使用 'syslog' 作为日志文件的路径。在这种情况下，所有搜索查询将发送到 syslog 守护进程，优先级为 `LOG_INFO`，以 '[query]' 开头而不是时间戳。要使用 syslog 选项，Manticore 必须在构建时配置为 `-–with-syslog`。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log = /var/log/query.log
```
<!-- end -->


### query_log_mode

<!-- example conf query_log_mode -->
query_log_mode 指令允许您为 searchd 和查询日志文件设置不同的权限。默认情况下，这些日志文件的权限为 600，这意味着只有运行服务器的用户和 root 用户可以读取日志文件。
如果您希望其他用户能够读取日志文件，例如，在非 root 用户下运行的监控解决方案，则此指令可能非常有用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_mode  = 666
```
<!-- end -->

### read_buffer_docs

<!-- example conf read_buffer_docs -->
read_buffer_docs 指令控制文档列表的每个关键词读取缓冲区大小。每个搜索查询中的每个关键词出现都有两个相关的读取缓冲区：一个用于文档列表，一个用于命中列表。此设置让您控制文档列表的缓冲区大小。

更大的缓冲区大小可能会增加每个查询的 RAM 使用量，但可能会减少 I/O 时间。对于慢速存储，设置较大的值是合理的，但对于能够提供高 IOPS 的存储，建议在低值区域进行实验。
默认值为256K，最小值为8K。您还可以在每个表的基础上设置[read_buffer_docs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs)，这将覆盖在服务器配置级别上设置的任何内容。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_docs = 128K
```
<!-- end -->


### read_buffer_hits

<!-- example conf read_buffer_hits -->
read_buffer_hits指令指定搜索查询中关键字的每个读取缓冲区大小。默认情况下，大小为256K，最小值为8K。在搜索查询中每个关键字出现时，会有两个相关的读取缓冲区，一个用于文档列表，另一个用于命中列表。增加缓冲区大小可以增加每个查询的RAM使用，但可以减少I/O时间。对于慢存储，更大的缓冲区大小是合理的，而对于能够高IOPS的存储，应在小值范围内进行实验。

该设置也可以通过使用[read_buffer_hits](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_hits)中的read_buffer_hits选项在每个表的基础上进行指定，这将覆盖服务器级别的设置。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_hits = 128K
```
<!-- end -->

### read_unhinted

<!-- example conf read_unhinted -->
无提示读取大小。可选，默认值为32K，最小值为1K

在查询时，一些读取已知道确切要读取多少数据，但有些当前不知道。最明显的，命中列表的大小目前未知。该设置允许您控制在这种情况下读取多少数据。它会影响命中列表的I/O时间，对于大于无提示读取大小的列表会减少时间，但对于较小的列表会增加时间。它**不**影响RAM使用，因为读取缓冲区将已经被分配。因此它不应大于read_buffer。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_unhinted = 32K
```
<!-- end -->

### reset_network_timeout_on_packet

<!-- example conf reset_network_timeout_on_packet -->
细化网络超时的行为（例如`network_timeout`、`read_timeout`和`agent_query_timeout`）。

设置为0时，超时限制发送整个请求/查询的最大时间。
设置为1（默认），超时限制网络活动之间的最长时间。

在复制时，一个节点可能需要将一个大文件（例如100GB）发送到另一个节点。假设网络可以以1GB/s的速度传输数据，每个数据包为4-5MB。传输整个文件需要100秒。默认的5秒超时只允许传输5GB，然后连接被中断。增加超时可能是解决方法，但不是可扩展的（例如，下一个文件可能是150GB，这将再次导致失败）。但是，默认情况下`reset_network_timeout_on_packet`设置为1，超时将应用于单个数据包，而不是整个传输。只要传输正在进行（并且在超时期间，网络上实际上收到了数据），它就会保持活动状态。如果传输卡住，以至于在数据包之间发生超时，它将被丢弃。

请注意，如果您设置了分布式表，则每个节点——主节点和代理——都应该进行调整。在主节点方面，`agent_query_timeout`受到影响；在代理方面，`network_timeout`是相关的。

<!-- intro -->

##### 示例：

<!-- request Example -->

```ini
reset_network_timeout_on_packet = 0
```

<!-- end -->


### rt_flush_period

<!-- example conf rt_flush_period -->
RT表的RAM块刷新检查周期，单位为秒（或[特殊后缀](../Server_settings/Special_suffixes.md)）。可选，默认值为10小时。

活跃更新的RT表完全适合于RAM块仍可能导致不断增长的binlog，从而影响磁盘使用和崩溃恢复时间。通过该指令，搜索服务器执行周期性刷新检查，符合条件的RAM块可以被保存，从而启用后续的binlog清理。有关更多详细信息，请参见[二进制日志记录](../Logging/Binary_logging.md)。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_flush_period = 3600 # 1小时
```
<!-- end -->


### rt_merge_iops

<!-- example conf rt_merge_iops -->
RT块合并线程允许启动的最大I/O操作数（每秒）。可选，默认值为0（无限制）。

该指令允许您降低`OPTIMIZE`语句带来的I/O影响。可保证所有RT优化活动所产生的磁盘I/O（每秒I/O）不会超过配置的限制。限制rt_merge_iops可以减少因合并导致的搜索性能下降。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_iops = 40
```
<!-- end -->

### rt_merge_maxiosize

<!-- example conf rt_merge_maxiosize -->
RT块合并线程允许启动的I/O操作的最大大小。可选，默认值为0（无限制）。

该指令允许您降低`OPTIMIZE`语句带来的I/O影响。大于此限制的I/O将被分解为两个或多个I/O，然后根据[rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops)限制作为单独的I/O进行计算。因此，保证所有优化活动不会产生超过（rt_merge_iops * rt_merge_maxiosize）字节的磁盘I/O每秒。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_maxiosize = 1M
```
<!-- end -->


### seamless_rotate

<!-- example conf seamless_rotate -->
防止 `searchd` 在轮换大量数据的表时卡住以进行预缓存。可选，默认值为 1（启用无缝轮换）。在 Windows 系统上，默认情况下禁用无缝轮换。

表中可能包含一些数据，需要在 RAM 中预缓存。目前，`.spa`、`.spb`、`.spi` 和 `.spm` 文件完全预缓存（分别包含属性数据、BLOB 属性数据、关键字表和被删除行映射）。如果没有无缝轮换，轮换表尽量使用尽可能少的 RAM，其工作流程如下：

1. 新查询被暂时拒绝（带有 "retry" 错误代码）；
2. `searchd` 等待所有正在运行的查询完成；
3. 旧表被释放，其文件被重命名；
4. 新表文件被重命名，并分配所需的 RAM；
5. 新表的属性和字典数据被预加载到 RAM 中；
6. `searchd` 从新表恢复服务查询。

但是，如果有大量的属性或字典数据，则预加载步骤可能需要相当长的时间 - 在预加载 1-5+ GB 文件的情况下，可能需要几分钟。

启用无缝轮换时，轮换的工作流程如下：

1. 分配新的表 RAM 存储；
2. 新表的属性和字典数据异步预加载到 RAM 中；
3. 如果成功，旧表被释放，并且两个表的文件被重命名；
4. 如果失败，新表被释放；
5. 在任何给定的时刻，查询要么来自旧表， 要么来自新表副本。

无缝轮换在轮换期间会导致更高的峰值内存使用（因为 `.spa/.spb/.spi/.spm` 数据的旧副本和新副本需要在预加载新副本时在 RAM 中）。平均使用量保持不变。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
seamless_rotate = 1
```
<!-- end -->

### secondary_indexes
<!-- example conf secondary_indexes -->

此选项启用/禁用在搜索查询中使用辅助索引。它是可选的，默认值为 1（启用）。请注意，您不需要为索引启用它，因为只要安装了 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)，它总是会启用。后者在搜索时使用索引时也是必需的。可用的模式有三种：

* `0`: 禁用在搜索中使用辅助索引。可以通过使用 [analyzer hints](../Searching/Options.md#Query-optimizer-hints) 为单个查询启用。
* `1`: 启用在搜索中使用辅助索引。可以通过使用 [analyzer hints](../Searching/Options.md#Query-optimizer-hints) 为单个查询禁用。
* `force`: 与启用相同，但在加载辅助索引期间遇到的任何错误将被报告，整个索引将不会加载到守护进程中。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
secondary_indexes = 1
```

<!-- end -->

### server_id

<!-- example conf server_id -->
整数，用作服务器标识符，作为生成唯一短 UUID 的种子，用于属于复制集群的节点。server_id 必须在集群的节点之间唯一，并且在 0 到 127 的范围内。如果未设置 server_id，则使用 MAC 地址和 PID 文件的路径的哈希值计算，或者使用随机数字作为短 UUID 的种子。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
server_id = 1
```
<!-- end -->


### shutdown_timeout

<!-- example conf shutdown_timeout -->
`searchd --stopwait` 等待时间，以秒为单位（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认值为 60 秒。

当您运行 `searchd --stopwait` 时，您的服务器需要在停止之前执行一些活动，例如完成查询、刷新 RT RAM 块、刷新属性以及更新 binlog。这些任务需要一些时间。`searchd --stopwait` 将等待最多 `shutdown_time` 秒，直到服务器完成其工作。适当的时间取决于您的表大小和负载。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
shutdown_timeout = 3m # 等待最多 3 分钟
```
<!-- end -->


### shutdown_token

SHA1 哈希密码，用于从 VIP Manticore SQL 连接调用 'shutdown' 命令。如果没有它，[debug](../Reporting_bugs.md#DEBUG) 'shutdown' 子命令将永远不会导致服务器停止。请注意，这种简单的哈希不应被视为强保护，因为我们没有使用盐哈希或任何现代哈希函数。它旨在作为本地网络中家务守护进程的防护措施。

### snippets_file_prefix

<!-- example conf snippets_file_prefix -->
在生成片段时要添加到本地文件名的前缀。可选，默认值为当前工作文件夹。

此前缀可用于与 `load_files` 或 `load_files_scattered` 选项一起分布式片段生成。

请注意，这是一个前缀，而 **不是** 路径！这意味着如果前缀设置为 "server1"，而请求涉及 "file23"，`searchd` 将尝试打开 "server1file23"（所有这些都不带引号）。因此，如果您需要它作为路径，则必须包含后斜杠。

在构建最终文件路径后，服务器展开所有相对目录并将最终结果与 `snippet_file_prefix` 的值进行比较。如果结果不以前缀开头，则这样的文件将被拒绝，并带有错误消息。

例如，如果您将其设置为 `/mnt/data`，并且某人以文件 `../../../etc/passwd` 作为来源调用片段生成，他们将收到错误消息：

`File '/mnt/data/../../../etc/passwd' escapes '/mnt/data/' scope`

而不是文件的内容。
另外，使用未设置的参数并读取 `/etc/passwd` 时，它实际上会读取 /daemon/working/folder/etc/passwd，因为该参数的默认值是服务器的工作文件夹。

还请注意，这是一个本地选项；它不会以任何方式影响代理。因此，您可以安全地在主服务器上设置前缀。路由到代理的请求将不会受到主服务器设置的影响。不过，它们将受到代理本身设置的影响。

这在某些情况下可能是有用的，例如，当文档存储位置（无论是本地存储还是 NAS 挂载点）在服务器之间不一致时。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
snippets_file_prefix = /mnt/common/server1/
```
<!-- end -->

> **警告：** 如果您仍希望从 FS 根目录访问文件，则必须将 `snippets_file_prefix` 显式设置为空值（通过 `snippets_file_prefix=` 行），或设置为根目录（通过 `snippets_file_prefix=/`）。


### sphinxql_state

<!-- example conf sphinxql_state -->
当前 SQL 状态将序列化到的文件路径。

在服务器启动时，此文件会被重放。在符合条件的状态更改（例如，SET GLOBAL）时，此文件会自动重写。这可以防止一个难以诊断的问题：如果您加载了 UDF 函数，但 Manticore 崩溃，当它被（自动）重新启动时，您的 UDF 和全局变量将不再可用。使用持久状态有助于确保平稳恢复，没有这样的惊喜。

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
使用 SQL 接口时请求之间的最大等待时间（以秒为单位，或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认值为 15 分钟。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
sphinxql_timeout = 15m
```
<!-- end -->


### ssl_ca

<!-- example conf ssl_ca -->
SSL 证书颁发机构（CA）证书文件的路径（也称为根证书）。可选，默认值为空。当不为空时，`ssl_cert` 中的证书应由该根证书签名。

服务器使用 CA 文件验证证书上的签名。该文件必须为 PEM 格式。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ssl_ca = keys/ca-cert.pem
```
<!-- end -->


### ssl_cert

<!-- example conf ssl_cert -->
服务器的 SSL 证书路径。可选，默认值为空。

服务器使用此证书作为自签名公钥来加密通过 SSL 的 HTTP 流量。该文件必须为 PEM 格式。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ssl_cert = keys/server-cert.pem
```
<!-- end -->


### ssl_key

<!-- example conf ssl_key -->
SSL 证书密钥的路径。可选，默认值为空。

服务器使用此私钥来加密通过 SSL 的 HTTP 流量。该文件必须为 PEM 格式。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ssl_key = keys/server-key.pem
```
<!-- end -->


### subtree_docs_cache

<!-- example conf subtree_docs_cache -->
每个查询的最大公共子树文档缓存大小。可选，默认值为 0（禁用）。

此设置限制公共子树优化器的 RAM 使用（请参阅 [multi-queries](../Searching/Multi-queries.md)）。每个查询至多消耗这么多 RAM 来缓存文档条目。将限制设置为 0 会禁用优化器。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
subtree_docs_cache = 8M
```
<!-- end -->


### subtree_hits_cache

<!-- example conf subtree_hits_cache -->
每个查询的最大公共子树命中缓存大小。可选，默认值为 0（禁用）。

此设置限制公共子树优化器的 RAM 使用（请参阅 [multi-queries](../Searching/Multi-queries.md)）。每个查询至多消耗这么多 RAM 来缓存关键字出现（命中）。将限制设置为 0 会禁用优化器。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
subtree_hits_cache = 16M
```
<!-- end -->

### threads

<!-- example threads -->
Manticore 守护进程的工作线程数（或线程池大小）。Manticore 在启动时创建这个数量的 OS 线程，它们在守护进程内部执行所有任务，例如执行查询、创建片段等。有些操作可以拆分为子任务并并行执行，例如：

* 在实时表中搜索
* 在由本地表组成的分布式表中搜索
* 传播查询调用
* 以及其他

默认情况下，它的值设置为服务器上的 CPU 核心数。Manticore 在启动时创建这些线程，并保持它们直到停止。每个子任务可以在需要时使用一个线程。当子任务完成时，它会释放该线程，以便另一个子任务可以使用它。

对于 I/O 类型负载密集的情况，设置的值可能需要高于 CPU 核心数更为合理。

<!-- request Example -->
```ini
threads = 10
```

<!-- end -->

### thread_stack

<!-- example conf thread_stack -->
作业（协程，一个搜索查询可能导致多个作业/协程）的最大堆栈大小。可选，默认值为 128K。

每个作业都有自己的 128K 堆栈。当您运行查询时，会检查所需的堆栈大小。如果默认的 128K 足够，则直接处理。如果需要更多，则会调度另一个具有增大堆栈的作业，继续处理。此类扩展堆栈的最大大小由此设置限制。

将值设置为合理较高的速率将有助于处理非常深的查询，而不意味着整体内存消耗会增长过高。例如，将其设置为 1G 并不意味着每个新作业都会消耗 1G 内存，但如果我们看到它需要，比如说，100M 堆栈，我们只为该作业分配 100M。其他作业在同一时间将使用他们的默认 128K 堆栈。以同样的方式，我们甚至可以运行需要 500M 的更复杂查询。只有当我们 **内部看到** 作业需要超过 1G 堆栈时，我们才会失败并报告线程堆栈过低。

然而，实际上，即使是需要 16M 堆栈的查询通常也是过于复杂的解析，消耗过多的时间和资源进行处理。因此，守护进程将处理它，但通过 `thread_stack` 设置限制此类查询看起来相当合理。

<!-- intro -->

##### 示例:

<!-- request 示例 -->

```ini

thread_stack = 8M

```

<!-- end -->

### unlink_old

<!-- example conf unlink_old -->

