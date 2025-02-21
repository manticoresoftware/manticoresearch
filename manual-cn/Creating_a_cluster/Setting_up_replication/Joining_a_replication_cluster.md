# 加入复制集群

<!-- example joining a replication cluster 1 -->

要加入现有集群，您必须至少指定：

- 集群的 [name](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#复制集群)
- 要加入的集群中另一个节点的 `host:port`

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

<!-- end -->

<!-- example joining a replication cluster 1_1 -->
在大多数情况下，当只有一个复制集群时，上述设置已足够。然而，如果您要创建多个复制集群，还必须设置 [path](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#复制集群) 选项，并确保该目录可用。

<!-- request SQL -->
```sql
JOIN CLUSTER c2 at '127.0.0.1:10201' 'c2' as path
```
<!-- end -->

节点通过从指定节点获取数据来加入集群，如果成功，它会更新所有其他集群节点的节点列表，方式与通过 [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) 手动完成的方式相同。该列表用于在重启时重新加入节点到集群。

有两个节点列表：
1.`cluster_<name>_nodes_set`：用于在重启时重新加入节点到集群。它在所有节点之间以与 [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) 相同的方式进行更新。`JOIN CLUSTER` 命令会自动执行此更新。[集群状态](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md) 显示此列表为 `cluster_<name>_nodes_set`。

2. `cluster_<name>_nodes_view`：该列表包含所有用于复制的活动节点，不需要手动管理。[ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) 实际上会将此节点列表复制到用于重启时重新加入的节点列表中。[集群状态](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md) 显示此列表为 `cluster_<name>_nodes_view`。

<!-- example joining a replication cluster  2 -->
当节点位于不同的网络段或数据中心时，可以明确设置 [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#复制集群) 选项。这可以最小化节点之间的流量，并利用网关节点在数据中心之间进行通信。以下代码示例使用 [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#复制集群) 选项加入现有集群。

> **注意：** 使用此语法时，集群的 `cluster_<name>_nodes_set` 列表不会自动更新。要更新它，请使用 [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md)。


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
<!-- end -->

`JOIN CLUSTER` 命令是同步工作的，并在节点从集群中的其他节点接收所有数据并与之同步后立即完成。
<!-- proofread -->
