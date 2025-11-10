# 范围查询


主查询需要获取所有文档，可能会对整个表施加读锁并阻塞并发查询（例如对 MyISAM 表的 INSERT），浪费大量内存用于结果集等。为避免这种情况，Manticore 支持所谓的*范围查询*。使用范围查询时，Manticore 首先从表中获取最小和最大文档 ID，然后将不同的 ID 区间替换到主查询文本中，运行修改后的查询以获取另一批文档。以下是一个示例。

范围查询使用示例：

```ini
sql_query_range = SELECT MIN(id),MAX(id) FROM documents
sql_range_step = 1000
sql_query = SELECT * FROM documents WHERE id>=$start AND id<=$end
```

如果表中包含从 1 到 2345 的文档 ID，则 sql_query 会运行三次：

1.  将 `$start` 替换为 1，`$end` 替换为 1000；
2.  将 `$start` 替换为 1001，`$end` 替换为 2000；
3.  将 `$start` 替换为 2001，`$end` 替换为 2345。

显然，对于 2000 行的表来说，这差别不大，但当涉及到索引 1000 万行的表时，范围查询可能会有所帮助。

### sql_query_range

定义范围查询。此选项中指定的查询必须获取将用作范围边界的最小和最大文档 ID。它必须返回恰好两个整数字段，先是最小 ID，后是最大 ID；字段名会被忽略。启用后，`sql_query` 需要包含 $start 和 $end 宏。注意，$start..$end 指定的区间不会重叠，因此不应从查询中移除恰好等于 $start 或 $end 的文档 ID。

### sql_range_step

此指令定义范围查询的步长。默认值为 1024。

### sql_ranged_throttle

此指令可用于限制范围查询的速度。默认情况下，没有限速。sql_ranged_throttle 的值应以毫秒为单位指定。

当索引器对数据库服务器施加过大负载时，限速非常有用。它会导致索引器在每个范围查询步长执行前无条件休眠指定时间。

```ini
sql_ranged_throttle = 1000 # sleep for 1 sec before each query step
```
<!-- proofread -->

