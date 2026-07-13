# Cluster recovery

If a replication node or an entire cluster becomes unavailable, the correct recovery procedure depends on how many nodes are still reachable and whether the shutdown was clean or abrupt.

A replication cluster should be treated as one logical system rather than a set of unrelated servers. That gives you multi-master writes and consistent data, but it also means you must recover quorum carefully. In particular, do not run the manual recovery command that restores writes on the surviving side until you are sure the missing nodes are really gone. That command is shown later in this page as `SET CLUSTER <name> GLOBAL 'pc.bootstrap' = 1`. If you run it too early, you can create split-brain and end up with two independent clusters.

For the examples below, assume a cluster with nodes A, B, and C unless noted otherwise.

## Before you recover anything

First identify which situation you are in:

- Is at least one node still online?
- Was the node stopped cleanly, or did it crash? A clean stop means `searchd` was shut down normally and had time to save its replication state before exiting. A crash, power loss, or `kill -9` is not a clean stop.
- Does the surviving part of the cluster still have quorum? Quorum means enough nodes can still see each other to safely remain the writable cluster.
- If all nodes are down, which node should be started first to bring the cluster back?

Useful checks:

- `SHOW STATUS LIKE 'cluster_<name>_status'`
- `SHOW STATUS LIKE 'cluster_<name>_size'`
- `SHOW STATUS LIKE 'cluster_<name>_node_state'`
- if all nodes are down, inspect `grastate.dat`, the small replication state file stored in the cluster data directory. Look especially at `seqno` and `safe_to_bootstrap`: on a clean shutdown, the best node to start first is usually the one with the most advanced `seqno` and `safe_to_bootstrap: 1`. For the full bootstrap procedure, see [Restarting a cluster](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md).

Example of what `grastate.dat` can look like after a clean shutdown:

```text
# saved replication state
version: 2.1
uuid:    <cluster-uuid>
seqno:   12345
safe_to_bootstrap: 1
```

In this example:

- `seqno: 12345` means this node knows about transactions up to sequence number 12345
- `safe_to_bootstrap: 1` means this node is marked as safe to start first

In a clean all-nodes-down recovery, this is usually the kind of node you start first with `--new-cluster` to bring the cluster back.

After recovery, wait until the restarted node reports `cluster_<name>_status=primary` and `cluster_<name>_node_state=synced` before treating it as fully writable again. You can check this with `SHOW STATUS LIKE 'cluster_<name>_status'` and `SHOW STATUS LIKE 'cluster_<name>_node_state'`. In local tests, restarted nodes sometimes spent a short time with `cluster_<name>_node_state=joining` and `cluster_<name>_status=disconnected` before reaching `synced`/`primary`.

### One node was shut down cleanly

If node A is stopped normally, nodes B and C keep serving writes. You can confirm that the cluster is still healthy on those nodes with `SHOW STATUS LIKE 'cluster_<name>_status'` and `SHOW STATUS LIKE 'cluster_<name>_size'`.

When node A starts again, it rejoins the cluster automatically. Until synchronization finishes, do not send writes to that node. Check `SHOW STATUS LIKE 'cluster_<name>_status'` and `SHOW STATUS LIKE 'cluster_<name>_node_state'` and wait for `primary` / `synced`.

If donor nodes B or C still have all the transactions that node A missed in their replication cache, node A can catch up using an incremental state transfer (IST). IST stands for incremental state transfer. It means the node receives only the transactions it missed, so recovery is usually faster and lighter. Otherwise it will require a snapshot state transfer (SST). SST stands for snapshot state transfer. It means copying table files from another node instead of just replaying the missing transactions. SST is heavier: it is usually slower, moves more data, and can make recovery more disruptive on large clusters.

### Two nodes were shut down cleanly and one node stayed online

If nodes A and B are stopped cleanly and node C remains online, node C can continue accepting writes. Check `SHOW STATUS LIKE 'cluster_<name>_status'` and `SHOW STATUS LIKE 'cluster_<name>_size'` on node C if you want to confirm it is now the only active node.

When nodes A and B start again, they rejoin automatically and synchronize from node C. While they are rejoining, check `SHOW STATUS LIKE 'cluster_<name>_status'`, `SHOW STATUS LIKE 'cluster_<name>_node_state'`, and `SHOW STATUS LIKE 'cluster_<name>_size'`. Wait until all nodes show `primary` / `synced` and the expected cluster size before treating recovery as complete.

If nodes A and B were intentionally removed from the cluster, for example with `EXIT CLUSTER`, and node C is now the only persisted node in the cluster, node C can also recover after its own clean restart with a normal daemon start. In that self-only case, `--new-cluster` is not required.

### All nodes were shut down cleanly

If all nodes in a multi-node cluster were stopped normally, the cluster is fully offline and must be started again in a special way so one node can become the first running node of the cluster.

On clean shutdown, each node writes its last transaction number to `grastate.dat`. The node that was stopped last is the safest node to start first:

- it has the most advanced `seqno`
- it has `safe_to_bootstrap: 1`

Start that node with [`--new-cluster`](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md). This tells Manticore to start a new copy of the cluster from that node. If you run Manticore via systemd on Linux, use `manticore_new_cluster`. It starts Manticore in `--new-cluster` mode for you.

After that, start the remaining nodes normally and let them rejoin. Verify recovery with `SHOW STATUS LIKE 'cluster_<name>_status'`, `SHOW STATUS LIKE 'cluster_<name>_node_state'`, and `SHOW STATUS LIKE 'cluster_<name>_size'`.

If you bootstrap a less advanced node first, a more advanced node may later join it and receive a full SST from an older state, which can discard transactions that existed only on the more advanced node. That is why the node with `safe_to_bootstrap: 1` should be your first choice.

### Recreate a cluster from existing local tables

Use this procedure as a last resort when no node can restore the cluster through the normal recovery methods above, but the cluster's tables are still present in the local data directories. This can happen when saved cluster metadata is incomplete or incompatible with the running version, or when another startup failure leaves the old cluster membership unusable.

This procedure creates a new cluster. It discards the old cluster membership and replication history, then copies the tables from one selected node to the other nodes. Do not use it while any node still has a healthy `primary` / `synced` copy of the old cluster. In that case, keep the healthy cluster and recover the other nodes through [JOIN CLUSTER](Joining_a_replication_cluster.md#joining-a-replication-cluster).

1. Stop Manticore on every node that belonged to the failed cluster. Do not write to the local tables during this procedure.
2. Back up the full `data_dir` on every node. Keep a copy of the old cluster descriptor if it contains custom `path`, `nodes`, or provider `options` that you need when recreating the cluster.
3. Choose the node whose local tables contain the data you want to keep. This node will become the source for the new cluster. Adding its tables to the cluster later replaces tables with the same names on the other nodes.
4. On every node, edit `<data_dir>/manticore.json`. From the top-level `clusters` object, remove only the entry for the failed cluster. For example, remove `clusters.posts` for a cluster named `posts`.

   Leave the `indexes` object and every table directory in place. Do not delete or recreate the tables.

5. Start Manticore normally on every node. The former cluster tables should now appear as local tables. Check the tables on the selected source node before continuing. If they are missing or incomplete, stop and restore the backup.
6. On the selected source node, create the cluster. Include any saved `path`, `nodes`, or provider options if you still need them. If authentication is enabled, add `'<replication-user>' AS user` to this statement and the `JOIN CLUSTER` statements below. The account must have matching stored authentication data and `replication` permission on every node. See [setting up replication](Setting_up_replication.md#setting-up-replication).

   ```sql
   CREATE CLUSTER posts
   ```

7. On every other node, join the new cluster through the first node:

   ```sql
   JOIN CLUSTER posts AT 'node-a.example:9312'
   ```

8. On the selected source node, attach the existing local tables to the new cluster:

   ```sql
   ALTER CLUSTER posts ADD table_a, table_b
   ```

9. Wait until every node reports `cluster_posts_status=primary` and `cluster_posts_node_state=synced` before resuming writes.

### One node crashed or became unreachable

If node A disappears because of a crash or a network problem, nodes B and C will first try to reconnect to it. If that fails, they remove it from the cluster, recalculate quorum, and continue working as the primary cluster.

In local tests this peer removal was not immediate: the surviving nodes stayed at the old cluster size for a few seconds before dropping the failed peer and switching to the smaller primary cluster.

When node A is started again, it rejoins automatically and catches up the same way as after a clean one-node shutdown. Again, use `SHOW STATUS LIKE 'cluster_<name>_status'`, `SHOW STATUS LIKE 'cluster_<name>_node_state'`, and `SHOW STATUS LIKE 'cluster_<name>_size'` to confirm recovery is finished.

### Two nodes are gone and only one node is still running

If nodes A and B are lost and only node C is still running, node C no longer has quorum in a three-node cluster. It switches to `non-primary` and rejects writes.

The write error is explicit:

```sql
ERROR 1064 (42000): cluster '<name>' is not ready, not primary state (synced)
```

If nodes A and B are only temporarily disconnected but can still see each other, they may continue accepting writes while node C remains isolated. Use `SHOW STATUS LIKE 'cluster_<name>_status'` on each side if you need to see which side is still writable.

If nodes A and B really crashed and node C is the only surviving copy you want to keep working with, run this command on node C to make it writable again:

<!-- example bootstrap-primary -->
If you have confirmed that the other nodes are truly offline, run:

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

Important:

- run this only after you are sure the other nodes are unreachable
- run it only on the side that must survive
- after bootstrapping, the node can accept writes again and the other nodes can later rejoin from it

### All nodes crashed

If every node crashed, `grastate.dat` is typically no longer trustworthy for normal bootstrap selection. In local tests, all nodes showed:

- `seqno: -1`
- `safe_to_bootstrap: 0`

In this situation, choose the node with the most recent data and start it with [`--new-cluster-force`](../../Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md). This forces Manticore to start a new copy of the cluster from that node even though the usual clean-shutdown metadata is not trustworthy. If you run Manticore via systemd on Linux, use `manticore_new_cluster --force`. It starts Manticore in `--new-cluster-force` mode for you.

Then start the remaining nodes normally and let them rejoin. Verify recovery with `SHOW STATUS LIKE 'cluster_<name>_status'`, `SHOW STATUS LIKE 'cluster_<name>_node_state'`, and `SHOW STATUS LIKE 'cluster_<name>_size'`.

### An even-sized cluster lost quorum, for example after a split

Split-brain risk is highest in even-sized clusters. For example, imagine four nodes split into two isolated pairs across two data centers. Each side has exactly half of the original members, so neither side has quorum and both sides stop accepting writes.

If you must restore writes before connectivity is fixed, choose only one side of the split and run the same recovery command there so that side becomes writable again. Before doing that, check `SHOW STATUS LIKE 'cluster_<name>_status'` on both sides so you know which side is currently non-primary.

<!-- example bootstrap-primary-split -->
Choose the side that should remain the writable cluster, then run:

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

Never issue that statement on both sides. If you do, you will create two separate primary clusters, and they will not merge back automatically when the network recovers.

In local testing, abruptly losing half of a four-node cluster reproduced the same `non-primary` behavior and the same recovery command brought one surviving half back to `primary`.
<!-- proofread -->
