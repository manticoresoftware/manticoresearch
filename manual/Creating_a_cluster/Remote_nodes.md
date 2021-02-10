# Adding a remote distributed index

Please read [distributed index](../Creating_an_index/Creating_a_distributed_index/Creating_a_distributed_index.md). Here we will just provide an example.

<!-- example conf dist 1 -->
Here we have split the data over 4 servers, each serving one of the shards. If one of the servers fails, our distributed index will still work, but we would miss the results from the failed shard.


<!-- intro -->
##### ini:

<!-- request ini -->
```ini
index mydist {
          type  = distributed
          agent = box1:9312:shard1
          agent = box2:9312:shard2
          agent = box3:9312:shard3
          agent = box4:9312:shard4
}
```
<!-- end -->

<!-- example conf dist 2 -->
Now we added mirrors, each shard is found on 2 servers. By default, the master (the searchd instance with the distributed index) will pick randomly one of the mirrors.

The mode used for picking mirrors can be set with [ha_strategy](../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy). In addition to random there's also `ha_strategy = roundrobin`.

More interesting strategies are the latency-weighted probabilities based ones. noerrors and nodeads not only that take out mirrors with issues, but also monitor the response times and do a balancing. If a mirror responds slower (for example due to some operations running on it), it will receive less requests. When the mirror recovers and provides better times, it will get more requests.


<!-- intro -->
##### ini:

<!-- request ini -->
```ini
index mydist {
          type  = distributed
          agent = box1:9312|box5:9312:shard1
          agent = box2:9312:|box6:9312:shard2
          agent = box3:9312:|box7:9312:shard3
          agent = box4:9312:|box8:9312:shard4
}
```
<!-- end -->