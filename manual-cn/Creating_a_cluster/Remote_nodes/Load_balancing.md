# 负载均衡

负载均衡默认启用，适用于任何使用 [镜像](../../Creating_a_cluster/Remote_nodes/Mirroring.md) 的 [分布式表](../../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md)。默认情况下，查询会在镜像之间随机分配。您可以通过使用 [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md) 来更改此行为。

## ha_strategy

```ini
ha_strategy = {random|nodeads|noerrors|roundrobin}
```

镜像选择策略用于负载均衡是可选的，默认设置为 `random`。

此指令控制镜像选择策略，也就是在分布式表中选择特定的 [代理镜像](../../Creating_a_cluster/Remote_nodes/Mirroring.md#代理镜像)。本质上，该指令控制主节点如何在配置的镜像代理节点之间进行负载均衡。实施的策略包括：

### 简单随机均衡

<!-- example conf balancing 1 -->
默认的均衡模式是简单线性随机分配给镜像。这意味着每个镜像分配了相等的选择概率。这类似于轮询（RR），但不强制执行严格的选择顺序。

<!-- intro -->
##### 示例：

<!-- request Example -->
```ini
ha_strategy = random
```
<!-- end -->

### 自适应随机均衡

默认的简单随机策略不考虑镜像的状态、错误率，以及最重要的实际响应延迟。为了解决异构集群和代理节点负载的临时峰值，有一组均衡策略根据主节点观察到的实际查询延迟动态调整概率。

基于 **延迟加权概率** 的自适应策略如下：

1. 在 `ha_period_karma` 秒的时间段内累计延迟统计数据。
2. 每个 Karma 周期重新计算延迟加权概率。
3. 每次请求（包括 ping 请求）都会调整 "死或活" 标志。

最初，所有概率是相等的。在每一步中，它们会根据最近 Karma 周期观察到的延迟的倒数进行缩放，然后重新归一化。例如，在主节点启动后的前 60 秒，如果 4 个镜像的延迟分别为 10 ms、5 ms、30 ms 和 3 ms，则第一次调整步骤如下：

1. 初始百分比：0.25，0.25，0.25，0.25。
2. 观察到的延迟：10 ms，5 ms，30 ms，3 ms。
3. 延迟的倒数：0.1，0.2，0.0333，0.333。
4. 缩放后的百分比：0.025，0.05，0.008333，0.0833。
5. 重新归一化的百分比：0.15，0.30，0.05，0.50。

这意味着，在下一个 Karma 周期中，第一个镜像被选择的机会为 15%，第二个镜像为 30%，第三个（延迟最高的，30 ms）仅为 5%，第四个（最快的，3 ms）则为 50%。在这个周期之后，第二次调整步骤将再次更新这些概率，依此类推。

其核心理念是，一旦 **观察到的延迟** 稳定，**延迟加权概率** 也将稳定。这些调整迭代旨在收敛到一个点，使得所有镜像的平均延迟大致相等。

<!-- example conf balancing 2 -->
#### nodeads
延迟加权概率，但会将死镜像排除在选择之外。"死" 镜像被定义为连续出现多个严重错误（例如网络故障或无响应等）的镜像。

<!-- intro -->
##### 示例：

<!-- request Example -->
```ini
ha_strategy = nodeads
```
<!-- end -->

<!-- example conf balancing 3 -->
#### noerrors

延迟加权概率，但会将错误/成功率较差的镜像排除在选择之外。

<!-- intro -->

##### 示例：

<!-- request Example -->

```ini
ha_strategy = noerrors
```
<!-- end -->

### Round-robin balancing

<!-- example conf balancing 4 -->
简单的轮询选择，即选择列表中的第一个镜像，然后第二个、第三个，以此类推，当达到列表末尾后重复该过程。与随机策略不同，轮询强制执行严格的查询顺序（1, 2, 3, ..., N-1, N, 1, 2, 3, ...），并*确保*不会向同一个镜像发送两个连续的查询。

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

`ha_period_karma` 定义了代理镜像统计窗口的大小，单位为秒（或带时间后缀）。这是可选的，默认值为 60。

对于带有代理镜像的分布式表，服务器跟踪多个每个镜像的计数器。这些计数器用于故障转移和负载均衡。（服务器根据计数器选择最佳镜像。）计数器在 `ha_period_karma` 秒的时间段内累积。

在开始新的时间段后，主节点仍可以使用之前累积的值，直到新的时间段达到一半。因此，任何以前的历史记录最多在 1.5 倍的 `ha_period_karma` 秒后停止影响镜像选择。

虽然在镜像选择中最多使用 2 个时间段，但实际上会存储最后 15 个时间段以供仪器监测。可以使用 `SHOW AGENT STATUS` 语句查看。

### ha_ping_interval

```ini
ha_ping_interval = 3s
```

`ha_ping_interval` 指令定义了发送到代理镜像的 ping 的间隔，单位为毫秒（或带时间后缀）。此选项是可选的，默认值为 1000。

对于带有代理镜像的分布式表，服务器在空闲期间向所有镜像发送 ping 命令，以跟踪它们的当前状态（是否存活、网络往返时间等）。ping 之间的间隔由 `ha_ping_interval` 设置确定。

如果希望禁用 ping，可以将 `ha_ping_interval` 设置为 0。

<!-- proofread -->
