# Ranker 插件

Ranker 插件允许你实现一个自定义的 ranker，它接收文档中所有匹配关键词的出现位置，并计算一个 `WEIGHT()` 值。它们可以通过以下方式调用：

```sql
SELECT id, attr1 FROM test WHERE match('hello') OPTION ranker=myranker('option1=1');
```
调用流程如下：

1. `XXX_init()` 在每个查询每个表的开始时调用一次。多个查询范围的选项通过 `SPH_RANKER_INIT` 结构传递给它，包括用户选项字符串（例如，上例中的 "option1=1"）。
2. 对于每个匹配的文档，`XXX_update()` 会被多次调用，每次传入一个匹配的关键词出现位置作为参数，即一个 `SPH_RANKER_HIT` 结构。保证每个文档内的出现序列按 `hit->hit_pos` 值的升序传递。
3. `XXX_finalize()` 在不再有关键词出现时，为每个匹配的文档调用一次。它必须返回 `WEIGHT()` 值。此函数是唯一必需实现的函数。
4. `XXX_deinit()` 在每个查询结束时调用一次。


<!-- proofread -->

