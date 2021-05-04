# Cluster recovery

There are cases of multiple possible scenarios where searchd service got stopped with no node in the cluster being able to serve requests. In these cases someone needs to recover the cluster or part of it. Due to multi-master nature of Galera library used for replication, a cluster is like one logical entity and takes care about each its node and node's data consistency and keeps a cluster's status as a whole. This allows safe writes on multiple nodes at the same time and maintains cluster integrity unlike a traditional asynchronous replication.

Here we take a cluster of nodes A, B and C as an example and consider scenarios where some or all nodes are out of service and what to do to bring them back.

### Case 1

Node A is stopped as usual. The other nodes receive "normal shutdown" message from node A. The cluster size is reduced and a quorum re-calculation is issued.

After node A got started as usual, it joins the cluster nodes. Node A will not serve any write transaction until the join is finished and it's fully synchronized with the cluster. If a writeset cache on a donor node B or C, which is set with a Galera cluster's option [gcache.size](https://galeracluster.com/library/documentation/galera-parameters.html#gcache-size), still has all transactions missed at node A, node A will receive a fast incremental state transfer ([IST](https://galeracluster.com/library/documentation/state-transfer.html#state-transfer-ist)), that is, a transfer of only missed transactions. Otherwise, a snapshot state transfer ([SST](https://galeracluster.com/library/documentation/state-transfer.html#state-transfer-sst)) will start, that is, a transfer of index files.

### Case 2

Nodes A and B are stopped as usual. That is the same situation as in previous case but the cluster's size is reduced to 1 and node C itself forms primary component that allows it to handle write transactions.

Nodes A and B may be started as usual and will join the cluster after the start. Node C becomes a "donor" and provides the transfer of the state to nodes A and B.

### Case 3

All nodes are stopped as usual and the cluster is off.

The problem now is how to initialize cluster and it is important that on a clean shutdown of searchd nodes write the number of the last executed transaction into the cluster directory [grastate.dat](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) file along with the `safe_to_bootstrap` key. The node stopped last will have the `safe_to_bootstrap: 1` option and the most advanced `seqno` number.

It is important that this node should start first to form a cluster. To bootstrap, a cluster server should be started on this node with the [--new-cluster](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) command line key. On Linux you can also run `manticore_new_cluster` which will start Manticore in `--new-cluster` mode via systemd.

If another node starts first and bootstraps the cluster, then the most advanced node joins that cluster, performs full SST and receives an index file where some transactions are missed in comparison with index files it got before. That is why it is important to start the node that shut down last or to look at the cluster directory [grastate.dat](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) file to find the node with the `safe_to_bootstrap: 1` option.

### Case 4

Node A disappears from the cluster due to crash or network failure.

Nodes B and C try to reconnect to missed node A and after failure remove node A from the cluster. The cluster quorum is valid as 2 out of 3 nodes are running and the cluster works as usual.

After node A restarts it will join the cluster automatically the same way as in [Case 1](../../Creating_a_cluster/Setting_up_replication/Cluster_recovery.md#Case-1).

### Case 5

Nodes A and B disappear. Node C is not able to form the quorum alone as 1 node is less than 1.5 (half of 3). So the cluster on node C is switched to non-primary state and node C rejects any write transactions with an error message.

Meanwhile, the single node C is waiting for other nodes to connect and try to connect them itself. If this happens, after the network is restored and nodes A and B are running again, the cluster will be formed again automatically. If nodes A and B are just cut from node C, but they can still reach each other, they keep working as usual because they still form the quorum.

<!-- example case 5 -->
However, if both nodes A and B crashed or restarted due to power outage, someone should turn on primary component on the C node with a following statement:

But someone must make sure that all the other nodes are really unreachable before doing that, otherwise split-brain happens and separate clusters get formed.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1
```
<!-- request HTTP -->

```json
POST /sql -d "mode=raw&query=
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1
"
```
<!-- end -->

### Case 6

All nodes crashed. In this case the [grastate.dat](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) file at cluster directory is not updated and does not contain a valid sequence number `seqno`.

If this happened, someone should find the most advanced node and start the server on it with the [--new-cluster-force](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) command line key. All other nodes will start as usual as in [Case 3](../../Creating_a_cluster/Setting_up_replication/Cluster_recovery.md#Case-3)).
On Linux you can also run `manticore_new_cluster --force`. It will start Manticore in `--new-cluster-force` mode via systemd.

### Case 7

Split-brain causes a cluster to get into non-primary state. For example, the cluster consists of even number of nodes (four), two couple of nodes being located in separate datacenters, and network failure interrupts the connection these datacenters. Split-brain happens as each group of nodes has exactly half of quorum. Both groups stop to handle write transactions as Galera replication model cares about data consistency and the cluster can not accept write transactions without quorum. But nodes in both groups try to re-connect to the nodes nodes from the other group to restore the cluster.

<!-- example case 7 -->
If someone wants to restore the cluster without network got restored the same steps as in [Case 5](../../Creating_a_cluster/Setting_up_replication/Cluster_recovery.md#Case-5) can be done but only at one group of nodes

After that, the group with the node we run this statement at can successfully handle write transactions again.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1
```
<!-- request HTTP -->

```json
POST /sql -d "mode=raw&query=
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1
"
```
<!-- end -->

However, we want to notice that if the statement gets issued at both groups this will end up with two separate clusters made, so the following network restoration will not make the groups to rejoin.
