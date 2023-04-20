# Setting variables online

## SET
```sql
SET [GLOBAL] server_variable_name = value
SET [INDEX index_name] GLOBAL @user_variable_name = (int_val1 [, int_val2, ...])
SET NAMES value [COLLATE value]
SET @@dummy_variable = ignored_value
```

The `SET` statement in Manticore Search allows you to modify variable values. Variable names are case-insensitive, and no variable value changes will persist after a server restart.

Manticore Search supports the `SET NAMES` statement and `SET @@variable_name` syntax for compatibility with third-party MySQL client libraries, connectors, and frameworks that may require running these statements when connecting. However, these statements do not have any effect on Manticore Search itself.

There are four classes of variables in Manticore Search:

1.  Per-session server variable: `set var_name = value`
2.  Global server variable: `set global var_name = value`
3.  Global user variable: `set global @var_name = (value)`
4.  Global distributed variable: `set index dist_index_name global @var_name = (value)`

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
* `WAIT_TIMEOUT = <value>` sets connection timeout, either per session or global. Global can only be set on a VIP connection.
* `PROFILING = {0 | 1}` enables query profiling in the current session. Defaults to 0. See also [show profile](../Node_info_and_management/Profiling/Query_profile.md).
* `MAX_THREADS_PER_QUERY = <POSITIVE_INT_VALUE>` redefines [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query) in the runtime. Per-session variable influences only the queries run in the same session (connection), i.e. up to disconnect. Value 0 means 'no limit'. If both per-session and the global variables are set, the per-session one has a higher priority.
* `ro = {1 | 0}` switches session to read-only mode or back. In `show variables` output the variable displayed with name `session_read_only`.

Known global server variables are:

* `QUERY_LOG_FORMAT = {plain | sphinxql}` Changes the current log format.
* `LOG_LEVEL = {info | debug | replication | debugv | debugvv}` Changes the current log verboseness level.
* `QCACHE_MAX_BYTES = <value>` Changes the [query_cache](../Searching/Query_cache.md) RAM use limit to a given value.
* `QCACHE_THRESH_MSEC = <value>` Changes the [query_cache>](../Searching/Query_cache.md) minimum wall time threshold to a given value.
* `QCACHE_TTL_SEC = <value>` Changes the [query_cache](../Searching/Query_cache.md) TTL for a cached result to a given value.
* `MAINTENANCE = {0 | 1}` When set to 1, puts the server in maintenance mode. Only clients with VIP connections can execute queries in this mode. All new non-VIP incoming connections are refused. Existing connections are left intact.
* `GROUPING_IN_UTC = {0 | 1}` When set to 1, causes timed grouping functions (day(), month(), year(), yearmonth(), yearmonthday()) to be calculated in UTC. Read the doc for [grouping_in_utc](../Server_settings/Searchd.md) config params for more details.
* `QUERY_LOG_MIN_MSEC = <value>` Changes the [query_log_min_msec](../Server_settings/Searchd.md#query_log_min_msec) searchd settings value. In this case, it expects the value exactly in milliseconds and doesn't parse time suffixes, as in config.
  > Warning: this is a very specific and 'hard' variable; filtered out messages will be just dropped and not written into the log at all. Better just filter your log with something like 'grep', in this case, you'll have at least the full original log as a backup.
* `LOG_DEBUG_FILTER = <string value>` Filters out redundant log messages. If the value is set, then all logs with level > INFO (i.e., `DEBUG`, `DEBUGV`, etc.) will be compared with the string and output only in the case they start with the given value.
* `MAX_THREADS_PER_QUERY = <POSITIVE_INT_VALUE>` Redefines [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query) at runtime. As global, it changes behavior for all sessions. Value 0 means 'no limit'. If both per-session and global variables are set, the per-session one has a higher priority.
* `NET_WAIT = {-1 | 0 | POSITIVE_INT_VALUE}` Changes the [net_wait_tm](../Server_settings/Searchd.md#net_wait_tm) searchd settings value.
* `IOSTATS = {0 | 1}` Enables or disables I/O operations (except for attributes) reporting in the query log.
* `CPUSTATS= {1|0}` Turns on/off [CPU time tracking](../Starting_the_server/Manually.md#searchd-command-line-options).
* `COREDUMP= {1|0}` Turns on/off saving a core file or a minidump of the server on crash. More details [here](../Starting_the_server/Manually.md#searchd-command-line-options).
* `PSEUDO_SHARDING = {1|0}` Turns on/off search [pseudo-sharding](../Server_settings/Searchd.md#pseudo_sharding).
* `SECONDARY_INDEXES = {1|0}` Turns on/off [secondary indexes](../Server_settings/Searchd.md#secondary_indexes) for search queries.
* `ES_COMPAT = {on/off/dashboards}` When set to `on` (default), Elasticsearch-like write requests are supported; `off` disables the support; `dashboards` enables the support and also allows requests from Kibana (this functionality is experimental).

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
