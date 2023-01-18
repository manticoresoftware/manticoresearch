# Adding and removing a table from a replication cluster 

<!-- example adding and removing a table from a replication cluster 1 -->
`ALTER CLUSTER <cluster_name> ADD <table_name>` adds an existing local table to the cluster. The node which receives the ALTER query sends the table to the other nodes in the cluster. All the local tables with the same name on the other nodes of the cluster get replaced with the new table.

After the table is replicated, write statements can be performed on any node but table name must be prefixed with the cluster name like `INSERT INTO <clusterName>:<table_name>`.


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
##### javascript:

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

<!-- end -->

<!-- example adding and removing a table from a replication cluster 2 -->
`ALTER CLUSTER <cluster_name> DROP <table_name>` forgets about a local table, i.e., it doesn't remove the table files on the nodes but just makes it an active non-replicated table.

After a table is removed from a cluster, it becomes a 'local' table and write statements must use just the table name as `INSERT INTO <table_name>`, without the cluster prefix.


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
##### javascript:

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

<!-- end -->
