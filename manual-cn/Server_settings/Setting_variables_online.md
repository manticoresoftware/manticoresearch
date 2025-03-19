# 在线设置变量

## SET
```sql
SET [GLOBAL] server_variable_name = value
SET [INDEX index_name] GLOBAL @user_variable_name = (int_val1 [, int_val2, ...])
SET NAMES value [COLLATE value]
SET @@dummy_variable = ignored_value
```

Manticore Search 中的 `SET` 语句允许你修改变量值。变量名称不区分大小写，且在服务器重启后，变量值的更改将不会保留。

Manticore Search 支持 `SET NAMES` 语句和 `SET @@variable_name` 语法，以兼容第三方 MySQL 客户端库、连接器和框架，这些可能需要在连接时运行这些语句。然而，这些语句对 Manticore Search 本身没有任何影响。

Manticore Search 中有四类变量：

1. 会话级服务器变量：`set var_name = value`
2. 全局服务器变量：`set global var_name = value`
3. 全局用户变量：`set global @var_name = (value)`
4. 全局分布式变量：`set index dist_index_name global @var_name = (value)`

全局用户变量在并发会话之间共享。支持的唯一值类型是 BIGINT 的列表，这些变量可以与 IN() 操作符结合使用以进行过滤。此功能的主要用例是将大量值上传到 `searchd` 一次，并在以后的多次使用中重用，减少网络开销。全局用户变量可以转发到分布式表的所有代理，或者在本地表定义在分布式表的情况下设置。例如：

```sql
// in session 1
mysql> SET GLOBAL @myfilter=(2,3,5,7,11,13);
Query OK, 0 rows affected (0.00 sec)

// later in session 2
mysql> SELECT * FROM test1 WHERE group_id IN @myfilter;
+------+--------+----------+------------+-----------------+------+
| id   | weight | group_id | date_added | title           | tag  |
+------+--------+----------+------------+-----------------+------+
|    3 |      1 |        2 | 1299338153 | another doc     | 15   |
|    4 |      1 |        2 | 1299338153 | doc number four | 7,40 |
+------+--------+----------+------------+-----------------+------+
2 rows in set (0.02 sec)
```

Manticore Search 支持会话级和全局服务器变量，这些变量在各自的作用域内影响特定的服务器设置。以下是已知的会话级和全局服务器变量列表：

已知的会话级服务器变量：

- `AUTOCOMMIT = {0 | 1}` 确定数据修改语句是否应隐式包裹在 `BEGIN` 和 `COMMIT` 之间。

- `COLLATION_CONNECTION = collation_name` 为后续查询中字符串值的 `ORDER BY` 或 `GROUP BY` 选择排序规则。有关已知排序规则名称的列表，请参见 [Collations](../Searching/Collations.md)。

- `WAIT_TIMEOUT/net_read_timeout = <value>` 设置连接超时，既可以是会话级的也可以是全局的。全局值只能在 VIP 连接上设置。

- `net_write_timeout = <value>`：调整写操作的网络超时，即发送数据。全局值只能在 VIP 权限下更改。

- `throttling_period = <INT_VALUE>`：当前正在运行的查询将重新调度的时间间隔（以毫秒为单位）。值为 0 禁用节流，意味着查询将在完成之前占用 CPU 核心。如果同时有来自其他连接的并发查询，它们将被分配到空闲核心，或者在核心释放之前挂起。提供负值（-1）将节流重置为默认编译值（100ms），这意味着查询将在每 100ms 被重新调度一次，让并发查询有机会执行。全局值（通过 `set global` 设置）只能在 VIP 连接上设置。

- `thread_stack = <value>`：实时更改分配给单个任务的栈大小的默认值。请注意，这里的“线程”不是指操作系统线程，而是指用户空间线程，也称为协程。如果，例如，你加载一个需要意外高要求的渗透表，这可能很有用。在这种情况下，'call pq' 将会失败，并提示栈大小不足。通常情况下，你应该停止守护进程，增加配置中的值，然后重新启动。但是，你也可以尝试在不重启的情况下设置新值。全局值也可以通过 `set global thread_stack` 在线更改，但这仅在 VIP 连接上可用。

- `optimize_by_id = {0 | 1}`：在某些 `debug` 命令中使用的内部标志。

- `threads_ex`（诊断）：强制 Manticore 以提供的配置运行，模拟具有指定配置的 CPU。例如，`set threads_ex='4/2+6/3'` 表示“你有 4 个空闲的 CPU 核心，当调度多个查询时，它们应该分成 2 个一组。此外，你有 6 个空闲的 CPU 核心用于伪分片，部分应分成 3 个一组”。此选项为诊断性质，非常有助于查看查询在你本地没有的配置下如何运行。例如，在 128 核 CPU 上。或者，反之亦然，可以快速限制守护进程行为为单线程，以定位瓶颈或调查崩溃。

<details>

  默认情况下，Manticore 启动一个线程池，其计算的 CPU 核心数量。所有典型任务会分配到这个线程池中，以确保最大化 CPU 利用率。例如，当实时表具有多个磁盘块时，搜索将在 CPU 核心之间并行化。同样，对于单个索引的单个全文搜索，守护进程将尝试使用称为“伪分片”的技术优化搜索执行。这两个特性都高度依赖于 CPU 核心的总数量和可立即使用的空闲核心数量。

  这种方法增强了性能，但可能使事件调查变得更加困难。例如，执行 `COUNT(*)` 的查询可能返回一个近似结果（例如，大于 100 个匹配），而随后执行同一查询可能会返回一个确切结果（例如，正好 120 个匹配）。这种变化取决于可用核心，但由于这一因素不可预测，通常会导致不可重现的结果。尽管这种情况通常可以接受，但有时也会成为问题。`threads_ex` 选项指定了所需的 CPU 核心配置，使查询在此配置下可重现。

  `threads_ex` 设置基本任务和伪分片的 CPU 模板，因为伪分片可以是标准并行化过程的一部分。例如，如果有多个磁盘块，它们将被并行查询，但每个块可能会进一步通过伪分片进行并行化。因此，要有效管理这种情况，你需要为每种任务类型准备几个模板。

  模板是一个字符串，类似于 `10/3`，其中 10 代表并发，3 代表批量大小。如果并发为 0，将使用默认并发。如果批量大小为 0，将使用默认的简单模板。任何零值都可以省略或用 `*` 替换。默认（简单）模板可以描述为 `''`，还可以表示为 `*/*`、`0/0`、`0/`、`*/`、`/0`、`*` 等。这意味着守护进程使用所有可用的 CPU 核心而没有特殊的批量限制。

  一个具有 20 个线程的简单模板可以表示为 `20/*`、`20/0`、`20/`，或者仅仅是 `20`。具有批量大小为 2 的轮询模板是 `*/2`、`0/2`，或简单地 `/2`。一个具有 20 个线程和批量大小为 3 的轮询调度器是 `20/3`。

  `threads_ex` 是一个用于基本任务和伪分片的模板，用 `+` 分隔，如：

  - `30+3` - 一个 30 个线程的简单基础 + 一个 3 个线程的简单伪分片
  - `+/2` - 一个简单基础 + 使用默认线程和批量大小为 2 的轮询伪分片
  - `10` - 一个 10 个线程的简单基础 + 默认简单伪分片
  - `/1+10` - 一个轮询基础，使用默认线程和批量大小为 1 + 一个 10 个线程的简单伪分片
  - `4/2+2/1` - 一个 4 个线程的轮询基础和批量大小为 2 + 一个 2 个线程和批量大小为 1 的轮询伪分片
  - `1+1` - 最确定的情况。恰好 1 个线程 + 1 个伪分片

该选项可以通过环境变量 `MANTICORE_THREADS_EX` 从外部全局设置，如：

```bash
export MANTICORE_THREADS_EX=8
export MANTICORE_THREADS_EX='16+8/2'
```

或者，通过 MySQL CLI，如：
```sql
SET threads_ex='16';
SET GLOBAL threads_ex='/2';
```

或者，作为查询参数，如：
```sql
SELECT ... OPTION threads_ex='1+1';
```

`threads_ex` 配置遵循一个层次结构：首先是环境变量，然后是全局变量，最后是查询选项，允许特定设置覆盖一般设置。

</details>

* `PROFILING = {0 | 1}`  在当前会话中启用查询分析，默认值为 0。另请参见 [show profile](../Node_info_and_management/Profiling/Query_profile.md)。
* `MAX_THREADS_PER_QUERY = <POSITIVE_INT_VALUE>` 在运行时重新定义 [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query)。每个会话变量仅影响在同一会话（连接）中运行的查询，即在断开连接之前。值为 0 表示“无限制”。如果同时设置了每个会话和全局变量，则每个会话的优先级更高。
* `ro = {1 | 0}` 切换会话到只读模式或恢复。输出中的变量以 `session_read_only` 名称显示。

已知的全局服务器变量包括：

- `QUERY_LOG_FORMAT = {plain | sphinxql}`：更改当前日志格式。

- `LOG_LEVEL = {info | debug | replication | debugv | debugvv}`：更改当前日志的详细程度。

- `QCACHE_MAX_BYTES = <value>`：更改 [query_cache](../Searching/Query_cache.md) 的 RAM 使用限制为给定值。

- `QCACHE_THRESH_MSEC = <value>`：更改 [query_cache](../Searching/Query_cache.md) 的最小墙时间阈值为给定值。

- `QCACHE_TTL_SEC = <value>`：更改 [query_cache](../Searching/Query_cache.md) 中缓存结果的 TTL 为给定值。

- `MAINTENANCE = {0 | 1}`：设置为 1 时，将服务器置于维护模式。只有具有 VIP 连接的客户端可以在此模式下执行查询，所有新的非 VIP 连接都会被拒绝，现有连接保持不变。

- `GROUPING_IN_UTC = {0 | 1}`：设置为 1 时，使得时间分组函数（day(), month(), year(), yearmonth(), yearmonthday()）以 UTC 计算。有关更多详细信息，请参见 [grouping_in_utc](../Server_settings/Searchd.md) 配置参数。

- `TIMEZONE = <value>`：指定日期/时间相关函数使用的时区。有关更多详细信息，请参见 [timezone](../Server_settings/Searchd.md) 配置参数。

- `QUERY_LOG_MIN_MSEC = <value>`：更改 [query_log_min_msec](../Server_settings/Searchd.md#query_log_min_msec) 的 searchd 设置值。此时，期望的值完全为毫秒，并且不解析时间后缀。

  > 警告：这是一个非常特定的“硬”变量；被过滤的消息将被丢弃，不会写入日志。最好用类似 `grep` 的工具过滤日志，这样你至少会保留完整的原始日志作为备份。

- `LOG_DEBUG_FILTER = <string value>`：过滤冗余日志消息。如果设置了该值，则所有级别 > INFO（即 `DEBUG`、`DEBUGV` 等）的日志将与该字符串进行比较，并仅在其以给定值开头时输出。

- `NET_WAIT = {-1 | 0 | POSITIVE_INT_VALUE}`：更改 [net_wait_tm](../Server_settings/Searchd.md#net_wait_tm) 的 searchd 设置值。

- `IOSTATS = {0 | 1}`：启用或禁用查询日志中的 I/O 操作（属性除外）报告。

- `CPUSTATS= {1|0}`：启用/禁用 [CPU 时间跟踪](../Starting_the_server/Manually.md#searchd-命令行选项)。

- `COREDUMP= {1|0}`：启用/禁用在崩溃时保存服务器的核心文件或小型转储。更多细节请参见 [这里](../Starting_the_server/Manually.md#searchd-命令行选项)。

- `AUTO_OPTIMIZE = {1|0}`：启用/禁用 [auto_optimize](../Server_settings/Searchd.md#auto_optimize)。

- `PSEUDO_SHARDING = {1|0}`：启用/禁用搜索 [pseudo-sharding](../Server_settings/Searchd.md#pseudo_sharding)。

- `SECONDARY_INDEXES = {1|0}`：启用/禁用搜索查询的 [secondary indexes](../Server_settings/Searchd.md#secondary_indexes)。

- `ES_COMPAT = {on/off/dashboards}`：设置为 `on`（默认值）时，支持类似 Elasticsearch 的写请求；设置为 `off` 则禁用支持；设置为 `dashboards` 则启用支持，并允许来自 Kibana 的请求（该功能仍在实验阶段）。

- `RESET_NETWORK_TIMEOUT_ON_PACKET = {1|0}`：更改 [reset_network_timeout_on_packet](../Server_settings/Searchd.md#reset_network_timeout_on_packet) 参数。只有具有 VIP 连接的客户端可以更改此变量。

- `optimize_cutoff = <value>`：动态更改配置的 [optimize_cutoff](../Server_settings/Searchd.md#optimize_cutoff) 设置的值。

- `accurate_aggregation`：为未来查询的 [accurate_aggregation](../Searching/Options.md#accurate_aggregation) 选项设置默认值。

- `distinct_precision_threshold`：为未来查询的 [distinct_precision_threshold](../Searching/Options.md#distinct_precision_threshold) 选项设置默认值。

- `expansion_merge_threshold_docs`：动态更改配置的 [expansion_merge_threshold_docs](../Server_settings/Searchd.md#expansion_merge_threshold_docs) 设置的值。

- `expansion_merge_threshold_hits`：动态更改配置的 [expansion_merge_threshold_hits](../Server_settings/Searchd.md#expansion_merge_threshold_hits) 设置的值。

示例：

```sql
mysql> SET autocommit=0;
Query OK, 0 rows affected (0.00 sec)

mysql> SET GLOBAL query_log_format=sphinxql;
Query OK, 0 rows affected (0.00 sec)

mysql> SET GLOBAL @banned=(1,2,3);
Query OK, 0 rows affected (0.01 sec)

mysql> SET INDEX users GLOBAL @banned=(1,2,3);
Query OK, 0 rows affected (0.01 sec)
```

要使用户变量持久化，请确保启用了 [sphinxql_state](../Server_settings/Searchd.md#sphinxql_state)。
<!-- proofread -->
