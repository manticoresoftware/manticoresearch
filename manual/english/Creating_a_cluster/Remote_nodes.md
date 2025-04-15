# Adding a distributed table with remote agents

To understand how to add a distributed table with remote agents, it is important to first have a basic understanding of  [distributed tables](../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md) In this article, we will focus on how to use a distributed table as the basis for creating a cluster of Manticore instances.
<!-- example conf dist 1 -->
Here is an example of how to split data over 4 servers, each serving one of the shards:


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
In the event of a server failure, the distributed table will still work, but the results from the failed shard will be missing.

<!-- example conf dist 2 -->
Now that we've added mirrors, each shard is found on 2 servers. By default, the master (the searchd instance with the distributed table) will randomly pick one of the mirrors.

The mode used for picking mirrors can be set using the [ha_strategy](../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) setting. In addition to the default `random` mode there's also `ha_strategy = roundrobin`.

More advanced strategies based on latency-weighted probabilities include `noerrors` and `nodeads`. These not only take out mirrors with issues but also monitor response times and do balancing. If a mirror responds slower (for example, due to some operations running on it), it will receive fewer requests. When the mirror recovers and provides better times, it will receive more requests.

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