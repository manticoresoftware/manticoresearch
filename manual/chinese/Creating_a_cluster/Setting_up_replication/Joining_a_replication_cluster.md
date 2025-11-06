# 加入复制集群

<!-- example joining a replication cluster 1 -->
要加入现有集群，您必须至少指定：
* 集群的 [名称](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster)
* 您要加入的集群中另一个节点的 `host:port`

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

<!-- example joining a replication cluster 1_1 -->
在大多数情况下，当只有一个复制集群时，上述内容是足够的。然而，如果您正在创建多个复制集群，您还必须设置 [路径](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster)，并确保该目录可用。

<!-- request SQL -->
```sql
JOIN CLUSTER c2 at '127.0.0.1:10201' 'c2' as path
```
<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "JOIN CLUSTER c2 at '127.0.0.1:10201' 'c2' as path"
```
<!-- end -->

节点通过从指定节点获取数据来加入集群，如果成功，则以与通过 [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) 手动完成相同的方式更新所有其他集群节点的节点列表。此列表用于在重启时重新加入集群。

有两个节点列表：
1.`cluster_<name>_nodes_set`：用于在重启时重新加入集群。它在所有节点之间以与 [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) 相同的方式进行更新。 `JOIN CLUSTER` 命令自动执行此更新。[集群状态](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md)将此列表显示为 `cluster_<name>_nodes_set`。
2. `cluster_<name>_nodes_view`：此列表包含所有用于复制的活动节点，不需要手动管理。[ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) 实际上会将此节点列表复制到用于在重启时重新加入的节点列表中。[集群状态](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md)将此列表显示为 `cluster_<name>_nodes_view`。

<!-- example joining a replication cluster  2 -->
当节点位于不同的网络段或数据中心时，可以显式设置 [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) 选项。这可以最小化节点之间的流量，并利用网关节点在数据中心之间进行互通。以下代码使用 [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) 选项加入现有集群。

> **注意：** 使用此语法时，集群 `cluster_<name>_nodes_set` 列表不会自动更新。要更新它，请使用 [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md)。


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

`JOIN CLUSTER` 命令以同步方式工作，并在节点从集群中的其他节点接收所有数据并与它们同步时完成。

`JOIN CLUSTER` 操作可能会失败，并提供指示重复的 [server_id](../../Server_settings/Searchd.md#server_id) 的错误消息。当加入的节点与集群中现有节点的 `server_id` 相同时，会发生此情况。要解决此问题，请确保复制集群中的每个节点都有一个唯一的 [server_id](../../Server_settings/Searchd.md#server_id)。您可以在配置文件的 "searchd" 部分将默认 [server_id](../../Server_settings/Searchd.md#server_id) 更改为唯一值，然后再尝试加入集群。
<!-- proofread -->

