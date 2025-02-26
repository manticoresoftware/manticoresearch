# 分段查询

主查询需要获取所有文档时，可能会对整个表施加读取锁，导致并发查询（例如对 MyISAM 表的 INSERT 操作）暂停，消耗大量内存用于结果集等。为避免此类问题，Manticore 支持所谓的*分段查询*。通过分段查询，Manticore 首先从表中获取最小和最大文档 ID，然后将不同的 ID 区间替换到主查询文本中，并运行修改后的查询以获取下一批文档。以下是一个示例。

分段查询使用示例：

```ini
sql_query_range = SELECT MIN(id),MAX(id) FROM documents
sql_range_step = 1000
sql_query = SELECT * FROM documents WHERE id>=$start AND id<=$end
```

如果表中包含的文档 ID 从 1 到 2345，那么 `sql_query` 将被运行三次：

1. `$start` 替换为 1，`$end` 替换为 1000；
2. `$start` 替换为 1001，`$end` 替换为 2000；
3. `$start` 替换为 2001，`$end` 替换为 2345。

显然，对于一个 2000 行的表来说，这并不会产生太大差异，但在处理包含 1000 万行的表时，分段查询可能会有所帮助。

### sql_query_range

定义范围查询。此选项中指定的查询必须获取将用作范围边界的最小和最大文档 ID。查询必须返回恰好两个整数字段，第一个为最小 ID，第二个为最大 ID；字段名将被忽略。启用此功能后，`sql_query` 必须包含 `$start` 和 `$end` 宏。请注意，`$start` 到 `$end` 指定的区间不会重叠，因此您不应从查询中移除与 `$start` 或 `$end` 完全相等的文档 ID。

### sql_range_step

此指令定义范围查询的步长。默认值为 1024。

### sql_ranged_throttle

此指令可用于限制分段查询的速率。默认情况下，没有速率限制。`sql_ranged_throttle` 的值应以毫秒为单位指定。

限流在索引器对数据库服务器施加过大负载时非常有用。它使索引器在每次分段查询步骤后暂停指定的时间。此暂停是无条件的，并在获取查询之前执行。

```ini
sql_ranged_throttle = 1000 # 在每个查询步骤前暂停 1 秒
```
<!-- proofread -->