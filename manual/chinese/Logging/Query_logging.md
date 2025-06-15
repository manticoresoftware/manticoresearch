# 查询日志

<!-- example query_logging -->
可以通过在配置文件的 searchd 部分设置 [query_log](../Server_settings/Searchd.md#query_log) 指令来启用查询日志。

查询也可以通过设置为 `syslog`（而不是文件路径）发送到系统日志。在这种情况下，所有搜索查询将以 `LOG_INFO` 优先级发送到 syslog 守护进程，前缀为 `[query]`，而不是时间戳。syslog 只支持 `plain` 日志格式。

<!-- intro -->
`query_log` 示例：
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
* `sphinxql`（默认）：以 SQL 格式记录日志。它还提供了一种简便的方法来重放记录的查询。
* `plain`：以简单文本格式记录全文查询。如果您的大部分查询主要是全文查询，或者您不需要记录非全文组件（例如基于属性的过滤、排序或分组），推荐使用此格式。用 `plain` 格式记录的查询无法重放。注意，通过 [Buddy](../Installation/Manticore_Buddy.md) 处理的查询在此模式下不会被记录。

您可以使用 searchd 设置中的 [query_log_format](../Server_settings/Searchd.md#query_log_format) 切换格式。

### SQL 日志格式

<!-- example sphixql_log -->
SQL 日志格式是默认设置。在此模式下，Manticore 会记录所有成功和失败的 select 查询。通过 SQL 或二进制 API 发送的请求会以 SQL 格式记录，而 JSON 查询则原样记录。此类型的日志仅适用于普通日志文件，不支持使用 'syslog' 服务进行日志记录。

<!-- intro -->
`query_log_format` 示例：
<!-- request Config -->
```ini
query_log_format = sphinxql # default
```

<!-- end -->

<!-- example sphixql_log2 -->
Manticore SQL 日志格式相较于[plain 格式](../Logging/Query_logging.md#Plain-log-format)的特性包括：
* 在可能的情况下记录完整语句数据。
* 记录错误和警告。
* 查询日志可以被重放。
* 记录额外的性能计数器（目前是每个代理的分布式查询时间）。
* 每条日志均为有效的 Manticore SQL/JSON 语句，可重建完整请求，除非为了性能原因日志记录的请求过大而被缩短。
* JSON 请求以及额外的信息、计数器等作为注释记录。

<!-- intro -->
`sphinxql` 日志条目示例：
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

### Plain 日志格式

<!-- example plain_log -->
在 `plain` 日志格式下，Manticore 以简单文本格式记录所有成功执行的搜索查询。查询中的非全文部分不被记录。JSON 查询以单行条目记录。通过 [Buddy](../Installation/Manticore_Buddy.md) 处理的查询不被记录。

<!-- intro -->
`query_log_format` 示例：
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
* `real-time` 是查询从开始到结束的时间。
* `wall-time` 类似于 real-time，但不包括等待代理和合并代理结果集的时间。
* `perf-stats` 包含当 Manticore 以 `--cpustats`（或者启用 `SET GLOBAL cpustats=1`）和/或 `--iostats`（或者启用 `SET GLOBAL iostats=1`）启动时的 CPU/IO 统计信息：
  - `ios` 是完成的文件 I/O 操作次数；
  - `kb` 是从表文件读取的数据量，单位为千字节；
  - `ms` 是在 I/O 操作上花费的时间；
  - `cpums` 是 CPU 用于处理查询的时间，单位为毫秒。
* `match-mode` 可以取下列值之一：
  - "all" 表示 `SPH_MATCH_ALL` 模式；
  - "any" 表示 `SPH_MATCH_ANY` 模式；
  - "phr" 表示 `SPH_MATCH_PHRASE` 模式；
  - "bool" 表示 `SPH_MATCH_BOOLEAN` 模式；
  - "ext" 表示 `SPH_MATCH_EXTENDED` 模式；
  - "ext2" 表示 `SPH_MATCH_EXTENDED2` 模式；
  - "scan" 表示使用了完整扫描模式，可能由 `SPH_MATCH_FULLSCAN` 指定或查询为空。
* `sort-mode` 可以取下列值之一：
  - "rel" 表示 `SPH_SORT_RELEVANCE` 模式；
  - "attr-" 表示 `SPH_SORT_ATTR_DESC` 模式；
  - "attr+" 表示 `SPH_SORT_ATTR_ASC` 模式；
  - "tsegs" 表示 `SPH_SORT_TIME_SEGMENTS` 模式；
  - "ext" 表示 `SPH_SORT_EXTENDED` 模式。

注意：`SPH*` 模式是 `sphinx` 旧接口特定的。SQL 和 JSON 接口大多数情况下会将 `ext2` 作为 `match-mode`，而 `ext` 和 `rel` 作为 `sort-mode` 记录。

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
默认情况下，所有查询都会被记录。如果只想记录执行时间超过指定限制的查询，可以使用 `query_log_min_msec` 指令。

其预期的单位是毫秒，但也可以使用时间后缀表达式。

<!-- intro -->
`query_log_min_msec` 示例：
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
默认情况下，searchd 和查询日志文件的权限为 `600`，因此只有运行 Manticore 的用户和 `root` 用户可以读取日志文件。`query_log_mode` 选项允许设置不同的权限。这有助于允许其他用户读取日志文件（例如，运行在非 root 用户上的监控解决方案）。

<!-- intro -->
`query_log_mode` 示例：
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

