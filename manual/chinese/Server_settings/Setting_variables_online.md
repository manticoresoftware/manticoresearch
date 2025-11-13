# 在线设置变量

## SET
```sql
SET [GLOBAL] server_variable_name = value
SET [INDEX table_name] GLOBAL @user_variable_name = (int_val1 [, int_val2, ...])
SET NAMES value [COLLATE value]
SET @@dummy_variable = ignored_value
```

Manticore Search 中的 `SET` 语句允许您修改变量值。变量名不区分大小写，服务器重启后变量值的更改不会保留。

Manticore Search 支持 `SET NAMES` 语句和 `SET @@variable_name` 语法，以兼容可能在连接时需要运行这些语句的第三方 MySQL 客户端库、连接器和框架。然而，这些语句对 Manticore Search 本身没有任何影响。

Manticore Search 中有四类变量：

1.  会话级服务器变量：`set var_name = value`
2.  全局服务器变量：`set global var_name = value`
3.  全局用户变量：`set global @var_name = (value)`
4.  全局分布式变量：`set index dist_table_name global @var_name = (value)`

全局用户变量在并发会话之间共享。唯一支持的值类型是 BIGINT 列表，这些变量可以与 IN() 操作符一起用于过滤。此功能的主要用例是一次性将大量值列表上传到 `searchd`，然后多次重用，减少网络开销。全局用户变量可以传递给分布式表的所有代理，或者在分布式表中定义的本地表情况下本地设置。示例：

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

Manticore Search 支持影响各自作用域中特定服务器设置的会话级和全局服务器变量。以下是已知的会话级和全局服务器变量列表：

已知的会话级服务器变量：

* `AUTOCOMMIT = {0 | 1}` 决定数据修改语句是否应隐式地由 `BEGIN` 和 `COMMIT` 包裹。
* `COLLATION_CONNECTION = collation_name` 选择后续查询中字符串值的 `ORDER BY` 或 `GROUP BY` 的排序规则。已知排序规则名称列表请参见 [排序规则](../Searching/Collations.md)。
* `MAX_THREADS_PER_QUERY = <POSITIVE_INT_VALUE>` 在运行时重新定义 [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query)。会话级变量仅影响同一会话（连接）中运行的查询，即直到断开连接。值为 0 表示“无限制”。如果同时设置了会话级和全局变量，会话级变量优先。
* `net_write_timeout = <value>`：调整写操作（即发送数据）的网络超时。全局值只能由 VIP 权限更改。
* `optimize_by_id = {0 | 1}`：某些 `debug` 命令中使用的内部标志。
* `PROFILING = {0 | 1}` 启用当前会话的查询分析。默认值为 0。另见 [show profile](../Node_info_and_management/Profiling/Query_profile.md)。
* `ro = {1 | 0}` 切换会话为只读模式或恢复。在 `show variables` 输出中，该变量显示为 `session_read_only`。
* `throttling_period = <INT_VALUE>`：当前运行查询将重新调度的时间间隔（毫秒）。值为 0 禁用节流，意味着查询将占用 CPU 核心直到完成。如果同时有来自其他连接的并发查询，它们将被分配到空闲核心或挂起直到核心释放。提供负值（-1）将节流重置为默认编译值（100ms），意味着查询每 100ms 重新调度一次，允许并发查询有执行机会。全局值（通过 `set global` 设置）只能在 VIP 连接上设置。
* `thread_stack = <value>`：动态更改默认值，限制分配给单个任务的栈大小。注意这里的“线程”指的是用户空间线程，也称为协程，而非操作系统线程。如果例如加载 percolate 表时需求异常高，这可能有用。在这种情况下，“call pq” 会因栈大小不足而失败。通常应停止守护进程，在配置中增加该值，然后重启。但也可以尝试不重启，直接用此变量设置新值。全局值也可以通过 `set global thread_stack` 在线更改，但仅限 VIP 连接。
* `threads_ex`（诊断用）：强制 Manticore 表现得像运行在指定 CPU 配置上一样。简短示例，`set threads_ex='4/2+6/3'` 表示“你有 4 个空闲 CPU 核心，调度多个查询时应按 2 批处理。同时，你有 6 个空闲 CPU 核心用于伪分片，部分应按 3 批处理”。此选项用于诊断，非常有用，例如查看查询在本地没有的配置（如 128 核 CPU）上的运行情况。或者快速限制守护进程表现为单线程，以定位瓶颈或调查崩溃。

  <details>

	默认情况下，Manticore 启动一个线程池，线程数基于计算的 CPU 核心数。所有典型任务随后分配到该线程池，以确保最大 CPU 利用率。例如，当实时表有多个磁盘块时，搜索将在 CPU 核心上并行化。同样，对于单个索引的全文搜索，守护进程将尝试使用称为“伪分片”的技术优化并行搜索执行。这两个功能都高度依赖于 CPU 核心总数和可立即使用的空闲核心数。

	这种方法提升了性能，但可能使事件调查更具挑战性。例如，执行 `COUNT(*)` 的查询可能返回近似结果（如大于 100 个匹配），而同一查询的后续执行可能返回精确结果（如正好 120 个匹配）。这种差异取决于可用核心数，但由于该因素不可预测，通常导致结果不可复现。虽然这通常可以接受，但有时可能成为问题。`threads_ex` 选项指定期望的 CPU 核心配置，使得使用该配置的查询结果可复现。

	`threads_ex` 设置标准任务和伪分片的 CPU 模板，因为伪分片可以是标准并行化过程的一部分。例如，如果有多个磁盘块，它们将并行查询，但每个块可能进一步使用伪分片进行并行化。因此，为了有效管理这种情况，您需要为每种任务类型准备几个模板。

	模板是类似 `10/3` 的字符串，其中 10 表示并发数，3 表示批处理大小。如果并发数为 0，则使用默认并发数。如果批处理大小为 0，则使用默认的简单模板。任何为零的值都可以省略或替换为 `*`。默认（简单）模板可以描述为 `''`，也可以是 `*/*`、`0/0`、`0/`、`*/`、`/0`、`*` 等。这意味着守护进程使用所有可用的 CPU 核心，没有特殊的批处理限制。

	带有 20 线程的简单模板可以表示为 `20/*`、`20/0`、`20/`，或简单地写为 `20`。批处理大小为 2 的轮询模板是 `*/2`、`0/2`，或简单地写为 `/2`。具有 20 线程和批处理大小为 3 的轮询调度器是 `20/3`。

	`threads_ex` 是基本任务和伪分片的模板，由 `+` 分隔，如：

	* `30+3` - 30 线程的简单基础 + 3 线程的简单伪分片
	* `+/2` - 简单基础 + 轮询伪分片，使用默认线程数和批处理大小为 2
	* `10` - 10 线程的简单基础 + 默认简单伪分片
	* `/1+10` - 轮询基础，使用默认线程数和批处理大小为 1 + 10 线程的简单伪分片
	* `4/2+2/1` - 4 线程、批处理大小为 2 的轮询基础 + 2 线程、批处理大小为 1 的轮询伪分片
	* `1+1` - 最确定性的情况。恰好 1 个线程 + 1 个伪分片，即完全没有并行化。使用此设置，您可以可重复地执行相同的有问题查询并调查行为细节。此外，如果设置 `throttling_period=0`，您的查询将在当前线程“粘住”，执行期间永远不会重新调度，创造理想的故障排查环境。

	该选项可以从外部全局设置为环境变量 `MANTICORE_THREADS_EX`，例如：

	```bash
	export MANTICORE_THREADS_EX=8
	export MANTICORE_THREADS_EX='16+8/2'
  ```

  或通过 MySQL CLI 设置，如：
  ```sql
  SET threads_ex='16';
	SET GLOBAL threads_ex='/2';
  ```

  或作为查询参数，如：
  ```sql
  SELECT ... OPTION threads_ex='1+1';
  ```

  `threads_ex` 配置遵循层级结构：首先是环境变量，然后是全局变量，最后是查询选项，允许特定设置覆盖通用设置。

  </details>
* `WAIT_TIMEOUT/net_read_timeout = <value>` 设置连接超时，可以是会话级别或全局级别。全局只能在 VIP 连接上设置。

已知的全局服务器变量有：

* `ACCURATE_AGGREGATION`：设置未来查询选项 [accurate_aggregation](../Searching/Options.md#accurate_aggregation) 的默认值。
* `AUTO_OPTIMIZE = {1|0}`：开启或关闭 [auto_optimize](../Server_settings/Searchd.md#auto_optimize)。
* `cluster_user = name`：设置用于 `mysqldump` / `mariadb-dump` 的用户名，以[启用复制模式](../Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)。
* `COREDUMP= {1|0}`：开启或关闭服务器崩溃时保存核心文件或迷你转储。更多详情见[这里](../Starting_the_server/Manually.md#searchd-command-line-options)。
* `CPUSTATS= {1|0}`：开启或关闭[CPU 时间跟踪](../Starting_the_server/Manually.md#searchd-command-line-options)。
* `DISTINCT_PRECISION_THRESHOLD`：设置未来查询选项 [distinct_precision_threshold](../Searching/Options.md#distinct_precision_threshold) 的默认值。
* `ES_COMPAT = {on/off/dashboards}`：设置为 `on`（默认）时，支持类似 Elasticsearch 的写请求；`off` 禁用支持；`dashboards` 启用支持并允许来自 Kibana 的请求（此功能为实验性）。
* `EXPANSION_MERGE_THRESHOLD_DOCS`：动态更改配置中的 [expansion_merge_threshold_docs](../Server_settings/Searchd.md#expansion_merge_threshold_docs) 设置值。
* `EXPANSION_MERGE_THRESHOLD_HITS`：动态更改配置中的 [expansion_merge_threshold_hits](../Server_settings/Searchd.md#expansion_merge_threshold_hits) 设置值。
* `GROUPING_IN_UTC = {0 | 1}`：设置为 1 时，使定时分组函数（day(), month(), year(), yearmonth(), yearmonthday()）以 UTC 计算。更多详情请参阅 [grouping_in_utc](../Server_settings/Searchd.md) 配置参数文档。
* `IOSTATS = {0 | 1}`：启用或禁用查询日志中的 I/O 操作报告（属性除外）。
* `LOG_DEBUG_FILTER = <string value>`：过滤冗余日志消息。如果设置了该值，则所有级别高于 INFO（即 `DEBUG`、`DEBUGV` 等）的日志将与该字符串比较，仅当日志以该字符串开头时才输出。
* `LOG_LEVEL = {info | debug | replication | debugv | debugvv}`：更改当前日志详细级别。
* `MAINTENANCE = {0 | 1}`：设置为 1 时，将服务器置于维护模式。只有 VIP 连接的客户端可以在此模式下执行查询。所有新的非 VIP 连接将被拒绝。现有连接保持不变。
* `MAX_THREADS_PER_QUERY = <POSITIVE_INT_VALUE>`：运行时重新定义 [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query)。作为全局变量时，改变所有会话的行为。值为 0 表示“无限制”。如果同时设置了会话级和全局变量，会话级优先。
* `NET_WAIT = {-1 | 0 | POSITIVE_INT_VALUE}`：更改 [net_wait_tm](../Server_settings/Searchd.md#net_wait_tm) searchd 设置值。
* `OPTIMIZE_CUTOFF = <value>`：动态更改配置中的 [optimize_cutoff](../Server_settings/Searchd.md#optimize_cutoff) 设置值。
* `PSEUDO_SHARDING = {1|0}`：开启或关闭搜索[伪分片](../Server_settings/Searchd.md#pseudo_sharding)。
* `QCACHE_MAX_BYTES = <value>` 更改 [query_cache](../Searching/Query_cache.md) 的 RAM 使用限制为指定值。
* `QCACHE_THRESH_MSEC = <value>` 更改 [query_cache](../Searching/Query_cache.md) 的最小墙时阈值为指定值。
* `QCACHE_TTL_SEC = <value>` 更改 [query_cache](../Searching/Query_cache.md) 缓存结果的 TTL 为指定值。
* `QUERY_LOG_FORMAT = {plain | sphinxql}` 更改当前日志格式。
* `QUERY_LOG_MIN_MSEC = <value>` 更改 [query_log_min_msec](../Server_settings/Searchd.md#query_log_min_msec) searchd 设置值。在此情况下，期望值为毫秒数，不解析配置中的时间后缀。
  > 警告：这是一个非常特殊且“硬性”的变量；被过滤的消息将直接丢弃，不会写入日志。最好使用类似 'grep' 的工具过滤日志，这样至少可以保留完整的原始日志作为备份。
* `RESET_NETWORK_TIMEOUT_ON_PACKET = {1|0}` 更改 [reset_network_timeout_on_packet](../Server_settings/Searchd.md#reset_network_timeout_on_packet) 参数。只有具有 VIP 连接的客户端可以更改此变量。
* `SECONDARY_INDEXES = {1|0}` 开启/关闭搜索查询的 [secondary indexes](../Server_settings/Searchd.md#secondary_indexes)。
* `TIMEZONE = <value>` 指定日期/时间相关函数使用的时区。更多详情请参阅 [timezone](../Server_settings/Searchd.md) 配置参数文档。

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

要使用户变量持久化，请确保启用 [sphinxql_state](../Server_settings/Searchd.md#sphinxql_state)。
<!-- proofread -->

