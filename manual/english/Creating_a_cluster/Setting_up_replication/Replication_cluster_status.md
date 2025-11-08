# Replication cluster status

<!-- example Example -->

You can view the cluster status information by checking the node status. This can be done using the [Node status](../../Node_info_and_management/Node_status.md) command, which displays various information about the node, including the cluster status variables.

The output format for the cluster status variables is as follows:  `cluster_name_variable_name` `variable_value`. Most of the variables are described in the [Galera Documentation Status Variables](https://galeracluster.com/library/documentation/galera-status-variables.html). In addition to these variables, Manticore Search also displays:

* `cluster_name` - the name of the cluster, as defined in the [replication setup](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster)
* `node_state` - the current state of the node: `closed`, `destroyed`, `joining`, `donor`, `synced`
* `indexes_count` - the number of tables managed by the cluster
* `indexes` - a list of table names managed by the cluster
* `nodes_set` - the list of nodes in the cluster defined using the `CREATE`, `JOIN` or `ALTER UPDATE` commands
* `nodes_view` - the actual list of nodes in the cluster that the current node can see.
* `state_uuid` - UUID state of the cluster. If it matches the value in local_state_uuid, the local and cluster nodes are in sync.
* `conf_id` - total number of cluster membership changes that have taken place.
* `status` - cluster component status. Possible values are primary (primary group configuration, quorum present), non_primary (non-primary group configuration, quorum lost), or disconnected (not connected to group, retrying).
* `size` - number of nodes currently in the cluster.
* `local_index` - the node's index in the cluster.
* `last_error` - the last recorded error message related to a cluster operation. The message provides a high-level summary of the problem. For more detailed context, you should consult the `searchd.log` file.

### SST Progress Metrics

During a State Snapshot Transfer (SST), a node provisions another by transferring a full data copy. This happens when a new node joins the cluster [JOIN CLUSTER](Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md) or when new tables are added [ALTER CLUSTER ADD](Creating_a_cluster/Setting_up_replication/Adding_and_removing_a_table_from_a_replication_cluster.md#Adding-and-removing-a-table-from-a-replication-cluster). While an SST is active, the following additional status variables will be available on both the donor and joiner nodes, with their progress kept in sync.

* `cluster_name_sst_total` - The overall progress of the entire SST operation, from 0 to 100. This is the primary counter to watch.
* `cluster_name_sst_stage` - The name of the current work phase. The process cycles through these stages for each table being transferred:
    * `await nodes sync`
    * `block checksum calculate`
    * `analyze remote`
    * `send files`
    * `activate tables`
* `cluster_name_sst_stage_total` - The progress of the current stage, from 0 to 100.
* `cluster_name_sst_tables` - The total number of tables being transferred in the SST.
* `cluster_name_sst_table` - The name and index of the table currently being processed (e.g., `3 (products)`).

For most use cases, `cluster_name_sst_total` is sufficient. However, the other counters can be useful for investigating stalls or performance issues during a specific SST stage or on a particular table.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS
```

<!-- response SQL-->

```sql
+---------------------------------+-------------------------------------------------------------------------------------+
| Counter                         | Value                                                                               |
+---------------------------------+-------------------------------------------------------------------------------------+
| cluster_name                    | post                                                                                |
| cluster_post_state_uuid         | fba97c45-36df-11e9-a84e-eb09d14b8ea7                                                |
| cluster_post_conf_id            | 1                                                                                   |
| cluster_post_status             | primary                                                                             |
| cluster_post_size               | 5                                                                                   |
| cluster_post_local_index        | 0                                                                                   |
| cluster_post_node_state         | donor                                                                               |
| cluster_post_indexes_count      | 2                                                                                   |
| cluster_post_indexes            | pq1,pq_posts                                                                        |
| cluster_post_nodes_set          | 10.10.0.1:9312                                                                      |
| cluster_post_nodes_view         | 10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication |
| cluster_post_sst_total          | 65                                                                                  |
| cluster_post_sst_stage          | send files                                                                       |
| cluster_post_sst_stage_total    | 78                                                                                  |
| cluster_post_sst_tables         | 5                                                                                   |
| cluster_post_sst_table          | 3 (products)                                                                        |
+---------------------------------+-------------------------------------------------------------------------------------+
```

<!-- request JSON -->

```json
POST /cli -d "
SHOW STATUS
"
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
{"Counter":"cluster_post_node_state", "Value":"donor"},
{"Counter":"cluster_post_indexes_count", "Value":"2"},
{"Counter":"cluster_post_indexes", "Value":"pq1,pq_posts"},
{"Counter":"cluster_post_nodes_set", "Value":"10.10.0.1:9312"},
{"Counter":"cluster_post_nodes_view", "Value":"10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication"},
{"Counter":"cluster_post_sst_total", "Value":"65"},
{"Counter":"cluster_post_sst_stage", "Value":"send files"},
{"Counter":"cluster_post_sst_stage_total", "Value":"78"},
{"Counter":"cluster_post_sst_tables", "Value":"5"},
{"Counter":"cluster_post_sst_table", "Value":"3 (products)"}
],
"total":0,
"error":"",
"warning":""
}
```

<!-- request PHP -->

```php
$params = [
    'body' => []
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
"cluster_post_node_state" => "donor",
"cluster_post_indexes_count" => 2,
"cluster_post_indexes" => "pq1,pq_posts",
"cluster_post_nodes_set" => "10.10.0.1:9312",
"cluster_post_nodes_view" => "10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication",
"cluster_post_sst_total" => 65,
"cluster_post_sst_stage" => "send files",
"cluster_post_sst_stage_total" => 78,
"cluster_post_sst_tables" => 5,
"cluster_post_sst_table" => "3 (products)"
)
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
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
	{u'Key': u'cluster_post_node_state', u'Value': u'donor'},
	{u'Key': u'cluster_post_indexes_count', u'Value': u'2'},
	{u'Key': u'cluster_post_indexes', u'Value': u'pq1,pq_posts'},
	{u'Key': u'cluster_post_nodes_set', u'Value': u'10.10.0.1:9312'},
	{u'Key': u'cluster_post_nodes_view', u'Value': u'10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication'},
	{u'Key': u'cluster_post_sst_total', u'Value': u'65'},
	{u'Key': u'cluster_post_sst_stage', u'Value': u'send files'},
	{u'Key': u'cluster_post_sst_stage_total', u'Value': u'78'},
	{u'Key': u'cluster_post_sst_tables', u'Value': u'5'},
	{u'Key': u'cluster_post_sst_table', u'Value': u'3 (products)'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('SHOW STATUS')
```
<!-- response Python-asyncio -->

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
	{u'Key': u'cluster_post_node_state', u'Value': u'donor'},
	{u'Key': u'cluster_post_indexes_count', u'Value': u'2'},
	{u'Key': u'cluster_post_indexes', u'Value': u'pq1,pq_posts'},
	{u'Key': u'cluster_post_nodes_set', u'Value': u'10.10.0.1:9312'},
	{u'Key': u'cluster_post_nodes_view', u'Value': u'10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication'},
	{u'Key': u'cluster_post_sst_total', u'Value': u'65'},
	{u'Key': u'cluster_post_sst_stage', u'Value': u'send files'},
	{u'Key': u'cluster_post_sst_stage_total', u'Value': u'78'},
	{u'Key': u'cluster_post_sst_tables', u'Value': u'5'},
	{u'Key': u'cluster_post_sst_table', u'Value': u'3 (products)'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('SHOW STATUS');
```

<!-- response Javascript -->

```javascript
{"columns": [{"Key": {"type": "string"}},
              {"Value": {"type": "string"}}],
 "data": [
	{"Key": "cluster_name", "Value": "post"},
	{"Key": "cluster_post_state_uuid", "Value": "fba97c45-36df-11e9-a84e-eb09d14b8ea7"},
	{"Key": "cluster_post_conf_id", "Value": "1"},
	{"Key": "cluster_post_status", "Value": "primary"},
	{"Key": "cluster_post_size", "Value": "5"},
	{"Key": "cluster_post_local_index", "Value": "0"},
	{"Key": "cluster_post_node_state", "Value": "donor"},
	{"Key": "cluster_post_indexes_count", "Value": "2"},
	{"Key": "cluster_post_indexes", "Value": "pq1,pq_posts"},
	{"Key": "cluster_post_nodes_set", "Value": "10.10.0.1:9312"},
	{"Key": "cluster_post_nodes_view", "Value": "10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication"},
	{"Key": "cluster_post_sst_total", "Value": "65"},
	{"Key": "cluster_post_sst_stage", "Value": "send files"},
	{"Key": "cluster_post_sst_stage_total", "Value": "78"},
	{"Key": "cluster_post_sst_tables", "Value": "5"},
	{"Key": "cluster_post_sst_table", "Value": "3 (products)"}],
 "error": "",
 "total": 0,
 "warning": ""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("SHOW STATUS");
```
<!-- response Java -->

```java
{columns=[{ Key : { type=string }},
              { Value : { type=string }}],
  data : [
	{ Key=cluster_name, Value=post},
	{ Key=cluster_post_state_uuid, Value=fba97c45-36df-11e9-a84e-eb09d14b8ea7},
	{ Key=cluster_post_conf_id, Value=1},
	{ Key=cluster_post_status, Value=primary},
	{ Key=cluster_post_size, Value=5},
	{ Key=cluster_post_local_index, Value=0},
	{ Key=cluster_post_node_state, Value=donor},
	{ Key=cluster_post_indexes_count, Value=2},
	{ Key=cluster_post_indexes, Value=pq1,pq_posts},
	{ Key=cluster_post_nodes_set, Value=10.10.0.1:9312},
	{ Key=cluster_post_nodes_view, Value=10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication},
	{ Key=cluster_post_sst_total, Value=65},
	{ Key=cluster_post_sst_stage, Value=send files},
	{ Key=cluster_post_sst_stage_total, Value=78},
	{ Key=cluster_post_sst_tables, Value=5},
	{ Key=cluster_post_sst_table, Value=3 (products)}],
  error= ,
  total=0,
  warning= }
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
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
	{ Key=cluster_post_node_state, Value=donor},
	{ Key=cluster_post_indexes_count, Value=2},
	{ Key=cluster_post_indexes, Value=pq1,pq_posts},
	{ Key=cluster_post_nodes_set, Value=10.10.0.1:9312},
	{ Key=cluster_post_nodes_view, Value=10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication},
	{ Key=cluster_post_sst_total, Value=65},
	{ Key=cluster_post_sst_stage, Value=send files},
	{ Key=cluster_post_sst_stage_total, Value=78},
	{ Key=cluster_post_sst_tables, Value=5},
	{ Key=cluster_post_sst_table, Value=3 (products)}],
  error="" ,
  total=0,
  warning="" }
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("SHOW STATUS", Some(true)).await;
```
<!-- response Rust -->

```rust
{columns=[{ Key : { type=String }},
              { Value : { type=String }}],
  data : [
	{ Key=cluster_name, Value=post},
	{ Key=cluster_post_state_uuid, Value=fba97c45-36df-11e9-a84e-eb09d14b8ea7},
	{ Key=cluster_post_conf_id, Value=1},
	{ Key=cluster_post_status, Value=primary},
	{ Key=cluster_post_size, Value=5},
	{ Key=cluster_post_local_index, Value=0},
	{ Key=cluster_post_node_state, Value=donor},
	{ Key=cluster_post_indexes_count, Value=2},
	{ Key=cluster_post_indexes, Value=pq1,pq_posts},
	{ Key=cluster_post_nodes_set, Value=10.10.0.1:9312},
	{ Key=cluster_post_nodes_view, Value=10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication},
	{ Key=cluster_post_sst_total, Value=65},
	{ Key=cluster_post_sst_stage, Value=send files},
	{ Key=cluster_post_sst_stage_total, Value=78},
	{ Key=cluster_post_sst_tables, Value=5},
	{ Key=cluster_post_sst_table, Value=3 (products)}],
  error="" ,
  total=0,
  warning="" }
```

<!-- end -->
<!-- proofread -->