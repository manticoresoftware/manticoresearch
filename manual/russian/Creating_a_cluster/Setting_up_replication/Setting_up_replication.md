# Настройка репликации

С Manticore транзакции записи (такие как `INSERT`, `REPLACE`, `DELETE`, `TRUNCATE`, `UPDATE`, `COMMIT`) могут реплицироваться на другие узлы кластера до полного применения транзакции на текущем узле. В настоящее время репликация поддерживается для таблиц `percolate`, `rt` и `distributed` в Linux и macOS.

[Нативные бинарные файлы Windows](../../Installation/Windows.md#Installing-Manticore-as-native-Windows-binaries) для Manticore не поддерживают репликацию. Мы рекомендуем [устанавливать Manticore через WSL](../../Installation/Windows.md#Installing-or-enabling-WSL2) (Подсистема Windows для Linux).

На [macOS](../../Installation/MacOS.md) репликация имеет ограниченную поддержку и рекомендуется только для целей разработки.

Репликация Manticore базируется на библиотеке [Galera](https://github.com/codership/galera) и обладает несколькими впечатляющими возможностями:

* Истинный multi-master: чтение и запись на любой узел в любое время.
* [Почти синхронная репликация](https://galeracluster.com/library/documentation/overview.html) — отсутствие задержек ведомых и потери данных после сбоя узла.
* Горячий резерв: отсутствие времени простоя при переключении (так как переключения нет).
* Плотно связанные узлы: все узлы содержат одно и то же состояние, расхождений данных между узлами не допускается.
* Автоматическое добавление узлов: нет необходимости вручную создавать резервные копии базы и восстанавливать их на новом узле.
* Простота использования и развёртывания.
* Обнаружение и автоматическое удаление ненадёжных узлов.
* Репликация на основе сертификации.

Для настройки репликации в Manticore Search:

* Опция [data_dir](../../Server_settings/Searchd.md#data_dir) должна быть задана в разделе "searchd" конфигурационного файла. Репликация не поддерживается в plain-режиме.
* Директива [listen](../../Server_settings/Searchd.md#listen) должна содержать IP-адрес, доступный для других узлов, либо должен быть указан [node_address](../../Server_settings/Searchd.md#node_address) с доступным IP-адресом.
* По желанию, можно задать уникальные значения для [server_id](../../Server_settings/Searchd.md#server_id) на каждом узле кластера. Если значение не задано, узел попытается использовать MAC-адрес или случайное число для генерации `server_id`.

Если директива `replication` [listen](../../Server_settings/Searchd.md#listen) не задана, Manticore использует первые два свободных порта из диапазона в 200 портов после порта прослушивания протокола по умолчанию для каждого созданного кластера. Для ручного задания портов репликации необходимо определить диапазон портов в директиве [listen](../../Server_settings/Searchd.md#listen) (типа `replication`), при этом пары адрес/диапазон портов не должны пересекаться между разными узлами на одном сервере. Как правило, диапазон портов должен содержать как минимум два порта на кластер. Когда вы определяете слушатель репликации с диапазоном портов (например, `listen = 192.168.0.1:9320-9328:replication`), Manticore не начинает прослушивать эти порты сразу. Вместо этого система будет выбирать случайные свободные порты из указанного диапазона только при начале использования репликации.

## Репликационный кластер

Репликационный кластер — это группа узлов, в которой реплицируются транзакции записи. Репликация настраивается на уровне таблицы, то есть одна таблица может принадлежать только одному кластеру. Нет ограничений на количество таблиц в кластере. Все операции `INSERT`, `REPLACE`, `DELETE`, `TRUNCATE` для любой percolate или real-time таблицы, входящей в кластер, реплицируются на все другие узлы этого кластера. В репликацию также могут быть включены [distributed](../../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md#Creating-a-distributed-table) таблицы. Репликация является multi-master, поэтому записи на любой узел или несколько узлов одновременно работают корректно.

Для создания кластера обычно используется команда [create cluster](../../Creating_a_cluster/Setting_up_replication/Creating_a_replication_cluster.md#Creating-a-replication-cluster) с `CREATE CLUSTER <имя кластера>`, а для присоединения к кластеру — команда [join cluster](../../Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md#Joining-a-replication-cluster) с `JOIN CLUSTER <имя кластера> at 'host:port'`. Однако в некоторых редких случаях может понадобиться тонкая настройка поведения `CREATE/JOIN CLUSTER`. Доступные опции:

### name

Эта опция задаёт имя кластера. Оно должно быть уникальным среди всех кластеров в системе.

> **Примечание:** Максимальная длина имени хоста для команды `JOIN` составляет **253** символа. Если предел превышен, searchd выдаст ошибку.

### path

Опция path задаёт каталог данных для [кэша набора записей для репликации](https://galeracluster.com/library/documentation/state-transfer.html#state-transfer-gcache) и для входящих таблиц с других узлов. Это значение должно быть уникальным среди всех кластеров системы и указываться как относительный путь к каталогу [data_dir](../../Server_settings/Searchd.md#data_dir). По умолчанию оно совпадает со значением [data_dir](../../Server_settings/Searchd.md#data_dir).

### nodes

Опция `nodes` — это список адресов и портов всех узлов в кластере, разделённых запятыми. Этот список должен быть получен через API узла и может включать адрес текущего узла. Он используется для присоединения узла к кластеру и для повторного присоединения после перезапуска.

### options

Опция `options` позволяет передавать дополнительные параметры непосредственно плагину репликации Galera, как описано в [Galera Documentation Parameters](https://galeracluster.com/library/documentation/galera-parameters.html)

## Операторы записи

<!-- example write statements 1 -->
При работе с репликационным кластером все операторы записи, такие как `INSERT`, `REPLACE`, `DELETE`, `TRUNCATE`, `UPDATE`, которые изменяют содержимое таблицы кластера, должны использовать выражение `cluster_name:table_name` вместо имени таблицы. Это гарантирует, что изменения будут распространены на все реплики в кластере. Если использовать неправильное выражение, будет выдана ошибка.

В JSON-интерфейсе свойство `cluster` должно быть установлено вместе с именем `table` для всех операций записи в таблицу кластера. Если свойство `cluster` не установлено, это приведет к ошибке.

[Автоматический ID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) для таблицы в кластере будет действителен, если [server_id](../../Server_settings/Searchd.md#server_id) настроен правильно.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
INSERT INTO posts:weekly_index VALUES ( 'iphone case' )
TRUNCATE RTINDEX click_query:weekly_index
UPDATE INTO posts:rt_tags SET tags=(101, 302, 304) WHERE MATCH ('use') AND id IN (1,101,201)
DELETE FROM clicks:rt WHERE MATCH ('dumy') AND gid>206
```

<!-- request JSON -->

```json
POST /insert -d '
{
  "cluster":"posts",
  "table":"weekly_index",
  "doc":
  {
    "title" : "iphone case",
    "price" : 19.85
  }
}'
POST /delete -d '
{
  "cluster":"posts",
  "table": "weekly_index",
  "id":1
}'
```

<!-- request PHP -->

```php
$index->addDocuments([
        1, ['title' => 'iphone case', 'price' => 19.85]
]);
$index->deleteDocument(1);
```

<!-- intro -->
##### Python:

<!-- request Python -->

``` python
indexApi.insert({"cluster":"posts","table":"weekly_index","doc":{"title":"iphone case","price":19.85}})
indexApi.delete({"cluster":"posts","table":"weekly_index","id":1})
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

``` python
await indexApi.insert({"cluster":"posts","table":"weekly_index","doc":{"title":"iphone case","price":19.85}})
await indexApi.delete({"cluster":"posts","table":"weekly_index","id":1})
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

``` javascript
res = await indexApi.insert({"cluster":"posts","table":"weekly_index","doc":{"title":"iphone case","price":19.85}});
 res = await indexApi.delete({"cluster":"posts","table":"weekly_index","id":1});
```

<!-- intro -->
##### java:

<!-- request Java -->

``` java
InsertDocumentRequest newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","Crossbody Bag with Tassel");
    put("price",19.85);
}};
newdoc.index("weekly_index").cluster("posts").id(1L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);

DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest();
deleteRequest.index("weekly_index").cluster("posts").setId(1L);
indexApi.delete(deleteRequest);

```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "Crossbody Bag with Tassel");
doc.Add("price", 19.85);
InsertDocumentRequest newdoc = new InsertDocumentRequest(table: "weekly_index", cluster:posts, id: 1, doc: doc);
var sqlresult = indexApi.Insert(newdoc);

DeleteDocumentRequest deleteDocumentRequest = new DeleteDocumentRequest(table: "weekly_index", cluster: "posts", id: 1);
indexApi.Delete(deleteDocumentRequest);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let mut doc = HashMap::new();
doc.insert("title".to_string(), serde_json::json!("Crossbody Bag with Tassel"));
doc.insert("price".to_string(), serde_json::json!(19.85));
let insert_req = InsertDocumentRequest {
    table: serde_json::json!("weekly_index"),
    doc: serde_json::json!(doc),
    cluster: serde_json::json!("posts"),
    id: serde_json::json!(1),
};
let insert_res = index_api.insert(insert_req).await;

let delete_req = DeleteDocumentRequest {
    table: serde_json::json!("weekly_index"),
    cluster: serde_json::json!("posts"),
    id: serde_json::json!(1),
};
index_api.delete(delete_req).await;
```

<!-- end -->

## Операции чтения

<!-- example write statements 2 -->
Операции чтения, такие как `SELECT`, `CALL PQ`, `DESCRIBE`, могут использовать обычные имена таблиц без префикса имени кластера или могут использовать формат `cluster_name:table_name`. Если используется последний, компонент `cluster_name` игнорируется.

При использовании HTTP-эндпоинта `json/search` свойство `cluster` можно указать при желании, но его также можно опустить.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT * FROM weekly_index
CALL PQ('posts:weekly_index', 'document is here')
```

<!-- request JSON -->

```json
POST /search -d '
{
  "cluster":"posts",
  "table":"weekly_index",
  "query":{"match":{"title":"keyword"}}
}'
POST /search -d '
{
  "table":"weekly_index",
  "query":{"match":{"title":"keyword"}}
}'
```

<!-- end -->

## Параметры кластера

<!-- example cluster parameters 1 -->
Параметры плагина репликации можно настроить с помощью оператора `SET`.

Список доступных опций можно найти в [Galera Documentation Parameters](https://galeracluster.com/library/documentation/galera-parameters.html).


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SET CLUSTER click_query GLOBAL 'pc.bootstrap' = 1
```
<!-- request JSON -->

```json
POST /cli -d "
SET CLUSTER click_query GLOBAL 'pc.bootstrap' = 1
"
```
<!-- end -->

## Кластер с расходящимися узлами

<!-- example cluster with diverged nodes  1 -->
Возможна ситуация, когда реплицированные узлы расходятся друг от друга, что приводит к состоянию, когда все узлы помечены как `non-primary`. Это может произойти в результате сетевого разделения между узлами, сбоя кластера или если плагин репликации столкнется с исключением при определении `primary component`. В таком сценарии необходимо выбрать узел и повысить его до роли `primary component`.

Чтобы определить узел, который нужно повысить, следует сравнить значение переменной состояния кластера `last_committed` на всех узлах. Если все серверы в настоящее время работают, нет необходимости перезапускать кластер. Вместо этого можно просто повысить узел с наибольшим значением last_committed до `primary component` с помощью оператора `SET` (как показано в примере).

Остальные узлы затем переподключатся к основному компоненту и повторно синхронизируют свои данные на основе этого узла.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1
```
<!-- request JSON -->

```json
POST /cli -d "
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1
"
```
<!-- end -->

## Репликация и кластер

<!-- example replication and cluster 1 -->
Для использования репликации необходимо определить один порт [listen](../../Server_settings/Searchd.md#listen) для протокола SphinxAPI и один [listen](../../Server_settings/Searchd.md#listen) для адреса репликации и диапазона портов в файле конфигурации. Также укажите папку [data_dir](../../Server_settings/Searchd.md#data_dir) для получения входящих таблиц.


<!-- intro -->
##### ini:

<!-- request ini -->
```ini
searchd {
  listen   = 9312
  listen   = 192.168.1.101:9360-9370:replication
  data_dir = /var/lib/manticore/
  ...
 }
```
<!-- end -->

<!-- example replication and cluster 2 -->
Для репликации таблиц необходимо создать кластер на сервере, который содержит локальные таблицы для репликации.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE CLUSTER posts
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE CLUSTER posts
"
```

<!-- request PHP -->

```php
$params = [
    'cluster' => 'posts'
    ]
];
$response = $client->cluster()->create($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE CLUSTER posts')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE CLUSTER posts')
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await utilsApi.sql('CREATE CLUSTER posts');
```

<!-- intro -->
##### Java:

<!-- request Java -->

```java
utilsApi.sql("CREATE CLUSTER posts");

```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE CLUSTER posts");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE CLUSTER posts", Some(true)).await;
```

<!-- end -->

<!-- example replication and cluster 3 -->
Добавьте эти локальные таблицы в кластер


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
ALTER CLUSTER posts ADD pq_title
ALTER CLUSTER posts ADD pq_clicks
```

<!-- request JSON -->

```json
POST /cli -d "
ALTER CLUSTER posts ADD pq_title
"
POST /cli -d "
ALTER CLUSTER posts ADD pq_clicks
"
```

<!-- request PHP -->

```php
$params = [
  'cluster' => 'posts',
  'body' => [
     'operation' => 'add',
     'table' => 'pq_title'

  ]
];
$response = $client->cluster()->alter($params);
$params = [
  'cluster' => 'posts',
  'body' => [
     'operation' => 'add',
     'table' => 'pq_clicks'

  ]
];
$response = $client->cluster()->alter($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('ALTER CLUSTER posts ADD pq_title')
utilsApi.sql('ALTER CLUSTER posts ADD pq_clicks')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('ALTER CLUSTER posts ADD pq_title')
await utilsApi.sql('ALTER CLUSTER posts ADD pq_clicks')
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await utilsApi.sql('ALTER CLUSTER posts ADD pq_title');
res = await utilsApi.sql('ALTER CLUSTER posts ADD pq_clicks');
```

<!-- intro -->
##### Java:

<!-- request Java -->

```java
utilsApi.sql("ALTER CLUSTER posts ADD pq_title");
utilsApi.sql("ALTER CLUSTER posts ADD pq_clicks");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("ALTER CLUSTER posts ADD pq_title");
utilsApi.Sql("ALTER CLUSTER posts ADD pq_clicks");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("ALTER CLUSTER posts ADD pq_title", Some(true)).await;
utils_api.sql("ALTER CLUSTER posts ADD pq_clicks", Some(true)).await;
```

<!-- end -->

<!-- example replication and cluster 4 -->
Все остальные узлы, которые хотят получить реплику таблиц кластера, должны присоединиться к кластеру следующим образом:


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
JOIN CLUSTER posts AT '192.168.1.101:9312'
```

<!-- request JSON -->

```json
POST /cli -d "
JOIN CLUSTER posts AT '192.168.1.101:9312'
"
```

<!-- request PHP -->

```php
$params = [
  'cluster' => 'posts',
  'body' => [
      '192.168.1.101:9312'
  ]
];
$response = $client->cluster->join($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('JOIN CLUSTER posts AT \'192.168.1.101:9312\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('JOIN CLUSTER posts AT \'192.168.1.101:9312\'')
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await utilsApi.sql('JOIN CLUSTER posts AT \'192.168.1.101:9312\'');
```

<!-- intro -->
##### Java:

<!-- request Java -->

```java
utilsApi.sql("JOIN CLUSTER posts AT '192.168.1.101:9312'");

```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("JOIN CLUSTER posts AT '192.168.1.101:9312'");

```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("JOIN CLUSTER posts AT '192.168.1.101:9312'", Some(true)).await;

```
<!-- end -->

<!-- example replication and cluster 5 -->
При выполнении запросов добавьте к имени таблицы префикс имени кластера `posts`: или используйте свойство `cluster` для объекта HTTP-запроса.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
INSERT INTO posts:pq_title VALUES ( 3, 'test me' )
```

<!-- request JSON -->

```json
POST /insert -d '
{
  "cluster":"posts",
  "table":"pq_title",
  "id": 3
  "doc":
  {
    "title" : "test me"
  }
}'
```

<!-- request PHP -->

```php
$index->addDocuments([
        3, ['title' => 'test me']
]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

``` python
indexApi.insert({"cluster":"posts","table":"pq_title","id":3"doc":{"title":"test me"}})

```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

``` python
await indexApi.insert({"cluster":"posts","table":"pq_title","id":3"doc":{"title":"test me"}})
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

``` javascript
res = await indexApi.insert({"cluster":"posts","table":"pq_title","id":3"doc":{"title":"test me"}});
```

<!-- intro -->
##### java:

<!-- request Java -->

``` java
InsertDocumentRequest newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","test me");
}};
newdoc.index("pq_title").cluster("posts").id(3L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);
```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "test me");
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "pq_title", cluster: "posts", id: 3, doc: doc);
var sqlresult = indexApi.Insert(newdoc);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

``` rust
let mut doc = HashMap::new();
doc.insert("title".to_string(), serde_json::json!("test me"));
let insert_req = InsertDocumentRequest {
    table: serde_json::json!("pq_title"),
    doc: serde_json::json!(doc),
    cluster: serde_json::json!("posts"),
    id: serde_json::json!(3),
};
let insert_res = index_api.insert(insert_req).await;
```
<!-- end -->

Все запросы, которые изменяют таблицы в кластере, теперь реплицируются на все узлы в кластере.
<!-- proofread -->

