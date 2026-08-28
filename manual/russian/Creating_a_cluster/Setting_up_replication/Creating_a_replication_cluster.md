# Создание репликационного кластера

<!-- example creating a replication cluster 1 -->
Чтобы создать репликационный кластер, необходимо как минимум задать его [имя](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#name).

Если вы создаете один кластер или первый кластер, вы можете опустить опцию [path](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#path). В этом случае в качестве пути кластера будет использоваться опция [data_dir](../../Server_settings/Searchd.md#data_dir). Однако для всех последующих кластеров вы должны указать [path](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#path), и путь должен быть доступен. Также может быть установлена опция [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#nodes) для перечисления всех узлов в кластере.


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

Если опция [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#nodes) не указана при создании кластера, первый узел, присоединившийся к кластеру, будет сохранен как опция [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#nodes).

<!-- proofread -->

