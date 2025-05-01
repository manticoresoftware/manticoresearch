# Section "Searchd" in configuration

以下设置用于 Manticore Search 配置文件的 `searchd` 部分，以控制服务器的行为。以下是每个设置的摘要：

### access_plain_attrs

此设置为 [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。它是可选的，默认值为 `mmap_preread`。

`access_plain_attrs` 指令允许您为所有由此 searchd 实例管理的表定义 [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令具有更高的优先级，并将覆盖此实例范围的默认值，从而提供更细粒度的控制。

### access_blob_attrs

此设置为 [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。它是可选的，默认值为 `mmap_preread`。

`access_blob_attrs` 指令允许您为所有由此 searchd 实例管理的表定义 [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令具有更高的优先级，并将覆盖此实例范围的默认值，从而提供更细粒度的控制。

### access_doclists

此设置为 [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。它是可选的，默认值为 `file`。

`access_doclists` 指令允许您为所有由此 searchd 实例管理的表定义 [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令具有更高的优先级，并将覆盖此实例范围的默认值，从而提供更细粒度的控制。

### access_hitlists

此设置为 [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。它是可选的，默认值为 `file`。

`access_hitlists` 指令允许您为所有由此 searchd 实例管理的表定义 [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令具有更高的优先级，并将覆盖此实例范围的默认值，从而提供更细粒度的控制。

### access_dict

此设置为 [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。它是可选的，默认值为 `mmap_preread`。

`access_dict` 指令允许您为所有由此 searchd 实例管理的表定义 [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令具有更高的优先级，并将覆盖此实例范围的默认值，从而提供更细粒度的控制。

### agent_connect_timeout

此设置为 [agent_connect_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) 参数设置实例范围的默认值。


### agent_query_timeout

此设置为 [agent_query_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) 参数设置实例范围的默认值。可以使用 `OPTION agent_query_timeout=XXX` 子句在每个查询的基础上覆盖。


### agent_retry_count

此设置是一个整数，指定 Manticore 在报告致命查询错误之前通过分布式表连接和查询远程代理的尝试次数。默认值为 0（即不重试）。您也可以使用 `OPTION retry_count=XXX` 子句在每个查询的基础上设置此值。如果提供了每个查询的选项，它将覆盖配置中指定的值。

请注意，如果您在分布式表的定义中使用 [agent mirrors](../Creating_a_cluster/Remote_nodes/Mirroring.md#Agent-mirrors)，服务器将在每次连接尝试时根据所选的 [ha_strategy](../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) 选择不同的镜像。在这种情况下，`agent_retry_count` 将为一组中的所有镜像聚合。

例如，如果您有 10 个镜像并设置 `agent_retry_count=5`，则服务器将最多重试 50 次，假设每个 10 个镜像平均尝试 5 次（在使用 `ha_strategy = roundrobin` 选项时，将是这种情况）。

但是，作为 [agent](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) 的 `retry_count` 选项提供的值作为绝对限制。换句话说，代理定义中的 `[retry_count=2]` 选项始终意味着最多 2 次尝试，无论您为代理指定了 1 个还是 10 个镜像。

### agent_retry_delay

此设置是以毫秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）为单位的整数，指定 Manticore 在查询远程代理失败后重试的延迟。此值仅在指定了非零的 [agent_retry_count](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 或非零的每查询 `retry_count` 时相关。默认值为 500。您也可以使用 `OPTION retry_delay=XXX` 子句在每个查询的基础上设置此值。如果提供了每个查询的选项，它将覆盖配置中指定的值。


### attr_flush_period

<!-- example conf attr_flush_period -->
在使用 [Update](../Data_creation_and_modification/Updating_documents/UPDATE.md) 实时修改文档属性时，变化首先写入属性的内存副本。 这些更新发生在内存映射文件中，这意味着操作系统决定何时将更改写入磁盘。 在正常关闭 `searchd`（由 `SIGTERM` 信号触发）时，所有更改都被强制写入磁盘。 

您还可以指示 `searchd` 定期将这些更改写回磁盘，以防止数据丢失。 这些刷新之间的间隔由 `attr_flush_period` 决定，以秒为单位（或 [special_suffixes](../Server_settings/Special_suffixes.md) 指定）。

默认情况下，该值为 0，这会禁用定期刷新。 但是，在正常关闭期间仍会发生刷新。 

<!-- intro -->
##### 示例：

<!-- request 示例 -->

```ini
attr_flush_period = 900 # persist updates to disk every 15 minutes
```
<!-- end -->

### auto_optimize

<!-- example conf auto_optimize -->
此设置控制表压缩的自动 [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 过程。

默认情况下，表压缩会自动发生。 您可以通过 `auto_optimize` 设置修改此行为：
* 0 禁用自动表压缩（您仍然可以手动调用 `OPTIMIZE`）
* 1 显式启用
* 在将优化阈值乘以 2 的同时启用。

默认情况下，OPTIMIZE 运行直到磁盘块的数量小于或等于逻辑 CPU 核心的数量乘以 2。

但是，如果表具有 KNN 索引的属性，则此阈值不同。 在这种情况下，它被设置为物理 CPU 核心的数量除以 2，以提高 KNN 搜索性能。

请注意，切换 `auto_optimize` 的开或关并不妨碍您手动运行 [OPTIMIZE TABLE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE)。

<!-- intro -->
##### 示例：

<!-- request 禁用 -->
```ini
auto_optimize = 0 # disable automatic OPTIMIZE
```

<!-- request 限制 -->
```ini
auto_optimize = 2 # OPTIMIZE starts at 16 chunks (on 4 cpu cores server)
```

<!-- end -->

### auto_schema

<!-- example conf auto_schema -->
Manticore 支持自动创建尚不存在但在 INSERT 语句中指定的表。 此功能默认启用。 要禁用它，请在配置中显式设置 `auto_schema = 0`。 要重新启用它，请设置 `auto_schema = 1` 或从配置中删除 `auto_schema` 设置。

请记住，`/bulk` HTTP 端点不支持自动表创建。

> 注意： [自动模式功能](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。 如果它无法工作，请确保 Buddy 已安装。

<!-- request 禁用 -->
```ini
auto_schema = 0 # disable automatic table creation
```

<!-- request 启用 -->
```ini
auto_schema = 1 # enable automatic table creation
```

<!-- end -->

### binlog_flush

<!-- example conf binlog_flush -->
此设置控制二进制日志事务刷新/同步模式。 可选，默认值为 2（每个事务刷新，每秒同步）。

该指令确定二进制日志将多频繁地刷新到操作系统并同步到磁盘。 有三种支持的模式：

*  0，每秒刷新和同步一次。 这提供了最佳性能，但在服务器崩溃或操作系统/硬件崩溃的情况下，可以丢失最多 1 秒已提交事务的数据。
*  1，每个事务刷新和同步。 这种模式提供最差的性能，但确保保存每个已提交事务的数据。
*  2，每个事务刷新，每秒同步一次。 这种模式提供良好的性能，并确保在服务器崩溃的情况下保存每个已提交的事务。 但是，在操作系统/硬件崩溃的情况下，可以丢失最多 1 秒已提交事务的数据。

对于熟悉 MySQL 和 InnoDB 的人来说，该指令类似于 `innodb_flush_log_at_trx_commit`。 在大多数情况下，默认的混合模式 2 提供了速度和安全性之间的良好平衡，能够完全保护 RT 表数据免受服务器崩溃的影响，并对硬件崩溃提供一定的保护。

<!-- intro -->
##### 示例：

<!-- request 示例 -->

```ini
binlog_flush = 1 # ultimate safety, low speed
```
<!-- end -->

### binlog_common

<!-- example conf binlog_common -->
此设置控制二进制日志文件的管理。 可选，默认值为 0（每个表一个单独的文件）。

您可以选择两种方式来管理二进制日志文件：

* 每个表一个单独的文件（默认，`0`）：每个表在其自己的日志文件中保存其更改。 如果有许多表在不同时间更新，此设置很好。 它允许在不等待其他表的情况下更新表。 而且，如果一个表的日志文件出现问题，不会影响其他表。
* 所有表使用一个单一文件（`1`）：所有表使用相同的二进制日志文件。 这种方法使处理文件更容易，因为文件数量更少。 但是，如果一个表仍然需要保存其更新，这可能会导致文件保留比所需的时间更长。 如果许多表需要同时更新，这一设置可能还会导致速度变慢，因为所有更改必须等待写入一个文件。

<!-- intro -->
##### 示例：

<!-- request 示例 -->

```ini
binlog_common = 1 # use a single binary log file for all tables
```
<!-- end -->

### binlog_max_log_size

<!-- example conf binlog_max_log_size -->
此设置控制最大二进制日志文件大小。 可选，默认值为 256 MB。

一旦当前的 binlog 文件达到此大小限制，将强制打开一个新的 binlog 文件。 这会导致日志的粒度更细，并在某些边缘负载下可以提高 binlog 磁盘使用效率。 值为 0 表示不应根据大小重新打开 binlog 文件。


<!-- intro -->
##### 示例：

<!-- request 示例 -->

```ini
binlog_max_log_size = 16M
```
<!-- end -->


### binlog_path

<!-- example conf binlog_path -->
此设置确定二进制日志（也称为事务日志）文件的路径。 可选，默认值为构建时配置的数据目录（例如，在 Linux 中，`/var/lib/manticore/data/binlog.*`）。

二进制日志用于恢复RT表数据的崩溃和用于平面磁盘索引的属性更新，这些索引将只存储在RAM中，直到刷新。当启用日志记录时，每个提交到RT表的事务都会被写入日志文件。然后，在不干净的关闭后，日志会在启动时自动重放，以恢复记录的更改。

`binlog_path`指令指定二进制日志文件的位置。它应仅包含路径；`searchd`将根据需要在目录中创建和取消链接多个`binlog.*`文件（包括binlog数据、元数据和锁文件等）。

空值会禁用二进制日志记录，这样可以提高性能，但会使RT表数据面临风险。


<!-- intro -->
##### 示例：

<!-- request 示例 -->

```ini
binlog_path = # disable logging
binlog_path = /var/lib/manticore/data # /var/lib/manticore/data/binlog.001 etc will be created
```
<!-- end -->


### buddy_path

<!-- example conf buddy_path -->
此设置决定了Manticore Buddy二进制文件的路径。它是可选的，默认值是构建时配置的路径，这在不同操作系统之间有所不同。通常，您无需修改此设置。然而，如果您希望以调试模式运行Manticore Buddy，进行更改或实现新插件，这可能会很有用。在后者的情况下，您可以从https://github.com/manticoresoftware/manticoresearch-buddy `git clone` Buddy，向目录`./plugins/`添加新插件，并在将目录更改为Buddy源代码后运行`composer install --prefer-source`以便于开发。

为确保您可以运行`composer`，您的机器上必须安装PHP 8.2或更高版本，并具有以下扩展：

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

您还可以选择针对Linux amd64可用的特殊`manticore-executor-dev`版本，例如： https://github.com/manticoresoftware/executor/releases/tag/v1.0.13

如果您选择此路径，请记得将manticore执行器的开发版本链接到`/usr/bin/php`。

要禁用Manticore Buddy，请将值设为空，如示例所示。

<!-- intro -->
##### 示例：

<!-- request 示例 -->

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
此设置确定在使用持久连接时请求之间等待的最长时间（以秒或[特殊后缀](../Server_settings/Special_suffixes.md)为单位）。它是可选的，默认值为五分钟。


<!-- intro -->
##### 示例：

<!-- request 示例 -->

```ini
client_timeout = 1h
```
<!-- end -->


### collation_libc_locale

<!-- example conf collation_libc_locale -->
服务器libc区域设置。可选，默认值为C。

指定libc区域设置，影响基于libc的排序。有关详细信息，请参见[排序](../Searching/Collations.md)部分。


<!-- intro -->
##### 示例：

<!-- request 示例 -->

```ini
collation_libc_locale = fr_FR
```
<!-- end -->


### collation_server

<!-- example conf collation_server -->
默认服务器排序。可选，默认值为libc_ci。

指定用于传入请求的默认排序。在每个查询的基础上可以覆盖排序。有关可用排序列表和其他详细信息，请参见[排序](../Searching/Collations.md)部分。


<!-- intro -->
##### 示例：

<!-- request 示例 -->

```ini
collation_server = utf8_ci
```
<!-- end -->


### data_dir

<!-- example conf data_dir -->
指定时，此设置启用[实时模式](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)，这是一种管理数据架构的必要方式。该值应是您要存储所有表、二进制日志以及此模式下为Manticore Search的正常运行所需的所有其他内容的目录路径。
在指定`data_dir`时，不允许对[平面表](../Creating_a_table/Local_tables/Plain_table.md)进行索引。有关RT模式和平面模式之间差异的更多信息，请参见[本节](../Read_this_first.md#Real-time-table-vs-plain-table)。

<!-- intro -->
##### 示例：

<!-- request 示例 -->

```ini
data_dir = /var/lib/manticore
```
<!-- end -->

### diskchunk_flush_search_timeout

<!-- example conf diskchunk_flush_search_timeout -->
防止在表中没有搜索时自动冲洗RAM块的超时选项。可选，默认值为30秒。

在确定是否自动冲洗之前检查搜索的时间。
仅当在过去的`diskchunk_flush_search_timeout`秒内表中至少有一次搜索时，才会发生自动冲洗。与[diskchunk_flush_write_timeout](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout)配合使用。相应的[每表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_search_timeout)具有更高的优先级，并将覆盖此实例范围的默认值，从而提供更细粒度的控制。

<!-- intro -->
##### 示例：

<!-- request 示例 -->

```ini
diskchunk_flush_search_timeout = 120s
```
<!-- end -->

### diskchunk_flush_write_timeout

<!-- example conf diskchunk_flush_write_timeout -->
等待写入的时间（以秒为单位），在没有写入之前自动冲洗RAM块到磁盘。可选，默认值为1秒。

如果在`diskchunk_flush_write_timeout`秒内没有在RAM块中发生写入，则该块将被冲洗到磁盘。与[diskchunk_flush_search_timeout](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout)配合使用。要禁用自动冲洗，请在配置中明确设置`diskchunk_flush_write_timeout = -1`。相应的[每表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_write_timeout)具有更高的优先级，将覆盖此实例范围的默认值，从而提供更细粒度的控制。

<!-- intro -->
##### 示例：

<!-- request 示例 -->

```ini
diskchunk_flush_write_timeout = 60s
```
<!-- end -->

### docstore_cache_size

<!-- example conf docstore_cache_size -->
此设置指定文档存储中保留在内存中的文档块的最大大小。它是可选的，默认值为16m（16兆字节）。

当使用 `stored_fields` 时，文档块从磁盘读取并解压缩。由于每个块通常包含多个文档，因此在处理下一个文档时可能会重用该块。为此，块保存在服务器范围的缓存中。缓存保存未压缩的块。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
docstore_cache_size = 8m
```
<!-- end -->

### 引擎

<!-- example conf engine -->
在 RT 模式下创建表时使用的默认属性存储引擎。可以是 `rowwise`（默认）或 `columnar`。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
engine = columnar
```
<!-- end -->


### 扩展限制

<!-- example conf expansion_limit -->
此设置决定单个通配符的最大扩展关键字数量。它是可选的，默认值为 0（无限制）。

在对启用了 `dict = keywords` 的表执行子字符串搜索时，单个通配符可能会导致成千上万甚至数百万个匹配的关键字（想象一下将 `a*` 与整个牛津字典进行匹配）。此指令允许您限制此类扩展的影响。设置 `expansion_limit = N` 将扩展限制为每个查询中的通配符最多 N 个最常见的匹配关键字。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_limit = 16
```
<!-- end -->

### 扩展合并阈值文档

<!-- example conf expansion_merge_threshold_docs -->
此设置决定允许将所有扩展关键字合并在一起的最大文档数。它是可选的，默认为 32。

在对启用了 `dict = keywords` 的表执行子字符串搜索时，单个通配符可能会导致成千上万甚至数百万个匹配的关键字。此指令允许您增加合并在一起的关键字的数量限制，以加快匹配速度，但在搜索中会使用更多内存。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_merge_threshold_docs = 1024
```
<!-- end -->

### 扩展合并阈值击中

<!-- example conf expansion_merge_threshold_hits -->
此设置决定允许将所有扩展关键字合并在一起的最大击中数。它是可选的，默认为 256。

在对启用了 `dict = keywords` 的表执行子字符串搜索时，单个通配符可能会导致成千上万甚至数百万个匹配的关键字。此指令允许您增加合并在一起的关键字的数量限制，以加快匹配速度，但在搜索中会使用更多内存。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_merge_threshold_hits = 512
```
<!-- end -->

### utc分组

此设置指定在 API 和 SQL 中的时间分组是根据本地时区还是 UTC 进行计算。它是可选的，默认值为 0（表示“本地时区”）。

默认情况下，所有“按时间分组”（例如在 API 中按天、周、月和年分组，以及在 SQL 中按天、月、年、年月、年月日分组）都是使用当地时间进行的。例如，如果您有属性以 `13:00 utc` 和 `15:00 utc` 时间标记的文档，则在分组的情况下，它们将根据您的本地时区设置划入设施组。如果您生活在 `utc`，它将是一天，但如果您生活在 `utc+10`，那么这些文档将匹配到不同的“按天分组”设施组（因为在 UTC+10 时区，13:00 utc 是当地时间的 23:00，但 15:00 是第二天的 01:00）。有时这种行为是不可接受的，希望时间分组不依赖于时区。您可以使用定义的全局 TZ 环境变量运行服务器，但这不仅会影响分组，还会影响日志中的时间戳，这也可能是不可取的。启用此选项（无论是在配置中还是使用 [SET global](../Server_settings/Setting_variables_online.md#SET) SQL 语句）将导致所有时间分组表达式在 UTC 中计算，而其他时间相关函数（即服务器日志记录）仍在本地时区中。


### 时区

此设置指定日期/时间相关函数使用的时区。默认情况下，使用本地时区，但您可以以 IANA 格式指定其他时区（例如，`Europe/Amsterdam`）。

请注意，此设置对日志记录没有影响，日志记录始终在本地时区中进行。

此外，请注意，如果使用了 `grouping_in_utc`，则“按时间分组”功能仍将使用 UTC，而其他日期/时间相关函数将使用指定的时区。总体而言，不推荐将 `grouping_in_utc` 和 `timezone` 混合使用。

您可以在配置中配置此选项或通过使用 [SET global](../Server_settings/Setting_variables_online.md#SET) SQL 语句。


### ha_period_karma

<!-- example conf ha_period_karma -->
此设置指定代理镜像统计窗口的大小，以秒为单位（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。它是可选的，默认值为 60 秒。

对于包含代理镜像的分布式表（有关更多信息，请参见 [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)），主服务器跟踪多个不同的每个镜像计数器。这些计数器随后用于故障转移和平衡（主服务器根据计数器选择最佳镜像）。计数器以 `ha_period_karma` 秒的块进行累积。

在开始一个新块之后，主服务器仍然可以使用从前一个块累积的值，直到新块的填充度达到一半。因此，任何之前的历史在最多 1.5 倍的 ha_period_karma 秒后将停止影响镜像选择。

尽管最多使用两个块来进行镜像选择，但为了仪表记录，会存储最多 15 个最后的块。这些块可以通过 [SHOW AGENT STATUS](../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS) 语句进行检查。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_period_karma = 2m
```
<!-- end -->


### ha_ping_interval

<!-- example conf ha_ping_interval -->
此设置配置代理镜像之间的间隔，单位为毫秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。这是可选的，默认值为1000毫秒。

对于包含代理镜像的分布式表（更多信息见 [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)），主服务器在空闲期间向所有镜像发送ping命令。这是为了跟踪当前代理状态（存活或死亡、网络往返时间等）。这种ping之间的间隔由此指令定义。要禁用ping，请将ha_ping_interval设置为0。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_ping_interval = 3s
```
<!-- end -->


### hostname_lookup

`hostname_lookup`选项定义了更新主机名的策略。默认情况下，代理主机名的IP地址在服务器启动时被缓存，以避免对DNS的过度访问。然而，在某些情况下，IP可能会动态变化（例如，云托管），可能希望不缓存IP。将此选项设置为`request`会禁用缓存，并在每个查询中查询DNS。IP地址也可以通过`FLUSH HOSTNAMES`命令手动更新。

### jobs_queue_size

jobs_queue_size设置定义队列中可以同时存在的“作业”数量。默认情况下是无限制的。

在大多数情况下，“作业”意味着对单个本地表（标准表或实时表的磁盘块）进行的一次查询。例如，如果您有一个由2个本地表组成的分布式表，或一个具有2个磁盘块的实时表，对它们的搜索查询通常会将2个作业放入队列中。然后，线程池（其大小由 [threads](../Server_settings/Searchd.md#threads) 定义）将处理它们。然而，在某些情况下，如果查询过于复杂，可能会创建更多作业。当 [max_connections](../Server_settings/Searchd.md#max_connections) 和 [threads](../Server_settings/Searchd.md#threads) 不足以找到所需性能与负载之间的平衡时，建议更改此设置。

### join_batch_size

表连接通过累积一批匹配项工作，这些匹配项是对左侧表执行的查询结果。然后将此批处理作为对右侧表的单个查询。

此选项允许您调整批处理大小。默认值为`1000`，将此选项设置为`0`可禁用批处理。

更大的批处理大小可能会提高性能；然而，对于某些查询，它可能导致过度的内存消耗。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
join_batch_size = 2000
```
<!-- end -->

### join_cache_size

每个在右侧表上执行的查询由特定的JOIN ON条件定义，这些条件决定从右侧表中检索的结果集。

如果只有少数唯一的JOIN ON条件，重用结果可能比在右侧表上反复执行查询更有效。为了启用此功能，结果集存储在缓存中。

此选项允许您配置此缓存的大小。默认值为`20 MB`，将此选项设置为0会禁用缓存。

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
listen_backlog设置决定了TCP监听 backlog的长度，用于处理入站连接。这对于逐一处理请求的Windows构建尤为重要。当连接队列达到其限制时，新来的连接将被拒绝。
对于非Windows构建，默认值通常可以正常工作，通常无需调整此设置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
listen_backlog = 20
```
<!-- end -->

### kibana_version_string

<!-- example conf kibana_version_string -->
要返回给Kibana或OpenSearch Dashboards的服务器版本字符串。可选 - 默认值为`7.6.0`。

一些版本的Kibana和OpenSearch Dashboards期望服务器报告特定的版本号，并可能根据它的不同而表现不同。为了解决此类问题，您可以使用此设置，使Manticore向Kibana或OpenSearch Dashboards报告自定义版本。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
kibana_version_string = 1.2.3
```
<!-- end -->

### listen

<!-- example conf listen -->
此设置让您指定一个IP地址和端口，或Unix域套接字路径，Manticore将接受该连接。

`listen`的一般语法为：

```ini
listen = ( address ":" port | port | path | address ":" port start - port end ) [ ":" protocol [ "_vip" ] [ "_readonly" ] ]
```

您可以指定：
* 要么是IP地址（或主机名）和端口号
* 要么仅是端口号
* 要么是Unix套接字路径（Windows不支持）
* 要么是IP地址和端口范围

如果您指定了端口号但没有地址，`searchd`将在所有网络接口上监听。Unix路径以斜杠开头。端口范围只能为复制协议设置。

您还可以指定用于该套接字连接的协议处理程序（侦听器）。侦听器有：

* **未指定** - Manticore将接受来自以下来源的连接：
  - 其他Manticore代理（即，远程分布式表）
  - 通过HTTP和HTTPS的客户端
  - [Manticore Buddy](https://manticoresearch.com/blog/manticoresearch-buddy-intro/)。**确保您拥有这种类型的侦听器（或如下所述的`http`侦听器），以避免Manticore功能的限制。**
* `mysql` MySQL协议，用于来自MySQL客户端的连接。注意：
  - 也支持压缩协议。
  - 如果启用了[SSL](../Security/SSL.md#SSL)，您可以建立加密连接。
* `replication` - 节点通信使用的复制协议。更多细节可以在 [replication](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md) 部分找到。您可以指定多个复制监听器，但它们必须都在同一个 IP 上监听；只有端口可以不同。当您使用端口范围定义一个复制监听器时（例如，`listen = 192.168.0.1:9320-9328:replication`），Manticore 不会立即在这些端口上开始监听。相反，它只会在您开始使用复制时从指定范围中获得随机空闲端口。范围内至少需要 2 个端口才能正确工作。
* `http` - 与 **未指定** 相同。Manticore 将通过 HTTP 和 HTTPS 在此端口接受来自远程代理和客户的连接。
* `https` - HTTPS 协议。Manticore 将仅在此端口接受 **仅** HTTPS 连接。更多细节可以在 [SSL](../Security/SSL.md) 部分找到。
* `sphinx` - 传统二进制协议。用于为来自远程 [SphinxSE](../Extensions/SphinxSE.md) 客户端的连接提供服务。一些 Sphinx API 客户端实现（一个例子是 Java 实现）需要显式声明监听器。

将后缀 `_vip` 添加到客户端协议（即，除了 `replication` 外的所有协议，例如 `mysql_vip` 或 `http_vip` 或仅 `_vip`）会强制为连接创建一个专用线程，以绕过不同的限制。这在节点维护时非常有用，尤其是在严重过载的情况下，当服务器要么停止响应，要么不让您通过常规端口连接。

后缀 `_readonly` 为监听器设置 [只读模式](../Security/Read_only.md)，并限制其仅接受读取查询。

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

可以有多个 `listen` 指令。`searchd` 将在所有指定的端口和套接字上监听客户端连接。Manticore 软件包中提供的默认配置定义了在以下端口上进行监听：
* `9308` 和 `9312` 用于来自远程代理和非 MySQL 客户端的连接
* 在端口 `9306` 上用于 MySQL 连接。

如果您根本没有在配置中指定任何 `listen`，Manticore 将在以下地址等待连接：
* `127.0.0.1:9306` 用于 MySQL 客户端
* `127.0.0.1:9312`  用于 HTTP/HTTPS 和来自其他 Manticore 节点及基于 Manticore 二进制 API 的客户端的连接。

#### 在特权端口上监听

默认情况下，Linux 不允许您让 Manticore 在 1024 以下的端口上监听（例如，`listen = 127.0.0.1:80:http` 或 `listen = 127.0.0.1:443:https`），除非您以 root 身份运行 searchd。如果您仍希望能够启动 Manticore，以便它在非 root 用户下监听 < 1024 的端口，请考虑执行以下操作之一（这些方法中的任何一个都应该有效）：
* 运行命令 `setcap CAP_NET_BIND_SERVICE=+eip /usr/bin/searchd`
* 将 `AmbientCapabilities=CAP_NET_BIND_SERVICE` 添加到 Manticore 的 systemd 单元，并重新加载守护进程（`systemctl daemon-reload`）。

#### 关于 Sphinx API 协议和 TFO 的技术细节
<details>
传统 Sphinx 协议有 2 个阶段：握手交换和数据流。握手由来自客户端的 4 字节数据包和来自守护进程的 4 字节数据包组成，其唯一目的是 - 客户端确定远程是一个真实的 Sphinx 守护进程，守护进程确定远程是一个真实的 Sphinx 客户端。主要的数据流相当简单：让双方都声明他们的握手，另一方进行检查。通过短数据包的交换暗示使用特殊的 `TCP_NODELAY` 标志，该标志关闭 Nagle 的 TCP 算法，并声明 TCP 连接将作为小包的对话进行。
然而，在这种协商中，谁先发言并没有严格定义。从历史上看，所有使用二进制 API 的客户端都先发言：发送握手，然后从守护进程读取 4 字节，然后发送请求并从守护进程读取答案。
当我们改进 Sphinx 协议的兼容性时，我们考虑了这些因素：

1. 通常，主代理通信是从已知客户端到已知主机在已知端口之间建立的。因此，不可能该端点提供错误的握手。因此，我们可以隐含地假设双方都是有效的，确实在使用 Sphinx 协议。
2. 鉴于这种假设，我们可以将握手“粘合”到实际请求中，并在一个数据包中发送。如果后端是一个传统的 Sphinx 守护进程，它将像读取 4 字节握手那样读取这个粘合的数据包，然后请求主体。由于它们都是在一个数据包中到达的，因此后端套接字的 RTT 为 -1，而前端缓冲区仍然正常工作，尽管通常情况下是如此。
3. 继续这个假设：由于“查询”数据包相当小，握手甚至更小，允许我们在初始的“ SYN” TCP 数据包中使用现代 TFO（tcp-fast-open）技术发送两者。也就是说：我们连接到带有粘合握手 + 主体数据包的远程节点。守护进程接受该连接，并立即在套接字缓冲区中拥有握手和主体，因为它们在第一个 TCP “SYN” 数据包中到达。这消除了另一个 RTT。
4. 最后，教会守护进程接受这种改进。实际上，这意味着从应用程序的角度来看，不能使用 `TCP_NODELAY`。而从系统的角度来看，这意味着要确保在守护进程那一侧激活了 TFO 接受功能，并在客户端那一侧也激活了 TFO 发送功能。默认情况下，在现代系统中，客户端 TFO 已默认激活，因此您只需调整服务器 TFO 便可确保一切正常工作。

所有这些改进而不实际更改协议本身，使我们能够消除连接中 1.5 个 RTT 的 TCP 协议。如果查询和答案能够放在一个单一的 TCP 数据包中，则将整个二进制 API 会话从 3.5 RTT 减少到 2 RTT - 这使得网络协商的速度提高了大约 2 倍。

所以，我们所有的改进都围绕一个最初未定义的声明进行：“谁先发言”。如果客户端先发言，我们可以应用所有这些优化，并有效地在单个 TFO 包中处理连接 + 握手 + 查询。此外，我们可以查看接收包的开头并确定一个真实的协议。这就是为什么您可以通过 API/http/https 连接到同一个端口。如果守护进程必须先发言，则所有这些优化都是不可能的，多协议也不可能。因此，我们为 MySQL 设置了一个专用端口，并没有将其与所有其他协议统一到同一个端口。突然之间，在所有客户端中，有一个是以守护进程应先发送握手为前提而编写的。这就是所有描述的改进的可能性。因此，这是 MySQL/MariaDB 的 SphinxSE 插件。因此，特别为这个单一客户端，我们专门定义了 `sphinx` 协议，以以最传统的方式工作。也就是说，双方都激活 `TCP_NODELAY` 并以较小的包进行交换。守护进程在连接时发送其握手，然后客户端发送其握手，然后一切按正常方式工作。这不是很优化，但就是可以工作。如果您使用 SphinxSE 连接到 Manticore - 您必须专门指定一个监听器，并明确声明 `sphinx` 协议。对于其他客户端 - 避免使用这个监听器，因为它速度较慢。如果您使用其他传统的 Sphinx API 客户端 - 首先检查它们是否能够使用非专用多协议端口。对于使用非专用（多协议）端口的主代理链接，启用客户端和服务器 TFO 运行良好，并且一定会使网络后端的工作更快，特别是在您的查询非常轻且快速的情况下。
</details>

### listen_tfo

此设置允许所有监听器的 TCP_FASTOPEN 标志。默认情况下，它由系统管理，但可以通过将其设置为 '0' 明确关闭。

有关 TCP Fast Open 扩展的一般知识，请参考 [Wikipedia](https://en.wikipedia.org/wiki/TCP_Fast_Open)。简而言之，它可以在建立连接时消除一个 TCP 往返延迟。

在实践中，在许多情况下使用 TFO 可以优化客户端-代理网络效率，就像 [持久代理](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 在起作用，但不保持活动连接，并且对于最大连接数没有限制。

在现代操作系统中，TFO 支持通常在系统级别设置为“开启”，但这只是一个“能力”，而非规则。Linux（作为最先进的）自 2011 年以来支持它，从 3.7 开始的内核（服务器端）。 Windows 从某些 Windows 10 构建开始支持它。其他操作系统（FreeBSD，MacOS）也在其中。

对于 Linux 系统，服务器检查变量 `/proc/sys/net/ipv4/tcp_fastopen` 并根据其行为。比特 0 管理客户端，1 管理监听器。默认情况下，系统将此参数设置为 1，即，客户端启用，监听器禁用。

### log

<!-- example conf log -->
日志设置指定记录所有 `searchd` 运行时事件的日志文件名称。如果未指定，默认名称为 'searchd.log'。

或者，您可以使用 'syslog' 作为文件名。在这种情况下，事件将发送到 syslog 守护进程。要使用 syslog 选项，您需要在构建时用 `-–with-syslog` 选项配置 Manticore。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
log = /var/log/searchd.log
```
<!-- end -->


### max_batch_queries

<!-- example conf max_batch_queries -->
限制每个批次的查询数量。可选，默认值为 32。

使 searchd 执行对提交到单个批次的查询数量的合理性检查，使用 [多查询](../Searching/Multi-queries.md)。将其设置为 0 以跳过检查。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_batch_queries = 256
```
<!-- end -->

### max_connections

<!-- example max_connections -->
最大同时客户端连接数。默认情况下无限制。通常只有在使用任何类型的持久连接时，例如 cli mysql 会话或来自远程分布式表的持续远程连接时才会显著。当超过限制时，您仍然可以使用 [VIP 连接](../Connecting_to_the_server/MySQL_protocol.md#VIP-connection) 连接到服务器。VIP 连接不计入限制。

<!-- request Example -->
```ini
max_connections = 10
```

<!-- end -->

### max_threads_per_query

<!-- example max_threads_per_query -->
每个操作可以使用的线程的实例级上限。默认情况下，适当的操作可以占用所有 CPU 内核，无法为其他操作留出空间。例如，针对大量渗透表的 `call pq` 可以在十几秒内利用所有线程。将 `max_threads_per_query` 设置为，比如说， [threads](../Server_settings/Searchd.md#threads) 的一半，会确保您可以并行运行几次这样的 `call pq` 操作。

您还可以在运行时将此设置作为会话变量或全局变量进行设置。

此外，您可以通过 [threads OPTION](../Searching/Options.md#threads) 来控制每个查询的行为。

<!-- intro -->
##### 示例：
<!-- request Example -->

```ini
max_threads_per_query = 4
```

<!-- end -->

### max_filters

<!-- example conf max_filters -->
允许的每个查询过滤器的最大数量。此设置仅用于内部合理性检查，不会直接影响 RAM 使用或性能。可选，默认值为 256。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_filters = 1024
```
<!-- end -->


### max_filter_values

<!-- example conf max_filter_values -->
允许的每个过滤器值的最大数量。此设置仅用于内部合理性检查，不会直接影响 RAM 使用或性能。可选，默认值为 4096。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_filter_values = 16384
```
<!-- end -->


### max_open_files

<!-- example conf max_open_files -->
服务器允许打开的最大文件数称为“软限制”。请注意，服务大型分段实时表可能需要将此限制设置得较高，因为每个磁盘块可能占用十个或更多文件。例如，具有1000个块的实时表可能需要同时打开数千个文件。如果在日志中遇到错误“打开的文件过多”，请尝试调整此选项，因为这可能有助于解决问题。

还有一个“硬限制”，该选项不能超过这个限制。此限制由系统定义，并可以在Linux的文件`/etc/security/limits.conf`中更改。其他操作系统可能有不同的方法，因此请查阅手册以获取更多信息。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = 10000
```
<!-- end -->

<!-- example conf max_open_files max -->
除了直接的数字值之外，您可以使用魔法字词“max”将限制设置为当前可用的硬限制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = max
```
<!-- end -->


### max_packet_size

<!-- example conf max_packet_size -->
最大允许的网络数据包大小。此设置限制来自客户端的查询数据包和来自远程代理的响应数据包在分布式环境中的大小。仅用于内部健康检查，不直接影响RAM使用或性能。可选，默认值为128M。


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

一些挑剔的MySQL客户端库依赖于MySQL使用的特定版本号格式，而且有时会根据报告的版本号（而不是指示的能力标志）选择不同的执行路径。例如，Python MySQLdb 1.2.2在版本号不是X.Y.ZZ格式时抛出异常；MySQL .NET连接器6.3.x在版本号1.x及某些标志组合下内部失败等。为了解决这个问题，您可以使用`mysql_version_string`指令，让`searchd`在通过MySQL协议连接的客户端上报告不同的版本。（默认情况下，它报告自己的版本。）


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
mysql_version_string = 5.0.37
```
<!-- end -->


### net_workers

网络线程数，默认值为1。

此设置在查询率极高时非常有用，当仅一个线程无法处理所有进入的查询时。


### net_wait_tm

控制网络线程的忙循环间隔。默认值为-1，可以设置为-1、0或正整数。

在服务器配置为纯主服务器并仅将请求路由到代理的情况下，处理请求时不允许延迟并且不允许网络线程休眠非常重要。为此，设置了一个忙循环。在接到请求后，如果`net_wait_tm`为正数，网络线程将使用CPU轮询`10 * net_wait_tm`毫秒；如果`net_wait_tm`为`0`，则仅使用CPU进行轮询。此外，可以使用`net_wait_tm = -1`禁用忙循环——在这种情况下，轮询器将超时设置为系统轮询调用上实际代理的超时。

> **警告：** CPU忙循环实际上会负载CPU核心，因此将此值设置为任何非默认值将导致即使在空闲服务器上也会显著使用CPU。


### net_throttle_accept

定义每次网络循环迭代中接受的客户端数量。默认值为0（无限制），对大多数用户来说应当没有问题。这是一个微调选项，用于控制高负载场景下网络循环的吞吐量。


### net_throttle_action

定义每次网络循环迭代中处理的请求数量。默认值为0（无限制），对大多数用户来说应当没有问题。这是一个微调选项，用于控制高负载场景下网络循环的吞吐量。

### network_timeout

<!-- example conf network_timeout -->
网络客户端请求读/写超时，单位为秒（或[special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认值为5秒。`searchd`将在此超时时强制关闭未能发送查询或读取结果的客户端连接。

还要注意参数[reset_network_timeout_on_packet](../Server_settings/Searchd.md#reset_network_timeout_on_packet)。此参数改变`network_timeout`的行为，使其适用于单个数据包而不是整个`query`或`result`。通常，一个查询/结果适合在一个或两个数据包内。然而，在需要大量数据的情况下，此参数在保持活动操作时可以是无价的。

<!-- request Example -->

```ini
network_timeout = 10s
```
<!-- end -->

### node_address

<!-- example conf node_address -->
此设置允许您指定节点的网络地址。默认情况下，它设置为复制的[listen](../Server_settings/Searchd.md#listen) 地址。在大多数情况下这是正确的；但是，有些情况下您必须手动指定它：

* 防火墙后的节点
* 启用网络地址转换（NAT）
* 容器部署，例如Docker或云部署
* 节点在多个区域的集群


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
node_address = 10.101.0.10
```
<!-- end -->

### not_terms_only_allowed

<!-- example conf not_terms_only_allowed -->
此设置决定是否允许仅使用[否定](../Searching/Full_text_matching/Operators.md#Negation-operator)的全文本操作符的查询。可选，默认值为0（失败仅使用NOT操作符的查询）。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
not_terms_only_allowed = 1
```
<!-- end -->

### optimize_cutoff

<!-- example conf optimize_cutoff -->
设置默认的表压缩阈值。详见此处 - [优化磁盘块的数量](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks)。此设置可以通过每个查询选项 [cutoff](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 进行覆盖。它也可以通过 [SET GLOBAL](../Server_settings/Setting_variables_online.md#SET) 动态更改。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
optimize_cutoff = 4
```
<!-- end -->

### persistent_connections_limit

<!-- example conf persistent_connections_limit -->
此设置确定对远程 [持久代理](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 的最大同时持久连接数。每当连接到 `agent_persistent` 下定义的代理时，我们尝试重用现有连接（如果有的话），或者连接并保存连接以供将来使用。然而，在某些情况下，限制这样的持久连接数是有意义的。此指令定义了限制。它影响所有分布式表中每个代理主机的连接数量。

合理的设置值应该等于或小于代理配置中的 [max_connections](../Server_settings/Searchd.md#max_connections) 选项。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
persistent_connections_limit = 29 # assume that each host of agents has max_connections = 30 (or 29).
```
<!-- end -->


### pid_file

<!-- example conf pid_file -->
pid_file 是 Manticore 搜索中的一个强制配置选项，指定 `searchd` 服务器的进程 ID 存储的文件路径。

searchd 进程 ID 文件在启动时重新创建并锁定，并且在服务器运行期间包含主服务器进程 ID。服务器关闭时将其解除链接。
此文件的目的是使 Manticore 能够执行各种内部任务，例如检查是否已经有运行中的 `searchd` 实例、停止 `searchd`，并通知它应该轮换表。该文件也可以用于外部自动化脚本。


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
基于执行时间（最大查询时间设置）在完成之前终止查询是一个不错的安全网，但它有一个固有的缺点：不确定性（不稳定）结果。也就是说，如果您多次以时间限制重复同一个（复杂的）搜索查询，时间限制将在不同阶段达到，您将获得*不同*的结果集。

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

有一个新选项，[SELECT … OPTION max_predicted_time](../Searching/Options.md#max_predicted_time)，可以让您限制查询时间*并*获得稳定、可重复的结果。它不是通过定期检查查询时的实际当前时间来评估查询的（这样是不可确定的），而是使用一个简单的线性模型来预测当前运行时间：

```ini
predicted_time =
    doc_cost * processed_documents +
    hit_cost * processed_hits +
    skip_cost * skiplist_jumps +
    match_cost * found_matches
```

当 `predicted_time` 达到给定限制时，查询将提前终止。

当然，这不是对实际耗时的硬性限制（但是，它确实对所做的*处理*工作量有硬性限制），而且简单的线性模型在任何情况下都不是理想精确的。因此，墙上时钟的时间*可能*低于或超过目标限制。不过，误差范围相当可接受：例如，在我们对100毫秒目标限制的实验中，大多数测试查询落在95到105毫秒的范围内，*所有*查询都在80到120毫秒的范围内。此外，作为一个不错的副作用，使用建模的查询时间而不是测量实际运行时间导致 gettimeofday() 调用略少。

没有两个服务器的品牌和型号是相同的，因此 `predicted_time_costs` 指令允许您配置上述模型的成本。出于方便，它们是整数，以纳秒为单位计数。（max_predicted_time中的限制以毫秒为单位计数，而将成本值指定为0.000128毫秒而不是128纳秒更容易出错。）不必一次性指定所有四个成本，因为未指定的将采用默认值。然而，我们强烈建议指定所有这些以提高可读性。


### preopen_tables

<!-- example conf preopen_tables -->
preopen_tables 配置指令指定是否在启动时强制预打开所有表。默认值为1，这意味着无论每个表的 [preopen](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Other-performance-related-settings) 设置如何，所有表都会被预打开。如果设置为0，那么每个表的设置可以生效，并且默认为0。

预打开表可以防止搜索查询和轮换之间的竞争，这可能偶尔导致查询失败。然而，它也会使用更多的文件句柄。在大多数情况下，建议预打开表。

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
pseudo_sharding 配置选项启用对本地普通和实时表的搜索查询的并行化，无论它们是通过分布式表直接查询还是通过其他方式查询。此功能将自动将查询并行化到 `searchd.threads` 中指定的线程数。

请注意，如果您的工作线程已经忙碌，因为您有：
* 高查询并发
* 任意形式的物理分片：
  - 包含多个普通/实时表的分布式表
  - 由过多磁盘块组成的实时表

然后启用伪分片可能不会提供任何好处，甚至可能导致吞吐量略微下降。如果您优先考虑更高的吞吐量而非更低的延迟，建议禁用此选项。

默认启用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
pseudo_sharding = 0
```
<!-- end -->


### replication_connect_timeout

`replication_connect_timeout` 指令定义了连接到远程节点的超时时间。默认情况下，假定值为毫秒，但可以有 [其他后缀](../../Server_settings/Special_suffixes.md)。默认值为 1000（1 秒）。

在连接到远程节点时，Manticore 最多将等待此时间以成功完成连接。如果达到超时但连接尚未建立，并且启用了 `retries`，则将启动重试。


### replication_query_timeout

`replication_query_timeout` 设置 searchd 等待远程节点完成查询的时间。默认值为 3000 毫秒（3 秒），但可以通过 `suffix` 指定不同的时间单位。

在建立连接后，Manticore 将等待最 长 `replication_query_timeout` 以完成远程节点的操作。请注意，此超时与 `replication_connect_timeout` 是分开的，远程节点可能导致的总延迟将是两个值之和。


### replication_retry_count

此设置是一个整数，指定 Manticore 在报告致命查询错误之前，尝试在复制期间连接并查询远程节点的次数。默认值为 3。


### replication_retry_delay

此设置是一个以毫秒为单位的整数（或 [special_suffixes](../Server_settings/Special_suffixes.md)），指定在复制期间失败时 Manticore 重新查询远程节点之前的延迟。仅当指定非零值时，此值才相关。默认值为 500。

### qcache_max_bytes

<!-- example conf qcache_max_bytes -->
此配置设置为缓存结果集分配的最大 RAM 数量，单位为字节。默认值为 16777216，相当于 16 兆字节。如果将值设置为 0，则禁用查询缓存。有关查询缓存的更多信息，请参阅 [查询缓存](../Searching/Query_cache.md) 以获取详细信息。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
qcache_max_bytes = 16777216
```
<!-- end -->


### qcache_thresh_msec

整数，以毫秒为单位。查询结果被缓存的最小墙时间阈值。默认值为 3000，或 3 秒。0 表示缓存所有内容。有关详细信息，请参阅 [查询缓存](../Searching/Query_cache.md)。该值也可以用时间 [special_suffixes](../Server_settings/Special_suffixes.md) 表示，但使用时要小心，不要与值名称中包含的 '_msec' 混淆。


### qcache_ttl_sec

整数，以秒为单位。缓存结果集的过期时间。默认值为 60，或 1 分钟。最小可能值为 1 秒。有关详细信息，请参阅 [查询缓存](../Searching/Query_cache.md)。该值也可以用时间 [special_suffixes](../Server_settings/Special_suffixes.md) 表示，但使用时要小心，不要与值名称中包含的 '_sec' 混淆。


### query_log_format

<!-- example conf query_log_format -->
查询日志格式。可选，允许的值为 `plain` 和 `sphinxql`，默认值为 `sphinxql`。

`sphinxql` 模式记录有效的 SQL 语句。`plain` 模式以纯文本格式记录查询（主要适用于纯文本使用案例）。此指令允许您在搜索服务器启动时在这两种格式之间切换。日志格式也可以通过使用 `SET GLOBAL query_log_format=sphinxql` 语法动态更改。有关更多详细信息，请参阅 [查询日志记录](../Logging/Query_logging.md)。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_format = sphinxql
```
<!-- end -->

### query_log_min_msec

限制（以毫秒为单位），防止查询被写入查询日志。可选，默认值为 0（所有查询都写入查询日志）。此指令指定只有执行时间超过指定限制的查询才会被记录（该值也可以用时间 [special_suffixes](../Server_settings/Special_suffixes.md) 表示，但使用时要小心，不要与值名称中包含的 `_msec` 混淆）。

### query_log

<!-- example conf query_log -->
查询日志文件名。可选，默认为空（不记录查询）。所有搜索查询（例如 SELECT ... 但不包括 INSERT/REPLACE/UPDATE 查询）将被记录到此文件中。格式在 [查询日志记录](../Logging/Query_logging.md) 中描述。在 'plain' 格式的情况下，您可以使用 'syslog' 作为日志文件的路径。在这种情况下，所有搜索查询将以 `LOG_INFO` 优先级发送到 syslog 守护程序，并前面加上 '[query]'，而不是时间戳。要使用 syslog 选项，Manticore 必须在构建时配置为 `-–with-syslog`。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log = /var/log/query.log
```
<!-- end -->


### query_log_mode

<!-- example conf query_log_mode -->
query_log_mode 指令允许您为 searchd 和查询日志文件设置不同的权限。默认情况下，这些日志文件以 600 权限创建，这意味着只有服务器运行的用户和 root 用户可以读取日志文件。
如果您想允许其他用户读取日志文件，此指令可能会派上用场，例如，运行在非 root 用户下的监控解决方案。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_mode  = 666
```
<!-- end -->

### read_buffer_docs

<!-- example conf read_buffer_docs -->
read_buffer_docs 指令控制文档列表每个关键词的读缓冲区大小。对于每个搜索查询中的每个关键词出现，有两个相关的读缓冲区：一个用于文档列表，一个用于命中列表。此设置允许您控制文档列表缓冲区的大小。

较大的缓冲区大小可能会增加每个查询的内存使用，但可能会减少I/O时间。在慢速存储上设置更大的值是合理的，但对于能够高IOPS的存储，应在较小值的范围内进行实验。

默认值为256K，最小值为8K。您还可以在每个表的基础上设置 [read_buffer_docs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs)，这将覆盖服务器配置级别上的任何设置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_docs = 128K
```
<!-- end -->


### read_buffer_hits

<!-- example conf read_buffer_hits -->
read_buffer_hits指令指定搜索查询中命中列表的每个关键字的读取缓冲区大小。默认情况下，大小为256K，最小值为8K。每次在搜索查询中出现关键字时，有两个关联的读取缓冲区，一个用于文档列表，一个用于命中列表。增加缓冲区大小可能会增加每个查询的内存使用，但减少I/O时间。对于慢速存储，较大的缓冲区大小是合理的，而对于能够高IOPS的存储，应在较小值的范围内进行实验。

该设置也可以在每个表的基础上使用[read_buffer_hits](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_hits)选项进行指定，这将覆盖服务器级别的设置。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_hits = 128K
```
<!-- end -->

### read_unhinted

<!-- example conf read_unhinted -->
未提示读取大小。可选，默认值为32K，最小值1K

在查询时，一些读取事先确切知道要读取的数据量，但某些当前不知道。最明显的是，命中列表的大小当前未知。此设置让您控制在这种情况下要读取多少数据。它影响命中列表的I/O时间，对于大于未提示读取大小的列表，会减少I/O时间，但对于较小的列表，会增加I/O时间。它不会影响内存使用，因为读取缓冲区将已经分配。因此，它不应大于read_buffer。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_unhinted = 32K
```
<!-- end -->

### reset_network_timeout_on_packet

<!-- example conf reset_network_timeout_on_packet -->
优化网络超时（例如`network_timeout`，`read_timeout`和`agent_query_timeout`）的行为。

设置为0时，超时限制发送整个请求/查询的最大时间。
设置为1（默认值）时，超时限制网络活动之间的最大时间。

在复制中，一个节点可能需要将一个大文件（例如100GB）发送到另一个节点。假设网络可以以1GB/s的速度传输数据，且每个数据包的大小为4-5MB。要传输整个文件，您需要100秒。默认的超时时间为5秒，这只允许传输5GB，然后连接被中断。增加超时可能是一个变通办法，但它不可扩展（例如，下一个文件可能是150GB，导致再次失败）。然而，默认的`reset_network_timeout_on_packet`设置为1时，超时不适用于整个传输，而适用于单个数据包。只要传输正在进行（并且在超时时间内实际在网络上接收数据），它就会保持活动。如果传输卡住，以至于在数据包之间发生超时，它将被中断。

请注意，如果您设置了分布式表，每个节点——包括主节点和代理——都应进行调整。在主节点方面，`agent_query_timeout`受到影响；在代理方面，`network_timeout`是相关的。

<!-- intro -->

##### 示例：

<!-- request Example -->

```ini
reset_network_timeout_on_packet = 0
```

<!-- end -->


### rt_flush_period

<!-- example conf rt_flush_period -->
RT表RAM块刷新检查周期，以秒为单位（或[特殊后缀](../Server_settings/Special_suffixes.md)）。可选，默认值为10小时。

积极更新的RT表完全适应RAM块，但仍可能导致不断增长的binlogs，从而影响磁盘使用和崩溃恢复时间。通过该指令，搜索服务器定期执行刷新检查，合格的RAM块可以被保存，从而促进后续的binlog清理。有关更多详细信息，请参见[二进制日志记录](../Logging/Binary_logging.md)。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_flush_period = 3600 # 1 hour
```
<!-- end -->


### rt_merge_iops

<!-- example conf rt_merge_iops -->
RT块合并线程允许启动的最大I/O操作数（每秒）。可选，默认值为0（无限制）。

该指令允许您限制由`OPTIMIZE`语句引起的I/O影响。保证所有RT优化活动不会生成比配置限制更多的磁盘IOPS（每秒I/O）。限制rt_merge_iops可以减少由于合并造成的搜索性能下降。

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

该指令让您限制由`OPTIMIZE`语句引起的I/O影响。大于此限制的I/O将被分解为两个或更多的I/O，然后将被视为与[rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops)限制相关的独立I/O。因此，保证所有优化活动每秒不会产生超过(rt_merge_iops * rt_merge_maxiosize)字节的磁盘I/O。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_maxiosize = 1M
```
<!-- end -->


### seamless_rotate

<!-- example conf seamless_rotate -->
防止在旋转大量数据以进行预缓存的表时`searchd`停滞。可选，默认值为1（启用无缝旋转）。在Windows系统上，默认情况下禁用无缝旋转。

表可能包含一些需要在RAM中预缓存的数据。目前，`.spa`、`.spb`、`.spi`和`.spm`文件被完全预缓存（它们分别包含属性数据、Blob属性数据、关键字表和已删除行映射）。在没有无缝旋转的情况下，旋转表尽量使用尽可能少的RAM，具体如下：

1. 新查询会被暂时拒绝（带有“重试”错误代码）；
2. `searchd`等待所有当前正在运行的查询完成；
3. 旧表被解除分配，文件被重命名；
4. 新表文件被重命名，并分配所需的RAM；
5. 新表的属性和字典数据被预加载到RAM；
6. `searchd`恢复从新表服务查询。

然而，如果有大量属性或字典数据，则预加载步骤可能会需要显著的时间 - 在预加载1-5+ GB文件时，可能需要几分钟。

启用无缝旋转后，旋转工作如下：

1. 为新表分配RAM存储；
2. 新表的属性和字典数据异步预加载到RAM；
3. 成功时，旧表被解除分配，两个表的文件被重命名；
4. 失败时，新表被解除分配；
5. 在任何给定时刻，查询要么从旧表，要么从新表副本中提供。

无缝旋转的代价是旋转期间的峰值内存使用量更高（因为在预加载新副本时，旧的和新的`.spa/.spb/.spi/.spm`数据副本都需要在RAM中）。平均使用量保持不变。


<!-- intro -->
##### 示例：

<!-- request 示例 -->

```ini
seamless_rotate = 1
```
<!-- end -->

### secondary_indexes
<!-- 示例 conf secondary_indexes -->

此选项启用/禁用搜索查询的二级索引的使用。它是可选的，默认值为1（启用）。请注意，您无需在索引时启用它，因为只要安装了[Manticore Columnar Library](https://github.com/manticoresoftware/columnar)，它始终是启用的。后者在搜索时也需要使用索引。提供三种模式：

* `0`：禁用搜索中的二级索引。它们可以通过使用[分析器提示](../Searching/Options.md#Query-optimizer-hints)为单个查询启用。
* `1`：启用搜索中的二级索引。它们可以通过使用[分析器提示](../Searching/Options.md#Query-optimizer-hints)为单个查询禁用。
* `force`：与启用相同，但在加载二级索引期间的任何错误将会被报告，并且整个索引将不会被加载到守护进程中。

<!-- intro -->
##### 示例：

<!-- request 示例 -->

```ini
secondary_indexes = 1
```

<!-- end -->

### server_id

<!-- 示例 conf server_id -->
作为服务器标识符的整数编号，用作生成属于复制集群的节点的唯一短UUID的种子。server_id必须在集群节点之间是唯一的，并且在0到127的范围内。如果未设置server_id，则计算为MAC地址和PID文件路径的哈希，或者会使用随机数作为短UUID的种子。


<!-- intro -->
##### 示例：

<!-- request 示例 -->

```ini
server_id = 1
```
<!-- end -->


### shutdown_timeout

<!-- 示例 conf shutdown_timeout -->
`searchd --stopwait`的等待时间，以秒为单位（或[特殊后缀](../Server_settings/Special_suffixes.md)）。可选，默认值为60秒。

当您运行`searchd --stopwait`时，您的服务器需要在停止之前执行一些活动，例如完成查询、刷新RT RAM块、刷新属性并更新binlog。这些任务需要一些时间。`searchd --stopwait`将最多等待`shutdown_time`秒以让服务器完成其工作。合适的时间取决于您的表大小和负载。


<!-- intro -->
##### 示例：

<!-- request 示例 -->

```ini
shutdown_timeout = 3m # wait for up to 3 minutes
```
<!-- end -->


### shutdown_token

用于从VIP Manticore SQL连接调用“shutdown”命令所需的密码的SHA1哈希。没有它，[debug](../Reporting_bugs.md#DEBUG)的“shutdown”子命令将永远不会导致服务器停止。请注意，这种简单的哈希不应被视为强保护，因为我们没有使用盐哈希或任何现代哈希函数。它旨在作为本地网络中家政守护进程的防错措施。

### snippets_file_prefix

<!-- 示例 conf snippets_file_prefix -->
生成片段时，在本地文件名中添加的前缀。可选，默认是当前工作文件夹。

此前缀可与`load_files`或`load_files_scattered`选项一起用于分布式片段生成。

请注意，这是一种前缀，而**不是**路径！这意味着如果前缀设置为“server1”，而请求引用“file23”，`searchd`将尝试打开“server1file23”（全部不带引号）。因此，如果需要将其视为路径，则必须包含尾随斜杠。

在构建最终文件路径后，服务器会展开所有相对目录，并将最终结果与`snippet_file_prefix`的值进行比较。如果结果不以前缀开头，则该文件将被拒绝，并显示错误信息。

例如，如果您将其设置为`/mnt/data`，而有人以文件`../../../etc/passwd`作为源调用片段生成，则会得到错误信息：

`File '/mnt/data/../../../etc/passwd' escapes '/mnt/data/' scope`

而不是文件内容。

此外，在没有设置参数且读取`/etc/passwd`时，它实际上会读取/daemon/working/folder/etc/passwd，因为该参数的默认值是服务器的工作文件夹。

还请注意，这是一个本地选项；它不会以任何方式影响代理。因此，您可以安全地在主服务器上设置前缀。路由到代理的请求将不会受到主服务器设置的影响。然而，它们将受到代理自身设置的影响。

例如，当文档存储位置（无论是本地存储还是NAS挂载点）在各个服务器之间不一致时，这可能会很有用。


<!-- intro -->
##### 示例：

<!-- request 示例 -->

```ini
snippets_file_prefix = /mnt/common/server1/
```
<!-- end -->

> **警告：** 如果您仍然想访问来自 FS 根的文件，您必须明确将 `snippets_file_prefix` 设置为空值（通过 `snippets_file_prefix=` 行），或设置为根（通过 `snippets_file_prefix=/`）。


### sphinxql_state

<!-- example conf sphinxql_state -->
指向将序列化当前 SQL 状态的文件的路径。

在服务器启动时，该文件会被重放。在合格的状态变化（例如，SET GLOBAL）时，该文件会自动被重写。这可以防止一个难以诊断的问题：如果您加载了 UDF 函数但 Manticore 崩溃，当它被（自动）重启时，您的 UDF 和全局变量将不再可用。使用持久状态可以确保优雅恢复，而不会有这样的意外情况。

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
使用 SQL 接口时，请求之间最大等待时间（以秒为单位，或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认值为 15 分钟。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
sphinxql_timeout = 15m
```
<!-- end -->


### ssl_ca

<!-- example conf ssl_ca -->
指向 SSL 证书颁发机构（CA）证书文件的路径（也称为根证书）。可选，默认值为空。当不为空时，`ssl_cert` 中的证书应由该根证书签署。

服务器使用 CA 文件来验证证书上的签名。该文件必须采用 PEM 格式。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ssl_ca = keys/ca-cert.pem
```
<!-- end -->


### ssl_cert

<!-- example conf ssl_cert -->
指向服务器的 SSL 证书的路径。可选，默认值为空。

服务器将此证书用作自签名公钥，以加密通过 SSL 的 HTTP 流量。该文件必须采用 PEM 格式。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ssl_cert = keys/server-cert.pem
```
<!-- end -->


### ssl_key

<!-- example conf ssl_key -->
指向 SSL 证书密钥的路径。可选，默认值为空。

服务器使用此私钥加密通过 SSL 的 HTTP 流量。该文件必须采用 PEM 格式。


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

此设置限制了公共子树优化器的 RAM 使用（见 [multi-queries](../Searching/Multi-queries.md)）。最多，这样的内存将用于缓存每个查询的文档条目。将限制设置为 0 将禁用优化器。


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

此设置限制了公共子树优化器的 RAM 使用（见 [multi-queries](../Searching/Multi-queries.md)）。最多，这样的内存将用于缓存每个查询的关键词出现（命中）。将限制设置为 0 将禁用优化器。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
subtree_hits_cache = 16M
```
<!-- end -->

### threads

<!-- example threads -->
Manticore 守护进程的工作线程数（或线程池的大小）。Manticore 在启动时创建这个数量的操作系统线程，它们执行守护进程中的所有任务，例如执行查询、创建片段等。一些操作可能被拆分成子任务并并行执行，例如：

* 在实时表中搜索
* 在由本地表组成的分布式表中搜索
* 渗透查询调用
* 以及其他

默认情况下，它的值设置为服务器上的 CPU 核心数量。Manticore 在启动时创建线程，并保持这些线程直到停止。每个子任务可以在需要时使用其中一个线程。当子任务完成时，它释放线程，以便其他子任务可以使用。

在密集 I/O 类型负载的情况下，设置一个高于 CPU 核心数量的值可能是有意义的。

<!-- request Example -->
```ini
threads = 10
```

<!-- end -->

### thread_stack

<!-- example conf thread_stack -->
作业的最大栈大小（协同程序，一个搜索查询可能导致多个作业/协同程序）。可选，默认值为 128K。

每个作业都有其自己的 128K 栈。当您运行查询时，会检查所需的栈大小。如果默认的 128K 足够，就直接处理。如果需要更多，则会调度另一个具有增大栈的作业，继续处理。这种先进栈的最大大小由此设置限制。

将值设置为合理的高值将有助于处理非常深的查询，而不意味着整体 RAM 消耗会增长得太高。例如，将其设置为 1G 并不意味着每个新作业将占用 1G 的 RAM，而是如果我们看到它需要，假设 100M 的栈，我们只分配 100M 给作业。其他作业将在同一时间以其默认的 128K 栈运行。以同样的方式，我们甚至可以运行需要 500M 的更复杂查询。只有当我们 **看到** 内部信息显示该作业需要超过 1G 的栈时，我们才会失败并报告 thread_stack 设置过低。

然而，在实践中，即使是需要 16M 栈的查询通常也过于复杂，无法解析，并且消耗了过多的时间和资源进行处理。因此，守护进程会处理它，但限制此类查询的 `thread_stack` 设置看起来相当合理。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
thread_stack = 8M
```
<!-- end -->


### unlink_old

<!-- example conf unlink_old -->
确定在成功轮换时是否取消链接 `.old` 表副本。可选，默认值为 1（取消链接）。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
unlink_old = 0
```
<!-- end -->


### watchdog

<!-- example conf watchdog -->
线程服务器监视程序。可选，默认值为 1（启用监视程序）。

当一个独角兽查询崩溃时，它可能会导致整个服务器崩溃。启用看门狗功能后，`searchd` 还会维护一个独立的轻量级进程，监视主服务器进程，并在异常终止的情况下自动重启它。看门狗默认启用。

<!-- request Example -->

```ini
watchdog = 0 # disable watchdog
```
<!-- end -->
<!-- proofread -->
