# Deleting a replication cluster 

<!-- example deleting a replication cluster  1 -->
Delete statement removes a cluster specified with [name](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#name). The cluster gets removed from all the nodes, but its indexes are left intact and become active local non-replicated indexes.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
DELETE CLUSTER click_query
```

<!-- request HTTP -->

```json
POST /sql -d "mode=raw&query=
DELETE CLUSTER click_query
"
```

<!-- request PHP -->

```php
$params = [
    'cluster' => 'click_query',
    'body' => []
];
$response = $client->cluster()->delete($params);                
```

<!-- end -->
