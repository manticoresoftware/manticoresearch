# 查询日志

<!-- example query_logging -->
查询日志可以通过在配置文件的searchd部分设置[query_log](../Server_settings/Searchd.md#query_log)指令来启用。

查询也可以通过设置`syslog`而不是文件路径发送到syslog。在这种情况下，所有搜索查询将使用`LOG_INFO`优先级发送到syslog守护进程，前面带有`[query]`而不是时间戳。仅支持syslog的日志格式为`plain`。

<!-- intro -->
`query_log`示例：
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


## 日志格式

支持两种查询日志格式：
* `sphinxql`（默认）：以SQL格式记录。这也提供了一种方便的方式来重放记录的查询。
* `plain`：以简单文本格式记录全文查询。如果大多数查询主要是全文查询，或者不需要记录非全文部分，如属性过滤、排序或分组，那么推荐使用此格式。以`plain`格式记录的查询无法重放。注意，通过[Buddy](../Installation/Manticore_Buddy.md)处理的查询在这种模式下不会被记录。

要切换格式，可以使用searchd设置[query_log_format](../Server_settings/Searchd.md#query_log_format)。

### SQL日志格式

<!-- example sphixql_log -->
SQL日志格式是默认设置。在这种模式下，Manticore记录所有成功的和不成功的select查询。以SQL或二进制API发送的请求以SQL格式记录，但JSON查询按原样记录。这种类型的记录仅适用于纯文本日志文件，并不支持`syslog`服务进行记录。

<!-- intro -->
`query_log_format`示例：
<!-- request Config -->
```ini
query_log_format = sphinxql # default
```

<!-- end -->

<!-- example sphixql_log2 -->
与[plain格式](../Logging/Query_logging.md#Plain-log-format)相比，Manticore SQL日志格式的特性包括：
* 尽可能记录完整的语句数据。
* 记录错误和警告。
* 查询日志可以重放。
* 记录额外的性能计数器（当前为每个代理分布式查询时间）。
* 每条日志条目都是一个有效的Manticore SQL/JSON语句，可以重建完整的请求，除非记录的请求太大，需要为了性能原因进行缩短。
* JSON请求作为注释记录，跳过元素之间的多余空格。
* 记录额外的消息、计数器等作为注释。

<!-- intro -->
`sphinxql`日志条目示例：
<!-- request Example -->
```sql
/* Sun Apr 28 12:38:02.808 2024 conn 2 (127.0.0.1:53228) real 0.000 wall 0.000 found 0 */ SELECT * FROM test WHERE MATCH('test') OPTION ranker=proximity;
/* Sun Apr 28 12:38:05.585 2024 conn 2 (127.0.0.1:53228) real 0.001 wall 0.001 found 0 */ SELECT * FROM test WHERE MATCH('test') GROUP BY channel_id OPTION ranker=proximity;
/* Sun Apr 28 12:40:57.366 2024 conn 4 (127.0.0.1:53256) real 0.000 wall 0.000 found 0 */  /*{ "index" : "test", "query": { "match": { "*" : "test" } }, "_source": ["f"], "limit": 30 } */
```
<!-- end -->

### Plain日志格式

<!-- example plain_log -->
使用`plain`日志格式，Manticore以简单文本格式记录所有成功执行的搜索查询。查询的非全文部分不会被记录。JSON查询记录为单行条目。通过[Buddy](../Installation/Manticore_Buddy.md)处理的查询不会被记录。

<!-- intro -->
`query_log_format`示例：
<!-- request Config -->
```ini
query_log_format = plain
```
<!-- end -->

<!-- example plain_log2 -->
日志格式如下：

```
[query-date] real-time wall-time [match-mode/filters-count/sort-mode total-matches (offset,limit) @groupby-attr] [table-name] {perf-stats} query
```

其中：
* `real-time` 是从查询开始到结束的端到端时间。在 SphinxQL 日志中，它对应 `real` 字段。
* `wall-time` 是 Manticore 的内部查询墙时间指标。在 SphinxQL 日志中，它对应 `wall` 字段，且此值也用于 `query_log_min_msec`。对于分布式和多源查询，`wall-time` 可能与 `real-time` 不同。
* `perf-stats`包括当Manticore以`--cpustats`（或通过`SET GLOBAL cpustats=1`启用）和/或`--iostats`（或通过`SET GLOBAL iostats=1`启用）启动时的CPU/IO统计信息：
  - `ios`是执行的文件I/O操作数；
  - `kb`是从表文件读取的数据量（以千字节为单位）；
  - `ms`是I/O操作所花费的时间。
  - `cpums`是花费在查询CPU处理上的时间（以毫秒为单位）。
* `match-mode`可以有以下值之一：
  - "all"对应`SPH_MATCH_ALL`模式；
  - "any"对应`SPH_MATCH_ANY`模式；
  - "phr"对应`SPH_MATCH_PHRASE`模式；
  - "bool"对应`SPH_MATCH_BOOLEAN`模式；
  - "ext"对应`SPH_MATCH_EXTENDED`模式；
  - "ext2"对应`SPH_MATCH_EXTENDED2`模式；
  - "scan"如果使用了全扫描模式，无论是通过`SPH_MATCH_FULLSCAN`指定的，还是查询为空。
* `sort-mode`可以有以下值之一：
  - "rel"对应`SPH_SORT_RELEVANCE`模式；
  - "attr-"对应`SPH_SORT_ATTR_DESC`模式；
  - "attr+"对应`SPH_SORT_ATTR_ASC`模式；
  - "tsegs"对应`SPH_SORT_TIME_SEGMENTS`模式；
  - "ext"对应`SPH_SORT_EXTENDED`模式。

注意：`SPH*`模式是`sphinx`遗留接口特有的。SQL和JSON接口通常会记录`ext2`作为`match-mode`，`ext`和`rel`作为`sort-mode`。

对于分布式查询，请使用 `SHOW STATUS` 计数器 `dist_wall`、`dist_local` 和 `dist_wait` 来分析时间消耗。这些计数器是互补的，不能直接替代查询日志中的 `real`/`wall`。

<!-- intro -->
查询日志示例：
<!-- request Example -->
```ini
[Fri Jun 29 21:17:58 2021] 0.004 sec [all/0/rel 35254 (0,20)] [lj] [ios=6 kb=111.1 ms=0.5] test
[Fri Jun 29 21:17:58 2021] 0.004 sec [all/0/rel 35254 (0,20)] [lj] [ios=6 kb=111.1 ms=0.5 cpums=0.3] test
[Sun Apr 28 15:09:38.712 2024] 0.000 sec 0.000 sec [ext2/0/ext 0 (0,20)] [test] test
[Sun Apr 28 15:09:44.974 2024] 0.000 sec 0.000 sec [ext2/0/ext 0 (0,20) @channel_id] [test] test
[Sun Apr 28 15:24:32.975 2024] 0.000 sec 0.000 sec [ext2/0/ext 0 (0,30)] [test] {     "table" : "test",     "query":     {         "match":         {             "*" : "test"         }     },     "_source": ["f"],     "limit": 30 }
```

<!-- end -->

## 仅记录慢查询

<!-- example query_log_min_msec -->
默认情况下，所有查询都会被记录。如果您只想记录执行时间超过指定限制的查询，可以使用`query_log_min_msec`指令。

预期的单位是毫秒，但也可以使用时间后缀表达式。

<!-- intro -->
`query_log_min_msec`示例：
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

## 日志文件权限模式

<!-- Example query_log_mode -->
默认情况下，searchd和查询日志文件的权限为`600`，因此只有Manticore运行的用户和`root`才能读取日志文件。`query_log_mode`选项允许设置不同的权限。这对于允许其他用户读取日志文件（例如，运行在非`root`用户上的监控解决方案）是有帮助的。

<!-- intro -->
`query_log_mode`示例：
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

