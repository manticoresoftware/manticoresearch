# 范围查询


主要查询，需要获取所有文档时，可能会对整个表施加读锁并阻塞并发查询（例如，MyISAM 表的 INSERT 操作），浪费大量内存用于结果集等。为了避免这种情况，Manticore 支持所谓的 *范围查询*。使用范围查询时，Manticore 首先从表中获取最小和最大文档 ID，然后将不同的 ID 区间插入到主查询文本中，并运行修改后的查询以获取另一批文档。这里有一个例子。

范围查询使用示例：

```ini
sql_query_range = SELECT MIN(id),MAX(id) FROM documents
sql_range_step = 1000
sql_query = SELECT * FROM documents WHERE id>=$start AND id<=$end
```

如果表包含从 1 到，比如说，2345 的文档 ID，那么 sql_query 将会运行三次：

1. 用 `$start` 替换为 1 并用 `$end` 替换为 1000；
2. 用 `$start` 替换为 1001 并用 `$end` 替换为 2000；
3. 用 `$start` 替换为 2001 并用 `$end` 替换为 2345。

显然，对于 2000 行的表来说这不是很大的区别，但当涉及到索引 10 百万行的表时，范围查询可能会有所帮助。

### sql_query_range

定义范围查询。此选项指定的查询必须获取将用作范围边界的最小和最大文档 ID。它必须返回恰好两个整数字段，最小 ID 先，最大 ID 后；字段名被忽略。启用时，`sql_query` 必须包含 $start 和 $end 宏。请注意，由 $start..$end 指定的区间不会重叠，因此在您的查询中不应删除等于 $start 或 $end 的文档 ID。

### sql_range_step

此指令定义了范围查询步长。默认值是 1024。

### sql_ranged_throttle

此指令可以用来限制范围查询的速度。默认情况下，没有限制。sql_ranged_throttle 的值应以毫秒为单位指定。

限制范围查询速度可能在索引器对数据库服务器施加过多负载时很有用。这会导致索引器在每个范围查询步骤后睡眠一段时间。这种睡眠是无条件的，在执行获取查询之前进行。

```ini
sql_ranged_throttle = 1000 # sleep for 1 sec before each query step
```
<!-- proofread -->

