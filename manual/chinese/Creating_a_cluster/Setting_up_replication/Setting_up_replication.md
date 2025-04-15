# 设置复制

使用 Manticore，写事务（例如 `INSERT`、`REPLACE`、`DELETE`、`TRUNCATE`、`UPDATE`、`COMMIT`）可以在当前节点完全应用之前复制到其他集群节点。目前，在 Linux 和 macOS 上，`percolate`、`rt` 和 `distributed` 表支持复制。

Manticore 的原生 Windows 二进制文件不支持复制。我们建议[通过 WSL 安装 Manticore](../../Installation/Windows.md#Installing-or-enabling-WSL2)（Windows 子系统 for Linux）。

在 [macOS](../../Installation/MacOS.md) 上，复制支持有限，只建议用于开发目的。

Manticore 的复制是由 [Galera 库](https://github.com/codership/galera) 提供支持，并且具有几个令人印象深刻的特性：

* 真正的多主：可以随时对任何节点进行读写。

* [几乎同步的复制](https://galeracluster.com/library/documentation/overview.html)，没有从属延迟，在节点崩溃后没有数据丢失。

* 热备份：故障切换期间没有停机时间（因为没有故障切换）。

* 紧密耦合：所有节点保持相同的状态，并且不允许节点之间存在分歧数据。

* 自动节点配置：无需手动备份数据库并在新节点上恢复。

* 易于使用和部署。

* 检测和自动驱逐不可靠节点。

* 基于认证的复制。

要在 Manticore Search 中设置复制：

* 必须在配置文件的 "searchd" 部分中设置 [data_dir](../../Server_settings/Searchd.md#data_dir) 选项。纯模式不支持复制。

* 必须指定一个 [listen](../../Server_settings/Searchd.md#listen) 指令，包含其他节点可访问的 IP 地址，或者一个具有可访问 IP 地址的 [node_address](../../Server_settings/Searchd.md#node_address)。

* 可选地，您可以在每个集群节点上设置唯一的 [server_id](../../Server_settings/Searchd.md#server_id) 值。如果未设置值，节点将尝试使用 MAC 地址或随机数生成 `server_id`。

如果没有设置 `replication` [listen](../../Server_settings/Searchd.md#listen) 指令，Manticore 将使用每个创建集群的默认协议监听端口后面的 200 个端口范围中的前两个空闲端口。要手动设置复制端口，必须定义 [listen](../../Server_settings/Searchd.md#listen) 指令（类型为 `replication`）的端口范围，并且相同服务器上不同节点之间的地址/端口范围对不能相交。作为经验法则，端口范围应该为每个集群指定至少两个端口。当您定义具有端口范围的复制监听器（例如 `listen = 192.168.0.1:9320-9328:replication`）时，Manticore 不会立即开始监听这些端口。相反，它将在您开始使用复制时从指定范围中随机选择空闲端口。

## 复制集群

复制集群是一组节点，其中写事务被复制。复制是按表设置的，这意味着一个表只能属于一个集群。集群中表的数量没有限制。对属于集群的任何穿透或实时表的 `INSERT`、`REPLACE`、`DELETE`、`TRUNCATE` 等事务都将复制到该集群中的所有其他节点。[分布式](../../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md#Creating-a-distributed-table) 表也可以成为复制过程的一部分。复制是多主的，因此对任何节点或多个节点同时的写入都能正常工作。

要创建集群，您通常可以使用命令 [create cluster](../../Creating_a_cluster/Setting_up_replication/Creating_a_replication_cluster.md#Creating-a-replication-cluster) 和 `CREATE CLUSTER <集群名称>`，要加入集群，您可以使用 [join cluster](../../Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md#Joining-a-replication-cluster) 和 `JOIN CLUSTER <集群名称> at 'host:port'`。然而，在某些罕见情况下，您可能希望微调 `CREATE/JOIN CLUSTER` 的行为。可用选项包括：

### name

此选项指定集群的名称。它在系统中的所有集群中应该是唯一的。

> **注意：** `JOIN` 命令允许的最大主机名长度为 **253** 个字符。如果超出此限制，searchd 将生成错误。

### path

path 选项指定用于 [写集缓存复制](https://galeracluster.com/library/documentation/state-transfer.html#state-transfer-gcache) 和来自其他节点的传入表的数据目录。此值在系统中的所有集群中应是唯一的，并且应作为相对路径指定到 [data_dir](../../Server_settings/Searchd.md#data_dir) 目录。默认情况下，它设置为 [data_dir](../../Server_settings/Searchd.md#data_dir) 的值。

### nodes

`nodes` 选项是一个地址:端口对的列表，包含集群中所有节点的地址，使用逗号分隔。此列表应使用节点的 API 接口获取，并且可以包含当前节点的地址。用于将节点加入集群并在重启后重新加入集群。

### options

`options` 选项允许您直接将附加选项传递给 Galera 复制插件，如 [Galera 文档参数](https://galeracluster.com/library/documentation/galera-parameters.html) 中所描述。

## 写语句

<!-- example write statements 1 -->
当使用复制集群时，所有的写语句如`INSERT`、`REPLACE`、`DELETE`、`TRUNCATE`、`UPDATE`等修改集群表内容的语句必须使用`cluster_name:table_name`表达式，而不是表名。这确保了更改会传播到集群中的所有副本。如果没有使用正确的表达式，将会触发错误。

在JSON接口中，`cluster`属性必须与所有写入集群表的`table`名称一起设置。如果没有设置`cluster`属性，将导致错误。

集群中表的[自动ID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID)应当是有效的，只要[server_id](../../Server_settings/Searchd.md#server_id)正确配置。

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

## 查询语句

<!-- example write statements 2 -->
查询语句如`SELECT`、`CALL PQ`、`DESCRIBE`可以使用常规的表名而不加集群名称，或者使用`cluster_name:table_name`格式。如果使用后者，则忽略`cluster_name`部分。

使用HTTP端点 `json/search` 时，`cluster` 属性可以根据需要指定，但也可以省略。


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
可以使用`SET`语句调整复制插件选项。

可用选项的列表可以在[Galera 文档参数](https://galeracluster.com/library/documentation/galera-parameters.html) 中找到。


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

## 与分歧节点的集群

<!-- example cluster with diverged nodes  1 -->
复制节点可能会彼此分歧，导致所有节点被标记为`non-primary`的状态。此情况可能由于节点之间的网络分裂、集群崩溃，或者当复制插件在确定`primary component`时出现异常而导致。在这种情况下，必须选择一个节点并将其提升为`primary component`的角色。

要识别需要提升的节点，应比较所有节点上的`last_committed`集群状态变量值。如果所有服务器当前都在运行，则无需重新启动集群。相反，只需使用`SET`语句（如示例中所示）将具有最高last_committed值的节点提升为`primary component`。

其他节点随后将重新连接到primary component并基于该节点重新同步它们的数据。


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

## 复制和集群

<!-- 示例复制和集群 1 -->
要使用复制，您需要在配置文件中为 SphinxAPI 协议定义一个 [listen](../../Server_settings/Searchd.md#listen) 端口和一个 [listen](../../Server_settings/Searchd.md#listen) 端口以用于复制地址和端口范围。同时，指定 [data_dir](../../Server_settings/Searchd.md#data_dir) 文件夹以接收传入的表。


<!-- 介绍 -->
##### ini:

<!-- 请求 ini -->
```ini
searchd {
  listen   = 9312
  listen   = 192.168.1.101:9360-9370:replication
  data_dir = /var/lib/manticore/
  ...
 }
```
<!-- end -->

<!-- 示例复制和集群 2 -->
要复制表，您必须在拥有要复制的本地表的服务器上创建一个集群。

<!-- 介绍 -->
##### SQL:

<!-- 请求 SQL -->

```sql
CREATE CLUSTER posts
```

<!-- 请求 JSON -->

```json
POST /cli -d "
CREATE CLUSTER posts
"
```

<!-- 请求 PHP -->

```php
$params = [
    'cluster' => 'posts'
    ]
];
$response = $client->cluster()->create($params);
```
<!-- 介绍 -->
##### Python:

<!-- 请求 Python -->

```python
utilsApi.sql('CREATE CLUSTER posts')
```
<!-- 介绍 -->
##### Javascript:

<!-- 请求 Javascript -->

```javascript
res = await utilsApi.sql('CREATE CLUSTER posts');
```

<!-- 介绍 -->
##### Java:

<!-- 请求 Java -->

```java
utilsApi.sql("CREATE CLUSTER posts");

```

<!-- 介绍 -->
##### C#:

<!-- 请求 C# -->

```clike
utilsApi.Sql("CREATE CLUSTER posts");

```
<!-- end -->

<!-- 示例复制和集群 3 -->
将这些本地表添加到集群


<!-- 介绍 -->
##### SQL:

<!-- 请求 SQL -->

```sql
ALTER CLUSTER posts ADD pq_title
ALTER CLUSTER posts ADD pq_clicks
```

<!-- 请求 JSON -->

```json
POST /cli -d "
ALTER CLUSTER posts ADD pq_title
"
POST /cli -d "
ALTER CLUSTER posts ADD pq_clicks
"
```

<!-- 请求 PHP -->

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
<!-- 介绍 -->
##### Python:

<!-- 请求 Python -->

```python
utilsApi.sql('ALTER CLUSTER posts ADD pq_title')
utilsApi.sql('ALTER CLUSTER posts ADD pq_clicks')
```
<!-- 介绍 -->
##### Javascript:

<!-- 请求 Javascript -->

```javascript
res = await utilsApi.sql('ALTER CLUSTER posts ADD pq_title');
res = await utilsApi.sql('ALTER CLUSTER posts ADD pq_clicks');
```

<!-- 介绍 -->
##### Java:

<!-- 请求 Java -->

```java
utilsApi.sql("ALTER CLUSTER posts ADD pq_title");
utilsApi.sql("ALTER CLUSTER posts ADD pq_clicks");
```

<!-- 介绍 -->
##### C#:

<!-- 请求 C# -->

```clike
utilsApi.Sql("ALTER CLUSTER posts ADD pq_title");
utilsApi.Sql("ALTER CLUSTER posts ADD pq_clicks");
```

<!-- end -->

<!-- 示例复制和集群 4 -->
所有希望接收集群表副本的其他节点应如下加入集群：


<!-- 介绍 -->
##### SQL:

<!-- 请求 SQL -->

```sql
JOIN CLUSTER posts AT '192.168.1.101:9312'
```

<!-- 请求 JSON -->

```json
POST /cli -d "
JOIN CLUSTER posts AT '192.168.1.101:9312'
"
```

<!-- 请求 PHP -->

```php
$params = [
  'cluster' => 'posts',
  'body' => [
      '192.168.1.101:9312'
  ]
];
$response = $client->cluster->join($params);
```
<!-- 介绍 -->
##### Python:

<!-- 请求 Python -->

```python
utilsApi.sql('JOIN CLUSTER posts AT \'192.168.1.101:9312\'')
```
<!-- 介绍 -->
##### Javascript:

<!-- 请求 Javascript -->

```javascript
res = await utilsApi.sql('JOIN CLUSTER posts AT \'192.168.1.101:9312\'');
```

<!-- 介绍 -->
##### Java:

<!-- 请求 Java -->

```java
utilsApi.sql("JOIN CLUSTER posts AT '192.168.1.101:9312'");

```

<!-- 介绍 -->
##### C#:

<!-- 请求 C# -->

```clike
utilsApi.Sql("JOIN CLUSTER posts AT '192.168.1.101:9312'");

```
<!-- end -->

<!-- 示例复制和集群 5 -->
在运行查询时，在表名之前加上集群名称 `posts`：或使用 HTTP 请求对象的 `cluster` 属性。


<!-- 介绍 -->
##### SQL:

<!-- 请求 SQL -->

```sql
INSERT INTO posts:pq_title VALUES ( 3, 'test me' )
```

<!-- 请求 JSON -->

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

<!-- 请求 PHP -->

```php
$index->addDocuments([
        3, ['title' => 'test me']
]);

```
<!-- 介绍 -->
##### Python:

<!-- 请求 Python -->

``` python
indexApi.insert({"cluster":"posts","table":"pq_title","id":3"doc":{"title":"test me"}})

```
<!-- 介绍 -->
##### Javascript:

<!-- 请求 Javascript -->

``` javascript
res = await indexApi.insert({"cluster":"posts","table":"pq_title","id":3"doc":{"title":"test me"}});
```

<!-- 介绍 -->
##### java:

<!-- 请求 Java -->

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
