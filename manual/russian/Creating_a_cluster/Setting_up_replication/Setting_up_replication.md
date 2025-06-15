# Настройка репликации

С Manticore транзакции записи (такие как `INSERT`, `REPLACE`, `DELETE`, `TRUNCATE`, `UPDATE`, `COMMIT`) могут быть реплицированы на другие узлы кластера до того, как транзакция будет полностью применена на текущем узле. В настоящее время репликация поддерживается для таблиц типов `percolate`, `rt` и `distributed` в Linux и macOS.

[Нативные бинарные файлы для Windows](../../Installation/Windows.md#Installing-Manticore-as-native-Windows-binaries) для Manticore не поддерживают репликацию. Рекомендуется [устанавливать Manticore через WSL](../../Installation/Windows.md#Installing-or-enabling-WSL2) (Windows Subsystem for Linux).

На [macOS](../../Installation/MacOS.md) репликация имеет ограниченную поддержку и рекомендуется только для целей разработки.

Репликация Manticore основана на библиотеке [Galera](https://github.com/codership/galera) и обладает несколькими впечатляющими особенностями:

* Настоящий multi-master: чтение и запись возможны на любом узле в любое время.
* [Почти синхронная репликация](https://galeracluster.com/library/documentation/overview.html) — отсутствует задержка реплики и потери данных после сбоя узла.
* Режим горячего резерва: отсутствие простоев при переключении (failover отсутствует).
* Плотная синхронизация: все узлы имеют одинаковое состояние и не допускается рассогласование данных между узлами.
* Автоматическое добавление узлов: не требуется вручную создавать резервные копии базы и восстанавливать их на новом узле.
* Простота использования и развертывания.
* Обнаружение и автоматическое исключение ненадежных узлов.
* Репликация на основе сертификации.

Чтобы настроить репликацию в Manticore Search:

* В секции "searchd" конфигурационного файла должна быть задана опция [data_dir](../../Server_settings/Searchd.md#data_dir). Репликация не поддерживается в plain-режиме.
* Должна быть указана директива [listen](../../Server_settings/Searchd.md#listen), содержащая IP-адрес, доступный другим узлам, либо [node_address](../../Server_settings/Searchd.md#node_address) с доступным IP-адресом.
* Опционально можно задать уникальные значения для [server_id](../../Server_settings/Searchd.md#server_id) на каждом узле кластера. Если значение не задано, узел попытается использовать MAC-адрес или случайное число для генерации `server_id`.

Если не задана директива `replication` в [listen](../../Server_settings/Searchd.md#listen), Manticore использует первые два свободных порта из диапазона 200 портов после порта стандартного протокола прослушивания для каждого созданного кластера. Для ручной установки портов репликации нужно определить диапазон портов в директиве [listen](../../Server_settings/Searchd.md#listen) (типа `replication`), при этом адреса и диапазоны портов не должны пересекаться между разными узлами на одном сервере. В общем случае диапазон портов должен включать минимум два порта на кластер. При определении слушателя репликации с диапазоном портов (например, `listen = 192.168.0.1:9320-9328:replication`) Manticore не начинает слушать эти порты сразу. Вместо этого он выделяет случайные свободные порты из указанного диапазона только при начале использования репликации.

## Кластер репликации

Кластер репликации — это группа узлов, в которой транзакция записи реплицируется. Репликация настраивается для каждой таблицы отдельно, т.е. одна таблица может принадлежать только одному кластеру. Нет ограничений на количество таблиц в кластере. Все транзакции, такие как `INSERT`, `REPLACE`, `DELETE`, `TRUNCATE` для любой percolate или реального времени таблицы, принадлежащей кластеру, реплицируются на все остальные узлы в этом кластере. Таблицы типа [Distributed](../../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md#Creating-a-distributed-table) также могут участвовать в процессе репликации. Репликация поддерживает multi-master, поэтому запись на любой узел или несколько узлов одновременно работает без проблем.

Для создания кластера обычно используется команда [create cluster](../../Creating_a_cluster/Setting_up_replication/Creating_a_replication_cluster.md#Creating-a-replication-cluster) с `CREATE CLUSTER <cluster name>`, а для присоединения к кластеру — команда [join cluster](../../Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md#Joining-a-replication-cluster) с `JOIN CLUSTER <cluster name> at 'host:port'`. Однако в редких случаях может понадобиться точная настройка поведения `CREATE/JOIN CLUSTER`. Доступные опции:

### name

Эта опция задает имя кластера. Оно должно быть уникальным среди всех кластеров в системе.

> **Примечание:** Максимальная длина имени хоста для команды `JOIN` ограничена **253** символами. При превышении этого лимита searchd выдаст ошибку.

### path

Опция path указывает директорию данных для [write-set cache replication](https://galeracluster.com/library/documentation/state-transfer.html#state-transfer-gcache) и входящих таблиц с других узлов. Это значение должно быть уникальным для всех кластеров в системе и задаваться как относительный путь к каталогу [data_dir](../../Server_settings/Searchd.md#data_dir). По умолчанию оно совпадает со значением [data_dir](../../Server_settings/Searchd.md#data_dir).

### nodes

Опция `nodes` представляет собой список пар адрес:порт всех узлов кластера, разделенных запятыми. Этот список получается с помощью API узла и может включать адрес текущего узла. Используется для присоединения узла к кластеру и повторного присоединения после перезапуска.

### options

Опция `options` позволяет передавать дополнительные параметры непосредственно плагину репликации Galera, как описано в [документации Galera по параметрам](https://galeracluster.com/library/documentation/galera-parameters.html).

## Операторы записи

<!-- example write statements 1 -->
При работе с кластером репликации все операторы записи, такие как `INSERT`, `REPLACE`, `DELETE`, `TRUNCATE`, `UPDATE`, которые модифицируют содержимое таблицы кластера, должны использовать выражение `cluster_name:table_name` вместо имени таблицы. Это гарантирует, что изменения будут распространяться на все реплики в кластере. Если используется неправильное выражение, будет вызвана ошибка.

В JSON-интерфейсе свойство `cluster` должно быть установлено вместе с именем `table` для всех инструкций записи в таблицу кластера. Если свойство `cluster` не установлено, будет выдана ошибка.

[Авто ID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) для таблицы в кластере должен быть действительным, если корректно настроен [server_id](../../Server_settings/Searchd.md#server_id).

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

## Запросы на чтение

<!-- example write statements 2 -->
Запросы на чтение, такие как `SELECT`, `CALL PQ`, `DESCRIBE`, могут использовать либо обычные имена таблиц без префикса с именем кластера, либо формат `cluster_name:table_name`. Если используется второй вариант, компонент `cluster_name` игнорируется.

При использовании HTTP-эндоинта `json/search` свойство `cluster` можно указать, если требуется, но его также можно опустить.


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
Параметры плагина репликации можно настроить с помощью инструкции `SET`.

Список доступных параметров можно найти в [Документации Galera Parameters](https://galeracluster.com/library/documentation/galera-parameters.html) .


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
Возможна ситуация, когда реплицируемые узлы расходятся друг с другом, приводя к состоянию, в котором все узлы отмечены как `non-primary`. Это может произойти вследствие сетевого разрыва между узлами, сбоя кластера или исключения плагина репликации при определении `primary component`. В таком случае необходимо выбрать узел и повысить его до роли `primary component`.

Для определения узла, который необходимо повысить, следует сравнить значение переменной статуса кластера `last_committed` на всех узлах. Если все серверы сейчас работают, перезапуск кластера не требуется. Можно просто повысить узел с наибольшим значением `last_committed` до `primary component` с помощью инструкции `SET` (как показано в примере).

Другие узлы затем переподключатся к primary component и повторно синхронизируют свои данные на основе этого узла.


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
Для использования репликации необходимо в конфигурационном файле определить один порт [listen](../../Server_settings/Searchd.md#listen) для протокола SphinxAPI и один  [listen](../../Server_settings/Searchd.md#listen) для адреса и порта репликации. Также укажите папку [data_dir](../../Server_settings/Searchd.md#data_dir) для приёма входящих таблиц.


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
Для репликации таблиц необходимо создать кластер на сервере, который имеет локальные таблицы, подлежащие репликации.

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
Все остальные узлы, желающие получать реплику таблиц кластера, должны присоединиться к кластеру следующим образом:


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
При выполнении запросов добавляйте префикс имени таблицы с именем кластера `posts`: или используйте свойство `cluster` в HTTP-запросе.


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

Все запросы, изменяющие таблицы в кластере, теперь реплицируются на все узлы кластера.
<!-- proofread -->

