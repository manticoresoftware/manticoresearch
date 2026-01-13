# 子查询

Manticore 通过以下格式支持 SQL 中的 SELECT 子查询：

```sql
SELECT * FROM (SELECT ... ORDER BY cond1 LIMIT X) ORDER BY cond2 LIMIT Y
```

外层 select 只允许包含 `ORDER BY` 和 `LIMIT` 子句。子查询目前有两个使用场景：

1. 当你有一个查询包含两个排序 UDF，一个非常快，另一个较慢，并且执行全文搜索时匹配结果集很大。没有子查询，查询看起来像这样：

    ```sql
    SELECT id,slow_rank() as slow,fast_rank() as fast FROM index
        WHERE MATCH(‘some common query terms’) ORDER BY fast DESC, slow DESC LIMIT 20
        OPTION max_matches=1000;
    ```

    使用子查询，查询可以重写为：

    ```sql
    SELECT * FROM
        (SELECT id,slow_rank() as slow,fast_rank() as fast FROM index WHERE
            MATCH(‘some common query terms’)
            ORDER BY fast DESC LIMIT 100 OPTION max_matches=1000)
    ORDER BY slow DESC LIMIT 20;
    ```

    在初始查询中，`slow_rank()` UDF 会对整个匹配结果集计算。使用 SELECT 子查询，仅对整个匹配结果集计算 `fast_rank()`，而只对有限集合计算 `slow_rank()`。

2. 第二种情况适用于来自分布式表的大型结果集。

    对于此查询：

    ```sql
    SELECT * FROM my_dist_index WHERE some_conditions LIMIT 50000;
    ```

    如果你有 20 个节点，每个节点最多可以返回 50K 条记录给主节点，结果是 20 x 50K = 100万条记录。然而，由于主节点只返回 50K 条（从 100万条中），节点只发送前 10K 条记录可能就足够了。使用子查询，你可以将查询重写为：

    ```sql
    SELECT * FROM
         (SELECT * FROM my_dist_index WHERE some_conditions LIMIT 10000)
     ORDER by some_attr LIMIT 50000;
    ```

    在这种情况下，节点仅接收内部查询并执行它。这意味着主节点将仅收到 20x10K=20万 条记录。主节点会对接收到的所有记录按外层子句重新排序，并返回最佳的 50K 条记录。子查询有助于减少主节点和节点之间的流量，同时减少主节点的计算时间（因为它只处理 20万 条记录而非 100万 条）。 
<!-- proofread -->

