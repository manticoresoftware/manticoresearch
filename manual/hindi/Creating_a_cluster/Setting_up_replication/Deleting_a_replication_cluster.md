# एक पुनरुत्पादन क्लस्टर को हटाना 

<!-- example deleting a replication cluster  1 -->
`DELETE CLUSTER` वक्तव्य निर्दिष्ट क्लस्टर को उसके  [name](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#name) के साथ हटा देता है। एक बार क्लस्टर हट जाने के बाद, यह सभी नोड्स से हटा दिया जाता है, लेकिन इसके टेबल्स बरकरार रहते हैं और सक्रिय स्थानीय गैर-पुनरुत्पादित टेबल्स में बदल जाते हैं।



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
# एक प्रतिकृति क्लस्टर हटाना 

<!-- उदाहरण प्रतिकृति क्लस्टर हटाने का  1 -->
`DELETE CLUSTER` बयान निर्दिष्ट क्लस्टर को इसके  [name](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#name) के साथ हटा देता है। एक बार क्लस्टर हट जाने के बाद, यह सभी नोड्स से हटा दिया जाता है, लेकिन इसके तालिकाएँ बरकरार रहती हैं और सक्रिय स्थानीय गैर-प्रतिलिपि तालिकाएँ बन जाती हैं।



<!-- परिचय -->
##### SQL:

<!-- अनुरोध SQL -->

```sql
DELETE CLUSTER click_query
```

<!-- अनुरोध JSON -->

```json
POST /cli -d "DELETE CLUSTER click_query"
```

<!-- अनुरोध PHP -->

```php
$params = [
    'cluster' => 'click_query',
    'body' => []
];
$response = $client->cluster()->delete($params);                
```
<!-- परिचय -->
##### Python:

<!-- अनुरोध Python -->

```python
utilsApi.sql('DELETE CLUSTER click_query')
```

<!-- प्रतिक्रिया Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```
<!-- परिचय -->
##### Javascript:

<!-- अनुरोध जावास्क्रिप्ट -->

```javascript
res = await utilsApi.sql('DELETE CLUSTER click_query');
```

<!-- प्रतिक्रिया जावास्क्रिप्ट -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- परिचय -->
##### java:

<!-- अनुरोध जावा -->

```java
utilsApi.sql("DELETE CLUSTER click_query");
```

<!-- परिचय -->
##### C#:

<!-- अनुरोध C# -->

```clike
utilsApi.Sql("DELETE CLUSTER click_query");
```
<!-- अंत -->
<!-- प्रूफरीड -->
