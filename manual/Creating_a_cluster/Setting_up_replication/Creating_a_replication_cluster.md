# Creating a replication cluster

<!-- example creating a replication cluster 1 -->
To create a replication cluster, you must set its [name](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#name) at a minimum.

If you are creating a single cluster or the first cluster, you may omit the [path](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#path) option. In this case, the [data_dir](../../Server_settings/Searchd.md#data_dir) option will be used as the cluster path. However, for all subsequent clusters, you must specify the [path](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#path) and the path must be available. The [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#nodes) option may also be set to list all nodes in the cluster.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE CLUSTER posts
CREATE CLUSTER click_query '/var/data/click_query/' as path
CREATE CLUSTER click_query '/var/data/click_query/' as path, 'clicks_mirror1:9312,clicks_mirror2:9312,clicks_mirror3:9312' as nodes
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE CLUSTER posts
"
POST /cli -d "
CREATE CLUSTER click_query '/var/data/click_query/' as path
"
POST /cli -d "
CREATE CLUSTER click_query '/var/data/click_query/' as path, 'clicks_mirror1:9312,clicks_mirror2:9312,clicks_mirror3:9312' as nodes
"
```

<!-- request PHP -->

```php
$params = [
    'cluster' => 'posts',
    ]
];
$response = $client->cluster()->create($params);
$params = [
    'cluster' => 'click_query',
    'body' => [
        'path' => '/var/data/click_query/'
    ]    
    ]
];
$response = $client->cluster()->create($params);
$params = [
    'cluster' => 'click_query',
    'body' => [
        'path' => '/var/data/click_query/',
        'nodes' => 'clicks_mirror1:9312,clicks_mirror2:9312,clicks_mirror3:9312'
    ]    
    ]
];
$response = $client->cluster()->create($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE CLUSTER posts')
utilsApi.sql('CREATE CLUSTER click_query \'/var/data/click_query/\' as path')
utilsApi.sql('CREATE CLUSTER click_query \'/var/data/click_query/\' as path, \'clicks_mirror1:9312,clicks_mirror2:9312,clicks_mirror3:9312\' as nodes')

```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE CLUSTER posts');
res = await utilsApi.sql('CREATE CLUSTER click_query \'/var/data/click_query/\' as path');
res = await utilsApi.sql('CREATE CLUSTER click_query \'/var/data/click_query/\' as path, \'clicks_mirror1:9312,clicks_mirror2:9312,clicks_mirror3:9312\' as nodes');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE CLUSTER posts");
utilsApi.sql("CREATE CLUSTER click_query '/var/data/click_query/' as path");
utilsApi.sql("CREATE CLUSTER click_query '/var/data/click_query/' as path, 'clicks_mirror1:9312,clicks_mirror2:9312,clicks_mirror3:9312' as nodes");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE CLUSTER posts");
utilsApi.Sql("CREATE CLUSTER click_query '/var/data/click_query/' as path");
utilsApi.Sql("CREATE CLUSTER click_query '/var/data/click_query/' as path, 'clicks_mirror1:9312,clicks_mirror2:9312,clicks_mirror3:9312' as nodes");
```
<!-- end -->

If the [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#nodes) option is not specified when creating a cluster, the first node that joins the cluster will be saved as the [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#nodes) option.

<!-- proofread -->