# एक प्रतिकृति क्लस्टर बनाने

<!-- example creating a replication cluster 1 -->
एक प्रतिकृति क्लस्टर बनाने के लिए, आपको कम से कम इसका [नाम](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#name) सेट करना होगा.

यदि आप एकल क्लस्टर या पहले क्लस्टर बना रहे हैं, तो आप [पथ](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#path) विकल्प को छोड़ सकते हैं। इस मामले में, [data_dir](../../Server_settings/Searchd.md#data_dir) विकल्प क्लस्टर पथ के रूप में उपयोग किया जाएगा। हालाँकि, सभी बाद के क्लस्टरों के लिए, आपको [पथ](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#path) निर्दिष्ट करना होगा और पथ उपलब्ध होना चाहिए। [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#nodes) विकल्प को भी क्लस्टर में सभी नोड्स की सूची बनाने के लिए सेट किया जा सकता है.


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
utilsApi.sql('CREATE CLUSTER click_query '/var/data/click_query/' as path')
utilsApi.sql('CREATE CLUSTER click_query '/var/data/click_query/' as path, 'clicks_mirror1:9312,clicks_mirror2:9312,clicks_mirror3:9312' as nodes')

```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE CLUSTER posts')
await utilsApi.sql('CREATE CLUSTER click_query '/var/data/click_query/' as path')
await utilsApi.sql('CREATE CLUSTER click_query '/var/data/click_query/' as path, 'clicks_mirror1:9312,clicks_mirror2:9312,clicks_mirror3:9312' as nodes')

```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE CLUSTER posts');
res = await utilsApi.sql('CREATE CLUSTER click_query '/var/data/click_query/' as path');
res = await utilsApi.sql('CREATE CLUSTER click_query '/var/data/click_query/' as path, 'clicks_mirror1:9312,clicks_mirror2:9312,clicks_mirror3:9312' as nodes');
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

यदि [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#nodes) विकल्प क्लस्टर बनाने के समय निर्दिष्ट नहीं किया जाता है, तो क्लस्टर में शामिल होने वाला पहला नोड [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#nodes) विकल्प के रूप में संगृहीत किया जाएगा.

<!-- proofread -->
# एक प्रतिकृति समूह बनाना

<!-- example creating a replication cluster 1 -->
एक प्रतिकृति समूह बनाने के लिए, आपको न्यूनतम इसके [नाम](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#name) को सेट करना होगा.

यदि आप एक एकल समूह या पहले समूह का निर्माण कर रहे हैं, तो आप [पथ](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#path) विकल्प को छोड़ सकते हैं। इस मामले में, [data_dir](../../Server_settings/Searchd.md#data_dir) विकल्प को समूह पथ के रूप में उपयोग किया जाएगा। हालांकि, सभी बाद के समूहों के लिए, आपको [पथ](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#path) निर्दिष्ट करना होगा और पथ उपलब्ध होना चाहिए। [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#nodes) विकल्प को समूह में सभी नोड्स की सूची तैयार करने के लिए भी सेट किया जा सकता है.


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
utilsApi.sql('CREATE CLUSTER click_query '/var/data/click_query/' as path')
utilsApi.sql('CREATE CLUSTER click_query '/var/data/click_query/' as path, 'clicks_mirror1:9312,clicks_mirror2:9312,clicks_mirror3:9312' as nodes')

```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE CLUSTER posts')
await utilsApi.sql('CREATE CLUSTER click_query '/var/data/click_query/' as path')
await utilsApi.sql('CREATE CLUSTER click_query '/var/data/click_query/' as path, 'clicks_mirror1:9312,clicks_mirror2:9312,clicks_mirror3:9312' as nodes')

```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE CLUSTER posts');
res = await utilsApi.sql('CREATE CLUSTER click_query '/var/data/click_query/' as path');
res = await utilsApi.sql('CREATE CLUSTER click_query '/var/data/click_query/' as path, 'clicks_mirror1:9312,clicks_mirror2:9312,clicks_mirror3:9312' as nodes');
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

यदि [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#nodes) विकल्प को दिए बिना एक समूह बनाया जाता है, तो समूह में पहले शामिल होने वाला पहला नोड [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#nodes) विकल्प के रूप में सहेजा जाएगा.

<!-- proofread -->
# एक पुनःप्रकाशन क्लस्टर बना रहे हैं

<!-- example creating a replication cluster 1 -->
एक पुनःप्रकाशन क्लस्टर बनाने के लिए, आपको न्यूनतम इसके [नाम](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#name) को सेट करना होगा.

यदि आप एकल क्लस्टर या पहले क्लस्टर का निर्माण कर रहे हैं, तो आप [पथ](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#path) विकल्प को छोड़ सकते हैं। इस मामले में, [data_dir](../../Server_settings/Searchd.md#data_dir) विकल्प क्लस्टर पथ के रूप में उपयोग किया जाएगा। हालाँकि, सभी बाद के क्लस्टरों के लिए, आपको [पथ](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#path) को निर्दिष्ट करना चाहिए और पथ उपलब्ध होना चाहिए। [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#nodes) विकल्प को पूरे क्लस्टर में सभी नोड्स की सूची देने के लिए भी सेट किया जा सकता है.

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

यदि [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#nodes) विकल्प को क्लस्टर बनाते समय निर्दिष्ट नहीं किया गया है, तो क्लस्टर में शामिल होने वाला पहला नोड [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#nodes) विकल्प के रूप में सहेजा जाएगा.

<!-- proofread -->

