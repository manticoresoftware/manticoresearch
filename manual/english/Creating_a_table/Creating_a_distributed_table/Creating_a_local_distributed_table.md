# Creating a local distributed table

<!-- example local_dist -->

A distributed table in Manticore Search acts as a "master node" that proxies the demanded query to other tables and provides merged results from the responses it receives. The table doesn't hold any data on its own. It can connect to both local tables and tables located on other servers. A local distributed table is just a distributed table whose children are all local tables. If you only need to search several local tables together, you can query them directly instead of creating a distributed table. If you do create a local distributed table, in SQL you can specify multiple local tables either by repeating `local='...'` or by passing them as a comma-separated list in a single `local='index1,index2'` clause.


<!-- intro -->
##### Config:

<!-- request Config -->
```ini
table index_dist {
  type  = distributed
  local = index1
  local = index2
  ...
 }
```

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
CREATE TABLE local_dist type='distributed' local='index1' local='index2';
```

<!-- request SQL -->
```sql
CREATE TABLE local_dist type='distributed' local='index1,index2';
```

<!-- intro -->
##### PHP:

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
    'table' => 'products'
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
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE local_dist type=\'distributed\' local=\'index1\' local=\'index2\'')
```

<!-- intro -->
##### Javascript:

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

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE local_dist type='distributed' local='index1' local='index2'", Some(true)).await;
```

<!-- end -->

<!-- example local_tables_direct_query -->
Querying several local tables directly works in both SQL and JSON.

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
SELECT * FROM index1, index2, index3;
```

<!-- intro -->
##### JSON:

<!-- request JSON -->
```json
POST /search
{
  "table": "index1,index2,index3",
  "query": { "match_all": {} }
}
```
<!-- end -->
