# 设置复制

在 Manticore 中，写事务（如 `INSERT`、`REPLACE`、`DELETE`、`TRUNCATE`、`UPDATE`、`COMMIT`）可以在当前节点完全应用事务之前，复制到集群中的其他节点。目前，复制支持 `percolate`、`rt` 和 `distributed` 表，并且仅支持 Linux 和 macOS 系统。Manticore Search 的 Windows 版本不提供复制支持。

Manticore 的复制基于 [Galera 库](https://github.com/codership/galera)，具有以下特点：

- 真正的多主架构：可以随时对任意节点进行读写操作。
- [几乎同步的复制](https://galeracluster.com/library/documentation/overview.html)：无从属节点延迟，无节点崩溃后的数据丢失。
- 热备：无故障切换期间的停机时间（因为没有故障切换）。
- 紧密耦合：所有节点保持相同状态，节点之间不允许数据分歧。
- 自动节点配置：无需手动备份数据库并在新节点上恢复。
- 易于使用和部署。
- 自动检测和移除不可靠节点。
- 基于认证的复制。

要在 Manticore Search 中设置复制：

- 必须在配置文件的 "searchd" 部分中设置 [data_dir](../../Server_settings/Searchd.md#data_dir) 选项。复制不支持 plain 模式。
- 必须指定包含其他节点可访问的 IP 地址的 [listen](../../Server_settings/Searchd.md#listen) 指令，或具有可访问 IP 地址的 [node_address](../../Server_settings/Searchd.md#node_address)。
- 可选地，您可以为每个集群节点设置唯一的 [server_id](../../Server_settings/Searchd.md#server_id)。如果未设置此值，节点将尝试使用 MAC 地址或随机数生成 `server_id`。

如果未设置 `replication` [listen](../../Server_settings/Searchd.md#listen) 指令，Manticore 将使用默认协议监听端口后 200 个端口中的前两个空闲端口为每个创建的集群分配复制端口。要手动设置复制端口，必须定义 [listen](../../Server_settings/Searchd.md#listen) 指令（`replication` 类型）的端口范围，并确保地址/端口范围对在同一服务器的不同节点之间不冲突。一般建议为每个集群至少指定两个端口。

## 复制集群

复制集群是一个写事务被复制到其他节点的节点组。复制在每个表的基础上设置，意味着一个表只能属于一个集群。一个集群可以拥有的表的数量没有限制。所有对属于集群的 `percolate` 或实时表的事务操作（如 `INSERT`、`REPLACE`、`DELETE`、`TRUNCATE`）都会复制到集群中的所有其他节点。[分布式](../../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md#Creating-a-distributed-table)表也可以作为复制过程的一部分。复制是多主架构，因此对任意节点或多个节点同时写入都能正常工作。

创建集群通常可以使用 `CREATE CLUSTER <cluster name>` 命令，加入集群可以使用 `JOIN CLUSTER <cluster name> at 'host:port'` 命令。然而，在某些罕见情况下，您可能需要微调 `CREATE/JOIN CLUSTER` 的行为。可用的选项如下：

### name

此选项指定集群的名称。在系统中的所有集群中应是唯一的。

> **注意**：`JOIN` 命令的最大主机名长度为 **253** 个字符。如果超出此限制，searchd 将生成错误。

### path

`path` 选项指定用于[写集缓存复制](https://galeracluster.com/library/documentation/state-transfer.html#state-transfer-gcache)和从其他节点接收的表的数据目录。该值应在系统中的所有集群中是唯一的，并应指定为相对于 [data_dir](../../Server_settings/Searchd.md#data_dir) 的路径。默认情况下，它设置为 [data_dir](../../Server_settings/Searchd.md#data_dir) 的值。

### nodes

`nodes` 选项是集群中所有节点的地址:端口对列表，地址之间用逗号分隔。此列表应通过节点的 API 接口获取，并可包括当前节点的地址。它用于将节点加入集群，并在重新启动后重新加入。

### options

`options` 选项允许您直接向 Galera 复制插件传递其他选项，如 [Galera Documentation Parameters](https://galeracluster.com/library/documentation/galera-parameters.html) 中所述。

## 写操作

<!-- example write statements 1 -->当使用复制集群时，所有修改集群表内容的写操作（如 `INSERT`、`REPLACE`、`DELETE`、`TRUNCATE`、`UPDATE`）都必须使用 `cluster_name:index_name` 表达式，而不是表名。这可以确保更改传播到集群中的所有副本。如果未使用正确的表达式，将触发错误。

在 JSON 接口中，必须为集群表的所有写操作设置 `cluster` 属性以及表名。如果未设置 `cluster` 属性，将导致错误。

对于集群中的表，[自动 ID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) 应保持有效，只要 [server_id](../../Server_settings/Searchd.md#server_id) 配置正确。

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
  "index":"weekly_index",
  "doc":
  {
    "title" : "iphone case",
    "price" : 19.85
  }
}'
POST /delete -d '
{
  "cluster":"posts",
  "index": "weekly_index",
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
indexApi.insert({"cluster":"posts","index":"weekly_index","doc":{"title":"iphone case","price":19.85}})
indexApi.delete({"cluster":"posts","index":"weekly_index","id":1})
```
<!-- intro -->
##### Javascript:

<!-- request Javascript -->

``` javascript
res = await indexApi.insert({"cluster":"posts","index":"weekly_index","doc":{"title":"iphone case","price":19.85}});
 res = await indexApi.delete({"cluster":"posts","index":"weekly_index","id":1});
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

## 读操作

<!-- example write statements 2 -->

读操作，如 `SELECT`、`CALL PQ`、`DESCRIBE`，可以使用未加前缀的常规表名，或者可以使用 `cluster_name:index_name` 格式。如果使用后者，`cluster_name` 部分将被忽略。

在使用 HTTP 端点 `json/search` 时，可以根据需要指定 `cluster` 属性，但也可以省略此属性。


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
  "index":"weekly_index",
  "query":{"match":{"title":"keyword"}}
}'
POST /search -d '
{
  "index":"weekly_index",
  "query":{"match":{"title":"keyword"}}
}'
```

<!-- end -->

## 集群参数

<!-- example cluster parameters 1 -->

可以使用 `SET` 语句调整复制插件选项。

可用选项的列表可以在 [Galera Documentation Parameters](https://galeracluster.com/library/documentation/galera-parameters.html) 中找到。


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
复制的节点可能会彼此分歧，导致所有节点标记为 `non-primary`。这种情况可能是由于节点之间的网络分裂、集群崩溃，或复制插件在确定 `primary component` 时遇到异常所导致。在这种情况下，需要选择一个节点并将其提升为 `primary component`。

要确定需要提升的节点，应比较所有节点的 `last_committed` 集群状态变量值。如果所有服务器当前都在运行，则无需重启集群。相反，您可以使用 `SET` 语句（如示例所示）将具有最高 `last_committed` 值的节点提升为 `primary component`。

其他节点将重新连接到主组件，并根据该节点重新同步数据。


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
要使用复制，您需要在配置文件中为 SphinxAPI 协议定义一个 [listen](../../Server_settings/Searchd.md#listen) 端口，以及一个用于复制的地址和端口范围的 [listen](../../Server_settings/Searchd.md#listen) 。同时，指定 [data_dir](../../Server_settings/Searchd.md#data_dir) 文件夹以接收传入的表。


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
要复制表，您必须在包含要复制的本地表的服务器上创建一个集群。

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
<!-- end -->

<!-- example replication and cluster 3 -->
将这些本地表添加到集群中。


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
     'index' => 'pq_title'

  ]
];
$response = $client->cluster()->alter($params);
$params = [
  'cluster' => 'posts',
  'body' => [
     'operation' => 'add',
     'index' => 'pq_clicks'

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

<!-- end -->

<!-- example replication and cluster 4 -->
所有其他希望接收集群表副本的节点应按如下方式加入集群：


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
<!-- end -->

<!-- example replication and cluster 5 -->
在运行查询时，在表名之前加上集群名称 `posts`，或者在 HTTP 请求对象中使用 `cluster` 属性。


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
  "index":"pq_title",
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
indexApi.insert({"cluster":"posts","index":"pq_title","id":3"doc":{"title":"test me"}})

```
<!-- intro -->
##### Javascript:

<!-- request Javascript -->

``` javascript
res = await indexApi.insert({"cluster":"posts","index":"pq_title","id":3"doc":{"title":"test me"}});
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
<!-- end -->

现在，所有修改集群中表的查询都会复制到集群中的所有节点。
<!-- proofread -->
