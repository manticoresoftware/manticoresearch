# 创建分布式表

Manticore 允许创建**分布式表**，它们表现得像普通的普通表或实时表，但实际上是用于搜索的一组子表。当查询发送到分布式表时，查询会分发到集合中的所有表。服务器随后收集并处理响应，以进行排序并在必要时重新计算聚合值。

从客户端的角度来看，查询就像是在查询一个单独的表。

分布式表可以由任何组合的表组成，包括：

* 本地存储表 ([普通表](../../Creating_a_table/Local_tables/Plain_table.md) 和 [实时表](../../Creating_a_table/Local_tables/Real-time_table.md))
* [远程表](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md)
* 本地表和远程表的组合
* [感知表](../../Creating_a_table/Local_tables/Percolate_table.md)（本地的、远程的或组合）
* 单个本地表和多个远程表，或者其他任意组合

不建议将感知表和模板表与普通表和实时表混合使用。

分布式表在配置文件中定义为类型 'distributed'，或通过 SQL 子句 `CREATE TABLE` 来定义。

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

分布式表的本质在于它所指向的子表列表。分布式表中有两种类型的子表：

1. [本地表](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table)：这些表是在与分布式表相同的服务器内提供服务的表。枚举本地表时，使用语法 `local =`。你可以使用多行 `local =` 列出多个本地表，或者用逗号分隔将它们合并成一个列表。

2. [远程表](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)：这些表是在服务器之外的任何地方提供服务的表。枚举远程表时，使用语法 `agent =`。每一行代表一个端点或代理。每个代理可以有多个外部位置和如何工作的选项。更多细节请见[这里](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)。值得注意的是，服务器没有关于它所处理表类型的任何信息。例如，如果你对远程表 'foo'（不是感知表）发出 `CALL PQ`，可能会导致错误。

<!-- proofread -->

