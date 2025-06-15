# 在复制集群中添加和移除表

<!-- example adding and removing a table from a replication cluster 1 -->
`ALTER CLUSTER <cluster_name> ADD <table_name>[, <table_name>]` 将一个或多个现有的本地表添加到集群中。接收 ALTER 查询的节点会将表发送给集群中的其他节点。集群中其他节点上同名的所有本地表将被新的表替换。

表一旦被复制，就可以在任何节点上执行写操作，但表名必须加上集群名称作为前缀，例如 `INSERT INTO <clusterName>:<table_name>`。


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
ALTER CLUSTER click_query ADD clicks_daily_index
```

<!-- request JSON -->

```json
POST /cli -d "
ALTER CLUSTER click_query ADD clicks_daily_index
"
```

<!-- request PHP -->

```php
$params = [
  'cluster' => 'click_query',
  'body' => [
     'operation' => 'add',
     'table' => 'clicks_daily_index'

  ]
];
$response = $client->cluster()->alter($params);
```


<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('ALTER CLUSTER click_query ADD clicks_daily_index')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('ALTER CLUSTER click_query ADD clicks_daily_index')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('ALTER CLUSTER click_query ADD clicks_daily_index');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("ALTER CLUSTER click_query ADD clicks_daily_index");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("ALTER CLUSTER click_query ADD clicks_daily_index");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("ALTER CLUSTER click_query ADD clicks_daily_index", Some(true)).await;
```

<!-- end -->

<!-- example adding and removing a table from a replication cluster 2 -->
`ALTER CLUSTER <cluster_name> DROP <table_name>[, <table_name>]` 忘记一个或多个现有的表，意思是不删除节点上的表文件，而只是使它们变为非活动、不复制的表。

表一旦从集群中移除，就变为 `local` 表，写操作必须仅使用表名，例如 `INSERT INTO <table_name>`，不带集群前缀。


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
ALTER CLUSTER posts DROP weekly_index
```

<!-- request JSON -->

```json
POST /cli -d "
ALTER CLUSTER posts DROP weekly_index
"
```

<!-- request PHP -->

```php
$params = [
  'cluster' => 'posts',
  'body' => [
     'operation' => 'drop',
     'table' => 'weekly_index'

  ]
];
$response = $client->cluster->alter($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('ALTER CLUSTER posts DROP weekly_index')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('ALTER CLUSTER posts DROP weekly_index')
```

<!-- response Python-asyncio -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('ALTER CLUSTER posts DROP weekly_index');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("ALTER CLUSTER posts DROP weekly_index");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("ALTER CLUSTER posts DROP weekly_index");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("ALTER CLUSTER posts DROP weekly_index", Some(true)).await;
```

<!-- end -->
<!-- proofread -->

