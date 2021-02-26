# Adding and removing an index from a replication cluster 

<!-- example adding and removing an index from a replication cluster 1 -->
`ALTER CLUSTER <cluster_name> ADD <index_name>` adds an existing local index to the cluster. The node which receives the ALTER query sends the index to the other nodes in the cluster. All the local indexes with the same name on the other nodes of the cluster get replaced with the new index.

After the index is replicated, write statements can be performed on any node but index name must be prefixed with the cluster name like `INSERT INTO <clusterName>:<indexName>`.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
ALTER CLUSTER click_query ADD clicks_daily_index
```

<!-- request HTTP -->

```json
POST /sql -d "mode=raw&query=
ALTER CLUSTER click_query ADD clicks_daily_index
"
```

<!-- request PHP -->

```php
$params = [
  'cluster' => 'click_query',
  'body' => [
     'operation' => 'add',
     'index' => 'clicks_daily_index'
      
  ]
];
$response = $client->cluster()->alter($params);        
```


<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('mode=raw&query=ALTER CLUSTER click_query ADD clicks_daily_index')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('mode=raw&query=ALTER CLUSTER click_query ADD clicks_daily_index');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("mode=raw&query=ALTER CLUSTER click_query ADD clicks_daily_index");
```

<!-- end -->

<!-- example adding and removing an index from a replication cluster 2 -->
`ALTER CLUSTER <cluster_name> DROP <index_name>` forgets about a local index, i.e., it doesn't remove the index files on the nodes but just makes it an active non-replicated index.

After an index is removed from a cluster, it becomes a 'local' index and write statements must use just the index name as `INSERT INTO <indexName>`, without the cluster prefix.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
ALTER CLUSTER posts DROP weekly_index
```

<!-- request HTTP -->

```json
POST /sql -d "mode=raw&query=
ALTER CLUSTER posts DROP weekly_index
"
```

<!-- request PHP -->

```php
$params = [
  'cluster' => 'posts',
  'body' => [
     'operation' => 'drop',
     'index' => 'weekly_index'
      
  ]
];
$response = $client->cluster->alter($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('mode=raw&query=ALTER CLUSTER posts DROP weekly_index')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('mode=raw&query=ALTER CLUSTER posts DROP weekly_index');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("mode=raw&query=ALTER CLUSTER posts DROP weekly_index");
```

<!-- end -->
