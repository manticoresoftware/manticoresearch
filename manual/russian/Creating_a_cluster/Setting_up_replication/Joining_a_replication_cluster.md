# Присоединение к кластеру репликации

<!-- example joining a replication cluster 1 -->
Чтобы присоединиться к существующему кластеру, необходимо указать как минимум:
* [имя](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) кластера
* `host:port` другого узла в кластере, к которому вы присоединяетесь

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

<!-- example joining a replication cluster 1_1 -->
В большинстве случаев вышеуказанного достаточно, если существует один кластер репликации. Однако, если вы создаёте несколько кластеров репликации, необходимо также задать [путь](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) и убедиться, что директория доступна.

<!-- request SQL -->
```sql
JOIN CLUSTER c2 at '127.0.0.1:10201' 'c2' as path
```
<!-- end -->

Узел присоединяется к кластеру, получая данные от указанного узла и, в случае успеха, обновляет списки узлов на всех остальных узлах кластера так же, как если бы это было сделано вручную через [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md). Этот список используется для повторного присоединения узлов к кластеру при перезапуске.

Существует два списка узлов:
1.`cluster_<name>_nodes_set`: используется для повторного присоединения узлов к кластеру при перезапуске. Он обновляется на всех узлах так же, как это делает [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md). Команда `JOIN CLUSTER` выполняет это обновление автоматически. [Статус кластера](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md) отображает этот список как `cluster_<name>_nodes_set`.
2. `cluster_<name>_nodes_view`: этот список содержит все активные узлы, используемые для репликации, и не требует ручного управления. [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) фактически копирует этот список узлов в список узлов, используемых для повторного присоединения при перезапуске. [Статус кластера](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md) отображает этот список как `cluster_<name>_nodes_view`.

<!-- example joining a replication cluster  2 -->
Когда узлы расположены в разных сетевых сегментах или дата-центрах, опция [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) может быть задана явно. Это минимизирует трафик между узлами и использует шлюзовые узлы для межсоединения между дата-центрами. Следующий код присоединяется к существующему кластеру с использованием опции [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster).

> **Примечание:** список кластера `cluster_<name>_nodes_set` не обновляется автоматически при использовании этого синтаксиса. Для его обновления используйте [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md).


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

Команда `JOIN CLUSTER` работает синхронно и завершается, как только узел получает все данные от других узлов кластера и синхронизируется с ними.

Операция `JOIN CLUSTER` может завершиться с ошибкой, указывающей на дублирование [server_id](../../Server_settings/Searchd.md#server_id). Это происходит, когда присоединяющийся узел имеет такой же `server_id`, как и существующий узел в кластере. Чтобы решить эту проблему, убедитесь, что каждый узел в кластере репликации имеет уникальный [server_id](../../Server_settings/Searchd.md#server_id). Вы можете изменить значение по умолчанию [server_id](../../Server_settings/Searchd.md#server_id) в разделе "searchd" вашего конфигурационного файла на уникальное значение перед попыткой присоединения к кластеру.
<!-- proofread -->

