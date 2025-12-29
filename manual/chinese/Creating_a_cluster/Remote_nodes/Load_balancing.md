# 负载均衡

对于使用[镜像](../../Creating_a_cluster/Remote_nodes/Mirroring.md)的[分布式表](../../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md)，默认开启负载均衡。默认情况下，查询会随机分布到各个镜像上。您可以使用[ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md)来更改此行为。

## ha_strategy

```ini
ha_strategy = {random|nodeads|noerrors|roundrobin}
```

负载均衡的镜像选择策略是可选的，默认设置为 `random`。

用于镜像选择的策略，换句话说，就是在分布式表中选择特定的[代理镜像](../../Creating_a_cluster/Remote_nodes/Mirroring.md#Agent-mirrors)，由此指令控制。实质上，该指令控制主节点如何在配置的镜像代理节点间执行负载均衡。实现了以下几种策略：

### 简单随机均衡

<!-- example conf balancing 1 -->
默认的均衡模式是在线性随机分布中简单地在镜像间分配。这意味着每个镜像被赋予相等的选择概率。这类似于轮询（RR），但不强制执行严格的选择顺序。

<!-- intro -->
##### 示例：

<!-- request Example -->
```ini
ha_strategy = random
```
<!-- end -->

### 自适应随机均衡

默认的简单随机策略没有考虑镜像的状态、错误率，最重要的是实际响应延迟。为了解决异构集群和代理节点负载的临时峰值，存在一组基于主节点实际观察到的查询延迟动态调整概率的均衡策略。

基于**延迟加权概率**的自适应策略工作原理如下：

1. 延迟统计以 ha_period_karma 秒为单位累积。
2. 延迟加权概率每个 karma 周期重新计算一次。
3. “存活或死亡”标志每次请求，包括 ping 请求时调整。

初始时概率相等。每一步概率按上一个 karma 周期内观察到的延迟的倒数进行缩放，然后归一化。例如，在主节点启动后前 60 秒内，4 个镜像的延迟分别为 10 毫秒、5 毫秒、30 毫秒和 3 毫秒，第一次调整步骤如下：

1. 初始百分比：0.25, 0.25, 0.25, 0.25。
2. 观察到的延迟：10 ms, 5 ms, 30 ms, 3 ms。
3. 延迟倒数：0.1, 0.2, 0.0333, 0.333。
4. 缩放后的百分比：0.025, 0.05, 0.008333, 0.0833。
5. 归一化百分比：0.15, 0.30, 0.05, 0.50。

这意味着，在下一个 karma 周期内，第一个镜像被选中的概率为 15%，第二个为 30%，第三个（最慢，30 毫秒）仅为 5%，第四个（最快，3 毫秒）为 50%。之后，第二次调整步骤会再次更新这些概率，依此类推。

其思想是，一旦**观察到的延迟**稳定，**延迟加权概率**也会稳定。所有这些调整迭代旨在收敛到一个点，使得所有镜像的平均延迟大致相等。

<!-- example conf balancing 2 -->
#### nodeads
延迟加权概率，但排除“死亡”的镜像。“死亡”镜像定义为连续发生多次硬错误（如网络故障或无响应等）的镜像。

<!-- intro -->
##### 示例：

<!-- request Example -->
```ini
ha_strategy = nodeads
```
<!-- end -->

<!-- example conf balancing 3 -->
#### noerrors

延迟加权概率，但排除错误率较高的镜像。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_strategy = noerrors
```
<!-- end -->

### 轮询均衡

<!-- example conf balancing 4 -->
简单轮询选择，即依次选择列表中的第一个镜像、第二个镜像、第三个镜像，依此类推，直到最后一个镜像，然后重复该过程。与随机策略不同，轮询施加严格的查询顺序（1, 2, 3, ..., N-1, N, 1, 2, 3, ...），并*保证*不会连续将两个查询发送至同一个镜像。

<!-- intro -->
##### 示例：

<!-- request Example -->
```ini
ha_strategy = roundrobin
```
<!-- end -->

## 实例级选项

### ha_period_karma

```ini
ha_period_karma = 2m
```

`ha_period_karma` 定义代理镜像统计窗口的大小，单位为秒（或带时间后缀）。可选，默认值为 60。

对于带代理镜像的分布式表，服务器会跟踪多个不同的每镜像计数器。服务器基于这些计数器进行故障转移和负载均衡。（服务器根据计数器选择最佳镜像使用。）计数器在 `ha_period_karma` 秒的区块中累积。

开始新计数块后，主节点可能在新块填充到一半之前仍使用之前块中累积的值。因此，任何先前的历史最多在 1.5 倍 ha_period_karma 秒后不再影响镜像选择。

虽然最多使用两个区块进行镜像选择，但实际上最多存储最近 15 个区块以支持工具分析。可通过 `SHOW AGENT STATUS` 语句查看。

### ha_ping_interval

```ini
ha_ping_interval = 3s
```

`ha_ping_interval` 指令定义发送给代理镜像的 ping 间隔，单位为毫秒（或带时间后缀）。该选项可选，默认值为 1000。

对于带代理镜像的分布式表，服务器在空闲期间向所有镜像发送 ping 命令以跟踪其当前状态（是否存活、网络往返时间等）。ping 发送间隔由 ha_ping_interval 配置决定。

如果想禁用 ping，可将 ha_ping_interval 设置为 0。

<!-- proofread -->

