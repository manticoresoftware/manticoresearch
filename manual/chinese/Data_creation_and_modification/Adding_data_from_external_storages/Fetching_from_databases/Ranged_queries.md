# 范围查询 


主要查询需要提取所有文档时，可能会对整个表施加读取锁，并阻塞并发查询（例如，对 MyISAM 表的 INSERT），浪费大量内存用于结果集等。为避免这种情况，Manticore 支持所谓的 *范围查询*。通过范围查询，Manticore 首先从表中获取最小和最大文档 ID，然后将不同的 ID 区间替换到主要查询文本中，并运行修改后的查询以获取另一批文档。以下是一个示例。

范围查询使用示例：

```ini
sql_query_range = SELECT MIN(id),MAX(id) FROM documents
sql_range_step = 1000
sql_query = SELECT * FROM documents WHERE id>=$start AND id<=$end
```

如果表包含从 1 到 2345 的文档 ID，那么 sql_query 将运行三次：

1.  将 `$start` 替换为 1，`$end` 替换为 1000；
2.  将 `$start` 替换为 1001，`$end` 替换为 2000；
3.  将 `$start` 替换为 2001，`$end` 替换为 2345。

显然，对于 2000 行的表而言差别不大，但在对 1000 万行的表进行索引时，范围查询可能会有所帮助。

### sql_query_range

定义范围查询。此选项中指定的查询必须提取将用作范围边界的最小和最大文档 ID。它必须返回正好两个整数字段，最小 ID 先，最大 ID 后；字段名称被忽略。当启用时，`sql_query` 将要求包含 $start 和 $end 宏。请注意，$start..$end 指定的区间不会重叠，因此您不应从查询中删除恰好等于 $start 或 $end 的文档 ID。
 
### sql_range_step

此指令定义范围查询的步长。默认值为 1024。

### sql_ranged_throttle

此指令可用于限制范围查询。默认情况下，没有限制。sql_ranged_throttle 的值应以毫秒为单位指定。

限制在索引器对数据库服务器施加过多负载时可能会很有用。它使索引器在每个范围查询步骤之前在给定的时间内休眠。此休眠是无条件的，并在提取查询之前执行。

```ini
sql_ranged_throttle = 1000 # 在每个查询步骤之前休眠 1 秒
```
<!-- proofread -->
