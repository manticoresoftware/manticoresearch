# 子查询

Manticore支持以下格式的SQL子查询：

```sql
SELECT * FROM (SELECT ... ORDER BY cond1 LIMIT X) ORDER BY cond2 LIMIT Y
```

外层查询只允许使用 `ORDER BY` 和 `LIMIT` 子句。子查询目前有两种使用场景：

1. 当你有一个包含两个排序UDF的查询，一个非常快，另一个很慢，并且执行全文搜索且匹配结果集较大时。如果没有子查询，查询如下：

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

    在初始查询中，`slow_rank()` UDF针对整个匹配结果集计算。使用SELECT子查询时，只有`fast_rank()`针对整个匹配结果集计算，而`slow_rank()`只针对有限的结果集计算。

2. 第二种情况对于来自分布式表的大结果集很有用。

    对于此查询：

    ```sql
    SELECT * FROM my_dist_index WHERE some_conditions LIMIT 50000;
    ```

    如果你有20个节点，每个节点可返回给主节点最多50K条记录，总计20 x 50K = 100万条记录。 但是由于主节点最终只返回50K（从1M中筛选），节点只发送前10K条记录可能已经足够。利用子查询，可以将查询重写为：

    ```sql
    SELECT * FROM
         (SELECT * FROM my_dist_index WHERE some_conditions LIMIT 10000)
     ORDER by some_attr LIMIT 50000;
    ```

    在此情况下，节点仅接收并执行内部查询。这意味着主节点将只接收20x10K=20万条记录。主节点将对收到的所有记录进行外层子句中的重新排序，并返回最佳的50K记录。子查询有助于减少主节点和节点之间的通信流量，同时减少主节点的计算时间（因为它只处理20万而不是100万条记录）。
<!-- proofread -->

