# 镜像

[代理](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)镜像可以在处理搜索查询时互换使用。托管有定义镜像代理的分布式表的 Manticore 实例会跟踪镜像状态（存活或失效）和响应时间，并基于这些信息执行自动故障转移和负载均衡。

## 代理镜像

```ini
agent = node1|node2|node3:9312:shard2
```

上述示例声明 `node1:9312`、`node2:9312` 和 `node3:9312` 都有一个名为 shard2 的表，并可以作为互换镜像使用。如果其中任何一台服务器出现故障，查询将分配给剩下的两台。当服务器重新上线时，主节点会检测到这一点，并重新开始将查询路由到所有三台节点。

镜像还可以包含单独的表列表，如下所示：

```ini
agent = node1:9312:node1shard2|node2:9312:node2shard2
```

这与前面的示例类似，但在查询不同服务器时将使用不同的表名。例如，在查询 `node1:9312` 时将使用 `node1shard2`，在查询 `node2:9312` 时将使用 `node2shard2`。

默认情况下，所有查询会路由到表现最好的镜像。最佳镜像的选择基于最近的统计数据，由 [ha_period_karma](../../Server_settings/Searchd.md#ha_period_karma) 配置指令控制。主节点为每个代理存储指标（总查询计数、错误计数、响应时间等），并按时间段分组。这段时间称为 karma，默认长度为 60 秒。最佳代理镜像会动态地基于最近的两个时间段确定。选择镜像所使用的具体算法可以通过 [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) 指令进行配置。

当没有查询时，主节点会在每个 [ha_ping_interval](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_ping_interval) 毫秒内发送定期 ping 命令，以收集统计数据并检查远程主机是否仍然存活。`ha_ping_interval` 默认值为 1000 毫秒。将其设置为 0 将禁用 ping，统计数据仅基于实际查询进行累积。

示例：

```ini
# sharding table over 4 servers total
# in just 2 shards but with 2 failover mirrors for each shard
# node1, node2 carry shard1 as local
# node3, node4 carry shard2 as local

# config on node1, node2
agent = node3:9312|node4:9312:shard2

# config on node3, node4
agent = node1:9312|node2:9312:shard1
```
<!-- proofread -->