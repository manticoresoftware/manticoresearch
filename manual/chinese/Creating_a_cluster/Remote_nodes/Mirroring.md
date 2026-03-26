# 镜像

[Agent](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) 镜像在处理搜索查询时可以互换使用。托管定义镜像代理的分布式表的 Manticore 实例会跟踪镜像状态（存活或宕机）和响应时间，并基于这些信息执行自动故障切换和负载均衡。

## 代理镜像

```ini
agent = node1|node2|node3:9312:shard2
```

上述示例声明 `node1:9312`、`node2:9312` 和 `node3:9312` 都有一个名为 shard2 的表，并且可以作为可互换的镜像使用。如果这些服务器中的任何一个宕机，查询将分配给剩下的两个。当服务器恢复上线时，主节点会检测到并开始再次将查询路由到所有三个节点。

镜像也可以包含单独的表列表，如下所示：

```ini
agent = node1:9312:node1shard2|node2:9312:node2shard2
```

这与前面的示例类似，但对不同服务器查询时会使用不同的表名。例如，查询 `node1:9312` 时将使用 node1shard2，查询 `node2:9312` 时将使用 node2shard。

默认情况下，所有查询都路由到最优的镜像。最优镜像是基于最近的统计数据选择的，由配置指令 [ha_period_karma](../../Server_settings/Searchd.md#ha_period_karma) 控制。主节点存储每个代理的指标（总查询数、错误数、响应时间等），并按时间段分组。这些时间段的长度就是 karma。最优代理镜像动态依据最近两个时间段确定。选择镜像的具体算法可以通过 [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) 指令配置。

karma 时间段以秒为单位，默认是 60 秒。主节点为监控目的存储最多 15 个带有每代理统计的 karma 时间段（参见 `SHOW AGENT STATUS` 语句）。然而，HA/LB 逻辑只使用这些中的最后两个时间段。

当没有查询时，主节点每 [ha_ping_interval](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_ping_interval) 毫秒定期发送 ping 命令，以收集统计信息并检查远程主机是否仍然存活。ha_ping_interval 默认是 1000 毫秒。设置为 0 会禁用 ping，统计将仅基于实际查询累计。

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

