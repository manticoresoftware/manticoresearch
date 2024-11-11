# Query logging

<!-- example query_logging -->
Query logging can be enabled by setting the [query_log](../Server_settings/Searchd.md#query_log) directive in the searchd section of the configuration file.

Queries can also be sent to syslog by setting `syslog` instead of a file path. In this case, all search queries will be sent to the syslog daemon with `LOG_INFO` priority, prefixed with `[query]` instead of a timestamp. Only the `plain` log format is supported for syslog.

<!-- intro -->
`query_log` example:
<!-- request Config -->
```ini
searchd {
...
    query_log = /var/log/query.log
    query_log_format = sphinxql # default
...
}
```
<!-- end -->


## Logging format

Two query log formats are supported:
* `sphinxql` (default): Logs in SQL format. It also provides an easy way to replay logged queries.
* `plain`: Logs full-text queries in a simple text format. This format is recommended if most of your queries are primarily full-text or if you do not need to log non-full-text components, such as filtering by attributes, sorting, or grouping. Queries logged in the `plain` format cannot be replayed. Note that queries processed through [Buddy](../Installation/Manticore_Buddy.md) are not logged in this mode.

To switch between the formats, you can use the searchd setting [query_log_format](../Server_settings/Searchd.md#query_log_format).

### SQL log format

<!-- example sphixql_log -->
The SQL log format is the default setting. In this mode, Manticore logs all successful and unsuccessful select queries. Requests sent as SQL or via the binary API are logged in the SQL format, but JSON queries are logged as is. This type of logging only works with plain log files and does not support the 'syslog' service for logging.

<!-- intro -->
`query_log_format` example:
<!-- request Config -->
```ini
query_log_format = sphinxql # default
```

<!-- end -->

<!-- example sphixql_log2 -->
The features of the Manticore SQL log format compared to the [plain format](../../Logging/Query_logging.md#Plain-log-format) include:
* Full statement data is logged where possible.
* Errors and warnings are logged.
* The query log can be replayed.
* Additional performance counters (currently, per-agent distributed query times) are logged.
* Each log entry is a valid Manticore SQL/JSON statement that reconstructs the full request, except if the logged request is too large and needs to be shortened for performance reasons.
* JSON requests and additional messages, counters, etc., are logged as comments.

<!-- intro -->
`sphinxql` log entries example:
<!-- request Example -->
```sql
/* Sun Apr 28 12:38:02.808 2024 conn 2 (127.0.0.1:53228) real 0.000 wall 0.000 found 0 */ SELECT * FROM test WHERE MATCH('test') OPTION ranker=proximity;
/* Sun Apr 28 12:38:05.585 2024 conn 2 (127.0.0.1:53228) real 0.001 wall 0.001 found 0 */ SELECT * FROM test WHERE MATCH('test') GROUP BY channel_id OPTION ranker=proximity;
/* Sun Apr 28 12:40:57.366 2024 conn 4 (127.0.0.1:53256) real 0.000 wall 0.000 found 0 */  /*{
    "table" : "test",
    "query":
    {
        "match":
        {
            "*" : "test"
        }
    },
    "_source": ["f"],
    "limit": 30
} */
```
<!-- end -->

### Plain log format

<!-- example plain_log -->
With the `plain` log format, Manticore logs all successfully executed search queries in a simple text format. Non-full-text parts of the queries are not logged. JSON queries are recorded as single-line entries. Queries processed through [Buddy](../Installation/Manticore_Buddy.md) are not logged.

<!-- intro -->
`query_log_format` example:
<!-- request Config -->
```ini
query_log_format = plain
```
<!-- end -->

<!-- example plain_log2 -->
The log format is as follows:

```
[query-date] real-time wall-time [match-mode/filters-count/sort-mode total-matches (offset,limit) @groupby-attr] [table-name] {perf-stats} query
```

where:
* `real-time` is the time from the start to the finish of the query.
* `wall-time` is similar to real-time, but excludes time spent waiting for agents and merging result sets from them.
* `perf-stats` includes CPU/IO stats when Manticore is started with `--cpustats` (or it was enabled via `SET GLOBAL cpustats=1`) and/or `--iostats` (or it was enabled via `SET GLOBAL iostats=1`):
  - `ios` is the number of file I/O operations carried out;
  - `kb` is the amount of data in kilobytes read from the table files;
  - `ms` is the time spent on I/O operations.
  - `cpums` is the time in milliseconds spent on CPU processing the query.
* `match-mode` can have one of the following values:
  - "all" for `SPH_MATCH_ALL` mode;
  - "any" for `SPH_MATCH_ANY` mode;
  - "phr" for `SPH_MATCH_PHRASE` mode;
  - "bool" for `SPH_MATCH_BOOLEAN` mode;
  - "ext" for `SPH_MATCH_EXTENDED` mode;
  - "ext2" for `SPH_MATCH_EXTENDED2` mode;
  - "scan" if the full scan mode was used, either by being specified with `SPH_MATCH_FULLSCAN` or if the query was empty.
* `sort-mode` can have one of the following values:
  - "rel" for `SPH_SORT_RELEVANCE` mode;
  - "attr-" for `SPH_SORT_ATTR_DESC` mode;
  - "attr+" for `SPH_SORT_ATTR_ASC` mode;
  - "tsegs" for `SPH_SORT_TIME_SEGMENTS` mode;
  - "ext" for `SPH_SORT_EXTENDED` mode.

Note: the `SPH*` modes are specific to the `sphinx` legacy interface. SQL and JSON interfaces will log, in most cases, `ext2` as `match-mode` and `ext` and `rel` as `sort-mode`.

<!-- intro -->
Query log example:
<!-- request Example -->
```ini
[Fri Jun 29 21:17:58 2021] 0.004 sec [all/0/rel 35254 (0,20)] [lj] [ios=6 kb=111.1 ms=0.5] test
[Fri Jun 29 21:17:58 2021] 0.004 sec [all/0/rel 35254 (0,20)] [lj] [ios=6 kb=111.1 ms=0.5 cpums=0.3] test
[Sun Apr 28 15:09:38.712 2024] 0.000 sec 0.000 sec [ext2/0/ext 0 (0,20)] [test] test
[Sun Apr 28 15:09:44.974 2024] 0.000 sec 0.000 sec [ext2/0/ext 0 (0,20) @channel_id] [test] test
[Sun Apr 28 15:24:32.975 2024] 0.000 sec 0.000 sec [ext2/0/ext 0 (0,30)] [test] {     "table" : "test",     "query":     {         "match":         {             "*" : "test"         }     },     "_source": ["f"],     "limit": 30 }
```

<!-- end -->

## Logging only slow queries

<!-- example query_log_min_msec -->
By default, all queries are logged. If you want to log only queries with execution times exceeding a specified limit, the `query_log_min_msec` directive can be used.

The expected unit of measurement is milliseconds, but time suffix expressions can also be used.

<!-- intro -->
`query_log_min_msec` example:
<!-- request Config -->
```ini
searchd {
...
    query_log = /var/log/query.log
    query_log_min_msec  = 1000
    # query_log_min_msec  = 1s
...
}
```

<!-- end -->

## Log file permission mode

<!-- Example query_log_mode -->
By default, the searchd and query log files are created with permission `600`, so only the user under which Manticore is running and `root` can read the log files. The `query_log_mode` option allows setting a different permission. This can be helpful for allowing other users to read the log files (for example, monitoring solutions running on non-root users).

<!-- intro -->
`query_log_mode` example:
<!-- request Config -->
```ini
searchd {
...
    query_log = /var/log/query.log
    query_log_mode = 666
...
}
```
<!-- end -->
<!-- proofread -->
