# Setting variables online

## SET
```sql
SET [GLOBAL] server_variable_name = value
SET [INDEX index_name] GLOBAL @user_variable_name = (int_val1 [, int_val2, ...])
SET NAMES value [COLLATE value]
SET @@dummy_variable = ignored_value
```

`SET` statement modifies a variable value. The variable names are case-insensitive. No variable value changes survive server restart.

`SET NAMES` statement and `SET @@variable_name` syntax, both introduced do nothing. They were implemented to maintain compatibility with 3rd party MySQL client libraries, connectors, and frameworks that may need to run this statement when connecting.

There are the following classes of the variables:

1.  per-session server variable: `set var_name = value`
2.  global server variable: `set global var_name = value`
3.  global user variable: `set global @var_name = (value)`
4.  global distributed variable: `set index dist_index_name global @var_name = (value)`

Global user variables are shared between concurrent sessions. Currently, the only supported value type is the list of BIGINTs, and these variables can only be used along with IN() for filtering purpose. The intended usage scenario is uploading huge lists of values to `searchd` (once) and reusing them (many times) later, saving on network overheads. Global user variables might be either transferred to all agents of distributed table or set locally in case of local table defined at distributed table. Example:

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

Per-session and global server variables affect certain server settings in the respective scope. Known per-session server variables are:

* `AUTOCOMMIT = {0 | 1}` Whether any data modification statement should be implicitly wrapped by `BEGIN` and `COMMIT`.
* `COLLATION_CONNECTION = collation_name` Selects the collation to be used for `ORDER BY` or `GROUP BY` on string values in the subsequent queries. Refer to [Collations](../Searching/Collations.md) for a list of known collation names.
* `CHARACTER_SET_RESULTS = charset_name` Does nothing; a placeholder to support frameworks, clients, and connectors that attempt to automatically enforce a charset when connecting to a Manticore server.
* `SQL_AUTO_IS_NULL = value` Does nothing; a placeholder to support frameworks, clients, and connectors that attempt to automatically enforce a charset when connecting to a Manticore server.
* `SQL_MODE = <value>` Does nothing; a placeholder to support frameworks, clients, and connectors that attempt to automatically enforce a charset when connecting to a Manticore server.
* `WAIT_TIMEOUT = <value>` Set connection timeout, either per session or global. Global can only be set on a VIP connection.
* `PROFILING = {0 | 1}` Enables query profiling in the current session. Defaults to 0. See also [show profile](../Node_info_and_management/Profiling/Query_profile.md)
* `MAX_THREADS_PER_QUERY = <POSITIVE_INT_VALUE>` Redefines [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query) in the runtime. Per-session variable influences only the queries run in the same session (connection), i.e. up to disconnect. Value 0 means 'no limit'. If both per-session and the global variables are set, the per-session one has a higher priority.
* `ro = {1 | 0}` switch session to read-only mode or back. In `show variables` output the variable displayed with name `session_read_only`.

Known global server variables are:

* `QUERY_LOG_FORMAT = {plain | sphinxql}` Changes the current log format.
* `LOG_LEVEL = {info | debug | replication | debugv | debugvv}` Changes the current log verboseness level.
* `QCACHE_MAX_BYTES = <value>` Changes the [query_cache](../Searching/Query_cache.md) RAM use limit to a given value.
* `QCACHE_THRESH_MSEC = <value>` Changes the [query_cache>](../Searching/Query_cache.md) minimum wall time threshold to a given value.
* `QCACHE_TTL_SEC = <value>` Changes the [query_cache](../Searching/Query_cache.md) TTL for a cached result to a given value.
* `MAINTENANCE = {0 | 1}` When set to 1, puts the server in maintenance mode. Only clients with vip connections can execute queries in this mode. All new non-vip incoming connections are refused. Existing connections are left intact.
* `GROUPING_IN_UTC = {0 | 1}` When set to 1, cause timed grouping functions (day(), month(), year(), yearmonth(), yearmonthday()) to be calculated in utc. Read the doc for [grouping_in_utc](../Server_settings/Searchd.md) config params for more details.
* `QUERY_LOG_MIN_MSEC = <value>` Changes the [query_log_min_msec](../Server_settings/Searchd.md#query_log_min_msec) searchd settings value. In this case it expects value exactly in milliseconds and doesn't parse time suffixes, as in config.
  > Warning: this is very specific and 'hard' variable; filtered out messages will be just dropped and not written into the log at all. Better just filter your log with something like 'grep', in this case you'll have at least full original log as backup.
* `LOG_DEBUG_FILTER = <string value>` Filters out redundant log messages. If the value is set, then all logs with level > INFO (i.e., `DEBUG`, `DEBUGV`, etc.) will be compared with the string and output only in the case they starts with given value.
* `MAX_THREADS_PER_QUERY = <POSITIVE_INT_VALUE>` Redefines [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query) in the runtime. As global it changes behaviour for all sessions. Value 0 means 'no limit'. If both per-session and the global variables are set, the per-session one has a higher priority.
* `NET_WAIT = {-1 | 0 | POSITIVE_INT_VALUE}` Changes the [net_wait_tm](../Server_settings/Searchd.md#net_wait_tm) searchd settings value.
* `IOSTATS = {0 | 1}` Enable or disable I/O operations (except for attributes) reporting in query log.
* `CPUSTATS= {1|0}` Turns on/off [cpu time tracking](../Starting_the_server/Manually.md#searchd-command-line-options).
* `COREDUMP= {1|0}` Turns on/off saving a core file or a minidump of the server on crash. More details [here](../Starting_the_server/Manually.md#searchd-command-line-options).
* `PSEUDO_SHARDING = {1|0}` Turns on/off search [pseudo-sharding](../Server_settings/Searchd.md#pseudo_sharding).
* `SECONDARY_INDEXES = {1|0}` Turns on/off [secondary indexes](../Server_settings/Searchd.md#secondary_indexes) for search queries.
* `ES_COMPAT = {on/off/dashboards}` When set to `on` (default) Elasticsearch-like write requests are supported, `off` disables the support, and `dashboards` enables the support, but also enables support for requests from Kibana (this functionality is experimental).

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

To make user variables persistent make sure [sphinxql_state](../Server_settings/Searchd.md#sphinxql_state) is enabled.
