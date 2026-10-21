# Joining a replication cluster

<!-- example joining a replication cluster 1 -->
To join an existing cluster, you must specify at least:
* The [name](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) of the cluster
* The `host:port` of another node in the cluster you are joining

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
JOIN CLUSTER posts AT '10.12.1.35:9312'
```

<!-- request JSON -->

```json
POST /cli -d "
JOIN CLUSTER posts AT '10.12.1.35:9312'
"
```

<!-- request PHP -->

```php
$params = [
  'cluster' => 'posts',
  'body' => [
      '10.12.1.35:9312'
  ]
];
$response = $client->cluster->join($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('JOIN CLUSTER posts AT \'10.12.1.35:9312\'')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('JOIN CLUSTER posts AT \'10.12.1.35:9312\'')
```

<!-- response Python-asyncio -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('JOIN CLUSTER posts AT \'10.12.1.35:9312\'');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("JOIN CLUSTER posts AT '10.12.1.35:9312'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("JOIN CLUSTER posts AT '10.12.1.35:9312'");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("JOIN CLUSTER posts AT '10.12.1.35:9312'", Some(true)).await;
```


<!-- end -->

If [authentication and authorization](../../Security/Authentication_and_authorization.md) is enabled, the effective replication user must have `replication` permission. You can specify that user in the `JOIN CLUSTER` statement:

```sql
GRANT replication ON 'posts' TO 'repl_user';
JOIN CLUSTER posts AT '10.12.1.35:9312' 'repl_user' AS user;
```

The joining node and donor nodes must have the same user with matching stored authentication data. Creating the same user name and password independently on each node is not enough, because the stored authentication data can differ.

If no user is specified, the current session user is used for the join operation. After a successful join, the stored cluster user is taken from the donor cluster metadata.

> NOTE: When authentication is enabled, a successful `JOIN CLUSTER` replaces all local authentication data on the joining node with the donor cluster's authentication data. If authentication logging is at `info` or higher, Manticore writes the previous local auth data to `searchd.log.auth` as a JSON backup before replacement. This backup includes salts and credential hashes, so keep the auth log private and redact it before sharing. If `JOIN CLUSTER` cannot fetch the donor user, verify the replication user and matching auth data on donor nodes, and check `searchd.log.auth` on the donor nodes for API authentication failures.

<!-- example joining a replication cluster 1_1 -->
In most cases, the above is sufficient when there is a single replication cluster. However, if you are creating multiple replication clusters, you must also set the [path](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) and ensure that the directory is available.

<!-- request SQL -->
```sql
JOIN CLUSTER c2 at '127.0.0.1:10201' 'c2' as path
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "JOIN CLUSTER c2 at '127.0.0.1:10201' 'c2' as path"
```
<!-- end -->

A node joins a cluster by obtaining data from a specified node and, if successful, updates the node lists across all other cluster nodes in the same way as if it was done manually through [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md). This list is used to re-join nodes to the cluster upon restart.

There are two lists of nodes:
1.`cluster_<name>_nodes_set`: used to re-join nodes to the cluster upon restart. It is updated across all nodes in the same way as [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) does. `JOIN CLUSTER` command performs this update automatically. The [Cluster status](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md) displays this list as `cluster_<name>_nodes_set`.
2. `cluster_<name>_nodes_view`:  this list contains all active nodes used for replication and does not require manual management. [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) actually copies this list of nodes to the list of nodes used to re-join upon restart. The [Cluster status](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md) displays this list as `cluster_<name>_nodes_view`.

<!-- example joining a replication cluster  2 -->
When nodes are located in different network segments or data centers, the [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) option may be set explicitly. This minimizes traffic between nodes and utilizes gateway nodes for intercommunication between data centers. The following code joins an existing cluster using the [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) option.

> **Note:** The cluster `cluster_<name>_nodes_set` list is not updated automatically when this syntax is used. To update it, use [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md).


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
JOIN CLUSTER click_query 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312' as nodes
```

<!-- request JSON -->

```json
POST /cli -d "
JOIN CLUSTER click_query 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312' as nodes
"
```

<!-- request PHP -->

```php
$params = [
  'cluster' => 'posts',
  'body' => [
      'nodes' => 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312'
  ]
];
$response = $client->cluster->join($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('JOIN CLUSTER click_query \'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312\' as nodes')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('JOIN CLUSTER click_query \'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312\' as nodes')
```

<!-- response Python-asyncio -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('JOIN CLUSTER click_query \'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312\' as nodes');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("JOIN CLUSTER click_query 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312' as nodes");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("JOIN CLUSTER click_query 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312' as nodes");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("JOIN CLUSTER click_query 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312' as nodes", Some(true)).await;
```

<!-- end -->

The `JOIN CLUSTER` command works synchronously and completes as soon as the node receives all data from the other nodes in the cluster and is in sync with them.

The `JOIN CLUSTER` operation can fail with an error message indicating a duplicate [server_id](../../Server_settings/Searchd.md#server_id). This occurs when the joining node has the same `server_id` as an existing node in the cluster. To resolve this issue, ensure that each node in the replication cluster has a unique [server_id](../../Server_settings/Searchd.md#server_id). You can change the default [server_id](../../Server_settings/Searchd.md#server_id) in the "searchd" section of your configuration file to a unique value before attempting to join the cluster.

Before any data is transferred, every node of the cluster checks that it can reach the joining node at its [node_address](../../Server_settings/Searchd.md#node_address) and API port, because the node that sends the data connects to the joining node there. If a node answers that it cannot, for example because it cannot resolve the joining node's name, `JOIN CLUSTER` fails with that node's error and the cluster is not affected. The check is repeated with growing pauses based on [replication_retry_delay](../../Server_settings/Searchd.md#replication_retry_delay) before it gives up. Nodes that do not answer the check, for example because they are restarting, are skipped. The same check runs when a node rejoins its cluster on startup. A node in the cluster's node list whose name no longer resolves, for example a removed container, is skipped too: joins, restarts and other cluster operations use the other nodes, and a join does not start a new cluster.

When the joining node needs a full copy of the cluster's tables (SST), Galera picks one of the synced nodes as the donor, and that node's `cluster_<name>_node_state` shows `donor`. The donor is not necessarily the node named in `AT`. While the donor sends the table files, writes to the cluster's tables on the donor wait until the transfer is finished. They are not rejected, so clients see them as slow and may hit their own timeouts. The donor also applies writes from the other nodes only after the transfer. Writes on the other nodes pause only briefly while the joining node enters the cluster, and reads are not blocked on any node. The wait on the donor grows with the size of the cluster's tables; you can follow the transfer with [cluster_<name>_sst_total](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md#SST-Progress-Metrics).

You cannot choose the donor, and it can be the node your clients write to. To keep writes going while a node joins, send them to the other nodes while any node reports `donor`, for example by making your load balancer's health check treat `donor` as unavailable for writes, and add nodes when the write load is low. A node that restarts after missing only recent transactions catches up with an incremental state transfer (IST) instead: no table files are copied and writes on the donor do not wait. IST is possible while the donor's write-set cache (`gcache.size` in the cluster [options](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#options)) still holds every transaction the node missed.
<!-- proofread -->
