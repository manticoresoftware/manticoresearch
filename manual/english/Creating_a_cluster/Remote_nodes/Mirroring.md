# Mirroring

[Agent](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) mirrors can be used interchangeably when processing a search query. The Manticore instance hosting the distributed table where the mirrored agents are defined keeps track of mirror status (alive or dead) and response times, and performs automatic failover and load balancing based on this information.

## Agent mirrors

```ini
agent = node1|node2|node3:9312:shard2
```

The above example declares that `node1:9312`, `node2:9312`, and `node3:9312` all have a table called `shard2`, and can be used as interchangeable mirrors. If any of these servers go down, the queries will be distributed between the remaining ones. When the server comes back online, the master will detect it and begin routing queries to all mirrors again.

A mirror may also include an individual table list, as follows:

```ini
agent = node1:9312:node1shard2|node2:9312:node2shard2
```

This works similarly to the previous example, but different table names will be used when querying different servers. For example, `node1shard2` will be used when querying `node1:9312`, and `node2shard2` will be used when querying `node2:9312`.

By default, mirror selection uses the global or table-level [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy). If you do not set it explicitly, the default strategy is `random`. The master stores metrics such as total query count, error count, and response time for each agent and groups them into time spans controlled by [ha_period_karma](../../Server_settings/Searchd.md#ha_period_karma). These statistics are then used by the balancing strategies described in [Load balancing](../../Creating_a_cluster/Remote_nodes/Load_balancing.md).

The [karma period](../../Server_settings/Searchd.md#ha_period_karma) is in seconds and defaults to 60 seconds. The master stores up to 15 karma spans with per-agent statistics for instrumentation purposes (see [SHOW AGENT STATUS](../../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS)). However, only the last two spans are used for HA/LB logic.

When there are no queries, the master sends a regular ping command every [ha_ping_interval](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_ping_interval) in order to collect statistics and check if the remote host is still alive. [ha_ping_interval](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_ping_interval) defaults to 1000 msec. Setting it to 0 disables pings, and statistics will only be accumulated based on actual queries. Together with [ha_period_karma](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_period_karma), this controls how quickly mirror health and latency changes influence load balancing.

## Mirrors vs shards

This distinction is important:

- One `agent='host1|host2:table'` entry means one remote shard with mirrored backends.
- Multiple `agent='...'` entries mean multiple remote shards.

For example:

```ini
# one shard, two mirrors
agent = node1|node2:9312:products

# two shards
agent = node1:9312:products_a
agent = node2:9312:products_b
```

## Defining mirrors

<!-- example mirroring-definition -->
The same mirroring concept can be configured either in a config file or with `CREATE TABLE`. For TCP connections, `agent=` must use the remote agent/API port (typically `9312`), not the MySQL port (`9306`). If you want a specific balancing policy for just one distributed table, set `ha_strategy` on that table instead of relying only on the global default.

<!-- intro -->
##### Config:

<!-- request Config -->
```ini
table products_dist {
  type = distributed
  agent = 127.0.0.1:9312|127.0.0.1:9313:products
  ha_strategy = roundrobin
}
```

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
CREATE TABLE products_dist type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  ha_strategy='roundrobin';
```
<!-- end -->

<!-- example mirroring-setup -->
Assume the mirrored tables already exist, for example:

<!-- intro -->
##### SQL on remote node 1:

<!-- request SQL -->
```sql
CREATE TABLE products(id bigint, title text, node string);
INSERT INTO products(id,title,node) VALUES(1,'same title','node1');
```

<!-- intro -->
##### SQL on remote node 2:

<!-- request SQL -->
```sql
CREATE TABLE products(id bigint, title text, node string);
INSERT INTO products(id,title,node) VALUES(1,'same title','node2');
```

<!-- intro -->
##### SQL on the master:

<!-- request SQL -->
```sql
CREATE TABLE products_dist type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  ha_strategy='roundrobin';
```
<!-- end -->

<!-- example mirroring-sql-usage-and-verification -->
Use the distributed table normally from the master, then verify how it was stored and how the mirrors are behaving.

<!-- intro -->
##### SQL request:

<!-- request SQL -->
```sql
SELECT id, title, node FROM products_dist;
SHOW CREATE TABLE products_dist;
SHOW AGENT STATUS;
```

<!-- intro -->
##### SQL response:

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
Example of sharding a table over 4 servers total, in 2 shards with 2 mirrors for each shard.

<!-- intro -->
##### Config:

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
