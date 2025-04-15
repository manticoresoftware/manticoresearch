# 管理复制节点

<!-- example managing replication nodes 1 -->
`ALTER CLUSTER <cluster_name> UPDATE nodes` 语句更新指定集群内每个节点上的节点列表，以包含集群中所有活动节点。有关节点列表的更多信息，请参见 [Joining a cluster](../../Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md)。


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
##### Javascript:

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


举例来说，当集群最初建立时，用于重新加入集群的节点列表是 `10.10.0.1:9312,10.10.1.1:9312`。此后，其他节点加入了集群，现在活动节点是 `10.10.0.1:9312,10.10.1.1:9312,10.15.0.1:9312,10.15.0.3:9312`。然而，用于重新加入集群的节点列表并没有更新。

为了纠正这一点，您可以运行 `ALTER CLUSTER ... UPDATE nodes` 语句，将活动节点的列表复制到用于重新加入集群的节点列表中。在此之后，用于重新加入集群的节点列表将包括集群中所有活动节点。

两个节点列表可以通过 [Cluster status](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md) 语句（`cluster_post_nodes_set` 和 `cluster_post_nodes_view`）查看。

## 从集群中移除节点

从复制集群中移除节点，请按照以下步骤操作：
1. 停止节点
2. 从已停止的节点的 `<data_dir>/manticore.json` 中移除关于集群的信息（通常为 `/var/lib/manticore/manticore.json`）。
3. 在其他任何节点上运行 `ALTER CLUSTER cluster_name UPDATE nodes`。

完成这些步骤后，其他节点将忘记已分离的节点，而已分离的节点将忘记集群。此操作不会影响集群中的表或已分离的节点。
<!-- proofread -->
