# 负载均衡

对于使用 [镜像](../../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md) 的任何 [分布式表](../../Creating_a_cluster/Remote_nodes/Mirroring.md)，默认情况下会启用负载均衡。默认情况下，查询会在镜像之间随机分配。您可以通过 [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) 更改此行为。

## ha_strategy

```ini
ha_strategy = {random|nodeads|noerrors|roundrobin}
```

负载均衡的镜像选择策略是可选的，默认设置为 `random`。

`ha_strategy` 可以在 `searchd` 配置中全局设置，也可以针对每个分布式表进行设置。此指令控制镜像选择策略，换句话说，就是在分布式表中选择特定 [代理镜像](../../Creating_a_cluster/Remote_nodes/Mirroring.md#Agent-mirrors) 的策略。实际上，此指令控制主节点如何在镜像代理节点之间平衡请求。已实现以下策略：

### 简单随机均衡

<!-- example conf balancing 1 -->
默认的平衡模式是在镜像之间进行简单的线性随机分配。每个镜像被分配相等的选择概率。这类似于轮询（RR），但不会强制执行严格的选择顺序。

<!-- intro -->
##### 配置:

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

### 自适应随机均衡

默认的简单随机策略不会考虑镜像的状态、错误率，最重要的是实际响应延迟。为了解决异构集群和代理节点负载的临时峰值问题，有一组基于查询延迟动态调整概率的平衡策略。

基于 **延迟加权概率** 的自适应策略工作方式如下。统计窗口的大小由 [`ha_period_karma`](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_period_karma) 控制，而空闲镜像健康状态和往返跟踪则由 [`ha_ping_interval`](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_ping_interval) 驱动。两者将在本文档和 [Searchd 设置](../../Server_settings/Searchd.md) 中进一步描述。

1. 延迟统计以 `ha_period_karma` 秒为一个块进行累积。
2. 延迟加权概率每个 karma 周期重新计算一次。
3. “存活或死亡”标志每次请求，包括 ping 请求时调整。

初始时概率相等。每一步概率按上一个 karma 周期内观察到的延迟的倒数进行缩放，然后归一化。例如，在主节点启动后前 60 秒内，4 个镜像的延迟分别为 10 毫秒、5 毫秒、30 毫秒和 3 毫秒，第一次调整步骤如下：

1. 初始百分比：0.25, 0.25, 0.25, 0.25。
2. 观察到的延迟：10 ms, 5 ms, 30 ms, 3 ms。
3. 延迟倒数：0.1, 0.2, 0.0333, 0.333。
4. 缩放后的百分比：0.025, 0.05, 0.008333, 0.0833。
5. 归一化百分比：0.15, 0.30, 0.05, 0.50。

这意味着，在下一个 karma 周期内，第一个镜像被选中的概率为 15%，第二个为 30%，第三个（最慢，30 毫秒）仅为 5%，第四个（最快，3 毫秒）为 50%。之后，第二次调整步骤会再次更新这些概率，依此类推。

这个想法是，一旦 **观察到的延迟** 稳定，**延迟加权概率** 也会稳定。所有这些调整迭代旨在收敛到一个点，即所有镜像的平均延迟大致相等。

#### nodeads

<!-- example conf balancing 2 -->
延迟加权概率，但连续失败次数过多的镜像将被排除在选择之外。只有完全成功的查询才会重置连续错误计数器。警告不会重置它。当某个镜像连续非成功结果超过 3 次时，它将被视为“死”镜像，因此前 3 次连续失败是可容忍的，但第 4 次失败将使该镜像在成功响应之前无法被 `nodeads` 选择。

<!-- intro -->
##### 配置:

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
延迟加权概率，但近期错误/成功比率较差的镜像会被降级，甚至可能被排除在选择之外。

`noerrors` 会检查最近 `ha_period_karma` 窗口内收集的每个镜像的计数器。它首先比较关键传输失败（`connect_timeouts`、`connect_failures`、`network_errors`、`wrong_replies`、`unexpected_closings`）与总近期活动的比率。比率在 3% 及以下的被视为有效为零。如果多个镜像在此处并列，它会进一步比较包含警告回复的更广泛的错误比率。在最近窗口内完全没有成功查询的镜像将被完全跳过。

因此，`noerrors` 最适用于间歇性传输层故障和性能下降的镜像，而不是保证每个逻辑远程表问题立即消失。例如，如果一个镜像指向一个缺失的表，您可能在该镜像积累足够的不良历史记录之前仍然看到查询失败。使用 [`SHOW AGENT STATUS`](../../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS) 检查每个镜像的近期成功、警告和网络错误。

<!-- intro -->
##### 配置:

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

### 轮询均衡

<!-- example conf balancing 4 -->
简单的轮询选择，即依次选择列表中的第一个镜像，然后是第二个，然后是第三个，依此类推，当到达列表中的最后一个镜像后，再重复该过程。与随机化策略不同，RR（轮询）强制执行一个严格的查询顺序（1, 2, 3, ..., N-1, N, 1, 2, 3, ... 等等），并保证在所有镜像都正常的情况下，不会将两个连续的查询发送到同一个镜像。

<!-- intro -->
##### 配置:

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

假设镜像分布式表已经存在，例如：

```sql
CREATE TABLE products_random type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products';

CREATE TABLE products_rr type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  ha_strategy='roundrobin';
```

<!-- example load-balancing-sql-behavior -->
当您重复查询镜像分布式表时，`random` 和 `roundrobin` 的区别就会显现出来。

<!-- intro -->
##### SQL 请求:

<!-- request SQL -->
```sql
SELECT node FROM products_random;
SELECT node FROM products_random;
SELECT node FROM products_rr;
SELECT node FROM products_rr;
```

<!-- intro -->
##### SQL 响应:

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
`SHOW AGENT STATUS` 是镜像分布式表的主要调试工具。有关完整语句语法和所有输出格式，请参见 [SHOW AGENT STATUS](../../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS)。就负载均衡而言，最有用的字段是：

- `ag_N_pingtripmsec` - 镜像 `N` 的当前 ping 往返时间
- `ag_N_errorsarow` - 连续的非成功结果；这就是 `nodeads` 实际上监控的内容
- `ag_N_1periods_connect_timeouts`, `connect_failures`, `network_errors`, `wrong_replies`, `unexpected_closings` - 最近的传输失败
- `ag_N_1periods_warnings` - 成功但返回警告的回复
- `ag_N_1periods_succeeded_queries` - 最近的成功查询
- `ag_N_1periods_msecsperquery` - 最近的平均查询时间

这些每周期计数器会针对 1、5 和 15 个滚动窗口进行报告，每个窗口的长度为 `ha_period_karma` 秒。

在主节点上使用这些命令来检查已存储的内容以及镜像的行为。

<!-- intro -->
##### SQL 请求:

<!-- request SQL -->
```sql
SHOW CREATE TABLE products_rr;
SHOW AGENT STATUS;
SHOW AGENT STATUS LIKE '%errorsarow%';
SHOW AGENT STATUS LIKE '%1periods_%';
```

<!-- intro -->
##### SQL 响应:

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

实际解释：

- 如果 `errorsarow` 上升到 3 以上，`nodeads` 会开始将该镜像视为已死。
- 如果某个镜像没有最近的 `succeeded_queries`，`noerrors` 会完全跳过它。
- 如果多个镜像的错误率较低，`noerrors` 仍然会优先选择延迟较低的镜像，因为正常的权重重新平衡机制仍然生效。
- `pingtripmsec` 帮助您区分“可到达但较慢”和“失败”的镜像。

## 相关选项和支持范围

与镜像、超时和重试相关的选项并非都在相同范围内得到支持。本页重点介绍它们如何影响负载均衡。如需完整的语法和守护进程级别的行为，请使用链接的参考页面。

| 选项 | 实例范围 | 每表 | 每查询 | 每代理 | 详细信息 |
|---|---|---|---|---|---|
| `ha_strategy` | 是 | 是 | 否 | 是 | [负载均衡](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy), [远程表](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) |
| `ha_period_karma` | 是 | 否 | 否 | 否 | [Searchd: ha_period_karma](../../Server_settings/Searchd.md#ha_period_karma) |
| `ha_ping_interval` | 是 | 否 | 否 | 否 | [Searchd: ha_ping_interval](../../Server_settings/Searchd.md#ha_ping_interval) |
| `agent_connect_timeout` | 是 | 是 | 否 | 否 | [Searchd: agent_connect_timeout](../../Server_settings/Searchd.md#agent_connect_timeout), [远程表: agent_connect_timeout](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) |
| `agent_query_timeout` | 是 | 是 | 是 | 否 | [Searchd: agent_query_timeout](../../Server_settings/Searchd.md#agent_query_timeout), [远程表: agent_query_timeout](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) |
| `agent_retry_count` / `mirror_retry_count` / `retry_count` | 是 (`agent_retry_count`) | 是 (`agent_retry_count` 或 `mirror_retry_count`) | 是 (`OPTION retry_count=...`) | 是 (`agent=...[retry_count=...]`) | [Searchd: agent_retry_count](../../Server_settings/Searchd.md#agent_retry_count), [远程表: agent_retry_count](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count), [远程表: mirror_retry_count](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count), [远程表: agent](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) |
| `agent_retry_delay` | 是 | 否 | 是 | 否 | [Searchd: agent_retry_delay](../../Server_settings/Searchd.md#agent_retry_delay) |

### ha_strategy

<!-- example ha-strategy-scopes -->
`ha_strategy` 控制如何选择镜像代理。它可以在 `searchd` 中全局设置，也可以在每个分布式表中设置，或者在 `agent = ... [ha_strategy=...]` 中每个代理内部设置。较窄的作用域会覆盖较宽的作用域。

上面的策略示例已经展示了全局和每表形式。有关每代理覆盖语法，请参见 [远程表](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)。

如果您希望某个分布式表使用与全局默认值不同的平衡策略，请在该表上设置它。

<!-- intro -->
##### 配置:

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
`ha_period_karma` 定义了上述自适应平衡策略使用的代理镜像统计窗口，仅支持作为实例范围的 `searchd` 设置。完整的守护进程级别详细信息请参见 [Searchd: ha_period_karma](../../Server_settings/Searchd.md#ha_period_karma)。

<!-- intro -->
##### 配置:

<!-- request Config -->
```ini
ha_period_karma = 2m
```
<!-- end -->

### ha_ping_interval

<!-- example ha-ping-interval-scope -->
`ha_ping_interval` 定义了对空闲镜像进行 ping 的频率，仅支持作为全局实例级的 `searchd` 设置。完整的守护进程级别详情请参见 [Searchd: ha_ping_interval](../../Server_settings/Searchd.md#ha_ping_interval)。

<!-- intro -->
##### 配置:

<!-- request Config -->
```ini
ha_ping_interval = 3s
```
<!-- end -->

### agent_connect_timeout

<!-- example agent-connect-timeout-scopes -->
`agent_connect_timeout` 定义了 Manticore 等待与远程代理建立连接的时间。它支持作为全局实例默认值和每个分布式表的设置。完整详情请参见 [Searchd: agent_connect_timeout](../../Server_settings/Searchd.md#agent_connect_timeout) 和 [远程表: agent_connect_timeout](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout)。

<!-- intro -->
##### 配置:

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
`agent_query_timeout` 定义了 Manticore 等待已连接的远程代理完成查询的时间。它支持作为全局实例默认值、每个分布式表以及每个查询的 `OPTION agent_query_timeout=...`。完整详情请参见 [Searchd: agent_query_timeout](../../Server_settings/Searchd.md#agent_query_timeout) 和 [远程表: agent_query_timeout](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout)。

如果达到 `agent_query_timeout`，查询不会自动重试；而是会生成一个警告。

<!-- intro -->
##### 配置:

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
##### SQL 查询选项:

<!-- request SQL -->
```sql
SELECT * FROM products_dist OPTION agent_query_timeout=750;
```
<!-- end -->

### agent_retry_count 和 mirror_retry_count

<!-- example agent-retry-count-scopes -->
`agent_retry_count` 定义了 Manticore 在报告致命查询错误之前重试远程代理工作的次数。名称因作用域而异：使用 `agent_retry_count` 作为全局实例设置，在分布式表上使用 `agent_retry_count` 或其别名 `mirror_retry_count`，在每个查询中使用 `OPTION retry_count=...`，并在单个 `agent=...` 声明内部使用 `[retry_count=...]`。完整详情请参见 [Searchd: agent_retry_count](../../Server_settings/Searchd.md#agent_retry_count)、[远程表: agent_retry_count](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) 和 [远程表: mirror_retry_count](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count)。

如果您使用代理镜像，重试次数将在镜像之间汇总。每个代理的 `[retry_count=...]` 选项对该特定代理声明起到绝对上限的作用。

<!-- intro -->
##### 配置:

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
##### SQL 查询选项:

<!-- request SQL -->
```sql
SELECT * FROM products_dist OPTION retry_count=1;
```

<!-- intro -->
##### 每个代理的配置上限:

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
`agent_retry_delay` 定义了重试尝试之间的延迟时间。它支持作为全局实例默认值和每个查询的 `OPTION retry_delay=...`，但不支持每个分布式表。完整详情请参见 [Searchd: agent_retry_delay](../../Server_settings/Searchd.md#agent_retry_delay)。

当通过 `agent_retry_count` 或 `OPTION retry_count=...` 启用重试时，此选项才相关。

<!-- intro -->
##### 配置:

<!-- request Config -->
```ini
agent_retry_delay = 500ms
```

<!-- intro -->
##### SQL 查询选项:

<!-- request SQL -->
```sql
SELECT * FROM products_dist OPTION retry_count=2, retry_delay=300;
```
<!-- end -->

<!-- proofread -->
