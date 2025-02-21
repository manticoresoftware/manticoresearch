# 在复制集群中添加和删除表 

<!-- example adding and removing a table from a replication cluster 1 -->

`ALTER CLUSTER <cluster_name> ADD <table_name>[, <table_name>]` 可以将一个或多个现有本地表添加到集群中。接收 ALTER 查询的节点会将表发送到集群中的其他节点。集群中其他节点上同名的所有本地表将被新表替换。

一旦表被复制，写操作可以在任何节点上执行，但表名必须以集群名称为前缀，例如 `INSERT INTO <clusterName>:<table_name>`。


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
     'index' => 'clicks_daily_index'
      
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

<!-- end -->

<!-- example adding and removing a table from a replication cluster 2 -->

`ALTER CLUSTER <cluster_name> DROP <table_name>[, <table_name>]` 将一个或多个现有表标记为不再使用，这意味着它不会删除节点上的表文件，而是将它们设置为不活动的、非复制的表。

一旦表从集群中移除，它将变为 `local` 表，写操作必须仅使用表名，例如 `INSERT INTO <table_name>`，而不带集群前缀。


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
     'index' => 'weekly_index'
      
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

<!-- end -->
<!-- proofread -->