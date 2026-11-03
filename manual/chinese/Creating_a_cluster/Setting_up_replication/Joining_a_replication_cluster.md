# 加入复制集群

<!-- example joining a replication cluster 1 -->
要加入现有集群，至少必须指定：
* 集群的[名称](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster)
* 要加入的集群中另一个节点的 `host:port`

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

如果启用了[身份验证和授权](../../Security/Authentication_and_authorization.md)，实际用于复制的用户必须具有 `replication` 权限。您可以在 `JOIN CLUSTER` 语句中指定该用户：

```sql
GRANT replication ON 'posts' TO 'repl_user';
JOIN CLUSTER posts AT '10.12.1.35:9312' 'repl_user' AS user;
```

加入节点和供体节点必须存在同一用户，且存储的身份验证数据必须一致。在各节点上分别创建相同的用户名和密码并不足够，因为存储的身份验证数据可能不同。

如果未指定用户，加入操作将使用当前会话用户。成功加入后，保存的集群用户将取自供体集群的元数据。

> 注意：启用身份验证后，成功执行 `JOIN CLUSTER` 会将加入节点上的所有本地身份验证数据替换为供体集群的身份验证数据。如果身份验证日志级别为 `info` 或更高，Manticore 会在替换前将原有的本地身份验证数据以 JSON 备份形式写入 `searchd.log.auth`。此备份包含盐值和凭据哈希，因此请妥善保管身份验证日志，并在分享前进行脱敏处理。如果 `JOIN CLUSTER` 无法获取供体用户，请核实供体节点上的复制用户及其身份验证数据是否一致，并检查供体节点的 `searchd.log.auth` 中是否存在 API 身份验证失败记录。

<!-- example joining a replication cluster 1_1 -->
在大多数情况下，如果只有一个复制集群，上述设置就已足够。但如果要创建多个复制集群，还必须设置[路径](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster)，并确保该目录可用。

<!-- request SQL -->
```sql
JOIN CLUSTER c2 at '127.0.0.1:10201' 'c2' as path
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "JOIN CLUSTER c2 at '127.0.0.1:10201' 'c2' as path"
```
<!-- end -->

节点通过从指定节点获取数据来加入集群。成功加入后，会更新集群中所有其他节点的节点列表，效果与手动执行 [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) 相同。此列表用于在节点重启后重新加入集群。

节点列表有两种：
1.`cluster_<name>_nodes_set`：用于在节点重启后重新加入集群。此列表会在所有节点上更新，效果与执行 [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) 相同。`JOIN CLUSTER` 命令会自动执行此更新。[集群状态](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md)将此列表显示为 `cluster_<name>_nodes_set`。
2. `cluster_<name>_nodes_view`：此列表包含所有用于复制的活跃节点，无需手动管理。[ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) 实际上会将此节点列表复制到重启后用于重新加入集群的节点列表中。[集群状态](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md)将此列表显示为 `cluster_<name>_nodes_view`。

<!-- example joining a replication cluster  2 -->
当节点位于不同网段或数据中心时，可以显式设置 [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) 选项。这样可以减少节点间的流量，并利用网关节点实现数据中心之间的通信。以下代码使用 [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) 选项加入现有集群。

> **注意：**使用此语法时，集群的 `cluster_<name>_nodes_set` 列表不会自动更新。要更新该列表，请使用 [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md)。


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

`JOIN CLUSTER` 命令以同步方式执行，节点接收完集群中其他节点的所有数据并与其同步后，命令即告完成。

`JOIN CLUSTER` 操作可能会失败，并提示 [server_id](../../Server_settings/Searchd.md#server_id) 重复。当加入节点的 `server_id` 与集群中现有节点相同时，就会出现此问题。要解决此问题，请确保复制集群中的每个节点都有唯一的 [server_id](../../Server_settings/Searchd.md#server_id)。在尝试加入集群之前，您可以将配置文件中 "searchd" 部分的默认 [server_id](../../Server_settings/Searchd.md#server_id) 改为唯一值。

在传输任何数据之前，集群中的每个节点都会检查能否通过加入节点的 [node_address](../../Server_settings/Searchd.md#node_address) 和 API 端口访问该节点，因为发送数据的节点需要通过此地址和端口连接到加入节点。如果某个节点报告无法访问，例如无法解析加入节点的名称，`JOIN CLUSTER` 就会失败并返回该节点的错误，集群不受影响。在放弃之前，系统会根据 [replication_retry_delay](../../Server_settings/Searchd.md#replication_retry_delay) 逐步延长间隔，重复检查。未响应检查的节点（例如正在重启的节点）会被跳过。节点启动时重新加入集群，也会执行相同的检查。集群节点列表中名称已无法解析的节点（例如已删除的容器）也会被跳过：加入、重启及其他集群操作会使用其余节点，加入操作不会启动新集群。

当加入节点需要获取集群表的完整副本（SST）时，Galera 会选择一个已同步的节点作为供体，该节点的 `cluster_<name>_node_state` 会显示为 `donor`。供体不一定是 `AT` 中指定的节点。在供体发送表文件期间，对供体上集群表的写入会等待传输完成。这些写入不会被拒绝，因此客户端会感到写入变慢，并可能触发自身的超时。供体也要等到传输完成后才会应用来自其他节点的写入。其他节点上的写入仅在加入节点进入集群时短暂停顿，所有节点上的读取均不会被阻塞。集群表越大，供体上的等待时间就越长；您可以通过 [cluster_<name>_sst_total](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md#SST-Progress-Metrics) 查看传输进度。

您无法指定供体，而供体可能恰好是客户端正在写入的节点。要在节点加入期间保持写入，只要有节点报告 `donor` 状态，就将写入发送到其他节点。例如，可以让负载均衡器的健康检查将 `donor` 视为不可写，并在写入负载较低时添加节点。如果重启的节点只缺少近期事务，则会通过增量状态传输（IST）追赶进度：此时不复制表文件，供体上的写入也无需等待。只要供体的写集缓存（集群[选项](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#options)中的 `gcache.size`）仍保留该节点缺失的全部事务，就可以使用 IST。
<!-- proofread -->
