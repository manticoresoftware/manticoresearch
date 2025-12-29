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
当使用 [更新](../Data_creation_and_modification/Updating_documents/UPDATE.md) 实时修改文档属性时，更改首先写入属性的内存副本。这些更新发生在内存映射文件中，这意味着操作系统决定何时将更改写入磁盘。在 `searchd` 的正常关闭（由 `SIGTERM` 信号触发）时，所有更改都会被强制写入磁盘。

您还可以指示 `searchd` 定期将这些更改写回磁盘以防止数据丢失。这些刷新之间的间隔由 `attr_flush_period` 决定，以秒为单位（或 [特殊后缀](../Server_settings/Special_suffixes.md)）。

默认情况下，该值为 0，禁用周期性刷新。但是，正常关闭时仍会进行刷新。

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
* 1 明确启用它
* 启用它同时将优化阈值乘以 2。

默认情况下，OPTIMIZE 会运行直到磁盘块的数量小于或等于逻辑 CPU 核心数乘以 2。

但是，如果表具有带有 KNN 索引的属性，则此阈值不同。在这种情况下，它设置为物理 CPU 核心数除以 2，以提高 KNN 搜索性能。

请注意，启用或关闭 `auto_optimize` 不会阻止您手动运行 [OPTIMIZE TABLE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE)。

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

默认值为 `max(1, min(2, threads/2))`。

可以通过 `SET GLOBAL parallel_chunk_merges = N` 在运行时更改此值，并通过 `SHOW VARIABLES` 查看。

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
此设置控制在单个 OPTIMIZE 任务中合并多少个 RT 磁盘块（N 路合并）。如果可用块少于此数量，任务将合并它能合并的块（最少 2 个）。

较低的值允许更多任务并行调度；较高的值会减少任务数量但增加每次合并的规模。

默认值为 `2`。

可以通过 `SET GLOBAL merge_chunks_per_job = N` 在运行时更改此值，并通过 `SHOW VARIABLES` 查看。

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

*  0，每秒刷新和同步。这提供最佳性能，但在服务器崩溃或操作系统/硬件崩溃时，最多可能丢失 1 秒内提交的事务。
*  1，每个事务刷新和同步。此模式性能最差，但保证每个提交的事务数据都被保存。
*  2，每个事务刷新，每秒同步。此模式提供良好的性能，并确保在服务器崩溃时每个提交的事务都被保存。然而，在操作系统/硬件崩溃时，最多可能丢失 1 秒内提交的事务。

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

您可以选择两种方式来管理二进制日志文件：

* 每个表使用单独文件（默认，`0`）：每个表将其更改保存在自己的日志文件中。如果您有多个在不同时间更新的表，这种设置是理想的。它允许表在不等待其他表的情况下进行更新。此外，如果一个表的日志文件出现问题，不会影响其他表。
* 所有表使用单个文件（`1`）：所有表使用同一个二进制日志文件。这种方法使文件处理更简单，因为文件数量更少。然而，如果某个表仍需要保存其更新，这可能会导致文件保留时间比需要的更长。如果许多表需要同时更新，此设置可能会因所有更改必须等待写入一个文件而变慢。

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

一旦当前二进制日志文件达到此大小限制，将强制打开一个新的二进制日志文件。这会导致日志的粒度更细，并在某些边缘工作负载下可能导致二进制日志磁盘使用更高效。值为0表示不应根据大小重新打开二进制日志文件。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
binlog_max_log_size = 16M
```
<!-- end -->


### binlog_path

<!-- example conf binlog_path -->
此设置确定二进制日志（也称为事务日志）文件的路径。它是可选的，默认值为构建时配置的数据目录（例如，Linux中的`/var/lib/manticore/data/binlog.*`）。

二进制日志用于RT表数据的崩溃恢复，以及普通磁盘索引的属性更新，这些属性否则仅存储在RAM中直到刷新。当启用日志记录时，每个提交到RT表的事务都会被写入日志文件。在非干净关闭后启动时，日志会自动重放，恢复记录的更改。

`binlog_path`指令指定二进制日志文件的位置。它应仅包含路径；`searchd`会根据需要在目录中创建和删除多个`binlog.*`文件（包括二进制日志数据、元数据和锁文件等）。

空值会禁用二进制日志记录，这会提高性能但会使RT表数据面临风险。


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
此设置控制[boolean_simplify](../Searching/Options.md#boolean_simplify)搜索选项的默认值。它是可选的，默认值为1（启用）。

当设置为1时，服务器将自动应用[布尔查询优化](../Searching/Full_text_matching/Boolean_optimization.md)以提高查询性能。当设置为0时，默认情况下查询将不进行优化。此默认值可以通过使用相应的搜索选项`boolean_simplify`在每个查询的基础上覆盖。

<!-- request Example -->
```ini
searchd {
    boolean_simplify = 0  # disable boolean optimization by default
}
```
<!-- end -->

### buddy_path

<!-- example conf buddy_path -->
此设置确定Manticore Buddy二进制文件的路径。它是可选的，默认值为构建时配置的路径，该路径在不同操作系统上有所不同。通常，您不需要修改此设置。但是，如果您希望在调试模式下运行Manticore Buddy、对Manticore Buddy进行更改或实现新插件，这可能会有用。在后一种情况下，您可以从https://github.com/manticoresoftware/manticoresearch-buddy克隆Buddy，将新插件添加到目录`./plugins/`中，并在更改目录到Buddy源代码后运行`composer install --prefer-source`以简化开发。

为了确保您可以运行`composer`，您的机器必须安装有PHP 8.2或更高版本，并安装以下扩展：

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

您也可以选择使用Linux amd64平台上的特殊`manticore-executor-dev`版本，例如：https://github.com/manticoresoftware/executor/releases/tag/v1.0.13

如果您选择此方法，请记住将manticore executor的开发版本链接到`/usr/bin/php`。

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
此设置确定在使用持久连接时等待请求之间的最大时间（以秒或[special_suffixes](../Server_settings/Special_suffixes.md)为单位）。它是可选的，默认值为五分钟。


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

指定影响基于libc的排序规则的libc区域设置。有关详细信息，请参阅[collations](../Searching/Collations.md)部分。


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

指定用于传入请求的默认排序规则。可以在每个查询的基础上覆盖排序规则。有关可用排序规则和其他详细信息的列表，请参阅[collations](../Searching/Collations.md)部分。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
collation_server = utf8_ci
```
<!-- end -->


### data_dir

<!-- example conf data_dir -->
当指定此设置时，将启用[实时模式](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)，这是一种管理数据模式的强制性方式。该值应为存储所有表、二进制日志以及Manticore Search在此模式下正常运行所需其他内容的目录路径。
当指定`data_dir`时，不允许对[普通表](../Creating_a_table/Local_tables/Plain_table.md)进行索引。有关RT模式与普通模式之间差异的更多信息，请参见[此部分](../Read_this_first.md#Real-time-table-vs-plain-table)。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
data_dir = /var/lib/manticore
```
<!-- end -->

### attr_autoconv_strict

<!-- example conf attr_autoconv_strict -->
此设置控制在INSERT和REPLACE操作期间将字符串转换为数字类型时的严格验证模式。可选，默认值为0（非严格模式，向后兼容）。

当设置为1（严格模式）时，无效的字符串到数字转换（例如，将空字符串`''`或非数字字符串`'a'`转换为bigint属性）将返回错误，而不是静默转换为0。这有助于在数据插入期间尽早发现数据质量问题。

当设置为0（非严格模式，默认）时，无效的转换将静默转换为0，保持与旧版本的向后兼容性。

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
防止在表中没有搜索时自动刷新RAM块的超时时间。可选，默认值为30秒。

检查搜索的时间，以确定是否自动刷新。
只有在表中最近`diskchunk_flush_search_timeout`秒内至少有一次搜索时，才会发生自动刷新。与[diskchunk_flush_write_timeout](../Server_settings/Searchd.md#diskchunk_flush_write_timeout)配合使用。相应的[每表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_search_timeout)具有更高的优先级，将覆盖此实例范围的默认值，提供更细粒度的控制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
diskchunk_flush_search_timeout = 120s
```
<!-- end -->

### diskchunk_flush_write_timeout

<!-- example conf diskchunk_flush_write_timeout -->
在没有写入操作的情况下等待自动将RAM块刷新到磁盘的秒数。可选，默认值为1秒。

如果在`diskchunk_flush_write_timeout`秒内RAM块中没有写入操作，该块将被刷新到磁盘。与[diskchunk_flush_search_timeout](../Server_settings/Searchd.md#diskchunk_flush_search_timeout)配合使用。要禁用自动刷新，请在配置中显式设置`diskchunk_flush_write_timeout = -1`。相应的[每表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_write_timeout)具有更高的优先级，将覆盖此实例范围的默认值，提供更细粒度的控制。

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

当使用`stored_fields`时，文档块将从磁盘读取并解压缩。由于每个块通常包含多个文档，当处理下一个文档时可能会被重复使用。为此，该块保留在服务器范围的缓存中。缓存保存的是未压缩的块。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
docstore_cache_size = 8m
```
<!-- end -->

### engine

<!-- example conf engine -->
在RT模式下创建表时使用的默认属性存储引擎。可以是`rowwise`（默认）或`columnar`。

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

当对使用`dict = keywords`启用的表执行子字符串搜索时，单个通配符可能会导致匹配数千甚至数百万个关键字（例如，将`a*`与整个牛津词典进行匹配）。此指令允许您限制此类扩展的影响。设置`expansion_limit = N`将限制每个查询中的通配符扩展到最多N个最频繁匹配的关键字。

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

当对使用`dict = keywords`启用的表执行子字符串搜索时，单个通配符可能会导致匹配数千甚至数百万个关键字。此指令允许您增加合并关键字的数量限制以加快匹配速度，但会增加搜索时的内存使用量。

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

当使用`dict = keywords`启用的表中执行子串搜索时，单个通配符可能引发大量或甚至百万级匹配关键词。该指令允许您提高如何合并关键词的数量，以加快匹配速度，但会增加搜索内存使用量。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_merge_threshold_hits = 512
```
<!-- end -->

### expansion_phrase_limit

<!-- example conf expansion_phrase_limit -->
该设置控制了由`PHRASE`、`PROXIMITY`和`QUORUM`等操作符中的`OR`操作符生成的替代短语变体的最大数量。该设置是可选的，默认值为1024。

当使用`|`（OR）操作符时，总组合数可能会随着指定的替代变体数量指数级增长。该设置有助于防止查询扩展过多，通过限制在查询处理过程中考虑的排列数。

如果生成的变体数量超过该设置的限制，查询将要么失败（默认行为），要么返回部分结果并提示警告，当`expansion_phrase_warning`启用时。

- ##### 示例：
- CODE_BLOCK_27

<!-- intro -->
### expansion_phrase_warning

<!-- request Example -->

```ini
该设置控制了当查询扩展限制（由`expansion_phrase_limit`定义）超过时的行为。
```
<!-- end -->

默认情况下，查询将失败。当`expansion_phrase_warning`设置为1时，查询将使用部分变换（不超过配置的变体数量），并返回用户提示信息和结果集。这允许查询过于复杂但不完整失败的请求仍然返回部分结果。

<!-- example conf expansion_phrase_warning -->
##### 示例：

expansion_phrase_warning = 1

<!-- intro -->
### grouping_in_utc

<!-- request Example -->

```ini
该设置指定API和SQL中是否在本地时区或UTC计算时间分组。该设置是可选的，默认值为0（表示“本地时区”）。
```
<!-- end -->

默认情况下，所有`group by time`表达式（如API中的`day`、`week`、`month`、`year`，以及SQL中的`day`、`month`、`year`、`yearmonth`、`yearmonthday`）均使用本地时间。例如，如果有文档属性为`13:00 utc`和`15:00 utc`，在分组时，它们都会根据本地时区设置的设施组进行匹配。如果在UTC时区，这些文档将被匹配到同一`group by day`设施组。如果在UTC+10时区，这些文档将被匹配到不同的`group by day`设施组（因为UTC+10时区中13:00对应本地时间23:00，而15:00对应01:00下一天）。

有时这种行为可能不可接受，因此建议将时间分组与时区分离。您可以运行服务器时指定全局时区环境变量，但会影响不仅分组，还包括日志记录，这可能也是不理想的。切换该选项（无论是配置文件中还是SQL中的`SET global`语句）将使所有时间分组表达式使用UTC，而其他日期时间相关函数将使用指定的时区。整体而言，不建议同时启用`grouping_in_utc`和`timezone`。

You can configure this option either in the config or by using the [SET global](../Server_settings/Setting_variables_online.md#SET) statement in SQL.


##### 示例：

ha_period_karma = 2m

### ha_period_karma

该设置指定代理镜像统计窗口大小，以秒为单位（或使用特殊后缀）。该设置是可选的，默认值为60秒。

对于包含代理镜像的分布式表（请参阅关于代理的更多信息，见[代理](../Server_settings/Setting_variables_online.md#SET)），主节点跟踪多个不同的每镜像计数器。这些计数器用于故障转移和平衡（主节点根据计数器选择最佳镜像）。计数器在`ha_period_karma`秒的块内累积。


开始新块后，主节点仍可能使用之前累积的值直到新块一半满。因此，任何之前的记录将停止影响镜像选择，直到1.5倍`ha_period_karma`秒时为止。

<!-- example conf ha_period_karma -->
尽管最多使用两个块用于镜像选择，但最多存储15个最后块用于仪表盘目的。这些块可以通过`SHOW AGENT STATUS`语句进行检查。

##### 示例：

ha_ping_interval = 3s

Even though at most two blocks are used for mirror selection, up to 15 last blocks are stored for instrumentation purposes. These blocks can be inspected using the [SHOW AGENT STATUS](../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS) statement.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
ha_period_karma = 2m
```
<!-- end -->


### ha_ping_interval

<!-- example conf ha_ping_interval -->
This setting configures the interval between agent mirror pings, in milliseconds (or [special_suffixes](../Server_settings/Special_suffixes.md)).  It is optional, with a default value of 1000 milliseconds.

For a distributed table with agent mirrors in it (see more in [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)),  the master sends all mirrors a ping command during idle periods. This is to track the current agent status (alive or dead, network roundtrip, etc). The interval between such pings is defined by this directive. To disable pings, set ha_ping_interval to 0.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
ha_ping_interval = 3s
```
<!-- end -->


### hostname_lookup

### hostname_lookup  

主机名重置策略。默认情况下，代理主机名的IP地址在服务器启动时被缓存以避免过多的DNS访问。但在某些情况下，IP地址可能会动态变化（例如云托管），并且可能希望不缓存IP地址。设置该选项为 `request` 将禁用缓存，并在每次查询时进行DNS查询。IP地址也可以通过 `FLUSH HOSTNAMES` 命令手动更新。

### jobs_queue_size  

队列大小设置定义在队列中可以同时运行的“任务数”。默认值为无限。  

在大多数情况下，“任务”表示对单个本地表（纯文本表或实时表的块）的查询。例如，如果有两个本地表或实时表的两个块，对其中任何一个的搜索查询通常会将两个任务放入队列。然后，线程池（其大小由 [threads](../Server_settings/Searchd.md#threads) 定义）将处理这些任务。然而，在某些情况下，如果查询过于复杂，可能会生成更多任务。修改该设置是推荐的，当 [max_connections](../Server_settings/Searchd.md#max_connections) 和 [threads](../Server_settings/Searchd.md#threads) 不足时。

### join_batch_size  

表连接工作通过累积匹配结果，这些结果是左表执行查询的结果。该批次随后作为右表的一个查询处理。  

该选项允许调整批次大小。默认值为 `1000`，设置为 `0` 将禁用批量处理。  

<!-- intro -->
更大的批次大小可能提高性能，但某些查询可能会导致过多的内存消耗。

<!-- request Example -->

```ini
##### 示例：
```
<!-- end -->

### join_cache_size  

每个右表执行的查询由特定的 JOIN ON 条件定义，这些条件决定了从右表获取结果集的内容。  

如果只有少数唯一的 JOIN ON 条件，使用结果可以比重复查询右表更高效。为启用此功能，结果集被存储在缓存中。  

该选项允许配置缓存的大小。默认值为 `20 MB`，设置为 `0` 将禁用缓存。  

每个线程维护自己的缓存，因此在估算总内存使用时，应考虑线程数。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
### listen_backlog  
```
<!-- end -->

监听会话队列的长度决定了TCP连接的监听 backlog。对于Windows编译器，这尤其重要。当连接队列满时，新的连接将被拒绝。  

<!-- example conf listen_backlog -->
对于非Windows编译器，该设置的默认值通常可以正常工作，通常不需要调整。
##### 示例：


<!-- intro -->
### kibana_version_string  

<!-- request Example -->

```ini
服务器版本字符串用于返回给Kibana或OpenSearch Dashboards。可选，默认值为 `7.6.0`。  
```
<!-- end -->

一些Kibana和OpenSearch Dashboards版本要求服务器报告特定版本号，这可能因版本不同而行为不同。为解决此类问题，可以使用该设置，使Manticore返回自定义版本到Kibana或OpenSearch Dashboards。

<!-- example conf kibana_version_string -->
##### 示例：

### kill_dictionary  

<!-- intro -->
控制RT块字典的删除。启用后，Manticore从docstore中重建每块删除的文档的词统计，并在查询时减去这些统计，因此`docs`和`hits`以及排名权重会反映已删除的行。该功能在`idle`模式下默认启用。

<!-- request Example -->

```ini
模式：
```
<!-- end -->

`0`：禁用，查询使用原始块字典统计。

<!-- example conf kill_dictionary -->
`realtime`：启用，且删除立即应用（每块删除的重新编码）。

`flush`：启用，但每块删除的更新在下次重建后进行。
- `idle`：启用，但每块删除的更新在写入流空闲时重新计算。
- ##### 示例：
- ### kill_dictionary_idle_timeout  
- 在`kill_dictionary=idle`模式下，当有未处理的删除操作且在指定时间未发生写入时，Manticore重建删除统计并保存`.spks`文件。设置为 `-1` 就会禁用空闲重建。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
kill_dictionary = flush
```
<!-- end -->

### kill_dictionary_idle_timeout

<!-- example conf kill_dictionary_idle_timeout -->
Idle timeout for rebuilding kill dictionary stats when `kill_dictionary=idle`. If there are pending kills and no writes occur for this many seconds, Manticore rebuilds kill stats and persists `.spks`. Set to `-1` to disable idle rebuilds.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
kill_dictionary_idle_timeout = 15s
```
<!-- end -->

### listen

<!-- example conf listen -->
This setting lets you specify an IP address and port, or Unix-domain socket path, that Manticore will accept connections on.

The general syntax for `listen` is:

```ini
listen = ( address ":" port | port | path | address ":" port start - port end ) [ ":" protocol [ "_vip" ] [ "_readonly" ] ]
```

You can specify:
* either an IP address (or hostname) and a port number
* or just a port number
* or a Unix socket path (not supported on Windows)
* or an IP address and port range

If you specify a port number but not an address, `searchd` will listen on all network interfaces. Unix path is identified by a leading slash. Port range can be set only for the replication protocol.

You can also specify a protocol handler (listener) to be used for connections on this socket. The listeners are:

* **未指定** - Manticore 将接受来自以下来源的连接：
  - 其他 Manticore 代理（即远程分布式表）
  - 通过 HTTP 和 HTTPS 的客户端
  - [Manticore Buddy](https://manticoresearch.com/blog/manticoresearch-buddy-intro/)。**确保您有此类监听器（或如下面提到的 `http` 监听器），以避免 Manticore 功能的限制。**
* `mysql` - 用于来自 MySQL 客户端的连接的 MySQL 协议。注意：
  - 也支持压缩协议。
  - 如果启用了 [SSL](../Security/SSL.md#SSL)，您可以建立加密连接。
* `replication` - 用于节点通信的复制协议。更多细节请参见 [复制](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md) 部分。您可以指定多个复制监听器，但它们必须全部监听同一 IP；只有端口可以不同。当您使用端口范围定义复制监听器（例如 `listen = 192.168.0.1:9320-9328:replication`）时，Manticore 不会立即在这些端口上开始监听。相反，它仅在您开始使用复制时，从指定范围内随机选择空闲端口。复制正常工作至少需要范围中有 2 个端口。
* `http` - 与 **未指定** 相同。Manticore 将通过此端口接受来自远程代理和客户端的 HTTP 和 HTTPS 连接。
* `https` - HTTPS 协议。Manticore 将在此端口上**仅**接受 HTTPS 连接。更多细节请参见 [SSL](../Security/SSL.md) 部分。
* `sphinx` - 旧版二进制协议。用于为远程 [SphinxSE](../Extensions/SphinxSE.md) 客户端提供服务。某些 Sphinx API 客户端实现（例如 Java 客户端）需要显式声明监听器。

将后缀 `_vip` 添加到客户端协议（即所有协议，除了 `replication`，例如 `mysql_vip` 或 `http_vip` 或仅 `_vip`）会强制为连接创建专用线程，以绕过不同限制。这在服务器因严重过载而停滞或无法通过常规端口连接时，对节点维护很有用。

后缀 `_readonly` 会为监听器设置 [只读模式](../Security/Read_only.md)，并限制其仅接受读取查询。

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

可以有多个 `listen` 指令。`searchd` 将在所有指定的端口和套接字上监听客户端连接。Manticore 软件包中提供的默认配置定义了监听端口：
* `9308` 和 `9312` 用于来自远程代理和非 MySQL 客户端的连接
* 以及端口 `9306` 用于 MySQL 连接。

如果您在配置中完全未指定任何 `listen`，Manticore 将等待连接：
* `127.0.0.1:9306` 用于 MySQL 客户端
* `127.0.0.1:9312` 用于 HTTP/HTTPS 和来自其他 Manticore 节点和基于 Manticore 二进制 API 的客户端的连接。

#### 在特权端口上监听

默认情况下，Linux 不允许您让 Manticore 在 1024 以下的端口（例如 `listen = 127.0.0.1:80:http` 或 `listen = 127.0.0.1:443:https`）上监听，除非您以 root 身份运行 searchd。如果您仍然希望能够在非 root 用户下启动 Manticore，使其监听小于 1024 的端口，请考虑以下方法之一（任一方法都应有效）：
* 运行命令 `setcap CAP_NET_BIND_SERVICE=+eip /usr/bin/searchd`
* 将 `AmbientCapabilities=CAP_NET_BIND_SERVICE` 添加到 Manticore 的 systemd 单元并重新加载守护进程（`systemctl daemon-reload`）。

#### 关于 Sphinx API 协议和 TFO 的技术细节
<details>
旧版 Sphinx 协议有 2 个阶段：握手交换和数据流。握手包括来自客户端的 4 字节数据包，以及来自守护进程的 4 字节数据包，其唯一目的是让客户端确定远程端是真正的 Sphinx 守护进程，守护进程确定远程端是真正的 Sphinx 客户端。主要数据流相当简单：让双方声明各自的握手，并由对方检查它们。这种短数据包的交换意味着使用特殊的 `TCP_NODELAY` 标志，该标志关闭 Nagle 的 TCP 算法，并声明 TCP 连接将作为小数据包的对话进行。
然而，这种协商中谁先发言并未严格定义。历史上，所有使用二进制 API 的客户端都会先发言：发送握手，然后从守护进程中读取 4 字节，然后发送请求并从守护进程中读取响应。
当我们改进 Sphinx 协议兼容性时，我们考虑了以下几点：

1. 通常，主代理通信是从已知客户端到已知主机的已知端口建立的。因此，端点提供错误握手的可能性非常小。因此，我们可以隐式假设双方都是有效的，并且确实使用 Sphinx 协议进行通信。
2. 基于这一假设，我们可以将握手与实际请求“粘合”在一起，并将其发送在一个数据包中。如果后端是旧版 Sphinx 守护进程，它将仅将此粘合数据包视为 4 字节的握手，然后是请求正文。由于它们都包含在一个数据包中，后端套接字的 RTT 为 -1，而前端缓冲区仍然正常工作，尽管通常方式如此。
3. 继续这一假设：由于“查询”数据包相当小，握手甚至更小，让我们使用现代 TFO（tcp-fast-open）技术在初始的“SYN”TCP 包中发送两者。也就是说：我们通过粘合的握手 + 正文包连接到远程节点。守护进程接受连接，并立即在套接字缓冲区中同时拥有握手和正文，因为它们都包含在第一个 TCP “SYN” 包中。这消除了另一个 RTT。
4. 最后，教会守护进程接受这一改进。实际上，从应用程序的角度来看，这意味着**不要使用 `TCP_NODELAY`**。从系统角度来看，这意味着确保在守护进程端激活 TFO 接收，并在客户端端激活 TFO 发送。在现代系统中，默认情况下客户端 TFO 已经激活，因此您只需调整服务器 TFO 即可使所有内容正常工作。

所有这些改进在不实际更改协议本身的情况下，使我们能够从TCP协议连接中消除1.5个RTT。也就是说，如果查询和答案能够放入一个TCP数据包中，整个二进制API会话将从3.5个RTT减少到2个RTT——这使网络协商速度大约提高2倍。

因此，我们所有的改进都围绕一个最初未定义的陈述展开：'谁先发言'。如果客户端先发言，我们可以应用所有这些优化，并有效地将连接+握手+查询封装在一个TFO数据包中。此外，我们可以通过接收到的数据包开头确定实际协议。这就是为什么你可以通过API/http/https连接到同一个端口。如果守护进程必须先发言，所有这些优化都不可能实现，多协议也无从谈起。这就是为什么我们为MySQL专门设置了专用端口，而没有将其与其他所有协议统一到同一个端口。突然间，在所有客户端中，有一个客户端是按照守护进程先发送握手的逻辑编写的。也就是说，无法实现上述所有改进。这就是SphinxSE插件用于mysql/mariadb的情况。因此，我们专门为这个单一客户端定义了`sphinx`协议，以最传统的方式工作。即：双方激活`TCP_NODELAY`并使用小数据包进行交换。守护进程在连接时发送其握手，然后客户端发送其握手，之后一切按常规方式工作。这不是很高效，但能正常工作。如果你使用SphinxSE连接到Manticore，你必须专门设置一个监听器并明确指定`sphinx`协议。对于其他客户端，应避免使用此监听器，因为它较慢。如果你使用其他传统的Sphinx API客户端，请首先检查它们是否能够与非专用的多协议端口一起工作。对于主-代理连接使用非专用（多协议）端口并启用客户端和服务器的TFO效果良好，肯定会加快网络后端的工作效率，特别是当你有非常轻量且快速的查询时。
</details>

### listen_tfo

此设置允许所有监听器使用TCP_FASTOPEN标志。默认情况下，它由系统管理，但可以通过将其设置为'0'显式关闭。

关于TCP Fast Open扩展的通用知识，请参阅[Wikipedia](https://en.wikipedia.org/wiki/TCP_Fast_Open)。简而言之，它允许在建立连接时消除一个TCP往返时间。

在实践中，许多情况下使用TFO可以优化客户端-代理的网络效率，特别是当使用[persistent agents](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)时，即使不保持活动连接，也不受最大连接数的限制。

在现代操作系统上，TFO支持通常在系统级别启用，但这只是一个“能力”，并非规则。Linux（作为最进步的系统）自2011年起在3.7内核（服务器端）开始支持。Windows从某些Windows 10版本开始支持。其他操作系统（FreeBSD、MacOS）也参与其中。

对于Linux系统服务器，检查变量`/proc/sys/net/ipv4/tcp_fastopen`并根据其行为。第0位管理客户端，第1位管理监听器。默认情况下，系统将此参数设置为1，即客户端启用，监听器禁用。

### log

<!-- example conf log -->
log设置指定了所有`searchd`运行时事件将被记录的日志文件名称。如果未指定，默认名称为'searchd.log'。

或者，你可以使用'syslog'作为文件名。在这种情况下，事件将发送到syslog守护进程。要使用syslog选项，你需要在构建时使用`-–with-syslog`选项配置Manticore。


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

当使用[multi-queries](../Searching/Multi-queries.md)时，searchd会对单个批次提交的查询数量执行合理性检查。将其设置为0以跳过检查。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_batch_queries = 256
```
<!-- end -->

### max_connections

<!-- example max_connections -->
最大同时客户端连接数。默认情况下无限制。这通常只有在使用任何类型的持久连接时才明显，例如cli mysql会话或远程分布式表的持久远程连接。当超过限制时，你仍然可以使用[VIP连接](../Connecting_to_the_server/MySQL_protocol.md#VIP-connection)连接到服务器。VIP连接不计入限制。

<!-- request Example -->
```ini
max_connections = 10
```

<!-- end -->

### max_threads_per_query

<!-- example max_threads_per_query -->
实例范围内一个操作可以使用的线程数限制。默认情况下，适当的操作可以占用所有CPU核心，没有其他操作的余地。例如，对一个相当大的percolate表执行`call pq`可能会在数十秒内使用所有线程。将`max_threads_per_query`设置为例如[threads](../Server_settings/Searchd.md#threads)的一半，可以确保你能并行运行几个这样的`call pq`操作。

你也可以在运行时将此设置作为会话或全局变量进行设置。

此外，你可以通过使用[threads OPTION](../Searching/Options.md#threads)在每个查询的基础上控制行为。

<!-- intro -->
##### 示例：
<!-- request Example -->

```ini
max_threads_per_query = 4
```

<!-- end -->

### max_filters

<!-- example conf max_filters -->
每个查询允许的最大过滤器数量。此设置仅用于内部合理性检查，不会直接影响RAM使用或性能。可选，默认值为256。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_filters = 1024
```
<!-- end -->


### max_filter_values

<!-- example conf max_filter_values -->
每个过滤器允许的最大值数量。此设置仅用于内部合理性检查，不会直接影响RAM使用情况或性能。可选，默认值为4096。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_filter_values = 16384
```
<!-- end -->


### max_open_files

<!-- example conf max_open_files -->
服务器允许打开的最大文件数称为“软限制”。请注意，服务大型碎片化实时表可能需要将此限制设置得较高，因为每个磁盘块可能占用十几个或更多的文件。例如，一个包含1000个块的实时表可能需要同时打开数千个文件。如果在日志中遇到“Too many open files”错误，请尝试调整此选项，这可能有助于解决问题。

还有一个“硬限制”，该限制无法通过选项超过。此限制由系统定义，可以在Linux上的文件`/etc/security/limits.conf`中更改。其他操作系统可能有不同的方法，因此请参阅相关手册以获取更多信息。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = 10000
```
<!-- end -->

<!-- example conf max_open_files max -->
除了直接的数字值外，还可以使用神奇的单词“max”将限制设置为当前可用的硬限制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = max
```
<!-- end -->


### max_packet_size

<!-- example conf max_packet_size -->
允许的最大网络数据包大小。此设置限制分布式环境中客户端的查询数据包和远程代理的响应数据包。仅用于内部合理性检查，不会直接影响RAM使用情况或性能。可选，默认值为128M。


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

一些挑剔的MySQL客户端库依赖于MySQL使用的特定版本号格式，而且有时会根据报告的版本号（而不是指示的能力标志）选择不同的执行路径。例如，Python MySQLdb 1.2.2在版本号不是X.Y.ZZ格式时会抛出异常；MySQL .NET连接器6.3.x在版本号为1.x并结合某些标志组合时会在内部失败等。为了解决这个问题，可以使用`mysql_version_string`指令，并让`searchd`通过MySQL协议向客户端报告不同的版本号。（默认情况下，它报告自己的版本。）


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
mysql_version_string = 5.0.37
```
<!-- end -->


### net_workers

网络线程数，默认为1。

在查询率极高且单个线程不足以处理所有传入查询的情况下，此设置非常有用。


### net_wait_tm

控制网络线程的繁忙循环间隔。默认值为-1，可以设置为-1、0或正整数。

当服务器配置为纯主节点并仅将请求路由到代理时，重要的是要无延迟地处理请求，并且不允许网络线程睡眠。为此有一个繁忙循环。如果`net_wait_tm`是正数，则在接收到请求后，网络线程使用CPU轮询`10 * net_wait_tm`毫秒；如果`net_wait_tm`为0，则仅使用CPU轮询。此外，可以通过`net_wait_tm = -1`禁用繁忙循环 - 在这种情况下，轮询器在系统轮询调用中将超时设置为实际代理的超时。

> **警告：** CPU繁忙循环实际上会加载CPU核心，因此将此值设置为任何非默认值会导致即使在空闲服务器上也会出现明显的CPU使用率。


### net_throttle_accept

定义网络循环的每次迭代中接受的客户端数量。默认值为0（无限制），对于大多数用户来说应该没问题。这是一个微调选项，用于在高负载场景下控制网络循环的吞吐量。


### net_throttle_action

定义网络循环的每次迭代中处理的请求数量。默认值为0（无限制），对于大多数用户来说应该没问题。这是一个微调选项，用于在高负载场景下控制网络循环的吞吐量。

### network_timeout

<!-- example conf network_timeout -->
网络客户端请求的读/写超时时间，以秒为单位（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认值为5秒。`searchd`将强制关闭在超时内未能发送查询或读取结果的客户端连接。

还要注意 [reset_network_timeout_on_packet](../Server_settings/Searchd.md#reset_network_timeout_on_packet) 参数。此参数会改变`network_timeout`的行为，使其适用于每个数据包而不是整个`query`或`result`。通常，查询/结果适合一两个数据包。然而，在需要大量数据的情况下，此参数在保持操作活跃方面可能非常有价值。

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
设置默认的表压缩阈值。更多信息请参见 - [优化磁盘块数量](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks)。此设置可以通过每个查询选项 [cutoff](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 覆盖。也可以通过 [SET GLOBAL](../Server_settings/Setting_variables_online.md#SET) 动态更改。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
optimize_cutoff = 4
```
<!-- end -->

### persistent_connections_limit

<!-- example conf persistent_connections_limit -->
此设置确定与远程 [持久代理](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 的最大同时持久连接数。每次连接在 `agent_persistent` 下定义的代理时，我们尝试重用现有连接（如果有的话），或连接并保存连接以供将来使用。然而，在某些情况下，限制此类持久连接的数量是有意义的。此指令定义了限制。它会影响所有分布式表中每个代理主机的连接数量。

将此值设置为代理配置中的 [max_connections](../Server_settings/Searchd.md#max_connections) 选项相等或更小是合理的。

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

searchd 进程 ID 文件在启动时会被重新创建并锁定，并在服务器运行时包含主服务器进程 ID。服务器关闭时会解除链接。
此文件的目的是使 Manticore 能够执行各种内部任务，例如检查是否已有运行中的 `searchd` 实例、停止 `searchd` 并通知其应轮换表。该文件也可用于外部自动化脚本。


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
基于执行时间（通过最大查询时间设置）在查询完成前终止查询是一种不错的安全网，但其本身存在一个固有缺点：结果不确定（不稳定）。也就是说，如果你重复执行完全相同的（复杂）搜索查询多次，时间限制将在不同阶段被触发，你将得到 *不同的* 结果集。

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

有一个选项 [SELECT … OPTION max_predicted_time](../Searching/Options.md#max_predicted_time)，它允许你限制查询时间 *并* 获得稳定、可重复的结果。与其在评估查询时定期检查实际当前时间（这会导致不确定的结果），不如使用一个简单的线性模型来预测当前运行时间：

```ini
predicted_time =
    doc_cost * processed_documents +
    hit_cost * processed_hits +
    skip_cost * skiplist_jumps +
    match_cost * found_matches
```

当 `predicted_time` 达到给定限制时，查询将被提前终止。

当然，这并不是实际花费时间的硬性限制（不过，这是对 *处理* 工作量的硬性限制），而且一个简单的线性模型远非理想精确。因此，墙钟时间 *可能* 低于或高于目标限制。然而，误差范围相当可接受：例如，在我们以 100 毫秒为目标限制的实验中，大多数测试查询落在 95 到 105 毫秒范围内，*所有* 查询都在 80 到 120 毫秒范围内。此外，作为额外的好处，使用建模的查询时间而不是测量实际运行时间，也会减少 gettimeofday() 调用次数。

没有两台服务器的型号和规格是完全相同的，因此 `predicted_time_costs` 指令允许你为上述模型配置成本。为了方便，这些成本以整数形式表示，单位为纳秒。（max_predicted_time 中的限制以毫秒计算，而需要将成本值指定为 0.000128 毫秒而不是 128 纳秒会更容易出错。）不需要一次性指定所有四个成本，未指定的成本将采用默认值。不过，我们强烈建议为可读性全部指定它们。


### preopen_tables

<!-- example conf preopen_tables -->
preopen_tables 配置指令指定是否在启动时强制预打开所有表。默认值为 1，这意味着无论每个表的 [preopen](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Other-performance-related-settings) 设置如何，所有表都将被预打开。如果设置为 0，则每个表的设置将生效，并且它们的默认值为 0。

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
pseudo_sharding 配置选项启用对本地普通表和实时表的搜索查询并行化，无论它们是直接查询还是通过分布式表查询。此功能将自动并行化查询，最多达到 `searchd.threads` 中指定的线程数。

注意，如果您的工作线程已经很忙，因为您有以下情况：
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

`replication_connect_timeout` 指令定义了连接到远程节点的超时时间。默认情况下，该值以毫秒为单位，但可以使用[其他后缀](../Server_settings/Special_suffixes.md)。默认值为 1000（1 秒）。

在连接到远程节点时，Manticore 最多将等待此时间以成功完成连接。如果达到超时但连接尚未建立，并且启用了 `retries`，将发起重试。


### replication_query_timeout

`replication_query_timeout` 设置了 searchd 等待远程节点完成查询的时间量。默认值为 3000 毫秒（3 秒），但可以使用 `suffixed` 来表示不同的时间单位。

在建立连接后，Manticore 将等待最多 `replication_query_timeout` 的时间以让远程节点完成。请注意，此超时与 `replication_connect_timeout` 是分开的，远程节点可能引起的总延迟将是这两个值的总和。


### replication_retry_count

此设置是一个整数，指定 Manticore 在复制过程中尝试连接和查询远程节点的次数，直到报告致命查询错误。默认值为 3。


### replication_retry_delay

此设置是一个以毫秒为单位的整数（或[特殊后缀](../Server_settings/Special_suffixes.md)），指定在复制过程中发生故障时，Manticore 重试查询远程节点之前的延迟。此值仅在指定非零值时相关。默认值为 500。

### qcache_max_bytes

<!-- example conf qcache_max_bytes -->
此配置设置了分配给缓存结果集的 RAM 最大字节数。默认值为 16777216，相当于 16 兆字节。如果值设置为 0，则禁用查询缓存。有关查询缓存的更多信息，请参阅[查询缓存](../Searching/Query_cache.md)。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
qcache_max_bytes = 16777216
```
<!-- end -->


### qcache_thresh_msec

整数，以毫秒为单位。查询结果被缓存的最小墙时间阈值。默认值为 3000，即 3 秒。0 表示缓存所有内容。有关详细信息，请参阅[查询缓存](../Searching/Query_cache.md)。此值也可以使用时间[特殊后缀](../Server_settings/Special_suffixes.md)表示，但使用时要小心，不要与值本身名称中的 `_msec` 混淆。


### qcache_ttl_sec

整数，以秒为单位。缓存结果集的过期时间。默认值为 60，即 1 分钟。最小可能值为 1 秒。有关详细信息，请参阅[查询缓存](../Searching/Query_cache.md)。此值也可以使用时间[特殊后缀](../Server_settings/Special_suffixes.md)表示，但使用时要小心，不要与值本身名称中的 `_sec` 混淆。


### query_log_format

<!-- example conf query_log_format -->
查询日志格式。可选，允许的值为 `plain` 和 `sphinxql`，默认为 `sphinxql`。

`sphinxql` 模式记录有效的 SQL 语句。`plain` 模式以纯文本格式记录查询（主要适用于纯全文搜索用例）。此指令允许您在搜索服务器启动时在两种格式之间切换。日志格式也可以通过使用 `SET GLOBAL query_log_format=sphinxql` 语法在运行时更改。有关更多详细信息，请参阅[查询日志](../Logging/Query_logging.md)。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_format = sphinxql
```
<!-- end -->

### query_log_min_msec

限制（以毫秒为单位），防止查询被写入查询日志。可选，默认为 0（所有查询都会被写入查询日志）。此指令指定只有执行时间超过指定限制的查询才会被记录（此值也可以使用时间[特殊后缀](../Server_settings/Special_suffixes.md)表示，但使用时要小心，不要与值本身名称中的 `_msec` 混淆）。

### query_log

<!-- example conf query_log -->
查询日志文件名。可选，默认为空（不记录查询）。所有搜索查询（如 SELECT ... 但不包括 INSERT/REPLACE/UPDATE 查询）都会记录在此文件中。格式在[查询日志](../Logging/Query_logging.md)中描述。如果使用 'plain' 格式，可以将 'syslog' 作为日志文件的路径。在这种情况下，所有搜索查询将通过 `LOG_INFO` 优先级发送到 syslog 守护进程，并以前缀 '[query]' 代替时间戳。要使用 syslog 选项，Manticore 必须在构建时配置为 `-–with-syslog`。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log = /var/log/query.log
```
<!-- end -->


### query_log_mode

<!-- example conf query_log_mode -->
`query_log_mode` 指令允许您为 searchd 和查询日志文件设置不同的权限。默认情况下，这些日志文件以 600 权限创建，这意味着只有运行服务器的用户和 root 用户可以读取日志文件。
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
read_buffer_docs 指令控制文档列表的每个关键词读取缓冲区大小。对于每个搜索查询中的每个关键词出现，都有两个相关的读取缓冲区：一个用于文档列表，一个用于命中列表。此设置可让您控制文档列表缓冲区大小。

更大的缓冲区大小可能会增加每个查询的 RAM 使用量，但可能会减少 I/O 时间。对于慢速存储，设置较大的值是有意义的，但对于能够实现高 IOPS 的存储，应在较低值范围内进行实验。

默认值为 256K，最小值为 8K。您还可以通过 [read_buffer_docs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs) 在每个表的基础上设置，这将覆盖服务器配置级别的任何设置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_docs = 128K
```
<!-- end -->


### read_buffer_hits

<!-- example conf read_buffer_hits -->
read_buffer_hits 指令指定搜索查询中命中列表的每个关键词读取缓冲区大小。默认大小为 256K，最小值为 8K。对于每个搜索查询中的每个关键词出现，都有两个相关的读取缓冲区，一个用于文档列表，一个用于命中列表。增加缓冲区大小可能会增加每个查询的 RAM 使用量，但会减少 I/O 时间。对于慢速存储，较大的缓冲区大小是有意义的，而对于能够实现高 IOPS 的存储，应在较低值范围内进行实验。

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

在查询时，某些读取可以提前知道确切的数据量，但有些目前还不能。最明显的是命中列表大小目前无法提前知道。此设置可让您控制在这些情况下读取多少数据。它会影响命中列表的 I/O 时间，对于大于未提示读取大小的列表会减少 I/O 时间，但对于较小的列表会增加 I/O 时间。它**不会**影响 RAM 使用，因为读取缓冲区已经分配。因此，它不应大于 read_buffer。


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
当设置为 1（默认）时，超时限制网络活动之间的最大时间。

在复制的情况下，一个节点可能需要向另一个节点发送一个大文件（例如 100GB）。假设网络传输速率为 1GB/s，每个数据包为 4-5MB。要传输整个文件，需要 100 秒。默认超时为 5 秒，只能允许传输 5GB 后连接断开。增加超时可能是一个变通方法，但不可扩展（例如，下一个文件可能是 150GB，再次导致失败）。但是，默认 `reset_network_timeout_on_packet` 设置为 1 时，超时不是应用于整个传输，而是应用于单个数据包。只要传输正在进行（在超时期间实际通过网络接收数据），连接就会保持活跃。如果传输卡住，导致数据包之间发生超时，连接将被丢弃。

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

即使主动更新的 RT 表完全适合 RAM 块，仍可能导致不断增长的 binlog，影响磁盘使用和崩溃恢复时间。通过此指令，搜索服务器执行定期刷新检查，符合条件的 RAM 块可以保存，从而实现后续的 binlog 清理。有关详细信息，请参阅 [Binary logging](../Logging/Binary_logging.md)。

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

此指令可限制由 `OPTIMIZE` 语句引起的 I/O 影响。可以保证所有 RT 优化活动不会生成超过配置限制的磁盘 IOPS（每秒 I/O 操作数）。限制 rt_merge_iops 可减少合并引起的搜索性能下降。

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

此指令可限制由 `OPTIMIZE` 语句引起的 I/O 影响。超过此限制的 I/O 将被分解为两个或多个 I/O，然后将根据 [rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops) 限制作为单独的 I/O 进行计算。因此，可以保证所有优化活动不会生成超过 (rt_merge_iops * rt_merge_maxiosize) 字节的磁盘 I/O 每秒。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_maxiosize = 1M
```
<!-- end -->


### seamless_rotate

<!-- example conf seamless_rotate -->
防止 `searchd` 在旋转包含大量数据的表时出现停滞。可选，缺省值为 1（启用无缝旋转）。在 Windows 系统中，无缝旋转默认是禁用的。

表中可能包含需要预加载到 RAM 中的一些数据。目前，`.spa`、`.spb`、`.spi` 和 `.spm` 文件会被完全预加载（它们分别包含属性数据、二进制属性数据、关键字表和已删除行映射）。如果没有启用无缝旋转，旋转表时会尽量少使用 RAM，并按以下步骤工作：

1. 新查询暂时被拒绝（返回 "retry" 错误码）；
2. `searchd` 等待所有当前运行的查询完成；
3. 旧表被释放，其文件被重命名；
4. 新表文件被重命名，并分配所需的 RAM；
5. 新表的属性和字典数据被预加载到 RAM；
6. `searchd` 恢复从新表提供查询服务。

然而，如果属性或字典数据很多，预加载步骤可能需要明显的时间——在预加载 1-5+ GB 文件的情况下，可能需要几分钟。

启用无缝旋转后，旋转过程如下：

1. 为新表分配 RAM 存储；
2. 新表的属性和字典数据异步预加载到 RAM；
3. 成功时，旧表被释放，两个表的文件被重命名；
4. 失败时，新表被释放；
5. 在任何时刻，查询都从旧表或新表副本中提供服务。

无缝旋转的代价是在旋转期间峰值内存使用量更高（因为旧表和新表的 `.spa/.spb/.spi/.spm` 数据副本都需要在 RAM 中，直到新副本预加载完成）。平均使用量保持不变。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
seamless_rotate = 1
```
<!-- end -->

### secondary_index_block_cache
<!-- example conf secondary_index_block_cache -->

此选项指定用于二级索引的块缓存大小。它是可选的，默认为 8 MB。当二级索引使用包含许多值的过滤器（例如 IN() 过滤器）时，它们会读取和处理这些值的元数据块。
在连接查询中，此过程对左表的每一行批次重复进行，每个批次可能在单个连接查询中重新读取相同的元数据。这会严重影响性能。元数据块缓存将这些块保留在内存中，以便后续批次可以重复使用。
缓存仅在连接查询中使用，对非连接查询没有影响。请注意，缓存大小限制是按属性和每个二级索引分别应用的。每个磁盘块中的每个属性都在此限制内运行。在最坏情况下，总内存使用量可以通过将限制乘以磁盘块数量和连接查询中使用的属性数量来估算。

将 `secondary_index_block_cache = 0` 可禁用缓存。
##### 示例：

secondary_index_block_cache = 16M

<!-- intro -->
### secondary_indexes

<!-- request Example -->

```ini
此选项启用或禁用搜索查询中使用二级索引。它是可选的，默认为 1（启用）。请注意，索引时不需要启用它，只要安装了 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)，它就始终启用。后者也是在搜索时使用索引所必需的。有三种模式可用：
```

<!-- end -->

`0`：在搜索中禁用二级索引。可以使用 [分析器提示](../Searching/Options.md#Query-optimizer-hints) 为单个查询启用它们
<!-- example conf secondary_indexes -->

`1`：在搜索中启用二级索引。可以使用 [分析器提示](https://github.com/manticoresoftware/columnar) 为单个查询禁用它们

* `force`：与启用相同，但在加载二级索引时发生的任何错误都会被报告，并且整个索引不会被加载到守护进程中。
* ##### 示例：
* CODE_BLOCK_74

<!-- intro -->
### server_id

<!-- request Example -->

```ini
作为服务器标识符的整数，用于生成节点的唯一短 UUID，这些节点属于复制集群。server_id 必须在集群的节点中唯一，并且范围在 0 到 127 之间。如果未设置 server_id，它将被计算为 MAC 地址和 PID 文件路径的哈希，或者使用随机数作为短 UUID 的种子。
```

<!-- end -->

##### 示例：

<!-- example conf server_id -->
server_id = 1


<!-- intro -->
### shutdown_timeout

<!-- request Example -->

```ini
`searchd --stopwait` 等待时间，以秒为单位（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，缺省值为 60 秒。
```
<!-- end -->


当你运行 `searchd --stopwait` 时，服务器需要在停止前执行一些操作，例如完成查询、刷新 RT RAM 块、刷新属性和更新 binlog。这些任务需要一些时间。`searchd --stopwait` 将等待最多 `shutdown_time` 秒，直到服务器完成其任务。合适的时间取决于你的表大小和负载。

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


### snippets_file_prefix

生成片段时添加到本地文件名前的前缀。可选，缺省值为当前工作目录。

此前缀可以与 `load_files` 或 `load_files_scattered` 选项一起用于分布式片段生成。

<!-- example conf snippets_file_prefix -->
A prefix to prepend to the local file names when generating snippets. Optional, default is the current working folder.

This prefix can be used in distributed snippets generation along with `load_files` or `load_files_scattered` options.

注意，这是一个前缀，**而不是**路径！这意味着如果前缀设置为 "server1"，而请求引用了 "file23"，`searchd` 将尝试打开 "server1file23"（所有内容都不带引号）。因此，如果你需要它作为路径，必须包含末尾的斜杠。

在构造最终文件路径后，服务器会解除所有相对目录，并将最终结果与 `snippet_file_prefix` 的值进行比较。如果结果不以该前缀开头，此类文件将因错误信息被拒绝。

例如，如果你将其设置为 `/mnt/data`，而有人使用文件 `../../../etc/passwd` 作为源来调用片段生成，他们将收到以下错误信息：

`File '/mnt/data/../../../etc/passwd' escapes '/mnt/data/' scope`

而不是文件内容。

另外，如果参数未设置且读取 `/etc/passwd`，实际上会读取 /daemon/working/folder/etc/passwd，因为该参数的默认值是服务器的工作目录。

还要注意，这是一个本地选项；它不会以任何方式影响代理。因此，你可以在主服务器上安全地设置前缀。路由到代理的请求不会受到主服务器设置的影响。但是，它们会受到代理自身的设置影响。

这在文档存储位置（无论是本地存储还是 NAS 挂载点）在服务器之间不一致时可能会很有用。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
snippets_file_prefix = /mnt/common/server1/
```
<!-- end -->

> **警告：** 如果你仍然想访问文件系统根目录下的文件，必须显式地将 `snippets_file_prefix` 设置为空值（通过 `snippets_file_prefix=` 行），或设置为根目录（通过 `snippets_file_prefix=/`）。


### sphinxql_state

<!-- example conf sphinxql_state -->
当前 SQL 状态将被序列化的文件路径。

服务器启动时，此文件会被重放。在符合条件的状态更改（例如，`SET GLOBAL`）时，此文件会自动被重写。这可以防止难以诊断的问题：如果你加载了 UDF 函数但 Manticore 崩溃，当它（自动）重新启动时，你的 UDF 和全局变量将不再可用。使用持久状态有助于确保优雅的恢复，避免此类意外。

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

此设置限制了公共子树优化器的内存使用（参见 [多查询](../Searching/Multi-queries.md)）。每个查询最多将使用这么多内存来缓存文档条目。将限制设置为 0 会禁用优化器。


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

此设置限制了公共子树优化器的内存使用（参见 [多查询](../Searching/Multi-queries.md)）。每个查询最多将使用这么多内存来缓存关键字出现次数（命中）。将限制设置为 0 会禁用优化器。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
subtree_hits_cache = 16M
```
<!-- end -->

### threads

<!-- example threads -->
Manticore 守护进程的工作线程数（或线程池大小）。Manticore 在启动时创建此数量的 OS 线程，它们在守护进程中执行所有任务，例如执行查询、创建片段等。某些操作可能会被拆分为子任务并并行执行，例如：

* 在实时表中搜索
* 在由本地表组成的分布式表中搜索
* 激活查询调用
* 其他操作

默认情况下，它设置为服务器上的 CPU 核心数。Manticore 在启动时创建线程，并在停止前一直保留它们。每个子任务在需要时可以使用其中一个线程。当子任务完成时，它会释放线程，以便另一个子任务可以使用它。

在 I/O 密集型负载的情况下，将值设置为高于 CPU 核心数可能有意义。

<!-- request Example -->
```ini
threads = 10
```

<!-- end -->

### thread_stack

<!-- example conf thread_stack -->
作业（协程，一个搜索查询可能导致多个作业/协程）的最大堆栈大小。可选，默认为 128K。

每个任务都有自己的128KB堆栈。当运行查询时，系统会检查所需堆栈的大小。如果默认的128KB足够，就进行处理。如果需要更多堆栈，系统会启动另一个堆栈，继续处理。高级堆栈的最大大小受该设置限制。

设置合理较高的堆栈率有助于处理非常深的查询，但不会暗示整体内存消耗会增长。例如，将堆栈率设置为1GB并不意味着每个新任务都需要1GB的内存，但如果发现某个任务需要100MB的堆栈，就为该任务分配100MB。同时，其他任务在默认128KB的堆栈下运行。同样，我们也可以处理需要500MB堆栈的复杂查询。只有当系统内部检测到该任务需要超过1GB的堆栈时，才会失败并报告线程栈不足。

然而，在实际应用中，即使需要16MB堆栈的查询通常过于复杂，且消耗过多时间和资源，系统仍会处理这些查询。因此，通过设置线程栈限制，这种查询的处理方式显得合理。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
thread_stack = 8M
```
<!-- end -->


### unlink_old

<!-- example conf unlink_old -->
决定在成功旋转 `.old` 表副本后是否进行删除。可选，默认为1（删除）。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
unlink_old = 0
```
<!-- end -->


### watchdog

<!-- example conf watchdog -->
带线程监控的服务器日志监控器。可选，默认为1（日志启用）。

当Manticore查询崩溃时，服务器会自动终止。启用日志功能后，`searchd` 也会运行一个轻量级进程，监控主服务器进程并自动重启。日志功能默认启用。

<!-- request Example -->

```ini
watchdog = 0 # disable watchdog
```
<!-- end -->
<!-- proofread -->
