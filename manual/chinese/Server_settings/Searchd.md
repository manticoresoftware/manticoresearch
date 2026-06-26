# 配置中的 "Searchd" 章节

下面这些设置用于 Manticore Search 配置文件的 `searchd` 段，以控制服务器行为。以下是每个设置的摘要：

### access_plain_attrs

此设置为 [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 提供实例级默认值。它是可选的，默认值为 `mmap_preread`。

`access_plain_attrs` 指令允许你为此 searchd 实例管理的所有表定义 [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。按表级的指令优先级更高，会覆盖这个实例级默认值，从而提供更细粒度的控制。

### access_blob_attrs

此设置为 [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 提供实例级默认值。它是可选的，默认值为 `mmap_preread`。

`access_blob_attrs` 指令允许你为此 searchd 实例管理的所有表定义 [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。按表级的指令优先级更高，会覆盖这个实例级默认值，从而提供更细粒度的控制。

### access_doclists

此设置为 [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 提供实例级默认值。它是可选的，默认值为 `file`。

`access_doclists` 指令允许你为此 searchd 实例管理的所有表定义 [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。按表级的指令优先级更高，会覆盖这个实例级默认值，从而提供更细粒度的控制。

### access_hitlists

此设置为 [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 提供实例级默认值。它是可选的，默认值为 `file`。

`access_hitlists` 指令允许你为此 searchd 实例管理的所有表定义 [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。按表级的指令优先级更高，会覆盖这个实例级默认值，从而提供更细粒度的控制。

### access_dict

此设置为 [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 提供实例级默认值。它是可选的，默认值为 `mmap_preread`。

`access_dict` 指令允许你为此 searchd 实例管理的所有表定义 [access_dict](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) 的默认值。按表级的指令优先级更高，会覆盖这个实例级默认值，从而提供更细粒度的控制。

### agent_connect_timeout

此设置为 [agent_connect_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) 参数提供实例级默认值。


### agent_query_timeout

此设置为 [agent_query_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) 参数提供实例级默认值。它可以通过 `OPTION agent_query_timeout=XXX` 子句按查询覆盖。


### agent_retry_count

此设置是一个整数，指定 Manticore 在通过分布式表连接并查询远程 agent 之前会重试多少次，然后才报告致命查询错误。默认值为 0（即不重试）。你也可以通过 `OPTION retry_count=XXX` 子句按查询设置这个值。如果提供了按查询选项，它会覆盖配置中指定的值。

请注意，如果你在分布式表定义中使用了 [agent mirrors](../Creating_a_cluster/Remote_nodes/Mirroring.md#Agent-mirrors)，服务器会根据所选的 [ha_strategy](../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) 为每次连接尝试选择不同的镜像。在这种情况下，`agent_retry_count` 会按一组镜像汇总计算。

例如，如果你有 10 个镜像并设置 `agent_retry_count=5`，服务器最多会重试 50 次，假设 10 个镜像平均各尝试 5 次（在 `ha_strategy = roundrobin` 选项下会是这种情况）。

不过，作为 [agent](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) 的 `retry_count` 选项值是一个绝对上限。换句话说，agent 定义中的 `[retry_count=2]` 选项始终表示最多尝试 2 次，不管你为该 agent 指定了 1 个还是 10 个镜像。

### agent_retry_delay

此设置是以毫秒为单位的整数（或 [special_suffixes](../Server_settings/Special_suffixes.md)），指定在失败时 Manticore 重试查询远程 agent 之前的延迟。只有在指定了非零的 [agent_retry_count](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 或非零的按查询 `retry_count` 时，这个值才有意义。默认值为 500。你也可以通过 `OPTION retry_delay=XXX` 子句按查询设置这个值。如果提供了按查询选项，它会覆盖配置中指定的值。


### attr_flush_period

<!-- example conf attr_flush_period -->
当使用 [Update](../Data_creation_and_modification/Updating_documents/UPDATE.md) 修改实时文档属性时，变更会先写入属性的内存副本。这些更新发生在内存映射文件中，这意味着由操作系统决定何时将变更写入磁盘。在 `searchd` 正常关闭时（由 `SIGTERM` 信号触发），所有变更都会被强制写入磁盘。

你也可以让 `searchd` 定期将这些变更写回磁盘，以防止数据丢失。两次刷新的间隔由 `attr_flush_period` 决定，单位是秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。

默认值为 0，表示禁用周期性刷盘。不过，在正常关闭时仍会执行刷盘。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
attr_flush_period = 900 # persist updates to disk every 15 minutes
```
<!-- end -->

### 认证

<!-- example conf auth -->
启用[认证和授权](../Security/Authentication_and_authorization.md)。可选，默认为空，表示禁用认证。

在[RT 模式](../Read_this_first.md#Real-time-mode-vs-plain-mode)下，使用 `auth = 1` 启用认证。Manticore 会将认证数据存储在 [data_dir](../Server_settings/Searchd.md#data_dir) 下的 `auth.json` 中。使用 `auth = 0` 或省略该设置可禁用认证。

在[普通模式](../Read_this_first.md#Real-time-mode-vs-plain-mode)下，将 `auth` 设为认证文件路径。不要在普通模式下使用 `auth = 1`。

启用认证后，如果认证文件不存在，守护进程会创建它。在首次引导之前，缺失或为空的存储都是有效的，包括零字节文件、仅包含空白字符的文件、空 JSON 对象，或空的用户和权限数组。引导完成后会写入完整的认证 JSON。启动时，守护进程会拒绝无效路径、不可读文件、组或其他用户可读文件、格式错误的 JSON、重复存储的密钥以及无效的 auth 数据结构。请保持该文件私有；守护进程创建的文件权限为 `600`。

<!-- intro -->
##### RT 模式：

<!-- request RT mode -->
```ini
searchd {
    data_dir = /var/lib/manticore
    auth = 1
}
```

<!-- request Disable -->
```ini
searchd {
    data_dir = /var/lib/manticore
    auth = 0
}
```

<!-- intro -->
##### 普通模式：

<!-- request Plain mode -->
```ini
searchd {
    auth = /path/to/auth.json
}
```
<!-- end -->

### auth_log_level

<!-- example conf auth_log_level -->
控制认证日志的详细程度。可选，默认值为 `info`。

认证事件会写入守护进程日志旁边的独立日志文件。如果 [log](../Server_settings/Searchd.md#log) 是 `/var/log/manticore/searchd.log`，则认证日志为 `/var/log/manticore/searchd.log.auth`。

支持的值：

* `disabled` - 不记录认证事件。
* `error` - 记录权限拒绝和严重故障。
* `warning` - 记录错误和认证失败尝试。
* `info` - 记录警告、成功的认证管理变更，以及集群加入时的 auth 数据备份。
* `all` - 记录 `info` 级事件以及面向用户的成功认证事件。
* `trace` - 记录 `all` 级事件，并额外记录成功的内部传输认证，例如 Manticore Buddy 和守护进程之间的 API 认证。

成功的授权允许检查在任何级别都不会被记录。权限拒绝会被记录，但允许检查可能出现在每个查询中，即使在 `trace` 模式下也会让认证日志变得很嘈杂。

当 `JOIN CLUSTER` 替换本地认证数据时，`info`、`all` 和 `trace` 日志会将先前本地 auth 数据的 JSON 备份写入 `searchd.log.auth`。该备份可能包含用户名、salt、密码哈希和 bearer 哈希。请将认证日志视为敏感的运维数据，并在共享前进行脱敏。

<!-- request Example -->
```ini
auth_log_level = warning
```
<!-- end -->

### auth_password_policy

<!-- example conf auth_password_policy -->
控制认证用户的密码校验。可选，默认值为 `LOW`。

支持的值：

* `LOW` - 要求密码非空，并满足 [auth_password_min_length](../Server_settings/Searchd.md#auth_password_min_length) 的长度要求。
* `MEDIUM` - 在 `LOW` 的基础上，还要求至少包含一个小写字母、一个大写字母、一个数字和一个非字母数字字符。

该策略适用于 `searchd --auth`、`CREATE USER` 和 `SET PASSWORD`。

<!-- request Example -->
```ini
auth_password_policy = MEDIUM
```
<!-- end -->

### auth_password_min_length

<!-- example conf auth_password_min_length -->
定义认证用户密码的最小长度。可选，默认值为 `8`。

最小长度适用于 `searchd --auth`、`CREATE USER` 和 `SET PASSWORD`。

<!-- request Example -->
```ini
auth_password_min_length = 12
```
<!-- end -->

### auto_optimize

<!-- example conf auto_optimize -->
此设置控制表压缩的自动 [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 过程。

默认情况下，表压缩会自动发生。你可以通过 `auto_optimize` 设置来修改这种行为：
* 0：禁用自动表压缩（仍可手动调用 `OPTIMIZE`）
* 1：启用自动表压缩，使用默认阈值
* 任何大于 1 的整数：启用自动表压缩，同时将阈值乘以该数值

默认情况下，阈值为逻辑 CPU 核心数乘以 2。

不过，如果表中包含带 KNN 索引的属性，默认阈值会不同。在这种情况下，它被设为物理 CPU 核心数除以 2，最小值为 1，以提升 KNN 搜索性能。

当 [optimize_cutoff](../Server_settings/Searchd.md#optimize_cutoff) 没有显式设置时（无论是全局还是按表），自动压缩都不会把表合并到少于 2 个磁盘块，即使计算出的默认阈值更低也是如此（这在 CPU 核心较少的服务器上可能发生，尤其是 KNN 表）。如果要让自动压缩继续合并到单个磁盘块，请将 `optimize_cutoff` 显式设置为 `1`。

请注意，切换 `auto_optimize` 的开关状态并不会阻止你手动运行 [OPTIMIZE TABLE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE)。

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
此设置控制在实时表执行 [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 时，服务器允许并行运行多少个磁盘 chunk 合并任务。

它只影响磁盘 chunk 合并（压缩），不影响查询并行度。

将其设为 `1` 可禁用并行 chunk 合并（合并任务将逐个执行）。更高的值可能会在具有快速存储的系统上加快压缩，但会增加并发磁盘 I/O。

默认情况下，Manticore 会使用 [threads](../Server_settings/Searchd.md#threads) 设置的值来进行计算；如果未配置 `threads`，则默认为逻辑 CPU 数量。`parallel_chunk_merges` 的默认结果在 `threads` 为 `1`、`2` 或 `3` 时为 `1`，在 `threads` 为 `4` 或更高时为 `2`（即使用整数除法时的 `max(1, min(2, threads/2))`）。

这个值可以在运行时通过 `SET GLOBAL parallel_chunk_merges = N` 修改，并可通过 `SHOW VARIABLES` 查看。

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
此设置控制单个 OPTIMIZE 任务中可合并多少个 RT 磁盘 chunk（N 路合并）。如果可用的 chunk 少于这个数量，任务会尽量合并现有的 chunk（最少 2 个）。

较低的值可以让更多任务并行调度；较高的值会减少任务数量，但会增加每次合并的规模。

默认值为 `2`。

这个值可以在运行时通过 `SET GLOBAL merge_chunks_per_job = N` 修改，并可通过 `SHOW VARIABLES` 查看。

<!-- intro -->
##### 示例：

<!-- request Increase -->
```ini
merge_chunks_per_job = 4
```

<!-- end -->

### knn_parallel_build

<!-- example conf knn_parallel_build -->
此设置控制在包含带 KNN 索引的 `float_vector` 属性的表上执行 HNSW 相关重操作时，用于构建 HNSW 图的工作线程数量。多个路径会使用这个开关：
* **chunk-save store pass**：当 RAM chunk 刷写到磁盘时，worker 会分担该 chunk 的 RAM 段，并并行向目标 HNSW 图中添加向量。
* **chunk-merge store pass**：`OPTIMIZE TABLE` 和自动优化合并会并行构建目标 chunk 的 HNSW 图，把所有输入 chunk 的存活行范围分摊给各个 worker。
* **ALTER KNN rebuild**：对 `float_vector` 属性执行 `ALTER TABLE ... ADD COLUMN`/`DROP COLUMN`，以及 `ALTER TABLE ... REBUILD KNN`，会并行重建磁盘 chunk 的 HNSW 图。

它只影响带 KNN 属性表的 HNSW 图构建。

将其设为 `1` 可禁用并行 KNN 构建（所有 store pass 都串行运行）。更高的值会在 HNSW 图构建占主导耗时的表上，加快 chunk 保存、`OPTIMIZE` 和 ALTER 重建。

并行 HNSW 构建插入向量的顺序可能与串行路径不同，因此生成的 `.spknn` 图不能保证与使用 `knn_parallel_build = 1` 构建的图完全逐位一致。

请注意，当 [`parallel_chunk_merges`](../Server_settings/Searchd.md#parallel_chunk_merges) > 1 时，多个合并可以并发运行，并且每个合并最多会消耗 `knn_parallel_build` 个 worker。

默认情况下，Manticore 会根据 [threads](../Server_settings/Searchd.md#threads) 设置派生该值：`max(1, min(4, threads/4))`。也就是说，当 `threads` 小于 8 时为 `1`（串行），当 `threads` <= 11 时为 `2`，当 `threads` <= 15 时为 `3`，当 `threads` 为 16 或更高时为 `4`（默认上限为 4）。拥有更大机器且希望更高并行度的运维可以显式设置该值。

这个值可以在运行时通过 `SET GLOBAL knn_parallel_build = N` 修改，并可通过 `SHOW VARIABLES` 查看。

<!-- intro -->
##### 示例：

<!-- request Disable -->
```ini
knn_parallel_build = 1
```

<!-- request Increase -->
```ini
knn_parallel_build = 4
```

<!-- end -->

### embeddings_threads

<!-- example conf embeddings_threads -->
此设置限制 Manticore 将文本转换为向量时可使用的 CPU 线程数。它适用于自动 embedding 运行的所有场景：向使用 `model_name`/`from` 的表插入行时，`ALTER TABLE` 重建自动嵌入的 `float_vector` 列时，以及 `knn(<field>, '<text>', ...)` 搜索将查询以文本形式提供时。

实际使用的线程数还会受到当前空闲 worker 数量的限制，因此即使上限很高，繁忙的服务器也会使用更少的线程。使用此选项可以避免某个大型 embedding 批次饿死并发搜索。

默认值为 `4`。设为 `0` 可取消上限，此时 embeddings 库会自行决定使用多少线程（仍受空闲 worker 数量限制）。

此值可在运行时通过 `SET GLOBAL embeddings_threads = N` 修改，并可通过 `SHOW VARIABLES` 查看。对于 KNN `SELECT` 查询，还可以使用 `OPTION embeddings_threads = N` 按查询覆盖（见 [KNN 向量搜索](../Searching/KNN.md#KNN-vector-search)）。

<!-- intro -->
##### 示例：

<!-- request Default -->
```ini
embeddings_threads = 4
```

<!-- request Uncapped -->
```ini
embeddings_threads = 0
```

<!-- end -->

### auto_schema

<!-- example conf auto_schema -->
Manticore 支持自动创建那些尚不存在、但在 INSERT 语句中指定的表。该功能默认启用。要禁用它，请在配置中显式设置 `auto_schema = 0`。要重新启用它，请设置 `auto_schema = 1`，或者从配置中移除 `auto_schema` 设置。

请注意，`/bulk` HTTP 端点不支持自动建表。

> 注意： [auto schema 功能](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不能工作，请确保已安装 Buddy。

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
此设置控制二进制日志事务的刷写/同步模式。它是可选的，默认值为 2（每个事务刷写，每秒同步一次）。

该指令决定二进制日志多频繁刷到操作系统并同步到磁盘。支持三种模式：

* 0：每秒刷写并同步一次。这提供了最佳性能，但在服务器崩溃或操作系统/硬件崩溃时，最多可能丢失 1 秒内已提交的事务。
* 1：每个事务都刷写并同步。这种模式性能最差，但能保证每个已提交事务的数据都被保存。
* 2：每个事务刷写，每秒同步一次。这种模式兼顾较好的性能，并能确保服务器崩溃时每个已提交事务都被保存。不过，在操作系统/硬件崩溃时，最多可能丢失 1 秒内已提交的事务。

对于熟悉 MySQL 和 InnoDB 的用户，这个指令类似于 `innodb_flush_log_at_trx_commit`。在大多数情况下，默认的混合模式 2 在速度和安全性之间提供了很好的平衡，可在服务器崩溃时完整保护 RT 表数据，并对硬件故障提供一定保护。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
binlog_flush = 1 # ultimate safety, low speed
```
<!-- end -->

### binlog_common

<!-- example conf binlog_common -->
此设置控制二进制日志文件的管理方式。它是可选的，默认值为 0（每个表一个独立文件）。

你可以选择两种方式来管理二进制日志文件：

* 每个表一个独立文件（默认，`0`）：每个表将自己的变更保存在独立的日志文件中。如果你有很多在不同时间更新的表，这种设置很合适。它允许表在不等待其他表的情况下更新。另外，如果某个表的日志文件出现问题，也不会影响其他表。
* 所有表共用一个文件（`1`）：所有表使用同一个二进制日志文件。这种方式让文件管理更容易，因为文件更少。不过，如果某个表仍需要保存更新，这可能会让文件保留的时间比必要的更长。如果很多表需要同时更新，这种设置也可能降低速度，因为所有变更都必须排队写入同一个文件。

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

当前 binlog 文件达到这个大小上限后，会强制打开新的 binlog 文件。这会让日志粒度更细，并在某些边界负载下带来更高效的 binlog 磁盘使用。值为 0 表示不会基于大小重新打开 binlog 文件。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
binlog_max_log_size = 16M
```
<!-- end -->


### binlog_path

<!-- example conf binlog_path -->
此设置决定二进制日志（也称事务日志）文件的路径。它是可选的，默认值为构建时配置的数据目录（例如 Linux 上的 `/var/lib/manticore/data/binlog.*`）。

二进制日志用于 RT 表数据的崩溃恢复，以及普通磁盘索引的属性更新，否则这些更新在刷盘前只会保存在 RAM 中。启用日志后，每个提交到 RT 表的事务都会写入日志文件。非正常关闭后，启动时会自动回放日志，恢复已记录的变更。

`binlog_path` 指令指定二进制日志文件所在位置。它只应包含路径；`searchd` 会按需在该目录中创建并删除多个 `binlog.*` 文件（包括 binlog 数据、元数据和锁文件等）。

空值会禁用二进制日志，这会提升性能，但会让 RT 表数据面临风险。


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

当设为 1 时，服务器会自动应用 [布尔查询优化](../Searching/Full_text_matching/Boolean_optimization.md) 以提升查询性能。当设为 0 时，查询默认不做优化。这个默认值可以通过相应的搜索选项 `boolean_simplify` 按查询覆盖。

<!-- request Example -->
```ini
searchd {
    boolean_simplify = 0  # disable boolean optimization by default
}
```
<!-- end -->

### buddy_path

<!-- example conf buddy_path -->
此设置决定 Manticore Buddy 可执行文件的路径。它是可选的，默认值是构建时配置的路径，不同操作系统可能不同。通常你不需要修改这个设置。不过，如果你希望以调试模式运行 Manticore Buddy、修改 Manticore Buddy，或实现新的插件，它会很有用。在后一种情况下，你可以从 https://github.com/manticoresoftware/manticoresearch-buddy `git clone` Buddy，向目录 `./plugins/` 添加新插件，并在切换到 Buddy 源码目录后运行 `composer install --prefer-source` 以便更方便地开发。

为了确保可以运行 `composer`，你的机器必须安装 PHP 8.2 或更高版本，并具备以下扩展：

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

你也可以在发布版中选择适用于 Linux amd64 的特殊 `manticore-executor-dev` 版本，例如：https://github.com/manticoresoftware/executor/releases/tag/v1.0.13

如果你选择这条路线，记得将开发版的 manticore executor 链接到 `/usr/bin/php`。

要禁用 Manticore Buddy，将值设为空，如示例所示。

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
此设置决定使用持久连接时，两次请求之间的最大等待时间（以秒为单位，或 [special_suffixes](../Server_settings/Special_suffixes.md)）。它是可选的，默认值为 5 分钟。


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

指定 libc locale，会影响基于 libc 的排序规则。详情请参见 [collations](../Searching/Collations.md) 章节。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
collation_libc_locale = fr_FR
```
<!-- end -->


### collation_server

<!-- example conf collation_server -->
服务器默认排序规则。可选，默认值为 libc_ci。

指定传入请求使用的默认排序规则。该排序规则可按查询覆盖。可参见 [collations](../Searching/Collations.md) 章节了解可用排序规则及其他细节。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
collation_server = utf8_ci
```
<!-- end -->


### data_dir

<!-- example conf data_dir -->
指定此设置时，会启用 [实时模式](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)，这是一种强制性的表结构管理方式。该值应为一个目录路径，用于存储所有表、二进制日志，以及此模式下 Manticore Search 正常运行所需的其他内容。
当指定了 `data_dir` 时，不允许对 [普通表](../Creating_a_table/Local_tables/Plain_table.md) 进行索引。关于 RT 模式与普通模式的区别，请阅读[这一节](../Read_this_first.md#Real-time-table-vs-plain-table)。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
data_dir = /var/lib/manticore
```
<!-- end -->

### attr_autoconv_strict

<!-- example conf attr_autoconv_strict -->
此设置控制在 INSERT 和 REPLACE 操作期间字符串到数字类型转换的严格验证模式。可选，默认值为 0（非严格模式，保持向后兼容）。

当设为 1（严格模式）时，无效的字符串到数字转换（例如将空字符串 `''` 或非数字字符串 `'a'` 转换为 bigint 属性）会返回错误，而不是静默地转为 0。这有助于在数据插入早期就发现数据质量问题。

当设为 0（非严格模式，默认）时，无效转换会静默地转为 0，以保持与旧版本的向后兼容。

严格模式会验证以下情况：
* 空字符串或无法转换的字符串
* 末尾带非数字字符的字符串（例如 `'123abc'`）
* 超出类型范围的数值（上溢/下溢）

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
attr_autoconv_strict = 1  # enable strict conversion mode
```
<!-- end -->

### diskchunk_flush_search_timeout

<!-- example conf diskchunk_flush_search_timeout -->
如果表中没有搜索，则防止自动刷写 RAM chunk 的超时设置。可选，默认值为 30 秒。

在判断是否自动刷写之前，用于检查是否有搜索的时间。
只有在最近 `diskchunk_flush_search_timeout` 秒内表中至少发生过一次搜索时，才会触发自动刷写。与 [diskchunk_flush_write_timeout](../Server_settings/Searchd.md#diskchunk_flush_write_timeout) 配合使用。对应的 [按表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_search_timeout) 优先级更高，会覆盖这个实例级默认值，从而提供更细粒度的控制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
diskchunk_flush_search_timeout = 120s
```
<!-- end -->

### diskchunk_flush_write_timeout

<!-- example conf diskchunk_flush_write_timeout -->
在没有写入的情况下，等待多久后自动将 RAM chunk 刷到磁盘，单位为秒。可选，默认值为 1 秒。

如果 RAM chunk 在 `diskchunk_flush_write_timeout` 秒内没有发生写入，就会被刷到磁盘。与 [diskchunk_flush_search_timeout](../Server_settings/Searchd.md#diskchunk_flush_search_timeout) 配合使用。要禁用自动刷写，请在配置中显式设置 `diskchunk_flush_write_timeout = -1`。对应的 [按表设置](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#diskchunk_flush_write_timeout) 优先级更高，会覆盖这个实例级默认值，从而提供更细粒度的控制。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
diskchunk_flush_write_timeout = 60s
```
<!-- end -->

### docstore_cache_size

<!-- example conf docstore_cache_size -->
此设置指定文档存储中保留在内存里的文档块最大大小。它是可选的，默认值为 16m（16 兆字节）。

当使用 `stored_fields` 时，文档块会从磁盘读取并解压。由于每个块通常包含多篇文档，它可能会在处理下一篇文档时被复用。为此，该块会保存在服务器级缓存中。缓存保存的是未压缩块。


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
此设置决定单个通配符可展开的最大关键词数量。它是可选的，默认值为 0（无限制）。

在对启用 `dict = keywords` 构建的表执行子串搜索时，单个通配符可能会匹配成千上万甚至数百万个关键词（想想用 `a*` 去匹配整个牛津词典）。该指令允许你限制此类扩展的影响。设置 `expansion_limit = N` 会将每个通配符的展开数量限制为不超过 N 个最常见的匹配关键词。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_limit = 16
```
<!-- end -->

### expansion_merge_threshold_docs

<!-- example conf expansion_merge_threshold_docs -->
此设置决定展开关键词中允许合并到一起的最大文档数。它是可选的，默认值为 32。

在对启用 `dict = keywords` 构建的表执行子串搜索时，单个通配符可能会匹配成千上万甚至数百万个关键词。该指令允许你提高可合并关键词数量的上限，以加快匹配，但会在搜索中使用更多内存。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_merge_threshold_docs = 1024
```
<!-- end -->

### expansion_merge_threshold_hits

<!-- example conf expansion_merge_threshold_hits -->
此设置决定展开关键词中允许合并到一起的最大命中数。它是可选的，默认值为 256。

在对启用 `dict = keywords` 构建的表执行子串搜索时，单个通配符可能会匹配成千上万甚至数百万个关键词。该指令允许你提高可合并关键词数量的上限，以加快匹配，但会在搜索中使用更多内存。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_merge_threshold_hits = 512
```
<!-- end -->

### expansion_phrase_limit

<!-- example conf expansion_phrase_limit -->
此设置控制由于 `PHRASE`、`PROXIMITY` 和 `QUORUM` 操作符中的 `OR` 运算符而生成的替代表达式数量上限。它是可选的，默认值为 1024。

在短语类操作符内部使用 `|`（OR）运算符时，展开后的组合总数可能会随着所指定的可选项数量呈指数增长。此设置通过限制查询处理期间考虑的排列数量，帮助防止查询过度展开。

如果生成的变体数量超过此限制，查询将会：

- 失败并返回错误（默认行为）
- 如果启用了 `expansion_phrase_warning`，则返回部分结果并给出警告

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_phrase_limit = 4096
```
<!-- end -->

### expansion_phrase_warning

<!-- example conf expansion_phrase_warning -->
此设置控制当超过 `expansion_phrase_limit` 定义的查询展开限制时的行为。

默认情况下，查询会失败并返回错误信息。将 `expansion_phrase_warning` 设为 1 时，搜索会继续使用短语的部分转换结果（直到配置的限制），并且服务器会将警告信息连同结果集一起返回给用户。这样，即使查询过于复杂而无法完全展开，也能返回部分结果而不会彻底失败。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
expansion_phrase_warning = 1
```
<!-- end -->

### grouping_in_utc

此设置指定 API 和 SQL 中的时间分组是按本地时区计算还是按 UTC 计算。它是可选的，默认值为 0（表示“本地时区”）。

默认情况下，所有按时间分组的表达式（例如 API 中按天、周、月、年分组，以及 SQL 中按天、月、年、yearmonth、yearmonthday 分组）都使用本地时间。例如，如果你有属性时间为 `13:00 utc` 和 `15:00 utc` 的文档，在分组时，它们都会根据你的本地时区设置落入相应组中。如果你位于 `utc`，它们会落在同一天；但如果你位于 `utc+10`，这两个文档会被分到不同的 `group by day` 组中（因为 UTC+10 时区里的 13:00 utc 是本地时间 23:00，而 15:00 是次日 01:00）。有时这种行为并不可接受，人们希望时间分组不依赖时区。你可以在启动服务器时定义全局 TZ 环境变量，但这不仅会影响分组，还会影响日志中的时间戳，这也可能不是你想要的。开启此选项（无论是在配置中，还是在 SQL 中使用 [SET global](../Server_settings/Setting_variables_online.md#SET) 语句）会让所有时间分组表达式都按 UTC 计算，而其余依赖时间的功能（例如服务器日志）仍保持本地时区。


### timezone

此设置指定日期/时间相关函数使用的时区。默认使用本地时区，但你可以指定不同的 IANA 格式时区（例如 `Europe/Amsterdam`）。

请注意，此设置不会影响日志记录，日志始终使用本地时区。

另外，请注意如果使用了 `grouping_in_utc`，那么“按时间分组”函数仍会使用 UTC，而其他日期/时间相关函数会使用指定的时区。总体而言，不建议混用 `grouping_in_utc` 和 `timezone`。

你可以在配置中设置此选项，也可以在 SQL 中使用 [SET global](../Server_settings/Setting_variables_online.md#SET) 语句进行配置。


### ha_period_karma

<!-- example conf ha_period_karma -->
此设置指定 agent 镜像统计窗口的大小，单位为秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。它是可选的，默认值为 60 秒。

对于包含 agent 镜像的分布式表（更多信息见 [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)），master 会跟踪多个不同的每镜像计数器。这些计数器随后用于故障转移和负载均衡（master 会根据计数器选择最佳镜像）。计数器按 `ha_period_karma` 秒为一个块进行累积。

在开始新的块之后，master 可能仍会继续使用前一个块中的累积值，直到新块填满一半。因此，任何先前历史在最多 1.5 倍 ha_period_karma 秒后就不再影响镜像选择。

虽然镜像选择最多只使用两个块，但为了做监控分析，系统会保留最多 15 个最近的块。可以使用 [SHOW AGENT STATUS](../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS) 语句查看这些块。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_period_karma = 2m
```
<!-- end -->


### ha_ping_interval

<!-- example conf ha_ping_interval -->
此设置配置 agent 镜像 ping 的间隔，单位为毫秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。它是可选的，默认值为 1000 毫秒。

对于包含 agent 镜像的分布式表（更多信息见 [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)），master 会在空闲期间向所有镜像发送 ping 命令。这用于跟踪当前 agent 状态（存活或死亡、网络往返时间等）。这些 ping 之间的间隔由此指令定义。要禁用 ping，可将 ha_ping_interval 设为 0。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_ping_interval = 3s
```
<!-- end -->


### hostname_lookup

`hostname_lookup` 选项定义了刷新主机名解析的策略。默认情况下，agent 主机名对应的 IP 地址会在服务器启动时缓存，以避免过度访问 DNS。但在某些场景下，IP 可能会动态变化（例如云主机），这时可能希望不要缓存 IP。将此选项设为 `request` 会禁用缓存，并为每个查询都去查询 DNS。也可以使用 [`FLUSH HOSTNAMES`](../Securing_and_compacting_a_table/Flushing_hostnames.md) 命令手动刷新 IP 地址。

### jobs_queue_size

jobs_queue_size 设置定义了队列中可以同时存在多少个“job”。默认情况下不限制。

在大多数情况下，一个“job”表示对单个本地表的一个查询（普通表，或实时表的一个磁盘 chunk）。例如，如果你有一个由 2 个本地表组成的分布式表，或者一个有 2 个磁盘 chunk 的实时表，那么对其中任一表发起的搜索查询通常会把 2 个 job 放入队列。然后，由 [threads](../Server_settings/Searchd.md#threads) 定义大小的线程池会处理它们。不过，在某些情况下，如果查询过于复杂，可能会创建更多 job。当 [max_connections](../Server_settings/Searchd.md#max_connections) 和 [threads](../Server_settings/Searchd.md#threads) 还不足以在所需性能之间找到平衡时，建议调整这个设置。

### join_batch_size

表连接通过累积一批匹配结果来工作，这些结果是对左表执行查询的输出。然后，这一批结果会作为一个单独的查询在右表上处理。

此选项允许你调整批大小。默认值为 `1000`，将此选项设为 `0` 会禁用批处理。

更大的批大小可能提升性能；不过，对于某些查询，它也可能导致过高的内存消耗。

<!-- example conf join_batch_size -->
<!-- intro -->
##### 配置：

<!-- request Config -->
```ini
join_batch_size = 2000
```
<!-- end -->

### join_cache_size

对右表执行的每个查询都由特定的 JOIN ON 条件定义，这些条件决定了从右表检索到的结果集。

如果只有少数几个唯一的 JOIN ON 条件，重用结果通常比反复执行右表查询更高效。为实现这一点，结果集会存入缓存。

此选项允许你配置该缓存的大小。默认值为 `20 MB`，将此选项设为 0 会禁用缓存。

请注意，每个线程都有自己的缓存，因此在估算总内存使用量时应考虑执行查询的线程数量。

<!-- example conf join_cache_size -->
<!-- intro -->
##### 配置：

<!-- request Config -->
```ini
join_cache_size = 10M
```
<!-- end -->

### listen_backlog

<!-- example conf listen_backlog -->
listen_backlog 设置决定传入连接的 TCP listen backlog 长度。这对逐个处理请求的 Windows 构建尤其相关。当连接队列达到上限时，新的传入连接会被拒绝。
对于非 Windows 构建，默认值通常已经足够，一般不需要调整此设置。


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

某些版本的 Kibana 和 OpenSearch Dashboards 期望服务器报告特定的版本号，并且可能会据此表现出不同的行为。为规避这类问题，你可以使用这个设置，让 Manticore 向 Kibana 或 OpenSearch Dashboards 报告自定义版本。

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

`listen` 的通用语法是：

```ini
listen = ( address ":" port | port | path | address ":" port start - port end ) [ ":" protocol [ "_vip" ] [ "_readonly" ] ]
```

你可以指定：
* IP 地址（或主机名）和端口号
* 或者只指定端口号
* 或者 Unix 套接字路径（Windows 不支持）
* 或者 IP 地址和端口范围

如果你指定了端口号但没有指定地址，`searchd` 会监听所有网络接口。Unix 路径通过前导斜杠识别。端口范围只能用于复制协议。

你还可以指定一个用于该套接字连接的协议处理器（监听器）。可用的监听器有：

* **未指定** - Manticore 将接受来自以下来源的连接：
  - 其他 Manticore agent（即远程分布式表）
  - 通过 HTTP 和 HTTPS 的客户端
  - [Manticore Buddy](https://manticoresearch.com/blog/manticoresearch-buddy-intro/)。**请确保有这种类型的监听器（或者下面提到的 `http` 监听器），以避免 Manticore 功能受限。**
* `mysql` - 用于来自 MySQL 客户端的 MySQL 协议连接。注意：
  - 也支持压缩协议。
  - 如果启用了 [SSL](../Security/SSL.md#SSL)，可以建立加密连接。
* `replication` - 用于节点通信的复制协议。更多细节见 [replication](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md) 章节。你可以指定多个 replication 监听器，但它们必须都监听同一个 IP；只允许端口不同。当你为 replication 监听器定义端口范围时（例如 `listen = 192.168.0.1:9320-9328:replication`），Manticore 不会立即开始在这些端口上监听。相反，它只会在你开始使用复制时，从指定范围中随机取用空闲端口。复制要正常工作，端口范围中至少需要 2 个端口。
* `http` - 与 **未指定** 相同。Manticore 将接受来自远程 agent 和通过 HTTP、HTTPS 的客户端在该端口上的连接。
* `https` - HTTPS 协议。Manticore 将在该端口上**只**接受 HTTPS 连接。更多细节见 [SSL](../Security/SSL.md) 章节。
* `sphinx` - 旧版二进制协议。用于为远程 [SphinxSE](../Extensions/SphinxSE.md) 客户端提供连接服务。某些 Sphinx API 客户端实现（例如 Java 版本）需要显式声明该监听器。

在客户端协议后添加后缀 `_vip`（也就是除 `replication` 之外的所有协议，例如 `mysql_vip`、`http_vip` 或直接使用 `_vip`）会强制为该连接创建专用线程，以绕过不同的限制。这在节点严重过载、否则会卡住或无法通过普通端口连接时，对节点维护非常有用。

后缀 `_readonly` 会为该监听器设置[只读模式](../Security/Read_only.md)，并将其限制为只接受读查询。

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

可以有多个 `listen` 指令。`searchd` 会在所有指定的端口和套接字上监听客户端连接。Manticore 包中提供的默认配置定义了以下监听端口：
* `9308` 和 `9312` 用于来自远程 agent 和非 MySQL 客户端的连接
* 以及 `9306` 端口用于 MySQL 连接。

如果你在配置中完全不指定任何 `listen`，Manticore 会在以下地址等待连接：
* `127.0.0.1:9306` 用于 MySQL 客户端
* `127.0.0.1:9312` 用于 HTTP/HTTPS，以及来自其他 Manticore 节点和基于 Manticore 二进制 API 的客户端的连接。

#### 监听特权端口

默认情况下，Linux 不允许你让 Manticore 监听 1024 以下的端口（例如 `listen = 127.0.0.1:80:http` 或 `listen = 127.0.0.1:443:https`），除非你以 root 身份运行 searchd。如果你仍希望在非 root 用户下启动 Manticore，并让它监听 < 1024 的端口，可以考虑以下方法之一（任一方法都应可行）：
* 运行命令 `setcap CAP_NET_BIND_SERVICE=+eip /usr/bin/searchd`
* 将 `AmbientCapabilities=CAP_NET_BIND_SERVICE` 添加到 Manticore 的 systemd 单元中，并重新加载守护进程（`systemctl daemon-reload`）。

#### 关于 Sphinx API 协议和 TFO 的技术细节
<details>
旧版 Sphinx 协议有两个阶段：握手交换和数据流。握手由客户端发出的 4 字节包和守护进程发出的 4 字节包组成，唯一目的在于客户端判断对端是否是真正的 Sphinx 守护进程，守护进程判断对端是否是真正的 Sphinx 客户端。主要数据流非常简单：双方先声明各自的握手，对方再进行校验。由于交换的是小包，因此需要使用特殊的 `TCP_NODELAY` 标志，它会关闭 Nagle 的 TCP 算法，并把 TCP 连接描述为一连串小包的对话。
不过，在这次协商中，谁先发话并没有被严格定义。历史上，所有使用二进制 API 的客户端都会先说话：先发送握手，然后读取守护进程返回的 4 字节，再发送请求并读取守护进程的应答。
在改进 Sphinx 协议兼容性时，我们考虑了这些事情：

1. 通常，master-agent 通信是从一个已知客户端到一个已知主机、已知端口建立的。所以，端点提供错误握手的可能性很低。因此，我们可以隐式假设双方都是有效的，并且确实在使用 Sphinx 协议。
2. 基于这个假设，我们可以把握手和真实请求“粘”在一起，并一次性发送。若后端是旧版 Sphinx 守护进程，它会把这个粘合后的包先当作 4 字节握手读取，再读取请求体。由于二者都在同一个包里到达，后端套接字相当于少了 1 次 RTT，而前端缓冲区依旧可以按通常方式工作。
3. 延续这个假设：由于“查询”包相当小，而握手更小，那就让两者一起通过现代 TFO（tcp-fast-open）技术放进初始的 SYN TCP 包里发送。也就是说：我们用粘合后的握手 + 请求体包连接到远程节点。守护进程接受连接后，会立即在套接字缓冲区里同时拿到握手和请求体，就像它们来自第一个 TCP SYN 包一样。这又消除了 1 次 RTT。
4. 最后，让守护进程接受这个改进。实际上，从应用层来说，这意味着不要使用 `TCP_NODELAY`。而从系统层来说，这意味着要确保守护进程端启用 TFO 接收，客户端端也启用 TFO 发送。默认情况下，现代系统里的客户端 TFO 通常已经启用，所以为了让一切正常工作，你只需要调好服务器端的 TFO。

这些改进在不改变协议本身的情况下，帮助我们减少了 1.5 次 TCP 往返。换句话说，如果查询和应答能够放进一个 TCP 包，那么整个二进制 API 会话可以从 3.5 RTT 降到 2 RTT，这使得网络协商大约快 2 倍。

因此，我们所有的改进都围绕一个最初未定义的问题展开：‘谁先说话。’如果由客户端先说，我们就可以应用所有这些优化，并实际把连接 + 握手 + 查询放进一个 TFO 包里处理。更进一步，我们可以查看收到的包的开头并判断真实协议。这就是为什么你可以通过 API/http/https 连接到同一个端口。如果必须由守护进程先说话，那么所有这些优化都不可能实现，多协议也不可能实现。这就是为什么我们为 MySQL 单独保留了一个端口，而没有把它和其他协议统一到同一个端口。突然之间，在所有客户端中，有一个客户端是按照“守护进程应该先发送握手”的前提编写的。也就是说，它无法利用上述所有改进。这就是用于 mysql/mariadb 的 SphinxSE 插件。所以，专门针对这一个客户端，我们保留了 `sphinx` 协议定义，以最传统的方式工作。也就是：双方都启用 `TCP_NODELAY`，并用小包交换。守护进程在连接时发送自己的握手，然后客户端发送自己的握手，接着一切照常运行。这并不算高效，但确实能工作。如果你使用 SphinxSE 连接到 Manticore，就必须为它单独配置一个显式声明为 `sphinx` 协议的监听器。对于其他客户端，不要使用这个监听器，因为它更慢。如果你使用其他旧版 Sphinx API 客户端，先检查它们是否能在非专用多协议端口上工作。对于 master-agent 连接，使用非专用（多协议）端口并启用客户端和服务器端 TFO 效果很好，肯定会让网络后端更快，尤其是在你的查询非常轻量且很快的情况下。
</details>

### listen_tfo

此设置为所有监听器允许 TCP_FASTOPEN 标志。默认情况下，它由系统管理，但可以通过设置为 '0' 显式关闭。

关于 TCP Fast Open 扩展的一般知识，请参阅 [Wikipedia](https://en.wikipedia.org/wiki/TCP_Fast_Open)。简而言之，它允许在建立连接时减少一次 TCP 往返。

在实践中，在许多场景下使用 TFO 可以优化客户端到 agent 的网络效率，就像使用 [persistent agents](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 一样，但又不需要保持活跃连接，也不受最大连接数限制。

在现代操作系统上，TFO 支持通常在系统层面已开启，但这只是“能力”，并非默认规则。Linux 作为最积极的系统，自 2011 年起，在从 3.7 开始的内核上就支持它（服务器端）。Windows 从某些 Windows 10 构建版本开始支持它。其他操作系统（FreeBSD、MacOS）也在支持之列。

对于 Linux，系统会检查变量 `/proc/sys/net/ipv4/tcp_fastopen` 并按其行为。bit 0 管理客户端侧，bit 1 管理监听器。默认情况下，该参数设为 1，即启用客户端，禁用监听器。

### log

<!-- example conf log -->
log 设置指定记录所有 `searchd` 运行时事件的日志文件名。如果未指定，默认名称为 `searchd.log`。

或者，你也可以将文件名设为 `syslog`。此时，事件会发送到 syslog 守护进程。要使用 syslog 选项，你需要在构建 Manticore 时使用 `-–with-syslog` 选项进行配置。


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

在使用 [multi-queries](../Searching/Multi-queries.md) 时，这会让 searchd 对单个批次中提交的查询数量进行合理性检查。将其设为 0 可跳过检查。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_batch_queries = 256
```
<!-- end -->

### max_connections

<!-- example max_connections -->
最大同时客户端连接数。默认不限制。通常只有在使用持久连接时才会明显体现，例如 cli mysql 会话或来自远程分布式表的持久远程连接。当达到上限时，你仍然可以通过 [VIP connection](../Connecting_to_the_server/MySQL_protocol.md#VIP-connection) 连接到服务器。VIP 连接不计入该上限。

<!-- request Example -->
```ini
max_connections = 10
```

<!-- end -->

### max_threads_per_query

<!-- example max_threads_per_query -->
单个操作可使用的线程数实例级上限。默认情况下，适合的操作可能会占用所有 CPU 核心，从而不给其他操作留下空间。例如，对一个非常大的 percolate 表执行 `call pq` 可能会占用所有线程几十秒。将 `max_threads_per_query` 设为比如 [threads](../Server_settings/Searchd.md#threads) 的一半，可以确保你能够并行运行几次这样的 `call pq` 操作。

你也可以在运行时将这个设置作为会话变量或全局变量设置。

此外，你还可以借助 [threads OPTION](../Searching/Options.md#threads) 按查询控制行为。

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
服务器允许打开的最大文件数称为“软限制”。请注意，服务大型碎片化实时表可能需要将这个限制设得很高，因为每个磁盘 chunk 可能占用十几个甚至更多文件。例如，一个有 1000 个 chunk 的实时表可能需要同时打开数千个文件。如果你在日志中遇到 'Too many open files' 错误，请尝试调整这个选项，它可能有助于解决问题。

还有一个“硬限制”，这个选项不能超过。该限制由系统定义，并可在 Linux 的 `/etc/security/limits.conf` 文件中修改。其他操作系统可能有不同的方式，请查阅相应手册以获取更多信息。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = 10000
```
<!-- end -->

<!-- example conf max_open_files max -->
除了直接使用数值外，你还可以使用神奇关键字 `max`，将限制设置为当前可用的硬限制值。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
max_open_files = max
```
<!-- end -->


### max_packet_size

<!-- example conf max_packet_size -->
允许的最大网络包大小。此设置同时限制来自客户端的查询包和分布式环境中远程 agent 的响应包。它仅用于内部合理性检查，不会直接影响 RAM 使用或性能。可选，默认值为 128M。


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

某些较挑剔的 MySQL 客户端库依赖于 MySQL 使用的特定版本号格式，而且有时会根据报告的版本号（而不是给出的能力标志）选择不同的执行路径。例如，Python MySQLdb 1.2.2 在版本号不是 X.Y.ZZ 格式时会抛出异常；MySQL .NET connector 6.3.x 在版本号为 1.x 且某些标志组合下会内部失败，等等。为了解决这个问题，你可以使用 `mysql_version_string` 指令，让 `searchd` 对通过 MySQL 协议连接的客户端报告不同的版本。（默认情况下，它会报告自己的版本。）


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
mysql_version_string = 5.0.37
```
<!-- end -->


### net_workers

网络线程数量，默认值为 1。

当仅靠一个线程不足以处理所有传入查询时，这个设置对极高查询速率场景很有用。


### net_wait_tm

控制网络线程忙循环的间隔。默认值为 -1，可设置为 -1、0 或正整数。

当服务器配置为纯 master 并且只将请求路由给 agent 时，重要的是要无延迟地处理请求，并且不让网络线程睡眠。为此会使用忙循环。收到传入请求后，如果 `net_wait_tm` 是正数，网络线程会在 `10 * net_wait_tm` 毫秒内进行 CPU 轮询；如果 `net_wait_tm` 为 `0`，则只用 CPU 轮询。也可以通过将 `net_wait_tm = -1` 来禁用忙循环，在这种情况下，轮询器会把超时设置为系统轮询调用中实际 agent 的超时。

> **警告：** CPU 忙循环实际上会占用 CPU 核心，因此将该值设为任何非默认值，都会让 CPU 使用率明显上升，即使服务器处于空闲状态也是如此。


### net_throttle_accept

定义网络循环每次迭代接受多少客户端。默认值为 0（不限制），对大多数用户都足够。这是一个用于高负载场景下控制网络循环吞吐量的精细调优选项。


### net_throttle_action

定义网络循环每次迭代处理多少个请求。默认值为 0（不限制），对大多数用户都足够。这是一个用于高负载场景下控制网络循环吞吐量的精细调优选项。

### network_timeout

<!-- example conf network_timeout -->
网络客户端请求读写超时，单位为秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认值为 5 秒。`searchd` 会强制关闭在此超时内未能发送查询或读取结果的客户端连接。

还要注意 [reset_network_timeout_on_packet](../Server_settings/Searchd.md#reset_network_timeout_on_packet) 参数。该参数会将 `network_timeout` 的行为从应用于整个 `query` 或 `result`，改为应用于单个数据包。通常，一个查询/结果会落在一到两个数据包内。不过，在需要大量数据的场景中，这个参数对于维持活跃操作非常有价值。

<!-- request Example -->

```ini
network_timeout = 10s
```
<!-- end -->

### node_address

<!-- example conf node_address -->
此设置允许你指定节点的网络地址。默认情况下，它会设置为复制 [listen](../Server_settings/Searchd.md#listen) 地址。在大多数情况下这是正确的；不过，在某些场景下你必须手动指定它：

* 节点位于防火墙之后
* 启用了网络地址转换（NAT）
* 容器化部署，例如 Docker 或云部署
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
此设置决定是否允许只包含 [negation](../Searching/Full_text_matching/Operators.md#Negation-operator) 全文操作符的查询。可选，默认值为 0（仅包含 NOT 操作符的查询会失败）。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
not_terms_only_allowed = 1
```
<!-- end -->

### optimize_cutoff

<!-- example conf optimize_cutoff -->
设置默认的表压缩阈值。更多内容请参见 - [优化的磁盘 chunk 数量](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks)。此设置可通过按查询选项 [cutoff](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 覆盖。也可以通过 [SET GLOBAL](../Server_settings/Setting_variables_online.md#SET) 动态修改。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
optimize_cutoff = 4
```
<!-- end -->

### persistent_connections_limit

<!-- example conf persistent_connections_limit -->
此设置决定到远程 [persistent agents](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) 的同时持久连接最大数量。每次连接在 `agent_persistent` 下定义的 agent 时，系统都会尝试复用现有连接（如果有），或者建立连接并保存起来供以后使用。不过，在某些情况下，限制这类持久连接数量是合理的。此指令定义了这个上限。它会影响所有分布式表中到每个 agent 主机的连接数。

通常将该值设为等于或小于 agent 配置中的 [max_connections](../Server_settings/Searchd.md#max_connections) 选项是合理的。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
persistent_connections_limit = 29 # assume that each host of agents has max_connections = 30 (or 29).
```
<!-- end -->


### pid_file

<!-- example conf pid_file -->
`pid_file` 是一个配置选项，用于指定保存 `searchd` 服务器进程 ID（PID）的文件路径。

PID 文件会在启动时创建并加锁，在服务器运行期间保存主服务器进程 ID。服务器关闭时会删除该文件。这个文件可帮助 Manticore 执行一些内部任务，例如：

* 验证 `searchd` 实例是否已经在运行。
* 停止 `searchd` 进程。
* 触发表轮转。

PID 文件也可供外部自动化脚本使用。

**要求：**
如果 `searchd` 以 `--console`、`--nodetach` 或 `--systemd` 选项运行，或者系统已自动检测到 systemd 管理，则 `pid_file` 是可选的。在其他所有情况下，此设置都是必需的。如果使用了 `--pidfile` 命令行选项，也需要它。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
pid_file = /run/manticore/searchd.pid
```
<!-- end -->

### preopen_tables

<!-- example conf preopen_tables -->
preopen_tables 配置指令指定是否在启动时强制预打开所有表。默认值为 1，这意味着无论按表的 [preopen](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Other-performance-related-settings) 设置如何，所有表都会被预打开。如果设为 0，则按表设置可以生效，并且它们的默认值也会是 0。

预打开表可以防止查询与轮转之间的竞争条件，避免查询偶尔失败。不过，它也会占用更多文件句柄。在大多数场景中，建议预打开表。

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
pseudo_sharding 配置选项会启用对本地普通表和实时表搜索查询的并行化，无论这些表是直接查询还是通过分布式表查询。此功能会自动将查询并行化，最多并行到 `searchd.threads` 指定的线程数。

请注意，如果你的 worker 线程已经很忙，因为你有：
* 很高的查询并发
* 任意形式的物理分片：
  - 由多个普通/实时表组成的分布式表
  - 由太多磁盘 chunk 构成的实时表

那么启用 pseudo_sharding 可能不会带来任何收益，甚至可能略微降低吞吐量。如果你更看重吞吐量而不是更低的延迟，建议禁用此选项。

默认启用。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
pseudo_sharding = 0
```
<!-- end -->


### replication_connect_timeout

`replication_connect_timeout` 指令定义连接远程节点的超时时间。默认情况下，值被视为毫秒，但也可以带有[其他后缀](../Server_settings/Special_suffixes.md)。默认值为 1000（1 秒）。

连接远程节点时，Manticore 最多会等待这么长时间以成功建立连接。如果达到超时时间但连接尚未建立，并且启用了 `retries`，则会发起重试。


### replication_query_timeout

`replication_query_timeout` 设置 searchd 等待远程节点完成查询的时间。默认值为 3000 毫秒（3 秒），但可以通过 `suffixed` 来表示不同的时间单位。

建立连接后，Manticore 最多会等待 `replication_query_timeout` 这么长时间让远程节点完成。请注意，这个超时与 `replication_connect_timeout` 是分开的，远程节点造成的总延迟可能是二者之和。


### replication_retry_count

此设置是一个整数，指定 Manticore 在复制期间尝试连接并查询远程节点多少次后，才报告致命查询错误。默认值为 3。


### replication_retry_delay

此设置是以毫秒为单位的整数（或 [special_suffixes](../Server_settings/Special_suffixes.md)），指定在复制失败时 Manticore 重试查询远程节点之前的延迟。只有在指定了非零值时，这个值才有意义。默认值为 500。

### qcache_max_bytes

<!-- example conf qcache_max_bytes -->
此配置设置可用于缓存结果集的最大 RAM 容量，单位为字节。默认值为 16777216，相当于 16 兆字节。如果将其设为 0，则会禁用查询缓存。有关查询缓存的更多信息，请参阅 [query cache](../Searching/Query_cache.md)。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
qcache_max_bytes = 16777216
```
<!-- end -->


### qcache_thresh_msec

整数，单位为毫秒。查询结果被缓存所需的最小耗时阈值。默认值为 3000，即 3 秒。0 表示缓存所有内容。详情请参阅 [query cache](../Searching/Query_cache.md)。此值也可以使用时间 [special_suffixes](../Server_settings/Special_suffixes.md) 表示，但请谨慎使用，不要被值名中的 `_msec` 搞混。


### qcache_ttl_sec

整数，单位为秒。缓存结果集的过期时间。默认值为 60，即 1 分钟。最小值为 1 秒。详情请参阅 [query cache](../Searching/Query_cache.md)。此值也可以使用时间 [special_suffixes](../Server_settings/Special_suffixes.md) 表示，但请谨慎使用，不要被值名中的 `_sec` 搞混。


### query_log_format

<!-- example conf query_log_format -->
查询日志格式。可选，允许的值为 `plain` 和 `sphinxql`，默认值为 `sphinxql`。

`sphinxql` 模式会记录有效的 SQL 语句。`plain` 模式会以纯文本格式记录查询（主要适用于纯全文检索场景）。此指令允许你在搜索服务器启动时在两种格式之间切换。日志格式也可以在运行时通过 `SET GLOBAL query_log_format=sphinxql` 语法更改。更多细节请参阅 [Query logging](../Logging/Query_logging.md)。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_format = sphinxql
```
<!-- end -->

### query_log_min_msec

用于阻止查询被写入查询日志的阈值（单位为毫秒）。可选，默认值为 0（所有查询都会写入查询日志）。此指令表示只有执行时间超过指定阈值的查询才会被记录（此值也可以使用时间 [special_suffixes](../Server_settings/Special_suffixes.md) 表示，但请谨慎使用，不要被值名中的 `_msec` 搞混）。

### query_log

<!-- example conf query_log -->
查询日志文件名。可选，默认值为空（不记录查询）。所有搜索查询（例如 SELECT ...，但不包括 INSERT/REPLACE/UPDATE 查询）都会记录到这个文件中。格式请参见 [Query logging](../Logging/Query_logging.md)。在 `plain` 格式下，你可以使用 `syslog` 作为日志文件路径。在这种情况下，所有搜索查询都会发送到 syslog 守护进程，优先级为 `LOG_INFO`，并以 `[query]` 作为前缀而不是时间戳。要使用 syslog 选项，Manticore 必须在构建时使用 `-–with-syslog` 进行配置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log = /var/log/query.log
```
<!-- end -->


### query_log_mode

<!-- example conf query_log_mode -->
query_log_mode 指令允许你为 searchd 和查询日志文件设置不同的权限。默认情况下，这些日志文件以 600 权限创建，这意味着只有运行服务器的用户和 root 用户可以读取日志文件。
如果你想让其他用户也能读取日志文件，这个指令会很方便，例如非 root 用户运行的监控方案。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
query_log_mode  = 666
```
<!-- end -->

### read_buffer_docs

<!-- example conf read_buffer_docs -->
read_buffer_docs 指令控制文档列表按关键词读取缓冲区的大小。对于搜索查询中的每次关键词出现，都有两个相关的读取缓冲区：一个用于文档列表，一个用于命中列表。此设置允许你控制文档列表缓冲区大小。

更大的缓冲区可能会增加每个查询的 RAM 使用量，但也可能减少 I/O 时间。对于慢速存储，设置更大的值是合理的；对于 IOPS 很高的存储，则应在较低值范围内尝试。

默认值为 256K，最小值为 8K。你也可以按表设置 [read_buffer_docs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs)，它会覆盖服务器配置级别的所有设置。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_docs = 128K
```
<!-- end -->


### read_buffer_hits

<!-- example conf read_buffer_hits -->
read_buffer_hits 指令指定搜索查询中命中列表按关键词读取缓冲区的大小。默认大小为 256K，最小值为 8K。对于搜索查询中的每次关键词出现，都有两个相关的读取缓冲区，一个用于文档列表，一个用于命中列表。增大缓冲区可能会增加每个查询的 RAM 使用量，但会减少 I/O 时间。对于慢速存储，较大的缓冲区是合理的；对于 IOPS 很高的存储，则应在较低值范围内尝试。

该设置也可以在按表级别通过 [read_buffer_hits](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_hits) 选项指定，它会覆盖服务器级设置。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
read_buffer_hits = 128K
```
<!-- end -->

### read_unhinted

<!-- example conf read_unhinted -->
未预估读取大小。可选，默认值为 32K，最小值为 1K

在查询时，有些读取事先就知道要读多少数据，而有些目前并不知道。最典型的是命中列表大小目前无法预先知道。此设置允许你控制在这种情况下要读取多少数据。它会影响命中列表的 I/O 时间，对于大于未预估读取大小的列表会减少时间，但对于更小的列表会增加时间。它**不会**影响 RAM 使用，因为读取缓冲区已经分配好了。因此它不应大于 read_buffer。


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

当设为 0 时，超时限制的是发送整个请求/查询的最大时间。
当设为 1（默认）时，超时限制的是网络活动之间的最大时间。

在复制场景中，某个节点可能需要向另一个节点发送一个大文件（例如 100GB）。假设网络传输速度为 1GB/s，包大小为 4-5MB，那么传完整个文件需要 100 秒。默认 5 秒的超时只允许传输 5GB，连接就会被断开。提高超时时间可以作为一种变通办法，但这并不具备可扩展性（例如下一个文件可能有 150GB，又会失败）。不过，在默认 `reset_network_timeout_on_packet` 设为 1 时，超时不是作用于整个传输，而是作用于单个数据包。只要传输仍在进行中（并且在超时期间确实有数据通过网络接收），连接就会保持存活。如果传输卡住，导致数据包之间出现超时，就会被断开。

请注意，如果你设置了分布式表，那么每个节点的参数都需要调优，包括 master 和 agent。master 端受 `agent_query_timeout` 影响；agent 端则相关的是 `network_timeout`。

<!-- intro -->

##### 示例：

<!-- request Example -->

```ini
reset_network_timeout_on_packet = 0
```

<!-- end -->


### rt_flush_period

<!-- example conf rt_flush_period -->
RT 表 RAM chunk 的刷写检查周期，单位为秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认值为 10 小时。

持续更新且完全放得下 RAM chunk 的 RT 表，仍可能导致 binlog 不断增长，影响磁盘使用和崩溃恢复时间。通过这个指令，搜索服务器会周期性执行刷写检查，符合条件的 RAM chunk 可以被保存，从而触发相应的 binlog 清理。更多细节请参见 [Binary logging](../Logging/Binary_logging.md)。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_flush_period = 3600 # 1 hour
```
<!-- end -->


### rt_merge_iops

<!-- example conf rt_merge_iops -->
RT chunk 合并线程允许启动的最大 I/O 操作数（每秒）。可选，默认值为 0（无限制）。

此指令允许你降低 `OPTIMIZE` 语句带来的 I/O 影响。可以保证所有 RT 优化活动不会产生超过配置上限的磁盘 IOPS（每秒 I/O 次数）。限制 rt_merge_iops 可以减轻合并导致的搜索性能下降。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_iops = 40
```
<!-- end -->

### rt_merge_maxiosize

<!-- example conf rt_merge_maxiosize -->
RT chunk 合并线程允许启动的最大 I/O 操作大小。可选，默认值为 0（无限制）。

此指令允许你降低 `OPTIMIZE` 语句带来的 I/O 影响。大于该限制的 I/O 会被拆分成两个或更多 I/O，并在 [rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops) 限制下按独立 I/O 计数。因此，可以保证所有优化活动每秒产生的磁盘 I/O 不超过 `(rt_merge_iops * rt_merge_maxiosize)` 字节。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
rt_merge_maxiosize = 1M
```
<!-- end -->


### seamless_rotate

<!-- example conf seamless_rotate -->
在轮转包含大量需预缓存数据的表时，防止 `searchd` 卡顿。可选，默认值为 1（启用无缝轮转）。在 Windows 系统上，默认禁用无缝轮转。

表中可能包含一些需要预缓存到 RAM 的数据。目前，`.spa`、`.spb`、`.spi` 和 `.spm` 文件会被完全预缓存（它们分别包含属性数据、blob 属性数据、关键词表和已删除行映射。）。如果没有无缝轮转，轮转表时会尽量少占用 RAM，并按如下方式工作：

1. 暂时拒绝新查询（返回 "retry" 错误码）；
2. `searchd` 等待当前正在运行的所有查询完成；
3. 释放旧表，并重命名其文件；
4. 重命名新表文件，并分配所需 RAM；
5. 将新表的属性和词典数据预加载到 RAM；
6. `searchd` 恢复使用新表提供查询服务。

不过，如果属性或词典数据很多，预加载步骤可能会花费明显的时间 - 在预加载 1-5+ GB 文件时甚至可能需要几分钟。

启用无缝轮转后，轮转过程如下：

1. 分配新表的 RAM 存储；
2. 异步将新表的属性和词典数据预加载到 RAM；
3. 成功后，释放旧表，并重命名两张表的文件；
4. 失败时，释放新表；
5. 在任意时刻，查询要么由旧表副本提供，要么由新表副本提供。

无缝轮转的代价是在轮转期间峰值内存使用更高（因为在预加载新副本时，`.spa/.spb/.spi/.spm` 数据的旧副本和新副本都需要在 RAM 中）。平均使用量保持不变。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
seamless_rotate = 1
```
<!-- end -->

### secondary_index_block_cache
<!-- example conf secondary_index_block_cache -->

此选项指定二级索引使用的块缓存大小。它是可选的，默认值为 8 MB。当二级索引与包含大量值的过滤器（例如 IN() 过滤器）协同时，它们会读取并处理这些值的元数据块。
在连接查询中，这个过程会对左表的每一批行重复执行，而且每一批都可能在同一个连接查询中再次读取相同的元数据。这会严重影响性能。元数据块缓存会将这些块保存在内存中，因此它们
可以被后续批次复用。

该缓存只在连接查询中使用，对非连接查询没有影响。请注意，缓存大小上限是按属性和按二级索引分别计算的。每个磁盘 chunk 中的每个属性都在这个限制内运行。在最坏情况下，总内存
使用量可以通过将该上限乘以磁盘 chunk 数量以及连接查询中使用的属性数量来估算。

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

此选项启用/禁用搜索查询使用二级索引。它是可选的，默认值为 1（启用）。请注意，你不需要为索引阶段显式启用它，因为只要安装了 [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)，它就始终处于启用状态。搜索时使用这些索引也需要后者。共有三种模式可用：

* `0`：禁用搜索时使用二级索引。可通过 [analyzer hints](../Searching/Options.md#Query-optimizer-hints) 为单个查询启用
* `1`：启用搜索时使用二级索引。可通过 [analyzer hints](../Searching/Options.md#Query-optimizer-hints) 为单个查询禁用
* `force`：与启用相同，但加载二级索引时出现的任何错误都会被报告，并且整个索引不会被加载到守护进程中。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
secondary_indexes = 1
```

<!-- end -->

### server_id

<!-- example conf server_id -->
作为服务器标识符使用的整数，用作种子，为复制集群中的节点生成唯一的短 UUID。server_id 在集群各节点之间必须唯一，且范围为 0 到 127。如果未设置 server_id，则会将 MAC 地址和 PID 文件路径的哈希值作为种子，或者使用随机数作为短 UUID 的种子。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
server_id = 1
```
<!-- end -->


### shutdown_timeout

<!-- example conf shutdown_timeout -->
`searchd --stopwait` 的等待时间，单位为秒（或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认值为 60 秒。

当你运行 `searchd --stopwait` 时，服务器在停止前需要执行一些操作，例如完成查询、刷新 RT RAM chunk、刷写属性以及更新 binlog。这些任务需要一些时间。`searchd --stopwait` 会最多等待 `shutdown_time` 秒，让服务器完成这些工作。合适的时间取决于你的表大小和负载。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
shutdown_timeout = 3m # wait for up to 3 minutes
```
<!-- end -->


### shutdown_token

从 VIP Manticore SQL 连接中调用 `shutdown` 命令所需密码的 SHA1 哈希。没有它，[debug](../Reporting_bugs.md#DEBUG) 的 `shutdown` 子命令将永远不会让服务器停止。请注意，这种简单哈希不应被视为强保护，因为我们没有使用带盐哈希，也没有使用任何现代哈希函数。它旨在作为本地网络中维护守护进程的防误操作措施。

### skiplist_cache_size

<!-- example conf skiplist_cache_size -->
此设置指定解压后的 skiplist 的内存缓存最大大小。可选，默认值为 64M。

skiplist 用于加速大型 doclist 的定位。缓存它们可以避免在多个查询之间反复解压相同的 skiplist 数据。将此选项设为 `0` 可禁用缓存。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
skiplist_cache_size = 128M
```
<!-- end -->

### snippets_file_prefix

<!-- example conf snippets_file_prefix -->
生成摘要时附加到本地文件名之前的前缀。可选，默认值为当前工作目录。

这个前缀可与 `load_files` 或 `load_files_scattered` 选项一起用于分布式摘要生成。

请注意，这里是前缀，**不是**路径！这意味着，如果前缀设为 "server1"，而请求引用了 "file23"，`searchd` 会尝试打开 "server1file23"（以上内容均不含引号）。因此，如果你需要它作为路径，就必须包含结尾斜杠。

构造最终文件路径后，服务器会展开所有相对目录，并将最终结果与 `snippet_file_prefix` 的值进行比较。如果结果不是以前缀开头，这个文件就会被错误拒绝。

例如，如果你把它设为 `/mnt/data`，而有人用文件 `../../../etc/passwd` 作为源来调用摘要生成，他们会收到如下错误：

`File '/mnt/data/../../../etc/passwd' escapes '/mnt/data/' scope`

而不是文件内容。

另外，在未设置该参数的情况下读取 `/etc/passwd`，实际上会读取 /daemon/working/folder/etc/passwd，因为该参数的默认值是服务器的工作目录。

还要注意这是一个本地选项；它不会以任何方式影响 agent。因此你可以放心地在 master 服务器上设置前缀。路由到 agent 的请求不会受到 master 设置的影响，但会受到 agent 自身设置的影响。

当各服务器上的文档存储位置不一致时，这可能会很有用，例如本地存储或 NAS 挂载点不同。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
snippets_file_prefix = /mnt/common/server1/
```
<!-- end -->

> **警告：** 如果你仍然希望访问文件系统根目录中的文件，必须显式将 `snippets_file_prefix` 设为空值（通过 `snippets_file_prefix=` 这一行），或者设为根目录（通过 `snippets_file_prefix=/`）。


### sphinxql_state

<!-- example conf sphinxql_state -->
用于序列化当前 SQL 状态的文件路径。

服务器启动时会回放该文件。在符合条件的状态变更（例如 `SET GLOBAL`）时，这个文件会自动重写。这样可以避免一个很难排查的问题：如果你加载了 UDF 函数但 Manticore 崩溃了，当它（自动）重启后，你的 UDF 和全局变量将不再可用。使用持久化状态有助于确保平稳恢复，不会出现这类意外。

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
使用 SQL 接口时，两次请求之间的最大等待时间（单位为秒，或 [special_suffixes](../Server_settings/Special_suffixes.md)）。可选，默认值为 15 分钟。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
sphinxql_timeout = 15m
```
<!-- end -->


### ssl_ca

<!-- example conf ssl_ca -->
SSL 证书颁发机构（CA）证书文件的路径（也称根证书）。可选，默认值为空。当不为空时，`ssl_cert` 中的证书应由这个根证书签名。

服务器使用 CA 文件来验证证书上的签名。该文件必须是 PEM 格式。

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

服务器使用该证书作为自签名公钥，通过 SSL 加密 HTTP 流量。该文件必须是 PEM 格式。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ssl_cert = keys/server-cert.pem
```
<!-- end -->


### ssl_key

<!-- example conf ssl_key -->
SSL 证书私钥的路径。可选，默认值为空。

服务器使用该私钥通过 SSL 加密 HTTP 流量。该文件必须是 PEM 格式。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ssl_key = keys/server-key.pem
```
<!-- end -->


### subtree_docs_cache

<!-- example conf subtree_docs_cache -->
每个查询的公共子树文档缓存最大大小。可选，默认值为 0（禁用）。

此设置限制公共子树优化器的 RAM 使用量（见 [multi-queries](../Searching/Multi-queries.md)）。每个查询最多会使用这么多 RAM 来缓存文档条目。将限制设为 0 会禁用该优化器。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
subtree_docs_cache = 8M
```
<!-- end -->


### subtree_hits_cache

<!-- example conf subtree_hits_cache -->
每个查询的公共子树命中缓存最大大小。可选，默认值为 0（禁用）。

此设置限制公共子树优化器的 RAM 使用量（见 [multi-queries](../Searching/Multi-queries.md)）。每个查询最多会使用这么多 RAM 来缓存关键词出现次数（hits）。将限制设为 0 会禁用该优化器。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
subtree_hits_cache = 16M
```
<!-- end -->

### threads

<!-- example threads -->
Manticore 守护进程的工作线程数（或线程池大小）。Manticore 在启动时会创建这么多 OS 线程，它们会在守护进程内部执行所有任务，例如执行查询、生成摘要等。某些操作可以拆分成子任务并行执行，例如：

* 在实时表中搜索
* 在由本地表组成的分布式表中搜索
* 执行 percolate 查询调用
* 以及其他操作

默认情况下，它被设置为服务器上的 CPU 核心数。Manticore 会在启动时创建这些线程，并在停止前一直保留它们。每个子任务在需要时都可以使用其中一个线程。子任务完成后，会释放该线程，供其他子任务使用。

在 I/O 密集型负载场景下，将这个值设置得高于 CPU 核心数可能更合适。

<!-- request Example -->
```ini
threads = 10
```

<!-- end -->

### thread_stack

<!-- example conf thread_stack -->
单个 job（协程，一个搜索查询可能导致多个 job/协程）可用的最大栈大小。可选，默认值为 128K。

每个 job 都有自己 128K 的栈。当你运行查询时，系统会检查它需要多少栈。如果默认的 128K 足够，就直接处理；如果不够，就会安排另一个具有更大栈的 job 继续处理。这种增强栈的最大大小受此设置限制。

把这个值设置得相对高一些，有助于处理非常深的查询，而不会让整体 RAM 消耗增长过高。例如，将其设为 1G 并不意味着每个新 job 都会占用 1G RAM，但如果发现某个 job 需要比如 100M 栈，我们就只给它分配 100M。其他同时运行的 job 仍会使用默认的 128K 栈。同理，我们也可以运行更复杂、需要 500M 的查询。只有当我们**内部检测到**该 job 需要超过 1G 的栈时，才会失败并报告 thread_stack 太小。

不过在实践中，即使是需要 16M 栈的查询，往往也已经过于复杂，解析时会消耗过多时间和资源。因此，守护进程会处理它，但通过 `thread_stack` 设置来限制这类查询看起来是合理的。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
thread_stack = 8M
```
<!-- end -->


### unlink_old

<!-- example conf unlink_old -->
决定在成功轮转后是否删除 `.old` 表副本。可选，默认值为 1（删除）。


<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
unlink_old = 0
```
<!-- end -->


### watchdog

<!-- example conf watchdog -->
线程化服务器看门狗。可选，默认值为 1（启用看门狗）。

当 Manticore 查询崩溃时，可能会拖垮整个服务器。启用 watchdog 功能后，`searchd` 还会维护一个独立的轻量级进程，用于监控主服务器进程，并在异常终止时自动重启它。

<!-- request Example -->

```ini
watchdog = 0 # disable watchdog
```
<!-- end -->
<!-- proofread -->
