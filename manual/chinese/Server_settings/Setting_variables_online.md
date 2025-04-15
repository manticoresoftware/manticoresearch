# 设置在线变量

## 设置
```sql
SET [GLOBAL] server_variable_name = value
SET [INDEX table_name] GLOBAL @user_variable_name = (int_val1 [, int_val2, ...])
SET NAMES value [COLLATE value]
SET @@dummy_variable = ignored_value
```

`SET` 语句在 Manticore Search 中允许您修改变量值。变量名称不区分大小写，且在服务器重启后，变量值的更改将不会持续存在。

Manticore Search 支持 `SET NAMES` 语句和 `SET @@variable_name` 语法，以便与可能在连接时需要运行这些语句的第三方 MySQL 客户端库、连接器和框架兼容。然而，这些语句对 Manticore Search 本身没有影响。

Manticore Search 中有四类变量：

1.  会话级服务器变量： `set var_name = value`
2.  全局服务器变量： `set global var_name = value`
3.  全局用户变量： `set global @var_name = (value)`
4.  全局分布式变量： `set index dist_table_name global @var_name = (value)`

全局用户变量在并发会话之间共享。唯一支持的值类型是 BIGINTs 列表，这些变量可以与 IN() 操作符结合使用进行过滤。此功能的主要用例是一次性上传大量值到 `searchd`，并在后续多次重用，从而减少网络开销。全局用户变量可以传递给分布式表的所有代理，或在定义在分布式表中的本地表情况下本地设置。示例：

```sql
// 在会话 1
mysql> SET GLOBAL @myfilter=(2,3,5,7,11,13);
Query OK, 0 rows affected (0.00 sec)

// 后来在会话 2
mysql> SELECT * FROM test1 WHERE group_id IN @myfilter;
+------+--------+----------+------------+-----------------+------+
| id   | weight | group_id | date_added | title           | tag  |
+------+--------+----------+------------+-----------------+------+
|    3 |      1 |        2 | 1299338153 | 另一个文档     | 15   |
|    4 |      1 |        2 | 1299338153 | 第四个文档     | 7,40 |
+------+--------+----------+------------+-----------------+------+
2 rows in set (0.02 sec)
```

Manticore Search 支持会话级和全局服务器变量，影响各自范围内的特定服务器设置。以下是已知的会话级和全局服务器变量的列表：

已知的会话级服务器变量：

* `AUTOCOMMIT = {0 | 1}` 确定数据修改语句是否应隐式封装在 `BEGIN` 和 `COMMIT` 之间。
* `COLLATION_CONNECTION = collation_name` 选择后续查询中字符串值的 `ORDER BY` 或 `GROUP BY` 的排序规则。有关已知排序规则名称的列表，请参见 [Collations](../Searching/Collations.md)。
* `MAX_THREADS_PER_QUERY = <POSITIVE_INT_VALUE>` 在运行时重新定义 [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query)。会话变量仅影响在同一会话（连接）中运行的查询，即在断开连接之前。值为 0 意味着“无限制”。如果同时设置了会话和全局变量，则会话级的优先级更高。
* `net_write_timeout = <value>`: 调整写操作的网络超时，即发送数据。全局值只能在 VIP 权限下更改。
* `optimize_by_id = {0 | 1}`: 在某些 `debug` 命令中使用的内部标志。
* `PROFILING = {0 | 1}` 启用当前会话中的查询分析。默认为 0。另请参见 [show profile](../Node_info_and_management/Profiling/Query_profile.md)。
* `ro = {1 | 0}` 切换会话为只读模式或返回。在 `show variables` 输出中，该变量以 `session_read_only` 名称显示。
* `throttling_period = <INT_VALUE>`: 当前正在运行的查询将重新调度的时间间隔（以毫秒为单位）。值为 0 禁用限流，意味着查询将占用 CPU 核心直到完成。如果其他连接在同一时间提交并发查询，它们将分配给空闲核心，或者将被暂停直到释放核心。提供负值 (-1) 将限流重置为默认编译值（100ms），这意味着查询将每 100ms 重新调度，允许并发查询有机会执行。全局值（通过 `set global` 设置）只能在 VIP 连接上设置。
* `thread_stack = <value>`: 动态更改一个任务提供的默认值，该值限制栈大小。请注意，这里的“线程”并不指操作系统线程，而是用户空间线程，也称为协程。如果例如您加载一个具有意外高需求的 percolate 表，这可能会很有用。在这种情况下，“call pq” 将失败并显示有关栈大小不足的消息。一般来说，您应停止守护进程，增加配置中的值，然后重新启动。然而，您也可以尝试在不重新启动的情况下设置一个新值。全局值也可以通过 `set global thread_stack` 在线更改，但仅在 VIP 连接中可用。
* `threads_ex` （诊断）: 强制 Manticore 表现得好像它在具有所提供配置的 CPU 上运行。作为一个简单的示例，`set threads_ex='4/2+6/3'` 表示“您有 4 个空闲 CPU 核心，当调度多个查询时，它们应按 2 分批。同时，您有 6 个空闲 CPU 核心用于伪分片，部分应按 3 分批。”此选项是诊断性的，因为它非常有帮助，例如，查看您的查询在您本地没有的配置下将如何运行。例如，在 128 核心 CPU 上。或者，相反，快速限制守护进程表现为单线程，以查找瓶颈或调查崩溃。

  <details>

	默认情况下，Manticore 启动一个线程池，计算出 CPU 核心的数量。所有典型的任务都被分配到这个线程池中，以确保最大 CPU 利用率。例如，当一个实时表有多个磁盘块时，搜索将在 CPU 核心上并行化。类似地，对于单个索引上的单个全文搜索，守护进程将尝试使用一种称为“伪分片”的技术来优化搜索执行的并行性。这两个特性都在很大程度上依赖于 CPU 核心的总数和可用于即时使用的空闲核心的数量。

	这种方法提高了性能，但可能使事件调查变得更加困难。例如，执行 `COUNT(*)` 的查询可能返回一个近似结果（例如，大于 100 个匹配），而之后执行的相同查询可能会产生一个精确结果（例如，正好 120 个匹配）。这种可变性取决于可用核心，但是由于这个因素是不可预测的，它通常会导致不可重现的结果。尽管这通常是可以接受的，但有时会造成问题。`threads_ex` 选项指定了所需的 CPU 核心配置，使得具有此配置的查询可重现。

	`threads_ex` 为标准任务和伪分片设置 CPU 模板，因为伪分片可以是标准并行化过程的一部分。例如，如果有多个磁盘块，它们将并行查询，但每个块可能会使用伪分片进一步并行化。因此，为了有效管理这种情况，您需要为每种任务类型准备几个模板。

	模板是一个字符串，例如 `10/3`，其中 10 表示并发，3 表示批大小。如果并发为 0，则使用默认并发。如果批大小为 0，则使用默认的平凡模板。任何零值可以省略或替换为 `*`。默认（平凡）模板可以描述为 `''`，还可以为 `*/*`、`0/0`、`0/`、`*/`、`/0`、`*` 等。这意味着守护进程使用所有可用的 CPU 核心，而没有特别的批处理限制。

	具有 20 个线程的平凡模板可以表示为 `20/*`、`20/0`、`20/`，或简单地表示为 `20`。具有批大小为 2 的轮询模板为 `*/2`、`0/2`，或简单地为 `/2`。具有 20 个线程和批大小为 3 的轮询调度器为 `20/3`。

	`threads_ex` 是基本任务和伪分片的模板，以 `+` 分隔，如下所示：

	* `30+3` - 30 个线程的平凡基数 + 3 个线程的平凡伪分片
	* `+/2` - 平凡基数 + 默认线程和批量=2 的轮询伪分片
	* `10` - 10 个线程的平凡基数 + 默认平凡伪分片
	* `/1+10` - 默认线程和批量=1 的轮询基数 + 10 个线程的平凡伪分片
	* `4/2+2/1` - 4 个线程和批量=2 的轮询基数 + 2 个线程和批量=1 的轮询伪分片
	* `1+1` - 最确定的情况。正好 1 个线程 + 1 个伪分片，即根本没有并行。通过此设置，您可以可重现地重复相同的问题查询并调查行为细节。此外，如果设置 `throttling_period=0`，您的查询将“粘附”到当前线程，并且在执行过程中不会被重新调度，从而创建一个理想的故障排除环境。

	该选项可以从外部全局设置为环境变量 `MANTICORE_THREADS_EX`，如：

	```bash
	export MANTICORE_THREADS_EX=8
	export MANTICORE_THREADS_EX='16+8/2'
  ```

  或通过 MySQL CLI，如：
  ```sql
  SET threads_ex='16';
	SET GLOBAL threads_ex='/2';
  ```

  或作为查询参数，如：
  ```sql
  SELECT ... OPTION threads_ex='1+1';
  ```

  `threads_ex` 配置遵循一个层级：首先是环境变量，然后是全局变量，最后是查询选项，允许特定设置覆盖一般设置。

  </details>
* `WAIT_TIMEOUT/net_read_timeout = <value>` 设置连接超时，可以是每个会话或全局。全局只能在 VIP 连接上设置。

已知的全局服务器变量有：

* `ACCURATE_AGGREGATION`：设置未来查询的选项 [accurate_aggregation](../Searching/Options.md#accurate_aggregation) 的默认值。
* `AUTO_OPTIMIZE = {1|0}` 打开/关闭 [auto_optimize](../Server_settings/Searchd.md#auto_optimize)。
* `cluster_user = name` 设置与 `mysqldump` / `mariadb-dump` 一起使用的用户名，以 [启用复制模式](../Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)。
* `COREDUMP= {1|0}` 打开/关闭在崩溃时保存服务器的核心文件或小型转储。更多细节 [这里](../Starting_the_server/Manually.md#searchd-command-line-options)。
* `CPUSTATS= {1|0}` 打开/关闭 [CPU 时间跟踪](../Starting_the_server/Manually.md#searchd-command-line-options)。
* `DISTINCT_PRECISION_THRESHOLD`：设置未来查询的选项 [distinct_precision_threshold](../Searching/Options.md#distinct_precision_threshold) 的默认值。
* `ES_COMPAT = {on/off/dashboards}` 设置为 `on`（默认）时，支持类似 Elasticsearch 的写请求；`off` 禁用支持；`dashboards` 启用支持并允许来自 Kibana 的请求（此功能为实验性）。
* `EXPANSION_MERGE_THRESHOLD_DOCS`：动态更改配置的 [expansion_merge_threshold_docs](Server_settings/Searchd.md#expansion_merge_threshold_docs) 设置的值。
* `EXPANSION_MERGE_THRESHOLD_HITS`：动态更改配置的 [expansion_merge_threshold_hits](Server_settings/Searchd.md#expansion_merge_threshold_hits) 设置的值。
* `GROUPING_IN_UTC = {0 | 1}` 设置为 1 时，使计时分组函数（day()、month()、year()、yearmonth()、yearmonthday()）以 UTC 计算。阅读文档以获取更多关于 [grouping_in_utc](../Server_settings/Searchd.md) 配置参数的详细信息。
* `IOSTATS = {0 | 1}` 启用或禁用查询日志中报告 I/O 操作（属性除外）。
* `LOG_DEBUG_FILTER = <string value>` 过滤冗余日志消息。如果设置了该值，则所有级别 > INFO（即 `DEBUG`、`DEBUGV` 等）的日志将与该字符串进行比较，并仅在它们以给定值开头时输出。
* `LOG_LEVEL = {info | debug | replication | debugv | debugvv}` 更改当前日志详细程度级别。
* `MAINTENANCE = {0 | 1}` 设置为 1 时，将服务器置于维护模式。仅具有 VIP 连接的客户端可以在此模式下执行查询。拒绝所有新的非 VIP 传入连接。现有连接保持不变。
* `MAX_THREADS_PER_QUERY = <POSITIVE_INT_VALUE>` 在运行时重新定义 [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query)。作为全局设置，它会改变所有会话的行为。值为 0 意味着“无限制”。如果同时设置了每会话变量和全局变量，则每会话变量具有更高的优先级。
* `NET_WAIT = {-1 | 0 | POSITIVE_INT_VALUE}` 更改 [net_wait_tm](../Server_settings/Searchd.md#net_wait_tm) searchd 设置值。
* `OPTIMIZE_CUTOFF = <value>`：动态更改配置的 [optimize_cutoff](../Server_settings/Searchd.md#optimize_cutoff) 设置值。
* `PSEUDO_SHARDING = {1|0}` 开启/关闭搜索 [pseudo-sharding](../Server_settings/Searchd.md#pseudo_sharding)。
* `QCACHE_MAX_BYTES = <value>` 将 [query_cache](../Searching/Query_cache.md) 的 RAM 使用限制更改为给定值。
* `QCACHE_THRESH_MSEC = <value>` 将 [query_cache](../Searching/Query_cache.md) 的最小墙时间阈值更改为给定值。
* `QCACHE_TTL_SEC = <value>` 将 [query_cache](../Searching/Query_cache.md) 的缓存结果 TTL 更改为给定值。
* `QUERY_LOG_FORMAT = {plain | sphinxql}` 更改当前日志格式。
* `QUERY_LOG_MIN_MSEC = <value>` 更改 [query_log_min_msec](../Server_settings/Searchd.md#query_log_min_msec) searchd 设置值。在这种情况下，它期望值精确以毫秒表示，并且不解析时间后缀，如在配置中。
  > 警告：这是一个非常具体和“硬”的变量；过滤掉的消息将被直接丢弃，而根本不会写入日志。最好用类似“grep”的东西过滤你的日志，在这种情况下，您至少会有完整的原始日志作为备份。
* `RESET_NETWORK_TIMEOUT_ON_PACKET = {1|0}` 更改 [reset_network_timeout_on_packet](../Server_settings/Searchd.md#reset_network_timeout_on_packet) 参数。仅具有 VIP 连接的客户端可以更改此变量。
* `SECONDARY_INDEXES = {1|0}` 开启/关闭用于搜索查询的 [secondary indexes](../Server_settings/Searchd.md#secondary_indexes)。
* `TIMEZONE = <value>` 指定用于日期/时间相关函数的时区。有关更多详细信息，请阅读 [timezone](../Server_settings/Searchd.md) 配置参数的文档。

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

要使用户变量持久化，请确保 [sphinxql_state](../Server_settings/Searchd.md#sphinxql_state) 被启用。
<!-- proofread -->
