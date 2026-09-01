# Understanding cluster status and node states

## Quick reference

The cluster status shows two key values that help you understand your cluster's health:

- `cluster_%_status` - whether the cluster can accept writes
- `cluster_%_node_state` - what this specific node is doing

## Status values

<!-- example cluster status values -->

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
SHOW STATUS LIKE 'cluster_%_status';
SHOW STATUS LIKE 'cluster_%_node_state';
```

<!-- response SQL-->
```sql
+----------------------------+-------+
| Counter                    | Value |
+----------------------------+-------+
| cluster_posts_status       | primary |
| cluster_posts_node_state   | synced |
+----------------------------+-------+
```

<!-- end -->

### cluster_%_status

- **`primary`** - Normal operation, can read and write
- **`non-primary`** - Lost quorum, **writes blocked**
- **`disconnected`** - Node isolated, trying to reconnect

### cluster_%_node_state  

- **`synced`** - Normal operation
- **`joining`** - Node joining cluster (temporary)
- **`donor`** - Helping another node join (temporary)
- **`closed`** - Node stopped
- **`destroyed`** - Node crashed, needs restart

## Common scenarios

| What happened | cluster_%_status | cluster_%_node_state | What to do |
|---------------|------------------|---------------------|------------|
| Normal operation | `primary` | `synced` | Nothing |
| Most nodes down | `non-primary` | `synced` | Bootstrap or wait |
| Node starting | `disconnected` | `joining` → `synced` | Wait |
| Node crashed | varies | `destroyed` | Restart node |
| Network partition (majority) | `primary` | `synced` | Continue normally |
| Network partition (minority) | `non-primary` | `synced` | Wait or bootstrap |

## Recovery from quorum loss

<!-- example quorum recovery -->
When most nodes are down and status shows `non-primary`:

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
-- Check which node has highest sequence number
SHOW STATUS LIKE 'cluster_%_last_committed';
-- Bootstrap the cluster (run on best node only)
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
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1
"
```

<!-- end -->

<!-- proofread -->
