# 使用远程代理添加分布式表

要了解如何使用远程代理添加分布式表，首先需要对 [分布式表](../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md) 有基本的理解。在本文中，我们将重点讨论如何将分布式表作为创建 Manticore 实例集群的基础。
<!-- example conf dist 1 -->
这里是如何将数据分配到 4 台服务器的示例，每台服务器负责一个分片：


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
在服务器故障的情况下，分布式表仍然可以工作，但来自故障分片的结果将缺失。

<!-- example conf dist 2 -->

现在我们已经添加了镜像，每个分片都存在于 2 台服务器上。默认情况下，主节点（带有分布式表的 searchd 实例）将随机选择其中一个镜像。

选择镜像的模式可以通过 [ha_strategy](../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) 设置进行配置。除了默认的 `random` 模式外，还有 `ha_strategy = roundrobin`。

基于延迟加权概率的更高级策略包括 `noerrors` 和 `nodeads`。这些策略不仅会排除有问题的镜像，还会监控响应时间并进行负载均衡。如果某个镜像响应较慢（例如，正在运行某些操作），它将接收较少的请求。当镜像恢复并提供更好的响应时间时，它将接收更多的请求。

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