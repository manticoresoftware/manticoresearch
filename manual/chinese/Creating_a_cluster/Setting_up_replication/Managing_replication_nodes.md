# 管理复制节点

<!-- example managing replication nodes 1 -->
`ALTER CLUSTER <cluster_name> UPDATE nodes` 语句会更新指定集群中每个节点上的节点列表，以包含集群中的所有活动节点。有关节点列表的更多信息，请参见 [加入集群](../../Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md)。


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
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('ALTER CLUSTER posts UPDATE nodes')
```

<!-- response Python-asyncio -->
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

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("ALTER CLUSTER posts UPDATE nodes", Some(true)).await;
```

<!-- end -->


例如，当集群最初建立时，用于重新加入集群的节点列表是 `10.10.0.1:9312,10.10.1.1:9312`。此后，其他节点加入了集群，现在的活动节点是 `10.10.0.1:9312,10.10.1.1:9312,10.15.0.1:9312,10.15.0.3:9312`。然而，用于重新加入集群的节点列表尚未更新。

为了解决此问题，您可以运行 `ALTER CLUSTER ... UPDATE nodes` 语句，将活动节点列表复制到用于重新加入集群的节点列表中。之后，用于重新加入集群的节点列表将包含集群中的所有活动节点。

两个节点列表都可以通过使用 [集群状态](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md) 语句查看（`cluster_post_nodes_set` 和 `cluster_post_nodes_view`）。

## 从集群中移除节点

要从复制集群中移除节点，请执行以下步骤：
1. 停止该节点
2. 从已停止节点的 `<data_dir>/manticore.json`（通常是 `/var/lib/manticore/manticore.json`）中删除有关集群的信息。
3. 在其他任一节点上运行 `ALTER CLUSTER cluster_name UPDATE nodes`。

完成这些步骤后，其他节点将忘记已分离的节点，已分离的节点也将忘记该集群。此操作不会影响集群中或已分离节点上的表。
<!-- proofread -->

