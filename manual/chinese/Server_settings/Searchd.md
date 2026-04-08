# 配置中的 "Searchd" 部分

以下设置用于 Manticore Search 配置文件的 `searchd` 部分，以控制服务器的行为。以下是每个设置的摘要：

### access_plain_attrs

此设置为 [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。它是可选的，默认值为 `mmap_preread`。

`access_plain_attrs` 指令允许您为由该 searchd 实例管理的所有表定义 [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令具有更高的优先级，将覆盖此实例范围的默认值，提供更细粒度的控制。

### access_blob_attrs

此设置为 [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。它是可选的，默认值为 `mmap_preread`。

`access_blob_attrs` 指令允许您为由该 searchd 实例管理的所有表定义 [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令具有更高的优先级，将覆盖此实例范围的默认值，提供更细粒度的控制。

### access_doclists

此设置为 [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。它是可选的，默认值为 `file`。

`access_doclists` 指令允许您为由该 searchd 实例管理的所有表定义 [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令具有更高的优先级，将覆盖此实例范围的默认值，提供更细粒度的控制。

### access_hitlists

此设置为 [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。它是可选的，默认值为 `file`。

`access_hitlists` 指令允许您为由该 searchd 实例管理的所有表定义 [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令具有更高的优先级，将覆盖此实例范围的默认值，提供更细粒度的控制。

### access_dict

此设置为 [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 设置实例范围的默认值。它是可选的，默认值为 `mmap_preread`。

`access_dict` 指令允许您为由该 searchd 实例管理的所有表定义 [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。每个表的指令具有更高的优先级，将覆盖此实例范围的默认值，提供更细粒度的控制。

### agent_connect_timeout

此设置为 [agent_connect_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) 参数设置实例范围的默认值。


### agent_query_timeout

此设置为 [agent_query_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) 参数设置实例范围的默认值。可以使用 `OPTION agent_query_timeout=XXX` 子句在每个查询的基础上覆盖此设置。


### agent_retry_count

此设置是一个整数，指定 Manticore 在通过分布式表连接和查询远程代理之前尝试的次数，然后再报告致命查询错误。默认值为 0（即不重试）。您也可以使用 `OPTION retry_count=XXX` 子句在每个查询的基础上设置此值。如果提供了每个查询的选项，它将覆盖配置中指定的值。

请注意，如果您在分布式表的定义中使用了 [agent mirrors](../Creating_a_cluster/Remote_nodes/Mirroring.md#Agent-mirrors)，服务器将根据所选的 [ha_strategy](../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) 为每次连接尝试选择不同的镜像。在这种情况下，`agent_retry_count` 将针对一组中的所有镜像进行汇总。

例如，如果您有 10 个镜像并设置 `agent_retry_count=5`，服务器将最多重试 50 次，假设每个镜像平均尝试 5 次（使用 `ha_strategy = roundrobin` 选项时，这将成立）。

但是，作为 [agent](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) 的 `retry_count` 选项提供的值是绝对限制。换句话说，代理定义中的 `[retry_count=2]` 选项始终意味着最多 2 次尝试，无论您为代理指定了 1 个还是 10 个镜像。

### agent_retry_delay

此设置是一个以毫秒为单位的整数（或 [special_suffixes](../Server_settings/Special_suffixes.md)），指定 Manticore 在查询远程代理失败后重试的延迟时间。此值仅在指定了非零的 [agent_retry_count](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 或非零的每个查询 `retry_count` 时相关。默认值为 500。您也可以使用 `OPTION retry_delay=XXX` 子句在每个查询的基础上设置此值。如果提供了每个查询的选项，它将覆盖配置中指定的值。


### attr_flush_period

<!-- example conf attr_flush_period -->
当使用 [Update](../Data_creation_and_modification/Updating_documents/UPDATE.md) 实时修改文档属性时，更改首先写入属性的内存副本。这些更新发生在内存映射文件中，这意味着操作系统决定何时将更改写入磁盘。在 `searchd` 正常关闭（由 `SIGTERM` 信号触发）时，所有更改都会被强制写入磁盘。

您还可以指示 `searchd` 定期将这些更改写回磁盘以防止数据丢失。这些刷新之间的间隔由 `attr_flush_period` 决定，以秒为单位（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。

默认情况下，该值为 0，这会禁用周期性刷新。但是，正常关闭时仍会进行刷新。

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

默认情况下，表压缩会自动进行。您可以使用 `auto_optimize` 设置修改此行为：
* 0 禁用自动表压缩（您仍然可以手动调用 `OPTIMIZE`）
* 1 启用自动表压缩并使用默认阈值
* 任何大于 1 的整数 启用自动表压缩，同时将阈值乘以该值

默认情况下，阈值是逻辑 CPU 核心数乘以 2。

但是，如果表具有带有 KNN 索引的属性，则默认阈值不同。在这种情况下，它设置为物理 CPU 核心数除以 2，最小值为 1，以提高 KNN 搜索性能。

请注意，启用或禁用 `auto_optimize` 不会阻止您手动运行 [OPTIMIZE TABLE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE)。

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
此设置控制服务器在 [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 过程中允许并行运行的磁盘块合并任务数量，适用于实时表。

这仅影响磁盘块合并（压缩），不影响查询并行性。

将其设置为 `1` 以禁用并行块合并（合并任务将逐个运行）。较高的值可能会在具有快速存储的系统上加快压缩速度，但会增加并发磁盘 I/O。

默认情况下，Manticore 使用 [threads](../Server_settings/Searchd.md#threads) 设置的值进行此计算；如果未配置 `threads`，则默认为逻辑 CPU 的数量。`parallel_chunk_merges` 的默认值为 `1`，当 `threads` 为 `1`、`2` 或 `3` 时，当 `threads` 为 `4` 或更高时默认值为 `2`（即 `max(1, min(2, threads/2))` 使用整数除法）。

可以在运行时使用 `SET GLOBAL parallel_chunk_merges = N` 更改此值，并通过 `SHOW VARIABLES` 查看。

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

### merge_chunks_per_job

<!-- example conf merge_chunks_per_job -->
此设置控制在单个 OPTIMIZE 任务中合并多少个 RT 磁盘块（N-way 合并）。如果可用块少于此数量，任务将合并它能合并的块（最小 2 个）。

较低的值允许更多任务并行调度；较高的值会减少任务数量但增加每次合并的规模。

默认值为 `2`。

可以在运行时使用 `SET GLOBAL merge_chunks_per_job = N` 更改此值，并通过 `SHOW VARIABLES` 查看。

<!-- intro -->
##### 示例：

<!-- request Increase -->
```ini
merge_chunks_per_job = 4
```

<!-- end -->

### auto_schema

<!-- example conf auto_schema -->
Manticore 支持自动创建尚未存在但在 INSERT 语句中指定的表。此功能默认启用。要禁用它，请在配置中显式设置 `auto_schema = 0`。要重新启用它，请设置 `auto_schema = 1` 或从配置中移除 `auto_schema` 设置。

请注意，`/bulk` HTTP 端点不支持自动表创建。

> 注意：[自动模式功能](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果不起作用，请确保已安装 Buddy。

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
此设置控制二进制日志事务刷新/同步模式。它是可选的，默认值为 2（每个事务刷新，每秒同步）。

该指令确定二进制日志刷新到操作系统和同步到磁盘的频率。支持三种模式：

*  0，每秒刷新和同步。这提供最佳性能，但在服务器崩溃或操作系统/硬件崩溃的情况下，最多可能丢失 1 秒内提交的事务。
*  1，每个事务刷新和同步。此模式性能最差，但保证每个提交的事务数据都被保存。
*  2，每个事务刷新，每秒同步。此模式提供良好的性能，并确保在服务器崩溃时每个提交的事务都被保存。然而，在操作系统/硬件崩溃的情况下，最多可能丢失 1 秒内提交的事务。

对于熟悉 MySQL 和 InnoDB 的用户，此指令类似于 `innodb_flush_log_at_trx_commit`。在大多数情况下，默认的混合模式 2 提供了速度和安全性的良好平衡，可提供完整的 RT 表数据保护以防服务器崩溃，并对硬件崩溃提供部分保护。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
binlog_flush = 1 # ultimate safety, low speed
```
<!-- end -->

### binlog_common

<!-- example conf binlog_common -->
此设置控制二进制日志文件的管理方式。它是可选的，默认值为 0（每个表使用单独的文件）。

您有两种管理二进制日志文件的方式：

* 每个表使用单独的文件（默认，`0`）：每个表将其更改保存在自己的日志文件中。如果您的数据库中有许多在不同时间更新的表，这种设置非常合适。它允许表在不等待其他表的情况下进行更新。此外，如果一个表的日志文件出现问题，不会影响其他表。
* 所有表使用单个文件（`1`）：所有表使用同一个二进制日志文件。这种方法使文件管理更简单，因为文件数量更少。然而，如果某个表仍需要保存其更新，这可能导致文件保留时间过长。如果许多表需要同时更新，此设置可能会因所有更改必须等待写入单个文件而变慢。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
binlog_common = 1 # use a single binary log file for all tables
```
<!-- end -->

### binlog_max_log_size

<!-- example conf binlog_max_log_size -->
此设置控制二进制日志文件的最大大小。它是可选的，默认值为 256 MB。

当前二进制日志文件达到此大小限制时，将强制打开一个新的 binlog 文件。这会导致日志的粒度更细，并在某些边缘工作负载下可能提高 binlog 磁盘使用效率。值为 0 表示不应根据大小重新打开 binlog 文件。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
binlog_max_log_size = 16M
```
<!-- end -->


### binlog_path

<!-- example conf binlog_path -->
此设置确定二进制日志（也称为事务日志）文件的路径。它是可选的，默认值为构建时配置的数据目录（例如 Linux 中的 `/var/lib/manticore/data/binlog.*`）。

二进制日志用于 RT 表数据的崩溃恢复，以及普通磁盘索引的属性更新，这些属性否则仅存储在 RAM 中直到刷新。当启用日志记录时，每个提交到 RT 表的事务都会被写入日志文件。在非干净关闭后启动时，日志会自动重放，恢复记录的更改。

`binlog_path` 指令指定二进制日志文件的位置。它应仅包含路径；`searchd` 会根据需要在目录中创建和删除多个 `binlog.*` 文件（包括 binlog 数据、元数据和锁文件等）。

空值会禁用二进制日志记录，这会提高性能，但会使 RT 表数据面临风险。


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
此设置确定 Manticore Buddy 二进制文件的路径。它是可选的，默认值为构建时配置的路径，该路径在不同操作系统上有所不同。通常，您不需要修改此设置。但是，如果您希望以调试模式运行 Manticore Buddy、对 Manticore Buddy 进行更改或实现新插件，这可能会很有用。在后一种情况下，您可以从 https://github.com/manticoresoftware/manticoresearch-buddy 克隆 Buddy，将新插件添加到目录 `./plugins/` 中，并在更改目录到 Buddy 源代码后运行 `composer install --prefer-source`，以便更轻松地进行开发。

要确保可以运行 `composer`，您的机器必须安装了 PHP 8.2 或更高版本，并包含以下扩展：

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

您还可以选择使用 Linux amd64 平台上的特殊 `manticore-executor-dev` 版本，例如：https://github.com/manticoresoftware/executor/releases/tag/v1.0.13

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
此设置确定在使用持久连接时，等待请求之间的最大时间（以秒或 [special_suffixes](../Server_settings/Special_suffixes.md) 为单位）。它是可选的，默认值为五分钟。


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

指定用于传入请求的默认排序规则。可以在每个查询的基础上覆盖排序规则。有关可用排序规则的列表和其他详细信息，请参阅 [排序规则](../Searching/Collations.md) 部分。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
collation_server = utf8_ci
```
<!-- end -->


### data_dir

<!-- example conf data_dir -->
当指定此设置时，将启用[实时模式](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)，这是一种管理数据模式的强制方式。该值应为存储所有表、二进制日志以及此模式下Manticore Search正常运行所需其他内容的目录路径。
当指定`data_dir`时，不允许对[普通表](../Creating_a_table/Local_tables/Plain_table.md)进行索引。有关实时模式与普通模式之间差异的更多信息，请参阅[此部分](../Read_this_first.md#Real-time-table-vs-plain-table)。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
data_dir = /var/lib/manticore
```
<!-- end -->

### attr_autoconv_strict

<!-- example conf attr_autoconv_strict -->
此设置控制在INSERT和REPLACE操作期间将字符串转换为数字类型时的严格验证模式。可选，缺省值为0（非严格模式，向后兼容）。

当设置为1（严格模式）时，无效的字符串到数字转换（例如，将空字符串`''`或非数字字符串`'a'`转换为bigint属性）将返回错误，而不是静默转换为0。这有助于在数据插入期间尽早发现数据质量问题。

当设置为0（非严格模式，默认值）时，无效的转换将静默转换为0，保持与旧版本的向后兼容性。

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
防止在表中没有搜索时自动刷新RAM块的超时时间。可选，缺省值为30秒。

检查搜索的超时时间，以确定是否自动刷新。
只有在表中最近`diskchunk_flush_search_timeout`秒内至少有一次搜索时，才会自动刷新。与[diskchunk_flush_write_timeout](../Server_settings/Searchd.md#diskchunk_flush_write_timeout)配合使用。对应的[每表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_search_timeout)具有更高优先级，将覆盖此实例范围的默认值，提供更细粒度的控制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
diskchunk_flush_search_timeout = 120s
```
<!-- end -->

### diskchunk_flush_write_timeout

<!-- example conf diskchunk_flush_write_timeout -->
在没有写入操作前等待自动将RAM块刷新到磁盘的秒数。可选，缺省值为1秒。

如果在`diskchunk_flush_write_timeout`秒内RAM块中没有写入操作，块将被刷新到磁盘。与[diskchunk_flush_search_timeout](../Server_settings/Searchd.md#diskchunk_flush_search_timeout)配合使用。要禁用自动刷新，请在配置中显式设置`diskchunk_flush_write_timeout = -1`。对应的[每表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_write_timeout)具有更高优先级，将覆盖此实例范围的默认值，提供更细粒度的控制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
diskchunk_flush_write_timeout = 60s
```
<!-- end -->

### docstore_cache_size

<!-- example conf docstore_cache_size -->
此设置指定从文档存储中保留在内存中的文档块的最大大小。它是可选的，默认值为16m（16兆字节）。

当使用`stored_fields`时，文档块将从磁盘读取并解压缩。由于每个块通常包含多个文档，当处理下一个文档时可能会被重复使用。为此，块将保留在服务器范围的缓存中。缓存保存的是未压缩的块。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
docstore_cache_size = 8m
```
<!-- end -->

### engine

<!-- example conf engine -->
在RT模式下创建表时默认使用的属性存储引擎。可以是`rowwise`（默认）或`columnar`。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
engine = columnar
```
<!-- end -->


### expansion_limit

<!-- example conf expansion_limit -->
此设置确定单个通配符的扩展关键字的最大数量。它是可选的，默认值为0（无限制）。

当对使用`dict = keywords`启用的表执行子字符串搜索时，单个通配符可能会导致数千甚至数百万个匹配的关键字（例如，将`a*`与整个牛津词典进行匹配）。此指令允许您限制此类扩展的影响。设置`expansion_limit = N`将限制每个查询中的通配符扩展为最多N个最频繁匹配的关键字。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_limit = 16
```
<!-- end -->

### expansion_merge_threshold_docs

<!-- example conf expansion_merge_threshold_docs -->
此设置确定扩展关键字中允许合并所有此类关键字的最大文档数。它是可选的，默认值为32。

当对使用`dict = keywords`启用的表执行子字符串搜索时，单个通配符可能会导致数千甚至数百万个匹配的关键字。此指令允许您增加合并关键字的数量限制以加快匹配速度，但会增加搜索时的内存使用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_merge_threshold_docs = 1024
```
<!-- end -->

### expansion_merge_threshold_hits

<!-- example conf expansion_merge_threshold_hits -->
此设置确定扩展关键字中允许合并所有此类关键字的最大命中数。它是可选的，默认值为256。

当对使用 `dict = keywords` 启用的表执行子字符串搜索时，单个通配符可能会导致匹配数千甚至数百万个关键词。此指令允许您增加合并在一起的关键词数量限制以加快匹配速度，但会使用更多内存进行搜索。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_merge_threshold_hits = 512
```
<!-- end -->

### expansion_phrase_limit

<!-- example conf expansion_phrase_limit -->
此设置控制由于 `PHRASE`、`PROXIMITY` 和 `QUORUM` 操作符内部的 `OR` 操作符而生成的替代短语变体的最大数量。它是可选的，默认值为 1024。

在短语类操作符内部使用 `|`（OR）操作符时，根据指定的替代项数量，展开的组合总数可能会呈指数级增长。此设置通过在查询处理期间限制考虑的排列数，有助于防止查询展开过多。

如果生成的变体数量超过此限制，查询将：

- 以错误失败（默认行为）
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
此设置控制当由 `expansion_phrase_limit` 定义的查询展开限制被超过时的行为。

默认情况下，查询将以错误消息失败。当 `expansion_phrase_warning` 设置为 1 时，搜索将继续使用短语的部分转换（最多达到配置的限制），服务器将向用户返回警告消息和结果集。这允许过于复杂无法完全展开的查询仍返回部分结果，而不会完全失败。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_phrase_warning = 1
```
<!-- end -->

### grouping_in_utc

此设置指定 API 和 SQL 中的时间分组是否以本地时区或 UTC 计算。它是可选的，默认值为 0（表示“本地时区”）。

默认情况下，所有“按时间分组”表达式（如 API 中的按天、周、月、年，以及 SQL 中的按天、月、年、年月、年月日）均使用本地时间。例如，如果您有属性时间为 `13:00 utc` 和 `15:00 utc` 的文档，在分组情况下，它们将根据您的本地时区设置归入设施组。如果您位于 `utc`，则为同一天，但如果您位于 `utc+10`，则这些文档将被归入不同的 `按天分组` 设施组（因为 UTC+10 时区的 13:00 utc 是 23:00 本地时间，而 15:00 是第二天的 01:00）。有时这种行为不可接受，希望时间分组不依赖于时区。您可以使用定义的全局 TZ 环境变量运行服务器，但这会影响分组以外的其他功能（如日志中的时间戳），这可能也不理想。通过在配置中启用此选项（或在 SQL 中使用 [SET global](../Server_settings/Setting_variables_online.md#SET) 语句），将导致所有时间分组表达式以 UTC 计算，而其余时间相关函数（即服务器日志）仍使用本地时区。


### timezone

此设置指定日期/时间相关函数使用的时区。默认使用本地时区，但您可以指定 IANA 格式的不同时区（例如 `Europe/Amsterdam`）。

请注意，此设置对日志无影响，日志始终以本地时区运行。

另外请注意，如果使用了 `grouping_in_utc`，则“按时间分组”函数仍将使用 UTC，而其他日期/时间相关函数将使用指定的时区。总体而言，不建议将 `grouping_in_utc` 与 `timezone` 混合使用。

您可以在配置中或通过在 SQL 中使用 [SET global](../Server_settings/Setting_variables_online.md#SET) 语句配置此选项。


### ha_period_karma

<!-- example conf ha_period_karma -->
此设置指定代理镜像统计窗口大小，以秒为单位（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。它是可选的，默认值为 60 秒。

对于包含代理镜像的分布式表（更多信息请参见 [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)），主节点会跟踪每个镜像的多个不同计数器。这些计数器随后用于故障转移和平衡（主节点根据计数器选择最佳镜像使用）。计数器以 `ha_period_karma` 秒为单位分块累积。

开始新的分块后，主节点仍可能使用前一个分块的累积值，直到新分块填充一半。因此，任何之前的记录最多在 1.5 倍 `ha_period_karma` 秒后将不再影响镜像选择。

尽管最多使用两个分块进行镜像选择，但最多存储最近的 15 个分块用于监控目的。这些分块可以通过 [SHOW AGENT STATUS](../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS) 语句进行检查。


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

`hostname_lookup` 选项定义了续订主机名的策略。默认情况下，代理主机名的 IP 地址在服务器启动时会被缓存，以避免对 DNS 的过度访问。然而，在某些情况下，IP 地址可能会动态变化（例如云托管），此时可能不希望缓存 IP 地址。将此选项设置为 `request` 会禁用缓存，并对每个查询都查询 DNS。也可以使用 `FLUSH HOSTNAMES` 命令手动续订 IP 地址。

### jobs_queue_size

jobs_queue_size 设置定义了队列中可以同时存在的“任务”数量。默认情况下是无限的。

在大多数情况下，“任务”意味着对单个本地表（普通表或实时表的磁盘块）的一次查询。例如，如果你有一个由 2 个本地表组成的分布式表，或者一个包含 2 个磁盘块的实时表，对其中任何一个的搜索查询通常会在队列中放入 2 个任务。然后，线程池（其大小由 [threads](../Server_settings/Searchd.md#threads) 定义）会处理这些任务。然而，在某些情况下，如果查询过于复杂，可能会生成更多的任务。当 [max_connections](../Server_settings/Searchd.md#max_connections) 和 [threads](../Server_settings/Searchd.md#threads) 无法在期望的性能之间达到平衡时，建议更改此设置。

### join_batch_size

表连接通过累积一批匹配项来工作，这些匹配项是左表上执行的查询结果。然后，这批匹配项作为右表上的单个查询进行处理。

此选项允许您调整批次大小。默认值为 `1000`，将此选项设置为 `0` 会禁用批处理。

较大的批次大小可能会提高性能；然而，对于某些查询，它可能导致内存消耗过多。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
join_batch_size = 2000
```
<!-- end -->

### join_cache_size

在右表上执行的每个查询都由特定的 JOIN ON 条件定义，这些条件决定了从右表检索的结果集。

如果只有少量唯一的 JOIN ON 条件，重用结果可能比反复在右表上执行查询更高效。为此，结果集存储在缓存中。

此选项允许您配置此缓存的大小。默认值为 `20 MB`，将此选项设置为 0 会禁用缓存。

请注意，每个线程维护自己的缓存，因此在估算总内存使用量时，应考虑执行查询的线程数量。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
join_cache_size = 10M
```
<!-- end -->

### listen_backlog

<!-- example conf listen_backlog -->
listen_backlog 设置决定了 TCP 监听队列的长度，用于处理传入连接。这对于逐个处理请求的 Windows 构建尤为重要。当连接队列达到其限制时，新的传入连接将被拒绝。
对于非 Windows 构建，默认值通常可以正常工作，通常不需要调整此设置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
listen_backlog = 20
```
<!-- end -->

### kibana_version_string

<!-- example conf kibana_version_string -->
返回给 Kibana 或 OpenSearch Dashboards 的服务器版本字符串。可选 — 默认设置为 `7.6.0`。

某些版本的 Kibana 和 OpenSearch Dashboards 期望服务器报告特定的版本号，并可能根据该版本号表现出不同的行为。为解决此类问题，您可以使用此设置，使 Manticore 向 Kibana 或 OpenSearch Dashboards 报告自定义版本。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
kibana_version_string = 1.2.3
```
<!-- end -->

### read_only

```ini
read_only = {0 | 1}
```

启用整个守护进程的 [只读模式](../Security/Read_only.md)。当设置为 `1` 时：

- 所有新的客户端连接都以只读模式开始
- 修改查询（如 `INSERT`、`REPLACE`、`DELETE`、`UPDATE`、`CREATE`、`DROP`、`ALTER`、`ATTACH`、`OPTIMIZE` 和类似语句）将被拒绝
- 即使在 VIP 连接上，`SET ro=0` 也不被允许

当 Manticore 应该明确避免修改表存储时，此设置非常有用，例如在从只读文件系统提供普通表时。在此模式下，Manticore 可以从只读存储中提供普通表，当文件系统不允许时跳过创建锁定文件，并在当前表文件仍可用时忽略待处理的 `.new` 普通表工件，同时发出警告。

此设置不会使 RT 模式（`data_dir`）在只读文件系统上工作。`data_dir` 仍然需要可写存储。`read_only` 也不能用于声明 RT 或 percolate 表的配置中。

### listen

<!-- example conf listen -->
此设置允许您指定 Manticore 将接受连接的 IP 地址和端口，或 Unix 域套接字路径。

`listen` 的一般语法为：

```ini
listen = ( address ":" port | port | path | address ":" port start - port end ) [ ":" protocol [ "_vip" ] [ "_readonly" ] ]
```

您可以指定：
* 一个 IP 地址（或主机名）和一个端口号
* 或仅一个端口号
* 或一个 Unix 套接字路径（Windows 不支持）
* 或一个 IP 地址和端口范围

如果您指定了端口号但未指定地址，`searchd` 将监听所有网络接口。Unix 路径以斜杠开头。端口范围只能为复制协议设置。

您还可以指定用于此套接字连接的协议处理程序（监听器）。监听器包括：

* **未指定** - Manticore 将接受来自以下来源的连接：
  - 其他 Manticore 代理（即远程分布式表）
  - 通过 HTTP 和 HTTPS 的客户端
  - [Manticore Buddy](https://manticoresearch.com/blog/manticoresearch-buddy-intro/)。**确保您有此类监听器（或如下面提到的 `http` 监听器），以避免 Manticore 功能的限制。**
* `mysql` - 用于来自 MySQL 客户端的连接的 MySQL 协议。注意：
  - 也支持压缩协议。
  - 如果启用了 [SSL](../Security/SSL.md#SSL)，您可以建立加密连接。
* `replication` - 用于节点通信的复制协议。更多细节请参见 [复制](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md) 部分。您可以指定多个复制监听器，但它们必须全部监听同一 IP；端口可以不同。当您使用端口范围定义复制监听器（例如 `listen = 192.168.0.1:9320-9328:replication`）时，Manticore 不会立即在这些端口上开始监听。相反，它仅在您开始使用复制时，从指定范围内随机选择空闲端口。复制正常工作至少需要范围内的 2 个端口。
* `http` - 与 **未指定** 相同。Manticore 将通过此端口接受来自远程代理和客户端的 HTTP 和 HTTPS 连接。
* `https` - HTTPS 协议。Manticore 将在此端口上**仅**接受 HTTPS 连接。更多细节请参见 [SSL](../Security/SSL.md) 部分。
* `sphinx` - 旧版二进制协议。用于为远程 [SphinxSE](../Extensions/SphinxSE.md) 客户端提供服务。某些 Sphinx API 客户端实现（例如 Java 实现）需要明确声明监听器。

将后缀 `_vip` 添加到客户端协议（即所有协议，除了 `replication`，例如 `mysql_vip` 或 `http_vip` 或仅 `_vip`）会强制为连接创建专用线程，以绕过不同限制。这在服务器因严重过载而停滞或无法通过常规端口连接时，对节点维护很有用。

后缀 `_readonly` 会为监听器设置 [只读模式](../Security/Read_only.md)，并限制其仅接受读取查询。如果全局启用了 `read_only`，所有监听器会自动表现为只读监听器。

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

可以有多个 `listen` 指令。`searchd` 将在所有指定的端口和套接字上监听客户端连接。Manticore 包中提供的默认配置定义了监听端口：
* `9308` 和 `9312` 用于来自远程代理和非 MySQL 基础客户端的连接
* 以及端口 `9306` 用于 MySQL 连接。

如果您在配置中完全未指定任何 `listen`，Manticore 将等待连接：
* `127.0.0.1:9306` 用于 MySQL 客户端
* `127.0.0.1:9312` 用于 HTTP/HTTPS 和来自其他 Manticore 节点和基于 Manticore 二进制 API 的客户端的连接。

#### 监听特权端口

默认情况下，Linux 不允许您让 Manticore 监听低于 1024 的端口（例如 `listen = 127.0.0.1:80:http` 或 `listen = 127.0.0.1:443:https`），除非您以 root 身份运行 searchd。如果您仍希望能够在非 root 用户下启动 Manticore，使其监听低于 1024 的端口，请考虑以下任一方法（任一方法都应有效）：
* 运行命令 `setcap CAP_NET_BIND_SERVICE=+eip /usr/bin/searchd`
* 将 `AmbientCapabilities=CAP_NET_BIND_SERVICE` 添加到 Manticore 的 systemd 单元并重新加载守护进程（`systemctl daemon-reload`）。

#### 关于 Sphinx API 协议和 TFO 的技术细节
<details>
旧版 Sphinx 协议有 2 个阶段：握手交换和数据流。握手包括来自客户端的 4 字节数据包，以及来自守护进程的 4 字节数据包，其唯一目的是：客户端确定远程是真正的 Sphinx 守护进程，守护进程确定远程是真正的 Sphinx 客户端。主要数据流相当简单：让双方声明各自的握手，并验证对方的握手。这种短数据包的交换意味着需要使用特殊的 `TCP_NODELAY` 标志，该标志关闭 Nagle 的 TCP 算法，并声明 TCP 连接将作为小数据包的对话进行。
然而，这种协商中谁先发言并未严格定义。历史上，所有使用二进制 API 的客户端都先发言：发送握手，然后从守护进程读取 4 字节，然后发送请求并从守护进程读取响应。
当我们改进 Sphinx 协议兼容性时，我们考虑了这些因素：

1. 通常，主代理通信是从已知客户端到已知主机的已知端口建立的。因此，端点提供错误握手的可能性非常小。因此，我们可以隐式地假设双方都是有效的，并且确实使用Sphinx协议进行通信。
2. 在这个假设下，我们可以将握手与实际请求“粘合”在一起，并将其发送在一个数据包中。如果后端是一个旧版Sphinx守护进程，它会将这个粘合的数据包视为4字节的握手，然后是请求体。由于它们都包含在一个数据包中，后端套接字的RTT为-1，而前端缓冲区仍然可以正常工作，尽管通常方式如此。
3. 继续这个假设：由于“查询”数据包相当小，而握手更小，我们可以使用现代TFO（TCP快速打开）技术，将两者都发送在初始的“SYN”TCP数据包中。也就是说：我们通过粘合的握手+正文数据包连接到远程节点。守护进程接受连接，并立即在套接字缓冲区中同时拥有握手和正文，因为它们都包含在第一个TCP“SYN”数据包中。这消除了另一个RTT。
4. 最后，教会守护进程接受这种改进。实际上，从应用程序的角度来看，这意味着不要使用`TCP_NODELAY`。从系统角度来看，这意味着确保在守护进程端启用了TFO接受功能，在客户端也启用了TFO发送功能。在现代系统中，默认情况下客户端TFO已经启用，因此你只需调整服务器端的TFO设置即可使所有功能正常工作。

这些改进在不实际更改协议本身的情况下，使我们能够从连接中消除1.5个TCP协议的RTT。也就是说，如果查询和答案能够放入一个TCP数据包中，整个二进制API会话将从3.5个RTT减少到2个RTT——这使网络协商速度提高了约两倍。

因此，我们所有改进都围绕一个最初未定义的陈述：“谁先说话”。如果客户端先说话，我们可以应用所有这些优化，并有效地在单个TFO数据包中处理连接+握手+查询。此外，我们可以通过检查接收到的数据包的开头来确定实际协议。这就是为什么你可以通过API/http/https连接到同一个端口。如果守护进程必须先说话，所有这些优化都不可能，多协议也不可能。这就是为什么我们为MySQL分配了专用端口，而没有将其与其他所有协议统一到同一个端口。突然间，所有客户端中有一个是按照守护进程应先发送握手的逻辑编写的。也就是说，无法实现上述所有改进。这就是SphinxSE插件用于mysql/mariadb的情况。因此，专门为这个单一客户端，我们定义了`sphinx`协议以最旧的方式工作。即：双方激活`TCP_NODELAY`并使用小数据包进行通信。守护进程在连接时发送其握手，然后客户端发送其握手，之后一切按常规方式工作。这不是很高效，但能正常工作。如果你使用SphinxSE连接到Manticore，你必须指定一个明确声明了`sphinx`协议的监听器。对于其他客户端，避免使用此监听器，因为它较慢。如果你使用其他旧版Sphinx API客户端，请首先检查它们是否能够与非专用多协议端口一起工作。对于主代理连接使用非专用（多协议）端口并启用客户端和服务器TFO，效果很好，肯定会加快网络后端的工作速度，特别是如果你有非常轻量且快速的查询时。
</details>

### listen_tfo

此设置允许所有监听器使用TCP_FASTOPEN标志。默认情况下，它由系统管理，但可以通过将其设置为'0'显式关闭。

关于TCP Fast Open扩展的通用知识，请参阅[Wikipedia](https://en.wikipedia.org/wiki/TCP_Fast_Open)。简而言之，它允许在建立连接时消除一个TCP往返时间。

在实践中，许多情况下使用TFO可以优化客户端-代理网络效率，特别是当使用[persistent agents](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)时，即使不保持活动连接，也不会对最大连接数有限制。

在现代操作系统中，TFO支持通常在系统级别上启用，但这只是一个“能力”，并非规则。Linux（作为最进步的系统）自2011年起，从3.7内核开始支持（服务器端）。Windows从某些Windows 10版本开始支持。其他操作系统（FreeBSD、MacOS）也参与其中。

对于Linux系统服务器，检查变量`/proc/sys/net/ipv4/tcp_fastopen`并根据其行为。第0位管理客户端，第1位管理监听器。默认情况下，系统将此参数设置为1，即客户端启用，监听器禁用。

### log

<!-- example conf log -->
log设置指定了所有`searchd`运行时事件将被记录的日志文件名称。如果未指定，默认名称是'searchd.log'。

或者，你可以使用'syslog'作为文件名。在这种情况下，事件将被发送到syslog守护进程。要使用syslog选项，你需要在构建时使用`-–with-syslog`选项配置Manticore。


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

当使用[multi-queries](../Searching/Multi-queries.md)时，searchd会对单个批次中提交的查询数量执行合理性检查。将其设置为0以跳过检查。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_batch_queries = 256
```
<!-- end -->

### max_connections

<!-- example max_connections -->
最大同时客户端连接数。默认情况下不受限制。通常只有在使用任何类型的持久连接时才会注意到这一点，比如 cli mysql 会话或来自远程分布式表的持久远程连接。当超出限制时，您仍然可以使用 [VIP 连接](../Connecting_to_the_server/MySQL_protocol.md#VIP-connection) 连接到服务器。VIP 连接不会计入限制。

<!-- request Example -->
```ini
max_connections = 10
```

<!-- end -->

### max_threads_per_query

<!-- example max_threads_per_query -->
实例范围内一个操作可以使用的线程数限制。默认情况下，适当的操作可以占用所有 CPU 核心，不留空间给其他操作。例如，对一个相当大的 percolate 表执行 `call pq` 可能会持续数十秒占用所有线程。将 `max_threads_per_query` 设置为例如 [threads](../Server_settings/Searchd.md#threads) 的一半，可以确保您能够并行运行几个这样的 `call pq` 操作。

您还可以在运行时将此设置作为会话或全局变量进行设置。

此外，您可以借助 [threads OPTION](../Searching/Options.md#threads) 在每个查询的基础上控制行为。

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
服务器允许打开的文件最大数量称为“软限制”。请注意，服务大型碎片化实时表可能需要将此限制设置得较高，因为每个磁盘块可能占用十几个或更多的文件。例如，一个包含 1000 个块的实时表可能需要同时打开数千个文件。如果您在日志中遇到“Too many open files”错误，请尝试调整此选项，因为它可能有助于解决问题。

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

一些挑剔的 MySQL 客户端库依赖于 MySQL 使用的特定版本号格式，而且有时会根据报告的版本号（而不是指示的能力标志）选择不同的执行路径。例如，Python MySQLdb 1.2.2 在版本号不是 X.Y.ZZ 格式时会抛出异常；MySQL .NET 连接器 6.3.x 在版本号为 1.x 并且与某些标志组合时会内部失败等。为了解决这个问题，您可以使用 `mysql_version_string` 指令，并让 `searchd` 向通过 MySQL 协议连接的客户端报告不同的版本号。（默认情况下，它会报告自己的版本。）


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
mysql_version_string = 5.0.37
```
<!-- end -->


### net_workers

网络线程数，默认为 1。

在查询率极高且单个线程不足以处理所有传入查询的情况下，此设置非常有用。


### net_wait_tm

控制网络线程的繁忙循环间隔。默认值为 -1，可以设置为 -1、0 或正整数。

当服务器配置为纯主服务器并仅将请求路由到代理时，重要的是要无延迟地处理请求，并且不允许网络线程睡眠。为此有一个繁忙循环。如果 `net_wait_tm` 是正数，则在传入请求后，网络线程使用 CPU 轮询 `10 * net_wait_tm` 毫秒；如果 `net_wait_tm` 为 0，则仅使用 CPU 轮询。此外，可以通过 `net_wait_tm = -1` 禁用繁忙循环 - 在这种情况下，轮询器在系统轮询调用中将超时设置为实际代理的超时。

> **警告：** CPU 繁忙循环实际上会加载 CPU 核心，因此将此值设置为任何非默认值即使在空闲服务器上也会导致明显的 CPU 使用率。


### net_throttle_accept

定义网络循环的每次迭代中接受的客户端数量。默认值为 0（无限制），对于大多数用户来说应该足够。这是一个微调选项，用于在高负载场景下控制网络循环的吞吐量。


### net_throttle_action

定义网络循环的每次迭代中处理的请求数量。默认值为 0（无限制），对于大多数用户来说应该足够。这是一个微调选项，用于在高负载场景下控制网络循环的吞吐量。

### network_timeout

<!-- example conf network_timeout -->
网络客户端请求读/写超时时间，以秒为单位（或 [特殊后缀](../Server_settings/Special_suffixes.md)）。可选，缺省值为5秒。`searchd` 将强制关闭在该超时时间内未能发送查询或读取结果的客户端连接。

还要注意 [reset_network_timeout_on_packet](../Server_settings/Searchd.md#reset_network_timeout_on_packet) 参数。此参数会改变 `network_timeout` 的行为，使其不再适用于整个 `query` 或 `result`，而是适用于单个数据包。通常，一个查询/结果会包含在一个或两个数据包中。然而，在需要大量数据的情况下，此参数在保持操作活跃性方面可能非常有用。

<!-- request Example -->

```ini
network_timeout = 10s
```
<!-- end -->

### node_address

<!-- example conf node_address -->
此设置允许您指定节点的网络地址。默认情况下，它设置为复制的 [listen](../Server_settings/Searchd.md#listen) 地址。这在大多数情况下是正确的；但是，有些情况下您必须手动指定它：

* 节点位于防火墙后
* 启用了网络地址转换（NAT）
* 容器部署，如 Docker 或云部署
* 节点位于多个区域的集群中


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
node_address = 10.101.0.10
```
<!-- end -->

### not_terms_only_allowed

<!-- example conf not_terms_only_allowed -->
此设置决定是否允许仅使用 [否定](../Searching/Full_text_matching/Operators.md#Negation-operator) 全文操作符的查询。可选，缺省值为0（仅使用NOT操作符的查询将失败）。


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
此设置确定到远程 [持久代理](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 的最大同时持久连接数。每次连接在 `agent_persistent` 下定义的代理时，我们尝试重用现有连接（如果有的话），或者连接并保存连接以供将来使用。然而，在某些情况下，限制此类持久连接的数量是有意义的。此指令定义了限制。它会影响所有分布式表中每个代理主机的连接数量。

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
pid_file 是 Manticore 搜索中强制性的配置选项，用于指定存储 `searchd` 服务器进程 ID 的文件路径。

searchd 进程 ID 文件在启动时会被重新创建并锁定，并且在服务器运行时包含主服务器进程 ID。服务器关闭时会将其删除。
此文件的目的是使 Manticore 能够执行各种内部任务，例如检查是否已经有运行中的 `searchd` 实例，停止 `searchd`，并通知其应旋转表。该文件也可用于外部自动化脚本。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
pid_file = /run/manticore/searchd.pid
```
<!-- end -->

### preopen_tables

<!-- example conf preopen_tables -->
preopen_tables 配置指令指定是否在启动时强制预打开所有表。默认值为1，这意味着无论每个表的 [preopen](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Other-performance-related-settings) 设置如何，所有表都将被预打开。如果设置为0，则每个表的设置可以生效，并且它们的默认值为0。

预打开表可以防止搜索查询和旋转之间的竞争，这可能导致查询偶尔失败。然而，它也会使用更多的文件句柄。在大多数情况下，建议预打开表。

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
pseudo_sharding 配置选项启用对本地普通表和实时表的搜索查询并行化，无论它们是直接查询还是通过分布式表查询。此功能将自动并行化查询，最多达到 `searchd.threads` 中指定的线程数。

请注意，如果您的工作线程已经很忙，因为您有：
* 高查询并发
* 任何形式的物理分片：
  - 由多个普通/实时表组成的分布式表
  - 由太多磁盘块组成的实时表

那么启用 pseudo_sharding 可能不会带来任何好处，甚至可能导致吞吐量略有下降。如果您优先考虑更高的吞吐量而不是更低的延迟，建议禁用此选项。

默认启用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
pseudo_sharding = 0
```
<!-- end -->


### replication_connect_timeout

`replication_connect_timeout` 指令定义连接到远程节点的超时时间。默认情况下，该值以毫秒为单位，但可以具有 [其他后缀](../Server_settings/Special_suffixes.md)。默认值为1000（1秒）。

当连接到远程节点时，Manticore 将最多等待此时间以成功完成连接。如果超时但连接尚未建立，并且启用了 `retries`，将发起重试。


### replication_query_timeout

`replication_query_timeout` 设置 searchd 等待远程节点完成查询的时间量。默认值为 3000 毫秒（3 秒），但可以使用 `suffixed` 来表示不同的时间单位。

建立连接后，Manticore 将等待最多 `replication_query_timeout` 时间以让远程节点完成。请注意，此超时与 `replication_connect_timeout` 是分开的，远程节点可能引起的总延迟将是这两个值的总和。


### replication_retry_count

此设置是一个整数，指定 Manticore 在复制过程中尝试连接和查询远程节点的次数，直到报告致命查询错误。默认值为 3。


### replication_retry_delay

此设置是一个以毫秒为单位的整数（或 [特殊后缀](../Server_settings/Special_suffixes.md)），指定 Manticore 在复制过程中查询远程节点失败后重试的延迟时间。此值仅在指定非零值时相关。默认值为 500。

### qcache_max_bytes

<!-- example conf qcache_max_bytes -->
此配置设置分配给缓存结果集的最大 RAM 量（以字节为单位）。默认值为 16777216，相当于 16 兆字节。如果值设置为 0，则禁用查询缓存。有关查询缓存的更多信息，请参阅 [查询缓存](../Searching/Query_cache.md) 了解详细信息。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
qcache_max_bytes = 16777216
```
<!-- end -->


### qcache_thresh_msec

整数，以毫秒为单位。查询结果被缓存的最小墙时间阈值。默认值为 3000，即 3 秒。0 表示缓存所有内容。有关详细信息，请参阅 [查询缓存](../Searching/Query_cache.md)。此值也可以使用时间 [特殊后缀](../Server_settings/Special_suffixes.md) 表示，但使用时要小心，并不要与值本身名称中的 `_msec` 混淆。


### qcache_ttl_sec

整数，以秒为单位。缓存结果集的过期时间。默认值为 60 秒，即 1 分钟。最小可能值为 1 秒。有关详细信息，请参阅 [查询缓存](../Searching/Query_cache.md)。此值也可以使用时间 [特殊后缀](../Server_settings/Special_suffixes.md) 表示，但使用时要小心，并不要与值本身名称中的 `_sec` 混淆。


### query_log_format

<!-- example conf query_log_format -->
查询日志格式。可选，允许的值为 `plain` 和 `sphinxql`，默认为 `sphinxql`。

`sphinxql` 模式记录有效的 SQL 语句。`plain` 模式以纯文本格式记录查询（主要适用于纯全文搜索用例）。此指令允许您在搜索服务器启动时在两种格式之间切换。日志格式也可以通过使用 `SET GLOBAL query_log_format=sphinxql` 语法在运行时更改。有关更多详细信息，请参阅 [查询日志](../Logging/Query_logging.md)。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_format = sphinxql
```
<!-- end -->

### query_log_min_msec

限制（以毫秒为单位），防止查询被写入查询日志。可选，默认为 0（所有查询都会被写入查询日志）。此指令指定只有执行时间超过指定限制的查询才会被记录（此值也可以使用时间 [特殊后缀](../Server_settings/Special_suffixes.md) 表示，但使用时要小心，并不要与值本身名称中的 `_msec` 混淆）。

### query_log

<!-- example conf query_log -->
查询日志文件名。可选，默认为空（不记录查询）。所有搜索查询（如 SELECT ... 但不包括 INSERT/REPLACE/UPDATE 查询）都会记录在此文件中。格式在 [查询日志](../Logging/Query_logging.md) 中描述。如果使用 'plain' 格式，可以将 'syslog' 作为日志文件的路径。在这种情况下，所有搜索查询将通过 `LOG_INFO` 优先级发送到 syslog 守护进程，并以前缀 '[query]' 代替时间戳。要使用 syslog 选项，Manticore 必须在构建时配置为 `-–with-syslog`。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log = /var/log/query.log
```
<!-- end -->


### query_log_mode

<!-- example conf query_log_mode -->
`query_log_mode` 指令允许您为 searchd 和查询日志文件设置不同的权限。默认情况下，这些日志文件以 600 权限创建，这意味着只有服务器运行的用户和 root 用户可以读取日志文件。
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
`read_buffer_docs` 指令控制每个关键字的文档列表读取缓冲区大小。对于每个搜索查询中的每个关键字出现，都有两个相关的读取缓冲区：一个用于文档列表，一个用于命中列表。此设置允许您控制文档列表缓冲区大小。

更大的缓冲区大小可能会增加每个查询的 RAM 使用量，但可能会减少 I/O 时间。对于慢速存储，设置较大的值是有意义的，但对于能够处理高 IOPS 的存储，应在较低值范围内进行实验。

默认值为 256K，最小值为 8K。您还可以在每个表的基础上设置 [read_buffer_docs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs)，这将覆盖服务器配置级别设置的任何内容。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_docs = 128K
```
<!-- end -->


### read_buffer_hits

<!-- example conf read_buffer_hits -->
read_buffer_hits 指令指定了搜索查询中命中列表的每个关键字的读取缓冲区大小。默认大小为 256K，最小值为 8K。对于搜索查询中的每个关键字出现，都有两个相关的读取缓冲区，一个用于文档列表，一个用于命中列表。增加缓冲区大小可以增加每个查询的 RAM 使用量，但减少 I/O 时间。对于慢速存储，较大的缓冲区大小是有意义的，而对于能够实现高 IOPS 的存储，则应在低值范围内进行实验。

此设置也可以通过 [read_buffer_hits](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_hits) 中的 read_buffer_hits 选项在每个表的基础上指定，这将覆盖服务器级别的设置。

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

在查询时，某些读取操作可以提前知道确切的读取数据量，但有些目前还不能。最明显的是命中列表的大小目前无法提前知道。此设置允许您控制在这些情况下读取多少数据。它会影响命中列表的 I/O 时间，对于大于未提示读取大小的列表会减少 I/O 时间，而对于较小的列表则会增加 I/O 时间。它**不会**影响 RAM 使用，因为读取缓冲区已经分配。因此，它不应大于 read_buffer。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_unhinted = 32K
```
<!-- end -->

### reset_network_timeout_on_packet

<!-- example conf reset_network_timeout_on_packet -->
细化网络超时（如 `network_timeout` 和 `agent_query_timeout`）的行为。

当设置为 0 时，超时限制发送整个请求/查询的最大时间。
当设置为 1（默认值）时，超时限制网络活动之间的最大时间。

在复制情况下，一个节点可能需要将一个大文件（例如 100GB）发送到另一个节点。假设网络传输速率为 1GB/s，每个数据包大小为 4-5MB。要传输整个文件，需要 100 秒。默认超时为 5 秒，只能在连接断开前传输 5GB。增加超时可能是一个变通方法，但不可扩展（例如，下一个文件可能是 150GB，再次导致失败）。然而，默认 `reset_network_timeout_on_packet` 设置为 1 时，超时不是应用于整个传输，而是应用于单个数据包。只要传输正在进行（在超时期间实际通过网络接收数据），连接就会保持活跃。如果传输卡住，导致数据包之间发生超时，连接将被丢弃。

请注意，如果您设置了分布式表，每个节点——包括主节点和代理节点——都应进行调整。在主节点端，`agent_query_timeout` 受影响；在代理节点端，`network_timeout` 是相关的。

<!-- intro -->

##### 示例：

<!-- request Example -->

```ini
reset_network_timeout_on_packet = 0
```

<!-- end -->


### rt_flush_period

<!-- example conf rt_flush_period -->
RT 表的 RAM 块刷新检查周期，以秒为单位（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认为 10 小时。

完全适合 RAM 块的 RT 表仍可能导致不断增长的 binlog，影响磁盘使用和崩溃恢复时间。通过此指令，搜索服务器执行定期刷新检查，符合条件的 RAM 块可以被保存，从而实现后续的 binlog 清理。有关详细信息，请参阅 [Binary logging](../Logging/Binary_logging.md)。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_flush_period = 3600 # 1 hour
```
<!-- end -->


### rt_merge_iops

<!-- example conf rt_merge_iops -->
RT 块合并线程允许启动的最大 I/O 操作数（每秒）。可选，默认为 0（无限制）。

此指令允许您限制由 `OPTIMIZE` 语句引起的 I/O 影响。可以保证所有 RT 优化活动不会生成超过配置限制的磁盘 IOPS（每秒 I/O 操作数）。限制 rt_merge_iops 可以减少合并引起的搜索性能下降。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_iops = 40
```
<!-- end -->

### rt_merge_maxiosize

<!-- example conf rt_merge_maxiosize -->
RT 块合并线程允许启动的 I/O 操作最大大小。可选，默认为 0（无限制）。

此指令允许您限制由 `OPTIMIZE` 语句引起的 I/O 影响。超过此限制的 I/O 操作将被拆分为两个或多个 I/O 操作，然后将分别计入 [rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops) 限制。因此，可以保证所有优化活动不会生成超过 (rt_merge_iops * rt_merge_maxiosize) 字节的磁盘 I/O 每秒。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_maxiosize = 1M
```
<!-- end -->


### seamless_rotate

<!-- example conf seamless_rotate -->
防止在旋转包含大量数据的表时 `searchd` 停滞，以进行预缓存。可选，默认为 1（启用无缝旋转）。在 Windows 系统上，默认禁用无缝旋转。

表中可能包含需要在 RAM 中预缓存的一些数据。目前，`.spa`、`.spb`、`.spi` 和 `.spm` 文件会被完全预缓存（它们分别包含属性数据、blob 属性数据、关键字表和已删除行映射）。没有无缝旋转时，旋转表会尝试使用尽可能少的 RAM，并按以下步骤工作：

1. 新查询暂时被拒绝（带有“重试”错误代码）；
2. `searchd` 等待所有当前运行的查询完成；
3. 旧表被释放，其文件被重命名；
4. 新表文件被重命名，并分配所需的 RAM；
5. 新表的属性和字典数据被预加载到 RAM；
6. `searchd` 从新表恢复查询服务。

然而，如果存在大量属性或字典数据，预加载步骤可能需要明显的时间 - 在预加载 1-5+ GB 文件的情况下，可能需要几分钟。

启用无缝旋转后，旋转过程如下：

1. 为新表分配 RAM 存储；
2. 新表的属性和字典数据异步预加载到 RAM；
3. 成功时，旧表被释放，两个表的文件被重命名；
4. 失败时，新表被释放；
5. 在任何时刻，查询从旧表或新表副本中提供服务。

无缝旋转的代价是在旋转期间更高的峰值内存使用（因为预加载新副本时，旧和新副本的 `.spa/.spb/.spi/.spm` 数据都需要在 RAM 中）。平均使用量保持不变。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
seamless_rotate = 1
```
<!-- end -->

### secondary_index_block_cache
<!-- example conf secondary_index_block_cache -->

此选项指定二级索引使用的块缓存大小。它是可选的，默认为 8 MB。当二级索引与包含许多值的过滤器（例如 IN() 过滤器）一起工作时，它们会读取和处理这些值的元数据块。
在连接查询中，此过程对左表的每一行批次重复进行，每个批次可能在单个连接查询中重新读取相同的元数据。这会严重影响性能。元数据块缓存将这些块保留在内存中，以便后续批次可以重复使用。
缓存仅在连接查询中使用，对非连接查询没有影响。请注意，缓存大小限制适用于每个属性和每个二级索引。每个磁盘块中的每个属性都在此限制内运行。在最坏情况下，总内存使用量可以通过将限制乘以磁盘块数量和连接查询中使用的属性数量来估算。

将 `secondary_index_block_cache = 0` 可禁用缓存。
##### 示例：

secondary_index_block_cache = 16M

<!-- intro -->
### secondary_indexes

<!-- request Example -->

```ini
此选项启用/禁用搜索查询中使用二级索引。它是可选的，默认为 1（启用）。请注意，对于索引不需要启用它，因为只要安装了 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)，它就始终启用。后者也是使用索引进行搜索所必需的。有三种模式可用：
```

<!-- end -->

`0`：在搜索中禁用二级索引。可以使用 [analyzer hints](../Searching/Options.md#Query-optimizer-hints) 为单个查询启用它们
<!-- example conf secondary_indexes -->

`1`：在搜索中启用二级索引。可以使用 [analyzer hints](../Searching/Options.md#Query-optimizer-hints) 为单个查询禁用它们

* `force`：与启用相同，但在加载二级索引时发生的任何错误都会被报告，并且整个索引不会被加载到守护进程中。
* ##### 示例：
* CODE_BLOCK_69

<!-- intro -->
### server_id

<!-- request Example -->

```ini
作为服务器标识符的整数，用于生成唯一短 UUID 的种子，该 UUID 用于属于复制集群的节点。server_id 必须在集群的节点中唯一，并且范围在 0 到 127 之间。如果未设置 server_id，它将被计算为 MAC 地址和 PID 文件路径的哈希，或者使用随机数作为短 UUID 的种子。
```

<!-- end -->

##### 示例：

<!-- example conf server_id -->
server_id = 1


<!-- intro -->
### shutdown_timeout

<!-- request Example -->

```ini
`searchd --stopwait` 等待时间，以秒为单位（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认为 60 秒。
```
<!-- end -->


当你运行 `searchd --stopwait` 时，服务器需要在停止前执行一些活动，例如完成查询、刷新 RT RAM 块、刷新属性和更新 binlog。这些任务需要一些时间。`searchd --stopwait` 将等待最多 `shutdown_time` 秒，直到服务器完成其任务。合适的时间取决于你的表大小和负载。

<!-- example conf shutdown_timeout -->
##### 示例：

shutdown_timeout = 3m # wait for up to 3 minutes


<!-- intro -->
### shutdown_token

<!-- request Example -->

```ini
用于从 VIP Manticore SQL 连接调用 'shutdown' 命令的密码的 SHA1 哈希。没有它，[debug](../Reporting_bugs.md#DEBUG) 'shutdown' 子命令将永远不会导致服务器停止。请注意，这种简单的哈希不应被视为强保护，因为我们没有使用加盐哈希或任何现代哈希函数。它旨在作为本地网络中维护守护进程的防呆措施。
```
<!-- end -->


### skiplist_cache_size

此设置指定解压跳过列表的内存缓存最大大小。可选，默认为 64M。

跳过列表用于加快大型文档列表中的查找速度。缓存它们可以避免在查询中重复解压相同的跳过列表数据。将此选项设置为 `0` 以禁用缓存。

<!-- example conf skiplist_cache_size -->
##### 示例：

skiplist_cache_size = 128M

<!-- intro -->
### snippets_file_prefix

<!-- request Example -->

```ini
在生成片段时，用于在本地文件名前添加的前缀。可选，默认为当前工作目录。
```
<!-- end -->

此前缀可以与 `load_files` 或 `load_files_scattered` 选项一起用于分布式片段生成。

<!-- example conf snippets_file_prefix -->
请注意，这是一个前缀，**而不是**路径！这意味着如果前缀设置为 "server1"，并且请求引用 "file23"，`searchd` 将尝试打开 "server1file23"（全部不带引号）。因此，如果你需要它作为路径，必须包含尾随斜杠。

在构建最终文件路径后，服务器会解除所有相对目录并将其最终结果与 `snippet_file_prefix` 的值进行比较。如果结果不以该前缀开头，此类文件将被拒绝并显示错误消息。

Note that this is a prefix and **not** a path! This means that if a prefix is set to "server1" and the request refers to "file23", `searchd` will attempt to open "server1file23" (all of that without quotes). So, if you need it to be a path, you have to include the trailing slash.

After constructing the final file path, the server unwinds all relative dirs and compares the final result with the value of `snippet_file_prefix`. If the result does not begin with the prefix, such a file will be rejected with an error message.

例如，如果你将其设置为 `/mnt/data`，而有人使用文件 `../../../etc/passwd` 作为源来调用片段生成，他们将收到错误信息：

`File '/mnt/data/../../../etc/passwd' escapes '/mnt/data/' scope`

而不是文件内容。

此外，如果参数未设置且读取 `/etc/passwd`，实际上会读取 `/daemon/working/folder/etc/passwd`，因为该参数的默认值是服务器的工作目录。

请注意，这是一个本地选项；它不会以任何方式影响代理。因此，你可以在主服务器上安全地设置前缀。路由到代理的请求不会受到主服务器设置的影响。但是，它们会受到代理自身的设置影响。

这在文档存储位置（无论是本地存储还是 NAS 挂载点）在服务器之间不一致时可能会很有用。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
snippets_file_prefix = /mnt/common/server1/
```
<!-- end -->

> **警告：** 如果你仍然想从文件系统根目录访问文件，你必须显式地将 `snippets_file_prefix` 设置为空值（通过 `snippets_file_prefix=` 行），或设置为根目录（通过 `snippets_file_prefix=/`）。


### sphinxql_state

<!-- example conf sphinxql_state -->
保存当前 SQL 状态的文件路径。

服务器启动时，此文件会被重放。在符合条件的状态更改（例如 `SET GLOBAL`）时，此文件会自动被重写。这可以防止一个难以诊断的问题：如果你加载了 UDF 函数但 Manticore 崩溃，当它（自动）重新启动时，你的 UDF 和全局变量将不再可用。使用持久状态有助于确保优雅恢复，避免此类意外。

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
使用 SQL 接口时，等待请求之间的最大时间（以秒为单位，或 [特殊后缀](../Server_settings/Special_suffixes.md)）。可选，默认为 15 分钟。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
sphinxql_timeout = 15m
```
<!-- end -->


### ssl_ca

<!-- example conf ssl_ca -->
SSL 证书颁发机构（CA）证书文件（也称为根证书）的路径。可选，默认为空。当不为空时，`ssl_cert` 中的证书应由该根证书签名。

服务器使用 CA 文件来验证证书上的签名。文件必须为 PEM 格式。

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

服务器使用此证书作为自签名的公钥，通过 SSL 加密 HTTP 流量。文件必须为 PEM 格式。


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
每个查询的公共子树文档缓存最大大小。可选，默认为 0（禁用）。

此设置限制了公共子树优化器的内存使用（参见 [多查询](../Searching/Multi-queries.md)）。每个查询最多将使用此数量的内存来缓存文档条目。将限制设置为 0 会禁用优化器。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
subtree_docs_cache = 8M
```
<!-- end -->


### subtree_hits_cache

<!-- example conf subtree_hits_cache -->
每个查询的公共子树命中缓存最大大小。可选，默认为 0（禁用）。

此设置限制了公共子树优化器的内存使用（参见 [多查询](../Searching/Multi-queries.md)）。每个查询最多将使用此数量的内存来缓存关键字出现次数（命中）。将限制设置为 0 会禁用优化器。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
subtree_hits_cache = 16M
```
<!-- end -->

### threads

<!-- example threads -->
Manticore 守护进程的工作线程数（或线程池大小）。Manticore 在启动时会创建此数量的 OS 线程，它们在守护进程中执行所有任务，例如执行查询、创建片段等。某些操作可能会被拆分为子任务并并行执行，例如：

* 在实时表中搜索
* 在由本地表组成的分布式表中搜索
* 激活查询调用
* 其他操作

默认情况下，它设置为服务器上的 CPU 核心数。Manticore 在启动时创建线程，并在停止前一直保留它们。每个子任务在需要时可以使用其中一个线程。当子任务完成时，它会释放线程，以便其他子任务使用。

在 I/O 密集型负载的情况下，将该值设置为高于 CPU 核心数可能有意义。

<!-- request Example -->
```ini
threads = 10
```

<!-- end -->

### thread_stack

<!-- example conf thread_stack -->
任务（协程，一个搜索查询可能引发多个任务/协程）的最大堆栈大小。可选，默认为 128K。

每个任务都有自己的 128K 堆栈。当你运行查询时，会检查它需要多少堆栈。如果默认的 128K 足够，它将直接处理。如果需要更多，会安排一个堆栈增加的任务，继续处理。这种高级堆栈的最大大小受此设置限制。

将值设置为合理较高的速率有助于处理非常深的查询，而不会导致整体RAM消耗过高。例如，将其设置为1G并不意味着每个新任务都会消耗1G的RAM，但如果我们发现它需要例如100M的堆栈，我们只需为该任务分配100M。同时运行的其他任务将使用默认的128K堆栈。同样，我们也可以运行甚至更复杂的需要500M的查询。只有当我们**看到**内部该任务需要超过1G的堆栈时，才会失败并报告线程堆栈过低。

然而，在实践中，即使一个查询需要16M的堆栈，通常也过于复杂，解析时会消耗过多时间和资源。因此，守护进程会处理它，但通过`thread_stack`设置限制此类查询看起来相当合理。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
thread_stack = 8M
```
<!-- end -->


### unlink_old

<!-- example conf unlink_old -->
确定在成功轮换时是否取消链接`.old`表副本。可选，默认为1（取消链接）。


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

当Manticore查询崩溃时，可能会导致整个服务器崩溃。启用看门狗功能后，`searchd`还会维护一个独立的轻量级进程，监控主服务器进程，并在发生异常终止时自动重启它。看门狗默认是启用的。

<!-- request Example -->

```ini
watchdog = 0 # disable watchdog
```
<!-- end -->
<!-- proofread -->
