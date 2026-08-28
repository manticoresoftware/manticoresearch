# 删除复制集群

<!-- example deleting a replication cluster  1 -->
`DELETE CLUSTER` 语句会删除指定的集群及其 [名称](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#name)。一旦删除集群，它将从所有节点中移除，但其表保持完整并成为活动的本地非复制表。

如果只需分离当前节点并让集群的其余部分继续运行，请改用 `EXIT CLUSTER`。请参阅 [管理复制节点](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md)。



<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
DELETE CLUSTER click_query
```

<!-- request JSON -->

```json
POST /cli -d "DELETE CLUSTER click_query"
```

<!-- request PHP -->

```php
$params = [
    'cluster' => 'click_query',
    'body' => []
];
$response = $client->cluster()->delete($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('DELETE CLUSTER click_query')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('DELETE CLUSTER click_query')
```

<!-- response Python-asyncio -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('DELETE CLUSTER click_query');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("DELETE CLUSTER click_query");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("DELETE CLUSTER click_query");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.Sql("DELETE CLUSTER click_query", Some(true)).await;
```

<!-- end -->
<!-- proofread -->
