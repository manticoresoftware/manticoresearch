# 镜像

[Agent](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) 镜像在处理搜索查询时可以互换使用。托管分布式表（定义了镜像代理）的 Manticore 实例会跟踪镜像状态（存活或宕机）和响应时间，并基于这些信息执行自动故障转移和负载均衡。

## Agent 镜像

```ini
agent = node1|node2|node3:9312:shard2
```

上述示例声明 `node1:9312`、`node2:9312` 和 `node3:9312` 都有一个名为 shard2 的表，可以作为可互换的镜像使用。如果这些服务器中的任何一个宕机，查询将分配到剩余的两个服务器。当服务器重新上线时，主节点会检测到并开始将查询路由到所有三个节点。

镜像也可以包含单独的表列表，如下所示：

```ini
agent = node1:9312:node1shard2|node2:9312:node2shard2
```

这与前面的示例类似，但查询不同服务器时会使用不同的表名。例如，查询 node1:9312 时使用 node1shard2，查询 node2:9312 时使用 node2shard。

默认情况下，所有查询都会路由到最佳镜像。最佳镜像基于最近的统计数据选择，由配置指令 [ha_period_karma](../../Server_settings/Searchd.md#ha_period_karma) 控制。主节点存储每个代理的指标（总查询数、错误数、响应时间等），并按时间段分组。karma 是时间段的长度。最佳代理镜像根据最近两个时间段动态确定。选择镜像的具体算法可以通过 [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) 指令配置。

karma 周期以秒为单位，默认值为 60 秒。主节点存储最多 15 个 karma 时间段的每代理统计数据用于监控（参见 `SHOW AGENT STATUS` 语句）。但只有最近的两个时间段用于 HA/LB 逻辑。

当没有查询时，主节点每隔 [ha_ping_interval](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_ping_interval) 毫秒发送一次常规 ping 命令，以收集统计数据并检查远程主机是否仍然存活。ha_ping_interval 默认值为 1000 毫秒。设置为 0 则禁用 ping，统计数据仅基于实际查询累积。

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

