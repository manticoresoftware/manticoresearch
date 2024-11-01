# 子查询

Manticore 支持通过 SQL 进行 `SELECT` 子查询，格式如下：

```sql
SELECT * FROM (SELECT ... ORDER BY cond1 LIMIT X) ORDER BY cond2 LIMIT Y
```

外部查询只允许 `ORDER BY` 和 `LIMIT` 子句。子查询目前有两个主要的应用场景：

1. 当您有一个查询中包含两个排名 UDF，一个非常快，另一个较慢，并且执行全文搜索时匹配到大量结果集。没有子查询时，查询可能如下：

    ```sql
    SELECT id,slow_rank() as slow,fast_rank() as fast FROM index
        WHERE MATCH(‘some common query terms’) ORDER BY fast DESC, slow DESC LIMIT 20
        OPTION max_matches=1000;
    ```

    使用子查询后，可以重写为：

    ```sql
    SELECT * FROM
        (SELECT id,slow_rank() as slow,fast_rank() as fast FROM index WHERE
            MATCH(‘some common query terms’)
            ORDER BY fast DESC LIMIT 100 OPTION max_matches=1000)
    ORDER BY slow DESC LIMIT 20;
    ```

    在初始查询中，`slow_rank()` UDF 需要为整个匹配结果集计算。而通过 `SELECT` 子查询，`fast_rank()` 仅为整个匹配集计算，而 `slow_rank()` 只为限制后的部分数据集计算。

2. 第二种情况适用于从分布式表返回的大型结果集。

    对于这样的查询：

    ```sql
    SELECT * FROM my_dist_index WHERE some_conditions LIMIT 50000;
    ```

    如果您有 20 个节点，每个节点可以返回最多 50K 条记录，结果是 20 x 50K = 1M 条记录。然而，由于主节点只返回其中的 50K 条（在 1M 条记录中选取），因此让节点只发送前 10K 条记录可能就足够了。使用子查询，您可以将查询重写为：

    ```sql
    SELECT * FROM
         (SELECT * FROM my_dist_index WHERE some_conditions LIMIT 10000)
     ORDER by some_attr LIMIT 50000;
    ```

    在这种情况下，节点只接收到内部查询并执行它。这意味着主节点将只接收到 20x10K=200K 条记录。主节点会将接收到的所有记录按外部查询条件重新排序，并返回最佳的 50K 条记录。子查询有助于减少主节点和其他节点之间的数据传输量，并减少主节点的计算时间（因为它只需处理 200K 条记录，而非 1M 条记录）。
<!-- proofread -->
