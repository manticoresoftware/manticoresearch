# Adding a distributed table with remote agents

Please read the article about [distributed tables](../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md) for general overview of distributed tables. Here we focus on using a distributed table as a basis for creating a cluster of Manticore instances.
<!-- example conf dist 1 -->
Here we have split the data over 4 servers, each serving one of the shards. If one of the servers fails, our distributed table will still work, but we would miss the results from the failed shard.


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

<!-- example conf dist 2 -->
Now that we've added mirrors, each shard is found on 2 servers. By default, the master (the searchd instance with the distributed table) will pick randomly one of the mirrors.

The mode used for picking mirrors can be set with [ha_strategy](../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy). In addition to the default `random` mode there's also `ha_strategy = roundrobin`.

More interesting strategies are those based on latency-weighted probabilities. `noerrors` and `nodeads`: not only those take out mirrors with issues, but also monitor the response times and do balancing. If a mirror responds slower (for example due to some operations running on it), it will receive less requests. When the mirror recovers and provides better times, it will get more requests.


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