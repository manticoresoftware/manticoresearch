# 设置复制

使用 Manticore，写事务（如 `INSERT`、`REPLACE`、`DELETE`、`TRUNCATE`、`UPDATE`、`COMMIT`）可以在当前节点完全应用事务之前复制到其他集群节点。目前，复制支持 Linux 和 macOS 上的 `percolate`、`rt` 和 `distributed` 表。

Manticore 的[原生 Windows 二进制文件](../../Installation/Windows.md#Installing-Manticore-as-native-Windows-binaries)不支持复制。我们建议通过 [WSL 安装 Manticore](../../Installation/Windows.md#Installing-or-enabling-WSL2)（Windows 子系统 Linux）。

在 [macOS](../../Installation/MacOS.md) 上，复制支持有限，仅建议用于开发目的。

Manticore 的复制由 [Galera 库](https://github.com/codership/galera) 提供支持，具有以下显著特性：

* 真正的多主：任何时间可对任意节点进行读写。
* [几乎同步复制](https://galeracluster.com/library/documentation/overview.html)，无从属延迟，节点崩溃后无数据丢失。
* 热备份：故障切换期间无停机（因为没有故障切换）。
* 紧密耦合：所有节点保持相同状态，不允许节点间数据分歧。
* 自动节点配置：无需手动备份数据库并在新节点恢复。
* 易于使用和部署。
* 不可靠节点的检测和自动剔除。
* 基于认证的复制。

要在 Manticore Search 中设置复制：

* 必须在配置文件的 "searchd" 部分设置 [data_dir](../../Server_settings/Searchd.md#data_dir) 选项。纯模式不支持复制。
* 必须指定一个包含其他节点可访问 IP 地址的 [listen](../../Server_settings/Searchd.md#listen) 指令，或带有可访问 IP 地址的 [node_address](../../Server_settings/Searchd.md#node_address)。
* 可选地，可以在每个集群节点上为 [server_id](../../Server_settings/Searchd.md#server_id) 设置唯一值。如果未设置，节点将尝试使用 MAC 地址或随机数生成 `server_id`。

如果未设置 `replication` 类型的 [listen](../../Server_settings/Searchd.md#listen) 指令，Manticore 将在默认协议监听端口之后的 200 端口范围内为每个创建的集群使用前两个空闲端口。要手动设置复制端口，必须定义 [listen](../../Server_settings/Searchd.md#listen) 指令（`replication` 类型）的端口范围，且同一服务器上不同节点的地址/端口范围对不得相交。一般来说，端口范围应为每个集群指定至少两个端口。当定义带端口范围的复制监听器（例如 `listen = 192.168.0.1:9320-9328:replication`）时，Manticore 不会立即开始监听这些端口，而是在开始使用复制时从指定范围中随机选择空闲端口。

## 复制集群

复制集群是一组节点，写事务会在其中复制。复制按表设置，意味着一个表只能属于一个集群。集群中表的数量没有限制。属于集群的任何 percolate 或实时表上的所有事务，如 `INSERT`、`REPLACE`、`DELETE`、`TRUNCATE`，都会复制到该集群的所有其他节点。[分布式](../../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md#Creating-a-distributed-table) 表也可以参与复制过程。复制是多主的，因此对任意节点或多个节点同时写入都能正常工作。

要创建集群，通常可以使用命令 [create cluster](../../Creating_a_cluster/Setting_up_replication/Creating_a_replication_cluster.md#Creating-a-replication-cluster) 通过 `CREATE CLUSTER <cluster name>`，要加入集群，可以使用 [join cluster](../../Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md#Joining-a-replication-cluster) 通过 `JOIN CLUSTER <cluster name> at 'host:port'`。不过，在某些罕见情况下，您可能想微调 `CREATE/JOIN CLUSTER` 的行为。可用选项如下：

### name

此选项指定集群名称。它应在系统中所有集群中唯一。

> **注意：** `JOIN` 命令允许的最大主机名长度为 **253** 个字符。超过此限制，searchd 会报错。

### path

path 选项指定用于[写集缓存复制](https://galeracluster.com/library/documentation/state-transfer.html#state-transfer-gcache)和来自其他节点的传入表的数据目录。此值应在系统中所有集群中唯一，并应作为相对于 [data_dir](../../Server_settings/Searchd.md#data_dir) 目录的相对路径指定。默认设置为 [data_dir](../../Server_settings/Searchd.md#data_dir) 的值。

### nodes

`nodes` 选项是集群中所有节点的地址:端口对列表，使用逗号分隔。此列表应通过节点的 API 接口获取，也可以包含当前节点的地址。它用于将节点加入集群以及重启后重新加入。

### options

`options` 选项允许您直接向 Galera 复制插件传递额外选项，详见 [Galera 文档参数](https://galeracluster.com/library/documentation/galera-parameters.html)

## 写语句

<!-- example write statements 1 -->
在使用复制集群时，所有修改集群表内容的写语句，如 `INSERT`、`REPLACE`、`DELETE`、`TRUNCATE`、`UPDATE`，必须使用 `cluster_name:table_name` 表达式替代表名。这确保更改传播到集群中的所有副本。如果未使用正确表达式，将触发错误。

在 JSON 接口中，所有写入集群表的语句必须同时设置 `cluster` 属性和 `table` 名称。未设置 `cluster` 属性将导致错误。

只要正确配置了 [server_id](../../Server_settings/Searchd.md#server_id)，集群中表的 [Auto ID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) 应该是有效的。

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

## 读取语句

<!-- example write statements 2 -->
读取语句如 `SELECT`、`CALL PQ`、`DESCRIBE` 可以使用未加集群名前缀的常规表名，也可以使用 `cluster_name:table_name` 格式。如果使用后者，`cluster_name` 部分会被忽略。

使用 HTTP 端点 `json/search` 时，可以选择指定 `cluster` 属性，也可以省略。


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

## 集群参数

<!-- example cluster parameters 1 -->
可以使用 `SET` 语句调整复制插件选项。

可用选项列表见 [Galera Documentation Parameters](https://galeracluster.com/library/documentation/galera-parameters.html) 。


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

## 节点分歧的集群

<!-- example cluster with diverged nodes  1 -->
复制节点可能会出现分歧，导致所有节点都被标记为 `non-primary`。这可能是由于节点间网络分割、集群崩溃，或复制插件在确定 `primary component` 时发生异常。此时需要选择一个节点并将其提升为 `primary component`。

要确定需要提升的节点，应比较所有节点上的 `last_committed` 集群状态变量值。如果所有服务器都在运行，则无需重启集群。只需使用 `SET` 语句将具有最高 last_committed 值的节点提升为 `primary component`（如示例所示）。

其他节点随后将重新连接到主组件，并基于该节点重新同步数据。


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

## 复制与集群

<!-- example replication and cluster 1 -->
要使用复制，需要在配置文件中定义一个用于 SphinxAPI 协议的 [listen](../../Server_settings/Searchd.md#listen) 端口，以及一个用于复制地址和端口范围的 [listen](../../Server_settings/Searchd.md#listen) 。还需指定接收传入表的 [data_dir](../../Server_settings/Searchd.md#data_dir) 文件夹。


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
要复制表，必须在拥有本地表的服务器上创建集群。

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
将这些本地表添加到集群中


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
所有希望接收集群表副本的其他节点应按如下方式加入集群：


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
运行查询时，在表名前加上集群名 `posts`：或在 HTTP 请求对象中使用 `cluster` 属性。


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

All queries that modify tables in the cluster are now replicated to all nodes in the cluster.
<!-- proofread -->

