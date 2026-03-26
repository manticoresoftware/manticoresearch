# Управление узлами репликации

<!-- example managing replication nodes 1 -->
Оператор `ALTER CLUSTER <cluster_name> UPDATE nodes` обновляет списки узлов на каждом узле в указанном кластере, чтобы включить все активные узлы кластера. Для получения дополнительной информации о списках узлов см. [Присоединение к кластеру](../../Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md).


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
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('ALTER CLUSTER posts UPDATE nodes')
```

<!-- response Python-asyncio -->
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

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("ALTER CLUSTER posts UPDATE nodes", Some(true)).await;
```

<!-- end -->


Например, при первоначальном создании кластера список узлов, используемых для повторного присоединения к кластеру, был `10.10.0.1:9312,10.10.1.1:9312`. С тех пор к кластеру присоединились другие узлы, и теперь активными узлами являются `10.10.0.1:9312,10.10.1.1:9312,10.15.0.1:9312,10.15.0.3:9312`. Однако список узлов, используемых для повторного присоединения к кластеру, не был обновлен.

Чтобы исправить это, можно выполнить оператор `ALTER CLUSTER ... UPDATE nodes`, чтобы скопировать список активных узлов в список узлов, используемых для повторного присоединения к кластеру. После этого список узлов, используемых для повторного присоединения к кластеру, будет включать все активные узлы кластера.

Оба списка узлов можно просмотреть с помощью оператора [Статус кластера](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md) (`cluster_post_nodes_set` и `cluster_post_nodes_view`).

## Удаление узла из кластера

Чтобы удалить узел из кластера репликации, выполните следующие шаги:
1. Остановите узел
2. Удалите информацию о кластере из файла `<data_dir>/manticore.json` (обычно `/var/lib/manticore/manticore.json`) на остановленном узле.
3. Выполните `ALTER CLUSTER cluster_name UPDATE nodes` на любом другом узле.

После этих шагов другие узлы забудут об отсоединенном узле, а отсоединенный узел забудет о кластере. Это действие не повлияет на таблицы в кластере или на отсоединенном узле.
<!-- proofread -->

