# 在配置中的“searchd”部分

以下设置应在 Manticore Search 配置文件的 `searchd` 部分中使用，以控制服务器的行为。以下是每个设置的摘要：

### access_plain_attrs

该设置为 access_plain_attrs 设置实例级默认值。可选，默认值为 `mmap_preread`。

`access_plain_attrs` 指令允许您为此 searchd 实例管理的所有表定义 access_plain_attrs 的默认值。每个表的指令优先级更高，并将覆盖此实例级默认值，以提供更细粒度的控制。

### access_blob_attrs

该设置为 [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#通过CREATE-TABLE创建实时表的示例) 设置实例级默认值。可选，默认值为 `mmap_preread`。

`access_blob_attrs` 指令允许您为此 searchd 实例管理的所有表定义 [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#通过CREATE-TABLE创建实时表的示例) 的默认值。每个表的指令优先级更高，并将覆盖此实例级默认值，以提供更细粒度的控制。

### access_doclists

该设置为 [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#通过CREATE-TABLE创建实时表的示例) 设置实例级默认值。可选，默认值为 `file`。

`access_doclists` 指令允许您为此 searchd 实例管理的所有表定义 [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#通过CREATE-TABLE创建实时表的示例) 的默认值。每个表的指令优先级更高，并将覆盖此实例级默认值，以提供更细粒度的控制。

### access_hitlists

该设置为 [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#通过CREATE-TABLE创建实时表的示例) 设置实例级默认值。可选，默认值为 `file`。

`access_hitlists` 指令允许您为此 searchd 实例管理的所有表定义 [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#通过CREATE-TABLE创建实时表的示例) 的默认值。每个表的指令优先级更高，并将覆盖此实例级默认值，以提供更细粒度的控制。

### access_dict

该设置为 [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#通过CREATE-TABLE创建实时表的示例) 设置实例级默认值。可选，默认值为 `mmap_preread`。

`access_dict` 指令允许您为此 searchd 实例管理的所有表定义 [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#通过CREATE-TABLE创建实时表的示例) 的默认值。每个表的指令优先级更高，并将覆盖此实例级默认值，以提供更细粒度的控制。

### agent_connect_timeout

该设置为 [agent_connect_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) 参数设置实例级默认值。


### agent_query_timeout

该设置为 [agent_query_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) 参数设置实例级默认值。可以使用 `OPTION agent_query_timeout=XXX` 子句在每个查询基础上进行覆盖。


### agent_retry_count

该设置是一个整数，指定 Manticore 在报告致命查询错误之前将尝试连接和查询远程代理的次数。默认值为 0（即无重试）。您也可以在每个查询基础上使用 `OPTION retry_count=XXX` 子句设置此值。如果提供了每个查询选项，它将覆盖配置中指定的值。

请注意，如果在分布式表的定义中使用了 [agent mirrors](../Creating_a_cluster/Remote_nodes/Mirroring.md#代理镜像)，服务器将在每次连接尝试时根据所选的 [ha_strategy](../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) 选择不同的镜像。在这种情况下，`agent_retry_count` 将对一组中的所有镜像进行聚合。

例如，如果您有 10 个镜像并设置 `agent_retry_count=5`，服务器将尝试最多 50 次，假设对每个 10 个镜像平均尝试 5 次（使用 `ha_strategy = roundrobin` 选项时，情况就是如此）。

但是，作为 [agent](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) 的 `retry_count` 选项提供的值作为绝对限制。换句话说，在代理定义中的 `[retry_count=2]` 选项始终意味着最多 2 次尝试，无论您为代理指定了 1 个还是 10 个镜像。

### agent_retry_delay

该设置是以毫秒为单位的整数（或 [special_suffixes](../Server_settings/Special_suffixes.md)），指定在 Manticore 重试查询远程代理时的延迟。此值仅在指定了非零的 [agent_retry_count](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 或非零的每个查询 `retry_count` 时相关。默认值为 500。您也可以在每个查询基础上使用 `OPTION retry_delay=XXX` 子句设置此值。如果提供了每个查询选项，它将覆盖配置中指定的值。


### attr_flush_period

<!-- example conf attr_flush_period -->auto_optimize

<!-- example conf auto_optimize -->
This setting controls the automatic [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) process for table compaction.

By default table compaction occurs automatically. You can modify this behavior with the `auto_optimize` setting:

* 0 to disable automatic table compaction (you can still call `OPTIMIZE` manually)
* 1 to explicitly enable it
* N to enable it, but allow OPTIMIZE to start when the number of disk chunks is greater than `# of CPU cores * 2 * N`

Note that toggling `auto_optimize` on or off doesn't prevent you from running [OPTIMIZE TABLE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) manually.

<!-- intro -->

##### Example:

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
Manticore supports the automatic creation of tables that don't yet exist but are specified in INSERT statements. This feature is enabled by default. To disable it, set `auto_schema = 0` explicitly in your configuration. To re-enable it, set `auto_schema = 1` or remove the `auto_schema` setting from the configuration.

Keep in mind that the `/bulk` HTTP endpoint does not support automatic table creation.

> NOTE: The [auto schema functionality](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#自动模式) requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

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
This setting controls the binary log transaction flush/sync mode. It is optional, with a default value of 2 (flush every transaction, sync every second).

The directive determines how frequently the binary log will be flushed to the OS and synced to disk. There are three supported modes:

*  0, flush and sync every second. This offers the best performance, but up to 1 second worth of committed transactions can be lost in the event of a server crash or an OS/hardware crash.
*  1, flush and sync every transaction. This mode provides the worst performance but guarantees that every committed transaction's data is saved.
*  2, flush every transaction, sync every second. This mode delivers good performance and ensures that every committed transaction is saved in case of a server crash. However, in the event of an OS/hardware crash, up to 1 second worth of committed transactions can be lost.

For those familiar with MySQL and InnoDB, this directive is similar to `innodb_flush_log_at_trx_commit`. In most cases, the default hybrid mode 2 provides a nice balance of speed and safety, with full RT table data protection against server crashes and some protection against hardware ones.

<!-- intro -->

##### Example:

<!-- request Example -->

```ini
binlog_flush = 1 # ultimate safety, low speed
```

<!-- end -->


### binlog_max_log_size

<!-- example conf binlog_max_log_size -->
This setting controls the maximum binary log file size. It is optional, with a default value of 256 MB.

A new binlog file will be forcibly opened once the current binlog file reaches this size limit. This results in a finer granularity of logs and can lead to more efficient binlog disk usage under certain borderline workloads. A value of 0 indicates that the binlog file should not be reopened based on size.


<!-- intro -->

##### Example:

<!-- request Example -->

```ini
binlog_max_log_size = 16M
```

<!-- end -->


### binlog_path

<!-- example conf binlog_path -->
This setting determines the path for binary log (also known as transaction log) files. It is optional, with a default value of the build-time configured data directory (e.g., `/var/lib/manticore/data/binlog.*` in Linux).

Binary logs are used for crash recovery of RT table data and for attribute updates of plain disk indices that would otherwise only be stored in RAM until flush. When logging is enabled, every transaction COMMIT-ted into an RT table is written into a log file. Logs are then automatically replayed on startup after an unclean shutdown, recovering the logged changes.

The `binlog_path` directive specifies the location of binary log files. It should only contain the path; `searchd` will create and unlink multiple `binlog.*` files in the directory as necessary (including binlog data, metadata, and lock files, etc).

An empty value disables binary logging, which improves performance but puts the RT table data at risk.


<!-- intro -->

##### Example:

<!-- request Example -->

```ini
binlog_path = # disable logging
binlog_path = /var/lib/manticore/data # /var/lib/manticore/data/binlog.001 etc will be created
```

<!-- end -->


### buddy_path

<!-- example conf buddy_path -->
This setting determines the path to the Manticore Buddy binary. It is optional, with a default value being the build-time configured path, which varies across different operating systems. Typically, you don't need to modify this setting. However, it may be useful if you wish to run Manticore Buddy in debug mode, make changes to Manticore Buddy, or implement a new plugin. In the latter case, you can `git clone` Buddy from https://github.com/manticoresoftware/manticoresearch-buddy, add a new plugin to the directory `./plugins/`, and run `composer install --prefer-source` for easier development after you change the directory to the Buddy source.

To ensure you can run `composer`, your machine must have PHP 8.2 or higher installed with the following extensions:

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

You can also opt for the special `manticore-executor-dev` version for Linux amd64 available in the releases, for example: https://github.com/manticoresoftware/executor/releases/tag/v1.0.13

If you go this route, remember to link the dev version of the manticore executor to `/usr/bin/php`.

To disable Manticore Buddy, set the value to empty as shown in the example.

<!-- intro -->

##### Example:

<!-- request Example -->

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
This setting determines the maximum time to wait between requests (in seconds or [special_suffixes](../Server_settings/Special_suffixes.md)) when using persistent connections. It is optional, with a default value of five minutes.


<!-- intro -->

##### 示例：

<!-- request Example -->

```ini
client_timeout = 1h
```

<!-- end -->


### collation_libc_locale

<!-- example conf collation_libc_locale -->
服务器 libc 语言环境。可选，默认值为 C。

指定 libc 语言环境，影响基于 libc 的排序。有关详细信息，请参阅 [collations](../Searching/Collations.md) 部分。

<!-- intro -->

<!-- intro -->

##### 示例：

<!-- request Example -->

```ini
attr_flush_period = 900 # persist updates to disk every 15 minutes
```
<!-- end -->

### auto_optimize

<!-- example conf auto_optimize -->

该设置控制表压缩的自动 [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 过程。

默认情况下，表压缩会自动发生。您可以使用 `auto_optimize` 设置修改此行为：

- 0 禁用自动表压缩（您仍然可以手动调用 `OPTIMIZE`）
- 1 显式启用它
- N 启用它，但允许在磁盘块数量大于 `# CPU 核心 * 2 * N` 时开始 OPTIMIZE

请注意，切换 `auto_optimize` 的开关不会阻止您手动运行 [OPTIMIZE TABLE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE)。

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

Manticore 支持自动创建尚不存在但在 INSERT 语句中指定的表。该功能默认启用。要禁用它，请在配置中显式设置 `auto_schema = 0`。要重新启用它，请设置 `auto_schema = 1` 或从配置中移除 `auto_schema` 设置。

请注意，`/bulk` HTTP 端点不支持自动表创建。

> 注意：[auto schema 功能](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#自动模式) 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不工作，请确保 Buddy 已安装。

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

该设置控制二进制日志事务的刷新/同步模式。可选，默认值为 2（每个事务刷新，每秒同步一次）。

该指令决定二进制日志刷新到操作系统并同步到磁盘的频率。有三种支持的模式：

- 0，每秒刷新和同步。这提供了最佳性能，但在服务器崩溃或操作系统/硬件崩溃时可能会丢失最多 1 秒的已提交事务。
- 1，每个事务刷新和同步。这种模式提供最差的性能，但保证每个已提交事务的数据被保存。
- 2，每个事务刷新，每秒同步。这种模式提供良好的性能，并确保在服务器崩溃时保存每个已提交事务的数据。然而，在操作系统/硬件崩溃时，可能会丢失最多 1 秒的已提交事务。

对于熟悉 MySQL 和 InnoDB 的用户，这个指令类似于 `innodb_flush_log_at_trx_commit`。在大多数情况下，默认的混合模式 2 提供了速度和安全性的良好平衡，完全保护 RT 表数据免受服务器崩溃的影响，并在一定程度上保护硬件。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
binlog_flush = 1 # ultimate safety, low speed
```
<!-- end -->


### binlog_max_log_size

<!-- example conf binlog_max_log_size -->

该设置控制最大二进制日志文件大小。可选，默认值为 256 MB。

一旦当前的 binlog 文件达到此大小限制，将强制打开一个新的 binlog 文件。这导致日志的粒度更细，并在某些临界工作负载下可以更有效地使用 binlog 磁盘。值为 0 表示不应根据大小重新打开 binlog 文件。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
binlog_max_log_size = 16M
```
<!-- end -->


### binlog_path

<!-- example conf binlog_path -->

该设置确定二进制日志（也称为事务日志）文件的路径。可选，默认值为构建时配置的数据目录路径（例如，在 Linux 中为 `/var/lib/manticore/data/binlog.*`）。

二进制日志用于 RT 表数据的崩溃恢复和对平面磁盘索引的属性更新，否则这些数据将在刷新之前仅存储在 RAM 中。当启用日志记录时，每个 COMMIT 事务写入 RT 表的日志文件。然后，在不干净的关闭后，日志会在启动时自动重放，以恢复已记录的更改。

`binlog_path` 指令指定二进制日志文件的位置。它应该只包含路径；`searchd` 将根据需要在目录中创建和删除多个 `binlog.*` 文件（包括 binlog 数据、元数据和锁文件等）。

空值禁用二进制日志，这会提高性能，但将 RT 表数据置于风险之中。


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

该设置确定 Manticore Buddy 二进制文件的路径。可选，默认值为构建时配置的路径，该路径在不同操作系统之间有所不同。通常情况下，您不需要修改此设置。但是，如果您希望以调试模式运行 Manticore Buddy、对 Manticore Buddy 进行更改或实现新插件，则可能会有用。在后者的情况下，您可以从 https://github.com/manticoresoftware/manticoresearch-buddy 克隆 Buddy，将新插件添加到目录 `./plugins/`，并在更改目录到 Buddy 源之后运行 `composer install --prefer-source` 以方便开发。

要确保您可以运行 `composer`，您的机器必须安装 PHP 8.2 或更高版本，并带有以下扩展：

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

您还可以选择用于 Linux amd64 的特殊 `manticore-executor-dev` 版本，示例： https://github.com/manticoresoftware/executor/releases/tag/v1.0.13

如果您选择这种方式，请记得将 manticore executor 的开发版本链接到 `/usr/bin/php`。

要禁用 Manticore Buddy，请将值设置为空，如示例所示。

<!-- intro -->
##### 示例：

<!-- request Example -->

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
该设置确定在使用持久连接时请求之间的最大等待时间（以秒或 [special_suffixes](../Server_settings/Special_suffixes.md) 为单位）。可选，默认值为五分钟。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
client_timeout = 1h
```
<!-- end -->


### collation_libc_locale

<!-- example conf collation_libc_locale -->
服务器 libc 语言环境。可选，默认值为 C。

指定 libc 语言环境，影响基于 libc 的排序。有关详细信息，请参阅 [collations](../Searching/Collations.md) 部分。

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

指定用于传入请求的默认排序规则。可以在每个查询的基础上覆盖排序规则。有关可用排序规则的列表和其他详细信息，请参阅 [collations](../Searching/Collations.md) 部分。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
collation_server = utf8_ci
```
<!-- end -->


### data_dir

<!-- example conf data_dir -->
指定后，此设置启用 [实时模式](../Creating_a_table/Local_tables.md#在线模式管理（实时模式）)，这是一种管理数据架构的必要方式。值应为要存储所有表、二进制日志以及在此模式下正常运行 Manticore Search 所需的其他内容的目录路径。 指定 `data_dir` 时，不允许对 [普通表](../Creating_a_table/Local_tables/Plain_table.md) 进行索引。有关 RT 模式和普通模式之间差异的更多信息，请参阅 [本节](../Read_this_first.md#实时表-vs-普通表)。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
data_dir = /var/lib/manticore
```
<!-- end -->

### docstore_cache_size

<!-- example conf docstore_cache_size -->

此设置指定文档存储中保留在内存中的文档块的最大大小。它是可选的，默认值为 16m（16 兆字节）。

当使用 `stored_fields` 时，文档块将从磁盘读取并解压缩。由于每个块通常包含多个文档，因此在处理下一个文档时，可以重用它。为此，块被保存在服务器范围的缓存中。缓存中保存的是未压缩的块。


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

此设置确定单个通配符扩展的最大关键字数量。它是可选的，默认值为 0（没有限制）。

在对启用了 `dict = keywords` 的表执行子字符串搜索时，单个通配符可能会导致成千上万甚至数百万个匹配的关键字（想象一下将 `a*` 与整个牛津词典进行匹配）。此指令允许您限制此类扩展的影响。设置 `expansion_limit = N` 将扩展限制为最多 N 个最常见的匹配关键字（每个查询中的每个通配符）。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_limit = 16
```
<!-- end -->

### expansion_merge_threshold_docs

<!-- example conf expansion_merge_threshold_docs -->

此设置确定扩展关键字中允许合并所有此类关键字的最大文档数量。它是可选的，默认值为 32。

在对启用了 `dict = keywords` 的表执行子字符串搜索时，单个通配符可能会导致成千上万甚至数百万个匹配的关键字。此指令允许您增加合并的关键字数量限制，以加快匹配速度，但会在搜索中使用更多内存。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_merge_threshold_docs = 1024
```
<!-- end -->

### expansion_merge_threshold_hits

<!-- example conf expansion_merge_threshold_hits -->

此设置确定扩展关键字中允许合并所有此类关键字的最大命中数量。它是可选的，默认值为 256。

在对启用了 `dict = keywords` 的表执行子字符串搜索时，单个通配符可能会导致成千上万甚至数百万个匹配的关键字。此指令允许您增加合并的关键字数量限制，以加快匹配速度，但会在搜索中使用更多内存。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_merge_threshold_hits = 512
```
<!-- end -->

### grouping_in_utc

此设置指定 API 和 SQL 中的时间分组是根据本地时区还是 UTC 计算。它是可选的，默认值为 0（表示“本地时区”）。

默认情况下，所有“按时间分组”表达式（例如在 API 中按天、周、月和年分组，以及在 SQL 中按天、月、年、年月、年月日分组）都是使用本地时间进行的。例如，如果您有具有属性时间 `13:00 utc` 和 `15:00 utc` 的文档，在分组的情况下，它们都将根据您的本地时区设置落入相同的设施组。如果您生活在 `utc`，它将是同一天，但如果您生活在 `utc+10`，那么这些文档将被匹配到不同的“按天分组”的设施组中（因为 13:00 utc 在 UTC+10 时区是 23:00 本地时间，但 15:00 是次日的 01:00）。有时这种行为是不可接受的，期望使时间分组不依赖于时区。您可以在定义的全局 TZ 环境变量下运行服务器，但这将影响不仅仅是分组，还会影响日志中的时间戳，这也可能是不可取的。启用此选项（在配置中或使用 SQL 中的 [SET global](../Server_settings/Setting_variables_online.md#SET) 语句）将导致所有时间分组表达式在 UTC 中计算，而将其他时间相关的功能（即服务器的日志记录）保留在本地时区中。


### timezone

此设置指定日期/时间相关函数使用的时区。默认情况下使用本地时区，但您可以在 IANA 格式中指定不同的时区（例如 `Europe/Amsterdam`）。

请注意，此设置对日志记录没有影响，日志始终在本地时区下操作。

另外，请注意，如果使用 `grouping_in_utc`，则“按时间分组”功能仍将使用 UTC，而其他日期/时间相关函数将使用指定的时区。总体而言，不建议将 `grouping_in_utc` 和 `timezone` 混合使用。

您可以在配置中配置此选项，也可以使用 SQL 中的 [SET global](../Server_settings/Setting_variables_online.md#SET) 语句。


### ha_period_karma

<!-- example conf ha_period_karma -->

此设置指定代理镜像统计窗口大小，以秒为单位（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。它是可选的，默认值为 60 秒。

对于具有代理镜像的分布式表（更多信息见 [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)，主节点跟踪多个不同的每个镜像计数器。这些计数器用于故障转移和平衡（主节点根据计数器选择最佳镜像）。计数器以 `ha_period_karma` 秒为单位累积在块中。

开始新的块后，主节点仍然可以使用来自前一个块的累积值，直到新块填满一半。因此，任何先前的历史记录在最多 1.5 倍的 ha_period_karma 秒后停止影响镜像选择。

尽管最多使用两个块进行镜像选择，但最多存储最后 15 个块以供仪器检查。这些块可以使用 [SHOW AGENT STATUS](../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS) 语句检查。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_period_karma = 2m
```
<!-- end -->


### ha_ping_interval

<!-- example conf ha_ping_interval -->

此设置配置代理镜像之间的ping间隔，单位为毫秒（或[特殊后缀](../Server_settings/Special_suffixes.md)）。这是可选的，默认值为1000毫秒。

对于包含代理镜像的分布式表（更多信息请参见[代理](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)），主节点在空闲期间向所有镜像发送ping命令。这是为了跟踪当前代理的状态（存活或死亡，网络往返时间等）。此类ping之间的间隔由此指令定义。要禁用ping，请将ha_ping_interval设置为0。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_ping_interval = 3s
```
<!-- end -->


### hostname_lookup

`hostname_lookup`选项定义了更新主机名的策略。默认情况下，代理主机名的IP地址在服务器启动时被缓存，以避免过多访问DNS。然而，在某些情况下，IP地址可能会动态变化（例如，云托管），这时可能希望不缓存IP。将此选项设置为`request`将禁用缓存，并在每次查询时查询DNS。IP地址也可以使用`FLUSH HOSTNAMES`命令手动更新。

### jobs_queue_size

jobs_queue_size设置定义了同时可以在队列中的“作业”数量。默认情况下是无限制的。

在大多数情况下，“作业”意味着对单个本地表（普通表或实时表的磁盘块）的一次查询。例如，如果你有一个包含2个本地表的分布式表或一个具有2个磁盘块的实时表，对其中任何一个的搜索查询通常会将2个作业放入队列。然后，线程池（其大小由[threads](../Server_settings/Searchd.md#threads)定义）将处理这些作业。然而，在某些情况下，如果查询过于复杂，可能会创建更多作业。当[max_connections](../Server_settings/Searchd.md#max_connections)和[threads](../Server_settings/Searchd.md#threads)不足以找到所需性能之间的平衡时，建议更改此设置。

### listen_backlog

<!-- example conf listen_backlog -->
listen_backlog设置决定了传入连接的TCP监听队列的长度。这在处理请求逐一处理的Windows构建中特别相关。当连接队列达到其限制时，新的传入连接将被拒绝。 对于非Windows构建，默认值应该工作良好，通常无需调整此设置。

<!-- intro -->

##### 示例：

<!-- request Example -->

```ini
listen_backlog = 20
```
<!-- end -->


### listen

<!-- example conf listen -->

此设置允许你指定Manticore将接受连接的IP地址和端口，或Unix域套接字路径。

`listen`的通用语法是：

```ini
listen = ( address ":" port | port | path | address ":" port start - port end ) [ ":" protocol [ "_vip" ] [ "_readonly" ] ]
```

你可以指定：

- 一个IP地址（或主机名）和一个端口号
- 或者仅一个端口号
- 或者一个Unix套接字路径（在Windows上不支持）
- 或者一个IP地址和端口范围

如果你指定了端口号但没有地址，`searchd`将监听所有网络接口。Unix路径以斜杠开头。端口范围仅可为复制协议设置。

你还可以指定用于该套接字连接的协议处理程序（监听器）。监听器包括：

- 未指定

   \- Manticore将接受来自以下来源的连接：

  - 其他Manticore代理（即远程分布式表）
  - 通过HTTP和HTTPS的客户端
  - [Manticore Buddy](https://manticoresearch.com/blog/manticoresearch-buddy-intro/)。**确保你有这种类型的监听器（或下面提到的`http`监听器），以避免Manticore功能的限制。**

- ```
  mysql
  ```

   MySQL协议，用于来自MySQL客户端的连接。注意：

  - 也支持压缩协议。
  - 如果启用了[SSL](../Security/SSL.md#SSL)，你可以进行加密连接。

- `replication` - 节点通信使用的复制协议。更多细节请参见[复制](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md)部分。你可以指定多个复制监听器，但它们必须都监听同一IP；只有端口可以不同。

- `http` - 与**未指定**相同。Manticore将接受来自远程代理和客户端的HTTP和HTTPS连接。

- `https` - HTTPS协议。Manticore将仅在此端口接受HTTPS连接。更多细节请参见[SSL](../Security/SSL.md)部分。

- `sphinx` - 旧版二进制协议。用于服务来自远程[SphinxSE](../Extensions/SphinxSE.md)客户端的连接。一些Sphinx API客户端实现（例如Java实现）要求明确声明监听器。

为客户端协议添加后缀`_vip`（即所有除`replication`外的协议，例如`mysql_vip`或`http_vip`或仅`_vip`）会强制为连接创建专用线程，以绕过不同的限制。这在节点维护时非常有用，在严重过载的情况下，服务器可能会停止响应或无法通过常规端口连接。

后缀`_readonly`为监听器设置[只读模式](../Security/Read_only.md)，并限制其仅接受读查询。

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

可以有多个`listen`指令。`searchd`将在所有指定的端口和套接字上监听客户端连接。Manticore软件包中提供的默认配置定义在以下端口上监听：

- `9308`和`9312`用于来自远程代理和非MySQL客户端的连接
- `9306`用于MySQL连接。

如果你在配置中根本不指定任何`listen`，Manticore将等待以下连接：

- `127.0.0.1:9306`用于MySQL客户端
- `127.0.0.1:9312`用于HTTP/HTTPS和来自其他Manticore节点及基于Manticore二进制API的客户端的连接。

#### 在特权端口上监听

默认情况下，Linux不允许你让Manticore在1024以下的端口上监听（例如，`listen = 127.0.0.1:80:http`或`listen = 127.0.0.1:443:https`），除非你以root身份运行searchd。如果你仍然希望能够启动Manticore，使其在非root用户下监听<1024的端口，可以考虑执行以下操作（这两种方法都应该有效）：

- 运行命令`setcap CAP_NET_BIND_SERVICE=+eip /usr/bin/searchd`
- 将`AmbientCapabilities=CAP_NET_BIND_SERVICE`添加到Manticore的systemd单元并重新加载守护程序（`systemctl daemon-reload`）。

#### Sphinx API 协议和 TFO 的技术细节
<details>
遗留的 Sphinx 协议分为两个阶段：握手交换和数据流。握手由客户端的 4 字节数据包和守护进程的 4 字节数据包组成，其唯一目的是客户端确定远程是否为真实的 Sphinx 守护进程，守护进程确定远程是否为真实的 Sphinx 客户端。主要数据流非常简单：双方声明各自的握手，另一方检查它们。这种短数据包的交换需要使用特殊的 TCP_NODELAY 标志，关闭 Nagle 的 TCP 算法，并声明 TCP 连接将以小包的对话形式进行。

然而，握手中的谁先发言并没有严格定义。历史上，所有使用二进制 API 的客户端都是首先发言：发送握手，然后从守护进程读取 4 字节数据，再发送请求并读取守护进程的答复。

在我们改善 Sphinx 协议兼容性时，考虑了以下几点：


1. 通常，主-agent 通信是从已知客户端到已知主机的已知端口。因此，终端提供错误握手的可能性非常小。因此，我们可以隐含地假设双方都是有效的，并且确实在使用 Sphinx 协议。
1. 在这个假设下，我们可以将握手“粘合”到实际请求中，并将其发送在一个数据包中。如果后端是遗留的 Sphinx 守护进程，它将把这个粘合的数据包当作握手的 4 字节读取，然后读取请求体。由于这两个数据包一起到达后端套接字，产生了 -1 RTT，而前端缓冲区仍然以常规方式工作。
1. 继续这个假设：由于“查询”数据包相当小，而握手甚至更小，因此可以使用现代的 TFO（TCP 快速打开）技术，在初始的 “SYN” TCP 数据包中发送两者。也就是说，我们与远程节点建立连接时，使用粘合的握手 + 主体数据包。守护进程接受连接，并立即在套接字缓冲区中有握手和主体，因为它们在第一个 TCP “SYN” 数据包中一起到达。这消除了另一个 RTT。
1. 最后，教守护进程接受这一改进。实际上，从应用程序的角度看，意味着不要使用 `TCP_NODELAY`。从系统的角度看，这意味着确保在守护进程端激活 TFO 接收，而在客户端端激活 TFO 发送。默认情况下，在现代系统中，客户端 TFO 已经默认启用，因此只需为服务器调整 TFO 即可使所有功能正常工作。

所有这些改进都没有实际改变协议本身，允许我们从连接中消除 1.5 RTT。这意味着如果查询和答案能够放入一个单独的 TCP 数据包中，将整个二进制 API 会话从 3.5 RTT 减少到 2 RTT，从而使网络协商速度约快 2 倍。

因此，所有的改进都是围绕一个最初未定义的陈述展开的：“谁先发言。”如果客户端先发言，我们可以应用所有这些优化，并有效地在一个 TFO 数据包中处理连接 + 握手 + 查询。此外，我们可以查看接收到的数据包的开始部分，确定实际协议。这就是为什么可以通过 API/http/https 连接到同一端口。如果守护进程必须先发言，所有这些优化都是不可能的，且多协议也无法实现。因此，我们为 MySQL 提供了专用端口，而没有将其与所有其他协议统一到同一端口。突然之间，在所有客户端中，有一个客户端是按照守护进程应首先发送握手的方式编写的。因此，无法进行所有上述改进。这就是针对 mysql/mariadb 的 SphinxSE 插件。因此，为了这个单一的客户端，我们专门定义了 `sphinx` 协议以最遗留的方式工作。具体来说：双方激活 `TCP_NODELAY` 并使用小包进行交换。守护进程在连接时发送其握手，然后客户端发送其握手，然后一切正常工作。虽然这不是非常优化，但确实可以工作。如果使用 SphinxSE 连接到 Manticore，则必须为显式声明 `sphinx` 协议的侦听器专门分配一个。如果使用其他客户端，请避免使用此侦听器，因为它速度较慢。如果您使用其他遗留的 Sphinx API 客户端，请先检查它们是否能够通过非专用的多协议端口工作。对于主-agent 链接，使用非专用（多协议）端口并启用客户端和服务器 TFO 的方法效果很好，肯定会使网络后端工作更快，特别是当您有非常轻和快速的查询时。</details>

### listen_tfo

此设置允许为所有侦听器启用 TCP_FASTOPEN 标志。默认情况下，它由系统管理，但可以通过将其设置为 '0' 明确关闭。

有关 TCP Fast Open 扩展的常规知识，请参考 [Wikipedia](https://en.wikipedia.org/wiki/TCP_Fast_Open)。简而言之，它允许在建立连接时消除一个 TCP 往返时间。

在实践中，在许多情况下使用 TFO 可以优化客户端-代理的网络效率，尤其是在使用 [持久代理](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 的情况下，但无需保持活动连接，也没有最大连接数的限制。

在现代操作系统中，TFO 支持通常在系统级别上处于“开启”状态，但这只是一个“能力”，并不是规则。Linux（作为最先进的）自 2011 年起就支持它，适用于 3.7 及以上版本的内核（服务器端）。Windows 从某些 Windows 10 的构建版本开始支持它。其他操作系统（FreeBSD、MacOS）也在支持之列。

对于 Linux 系统，服务器检查变量 `/proc/sys/net/ipv4/tcp_fastopen` 并根据其值进行相应行为。位 0 管理客户端，位 1 管理侦听器。默认情况下，系统将此参数设置为 1，即客户端启用，侦听器禁用。

### log

<!-- example conf log -->

日志设置指定了所有 `searchd` 运行时事件将记录的日志文件名称。如果未指定，默认名称为 'searchd.log'。

另外，您可以将文件名设置为 'syslog'。在这种情况下，事件将发送到 syslog 守护进程。要使用 syslog 选项，您需要在构建时用 `--with-syslog` 选项配置 Manticore。


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

使 searchd 在使用 [多查询](../Searching/Multi-queries.md) 时对提交的单个批次中的查询数量进行合理性检查。将其设置为 0 以跳过检查。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_batch_queries = 256
```
<!-- end -->

### max_connections

<!-- example max_connections -->
允许的最大同时客户端连接数。默认情况下没有限制。这通常在使用任何形式的持久连接时才会显现出来，例如 CLI MySQL 会话或来自远程分布式表的持久远程连接。当超过限制时，您仍然可以使用 [VIP 连接](../Connecting_to_the_server/MySQL_protocol.md#VIP-连接) 连接到服务器。VIP 连接不计入限制。

<!-- request Example -->
```ini
max_connections = 10
```

<!-- end -->

### max_threads_per_query

<!-- example max_threads_per_query -->

实例级别的单个操作可以使用的线程限制。默认情况下，适当的操作可以占用所有 CPU 核心，不留其他操作的空间。例如，针对相当大的渗透表执行 `call pq` 可能会占用所有线程数十秒。将 `max_threads_per_query` 设置为，例如，线程数的一半 [threads](../Server_settings/Searchd.md#threads)，将确保您可以并行运行几次这样的 `call pq` 操作。

您还可以在运行时将此设置作为会话或全局变量进行控制。

此外，您可以通过 [threads 选项](../Searching/Options.md#threads) 在每个查询的基础上控制行为。

<!-- intro -->
##### 示例：
<!-- request Example -->

```ini
max_threads_per_query = 4
```

<!-- end -->

### max_filters

<!-- example conf max_filters -->
每个查询允许的最大过滤器数量。此设置仅用于内部合理性检查，并不会直接影响内存使用或性能。可选，默认值为 256。


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
max_filters = 1024
```
<!-- end -->


### max_filter_values

<!-- example conf max_filter_values -->
每个过滤器允许的最大值数量。此设置仅用于内部合理性检查，并不会直接影响内存使用或性能。可选，默认值为 4096。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_filter_values = 16384
```
<!-- end -->


### max_open_files

<!-- example conf max_open_files -->

服务器允许打开的最大文件数称为“软限制”。请注意，处理大型分散的实时表可能需要将此限制设置得很高，因为每个磁盘块可能占用十个或更多文件。例如，具有 1000 个块的实时表可能需要同时打开数千个文件。如果在日志中遇到“打开文件过多”的错误，请尝试调整此选项，因为它可能有助于解决该问题。

此外，还有一个“硬限制”，该选项无法超过。此限制由系统定义，可以在 Linux 中的文件 `/etc/security/limits.conf` 中进行更改。其他操作系统可能采用不同的方法，请参考相关手册获取更多信息。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = 10000
```
<!-- end -->

<!-- example conf max_open_files max -->
除了直接的数值，您可以使用魔术字“max”将限制设置为当前可用硬限制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = max
```
<!-- end -->


### max_packet_size

<!-- example conf max_packet_size -->
允许的最大网络数据包大小。此设置限制来自客户端的查询数据包和来自分布式环境中远程代理的响应数据包。仅用于内部合理性检查，并不会直接影响内存使用或性能。可选，默认值为 128M。


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

一些挑剔的 MySQL 客户端库依赖于 MySQL 使用的特定版本号格式，而且有时会根据报告的版本号选择不同的执行路径（而不是指示的能力标志）。例如，Python MySQLdb 1.2.2 在版本号不是 X.Y.ZZ 格式时会抛出异常；MySQL .NET 连接器 6.3.x 在版本号为 1.x 以及某些标志组合时会内部失败等。为了解决这个问题，您可以使用 `mysql_version_string` 指令，让 `searchd` 向通过 MySQL 协议连接的客户端报告不同的版本。（默认情况下，它报告自己的版本。）


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
mysql_version_string = 5.0.37
```
<!-- end -->


### net_workers

网络线程的数量，默认值为 1。

此设置对于极高查询速率时，当一个线程不足以处理所有传入查询时非常有用。


### net_wait_tm

控制网络线程的忙碌循环间隔。默认值为 -1，可以设置为 -1、0 或正整数。

在服务器配置为纯主机并仅路由请求到代理的情况下，重要的是无延迟地处理请求，不允许网络线程休眠。为此，有一个忙碌循环。在传入请求后，网络线程在 `net_wait_tm` 为正数时使用 CPU 轮询 `10 * net_wait_tm` 毫秒，或者在 `net_wait_tm` 为 0 时仅使用 CPU 轮询。也可以通过将 `net_wait_tm` 设置为 -1 禁用忙碌循环——在这种情况下，轮询器将超时设置为系统轮询调用上实际代理的超时。

> **警告：** CPU 忙碌循环实际上会增加 CPU 核心的负载，因此将此值设置为任何非默认值即使在空闲服务器上也会导致明显的 CPU 使用率。


### net_throttle_accept

定义每次网络循环迭代接受的客户端数量。默认值为 0（无限制），这对于大多数用户应该是合适的。这是一个微调选项，用于在高负载场景中控制网络循环的吞吐量。


### net_throttle_action

定义每次网络循环迭代处理的请求数量。默认值为 0（无限制），这对于大多数用户应该是合适的。这是一个微调选项，用于在高负载场景中控制网络循环的吞吐量。

### network_timeout

<!-- example conf network_timeout -->

网络客户端请求读/写超时，单位为秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认值为 5 秒。`searchd` 将强制关闭未能在此超时内发送查询或读取结果的客户端连接。

请注意 [reset_network_timeout_on_packet](../Server_settings/Searchd.md#reset_network_timeout_on_packet) 参数。此参数改变 `network_timeout` 的行为，使其应用于单个数据包而不是整个 `query` 或 `result`。通常，查询/结果适合于一个或两个数据包。然而，在需要大量数据的情况下，此参数在保持活动操作方面可能非常有价值。

<!-- request Example -->

```ini
network_timeout = 10s
```
<!-- end -->

### node_address

<!-- example conf node_address -->

此设置允许您指定节点的网络地址。默认情况下，设置为复制 [listen](../Server_settings/Searchd.md#listen) 地址。这在大多数情况下是正确的；但是，有些情况下您需要手动指定它：

- 节点在防火墙后面
- 启用网络地址转换（NAT）
- 容器部署，如 Docker 或云部署
- 节点位于多个区域的集群


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
node_address = 10.101.0.10
```
<!-- end -->

### not_terms_only_allowed

<!-- example conf not_terms_only_allowed -->
此设置决定是否允许仅使用 [否定](../Searching/Full_text_matching/Operators.md#否定操作符) 全文操作符的查询。可选，默认值为 0（失败仅带 NOT 操作符的查询）。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
not_terms_only_allowed = 1
```
<!-- end -->

### optimize_cutoff

<!-- example conf optimize_cutoff -->
设置默认表压缩阈值。更多信息请参见 [优化的磁盘块数量](../Securing_and_compacting_a_table/Compacting_a_table.md#优化后的磁盘块数量)。此设置可以通过每个查询选项 [cutoff](../Securing_and_compacting_a_table/Compacting_a_table.md#优化后的磁盘块数量) 被重写。它也可以通过 [SET GLOBAL](../Server_settings/Setting_variables_online.md#SET) 动态更改。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
optimize_cutoff = 4
```
<!-- end -->

### persistent_connections_limit

<!-- example conf persistent_connections_limit -->

此设置决定对远程 [持久代理](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 的最大同时持久连接数。每次连接定义在 `agent_persistent` 下的代理时，我们尝试重用现有连接（如果有），或者连接并保存该连接以供将来使用。然而，在某些情况下，限制这些持久连接的数量是有意义的。此指令定义了限制。它影响到所有分布式表中每个代理主机的连接数。

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

pid_file 是 Manticore 搜索中一个强制性的配置选项，指定存储 `searchd` 服务器进程 ID 的文件路径。

`searchd` 进程 ID 文件在启动时重新创建并锁定，包含服务器运行时的主服务器进程 ID。服务器关闭时，该文件被取消链接。 此文件的目的是使 Manticore 能够执行各种内部任务，例如检查是否已经有正在运行的 `searchd` 实例、停止 `searchd` 和通知它应轮换表格。该文件也可以用于外部自动化脚本。


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
在基于查询执行时间（使用最大查询时间设置）终止查询之前，这是一个很好的安全网，但它有一个固有的缺点：不确定性（不稳定）的结果。也就是说，如果您多次以时间限制重复相同的（复杂的）搜索查询，则时间限制将在不同阶段达到，您将获得*不同*的结果集。

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

有一个新选项， [SELECT … OPTION max_predicted_time](../Searching/Options.md#max_predicted_time)，可以让您限制查询时间 *并* 获取稳定、可重复的结果。它不是在评估查询时定期检查实际的当前时间（这是不确定的），而是使用一个简单的线性模型预测当前的运行时间：

```ini
predicted_time =
    doc_cost * processed_documents +
    hit_cost * processed_hits +
    skip_cost * skiplist_jumps +
    match_cost * found_matches
```

当 `predicted_time` 达到给定限制时，查询将提前终止。

当然，这并不是实际花费时间的硬限制（不过，它确实是对处理工作量的硬限制），而且简单的线性模型并不是一个理想的精确模型。因此，实际墙钟时间可能低于或高于目标限制。然而，误差范围相当可接受：例如，在我们的实验中，针对100毫秒的目标限制，大多数测试查询落在95到105毫秒范围内，所有查询都在80到120毫秒范围内。此外，使用建模的查询时间而不是测量实际运行时间也减少了 `gettimeofday()` 的调用。

由于没有两个服务器的型号完全相同，因此 `predicted_time_costs` 指令允许您配置上述模型的成本。为了方便起见，它们是以纳秒为单位的整数（`max_predicted_time` 中的限制以毫秒为单位，指定成本值为0.000128毫秒而非128纳秒可能更易出错）。并不需要一次性指定所有四个成本，未指定的将采用默认值。然而，我们强烈建议为了可读性而指定所有成本。


### preopen_tables

<!-- example conf preopen_tables -->

`preopen_tables` 配置指令指定在启动时是否强制预打开所有表。默认值为1，这意味着所有表将被预打开，而不考虑每个表的 `preopen` 设置。如果设置为0，则每个表的设置将生效，默认值为0。

预打开表可以防止搜索查询与轮换之间的竞争，从而偶尔导致查询失败。然而，它也会使用更多的文件句柄。在大多数情况下，建议预打开表。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
preopen_tables = 1
```
<!-- end -->

### pseudo_sharding

<!-- example conf pseudo_sharding -->

伪分片配置选项使搜索查询能够并行化到本地的普通表和实时表，无论是直接查询还是通过分布式表查询。该功能会自动将查询并行化到 `searchd.threads` 指定的线程数量。

请注意，如果您的工作线程已经很忙，因为您有：

* 高查询并发
* 任何形式的物理分片：
  * 由多个普通/实时表组成的分布式表
  * 包含过多磁盘块的实时表


则启用伪分片可能不会带来任何好处，甚至可能导致吞吐量略微下降。如果您更优先考虑更高的吞吐量而非较低的延迟，建议禁用此选项。

默认启用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
pseudo_sharding = 0
```
<!-- end -->


### replication_connect_timeout

`replication_connect_timeout` 指令定义连接到远程节点的超时时间。默认值假定为毫秒，但可以使用 [其他后缀](../../Server_settings/Special_suffixes.md)。默认值为 1000（1 秒）。

连接到远程节点时，Manticore 最多将等待此时间以成功完成连接。如果超时达到但连接尚未建立，并且启用了 `retries`，则会发起重试。


### replication_query_timeout

`replication_query_timeout` 设置 searchd 等待远程节点完成查询的时间。默认值为 3000 毫秒（3 秒），但可以使用 `后缀` 表示不同的时间单位。

在建立连接后，Manticore 将最多等待 `replication_query_timeout` 来完成远程节点的查询。请注意，此超时与 `replication_connect_timeout` 是分开的，远程节点可能导致的总延迟将是两者值的总和。


### replication_retry_count

此设置是一个整数，指定 Manticore 在报告致命查询错误之前在复制过程中尝试连接和查询远程节点的次数。默认值为 3。


### replication_retry_delay

此设置是一个以毫秒为单位的整数（或 [特殊后缀](../Server_settings/Special_suffixes.md)），指定在复制过程中失败后，Manticore 重试查询远程节点之前的延迟。此值仅在指定非零值时相关。默认值为 500。

### qcache_max_bytes

<!-- example conf qcache_max_bytes -->
此配置设置用于缓存结果集的最大 RAM 数量（以字节为单位）。默认值为 16777216，相当于 16 兆字节。如果值设置为 0，则查询缓存被禁用。有关查询缓存的更多信息，请参阅 [查询缓存](../Searching/Query_cache.md) 的详细信息。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
qcache_max_bytes = 16777216
```
<!-- end -->


### qcache_thresh_msec

整数，以毫秒为单位。缓存查询结果的最小壁钟时间阈值。默认值为 3000，或 3 秒。0 意味着缓存所有内容。有关详细信息，请参阅 [查询缓存](../Searching/Query_cache.md)。此值也可以使用时间 [特殊后缀](../Server_settings/Special_suffixes.md) 表示，但请谨慎使用，避免与值的名称混淆，名称中包含 '_msec'。


### qcache_ttl_sec

整数，以秒为单位。缓存结果集的过期时间。默认值为 60，或 1 分钟。最小可能值为 1 秒。有关详细信息，请参阅 [查询缓存](../Searching/Query_cache.md)。此值也可以使用时间 [特殊后缀](../Server_settings/Special_suffixes.md) 表示，但请谨慎使用，避免与值的名称混淆，名称中包含 '_sec'。


### query_log_format

<!-- example conf query_log_format -->

查询日志格式。可选，允许的值为 `plain` 和 `sphinxql`，默认值为 `sphinxql`。

`sphinxql` 模式记录有效的 SQL 语句。`plain` 模式以纯文本格式记录查询（主要适用于纯全文使用场景）。此指令允许您在搜索服务器启动时在两种格式之间切换。日志格式也可以通过 `SET GLOBAL query_log_format=sphinxql` 语法动态更改。有关更多详细信息，请参阅 [查询日志记录](../Logging/Query_logging.md)。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_format = sphinxql
```
<!-- end -->

### query_log_min_msec

限制（以毫秒为单位），防止查询写入查询日志。可选，默认值为 0（所有查询都写入查询日志）。此指令指定仅记录执行时间超过指定限制的查询（此值也可以使用时间 [特殊后缀](../Server_settings/Special_suffixes.md) 表示，但请谨慎使用，避免与值的名称混淆，名称中包含 `_msec`）。

### query_log

<!-- example conf query_log -->
查询日志文件名。可选，默认值为空（不记录查询）。所有搜索查询（如 SELECT ... 但不包括 INSERT/REPLACE/UPDATE 查询）将记录在此文件中。格式在 [查询日志记录](../Logging/Query_logging.md) 中描述。如果是 'plain' 格式，您可以将 'syslog' 作为日志文件的路径。在这种情况下，所有搜索查询将以 `LOG_INFO` 优先级发送到 syslog 守护进程，并以 '[query]' 作为前缀，而不是时间戳。要使用 syslog 选项，Manticore 必须在构建时配置 `-–with-syslog`。

<!-- intro -->

##### 示例：

<!-- request Example -->

```ini
query_log = /var/log/query.log
```
<!-- end -->


### query_log_mode

<!-- example conf query_log_mode -->
query_log_mode 指令允许您为 searchd 和查询日志文件设置不同的权限。默认情况下，这些日志文件的权限为 600，意味着只有运行服务器的用户和 root 用户可以读取日志文件。 此指令对于希望允许其他用户读取日志文件的情况非常有用，例如，在非 root 用户上运行的监控解决方案。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_mode  = 666
```
<!-- end -->

### read_buffer_docs

<!-- example conf read_buffer_docs -->

`read_buffer_docs` 指令控制文档列表的每个关键词读取缓冲区大小。每个搜索查询中每个关键词出现时，会有两个相关的读取缓冲区：一个用于文档列表，另一个用于命中列表。此设置允许您控制文档列表缓冲区的大小。

较大的缓冲区可能会增加每次查询的内存使用，但可能会减少I/O时间。对于慢存储，设置较大的值是合理的；而对于高IOPS存储，则应在较小的值区域进行实验。

默认值为256K，最小值为8K。您还可以在每个表的基础上设置 `read_buffer_docs`，这将覆盖服务器配置级别的设置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_docs = 128K
```
<!-- end -->


### read_buffer_hits

<!-- example conf read_buffer_hits -->

`read_buffer_hits` 指令指定搜索查询中命中列表的每个关键词读取缓冲区大小。默认大小为256K，最小值为8K。每个搜索查询中每个关键词出现时，会有两个相关的读取缓冲区：一个用于文档列表，另一个用于命中列表。增加缓冲区大小可以增加每次查询的内存使用，但减少I/O时间。对于慢存储，较大的缓冲区大小是合适的；对于高IOPS存储，则应在较小的值区域进行实验。

此设置也可以在每个表的基础上指定，使用 `read_buffer_hits` 选项，这将覆盖服务器级别的设置。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_hits = 128K
```
<!-- end -->

### read_unhinted

<!-- example conf read_unhinted -->

未提示读取大小。可选，默认值为32K，最小值为1K。

在查询时，某些读取可以预先确切知道要读取多少数据，但某些则无法。最显著的是，命中列表的大小当前并不知道。此设置允许您控制在这种情况下要读取多少数据。它影响命中列表的I/O时间，对于大于未提示读取大小的列表，可以减少I/O时间，但对于较小的列表则可能增加。它不会影响内存使用，因为读取缓冲区已经分配。因此，它不应大于 `read_buffer`。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_unhinted = 32K
```
<!-- end -->

### reset_network_timeout_on_packet

<!-- example conf reset_network_timeout_on_packet -->

细化网络超时的行为（如 `network_timeout`、`read_timeout` 和 `agent_query_timeout`）。

设置为0时，超时限制发送整个请求/查询的最大时间。设置为1（默认）时，超时限制网络活动之间的最大时间。

在复制过程中，一个节点可能需要将大文件（例如100GB）发送到另一个节点。假设网络可以以1GB/s的速度传输数据，并且每个包大小为4-5MB。传输整个文件需要100秒。如果默认超时为5秒，则只允许传输5GB后连接被断开。增加超时可能是一个解决方法，但并不可扩展（例如，下一个文件可能是150GB，导致再次失败）。但是，默认的 `reset_network_timeout_on_packet` 设置为1时，超时适用于单个数据包，而不是整个传输。只要传输在进行中（并且在超时期间确实有数据通过网络接收），连接就会保持活跃。如果传输被卡住，超时发生在包之间，则连接将被断开。

请注意，如果您设置了分布式表，每个节点——包括主节点和代理——都应进行调整。在主节点上，`agent_query_timeout` 会受到影响；在代理上，`network_timeout` 是相关的。

<!-- intro -->

##### 示例：

<!-- request Example -->

```ini
reset_network_timeout_on_packet = 0
```

<!-- end -->


### rt_flush_period

<!-- example conf rt_flush_period -->

实时表RAM块刷新检查周期，以秒为单位（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认值为10小时。

积极更新的RT表如果完全适合RAM块，仍可能导致不断增长的binlog，从而影响磁盘使用和崩溃恢复时间。通过此指令，搜索服务器会定期执行刷新检查，合适的RAM块可以被保存，从而进行后续的binlog清理。有关详细信息，请参见 [二进制日志](../Logging/Binary_logging.md)。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_flush_period = 3600 # 1 hour
```
<!-- end -->


### rt_merge_iops

<!-- example conf rt_merge_iops -->

实时块合并线程允许启动的最大I/O操作次数（每秒）。可选，默认值为0（无上限）。

此指令允许您限制由于 `OPTIMIZE` 语句产生的I/O影响。可以保证所有RT优化活动生成的磁盘I/O（每秒的I/O次数）不会超过配置的限制。限制 `rt_merge_iops` 可以减少由于合并导致的搜索性能下降。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_iops = 40
```
<!-- end -->

### rt_merge_maxiosize

<!-- example conf rt_merge_maxiosize -->

实时块合并线程允许启动的最大I/O操作大小。可选，默认值为0（无上限）。

此指令允许您限制由于 `OPTIMIZE` 语句产生的I/O影响。大于此限制的I/O将被拆分为两个或更多的I/O，这将被视为与 [rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops) 限制相关的单独I/O。因此，可以保证所有优化活动每秒生成的磁盘I/O不会超过 (rt_merge_iops * rt_merge_maxiosize) 字节。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_maxiosize = 1M
```
<!-- end -->


### seamless_rotate

<!-- example conf seamless_rotate -->

防止 `searchd` 在旋转包含大量数据的表时发生停顿以进行预缓存。可选，默认值为 1（启用无缝旋转）。在 Windows 系统上，默认情况下禁用无缝旋转。

表可能包含一些需要在 RAM 中预缓存的数据。目前，`.spa`、`.spb`、`.spi` 和 `.spm` 文件会被完全预缓存（它们分别包含属性数据、BLOB 属性数据、关键词表和被删除行映射）。如果没有无缝旋转，旋转表时会尝试尽可能少地使用 RAM，并按如下方式工作：

1. 暂时拒绝新查询（带有“重试”错误代码）；
1. `searchd` 等待所有正在运行的查询完成；
1. 释放旧表并重命名其文件；
1. 重命名新表文件并分配所需的 RAM；
1. 将新表的属性和字典数据预加载到 RAM 中；
1. `searchd` 从新表恢复服务查询。

然而，如果有大量的属性或字典数据，则预加载步骤可能需要相当长的时间——在预加载 1-5+ GB 文件的情况下，可能需要几分钟。

启用无缝旋转后，旋转按以下方式工作：

1. 分配新表的 RAM 存储；
2. 新表的属性和字典数据异步预加载到 RAM 中；
3. 成功后，释放旧表并重命名两个表的文件；
4. 失败时，释放新表；
5. 在任何时刻，查询要么从旧表要么从新表提供。

无缝旋转的代价是旋转期间的峰值内存使用量更高（因为在预加载新副本时，旧副本和新副本的 `.spa/.spb/.spi/.spm` 数据都需要在 RAM 中）。平均使用量保持不变。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
seamless_rotate = 1
```
<!-- end -->

### secondary_indexes
<!-- example conf secondary_indexes -->

此选项用于启用/禁用搜索查询的二级索引。可选，默认值为 1（启用）。注意，您不需要为索引启用它，因为只要安装了 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)，它始终处于启用状态。后者在搜索时也必须使用这些索引。可用三种模式：

- `0`：禁用搜索时使用二级索引。可以使用 [分析器提示](../Searching/Options.md#expansion_limit) 为单个查询启用它们。
- `1`：启用搜索时使用二级索引。可以使用 [分析器提示](../Searching/Options.md#expansion_limit) 为单个查询禁用它们。
- `force`：与启用相同，但会报告加载二级索引时的任何错误，并且不会将整个索引加载到守护进程中。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
secondary_indexes = 1
```

<!-- end -->

### server_id

<!-- example conf server_id -->
作为服务器标识符的整数，用于生成唯一的短 UUID，以用于作为复制集群一部分的节点。server_id 必须在 0 到 127 的范围内并且在集群节点中唯一。如果未设置 server_id，将使用 MAC 地址或随机数作为短 UUID 的种子。


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
server_id = 1
```
<!-- end -->


### shutdown_timeout

<!-- example conf shutdown_timeout -->

`searchd --stopwait` 等待时间，以秒为单位（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认值为 60 秒。

当您运行 `searchd --stopwait` 时，服务器需要执行一些活动才能停止，例如完成查询、刷新 RT RAM 块、刷新属性和更新二进制日志。这些任务需要一些时间。`searchd --stopwait` 将等待最多 `shutdown_time` 秒，以便服务器完成其任务。合适的时间取决于您的表大小和负载。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
shutdown_timeout = 3m # wait for up to 3 minutes
```
<!-- end -->


### shutdown_token

用于调用 VIP Manticore SQL 连接的“shutdown”命令所需的 SHA1 密码哈希。没有它，[debug](../Reporting_bugs.md#DEBUG) 的“shutdown”子命令将永远不会导致服务器停止。请注意，这种简单哈希不应被视为强保护，因为我们没有使用加盐哈希或任何现代哈希函数。它旨在作为本地网络中的 housekeeping 守护程序的傻瓜保护措施。

### snippets_file_prefix

<!-- example conf snippets_file_prefix -->

在生成片段时要添加到本地文件名的前缀。可选，默认值为当前工作文件夹。

此前缀可用于与 `load_files` 或 `load_files_scattered` 选项一起在分布式片段生成中使用。

请注意，这只是一个前缀，而不是路径！这意味着，如果前缀设置为“server1”，并且请求引用“file23”，则 `searchd` 将尝试打开“server1file23”（所有内容不带引号）。因此，如果您需要它作为路径，必须包含尾部斜杠。

构造最终文件路径后，服务器会展开所有相对目录，并将最终结果与 `snippet_file_prefix` 的值进行比较。如果结果不以前缀开头，则该文件将被拒绝并显示错误消息。

例如，如果您将其设置为 `/mnt/data`，并且有人调用片段生成时使用源文件 `../../../etc/passwd`，则他们将收到错误消息：

```
File '/mnt/data/../../../etc/passwd' escapes '/mnt/data/' scope
```

而不是文件的内容。

还要注意，这是一个本地选项；它不会影响代理。因此，您可以在主服务器上安全地设置前缀。路由到代理的请求将不会受到主服务器设置的影响。但是，它们将受到代理自身设置的影响。

这在文档存储位置（无论是本地存储还是 NAS 挂载点）在服务器之间不一致时可能会很有用。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
snippets_file_prefix = /mnt/common/server1/
```
<!-- end -->

> **警告：** 如果您仍然想从 FS 根目录访问文件，必须将 `snippets_file_prefix` 显式设置为空值（通过 `snippets_file_prefix=` 行）或根目录（通过 `snippets_file_prefix=/`）。


### sphinxql_state

<!-- example conf sphinxql_state -->

指向一个文件的路径，该文件将序列化当前 SQL 状态。

在服务器启动时，该文件将被重放。在合格的状态变化（例如，SET GLOBAL）时，该文件会自动重写。这可以防止一个难以诊断的问题：如果您加载 UDF 函数但 Manticore 崩溃，当它（自动）重新启动时，您的 UDF 和全局变量将不再可用。使用持久状态有助于确保优雅恢复，没有这样的意外。

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
使用 SQL 接口时，请求之间的最大等待时间（以秒为单位或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认值为 15 分钟。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
sphinxql_timeout = 15m
```
<!-- end -->


### ssl_ca

<!-- example conf ssl_ca -->

SSL 证书授权（CA）证书文件的路径（也称为根证书）。可选，默认值为空。当不为空时，`ssl_cert` 中的证书应由该根证书签名。

服务器使用 CA 文件来验证证书的签名。文件必须为 PEM 格式。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ssl_ca = keys/ca-cert.pem
```
<!-- end -->


### ssl_cert

<!-- example conf ssl_cert -->

服务器 SSL 证书的路径。可选，默认值为空。

服务器使用该证书作为自签名公钥来加密通过 SSL 的 HTTP 流量。文件必须为 PEM 格式。


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

服务器使用此私钥来加密通过 SSL 的 HTTP 流量。文件必须为 PEM 格式。


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

此设置限制公共子树优化器的 RAM 使用量（见 [多查询](../Searching/Multi-queries.md)）。每个查询最多会使用这部分 RAM 来缓存文档条目。将限制设置为 0 将禁用优化器。


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

此设置限制公共子树优化器的 RAM 使用量（见 [多查询](../Searching/Multi-queries.md)）。每个查询最多会使用这部分 RAM 来缓存关键词出现次数（命中）。将限制设置为 0 将禁用优化器。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
subtree_hits_cache = 16M
```
<!-- end -->

### threads

<!-- example threads -->

Manticore 守护进程的工作线程数量（或线程池大小）。Manticore 在启动时创建这个数量的操作系统线程，它们执行守护进程内部的所有工作，例如执行查询、创建片段等。一些操作可能会拆分为子任务并并行执行，例如：

- 在实时表中搜索
- 在由本地表组成的分布式表中搜索
- 调用渗透查询
- 以及其他操作

默认情况下，它设置为服务器上的 CPU 核心数量。Manticore 在启动时创建线程，并在停止之前保持它们。每个子任务在需要时可以使用一个线程。当子任务完成后，它会释放线程，以便另一个子任务可以使用它。

在高强度 I/O 类型的负载下，将此值设置为超过 CPU 核心数量可能是有意义的。

<!-- request Example -->
```ini
threads = 10
```

<!-- end -->

### thread_stack

<!-- example conf thread_stack -->

作业（协程，单个搜索查询可能导致多个作业/协程）的最大栈大小。可选，默认值为 128K。

每个作业有自己的 128K 栈。当您运行查询时，会检查所需的栈大小。如果默认的 128K 足够，它将被处理。如果需要更多，将调度另一个具有增加栈的作业继续处理。此设置限制此类高级栈的最大大小。

将值设置为合理的高值将有助于处理非常深的查询，而不会导致整体 RAM 消耗过高。例如，将其设置为 1G 并不意味着每个新作业将占用 1G RAM，但如果我们发现它需要，比如说，100M 栈，我们只会为该作业分配 100M。其他作业在同一时间将以其默认的 128K 栈运行。同样，我们可以运行需要 500M 的更复杂查询。只有在我们内部发现作业需要超过 1G 栈时，我们才会失败并报告线程栈过低。

然而，在实践中，即使需要 16M 栈的查询也通常过于复杂，处理起来消耗的时间和资源过多。因此，守护进程会处理它，但通过 `thread_stack` 设置限制此类查询看起来相当合理。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
thread_stack = 8M
```
<!-- end -->


### unlink_old

<!-- example conf unlink_old -->
确定在成功旋转时是否删除 `.old` 表副本。可选，默认值为 1（删除）。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
unlink_old = 0
```
<!-- end -->


### watchdog

<!-- example conf watchdog -->

线程化服务器监视器。可选，默认值为 1（启用监视器）。

当 Manticore 查询崩溃时，可能会导致整个服务器关闭。启用监视器功能后，`searchd` 还维护一个单独的轻量级进程，监视主服务器进程，并在异常终止时自动重新启动它。监视器默认启用。

<!-- request Example -->

```ini
watchdog = 0 # disable watchdog
```
<!-- end -->
<!-- proofread -->
