# Managing replication nodes 

<!-- example managing replication nodes 1 -->
The `ALTER CLUSTER <cluster_name> UPDATE nodes`  statement updates the node lists on each node within the specified cluster to include all active nodes in the cluster. For more information on node lists, see [Joining a cluster](../../Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md).


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
ALTER CLUSTER posts UPDATE nodes
```

<!-- request JSON -->

```json
POST /cli -d "
ALTER CLUSTER posts UPDATE nodes
"
```

<!-- request PHP -->

```php
$params = [
  'cluster' => 'posts',
  'body' => [
     'operation' => 'update',
     
  ]
];
$response = $client->cluster()->alter($params); 
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('ALTER CLUSTER posts UPDATE nodes')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('ALTER CLUSTER posts UPDATE nodes');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("ALTER CLUSTER posts UPDATE nodes");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("ALTER CLUSTER posts UPDATE nodes");
```
<!-- end -->


For instance, when the cluster was initially established, the list of nodes used to rejoin the cluster was `10.10.0.1:9312,10.10.1.1:9312`. Since then, other nodes joined the cluster and now the active nodes are `10.10.0.1:9312,10.10.1.1:9312,10.15.0.1:9312,10.15.0.3:9312`.However, the list of nodes used to rejoin the cluster has not been updated.

To rectify this, you can run the `ALTER CLUSTER ... UPDATE nodes` statement to copy the list of active nodes to the list of nodes used to rejoin the cluster. After this, the list of nodes used to rejoin the cluster will include all the active nodes in the cluster. 

Both lists of nodes can be viewed using the [Cluster status](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md) statement (`cluster_post_nodes_set` and `cluster_post_nodes_view`).

## Removing node from cluster

To remove a node from the replication cluster, follow these steps:
1. Stop the node
2. Remove the information about the cluster from  `<data_dir>/manticore.json` (usually `/var/lib/manticore/manticore.json`) on the node that has been stopped.
3. Run `ALTER CLUSTER cluster_name UPDATE nodes` on any other node.

After these steps, the other nodes will forget about the detached node and the detached node will forget about the cluster. This action will not impact the tables in the cluster or on the detached node.
<!-- proofread -->