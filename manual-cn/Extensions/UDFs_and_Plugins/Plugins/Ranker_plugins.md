# Ranker 插件

Ranker 插件允许您实现自定义的排序器，该排序器会接收在文档中匹配到的所有关键字出现次数，并计算出 `WEIGHT()` 值。可以通过以下方式调用：

```sql
SELECT id, attr1 FROM test WHERE match('hello') OPTION ranker=myranker('option1=1');
```
调用流程如下：

1. 在每次查询的每个表开始时，会调用 `XXX_init()`，并通过 `SPH_RANKER_INIT` 结构传递多个查询级别的选项，包括用户的选项字符串（例如上述例子中的 "option1=1"）。
2. 对于每个匹配的文档，`XXX_update()` 会被多次调用，并以 `SPH_RANKER_HIT` 结构的形式传递每个匹配的关键字出现。保证每个文档内的出现按 `hit->hit_pos` 值的升序传递。
3. 当文档中没有更多的关键字出现时，会调用 `XXX_finalize()`，它必须返回 `WEIGHT()` 值。此函数是唯一必须实现的函数。
4. 在查询结束时，会调用 `XXX_deinit()`。


<!-- proofread -->