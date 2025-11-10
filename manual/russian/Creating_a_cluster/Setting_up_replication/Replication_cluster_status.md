# Статус кластера репликации

<!-- example Example -->

Вы можете просмотреть информацию о статусе кластера, проверив статус узла. Это можно сделать с помощью команды [Node status](../../Node_info_and_management/Node_status.md), которая отображает различную информацию об узле, включая переменные статуса кластера.

Формат вывода переменных статуса кластера следующий:  `cluster_name_variable_name` `variable_value`. Большинство переменных описаны в [Galera Documentation Status Variables](https://galeracluster.com/library/documentation/galera-status-variables.html). В дополнение к этим переменным, Manticore Search также отображает:

* `cluster_name` - имя кластера, как определено в [настройке репликации](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster)
* `node_state` - текущее состояние узла: `closed`, `destroyed`, `joining`, `donor`, `synced`
* `indexes_count` - количество таблиц, управляемых кластером
* `indexes` - список имен таблиц, управляемых кластером
* `nodes_set` - список узлов в кластере, определенный с помощью команд `CREATE`, `JOIN` или `ALTER UPDATE`
* `nodes_view` - фактический список узлов в кластере, которые видит текущий узел.
* `state_uuid` - UUID состояния кластера. Если он совпадает со значением в local_state_uuid, локальные и кластерные узлы синхронизированы.
* `conf_id` - общее количество изменений членства в кластере, которые произошли.
* `status` - статус компонента кластера. Возможные значения: primary (конфигурация первичной группы, кворум присутствует), non_primary (конфигурация непервичной группы, кворум потерян), или disconnected (не подключен к группе, повторная попытка).
* `size` - количество узлов, в настоящее время находящихся в кластере.
* `local_index` - индекс узла в кластере.
* `last_error` - последнее зарегистрированное сообщение об ошибке, связанной с операцией кластера. Сообщение предоставляет общий обзор проблемы. Для более подробного контекста следует обратиться к файлу `searchd.log`.

### Метрики прогресса SST

Во время передачи снимка состояния (SST) один узел обеспечивает другой, передавая полную копию данных. Это происходит, когда новый узел присоединяется к кластеру [JOIN CLUSTER](Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md) или когда добавляются новые таблицы [ALTER CLUSTER ADD](Creating_a_cluster/Setting_up_replication/Adding_and_removing_a_table_from_a_replication_cluster.md#Adding-and-removing-a-table-from-a-replication-cluster). Пока SST активен, на узлах-доноре и присоединяющемся будут доступны следующие дополнительные переменные статуса, прогресс которых синхронизирован.

* `cluster_name_sst_total` - общий прогресс всей операции SST, от 0 до 100. Это основной счетчик для отслеживания.
* `cluster_name_sst_stage` - название текущей фазы работы. Процесс проходит через эти стадии для каждой передаваемой таблицы:
    * `await nodes sync`
    * `block checksum calculate`
    * `analyze remote`
    * `send files`
    * `activate tables`
* `cluster_name_sst_stage_total` - прогресс текущей стадии, от 0 до 100.
* `cluster_name_sst_tables` - общее количество таблиц, передаваемых в SST.
* `cluster_name_sst_table` - имя и индекс таблицы, которая в данный момент обрабатывается (например, `3 (products)`).

Для большинства случаев использования достаточно `cluster_name_sst_total`. Однако другие счетчики могут быть полезны для расследования зависаний или проблем с производительностью на конкретной стадии SST или с определенной таблицей.

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