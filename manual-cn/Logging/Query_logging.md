# 查询日志

<!-- example query_logging -->

可以通过在配置文件的 `searchd` 部分设置 [query_log](../Server_settings/Searchd.md#query_log) 指令来启用查询日志。

也可以通过将 `syslog` 设置为日志路径，将查询日志发送到系统日志（syslog）。在这种情况下，所有搜索查询将以 `LOG_INFO` 优先级发送到 syslog 守护进程，并且日志前缀为 `[query]`，而不是时间戳。对于 syslog，只有 `plain` 日志格式受支持。

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

- `sphinxql`（默认）：以 SQL 格式记录日志，提供了一种轻松重放已记录查询的方式。
- `plain`：以简单文本格式记录全文查询。如果大多数查询主要是全文查询，或者不关心查询中的非全文组件（如按属性过滤、排序、分组等），建议使用此格式。以 `plain` 格式记录的查询无法重放。

您可以使用 `searchd` 设置 [query_log_format](../Server_settings/Searchd.md#query_log_format) 在这两种格式之间切换。

### SQL 日志格式

<!-- example sphixql_log -->
SQL 日志格式是默认设置。在此模式下，Manticore 会记录所有成功和失败的 `select` 查询。以 SQL 或二进制 API 发送的请求会以 SQL 格式记录，但 JSON 查询则按原样记录。此类日志记录仅适用于普通日志文件，不支持将日志发送到 `syslog` 服务。

<!-- intro -->
`query_log_format` 示例：
<!-- request Config -->

```ini
query_log_format = sphinxql # default
```

<!-- end -->

<!-- example sphixql_log2 -->

Manticore SQL 日志格式的特点（相较于 [plain 格式](../Logging/Query_logging.md#Plain 日志格式)）包括：

- 尽可能记录完整的语句数据。
- 错误和警告都会记录。
- 查询日志可以被重放。
- 记录额外的性能计数器（目前是每个代理的分布式查询时间）。
- 每个日志条目都是有效的 Manticore SQL/JSON 语句，能够重建完整请求，除非记录的请求过大，为了性能需要缩短。
- JSON 请求和其他消息、计数器等会作为注释记录。

<!-- intro -->
`sphinxql` 日志条目示例：
<!-- request Example -->

```sql
/* Sun Apr 28 12:38:02.808 2024 conn 2 (127.0.0.1:53228) real 0.000 wall 0.000 found 0 */ SELECT * FROM test WHERE MATCH('test') OPTION ranker=proximity;
/* Sun Apr 28 12:38:05.585 2024 conn 2 (127.0.0.1:53228) real 0.001 wall 0.001 found 0 */ SELECT * FROM test WHERE MATCH('test') GROUP BY channel_id OPTION ranker=proximity;
/* Sun Apr 28 12:40:57.366 2024 conn 4 (127.0.0.1:53256) real 0.000 wall 0.000 found 0 */  /*{
    "index" : "test",
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
在 `plain` 日志格式下，Manticore 以简单文本格式记录所有成功执行的搜索查询。查询中的非全文部分不会被记录。JSON 查询则会被压缩为单行记录。

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

- `real-time` 是查询从开始到结束的时间。

- `wall-time` 类似于 `real-time`，但不包括等待代理结果和合并结果集的时间。

- `perf-stats`包含 CPU/IO 统计信息（当使用 `--cpustats`启动 Manticore 或通过 `SET GLOBAL cpustats=1`启用时）：

  - `ios` 是执行的文件 I/O 操作次数；
  - `kb` 是从表文件中读取的数据量（以 KB 为单位）；
  - `ms` 是花费在 I/O 操作上的时间；
  - `cpums` 是处理查询花费的 CPU 时间（毫秒）。

- `match-mode`可以是以下之一：

  - "all" 对应 `SPH_MATCH_ALL` 模式；
  - "any" 对应 `SPH_MATCH_ANY` 模式；
  - "phr" 对应 `SPH_MATCH_PHRASE` 模式；
  - "bool" 对应 `SPH_MATCH_BOOLEAN` 模式；
  - "ext" 对应 `SPH_MATCH_EXTENDED` 模式；
  - "ext2" 对应 `SPH_MATCH_EXTENDED2` 模式；
  - "scan" 表示使用了全扫描模式，或者通过 `SPH_MATCH_FULLSCAN` 指定，或者查询为空。

- `sort-mode`

   可以是以下之一：

  - "rel" 对应 `SPH_SORT_RELEVANCE` 模式；
  - "attr-" 对应 `SPH_SORT_ATTR_DESC` 模式；
  - "attr+" 对应 `SPH_SORT_ATTR_ASC` 模式；
  - "tsegs" 对应 `SPH_SORT_TIME_SEGMENTS` 模式；
  - "ext" 对应 `SPH_SORT_EXTENDED` 模式。

注意：`SPH*` 模式特定于 `sphinx` 的旧接口。在 SQL 和 JSON 接口中，大多数情况下，`match-mode` 记录为 `ext2`，`sort-mode` 记录为 `ext` 或 `rel`。

查询日志示例：

```
ini复制代码[Fri Jun 29 21:17:58 2021] 0.004 sec [all/0/rel 35254 (0,20)] [lj] [ios=6 kb=111.1 ms=0.5] test
[Fri Jun 29 21:17:58 2021] 0.004 sec [all/0/rel 35254 (0,20)] [lj] [ios=6 kb=111.1 ms=0.5 cpums=0.3] test
[Sun Apr 28 15:09:38.712 2024] 0.000 sec 0.000 sec [ext2/0/ext 0 (0,20)] [test] test
[Sun Apr 28 15:09:44.974 2024] 0.000 sec 0.000 sec [ext2/0/ext 0 (0,20) @channel_id] [test] test
[Sun Apr 28 15:24:32.975 2024] 0.000 sec 0.000 sec [ext2/0/ext 0 (0,30)] [test] {     "index" : "test",     "query":     {         "match":         {             "*" : "test"         }     },     "_source": ["f"],     "limit": 30 }
```

### 仅记录慢查询

<!-- example query_log_min_msec -->

默认情况下，所有查询都会被记录。如果您只希望记录执行时间超过指定限制的查询，可以使用 `query_log_min_msec` 指令。

单位默认为毫秒，但也可以使用时间后缀表达式。

<!-- intro -->
查询日志示例：
<!-- request Example -->

```ini
[Fri Jun 29 21:17:58 2021] 0.004 sec [all/0/rel 35254 (0,20)] [lj] [ios=6 kb=111.1 ms=0.5] test
[Fri Jun 29 21:17:58 2021] 0.004 sec [all/0/rel 35254 (0,20)] [lj] [ios=6 kb=111.1 ms=0.5 cpums=0.3] test
[Sun Apr 28 15:09:38.712 2024] 0.000 sec 0.000 sec [ext2/0/ext 0 (0,20)] [test] test
[Sun Apr 28 15:09:44.974 2024] 0.000 sec 0.000 sec [ext2/0/ext 0 (0,20) @channel_id] [test] test
[Sun Apr 28 15:24:32.975 2024] 0.000 sec 0.000 sec [ext2/0/ext 0 (0,30)] [test] {     "index" : "test",     "query":     {         "match":         {             "*" : "test"         }     },     "_source": ["f"],     "limit": 30 }
```

<!-- end -->

## 仅记录慢查询

<!-- example query_log_min_msec -->

默认情况下，所有查询都会被记录。如果您只希望记录执行时间超过指定限制的查询，可以使用 `query_log_min_msec` 指令。

单位默认为毫秒，但也可以使用时间后缀表达式。

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
默认情况下，`searchd` 和查询日志文件的权限为 `600`，因此只有运行 Manticore 的用户和 `root` 用户可以读取日志文件。`query_log_mode` 选项允许设置不同的权限，这在需要其他用户读取日志文件（如非 `root` 用户的监控解决方案）时很有帮助。

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
