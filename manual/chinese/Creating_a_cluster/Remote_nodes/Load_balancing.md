# 负载均衡

对于使用[镜像](../../Creating_a_cluster/Remote_nodes/Mirroring.md)的任何[分布式表](../../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md)，负载均衡默认是开启的。默认情况下，查询会在镜像之间随机分配。你可以通过使用[ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md)来改变这一行为。

## ha_strategy

```ini
ha_strategy = {random|nodeads|noerrors|roundrobin}
```

负载均衡的镜像选择策略是可选的，默认设置为`random`。

用于镜像选择的策略，换句话说，就是在分布式表中选择特定的[代理镜像](../../Creating_a_cluster/Remote_nodes/Mirroring.md#Agent-mirrors)，由该指令控制。基本上，此指令控制主节点如何在配置的镜像代理节点之间执行负载均衡。实现了以下策略：

### 简单随机均衡

<!-- example conf balancing 1 -->
默认的均衡模式是在镜像之间简单线性随机分布。这意味着给每个镜像分配了相等的选择概率。这类似于轮询（RR），但不强制执行严格的选择顺序。

<!-- intro -->
##### 例子：

<!-- request Example -->
```ini
ha_strategy = random
```
<!-- end -->

### 自适应随机均衡

默认的简单随机策略没有考虑镜像的状态、错误率，最重要的是实际响应延迟。为了应对异构集群和代理节点负载的暂时激增，有一组负载均衡策略基于主节点观察到的实际查询延迟动态调整概率。

基于**延迟加权概率**的自适应策略工作方式如下：

1. 延迟统计以ha_period_karma秒为周期累积。
2. 延迟加权概率每个karma周期重新计算一次。
3. “存活或死亡”标志在每个请求时调整，包括ping请求。

初始情况下，概率是相等的。在每一步中，它们被乘以上一个karma周期内观察到的延迟的倒数，然后重新归一化。例如，如果在主节点启动后的前60秒内，4个镜像的延迟分别是10毫秒、5毫秒、30毫秒和3毫秒，第一次调整步骤如下：

1. 初始百分比：0.25，0.25，0.25，0.25。
2. 观察到的延迟：10毫秒，5毫秒，30毫秒，3毫秒。
3. 延迟倒数：0.1，0.2，0.0333，0.333。
4. 缩放百分比：0.025，0.05，0.008333，0.0833。
5. 重新归一化百分比：0.15，0.30，0.05，0.50。

这意味着在接下来的karma周期中，第一个镜像被选中的概率为15%，第二个为30%，第三个（30毫秒最慢）仅为5%，第四个（3毫秒最快）为50%。之后，第二次调整步骤将再次更新这些概率，依此类推。

其思想是，一旦**观察到的延迟**稳定，**延迟加权概率**也将稳定。所有这些调整迭代旨在收敛到一个点，使所有镜像的平均延迟大致相等。

<!-- example conf balancing 2 -->
#### nodeads
延迟加权概率，但不会从选择中包含已死镜像。被定义为“死”的镜像是连续产生多次严重错误（如网络失败或无响应等）的镜像。

<!-- intro -->
##### 例子：

<!-- request Example -->
```ini
ha_strategy = nodeads
```
<!-- end -->

<!-- example conf balancing 3 -->
#### noerrors

延迟加权概率，但从选择中排除错误/成功比例较差的镜像。

<!-- intro -->
##### 例子：

<!-- request Example -->

```ini
ha_strategy = noerrors
```
<!-- end -->

### 轮询均衡

<!-- example conf balancing 4 -->
简单轮询选择，即依次选择列表中的第一个镜像，然后第二个，第三个，依此类推，直到列表末尾，然后重复该过程。与随机策略不同，轮询强制执行严格的查询顺序（1，2，3，…，N-1，N，1，2，3，…），并*保证*不会连续发送两个查询到同一个镜像。

<!-- intro -->
##### 例子：

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

`ha_period_karma` 定义了代理镜像统计窗口的大小，单位为秒（或带时间后缀）。可选，默认值为60。

对于带有代理镜像的分布式表，服务器跟踪几个不同的每镜像计数器。这些计数器随后用于故障转移和负载均衡。（服务器基于这些计数器选择最佳镜像使用。）计数器以 `ha_period_karma` 秒为周期累积。

开始一个新周期后，主节点可能仍使用上一个周期累积的值，直到新周期数据达到一半。因此，任何之前的历史最多会影响镜像选择1.5倍的 `ha_period_karma` 秒。

尽管为镜像选择最多使用两个周期，但实际上存储了最多15个最近周期用于监控目的。它们可以通过 `SHOW AGENT STATUS` 语句查看。

### ha_ping_interval

```ini
ha_ping_interval = 3s
```

`ha_ping_interval` 指令定义了发送给代理镜像的ping命令之间的间隔，单位为毫秒（或带时间后缀）。此选项可选，默认值为1000。

对于带有代理镜像的分布式表，服务器在空闲期间向所有镜像发送ping命令以跟踪其当前状态（是否存活，网络往返时间等）。ping间隔由ha_ping_interval设置。

如果想禁用ping操作，将ha_ping_interval设置为0。

<!-- proofread -->

