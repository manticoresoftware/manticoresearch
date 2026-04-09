# 创建分布式表

Manticore 允许创建**分布式表**，它们的行为类似普通的普通表或实时表，但实际上是用于搜索的子表集合。当向分布式表发送查询时，查询会被分配到集合中的所有表。服务器随后收集并处理响应，必要时对聚合值进行排序和重新计算。

从客户端的角度来看，感觉就像是在查询一个单一的表。

分布式表可以由以下任意组合的表组成：

* 本地存储表（[普通表](../../Creating_a_table/Local_tables/Plain_table.md) 和 [实时表](../../Creating_a_table/Local_tables/Real-time_table.md)）
* [远程表](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md)
* 本地和远程表的组合
* [Percolate 表](../../Creating_a_table/Local_tables/Percolate_table.md)（本地、远程或组合）
* 单个本地表和多个远程表，或任何其他组合

不建议将 percolate 和模板表与普通表和实时表混合使用。

分布式表在配置文件中定义为类型 'distributed'，或者通过 SQL 子句 `CREATE TABLE` 定义。

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

分布式表的核心在于其所指向的子表列表。分布式表中有两种类型的子表：

1. [本地表](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table)：这些是与分布式表处于同一服务器上的表。要枚举本地表，使用语法 `local =`。你可以通过多行 `local =` 列出多个本地表，或者用逗号分隔将它们合并到一行列表中。

2. [远程表](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)：这些是部署在服务器外部的表。要枚举远程表，使用语法 `agent =`。每一行代表一个端点或代理。每个代理可以包含多个外部位置和选项以决定其工作方式。更多详情见[这里](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)。需要注意的是，服务器对正在处理的表的类型不拥有任何信息。例如，如果你对一个不是 percolate 表的远程表 'foo' 发出 `CALL PQ`，可能会导致错误。

<!-- proofread -->

