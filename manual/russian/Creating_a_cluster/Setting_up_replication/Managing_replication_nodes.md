# Управление узлами репликации 

<!-- example managing replication nodes 1 -->
Запрос `ALTER CLUSTER <cluster_name> UPDATE nodes`  обновляет списки узлов на каждом узле в указанном кластере, включая все активные узлы в кластере. Для получения дополнительной информации о списках узлов смотрите [Присоединение к кластеру](../../Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md).


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
ALTER CLUSTER posts UPDATE nodes
```

<!-- request JSON -->

```json
POST /cli -d "
ALTER CLUSTER posts UPDATE nodes
"
```

<!-- request PHP -->

```php
$params = [
  'cluster' => 'posts',
  'body' => [
     'operation' => 'update',
     
  ]
];
$response = $client->cluster()->alter($params); 
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('ALTER CLUSTER posts UPDATE nodes')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('ALTER CLUSTER posts UPDATE nodes');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("ALTER CLUSTER posts UPDATE nodes");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("ALTER CLUSTER posts UPDATE nodes");
```
<!-- end -->


Например, когда кластер был изначально установлен, список узлов, использованный для повторного присоединения к кластеру, был `10.10.0.1:9312,10.10.1.1:9312`. С тех пор другие узлы присоединились к кластеру, и теперь активные узлы - это `10.10.0.1:9312,10.10.1.1:9312,10.15.0.1:9312,10.15.0.3:9312`. Однако список узлов, использованный для повторного присоединения к кластеру, не был обновлен.

Чтобы исправить это, вы можете выполнить запрос `ALTER CLUSTER ... UPDATE nodes`, чтобы скопировать список активных узлов в список узлов, используемый для повторного присоединения к кластеру. После этого список узлов, используемый для повторного присоединения к кластеру, будет включать все активные узлы в кластере. 

Оба списка узлов можно просмотреть с помощью запроса [Статус кластера](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md) (`cluster_post_nodes_set` и `cluster_post_nodes_view`).

## Удаление узла из кластера

Чтобы удалить узел из репликационного кластера, выполните следующие шаги:
1. Остановите узел
2. Удалите информацию о кластере из  `<data_dir>/manticore.json` (обычно `/var/lib/manticore/manticore.json`) на остановленном узле.
3. Выполните `ALTER CLUSTER cluster_name UPDATE nodes` на любом другом узле.

После этих шагов остальные узлы забудут об отвязанном узле, а отвязанный узел забудет о кластере. Это действие не повлияет на таблицы в кластере или на отвязанный узел.
<!-- proofread -->
