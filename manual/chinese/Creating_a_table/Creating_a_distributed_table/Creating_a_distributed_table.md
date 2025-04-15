# 创建分布式表

Manticore 允许创建 **分布式表**，这类表像常规的普通表或实时表一样，但实际上是用于搜索的一组子表。当查询发送到分布式表时，它会在集合中的所有表之间进行分发。服务器然后收集并处理响应，以便在必要时对聚合值进行排序和重新计算。

从客户端的角度来看，它似乎是在查询一个单一的表。

分布式表可以由任何组合的表组成，包括：

* 本地存储表 ([plain table](../../Creating_a_table/Local_tables/Plain_table.md) 和 [Real-Time](../../Creating_a_table/Local_tables/Real-time_table.md))
* [远程表](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md)
* 本地和远程表的组合
* [Percolate 表](../../Creating_a_table/Local_tables/Percolate_table.md)（本地、远程或组合）
* 单一本地表和多个远程表，或任何其他组合

不推荐将 Percolate 表和模板表与普通表和实时表混合使用。

分布式表在配置文件中被定义为类型 'distributed'，或通过 SQL 子句 `CREATE TABLE`

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

#### 通过 SQL

```sql
CREATE TABLE distributed_index type='distributed' local='local_index' agent='127.0.0.1:9312:remote_table'
```

#### 子表

分布式表的本质在于其子表列表，它指向这些子表。分布式表中有两种类型的子表：

1. [本地表](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table)：这些是在与分布式表同一服务器上提供的表。要列举本地表，可以使用语法 `local =`。可以使用多个 `local =` 行列出几个本地表，或者将它们组合成一个用逗号分隔的列表。

2. [远程表](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)：这些是在服务器以外的任何地方提供的表。要列举远程表，可以使用语法 `agent =`。每一行代表一个端点或代理。每个代理可以有多个外部位置和其工作方式的选项。更多详情 [这里](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)。需要注意的是，服务器并不知道它正在处理的表的类型。如果，例如，您对一个不是 Percolate 表的远程表 'foo' 发出 `CALL PQ`，这可能会导致错误。

<!-- proofread -->
