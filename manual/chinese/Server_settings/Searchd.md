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
在使用 [Update](../Data_creation_and_modification/Updating_documents/UPDATE.md) 实时修改文档属性时，更改首先写入到属性的内存副本中。这些更新发生在内存映射文件中，意味着操作系统决定何时将更改写入磁盘。在正常关闭 `searchd`（由 `SIGTERM` 信号触发）时，所有更改都会被强制写入磁盘。

您还可以指示 `searchd` 定期将这些更改写回磁盘以防止数据丢失。两次刷新之间的间隔由 `attr_flush_period` 决定，单位为秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。

默认值为 0，表示禁用周期性刷新。但在正常关闭期间仍会进行刷新。

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

默认情况下，表压缩会自动执行。您可以通过 `auto_optimize` 设置修改此行为：
* 0 禁用自动表压缩（您仍然可以手动调用 `OPTIMIZE`）
* 1 明确启用自动压缩
* 在启用的同时将优化阈值乘以 2。

默认情况下，OPTIMIZE 会运行，直到磁盘块数量小于或等于逻辑 CPU 核心数乘以 2。

但是，如果表具有带 KNN 索引的属性，则此阈值不同。在这种情况下，阈值设置为物理 CPU 核心数除以 2，以提高 KNN 搜索性能。

请注意，打开或关闭 `auto_optimize` 并不会阻止您手动运行 [OPTIMIZE TABLE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE)。

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

### parallel_chunk_merges

<!-- example conf parallel_chunk_merges -->
此设置控制服务器在 [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 实时表时允许并行运行的磁盘块合并作业数量。

这只影响磁盘块合并（压缩），不影响查询并行度。

设置为 `1` 可禁用并行块合并（合并作业将逐个运行）。在具有高速存储的系统上，较高的值可能加速压缩，但会增加并发磁盘 I/O。

默认值为 `max(1, min(2, threads/2))`。

此值可通过运行时命令 `SET GLOBAL parallel_chunk_merges = N` 更改，并通过 `SHOW VARIABLES` 查看。

<!-- intro -->
##### 示例：

<!-- request Disable -->
```ini
parallel_chunk_merges = 1
```

<!-- request Increase -->
```ini
parallel_chunk_merges = 4
```

<!-- end -->

### auto_schema

<!-- example conf auto_schema -->
Manticore 支持自动创建尚不存在但在 INSERT 语句中指定的表。此功能默认启用。要禁用它，请在配置中显式设置 `auto_schema = 0`。要重新启用，设置 `auto_schema = 1` 或从配置中删除该设置。

请注意，`/bulk` HTTP 端点不支持自动创建表。

> 注意：[自动模式功能](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果不起作用，请确保 Buddy 已安装。

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
此设置控制二进制日志事务的刷新/同步模式。可选，默认值为 2（每个事务刷新，每秒同步一次）。

该指令决定二进制日志刷新到操作系统及同步到磁盘的频率。支持三种模式：

*  0，每秒刷新和同步一次。性能最佳，但在服务器崩溃或操作系统/硬件故障时，最多可能丢失 1 秒内的已提交事务。
*  1，每个事务都刷新和同步一次。性能最差，但可保证每个已提交事务的数据都被保存。
*  2，每个事务刷新，每秒同步一次。性能良好，在服务器崩溃情况下确保所有已提交事务数据被保存，但在操作系统/硬件故障时，最多可能丢失 1 秒内的已提交事务。

对于熟悉 MySQL 和 InnoDB 的用户，此指令类似于 `innodb_flush_log_at_trx_commit`。通常，默认的混合模式 2 在速度和安全之间提供良好平衡，能充分保护实时表数据免受服务器崩溃影响，并对硬件故障提供一定保护。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
binlog_flush = 1 # ultimate safety, low speed
```
<!-- end -->

### binlog_common

<!-- example conf binlog_common -->
此设置控制二进制日志文件的管理方式。可选，默认值为 0（每个表单独文件）。

您可以在两种二进制日志文件管理方式之间选择：

* 每个表单独一个文件（默认，`0`）：每个表将其更改保存到自己的日志文件中。如果您有许多在不同时间更新的表，此设置很好。它允许表独立更新，无需等待其他表。此外，一个表的日志文件出现问题不会影响其他表。
* 所有表共用一个文件（`1`）：所有表使用相同的二进制日志文件。此方法使文件管理更简单，因为文件更少。但如果某个表仍需保存更新，文件可能保持时间较长。如果许多表同时需要更新，这可能降低性能，因为所有更改必须等待写入同一个文件。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
binlog_common = 1 # use a single binary log file for all tables
```
<!-- end -->

### binlog_max_log_size

<!-- example conf binlog_max_log_size -->
此设置控制最大二进制日志文件大小。此设置为可选，默认值为256 MB。

一旦当前binlog文件达到此大小限制，将强制打开一个新的binlog文件。这导致日志的粒度更细，并且在某些边界工作负载下可以实现更高效的binlog磁盘使用。值为0表示不应基于大小重新打开binlog文件。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
binlog_max_log_size = 16M
```
<!-- end -->


### binlog_path

<!-- example conf binlog_path -->
此设置决定二进制日志（也称为事务日志）文件的路径。此设置为可选，默认值为构建时配置的数据目录（例如Linux中的`/var/lib/manticore/data/binlog.*`）。

二进制日志用于RT表数据的崩溃恢复以及普通磁盘索引的属性更新，这些属性更新否则仅存储在RAM中直到刷新。启用日志记录时，每个提交到RT表的事务都会写入日志文件。在非正常关闭后启动时，日志会自动重播以恢复记录的更改。

`binlog_path`指令指定二进制日志文件的位置。它应仅包含路径；`searchd`将在该目录中根据需要创建和删除多个`binlog.*`文件（包括binlog数据、元数据和锁文件等）。

空值禁用二进制日志记录，这提高性能但会使RT表数据面临风险。


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
此设置控制[boolean_simplify](../Searching/Options.md#boolean_simplify)搜索选项的默认值。此设置为可选，默认值为1（启用）。

设置为1时，服务器将自动应用[布尔查询优化](../Searching/Full_text_matching/Boolean_optimization.md)以提升查询性能。设置为0时，查询默认不经过优化。此默认值可通过相应的搜索选项`boolean_simplify`按查询覆盖。

<!-- request Example -->
```ini
searchd {
    boolean_simplify = 0  # disable boolean optimization by default
}
```
<!-- end -->

### buddy_path

<!-- example conf buddy_path -->
此设置决定Manticore Buddy二进制文件的路径。此设置为可选，默认值为构建时配置的路径，不同操作系统间有所差异。通常情况下无需修改此设置。然而，如果您希望以调试模式运行Manticore Buddy，修改Manticore Buddy，或实现新的插件，此设置将非常有用。在后两种情况下，您可以从https://github.com/manticoresoftware/manticoresearch-buddy克隆Buddy，将新插件添加到`./plugins/`目录中，并切换到Buddy源目录后运行`composer install --prefer-source`以便于开发。

为确保可以运行`composer`，机器必须安装PHP 8.2或更高版本，并启用以下扩展：

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

您还可以选择针对Linux amd64的特殊版本`manticore-executor-dev`，可在发布页面例如：https://github.com/manticoresoftware/executor/releases/tag/v1.0.13 下载。

如果选择此方案，请记得将manticore executor的开发版链接到`/usr/bin/php`。

若要禁用Manticore Buddy，请将值设置为空，如示例所示。

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
此设置决定在使用持久连接时请求之间的最大等待时间（单位为秒或[special_suffixes](../Server_settings/Special_suffixes.md)）。此设置为可选，默认值为五分钟。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
client_timeout = 1h
```
<!-- end -->


### collation_libc_locale

<!-- example conf collation_libc_locale -->
服务器libc语言环境。可选，默认是C。

指定libc语言环境，影响基于libc的排序规则。详情请参阅[排序规则](../Searching/Collations.md)部分。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
collation_libc_locale = fr_FR
```
<!-- end -->


### collation_server

<!-- example conf collation_server -->
默认服务器排序规则。可选，默认是libc_ci。

指定用于传入请求的默认排序规则。排序规则可以在每个查询中覆盖。详情请参阅[排序规则](../Searching/Collations.md)部分中的可用排序规则及其他信息。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
collation_server = utf8_ci
```
<!-- end -->


### data_dir

<!-- example conf data_dir -->
指定此设置时，将启用[实时模式](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)，这是一种管理数据架构的命令式方式。该值应为存储所有表、二进制日志及其他确保Manticore Search在此模式下正常运行所需内容的目录路径。
当指定`data_dir`时，不允许对[普通表](../Creating_a_table/Local_tables/Plain_table.md)进行索引。关于实时模式与普通模式的区别，请阅读[此部分](../Read_this_first.md#Real-time-table-vs-plain-table)。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
data_dir = /var/lib/manticore
```
<!-- end -->

### diskchunk_flush_search_timeout

<!-- example conf diskchunk_flush_search_timeout -->
如果表中无搜索请求，防止RAM区块自动刷新超时时间。可选，默认是30秒。

判断是否自动刷新的搜索检查时间。
自动刷新仅在表中最近 `diskchunk_flush_search_timeout` 秒内至少有一次查询时发生。与 [diskchunk_flush_write_timeout](../Server_settings/Searchd.md#diskchunk_flush_write_timeout) 结合使用。对应的[每表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_search_timeout)优先级更高，会覆盖此实例级默认设置，提供更精细的控制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
diskchunk_flush_search_timeout = 120s
```
<!-- end -->

### diskchunk_flush_write_timeout

<!-- example conf diskchunk_flush_write_timeout -->
在写入超时且无写操作时，等待的秒数，之后自动将RAM块刷新到磁盘。可选，默认值为1秒。

如果在 `diskchunk_flush_write_timeout` 秒内 RAM 块没有写操作，则该块将被刷新到磁盘。与 [diskchunk_flush_search_timeout](../Server_settings/Searchd.md#diskchunk_flush_search_timeout) 配合使用。若要禁用自动刷新，请在配置中显式设置 `diskchunk_flush_write_timeout = -1`。对应的[每表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_write_timeout)优先级更高，会覆盖此实例级默认设置，提供更细粒度的控制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
diskchunk_flush_write_timeout = 60s
```
<!-- end -->

### docstore_cache_size

<!-- example conf docstore_cache_size -->
此设置指定了文档存储中文档块在内存中保留的最大大小。可选，默认值为16m（16兆字节）。

当使用 `stored_fields` 时，文档块从磁盘读取并解压缩。由于每个块通常包含多个文档，因此在处理下一个文档时可能重用该块。为此，块被保存在服务器范围的缓存中。缓存保存的是解压缩的块。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
docstore_cache_size = 8m
```
<!-- end -->

### engine

<!-- example conf engine -->
创建RT模式表时使用的默认属性存储引擎。可为 `rowwise`（默认）或 `columnar`。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
engine = columnar
```
<!-- end -->


### expansion_limit

<!-- example conf expansion_limit -->
此设置确定单个通配符可扩展的最大关键词数量。可选，默认值为0（无限制）。

在对启用了 `dict = keywords` 的表进行子字符串搜索时，单个通配符可能匹配数千甚至数百万个关键词（比如匹配整个牛津词典中的 `a*`）。此指令允许限制这类扩展的影响。设置 `expansion_limit = N` 限制扩展结果不超过每个通配符的N个最常见匹配关键词。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_limit = 16
```
<!-- end -->

### expansion_merge_threshold_docs

<!-- example conf expansion_merge_threshold_docs -->
此设置确定允许合并所有扩展关键词的最大文档数量。可选，默认值为32。

在对启用了 `dict = keywords` 的表进行子字符串搜索时，单个通配符可能匹配成千上万甚至上百万的关键词。此指令允许增加合并关键词的限制，以加快匹配速度，但这会增加搜索使用的内存。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_merge_threshold_docs = 1024
```
<!-- end -->

### expansion_merge_threshold_hits

<!-- example conf expansion_merge_threshold_hits -->
此设置确定允许合并所有扩展关键词的最大命中次数。可选，默认值为256。

在对启用了 `dict = keywords` 的表进行子字符串搜索时，单个通配符可能匹配成千上万甚至上百万个关键词。此指令允许增加合并关键词的限制，以加快匹配速度，但会增加搜索时的内存消耗。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_merge_threshold_hits = 512
```
<!-- end -->

### expansion_phrase_limit

<!-- example conf expansion_phrase_limit -->
此设置控制由于 `PHRASE`、`PROXIMITY` 和 `QUORUM` 操作符内部的 `OR` 操作符所生成的最大替代短语变体数。可选，默认值为1024。

当在类似短语的操作符内使用 `|`（OR）操作符时，展开的组合总数可能随着指定的候选项数量呈指数增长。此设置有助于限制查询扩展，防止过度扩展，通过限制查询处理时考虑的排列数量。

如果生成的变体数量超过此限制，查询将：

- 报错失败（默认行为）
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
此设置控制当查询扩展限制 `expansion_phrase_limit` 被超出时的处理行为。

默认情况下，查询将失败并显示错误信息。设置 `expansion_phrase_warning = 1` 后，搜索将继续使用短语的部分转换（最多到配置的限制），并向用户返回带有警告信息的结果集。这允许对过于复杂而不能完全扩展的查询返回部分结果，而不完全失败。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_phrase_warning = 1
```
<!-- end -->

### grouping_in_utc

此设置指定API和SQL中的定时分组是按本地时区计算还是按UTC计算。该设置是可选的，默认值为0（表示“本地时区”）。

默认情况下，所有“按时间分组”表达式（如API中的按天、周、月和年分组，SQL中的按天、月、年、年月、年月日分组）均使用本地时间进行。例如，如果您有属性时间为`13:00 utc`和`15:00 utc`的文档，在分组时，它们都将根据本地时区设置归入相应的分组。如果您位于`utc`时区，则它们属于同一天，但如果您位于`utc+10`，则这些文档将被匹配到不同的“按天分组”群组（因为UTC+10时区的13:00 utc是本地时间23:00，而15:00 utc是第二天01:00）。有时这种行为不可接受，且希望时间分组不依赖时区。您可以通过设置全局TZ环境变量运行服务器，但这不仅影响分组，还会影响日志中的时间戳，这可能也是不希望的。启用此选项（无论是在配置中还是使用SQL中的[SET global](../Server_settings/Setting_variables_online.md#SET)语句）将使所有时间分组表达式按UTC计算，而其他时间相关功能（如服务器日志记录）仍使用本地时区。


### timezone

此设置指定日期/时间相关函数使用的时区。默认情况下使用本地时区，但您可以指定IANA格式的其他时区（例如`Europe/Amsterdam`）。

请注意，该设置不影响日志记录，日志始终使用本地时区。

还要注意，如果使用了`grouping_in_utc`，则“按时间分组”功能仍使用UTC，而其他日期/时间相关函数将使用指定的时区。总体来说，不建议混用`grouping_in_utc`和`timezone`。

您可以在配置文件中设置此选项，或使用SQL中的[SET global](../Server_settings/Setting_variables_online.md#SET)语句进行配置。


### ha_period_karma

<!-- example conf ha_period_karma -->
此设置指定代理镜像统计的时间窗口大小，单位为秒（或[special_suffixes](../Server_settings/Special_suffixes.md)）。该设置是可选的，默认值为60秒。

对于含有代理镜像的分布式表（详情参见[agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)），主节点跟踪每个镜像的多个不同计数器。这些计数器用于故障切换和平衡（主节点基于计数器选择最佳镜像）。计数器累积的时间块大小为`ha_period_karma`秒。

在开始新时间块后，主节点可能仍使用之前时间块的累计值，直到新时间块达到一半容量。因此，以前的历史数据最多在1.5倍ha_period_karma秒后不再影响镜像选择。

尽管镜像选择最多使用两个时间块，但会存储最多15个最近时间块以供监控使用。可使用[SHOW AGENT STATUS](../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS)语句查看这些时间块。


<!-- intro -->
##### 例子：

<!-- request Example -->

```ini
ha_period_karma = 2m
```
<!-- end -->


### ha_ping_interval

<!-- example conf ha_ping_interval -->
此设置配置代理镜像之间ping命令的间隔，单位为毫秒（或[special_suffixes](../Server_settings/Special_suffixes.md)）。该设置是可选的，默认值为1000毫秒。

对于含有代理镜像的分布式表（详情参见[agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)），主节点会在空闲时向所有镜像发送ping命令，以跟踪代理当前状态（是否存活，网络往返时间等）。ping命令的间隔由此指令定义。若要禁用ping，将ha_ping_interval设置为0。


<!-- intro -->
##### 例子：

<!-- request Example -->

```ini
ha_ping_interval = 3s
```
<!-- end -->


### hostname_lookup

`hostname_lookup`选项定义主机名更新策略。默认情况下，服务器启动时会缓存代理主机名的IP地址，以避免过度访问DNS。然而，在某些情况下，IP可能动态变化（例如云托管），此时可能希望不缓存IP。将此设置为`request`可禁用缓存，每次查询都请求DNS。也可以使用`FLUSH HOSTNAMES`命令手动更新IP地址。

### jobs_queue_size

jobs_queue_size设置定义队列中可同时存在的“作业”数量。默认情况下没有限制。

在大多数情况下，一个“作业”意味着对单个本地表（普通表或实时表的磁盘分区）的一次查询。例如，如果您有一个由2个本地表组成的分布式表，或一个具有2个磁盘分区的实时表，对它们的搜索查询通常会在队列中放入2个作业。然后，线程池（大小由[threads](../Server_settings/Searchd.md#threads)定义）处理这些作业。但在某些情况下，如果查询过于复杂，可能生成更多作业。当[ max_connections](../Server_settings/Searchd.md#max_connections)和[threads](../Server_settings/Searchd.md#threads)不足以达到性能平衡时，建议调整此设置。

### join_batch_size

表连接的工作方式是累计一批匹配项，这些匹配项是针对左表执行查询所得的结果。然后，这批结果作为单次查询发送到右表进行处理。

此选项允许您调整批处理大小。默认值为`1000`，将此选项设置为`0`即可禁用批处理。

较大的批处理大小可能提升性能；但对于某些查询，可能导致内存消耗过大。

<!-- intro -->
##### 例子：

<!-- request Example -->

```ini
join_batch_size = 2000
```
<!-- end -->

### join_cache_size

右表上执行的每个查询由特定的 JOIN ON 条件定义，这些条件决定了从右表检索的结果集。

如果只有少数唯一的 JOIN ON 条件，重用结果集通常比重复执行右表查询更高效。为实现此目的，结果集被存储在缓存中。

此选项允许您配置该缓存的大小。默认值为 `20 MB`，将此选项设置为 0 则禁用缓存。

请注意，每个线程维护其自己的缓存，因此在估计总内存使用时应考虑执行查询的线程数。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
join_cache_size = 10M
```
<!-- end -->

### listen_backlog

<!-- example conf listen_backlog -->
listen_backlog 设置决定了用于传入连接的 TCP 监听队列的长度。这对于逐个处理请求的 Windows 版本尤其相关。当连接队列达到其限制时，新的传入连接将被拒绝。
对于非 Windows 版本，默认值通常工作良好，通常无需调整此设置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
listen_backlog = 20
```
<!-- end -->

### kibana_version_string

<!-- example conf kibana_version_string -->
返回给 Kibana 或 OpenSearch Dashboards 的服务器版本字符串。可选——默认值为 `7.6.0`。

某些版本的 Kibana 和 OpenSearch Dashboards 期望服务器报告特定的版本号，可能会据此表现不同。为了解决此类问题，可以使用此设置，使 Manticore 向 Kibana 或 OpenSearch Dashboards 报告自定义版本。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
kibana_version_string = 1.2.3
```
<!-- end -->

### listen

<!-- example conf listen -->
此设置允许您指定 Manticore 接受连接的 IP 地址和端口，或 Unix 域套接字路径。

`listen` 的通用语法是：

```ini
listen = ( address ":" port | port | path | address ":" port start - port end ) [ ":" protocol [ "_vip" ] [ "_readonly" ] ]
```

您可以指定：
* IP 地址（或主机名）和端口号
* 或仅端口号
* 或 Unix 套接字路径（Windows 上不支持）
* 或 IP 地址和端口范围

如果您指定了端口号但未指定地址，`searchd` 将监听所有网络接口。Unix 路径通过前导斜杠识别。端口范围仅可用于复制协议。

您还可以为此套接字上的连接指定协议处理器（监听器）。监听器为：

* **未指定** - Manticore 将在此端口接受来自：
  - 其他 Manticore 代理（即远程分布式表）
  - 通过 HTTP 和 HTTPS 的客户端
  - [Manticore Buddy](https://manticoresearch.com/blog/manticoresearch-buddy-intro/)。**确保您有此类监听器（或下面提到的 `http` 监听器），以避免 Manticore 功能受限。**
* `mysql` - 用于来自 MySQL 客户端连接的 MySQL 协议。注意：
  - 也支持压缩协议。
  - 如果启用 [SSL](../Security/SSL.md#SSL)，可以建立加密连接。
* `replication` - 用于节点通信的复制协议。更多详情见 [复制](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md) 章节。可以指定多个复制监听器，但它们必须监听相同 IP；只有端口可以不同。当您定义带端口范围的复制监听器（例如 `listen = 192.168.0.1:9320-9328:replication`）时，Manticore 不会立即开始监听这些端口。只有在开始使用复制时，才会从指定范围内随机选择空闲端口。复制正常工作至少需要 2 个端口。
* `http` - 与 **未指定** 相同。Manticore 将接受来自远程代理和通过 HTTP 及 HTTPS 的客户端连接。
* `https` - HTTPS 协议。Manticore 只接受此端口上的 HTTPS 连接。详情参见 [SSL](../Security/SSL.md) 章节。
* `sphinx` - 传统二进制协议。用于服务远程 [SphinxSE](../Extensions/SphinxSE.md) 客户端连接。一些 Sphinx API 客户端实现（例如 Java 版）需要显式声明监听器。

向客户端协议（即除 `replication` 外的所有协议，例如 `mysql_vip`、`http_vip` 或仅 `_vip`）添加后缀 `_vip` 会强制为连接创建专用线程，以绕过各种限制。在服务器严重过载并可能停顿或无法通过普通端口连接时，这对节点维护非常有用。

后缀 `_readonly` 为监听器设置[只读模式](../Security/Read_only.md)，并限制其仅接受读查询。

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

可以有多个 `listen` 指令。`searchd` 将在所有指定的端口和套接字上监听客户端连接。Manticore 软件包提供的默认配置在以下端口监听：
* `9308` 和 `9312`，用于远程代理和非 MySQL 客户端的连接
* 以及端口 `9306`，用于 MySQL 连接。

如果您在配置中完全未指定任何 `listen`，Manticore 将等待以下连接：
* `127.0.0.1:9306` 的 MySQL 客户端连接
* `127.0.0.1:9312` 的 HTTP/HTTPS 连接以及来自其他 Manticore 节点和基于 Manticore 二进制 API 的客户端的连接

#### 监听特权端口

默认情况下，Linux 不允许 Manticore 监听 1024 以下的端口（例如 `listen = 127.0.0.1:80:http` 或 `listen = 127.0.0.1:443:https`），除非以 root 用户身份运行 searchd。如果您仍希望在非 root 用户下启动 Manticore 监听 1024 以下端口，考虑执行以下任一操作（任一操作均可）：
* 运行命令 `setcap CAP_NET_BIND_SERVICE=+eip /usr/bin/searchd`
* 向 Manticore 的 systemd 单元添加 `AmbientCapabilities=CAP_NET_BIND_SERVICE` 并重新加载守护进程（`systemctl daemon-reload`）。

#### 关于 Sphinx API 协议和 TFO 的技术细节
<details>
传统的 Sphinx 协议有两个阶段：握手交换和数据流。握手由客户端发送的4字节数据包和守护进程发送的4字节数据包组成，唯一目的——客户端确定远程是一个真实的 Sphinx 守护进程，守护进程确定远程是一个真实的 Sphinx 客户端。主要的数据流非常简单：双方都声明自己的握手包，对方校验它们。用短包交换意味着使用特殊的 `TCP_NODELAY` 标志，它关闭了 Nagle 的 TCP 算法，并声明 TCP 连接将以小包对话的方式执行。
但是，协议中并没有严格定义谁先发言。历史上，所有使用二进制 API 的客户端都会先说话：发送握手，接着读取守护进程返回的4字节，再发送请求并读取守护进程的回答。
当我们改进 Sphinx 协议兼容性时，考虑了以下几个方面：

1. 通常主代理通信由已知客户端连接到已知主机上的已知端口。因此，终端提供错误握手的可能性很低。因此，我们可以隐含假设双方都是有效且确实是用 Sphinx 协议通信。
2. 基于此假设，我们可以将握手“粘合”到真实请求中，并将其作为一个包发送。如果后端是传统 Sphinx 守护进程，它会先读取这4字节的握手然后读取请求体。由于它们都在一个包中，后端套接字节省了1个 RTT，而前端缓冲区依然照常工作。
3. 延续此假设：由于“查询”包非常小，握手包更小，让我们使用现代的 TFO（tcp-fast-open）技术，在初始的 'SYN' TCP 包中发送两者。即：连接远程节点时，以粘合的握手加请求体包进行连接。守护进程接受连接时，这两部分数据已立刻到达套接字缓冲区，就像它们在第一个 TCP 'SYN' 包里一样，消除了又一个 RTT。
4. 最后，教会守护进程支持此改进。实际上，这意味着应用层不使用 `TCP_NODELAY`。系统层面要确保守护进程端启用 TFO 的接受，客户端同时启用 TFO 的发送。现代系统默认客户端 TFO 已启用，只需调试服务端的 TFO 即可正常工作。

所有这些改进，在不改变协议本身的前提下，使我们消除了 TCP 连接中的1.5个 RTT。如果查询和回答能够放入一个 TCP 包，则整个二进制 API 会话从3.5个 RTT减少到2个 RTT——网络协商速度提升了一倍。

所有改进都基于最初未定义的命题：“谁先说话？”如果客户端先说话，我们就可以应用这些优化，在单个 TFO 包中高效完成连接＋握手＋查询。此外，我们可以查看接收包的开头判断实际协议。这就是为什么同一个端口可以通过 API/http/https 多种方式连接。如果守护进程必须先说话，所有这些优化都无法实现，多协议也无法支持。这就是为什么 MySQL 有专用端口而不统一到其他协议端口。某客户端要求守护进程先发送握手——不可能做上述优化，那就是 MySQL/MariaDB 的 SphinxSE 插件。针对它，我们专门设立了 `sphinx` 协议定义，最大程度以传统方式工作。即双方启用 `TCP_NODELAY`，以小包交换，守护进程连接时先发握手，然后客户端发握手，然后以常规方式工作。这种方式不够优化，但能正常工作。若用 SphinxSE 连接 Manticore，需专门监听并显式用 `sphinx` 协议；其他客户端则尽量避免使用此监听器，因为速度较慢。若用其他传统 Sphinx API 客户端，需先确认它们是否支持非专用多协议端口。对于主-agent 链接，使用非专用（多协议）端口及同时启用客户端和服务器的 TFO 表现良好，能显著提升网络后端速度，特别是在查询轻量快速时效果明显。
</details>

### listen_tfo

此设置允许为所有监听器启用 TCP_FASTOPEN 标志。默认由系统管理，但可显式设置为 '0' 关闭。

有关 TCP Fast Open 扩展的一般知识，请参阅 [Wikipedia](https://en.wikipedia.org/wiki/TCP_Fast_Open)。简而言之，它允许建立连接时省去一个 TCP 往返。

实际上，在许多情况下，使用 TFO 可优化客户端与代理间的网络效率，就像使用 [持久代理](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 一样，但不保持活动连接，并且没有连接数最大限制。

在现代操作系统中，TFO 支持通常默认启用，但这只是“能力”而非规则。Linux（作为最先进系统）从2011年起支持此功能，内核自3.7开始支持服务器端。Windows 在 Windows 10 的某些版本支持。其他操作系统（FreeBSD，MacOS）也在支持中。

Linux 系统服务器查看变量 `/proc/sys/net/ipv4/tcp_fastopen` 并据此行为。第0位控制客户端，第1位控制监听器。默认系统设置是1，代表客户端启用，监听器禁用。

### log

<!-- example conf log -->
log 设置指明了所有 `searchd` 运行时事件的日志文件名。如果未指定，默认名称是 'searchd.log'。

或者，您也可以使用 'syslog' 作为文件名。在这种情况下，事件将发送到 syslog 守护进程。要使用 syslog 选项，您需要在构建过程中使用 `-–with-syslog` 选项配置 Manticore。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
log = /var/log/searchd.log
```
<!-- end -->


### max_batch_queries

<!-- example conf max_batch_queries -->
限制每个批次的查询数量。可选，默认值是 32。

使 searchd 在使用[多查询](../Searching/Multi-queries.md)时，对单个批次中提交的查询数量进行合理性检查。将其设置为 0 可以跳过该检查。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_batch_queries = 256
```
<!-- end -->

### max_connections

<!-- example max_connections -->
最大同时客户端连接数。默认无限制。通常只有在使用任何类型的持久连接时才会注意到，比如 cli mysql 会话或来自远程分布式表的持久远程连接。当超过限制时，您仍然可以使用[VIP 连接](../Connecting_to_the_server/MySQL_protocol.md#VIP-connection)连接服务器。VIP 连接不计入连接数限制。

<!-- request Example -->
```ini
max_connections = 10
```

<!-- end -->

### max_threads_per_query

<!-- example max_threads_per_query -->
单个操作可使用的线程实例级限制。默认情况下，适当的操作可以占用所有 CPU 核心，不给其他操作留空间。例如，对相当大的实时表进行 `call pq` 可能会利用所有线程数持续数十秒。将 `max_threads_per_query` 设置为比如[threads](../Server_settings/Searchd.md#threads)的一半，可以确保可以并行运行几个这样的 `call pq` 操作。

您也可以在运行时将此设置作为会话或全局变量进行设置。

此外，您还可以借助[threads 选项](../Searching/Options.md#threads)，对单个查询的行为进行控制。

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
每个过滤器允许的最大值数量。此设置仅用于内部合理性检查，不直接影响内存使用或性能。可选，默认值是 4096。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_filter_values = 16384
```
<!-- end -->


### max_open_files

<!-- example conf max_open_files -->
服务器允许打开的最大文件数称为“软限制”。请注意，服务大型碎片化实时表可能需要将此限制设置得较高，因为每个磁盘块可能占用十几个或更多文件。例如，一个拥有 1000 个块的实时表可能需要同时打开数千个文件。如果日志中出现 “Too many open files” 错误，请尝试调整此选项，可能有助于解决问题。

此外还有一个无法被选项超越的“硬限制”。此限制由系统定义，可以在 Linux 系统的 `/etc/security/limits.conf` 文件中更改。其他操作系统可能有不同的方式，请参考您的手册获取更多信息。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = 10000
```
<!-- end -->

<!-- example conf max_open_files max -->
除了直接的数值外，您还可以使用魔法字 'max' 将限制设置为当前可用的硬限制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = max
```
<!-- end -->


### max_packet_size

<!-- example conf max_packet_size -->
允许的最大网络数据包大小。此设置限制来自客户端的查询包以及分布式环境中远程代理的响应包。仅用于内部合理性检查，不直接影响内存使用或性能。可选，默认值为 128M。


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

一些挑剔的 MySQL 客户端库依赖于 MySQL 使用的特定版本号格式，而且有时基于报告的版本号（而不是能力标志）选择不同的执行路径。例如，Python MySQLdb 1.2.2 如果版本号不是 X.Y.ZZ 格式会抛出异常；MySQL .NET connector 6.3.x 在版本号为 1.x 并且有某些标志组合时内部分失败，等等。为了解决这个问题，您可以使用 `mysql_version_string` 指令，让 `searchd` 向通过 MySQL 协议连接的客户端报告不同的版本。（默认情况下，它报告自身版本。）


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
mysql_version_string = 5.0.37
```
<!-- end -->


### net_workers

网络线程数，默认是 1。

此设置在极高查询率下非常有用，当单个线程不足以处理所有传入查询时。


### net_wait_tm

控制网络线程的忙等待循环间隔。默认是 -1，可以设置为 -1、0 或正整数。

在服务器被配置为纯主服务器并且仅将请求路由到代理的情况下，重要的是能无延迟地处理请求，并且不允许网络线程进入休眠状态。为此，存在一个忙循环。在收到请求后，如果 `net_wait_tm` 为正数，网络线程会使用 CPU 轮询 `10 * net_wait_tm` 毫秒；如果 `net_wait_tm` 为 `0`，则仅用 CPU 进行轮询。同时，可以通过将 `net_wait_tm = -1` 来禁用忙循环 —— 在这种情况下，轮询器在系统轮询调用时将超时设置为实际代理的超时时间。

> **警告：** CPU 忙循环实际上会使 CPU 核心处于高负载，因此将此值设置为任何非默认值都会导致即使在服务器空闲时也能观察到明显的 CPU 使用率。


### net_throttle_accept

定义每次网络循环迭代接受多少客户端连接。默认值为 0（无限制），对大多数用户来说已经足够。这个选项是用于在高负载场景下对网络循环吞吐量进行微调的手段。


### net_throttle_action

定义每次网络循环迭代处理多少请求。默认值为 0（无限制），对大多数用户来说已经足够。这个选项是用于在高负载场景下对网络循环吞吐量进行微调的手段。

### network_timeout

<!-- example conf network_timeout -->
网络客户端请求读写超时时间，单位为秒（或使用 [特殊后缀](../Server_settings/Special_suffixes.md)）。此参数为可选，默认值为 5 秒。`searchd` 将强制关闭在此超时内未能发送查询或读取结果的客户端连接。

另请注意 [reset_network_timeout_on_packet](../Server_settings/Searchd.md#reset_network_timeout_on_packet) 参数。该参数将 `network_timeout` 的行为从应用于整个 `query` 或 `result` 变更为仅应用于单个数据包。通常，一个查询或结果会包含一个或两个数据包，但在需要大量数据的情况下，这个参数对维持活动操作十分有用。

<!-- request Example -->

```ini
network_timeout = 10s
```
<!-- end -->

### node_address

<!-- example conf node_address -->
此设置允许您指定节点的网络地址。默认情况下，它被设置为复制的 [listen](../Server_settings/Searchd.md#listen) 地址。在大多数情况下这是正确的；然而，在以下场景中，您需要手动指定：

* 节点位于防火墙后面
* 启用了网络地址转换（NAT）
* 容器部署，比如 Docker 或云部署
* 集群中的节点分布在多个区域


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
node_address = 10.101.0.10
```
<!-- end -->

### not_terms_only_allowed

<!-- example conf not_terms_only_allowed -->
此设置确定是否允许仅包含[否定](../Searching/Full_text_matching/Operators.md#Negation-operator)全文操作符的查询。此参数为可选，默认值为 0（拒绝仅包含 NOT 操作符的查询）。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
not_terms_only_allowed = 1
```
<!-- end -->

### optimize_cutoff

<!-- example conf optimize_cutoff -->
设置默认的表压缩阈值。详细信息请参阅 - [已优化磁盘块数量](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks)。该设置可以通过每个查询选项 [cutoff](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 覆盖。也可以通过 [SET GLOBAL](../Server_settings/Setting_variables_online.md#SET) 动态修改。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
optimize_cutoff = 4
```
<!-- end -->

### persistent_connections_limit

<!-- example conf persistent_connections_limit -->
此设置确定远程 [持久代理](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)的最大同时持久连接数。每次连接 `agent_persistent` 下定义的代理时，我们都会尝试重用现有连接（如果有），或者建立新连接并保存以供将来使用。不过，在某些情况下，限制持久连接数量是有意义的。该指令定义了该限制。它影响所有分布式表中每个代理主机的连接数。

合理的做法是将该值设置为不高于代理配置中的 [max_connections](../Server_settings/Searchd.md#max_connections) 选项。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
persistent_connections_limit = 29 # assume that each host of agents has max_connections = 30 (or 29).
```
<!-- end -->


### pid_file

<!-- example conf pid_file -->
pid_file 是 Manticore search 中的一个必填配置选项，指定存储 `searchd` 服务器进程 ID 的文件路径。

searchd 进程 ID 文件在启动时被重新创建并锁定，服务器运行时包含主进程的进程 ID。服务器关闭时该文件会被删除。
该文件的目的是让 Manticore 执行各种内部任务，例如检查是否已有 `searchd` 进程在运行、停止 `searchd` 以及通知它应当旋转表。该文件也可用于外部自动化脚本。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
pid_file = /run/manticore/searchd.pid
```
<!-- end -->


### predicted_time_costs

<!-- example conf predicted_time_costs -->
查询时间预测模型的成本，单位为纳秒。选填，默认值为 `doc=64, hit=48, skip=2048, match=64`。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
predicted_time_costs = doc=128, hit=96, skip=4096, match=128
```
<!-- end -->

<!-- example conf predicted_time_costs 1 -->
基于执行时间（使用最大查询时间设置）在查询完成前终止查询是一种不错的保护措施，但它有一个固有缺点：结果具有不确定性（不稳定）。也就是说，如果对完全相同（复杂）的带有时间限制的搜索查询重复多次，时间限制会在不同阶段被触发，因而得到 *不同* 的结果集。

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

有一个选项，[SELECT … OPTION max_predicted_time](../Searching/Options.md#max_predicted_time)，它允许你限制查询时间*并且*获得稳定、可重复的结果。它不是在评估查询时定期检查实际当前时间（这是不确定的），而是使用一个简单的线性模型来预测当前运行时间：

```ini
predicted_time =
    doc_cost * processed_documents +
    hit_cost * processed_hits +
    skip_cost * skiplist_jumps +
    match_cost * found_matches
```

当 `predicted_time` 达到给定限制时，查询会被提前终止。

当然，这并不是对实际花费时间的硬限制（但确实是对*处理*工作量的硬限制），而且简单的线性模型绝不是理想的精确模型。所以实时时钟时间*可能*低于或超过目标限制。然而，误差范围相当可以接受：例如，在我们以100毫秒为目标限制的实验中，大多数测试查询在95到105毫秒范围内，且*所有*查询都在80到120毫秒范围内。此外，作为一个额外的好处，使用建模的查询时间代替测量的实际运行时间，也减少了部分 gettimeofday() 调用。

没有两个服务器的制造商和型号是完全相同的，所以 `predicted_time_costs` 指令允许你配置上述模型的成本。为方便起见，它们是以纳秒为单位的整数。（max_predicted_time 中的限制以毫秒计，必须将成本值指定为 0.000128 毫秒而非128纳秒会更易出错。）不必一次指定所有四个成本，未指定的将采用默认值。但是，我们强烈建议全部指定以便于阅读。


### preopen_tables

<!-- example conf preopen_tables -->
preopen_tables 配置指令指定是否在启动时强制预打开所有表。默认值是1，意味着无论单表的 [preopen](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Other-performance-related-settings) 设置如何，都会预打开所有表。如果设置为0，则单表设置生效，默认为0。

预打开表可以防止搜索查询和轮换之间的竞争条件导致查询偶尔失败。然而，这也会使用更多的文件句柄。在大多数场景下，建议预打开表。

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
pseudo_sharding 配置选项开启对本地普通表和实时表搜索查询的并行化，无论它们是直接查询还是通过分布式表查询。此功能将自动将查询并行化至 `searchd.threads` 指定的线程数。

请注意，如果你的工作线程已经繁忙，因为你有：
* 高查询并发
* 任何形式的物理分片：
  - 多个普通/实时表组成的分布式表
  - 由过多磁盘块组成的实时表

那么启用 pseudo_sharding 可能不会带来任何好处，甚至可能导致吞吐量略有下降。如果你优先考虑更高的吞吐量而非更低的延迟，建议禁用此选项。

默认启用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
pseudo_sharding = 0
```
<!-- end -->


### replication_connect_timeout

`replication_connect_timeout` 指令定义连接远程节点的超时时间。默认单位为毫秒，但可以使用[其他后缀](../Server_settings/Special_suffixes.md)。默认值为1000（1秒）。

连接远程节点时，Manticore 最多等待此时间以成功建立连接。如果超时且连接未建立，且启用 `retries`，则会重试。


### replication_query_timeout

`replication_query_timeout` 设置 searchd 等待远程节点完成查询的时间。默认值是3000毫秒（3秒），也可以添加后缀指定不同时间单位。

建立连接后，Manticore 会等待最多 `replication_query_timeout` 时间以获得远程节点的结果。注意，该超时与 `replication_connect_timeout` 分开，总延迟可能为两者之和。


### replication_retry_count

此设置为整数，指定 Manticore 在复制期间尝试连接和查询远程节点的次数上限，超过则报告致命错误。默认值为3。


### replication_retry_delay

此设置为整数，单位为毫秒（或[特殊后缀](../Server_settings/Special_suffixes.md)），指定在复制失败时重试查询远程节点之前的延迟时间。当指定非零值时生效。默认值为500。

### qcache_max_bytes

<!-- example conf qcache_max_bytes -->
此配置设置用于缓存结果集的最大RAM量，单位为字节。默认值为16777216，即16兆字节。如果设置为0，则禁用查询缓存。更多查询缓存信息请参阅 [query cache](../Searching/Query_cache.md)。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
qcache_max_bytes = 16777216
```
<!-- end -->


### qcache_thresh_msec

整数，单位为毫秒。查询结果进入缓存的最小壁钟时间阈值。默认为3000，即3秒。0 表示缓存所有。详情请参阅[查询缓存](../Searching/Query_cache.md)。该值也可用时间[特殊后缀](../Server_settings/Special_suffixes.md)表示，但使用时需小心，避免和名称中含有 '_msec' 混淆。


### qcache_ttl_sec

整数，单位为秒。缓存结果集的过期时间。默认值为60秒，即1分钟。最小值为1秒。详情请参考[查询缓存](../Searching/Query_cache.md)。该值也可以用时间[特殊后缀](../Server_settings/Special_suffixes.md)表示，但使用时需小心，不要与包含“_sec”的值名混淆。


### query_log_format

<!-- example conf query_log_format -->
查询日志格式。可选，允许的值为`plain`和`sphinxql`，默认值为`sphinxql`。

`sphinxql`模式记录有效的SQL语句。`plain`模式以纯文本格式记录查询（主要适用于纯全文搜索场景）。该指令允许您在搜索服务器启动时切换两种格式。日志格式也可以动态修改，使用`SET GLOBAL query_log_format=sphinxql`语法。详情请参阅[查询日志](../Logging/Query_logging.md)。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_format = sphinxql
```
<!-- end -->

### query_log_min_msec

限制（毫秒），防止查询被写入查询日志。可选，默认值为0（所有查询均写入查询日志）。该指令指定仅执行时间超过指定限制的查询将被记录（该值也可用时间[特殊后缀](../Server_settings/Special_suffixes.md)表示，但使用时需小心，不要与包含`_msec`的值名混淆）。

### query_log

<!-- example conf query_log -->
查询日志文件名。可选，默认值为空（不记录查询）。所有搜索查询（如SELECT ...，但不包括INSERT/REPLACE/UPDATE查询）将记录在此文件中。格式详见[查询日志](../Logging/Query_logging.md)。采用“plain”格式时，可以使用“syslog”作为日志文件路径。在此情况下，所有搜索查询将使用`LOG_INFO`优先级发送到syslog守护进程，前缀为“[query]”而非时间戳。要使用syslog选项，构建Manticore时必须配置`-–with-syslog`。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log = /var/log/query.log
```
<!-- end -->


### query_log_mode

<!-- example conf query_log_mode -->
query_log_mode指令允许您为searchd和查询日志文件设置不同的权限。默认情况下，这些日志文件以600权限创建，意味着只有运行服务器的用户和root用户可以读取日志文件。
如果您想允许其他用户读取日志文件（例如，运行在非root用户下的监控解决方案），此指令非常有用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_mode  = 666
```
<!-- end -->

### read_buffer_docs

<!-- example conf read_buffer_docs -->
read_buffer_docs指令控制每个关键字用于文档列表读取缓冲区的大小。每个搜索查询中的每个关键字出现都会关联两个读取缓冲区：一个用于文档列表，一个用于命中列表。该设置让您控制文档列表缓冲区的大小。

较大的缓冲区可能增加每个查询的内存使用，但可能减少I/O时间。对于慢速存储，设置较大值是合理的，对于支持高IOPS的存储，应在较低值范围内进行实验。

默认值为256K，最小值为8K。您还可以为每个表设置[read_buffer_docs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs)，覆盖服务器配置级别的设置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_docs = 128K
```
<!-- end -->


### read_buffer_hits

<!-- example conf read_buffer_hits -->
read_buffer_hits指令指定搜索查询中每个关键字用于命中列表的读取缓冲区大小。默认大小为256K，最小值为8K。每个搜索查询的每个关键字都有两个关联的读取缓冲区，一个用于文档列表，一个用于命中列表。增加缓冲区大小可增加每个查询的内存使用，但减少I/O时间。对慢速存储，较大缓冲区更合理，对高IOPS存储，应在较低值范围进行实验。

该设置也可通过每个表级别的read_buffer_hits选项指定，详见[read_buffer_hits](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_hits)，可覆盖服务器级别的设置。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_hits = 128K
```
<!-- end -->

### read_unhinted

<!-- example conf read_unhinted -->
未提示的读取大小。可选，默认值为32K，最小为1K。

查询时，有些读取能够提前知道需要读取的数据量，但有些则不能，最明显的是命中列表大小目前无法预先知道。该设置允许您控制在此类情况下读取的数据量。它影响命中列表的I/O时间，对大于未提示读取大小的列表可以降低时间，但会增加较小列表的时间。它**不影响**内存使用，因为读取缓冲区已经分配。因此，不应大于read_buffer。


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

设置为0时，超时限制整个请求/查询的最大发送时间。
设置为1（默认）时，超时限制网络活动之间的最大时间。

通过复制，一个节点可能需要将一个大文件（例如100GB）发送到另一个节点。假设网络传输速率为1GB/s，使用一系列4-5MB的包。传输整个文件需要100秒。默认的5秒超时只允许传输5GB数据，之后连接将被断开。增加超时是一个解决方法，但不具备可扩展性（例如，下一个文件可能是150GB，仍会导致失败）。然而，默认情况下`reset_network_timeout_on_packet`设置为1，超时应用于单个数据包，而不是整个传输。只要传输在进行中（且在超时时间内实际上通过网络接收数据），连接就会保持活跃。如果传输卡住，导致数据包间发生超时，则连接会被断开。

请注意，如果设置了分布式表，主节点和代理节点都应进行调整。主节点影响的是`agent_query_timeout`；代理节点影响的是`network_timeout`。

<!-- intro -->

##### 示例：

<!-- request Example -->

```ini
reset_network_timeout_on_packet = 0
```

<!-- end -->


### rt_flush_period

<!-- example conf rt_flush_period -->
RT表内存块刷新检查周期，单位为秒（或[特殊后缀](../Server_settings/Special_suffixes.md)）。可选，默认值为10小时。

完全适合内存块的动态更新RT表仍可能导致不断增长的二进制日志，影响磁盘使用和崩溃恢复时间。通过此指令，搜索服务器会周期性执行刷新检查，可保存符合条件的内存块，从而实现随后的二进制日志清理。详情参见[二进制日志](../Logging/Binary_logging.md)。

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

此指令允许你限制来自`OPTIMIZE`语句的I/O影响。保障所有RT优化活动生成的磁盘IOPS（每秒I/O次数）不会超过配置的限制。限制rt_merge_iops可以减少合并引起的搜索性能下降。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_iops = 40
```
<!-- end -->

### rt_merge_maxiosize

<!-- example conf rt_merge_maxiosize -->
RT块合并线程允许启动的单次最大I/O操作大小。可选，默认值为0（无限制）。

此指令允许你限制来自`OPTIMIZE`语句的I/O影响。大小超过此限制的I/O操作会被拆分成两个或多个I/O，并作为独立的操作计入[rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops)限制。因此，保证所有优化活动每秒生成的磁盘I/O不超过(rt_merge_iops * rt_merge_maxiosize) 字节。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_maxiosize = 1M
```
<!-- end -->


### seamless_rotate

<!-- example conf seamless_rotate -->
防止在旋转包含大量数据以预缓存的表时`searchd`卡顿。可选，默认值为1（启用无缝旋转）。在Windows系统上，无缝旋转默认关闭。

表中可能包含需要预缓存到内存的数据。目前，`.spa`、`.spb`、`.spi`和`.spm`文件将被完全预缓存（它们分别包含属性数据、blob属性数据、关键词表和已删除行映射）。未启用无缝旋转时，旋转表会尽量少用内存，过程如下：

1. 新查询暂时被拒绝（错误码“重试”）；
2. `searchd`等待所有当前运行的查询完成；
3. 旧表释放，文件重命名；
4. 新表文件重命名，分配所需内存；
5. 新表属性和词典数据预加载到内存；
6. `searchd`开始为新表服务查询。

但是如果属性或词典数据过大，预加载步骤可能需要较长时间——在预加载1-5GB及以上文件时可能耗时几分钟。

启用无缝旋转后，旋转过程如下：

1. 分配新表的内存存储；
2. 异步预加载新表属性和词典数据到内存；
3. 预加载成功后，释放旧表，同时重命名两个表的文件；
4. 预加载失败时，释放新表；
5. 任何时刻，查询从旧表或新表副本中提供服务。

无缝旋转导致旋转期间峰值内存使用增加（因为预加载期间需同时将旧表和新表的`.spa/.spb/.spi/.spm`数据保留在内存中），但平均内存使用保持不变。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
seamless_rotate = 1
```
<!-- end -->

### secondary_index_block_cache
<!-- example conf secondary_index_block_cache -->

此选项指定二级索引使用的块缓存大小。可选，默认值为8MB。当二级索引处理包含大量值的过滤器（例如IN()过滤器）时，它们会读取并处理这些值的元数据块。
在连接查询中，此过程对左表的每批行重复进行，每批可能在单次连接查询内重新读取相同的元数据。这会严重影响性能。元数据块缓存将这些块保存在内存中，以供后续批次重复使用。
该缓存仅在连接查询中使用，对非连接查询无影响。缓存大小限制是针对每个属性和每个二级索引的。每个磁盘块中的每个属性都在此限制范围内运行。最坏情况下，总内存使用量可估算为限制大小乘以磁盘块数量再乘以连接查询中使用的属性数量。

设置`secondary_index_block_cache = 0`可禁用缓存。
##### 示例：

secondary_index_block_cache = 16M

<!-- intro -->
### secondary_indexes

<!-- request Example -->

```ini
This option enables/disables the use of secondary indexes for search queries. It is optional, and the default is 1 (enabled). Note that you don't need to enable it for indexing as it is always enabled as long as the [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) is installed. The latter is also required for using the indexes when searching. There are three modes available:
```

<!-- end -->

* `0`: Disable the use of secondary indexes on search. They can be enabled for individual queries using [analyzer hints](../Searching/Options.md#Query-optimizer-hints)
<!-- example conf secondary_indexes -->

* `1`: Enable the use of secondary indexes on search. They can be disabled for individual queries using [analyzer hints](../Searching/Options.md#Query-optimizer-hints)

* `force`: Same as enable, but any errors during the loading of secondary indexes will be reported, and the whole index will not be loaded into the daemon.
##### Example:
secondary_indexes = 1

<!-- intro -->
### server_id

<!-- request Example -->

```ini
Integer number that serves as a server identifier used as a seed to generate a unique short UUID for nodes that are part of a replication cluster. The server_id must be unique across the nodes of a cluster and in the range from 0 to 127. If server_id is not set, it is calculated as a hash of the MAC address and the path to the PID file or a random number will be used as a seed for the short UUID.
```

<!-- end -->

##### Example:

<!-- example conf server_id -->
server_id = 1


<!-- intro -->
### shutdown_timeout

<!-- request Example -->

```ini
`searchd --stopwait` waiting time, in seconds (or [special_suffixes](../Server_settings/Special_suffixes.md)). Optional, default is 60 seconds.
```
<!-- end -->


When you run `searchd --stopwait` your server needs to perform some activities before stopping, such as finishing queries, flushing RT RAM chunks, flushing attributes, and updating the binlog. These tasks require some time. `searchd --stopwait` will wait up to `shutdown_time` seconds for the server to finish its jobs. The suitable time depends on your table size and load.

<!-- example conf shutdown_timeout -->
##### Example:

shutdown_timeout = 3m # wait for up to 3 minutes


<!-- intro -->
### shutdown_token

<!-- request Example -->

```ini
SHA1 hash of the password required to invoke the 'shutdown' command from a VIP Manticore SQL connection. Without it,[debug](../Reporting_bugs.md#DEBUG) 'shutdown' subcommand will never cause the server to stop. Note that such simple hashing should not be considered strong protection, as we don't use a salted hash or any kind of modern hash function. It is intended as a fool-proof measure for housekeeping daemons in a local network.
```
<!-- end -->


### snippets_file_prefix

A prefix to prepend to the local file names when generating snippets. Optional, default is the current working folder.

This prefix can be used in distributed snippets generation along with `load_files` or `load_files_scattered` options.

<!-- example conf snippets_file_prefix -->
Note that this is a prefix and **not** a path! This means that if a prefix is set to "server1" and the request refers to "file23", `searchd` will attempt to open "server1file23" (all of that without quotes). So, if you need it to be a path, you have to include the trailing slash.

After constructing the final file path, the server unwinds all relative dirs and compares the final result with the value of `snippet_file_prefix`. If the result does not begin with the prefix, such a file will be rejected with an error message.

For example, if you set it to `/mnt/data` and someone calls snippet generation with the file `../../../etc/passwd` as the source, they will get the error message:

`File '/mnt/data/../../../etc/passwd' escapes '/mnt/data/' scope`

instead of the content of the file.

Also, with a non-set parameter and reading `/etc/passwd`, it will actually read /daemon/working/folder/etc/passwd since the default for the parameter is the server's working folder.

Note also that this is a local option; it does not affect the agents in any way. So you can safely set a prefix on a master server. The requests routed to the agents will not be affected by the master's setting. They will, however, be affected by the agent's own settings.

This might be useful, for instance, when the document storage locations (whether local storage or NAS mountpoints) are inconsistent across the servers.

##### Example:

snippets_file_prefix = /mnt/common/server1/


<!-- intro -->
> **WARNING:** If you still want to access files from the FS root, you have to explicitly set `snippets_file_prefix` to empty value (by `snippets_file_prefix=` line), or to root (by `snippets_file_prefix=/`).

<!-- request Example -->

```ini
### sphinxql_state
```
<!-- end -->

Path to a file where the current SQL state will be serialized.


On server startup, this file gets replayed. On eligible state changes (e.g., SET GLOBAL), this file gets rewritten automatically. This can prevent a hard-to-diagnose problem: If you load UDF functions but Manticore crashes, when it gets (automatically) restarted, your UDF and global variables will no longer be available. Using persistent state helps ensure a graceful recovery with no such surprises.

<!-- example conf sphinxql_state -->
`sphinxql_state` cannot be used to execute arbitrary commands, such as `CREATE TABLE`.

##### Example:

sphinxql_state = uservars.sql

<!-- intro -->
### sphinxql_timeout

<!-- request Example -->

```ini
Maximum time to wait between requests (in seconds, or [special_suffixes](../Server_settings/Special_suffixes.md)) when using the SQL interface. Optional, default is 15 minutes.
```
<!-- end -->


##### Example:

<!-- example conf sphinxql_timeout -->
sphinxql_timeout = 15m


<!-- intro -->
### ssl_ca

<!-- request Example -->

```ini
Path to the SSL Certificate Authority (CA) certificate file (also known as root certificate). Optional, default is empty. When not empty, the certificate in `ssl_cert` should be signed by this root certificate.
```
<!-- end -->


服务器使用 CA 文件来验证证书上的签名。该文件必须是 PEM 格式。

<!-- example conf ssl_ca -->
##### 示例：

ssl_ca = keys/ca-cert.pem

<!-- intro -->
### ssl_cert

<!-- request Example -->

```ini
服务器 SSL 证书的路径。可选，默认值为空。
```
<!-- end -->


服务器使用此证书作为自签名公钥，通过 SSL 加密 HTTP 流量。该文件必须是 PEM 格式。

<!-- example conf ssl_cert -->
##### 示例：

ssl_cert = keys/server-cert.pem


<!-- intro -->
### ssl_key

<!-- request Example -->

```ini
SSL 证书密钥的路径。可选，默认值为空。
```
<!-- end -->


服务器使用此私钥通过 SSL 加密 HTTP 流量。该文件必须是 PEM 格式。

<!-- example conf ssl_key -->
##### 示例：

ssl_key = keys/server-key.pem


<!-- intro -->
### subtree_docs_cache

<!-- request Example -->

```ini
每查询最大公共子树文档缓存大小。可选，默认值为 0（禁用）。
```
<!-- end -->


此设置限制公共子树优化器的 RAM 使用（参见 [multi-queries](../Searching/Multi-queries.md)）。每个查询最多消耗这么多 RAM 用于缓存文档条目。将此限制设置为 0 会禁用该优化器。

<!-- example conf subtree_docs_cache -->
##### 示例：

subtree_docs_cache = 8M


<!-- intro -->
### subtree_hits_cache

<!-- request Example -->

```ini
每查询最大公共子树命中缓存大小。可选，默认值为 0（禁用）。
```
<!-- end -->


此设置限制公共子树优化器的 RAM 使用（参见 [multi-queries](../Searching/Multi-queries.md)）。每个查询最多消耗这么多 RAM 用于缓存关键字出现位置（命中）。将此限制设置为 0 会禁用该优化器。

<!-- example conf subtree_hits_cache -->
##### 示例：

subtree_hits_cache = 16M


<!-- intro -->
### threads

<!-- request Example -->

```ini
Manticore 守护进程的工作线程数（或线程池大小）。Manticore 启动时会创建这么多操作系统线程，它们执行守护进程内的所有任务，例如执行查询、生成摘要等。有些操作可以拆分为子任务并行执行，例如：
```
<!-- end -->

* 在实时表中搜索

<!-- example threads -->
* 在由本地表组成的分布式表中搜索

* 继续调用 Percolate 查询
* 以及其他操作
默认情况下，线程数设置为服务器 CPU 核心数。Manticore 启动时创建线程并保持直到停止。每个子任务需要时可以使用一个线程，子任务完成后释放线程供其他子任务使用。
在高强度 I/O 负载情况下，将此值设置高于 CPU 核心数可能更合理。

threads = 10

### thread_stack

<!-- request Example -->
```ini
作业的最大栈大小（协程，一个搜索查询可能产生多个作业/协程）。可选，默认值为 128K。
```

<!-- end -->

每个作业拥有其自己的 128K 栈。运行查询时，会判断其所需栈大小。如果默认的 128K 足够，则直接处理；如果需要更大栈，则调度另一个具有增大栈的作业继续处理。此类扩展栈的最大大小受此设置限制。

<!-- example conf thread_stack -->
合理设置较大的此值可以帮助处理非常深层的查询，而不至于整体 RAM 占用大幅增加。例如，设置为 1G 不意味着每个新作业都占用 1G RAM，而是如果某作业需要 100M 栈，就只为该作业分配 100M。其他作业同时运行时仍使用默认 128K 栈。同理，能处理更复杂需 500M 栈的查询。只有当内部检测到作业需求超过 1G 栈时，才会失败并报告 thread_stack 过小。

实际上，需要 16M 栈的查询通常过于复杂，解析时耗费大量时间和资源。守护进程会处理该查询，但通过 `thread_stack` 设置限制此类查询是合理的。

##### 示例：

thread_stack = 8M


<!-- intro -->
### unlink_old

<!-- request Example -->

```ini
确定成功旋转后是否解除 `.old` 表复制的链接。可选，默认值为 1（解除链接）。
```
<!-- end -->


##### 示例：

<!-- example conf unlink_old -->
unlink_old = 0


<!-- intro -->
### watchdog

<!-- request Example -->

```ini
线程化服务器看门狗。可选，默认值为 1（启用看门狗）。
```
<!-- end -->


当 Manticore 查询崩溃时，可能导致整个服务器宕机。启用看门狗功能后，`searchd` 会维护一个轻量级独立进程，监控主服务器进程并在异常终止时自动重启服务器。看门狗默认启用。

<!-- example conf watchdog -->
watchdog = 0 # disable watchdog

When a Manticore query crashes, it can take down the entire server. With the watchdog feature enabled, `searchd` also maintains a separate lightweight process that monitors the main server process and automatically restarts it in case of abnormal termination. The watchdog is enabled by default.

<!-- request Example -->

```ini
watchdog = 0 # disable watchdog
```
<!-- end -->
<!-- proofread -->
