# Setting variables online

## SET
```sql
SET [GLOBAL] server_variable_name = value
SET [INDEX table_name] GLOBAL @user_variable_name = (int_val1 [, int_val2, ...])
SET NAMES value [COLLATE value]
SET @@dummy_variable = ignored_value
```

The `SET` statement in Manticore Search allows you to modify variable values. Variable names are case-insensitive, and no variable value changes will persist after a server restart.

Manticore Search supports the `SET NAMES` statement and `SET @@variable_name` syntax for compatibility with third-party MySQL client libraries, connectors, and frameworks that may require running these statements when connecting. However, these statements do not have any effect on Manticore Search itself.

There are four classes of variables in Manticore Search:

1.  Per-session server variable: `set var_name = value`
2.  Global server variable: `set global var_name = value`
3.  Global user variable: `set global @var_name = (value)`
4.  Global distributed variable: `set index dist_table_name global @var_name = (value)`

Global user variables are shared between concurrent sessions. The only supported value type is a list of BIGINTs, and these variables can be used with the IN() operator for filtering purposes. The primary use case for this feature is to upload large lists of values to `searchd` once and reuse them multiple times later, reducing network overhead. Global user variables can be transferred to all agents of a distributed table or set locally in the case of a local table defined in a distributed table. Example:

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

Manticore Search supports per-session and global server variables that affect specific server settings in their respective scopes. Below is a list of known per-session and global server variables:

Known per-session server variables:

* `AUTOCOMMIT = {0 | 1}` determines if data modification statements should be implicitly wrapped by `BEGIN` and `COMMIT`.
* `COLLATION_CONNECTION = collation_name` selects the collation for `ORDER BY` or `GROUP BY` on string values in subsequent queries. Refer to [Collations](../Searching/Collations.md) for a list of known collation names.
* `MAX_THREADS_PER_QUERY = <POSITIVE_INT_VALUE>` redefines [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query) in the runtime. Per-session variable influences only the queries run in the same session (connection), i.e., up to disconnect. Value 0 means 'no limit'. If both per-session and the global variables are set, the per-session one has a higher priority.
* `net_write_timeout = <value>`: Tunes the network timeout for write operations, i.e., sending data. The global value can be changed only with VIP privileges.
* `optimize_by_id = {0 | 1}`: Internal flag used in some `debug` commands.
* `PROFILING = {0 | 1}` enables query profiling in the current session. Defaults to 0. See also [show profile](../Node_info_and_management/Profiling/Query_profile.md).
* `ro = {1 | 0}` switches session to read-only mode or back. In `show variables` output the variable displayed with name `session_read_only`.
* `throttling_period = <INT_VALUE>`: Interval (in milliseconds) during which the current running query will reschedule. A value of 0 disables throttling, meaning the query will occupy CPU cores until it finishes. If concurrent queries come from other connections at the same time, they will be allocated to free cores or will be suspended until a core is released. Providing a negative value (-1) resets throttling to the default compiled-in value (100ms), which means the query will be rescheduled every 100ms, allowing concurrent queries a chance to be executed. The global value (set via `set global`) can only be set on a VIP connection.
* `thread_stack = <value>`: Changes the default value on-the-fly, which limits the stack size provided to one task. Note that here 'thread' refers not to an OS thread, but to a userspace thread, also known as a coroutine. This can be useful if, for example, you load a percolate table with unexpectedly high requirements. In such cases, 'call pq' would fail with a message about insufficient stack size. Generally, you should stop the daemon, increase the value in the config, and then restart. However, you can also try a new value without restarting, by setting a new one with this variable. The global value can also be changed online with `set global thread_stack`, but this is available only from a VIP connection.
* `threads_ex` (diagnostic): Forces Manticore to behave as if it is running on a CPU with the provided profile. As a short example, `set threads_ex='4/2+6/3'` indicates 'you have 4 free CPU cores, when scheduling multiple queries they should be batched by 2. Also, you have 6 free CPU cores for pseudo-sharding, parts should be batched by 3'. This option is diagnostic, as it is very helpful, for example, to see how your query would run on a configuration you don't have locally. For instance, on a 128-core CPU. Or, conversely, to quickly limit the daemon to behave as single-threaded, to locate a bottleneck or investigate a crash.

  <details>

	By default, Manticore starts a thread pool with a calculated number of CPU cores. All typical tasks are then distributed into this thread pool to ensure maximum CPU utilization. For instance, when a real-time table has several disk chunks, the search will be parallelized over the CPU cores. Similarly, for a single full-text search over a single index, the daemon will attempt to optimize search execution in parallel, using a technique referred to as "pseudo-sharding". Both features heavily depend on the total number of CPU cores and the number of free cores available for immediate use.

	This approach enhances performance but can make incident investigation more challenging. For example, a query doing `COUNT(*)` may return an approximate result (e.g., greater than 100 matches), and a subsequent execution of the same query may yield an exact result (e.g., exactly 120 matches). This variability depends on the available cores, but since this factor is unpredictable, it generally leads to non-reproducible results. Although this is usually acceptable, it can sometimes pose a problem. The `threads_ex` option specifies a desired CPU cores configuration, making queries with this configuration reproducible.

	`threads_ex` sets the CPU template for standard tasks and for pseudo-sharding, as pseudo-sharding can be part of the standard parallelization process. For example, if there are several disk chunks, they will be queried in parallel, but each may be further parallelized using pseudo-sharding. Thus, to manage this situation effectively, you need a couple of templates for each task type.

	A template is a string like `10/3`, where 10 represents concurrency and 3 represents batch size. If concurrency is 0, the default concurrency will be used. If batch size is 0, the default trivial template will be used. Any zero value can be omitted or replaced with `*`. The default (trivial) template can be described as `''`, and also as `*/*`, `0/0`, `0/`, `*/`, `/0`, `*`, etc. This means the daemon uses all available CPU cores without special batching limitations.

	A trivial template with 20 threads can be expressed as `20/*`, `20/0`, `20/`, or simply `20`. A round-robin template with a batch size of 2 is `*/2`, `0/2`, or simply `/2`. A round-robin dispatcher with 20 threads and a batch size of 3 is `20/3`.

	`threads_ex` is a template for basic tasks and for pseudo-sharding, separated by `+`, like:

	* `30+3` - a trivial base of 30 threads + trivial pseudo-sharding of 3 threads
	* `+/2` - a trivial base + round-robin pseudo-sharding with default threads and batch=2
	* `10` - a trivial base of 10 threads + default trivial pseudo-sharding
	* `/1+10` - a round-robin base with default threads and batch=1 + trivial pseudo-sharding with 10 threads
	* `4/2+2/1` - a round-robin base with 4 threads and batch=2 + round-robin pseudo-sharding with 2 threads and batch=1
	* `1+1` - the most deterministic case. Exactly 1 thread + 1 pseudo-shard, i.e., no parallelization at all. With this setting, you can reproducibly repeat the same problematic query and investigate behavior details. Furthermore, if you set `throttling_period=0`, your query will 'stick' to the current thread and never be rescheduled during execution, creating an ideal environment for troubleshooting.

	The option can be set globally from outside as an environment variable `MANTICORE_THREADS_EX`, like:

	```bash
	export MANTICORE_THREADS_EX=8
	export MANTICORE_THREADS_EX='16+8/2'
  ```

  Or, via the MySQL CLI, as:
  ```sql
  SET threads_ex='16';
	SET GLOBAL threads_ex='/2';
  ```

  Or, as a query parameter, like:
  ```sql
  SELECT ... OPTION threads_ex='1+1';
  ```

  The `threads_ex` configuration follows a hierarchy: environment variables first, then the global variable, and lastly, query options, allowing specific settings to override general ones.

  </details>
* `WAIT_TIMEOUT/net_read_timeout = <value>` sets connection timeout, either per session or global. Global can only be set on a VIP connection.

Known global server variables are:

* `ACCURATE_AGGREGATION`: Sets the default value for the option [accurate_aggregation](../Searching/Options.md#accurate_aggregation) of future queries.
* `AUTO_OPTIMIZE = {1|0}` Turns on/off [auto_optimize](../Server_settings/Searchd.md#auto_optimize).
* `cluster_user = name` Sets the username used with `mysqldump` / `mariadb-dump` to [enable replication mode](../Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump).
* `COREDUMP= {1|0}` Turns on/off saving a core file or a minidump of the server on crash. More details [here](../Starting_the_server/Manually.md#searchd-command-line-options).
* `CPUSTATS= {1|0}` Turns on/off [CPU time tracking](../Starting_the_server/Manually.md#searchd-command-line-options).
* `DISTINCT_PRECISION_THRESHOLD`: Sets the default value for the option [distinct_precision_threshold](../Searching/Options.md#distinct_precision_threshold) of future queries.
* `ES_COMPAT = {on/off/dashboards}` When set to `on` (default), Elasticsearch-like write requests are supported; `off` disables the support; `dashboards` enables the support and also allows requests from Kibana (this functionality is experimental).
* `EXPANSION_MERGE_THRESHOLD_DOCS`: Changes the value of the config's [expansion_merge_threshold_docs](Server_settings/Searchd.md#expansion_merge_threshold_docs) setting on-the-fly.
* `EXPANSION_MERGE_THRESHOLD_HITS`: Changes the value of the config's [expansion_merge_threshold_hits](Server_settings/Searchd.md#expansion_merge_threshold_hits) setting on-the-fly.
* `GROUPING_IN_UTC = {0 | 1}` When set to 1, causes timed grouping functions (day(), month(), year(), yearmonth(), yearmonthday()) to be calculated in UTC. Read the doc for [grouping_in_utc](../Server_settings/Searchd.md) config params for more details.
* `IOSTATS = {0 | 1}` Enables or disables I/O operations (except for attributes) reporting in the query log.
* `LOG_DEBUG_FILTER = <string value>` Filters out redundant log messages. If the value is set, then all logs with level > INFO (i.e., `DEBUG`, `DEBUGV`, etc.) will be compared with the string and output only in the case they start with the given value.
* `LOG_LEVEL = {info | debug | replication | debugv | debugvv}` Changes the current log verboseness level.
* `MAINTENANCE = {0 | 1}` When set to 1, puts the server in maintenance mode. Only clients with VIP connections can execute queries in this mode. All new non-VIP incoming connections are refused. Existing connections are left intact.
* `MAX_THREADS_PER_QUERY = <POSITIVE_INT_VALUE>` Redefines [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query) at runtime. As global, it changes behavior for all sessions. Value 0 means 'no limit'. If both per-session and global variables are set, the per-session one has a higher priority.
* `NET_WAIT = {-1 | 0 | POSITIVE_INT_VALUE}` Changes the [net_wait_tm](../Server_settings/Searchd.md#net_wait_tm) searchd settings value.
* `OPTIMIZE_CUTOFF = <value>`: Changes the value of the config's [optimize_cutoff](../Server_settings/Searchd.md#optimize_cutoff) setting on-the-fly.
* `PSEUDO_SHARDING = {1|0}` Turns on/off search [pseudo-sharding](../Server_settings/Searchd.md#pseudo_sharding).
* `QCACHE_MAX_BYTES = <value>` Changes the [query_cache](../Searching/Query_cache.md) RAM use limit to a given value.
* `QCACHE_THRESH_MSEC = <value>` Changes the [query_cache](../Searching/Query_cache.md) minimum wall time threshold to a given value.
* `QCACHE_TTL_SEC = <value>` Changes the [query_cache](../Searching/Query_cache.md) TTL for a cached result to a given value.
* `QUERY_LOG_FORMAT = {plain | sphinxql}` Changes the current log format.
* `QUERY_LOG_MIN_MSEC = <value>` Changes the [query_log_min_msec](../Server_settings/Searchd.md#query_log_min_msec) searchd settings value. In this case, it expects the value exactly in milliseconds and doesn't parse time suffixes, as in config.
  > Warning: this is a very specific and 'hard' variable; filtered out messages will be just dropped and not written into the log at all. Better just filter your log with something like 'grep', in this case, you'll have at least the full original log as a backup.
* `RESET_NETWORK_TIMEOUT_ON_PACKET = {1|0}` changes [reset_network_timeout_on_packet](../Server_settings/Searchd.md#reset_network_timeout_on_packet) param. Only clients with VIP connections can change this variable.
* `SECONDARY_INDEXES = {1|0}` Turns on/off [secondary indexes](../Server_settings/Searchd.md#secondary_indexes) for search queries.
* `TIMEZONE = <value>` Specifies the timezone used by date/time-related functions. Read the doc for [timezone](../Server_settings/Searchd.md) config param for more details.

Examples:

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

To make user variables persistent, make sure [sphinxql_state](../Server_settings/Searchd.md#sphinxql_state) is enabled.
<!-- proofread -->
