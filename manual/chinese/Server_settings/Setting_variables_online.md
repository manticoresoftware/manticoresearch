# 在线设置变量

## SET
```sql
SET [GLOBAL] server_variable_name = value
SET [INDEX table_name] GLOBAL @user_variable_name = (int_val1 [, int_val2, ...])
SET NAMES value [COLLATE value]
SET @@dummy_variable = ignored_value
```

Manticore Search 中的 `SET` 语句允许您修改变量的值。变量名不区分大小写，且变量值的更改在服务器重启后不会持续存在。

Manticore Search 支持 `SET NAMES` 语句和 `SET @@variable_name` 语法，以兼容可能在连接时要求执行这些语句的第三方 MySQL 客户端库、连接器和框架。但这些语句对 Manticore Search 本身没有任何影响。

Manticore Search 中有四类变量：

1.  会话级服务器变量：`set var_name = value`
2.  全局服务器变量：`set global var_name = value`
3.  全局用户变量：`set global @var_name = (value)`
4.  全局分布式变量：`set index dist_table_name global @var_name = (value)`

全局用户变量在并发会话之间共享。唯一支持的值类型是 BIGINT 列表，这些变量可以用于 IN() 操作符进行过滤。此功能的主要用例是在 `searchd` 中一次上传大量值列表，然后多次重用，从而减少网络开销。全局用户变量可以传送至分布式表的所有代理，或在定义于分布式表内的本地表中进行本地设置。示例：

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

Manticore Search 支持影响特定服务器设置的会话级和全局服务器变量。以下是已知的会话级和全局服务器变量列表：

已知会话级服务器变量：

* `AUTOCOMMIT = {0 | 1}` 决定数据修改语句是否应隐式地由 `BEGIN` 和 `COMMIT` 包裹。
* `COLLATION_CONNECTION = collation_name` 选择后续查询中对字符串值进行 `ORDER BY` 或 `GROUP BY` 时使用的排序规则。已知排序规则名称列表请参阅 [Collations](../Searching/Collations.md)。
* `MAX_THREADS_PER_QUERY = <POSITIVE_INT_VALUE>` 在运行时重新定义 [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query)。会话级变量只影响同一会话（连接）中运行的查询，即直到断开连接。值 0 表示“不限制”。如果同时设置了会话级和全局变量，会话级优先级更高。
* `NET_WRITE_TIMEOUT = <value>`
* `NET_READ_TIMEOUT = <value>`
* `INTERACTIVE_TIMEOUT = <value>`：为交互式 SQL 会话设置 SQL 连接超时（空闲时间）。一个常见例子是使用命令行客户端（CLI）如 `mysql` 连接到 Manticore 服务器。之前存在全局的“读取”和“写入”超时，但证明容易混淆。现在，`NET_WRITE_TIMEOUT` 和 `NET_READ_TIMEOUT` 是 `INTERACTIVE_TIMEOUT` 的别名。它们都控制同一个变量，仅与 SQL 端点相关：允许连接的最大空闲时间。这类似于 `WAIT_TIMEOUT`，但专门用于交互式会话。
* `WAIT_TIMEOUT = <value>`：设置非交互式 SQL 会话的 SQL 连接超时（空闲时间）。一个常见示例是客户端库连接到 Manticore 服务器并维护连接池以供重复使用。这类似于 `INTERACTIVE_TIMEOUT`，但专用于非交互式会话。
* `OPTIMIZE_BY_ID = {0 | 1}`：用于某些 `debug` 命令的内部标志。
* `PROFILING = {0 | 1}` 在当前会话启用查询分析。默认为 0。另见 [show profile](../Node_info_and_management/Profiling/Query_profile.md)。
* `ro = {1 | 0}` 将会话切换为只读模式或恢复为非只读模式。在 `show variables` 的输出中，该变量显示为 `session_read_only`。
* `throttling_period = <INT_VALUE>`：当前运行查询的重新调度间隔（毫秒）。值为 0 表示禁用节流，查询将占用 CPU 核心直到完成。如果同时有其他连接的并发查询，它们将被分配到空闲核心，或挂起直到核心释放。提供负值（-1）时将节流重置为默认编译值（100ms），意味着查询每 100ms 重新调度一次，允许并发查询被执行。全局值（通过 `set global` 设置）仅从 VIP 连接设置时有效。
* `thread_stack = <value>`：动态修改栈大小限制，即单个任务的线程栈大小。这里的“线程”指的是用户空间线程，也称为协程，而非操作系统线程。这在加载需求意外高的 percolate 表时特别有用，此时“call pq”可能因栈空间不足失败。通常应停止守护进程，在配置中增加该值然后重启。但也可以通过设置该变量在线尝试新值，无需重启。全局值也可在线通过 `set global thread_stack` 修改，但仅限 VIP 连接。
* `threads_ex`（诊断用）：强制 Manticore 以所提供的 CPU 配置运行。简而言之，`set threads_ex='4/2+6/3'` 表示“你有 4 个空闲 CPU 核心，调度多查询时每 2 个批处理；另有 6 个 CPU 核心用于伪分片，分片大小为 3”。此选项用于诊断，例如，查看查询如何在本地没有的配置（如 128 核 CPU）上执行；或快速限制守护进程为单线程模式，以定位瓶颈或调查崩溃。

  <details>

	Manticore 默认启动一个线程池，其大小基于计算的 CPU 核心数。所有典型任务均分配至此线程池以确保最大 CPU 利用率。例如，当实时表有多个磁盘分片时，搜索将并行运行于多个 CPU 核心。同样，对单个索引的全文搜索，守护进程会尝试使用称为“伪分片”的技术进行并行优化。这两种功能高度依赖 CPU 总核心数和可用空闲核心数。

	这种方式提升性能，但可能增加故障调查难度。例如，一个执行 `COUNT(*)` 的查询可能返回近似结果（如大于 100 个匹配项），随后的同一查询可能返回精确结果（如正好 120 个匹配项）。这种差异取决于可用核心数，由于该因素不可预知，通常导致结果不可复现。尽管通常接受，但有时可能成为问题。`threads_ex` 选项指定期望的 CPU 核心配置，使得拥有此配置的查询结果可以复现。

	`threads_ex` 设置标准任务和伪分片的 CPU 模板，因为伪分片可以作为标准并行化过程的一部分。例如，如果有多个磁盘块，将并行查询它们，但每个块可能会使用伪分片进一步并行化。因此，为了有效管理这种情况，你需要为每种任务类型准备几个模板。

	模板是一个类似 `10/3` 的字符串，其中 10 表示并发数，3 表示批处理大小。如果并发数为 0，则使用默认并发数。如果批处理大小为 0，则使用默认的简单模板。任何零值都可以省略或替换为 `*`。默认（简单）模板可以描述为 `''`，也可以是 `*/*`、`0/0`、`0/`、`*/`、`/0`、`*` 等。这意味着守护进程使用所有可用的 CPU 核心，没有特殊的批处理限制。

	一个带有 20 个线程的简单模板可以表示为 `20/*`、`20/0`、`20/` 或干脆就是 `20`。一个批处理大小为 2 的轮询模板是 `*/2`、`0/2` 或干脆是 `/2`。一个线程数为 20，批处理大小为 3 的轮询调度器是 `20/3`。

	`threads_ex` 是基本任务和伪分片的模板，由 `+` 分隔，如：

	* `30+3` - 一个简单的基础 30 线程 + 简单的伪分片 3 线程
	* `+/2` - 一个简单的基础 + 轮询伪分片，默认线程数和批处理=2
	* `10` - 一个简单的基础 10 线程 + 默认简单伪分片
	* `/1+10` - 轮询基础，默认线程数和批处理=1 + 简单伪分片 10 线程
	* `4/2+2/1` - 轮询基础，4 线程，批处理=2 + 轮询伪分片，2 线程，批处理=1
	* `1+1` - 最确定的情况。精确 1 个线程 + 1 个伪分片，即完全没有并行化。通过此设置，你可以可重复地执行相同的有问题查询并调查行为细节。此外，如果设置 `throttling_period=0`，查询将在当前线程“粘滞”，执行期间永不重新调度，创建理想的故障排除环境。

	该选项可以从外部全局设置为环境变量 `MANTICORE_THREADS_EX`，如：

	```bash
	export MANTICORE_THREADS_EX=8
 	# or
	export MANTICORE_THREADS_EX='16+8/2'
  ```

	或者，通过 MySQL CLI，设置为：
  ```sql
  SET threads_ex='16';
  -- or 
  SET GLOBAL threads_ex='/2';
  ```

	或者，作为查询参数，如：
  ```sql
  SELECT ... OPTION threads_ex='1+1';
  ```

	`threads_ex` 配置遵循层级结构：首先是环境变量，然后是全局变量，最后是查询选项，允许特定设置覆盖通用设置。

	</details>

已知的全局服务器变量有：

* `WAIT_TIMEOUT = <value>` 设置 SQL 连接超时时间为非交互式连接允许的最大空闲时间。您可以通过会话（当前连接）或全局配置此设置，但全局设置仅适用于 VIP 连接。
* `INTERACTIVE_TIMEOUT = <value>` 设置 SQL 连接超时时间为交互式连接允许的最大空闲时间。与 `WAIT_TIMEOUT` 一样，此设置可以按会话或全局配置，但全局设置仅适用于 VIP 连接。
* `ACCURATE_AGGREGATION`：设置未来查询的选项 [accurate_aggregation](../Searching/Options.md#accurate_aggregation) 的默认值。
* `AUTO_OPTIMIZE = {1|0}` 打开/关闭 [auto_optimize](../Server_settings/Searchd.md#auto_optimize)。
* `cluster_user = name` 设置使用 `mysqldump` / `mariadb-dump` 的用户名以[启用复制模式](../Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)。
* `COREDUMP= {1|0}` 打开/关闭崩溃时保存服务器核心文件或迷你转储。更多详情见[此处](../Starting_the_server/Manually.md#searchd-command-line-options)。
* `CPUSTATS= {1|0}` 打开/关闭[CPU 时间跟踪](../Starting_the_server/Manually.md#searchd-command-line-options)。
* `DISTINCT_PRECISION_THRESHOLD`：设置未来查询的选项 [distinct_precision_threshold](../Searching/Options.md#distinct_precision_threshold) 的默认值。
* `ES_COMPAT = {on/off/dashboards}` 设置为 `on`（默认）时，支持类似 Elasticsearch 的写请求；`off` 禁用该支持；`dashboards` 启用支持且允许来自 Kibana 的请求（此功能为实验性）。
* `EXPANSION_MERGE_THRESHOLD_DOCS`：动态更改配置项 [expansion_merge_threshold_docs](../Server_settings/Searchd.md#expansion_merge_threshold_docs) 的值。
* `EXPANSION_MERGE_THRESHOLD_HITS`：动态更改配置项 [expansion_merge_threshold_hits](../Server_settings/Searchd.md#expansion_merge_threshold_hits) 的值。
* `GROUPING_IN_UTC = {0 | 1}` 设置为 1 时，时间分组函数（day(), month(), year(), yearmonth(), yearmonthday()）将以 UTC 计算。更多细节请阅读 [grouping_in_utc](../Server_settings/Searchd.md) 配置参数文档。
* `IOSTATS = {0 | 1}` 启用或禁用在查询日志中报告的 I/O 操作（属性除外）。
* `LOG_DEBUG_FILTER = <string value>` 过滤冗余日志信息。若设置此值，所有级别高于 INFO 的日志（如 `DEBUG`, `DEBUGV` 等）将与该字符串比较，仅当日志内容以该字符串开头时才输出。
* `LOG_LEVEL = {info | debug | replication | debugv | debugvv}` 更改当前日志详细级别。
* `MAINTENANCE = {0 | 1}` 设为 1 时，将服务器置于维护模式。仅具有 VIP 连接的客户端可以执行查询。所有新的非 VIP 连接将被拒绝。现有连接则保持不变。
* `MAX_THREADS_PER_QUERY = <POSITIVE_INT_VALUE>` 运行时重新定义 [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query)。作为全局变量，改变所有会话的行为。值为 0 表示“不限制”。如果同时设置了会话级和全局变量，会话级优先。
* `NET_WAIT = {-1 | 0 | POSITIVE_INT_VALUE}` 更改 [net_wait_tm](../Server_settings/Searchd.md#net_wait_tm) 的 searchd 设置值。
* `OPTIMIZE_CUTOFF = <value>`：动态更改配置项 [optimize_cutoff](../Server_settings/Searchd.md#optimize_cutoff) 的值。
* `PSEUDO_SHARDING = {1|0}` 打开/关闭搜索[伪分片](../Server_settings/Searchd.md#pseudo_sharding)。
* `QCACHE_MAX_BYTES = <value>` 更改 [query_cache](../Searching/Query_cache.md) 的 RAM 使用限制到指定值。
* `QCACHE_THRESH_MSEC = <value>` 更改 [query_cache](../Searching/Query_cache.md) 的最小响应时间阈值到指定值。
* `QCACHE_TTL_SEC = <value>` 更改 [query_cache](../Searching/Query_cache.md) 缓存结果的 TTL 到指定值。
* `QUERY_LOG_FORMAT = {plain | sphinxql}` 更改当前日志格式。
* `QUERY_LOG_MIN_MSEC = <value>` 更改 [query_log_min_msec](../Server_settings/Searchd.md#query_log_min_msec) 的 searchd 设置值。此处值需严格为毫秒数，不支持配置文件中的时间后缀解析。
  > 警告：这是一个非常特殊且“严格”的变量；被过滤的消息将直接丢弃，不会写入日志。建议仅使用如 'grep' 之类的工具过滤日志，这样至少可以保留完整的原始日志备份。
* `RESET_NETWORK_TIMEOUT_ON_PACKET = {1|0}` 更改 [reset_network_timeout_on_packet](../Server_settings/Searchd.md#reset_network_timeout_on_packet) 参数。仅 VIP 连接客户端可以更改此变量。
* `SECONDARY_INDEXES = {1|0}` 打开/关闭查询的[二级索引](../Server_settings/Searchd.md#secondary_indexes)。
* `TIMEZONE = <value>` 指定日期/时间相关函数使用的时区。更多细节请阅读 [timezone](../Server_settings/Searchd.md) 配置参数文档。

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

要使用户变量保持持久性，请确保启用了 [sphinxql_state](../Server_settings/Searchd.md#sphinxql_state)。
<!-- proofread -->

