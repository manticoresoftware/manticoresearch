# 在线设置变量

## 设置
```sql
SET [GLOBAL] server_variable_name = value
SET [INDEX table_name] GLOBAL @user_variable_name = (int_val1 [, int_val2, ...])
SET NAMES value [COLLATE value]
SET @@dummy_variable = ignored_value
```

Manticore 搜索中的 `SET` 语句允许你修改变量值。变量名不区分大小写，并且在服务器重启后不会保留任何变量值的更改。

Manticore 搜索支持 `SET NAMES` 语句和 `SET @@variable_name` 语法，以兼容第三方 MySQL 客户端库、连接器和框架，后者在连接时可能需要执行这些语句。然而，这些语句对 Manticore 搜索本身没有任何影响。

在 Manticore 搜索中，有四类变量：

1.  会话级服务器变量：`set var_name = value`
2.  全局服务器变量：`set global var_name = value`
3.  全局用户变量：`set global @var_name = (value)`
4.  全局分布式变量：`set index dist_table_name global @var_name = (value)`

全局用户变量在并发会话之间共享。唯一支持的值类型是 BIGINT 列表，这些变量可以与 IN() 运算符结合使用进行筛选。此功能的主要用途是在上传大量值列表到 `searchd` 后，存储一次并在之后多次重用，以减少网络开销。全局用户变量可以传递给分布式表的所有代理，也可以在本地定义的本地表中本地设置。例如：

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

Manticore 搜索支持影响特定服务器设置的会话级和全局服务器变量，分别在它们的作用域内生效。以下是已知的会话级和全局服务器变量列表：

已知会话级服务器变量：

* `AUTOCOMMIT = {0 | 1}`：决定数据修改语句是否隐式由 `BEGIN` 和 `COMMIT` 包裹。
* `COLLATION_CONNECTION = 字符集名`：为后续查询中的字符串值选择排序规则，用于 `ORDER BY` 或 `GROUP BY`。请参阅 [排序规则](../Searching/Collations.md) 获取已知排序规则名称列表。
* `MAX_THREADS_PER_QUERY = <正整数值>`：在运行时重新定义 [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query)。会话变量只影响在同一会话（连接）中运行的查询，即直到断开连接。值为 0 表示“无上限”。如果同时设置了会话和全局变量，则以会话变量为优先。
* `net_write_timeout = <值>`：调整网络写操作的超时时间（即数据发送）。全局值只能通过 VIP 权限进行更改。
* `optimize_by_id = {0 | 1}`：在某些 `debug` 命令中使用的内部标志。
* `PROFILING = {0 | 1}`：启用当前会话的查询分析。默认为 0。另请参阅 [查询分析](../Node_info_and_management/Profiling/Query_profile.md)。
* `ro = {1 | 0}`：切换会话为只读模式或恢复普通模式。在 `show variables` 输出中，变量显示名为 `session_read_only`。
* `throttling_period = <整数值>`：（毫秒）间隔，当前运行的查询在此期间将重新调度。值为 0 时禁用节流，查询将占用 CPU 核心直至完成。如果同时有来自其他连接的并发查询，它们会被分配到空闲核心或暂停，直到释放核心。提供负值（-1）会将节流重置为默认值（100ms），意味着查询每 100ms 会被重新调度一次，允许并发查询有执行的机会。通过 `set global` 设置的全局值只能在 VIP 连接上设置。
* `thread_stack = <值>`：动态更改默认堆栈大小（限制为一个任务所提供的堆栈）。注意这里的 'thread' 并非操作系统线程，而是用户空间线程，也称为协程。例如，如果加载了具有意外高需求的 percolate 表，使用 `call pq` 时会因堆栈不足而失败。通常，应停止守护进程，增加配置中的值，然后重启。但也可以在不重启的情况下，设置新值以便立即生效。全局值也可以通过 `set global thread_stack` 在线更改，但此操作仅支持 VIP 连接。
* `threads_ex`（诊断）：强制 Manticore 行为如同在带有提供的配置文件的 CPU 上运行。例如，`set threads_ex='4/2+6/3'` 表示“你有 4 个空闲 CPU 核心，调度多个查询时每两个一批；此外，你有 6个空闲 CPU 核心供伪分片，部分应每三批”。此选项为诊断用途，非常有用，例如，查看你的查询在没有本地配置的情况下在特定配置（如128核 CPU）上会如何运行，或快速限制守护进程行为为单线程，以定位瓶颈或调查崩溃。

  <details>

默认情况下，Manticore 会启动一个线程池，其核心数量由 CPU 核数决定。所有常规任务都会被分配到此线程池中，以确保最大化 CPU 利用率。例如，实时表有多个磁盘块时，搜索将会在 CPU 核心上进行并行化。类似地，对于单一索引的全文搜索，守护进程会尝试以“伪分片”的方式并行优化搜索执行。这两项功能在很大程度上依赖于 CPU 核心总数和可立即使用的空闲核心数。

这种方式可以提升性能，但同时也会使问题排查变得更复杂。例如，执行 `COUNT(*)` 的查询可能返回一个近似结果（如超过 100 条匹配），而相同查询的下一次执行可能会得到精确结果（比如正好 120 条匹配）。这种变动性取决于可用核心，但由于这一因素不可预测，通常会导致结果不可重现。虽然通常可以接受，但有时也会带来问题。`threads_ex` 选项允许你配置可重现的 CPU 核心数，从而使相关查询的结果变得稳定。

`threads_ex` 设置用于标准任务和伪分片的 CPU 模板，因为伪分片可以是标准并行化过程的一部分。例如，如果有多个磁盘块，它们将同时查询，但每个也可以通过伪分片进行进一步的并行化。因此，为了有效管理这种情况，你需要为每个任务类型准备几个模板。

模板是类似于 `10/3` 的字符串，其中 10 表示并发数，3 表示批处理大小。如果并发数为 0，则将使用默认并发数。如果批处理大小为 0，则使用默认的基础模板。任何零值都可以省略或用 `*` 代替。默认（基础）模板可以描述为 `''`，也可以是 `*/*`、`0/0`、`0/`、`*/`、`/0`、`*` 等。这意味着守护进程使用所有可用的 CPU 核心，而没有特殊的批处理限制。

一个有 20 个线程的基础模板可以用 `20/*`、`20/0`、`20/` 或简单的 `20` 表示。一个带有批处理大小为 2 的轮询模板是 `*/2`、`0/2` 或简单的 `/2`。一个带有 20 线程和批处理大小为 3 的轮询调度器是 `20/3`。

`threads_ex` 是基本任务和伪分片的模板，使用 `+` 分隔，例如：

* `30+3` - 30 线程的基础模板 + 3 线程的伪分片
* `+/2` - 基础模板 + 批处理大小为 2 的轮询伪分片（使用默认线程数）
* `10` - 10 线程的基础模板 + 默认的基础伪分片
* `/1+10` - 批量为 1、线程数为默认的轮询基础模板 + 具有 10 线程的基础伪分片
* `4/2+2/1` - 具有 4 线程、批处理为 2 的轮询基础模板 + 具有 2 线程、批处理为 1 的轮询伪分片
* `1+1` - 最可预测的情况。恰好 1 个线程 + 1 个伪分片，也就是说完全没有并行化。使用此设置，你可以可重复地测试同一个有问题的查询并深入分析行为细节。此外，如果你设置 `throttling_period=0`，你的查询将“粘”在当前线程上，在执行期间永远不会重新调度，为故障排除创造了理想环境。

此选项可以在外部作为环境变量 `MANTICORE_THREADS_EX` 全局设置，例如：

```bash
export MANTICORE_THREADS_EX=8
export MANTICORE_THREADS_EX='16+8/2'
```

或者，通过 MySQL CLI 设置为：
```sql
SET threads_ex='16';
SET GLOBAL threads_ex='/2';
```

或者，作为查询参数，例如：
```sql
SELECT ... OPTION threads_ex='1+1';
```

`threads_ex` 配置遵循层级：首先是环境变量，其次是全局变量，最后是查询选项，允许特定设置覆盖通用设置。

</details>
* `WAIT_TIMEOUT/net_read_timeout = <value>` 设置连接超时，可以是每个会话或全局的。全局只能在 VIP 连接上设置。

已知的全局服务器变量有：

* `ACCURATE_AGGREGATION`：设置未来查询的 [accurate_aggregation](../Searching/Options.md#accurate_aggregation) 选项的默认值。
* `AUTO_OPTIMIZE = {1|0}` 开启/关闭 [auto_optimize](../Server_settings/Searchd.md#auto_optimize)。
* `cluster_user = name` 设置用于 `mysqldump` / `mariadb-dump` 的用户名，以 [启用复制模式](../Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)。
* `COREDUMP= {1|0}` 开启/关闭在服务器崩溃时保存核心文件或迷你转储。更多细节[这里](../Starting_the_server/Manually.md#searchd-command-line-options)。
* `CPUSTATS= {1|0}` 开启/关闭 [CPU 时间跟踪](../Starting_the_server/Manually.md#searchd-command-line-options)。
* `DISTINCT_PRECISION_THRESHOLD`：设置未来查询的 [distinct_precision_threshold](../Searching/Options.md#distinct_precision_threshold) 选项的默认值。
* `ES_COMPAT = {on/off/dashboards}` 设置为 `on`（默认）时，支持 Elasticsearch 类写入请求；设置为 `off` 时禁用支持；设置为 `dashboards` 时启用支持且允许来自 Kibana 的请求（此功能为实验性）。
* `EXPANSION_MERGE_THRESHOLD_DOCS`：动态修改配置中的 [expansion_merge_threshold_docs](../Server_settings/Searchd.md#expansion_merge_threshold_docs) 设置值。
* `EXPANSION_MERGE_THRESHOLD_HITS`：动态修改配置中的 [expansion_merge_threshold_hits](../Server_settings/Searchd.md#expansion_merge_threshold_hits) 设置值。
* `GROUPING_IN_UTC = {0 | 1}` 当设置为 1 时，时间分组函数（day()、month()、year()、yearmonth()、yearmonthday()）将以 UTC 计算。详见 [grouping_in_utc](../Server_settings/Searchd.md) 配置参数的文档。
* `IOSTATS = {0 | 1}` 启用或禁用查询日志中的 I/O 操作（除属性外）的报告。
* `LOG_DEBUG_FILTER = <字符串值>` 过滤掉冗余的日志消息。如果设置了该值，则所有级别 > INFO（即 `DEBUG`、`DEBUGV` 等）的日志将与该字符串进行比较，只有当它们以给定值开头时才输出。
* `LOG_LEVEL = {info | debug | replication | debugv | debugvv}` 改变当前日志的详细级别。
* `MAINTENANCE = {0 | 1}` 设置为 1 时，使服务器进入维护模式。只有 VIP 连接的客户端可以在此模式下执行查询。所有新的非 VIP 连接都将被拒绝。现有连接保持不变。
* `MAX_THREADS_PER_QUERY = <正整数值>` 在运行时重新定义 [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query)。作为全局变量，它会影响所有会话。值 0 表示“无限制”。如果同时设置会话变量和全局变量，会话变量具有更高优先级。
* `NET_WAIT = {-1 | 0 | 正整数值}` 改变[net_wait_tm](../Server_settings/Searchd.md#net_wait_tm) searchd 的设置值。
* `OPTIMIZE_CUTOFF = <值>`：动态修改配置中的 [optimize_cutoff](../Server_settings/Searchd.md#optimize_cutoff) 设置值。
* `PSEUDO_SHARDING = {1|0}` 开启/关闭搜索 [伪分片](../Server_settings/Searchd.md#pseudo_sharding)。
* `QCACHE_MAX_BYTES = <value>` 将 [query_cache](../Searching/Query_cache.md) 的内存使用限制更改为指定值。
* `QCACHE_THRESH_MSEC = <value>` 将 [query_cache](../Searching/Query_cache.md) 的最小响应时间阈值更改为指定值。
* `QCACHE_TTL_SEC = <value>` 将 [query_cache](../Searching/Query_cache.md) 缓存结果的 TTL 更改为指定值。
* `QUERY_LOG_FORMAT = {plain | sphinxql}` 更改当前日志格式。
* `QUERY_LOG_MIN_MSEC = <value>` 更改 [query_log_min_msec](../Server_settings/Searchd.md#query_log_min_msec) 的 searchd 设置值。在此情况下，期望的值以毫秒为单位，且不解析配置中的时间后缀。
  > 警告：这是一个非常特殊且“硬性”的变量；被过滤的消息将被直接丢弃，不会写入日志。最好使用类似“grep”的工具过滤日志，这样至少你会有完整的原始日志作为备份。
* `RESET_NETWORK_TIMEOUT_ON_PACKET = {1|0}` 更改 [reset_network_timeout_on_packet](../Server_settings/Searchd.md#reset_network_timeout_on_packet) 参数。只有拥有 VIP 连接的客户端才能更改此变量。
* `SECONDARY_INDEXES = {1|0}` 开启/关闭用于搜索查询的[二级索引](../Server_settings/Searchd.md#secondary_indexes)。
* `TIMEZONE = <value>` 指定日期/时间相关函数使用的时区。详情请参阅 [timezone](../Server_settings/Searchd.md) 配置参数文档。

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

