# Replication cluster status 

<!-- example Example -->

You can view the cluster status information by checking the node status. This can be done using the [Node status](../../Node_info_and_management/Node_status.md) command, which displays various information about the node, including the cluster status variables.

The output format for the cluster status variables is as follows:  `cluster_name_variable_name` `variable_value`. Most of the variables are described in the [Galera Documentation Status Variables](https://galeracluster.com/library/documentation/galera-status-variables.html). In addition to these variables, Manticore Search also displays:

* cluster_name - the name of the cluster, as defined in the [replication setup](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) 
* node_state - the current state of the node: `closed`, `destroyed`, `joining`, `donor`, `synced`
* indexes_count - the number of tables managed by the cluster
* indexes - a list of table names managed by the cluster
* nodes_set - the list of nodes in the cluster defined using the `CREATE`, `JOIN` or `ALTER UPDATE` commands
* nodes_view - the actual list of nodes in the cluster that the current node can see.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW STATUS
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
{"Counter":"cluster_post_node_state", "Value":"synced"},
{"Counter":"cluster_post_indexes_count", "Value":"2"},
{"Counter":"cluster_post_indexes", "Value":"pq1,pq_posts"},
{"Counter":"cluster_post_nodes_set", "Value":"10.10.0.1:9312"},
{"Counter":"cluster_post_nodes_view", "Value":"10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication"}
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
"cluster_post_node_state" => "synced",
"cluster_post_indexes_count" => 2,
"cluster_post_indexes" => "pq1,pq_posts",
"cluster_post_nodes_set" => "10.10.0.1:9312",
"cluster_post_nodes_view" => "10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication"
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
	{u'Key': u'cluster_post_node_state', u'Value': u'synced'},
	{u'Key': u'cluster_post_indexes_count', u'Value': u'2'},
	{u'Key': u'cluster_post_indexes', u'Value': u'pq1,pq_posts'},
	{u'Key': u'cluster_post_nodes_set', u'Value': u'10.10.0.1:9312'},
	{u'Key': u'cluster_post_nodes_view', u'Value': u'10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```
<!-- intro -->
##### javascript:

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
	{ Key=cluster_post_node_state, Value=synced},
	{ Key=cluster_post_indexes_count, Value=2},
	{ Key=cluster_post_indexes, Value=pq1,pq_posts},
	{ Key=cluster_post_nodes_set, Value=10.10.0.1:9312},
	{ Key=cluster_post_nodes_view, Value=10.10.0.1:9312,10.10.0.1:9320:replication,10.10.1.1:9312,10.10.1.1:9320:replication}],
  error= ,
  total=0,
  warning= }
```
<!-- end -->
<!-- proofread -->