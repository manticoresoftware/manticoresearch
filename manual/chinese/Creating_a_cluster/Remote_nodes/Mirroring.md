# 镜像

[Agent](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) 镜像在处理搜索查询时可以互换使用。托管分布式表的 Manticore 实例会跟踪镜像状态（存活或死亡）和响应时间，并基于这些信息执行自动故障转移和负载均衡。

## 代理镜像

```ini
agent = node1|node2|node3:9312:shard2
```

上面的示例声明 `node1:9312`、`node2:9312` 和 `node3:9312` 都有一个名为 `shard2` 的表，并且可以作为可互换的镜像使用。如果这些服务器中的任何一个宕机，查询将分布在其余的服务器之间。当服务器重新上线时，主节点会检测到并重新开始将查询路由到所有镜像。

镜像也可以包含单独的表列表，如下所示：

```ini
agent = node1:9312:node1shard2|node2:9312:node2shard2
```

这与前面的示例类似，但在查询不同服务器时会使用不同的表名。例如，查询 `node1:9312` 时将使用 `node1shard2`，查询 `node2:9312` 时将使用 `node2shard2`。

默认情况下，镜像选择使用全局或表级的 [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy)。如果您未显式设置它，默认策略是 `random`。主节点会存储每个代理的指标，如总查询数、错误数和响应时间，并将其分组到由 [ha_period_karma](../../Server_settings/Searchd.md#ha_period_karma) 控制的时间段中。这些统计信息随后用于 [Load balancing](../../Creating_a_cluster/Remote_nodes/Load_balancing.md) 中描述的平衡策略。

[karma period](../../Server_settings/Searchd.md#ha_period_karma) 以秒为单位，默认为 60 秒。主节点为每个代理存储最多 15 个 karma 时间段的统计信息以供仪器使用（参见 [SHOW AGENT STATUS](../../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS)）。但是，仅最后两个时间段用于 HA/LB 逻辑。

当没有查询时，主节点会定期发送 ping 命令，间隔为 [ha_ping_interval](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_ping_interval)，以收集统计信息并检查远程主机是否仍然存活。[ha_ping_interval](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_ping_interval) 默认为 1000 毫秒。将其设置为 0 会禁用 ping，统计信息将仅基于实际查询进行累积。结合 [ha_period_karma](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_period_karma)，这控制镜像健康状况和延迟变化影响负载均衡的速度。

## 镜像与分片

这一区别很重要：

- 一个 `agent='host1|host2:table'` 条目表示一个具有镜像后端的远程分片。
- 多个 `agent='...'` 条目表示多个远程分片。

例如：

```ini
# one shard, two mirrors
agent = node1|node2:9312:products

# two shards
agent = node1:9312:products_a
agent = node2:9312:products_b
```

## 定义镜像

<!-- example mirroring-definition -->
镜像概念可以通过配置文件或 `CREATE TABLE` 进行配置。对于 TCP 连接，`agent=` 必须使用远程代理/API 端口（通常是 `9312`），而不是 MySQL 端口（`9306`）。如果您只想为一个分布式表设置特定的平衡策略，请在该表上设置 `ha_strategy`，而不是仅依赖全局默认值。

<!-- intro -->
##### 配置：

<!-- request Config -->
```ini
table products_dist {
  type = distributed
  agent = 127.0.0.1:9312|127.0.0.1:9313:products
  ha_strategy = roundrobin
}
```

<!-- intro -->
##### SQL：

<!-- request SQL -->
```sql
CREATE TABLE products_dist type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  ha_strategy='roundrobin';
```
<!-- end -->

<!-- example mirroring-setup -->
假设镜像表已经存在，例如：

<!-- intro -->
##### 远程节点 1 上的 SQL：

<!-- request SQL -->
```sql
CREATE TABLE products(id bigint, title text, node string);
INSERT INTO products(id,title,node) VALUES(1,'same title','node1');
```

<!-- intro -->
##### 远程节点 2 上的 SQL：

<!-- request SQL -->
```sql
CREATE TABLE products(id bigint, title text, node string);
INSERT INTO products(id,title,node) VALUES(1,'same title','node2');
```

<!-- intro -->
##### 主节点上的 SQL：

<!-- request SQL -->
```sql
CREATE TABLE products_dist type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  ha_strategy='roundrobin';
```
<!-- end -->

<!-- example mirroring-sql-usage-and-verification -->
从主节点正常使用分布式表，然后验证它是如何存储的以及镜像的行为。

<!-- intro -->
##### SQL 请求：

<!-- request SQL -->
```sql
SELECT id, title, node FROM products_dist;
SHOW CREATE TABLE products_dist;
SHOW AGENT STATUS;
```

<!-- intro -->
##### SQL 响应：

<!-- response SQL -->
```sql
+------+------------+-------+
| id   | title      | node  |
+------+------------+-------+
|    1 | same title | node1 |
+------+------------+-------+

+---------------+----------------------------------------------------------------------------------+
| Table         | Create Table                                                                     |
+---------------+----------------------------------------------------------------------------------+
| products_dist | CREATE TABLE products_dist type='distributed' agent='127.0.0.1:9312:products|... |
+---------------+----------------------------------------------------------------------------------+
```
<!-- end -->

<!-- example mirroring-sharded-cluster -->
在总共 4 台服务器上对表进行分片的示例，分为 2 个分片，每个分片有 2 个镜像。

<!-- intro -->
##### 配置：

<!-- request Config -->
```ini
# node1, node2 carry shard1 as local
# node3, node4 carry shard2 as local

# config on node1, node2
agent = node3:9312|node4:9312:shard2

# config on node3, node4
agent = node1:9312|node2:9312:shard1
```
<!-- end -->
<!-- proofread -->
