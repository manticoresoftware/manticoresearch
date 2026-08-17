# 加入复制集群

<!-- example joining a replication cluster 1 -->
要加入一个现有集群，您必须至少指定：
* 集群的[名称](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster)
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

如果启用了[身份验证和授权](../../Security/Authentication_and_authorization.md)，有效的复制用户必须具备 `replication` 权限。你可以在 `JOIN CLUSTER` 语句中指定该用户：

```sql
GRANT replication ON 'posts' TO 'repl_user';
JOIN CLUSTER posts AT '10.12.1.35:9312' 'repl_user' AS user;
```

加入节点和捐赠节点必须使用同一个用户，且其存储的身份验证数据必须匹配。仅仅在每个节点上分别创建相同的用户名和密码还不够，因为存储的身份验证数据可能不同。

如果未指定用户，则会使用当前会话用户执行加入操作。成功加入后，存储的集群用户将从捐赠集群元数据中获取。

> 注意：启用身份验证时，成功执行 `JOIN CLUSTER` 会用捐赠集群的身份验证数据替换加入节点上的所有本地身份验证数据。如果身份验证日志级别为 `info` 或更高，Manticore 会在替换前将之前的本地认证数据以 JSON 备份形式写入 `searchd.log.auth`。这份备份包含盐值和凭据哈希，因此请妥善保管认证日志，不要公开分享，分享前务必脱敏。如果 `JOIN CLUSTER` 无法获取捐赠方用户，请检查捐赠节点上的复制用户和匹配的认证数据，并查看捐赠节点上的 `searchd.log.auth`，确认是否存在 API 身份验证失败。

<!-- example joining a replication cluster 1_1 -->
在大多数情况下，当只有一个复制集群时，上述信息就足够了。但是，如果您要创建多个复制集群，则还必须设置[路径](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster)并确保目录可用。

<!-- request SQL -->
```sql
JOIN CLUSTER c2 at '127.0.0.1:10201' 'c2' as path
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "JOIN CLUSTER c2 at '127.0.0.1:10201' 'c2' as path"
```
<!-- end -->

节点通过从指定节点获取数据来加入集群，如果成功，它会更新所有其他集群节点的节点列表，就像通过[ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md)手动完成一样。此列表用于在重启时将节点重新加入集群。

节点列表有两种：
1.`cluster_<name>_nodes_set`：用于在重启时将节点重新加入集群。它在所有节点上更新，方式与[ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md)相同。`JOIN CLUSTER`命令会自动执行此更新。[集群状态](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md)将此列表显示为`cluster_<name>_nodes_set`。
2. `cluster_<name>_nodes_view`：此列表包含所有用于复制的活动节点，无需手动管理。[ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md)实际上将此节点列表复制到用于重启时重新加入的节点列表中。[集群状态](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md)将此列表显示为`cluster_<name>_nodes_view`。

<!-- example joining a replication cluster  2 -->
当节点位于不同的网段或数据中心时，可以显式设置[nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster)选项。这可以最小化节点间的流量，并利用网关节点进行数据中心间的互通。以下代码使用[nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster)选项加入现有集群。

> **注意：** 使用此语法时，集群的`cluster_<name>_nodes_set`列表不会自动更新。要更新它，请使用[ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md)。


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

`JOIN CLUSTER`命令是同步工作的，一旦节点从集群中的其他节点接收到所有数据并与它们同步，命令就会完成。

`JOIN CLUSTER`操作可能会失败，并显示指示重复[server_id](../../Server_settings/Searchd.md#server_id)的错误消息。当加入节点的`server_id`与集群中现有节点的`server_id`相同时，会发生这种情况。要解决此问题，请确保复制集群中的每个节点都有唯一的[server_id](../../Server_settings/Searchd.md#server_id)。您可以在尝试加入集群之前，在配置文件的"searchd"部分将默认的[server_id](../../Server_settings/Searchd.md#server_id)更改为唯一值。
<!-- proofread -->
