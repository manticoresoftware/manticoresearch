# Cluster recovery

There may be cases when Mantcore search daemon gets stopped with no node in the cluster left to serve requests. In these cases someone needs to recover the cluster or a part of it. Due to multi-master nature of Galera library used for replication, Manticore replication cluster constitutes one logical entity and takes care about each of its nodes and node's data consistency and keeps the cluster's status as a whole. This enables safe writes on multiple nodes at the same time and maintains cluster integrity unlike a traditional asynchronous replication.

However it comes with its challenges. Below let's look at what will happen in this or that case. For example let's take a cluster of nodes A, B and C and consider scenarios where some or all nodes get out of service and what one has to do to bring them back.

### Case 1

Node A is stopped as usual. The other nodes receive "normal shutdown" message from node A. The cluster size is reduced and a quorum re-calculation is issued.

After node A gets started as usual, it joins the cluster nodes. Node A will not serve any write transaction until the join is complete and it's fully synchronized with the cluster. If a writeset cache on donor node B or C (which can be controlled with a Galera cluster's option [gcache.size](https://galeracluster.com/library/documentation/galera-parameters.html#gcache-size)) still has all transactions missed at node A, node A will receive a fast incremental state transfer ([IST](https://galeracluster.com/library/documentation/state-transfer.html#state-transfer-ist)), that is, a transfer of only missed transactions. Otherwise, a snapshot state transfer ([SST](https://galeracluster.com/library/documentation/state-transfer.html#state-transfer-sst)) will start, that is, a transfer of table files.

### Case 2

Nodes A and B are stopped as usual. That is the same situation as in the previous case but the cluster's size is reduced to 1 and node C itself forms a primary component that allows it to handle write transactions.

Nodes A and B may be started as usual and will join the cluster after the start. Node C becomes a "donor" and provides the transfer of the state to nodes A and B.

### Case 3

All nodes are stopped as usual and the cluster is off.

The problem now is how to initialize the cluster. It's important that on a clean shutdown of searchd the nodes write the number of last executed transaction into the cluster directory [grastate.dat](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) file along with flag `safe_to_bootstrap`. The node which was stopped last will have option `safe_to_bootstrap: 1` and the most advanced `seqno` number.

It is important that this node starts first to form the cluster. To bootstrap a cluster the server should be started on this node with flag [--new-cluster](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md). On Linux you can also run `manticore_new_cluster` which will start Manticore in `--new-cluster` mode via systemd.

If another node starts first and bootstraps the cluster, then the most advanced node joins that cluster, performs full SST and receives a table file where some transactions are missed in comparison with the table files it got before. That is why it is important to start first the node which was shut down last, it should have flag `safe_to_bootstrap: 1` in [grastate.dat](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md).

### Case 4

Node A disappears from the cluster due to a crash or a network failure.

Nodes B and C try to reconnect to the missed node A and after a failure remove node A from the cluster. The cluster quorum is valid as 2 out of 3 nodes are running and the cluster keep working as usual.

After node A is restarted it will join the cluster automatically the same way as in [Case 1](../../Creating_a_cluster/Setting_up_replication/Cluster_recovery.md#Case-1).

### Case 5

Nodes A and B disappear. Node C is not able to form the quorum alone as 1 node is less than 1.5 (half of 3). So the cluster on node C is switched to non-primary state and node C rejects any write transactions with an error message.

Meanwhile, the single node C is waiting for other nodes to connect and try to connect to them itself. If it happens, after the network is restored and nodes A and B are running again, the cluster will be formed again automatically. If nodes A and B are just cut from node C, but they can still reach each other, they keep working as usual because they still form the quorum.

<!-- example case 5 -->
However, if both nodes A and B crashed or restarted due to power outage, someone should turn on primary component on the C node with the following statement:


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1
```
<!-- request HTTP -->

```json
POST /cli -d "
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1
"
```
<!-- end -->

But before doing that you need to make sure that the other nodes are really unreachable, otherwise split-brain happens and separate clusters get formed.

### Case 6

All nodes crashed. In this case the [grastate.dat](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) file in the cluster directory is not updated and does not contain a valid sequence number `seqno`.

If this happens, someone should find the most advanced node and start the server on it with the [--new-cluster-force](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) command line key. All other nodes will start as usual as in [Case 3](../../Creating_a_cluster/Setting_up_replication/Cluster_recovery.md#Case-3)).
On Linux you can also run `manticore_new_cluster --force`. It will start Manticore in `--new-cluster-force` mode via systemd.

### Case 7

Split-brain causes the cluster to get into non-primary state. For example, the cluster consists of even number of nodes (four), e.g. two couple of nodes located in different datacenters, and network failure interrupts the connection between the datacenters. Split-brain happens as each group of nodes has exactly half of the quorum. The both groups stop handling write transactions as Galera replication model cares about data consistency and the cluster cannot accept write transactions without quorum. But nodes in the both groups try to re-connect to the nodes from the other group to restore the cluster.

<!-- example case 7 -->
If someone wants to restore the cluster without network got restored the same steps as in [Case 5](../../Creating_a_cluster/Setting_up_replication/Cluster_recovery.md#Case-5) should be done, but only at one group of the nodes.

After that, the group with the node we run this statement at can successfully handle write transactions again.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1
```
<!-- request HTTP -->

```json
POST /cli -d "
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1
"
```
<!-- end -->

However, we want to notice that if the statement gets issued at both groups it will result in two separate clusters, so the following network recovery will not make the groups to rejoin.
