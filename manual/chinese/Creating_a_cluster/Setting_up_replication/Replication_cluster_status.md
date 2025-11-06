# 复制集群状态

<!-- example Example -->

您可以通过检查节点状态来查看集群状态信息。这可以使用 [Node status](../../Node_info_and_management/Node_status.md) 命令完成，该命令显示有关节点的各种信息，包括集群状态变量。

集群状态变量的输出格式如下：`cluster_name_variable_name` `variable_value`。大多数变量在 [Galera Documentation Status Variables](https://galeracluster.com/library/documentation/galera-status-variables.html) 中有说明。除了这些变量外，Manticore Search 还显示：

* `cluster_name` - 集群名称，如[复制设置](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster)中定义
* `node_state` - 节点当前状态：`closed`，`destroyed`，`joining`，`donor`，`synced`
* `indexes_count` - 集群管理的表数量
* `indexes` - 集群管理的表名列表
* `nodes_set` - 使用 `CREATE`、`JOIN` 或 `ALTER UPDATE` 命令定义的集群节点列表
* `nodes_view` - 当前节点可见的集群实际节点列表
* `state_uuid` - 集群状态的 UUID。如果它与 local_state_uuid 的值相匹配，表示本地节点和集群节点同步
* `conf_id` - 集群成员变更的总次数
* `status` - 集群组件状态。可能值有 primary（主组配置，有法定人数），non_primary（非主组配置，失去法定人数），或 disconnected（未连接到组，正在重试）
* `size` - 当前集群中的节点数
* `local_index` - 节点在集群中的索引
* `last_error` - 最近记录的与集群操作相关的错误消息。该消息提供问题的高级摘要。如需更详尽的上下文，请查看 `searchd.log` 文件

### SST 进度变量

在状态快照传输（State Snapshot Transfer，SST）期间，一个节点通过传输完整数据副本为另一个节点提供数据。这发生在新节点加入集群 [JOIN CLUSTER](Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md) 或添加新表时 [`ALTER CLUSTER ADD`](Creating_a_cluster/Setting_up_replication/Adding_and_removing_a_table_from_a_replication_cluster.md#Adding-and-removing-a-table-from-a-replication-cluster)。当 SST 处于活动状态时，在 donor 和 joiner 两个节点上都会可用以下额外状态变量，并保持同步进度。

* `cluster_name_sst_total` - 整个 SST 操作的总体进度，范围从 0 到 100。此计数器为主要观察指标。
* `cluster_name_sst_stage` - 当前工作阶段名称。每个传输表的过程依次包含以下阶段：
    * `await nodes sync`
    * `analyze remote`
    * `send files`
    * `activate tables`
* `cluster_name_sst_stage_total` - 当前阶段的进度，范围从 0 到 100
* `cluster_name_sst_tables` - SST 传输的表总数
* `cluster_name_sst_table` - 当前处理的表名及其索引（例如，`3 (products)`）

对大多数使用场景而言，`cluster_name_sst_total` 即足够。但其他计数器对于调查特定 SST 阶段或某个表的停顿或性能问题时很有帮助。
* node_state - 节点当前状态：`closed`，`destroyed`，`joining`，`donor`，`synced`
* indexes_count - 集群管理的表的数量
* indexes - 集群管理的表名列表
* nodes_set - 使用 `CREATE`、`JOIN` 或 `ALTER UPDATE` 命令定义的集群中节点列表
* nodes_view - 当前节点可见的集群中实际节点列表。


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS
+---------------------------------+-------------------------------------------------------------------------------------+
| 计数器                          | 值                                                                                  |
+---------------------------------+-------------------------------------------------------------------------------------+
| cluster_name                    | post                                                                                |
| cluster_post_state_uuid         | fba97c45-36df-11e9-a84e-eb09d14b8ea7                                               |
| cluster_post_conf_id            | 1                                                                                   |
| cluster_post_status             | primary                                                                             |
| cluster_post_size               | 5                                                                                   |
| cluster_post_local_index        | 0                                                                                   |
| cluster_post_node_state         | donor                                                                               |
| cluster_post_indexes_count      | 2                                                                                   |
| cluster_post_indexes            | pq1,pq_posts                                                                        |
| cluster_post_nodes_set          | 10.10.0.1:9312                                                                     |
| cluster_post_nodes_view         | 10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication  |
| cluster_post_sst_stage          | send files                                                                          |
| cluster_post_sst_stage_total    | 78                                                                                  |
| cluster_post_sst_tables         | 5                                                                                   |
| cluster_post_sst_table          | 3 (products)                                                                        |
```

<!-- response SQL-->

```sql
+----------------------------+-------------------------------------------------------------------------------------+
| Counter                    | Value                                                                               |
+----------------------------+-------------------------------------------------------------------------------------+
| cluster_name               | post                                                                                |
| cluster_post_state_uuid    | fba97c45-36df-11e9-a84e-eb09d14b8ea7                                                |
| cluster_post_conf_id       | 1                                                                                   |
| cluster_post_status        | primary                                                                             |
| cluster_post_size          | 5                                                                                   |
| cluster_post_local_index   | 0                                                                                   |
| cluster_post_node_state    | synced                                                                              |
| cluster_post_indexes_count | 2                                                                                   |
| cluster_post_indexes       | pq1,pq_posts                                                                        |
| cluster_post_nodes_set     | 10.10.0.1:9312                                                                      |
| cluster_post_nodes_view    | 10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication |
```

<!-- request JSON -->

{"Counter":"cluster_post_node_state", "Value":"donor"},
```json
POST /cli -d "
SHOW STATUS
{"Counter":"cluster_post_nodes_view", "Value":"10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication"},
{"Counter":"cluster_post_sst_total", "Value":"65"},
{"Counter":"cluster_post_sst_stage", "Value":"send files"},
{"Counter":"cluster_post_sst_stage_total", "Value":"78"},
{"Counter":"cluster_post_sst_table", "Value":"3 (products)"}
```

<!-- response JSON-->

```json
"
{"columns":[{"Counter":{"type":"string"}},{"Value":{"type":"string"}}],
"data":[
{"Counter":"cluster_name", "Value":"post"},
{"Counter":"cluster_post_state_uuid", "Value":"fba97c45-36df-11e9-a84e-eb09d14b8ea7"},
{"Counter":"cluster_post_conf_id", "Value":"1"},
{"Counter":"cluster_post_status", "Value":"primary"},
{"Counter":"cluster_post_size", "Value":"5"},
{"Counter":"cluster_post_local_index", "Value":"0"},
{"Counter":"cluster_post_node_state", "Value":"synced"},
{"Counter":"cluster_post_indexes_count", "Value":"2"},
{"Counter":"cluster_post_indexes", "Value":"pq1,pq_posts"},
{"Counter":"cluster_post_nodes_set", "Value":"10.10.0.1:9312"},
{"Counter":"cluster_post_nodes_view", "Value":"10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication"}
],
"total":0,
"error":"",
}
```

<!-- request PHP -->

```php
$params = [
    'body' => []
"cluster_post_nodes_view" => "10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication",
"cluster_post_sst_total" => 65,
"cluster_post_sst_stage" => "发送文件",
"cluster_post_sst_stage_total" => 78,
"cluster_post_sst_tables" => 5,
"cluster_post_sst_table" => "3 (产品)"
];
$response = $client->nodes()->status($params);
```

<!-- response PHP -->

```php
(
"cluster_name" => "post",
"cluster_post_state_uuid" => "fba97c45-36df-11e9-a84e-eb09d14b8ea7",
"cluster_post_conf_id" => 1,
"cluster_post_status" => "primary",
"cluster_post_size" => 5,
"cluster_post_local_index" => 0,
"cluster_post_node_state" => "synced",
"cluster_post_indexes_count" => 2,
"cluster_post_nodes_set" => "10.10.0.1:9312",
"cluster_post_nodes_view" => "10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication"
)
```
##### Python:
	{u'Key': u'cluster_post_node_state', u'Value': u'donor'},

<!-- request Python -->

```python
	{u'Key': u'cluster_post_nodes_view', u'Value': u'10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication'},
	{u'Key': u'cluster_post_sst_total', u'Value': u'65'},
	{u'Key': u'cluster_post_sst_stage', u'Value': u'发送文件'},
	{u'Key': u'cluster_post_sst_stage_total', u'Value': u'78'},
	{u'Key': u'cluster_post_sst_tables', u'Value': u'5'},
	{u'Key': u'cluster_post_sst_table', u'Value': u'3 (产品)'}],
utilsApi.sql('SHOW STATUS')
```
<!-- response Python -->

```python
{u'columns': [{u'Key': {u'type': u'string'}},
              {u'Value': {u'type': u'string'}}],
 u'data': [
	{u'Key': u'cluster_name', u'Value': u'post'},
	{u'Key': u'cluster_post_state_uuid', u'Value': u'fba97c45-36df-11e9-a84e-eb09d14b8ea7'},
	{u'Key': u'cluster_post_conf_id', u'Value': u'1'},
	{u'Key': u'cluster_post_status', u'Value': u'primary'},
	{u'Key': u'cluster_post_size', u'Value': u'5'},
	{u'Key': u'cluster_post_local_index', u'Value': u'0'},
	{u'Key': u'cluster_post_node_state', u'Value': u'synced'},
	{u'Key': u'cluster_post_indexes', u'Value': u'pq1,pq_posts'},
	{u'Key': u'cluster_post_nodes_set', u'Value': u'10.10.0.1:9312'},
	{u'Key': u'cluster_post_nodes_view', u'Value': u'10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication'}],
 u'error': u'',
 u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:
	{u'Key': u'cluster_post_node_state', u'Value': u'donor'},

<!-- request Python-asyncio -->

```python
	{u'Key': u'cluster_post_nodes_view', u'Value': u'10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication'},
	{u'Key': u'cluster_post_sst_total', u'Value': u'65'},
	{u'Key': u'cluster_post_sst_stage', u'Value': u'发送文件'},
	{u'Key': u'cluster_post_sst_stage_total', u'Value': u'78'},
	{u'Key': u'cluster_post_sst_tables', u'Value': u'5'},
	{u'Key': u'cluster_post_sst_table', u'Value': u'3 (产品)'}],
await utilsApi.sql('SHOW STATUS')
```
<!-- response Python-asyncio -->

```python
{u'columns': [{u'Key': {u'type': u'string'}},
              {u'Value': {u'type': u'string'}}],
 u'data': [
	{u'Key': u'cluster_name', u'Value': u'post'},
	{u'Key': u'cluster_post_state_uuid', u'Value': u'fba97c45-36df-11e9-a84e-eb09d14b8ea7'},
	{u'Key': u'cluster_post_status', u'Value': u'primary'},
	{u'Key': u'cluster_post_size', u'Value': u'5'},
	{u'Key': u'cluster_post_local_index', u'Value': u'0'},
	{u'Key': u'cluster_post_node_state', u'Value': u'synced'},
	{u'Key': u'cluster_post_indexes', u'Value': u'pq1,pq_posts'},
	{u'Key': u'cluster_post_nodes_set', u'Value': u'10.10.0.1:9312'},
	{u'Key': u'cluster_post_nodes_view', u'Value': u'10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```

<!-- intro -->
##### Javascript:

	{"Key": "cluster_post_node_state", "Value": "donor"},
<!-- request javascript -->

```javascript
res = await utilsApi.sql('SHOW STATUS');
	{"Key": "cluster_post_nodes_view", "Value": "10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication"},
	{"Key": "cluster_post_sst_total", "Value": "65"},
	{"Key": "cluster_post_sst_stage", "Value": "发送文件"},
	{"Key": "cluster_post_sst_stage_total", "Value": "78"},
	{"Key": "cluster_post_sst_tables", "Value": "5"},
	{"Key": "cluster_post_sst_table", "Value": "3 (产品)"}],
```

<!-- response Javascript -->

{"columns": [{"Key": {"type": "string"}},
              {"Value": {"type": "string"}}],
 "data": [
	{"Key": "cluster_name", "Value": "post"},
	{"Key": "cluster_post_conf_id", "Value": "1"},
	{"Key": "cluster_post_status", "Value": "primary"},
	{"Key": "cluster_post_size", "Value": "5"},
	{"Key": "cluster_post_local_index", "Value": "0"},
	{"Key": "cluster_post_node_state", "Value": "synced"},
	{"Key": "cluster_post_indexes_count", "Value": "2"},
	{"Key": "cluster_post_indexes", "Value": "pq1,pq_posts"},
	{"Key": "cluster_post_nodes_set", "Value": "10.10.0.1:9312"},
	{"Key": "cluster_post_nodes_view", "Value": "10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication"}],
 "error": "",
 "total": 0,
 "warning": ""}
```

<!-- intro -->
##### java:
	{ Key=cluster_post_node_state, Value=donor},

<!-- request Java -->

```java
	{ Key=cluster_post_nodes_view, Value=10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication},
	{ Key=cluster_post_sst_total, Value=65},
	{ Key=cluster_post_sst_stage, Value=发送文件},
	{ Key=cluster_post_sst_stage_total, Value=78},
	{ Key=cluster_post_sst_tables, Value=5},
utilsApi.sql("SHOW STATUS");
```
<!-- response Java -->

{columns=[{ Key : { type=string }},
              { Value : { type=string }}],
  data : [
	{ Key=cluster_name, Value=post},
	{ Key=cluster_post_state_uuid, Value=fba97c45-36df-11e9-a84e-eb09d14b8ea7},
	{ Key=cluster_post_conf_id, Value=1},
	{ Key=cluster_post_status, Value=primary},
	{ Key=cluster_post_local_index, Value=0},
	{ Key=cluster_post_node_state, Value=synced},
	{ Key=cluster_post_indexes_count, Value=2},
	{ Key=cluster_post_indexes, Value=pq1,pq_posts},
	{ Key=cluster_post_nodes_set, Value=10.10.0.1:9312},
	{ Key=cluster_post_nodes_view, Value=10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication}],
  error= ,
  total=0,
  warning= }
```

<!-- intro -->
##### C#:
	{ Key=cluster_post_node_state, Value=donor},

<!-- request C# -->

```clike
	{ Key=cluster_post_nodes_view, Value=10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication},
	{ Key=cluster_post_sst_total, Value=65},
	{ Key=cluster_post_sst_stage, Value=发送文件},
	{ Key=cluster_post_sst_stage_total, Value=78},
	{ Key=cluster_post_sst_tables, Value=5},
	{ Key=cluster_post_sst_table, Value=3 (产品)}],
utilsApi.sql("SHOW STATUS");
```
<!-- response C# -->

```C#
{columns=[{ Key : { type=String }},
              { Value : { type=String }}],
  data : [
	{ Key=cluster_name, Value=post},
	{ Key=cluster_post_state_uuid, Value=fba97c45-36df-11e9-a84e-eb09d14b8ea7},
	{ Key=cluster_post_conf_id, Value=1},
	{ Key=cluster_post_status, Value=primary},
	{ Key=cluster_post_size, Value=5},
	{ Key=cluster_post_local_index, Value=0},
	{ Key=cluster_post_node_state, Value=synced},
	{ Key=cluster_post_indexes_count, Value=2},
	{ Key=cluster_post_indexes, Value=pq1,pq_posts},
	{ Key=cluster_post_nodes_set, Value=10.10.0.1:9312},
	{ Key=cluster_post_nodes_view, Value=10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication}],
  error="" ,
  total=0,
  warning="" }
```

<!-- intro -->
##### Rust:
	{ Key=cluster_post_node_state, Value=donor},

<!-- request Rust -->

```rust
	{ Key=cluster_post_nodes_view, Value=10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication},
	{ Key=cluster_post_sst_total, Value=65},
	{ Key=cluster_post_sst_stage, Value=发送文件},
	{ Key=cluster_post_sst_stage_total, Value=78},
	{ Key=cluster_post_sst_tables, Value=5},
	{ Key=cluster_post_sst_table, Value=3 (产品)}],
utils_api.sql("SHOW STATUS", Some(true)).await;
```
<!-- response Rust -->

```rust
{columns=[{ Key : { type=String }},
              { Value : { type=String }}],
<!-- proofread -->
  data : [
	{ Key=cluster_name, Value=post},
	{ Key=cluster_post_state_uuid, Value=fba97c45-36df-11e9-a84e-eb09d14b8ea7},
	{ Key=cluster_post_conf_id, Value=1},
	{ Key=cluster_post_status, Value=primary},
	{ Key=cluster_post_size, Value=5},
	{ Key=cluster_post_local_index, Value=0},
	{ Key=cluster_post_node_state, Value=synced},
	{ Key=cluster_post_indexes_count, Value=2},
	{ Key=cluster_post_indexes, Value=pq1,pq_posts},
	{ Key=cluster_post_nodes_set, Value=10.10.0.1:9312},
	{ Key=cluster_post_nodes_view, Value=10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication}],
  error="" ,
  total=0,
  warning="" }
```

<!-- end -->
<!-- proofread -->

