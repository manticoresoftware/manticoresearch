# Setting up replication

Manticore can replicate a write transaction (`INSERT`, `REPLACE`, `DELETE`, `TRUNCATE`, `UPDATE`, etc) in an index to other nodes in the cluster. Currently percolate and rt indexes are supported. Only Linux packages and builds support replication, Windows and MacOS packages do not support replication.

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
  -  or [node_address](../../Server_settings/Searchd.md#node_address) with an accessible IP address
* optionally set unique values for [server_id](../../Server_settings/Searchd.md#server_id) on each cluster node. If no value set, the node will try to use the MAC address (or a random number if that fails) to generate the server_id.

If there is no replication [listen](../../Server_settings/Searchd.md#listen) directive set Manticore will use first couple of free ports in a range of 200 ports after the port at which the daemon is listening (default protocol) for each cluster created. For manual declaration of replication ports the [listen](../../Server_settings/Searchd.md#listen) directive
port range should be defined and these "address - port range" pairs should be different for all Manticore instances on the same host. As a rule of thumb, the port range should specify no less than two ports per cluster.

## Replication cluster

Replication cluster is a set of nodes among which a write transaction gets replicated. Replication is configured on the per-index basis. One index can be assigned to only one cluster. There is no restriction on how many indexes a cluster may have. All transactions such as `INSERT`, `REPLACE`, `DELETE`, `TRUNCATE` in any percolate index belonging to a cluster are replicated to all the other nodes in the cluster. Replication is multi-master, so writes to any particular node or to multiple nodes simultaneously work equally well.

Replication cluster configuration options are:

### name

Specifies a name for the cluster. Should be unique.

### path

Data directory for a [write-set cache replication](https://galeracluster.com/library/documentation/state-transfer.html#state-transfer-gcache) and incoming indexes from other nodes. Should be unique among the other clusters in the node. Default is [data_dir](../../Server_settings/Searchd.md#data_dir).

### nodes

A list of address:port pairs for all the nodes in the cluster (comma separated). A node's API interface should be used for this option. It  can contain the current node's address too. This list is used to join a node to the cluster and rejoin it after restart.

### options

Options passed directly to Galera replication plugin as described here [Galera Documentation Parameters](https://galeracluster.com/library/documentation/galera-parameters.html)

## Write statements

<!-- example write statements 1 -->
For SQL interface all write statements such as `INSERT`, `REPLACE`, `DELETE`, `TRUNCATE`, `UPDATE` that change the content of a cluster's index should use `cluster_name:index_name` expression in place of an index name to make sure the change is propagated to all replicas in the cluster. An error will be triggered otherwise.

All write statements for HTTP interface to a cluster's index should set `cluster` property along with `index` name. An error will be triggered otherwise.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
INSERT INTO posts:weekly_index VALUES ( 'iphone case' )
TRUNCATE RTINDEX click_query:weekly_index
UPDATE INTO posts:rt_tags SET tags=(101, 302, 304) WHERE MATCH ('use') AND id IN (1,101,201)
DELETE FROM clicks:rt WHERE MATCH ('dumy') AND gid>206
```

<!-- request HTTP -->

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

<!-- example write statements 2 -->
Read statements such as `CALL PQ`, `SELECT` or `DESCRIBE` can use either regular index names not prepended with a cluster name or `cluster_name:index_name`. `cluster_name:index_name` syntax ignores the cluster name and may be used on an index that doesn't belong to the cluster.

HTTP endpoint `json/search` could use either a `cluster` property or not.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT * FROM weekly_index
CALL PQ('posts:weekly_index', 'document is here')
```

<!-- request HTTP -->

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

ID auto generation uses UUID_SHORT similar to MySQL function. It is valid cluster wide UUID when [server_id](../../Server_settings/Searchd.md#server_id) properly configured.

> **Note:** `UpdateAttributes` statement from API interface to specific index always set proper cluster at server and there is no way to know is update to index got propagated into cluster properly or node diverged and statement updated only local index.

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
<!-- request HTTP -->

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
<!-- request HTTP -->

```json
POST /cli -d "
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1
"
```
<!-- end -->

## Replication and cluster

<!-- example replication and cluster 1 -->
To use replication define one [listen](../../Server_settings/Searchd.md#listen) port for SphinxAPI protocol and one [listen](../../Server_settings/Searchd.md#listen) for replication address and port range in the config. Define [data_dir](../../Server_settings/Searchd.md#data_dir) folder for incoming indexes.


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
Create a cluster at the server that has local indexes that need to be replicated


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE CLUSTER posts
```

<!-- request HTTP -->

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
utilsApi.sql('mode=raw&query=CREATE CLUSTER posts')
```
<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await utilsApi.sql('mode=raw&query=CREATE CLUSTER posts');
```

<!-- intro -->
##### Java:

<!-- request Java -->

```java
utilsApi.sql("mode=raw&query=CREATE CLUSTER posts");

```
<!-- end -->

<!-- example replication and cluster 3 -->
Add these local indexes to cluster


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
ALTER CLUSTER posts ADD pq_title
ALTER CLUSTER posts ADD pq_clicks
```

<!-- request HTTP -->

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
utilsApi.sql('mode=raw&query=ALTER CLUSTER posts ADD pq_title')
utilsApi.sql('mode=raw&query=ALTER CLUSTER posts ADD pq_clicks')
```
<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await utilsApi.sql('mode=raw&query=ALTER CLUSTER posts ADD pq_title');
res = await utilsApi.sql('mode=raw&query=ALTER CLUSTER posts ADD pq_clicks');
```

<!-- intro -->
##### Java:

<!-- request Java -->

```java
utilsApi.sql("mode=raw&query=ALTER CLUSTER posts ADD pq_title");
utilsApi.sql("mode=raw&query=ALTER CLUSTER posts ADD pq_clicks");
```
<!-- end -->

<!-- example replication and cluster 4 -->
All other nodes that want replica of cluster's indexes should join cluster as


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
JOIN CLUSTER posts AT '192.168.1.101:9312'
```

<!-- request HTTP -->

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
utilsApi.sql('mode=raw&query=JOIN CLUSTER posts AT \'192.168.1.101:9312\'')
```
<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await utilsApi.sql('mode=raw&query=JOIN CLUSTER posts AT \'192.168.1.101:9312\'');
```

<!-- intro -->
##### Java:

<!-- request Java -->

```java
utilsApi.sql("mode=raw&query=JOIN CLUSTER posts AT '192.168.1.101:9312'");

```
<!-- end -->

<!-- example replication and cluster 5 -->
When running queries for SQL prepend the index name with the cluster name `posts:` or use `cluster` property for HTTP request object.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
INSERT INTO posts:pq_title VALUES ( 3, 'test me' )
```

<!-- request HTTP -->

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

Now all such queries that modify indexes in the cluster are replicated to all nodes in the cluster.
