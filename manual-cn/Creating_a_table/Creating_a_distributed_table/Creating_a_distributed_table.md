# 创建分布式表

Manticore 支持创建 **分布式表**，其行为类似于普通表或实时表，但实际上是由多个子表组成的集合，用于执行搜索。当查询被发送到分布式表时，查询会被分发到集合中的所有表。服务器收集并处理响应，必要时对聚合值进行排序和重新计算。

从客户端的角度来看，似乎他们在查询一个单一的表。

分布式表可以由任意组合的表组成，包括：

- 本地存储表（[普通表](../../Creating_a_table/Local_tables/Plain_table.md) 和 [实时表](../../Creating_a_table/Local_tables/Real-time_table.md)）
- [远程表](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md)
- 本地表与远程表的组合
- [过滤表](../../Creating_a_table/Local_tables/Percolate_table.md)（本地、远程或混合使用）
- 单个本地表和多个远程表，或任何其他组合

不建议将过滤表和模板表与普通表和实时表混合使用。

分布式表在配置文件中定义为 `distributed` 类型，或者通过 SQL 语句 `CREATE TABLE` 来创建。

#### 在配置文件中

```ini
table foo {
    type = distributed
    local = bar
    local = bar1, bar2
    agent = 127.0.0.1:9312:baz
    agent = host1|host2:tbl
    agent = host1:9301:tbl1|host2:tbl2 [ha_strategy=random retry_count=10]
    ...
}
```

#### 通过SQL

```sql
CREATE TABLE distributed_index type='distributed' local='local_index' agent='127.0.0.1:9312:remote_index'
```

#### 子表

分布式表的核心是其指向的子表列表。分布式表中的子表有两种类型：

1. [本地表](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#创建本地分布式表)：这些表由与分布式表相同的服务器提供服务。列举本地表时，使用 `local =` 语法。可以通过多行 `local =` 列举多个本地表，或者将它们组合到一行中并用逗号分隔。
2. [远程表](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)：这些表由服务器以外的其他位置提供服务。列举远程表时，使用 `agent =` 语法。每行代表一个端点或代理。每个代理可以包含多个外部位置和操作选项。更多详情参见[此处](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)。需要注意的是，服务器并不了解它正在处理的表的类型，因此可能会导致错误。例如，如果你对一个远程表 `foo` 执行 `CALL PQ` 操作，而该表并不是过滤表，则可能会出现错误。

<!-- proofread -->
