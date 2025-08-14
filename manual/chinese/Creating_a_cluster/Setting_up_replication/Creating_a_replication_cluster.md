# 创建复制集群

<!-- example creating a replication cluster 1 -->
要创建复制集群，至少必须设置其[名称](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#name)。

如果您正在创建单个集群或第一个集群，可以省略[path](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#path)选项。在这种情况下，将使用[data_dir](../../Server_settings/Searchd.md#data_dir)选项作为集群路径。但是，对于所有后续集群，必须指定[path](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#path)，且路径必须可用。也可以设置[nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#nodes)选项来列出集群中的所有节点。


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
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE CLUSTER posts')
await utilsApi.sql('CREATE CLUSTER click_query \'/var/data/click_query/\' as path')
await utilsApi.sql('CREATE CLUSTER click_query \'/var/data/click_query/\' as path, \'clicks_mirror1:9312,clicks_mirror2:9312,clicks_mirror3:9312\' as nodes')

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

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE CLUSTER posts", Some(true)).await;
utils_api.sql("CREATE CLUSTER click_query '/var/data/click_query/' as path", Some(true)).await;
utils_api.sql("CREATE CLUSTER click_query '/var/data/click_query/' as path, 'clicks_mirror1:9312,clicks_mirror2:9312,clicks_mirror3:9312' as nodes", Some(true)).await;
```

<!-- end -->

如果在创建集群时未指定[nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#nodes)选项，第一个加入集群的节点将被保存为[nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#nodes)选项。

<!-- proofread -->

