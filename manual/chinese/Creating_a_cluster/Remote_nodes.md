# 添加带有远程代理的分布式表

要理解如何添加带有远程代理的分布式表，首先需要基本了解 [分布式表](../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md)。本文将重点介绍如何使用分布式表作为创建 Manticore 实例集群的基础。
<!-- example conf dist 1 -->
以下是一个将数据拆分到 4 台服务器上的示例，每台服务器负责一个分片：


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
如果发生服务器故障，分布式表仍能工作，但来自故障分片的结果将会缺失。

<!-- example conf dist 2 -->
现在我们已经添加了镜像，每个分片都位于 2 台服务器上。默认情况下，主节点（拥有分布式表的 searchd 实例）会随机选择一个镜像。

用于选择镜像的模式可以通过 [ha_strategy](../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) 设置来配置。除了默认的 `random` 模式外，还有 `ha_strategy = roundrobin`。

基于延迟加权概率的更高级策略包括 `noerrors` 和 `nodeads`。这些策略不仅会排除有问题的镜像，还会监控响应时间并进行负载均衡。如果某个镜像响应较慢（例如，由于其上正在运行某些操作），它将收到更少的请求。当镜像恢复并提供更好的响应时间时，它将收到更多的请求。

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

