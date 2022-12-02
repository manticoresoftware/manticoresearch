# Load balancing

Load balancing is turned on by default for any [distributed table](../../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md) using [mirroring](../../Creating_a_cluster/Remote_nodes/Mirroring.md). By default queries are distributed randomly among the mirrors. To change this behaviour you can use [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md).

## ha_strategy

```ini
ha_strategy = {random|nodeads|noerrors|roundrobin}
```

Agent mirror selection strategy for load balancing. Optional, default is `random`.

The strategy used for mirror selection, or in other words, choosing a specific [agent mirror](../../Creating_a_cluster/Remote_nodes/Mirroring.md#Agent-mirrors) in a distributed table. Essentially, this directive controls how exactly master does the load balancing between the configured mirror agent nodes. The following strategies are implemented:

### Simple random balancing

<!-- example conf balancing 1 -->
The default balancing mode. Simple linear random distribution among the mirrors. That is, equal selection probability are assigned to every mirror. Kind of similar to round-robin (RR), but unlike RR, does not impose a strict selection order.

<!-- intro -->
##### Example:

<!-- request Example -->
```ini
ha_strategy = random
```
<!-- end -->

### Adaptive randomized balancing

The default simple random strategy does not take mirror status, error rate and, most importantly, actual response latencies into account. So to accommodate for heterogeneous clusters and/or temporary spikes in agent node load, we have a group of balancing strategies that dynamically adjusts the probabilities based on the actual query latencies observed by the master.

The adaptive strategies based on **latency-weighted probabilities** basically work as follows:

* latency stats are accumulated in blocks of ha_period_karma seconds;
* once per karma period latency-weighted probabilities get recomputed;
* once per request (including ping requests) "dead or alive" flag is adjusted.

Currently, we begin with equal probabilities (or percentages, for brevity), and on every step, scale them by the inverse of the latencies observed during the last "karma" period, and then renormalize them. For example, if during the first 60 seconds after the master startup 4 mirrors had latencies of 10, 5, 30, and 3 msec/query respectively, the first adjustment step would go as follow:

* initial percentages: 0.25, 0.25, 0.25, 0.25;
* observed latencies: 10 ms, 5 ms, 30 ms, 3 ms;
* inverse latencies: 0.1, 0.2, 0.0333, 0.333;
* scaled percentages: 0.025, 0.05, 0.008333, 0.0833;
* renormalized percentages: 0.15, 0.30, 0.05, 0.50.

Meaning that the 1st mirror would have a 15% chance of being chosen during the next karma period, the 2nd one a 30% chance, the 3rd one (slowest at 30 ms) only a 5% chance, and the 4th and the fastest one (at 3 ms) a 50% chance. Then, after that period, the second adjustment step would update those chances again, and so on.

The rationale here is, once the **observed latencies** stabilize, the **latency weighted probabilities** stabilize as well. So all these adjustment iterations are supposed to converge at a point where the average latencies are (roughly) equal over all mirrors.

<!-- example conf balancing 2 -->
#### nodeads
Latency-weighted probabilities, but dead mirrors are excluded from the selection. "Dead" mirror is defined as a mirror that resulted in multiple hard errors (eg. network failure, or no answer, etc) in a row.

<!-- intro -->
##### Example:

<!-- request Example -->
```ini
ha_strategy = nodeads
```
<!-- end -->

<!-- example conf balancing 3 -->
#### noerrors

Latency-weighted probabilities, but mirrors with worse errors/success ratio are excluded from the selection.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
ha_strategy = noerrors
```
<!-- end -->

### Round-robin balancing

<!-- example conf balancing 4 -->
Simple round-robin selection, that is, selecting the 1st mirror in the list, then the 2nd one, then the 3rd one, etc, and then repeating the process once the last mirror in the list is reached. Unlike with the randomized strategies, RR imposes a strict querying order (1, 2, 3, ..., N-1, N, 1, 2, 3, ... and so on) and *guarantees* that no two subsequent queries will be sent to the same mirror.

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

`ha_period_karma` defines agent mirror statistics window size, in seconds (or time suffixed). Optional, default is 60.

For a distributed table with agent mirrors in it, server tracks several different per-mirror counters. These counters are then used for failover and balancing. (Server picks the best mirror to use based on the counters.) Counters are accumulated in blocks of `ha_period_karma` seconds.

After beginning a new block, master may still use the accumulated values from the previous one, until the new one is half full. Thus, any previous history stops affecting the mirror choice after 1.5 times ha_period_karma seconds at most.

Despite that at most 2 blocks are used for mirror selection, up to 15 last blocks are actually stored, for instrumentation purposes. They can be inspected using `SHOW AGENT STATUS` statement.

### ha_ping_interval

```ini
ha_ping_interval = 3s
```

`ha_ping_interval` defines interval between agent mirror pings, in milliseconds (or time suffixed). Optional, default is 1000.

For a distributed table with agent mirrors in it, server sends all mirrors a ping command during the idle periods. This is to track the current agent status (alive or dead, network roundtrip, etc). The interval between such pings is defined by this directive.

To disable pings, set ha_ping_interval to 0.
