# 添加和移除复制集群中的表

<!-- example adding and removing a table from a replication cluster 1 -->
`ALTER CLUSTER <cluster_name> ADD <table_name>[, <table_name>]` 将一个或多个现有的本地表添加到集群中。接收 ALTER 查询的节点将表发送给集群中的其他节点。集群中其他节点上同名的所有本地表都将被新的表替换。

一旦表被复制，任何节点上都可以执行写入语句，但表名必须以集群名称为前缀，例如 `INSERT INTO <clusterName>:<table_name>`。


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
`ALTER CLUSTER <cluster_name> DROP <table_name>[, <table_name>]` 忘记一个或多个现有的表，意味着它并不删除节点上的表文件，而只是使其不活动，变成非复制的表。

一旦表从集群中移除，它将成为 `local` 表，写入语句必须只使用表名，例如 `INSERT INTO <table_name>`，而不带集群前缀。


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
