# Cluster recovery

In the event that the Manticore search daemon stops with no remaining nodes in the cluster to serve requests, recovery is necessary. Due to the multi-master nature of the Galera library used for replication, Manticore replication cluster is a single logical entity that maintains the consistency of its nodes and data, and the status of the entire cluster. This allows for safe writes on multiple nodes simultaneously and ensures the integrity of the cluster.

However, this also poses challenges. Let's examine several scenarios, using a cluster of nodes A, B, and C, to see what needs to be done when some or all nodes become unavailable.

### Case 1

 When node A is stopped, the other nodes receive a "normal shutdown" message. The cluster size is reduced, and a quorum re-calculation takes place. 
 
Upon starting node A, it joins the cluster and will not serve any write transactions until it is fully synchronized with the cluster. If the writeset cache on donor nodes B or C (which can be controlled with the Galera cluster's [gcache.size](https://galeracluster.com/library/documentation/galera-parameters.html#gcache-size)) still contains all of the transactions missed at node A, node A will receive a fast incremental state transfer ([IST](https://galeracluster.com/library/documentation/state-transfer.html#state-transfer-ist)), that is, a transfer of only missed transactions. If not, a snapshot state transfer  ([SST](https://galeracluster.com/library/documentation/state-transfer.html#state-transfer-sst)) will occur, which involves the transfer of table files.

### Case 2

In the scenario where nodes A and B are stopped, the cluster size is reduced to one, with node C forming the primary component to handle write transactions. 

Nodes A and B can then be started as usual and will join the cluster after start-up. Node C acts as the donor, providing the state transfer to nodes A and B.

### Case 3

All nodes are stopped as usual and the cluster is off.

The problem now is how to initialize the cluster. It's important that on a clean shutdown of searchd the nodes write the number of last executed transaction into the cluster directory [grastate.dat](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) file along with flag `safe_to_bootstrap`. The node which was stopped last will have option `safe_to_bootstrap: 1` and the most advanced `seqno` number.

It is important that this node starts first to form the cluster. To bootstrap a cluster the server should be started on this node with flag [--new-cluster](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md). On Linux you can also run `manticore_new_cluster` which will start Manticore in `--new-cluster` mode via systemd.

If another node starts first and bootstraps the cluster, then the most advanced node joins that cluster, performs full SST and receives a table file where some transactions are missed in comparison with the table files it got before. That is why it is important to start first the node which was shut down last, it should have flag `safe_to_bootstrap: 1` in [grastate.dat](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md).

### Case 4

In the event of a crash or network failure causing Node A to disappear from the cluster, nodes B and C will attempt to reconnect with Node A. Upon failure, they will remove Node A from the cluster. With two out of the three nodes still running, the cluster maintains its quorum and continues to operate normally.

When Node A is restarted, it will join the cluster automatically, as outlined in [Case 1](../../Creating_a_cluster/Setting_up_replication/Cluster_recovery.md#Case-1).

### Case 5

Nodes A and B have gone offline. Node C is unable to form a quorum on its own as 1 node is less than half of the total nodes (3). As a result, the cluster on node C is shifted to a non-primary state and rejects any write transactions with an error message.

Meanwhile, node C waits for the other nodes to connect and also tries to connect to them. If this happens, and the network is restored and nodes A and B are back online, the cluster will automatically reform. If nodes A and B are just temporarily disconnected from node C but can still communicate with each other, they will continue to operate as normal, as they still form the quorum.

<!-- example case 5 -->
However, if both nodes A and B have crashed or restarted due to a power failure, someone must activate the primary component on node C using the following command:

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1
```
<!-- request JSON -->

```json
POST /cli -d "
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1
"
```
<!-- end -->

t's important to note that before executing this command, you must confirm that the other nodes are truly unreachable. Otherwise, a split-brain scenario may occur and separate clusters may form.

### Case 6

All nodes have crashed. In this situation, the [grastate.dat](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) file in the cluster directory has not been updated and does not contain a valid `seqno`sequence number.

If this occurs, someone needs to locate the node with the most recent data and start the server on it using the [--new-cluster-force](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md) command line key. All other nodes will start as normal, as described in [Case 3](../../Creating_a_cluster/Setting_up_replication/Cluster_recovery.md#Case-3)).
On Linux, you can also use the `manticore_new_cluster --force`, command, which will start Manticore in `--new-cluster-force` mode via systemd.

### Case 7

Split-brain can cause the cluster to transition into a non-primary state. For example, consider a cluster comprised of an even number of nodes (four), such as two pairs of nodes located in different data centers. If a network failure interrupts the connection between the data centers, split-brain occurs as each group of nodes holds exactly half of the quorum. As a result, both groups stop handling write transactions, since the Galera replication model prioritizes data consistency, and the cluster cannot accept write transactions without a quorum. However, nodes in both groups attempt to reconnect with the nodes from the other group in an effort to restore the cluster.

<!-- example case 7 -->
If someone wants to restore the cluster before the network is restored, the same steps outlined in [Case 5](../../Creating_a_cluster/Setting_up_replication/Cluster_recovery.md#Case-5) hould be taken, but only at one group of nodes. 

After the statement is executed, the group with the node that it was run on will be able to handle write transactions once again.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1
```
<!-- request JSON -->

```json
POST /cli -d "
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1
"
```
<!-- end -->

However, it's important to note that if the statement is issued at both groups, it will result in the formation of two separate clusters, and the subsequent network recovery will not result in the groups rejoining.
<!-- proofread -->