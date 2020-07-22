# Replication cluster status 

<!-- example Example -->

[Node status](Profiling_and_monitoring/Node_status.md) outputs, among other information, cluster status variables.

The output format is `cluster_name_variable_name` `variable_value`. Most of them are described in [Galera Documentation Status Variables](https://galeracluster.com/library/documentation/galera-status-variables.html). Additionally we display:

* cluster_name - [name](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) of the cluster
* node_state - current state of the node: `closed`, `destroyed`, `joining`, `donor`, `synced`
* indexes_count - number of indexes managed by the cluster
* indexes - list of index names managed by the cluster
* nodes_set - list of nodes in the cluster defined with cluster `CREATE`, `JOIN` or `ALTER UPDATE` commands
* nodes_view - actual list of nodes in cluster which this node sees


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

<!-- request HTTP -->

```json
POST /sql -d "mode=raw&query=
SHOW STATUS
"
```

<!-- response HTTP-->

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

<!-- end -->
