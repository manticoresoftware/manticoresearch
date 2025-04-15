# Mirroring

[Agent](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) mirrors can be used interchangeably when processing a search query. The Manticore instance(s) hosting the distributed table where the mirrored agents are defined keeps track of mirror status (alive or dead) and response times, and performs automatic failover and load balancing based on this information.

## Agent mirrors

```ini
agent = node1|node2|node3:9312:shard2
```

The above example declares that `node1:9312`, `node2:9312`, and `node3:9312` all have a table called shard2, and can be used as interchangeable mirrors. If any of these servers go down, the queries will be distributed between the remaining two. When the server comes back online, the master will detect it and begin routing queries to all three nodes again.

A mirror may also include an individual table list, as follows:

```ini
agent = node1:9312:node1shard2|node2:9312:node2shard2
```

This works similarly to the previous example, but different table names will be used when querying different servers. For example, node1shard2 will be used when querying node1:9312, and node2shard will be used when querying node2:9312.

By default, all queries are routed to the best of the mirrors. The best mirror is selected based on recent statistics, as controlled by the [ha_period_karma](../../Server_settings/Searchd.md#ha_period_karma) config directive. The master stores metrics (total query count, error count, response time, etc.) for each agent and groups these by time spans. The karma is the length of the time span. The best agent mirror is then determined dynamically based on the last two such time spans. The specific algorithm used to pick a mirror can be configured with the [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) directive.

The karma period is in seconds and defaults to 60 seconds. The master stores up to 15 karma spans with per-agent statistics for instrumentation purposes (see `SHOW AGENT STATUS` statement). However, only the last two spans out of these are used for HA/LB logic.

When there are no queries, the master sends a regular ping command every  [ha_ping_interval](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_ping_interval) milliseconds in order to collect statistics and check if the remote host is still alive. The ha_ping_interval defaults to 1000 msec. Setting it to 0 disables pings, and statistics will only be accumulated based on actual queries.

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
<!-- proofread -->