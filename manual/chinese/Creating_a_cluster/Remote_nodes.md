# 添加分布式表与远程代理

要理解如何添加一个带有远程代理的分布式表，首先需要对 [分布式表](../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md) 有基本的了解。在本文中，我们将重点介绍如何使用分布式表作为创建 Manticore 实例集群的基础。
<!-- example conf dist 1 -->
下面是如何在 4 台服务器上划分数据的示例，每个服务器服务于一个分片：


<!-- intro -->
##### ini:

<!-- request ini -->
```ini
table mydist {
          type  = distributed
          agent = box1:9312:shard1
          agent = box2:9312:shard2
          agent = box3:9312:shard3
          agent = box4:9312:shard4
}
```
<!-- end -->
在服务器发生故障的情况下，分布式表仍然会正常工作，但失败的分片的结果将会缺失。

<!-- example conf dist 2 -->
现在我们添加了镜像，每个分片在 2 台服务器上都有。默认情况下，主服务器（具有分布式表的 searchd 实例）将随机选择其中一个镜像。

用于选择镜像的模式可以使用 [ha_strategy](../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) 设置进行调整。除了默认的 `random` 模式外，还有 `ha_strategy = roundrobin`。

基于延迟加权概率的更高级策略包括 `noerrors` 和 `nodeads`。这些策略不仅会排除存在问题的镜像，还会监控响应时间并进行负载均衡。如果某个镜像的响应较慢（例如，由于其上运行的一些操作），它将收到更少的请求。当镜像恢复并提供更好的响应时间时，它将收到更多的请求。

<!-- intro -->
##### ini:

<!-- request ini -->
```ini
table mydist {
          type  = distributed
          agent = box1:9312|box5:9312:shard1
          agent = box2:9312:|box6:9312:shard2
          agent = box3:9312:|box7:9312:shard3
          agent = box4:9312:|box8:9312:shard4
}
```
<!-- end -->

<!-- proofread -->
