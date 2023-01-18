# Joining a replication cluster

<!-- example joining a replication cluster 1 -->
To join an existing cluster you should specify at least:
* [name](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster)
* and `host:port` of another working node of the cluster you are joining

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
<!-- end -->

<!-- example joining a replication cluster 1_1 -->
In case of a single replication cluster, i.e. in most cases the above is just enough. In case you are creating multiple replication clusters [path](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) needs to be set as well and the directory should be available.

<!-- request SQL -->
```sql
JOIN CLUSTER c2 at '127.0.0.1:10201' 'c2' as path
```
<!-- end -->

A node joins a cluster by getting data from another specified node and, if successful, it updates node lists in all other cluster nodes similar to how it's done manually via [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md). This list is used to rejoin nodes to the cluster on restart.

There are two lists of nodes:
* `cluster_<name>_nodes_set`: used to rejoin nodes to the cluster  on restart, it is updated across all nodes same way as [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) does. `JOIN CLUSTER` does the same update automatically. [Cluster status](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md) shows this list as `cluster_<name>_nodes_set`.
* `cluster_<name>_nodes_view`: list of all active nodes used for replication. This list doesn't require manual management. [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) actually copies this list of nodes to the list of nodes used to rejoin on restart. [Cluster status](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md) shows this list as `cluster_<name>_nodes_view`.

<!-- example joining a replication cluster  2 -->
When nodes are located in different network segments or in different datacenters, [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) option may be set explicitly. That allows to minimize traffic between nodes and to use gateway nodes for datacenters intercommunication. The following command joins an existing cluster using the [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) option.

> **Note:** that when this syntax is used, `cluster_<name>_nodes_set` list is not updated automatically. Use [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) to update it.


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
<!-- end -->

`JOIN CLUSTER` works synchronously and completes as soon as the node receives all data from the other nodes in the cluster and is in sync with them.
