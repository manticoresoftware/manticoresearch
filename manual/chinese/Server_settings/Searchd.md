# Section "Searchd" 在配置文件中的说明

以下设置用于在 Manticore 搜索配置文件的 `searchd` 部分控制服务器的行为。以下是每个设置的摘要：

### access_plain_attrs

此设置为 [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围内的默认值。它是可选的，默认值为 `mmap_preread`。

`access_plain_attrs` 指令允许您定义此 `searchd` 实例管理的所有表的 [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令具有更高的优先级，并将覆盖此实例范围内的默认值，从而提供更精细的控制。

### access_blob_attrs

此设置为 [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围内的默认值。它是可选的，默认值为 `mmap_preread`。

`access_blob_attrs` 指令允许您定义此 `searchd` 实例管理的所有表的 [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令具有更高的优先级，并将覆盖此实例范围内的默认值，从而提供更精细的控制。

### access_doclists

此设置为 [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围内的默认值。它是可选的，默认值为 `file`。

`access_doclists` 指令允许您定义此 `searchd` 实例管理的所有表的 [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令具有更高的优先级，并将覆盖此实例范围内的默认值，从而提供更精细的控制。

### access_hitlists

此设置为 [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围内的默认值。它是可选的，默认值为 `file`。

`access_hitlists` 指令允许您定义此 `searchd` 实例管理的所有表的 [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令具有更高的优先级，并将覆盖此实例范围内的默认值，从而提供更精细的控制。

### access_dict

此设置为 [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围内的默认值。它是可选的，默认值为 `mmap_preread`。

`access_dict` 指令允许您定义此 `searchd` 实例管理的所有表的 [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令具有更高的优先级，并将覆盖此实例范围内的默认值，从而提供更精细的控制。

### agent_connect_timeout

此设置为 [agent_connect_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) 参数设置实例范围内的默认值。


### agent_query_timeout

此设置为 [agent_query_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) 参数设置实例范围内的默认值。可以在每个查询的基础上使用 `OPTION agent_query_timeout=XXX` 子句进行覆盖。


### agent_retry_count

此设置是一个整数，指定 Manticore 在通过分布式表连接和查询远程代理之前报告致命查询错误前尝试的次数。默认值为 0（即不重试）。您也可以在每个查询的基础上使用 `OPTION retry_count=XXX` 子句设置此值。如果提供了每个查询选项，则会覆盖配置中指定的值。

请注意，如果您在分布式表的定义中使用了 [agent mirrors](../Creating_a_cluster/Remote_nodes/Mirroring.md#Agent-mirrors)，服务器将在每次连接尝试时根据选择的 [ha_strategy](../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) 选择不同的镜像。在这种情况下，`agent_retry_count` 将针对一组镜像聚合。

例如，如果您有 10 个镜像并设置了 `agent_retry_count=5`，服务器最多将重试 50 次，假设平均每个镜像有 5 次尝试（使用 `ha_strategy = roundrobin` 选项时，这种情况会发生）。

然而，[agent](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) 定义中的 `retry_count` 选项提供的值作为绝对限制。换句话说，`[retry_count=2]` 选项始终意味着最大 2 次尝试，无论您为代理指定了 1 个还是 10 个镜像。

### agent_retry_delay

此设置是一个以毫秒为单位的整数（或 [special_suffixes](../Server_settings/Special_suffixes.md)），指定在失败后 Manticore 重新查询远程代理之前的延迟时间。只有当指定了非零的 [agent_retry_count](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 或非零的每个查询 `retry_count` 时，此值才相关。默认值为 500。您也可以在每个查询的基础上使用 `OPTION retry_delay=XXX` 子句设置此值。如果提供了每个查询选项，则会覆盖配置中指定的值。


### attr_flush_period

<!-- example conf attr_flush_period -->
当使用[UPDATE](../Data_creation_and_modification/Updating_documents/UPDATE.md)实时修改文档属性时，更改首先写入属性的内存副本。这些更新发生在内存映射文件中，意味着由操作系统决定何时将更改写入磁盘。在`searchd`正常关闭（由`SIGTERM`信号触发）时，所有更改都会被强制写入磁盘。

您也可以指示`searchd`定期将这些更改写回磁盘以防止数据丢失。这些刷新操作之间的间隔由`attr_flush_period`决定，以秒为单位（或使用[特殊后缀](../Server_settings/Special_suffixes.md)）。

默认值为0，表示禁用定期刷新。但是，在正常关闭时仍会发生刷新。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
attr_flush_period = 900 # persist updates to disk every 15 minutes
```
<!-- end -->

### auto_optimize

<!-- example conf auto_optimize -->
此设置控制表的自动[OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE)压缩过程。

默认情况下，表压缩会自动进行。您可以通过`auto_optimize`设置修改此行为：
* 0 表示禁用自动表压缩（您仍可以手动调用`OPTIMIZE`）
* 1 表示显式启用
* 启用它，同时将优化阈值乘以2。

默认情况下，OPTIMIZE会一直运行，直到磁盘块数量小于或等于逻辑CPU核心数乘以2。

但是，如果表具有KNN索引属性，则此阈值不同。在这种情况下，为了提高KNN搜索性能，它被设置为物理CPU核心数除以2。

请注意，开启或关闭`auto_optimize`不会阻止您手动运行[OPTIMIZE TABLE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE)。

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
Manticore支持自动创建尚不存在但在INSERT语句中指定的表。此功能默认启用。要禁用它，请在配置中显式设置`auto_schema = 0`。要重新启用，请设置`auto_schema = 1`或从配置中移除`auto_schema`设置。

请注意，`/bulk` HTTP端点不支持自动表创建。

> 注意：[自动模式功能](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)需要[Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不工作，请确保Buddy已安装。

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
此设置控制二进制日志事务刷新/同步模式。它是可选的，默认值为2（每次事务刷新，每秒同步一次）。

该指令决定二进制日志刷新到操作系统并同步到磁盘的频率。支持三种模式：

*  0，每秒刷新和同步一次。这提供了最佳性能，但在服务器崩溃或操作系统/硬件崩溃时，最多可能丢失一秒钟内已提交的事务。
*  1，每次事务都刷新和同步。此模式性能最差，但保证每个已提交事务的数据都被保存。
*  2，每次事务刷新，每秒同步一次。此模式提供良好的性能，并确保在服务器崩溃时每个已提交的事务都被保存。但是，在操作系统/硬件崩溃的情况下，最多可能丢失一秒钟内已提交的事务。

对于熟悉MySQL和InnoDB的用户，此指令类似于`innodb_flush_log_at_trx_commit`。在大多数情况下，默认的混合模式2在速度和安全性之间提供了良好的平衡，既能完全保护RT表数据免受服务器崩溃的影响，也能在一定程度上防范硬件崩溃。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
binlog_flush = 1 # ultimate safety, low speed
```
<!-- end -->

### binlog_common

<!-- example conf binlog_common -->
此设置控制二进制日志文件的管理方式。它是可选的，默认值为0（每个表单独文件）。

您可以选择两种管理二进制日志文件的方式：

* 每个表单独文件（默认，`0`）：每个表将其更改保存在自己的日志文件中。如果您有许多在不同时间更新的表，这种设置很好。它允许表在更新时无需等待其他表。此外，如果一个表的日志文件出现问题，不会影响其他表。
* 所有表共享单个文件（`1`）：所有表使用同一个二进制日志文件。这种方法使文件管理更容易，因为文件数量更少。但是，如果一个表仍需要保存其更新，这可能会导致文件保留时间超过必要。如果许多表需要同时更新，此设置也可能减慢速度，因为所有更改都必须等待写入一个文件。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
binlog_common = 1 # use a single binary log file for all tables
```
<!-- end -->

### binlog_max_log_size

<!-- example conf binlog_max_log_size -->
此设置控制二进制日志文件的最大大小。它是可选的，默认值为256 MB。

一旦当前二进制日志文件达到此大小限制，将强制打开一个新的二进制日志文件。这导致日志粒度更细，在某些边界工作负载下可以实现更高效的二进制日志磁盘使用。值为0表示不应基于大小重新打开二进制日志文件。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
binlog_max_log_size = 16M
```
<!-- end -->


### binlog_path

<!-- example conf binlog_path -->
此设置确定二进制日志（也称为事务日志）文件的路径。它是可选的，默认值为构建时配置的数据目录（例如，Linux中为`/var/lib/manticore/data/binlog.*`）。

二进制日志用于RT表数据的崩溃恢复，以及属性更新的普通磁盘索引，否则这些数据只会存储在RAM中直到刷新。当启用日志记录时，每个提交到RT表的事务都会被写入日志文件。日志会在非正常关机后启动时自动重放，从而恢复已记录的更改。

`binlog_path` 指令指定二进制日志文件的位置。它只应包含路径；`searchd`将在该目录下根据需要创建和解除链接多个 `binlog.*` 文件（包括binlog数据、元数据以及锁文件等）。

设为空值会禁用二进制日志，这可以提升性能，但会使RT表数据处于风险中。


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
此设置控制 [boolean_simplify](../Searching/Options.md#boolean_simplify) 搜索选项的默认值。它是可选的，默认值为1（启用）。

当设置为1时，服务器会自动应用 [布尔查询优化](../Searching/Full_text_matching/Boolean_optimization.md) 来提升查询性能。当设置为0时，默认不进行优化执行查询。此默认值可以通过对应的搜索选项 `boolean_simplify` 按每个查询进行覆盖。

<!-- request Example -->
```ini
searchd {
    boolean_simplify = 0  # disable boolean optimization by default
}
```
<!-- end -->

### buddy_path

<!-- example conf buddy_path -->
此设置决定 Manticore Buddy 可执行文件的路径。它是可选的，默认值为构建时配置的路径，在不同操作系统之间可能不同。通常情况下，无需修改此设置。但如果你希望以调试模式运行 Manticore Buddy、修改 Manticore Buddy 或实现新插件，此设置将很有用。对于后一种情形，你可以从 https://github.com/manticoresoftware/manticoresearch-buddy 克隆Buddy，向 `./plugins/` 目录添加新插件，并在切换至Buddy源代码目录后运行 `composer install --prefer-source` 便于开发。

为确保你可以运行 `composer`，你的机器必须安装PHP 8.2或更高，并包含以下扩展：

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

你也可以选择releases中的Linux amd64专用 `manticore-executor-dev` 版本，例如：https://github.com/manticoresoftware/executor/releases/tag/v1.0.13

如果采用此方法，请记住将manticore executor的dev版本链接至 `/usr/bin/php`。

要禁用 Manticore Buddy，只需如示例所示将值设为空即可。

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
此设置用于指定使用持久连接时两次请求之间的最大等待时间（以秒或[特殊后缀](../Server_settings/Special_suffixes.md)为单位）。它是可选的，默认值为五分钟。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
client_timeout = 1h
```
<!-- end -->


### collation_libc_locale

<!-- example conf collation_libc_locale -->
服务器libc区域设置。可选，默认是C。

指定libc区域设置，影响基于libc的排序规则。详情请参阅 [collations](../Searching/Collations.md) 部分。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
collation_libc_locale = fr_FR
```
<!-- end -->


### collation_server

<!-- example conf collation_server -->
服务器默认排序规则。可选，默认是libc_ci。

指定用于传入请求的默认排序规则。排序规则可以在每个查询上单独覆盖。有关可用排序规则及其它详细信息，请参阅 [collations](../Searching/Collations.md) 部分。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
collation_server = utf8_ci
```
<!-- end -->


### data_dir

<!-- example conf data_dir -->
当指定此设置后，会启用[实时模式](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)，这是管理数据模式的一种命令式方式。该值应为你希望存储所有表、二进制日志及Manticore Search在此模式下正常运行所需所有内容的目录路径。
当指定 `data_dir` 时，不允许为[普通表](../Creating_a_table/Local_tables/Plain_table.md)建立索引。关于RT模式与普通模式的区别，请参见[本节](../Read_this_first.md#Real-time-table-vs-plain-table)。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
data_dir = /var/lib/manticore
```
<!-- end -->

### diskchunk_flush_search_timeout

<!-- example conf diskchunk_flush_search_timeout -->
用于防止在表内无搜索时自动刷新RAM块的超时时间。可选，默认30秒。

用于在自动刷新前检测表中有无搜索的时间。
只有在最近 `diskchunk_flush_search_timeout` 秒内表中至少有一次搜索，才会自动刷新。该设置与 [diskchunk_flush_write_timeout](../Server_settings/Searchd.md#diskchunk_flush_write_timeout) 联合工作。对应的[每表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_search_timeout)具有更高优先级，并将覆盖该实例级默认值，为你带来更细致的控制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
diskchunk_flush_search_timeout = 120s
```
<!-- end -->

### diskchunk_flush_write_timeout

<!-- example conf diskchunk_flush_write_timeout -->
在没有写入的情况下等待多少秒后自动将RAM块刷新到磁盘。可选，默认值为1秒。

如果在 `diskchunk_flush_write_timeout` 秒内没有写入发生，该 RAM 块将被刷新到磁盘。此设置与 [diskchunk_flush_search_timeout](../Server_settings/Searchd.md#diskchunk_flush_search_timeout) 配合使用。要禁用自动刷新，请在配置中显式设置 `diskchunk_flush_write_timeout = -1`。相应的[每表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_write_timeout)优先级更高，会覆盖此实例范围的默认值，提供更细粒度的控制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
diskchunk_flush_write_timeout = 60s
```
<!-- end -->

### docstore_cache_size

<!-- example conf docstore_cache_size -->
此设置指定了文档存储中保存在内存中的文档块的最大大小。此项为可选，默认值为 16m（16 兆字节）。

当使用 `stored_fields` 时，会从磁盘读取并解压文档块。由于每个块通常包含多个文档，因此在处理下一个文档时可以重用该块。为此，块被保存在服务器范围的缓存中。缓存中保存的是未压缩的块。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
docstore_cache_size = 8m
```
<!-- end -->

### engine

<!-- example conf engine -->
在 RT 模式创建表时使用的默认属性存储引擎。可以是 `rowwise`（默认）或者 `columnar`。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
engine = columnar
```
<!-- end -->


### expansion_limit

<!-- example conf expansion_limit -->
此设置决定单个通配符展开的最大关键字数。可选，默认值为 0（无限制）。

当对启用了 `dict = keywords` 的表执行子字符串搜索时，单个通配符可能会匹配成千上万甚至数百万个关键字（比如匹配整个牛津词典中的 `a*`）。该指令允许限制此类展开的影响。设置 `expansion_limit = N` 可限制每个通配符在查询中最多展开为 N 个出现频率最高的匹配关键字。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_limit = 16
```
<!-- end -->

### expansion_merge_threshold_docs

<!-- example conf expansion_merge_threshold_docs -->
此设置决定允许合并所有此类关键字的扩展关键字中的最大文档数。可选，默认值为 32。

当对启用了 `dict = keywords` 的表执行子字符串搜索时，单个通配符可能会匹配成千上万甚至数百万个关键字。此指令允许你增加允许合并的关键字数目，以加快匹配速度，但会使用更多内存。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_merge_threshold_docs = 1024
```
<!-- end -->

### expansion_merge_threshold_hits

<!-- example conf expansion_merge_threshold_hits -->
此设置决定允许合并所有此类关键字的扩展关键字中的最大命中数。可选，默认值为 256。

当对启用了 `dict = keywords` 的表执行子字符串搜索时，单个通配符可能会匹配成千上万甚至数百万个关键字。此指令允许你增加允许合并的关键字数目，以加快匹配速度，但会使用更多内存。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_merge_threshold_hits = 512
```
<!-- end -->

### expansion_phrase_limit

<!-- example conf expansion_phrase_limit -->
此设置控制由于 `PHRASE`、`PROXIMITY` 和 `QUORUM` 操作符内部的 `OR` 运算符所生成的最大替代短语变体数。可选，默认值为 1024。

当在类似短语的操作符中使用 `|`（或）运算符时，展开的总组合数可能会根据指定的替代项数量呈指数增长。该设置通过限制查询处理期间考虑的排列数，有助于防止过度的查询展开。

如果生成的变体数超过此限制，查询将：

- 失败并返回错误（默认行为）
- 如果启用 `expansion_phrase_warning`，则返回带有警告的部分结果

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_phrase_limit = 4096
```
<!-- end -->

### expansion_phrase_warning

<!-- example conf expansion_phrase_warning -->
此设置控制当查询展开限制由 `expansion_phrase_limit` 超过时的行为。

默认情况下，查询将失败并返回错误信息。当 `expansion_phrase_warning` 设置为 1 时，搜索会继续使用短语的部分转换（直到配置限制），并且服务器会向用户返回带有警告信息的结果集。这样，过于复杂而无法完全展开的查询仍能返回部分结果而不会完全失败。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_phrase_warning = 1
```
<!-- end -->

### grouping_in_utc

此设置指定 API 和 SQL 中基于时间的分组计算是使用本地时区还是 UTC。可选，默认值为 0（表示‘本地时区’）。

By default，所有“按时间分组”表达式（例如在API中的按日、周、月、年分组，以及在SQL中的按日、月、年、年月、年月日分组）均使用本地时间进行计算。例如，若您拥有文档属性时间标记为`13:00 utc`与`15:00 utc`，在分组操作时，它们都将依据您的本地时区设置归入相应的分组。若您位于`utc`时区，它们将归入同一日期；但若您位于`utc+10`时区，则这些文档将被匹配到不同的`按日分组`分组（因为13:00 utc在UTC+10时区为23:00本地时间，而15:00则为次日01:00）。有时，这种行为不被接受，且期望时间分组不依赖于时区。您可运行服务器并定义全局TZ环境变量，但这不仅影响分组，亦影响日志时间标记，此亦可能不期望。启用此选项（于配置或使用SQL中的[SET global](../Server_settings/Setting_variables_online.md#SET)语句）将导致所有时间分组表达式以UTC计算，而其余时间相关函数（例如服务器日志）仍使用本地TZ。


### timezone

此设置指定日期/时间相关函数使用时区。默认使用本地时区，但您可指定不同时区于IANA格式（例如`Europe/Amsterdam`）。

注意此设置不影响日志，日志始终使用本地时区。

同时注意若使用`grouping_in_utc`，则“按时间分组”函数仍使用UTC，而其余日期/时间相关函数使用指定时区。总体，不建议混合使用`grouping_in_utc`与`timezone`。

您可配置此选项于配置或使用SQL中的[SET global](../Server_settings/Setting_variables_online.md#SET)语句。


### ha_period_karma

<!-- example conf ha_period_karma -->
此设置指定代理镜像统计窗口大小，单位为秒（或[特殊后缀](../Server_settings/Special_suffixes.md)）。此选项可选，默认值为60秒。

对于包含代理镜像的分布式表（详见[代理](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)），主节点跟踪多个不同每镜像计数器。这些计数器随后用于故障转移与平衡（主节点基于计数器选择最佳镜像使用）。计数器累积于`ha_period_karma`秒区块。

开始新区块后，主节点仍可使用前区块累积值直至新区块半满。因此，任何前历史最多于1.5倍ha_period_karma秒后停止影响镜像选择。

尽管最多两区块用于镜像选择，为诊断目的存储至15最近区块。这些区块可通过[SHOW AGENT STATUS](../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS)语句检查。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_period_karma = 2m
```
<!-- end -->


### ha_ping_interval

<!-- example conf ha_ping_interval -->
此设置配置代理镜像ping间隔，单位为毫秒（或[特殊后缀](../Server_settings/Special_suffixes.md)）。此选项可选，默认值为1000毫秒。

对于包含代理镜像的分布式表（详见[代理](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)），主节点于空闲期间向所有镜像发送ping命令。此用于跟踪当前代理状态（存活或死亡，网络往返等）。此类ping间隔由此指令定义。为禁用ping，设置ha_ping_interval为0。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_ping_interval = 3s
```
<!-- end -->


### hostname_lookup

`hostname_lookup`选项定义更新主机名策略。默认，代理主机名IP地址于服务器启动缓存以避免过度DNS访问。然而，于某些情况，IP可动态更改（例如云主机）且可能期望不缓存IP。设置此选项为`request`禁用缓存且每查询查询DNS。IP地址亦可手动更新使用`FLUSH HOSTNAMES`命令。

### jobs_queue_size

jobs_queue_size设置定义同时队列可容纳多少“任务”。默认无限制。

于多数情况，“任务”意味对单个本地表（普通表或实时表磁盘块）一查询。例如，若您拥有包含2本地表的分布式表或实时表拥有2磁盘块，搜索查询对任一将多数放置2任务于队列。随后，线程池（其大小定义于[线程](../Server_settings/Searchd.md#threads)）将处理它们。然而，于某些情况，若查询过于复杂，更多任务可创建。调整此设置建议当[最大连接](../Server_settings/Searchd.md#max_connections)与[线程](../Server_settings/Searchd.md#threads)不足以平衡期望性能。

### join_batch_size

表连接工作通过累积匹配批次，这些是左表查询执行结果。此批次随后作为右表单一查询处理。

此选项允许您调整批次大小。默认值为`1000`，设置此选项为`0`禁用批次。

更大批次大小可能提升性能；然而，对于某些查询，可能导致过度内存消耗。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
join_batch_size = 2000
```
<!-- end -->

### join_cache_size

每右表执行查询定义特定JOIN ON条件，这些决定右表获取结果集。

如果只有少数几个唯一的 JOIN ON 条件，重用结果比反复对右表执行查询更有效。为实现这一点，结果集会存储在缓存中。

此选项允许您配置该缓存的大小。默认值为 `20 MB`，将此选项设为 0 则禁用缓存。

请注意，每个线程维护自己的缓存，因此在估算总内存使用时，应考虑执行查询的线程数量。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
join_cache_size = 10M
```
<!-- end -->

### listen_backlog

<!-- example conf listen_backlog -->
listen_backlog 设置决定了 TCP 监听传入连接的队列长度。这对于逐个处理请求的 Windows 版本尤为重要。当连接队列达到限制时，新进入的连接将被拒绝。
对于非 Windows 版本，默认值通常即可，通常无需调整此设置。


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

某些版本的 Kibana 和 OpenSearch Dashboards 期望服务器报告特定的版本号，且根据版本号可能表现不同。为解决此类问题，可以使用此设置，让 Manticore 向 Kibana 或 OpenSearch Dashboards 报告自定义版本。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
kibana_version_string = 1.2.3
```
<!-- end -->

### listen

<!-- example conf listen -->
该设置允许您指定 Manticore 接受连接的 IP 地址及端口，或 Unix 域套接字路径。

`listen` 的通用语法是：

```ini
listen = ( address ":" port | port | path | address ":" port start - port end ) [ ":" protocol [ "_vip" ] [ "_readonly" ] ]
```

您可以指定：
* IP 地址（或主机名）和端口号
* 或仅端口号
* 或 Unix 套接字路径（Windows 不支持）
* 或 IP 地址和端口范围

如果您指定端口号但未指定地址，`searchd` 将监听所有网络接口。Unix 路径以斜杠开头标识。端口范围只能为复制协议设置。

您还可以指定用于该套接字连接的协议处理程序（监听器）。监听器包括：

* **未指定** - Manticore 将接受此端口上传入连接，来源于：
  - 其他 Manticore 代理（例如远程分布式表）
  - 通过 HTTP 和 HTTPS 的客户端
  - [Manticore Buddy](https://manticoresearch.com/blog/manticoresearch-buddy-intro/)。**确保您有此类监听器（或如下面提及的 `http` 监听器）以避免 Manticore 功能受限。**
* `mysql` MySQL 协议，供 MySQL 客户端连接。注意：
  - 也支持压缩协议。
  - 如果启用了 [SSL](../Security/SSL.md#SSL)，可以进行加密连接。
* `replication` - 用于节点间通信的复制协议。更多细节见[复制](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md)部分。您可以指定多个复制监听器，但它们必须监听相同的 IP，只能端口不同。若您定义带端口范围的复制监听器（如 `listen = 192.168.0.1:9320-9328:replication`），Manticore 不会立即监听这些端口，而是在开始使用复制时，从指定范围内随机选取空闲端口监听。为保证复制正常工作，端口范围内至少要有 2 个端口。
* `http` - 同 **未指定**。Manticore 将接受远程代理及通过 HTTP 和 HTTPS 的客户端连接。
* `https` - HTTPS 协议。Manticore 将 **仅** 在此端口接受 HTTPS 连接。更多细节见[SSL](../Security/SSL.md)章节。
* `sphinx` - 传统二进制协议。用于接受远程 [SphinxSE](../Extensions/SphinxSE.md) 客户端连接。某些 Sphinx API 客户端实现（如 Java 客户端）需要显式声明监听器。

对客户端协议添加后缀 `_vip`（即除 `replication` 外的所有，例如 `mysql_vip`、`http_vip` 或仅 `_vip`）会强制为该连接创建专用线程，以绕过各种限制。在节点维护或严重过载时服务器可能挂起或无法通过普通端口连接时，此功能很有用。

后缀 `_readonly` 会为监听器设置[只读模式](../Security/Read_only.md)，并限制仅接受读取查询。

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

可以配置多个 `listen` 指令。`searchd` 会在所有指定的端口和套接字上监听客户端连接。Manticore 包中的默认配置定义了监听端口：
* `9308` 和 `9312`，用于来自远程代理及非 MySQL 客户端的连接
* 以及端口 `9306`，用于 MySQL 连接。

如果配置文件中完全未指定 `listen`，Manticore 会默认等待以下连接：
* `127.0.0.1:9306` 用于 MySQL 客户端
* `127.0.0.1:9312` 用于 HTTP/HTTPS 及来自其他 Manticore 节点和基于 Manticore 二进制 API 的客户端连接

#### 监听特权端口

默认情况下，Linux 不允许您让 Manticore 监听 1024 以下的端口（例如 `listen = 127.0.0.1:80:http` 或 `listen = 127.0.0.1:443:https`），除非您以 root 用户运行 searchd。如果您仍想在非 root 用户下启动 Manticore，让其监听 1024 以下端口，可以考虑以下任一方法（任选一项即可）：
* 执行命令 `setcap CAP_NET_BIND_SERVICE=+eip /usr/bin/searchd`
* 在 Manticore 的 systemd 单元中添加 `AmbientCapabilities=CAP_NET_BIND_SERVICE`，然后重新加载守护进程（`systemctl daemon-reload`）。

#### 技术细节关于Sphinx API协议和TFO
<details>
遗留的Sphinx协议有两个阶段：握手交换和数据流。握手由客户端发送4字节的数据包，然后由守护进程返回4字节的数据包。其唯一目的是：客户端确定远程端是一个真实的Sphinx守护进程，守护进程确定远程端是一个真实的Sphinx客户端。主要的数据流相当简单：双方声明自己的握手，对方检查。这种使用短数据包的交换意味着使用特殊的`TCP_NODELAY`标志，该标志关闭了Nagle的TCP算法，并声明TCP连接将作为小数据包对话进行。
然而，在这次协商中谁先发言并没有严格定义。历史上，所有使用二进制API的客户端都是首先发言：发送握手，然后从守护进程读取4字节，然后发送请求并从守护进程读取答案。
当我们改进Sphinx协议兼容性时，考虑了以下几点：

1. 通常，主节点-代理通信是从已知客户端到已知主机上的已知端口建立的。因此，端点提供错误握手的可能性非常小。因此，我们可以隐式地假设双方都是有效的，并且真的在使用Sphinx协议进行通信。
2. 给定这个假设，我们可以将握手粘接到实际请求上，并在一个数据包中发送它。如果后端是一个遗留的Sphinx守护进程，它将只读取这个粘接的数据包中的4字节握手，然后是请求主体。由于它们在一个数据包中，后端套接字具有-1 RTT，前端缓冲区仍然可以像通常那样工作。
3. 继续这个假设：由于“查询”数据包相当小，而握手更小，让我们使用现代TFO（tcp-fast-open）技术在初始“SYN”TCP数据包中发送两者。也就是说：我们与远程节点建立连接，带有粘接的握手+主体数据包。守护进程接受连接，并立即在套接字缓冲区中拥有握手和主体，因为它们在第一个TCP“SYN”数据包中到达。这消除了另一个RTT。
4. 最后，教守护进程接受这一改进。实际上，从应用程序的角度来看，这意味着不要使用`TCP_NODELAY`。从系统角度来看，这意味着确保在守护进程侧启用TFO，并在客户端侧也启用TFO。默认情况下，在现代系统中，客户端TFO已经默认启用，所以您只需调整服务器TFO即可使所有事情正常工作。

所有这些改进没有实际改变协议本身，允许我们从连接中消除TCP协议的1.5 RTT。如果查询和答案能够放在一个TCP数据包中，则整个二进制API会话从3.5 RTT减少到2 RTT，从而使网络协商速度提高约两倍。

因此，我们的所有改进都围绕一个最初未定义的陈述：“谁先发言”。如果客户端先发言，我们可以应用所有这些优化，并有效地处理连接+握手+查询在一个TFO数据包中。此外，我们可以查看接收数据包的开头来确定真正的协议。这就是为什么您可以使用API/http/https之一连接到同一个端口。如果守护进程必须先发言，所有这些优化都不可能，多协议也不可能。这就是为什么我们为MySQL分配了一个专用端口，并没有将其与其他协议统一到同一个端口。突然之间，在所有客户端中，有一个客户端被编写成假设守护进程应该首先发送握手。即，无法实现所有描述的改进。这就是SphinxSE插件用于mysql/mariadb。因此，特别为此单一客户端专门定义`sphinx`协议以按最传统的模式工作。具体来说：双方激活`TCP_NODELAY`并用小数据包进行交换。守护进程在连接时发送其握手，然后客户端发送其，然后一切按常规方式工作。这不是最优的，但只是能工作。如果您使用SphinxSE连接到Manticore - 您需要专门使用明确声明`sphinx`协议的监听器。对于其他客户端 - 避免使用此监听器，因为它较慢。如果您使用其他遗留Sphinx API客户端 - 先检查它们是否能够使用非专用（多协议）端口工作。对于使用非专用（多协议）端口的主节点-代理链接并启用客户端和服务器TFO效果很好，将肯定使网络后端更快，特别是如果您有非常轻量级和快速的查询。
</details>

### listen_tfo

此设置允许所有监听器使用TCP_FASTOPEN标志。默认情况下，它由系统管理，可以通过将其设置为'0'显式关闭。

有关TCP Fast Open扩展的一般知识，请参阅[维基百科](https://en.wikipedia.org/wiki/TCP_Fast_Open)。简而言之，它允许在建立连接时消除一个TCP往返。

在实践中，在许多情况下使用TFO可以优化客户端-代理网络效率，例如当存在[持久代理](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)时，但无需保持活动连接，也没有最大连接数限制。

在现代操作系统中，TFO支持通常在系统级别打开，但这只是一个“能力”，不是规则。Linux（作为最先进的）自2011年以来就支持它，从内核3.7版本开始（对于服务器端）。Windows从某些Windows 10构建开始支持它。其他操作系统（FreeBSD、MacOS）也在参与。

对于Linux系统服务器，检查变量`/proc/sys/net/ipv4/tcp_fastopen`并根据其行为。位0管理客户端侧，位1管理监听器。默认情况下，系统将此参数设置为1，即客户端启用，监听器禁用。

### log

<!-- example conf log -->
日志设置指定所有`searchd`运行时事件将记录的日志文件名。如果没有指定，默认名称为'searchd.log'。

或者，您可以使用 'syslog' 作为文件名。在这种情况下，事件将被发送到 syslog 守护进程。要使用 syslog 选项，您需要在构建期间使用 `-–with-syslog` 选项配置 Manticore。


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

当使用 [multi-queries](../Searching/Multi-queries.md) 时，使 searchd 对单个批处理中提交的查询数量进行合理性检查。设置为 0 可跳过检查。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_batch_queries = 256
```
<!-- end -->

### max_connections

<!-- example max_connections -->
最大同时客户端连接数。默认无限制。通常只有在使用任何类型的持久连接时才会显著影响，如 cli mysql 会话或来自远程分布式表的持久远程连接。超过限制后，您仍然可以使用 [VIP connection](../Connecting_to_the_server/MySQL_protocol.md#VIP-connection) 连接到服务器。VIP 连接不计入限制。

<!-- request Example -->
```ini
max_connections = 10
```

<!-- end -->

### max_threads_per_query

<!-- example max_threads_per_query -->
一个操作可使用的线程实例范围限制。默认情况下，适当的操作可以占用所有 CPU 核心，导致没有空间给其他操作。例如，对一个相当大的 percolate 表执行 `call pq` 可以使用所有线程持续几十秒。将 `max_threads_per_query` 设置为比如说 [threads](../Server_settings/Searchd.md#threads) 的一半，将确保您可以并行运行几个这样的 `call pq` 操作。

您还可以在运行时作为会话或全局变量设置此配置。

此外，您还可以借助 [threads OPTION](../Searching/Options.md#threads) 对单个查询进行控制。

<!-- intro -->
##### 示例：
<!-- request Example -->

```ini
max_threads_per_query = 4
```

<!-- end -->

### max_filters

<!-- example conf max_filters -->
单查询允许的最大过滤器数量。此设置仅用于内部合理性检查，不直接影响内存使用或性能。可选，默认值为 256。


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
服务器允许打开的最大文件数称为“软限制”。请注意，服务大型碎片化实时表可能需要将此限制设置得较高，因为每个磁盘块可能占用十几个或更多文件。例如，一个有 1000 个块的实时表可能需要同时打开数千个文件。如果您在日志中遇到“Too many open files”错误，请尝试调整此选项，因为它可能有助于解决该问题。

还有一个“硬限制”是选项无法超越的限制。此限制由系统定义，可以在 Linux 的 `/etc/security/limits.conf` 文件中更改。其他操作系统可能有不同的方法，具体请查询相关手册。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = 10000
```
<!-- end -->

<!-- example conf max_open_files max -->
除了直接的数字值，您还可以使用魔法字 'max' 将限制设置为当前可用的硬限制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = max
```
<!-- end -->


### max_packet_size

<!-- example conf max_packet_size -->
允许的最大网络包大小。本设置限制分布式环境中客户端的查询包和远程代理的响应包。仅用于内部合理性检查，不直接影响内存使用或性能。可选，默认值为 128M。


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

某些挑剔的 MySQL 客户端库依赖 MySQL 使用的特定版本号格式，而且有时根据报告的版本号（而非指示的功能标志）选择不同的执行路径。例如，Python MySQLdb 1.2.2 当版本号不符合 X.Y.ZZ 格式时会抛出异常；MySQL .NET 连接器 6.3.x 对版本号为 1.x 并且带有某些组合标志时会内部失败，等等。为了解决这些问题，您可以使用 `mysql_version_string` 指令，让 `searchd` 向通过 MySQL 协议连接的客户端报告不同的版本号。（默认情况下，它报告自己的版本。）


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
mysql_version_string = 5.0.37
```
<!-- end -->


### net_workers

网络线程数，默认为 1。

此设置对于极高查询速率时非常有用，当只有一个线程无法管理所有传入查询时。


### net_wait_tm

控制网络线程的忙循环间隔。默认值为 -1，可设置为 -1、0 或正整数。

当服务器配置为纯主服务器，仅将请求路由到代理时，必须无延迟地处理请求且不能让网络线程睡眠。为此存在忙循环。在接收到请求后，如果 `net_wait_tm` 是正数，则网络线程使用 CPU 轮询 `10 * net_wait_tm` 毫秒；如果 `net_wait_tm` 为 0，则只使用 CPU 轮询。若将 `net_wait_tm` 设为 -1，则会禁用忙循环——在这种情况下，轮询器在系统调用中设置超时时间为代理的实际超时。

> **警告：** CPU忙循环实际上会加载CPU核心，因此将此值设置为任何非默认值都会导致明显的CPU使用率，即使服务器处于空闲状态。


### net_throttle_accept

定义在网络循环的每次迭代中接受多少个客户端。默认值为0（无限制），这对大多数用户来说应该没问题。这是一个微调选项，用于在高负载场景下控制网络循环的吞吐量。


### net_throttle_action

定义在网络循环的每次迭代中处理多少个请求。默认值为0（无限制），这对大多数用户来说应该没问题。这是一个微调选项，用于在高负载场景下控制网络循环的吞吐量。

### network_timeout

<!-- example conf network_timeout -->
网络客户端请求读/写超时，单位为秒（或[特殊后缀](../Server_settings/Special_suffixes.md)）。可选，默认值为5秒。`searchd` 将强制关闭在此超时时间内未能发送查询或读取结果的客户端连接。

另请注意 [reset_network_timeout_on_packet](../Server_settings/Searchd.md#reset_network_timeout_on_packet) 参数。此参数将 `network_timeout` 的行为从应用于整个 `query` 或 `result` 更改为应用于单个数据包。通常，一个查询/结果适合在一个或两个数据包内。但是，在需要大量数据的情况下，此参数对于保持操作活动非常宝贵。

<!-- request Example -->

```ini
network_timeout = 10s
```
<!-- end -->

### node_address

<!-- example conf node_address -->
此设置允许您指定节点的网络地址。默认情况下，它设置为复制 [listen](../Server_settings/Searchd.md#listen) 地址。在大多数情况下这是正确的；但是，在某些情况下您必须手动指定它：

* 节点位于防火墙后面
* 启用了网络地址转换（NAT）
* 容器部署，例如 Docker 或云部署
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
此设置决定是否允许仅包含[否定](../Searching/Full_text_matching/Operators.md#Negation-operator)全文运算符的查询。可选，默认值为0（使仅包含NOT运算符的查询失败）。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
not_terms_only_allowed = 1
```
<!-- end -->

### optimize_cutoff

<!-- example conf optimize_cutoff -->
设置默认的表压缩阈值。在此处阅读更多信息 - [优化磁盘块数](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks)。此设置可以通过每个查询选项 [cutoff](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 覆盖。也可以通过 [SET GLOBAL](../Server_settings/Setting_variables_online.md#SET) 动态更改。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
optimize_cutoff = 4
```
<!-- end -->

### persistent_connections_limit

<!-- example conf persistent_connections_limit -->
此设置确定到远程[持久代理](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)的最大同时持久连接数。每次连接到 `agent_persistent` 下定义的代理时，我们尝试重用现有连接（如果有），或者连接并保存该连接以供将来使用。但是，在某些情况下，限制此类持久连接的数量是有意义的。此指令定义了该限制。它影响跨所有分布式表到每个代理主机的连接数。

将此值设置为等于或小于代理配置中的 [max_connections](../Server_settings/Searchd.md#max_connections) 选项是合理的。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
persistent_connections_limit = 29 # assume that each host of agents has max_connections = 30 (or 29).
```
<!-- end -->


### pid_file

<!-- example conf pid_file -->
pid_file 是 Manticore 搜索中的一个必需配置选项，它指定存储 `searchd` 服务器进程ID的文件路径。

searchd 进程ID文件在启动时重新创建并锁定，并在服务器运行时包含主服务器进程ID。在服务器关闭时取消链接。
此文件的目的是使 Manticore 能够执行各种内部任务，例如检查是否已有正在运行的 `searchd` 实例、停止 `searchd` 以及通知它应该轮换表。该文件也可用于外部自动化脚本。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
pid_file = /run/manticore/searchd.pid
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
基于执行时间（通过最大查询时间设置）在完成前终止查询是一个很好的安全网，但它有一个固有的缺点：非确定性（不稳定）的结果。也就是说，如果您多次重复执行相同的（复杂）搜索查询并设置时间限制，时间限制将在不同阶段被触发，并且您将得到*不同的*结果集。

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

有一个选项，[SELECT … OPTION max_predicted_time](../Searching/Options.md#max_predicted_time)，可以让您限制查询时间*并*获得稳定、可重复的结果。它不是定期检查评估查询时的实际当前时间（这是非确定性的），而是使用一个简单的线性模型来预测当前运行时间：

```ini
predicted_time =
    doc_cost * processed_documents +
    hit_cost * processed_hits +
    skip_cost * skiplist_jumps +
    match_cost * found_matches
```

然后，当 `predicted_time` 达到给定限制时，查询会提前终止。

当然，这并非对实际耗时（但确实是对*处理*工作量的硬性限制）的硬性限制，而且简单的线性模型也绝非理想精确的模型。因此，挂钟时间*可能*低于或超过目标限制。然而，误差范围是相当可接受的：例如，在我们以100毫秒为目标限制的实验中，大多数测试查询落在95到105毫秒的范围内，而*所有*查询都在80到120毫秒的范围内。此外，作为一个不错的副作用，使用建模的查询时间而非测量实际运行时间，也会略微减少gettimeofday()的调用次数。

没有两台服务器的品牌和型号是完全相同的，因此`predicted_time_costs`指令允许您为上述模型配置成本值。为了方便起见，这些值是整数，以纳秒为单位计数。（max_predicted_time中的限制以毫秒为单位计数，而将成本值指定为0.000128毫秒而非128纳秒更容易出错。）不必一次性指定所有四个成本值，未指定的将采用默认值。然而，为了可读性，我们强烈建议指定所有值。


### preopen_tables

<!-- example conf preopen_tables -->
preopen_tables配置指令指定是否在启动时强制预打开所有表。默认值为1，这意味着无论每个表的[preopen](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Other-performance-related-settings)设置如何，所有表都将被预打开。如果设置为0，则每个表的设置可以生效，并且它们将默认为0。

预打开表可以防止搜索查询和轮换之间的竞争，这种竞争偶尔会导致查询失败。然而，它也会使用更多的文件句柄。在大多数场景中，建议预打开表。

以下是一个配置示例：

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
preopen_tables = 1
```
<!-- end -->

### pseudo_sharding

<!-- example conf pseudo_sharding -->
pseudo_sharding配置选项启用对本地普通表和实时表的搜索查询并行化，无论它们是直接查询还是通过分布式表查询。此功能将自动并行化查询，最多使用`searchd.threads`中指定的线程数。

请注意，如果您的worker线程已经繁忙，因为您有以下情况：
* 高查询并发
* 任何类型的物理分片：
  - 由多个普通/实时表组成的分布式表
  - 包含过多磁盘块的实时表

那么启用pseudo_sharding可能不会带来任何好处，甚至可能导致吞吐量略有下降。如果您更看重高吞吐量而非低延迟，建议禁用此选项。

默认启用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
pseudo_sharding = 0
```
<!-- end -->


### replication_connect_timeout

`replication_connect_timeout`指令定义了连接到远程节点的超时时间。默认情况下，该值假定以毫秒为单位，但可以具有[其他后缀](../Server_settings/Special_suffixes.md)。默认值为1000（1秒）。

连接到远程节点时，Manticore将最多等待此时间以成功完成连接。如果达到超时但连接尚未建立，并且启用了`retries`，则将发起重试。


### replication_query_timeout

`replication_query_timeout`设置searchd等待远程节点完成查询的时间量。默认值为3000毫秒（3秒），但可以添加`后缀`以表示不同的时间单位。

建立连接后，Manticore将最多等待`replication_query_timeout`让远程节点完成。请注意，此超时与`replication_connect_timeout`是分开的，远程节点可能造成的总延迟将是这两个值的总和。


### replication_retry_count

此设置是一个整数，指定Manticore在复制期间尝试连接和查询远程节点的次数，然后报告致命查询错误。默认值为3。


### replication_retry_delay

此设置是一个以毫秒（或[special_suffixes](../Server_settings/Special_suffixes.md)）为单位的整数，指定Manticore在复制期间查询远程节点失败时重试前的延迟。仅当指定了非零值时，此值才相关。默认值为500。

### qcache_max_bytes

<!-- example conf qcache_max_bytes -->
此配置设置用于缓存结果集的最大RAM量，单位为字节。默认值为16777216，相当于16兆字节。如果该值设置为0，则查询缓存被禁用。有关查询缓存的更多信息，请参阅[query cache](../Searching/Query_cache.md)了解详情。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
qcache_max_bytes = 16777216
```
<!-- end -->


### qcache_thresh_msec

整数，单位为毫秒。查询结果被缓存的最小挂钟时间阈值。默认为3000，即3秒。0表示缓存所有内容。详情请参阅[query cache](../Searching/Query_cache.md)。此值也可以用时间[special_suffixes](../Server_settings/Special_suffixes.md)表示，但请谨慎使用，不要与值本身的名称混淆，该名称包含'_msec'。


### qcache_ttl_sec

整数，单位为秒。缓存结果集的过期期限。默认为60，即1分钟。最小可能值为1秒。详情请参阅[query cache](../Searching/Query_cache.md)。此值也可以用时间[special_suffixes](../Server_settings/Special_suffixes.md)表示，但请谨慎使用，不要与值本身的名称混淆，该名称包含'_sec'。


### query_log_format

<!-- example conf query_log_format -->
查询日志格式。可选，允许的值为`plain`和`sphinxql`，默认为`sphinxql`。

`sphinxql` 模式记录有效的 SQL 语句。`plain` 模式以纯文本格式记录查询（主要适用于纯全文本用例）。此指令允许您在搜索服务器启动时在这两种格式之间切换。日志格式也可以使用 `SET GLOBAL query_log_format=sphinxql` 语法动态更改。更多详细信息请参阅 [查询日志](../Logging/Query_logging.md)。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_format = sphinxql
```
<!-- end -->

### query_log_min_msec

防止查询被写入查询日志的时间限制（以毫秒为单位）。可选，默认值为 0（所有查询都会写入查询日志）。此指令指定只有执行时间超过所设置限制的查询才会被记录（该值也可以使用时间 [特殊后缀](../Server_settings/Special_suffixes.md) 表示，但请谨慎使用，不要将该值本身包含的 `_msec` 名称弄混）。

### query_log

<!-- example conf query_log -->
查询日志文件名。可选，默认为空（不记录查询）。所有搜索查询（如 SELECT ...，但不包括 INSERT/REPLACE/UPDATE 查询）都会记录在该文件中。格式描述见 [查询日志](../Logging/Query_logging.md)。在使用 'plain' 格式时，可以使用 'syslog' 作为日志文件路径。在这种情况下，所有搜索查询将以 `LOG_INFO` 优先级发送到 syslog 守护进程，前缀为 '[query]'，而不是时间戳。要使用 syslog 选项，Manticore 必须在构建时配置为 `--with-syslog`。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log = /var/log/query.log
```
<!-- end -->


### query_log_mode

<!-- example conf query_log_mode -->
query_log_mode 指令允许您为 searchd 和查询日志文件设置不同的权限。默认情况下，这些日志文件创建时权限为 600，意味着只有运行服务器的用户和 root 用户可以读取日志文件。
如果您希望允许其他用户读取日志文件，比如在非 root 用户下运行的监控方案，这个指令非常有用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_mode  = 666
```
<!-- end -->

### read_buffer_docs

<!-- example conf read_buffer_docs -->
read_buffer_docs 指令控制关键词文档列表的读取缓冲区大小。对于每个搜索查询中的每个关键词出现，都会有两个相关的读取缓冲区：一个用于文档列表，一个用于命中列表。此设置允许您控制文档列表的缓冲区大小。

更大的缓冲区大小可能增加每查询的内存使用，但可能减少 I/O 时间。对于慢速存储，设置较大值是合理的，但对于能够实现高 IOPS 的存储，应在较小值区域进行试验。

默认值为 256K，最小值为 8K。您也可以在每个表级别设置 [read_buffer_docs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs)，它会覆盖服务器配置中的设置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_docs = 128K
```
<!-- end -->


### read_buffer_hits

<!-- example conf read_buffer_hits -->
read_buffer_hits 指令指定关键词命中列表的读取缓冲区大小。默认大小为 256K，最小值为 8K。对于搜索查询中的每个关键词出现，都会有两个相关的读取缓冲区，一个用于文档列表，一个用于命中列表。增加缓冲区大小可以增加每查询的内存使用，但能减少 I/O 时间。对于慢速存储，较大的缓冲区大小是合理的，而对于高 IOPS 存储，则应在较小值区域进行试验。

该设置也可以通过在每个表级别使用 read_buffer_hits 选项在 [read_buffer_hits](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_hits) 中设置，从而覆盖服务器级别的设置。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_hits = 128K
```
<!-- end -->

### read_unhinted

<!-- example conf read_unhinted -->
无提示读取大小。可选，默认值为 32K，最小值为 1K

在查询时，一些读取操作预先知道有多少数据要读，但有些尚未知晓。最明显的是，当前并不知道命中列表的大小。此设置让您控制在这种情况下要读取多少数据。它影响命中列表的 I/O 时间，对于大于无提示读取大小的列表可以减少 I/O 时间，但对于较小的列表则会增加。这**不**影响内存使用，因为读取缓冲区已经分配好。因此，它不应大于 read_buffer。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_unhinted = 32K
```
<!-- end -->

### reset_network_timeout_on_packet

<!-- example conf reset_network_timeout_on_packet -->
细化网络超时行为（例如 `network_timeout` 和 `agent_query_timeout`）。

设置为 0 时，超时限制的是发送整个请求/查询的最大时间。
设置为 1（默认）时，超时限制的是网络活动之间的最大间隔时间。

在复制过程中，一个节点可能需要向另一个节点发送一个大文件（例如 100GB）。假设网络传输速度为 1GB/s，使用大小为 4-5MB 的一系列数据包。传输整个文件需要 100 秒。默认的 5 秒超时只允许传输 5GB 数据，之后连接会断开。增加超时可以作为一种解决办法，但这不具备可扩展性（例如，下一个文件可能是 150GB，同样会失败）。然而，默认 `reset_network_timeout_on_packet` 设置为 1 时，超时不是应用于整个传输，而是应用于单个数据包。只要传输在进行中（且在超时期间网络上有实际接收数据），连接就保持活跃。如果传输出现卡顿，在数据包之间发生超时，则连接会断开。

请注意，如果设置了分布式表，则每个节点——包括主节点和代理节点——都应进行调整。在主节点方面，`agent_query_timeout` 受到影响；在代理节点方面，相关的是 `network_timeout`。

<!-- intro -->

##### 示例：

<!-- request Example -->

```ini
reset_network_timeout_on_packet = 0
```

<!-- end -->


### rt_flush_period

<!-- example conf rt_flush_period -->
RT 表的 RAM 块刷新检查周期，单位为秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认值为 10 小时。

完全适合于 RAM 块的主动更新的 RT 表仍可能导致不断增长的二进制日志，影响磁盘使用和崩溃恢复时间。使用此指令，搜索服务器执行定期的刷新检查，符合条件的 RAM 块可以被保存，从而实现相应的二进制日志清理。更多详情请参见 [Binary logging](../Logging/Binary_logging.md)。

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

此指令允许您限制由 `OPTIMIZE` 语句导致的 I/O 影响。保证所有 RT 优化活动产生的磁盘 IOPS（每秒 I/O 操作）不会超过配置的限制。限制 rt_merge_iops 可以减少合并操作引起的搜索性能下降。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_iops = 40
```
<!-- end -->

### rt_merge_maxiosize

<!-- example conf rt_merge_maxiosize -->
RT 块合并线程允许启动的最大单次 I/O 操作大小。可选，默认值为 0（无限制）。

此指令允许您限制由 `OPTIMIZE` 语句导致的 I/O 影响。超过此大小的 I/O 操作将被拆分成两个或多个操作，随后按照 [rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops) 限制计算为多个 I/O 操作。因此，保证所有优化活动产生的磁盘 I/O 不会超过 (rt_merge_iops * rt_merge_maxiosize) 字节每秒。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_maxiosize = 1M
```
<!-- end -->


### seamless_rotate

<!-- example conf seamless_rotate -->
防止在旋转包含大量数据的表以进行预缓存时 `searchd` 停顿。可选，默认值为 1（启用无缝旋转）。在 Windows 系统上，默认禁用无缝旋转。

表可能包含需要预先缓存到 RAM 的数据。目前，`.spa`、`.spb`、`.spi` 和 `.spm` 文件被完全预缓存（它们分别包含属性数据、blob 属性数据、关键字表和已删除行映射）。如果不使用无缝旋转，旋转表会尽可能少用内存，流程如下：

1. 临时拒绝新查询（返回“重试”错误代码）；
2. `searchd` 等待所有正在运行的查询完成；
3. 旧表被释放，其文件被重命名；
4. 新表文件被重命名，并分配所需的 RAM；
5. 新表的属性和字典数据预加载到 RAM；
6. `searchd` 继续提供来自新表的查询服务。

但是，如果属性或字典数据很多，预加载步骤可能需要显著时间——在预加载 1-5+ GB 的文件时，可能长达数分钟。

启用无缝旋转后，旋转流程如下：

1. 分配新表的 RAM 存储；
2. 异步预加载新表的属性和字典数据至 RAM；
3. 预加载成功后，释放旧表，同时重命名两个表的文件；
4. 预加载失败时，释放新表；
5. 在任何时刻，查询要么从旧表，要么从新表副本提供服务。

无缝旋转代价是旋转期间更高的峰值内存使用（因为预加载新副本时，旧副本和新副本的 `.spa/.spb/.spi/.spm` 数据都必须保留在 RAM 中）。平均内存使用量保持不变。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
seamless_rotate = 1
```
<!-- end -->

### secondary_index_block_cache
<!-- example conf secondary_index_block_cache -->

此选项指定二级索引使用的块缓存大小。可选，默认值为 8 MB。当二级索引处理包含大量值的过滤器（例如 IN() 过滤器）时，它们会读取并处理这些值的元数据块。
在联接查询中，这个过程会对左表的每个行批重复执行，每个批次可能在单个联接查询内多次重读相同的元数据。这会严重影响性能。元数据块缓存会把这些块保存在内存中，
以便后续批次重用。

该缓存仅在联接查询中使用，对非联接查询无影响。请注意，缓存大小限制是针对每个属性和每个二级索引的。每个磁盘块中的每个属性都在这个限制范围内运行。在最坏情况下，总内存
使用量可以估算为缓存大小限制乘以磁盘块数量和联接查询中使用的属性数量。

设置 `secondary_index_block_cache = 0` 将禁用缓存。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
secondary_index_block_cache = 16M
```

<!-- end -->

### secondary_indexes
<!-- example conf secondary_indexes -->

此选项启用或禁用搜索查询时使用二级索引。可选，默认值为 1（启用）。请注意，建立索引时无需启用它，只要安装了 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)，它始终启用。使用该索引搜索时也需要此库。提供三种模式：

* `0`: 禁用在搜索中使用二级索引。可以通过 [analyzer hints](../Searching/Options.md#Query-optimizer-hints) 为单个查询启用
* `1`: 启用在搜索中使用二级索引。可以通过 [analyzer hints](../Searching/Options.md#Query-optimizer-hints) 为单个查询禁用
* `force`: 与启用相同，但加载二级索引时的任何错误都会被报告，且整个索引不会加载到守护进程中。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
secondary_indexes = 1
```

<!-- end -->

### server_id

<!-- example conf server_id -->
作为服务器标识符的整数，用于生成复制集群中节点唯一的短 UUID 种子。server_id 在集群的所有节点中必须唯一，且范围在 0 到 127 之间。如果未设置 server_id，则将其计算为 MAC 地址和 PID 文件路径的哈希，或使用随机数作为短 UUID 的种子。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
server_id = 1
```
<!-- end -->


### shutdown_timeout

<!-- example conf shutdown_timeout -->
`searchd --stopwait` 的等待时间，单位为秒（或[特殊后缀](../Server_settings/Special_suffixes.md)）。可选，默认为 60 秒。

当你运行 `searchd --stopwait` 时，服务器需要执行一些操作后才能停止，例如完成查询、刷新 RT RAM 块、刷新属性和更新二进制日志。这些任务需要一定时间。`searchd --stopwait` 最多等待 `shutdown_time` 秒让服务器完成任务。合适的时间取决于你的表大小和负载。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
shutdown_timeout = 3m # wait for up to 3 minutes
```
<!-- end -->


### shutdown_token

从 VIP Manticore SQL 连接调用“shutdown”命令所需的密码的 SHA1 哈希值。没有它，[debug](../Reporting_bugs.md#DEBUG) 的“shutdown”子命令永远不会导致服务器停止。请注意，这种简单的哈希不能被视为强保护，因为我们没有使用带盐哈希或任何现代哈希函数。它旨在作为局域网中维护守护进程的防呆措施。

### snippets_file_prefix

<!-- example conf snippets_file_prefix -->
生成摘录时，预先添加到本地文件名的前缀。可选，默认是当前工作文件夹。

此前缀可与 `load_files` 或 `load_files_scattered` 选项一起用于分布式摘录生成。

注意，这个是前缀，而**不是**路径！这意味着，如果设置前缀为“server1”，且请求引用“file23”，`searchd` 会尝试打开“server1file23”（不含引号）。因此，如果需要它是路径，必须包含结尾的斜杠。

构造最终文件路径后，服务器会展开所有相对目录，并将结果与 `snippet_file_prefix` 值进行比较。如果结果不以该前缀开头，则此文件会被拒绝并显示错误消息。

例如，如果将其设置为 `/mnt/data`，有人用 `../../../etc/passwd` 作为源文件调用摘录生成时，将收到错误消息：

`File '/mnt/data/../../../etc/passwd' escapes '/mnt/data/' scope`

而不是文件内容。

此外，若参数未设置且读取 `/etc/passwd`，实际将读取 /daemon/working/folder/etc/passwd，因为参数的默认值是服务器的工作文件夹。

还要注意这是一个本地选项；不会以任何方式影响代理。所以你可以安全地在主服务器上设置前缀。路由到代理的请求不会受到主服务器设置的影响，但会受代理自身设置的影响。

这在文档存储位置（无论是本地存储还是 NAS 挂载点）在服务器间不一致时非常有用。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
snippets_file_prefix = /mnt/common/server1/
```
<!-- end -->

> **警告:** 如果你仍希望访问文件系统根目录的文件，必须明确将 `snippets_file_prefix` 设置为空值（通过 `snippets_file_prefix=` 行），或设置为根目录（通过 `snippets_file_prefix=/`）。


### sphinxql_state

<!-- example conf sphinxql_state -->
当前 SQL 状态序列化文件的路径。

服务器启动时，会重放此文件。在符合条件的状态更改（例如，SET GLOBAL）时，会自动重写此文件。这样可以防止难以诊断的问题：如果加载了 UDF 函数，但 Manticore 崩溃且自动重启时，UDF 和全局变量将不再可用。使用持久化状态有助于确保优雅恢复，无意外。

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
使用 SQL 接口时，两个请求之间最大等待时间（单位为秒，或[特殊后缀](../Server_settings/Special_suffixes.md)）。可选，默认为 15 分钟。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
sphinxql_timeout = 15m
```
<!-- end -->


### ssl_ca

<!-- example conf ssl_ca -->
SSL 证书颁发机构 (CA) 证书文件（也称为根证书）的路径。可选，默认为空。不为空时，`ssl_cert` 中的证书应由此根证书签名。

服务器使用 CA 文件验证证书签名。文件必须是 PEM 格式。

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

服务器使用此证书作为自签名公钥，来通过 SSL 加密 HTTP 流量。文件必须是 PEM 格式。


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

服务器使用此私钥通过SSL加密HTTP流量。文件必须为PEM格式。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ssl_key = keys/server-key.pem
```
<!-- end -->


### subtree_docs_cache

<!-- example conf subtree_docs_cache -->
每个查询的最大公共子树文档缓存大小。可选，缺省值为0（禁用）。

此设置限制公共子树优化器的RAM使用量（参见 [多查询](../Searching/Multi-queries.md)）。每个查询最多将使用此数量的RAM来缓存文档条目。将限制设为0会禁用优化器。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
subtree_docs_cache = 8M
```
<!-- end -->


### subtree_hits_cache

<!-- example conf subtree_hits_cache -->
每个查询的最大公共子树命中缓存大小。可选，缺省值为0（禁用）。

此设置限制公共子树优化器的RAM使用量（参见 [多查询](../Searching/Multi-queries.md)）。每个查询最多将使用此数量的RAM来缓存关键字出现次数（命中）。将限制设为0会禁用优化器。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
subtree_hits_cache = 16M
```
<!-- end -->

### threads

<!-- example threads -->
Manticore守护进程的工作线程数（或线程池大小）。Manticore在启动时创建此数量的操作系统线程，它们在守护进程中执行所有任务，例如执行查询、创建片段等。某些操作可能会被拆分为子任务并并行执行，例如：

* 在实时表中搜索
* 在由本地表组成的分布式表中搜索
* 激活查询调用
* 其他操作

默认情况下，它设置为服务器上的CPU核心数。Manticore在启动时创建线程，并在停止时保留它们。每个子任务在需要时可以使用其中一个线程。当子任务完成时，它会释放线程，以便其他子任务可以使用它。

在I/O密集型负载的情况下，将该值设置为高于CPU核心数可能有意义。

<!-- request Example -->
```ini
threads = 10
```

<!-- end -->

### thread_stack

<!-- example conf thread_stack -->
每个任务（协程，一个搜索查询可能引发多个任务/协程）的最大堆栈大小。可选，缺省值为128K。

每个任务都有自己的128K堆栈。当您运行查询时，会检查它需要多少堆栈。如果默认的128K足够，它将直接处理。如果需要更多，将安排另一个堆栈增加的任务，继续处理。这种高级堆栈的最大大小受此设置限制。

将值设置为合理高的数值有助于处理非常深的查询，而不会导致整体RAM消耗过高。例如，将其设置为1G并不意味着每个新任务都会占用1G的RAM，但如果我们看到它需要比如说100M堆栈，我们只需为该任务分配100M。同时，其他任务将使用默认的128K堆栈。同样，我们可以运行甚至更复杂的查询，这些查询需要500M。只有当我们**内部看到**任务需要超过1G的堆栈时，才会失败并报告thread_stack过低。

然而，实际上，即使一个查询需要16M的堆栈，通常对于解析来说也过于复杂，会消耗过多时间和资源。因此，守护进程会处理它，但通过`thread_stack`设置限制此类查询看起来非常合理。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
thread_stack = 8M
```
<!-- end -->


### unlink_old

<!-- example conf unlink_old -->
确定在成功轮换时是否删除`.old`表副本。可选，缺省值为1（执行删除）。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
unlink_old = 0
```
<!-- end -->


### watchdog

<!-- example conf watchdog -->
线程式服务器看门狗。可选，缺省值为1（启用看门狗）。

当Manticore查询崩溃时，可能会导致整个服务器崩溃。启用看门狗功能后，`searchd`还会维护一个独立的轻量级进程，监控主服务器进程，并在发生异常终止时自动重新启动它。看门狗默认是启用的。

<!-- request Example -->

```ini
watchdog = 0 # disable watchdog
```
<!-- end -->
<!-- proofread -->

