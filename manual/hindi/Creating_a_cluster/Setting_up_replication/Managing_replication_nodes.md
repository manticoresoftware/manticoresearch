# प्रतिकृति नोड का प्रबंधन 

<!-- example managing replication nodes 1 -->
`ALTER CLUSTER <cluster_name> UPDATE nodes`  वक्तव्य निर्दिष्ट क्लस्टर के भीतर प्रत्येक नोड पर नोड सूचियों को अपडेट करता है ताकि क्लस्टर के सभी सक्रिय नोड शामिल हों। नोड सूचियों के बारे में अधिक जानकारी के लिए, देखें [Joining a cluster](../../Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md).


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
ALTER CLUSTER posts UPDATE nodes
```

<!-- request JSON -->

```json
POST /cli -d "
ALTER CLUSTER posts UPDATE nodes
"
```

<!-- request PHP -->

```php
$params = [
  'cluster' => 'posts',
  'body' => [
     'operation' => 'update',
     
  ]
];
$response = $client->cluster()->alter($params); 
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('ALTER CLUSTER posts UPDATE nodes')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('ALTER CLUSTER posts UPDATE nodes')
```

<!-- response Python-asyncio -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('ALTER CLUSTER posts UPDATE nodes');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("ALTER CLUSTER posts UPDATE nodes");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("ALTER CLUSTER posts UPDATE nodes");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("ALTER CLUSTER posts UPDATE nodes", Some(true)).await;
```

<!-- end -->


उदाहरण के लिए, जब क्लस्टर को आरंभ में स्थापित किया गया था, तो क्लस्टर में फिर से शामिल होने के लिए उपयोग की गई नोडों की सूची `10.10.0.1:9312,10.10.1.1:9312` थी। तब से, अन्य नोड क्लस्टर में शामिल हो गए हैं और अब सक्रिय नोड `10.10.0.1:9312,10.10.1.1:9312,10.15.0.1:9312,10.15.0.3:9312` हैं।हालांकि, क्लस्टर में फिर से शामिल होने के लिए उपयोग की गई नोडों की सूची अपडेट नहीं की गई है।

इसे सही करने के लिए, आप सक्रिय नोडों की सूची को क्लस्टर में फिर से शामिल होने के लिए उपयोग की जाने वाली नोडों की सूची में कॉपी करने के लिए `ALTER CLUSTER ... UPDATE nodes` वक्तव्य चला सकते हैं। इसके बाद, क्लस्टर में फिर से शामिल होने के लिए उपयोग की जाने वाली नोडों की सूची में क्लस्टर के सभी सक्रिय नोड शामिल होंगे। 

नोडों की दोनों सूचियां [Cluster status](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md) वक्तव्य का उपयोग करके देखी जा सकती हैं (`cluster_post_nodes_set` और `cluster_post_nodes_view`).

## क्लस्टर से नोड हटाना

प्रतिकृति क्लस्टर से नोड हटाने के लिए, इन चरणों का पालन करें:
1. नोड को रोकें
2. `<data_dir>/manticore.json` (सामान्य रूप से `/var/lib/manticore/manticore.json`) पर उस नोड से क्लस्टर के बारे में जानकारी निकालें जिसे रोका गया है।
3. किसी अन्य नोड पर `ALTER CLUSTER cluster_name UPDATE nodes` चलाएं।

इन चरणों के बाद, अन्य नोड हटा दिए गए नोड को भूल जाएंगे और हटाए गए नोड क्लस्टर को भूल जाएगा। यह क्रिया क्लस्टर में टेबल या हटाए गए नोड पर प्रभाव नहीं डालेगी।
<!-- proofread -->
# नकल नोड्स का प्रबंधन
<!-- example managing replication nodes 1 -->
`ALTER CLUSTER <cluster_name> UPDATE nodes` कथन निर्दिष्ट क्लस्टर के प्रत्येक नोड पर नोड सूची को अद्यतन करता है ताकि क्लस्टर में सभी सक्रिय नोड्स को शामिल किया जा सके। नोड सूचियों के बारे में अधिक जानकारी के लिए, [क्लस्टर में शामिल होना](../../Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md) देखें।

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
ALTER CLUSTER posts UPDATE nodes
```

<!-- request JSON -->

```json
POST /cli -d "
ALTER CLUSTER posts UPDATE nodes
"
```

<!-- request PHP -->

```php
$params = [
  'cluster' => 'posts',
  'body' => [
     'operation' => 'update',
     
  ]
];
$response = $client->cluster()->alter($params); 
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('ALTER CLUSTER posts UPDATE nodes')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('ALTER CLUSTER posts UPDATE nodes');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("ALTER CLUSTER posts UPDATE nodes");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("ALTER CLUSTER posts UPDATE nodes");
```
<!-- end -->

उदाहरण के लिए, जब क्लस्टर प्रारंभिक रूप से स्थापित किया गया था, क्लस्टर में फिर से शामिल होने के लिए उपयोग की जाने वाली नोड्स की सूची थी `10.10.0.1:9312,10.10.1.1:9312`। तब से, अन्य नोड्स ने क्लस्टर में शामिल हो गए हैं और अब सक्रिय नोड्स हैं `10.10.0.1:9312,10.10.1.1:9312,10.15.0.1:9312,10.15.0.3:9312`। हालाँकि, क्लस्टर में फिर से शामिल होने के लिए उपयोग की जाने वाली नोड्स की सूची को अपडेट नहीं किया गया है।

इसे सही करने के लिए, आप सक्रिय नोड्स की सूची को क्लस्टर में फिर से शामिल होने के लिए उपयोग की जाने वाली नोड्स की सूची में कॉपी करने के लिए `ALTER CLUSTER ... UPDATE nodes` कथन चला सकते हैं। इसके बाद, क्लस्टर में फिर से शामिल होने के लिए उपयोग की जाने वाली नोड्स की सूची में क्लस्टर में सभी सक्रिय नोड्स शामिल होंगे। 

नोड्स की दोनों सूचियाँ [क्लस्टर स्थिति](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md) कथन (`cluster_post_nodes_set` और `cluster_post_nodes_view`) का उपयोग करके देखी जा सकती हैं।

## क्लस्टर से नोड हटाना

नकल क्लस्टर से नोड हटाने के लिए, इन चरणों का पालन करें:
1. नोड को रोकें
2. रोके गए नोड पर `<data_dir>/manticore.json` (आमतौर पर `/var/lib/manticore/manticore.json`) से क्लस्टर के बारे में जानकारी हटा दें।
3. किसी अन्य नोड पर `ALTER CLUSTER cluster_name UPDATE nodes` चलाएँ।

इन चरणों के बाद, अन्य नोड्स अटके हुए नोड को भूल जाएंगे और अटका हुआ नोड क्लस्टर को भूलेगा। यह क्रिया क्लस्टर या अटके हुए नोड में तालिकाओं पर प्रभाव नहीं डालेगी।
<!-- proofread -->



