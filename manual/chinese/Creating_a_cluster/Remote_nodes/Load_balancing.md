# 负载均衡

对于使用[镜像](../../Creating_a_cluster/Remote_nodes/Mirroring.md)的任何[分布式表](../../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md)，默认启用负载均衡。默认情况下，查询会在镜像之间随机分配。您可以使用[ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md)来更改此行为。

## ha_strategy

```ini
ha_strategy = {random|nodeads|noerrors|roundrobin}
```

负载均衡的镜像选择策略是可选的，默认设置为`random`。

用于镜像选择的策略，换句话说，就是在分布式表中选择特定的[代理镜像](../../Creating_a_cluster/Remote_nodes/Mirroring.md#Agent-mirrors)，由此指令控制。本质上，该指令控制主节点如何在配置的镜像代理节点之间执行负载均衡。实现了以下策略：

### 简单随机均衡

<!-- example conf balancing 1 -->
默认的均衡模式是在镜像之间简单线性随机分布。这意味着为每个镜像分配相等的选择概率。这类似于轮询（RR），但不强制执行严格的选择顺序。

<!-- intro -->
##### 示例：

<!-- request Example -->
```ini
ha_strategy = random
```
<!-- end -->

### 自适应随机均衡

默认的简单随机策略不考虑镜像的状态、错误率，最重要的是实际响应延迟。为了解决异构集群和代理节点负载的临时峰值，存在一组基于实际查询延迟动态调整概率的均衡策略。

基于**延迟加权概率**的自适应策略工作原理如下：

1. 延迟统计以ha_period_karma秒为块进行累积。
2. 延迟加权概率每个karma周期重新计算一次。
3. “存活或死亡”标志每个请求调整一次，包括ping请求。

初始时，概率是相等的。每一步，它们按上一个karma周期观察到的延迟的倒数进行缩放，然后重新归一化。例如，如果在主节点启动后的前60秒内，4个镜像的延迟分别为10毫秒、5毫秒、30毫秒和3毫秒，第一次调整步骤如下：

1. 初始百分比：0.25，0.25，0.25，0.25。
2. 观察到的延迟：10毫秒，5毫秒，30毫秒，3毫秒。
3. 延迟倒数：0.1，0.2，0.0333，0.333。
4. 缩放后的百分比：0.025，0.05，0.008333，0.0833。
5. 重新归一化的百分比：0.15，0.30，0.05，0.50。

这意味着在下一个karma周期内，第一个镜像被选中的概率为15%，第二个为30%，第三个（最慢的30毫秒）仅为5%，第四个（最快的3毫秒）为50%。之后，第二次调整步骤将再次更新这些概率，依此类推。

其思想是，一旦**观察到的延迟**稳定，**延迟加权概率**也将稳定。所有这些调整迭代旨在收敛到一个点，使所有镜像的平均延迟大致相等。

<!-- example conf balancing 2 -->
#### nodeads
延迟加权概率，但排除“死亡”镜像。定义“死亡”镜像为连续出现多个硬错误（例如网络故障或无响应等）的镜像。

<!-- intro -->
##### 示例：

<!-- request Example -->
```ini
ha_strategy = nodeads
```
<!-- end -->

<!-- example conf balancing 3 -->
#### noerrors

延迟加权概率，但排除错误/成功率较差的镜像。

<!-- intro -->
##### 示例：

<!-- request Example -->

```ini
ha_strategy = noerrors
```
<!-- end -->

### 轮询均衡

<!-- example conf balancing 4 -->
简单的轮询选择，即依次选择列表中的第一个镜像，然后是第二个，第三个，依此类推，直到列表末尾后重复该过程。与随机策略不同，RR强制执行严格的查询顺序（1，2，3，...，N-1，N，1，2，3，...，依此类推），并*保证*不会连续两次查询发送到同一镜像。

<!-- intro -->
##### 示例：

<!-- request Example -->
```ini
ha_strategy = roundrobin
```
<!-- end -->

## 实例范围选项

### ha_period_karma

```ini
ha_period_karma = 2m
```

`ha_period_karma`定义代理镜像统计窗口的大小，单位为秒（或带时间后缀）。可选，默认值为60。

对于带有代理镜像的分布式表，服务器跟踪多个不同的每镜像计数器。这些计数器用于故障转移和负载均衡。（服务器基于计数器选择最佳镜像。）计数器以`ha_period_karma`秒为块进行累积。

开始新块后，主节点可能仍使用前一个块的累积值，直到新块填充到一半。因此，任何之前的历史最多在1.5倍ha_period_karma秒后停止影响镜像选择。

虽然最多使用2个块进行镜像选择，但实际上会存储最多15个最近的块用于监控目的。可以使用`SHOW AGENT STATUS`语句查看。

### ha_ping_interval

```ini
ha_ping_interval = 3s
```

`ha_ping_interval`指令定义发送给代理镜像的ping间隔，单位为毫秒（或带时间后缀）。此选项为可选，默认值为1000。

对于带有代理镜像的分布式表，服务器在空闲期间向所有镜像发送ping命令，以跟踪其当前状态（是否存活，网络往返时间等）。ping间隔由ha_ping_interval设置决定。

如果想禁用ping，将ha_ping_interval设置为0。

<!-- proofread -->

