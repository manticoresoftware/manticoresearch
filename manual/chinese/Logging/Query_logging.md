# 查询日志

<!-- example query_logging -->
查询日志可以通过在配置文件的 searchd 部分设置 [query_log](../Server_settings/Searchd.md#query_log) 指令来启用。

查询也可以通过设置 `syslog` 而不是文件路径发送到 syslog。在这种情况下，所有搜索查询将以 `LOG_INFO` 优先级发送到 syslog 守护进程，前缀为 `[query]` 而不是时间戳。仅支持 syslog 的 `plain` 日志格式。

<!-- intro -->
`query_log` 示例：
<!-- request Config -->
```ini
searchd {
...
    query_log = /var/log/query.log
    query_log_format = sphinxql # 默认
...
}
```
<!-- end -->


## 日志格式

支持两种查询日志格式：
* `sphinxql`（默认）：以 SQL 格式记录日志。它还提供了一种简单的方式来重放已记录的查询。
* `plain`：以简单文本格式记录全文查询。推荐使用此格式，如果您的大多数查询主要是全文，或者您不需要记录非全文组件，例如按属性过滤、排序或分组。以 `plain` 格式记录的查询无法重放。请注意，通过 [Buddy](../Installation/Manticore_Buddy.md) 处理的查询在此模式下不会被记录。

要在格式之间切换，可以使用 searchd 设置 [query_log_format](../Server_settings/Searchd.md#query_log_format)。

### SQL 日志格式

<!-- example sphixql_log -->
SQL 日志格式是默认设置。在此模式下，Manticore 记录所有成功和失败的选择查询。作为 SQL 或通过二进制 API 发送的请求以 SQL 格式记录，但 JSON 查询按原样记录。这种类型的日志仅与普通日志文件一起使用，并且不支持用于日志记录的 'syslog' 服务。

<!-- intro -->
`query_log_format` 示例：
<!-- request Config -->
```ini
query_log_format = sphinxql # 默认
```

<!-- end -->

<!-- example sphixql_log2 -->
Manticore SQL 日志格式相比于 [plain 格式](../../Logging/Query_logging.md#Plain-log-format) 的特点包括：
* 在可能的情况下记录完整的语句数据。
* 记录错误和警告。
* 查询日志可以重放。
* 记录额外的性能计数器（目前，为每个代理的分布式查询时间）。
* 每个日志条目都是有效的 Manticore SQL/JSON 语句，可重建完整请求，除非记录的请求过大，需要因性能原因缩短。
* JSON 请求和其他消息、计数器等，作为注释记录。

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

### 普通日志格式

<!-- example plain_log -->
使用 `plain` 日志格式，Manticore 以简单文本格式记录所有成功执行的搜索查询。查询的非全文部分不会被记录。JSON 查询作为单行条目记录。通过 [Buddy](../Installation/Manticore_Buddy.md) 处理的查询不会被记录。

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
[query-date] 实时墙面时间 [匹配模式/过滤器数量/排序模式 总匹配 (偏移,限制) @分组属性] [表名] {性能统计} 查询
```

其中：
* `实时` 是从查询开始到结束的时间。
* `墙面时间` 类似于实时，但排除了等待代理和合并结果集所花费的时间。
* `性能统计` 包括 CPU/IO 统计数据，当 Manticore 以 `--cpustats` 启动时（或通过 `SET GLOBAL cpustats=1` 启用）和/或 `--iostats` （或通过 `SET GLOBAL iostats=1` 启用）：
  - `ios` 是执行的文件 I/O 操作的数量；
  - `kb` 是从表文件读取的数据量（千字节）；
  - `ms` 是在 I/O 操作上花费的时间。
  - `cpums` 是在 CPU 处理查询上花费的时间（毫秒）。
* `匹配模式` 可取以下值之一：
  - "all" 表示 `SPH_MATCH_ALL` 模式；
  - "any" 表示 `SPH_MATCH_ANY` 模式；
  - "phr" 表示 `SPH_MATCH_PHRASE` 模式；
  - "bool" 表示 `SPH_MATCH_BOOLEAN` 模式；
  - "ext" 表示 `SPH_MATCH_EXTENDED` 模式；
  - "ext2" 表示 `SPH_MATCH_EXTENDED2` 模式；
  - "scan" 如果使用了完全扫描模式，或者是通过 `SPH_MATCH_FULLSCAN` 指定，或者查询为空。
* `排序模式` 可取以下值之一：
  - "rel" 表示 `SPH_SORT_RELEVANCE` 模式；
  - "attr-" 表示 `SPH_SORT_ATTR_DESC` 模式；
  - "attr+" 表示 `SPH_SORT_ATTR_ASC` 模式；
  - "tsegs" 表示 `SPH_SORT_TIME_SEGMENTS` 模式；
  - "ext" 表示 `SPH_SORT_EXTENDED` 模式。

注意：`SPH*` 模式特定于 `sphinx` 旧版接口。SQL 和 JSON 接口在大多数情况下将记录 `ext2` 作为 `匹配模式`，`ext` 和 `rel` 作为 `排序模式`。

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
默认情况下，所有查询都会被记录。如果您只想记录执行时间超过指定限制的查询，可以使用 `query_log_min_msec` 指令。

期望的计量单位是毫秒，但也可以使用时间后缀表达式。

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
默认情况下，searchd 和查询日志文件的权限设置为 `600`，因此只有运行 Manticore 的用户和 `root` 可以读取日志文件。`query_log_mode` 选项允许设置不同的权限。这对于允许其他用户读取日志文件（例如，在非根用户下运行的监控解决方案）很有帮助。

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
