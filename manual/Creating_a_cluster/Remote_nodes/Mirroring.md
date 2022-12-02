# Mirroring

[Agent](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) mirrors can be used interchangeably when processing a search query. Manticore instance (can be multiple) hosting the distributed table where the mirrored agents are defined keeps track of mirror status (alive or dead) and response times, and does automatic failover and load balancing based on that.

## Agent mirrors

```ini
agent = node1|node2|node3:9312:shard2
```

The above example declares that 'node1:9312', 'node2:9312', and 'node3:9312' all have a table called shard2, and can be used as interchangeable mirrors. If any single of those servers go down, the queries will be distributed between the other two. When it gets back up, master will detect that and begin routing queries to all three nodes again.

Mirror may also include individual table list, as:

```ini
agent = node1:9312:node1shard2|node2:9312:node2shard2
```

This works essentially the same as the previous example, but different table names will be used when querying different severs: node1shard2 when querying node1:9312, and node2shard when querying node2:9312.

By default, all queries are routed to the best of the mirrors. The best one is picked based on the recent statistics, as controlled by the [ha_period_karma](../../Server_settings/Searchd.md#ha_period_karma) config directive. Master stores a number of metrics (total query count, error count, response time, etc) recently observed for every agent. It groups those by time spans, and karma is that time span length. The best agent mirror is then determined dynamically based on the last 2 such time spans. Specific algorithm that will be used to pick a mirror can be configured [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) directive.

The karma period is in seconds and defaults to 60 seconds. Master stores up to 15 karma spans with per-agent statistics for instrumentation purposes (see `SHOW AGENT STATUS` statement). However, only the last 2 spans out of those are ever used for HA/LB logic.

When there are no queries, master sends a regular ping command every [ha_ping_interval](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_ping_interval) milliseconds in order to have some statistics and at least check, whether the remote host is still alive. ha_ping_interval defaults to 1000 msec. Setting it to 0 disables pings and statistics will only be accumulated based on actual queries.

Example:

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