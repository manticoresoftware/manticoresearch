# प्रतिकृति समूह में शामिल होना

<!-- example joining a replication cluster 1 -->
मौजूदा समूह में शामिल होने के लिए, आपको कम से कम निम्नलिखित निर्दिष्ट करना होगा:
* समूह का [नाम](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster)
* आप जिस समूह में शामिल हो रहे हैं, उसमें किसी अन्य नोड का `host:port`

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
JOIN CLUSTER posts AT '10.12.1.35:9312'
```

<!-- request JSON -->

```json
POST /cli -d "
JOIN CLUSTER posts AT '10.12.1.35:9312'
"
```

<!-- request PHP -->

```php
$params = [
  'cluster' => 'posts',
  'body' => [
      '10.12.1.35:9312'
  ]
];
$response = $client->cluster->join($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('JOIN CLUSTER posts AT '10.12.1.35:9312'')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('JOIN CLUSTER posts AT '10.12.1.35:9312'')
```

<!-- response Python-asyncio -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### जावास्क्रिप्ट:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('JOIN CLUSTER posts AT '10.12.1.35:9312'');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### जावा:

<!-- request Java -->

```java
utilsApi.sql("JOIN CLUSTER posts AT '10.12.1.35:9312'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("JOIN CLUSTER posts AT '10.12.1.35:9312'");
```

<!-- intro -->
##### रस्ट:

<!-- request Rust -->

```rust
utils_api.sql("JOIN CLUSTER posts AT '10.12.1.35:9312'", Some(true)).await;
```

<!-- end -->

<!-- example joining a replication cluster 1_1 -->
अधिकांश मामलों में, उपरोक्त एक एकल प्रतिकृति समूह होने पर पर्याप्त है। हालांकि, यदि आप एकाधिक प्रतिकृति समूह बना रहे हैं, तो आपको [पथ](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) भी सेट करना होगा और यह सुनिश्चित करना होगा कि निर्देशिका उपलब्ध है.

<!-- request SQL -->
```sql
JOIN CLUSTER c2 at '127.0.0.1:10201' 'c2' as path
```
<!-- end -->

एक नोड एक निर्दिष्ट नोड से डेटा प्राप्त करके समूह में शामिल होता है और, यदि सफल होता है, तो अन्य सभी समूह नोड्स में नोड सूचियों को उसी प्रकार अपडेट करता है जैसे कि [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) के माध्यम से मैन्युअली किया जाता है। इस सूची का उपयोग पुनरारंभ के समय नोड्स को समूह में पुनः शामिल करने के लिए किया जाता है.

नोड्स की दो सूचियाँ होती हैं:
1.`cluster_<name>_nodes_set`: पुनरारंभ के समय नोड्स को समूह में पुनः शामिल करने के लिए उपयोग किया जाता है। इसे [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) द्वारा की गई तरह सभी नोड्स में अपडेट किया जाता है। `JOIN CLUSTER` कमांड यह अपडेट स्वचालित रूप से करता है। [Cluster status](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md) इस सूची को `cluster_<name>_nodes_set` के रूप में प्रदर्शित करता है.
2. `cluster_<name>_nodes_view`: यह सूची सभी सक्रिय नोड्स को शामिल करती है जो प्रतिकृति के लिए उपयोग में लाए जाते हैं और इसके लिए मैन्युअल प्रबंधन की आवश्यकता नहीं होती है। [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) वास्तव में इस नोड्स की सूची को पुनरारंभ के समय पुनः शामिल करने के लिए प्रयुक्त नोड्स की सूची में कॉपी कर देता है। [Cluster status](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md) इस सूची को `cluster_<name>_nodes_view` के रूप में प्रदर्शित करता है.

<!-- example joining a replication cluster  2 -->
जब नोड्स विभिन्न नेटवर्क सेगमेंट या डेटा केंद्रों में स्थित होते हैं, तो [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) विकल्प को स्पष्ट रूप से सेट किया जा सकता है। इससे नोड्स के बीच ट्रैफ़िक कम होता है और डेटा केंद्रों के बीच आपसी संचार के लिए गेटवे नोड्स का उपयोग किया जाता है। निम्नलिखित कोड [nodes](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) विकल्प का उपयोग करके मौजूदा समूह में शामिल होता है.

> **नोट:** इस सिंटैक्स का उपयोग करते समय क्लस्टर `cluster_<name>_nodes_set` सूची स्वचालित रूप से अपडेट नहीं होती है। इसे अपडेट करने के लिए, [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) का उपयोग करें.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
JOIN CLUSTER click_query 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312' as nodes
```

<!-- request JSON -->

```json
POST /cli -d "
JOIN CLUSTER click_query 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312' as nodes
"
```

<!-- request PHP -->

```php
$params = [
  'cluster' => 'posts',
  'body' => [
      'nodes' => 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312'
  ]
];
$response = $client->cluster->join($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('JOIN CLUSTER click_query 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312' as nodes')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('JOIN CLUSTER click_query 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312' as nodes')
```

<!-- response Python-asyncio -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### जावास्क्रिप्ट:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('JOIN CLUSTER click_query 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312' as nodes');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("JOIN CLUSTER click_query 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312' as nodes");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("JOIN CLUSTER click_query 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312' as nodes");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("JOIN CLUSTER click_query 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312' as nodes", Some(true)).await;
```

<!-- end -->

`JOIN CLUSTER` कमांड समकालिक रूप से काम करती है और जब नोड क्लस्टर के अन्य नोड्स से सभी डेटा प्राप्त कर लेता है और उनके साथ समन्वय में होता है, तब यह पूरा हो जाती है।

`JOIN CLUSTER` ऑपरेशन एक त्रुटि संदेश के साथ विफल हो सकता है जो एक डुप्लिकेट [server_id](../../Server_settings/Searchd.md#server_id) को इंगित करता है। यह तब होता है जब शामिल हो रहे नोड का `server_id` क्लस्टर में एक मौजूदा नोड के समान होता है। इस समस्या को हल करने के लिए, यह सुनिश्चित करें कि प्रतिकृति क्लस्टर के प्रत्येक नोड का एक अद्वितीय [server_id](../../Server_settings/Searchd.md#server_id) हो। आप अपने कॉन्फ़िगरेशन फ़ाइल के "searchd" अनुभाग में डिफ़ॉल्ट [server_id](../../Server_settings/Searchd.md#server_id) को एक अद्वितीय मान में बदल सकते हैं इससे पहले कि आप क्लस्टर में शामिल होने का प्रयास करें।
<!-- proofread -->
# एक प्रतिसं复制 क्लस्टर में शामिल होना

<!-- example joining a replication cluster 1 -->
मौजूदा क्लस्टर में शामिल होने के लिए, आपको कम से कम निम्नलिखित निर्दिष्ट करना होगा:
* क्लस्टर का [नाम](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster)
* क्लस्टर में शामिल होने वाले दूसरे नोड का `host:port`

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
JOIN CLUSTER posts AT '10.12.1.35:9312'
```

<!-- request JSON -->

```json
POST /cli -d "
JOIN CLUSTER posts AT '10.12.1.35:9312'
"
```

<!-- request PHP -->

```php
$params = [
  'cluster' => 'posts',
  'body' => [
      '10.12.1.35:9312'
  ]
];
$response = $client->cluster->join($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('JOIN CLUSTER posts AT \'10.12.1.35:9312\'')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('JOIN CLUSTER posts AT \'10.12.1.35:9312\'');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("JOIN CLUSTER posts AT '10.12.1.35:9312'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("JOIN CLUSTER posts AT '10.12.1.35:9312'");
```

<!-- end -->

<!-- example joining a replication cluster 1_1 -->
अधिकांश मामलों में, उपर्युक्त एकल प्रतिसं复制 क्लस्टर के लिए पर्याप्त है। हालांकि, यदि आप कई प्रतिसं复制 क्लस्टर बना रहे हैं, तो आपको [पथ](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) भी सेट करना होगा और सुनिश्चित करना होगा कि निर्देशिका उपलब्ध है।

<!-- request SQL -->
```sql
JOIN CLUSTER c2 at '127.0.0.1:10201' 'c2' as path
```
<!-- end -->

एक नोड एक निर्दिष्ट नोड से डेटा प्राप्त करके क्लस्टर में शामिल होता है और, यदि सफल होता है, तो सभी अन्य क्लस्टर नोड्स में नोड सूचियों को उसी तरह अपडेट करता है जैसे कि इसे [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) के माध्यम से मैन्युअल रूप से किया गया हो। इस सूची का उपयोग नोड्स को पुनः प्रारंभ पर क्लस्टर में फिर से शामिल करने के लिए किया जाता है।

नोड्स की दो सूचियाँ हैं:
1.`cluster_<name>_nodes_set`: इसे पुनः प्रारंभ पर नोड्स को क्लस्टर में फिर से शामिल करने के लिए उपयोग किया जाता है। इसे सभी नोड्स में उसी तरह अपडेट किया जाता है जैसे [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) करता है। `JOIN CLUSTER` आदेश स्वतः इस अपडेट को निष्पादित करता है। [क्लस्टर स्थिति](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md) इस सूची को `cluster_<name>_nodes_set` के रूप में प्रदर्शित करता है।
2. `cluster_<name>_nodes_view`: यह सूची सभी सक्रिय नोड्स को दर्शाती है जो प्रतिसं复制 के लिए उपयोग की जाती हैं और यह मैन्युअल प्रबंधन की आवश्यकता नहीं होती है। [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) वास्तव में इस नोड्स की सूची को पुनः प्रारंभ पर शामिल करने के लिए उपयोग की जाने वाली नोड्स की सूची में कॉपी करता है। [क्लस्टर स्थिति](../../Creating_a_cluster/Setting_up_replication/Replication_cluster_status.md) इस सूची को `cluster_<name>_nodes_view` के रूप में प्रदर्शित करता है।

<!-- example joining a replication cluster  2 -->
जब नोड्स विभिन्न नेटवर्क सेगमेंट या डेटा केंद्रों में स्थित होते हैं, तो [नोड्स](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) विकल्प को स्पष्ट रूप से सेट किया जा सकता है। इससे नोड्स के बीच ट्रैफ़िक कम होता है और डेटा केंद्रों के बीच अंतःसंचार के लिए गेटवे नोड्स का उपयोग किया जाता है। निम्नलिखित कोड [नोड्स](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Replication-cluster) विकल्प का उपयोग करके मौजूदा क्लस्टर में शामिल होता है।

> **नोट:** जब इस वाक्यांश का उपयोग किया जाता है, तो क्लस्टर `cluster_<name>_nodes_set` सूची स्वचालित रूप से अपडेट नहीं होती है। इसे अपडेट करने के लिए, [ALTER CLUSTER ... UPDATE nodes](../../Creating_a_cluster/Setting_up_replication/Managing_replication_nodes.md) का उपयोग करें।


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
JOIN CLUSTER click_query 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312' as nodes
```

<!-- request JSON -->

```json
POST /cli -d "
JOIN CLUSTER click_query 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312' as nodes
"
```

<!-- request PHP -->

```php
$params = [
  'cluster' => 'posts',
  'body' => [
      'nodes' => 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312'
  ]
];
$response = $client->cluster->join($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('JOIN CLUSTER click_query \'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312\' as nodes')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('JOIN CLUSTER click_query \'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312\' as nodes');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("JOIN CLUSTER click_query 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312' as nodes");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("JOIN CLUSTER click_query 'clicks_mirror1:9312;clicks_mirror2:9312;clicks_mirror3:9312' as nodes");
```
<!-- end -->

`JOIN CLUSTER` आदेश समकालिक रूप से कार्य करता है और जैसे ही नोड क्लस्टर के अन्य नोड्स से सभी डेटा प्राप्त करता है और उनके साथ समन्वित होता है, यह पूरा होता है।
`JOIN CLUSTER` ऑपरेशन एक त्रुटि संदेश के साथ विफल हो सकता है जो एक डुप्लीकेट [server_id](../../Server_settings/Searchd.md#server_id) को इंगित करता है। यह तब होता है जब जोड़ने वाला नोड क्लस्टर में एक मौजूदा नोड के समान `server_id` रखता है। इस मुद्दे को हल करने के लिए, सुनिश्चित करें कि प्रजनन क्लस्टर में प्रत्येक नोड के पास एक अद्वितीय [server_id](../../Server_settings/Searchd.md#server_id) है। आप अपने कॉन्फ़िगरेशन फ़ाइल के "searchd" अनुभाग में डिफ़ॉल्ट [server_id](../../Server_settings/Searchd.md#server_id) को एक अद्वितीय मान में बदल सकते हैं, इससे पहले कि आप क्लस्टर में शामिल होने का प्रयास करें।
<!-- proofread -->

