# Creating a local distributed table

<!-- example local_dist -->

A distributed table in Manticore Search acts as a "master node" that proxies the demanded query to other tables and provides merged results from the responses it receives. The table doesn't hold any data on its own. It can connect to both local tables and tables located on other servers. Here's an example of a simple distributed table:

<!-- intro -->
##### Configuration file:

<!-- request Configuration file -->
```ini
table index_dist {
  type  = distributed
  local = index1
  local = index2
  ...
 }
```

<!-- request RT mode -->
```sql
CREATE TABLE local_dist type='distributed' local='index1' local='index2';
```


<!-- request PHP -->

```php
$params = [
    'body' => [
        'settings' => [
            'type' => 'distributed',
            'local' => [
                'index1',
                'index2'
            ]
        ]
    ],
    'index' => 'products'
];
$index = new \Manticoresearch\Index($client);
$index->create($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE local_dist type=\'distributed\' local=\'index1\' local=\'index2\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE local_dist type=\'distributed\' local=\'index1\' local=\'index2\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE local_dist type='distributed' local='index1' local='index2'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE local_dist type='distributed' local='index1' local='index2'");
```

<!-- end -->
<!-- proofread -->