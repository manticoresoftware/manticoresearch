# Удаление кластера репликации 

<!-- example deleting a replication cluster  1 -->
Оператор `DELETE CLUSTER` удаляет указанный кластер с его [имя](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#name). После удаления кластера он исключается из всех узлов, но его таблицы остаются нетронутыми и становятся активными локальными нереплицированными таблицами.



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
