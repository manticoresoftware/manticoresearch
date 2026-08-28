# 创建本地分布式表

<!-- example local_dist -->

在 Manticore Search 中，分布式表充当 "主节点"，将请求的查询代理到其他表，并提供从接收到的响应中合并的结果。该表本身不存储任何数据。它可以连接到本地表以及位于其他服务器上的表。本地分布式表只是一个所有子表都是本地表的分布式表。如果您只需要一起搜索几个本地表，可以直接查询它们，而无需创建分布式表。如果您确实创建了本地分布式表，在 SQL 中，您可以通过重复 `local='...'` 或通过将它们作为逗号分隔的列表传递到单个 `local='index1,index2'` 子句中来指定多个本地表。


<!-- intro -->
##### 配置:

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
##### Python：

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE local_dist type=\'distributed\' local=\'index1\' local=\'index2\'')
```

<!-- intro -->
##### Python-asyncio：

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE local_dist type=\'distributed\' local=\'index1\' local=\'index2\'')
```

<!-- intro -->
##### JavaScript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE local_dist type=\'distributed\' local=\'index1\' local=\'index2\'');
```

<!-- intro -->
##### Java：
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE local_dist type='distributed' local='index1' local='index2'");
```

<!-- intro -->
##### C#：
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE local_dist type='distributed' local='index1' local='index2'");
```

<!-- intro -->
##### Rust：

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE local_dist type='distributed' local='index1' local='index2'", Some(true)).await;
```

<!-- end -->

<!-- example local_tables_direct_query -->
在 SQL 和 JSON 中，直接查询多个本地表都有效。

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
