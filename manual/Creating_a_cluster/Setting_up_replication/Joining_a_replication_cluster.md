# Joining a replication cluster

<!-- example joining a replication cluster 1 -->
To join an existing cluster, you must specify at least:
* The [name](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster)of the cluster
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
##### javascript:

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

<!-- end -->

<!-- example joining a replication cluster 1_1 -->
In most cases, the above is sufficient when there is a single replication cluster. However, if you are creating multiple replication clusters, you must also set the [path](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) and ensure that the directory is available.

<!-- request SQL -->
```sql
JOIN CLUSTER c2 at '127.0.0.1:10201' 'c2' as path
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
##### javascript:

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
<!-- end -->

The `JOIN CLUSTER` command works synchronously and completes as soon as the node receives all data from the other nodes in the cluster and is in sync with them.
<!-- proofread -->