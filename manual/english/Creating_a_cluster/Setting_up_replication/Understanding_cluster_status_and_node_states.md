# Understanding cluster status and node states

This section provides detailed explanations of the `cluster_%_status` and `cluster_%_node_state` values that appear in cluster status output, including what each combination means in different operational scenarios.

## cluster_%_status values

The `cluster_%_status` variable indicates the cluster component status from this node's perspective. Understanding these statuses is crucial for proper cluster management and troubleshooting.

### PRIMARY status

<!-- example primary status -->
When the cluster status shows `PRIMARY`, the cluster has quorum and can accept both read and write operations. This is the normal operational state.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_status'
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_status       | primary |
+----------------------------+-------+
```

<!-- end -->

A `PRIMARY` status means:
* The majority of nodes in the cluster are connected and communicating
* Write operations are allowed and will be replicated
* The cluster is functioning normally
* Quorum is maintained

### NON_PRIMARY status

<!-- example non_primary status -->
When the cluster status shows `NON_PRIMARY`, the cluster has lost quorum. This is a critical state where write operations are blocked to prevent split-brain scenarios.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_status'
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_status       | non-primary |
+----------------------------+-------+
```

<!-- end -->

A `NON_PRIMARY` status occurs when:
* More than half the nodes in the cluster are unreachable
* Network partitioning has split the cluster
* The current node group cannot form a majority

**Critical**: Write operations are rejected in this state. Manual intervention may be required using the bootstrap command:

<!-- example non_primary recovery -->

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

### DISCONNECTED status

<!-- example disconnected status -->
When the cluster status shows `DISCONNECTED`, the node is not connected to any cluster component and is attempting to reconnect.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_status'
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_status       | disconnected |
+----------------------------+-------+
```

<!-- end -->

A `DISCONNECTED` status typically occurs during:
* Node startup before joining the cluster
* Network connectivity issues
* Cluster formation problems
* Temporary isolation from other cluster members

## cluster_%_node_state values

The `cluster_%_node_state` variable shows the current operational state of this specific node within the cluster.

### synced state

<!-- example synced state -->
When the node state shows `synced`, the node is fully synchronized and operational. This is the normal working state.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_node_state'
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_node_state   | synced |
+----------------------------+-------+
```

<!-- end -->

A `synced` state means:
* The node can serve both read and write requests
* All transactions are being replicated normally
* The node is contributing to cluster quorum
* Data is consistent with other cluster members

### joining state

<!-- example joining state -->
When the node state shows `joining`, the node is in the process of joining the cluster and receiving state synchronization.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_node_state'
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_node_state   | joining |
+----------------------------+-------+
```

<!-- end -->

During the `joining` state:
* The node is receiving cluster state (IST or SST)
* The node cannot serve read or write requests yet
* State transfer is in progress
* The node will transition to `synced` when complete

### donor state

<!-- example donor state -->
When the node state shows `donor`, the node is providing state transfer to a joining node.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_node_state'
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_node_state   | donor |
+----------------------------+-------+
```

<!-- end -->

During the `donor` state:
* The node continues to serve requests but may experience latency
* State transfer (SST) is being provided to another node
* The node returns to `synced` when transfer completes
* Performance may be temporarily impacted

### closed state

<!-- example closed state -->
When the node state shows `closed`, the node is shut down or not yet started.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_node_state'
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_node_state   | closed |
+----------------------------+-------+
```

<!-- end -->

The `closed` state indicates:
* The node has been shut down cleanly
* The node has not been started yet
* This is the initial state before joining a cluster
* Normal state during planned maintenance

### destroyed state

<!-- example destroyed state -->
When the node state shows `destroyed`, the node encountered an error and closed abnormally.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_node_state'
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_node_state   | destroyed |
+----------------------------+-------+
```

<!-- end -->

The `destroyed` state indicates:
* A serious problem occurred requiring investigation
* The node may need manual intervention to restart
* Potential data corruption or system failure
* Review logs for error details before restarting

## Status combinations in different scenarios

### Normal operations

| Scenario | cluster_%_status | cluster_%_node_state | Description |
|----------|------------------|---------------------|-------------|
| Healthy cluster | `primary` | `synced` | Normal operation, can read/write |
| Node starting | `disconnected` → `primary` | `closed` → `joining` → `synced` | Normal startup sequence |
| New node joining | `primary` | `donor` (helper), `joining` (new) | SST in progress |

### Network issues

| Scenario | cluster_%_status | cluster_%_node_state | Action required |
|----------|------------------|---------------------|-----------------|
| Minority partition (2 of 5 nodes) | `non-primary` | `synced` | Wait for network or bootstrap |
| Majority partition (3 of 5 nodes) | `primary` | `synced` | Continue normal operations |
| Complete isolation | `disconnected` | `synced` | Check network connectivity |

### Node failures

| Scenario | cluster_%_status | cluster_%_node_state | Impact |
|----------|------------------|---------------------|--------|
| Single node down (5-node cluster) | `primary` | `synced` | Cluster continues normally |
| Quorum lost (3+ nodes down) | `non-primary` | `synced` | No writes allowed |
| Node crashed | varies | `destroyed` | Node needs restart/recovery |

### Critical failure scenarios

#### Most nodes down (quorum lost)

<!-- example quorum lost scenario -->
When the majority of nodes in your cluster become unavailable, remaining nodes will show `NON_PRIMARY` status and reject write operations.

In a 5-node cluster where 3+ nodes fail:

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_status';
SHOW STATUS LIKE 'cluster_%_node_state';
SHOW STATUS LIKE 'cluster_%_size';
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_status       | non-primary |
| cluster_posts_node_state   | synced |
| cluster_posts_size         | 2 |
+----------------------------+-------+
```

<!-- end -->

To recover from this scenario, identify the node with the highest `last_committed` value and bootstrap it:

<!-- example quorum recovery -->

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_last_committed';
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1;
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_last_committed | 1547 |
+----------------------------+-------+
```

<!-- request JSON -->

```json
POST /cli -d "
SHOW STATUS LIKE 'cluster_%_last_committed';
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1;
"
```

<!-- end -->

#### Network partition (split-brain prevention)

<!-- example network partition -->
In a network partition scenario, only the side with the majority maintains `PRIMARY` status:

**Majority side (3 nodes in 5-node cluster):**

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_status';
SHOW STATUS LIKE 'cluster_%_size';
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_status       | primary |
| cluster_posts_size         | 3 |
+----------------------------+-------+
```

**Minority side (2 nodes in 5-node cluster):**

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_status';
SHOW STATUS LIKE 'cluster_%_size';
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_status       | non-primary |
| cluster_posts_size         | 2 |
+----------------------------+-------+
```

<!-- end -->

### State transitions during common events

#### New node joining (successful)
```
New Node: disconnected/closed → disconnected/joining → primary/synced
Donor Node: primary/synced → primary/donor → primary/synced
Other Nodes: primary/synced (unchanged)
```

#### Network partition recovery
```
Minority Partition: non-primary/synced → primary/synced (when reconnected)
Majority Partition: primary/synced (unchanged throughout)
```

#### Manual recovery after total cluster loss
```
First node: disconnected/closed → primary/synced (after pc.bootstrap=1)
Other nodes: disconnected/closed → primary/joining → primary/synced
```

### Monitoring cluster health

<!-- example monitoring cluster health -->
To monitor cluster health and identify issues, check these key status variables:

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_status';
SHOW STATUS LIKE 'cluster_%_node_state';
SHOW STATUS LIKE 'cluster_%_size';
SHOW STATUS LIKE 'cluster_%_last_committed';
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_status       | primary |
| cluster_posts_node_state   | synced |
| cluster_posts_size         | 3 |
| cluster_posts_last_committed | 1547 |
+----------------------------+-------+
```

<!-- end -->

<!-- example monitoring nodes view -->
To see the difference between configured and visible nodes:

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_nodes_set';
SHOW STATUS LIKE 'cluster_%_nodes_view';
```

<!-- response SQL-->

```sql
+----------------------------+--------------------------------------------------------+
| Counter                    | Value                                                  |
+----------------------------+--------------------------------------------------------+
| cluster_posts_nodes_set    | 10.10.0.1:9312,10.10.1.1:9312,10.10.2.1:9312        |
| cluster_posts_nodes_view   | 10.10.0.1:9312:replication,10.10.1.1:9312:replication |
+----------------------------+--------------------------------------------------------+
```

<!-- end -->

This shows that one node (10.10.2.1:9312) is configured but not currently visible to the cluster, indicating a potential connectivity issue.

## Troubleshooting common issues

### Write operations rejected

<!-- example write rejected -->
When cluster status is `NON_PRIMARY`, write operations will be rejected:

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
INSERT INTO posts:articles VALUES (1, 'test article');
```

<!-- response SQL-->

```sql
ERROR 1064 (42000): cluster posts is not ready, not primary state (replication error)
```

<!-- end -->

**Solution**: Check cluster status and either wait for quorum recovery or manually bootstrap if nodes are permanently unavailable.

### Node stuck in joining state

<!-- example stuck joining -->
If a node remains in `joining` state for extended periods:

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_node_state';
SHOW STATUS LIKE 'cluster_%_local_recv_queue';
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_node_state   | joining |
| cluster_posts_local_recv_queue | 150 |
+----------------------------+-------+
```

<!-- end -->

This may indicate:
* Network bandwidth issues during SST
* Large dataset transfer in progress
* Donor node performance problems

Check the donor node status and network connectivity between nodes.

### Cluster size mismatch

<!-- example size mismatch -->
When `cluster_%_size` doesn't match expected node count:

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS LIKE 'cluster_%_size';
SHOW STATUS LIKE 'cluster_%_nodes_view';
```

<!-- response SQL-->

```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_size         | 2 |
| cluster_posts_nodes_view   | 10.10.0.1:9312:replication,10.10.1.1:9312:replication |
+----------------------------+-------+
```

<!-- end -->

This indicates that some configured nodes are not currently accessible. Check network connectivity and node status on missing nodes.

<!-- proofread -->
