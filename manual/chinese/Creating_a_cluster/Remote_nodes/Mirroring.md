# 镜像

[代理](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)镜像可以在处理搜索查询时互换使用。托管镜像代理定义的分布式表的 Manticore 实例会跟踪镜像状态（存活或死亡）和响应时间，并根据这些信息执行自动故障转移和负载均衡。

## 代理镜像

```ini

agent = node1|node2|node3:9312:shard2

```

上面的示例声明 `node1:9312`、`node2:9312` 和 `node3:9312` 都有一个名为 shard2 的表，可以作为互换的镜像使用。如果这些服务器中的任何一台宕机，查询将分配到剩下的两台。当服务器重新上线时，主节点会检测到并开始将查询路由到所有三个节点。

镜像还可以包含单独的表列表，如下所示：

```ini

agent = node1:9312:node1shard2|node2:9312:node2shard2

```

这与前面的示例类似，但在查询不同服务器时将使用不同的表名。例如，查询 node1:9312 时将使用 node1shard2，查询 node2:9312 时将使用 node2shard。

默认情况下，所有查询都路由到最佳镜像。最佳镜像是根据最近的统计信息选出的，这些统计信息由 [ha_period_karma](../../Server_settings/Searchd.md#ha_period_karma) 配置指令控制。主节点存储每个代理的指标（总查询数、错误数、响应时间等），并按时间段对这些指标进行分组。卡玛是时间段的长度。然后根据最后两个这样的时间段动态确定最佳代理镜像。选择镜像的特定算法可以通过 [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) 指令进行配置。

卡玛周期以秒为单位，默认值为 60 秒。主节点最多存储 15 个卡玛时间段的每个代理统计信息以供仪器使用（请参见 `SHOW AGENT STATUS` 语句）。然而，仅使用这两个时间段中的最后两个进行 HA/LB 逻辑。

当没有查询时，主节点每 [ha_ping_interval](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_ping_interval) 毫秒发送一个常规 ping 命令，以便收集统计信息并检查远程主机是否仍然存活。ha_ping_interval 的默认值为 1000 毫秒。将其设置为 0 将禁用 ping，统计信息将仅基于实际查询进行累积。

示例：

```ini

# 在 4 个服务器上进行分片

# 仅在 2 个分片中，但每个分片有 2 个故障转移镜像

# node1、node2 作为本地承载 shard1

# node3、node4 作为本地承载 shard2
