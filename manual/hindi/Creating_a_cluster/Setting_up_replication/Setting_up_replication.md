# प्रतिकृति सेट करना
Manticore के साथ, लेखन लेनदेन (जैसे `INSERT`, `REPLACE`, `DELETE`, `TRUNCATE`, `UPDATE`, `COMMIT`) को अन्य क्लस्टर नोड्स पर प्रतिकृत किया जा सकता है इससे पहले कि लेनदेन को वर्तमान नोड पर पूरी तरह से लागू किया जा सके। वर्तमान में, प्रतिकृति `percolate`, `rt` और `distributed` टेबल के लिए लिनक्स और मैकोज़ में समर्थित है।
Manticore के लिए [स्थानीय विंडोज बाइनरी](../../Installation/Windows.md#Installing-Manticore-as-native-Windows-binaries) प्रतिकृति का समर्थन नहीं करते हैं। हम [WSL के माध्यम से Manticore स्थापित करने](../../Installation/Windows.md#Installing-or-enabling-WSL2) की सिफारिश करते हैं (विंडोज सबसिस्टम फॉर लिनक्स)।
[macOS](../../Installation/MacOS.md) पर, प्रतिकृति सीमित समर्थन के साथ है और इसे केवल विकास उद्देश्यों के लिए ही सिफारिश की जाती है।
Manticore की प्रतिकृति [Galera पुस्तकालय](https://github.com/codership/galera) द्वारा संचालित है और कई प्रभावशाली विशेषताओं का दावा करती है:
* वास्तविक मल्टी-मास्टर: किसी भी नोड पर किसी भी समय पढ़ें और लिखें।
* [प्रायोगिक रूप से समकालिक प्रतिकृति](https://galeracluster.com/library/documentation/overview.html) कोई दास विलंब नहीं और नोड क्रैश के बाद कोई डेटा हानि नहीं।
* हॉट स्टैंडबाय: फेलओवर के दौरान कोई डाउनटाइम नहीं (क्योंकि कोई फेलओवर नहीं है)।
* सख्ती से जुड़े हुए: सभी नोड्स समान स्थिति रखते हैं और नोड्स के बीच भिन्न डेटा की अनुमति नहीं है।
* स्वचालित नोड प्रोविजनिंग: डेटाबेस का मैन्युअल बैकअप लेने और उसे नए नोड पर पुनर्स्थापित करने की कोई आवश्यकता नहीं है।
* उपयोग और तैनाती में आसान।
* अस्थिर नोड्स का पता लगाने और स्वचालित निष्कासन।
* प्रमाणन आधारित प्रतिकृति।
Manticore सर्च में प्रतिकृति सेट करने के लिए:
* "searchd" कॉन्फ़िगरेशन फ़ाइल के अनुभाग में [data_dir](../../Server_settings/Searchd.md#data_dir) विकल्प को सेट करना आवश्यक है। सामान्य मोड में प्रतिकृति का समर्थन नहीं किया जाता है।
* एक [listen](../../Server_settings/Searchd.md#listen) निर्देशांक निर्दिष्ट किया जाना चाहिए, जिसमें ऐसा IP पते शामिल है जो अन्य नोड्स द्वारा पहुंच योग्य हो, या एक [node_address](../../Server_settings/Searchd.md#node_address) के साथ एक पहुंच योग्य IP पता।
* वैकल्पिक रूप से, आप प्रत्येक क्लस्टर नोड पर [server_id](../../Server_settings/Searchd.md#server_id) के लिए अद्वितीय मान सेट कर सकते हैं। यदि कोई मान सेट नहीं है, तो नोड MAC पते या एक यादृच्छिक संख्या का उपयोग करके `server_id` उत्पन्न करने का प्रयास करेगा।
यदि कोई `प्रतिकृति` [listen](../../Server_settings/Searchd.md#listen) निर्देशांक सेट नहीं है, तो Manticore प्रत्येक बनाए गए क्लस्टर के लिए डिफ़ॉल्ट प्रोटोकॉल सुनने वाले पोर्ट के बाद 200 पोर्ट के श्रेणी में पहले दो मुक्त पोर्ट का उपयोग करेगा। प्रतिकृति पोर्ट को मैन्युअल रूप से सेट करने के लिए, [listen](../../Server_settings/Searchd.md#listen) निर्देशांक (जिसका `प्रतिकृति` प्रकार है) पोर्ट श्रेणी को परिभाषित किया जाना चाहिए और पता/पोर्ट श्रेणी जोड़ी का अन्य नोड्स के बीच एक ही सर्वर पर प्रतिच्छेद नहीं होना चाहिए। एक नियम के रूप में, पोर्ट श्रेणी को प्रति क्लस्टर कम से कम दो पोर्ट निर्दिष्ट करने चाहिए। जब आप एक पोर्ट श्रेणी के साथ एक प्रतिकृति श्रोता को परिभाषित करते हैं (जैसे, `listen = 192.168.0.1:9320-9328:replication`), Manticore तुरंत इन पोर्ट पर सुनना शुरू नहीं करता है। इसके बजाय, जब आप प्रतिकृति का उपयोग करना शुरू करते हैं, तो यह निर्दिष्ट श्रेणी में से यादृच्छिक मुक्त पोर्ट लेगा।
## प्रतिकृति क्लस्टर
एक प्रतिकृति क्लस्टर नोड्स का एक समूह है जिसमें एक लेखन लेनदेन प्रतिकृत किया जाता है। प्रतिकृति प्रति-टेबल आधार पर सेट की जाती है, जिसका अर्थ है कि एक तालिका केवल एक क्लस्टर की हो सकती है। एक क्लस्टर में तालिकाओं की संख्या पर कोई सीमा नहीं है। `INSERT`, `REPLACE`, `DELETE`, `TRUNCATE` जैसे सभी लेनदेन किसी भी पर्कोलेट या वास्तविक समय की तालिका पर जो क्लस्टर से संबंधित हैं, उस क्लस्टर में सभी अन्य नोड्स पर प्रतिकृत किए जाते हैं। [वितरित](../../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md#Creating-a-distributed-table) तालिकाएँ भी प्रतिकृति प्रक्रिया का हिस्सा हो सकती हैं। प्रतिकृति मल्टी-मास्टर है, इसलिए किसी भी नोड या कई नोड्स पर एक साथ लिखने में कोई समस्या नहीं होगी।
एक क्लस्टर बनाने के लिए, आप सामान्यत: कमांड [create cluster](../../Creating_a_cluster/Setting_up_replication/Creating_a_replication_cluster.md#Creating-a-replication-cluster) का उपयोग कर सकते हैं `CREATE CLUSTER <cluster name>` के साथ, और एक क्लस्टर में शामिल होने के लिए, आप [join cluster](../../Creating_a_cluster/Setting_up_replication/Joining_a_replication_cluster.md#Joining-a-replication-cluster) का उपयोग कर सकते हैं `JOIN CLUSTER <cluster name> at 'host:port'` के साथ। हालाँकि, कुछ दुर्लभ मामलों में, आप `CREATE/JOIN CLUSTER` के व्यवहार को बेहतर रूप से समायोजित करना चाह सकते हैं। उपलब्ध विकल्प हैं:
### नाम
यह विकल्प क्लस्टर का नाम निर्दिष्ट करता है। यह सिस्टम में सभी क्लस्टरों के बीच अद्वितीय होना चाहिए।
> **नोट:** `JOIN` कमांड के लिए अधिकतम अनुमत होस्टनाम लंबाई **253** अक्षर है। यदि आप इस सीमा को पार करते हैं, तो searchd एक त्रुटि उत्पन्न करेगा।
### पथ
पथ विकल्प [write-set cache replication](https://galeracluster.com/library/documentation/state-transfer.html#state-transfer-gcache) और अन्य नोड्स से आने वाली तालिकाओं के लिए डेटा निर्देशिका निर्दिष्ट करता है। यह मान सिस्टम में सभी क्लस्टरों के बीच अद्वितीय होना चाहिए और [data_dir](../../Server_settings/Searchd.md#data_dir). निर्देशिका के लिए सापेक्ष पथ के रूप में निर्दिष्ट किया जाना चाहिए। डिफ़ॉल्ट रूप से, इसे [data_dir](../../Server_settings/Searchd.md#data_dir) के मान पर सेट किया जाता है।
### नोड्स
`nodes` विकल्प क्लस्टर में सभी नोड्स के लिए पता:पोर्ट जोड़ी की एक सूची है, जिसे अंश द्वारा अलग किया जाता है। यह सूची नोड के API इंटरफ़ेस का उपयोग करके प्राप्त की जानी चाहिए और इसमें वर्तमान नोड का पता भी शामिल हो सकता है। इसका उपयोग नोड को क्लस्टर में शामिल करने और पुनरारंभ के बाद फिर से शामिल करने के लिए किया जाता है।
### विकल्प
`options` विकल्प आपको Galera प्रतिकृति प्लगइन को सीधे अतिरिक्त विकल्प देने की अनुमति देता है, जैसा कि [Galera दस्तावेज़ीकरण पैरामीटर](https://galeracluster.com/library/documentation/galera-parameters.html) में वर्णित है।
## लेखन कथन
<!-- उदाहरण लेखन कथन 1 -->
जब किसी रेप्लिकेशन क्लस्टर के साथ काम कर रहे हों, तो सभी राइट स्टेटमेंट जैसे `INSERT`, `REPLACE`, `DELETE`, `TRUNCATE`, `UPDATE` जो क्लस्टर की टेबल की सामग्री को संशोधित करते हैं, को `cluster_name:table_name` अभिव्यक्ति का उपयोग करना चाहिए टेबल नाम के बजाय। यह सुनिश्चित करता है कि परिवर्तन क्लस्टर के सभी रेप्लिका में प्रसारित हो जाते हैं। यदि सही अभिव्यक्ति का उपयोग नहीं किया जाता है, तो एक त्रुटि ट्रिगर होगी।

JSON इंटरफ़ेस में, `cluster` गुण को `table` नाम के साथ सेट किया जाना चाहिए क्लस्टर की टेबल पर सभी राइट स्टेटमेंट के लिए। `cluster` गुण को सेट न करने पर एक त्रुटि होगी।

क्लस्टर में किसी टेबल के लिए [ऑटो ID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) तब तक वैध होगा जब तक [server_id](../../Server_settings/Searchd.md#server_id) सही ढंग से कॉन्फ़िगर किया गया हो।

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
INSERT INTO posts:weekly_index VALUES ( 'iphone case' )
TRUNCATE RTINDEX click_query:weekly_index
UPDATE INTO posts:rt_tags SET tags=(101, 302, 304) WHERE MATCH ('use') AND id IN (1,101,201)
DELETE FROM clicks:rt WHERE MATCH ('dumy') AND gid>206
```

<!-- request JSON -->

```json
POST /insert -d '
{
  "cluster":"posts",
  "table":"weekly_index",
  "doc":
  {
    "title" : "iphone case",
    "price" : 19.85
  }
}'
POST /delete -d '
{
  "cluster":"posts",
  "table": "weekly_index",
  "id":1
}'
```

<!-- request PHP -->

```php
$index->addDocuments([
        1, ['title' => 'iphone case', 'price' => 19.85]
]);
$index->deleteDocument(1);
```

<!-- intro -->
##### Python:

<!-- request Python -->

``` python
indexApi.insert({"cluster":"posts","table":"weekly_index","doc":{"title":"iphone case","price":19.85}})
indexApi.delete({"cluster":"posts","table":"weekly_index","id":1})
```
<!-- intro -->
##### Javascript:

<!-- request Javascript -->

``` javascript
res = await indexApi.insert({"cluster":"posts","table":"weekly_index","doc":{"title":"iphone case","price":19.85}});
 res = await indexApi.delete({"cluster":"posts","table":"weekly_index","id":1});
```

<!-- intro -->
##### java:

<!-- request Java -->

``` java
InsertDocumentRequest newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","Crossbody Bag with Tassel");
    put("price",19.85);
}};
newdoc.index("weekly_index").cluster("posts").id(1L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);

DeleteDocumentRequest deleteRequest = new DeleteDocumentRequest();
deleteRequest.index("weekly_index").cluster("posts").setId(1L);
indexApi.delete(deleteRequest);

```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "Crossbody Bag with Tassel");
doc.Add("price", 19.85);
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "weekly_index", cluster:posts, id: 1, doc: doc);
var sqlresult = indexApi.Insert(newdoc);

DeleteDocumentRequest deleteDocumentRequest = new DeleteDocumentRequest(index: "weekly_index", cluster: "posts", id: 1);
indexApi.Delete(deleteDocumentRequest);
```
<!-- end -->

## रीड स्टेटमेंट

<!-- example write statements 2 -->
रीड स्टेटमेंट जैसे `SELECT`, `CALL PQ`, `DESCRIBE` या तो नियमित टेबल नाम का उपयोग कर सकते हैं जो क्लस्टर नाम के साथ पूर्वलिखित नहीं हैं, या वे `cluster_name:table_name` प्रारूप का उपयोग कर सकते हैं। यदि बाद वाला उपयोग किया जाता है, तो `cluster_name` घटक को नज़रअंदाज कर दिया जाता है।

HTTP एंडपॉइंट `json/search` का उपयोग करते समय, `cluster` गुण को विनिर्दिष्ट किया जा सकता है यदि वांछित हो, लेकिन इसे छोड़ा भी जा सकता है।


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT * FROM weekly_index
CALL PQ('posts:weekly_index', 'document is here')
```

<!-- request JSON -->

```json
POST /search -d '
{
  "cluster":"posts",
  "table":"weekly_index",
  "query":{"match":{"title":"keyword"}}
}'
POST /search -d '
{
  "table":"weekly_index",
  "query":{"match":{"title":"keyword"}}
}'
```

<!-- end -->

## क्लस्टर पैरामीटर

<!-- example cluster parameters 1 -->
रेप्लिकेशन प्लगइन विकल्पों को `SET` स्टेटमेंट का उपयोग करके समायोजित किया जा सकता है।

उपलब्ध विकल्पों की सूची [गलेरा दस्तावेज़ पैरामीटर](https://galeracluster.com/library/documentation/galera-parameters.html) में पाई जा सकती है।


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SET CLUSTER click_query GLOBAL 'pc.bootstrap' = 1
```
<!-- request JSON -->

```json
POST /cli -d "
SET CLUSTER click_query GLOBAL 'pc.bootstrap' = 1
"
```
<!-- end -->

## अलग-अलग नोड्स वाला क्लस्टर

<!-- example cluster with diverged nodes  1 -->
यह संभव है कि प्रतिकृति नोड एक-दूसरे से अलग हो जाएं, जिससे एक ऐसी स्थिति बन जाए जहाँ सभी नोड को `non-primary` के रूप में लेबल किया जाता है। यह नोड्स के बीच नेटवर्क विभाजन, क्लस्टर दुर्घटना, या यदि रेप्लिकेशन प्लगइन `primary component` निर्धारित करने में कोई अपवाद का अनुभव करता है तो हो सकता है। ऐसी स्थिति में, किसी नोड को चुनने और उसे `primary component` की भूमिका में पदोन्नत करने की आवश्यकता होती है।

उस नोड की पहचान करने के लिए जिसे पदोन्नत किया जाना चाहिए, आपको सभी नोड्स पर `last_committed` क्लस्टर स्थिति चर के मान की तुलना करनी चाहिए। यदि सभी सर्वर वर्तमान में चल रहे हैं, तो क्लस्टर को पुनरारंभ करने की आवश्यकता नहीं है। इसके बजाय, आप बस `SET` स्टेटमेंट का उपयोग करके सबसे अधिक last_committed मान वाले नोड को `primary component` में पदोन्नत कर सकते हैं (जैसा कि उदाहरण में दर्शाया गया है)।

अन्य नोड्स फिर primary component से पुनः कनेक्ट होंगे और इस नोड के आधार पर अपना डेटा पुनः सिंक्रोनाइज़ करेंगे।


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SET CLUSTER posts GLOBAL 'pc.bootstrap' = 1
```
<!-- request JSON -->

```json
POST /cli -d "
सेट क्लस्टर पोस्ट्स ग्लोबल 'pc.bootstrap' = 1
"
```
<!-- end -->

## रिप्लिकेशन और क्लस्टर

<!-- example replication and cluster 1 -->
रिप्लिकेशन का उपयोग करने के लिए, आपको SphinxAPI प्रोटोकॉल के लिए एक [listen](../../Server_settings/Searchd.md#listen) पोर्ट और कॉन्फ़िगरेशन फ़ाइल में रिप्लिकेशन पते और पोर्ट रेंज के लिए एक [listen](../../Server_settings/Searchd.md#listen) पोर्ट को परिभाषित करने की आवश्यकता है। साथ ही, आने वाली तालिकाओं को प्राप्त करने के लिए [data_dir](../../Server_settings/Searchd.md#data_dir) फ़ोल्डर निर्दिष्ट करें।


<!-- intro -->
##### ini:

<!-- request ini -->
```ini
searchd {
  listen   = 9312
  listen   = 192.168.1.101:9360-9370:रिप्लिकेशन
  data_dir = /var/lib/manticore/
  ...
 }
```
<!-- end -->

<!-- example replication and cluster 2 -->
तालिकाओं का रिप्लिकेशन करने के लिए, आपको उस सर्वर पर एक क्लस्टर बनाना होगा जिसमें स्थानीय तालिकाएँ हों जिन्हें रिप्लिकेट किया जाना है।

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE CLUSTER posts
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE CLUSTER posts
"
```

<!-- request PHP -->

```php
$params = [
    'cluster' => 'posts'
    ]
];
$response = $client->cluster()->create($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE CLUSTER posts')
```
<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await utilsApi.sql('CREATE CLUSTER posts');
```

<!-- intro -->
##### Java:

<!-- request Java -->

```java
utilsApi.sql("CREATE CLUSTER posts");

```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE CLUSTER posts");

```
<!-- end -->

<!-- example replication and cluster 3 -->
इन स्थानीय तालिकाओं को क्लस्टर में जोड़ें


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
ALTER CLUSTER posts ADD pq_title
ALTER CLUSTER posts ADD pq_clicks
```

<!-- request JSON -->

```json
POST /cli -d "
ALTER CLUSTER posts ADD pq_title
"
POST /cli -d "
ALTER CLUSTER posts ADD pq_clicks
"
```

<!-- request PHP -->

```php
$params = [
  'cluster' => 'posts',
  'body' => [
     'operation' => 'add',
     'table' => 'pq_title'

  ]
];
$response = $client->cluster()->alter($params);
$params = [
  'cluster' => 'posts',
  'body' => [
     'operation' => 'add',
     'table' => 'pq_clicks'

  ]
];
$response = $client->cluster()->alter($params);   
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('ALTER CLUSTER posts ADD pq_title')
utilsApi.sql('ALTER CLUSTER posts ADD pq_clicks')
```
<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await utilsApi.sql('ALTER CLUSTER posts ADD pq_title');
res = await utilsApi.sql('ALTER CLUSTER posts ADD pq_clicks');
```

<!-- intro -->
##### Java:

<!-- request Java -->

```java
utilsApi.sql("ALTER CLUSTER posts ADD pq_title");
utilsApi.sql("ALTER CLUSTER posts ADD pq_clicks");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("ALTER CLUSTER posts ADD pq_title");
utilsApi.Sql("ALTER CLUSTER posts ADD pq_clicks");
```

<!-- end -->

<!-- example replication and cluster 4 -->
क्लस्टर की तालिकाओं की एक प्रति प्राप्त करने की इच्छा रखने वाले सभी अन्य नोड्स को निम्नलिखित तरीके से क्लस्टर में शामिल होना चाहिए:


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
JOIN CLUSTER posts AT '192.168.1.101:9312'
```

<!-- request JSON -->

```json
POST /cli -d "
JOIN CLUSTER posts AT '192.168.1.101:9312'
"
```

<!-- request PHP -->

```php
$params = [
  'cluster' => 'posts',
  'body' => [
      '192.168.1.101:9312'
  ]
];
$response = $client->cluster->join($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('JOIN CLUSTER posts AT \'192.168.1.101:9312\'')
```
<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await utilsApi.sql('JOIN CLUSTER posts AT \'192.168.1.101:9312\'');
```

<!-- intro -->
##### Java:

<!-- request Java -->

```java
utilsApi.sql("JOIN CLUSTER posts AT '192.168.1.101:9312'");

```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("JOIN CLUSTER posts AT '192.168.1.101:9312'");

```
<!-- end -->

<!-- example replication and cluster 5 -->
जब क्वेरी चलाते हैं, तो तालिका के नाम के साथ क्लस्टर का नाम `posts` जोड़ें: या HTTP अनुरोध ऑब्जेक्ट के लिए `cluster` प्रॉपर्टी का उपयोग करें।


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
INSERT INTO posts:pq_title VALUES ( 3, 'test me' )
```

<!-- request JSON -->

```json
POST /insert -d '
{
  "cluster":"posts",
  "table":"pq_title",
  "id": 3
  "doc":
  {
    "title" : "test me"
  }
}'
```

<!-- request PHP -->

```php
$index->addDocuments([
        3, ['title' => 'test me']
]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

``` python
indexApi.insert({"cluster":"posts","table":"pq_title","id":3"doc":{"title":"test me"}})

```
<!-- intro -->
##### Javascript:

<!-- request Javascript -->

``` javascript
res = await indexApi.insert({"cluster":"posts","table":"pq_title","id":3"doc":{"title":"test me"}});
```

<!-- intro -->
##### java:

<!-- request Java -->

``` java
InsertDocumentRequest newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","test me");
}};
newdoc.index("pq_title").cluster("posts").id(3L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);
```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "test me");
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "pq_title", cluster: "posts", id: 3, doc: doc);
var sqlresult = indexApi.Insert(newdoc);
```
<!-- end -->

क्लस्टर में तालिकाओं को संशोधित करने वाले सभी प्रश्न अब क्लस्टर के सभी नोड्स पर दोहराए जाते हैं।
<!-- proofread -->






















