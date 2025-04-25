# Статус кластера репликации 

<!-- example Example -->

Вы можете просмотреть информацию о статусе кластера, проверив статус узла. Это можно сделать с помощью команды [Статус узла](../../Node_info_and_management/Node_status.md), которая отображает различную информацию об узле, включая переменные статуса кластера.

Формат вывода для переменных статуса кластера следующий:  `cluster_name_variable_name` `variable_value`. Большинство переменных описаны в [Документации Galera Статус Переменные](https://galeracluster.com/library/documentation/galera-status-variables.html). В дополнение к этим переменным, Manticore Search также отображает:

* cluster_name - название кластера, как определено в [настройке репликации](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) 
* node_state - текущее состояние узла: `closed`, `destroyed`, `joining`, `donor`, `synced`
* indexes_count - количество таблиц, управляемых кластером
* indexes - список имен таблиц, управляемых кластером
* nodes_set - список узлов в кластере, определенный с использованием команд `CREATE`, `JOIN` или `ALTER UPDATE`
* nodes_view - фактический список узлов в кластере, которые видит текущий узел.


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
# Статус репликационного кластера

<!-- example Example -->

Вы можете просмотреть информацию о статусе кластера, проверив статус узла. Это можно сделать с помощью команды [Статус узла](../../Node_info_and_management/Node_status.md), которая отображает различную информацию об узле, включая переменные статуса кластера.

Формат вывода для переменных статуса кластера следующий:  `cluster_name_variable_name` `variable_value`. Большинство переменных описано в [Документации Galera по переменным статуса](https://galeracluster.com/library/documentation/galera-status-variables.html). В дополнение к этим переменным, Manticore Search также отображает:

* cluster_name - имя кластера, как определено в [настройках репликации](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) 
* node_state - текущее состояние узла: `closed`, `destroyed`, `joining`, `donor`, `synced`
* indexes_count - количество таблиц, управляемых кластером
* indexes - список имен таблиц, управляемых кластером
* nodes_set - список узлов в кластере, определенный с помощью команд `CREATE`, `JOIN` или `ALTER UPDATE`
* nodes_view - фактический список узлов в кластере, которые видит текущий узел.


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
	{"Key": "имя_кластера", "Value": "пост"},
	{"Key": "uuid_состояния_кластера_поста", "Value": "fba97c45-36df-11e9-a84e-eb09d14b8ea7"},
	{"Key": "идентификатор_конфигурации_кластера_поста", "Value": "1"},
	{"Key": "статус_кластера_поста", "Value": "основной"},
	{"Key": "размер_кластера_поста", "Value": "5"},
	{"Key": "локальный_индекс_кластера_поста", "Value": "0"},
	{"Key": "состояние_узла_кластера_поста", "Value": "синхронизировано"},
	{"Key": "количество_индексов_кластера_поста", "Value": "2"},
	{"Key": "индексы_кластера_поста", "Value": "pq1,pq_posts"},
	{"Key": "набор_узлов_кластера_поста", "Value": "10.10.0.1:9312"},
	{"Key": "представление_узлов_кластера_поста", "Value": "10.10.0.1:9312,10.10.0.1:9320:репликация,10.10.1.1:9312,10.10.1.1:9320:репликация"}],
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
	{ Key=имя_кластера, Value=пост},
	{ Key=uuid_состояния_кластера_поста, Value=fba97c45-36df-11e9-a84e-eb09d14b8ea7},
	{ Key=идентификатор_конфигурации_кластера_поста, Value=1},
	{ Key=статус_кластера_поста, Value=основной},
	{ Key=размер_кластера_поста, Value=5},
	{ Key=локальный_индекс_кластера_поста, Value=0},
	{ Key=состояние_узла_кластера_поста, Value=синхронизировано},
	{ Key=количество_индексов_кластера_поста, Value=2},
	{ Key=индексы_кластера_поста, Value=pq1,pq_posts},
	{ Key=набор_узлов_кластера_поста, Value=10.10.0.1:9312},
	{ Key=представление_узлов_кластера_поста, Value=10.10.0.1:9312,10.10.0.1:9320:репликация,10.10.1.1:9312,10.10.1.1:9320:репликация}],
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
	{ Key=имя_кластера, Value=пост},
	{ Key=uuid_состояния_кластера_поста, Value=fba97c45-36df-11e9-a84e-eb09d14b8ea7},
	{ Key=идентификатор_конфигурации_кластера_поста, Value=1},
	{ Key=статус_кластера_поста, Value=основной},
	{ Key=размер_кластера_поста, Value=5},
	{ Key=локальный_индекс_кластера_поста, Value=0},
	{ Key=состояние_узла_кластера_поста, Value=синхронизировано},
	{ Key=количество_индексов_кластера_поста, Value=2},
	{ Key=индексы_кластера_поста, Value=pq1,pq_posts},
	{ Key=набор_узлов_кластера_поста, Value=10.10.0.1:9312},
	{ Key=представление_узлов_кластера_поста, Value=10.10.0.1:9312,10.10.0.1:9320:репликация,10.10.1.1:9312,10.10.1.1:9320:репликация}],
  error="" ,
  total=0,
  warning="" }
```
<!-- end -->
<!-- proofread -->
