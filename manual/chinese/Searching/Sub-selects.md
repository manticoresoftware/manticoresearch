# 子查询

Manticore 支持通过 SQL 以以下格式进行 SELECT 子查询：

```sql
SELECT * FROM (SELECT ... ORDER BY cond1 LIMIT X) ORDER BY cond2 LIMIT Y
```

外层 select 只允许 `ORDER BY` 和 `LIMIT` 子句。子查询目前有两个使用场景：

1. 当你有一个包含两个排序 UDF 的查询，一个非常快，另一个很慢，并且执行全文搜索时匹配结果集很大。没有子查询，查询看起来像：

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

    在初始查询中，`slow_rank()` UDF 会对整个匹配结果集进行计算。使用 SELECT 子查询时，只有 `fast_rank()` 会对整个匹配结果集计算，而 `slow_rank()` 只对有限的集合计算。

2. 第二种情况适用于来自分布式表的大型结果集。

    对于此查询：

    ```sql
    SELECT * FROM my_dist_index WHERE some_conditions LIMIT 50000;
    ```

    如果你有 20 个节点，每个节点最多可以向主节点发送 50K 条记录，结果是 20 x 50K = 1M 条记录。然而，由于主节点只返回 50K（从 1M 中），节点只发送前 10K 条记录可能就足够了。使用子查询，你可以将查询重写为：

    ```sql
    SELECT * FROM
         (SELECT * FROM my_dist_index WHERE some_conditions LIMIT 10000)
     ORDER by some_attr LIMIT 50000;
    ```

    在这种情况下，节点只接收内部查询并执行它。这意味着主节点只会收到 20x10K=200K 条记录。主节点将接收的所有记录重新排序（根据外层子句），并返回最好的 50K 条记录。子查询有助于减少主节点和节点之间的流量，同时减少主节点的计算时间（因为它只处理 200K 条记录，而不是 1M 条记录）。
<!-- proofread -->

