# Load balancing

Load balancing is turned on by default for any [distributed table](../../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md) that uses [mirroring](../../Creating_a_cluster/Remote_nodes/Mirroring.md). By default, queries are distributed randomly among mirrors. You can change this behavior by using [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy).

## ha_strategy

```ini
ha_strategy = {random|nodeads|noerrors|roundrobin}
```

The mirror selection strategy for load balancing is optional and is set to `random` by default.

`ha_strategy` can be set globally in the `searchd` configuration or per distributed table. The strategy used for mirror selection, or in other words, choosing a specific [agent mirror](../../Creating_a_cluster/Remote_nodes/Mirroring.md#Agent-mirrors) in a distributed table, is controlled by this directive. In practice, this directive controls how the master balances requests between mirrored agent nodes. The following strategies are implemented:

### Simple random balancing

<!-- example conf balancing 1 -->
The default balancing mode is simple linear random distribution among mirrors. Equal selection probabilities are assigned to each mirror. This is similar to round-robin (RR), but does not impose a strict selection order.

<!-- intro -->
##### Config:

<!-- request Config -->
```ini
ha_strategy = random
```

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
CREATE TABLE products_dist type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  ha_strategy='random';
```
<!-- end -->

### Adaptive randomized balancing

The default simple random strategy does not take into account the status of mirrors, error rates, and, most importantly, actual response latencies. To address heterogeneous clusters and temporary spikes in agent node load, there is a group of balancing strategies that dynamically adjust the probabilities based on the query latencies observed by the master.

The adaptive strategies based on **latency-weighted probabilities** work as follows. The size of that statistics window is controlled by [`ha_period_karma`](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_period_karma), while idle mirror health and round-trip tracking are driven by [`ha_ping_interval`](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_ping_interval). Both are described later on this page and in [Searchd settings](../../Server_settings/Searchd.md).

1. Latency stats are accumulated in blocks of `ha_period_karma` seconds.
2. Latency-weighted probabilities are recomputed once per karma period.
3. The "dead or alive" flag is adjusted once per request, including ping requests.

Initially, the probabilities are equal. On every step, they are scaled by the inverse of the latencies observed during the last karma period, and then renormalized. For example, if during the first 60 seconds after the master startup, 4 mirrors had latencies of 10 ms, 5 ms, 30 ms, and 3 ms respectively, the first adjustment step would go as follows:

1. Initial percentages: 0.25, 0.25, 0.25, 0.25.
2. Observed latencies: 10 ms, 5 ms, 30 ms, 3 ms.
3. Inverse latencies: 0.1, 0.2, 0.0333, 0.333.
4. Scaled percentages: 0.025, 0.05, 0.008333, 0.0833.
5. Renormalized percentages: 0.15, 0.30, 0.05, 0.50.

This means that the first mirror would have a 15% chance of being chosen during the next karma period, the second one a 30% chance, the third one (slowest at 30 ms) only a 5% chance, and the fourth and fastest one (at 3 ms) a 50% chance. After that period, the second adjustment step would update those chances again, and so on.

The idea is that once the **observed latencies** stabilize, the **latency-weighted probabilities** will stabilize as well. All these adjustment iterations are meant to converge at a point where the average latencies are roughly equal across all mirrors.

#### nodeads

<!-- example conf balancing 2 -->
Latency-weighted probabilities, but mirrors with too many consecutive failed attempts are excluded from selection. The consecutive-error counter is reset only by a fully successful query. Warnings do not reset it. A mirror starts being treated as dead after more than 3 consecutive non-success outcomes, so the first 3 failures in a row are tolerated but the 4th makes that mirror ineligible for `nodeads` selection until it answers successfully again.

<!-- intro -->
##### Config:

<!-- request Config -->
```ini
ha_strategy = nodeads
```

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
CREATE TABLE products_dist type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  ha_strategy='nodeads';
```
<!-- end -->

#### noerrors

<!-- example conf balancing 3 -->
Latency-weighted probabilities, but mirrors with a worse recent error/success ratio are deprioritized and may be excluded from selection.

`noerrors` looks at recent per-mirror counters collected over the last `ha_period_karma` windows. It first compares the ratio of critical transport failures (`connect_timeouts`, `connect_failures`, `network_errors`, `wrong_replies`, `unexpected_closings`) to total recent activity. Ratios at or below 3% are treated as effectively zero. If several mirrors tie there, it then compares the broader error ratio that also includes warning-bearing replies. Mirrors with no successful queries at all in the recent window are skipped entirely.

`noerrors` is therefore most useful for intermittent transport-level failures and degraded mirrors, not as a guarantee that every logical remote-table problem disappears immediately. For example, if one mirror points to a missing table, you may still see query failures before that mirror accumulates enough bad history to stop being preferred. Use [`SHOW AGENT STATUS`](../../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS) to inspect recent successes, warnings, and network errors for each mirror.

<!-- intro -->
##### Config:

<!-- request Config -->
```ini
ha_strategy = noerrors
```

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
CREATE TABLE products_dist type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  ha_strategy='noerrors';
```
<!-- end -->

### Round-robin balancing

<!-- example conf balancing 4 -->
Simple round-robin selection, that is, selecting the first mirror in the list, then the second one, then the third one, etc, and then repeating the process once the last mirror in the list is reached. Unlike with the randomized strategies, RR imposes a strict querying order (1, 2, 3, ..., N-1, N, 1, 2, 3, ..., and so on) and guarantees that no two consecutive queries will be sent to the same mirror while all mirrors are healthy.

<!-- intro -->
##### Config:

<!-- request Config -->
```ini
ha_strategy = roundrobin
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

Assume the mirrored distributed tables already exist, for example:

```sql
CREATE TABLE products_random type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products';

CREATE TABLE products_rr type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  ha_strategy='roundrobin';
```

<!-- example load-balancing-sql-behavior -->
The difference between `random` and `roundrobin` becomes visible when you query mirrored distributed tables repeatedly.

<!-- intro -->
##### SQL request:

<!-- request SQL -->
```sql
SELECT node FROM products_random;
SELECT node FROM products_random;
SELECT node FROM products_rr;
SELECT node FROM products_rr;
```

<!-- intro -->
##### SQL response:

<!-- response SQL -->
```sql
+-------+
| node  |
+-------+
| node2 |
+-------+

+-------+
| node  |
+-------+
| node1 |
+-------+

+-------+
| node  |
+-------+
| node1 |
+-------+

+-------+
| node  |
+-------+
| node2 |
+-------+
```
<!-- end -->

## SHOW AGENT STATUS

<!-- example load-balancing-sql-verification -->
`SHOW AGENT STATUS` is the main debugging tool for mirrored distributed tables. For the full statement syntax and all output formats, see [SHOW AGENT STATUS](../../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS). For load balancing specifically, the most useful fields are:

- `ag_N_pingtripmsec` - current ping round-trip time for mirror `N`
- `ag_N_errorsarow` - consecutive non-success outcomes; this is what `nodeads` effectively watches
- `ag_N_1periods_connect_timeouts`, `connect_failures`, `network_errors`, `wrong_replies`, `unexpected_closings` - recent transport failures
- `ag_N_1periods_warnings` - replies that succeeded but returned warnings
- `ag_N_1periods_succeeded_queries` - recent successful queries
- `ag_N_1periods_msecsperquery` - recent average query time

These per-period counters are reported for 1, 5, and 15 rolling windows, where each window is `ha_period_karma` seconds long.

Use these commands on the master to inspect what was stored and how the mirrors are behaving.

<!-- intro -->
##### SQL request:

<!-- request SQL -->
```sql
SHOW CREATE TABLE products_rr;
SHOW AGENT STATUS;
SHOW AGENT STATUS LIKE '%errorsarow%';
SHOW AGENT STATUS LIKE '%1periods_%';
```

<!-- intro -->
##### SQL response:

<!-- response SQL -->
```sql
+---------------------------------+-------+
| Key                             | Value |
+---------------------------------+-------+
| ag_0_pingtripmsec               | 0.233 |
| ag_0_errorsarow                 | 0     |
| ag_0_1periods_network_errors    | 0     |
| ag_0_1periods_warnings          | 0     |
| ag_0_1periods_succeeded_queries | 11    |
| ag_0_1periods_msecsperquery     | n/a   |
+---------------------------------+-------+
```
<!-- end -->

Practical interpretation:

- If `errorsarow` climbs above 3, `nodeads` starts treating that mirror as dead.
- If a mirror has no recent `succeeded_queries`, `noerrors` will skip it entirely.
- If several mirrors have low error ratios, `noerrors` still prefers the ones with lower latency because the normal weight rebalancing stays in play.
- `pingtripmsec` helps you distinguish "reachable but slow" from "failing" mirrors.

## Related options and supported scopes

The options related to mirroring, timeouts, and retries are not all supported at the same scope. This page focuses on how they affect load balancing. For full syntax and daemon-level behavior, use the linked reference pages.

| Option | Instance-wide | Per table | Per query | Per agent | Full details |
|---|---|---|---|---|---|
| `ha_strategy` | yes | yes | no | yes | [Load balancing](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy), [Remote tables](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) |
| `ha_period_karma` | yes | no | no | no | [Searchd: ha_period_karma](../../Server_settings/Searchd.md#ha_period_karma) |
| `ha_ping_interval` | yes | no | no | no | [Searchd: ha_ping_interval](../../Server_settings/Searchd.md#ha_ping_interval) |
| `agent_connect_timeout` | yes | yes | no | no | [Searchd: agent_connect_timeout](../../Server_settings/Searchd.md#agent_connect_timeout), [Remote tables: agent_connect_timeout](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) |
| `agent_query_timeout` | yes | yes | yes | no | [Searchd: agent_query_timeout](../../Server_settings/Searchd.md#agent_query_timeout), [Remote tables: agent_query_timeout](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) |
| `agent_retry_count` / `mirror_retry_count` / `retry_count` | yes (`agent_retry_count`) | yes (`agent_retry_count` or `mirror_retry_count`) | yes (`OPTION retry_count=...`) | yes (`agent=...[retry_count=...]`) | [Searchd: agent_retry_count](../../Server_settings/Searchd.md#agent_retry_count), [Remote tables: agent_retry_count](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count), [Remote tables: mirror_retry_count](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count), [Remote tables: agent](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) |
| `agent_retry_delay` | yes | no | yes | no | [Searchd: agent_retry_delay](../../Server_settings/Searchd.md#agent_retry_delay) |

### ha_strategy

<!-- example ha-strategy-scopes -->
`ha_strategy` controls how mirrored agents are selected. It can be set globally in `searchd`, per distributed table, or per agent inside `agent = ... [ha_strategy=...]`. A narrower scope overrides a broader one.

The strategy examples above already show both the global and per-table forms. For the per-agent override syntax, see [Remote tables](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent).

If you want one distributed table to use a different balancing strategy than the global default, set it on that table.

<!-- intro -->
##### Config:

<!-- request Config -->
```ini
ha_strategy = random

table products_rr {
  type = distributed
  agent = 127.0.0.1:9312|127.0.0.1:9313:products
  ha_strategy = roundrobin
}
```

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
CREATE TABLE products_rr type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  ha_strategy='roundrobin';
```
<!-- end -->

### ha_period_karma

<!-- example ha-period-karma-scope -->
`ha_period_karma` defines the agent mirror statistics window used by the adaptive balancing strategies above and is supported only as an instance-wide `searchd` setting. Full daemon-level details are in [Searchd: ha_period_karma](../../Server_settings/Searchd.md#ha_period_karma).

<!-- intro -->
##### Config:

<!-- request Config -->
```ini
ha_period_karma = 2m
```
<!-- end -->

### ha_ping_interval

<!-- example ha-ping-interval-scope -->
`ha_ping_interval` defines how often idle mirrors are pinged and is supported only as an instance-wide `searchd` setting. Full daemon-level details are in [Searchd: ha_ping_interval](../../Server_settings/Searchd.md#ha_ping_interval).

<!-- intro -->
##### Config:

<!-- request Config -->
```ini
ha_ping_interval = 3s
```
<!-- end -->

### agent_connect_timeout

<!-- example agent-connect-timeout-scopes -->
`agent_connect_timeout` defines how long Manticore waits to establish a connection to a remote agent. It is supported as an instance-wide default and per distributed table. Full details are in [Searchd: agent_connect_timeout](../../Server_settings/Searchd.md#agent_connect_timeout) and [Remote tables: agent_connect_timeout](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout).

<!-- intro -->
##### Config:

<!-- request Config -->
```ini
agent_connect_timeout = 300ms
```

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
CREATE TABLE products_dist type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  agent_connect_timeout='300ms';
```
<!-- end -->

### agent_query_timeout

<!-- example agent-query-timeout-scopes -->
`agent_query_timeout` defines how long Manticore waits for a connected remote agent to finish the query. It is supported as an instance-wide default, per distributed table, and per query as `OPTION agent_query_timeout=...`. Full details are in [Searchd: agent_query_timeout](../../Server_settings/Searchd.md#agent_query_timeout) and [Remote tables: agent_query_timeout](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout).

If `agent_query_timeout` is reached, the query is not retried automatically; a warning is produced instead.

<!-- intro -->
##### Config:

<!-- request Config -->
```ini
agent_query_timeout = 500ms
```

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
CREATE TABLE products_dist type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  agent_query_timeout='500ms';
```

<!-- intro -->
##### SQL query option:

<!-- request SQL -->
```sql
SELECT * FROM products_dist OPTION agent_query_timeout=750;
```
<!-- end -->

### agent_retry_count and mirror_retry_count

<!-- example agent-retry-count-scopes -->
`agent_retry_count` defines how many times Manticore retries remote-agent work before reporting a fatal query error. The name varies by scope: use `agent_retry_count` as the instance-wide setting, `agent_retry_count` or its alias `mirror_retry_count` on a distributed table, `OPTION retry_count=...` per query, and `[retry_count=...]` inside an individual `agent=...` declaration. Full details are in [Searchd: agent_retry_count](../../Server_settings/Searchd.md#agent_retry_count), [Remote tables: agent_retry_count](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count), and [Remote tables: mirror_retry_count](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count).

If you use agent mirrors, the retry count is aggregated across mirrors. The per-agent `[retry_count=...]` option acts as an absolute cap for that specific agent declaration.

<!-- intro -->
##### Config:

<!-- request Config -->
```ini
agent_retry_count = 2
```

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
CREATE TABLE products_dist type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  mirror_retry_count='2';
```

<!-- intro -->
##### SQL query option:

<!-- request SQL -->
```sql
SELECT * FROM products_dist OPTION retry_count=1;
```

<!-- intro -->
##### Config per-agent cap:

<!-- request Config -->
```ini
table products_dist {
  type = distributed
  agent = 127.0.0.1:9312|127.0.0.1:9313:products[retry_count=2]
}
```
<!-- end -->

### agent_retry_delay

<!-- example agent-retry-delay-scopes -->
`agent_retry_delay` defines the delay between retry attempts. It is supported as an instance-wide default and per query as `OPTION retry_delay=...`, but not per distributed table. Full details are in [Searchd: agent_retry_delay](../../Server_settings/Searchd.md#agent_retry_delay).

This option is only relevant when retries are enabled through `agent_retry_count` or `OPTION retry_count=...`.

<!-- intro -->
##### Config:

<!-- request Config -->
```ini
agent_retry_delay = 500ms
```

<!-- intro -->
##### SQL query option:

<!-- request SQL -->
```sql
SELECT * FROM products_dist OPTION retry_count=2, retry_delay=300;
```
<!-- end -->

<!-- proofread -->
