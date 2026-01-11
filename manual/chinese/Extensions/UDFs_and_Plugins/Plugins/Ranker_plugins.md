# 排序插件

排序插件允许您实现一个自定义排序器，该排序器接收文档中匹配的所有关键词的出现情况，并计算一个 `WEIGHT()` 值。它们可以按照以下方式调用：

```sql
SELECT id, attr1 FROM test WHERE match('hello') OPTION ranker=myranker('option1=1');
```
调用流程如下：

1. `XXX_init()` 在每次查询每次表时仅调用一次，在最开始时被调用。通过一个 `SPH_RANKER_INIT` 结构传递一些查询范围内的选项，包括用户选项字符串（例如，上面示例中的 "option1=1"）。
2. `XXX_update()` 对于每个匹配的文档，被多次调用，每次调用都提供一个匹配关键词的出现情况，一个 `SPH_RANKER_HIT` 结构。每个文档内的出现情况保证按照 `hit->hit_pos` 值的升序传递。
3. `XXX_finalize()` 在没有更多关键词出现情况时，对每个匹配的文档调用一次，必须返回 `WEIGHT()` 值。这是唯一必须实现的函数。
4. `XXX_deinit()` 在每次查询结束时调用一次。


<!-- proofread -->

