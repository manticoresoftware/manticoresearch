# Load balancing

Load balancing is turned on by default for any [distributed table](../../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md) that uses [mirroring](../../Creating_a_cluster/Remote_nodes/Mirroring.md). By default, queries are distributed randomly among the mirrors. You can change this behavior by using the [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md).

## ha_strategy

```ini
ha_strategy = {random|nodeads|noerrors|roundrobin}
```

The mirror selection strategy for load balancing is optional and is set to `random` by default.

The strategy used for mirror selection, or in other words, choosing a specific [agent mirror](../../Creating_a_cluster/Remote_nodes/Mirroring.md#Agent-mirrors) in a distributed table, is controlled by this directive. Essentially, this directive controls how master performs the load balancing between the configured mirror agent nodes. The following strategies are implemented:

### Simple random balancing

<!-- example conf balancing 1 -->
The default balancing mode is simple linear random distribution among the mirrors. This means that equal selection probabilities are assigned to each mirror. This is similar to round-robin (RR), but does not impose a strict selection order.

<!-- intro -->
##### Example:

<!-- request Example -->
```ini
ha_strategy = random
```
<!-- end -->

### Adaptive randomized balancing

The default simple random strategy does not take into account the status of mirrors, error rates, and most importantly, actual response latencies. To address heterogeneous clusters and temporary spikes in agent node load, there are a group of balancing strategies that dynamically adjust the probabilities based on the actual query latencies observed by the master.

The adaptive strategies based on **latency-weighted probabilities**  work as follows:

1. Latency stats are accumulated in blocks of ha_period_karma seconds.
2. Latency-weighted probabilities are recomputed once per karma period.
3. The "dead or alive" flag is adjusted once per request, including ping requests.

Initially, the probabilities are equal. On every step, they are scaled by the inverse of the latencies observed during the last karma period, and then renormalized. For example, if during the first 60 seconds after the master startup, 4 mirrors had latencies of 10 ms, 5 ms, 30 ms, and 3 ms respectively, the first adjustment step would go as follows:

1. Initial percentages: 0.25, 0.25, 0.25, 0.25.
2. Observed latencies: 10 ms, 5 ms, 30 ms, 3 ms.
3. Inverse latencies: 0.1, 0.2, 0.0333, 0.333.
4. Scaled percentages: 0.025, 0.05, 0.008333, 0.0833.
5. Renormalized percentages: 0.15, 0.30, 0.05, 0.50.

This means that the first mirror would have a 15% chance of being chosen during the next karma period, the second one a 30% chance, the third one (slowest at 30 ms) only a 5% chance, and the fourth and fastest one (at 3 ms) a 50% chance. After that period, the second adjustment step would update those chances again, and so on.

The idea is that once the  **observed latencies** stabilize, the **latency weighted probabilities** will stabilize as well. All these adjustment iterations are meant to converge at a point where the average latencies are roughly equal across all mirrors.

<!-- example conf balancing 2 -->
#### nodeads
Latency-weighted probabilities, but dead mirrors are excluded from the selection. A "dead" mirror is defined as a mirror that has resulted in multiple hard errors (e.g. network failure, or no answer, etc) in a row.

<!-- intro -->
##### Example:

<!-- request Example -->
```ini
ha_strategy = nodeads
```
<!-- end -->

<!-- example conf balancing 3 -->
#### noerrors

Latency-weighted probabilities, but mirrors with a worse error/success ratio are excluded from selection.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
ha_strategy = noerrors
```
<!-- end -->

### Round-robin balancing

<!-- example conf balancing 4 -->
Simple round-robin selection, that is, selecting the first mirror in the list, then the second one, then the third one, etc, and then repeating the process once the last mirror in the list is reached. Unlike with the randomized strategies, RR imposes a strict querying order (1, 2, 3, ..., N-1, N, 1, 2, 3, ..., and so on) and *guarantees* that no two consecutive queries will be sent to the same mirror.

<!-- intro -->
##### Example:

<!-- request Example -->
```ini
ha_strategy = roundrobin
```
<!-- end -->

## Instance-wide options

### ha_period_karma

```ini
ha_period_karma = 2m
```

`ha_period_karma` defines the size of the agent mirror statistics window, in seconds (or a time suffix). Optional, the default is 60.

For a distributed table with agent mirrors, the server tracks several different per-mirror counters. These counters are then used for failover and balancing. (The server picks the best mirror to use based on the counters.) Counters are accumulated in blocks of `ha_period_karma` seconds.

After beginning a new block, the master may still use the accumulated values from the previous one until the new one is half full. Thus, any previous history stops affecting the mirror choice after at most 1.5 times ha_period_karma seconds.

Although at most 2 blocks are used for mirror selection, up to 15 last blocks are actually stored for instrumentation purposes. They can be inspected using the `SHOW AGENT STATUS` statement.

### ha_ping_interval

```ini
ha_ping_interval = 3s
```

`ha_ping_interval` directive defines the interval between pings sent to the agent mirrors, in milliseconds (or with a time suffix). This option is optional and its default value is 1000.

For a distributed table with agent mirrors, the server sends all mirrors a ping command during idle periods to track their current status (whether they are alive or dead, network roundtrip time, etc.). The interval between pings is determined by the ha_ping_interval setting.

If you want to disable pings, set ha_ping_interval to 0.

<!-- proofread -->
