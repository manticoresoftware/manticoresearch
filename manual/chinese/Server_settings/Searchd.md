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

二进制日志用于 RT 表数据的崩溃恢复，以及普通磁盘索引的属性更新，这些属性更新否则将仅存储在 RAM 中直到刷新。当启用日志记录时，每个提交到 RT 表的事务都会被写入日志文件。在不干净关闭后启动时，日志会自动重放，恢复记录的更改。

`binlog_path` 指令指定二进制日志文件的位置。它应仅包含路径；`searchd` 会根据需要在目录中创建和删除多个 `binlog.*` 文件（包括二进制日志数据、元数据和锁文件等）。

空值会禁用二进制日志记录，这会提高性能但会使 RT 表数据面临风险。


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
此设置控制 [boolean_simplify](../Searching/Options.md#boolean_simplify) 搜索选项的默认值。它是可选的，默认值为 1（启用）。

当设置为 1 时，服务器将自动应用 [布尔查询优化](../Searching/Full_text_matching/Boolean_optimization.md) 以提高查询性能。当设置为 0 时，默认情况下查询将不进行优化。此默认值可以通过使用相应的搜索选项 `boolean_simplify` 在每个查询的基础上覆盖。

<!-- request Example -->
```ini
searchd {
    boolean_simplify = 0  # disable boolean optimization by default
}
```
<!-- end -->

### buddy_path

<!-- example conf buddy_path -->
此设置确定 Manticore Buddy 二进制文件的路径。它是可选的，默认值是构建时配置的路径，这在不同操作系统上会有所不同。通常，您不需要修改此设置。但是，如果您希望在调试模式下运行 Manticore Buddy、对 Manticore Buddy 进行更改或实现新插件，这可能会很有用。在后一种情况下，您可以从 https://github.com/manticoresoftware/manticoresearch-buddy 克隆 Buddy，将新插件添加到目录 `./plugins/` 中，并在更改目录到 Buddy 源代码后运行 `composer install --prefer-source`，以便更轻松地进行开发。

为了确保您可以运行 `composer`，您的机器必须安装了 PHP 8.2 或更高版本，并包含以下扩展：

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

您也可以选择使用 Linux amd64 平台上的特殊 `manticore-executor-dev` 版本，例如：https://github.com/manticoresoftware/executor/releases/tag/v1.0.13

如果您选择此方法，请记住将 manticore executor 的开发版本链接到 `/usr/bin/php`。

要禁用 Manticore Buddy，请将值设置为空，如示例所示。

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
此设置确定在使用持久连接时等待请求之间的最大时间（以秒或 [special_suffixes](../Server_settings/Special_suffixes.md) 为单位）。它是可选的，默认值为五分钟。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
client_timeout = 1h
```
<!-- end -->


### collation_libc_locale

<!-- example conf collation_libc_locale -->
服务器 libc 本地化设置。可选，默认为 C。

指定 libc 本地化设置，影响基于 libc 的排序规则。有关详细信息，请参阅 [排序规则](../Searching/Collations.md) 部分。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
collation_libc_locale = fr_FR
```
<!-- end -->


### collation_server

<!-- example conf collation_server -->
默认服务器排序规则。可选，默认为 libc_ci。

指定用于传入请求的默认排序规则。可以在每个查询的基础上覆盖排序规则。有关可用排序规则和其他详细信息的列表，请参阅 [排序规则](../Searching/Collations.md) 部分。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
collation_server = utf8_ci
```
<!-- end -->


### data_dir

<!-- example conf data_dir -->
当指定此设置时，它会启用 [实时模式](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)，这是一种管理数据模式的强制方法。值应为存储所有表、二进制日志以及此模式下 Manticore Search 正常运行所需所有其他内容的目录路径。
当指定 `data_dir` 时，不允许对 [普通表](../Creating_a_table/Local_tables/Plain_table.md) 进行索引。有关 RT 模式和普通模式之间差异的更多信息，请参阅 [此部分](../Read_this_first.md#Real-time-table-vs-plain-table)。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
data_dir = /var/lib/manticore
```
<!-- end -->

### attr_autoconv_strict

<!-- example conf attr_autoconv_strict -->
此设置控制在 INSERT 和 REPLACE 操作期间字符串到数字类型转换的严格验证模式。可选，默认为 0（非严格模式，向后兼容）。

当设置为 1（严格模式）时，无效的字符串到数字转换（例如，将空字符串 `''` 或非数字字符串 `'a'` 转换为 bigint 属性）将返回错误，而不是静默转换为 0。这有助于在数据插入期间尽早发现数据质量问题。

当设置为 0（非严格模式，默认）时，无效的转换将静默转换为 0，保持与旧版本的向后兼容性。

严格模式验证以下情况：
* 空字符串或无法转换的字符串
* 以非数字字符结尾的字符串（例如，`'123abc'`）
* 超出类型范围的数值（溢出/下溢）

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
attr_autoconv_strict = 1  # enable strict conversion mode
```
<!-- end -->

### diskchunk_flush_search_timeout

<!-- example conf diskchunk_flush_search_timeout -->
防止在表中没有搜索时自动刷新 RAM 块的超时时间。可选，默认为 30 秒。

检查在确定是否自动刷新之前进行搜索的时间。  
只有在表中最近 `diskchunk_flush_search_timeout` 秒内至少有一次搜索时，才会发生自动刷新。与 [diskchunk_flush_write_timeout](../Server_settings/Searchd.md#diskchunk_flush_write_timeout) 配合使用。对应的 [每表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_search_timeout) 优先级更高，将覆盖此全局默认值，提供更细粒度的控制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
diskchunk_flush_search_timeout = 120s
```
<!-- end -->

### diskchunk_flush_write_timeout

<!-- example conf diskchunk_flush_write_timeout -->
等待没有写入操作后自动将 RAM 块刷新到磁盘的秒数。可选，默认值为 1 秒。

如果在 `diskchunk_flush_write_timeout` 秒内 RAM 块中没有写入操作，该块将被刷新到磁盘。与 [diskchunk_flush_search_timeout](../Server_settings/Searchd.md#diskchunk_flush_search_timeout) 配合使用。要禁用自动刷新，请在配置中显式设置 `diskchunk_flush_write_timeout = -1`。对应的 [每表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_write_timeout) 优先级更高，将覆盖此全局默认值，提供更细粒度的控制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
diskchunk_flush_write_timeout = 60s
```
<!-- end -->

### docstore_cache_size

<!-- example conf docstore_cache_size -->
此设置指定从文档存储中保留在内存中的文档块的最大大小。可选，默认值为 16m（16 兆字节）。

当使用 `stored_fields` 时，文档块会从磁盘读取并解压缩。由于每个块通常包含多个文档，处理下一个文档时可能会重复使用该块。为此，块会保留在一个全局缓存中。缓存保存的是未压缩的块。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
docstore_cache_size = 8m
```
<!-- end -->

### engine

<!-- example conf engine -->
在 RT 模式下创建表时默认使用的属性存储引擎。可以是 `rowwise`（默认）或 `columnar`。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
engine = columnar
```
<!-- end -->


### expansion_limit

<!-- example conf expansion_limit -->
此设置确定单个通配符的扩展关键字的最大数量。可选，默认值为 0（无限制）。

当对使用 `dict = keywords` 启用的表执行子字符串搜索时，单个通配符可能会导致匹配数千甚至数百万个关键字（例如，将 `a*` 与整个牛津词典匹配）。此指令允许您限制此类扩展的影响。设置 `expansion_limit = N` 会将扩展限制为每个查询中的通配符最多匹配 N 个最常用的关键字。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_limit = 16
```
<!-- end -->

### expansion_merge_threshold_docs

<!-- example conf expansion_merge_threshold_docs -->
此设置确定扩展关键字中允许合并所有此类关键字的最大文档数。可选，默认值为 32。

当对使用 `dict = keywords` 启用的表执行子字符串搜索时，单个通配符可能会导致匹配数千甚至数百万个关键字。此指令允许您增加合并的关键字数量限制以加快匹配速度，但会增加搜索时的内存使用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_merge_threshold_docs = 1024
```
<!-- end -->

### expansion_merge_threshold_hits

<!-- example conf expansion_merge_threshold_hits -->
此设置确定扩展关键字中允许合并所有此类关键字的最大命中数。可选，默认值为 256。

当对使用 `dict = keywords` 启用的表执行子字符串搜索时，单个通配符可能会导致匹配数千甚至数百万个关键字。此指令允许您增加合并的关键字数量限制以加快匹配速度，但会增加搜索时的内存使用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_merge_threshold_hits = 512
```
<!-- end -->

### expansion_phrase_limit

<!-- example conf expansion_phrase_limit -->
此设置控制由于 `PHRASE`、`PROXIMITY` 和 `QUORUM` 操作符内部的 `OR` 操作符而生成的替代短语变体的最大数量。可选，默认值为 1024。

在短语类操作符中使用 `|`（OR）操作符时，根据指定的替代项数量，扩展组合的总数可能会呈指数级增长。此设置通过限制查询处理过程中考虑的排列数来防止过度的查询扩展。

如果生成的变体数量超过此限制，查询将：

- 默认情况下以错误失败  
- 如果启用了 `expansion_phrase_warning`，则返回部分结果并附带警告

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_phrase_limit = 4096
```
<!-- end -->

### expansion_phrase_warning

<!-- example conf expansion_phrase_warning -->
此设置控制当 `expansion_phrase_limit` 定义的查询扩展限制被超过时的行为。

默认情况下，查询将以错误消息失败。当 `expansion_phrase_warning` 设置为 1 时，搜索将继续使用短语的部分转换（最多达到配置的限制），并返回警告消息给用户，同时附带结果集。这允许过于复杂无法完全扩展的查询仍能返回部分结果，而不会完全失败。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_phrase_warning = 1
```
<!-- end -->

### grouping_in_utc

此设置指定在 API 和 SQL 中的定时分组是使用本地时区还是 UTC 进行计算。它是可选的，默认值为 0（表示“本地时区”）。

默认情况下，所有“按时间分组”的表达式（如 API 中的 group by day、week、month、year，以及 SQL 中的 group by day、month、year、yearmonth、yearmonthday）均使用本地时间进行计算。例如，如果您有属性时间为 `13:00 utc` 和 `15:00 utc` 的文档，在分组的情况下，它们将根据您的本地时区设置归入设施组。如果您位于 `utc`，则为同一天，但如果您位于 `utc+10`，则这些文档将被匹配到不同的 `group by day` 设施组（因为 UTC+10 时区的 13:00 utc 是 23:00 本地时间，而 15:00 是第二天的 01:00）。有时这种行为是不可接受的，希望时间分组不依赖于时区。您可以使用定义的全局 TZ 环境变量运行服务器，但这会影响分组以外的其他功能，例如日志中的时间戳，这可能也不理想。将此选项切换为“开启”（在配置中或使用 SQL 中的 [SET global](../Server_settings/Setting_variables_online.md#SET) 语句）将导致所有时间分组表达式以 UTC 计算，而其余时间相关函数（即服务器日志）则使用本地时区。


### timezone

此设置指定与日期/时间相关的函数使用的时区。默认情况下使用本地时区，但您可以使用 IANA 格式（例如 `Europe/Amsterdam`）指定不同的时区。

请注意，此设置对日志没有影响，日志始终以本地时区运行。

另外请注意，如果使用了 `grouping_in_utc`，则“按时间分组”功能仍将使用 UTC，而其他日期/时间相关函数将使用指定的时区。总体而言，不建议将 `grouping_in_utc` 和 `timezone` 混合使用。

您可以在配置中或通过在 SQL 中使用 [SET global](../Server_settings/Setting_variables_online.md#SET) 语句配置此选项。


### ha_period_karma

<!-- example conf ha_period_karma -->
此设置指定代理镜像统计窗口大小，以秒为单位（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。它是可选的，默认值为 60 秒。

对于包含代理镜像的分布式表（更多信息请参见 [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)），主节点会跟踪每个镜像的多个不同计数器。这些计数器随后用于故障转移和平衡（主节点根据计数器选择最佳镜像使用）。计数器以 `ha_period_karma` 秒为块进行累积。

在开始新的块后，主节点仍可能使用前一个块的累积值，直到新块填充一半。因此，任何之前的记录最多在 1.5 倍 ha_period_karma 秒后将不再影响镜像选择。

尽管最多使用两个块进行镜像选择，但最多存储 15 个最近的块用于仪器目的。这些块可以通过 [SHOW AGENT STATUS](../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS) 语句进行检查。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_period_karma = 2m
```
<!-- end -->


### ha_ping_interval

<!-- example conf ha_ping_interval -->
此设置配置代理镜像 ping 间隔，以毫秒为单位（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。它是可选的，默认值为 1000 毫秒。

对于包含代理镜像的分布式表（更多信息请参见 [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)），主节点在空闲期间会向所有镜像发送 ping 命令。这是为了跟踪当前代理状态（存活或死亡、网络往返时间等）。此类 ping 的间隔由此指令定义。要禁用 ping，请将 ha_ping_interval 设置为 0。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_ping_interval = 3s
```
<!-- end -->


### hostname_lookup

`hostname_lookup` 选项定义了更新主机名的策略。默认情况下，服务器启动时会缓存代理主机名的 IP 地址，以避免对 DNS 的过度访问。然而，在某些情况下，IP 可能会动态更改（例如云托管），此时可能不希望缓存 IP。将此选项设置为 `request` 会禁用缓存，并对每个查询查询 DNS。也可以使用 `FLUSH HOSTNAMES` 命令手动更新 IP 地址。

### jobs_queue_size

jobs_queue_size 设置定义了队列中可以同时存在的“任务”数量。默认情况下是无限的。

在大多数情况下，“任务”意味着对单个本地表（普通表或实时表的磁盘块）的一个查询。例如，如果您有一个由 2 个本地表组成的分布式表或一个包含 2 个磁盘块的实时表，对其中任何一个的搜索查询通常会在队列中放入 2 个任务。然后，线程池（其大小由 [threads](../Server_settings/Searchd.md#threads) 定义）会处理它们。然而，在某些情况下，如果查询过于复杂，可能会生成更多任务。当 [max_connections](../Server_settings/Searchd.md#max_connections) 和 [threads](../Server_settings/Searchd.md#threads) 不足以在所需性能之间找到平衡时，建议更改此设置。

### join_batch_size

表连接通过累积一批匹配项来工作，这些匹配项是左表上执行的查询结果。然后，该批处理作为右表上的单个查询进行处理。

此选项允许您调整批处理大小。默认值为 `1000`，将此选项设置为 `0` 会禁用批处理。

较大的批处理大小可能会提高性能；然而，对于某些查询，它可能导致内存消耗过多。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
join_batch_size = 2000
```
<!-- end -->

### join_cache_size

每个查询执行在右侧表格上，由特定的 JOIN ON 条件定义，这些条件决定了从右侧表格中检索的最终结果集。

如果仅有少数唯一的 JOIN ON 条件，重复执行右侧表格查询可以比多次从右侧表格中执行查询更高效。为了实现这一点，结果集被存储在缓存中。

该选项允许您配置缓存的大小。默认值为 20 MB，设置该选项为 0 就会关闭缓存。

请注意，每个线程都维护自己的缓存，因此在估算总内存使用量时，应考虑正在执行查询的线程数量。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
join_cache_size = 10M
```
<!-- end -->

### listen_backlog

<!-- example conf listen_backlog -->
该设置决定了 TCP 向来的监听队列长度。对于 Windows 构建，这在处理请求逐个处理时尤为重要。当连接队列达到其限制时，新的连接将被拒绝。
对于非 Windows 构建，该设置的默认值通常可以正常工作，通常不需要进行调整。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
listen_backlog = 20
```
<!-- end -->

### kibana_version_string

<!-- example conf kibana_version_string -->
一个服务器版本字符串，用于返回 Kibana 或 OpenSearch Dashboards。可选——默认值为 `7.6.0`。

一些 Kibana 和 OpenSearch Dashboards 版本要求服务器报告特定的版本号，这可能会根据版本号的不同而行为不同。为解决此类问题，可以使用该设置，该设置使 Manticore 报告一个自定义版本到 Kibana 或 OpenSearch Dashboards。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
kibana_version_string = 1.2.3
```
<!-- end -->

### kill_dictionary

<!-- example conf kill_dictionary -->
控制 RT 盘块的 kill 字典。当启用时，Manticore 从 docstore 中重新计算每块删除的文档的词统计，并在查询时减去这些统计，因此 `docs` 和 `hits` 和排名权重会反映已删除的行。该功能默认启用在 `idle` 模式下。

模式：
- `0`：禁用，查询使用原始块字典统计。
- `realtime`：启用，且每次删除都会立即应用（每块重新编码）。
- `flush`：启用，但每块更新在下次 flush/合并重建时被推迟。
- `idle`：启用，但每块更新在写入流空闲时间（通过 `kill_dictionary_idle_timeout`）后才被重建。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
kill_dictionary = flush
```
<!-- end -->

### kill_dictionary_idle_timeout

<!-- example conf kill_dictionary_idle_timeout -->
在 `kill_dictionary=idle` 模式下，当有 pending kills 且没有发生写入 1 分钟内时，Manticore 重建 kill 字典并保存 `.spks` 文件。设置为 `-1` 就会关闭 idle 建模。接受秒或 `ms/s/m/h/d` 的单位。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
kill_dictionary_idle_timeout = 15s
```
<!-- end -->

### listen

<!-- example conf listen -->
该设置允许您指定一个 IP 地址和端口，或 Unix 域名路径，以指定 Manticore 接收连接的地址和端口。

一般语法为：

```ini
listen = ( address ":" port | port | path | address ":" port start - port end ) [ ":" protocol [ "_vip" ] [ "_readonly" ] ]
```

您可以在以下方式指定：
* 一个 IP 地址和端口
* 仅一个端口
* 一个 Unix 域名路径（不支持在 Windows）
* 一个 IP 地址和端口范围

如果指定一个端口但未指定地址，`searchd` 将监听所有网络接口。Unix 路径以 `/` 开头。端口范围只能在复制协议上使用。

您还可以指定一个协议处理程序（监听器）用于该 socket 的连接。监听器包括：

* 未指定：Manticore 将从指定的端口接受来自：
  - 其他 Manticore 监听器（即远程分布式表）
  - 通过 HTTP 和 HTTPS 接收的客户端
  - [Manticore Buddy](https://manticoresearch.com/blog/manticoresearch-buddy-intro/)。请确保您有该监听器（或一个 `http` 监听器，如在下文所述）以避免 Manticore 功能的限制。
* `mysql` MySQL 协议用于从 MySQL 客户端接收连接。注意：
  - 压缩协议也支持。
  - 如果 [SSL](../Security/SSL.md#SSL) 已启用，您可以使用加密连接。
* `replication` - 复制协议用于节点通信。更多细节可在 [复制](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md) 部分找到。您可以在该设置中指定多个复制监听器，但它们必须监听相同的 IP；仅允许不同的端口。当定义一个复制监听器时，指定端口范围（例如 `listen = 192.168.0.1:9320-9328:replication`），Manticore 不立即开始监听这些端口。而是在启动复制时，它会从指定范围中随机选择可用的端口。至少需要 2 个端口 以确保复制正常工作。
* `http` - 类似于未指定的模式。Manticore 将从指定的端口接受来自远程代理和 HTTP/HTTPS 客户端。
* `https` - HTTPS 协议。Manticore 将仅接受 HTTPS 连接。更多细节可在 [SSL](../Security/SSL.md) 部分找到。
* `sphinx` - 旧的二进制协议。用于从 SphinxSE 客户端接收连接。一些 Sphinx API 客户端 实现（例如 Java 一个）需要显式声明监听器。

添加 `_vip` 后缀（即所有除了 `replication` 的协议，例如 `mysql_vip` 或 `http_vip` 或仅 `_vip`）会强制创建一个专用线程以避免不同限制。这对于在服务器负载严重时，要么卡顿，要么无法通过常规端口连接的情况非常有用。

添加 `_readonly` 后缀（即所有除了 `replication` 的协议，例如 `mysql_readonly` 或 `http_readonly` 或仅 `_readonly`）会设置监听器为只接受读查询。

<!-- intro -->
##### Example:

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

可以有多个 `listen` 指令。`searchd` 会在所有指定的端口和套接字上监听客户端连接。Manticore 软件包中提供的默认配置定义了监听端口：
* `9308` 和 `9312` 用于远程代理和非 MySQL 基础客户端的连接
* 以及端口 `9306` 用于 MySQL 连接。

如果你在配置中完全不指定任何 `listen`，Manticore 将等待连接：
* `127.0.0.1:9306` 用于 MySQL 客户端
* `127.0.0.1:9312` 用于 HTTP/HTTPS 和来自其他 Manticore 节点及基于 Manticore 二进制 API 的客户端的连接。

#### 在特权端口上监听

默认情况下，Linux 不允许你让 Manticore 监听低于 1024 的端口（例如 `listen = 127.0.0.1:80:http` 或 `listen = 127.0.0.1:443:https`），除非你在 root 用户下运行 searchd。如果你仍然希望能够在非 root 用户下启动 Manticore，使其监听低于 1024 的端口，可以考虑以下方法（以下任一方法都应有效）：
* 运行命令 `setcap CAP_NET_BIND_SERVICE=+eip /usr/bin/searchd`
* 在 Manticore 的 systemd 单元中添加 `AmbientCapabilities=CAP_NET_BIND_SERVICE` 并重新加载守护进程（`systemctl daemon-reload`）。

#### 关于 Sphinx API 协议和 TFO 的技术细节
<details>
旧版 Sphinx 协议有 2 个阶段：握手交换和数据流。握手包括客户端发送的 4 字节数据包，以及守护进程发送的 4 字节数据包，其唯一目的是让客户端确定远程端是真正的 Sphinx 守护进程，守护进程确定远程端是真正的 Sphinx 客户端。主要数据流相当简单：让双方声明各自的握手，并由对方检查。这种短数据包的交换意味着使用特殊的 `TCP_NODELAY` 标志，该标志关闭 Nagle 的 TCP 算法，并声明 TCP 连接将作为小数据包的对话进行。
然而，这种协商中谁先发言并没有严格定义。历史上，所有使用二进制 API 的客户端都先发言：发送握手，然后从守护进程读取 4 字节，然后发送请求并从守护进程读取响应。
当我们改进 Sphinx 协议兼容性时，我们考虑了以下几点：

1. 通常，主代理通信是从已知客户端到已知主机的已知端口建立的。因此，端点提供错误握手的可能性非常小。因此，我们可以隐式假设双方都是有效的，并且确实使用 Sphinx 协议进行通信。
2. 基于这一假设，我们可以将握手与真实请求“粘合”在一起，并在一个数据包中发送。如果后端是旧版 Sphinx 守护进程，它将只是将这个粘合的数据包读取为 4 字节的握手，然后是请求体。由于它们都来自一个数据包，后端套接字具有 -1 RTT，而前端缓冲区仍然正常工作，尽管通常方式如此。
3. 继续这一假设：由于“查询”数据包相当小，而握手甚至更小，让我们使用现代 TFO（tcp-fast-open）技术在初始的 'SYN' TCP 包中发送两者。也就是说：我们通过连接到远程节点，发送粘合的握手 + 请求体包。守护进程接受连接，并立即在套接字缓冲区中拥有握手和请求体，因为它们来自第一个 TCP 'SYN' 包。这消除了另一个 RTT。
4. 最后，教会守护进程接受这一改进。实际上，从应用程序的角度来看，这意味着不要使用 `TCP_NODELAY`。从系统角度来看，这意味着确保在守护进程端激活 TFO 接收，并在客户端端激活 TFO 发送。在现代系统中，默认情况下客户端 TFO 已经激活，因此你只需调整服务器 TFO 以使所有内容正常工作。

所有这些改进在不实际更改协议本身的情况下，使我们能够从连接中消除 1.5 个 RTT 的 TCP 协议。这表示，如果查询和响应能够放入一个 TCP 包中，整个二进制 API 会话从 3.5 个 RTT 减少到 2 个 RTT，从而使网络协商速度提高约 2 倍。

因此，我们所有的改进都围绕着最初未定义的陈述：“谁先发言”。如果客户端先发言，我们可以应用所有这些优化，并有效地在单个 TFO 包中处理连接 + 握手 + 查询。此外，我们可以通过接收到的数据包的开头确定实际协议。这就是为什么你可以通过 API/http/https 连接到同一个端口。如果守护进程必须先发言，所有这些优化都不可能，多协议也不可能。这就是为什么我们为 MySQL 专门分配了一个端口，而没有将其与其他所有协议统一到同一个端口。突然间，在所有客户端中，有一个是按照守护进程应先发送握手的方式编写的。也就是说，无法实现上述所有改进。这就是 SphinxSE 插件用于 mysql/mariadb。因此，为了这个单一客户端，我们专门定义了 `sphinx` 协议以最旧的方式工作。即：双方激活 `TCP_NODELAY` 并通过小数据包交换。守护进程在连接时发送其握手，然后客户端发送其握手，然后一切按常规方式工作。这不是很高效，但确实有效。如果你使用 SphinxSE 连接到 Manticore，你必须专门指定一个带有明确声明的 `sphinx` 协议的监听器。对于其他客户端，避免使用此监听器，因为它较慢。如果你使用其他旧版 Sphinx API 客户端，请首先检查它们是否能够与非专用多协议端口一起工作。对于使用非专用（多协议）端口的主代理连接并启用客户端和服务器 TFO，效果很好，并且肯定会加快网络后端的工作速度，特别是如果你有非常轻量且快速的查询。
</details>

### listen_tfo

此设置允许所有监听器使用 TCP_FASTOPEN 标志。默认情况下，它由系统管理，但可以通过将其设置为 '0' 明确关闭。

关于 TCP Fast Open 扩展的一般知识，请参阅 [维基百科](https://en.wikipedia.org/wiki/TCP_Fast_Open)。简而言之，它允许在建立连接时消除一个 TCP 往返时间。

在实践中，许多情况下使用 TFO 可能会优化客户端与代理的网络效率，就像 [持久代理](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 起作用时一样，但无需保持活动连接，也无需限制最大连接数。

在现代操作系统中，TFO 支持通常在系统级别默认开启，但这只是一个“能力”，并非规则。Linux（作为最进步的系统）自 2011 年起在 3.7 版本内核（服务器端）开始支持。Windows 从某些 Windows 10 构建版本开始支持。其他操作系统（FreeBSD、MacOS）也参与其中。

对于 Linux 系统，服务器检查变量 `/proc/sys/net/ipv4/tcp_fastopen` 并根据其行为。第 0 位管理客户端，第 1 位管理监听器。默认情况下，系统将此参数设置为 1，即客户端启用，监听器禁用。

### log

<!-- example conf log -->
日志设置指定所有 `searchd` 运行时事件将被记录的日志文件名称。如果未指定，默认名称为 'searchd.log'。

或者，您可以使用 'syslog' 作为文件名。在这种情况下，事件将被发送到 syslog 守护进程。要使用 syslog 选项，您需要在构建时使用 `-–with-syslog` 选项配置 Manticore。


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

当使用 [多查询](../Searching/Multi-queries.md) 时，searchd 会对单个批次中提交的查询数量执行合理性检查。将其设置为 0 以跳过检查。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_batch_queries = 256
```
<!-- end -->

### max_connections

<!-- example max_connections -->
最大同时客户端连接数。默认情况下无限制。通常只有在使用任何类型的持久连接时才会注意到这一点，例如 cli mysql 会话或来自远程分布式表的持久远程连接。当超过限制时，您仍然可以使用 [VIP 连接](../Connecting_to_the_server/MySQL_protocol.md#VIP-connection) 连接到服务器。VIP 连接不计入限制。

<!-- request Example -->
```ini
max_connections = 10
```

<!-- end -->

### max_threads_per_query

<!-- example max_threads_per_query -->
单个操作可以使用的线程的全局限制。默认情况下，适当的操作可以占用所有 CPU 核心，没有其他操作的余地。例如，对一个相当大的 percolate 表执行 `call pq` 可能会持续数十秒占用所有线程。将 `max_threads_per_query` 设置为例如 [threads](../Server_settings/Searchd.md#threads) 的一半，可以确保您能够并行运行几个这样的 `call pq` 操作。

您还可以在运行时将此设置作为会话或全局变量进行设置。

此外，您可以使用 [threads OPTION](../Searching/Options.md#threads) 在每个查询的基础上控制行为。

<!-- intro -->
##### 示例：
<!-- request Example -->

```ini
max_threads_per_query = 4
```

<!-- end -->

### max_filters

<!-- example conf max_filters -->
每个查询允许的最大过滤器数量。此设置仅用于内部合理性检查，不会直接影响 RAM 使用或性能。可选，默认值为 256。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_filters = 1024
```
<!-- end -->


### max_filter_values

<!-- example conf max_filter_values -->
每个过滤器允许的最大值数量。此设置仅用于内部合理性检查，不会直接影响 RAM 使用或性能。可选，默认值为 4096。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_filter_values = 16384
```
<!-- end -->


### max_open_files

<!-- example conf max_open_files -->
服务器允许打开的最大文件数称为“软限制”。请注意，服务大型碎片化实时表可能需要将此限制设置得较高，因为每个磁盘块可能占用十几个或更多的文件。例如，一个包含 1000 个块的实时表可能需要同时打开数千个文件。如果您在日志中遇到“Too many open files”错误，请尝试调整此选项，因为它可能有助于解决问题。

还有一个“硬限制”，该限制无法通过选项超过。此限制由系统定义，可以在 Linux 的文件 `/etc/security/limits.conf` 中更改。其他操作系统可能有不同的方法，因此请参阅相关手册以获取更多信息。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = 10000
```
<!-- end -->

<!-- example conf max_open_files max -->
除了直接的数字值外，您还可以使用“max”这个魔法词将限制设置为当前可用的硬限制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = max
```
<!-- end -->


### max_packet_size

<!-- example conf max_packet_size -->
允许的最大网络数据包大小。此设置限制了来自客户端的查询数据包和分布式环境中远程代理的响应数据包。仅用于内部合理性检查，不会直接影响 RAM 使用或性能。可选，默认值为 128M。


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

多个挑剔的MySQL客户端库依赖于MySQL使用的特定版本号格式，而且有时会根据报告的版本号（而不是指示的能力标志）选择不同的执行路径。例如，Python MySQLdb 1.2.2在版本号不是X.Y.ZZ格式时会抛出异常；MySQL .NET连接器6.3.x在版本号为1.x以及某些标志组合时会内部失败等。为了解决这个问题，可以使用`mysql_version_string`指令，并让`searchd`向通过MySQL协议连接的客户端报告不同的版本号。（默认情况下，它会报告自己的版本。）


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
mysql_version_string = 5.0.37
```
<!-- end -->


### net_workers

网络线程的数量，默认值为1。

当仅有一个线程不足以处理所有传入查询时，此设置在极高查询率的情况下非常有用。


### net_wait_tm

控制网络线程的繁忙循环间隔。默认值为-1，可以设置为-1、0或正整数。

当服务器配置为纯主节点并仅将请求路由到代理时，重要的是要无延迟地处理请求，并且不允许网络线程睡眠。为此有一个繁忙循环。如果`net_wait_tm`是正数，则在接收到请求后，网络线程会使用CPU轮询`10 * net_wait_tm`毫秒；如果`net_wait_tm`为0，则仅使用CPU轮询。此外，可以通过`net_wait_tm = -1`禁用繁忙循环 - 在这种情况下，轮询器会在系统轮询调用中将超时设置为实际代理的超时。

> **警告：** CPU繁忙循环实际上会加载CPU核心，因此将此值设置为任何非默认值会导致即使在空闲服务器上也会出现明显的CPU使用率。


### net_throttle_accept

定义网络循环的每次迭代中接受的客户端数量。默认值为0（无限制），对于大多数用户来说应该没问题。这是一个微调选项，用于在高负载场景下控制网络循环的吞吐量。


### net_throttle_action

定义网络循环的每次迭代中处理的请求数量。默认值为0（无限制），对于大多数用户来说应该没问题。这是一个微调选项，用于在高负载场景下控制网络循环的吞吐量。

### network_timeout

<!-- example conf network_timeout -->
网络客户端请求的读写超时时间，以秒为单位（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认值为5秒。`searchd`将强制关闭在超时内未能发送查询或读取结果的客户端连接。

还要注意 [reset_network_timeout_on_packet](../Server_settings/Searchd.md#reset_network_timeout_on_packet) 参数。此参数会将 `network_timeout` 的行为从应用于整个 `query` 或 `result` 改为应用于单个数据包。通常，查询/结果适合一个或两个数据包。然而，在需要大量数据的情况下，此参数在保持操作活跃方面可能非常有价值。

<!-- request Example -->

```ini
network_timeout = 10s
```
<!-- end -->

### node_address

<!-- example conf node_address -->
此设置允许您指定节点的网络地址。默认情况下，它设置为复制的 [listen](../Server_settings/Searchd.md#listen) 地址。这在大多数情况下是正确的；但是，有些情况下您必须手动指定它：

* 防火墙后的节点
* 启用了网络地址转换（NAT）
* 容器部署，如Docker或云部署
* 跨多个区域的集群中的节点


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
node_address = 10.101.0.10
```
<!-- end -->

### not_terms_only_allowed

<!-- example conf not_terms_only_allowed -->
此设置确定是否允许仅使用 [negation](../Searching/Full_text_matching/Operators.md#Negation-operator) 全文操作符的查询。可选，默认值为0（仅使用NOT操作符的查询失败）。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
not_terms_only_allowed = 1
```
<!-- end -->

### optimize_cutoff

<!-- example conf optimize_cutoff -->
设置默认的表压缩阈值。更多信息请参见 - [优化的磁盘块数量](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks)。此设置可以通过每个查询选项 [cutoff](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 覆盖。也可以通过 [SET GLOBAL](../Server_settings/Setting_variables_online.md#SET) 动态更改。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
optimize_cutoff = 4
```
<!-- end -->

### persistent_connections_limit

<!-- example conf persistent_connections_limit -->
此设置确定到远程 [persistent agents](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 的最大同时持久连接数。每次连接在`agent_persistent`下定义的代理时，我们尝试重用现有连接（如果有的话），或者连接并保存连接以供将来使用。然而，在某些情况下，限制此类持久连接的数量是有意义的。此指令定义了限制。它会影响所有分布式表中每个代理主机的连接数量。

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
pid_file 是 Manticore 搜索中必填的配置选项，用于指定存储 `searchd` 服务器进程 ID 的文件路径。

搜索守护进程的进程 ID 文件在启动时会被重新创建并锁定，服务器运行时该文件包含主服务器进程的 ID。服务器关闭时会解除该文件的链接。
该文件的作用是使 Manticore 能够执行各种内部任务，例如检查是否已有正在运行的 `searchd` 实例、停止 `searchd`，并通知其应轮换表。该文件还可用于外部自动化脚本。


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
基于执行时间（通过最大查询时间设置）在查询完成前终止查询是一种不错的安全网，但这也伴随着一个固有的缺点：结果不可确定（不稳定）。也就是说，如果你多次重复执行完全相同的（复杂）搜索查询并设置时间限制，时间限制将在不同阶段被触发，你将得到*不同的*结果集。

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

有一个选项 [SELECT … OPTION max_predicted_time](../Searching/Options.md#max_predicted_time)，它允许你限制查询时间*并*获得稳定、可重复的结果。与其在评估查询时定期检查实际当前时间（这会导致不可确定性），不如使用一个简单的线性模型来预测当前运行时间：

```ini
predicted_time =
    doc_cost * processed_documents +
    hit_cost * processed_hits +
    skip_cost * skiplist_jumps +
    match_cost * found_matches
```

当 `predicted_time` 达到给定限制时，查询将被提前终止。

当然，这并不是实际耗时的硬性限制（不过，这是对*处理*工作量的硬性限制），而一个简单的线性模型远非理想精确。因此，墙钟时间*可能*低于或高于目标限制。然而，误差范围相当可接受：例如，在我们以 100 毫秒为目标限制的实验中，大多数测试查询落在 95 到 105 毫秒范围内，*所有*查询都在 80 到 120 毫秒范围内。此外，作为额外的好处，使用建模的查询时间而不是测量实际运行时间，也会减少 gettimeofday() 调用次数。

没有两台服务器的型号和规格是完全相同的，因此 `predicted_time_costs` 指令允许你为上述模型配置成本。为了方便，这些成本以整数形式表示，单位为纳秒。（`max_predicted_time` 中的限制以毫秒为单位，而需要将成本值指定为 0.000128 毫秒而不是 128 纳秒会更容易出错。）不需要一次性指定所有四个成本，未指定的成本将采用默认值。不过，我们强烈建议为所有成本指定值以提高可读性。


### preopen_tables

<!-- example conf preopen_tables -->
`preopen_tables` 配置指令指定是否在启动时强制预打开所有表。默认值为 1，这意味着无论每个表的 [preopen](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Other-performance-related-settings) 设置如何，所有表都将被预打开。如果设置为 0，则每个表的设置将生效，并默认为 0。

预打开表可以防止搜索查询和轮换之间的竞争，这可能导致查询偶尔失败。然而，它也会使用更多的文件句柄。在大多数情况下，建议预打开表。

以下是一个示例配置：

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
preopen_tables = 1
```
<!-- end -->

### pseudo_sharding

<!-- example conf pseudo_sharding -->
`pseudo_sharding` 配置选项允许对本地普通表和实时表的搜索查询进行并行化，无论这些查询是直接进行还是通过分布式表进行。此功能将自动并行化查询，最多达到 `searchd.threads` 中指定的线程数。

请注意，如果你的工作线程已经很忙，因为：
* 高查询并发
* 任何形式的物理分片：
  - 由多个普通/实时表组成的分布式表
  - 由太多磁盘块组成的实时表

那么启用 `pseudo_sharding` 可能不会带来任何好处，甚至可能导致吞吐量略有下降。如果你优先考虑更高的吞吐量而不是更低的延迟，建议禁用此选项。

默认情况下是启用的。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
pseudo_sharding = 0
```
<!-- end -->


### replication_connect_timeout

`replication_connect_timeout` 指令定义连接到远程节点的超时时间。默认情况下，该值以毫秒为单位，但可以使用 [其他后缀](../Server_settings/Special_suffixes.md)。默认值为 1000（1 秒）。

在连接到远程节点时，Manticore 最多将等待此时间以成功完成连接。如果超时时间已到但连接尚未建立，并且启用了 `retries`，将启动重试。


### replication_query_timeout

`replication_query_timeout` 设置 searchd 等待远程节点完成查询的时间量。默认值为 3000 毫秒（3 秒），但可以使用 `suffixed` 来表示不同的时间单位。

在建立连接后，Manticore 将等待最多 `replication_query_timeout` 的时间以让远程节点完成查询。请注意，此超时与 `replication_connect_timeout` 是分开的，远程节点可能导致的总延迟将是这两个值的总和。


### replication_retry_count

此设置是一个整数，指定 Manticore 在复制过程中尝试连接和查询远程节点的次数，直到报告致命查询错误。默认值为 3。


### replication_retry_delay

此设置是一个以毫秒为单位的整数（或[special_suffixes](../Server_settings/Special_suffixes.md)），它指定了在复制过程中发生故障时，Manticore 重新尝试查询远程节点之前的延迟时间。只有在指定非零值时，此值才相关。默认值为 500。

### qcache_max_bytes

<!-- example conf qcache_max_bytes -->
此配置设置分配给缓存结果集的最大 RAM 量（以字节为单位）。默认值为 16777216，相当于 16 兆字节。如果值设置为 0，则禁用查询缓存。有关查询缓存的更多信息，请参阅[查询缓存](../Searching/Query_cache.md)以获取详细信息。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
qcache_max_bytes = 16777216
```
<!-- end -->


### qcache_thresh_msec

以毫秒为单位的整数。查询结果被缓存的最小墙时间阈值。默认值为 3000，即 3 秒。0 表示缓存所有内容。有关详细信息，请参阅[查询缓存](../Searching/Query_cache.md)。此值也可以使用时间[special_suffixes](../Server_settings/Special_suffixes.md)表示，但使用时要小心，并不要与值本身名称中的 '_msec' 混淆。


### qcache_ttl_sec

以秒为单位的整数。缓存结果集的过期时间。默认值为 60，即 1 分钟。最小可能值为 1 秒。有关详细信息，请参阅[查询缓存](../Searching/Query_cache.md)。此值也可以使用时间[special_suffixes](../Server_settings/Special_suffixes.md)表示，但使用时要小心，并不要与值本身名称中的 '_sec' 混淆。


### query_log_format

<!-- example conf query_log_format -->
查询日志格式。可选，允许的值为 `plain` 和 `sphinxql`，默认为 `sphinxql`。

`sphinxql` 模式会记录有效的 SQL 语句。`plain` 模式以纯文本格式记录查询（主要适用于纯全文搜索用例）。此指令允许您在搜索服务器启动时在两种格式之间切换。也可以使用 `SET GLOBAL query_log_format=sphinxql` 语法在运行时更改日志格式。有关更多详细信息，请参阅[查询日志](../Logging/Query_logging.md)。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_format = sphinxql
```
<!-- end -->

### query_log_min_msec

限制（以毫秒为单位），防止查询被写入查询日志。可选，默认为 0（所有查询都会被写入查询日志）。此指令指定只有执行时间超过指定限制的查询才会被记录（此值也可以使用时间[special_suffixes](../Server_settings/Special_suffixes.md)表示，但使用时要小心，并不要与值本身名称中的 `_msec` 混淆）。

### query_log

<!-- example conf query_log -->
查询日志文件名。可选，默认为空（不记录查询）。所有搜索查询（如 SELECT ... 但不包括 INSERT/REPLACE/UPDATE 查询）都会记录在此文件中。格式在[查询日志](../Logging/Query_logging.md)中描述。如果使用 'plain' 格式，可以将 'syslog' 作为日志文件的路径。在这种情况下，所有搜索查询将通过 `LOG_INFO` 优先级发送到 syslog 守护进程，并以前缀 '[query]' 代替时间戳。要使用 syslog 选项，Manticore 必须在构建时使用 `-–with-syslog` 配置。


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
如果希望允许其他用户读取日志文件（例如，以非 root 用户运行的监控解决方案），此指令会很有用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_mode  = 666
```
<!-- end -->

### read_buffer_docs

<!-- example conf read_buffer_docs -->
read_buffer_docs 指令控制每个关键字的文档列表读取缓冲区大小。对于每个搜索查询中的每个关键字出现，都有两个相关的读取缓冲区：一个用于文档列表，一个用于命中列表。此设置允许您控制文档列表缓冲区大小。

更大的缓冲区大小可能会增加每个查询的 RAM 使用量，但可能会减少 I/O 时间。对于慢速存储，设置较大的值是有意义的，但对于能够处理高 IOPS 的存储，应在较低值范围内进行实验。

默认值为 256K，最小值为 8K。您还可以在[read_buffer_docs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs)中按表设置此值，这将覆盖服务器配置级别的任何设置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_docs = 128K
```
<!-- end -->


### read_buffer_hits

<!-- example conf read_buffer_hits -->
read_buffer_hits 指令指定搜索查询中每个关键字的命中列表读取缓冲区大小。默认大小为 256K，最小值为 8K。对于每个搜索查询中的每个关键字出现，都有两个相关的读取缓冲区，一个用于文档列表，一个用于命中列表。增加缓冲区大小可能会增加每个查询的 RAM 使用量，但会减少 I/O 时间。对于慢速存储，较大的缓冲区大小是有意义的，而对于能够处理高 IOPS 的存储，应在较低值范围内进行实验。

此设置也可以通过在[read_buffer_hits](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_hits)中使用 read_buffer_hits 选项按表指定，这将覆盖服务器级别的设置。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_hits = 128K
```
<!-- end -->

### read_unhinted

<!-- example conf read_unhinted -->
未提示的读取大小。可选，默认为 32K，最小为 1K

当查询时，某些读取操作提前知道确切需要读取的数据量，但有些目前还不知道。最明显的是命中列表的大小目前无法提前知道。此设置可让您控制在这些情况下读取多少数据。它会影响命中列表的I/O时间，对于大于未提示读取大小的列表会减少I/O时间，但对于较小的列表会增加I/O时间。它**不会**影响RAM使用，因为读取缓冲区已经分配。因此，它不应大于read_buffer。


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

当设置为0时，超时限制发送整个请求/查询的最大时间。
当设置为1（默认值）时，超时限制网络活动之间的最大时间。

在复制环境中，一个节点可能需要将大文件（例如100GB）发送到另一个节点。假设网络传输速率为1GB/s，数据包大小为4-5MB。传输整个文件需要100秒。默认超时为5秒，仅允许传输5GB后连接断开。增加超时可能是解决方法，但不可扩展（例如，下一个文件可能是150GB，再次导致失败）。然而，默认`reset_network_timeout_on_packet`设置为1时，超时应用于单个数据包而非整个传输。只要传输正在进行（在超时期间实际通过网络接收数据），连接将保持活跃。如果传输卡住，导致数据包之间发生超时，连接将被丢弃。

请注意，如果您设置了分布式表，每个节点——包括主节点和代理节点——都应进行调整。在主节点端，`agent_query_timeout`受影响；在代理节点端，`network_timeout`相关。

<!-- intro -->

##### 示例：

<!-- request Example -->

```ini
reset_network_timeout_on_packet = 0
```

<!-- end -->


### rt_flush_period

<!-- example conf rt_flush_period -->
RT表RAM块刷新检查周期，以秒为单位（或[special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认为10小时。

即使主动更新的RT表完全适合RAM块，仍可能导致binlog持续增长，影响磁盘使用和崩溃恢复时间。通过此指令，搜索服务器执行定期刷新检查，符合条件的RAM块可以被保存，从而实现后续的binlog清理。有关详细信息，请参阅[二进制日志](../Logging/Binary_logging.md)。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_flush_period = 3600 # 1 hour
```
<!-- end -->


### rt_merge_iops

<!-- example conf rt_merge_iops -->
RT块合并线程允许启动的最大I/O操作数（每秒）。可选，默认为0（无限制）。

此指令可限制由`OPTIMIZE`语句引起的I/O影响。可以保证所有RT优化活动生成的磁盘IOPS（每秒I/O数）不会超过配置的限制。限制rt_merge_iops可以减少合并操作引起的搜索性能下降。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_iops = 40
```
<!-- end -->

### rt_merge_maxiosize

<!-- example conf rt_merge_maxiosize -->
RT块合并线程允许启动的单个I/O操作的最大大小。可选，默认为0（无限制）。

此指令可限制由`OPTIMIZE`语句引起的I/O影响。超过此限制的I/O将被拆分为两个或更多I/O，然后将分别计入[rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops)限制。因此，可以保证所有优化活动生成的磁盘I/O每秒不会超过（rt_merge_iops * rt_merge_maxiosize）字节。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_maxiosize = 1M
```
<!-- end -->


### seamless_rotate

<!-- example conf seamless_rotate -->
在旋转包含大量数据的表时防止`searchd`停滞。可选，默认为1（启用无缝旋转）。在Windows系统中，默认禁用无缝旋转。

表可能包含需要预加载到RAM中的某些数据。目前，`.spa`、`.spb`、`.spi`和`.spm`文件会被完全预加载（它们分别包含属性数据、二进制属性数据、关键字表和已删除行映射）。如果没有启用无缝旋转，旋转表会尝试使用尽可能少的RAM，并按以下步骤工作：

1. 新查询暂时被拒绝（返回“retry”错误代码）；
2. `searchd`等待所有当前运行的查询完成；
3. 旧表被释放，其文件被重命名；
4. 新表文件被重命名，并分配所需的RAM；
5. 新表的属性和字典数据被预加载到RAM；
6. `searchd`从新表恢复查询服务。

然而，如果属性或字典数据很多，预加载步骤可能需要明显的时间——在预加载1-5+GB文件的情况下，可能需要几分钟。

启用无缝旋转后，旋转过程如下：

1. 为新表分配RAM存储；
2. 新表的属性和字典数据异步预加载到RAM；
3. 成功时，旧表被释放，两个表的文件被重命名；
4. 失败时，新表被释放；
5. 在任何时刻，查询从旧表或新表副本中提供服务。

无缝旋转的代价是在旋转期间更高的峰值内存使用（因为旧表和新表的`.spa/.spb/.spi/.spm`数据在预加载新副本时都需要在RAM中）。平均使用量保持不变。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
seamless_rotate = 1
```
<!-- end -->

### secondary_index_block_cache
<!-- example conf secondary_index_block_cache -->

此选项指定二级索引使用的块缓存大小。它是可选的，默认值为8 MB。当二级索引与包含许多值的过滤器（例如，IN() 过滤器）一起工作时，它们会读取和处理这些值的元数据块。
在连接查询中，此过程会针对左表的每一行批次重复执行，每个批次可能会在单个连接查询中重新读取相同的元数据。这可能会严重影响性能。元数据块缓存将这些块保留在内存中，以便后续批次可以重复使用。
缓存仅在连接查询中使用，对非连接查询没有影响。请注意，缓存大小限制适用于每个属性和每个二级索引。每个磁盘块中的每个属性都在此限制内运行。在最坏情况下，总内存使用量可以通过将限制乘以磁盘块数量和连接查询中使用的属性数量来估算。

设置 `secondary_index_block_cache = 0` 会禁用缓存。
##### 示例：

secondary_index_block_cache = 16M

<!-- intro -->
### secondary_indexes

<!-- request Example -->

```ini
此选项启用或禁用搜索查询中使用二级索引。它是可选的，默认值为1（启用）。请注意，对于索引操作不需要启用它，只要安装了 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)，它就始终处于启用状态。后者也是在搜索时使用索引所必需的。有三种模式可用：
```

<!-- end -->

`0`：在搜索中禁用二级索引。可以使用 [分析器提示](../Searching/Options.md#Query-optimizer-hints) 为单个查询启用它们
<!-- example conf secondary_indexes -->

`1`：在搜索中启用二级索引。可以使用 [分析器提示](https://github.com/manticoresoftware/columnar) 为单个查询禁用它们

* `force`：与启用相同，但在加载二级索引时发生的任何错误都会被报告，并且整个索引不会被加载到守护进程中。
* ##### 示例：
* CODE_BLOCK_71

<!-- intro -->
### server_id

<!-- request Example -->

```ini
用作服务器标识符的整数，用于生成复制集群中节点的唯一短UUID的种子。server_id 必须在集群的节点之间唯一，并且范围在0到127之间。如果未设置 server_id，它将被计算为MAC地址和PID文件路径的哈希值，或者使用随机数作为短UUID的种子。
```

<!-- end -->

##### 示例：

<!-- example conf server_id -->
server_id = 1


<!-- intro -->
### shutdown_timeout

<!-- request Example -->

```ini
`searchd --stopwait` 等待时间，以秒为单位（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认为60秒。
```
<!-- end -->


当你运行 `searchd --stopwait` 时，服务器需要在停止前执行一些活动，例如完成查询、刷新RT RAM块、刷新属性和更新binlog。这些任务需要一些时间。`searchd --stopwait` 将等待最多 `shutdown_time` 秒，直到服务器完成其任务。合适的时间取决于你的表大小和负载。

<!-- example conf shutdown_timeout -->
##### 示例：

shutdown_timeout = 3m # wait for up to 3 minutes


<!-- intro -->
### shutdown_token

<!-- request Example -->

```ini
用于从VIP Manticore SQL连接调用 'shutdown' 命令的密码的SHA1哈希。没有它，[debug](../Reporting_bugs.md#DEBUG) 'shutdown' 子命令将永远不会导致服务器停止。请注意，这种简单的哈希不应被视为强保护，因为我们没有使用加盐哈希或任何现代哈希函数。它旨在作为本地网络中管理守护进程的防呆措施。
```
<!-- end -->


### snippets_file_prefix

生成片段时添加到本地文件名前的前缀。可选，默认是当前工作目录。

此前缀可以与 `load_files` 或 `load_files_scattered` 选项一起用于分布式片段生成。

<!-- example conf snippets_file_prefix -->
请注意，这是一个前缀，**不是**路径！这意味着如果前缀设置为 "server1"，并且请求引用 "file23"，`searchd` 将尝试打开 "server1file23"（全部不带引号）。因此，如果你需要它作为路径，必须包含末尾的斜杠。

在构建最终文件路径后，服务器会解除所有相对目录，并将最终结果与 `snippet_file_prefix` 的值进行比较。如果结果不以该前缀开头，此类文件将被拒绝并显示错误消息。

例如，如果你将其设置为 `/mnt/data`，并且有人使用 `../../../etc/passwd` 作为源文件调用片段生成，他们将收到错误消息：

`File '/mnt/data/../../../etc/passwd' escapes '/mnt/data/' scope`

而不是文件内容。

此外，如果参数未设置且读取 `/etc/passwd`，它实际上会读取 `/daemon/working/folder/etc/passwd`，因为该参数的默认值是服务器的工作目录。

还要注意，这是一个本地选项；它不会以任何方式影响代理。因此，你可以在主服务器上安全地设置一个前缀。路由到代理的请求不会受到主服务器设置的影响。但是，它们会受到代理自身的设置影响。

这在文档存储位置（无论是本地存储还是NAS挂载点）在服务器之间不一致时可能会很有用。

##### 示例：

snippets_file_prefix = /mnt/common/server1/


<!-- intro -->
> **警告：** 如果你仍然想从文件系统根目录访问文件，必须显式地将 `snippets_file_prefix` 设置为空值（通过 `snippets_file_prefix=` 行），或设置为根目录（通过 `snippets_file_prefix=/`）。

<!-- request Example -->

```ini
### sphinxql_state
```
<!-- end -->

当前SQL状态将被序列化的文件路径。


### sphinxql_state

<!-- example conf sphinxql_state -->
Path to a file where the current SQL state will be serialized.

在服务器启动时，此文件会被重放。在符合条件的状态更改（例如，SET GLOBAL）时，此文件会自动重写。这可以防止一个难以诊断的问题：如果您加载了UDF函数但Manticore崩溃，当它被（自动）重新启动时，您的UDF和全局变量将不再可用。使用持久状态有助于确保优雅的恢复，避免此类意外情况。

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
使用SQL接口时，等待请求之间的最大时间（以秒为单位，或[special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认为15分钟。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
sphinxql_timeout = 15m
```
<!-- end -->


### ssl_ca

<!-- example conf ssl_ca -->
SSL证书颁发机构（CA）证书文件（也称为根证书）的路径。可选，默认为空。当不为空时，`ssl_cert`中的证书应由该根证书签名。

服务器使用CA文件验证证书上的签名。文件必须为PEM格式。

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

服务器使用此证书作为自签名的公钥，通过SSL加密HTTP流量。文件必须为PEM格式。


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
每个查询的公共子树文档缓存最大大小。可选，默认为0（禁用）。

此设置限制了公共子树优化器的RAM使用量（参见[multi-queries](../Searching/Multi-queries.md)）。每个查询最多将花费这么多RAM来缓存文档条目。将限制设置为0会禁用优化器。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
subtree_docs_cache = 8M
```
<!-- end -->


### subtree_hits_cache

<!-- example conf subtree_hits_cache -->
每个查询的公共子树命中缓存最大大小。可选，默认为0（禁用）。

此设置限制了公共子树优化器的RAM使用量（参见[multi-queries](../Searching/Multi-queries.md)）。每个查询最多将花费这么多RAM来缓存关键字出现次数（命中）。将限制设置为0会禁用优化器。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
subtree_hits_cache = 16M
```
<!-- end -->

### threads

<!-- example threads -->
Manticore守护进程的工作线程数（或线程池大小）。Manticore在启动时创建此数量的OS线程，它们在守护进程中执行所有任务，例如执行查询、创建片段等。某些操作可能会被拆分为子任务并并行执行，例如：

* 在实时表中搜索
* 在由本地表组成的分布式表中搜索
* 激活查询调用
* 其他

默认情况下，它设置为服务器上的CPU核心数。Manticore在启动时创建线程，并在停止前保持它们。每个子任务在需要时可以使用其中一个线程。当子任务完成时，它会释放线程，以便另一个子任务可以使用它。

在I/O密集型负载的情况下，将值设置为高于CPU核心数可能有意义。

<!-- request Example -->
```ini
threads = 10
```

<!-- end -->

### thread_stack

<!-- example conf thread_stack -->
任务（协程，一个搜索查询可能导致多个任务/协程）的最大堆栈大小。可选，默认为128K。

每个任务都有自己的128K堆栈。当您运行查询时，会检查它需要多少堆栈。如果默认的128K足够，它将直接处理。如果需要更多，将安排另一个堆栈增加的任务，继续处理。这种高级堆栈的最大大小受此设置限制。

将值设置为合理高的速率有助于处理非常深的查询，而不会导致总体RAM消耗过高。例如，将其设置为1G并不意味着每个新任务都会占用1G的RAM，但如果看到它需要大约100M堆栈，我们只需为任务分配100M。同时运行的其他任务将使用默认的128K堆栈。同样，我们甚至可以运行需要500M的更复杂查询。只有当我们**内部看到**任务需要超过1G的堆栈时，才会失败并报告thread_stack太低。

然而，实际上，即使需要16M堆栈的查询通常对于解析来说过于复杂，消耗太多时间和资源来处理。因此，守护进程会处理它，但通过`thread_stack`设置限制此类查询看起来相当合理。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
thread_stack = 8M
```
<!-- end -->


### unlink_old

<!-- example conf unlink_old -->
在成功轮换时是否删除`.old`表副本。可选，默认为1（删除）。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
unlink_old = 0
```
<!-- end -->


### watchdog

<!-- example conf watchdog -->
线程服务器看门狗。可选，默认为1（启用看门狗）。

当Manticore查询崩溃时，可能会导致整个服务器崩溃。启用看门狗功能后，`searchd`还会维护一个独立的轻量级进程，监控主服务器进程，并在发生异常终止时自动重新启动它。看门狗默认是启用的。

<!-- request Example -->

```ini
watchdog = 0 # disable watchdog
```
<!-- end -->
<!-- proofread -->

