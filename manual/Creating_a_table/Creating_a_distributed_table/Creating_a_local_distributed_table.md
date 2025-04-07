# Creating a local distributed table

<!-- example local_dist -->

A distributed table in Manticore Search acts as a "master node" that proxies the demanded query to other tables and provides merged results from the responses it receives. The table doesn't hold any data on its own. It can connect to both local tables and tables located on other servers. Here's an example of a simple distributed table:

<!-- intro -->
##### Configuration file:

<!-- request Configuration file -->
```ini
table table_dist {
  type  = distributed
  local = tbl1
  local = tbl2
  ...
 }
```

<!-- request RT mode -->
```sql
CREATE TABLE local_dist type='distributed' local='tbl1' local='tbl2';
```


<!-- request PHP -->

```php
$params = [
    'body' => [
        'settings' => [
            'type' => 'distributed',
            'local' => [
                'tbl1',
                'tbl2'
            ]
        ]
    ],
    'table' => 'products'
];
$table = new \Manticoresearch\Table($client);
$table->create($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE local_dist type=\'distributed\' local=\'tbl1\' local=\'tbl2\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE local_dist type=\'distributed\' local=\'tbl1\' local=\'tbl2\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE local_dist type='distributed' local='tbl1' local='tbl2'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE local_dist type='distributed' local='tbl1' local='tbl2'");
```

<!-- end -->
<!-- proofread -->