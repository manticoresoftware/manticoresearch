# 排名插件 

排名插件允许您实现自定义排名器，该排名器接收文档中匹配关键词的所有出现位置，并计算 `WEIGHT()` 值。它们可以按以下方式调用：

```sql
SELECT id, attr1 FROM test WHERE match('hello') OPTION ranker=myranker('option1=1');
```
调用工作流程如下：

1. 在查询开始时，为每个表调用一次 `XXX_init()`。通过 `SPH_RANKER_INIT` 结构传递几个查询范围的选项，包括用户选项字符串（例如上面示例中的 "option1=1"）。
2. 对于每个匹配的文档，多次调用 `XXX_update()`，其中每次关键词出现位置作为参数，是一个 `SPH_RANKER_HIT` 结构。保证在每个文档内，出现位置是按照 `hit->hit_pos` 值的升序传递。
3. 当不再有关键词出现位置时，为每个匹配的文档调用一次 `XXX_finalize()`。它必须返回 `WEIGHT()` 值。这是唯一必须实现的函数。
4. 在查询结束时，调用一次 `XXX_deinit()`。


<!-- proofread -->
