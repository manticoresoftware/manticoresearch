# Deleting a replication cluster 

<!-- example deleting a replication cluster  1 -->
The `DELETE CLUSTER` statement removes the specified cluster with its  [name](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#name). Once the cluster is deleted, it is removed from all nodes, but its tables remain intact and become active local non-replicated tables.



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
<!-- end -->
<!-- proofread -->