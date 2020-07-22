# Adding and removing an index from a replication cluster 

<!-- example adding and removing an index from a replication cluster 1 -->
`ALTER CLUSTER <cluster_name> ADD <index_name>` adds an existing local index to the cluster. The node which receives the ALTER query sends the index to the other nodes in the cluster. All the local indexes with the same name on the other nodes of the cluster get replaced with the new index.


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
<!-- end -->

<!-- example adding and removing an index from a replication cluster 2 -->
`ALTER CLUSTER <cluster_name> DROP <index_name>` forgets about a local index, i.e., it doesn't remove the index files on the nodes but just makes it an active non-replicated index.


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

<!-- end -->
