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

1.  per-session server variable
2.  global server variable
3.  global user variable
4.  global distributed variable

Global user variables are shared between concurrent sessions. Currently, the only supported value type is the list of BIGINTs, and these variables can only be used along with IN() for filtering purpose. The intended usage scenario is uploading huge lists of values to `searchd` (once) and reusing them (many times) later, saving on network overheads. Global user variables might be either transferred to all agents of distributed index or set locally in case of local index defined at distributed index. Example:

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

*   `AUTOCOMMIT = {0 | 1}` Whether any data modification statement should be implicitly wrapped by `BEGIN` and `COMMIT`.
*   `COLLATION_CONNECTION = collation_name` Selects the collation to be used for `ORDER BY` or `GROUP BY` on string values in the subsequent queries. Refer to [Collations](Searching/Collations.md) for a list of known collation names.
*   `CHARACTER_SET_RESULTS = charset_name` Does nothing; a placeholder to support frameworks, clients, and connectors that attempt to automatically enforce a charset when connecting to a Manticore server.
*   `SQL_AUTO_IS_NULL = value` Does nothing; a placeholder to support frameworks, clients, and connectors that attempt to automatically enforce a charset when connecting to a Manticore server.
*   `SQL_MODE = value` Does nothing; a placeholder to support frameworks, clients, and connectors that attempt to automatically enforce a charset when connecting to a Manticore server.
*   `WAIT_TIMEOUT = value` Set connection timeout, either per session or change the global one. Can only be set on a VIP connection.
*   `PROFILING = {0 | 1}` Enables query profiling in the current session. Defaults to 0. See also [show profile](Profiling_and_monitoring/Profiling/Query_profile.md)

Known global server variables are:

* `QUERY_LOG_FORMAT = {plain | sphinxql}` Changes the current log format.
* `LOG_LEVEL = {info | debug | replication | debugv | debugvv}` Changes the current log verboseness level.
* `QCACHE_MAX_BYTES = <value>` Changes the [query_cache](Searching/Query_cache.md) RAM use limit to a given value.
* `QCACHE_THRESH_MSEC = <value>` Changes the [query_cache>](Searching/Query_cache.md) minimum wall time threshold to a given value.
* `QCACHE_TTL_SEC = <value>` Changes the [query_cache](Searching/Query_cache.md) TTL for a cached result to a given value.
* `MAINTENANCE = {0 | 1}` When set to 1, puts the server in maintenance mode. Only clients with vip connections can execute queries in this mode. All new non-vip incoming connections are refused.
* `GROUPING_IN_UTC = {0 | 1}` When set to 1, cause timed grouping functions (day(), month(), year(), yearmonth(), yearmonthday()) to be calculated in utc. Read the doc for [grouping_in_utc](Server_settings/Searchd.md) config params for more details.
* `QUERY_LOG_MIN_MSEC = <value>` Changes the [query_log_min_msec](Server_settings/Searchd.md#query_log_min_msec) searchd settings value. In this case it expects value exactly in milliseconds and doesn't parse time suffixes, as in config.
* `LOG_DEBUG_FILTER = <string value>` Option exists since Sphinx 2.11 and gives a chance to filter out redundant log messages. If the value is set, then all logs with level > INFO (i.e., `DEBUG`, `DEBUGV`, etc.) will be compared with the string and output only in the case they starts with given value.
* `CPUSTAT = {0 | 1}` Enable or disable CPU time reporting in query log and status reports.
* `IOSTAT = {0 | 1}` Enable or disable I/O operations reporting in query log.
* `COREDUMP = {0 | 1}` Enable or disable saving the core file or minidump on case of a crash.


> Warning
This is very specific and 'hard' variable; filtered out messages will be just dropped and not written into the log at all. Better just filter your log with something like 'grep', in this case you'll have at least full original log as backup.

* `NET_WAIT = {-1 | 0 | POSITIVE_INT_VALUE}` Changes the [net_wait_tm](Server_settings/Searchd.md#net_wait_tm) searchd settings value.
* `cpustats= {1|0}` Turns on/off [cpu time tracking](Starting_the_server/Manually#searchd-command-line-options).
* `coredump= {1|0}` Tunes on/off saving a core file or a minidump of the server on crash. More details [here](Starting_the_server/Manually#searchd-command-line-options).

Examples:

```sql
mysql> SET autocommit=0;
Query OK, 0 rows affected (0.00 sec)

mysql> SET GLOBAL query_log_format=sphinxql;
Query OK, 0 rows affected (0.00 sec)
```
