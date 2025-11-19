# Ranker 插件

Ranker 插件允许您实现一个自定义的 ranker，该 ranker 接收文档中匹配的所有关键词出现，并计算一个 `WEIGHT()` 值。它们可以按如下方式调用：

```sql
SELECT id, attr1 FROM test WHERE match('hello') OPTION ranker=myranker('option1=1');
```
调用流程如下：

1. `XXX_init()` 在每个查询每个表开始时调用一次。通过 `SPH_RANKER_INIT` 结构传递多个查询范围的选项，包括用户选项字符串（例如，上面示例中的 "option1=1"）。
2. `XXX_update()` 对每个匹配的文档调用多次，每次传入一个匹配的关键词出现，作为其参数，即 `SPH_RANKER_HIT` 结构。保证每个文档内的出现按 `hit->hit_pos` 值升序传递。
3. `XXX_finalize()` 在没有更多关键词出现时，对每个匹配的文档调用一次。它必须返回 `WEIGHT()` 值。此函数是唯一必需的。
4. `XXX_deinit()` 在每个查询结束时调用一次。


<!-- proofread -->

