# Добата табли с уда агents

Для понима как добата табли с уда агents, важно first иметь базовое understanding о [distributed tables](../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md) In this article, we will focus на как использовать distributed table как basis для созда кластер Мanticore instances.
<!-- example conf dist 1 -->
Here is example как split data over 4 servers, each serving one of shards:


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
In event server failure, distributed table will still work, but results from failed shard will be missing.

<!-- example conf dist 2 -->
Now that we've added mirrors, each shard is found on 2 servers. By default, master (searchd instance with distributed table) will randomly pick one of mirrors.

Mode used for picking mirrors can be set using [ha_strategy](../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) setting. In addition to default `random` mode there's also `ha_strategy = roundrobin`.

More advanced strategies based on latency-weighted probabilities include `noerrors` and `nodeads`. These not only take out mirrors with issues but also monitor response times and do balancing. If mirror responds slower (for example, due to some operations running on it), it will receive fewer requests. When mirror recovers and provides better times, it will receive more requests.

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

