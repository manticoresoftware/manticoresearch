# Setting up replication

Write transaction (any result of `INSERT`, `REPLACE`, `DELETE`, `TRUNCATE`, `UPDATE`, `COMMIT`) can be replicated to other cluster nodes before the transaction is fully applied on the current node. Currently replication is supported for `percolate` and `rt` tables in Linux an MacOS. Manticore Search packages for Windows do not provide replication support.

Manticore's replication is based on [Galera library](https://github.com/codership/galera) and features the following:

* true multi-master - read and write to any node at any time
* [virtually synchronous replication](https://galeracluster.com/library/documentation/overview.html) - no slave lag, no data is lost after a node crash
* hot standby - no downtime during failover (since there is no failover)
* tightly coupled - all the nodes hold the same state. No diverged data between nodes allowed
* automatic node provisioning - no need to manually back up the database and restore it on a new node
* easy to use and deploy
* detection and automatic eviction of unreliable nodes
* certification based replication

To use replication in Manticore Search:

* [data_dir](../../Server_settings/Searchd.md#data_dir) option should be set in section "searchd" of the configuration file. Replication is not supported in the plain mode
* there should be either:
  - a [listen](../../Server_settings/Searchd.md#listen) directive specified (without specifying a protocol) containing an IP address accessible by other nodes
  - or [node_address](../../Server_settings/Searchd.md#node_address) with an accessible IP address
* optionally you can set unique values for [server_id](../../Server_settings/Searchd.md#server_id) on each cluster node. If no value is set, the node will try to use the MAC address (or a random number if that fails) to generate the `server_id`.

If there is no `replication` [listen](../../Server_settings/Searchd.md#listen) directive set Manticore will use the first two free ports in the range of 200 ports after the default protocol listening port for each created cluster. To set replication ports manually the [listen](../../Server_settings/Searchd.md#listen) directive (of `replication` type) port range should be defined and the address/port range pairs should not intersect between different nodes on the same server. As a rule of thumb, the port range should specify no less than two ports per cluster.

## Replication cluster

Replication cluster is a set of nodes among which a write transaction gets replicated. Replication is configured on per-table basis, meaning that one table can be assigned to only one cluster. There is no restriction on how many tables a cluster can have. All transactions such as `INSERT`, `REPLACE`, `DELETE`, `TRUNCATE` in any percolate or real-time table belonging to a cluster are replicated to all the other nodes in the cluster. Replication is multi-master, so writes to any particular node or to multiple nodes simultaneously work equally well.

In most cases you [create cluster](../../Creating_a_cluster/Setting_up_replication/Creating_a_replication_cluster.md#Creating-a-replication-cluster) with `CREATE CLUSTER <cluster name>` and [join cluster](../../Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md#Joining-a-replication-cluster) with `JOIN CLUSTER <cluster name> at 'host:port'`, but in rare cases you may want to fine-tune the behaviour of `CREATE/JOIN CLUSTER`. The options are:

### name

Specifies cluster name. Should be unique.

### path

Data directory for a [write-set cache replication](https://galeracluster.com/library/documentation/state-transfer.html#state-transfer-gcache) and incoming tables from other nodes. Should be unique among the other clusters in the node. Default is [data_dir](../../Server_settings/Searchd.md#data_dir). Should be specified in the form of a path to an existing directory relative to the [data_dir](../../Server_settings/Searchd.md#data_dir).

### nodes

List of address:port pairs for all the nodes in the cluster (comma separated). Node's API interface should be used for this option. It can contain the current node's address too. This list is used to join a node to the cluster and rejoin it after restart.

### options

Other options that are passed over directly to Galera replication plugin as described here [Galera Documentation Parameters](https://galeracluster.com/library/documentation/galera-parameters.html)

## Write statements

<!-- example write statements 1 -->
For SQL interface all write statements such as `INSERT`, `REPLACE`, `DELETE`, `TRUNCATE`, `UPDATE` that change the content of a cluster's table should use `cluster_name:index_name` expression in place of a table name to make sure the change is propagated to all replicas in the cluster. An error will be triggered otherwise.

All write statements for HTTP interface to a cluster's table should set `cluster` property along with `table` name. An error will be triggered otherwise.

[Auto ID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) generated for a table in a cluster should be valid as soon as [server_id](../../Server_settings/Searchd.md#server_id) is not misconfigured.

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
<!-- end -->

## Read statements

<!-- example write statements 2 -->
Read statements such as `SELECT`, `CALL PQ`, `DESCRIBE` can use either regular table names not prepended with a cluster name or `cluster_name:index_name`. In this case `cluster_name` is just ignored.

In HTTP endpoint `json/search` you can specify `cluster` property if you like, but can also omit it.


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

## Cluster parameters

<!-- example cluster parameters 1 -->
Replication plugin options can be changed using `SET` statement (see the example).

See [Galera Documentation Parameters](https://galeracluster.com/library/documentation/galera-parameters.html) for a list of available options.


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

## Cluster with diverged nodes

<!-- example cluster with diverged nodes  1 -->
Sometimes replicated nodes can diverge from each other. The state of all the nodes might turn into `non-primary` due to a network split between nodes, a cluster crash, or if the replication plugin hits an exception when determining the `primary component`. Then it's necessary to select a node and promote it to the `primary component`.

To determine which node needs to be a reference, compare the `last_committed` cluster status variable value on all nodes. If all the servers are already running there's no need to start the cluster again. You just need to promote the most advanced node to the `primary component` with `SET` statement (see the example).

All other nodes will reconnect to the node and resync their data based on this node.


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

## Replication and cluster

<!-- example replication and cluster 1 -->
To use replication define one [listen](../../Server_settings/Searchd.md#listen) port for SphinxAPI protocol and one [listen](../../Server_settings/Searchd.md#listen) for replication address and port range in the config. Define [data_dir](../../Server_settings/Searchd.md#data_dir) folder for incoming tables.


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
Create a cluster at the server that has local tables that need to be replicated


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
<!-- end -->

<!-- example replication and cluster 3 -->
Add these local tables to cluster


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
<!-- end -->

<!-- example replication and cluster 4 -->
All other nodes that want replica of cluster's tables should join cluster as


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
<!-- end -->

<!-- example replication and cluster 5 -->
When running queries for SQL prepend the table name with the cluster name `posts:` or use `cluster` property for HTTP request object.


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
<!-- end -->

Now all such queries that modify tables in the cluster are replicated to all nodes in the cluster.
