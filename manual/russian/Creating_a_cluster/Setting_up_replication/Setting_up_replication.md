# Настройка репликации

С Manticore транзакции записи (такие как `INSERT`, `REPLACE`, `DELETE`, `TRUNCATE`, `UPDATE`, `COMMIT`) могут быть реплицированы на другие узлы кластера до того, как транзакция будет полностью применена на текущем узле. В настоящее время репликация поддерживается для таблиц `percolate`, `rt` и `distributed` в Linux и macOS.

[Нативные Windows-архивы](../../Installation/Windows.md#Installing-Manticore-as-native-Windows-binaries) для Manticore не поддерживают репликацию. Мы рекомендуем [установить Manticore с помощью WSL](../../Installation/Windows.md#Installing-or-enabling-WSL2) (Подсистема Windows для Linux).

На [macOS](../../Installation/MacOS.md) репликация имеет ограниченную поддержку и рекомендуется только для целей разработки.

Репликация Manticore поддерживается [библиотекой Galera](https://github.com/codership/galera) и обладает несколькими впечатляющими особенностями:

* Истинная многомастеровость: читайте и пишите на любом узле в любое время.
* [Практически синхронная репликация](https://galeracluster.com/library/documentation/overview.html) без задержек отстающих узлов и потерь данных после сбоя узла.
* Горячий резерв: отсутствие времени простоя при переключении на резервный узел (так как переключение на резервный узел не требуется).
* Плотная связка: все узлы имеют одно и то же состояние, и не допускается расхождение данных между узлами.
* Автоматическое развертывание узлов: нет необходимости вручную создавать резервные копии базы данных и восстанавливать ее на новом узле.
* Легко использовать и развертывать.
* Обнаружение и автоматическое исключение ненадежных узлов.
* Репликация на основе сертификации.

Чтобы настроить репликацию в Manticore Search:

* Опция [data_dir](../../Server_settings/Searchd.md#data_dir) должна быть задана в разделе "searchd" конфигурационного файла. Репликация не поддерживается в обычном режиме.
* Следует указать директиву [listen](../../Server_settings/Searchd.md#listen), содержащую IP-адрес, доступный другим узлам, или [node_address](../../Server_settings/Searchd.md#node_address) с доступным IP-адресом.
* При желании вы можете установить уникальные значения для [server_id](../../Server_settings/Searchd.md#server_id) на каждом узле кластера. Если значение не задано, узел попытается использовать MAC-адрес или случайное число для генерации `server_id`.

Если директива `replication` [listen](../../Server_settings/Searchd.md#listen) не задана, Manticore будет использовать первые два свободных порта из диапазона 200 портов после стандартного порта прослушивания протокола для каждого созданного кластера. Чтобы вручную задать порты репликации, необходимо определить диапазон портов в директиве [listen](../../Server_settings/Searchd.md#listen) (типа `replication`), и пары адрес/порт не должны пересекаться между различными узлами на одном сервере. Как правило, диапазон портов должен указывать как минимум два порта на кластер. Когда вы определяете слушатель репликации с диапазоном портов (например, `listen = 192.168.0.1:9320-9328:replication`), Manticore не начинает сразу прослушивание на этих портах. Вместо этого он будет использовать случайные свободные порты из указанного диапазона только когда вы начнете использовать репликацию.

## Репликационный кластер

Репликационный кластер - это группа узлов, в которой реплицируется транзакция записи. Репликация настраивается на основе таблиц, что означает, что одна таблица может принадлежать только одному кластеру. Нет ограничений на количество таблиц, которые может иметь кластер. Все транзакции, такие как `INSERT`, `REPLACE`, `DELETE`, `TRUNCATE`, на любой таблице перколации или реальном времени, принадлежащей кластеру, реплицируются на все остальные узлы этого кластера. [Распределенные](../../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md#Creating-a-distributed-table) таблицы также могут быть частью процесса репликации. Репликация выполнена в режиме многомастеровости, так что записи на любой узел или несколько узлов одновременно будут работать так же хорошо.

Чтобы создать кластер, вы можете обычно использовать команду [create cluster](../../Creating_a_cluster/Setting_up_replication/Creating_a_replication_cluster.md#Creating-a-replication-cluster) с `CREATE CLUSTER <имя кластера>`, а чтобы присоединиться к кластеру, вы можете использовать [join cluster](../../Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md#Joining-a-replication-cluster) с `JOIN CLUSTER <имя кластера> at 'host:port'`. Однако в некоторых редких случаях вам может понадобиться тонко настроить параметры `CREATE/JOIN CLUSTER`. Доступные параметры:

### name

Этот параметр указывает имя кластера. Оно должно быть уникальным среди всех кластеров в системе.

> **Примечание:** Максимально допустимая длина имени хоста для команды `JOIN` составляет **253** символа. Если вы превышаете этот лимит, searchd сгенерирует ошибку.

### path

Опция пути указывает каталог данных для [репликации кэша наборов записей](https://galeracluster.com/library/documentation/state-transfer.html#state-transfer-gcache) и входящих таблиц от других узлов. Это значение должно быть уникальным среди всех кластеров в системе и должно задаваться как относительный путь к каталогу [data_dir](../../Server_settings/Searchd.md#data_dir). По умолчанию оно устанавливается в значение [data_dir](../../Server_settings/Searchd.md#data_dir).

### nodes

Опция `nodes` - это список пар адрес:порт для всех узлов в кластере, разделенных запятыми. Этот список должен быть получен с помощью API-интерфейса узла и может включать адрес текущего узла. Он используется для присоединения узла к кластеру и его повторного присоединения после перезагрузки.

### options

Опция `options` позволяет передавать дополнительные параметры напрямую в плагин репликации Galera, как указано в [Параметрах документации Galera](https://galeracluster.com/library/documentation/galera-parameters.html)

## Операторы записи

<!-- example write statements 1 -->
Когда вы работаете с кластером репликации, все инструкции на запись, такие как `INSERT`, `REPLACE`, `DELETE`, `TRUNCATE`, `UPDATE`, которые изменяют содержимое таблицы кластера, должны использовать выражение `cluster_name:table_name` вместо имени таблицы. Это обеспечивает распространение изменений на все реплики в кластере. Если правильное выражение не используется, будет вызвана ошибка.

В JSON-интерфейсе свойство `cluster` должно быть указано вместе с именем `table` для всех инструкций на запись в таблицу кластера. Несоответствие задания свойства `cluster` приведет к ошибке.

[Auto ID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) для таблицы в кластере должен быть действительным, так как [server_id](../../Server_settings/Searchd.md#server_id) правильно настроен.

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

## Чтение заявлений

<!-- example write statements 2 -->
Заявления на чтение, такие как `SELECT`, `CALL PQ`, `DESCRIBE`, могут использовать либо обычные имена таблиц, которые не предваряются именем кластера, либо они могут использовать формат `cluster_name:table_name`. Если используется последний, компонент `cluster_name` игнорируется.

При использовании HTTP-эндпоинта `json/search` свойство `cluster` может быть указано при желании, но его также можно опустить.


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
Опции плагина репликации могут быть настроены с помощью инструкции `SET`.

Список доступных опций можно найти в [Параметры документации Galera](https://galeracluster.com/library/documentation/galera-parameters.html).


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

## Кластер с разошедшимися узлами

<!-- example cluster with diverged nodes  1 -->
Возможно, что реплицируемые узлы расходятся друг с другом, что приводит к состоянию, где все узлы помечены как `non-primary`. Это может произойти в результате сетевого разделения между узлами, сбоя кластера или если плагин репликации сталкивается с исключением при определении `primary component`. В такой ситуации необходимо выбрать узел и повысить его до роли `primary component`.

Чтобы определить узел, который нуждается в повышении, следует сравнить значение переменной состояния кластера `last_committed` на всех узлах. Если все серверы в настоящее время работают, нет необходимости перезапускать кластер. Вместо этого можно просто повысить узел с наибольшим значением last_committed до `primary component` с помощью оператора `SET` (как показано в примере).

Затем другие узлы reconnect к primary component и синхронизируют свои данные на основе этого узла.


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
Чтобы использовать репликацию, необходимо определить один [listen](../../Server_settings/Searchd.md#listen) порт для протокола SphinxAPI и один [listen](../../Server_settings/Searchd.md#listen) для адреса и диапазона портов репликации в файле конфигурации. Также укажите папку [data_dir](../../Server_settings/Searchd.md#data_dir) для получения входящих таблиц.


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
Чтобы реплицировать таблицы, необходимо создать кластер на сервере, который имеет локальные таблицы для репликации.

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
Все другие узлы, которые хотят получить реплику таблиц кластера, должны присоединиться к кластеру следующим образом:


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
utilsApi.sql('JOIN CLUSTER posts AT '192.168.1.101:9312'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('JOIN CLUSTER posts AT '192.168.1.101:9312'')
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await utilsApi.sql('JOIN CLUSTER posts AT '192.168.1.101:9312'');
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
Когда выполняются запросы, добавьте имя таблицы перед именем кластера `posts`: или используйте свойство `cluster` для объекта HTTP-запроса.


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
# Настройка репликации

С помощью Manticore операции записи (такие как `INSERT`, `REPLACE`, `DELETE`, `TRUNCATE`, `UPDATE`, `COMMIT`) могут быть реплицированы на другие узлы кластера до того, как транзакция будет полностью применена на текущем узле. В настоящее время репликация поддерживается для таблиц `percolate`, `rt` и `distributed` в Linux и macOS.

[Нативные Windows бинарные файлы](../../Installation/Windows.md#Installing-Manticore-as-native-Windows-binaries) для Manticore не поддерживают репликацию. Рекомендуем [установить Manticore через WSL](../../Installation/Windows.md#Installing-or-enabling-WSL2) (Подсистема Windows для Linux).

В [macOS](../../Installation/MacOS.md) репликация имеет ограниченную поддержку и рекомендуется только для целей разработки.

Репликация Manticore работает на базе [библиотеки Galera](https://github.com/codership/galera) и обладает несколькими впечатляющими функциями:

* Истинная многомастерская: чтение и запись в любой узел в любое время.
* [Практически синхронная репликация](https://galeracluster.com/library/documentation/overview.html): отсутствие задержки на слейвах и отсутствие потери данных после сбоя узла.
* Горячий резерв: отсутствие времени простоя во время переключения (поскольку нет переключения).
* Плотно связанные: все узлы имеют одно и то же состояние, и отсутствие расхождений в данных между узлами не допускается.
* Автоматическое предоставление узлов: нет необходимости вручную создавать резервную копию базы данных и восстанавливать ее на новом узле.
* Легкость в использовании и развертывании.
* Обнаружение и автоматическое исключение ненадежных узлов.
* Репликация на основе сертификации.

Чтобы настроить репликацию в Manticore Search:

* Опция [data_dir](../../Server_settings/Searchd.md#data_dir) должна быть установлена в разделе "searchd" конфигурационного файла. Репликация не поддерживается в обычном режиме.
* Директива [listen](../../Server_settings/Searchd.md#listen) должна быть указана, содержащая IP-адрес, доступный для других узлов, или [node_address](../../Server_settings/Searchd.md#node_address) с доступным IP-адресом.
* По желанию, вы можете установить уникальные значения для [server_id](../../Server_settings/Searchd.md#server_id) на каждом узле кластера. Если значение не установлено, узел попытается использовать MAC-адрес или случайное число для генерации `server_id`.

Если директива `replication` [listen](../../Server_settings/Searchd.md#listen) не установлена, Manticore будет использовать первые два свободных порта в диапазоне 200 портов после порта по умолчанию для каждого созданного кластера. Чтобы вручную установить порты репликации, диапазон портов директивы [listen](../../Server_settings/Searchd.md#listen) (типа `replication`) должен быть определен, и пары адрес/порт не должны пересекаться между разными узлами на одном сервере. Как правило, диапазон портов должен указывать как минимум два порта на кластер. Когда вы определяете слушатель репликации с диапазоном портов (например, `listen = 192.168.0.1:9320-9328:replication`), Manticore не начинает немедленно прослушивать эти порты. Вместо этого он будет использовать случайные свободные порты из указанного диапазона только тогда, когда вы начнете использовать репликацию.

## Репликационный кластер

Репликационный кластер — это группа узлов, в которой реплицируется операция записи. Репликация настраивается на основе таблиц, что означает, что одна таблица может принадлежать только одному кластеру. Нет ограничений на количество таблиц, которые может иметь кластер. Все транзакции, такие как `INSERT`, `REPLACE`, `DELETE`, `TRUNCATE` на любой таблице percolate или реального времени, которая принадлежит кластеру, реплицируются на все остальные узлы в этом кластере. [Распределенные](../../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md#Creating-a-distributed-table) таблицы также могут участвовать в процессе репликации. Репликация является многомастерской, поэтому записи в любой узел или в несколько узлов одновременно будут работать так же хорошо.

Чтобы создать кластер, вы можете обычно использовать команду [create cluster](../../Creating_a_cluster/Setting_up_replication/Creating_a_replication_cluster.md#Creating-a-replication-cluster) с `CREATE CLUSTER <имя кластера>`, а чтобы присоединиться к кластеру, можно использовать [join cluster](../../Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md#Joining-a-replication-cluster) с `JOIN CLUSTER <имя кластера> at 'host:port'`. Однако в некоторых редких случаях вы можете захотеть настроить поведение `CREATE/JOIN CLUSTER`. Доступные параметры:

### имя

Этот параметр указывает имя кластера. Оно должно быть уникальным среди всех кластеров в системе.

> **Примечание:** Максимально допустимая длина имени узла для команды `JOIN` составляет **253** символа. Если вы превысите этот лимит, searchd сгенерирует ошибку.

### путь

Параметр пути указывает каталог данных для [репликации кеша write-set](https://galeracluster.com/library/documentation/state-transfer.html#state-transfer-gcache) и входящих таблиц от других узлов. Это значение должно быть уникальным среди всех кластеров в системе и должно указываться как относительный путь к каталогу [data_dir](../../Server_settings/Searchd.md#data_dir). По умолчанию оно устанавливается на значение [data_dir](../../Server_settings/Searchd.md#data_dir).

### узлы

Параметр `nodes` представляет собой список пар адрес:порт для всех узлов в кластере, разделенных запятыми. Этот список должен быть получен с помощью интерфейса API узла и может включать адрес текущего узла. Он используется для присоединения узла к кластеру и повторного присоединения после перезапуска.

### параметры

Параметр `options` позволяет передавать дополнительные параметры непосредственно плагину репликации Galera, как указано в [Параметрах документации Galera](https://galeracluster.com/library/documentation/galera-parameters.html)

## Операции записи

<!-- example write statements 1 -->
При работе с кластером репликации, все операторы записи, такие как `INSERT`, `REPLACE`, `DELETE`, `TRUNCATE`, `UPDATE`, которые изменяют содержимое таблицы кластера, должны использовать выражение `cluster_name:table_name` вместо имени таблицы. Это гарантирует, что изменения будут распространены на все реплики в кластере. Если не используется правильное выражение, будет вызвана ошибка.

В интерфейсе JSON свойство `cluster` должно быть установлено вместе с именем `table` для всех операторов записи в таблицу кластера. Если свойство `cluster` не установлено, это приведет к ошибке.

[Auto ID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) для таблицы в кластере должен быть действительным, если [server_id](../../Server_settings/Searchd.md#server_id) настроен правильно.

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
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "weekly_index", cluster:posts, id: 1, doc: doc);
var sqlresult = indexApi.Insert(newdoc);

DeleteDocumentRequest deleteDocumentRequest = new DeleteDocumentRequest(index: "weekly_index", cluster: "posts", id: 1);
indexApi.Delete(deleteDocumentRequest);
```
<!-- end -->

## Операторы чтения

<!-- example write statements 2 -->
Операторы чтения, такие как `SELECT`, `CALL PQ`, `DESCRIBE`, могут использовать либо обычные названия таблиц без добавления имени кластера, либо формат `cluster_name:table_name`. При использовании последнего компонента `cluster_name` игнорируется.

При использовании HTTP-эндпоинта `json/search` свойство `cluster` может быть указано по желанию, но оно также может быть пропущено.


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
Опции плагина репликации могут быть изменены с помощью оператора `SET`.

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

## Кластер с расхождениями узлов

<!-- example cluster with diverged nodes  1 -->
Возможно, что реплицированные узлы расходятся друг с другом, приводя к состоянию, когда все узлы помечены как `non-primary`. Это может произойти в результате разделения сети между узлами, аварии кластера или если плагин репликации столкнется с исключением при определении `primary component`. В такой ситуации необходимо выбрать узел и повысить его до роли `primary component`.

Чтобы идентифицировать узел, который нужно повысить, следует сравнить значение переменной состояния `last_committed` на всех узлах. Если все серверы в настоящее время работают, нет необходимости перезапускать кластер. Вместо этого можно просто повысить узел с наивысшим значением last_committed до `primary component` с помощью оператора `SET` (как показано в примере).

Остальные узлы затем переподключатся к основному компоненту и синхронизируют свои данные на основе этого узла.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1
```
<!-- request JSON -->

```json
POST /cli -d "
УСТАНОВИТЕ КЛАСТЕР posts GLOBAL 'pc.bootstrap' = 1
"
```
<!-- end -->

## Репликация и кластер

<!-- пример репликации и кластера 1 -->
Чтобы использовать репликацию, вам нужно определить один [listen](../../Server_settings/Searchd.md#listen) порт для протокола SphinxAPI и один [listen](../../Server_settings/Searchd.md#listen) для адреса и диапазона портов репликации в файле конфигурации. Также укажите папку [data_dir](../../Server_settings/Searchd.md#data_dir) для получения входящих таблиц.


<!-- intro -->
##### ini:

<!-- запрос ini -->
```ini
searchd {
  listen   = 9312
  listen   = 192.168.1.101:9360-9370:репликация
  data_dir = /var/lib/manticore/
  ...
 }
```
<!-- end -->

<!-- пример репликации и кластера 2 -->
Чтобы реплицировать таблицы, вы должны создать кластер на сервере, который имеет локальные таблицы для репликации.

<!-- intro -->
##### SQL:

<!-- запрос SQL -->

```sql
СОЗДАТЬ КЛАСТЕР posts
```

<!-- запрос JSON -->

```json
POST /cli -d "
СОЗДАТЬ КЛАСТЕР posts
"
```

<!-- запрос PHP -->

```php
$params = [
    'cluster' => 'posts'
    ]
];
$response = $client->cluster()->create($params);
```
<!-- intro -->
##### Python:

<!-- запрос Python -->

```python
utilsApi.sql('СОЗДАТЬ КЛАСТЕР posts')
```
<!-- intro -->
##### Javascript:

<!-- запрос Javascript -->

```javascript
res = await utilsApi.sql('СОЗДАТЬ КЛАСТЕР posts');
```

<!-- intro -->
##### Java:

<!-- запрос Java -->

```java
utilsApi.sql("СОЗДАТЬ КЛАСТЕР posts");

```

<!-- intro -->
##### C#:

<!-- запрос C# -->

```clike
utilsApi.Sql("СОЗДАТЬ КЛАСТЕР posts");

```
<!-- end -->

<!-- пример репликации и кластера 3 -->
Добавьте эти локальные таблицы в кластер


<!-- intro -->
##### SQL:

<!-- запрос SQL -->

```sql
ИЗМЕНИТЬ КЛАСТЕР posts ДОБАВИТЬ pq_title
ИЗМЕНИТЬ КЛАСТЕР posts ДОБАВИТЬ pq_clicks
```

<!-- запрос JSON -->

```json
POST /cli -d "
ИЗМЕНИТЬ КЛАСТЕР posts ДОБАВИТЬ pq_title
"
POST /cli -d "
ИЗМЕНИТЬ КЛАСТЕР posts ДОБАВИТЬ pq_clicks
"
```

<!-- запрос PHP -->

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

<!-- запрос Python -->

```python
utilsApi.sql('ИЗМЕНИТЬ КЛАСТЕР posts ДОБАВИТЬ pq_title')
utilsApi.sql('ИЗМЕНИТЬ КЛАСТЕР posts ДОБАВИТЬ pq_clicks')
```
<!-- intro -->
##### Javascript:

<!-- запрос Javascript -->

```javascript
res = await utilsApi.sql('ИЗМЕНИТЬ КЛАСТЕР posts ДОБАВИТЬ pq_title');
res = await utilsApi.sql('ИЗМЕНИТЬ КЛАСТЕР posts ДОБАВИТЬ pq_clicks');
```

<!-- intro -->
##### Java:

<!-- запрос Java -->

```java
utilsApi.sql("ИЗМЕНИТЬ КЛАСТЕР posts ДОБАВИТЬ pq_title");
utilsApi.sql("ИЗМЕНИТЬ КЛАСТЕР posts ДОБАВИТЬ pq_clicks");
```

<!-- intro -->
##### C#:

<!-- запрос C# -->

```clike
utilsApi.Sql("ИЗМЕНИТЬ КЛАСТЕР posts ДОБАВИТЬ pq_title");
utilsApi.Sql("ИЗМЕНИТЬ КЛАСТЕР posts ДОБАВИТЬ pq_clicks");
```

<!-- end -->

<!-- пример репликации и кластера 4 -->
Все остальные узлы, которые хотят получить копию таблиц кластера, должны присоединиться к кластеру следующим образом:


<!-- intro -->
##### SQL:

<!-- запрос SQL -->

```sql
ПРИСОЕДИНИТЬСЯ К КЛАСТЕРУ posts AT '192.168.1.101:9312'
```

<!-- запрос JSON -->

```json
POST /cli -d "
ПРИСОЕДИНИТЬСЯ К КЛАСТЕРУ posts AT '192.168.1.101:9312'
"
```

<!-- запрос PHP -->

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

<!-- запрос Python -->

```python
utilsApi.sql('ПРИСОЕДИНИТЬСЯ К КЛАСТЕРУ posts AT \'192.168.1.101:9312\'')
```
<!-- intro -->
##### Javascript:

<!-- запрос Javascript -->

```javascript
res = await utilsApi.sql('ПРИСОЕДИНИТЬСЯ К КЛАСТЕРУ posts AT \'192.168.1.101:9312\'');
```

<!-- intro -->
##### Java:

<!-- запрос Java -->

```java
utilsApi.sql("ПРИСОЕДИНИТЬСЯ К КЛАСТЕРУ posts AT '192.168.1.101:9312'");

```

<!-- intro -->
##### C#:

<!-- запрос C# -->

```clike
utilsApi.Sql("ПРИСОЕДИНИТЬСЯ К КЛАСТЕРУ posts AT '192.168.1.101:9312'");

```
<!-- end -->

<!-- пример репликации и кластера 5 -->
При выполнении запросов предваряйте имя таблицы именем кластера `posts`: или используйте свойство `cluster` для объекта HTTP-запроса.


<!-- intro -->
##### SQL:

<!-- запрос SQL -->

```sql
ВСТАВИТЬ В posts:pq_title ЗНАЧЕНИЯ ( 3, 'тестируй меня' )
```

<!-- запрос JSON -->

```json
POST /insert -d '
{
  "cluster":"posts",
  "table":"pq_title",
  "id": 3
  "doc":
  {
    "title" : "тестируй меня"
  }
}'
```

<!-- запрос PHP -->

```php
$index->addDocuments([
        3, ['title' => 'тестируй меня']
]);

```
<!-- intro -->
##### Python:

<!-- запрос Python -->

``` python
indexApi.insert({"cluster":"posts","table":"pq_title","id":3"doc":{"title":"тестируй меня"}})

```
<!-- intro -->
##### Javascript:

<!-- запрос Javascript -->

``` javascript
res = await indexApi.insert({"cluster":"posts","table":"pq_title","id":3"doc":{"title":"тестируй меня"}});
```

<!-- intro -->
##### java:

<!-- запрос Java -->

``` java
InsertDocumentRequest newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","тестируй меня");
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
<!-- end -->

Все запросы, которые изменяют таблицы в кластере, теперь реплицируются на все узлы в кластере.
<!-- proofread -->
