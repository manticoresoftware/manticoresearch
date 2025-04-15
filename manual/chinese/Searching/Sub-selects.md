# 子查询

Manticore 支持通过 SQL 以以下格式进行 SELECT 子查询：

```sql
SELECT * FROM (SELECT ... ORDER BY cond1 LIMIT X) ORDER BY cond2 LIMIT Y
```

外部选择仅允许 `ORDER BY` 和 `LIMIT` 子句。子选择查询当前有两个用例：

1. 当你有一个包含两个排名 UDF 的查询，一个非常快而另一个较慢，并且执行一个大型匹配结果集的全文搜索时。如果没有子选择，查询将如下所示：

    ```sql
    SELECT id,slow_rank() as slow,fast_rank() as fast FROM index
        WHERE MATCH(‘一些常见查询词’) ORDER BY fast DESC, slow DESC LIMIT 20
        OPTION max_matches=1000;
    ```

    使用子选择时，查询可以重写为：

    ```sql
    SELECT * FROM
        (SELECT id,slow_rank() as slow,fast_rank() as fast FROM index WHERE
            MATCH(‘一些常见查询词’)
            ORDER BY fast DESC LIMIT 100 OPTION max_matches=1000)
    ORDER BY slow DESC LIMIT 20;
    ```

    在初始查询中，`slow_rank()` UDF 是为整个匹配结果集计算的。使用 SELECT 子查询时，仅为整个匹配结果集计算 `fast_rank()`，而 `slow_rank()` 则是为有限集计算的。

2. 第二种情况对于来自分布式表的大型结果集非常有用。

    对于此查询：

    ```sql
    SELECT * FROM my_dist_index WHERE some_conditions LIMIT 50000;
    ```

    如果你有 20 个节点，每个节点最多可以向主节点发送 50K 条记录，结果为 20 x 50K = 1M 条记录。然而，由于主节点只返回 50K（从 1M 中），因此节点发送仅前 10K 条记录可能是足够的。通过子选择，你可以将查询重写为：

    ```sql
    SELECT * FROM
         (SELECT * FROM my_dist_index WHERE some_conditions LIMIT 10000)
     ORDER by some_attr LIMIT 50000;
    ```

    在这种情况下，节点只接收内部查询并执行它。这意味着主节点只会收到 20x10K=200K 条记录。主节点会将所有收到的记录进行重新排序，通过外部子句，并返回最佳的 50K 条记录。子选择有助于减少主节点和节点之间的流量，以及减少主节点的计算时间（因为它仅处理 200K 而不是 1M 条记录）。
<!-- proofread -->
