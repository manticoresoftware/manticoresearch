# वास्तविक समय की तालिका में दस्तावेज़ जोड़ना

> यदि आप साधारण तालिका में दस्तावेज़ जोड़ने की जानकारी की तलाश कर रहे हैं, तो कृपया [बाहरी संग्रहों से डेटा जोड़ने](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md) के अनुभाग को देखें।

<!-- example insert -->
वास्तविक समय में दस्तावेज़ जोड़ने का समर्थन केवल [रियल-टाइम](../../Creating_a_table/Local_tables/Real-time_table.md) और [परकोलेट](../../Creating_a_table/Local_tables/Percolate_table.md) तालिकाओं के लिए किया जाता है। संबंधित SQL कमांड, HTTP एंडपॉइंट, या क्लाइंट फ़ंक्शन प्रदान की गई फ़ील्ड मानों के साथ तालिका में नए पंक्तियाँ (दस्तावेज़) डालते हैं। दस्तावेज़ जोड़ने से पहले तालिका का अस्तित्व आवश्यक नहीं है। यदि तालिका मौजूद नहीं है, तो Manticore स्वचालित रूप से इसे बनाने का प्रयास करेगा। अधिक जानकारी के लिए देखें [ऑटो स्कीमा](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real_time_table.md#Auto-schema)।

आप तालिका के लिए सभी फ़ील्ड या सिर्फ कुछ का मान संगृहीत करते हुए एकल या [कई दस्तावेज़](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real_time_table.md#Bulk-adding-documents) डाल सकते हैं। इस मामले में, अन्य फ़ील्ड उनके डिफ़ॉल्ट मानों (स्केलर प्रकारों के लिए 0, पाठ प्रकारों के लिए एक खाली स्ट्रिंग) से भरे जाएंगे।

वर्तमान में `INSERT` में अभिव्यक्तियों का समर्थन नहीं किया गया है, इसलिए मानों को स्पष्ट रूप से निर्दिष्ट किया जाना चाहिए।

आईडी फ़ील्ड/मान को छोड़ा जा सकता है, क्योंकि RT और PQ तालिकाएँ [ऑटो-आईडी](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real_time_table.md#Auto-ID) कार्यक्षमता का समर्थन करती हैं। आप स्वचालित आईडी जनरेशन को मजबूर करने के लिए आईडी मान के रूप में `0` का भी उपयोग कर सकते हैं। समान आईडी वाली पंक्तियों को `INSERT` द्वारा अधिलेखित नहीं किया जाएगा। इसके बजाय, इसके लिए आप [REPLACE](../../Data_creation_and_modification/Updating_documents/REPLACE.md) का उपयोग कर सकते हैं।

HTTP JSON प्रोटोकॉल का उपयोग करते समय, आपके पास चुनने के लिए दो विभिन्न अनुरोध प्रारूप हैं: एक सामान्य Manticore प्रारूप और एक Elasticsearch-समान प्रारूप। दोनों प्रारूपों को नीचे दिए गए उदाहरणों में प्रदर्शित किया गया है।

इसके अतिरिक्त, जब आप Manticore JSON अनुरोध प्रारूप का उपयोग कर रहे हैं, तो ध्यान रखें कि `doc` नोड आवश्यक है, और सभी मान इसे के भीतर प्रदान किए जाने चाहिए।


<!-- intro -->
##### SQL:
<!-- request SQL -->
सामान्य सिंटैक्स:

```sql
INSERT INTO <table name> [(column, ...)]
VALUES (value, ...)
[, (...)]
```

```sql
INSERT INTO products(title,price) VALUES ('Crossbody Bag with Tassel', 19.85);
INSERT INTO products(title) VALUES ('Crossbody Bag with Tassel');
INSERT INTO products VALUES (0,'Yellow bag', 4.95);
```
<!-- response SQL -->

```sql
Query OK, 1 rows affected (0.00 sec)
Query OK, 1 rows affected (0.00 sec)
Query OK, 1 rows affected (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```json
POST /insert
{
  "table":"products",
  "id":1,
  "doc":
  {
    "title" : "Crossbody Bag with Tassel",
    "price" : 19.85
  }
}

POST /insert
{
  "table":"products",
  "id":2,
  "doc":
  {
    "title" : "Crossbody Bag with Tassel"
  }
}

POST /insert
{
  "table":"products",
  "id":0,
  "doc":
  {
    "title" : "Yellow bag"
  }
}

```

<!-- response JSON -->

```json
{
  "table": "products",
  "_id": 1,
  "created": true,
  "result": "created",
  "status": 201
}
{
  "table": "products",
  "_id": 2,
  "created": true,
  "result": "created",
  "status": 201
}
{
  "table": "products",
  "_id": 1657860156022587406,
  "created": true,
  "result": "created",
  "status": 201
}

```

<!-- intro -->
##### Elasticsearch:

<!-- request Elasticsearch -->

> NOTE: `_create` requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

```json
POST /products/_create/3
{
  "title": "Yellow Bag with Tassel",
  "price": 19.85
}

POST /products/_create/
{
  "title": "Red Bag with Tassel",
  "price": 19.85
}

```
<!-- response Elasticsearch -->

```json
{
"_id":3,
"table":"products",
"_primary_term":1,
"_seq_no":0,
"_shards":{
    "failed":0,
    "successful":1,
    "total":1
},
"_type":"_doc",
"_version":1,
"result":"updated"
}
{
"_id":2235747273424240642,
"table":"products",
"_primary_term":1,
"_seq_no":0,
"_shards":{
    "failed":0,
    "successful":1,
    "total":1
},
"_type":"_doc",
"_version":1,
"result":"updated"
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$index->addDocuments([
        ['id' => 1, 'title' => 'Crossbody Bag with Tassel', 'price' => 19.85]
]);
$index->addDocuments([
        ['id' => 2, 'title' => 'Crossbody Bag with Tassel']
]);
$index->addDocuments([
        ['id' => 0, 'title' => 'Yellow bag']
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

``` python
indexApi.insert({"table" : "test", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}})
indexApi.insert({"table" : "test", "id" : 2, "doc" : {"title" : "Crossbody Bag with Tassel"}})
indexApi.insert({"table" : "test", "id" : 0, "doc" : {{"title" : "Yellow bag"}})
```
<!-- intro -->
##### Javascript:

<!-- request Javascript -->

``` javascript
res = await indexApi.insert({"table" : "test", "id" : 1, "doc" : {"title" : "Crossbody Bag with Tassel", "price" : 19.85}});
res = await indexApi.insert({"table" : "test", "id" : 2, "doc" : {"title" : "Crossbody Bag with Tassel"}});
res = await indexApi.insert({"table" : "test", "id" : 0, "doc" : {{"title" : "Yellow bag"}});
```

<!-- intro -->
##### java:

<!-- request Java -->

``` java
InsertDocumentRequest newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","क्रॉसबॉडी बैग विद टैसल");
    put("price",19.85);
}};
newdoc.index("products").id(1L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);

newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","क्रॉसबॉडी बैग विद टैसल");
}};
newdoc.index("products").id(2L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);

newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","पीला बैग");
 }};
newdoc.index("products").id(0L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);

```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "क्रॉसबॉडी बैग विद टैसल");
doc.Add("price", 19.85);
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", id: 1, doc: doc);
var sqlresult = indexApi.Insert(newdoc);

doc = new Dictionary<string, Object>();
doc.Add("title", "क्रॉसबॉडी बैग विद टैसल");
newdoc = new InsertDocumentRequest(index: "products", id: 2, doc: doc);
sqlresult = indexApi.Insert(newdoc);

doc = new Dictionary<string, Object>();
doc.Add("title", "पीला बैग");
newdoc = new InsertDocumentRequest(index: "products", id: 0, doc: doc);
sqlresult = indexApi.Insert(newdoc);

```

<!-- end -->

## ऑटो स्कीमा

> NOTE: ऑटो स्कीमा के लिए [Manticore Buddy](../Installation/Manticore_Buddy.md) की आवश्यकता होती है। यदि यह काम नहीं करता है, तो सुनिश्चित करें कि बडी इंस्टॉल है।

Manticore में एक स्वचालित तालिका निर्माण तंत्र है, जो तब सक्रिय होता है जब अनु插 में निर्दिष्ट तालिका अभी तक मौजूद नहीं है। यह तंत्र डिफ़ॉल्ट रूप से सक्षम है। इसे बंद करने के लिए, अपने Manticore कॉन्फ़िगरेशन फ़ाइल के [Searchd](../../Server_settings/Searchd.md#auto_schema) अनुभाग में `auto_schema = 0` सेट करें।

<!-- example auto-schema -->

डिफ़ॉल्ट रूप से, `VALUES` खंड में सभी टेक्स्ट मानों को `text` प्रकार का माना जाता है, सिवाय उन मानों के जो वैध ईमेल पते का प्रतिनिधित्व करते हैं, जिन्हें `string` प्रकार के रूप में माना जाता है।

यदि आप एक ही फ़ील्ड के लिए विभिन्न, असंगत मान प्रकारों के साथ कई पंक्तियों को INSERT करने का प्रयास करते हैं, तो स्वचालित तालिका निर्माण रद्द कर दिया जाएगा, और एक त्रुटि संदेश लौटाया जाएगा। हालाँकि, यदि विभिन्न मान प्रकार संगत हैं, तो परिणामी फ़ील्ड प्रकार वह होगा जो सभी मानों को समायोजित करता है। स्वचालित डेटा प्रकार रूपांतरणों में जो हो सकते हैं, उनमें शामिल हैं:
* mva -> mva64
* uint -> bigint -> float (यह कुछ सटीकता हानि का कारण बन सकता है)
* string -> text

इसके अलावा, निम्नलिखित तिथियों के रूप को पहचाना जाएगा और टाइमस्टैम्प में परिवर्तित किया जाएगा जबकि बाकी सभी तारीखों के प्रारूपों को स्ट्रिंग के रूप में माना जाएगा:
- `%Y-%m-%dT%H:%M:%E*S%Z`
- `%Y-%m-%d'T'%H:%M:%S%Z`
- `%Y-%m-%dT%H:%M:%E*S`
- `%Y-%m-%dT%H:%M:%s`
- `%Y-%m-%dT%H:%M`
- `%Y-%m-%dT%H`


ध्यान रखें कि `/bulk` HTTP एंडपॉइंट स्वचालित तालिका निर्माण (ऑटो स्कीमा) का समर्थन नहीं करता है। केवल `/_bulk` (Elasticsearch-like) HTTP एंडपॉइंट और SQL इंटरफ़ेस इस सुविधा का समर्थन करते हैं।

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
MySQL [(none)]> drop table if exists t; insert into t(i,f,t,s,j,b,m,mb) values(123,1.2,'text here','test@mail.com','{"a": 123}',1099511627776,(1,2),(1099511627776,1099511627777)); desc t; select * from t;
```

<!-- response SQL -->

```sql
--------------
drop table if exists t
--------------

Query OK, 0 rows affected (0.42 sec)

--------------
insert into t(i,f,t,j,b,m,mb) values(123,1.2,'text here','{"a": 123}',1099511627776,(1,2),(1099511627776,1099511627777))
--------------

Query OK, 1 row affected (0.00 sec)

--------------
desc t
--------------

+-------+--------+----------------+
| Field | Type   | Properties     |
+-------+--------+----------------+
| id    | bigint |                |
| t     | text   | indexed stored |
| s     | string |                |
| j     | json   |                |
| i     | uint   |                |
| b     | bigint |                |
| f     | float  |                |
| m     | mva    |                |
| mb    | mva64  |                |
+-------+--------+----------------+
8 rows in set (0.00 sec)

--------------
select * from t
--------------

+---------------------+------+---------------+----------+------+-----------------------------+-----------+---------------+------------+
| id                  | i    | b             | f        | m    | mb                          | t         | s             | j          |
+---------------------+------+---------------+----------+------+-----------------------------+-----------+---------------+------------+
| 5045949922868723723 |  123 | 1099511627776 | 1.200000 | 1,2  | 1099511627776,1099511627777 | text here | test@mail.com | {"a": 123} |
+---------------------+------+---------------+----------+------+-----------------------------+-----------+---------------+------------+
1 row in set (0.00 sec)
```

<!-- request JSON -->

```json
POST /insert  -d
{
 "table":"t",
 "id": 2,
 "doc":
 {
   "i" : 123,
   "f" : 1.23,
   "t": "text here",
   "s": "test@mail.com",
   "j": {"a": 123},
   "b": 1099511627776,
   "m": [1,2],
   "mb": [1099511627776,1099511627777]
 }
}
```

<!-- response JSON -->

```json
{"table":"t","_id":2,"created":true,"result":"created","status":201}
```

<!-- end -->

## ऑटो आईडी
<!-- example autoid -->
Manticore वास्तविक समय या [Percolate तालिका](../../Creating_a_table/Local_tables/Percolate_table.md) में डाले गए या प्रतिस्थापित किए गए दस्तावेज़ों के स्तंभ ID के लिए एक ऑटो ID उत्पादन कार्यक्षमता प्रदान करता है। जनरेटर कुछ गारंटियों के साथ दस्तावेज़ के लिए एक अद्वितीय ID उत्पन्न करता है, लेकिन इसे एक ऑटो-इन्क्रिमेंटेड ID के रूप में नहीं माना जाना चाहिए।
उत्पन्न ID मान निम्नलिखित शर्तों के तहत अद्वितीय होने की garantee है:
* वर्तमान सर्वर का [server_id](../../Server_settings/Searchd.md#server_id) मान 0 से 127 की सीमा में है और क्लस्टर में नोड्स के बीच अद्वितीय है, या यह MAC पते से उत्पन्न डिफ़ॉल्ट मान का उपयोग करता है
* सर्वर पुनः आरंभ के बीच Manticore नोड के लिए प्रणाली समय नहीं बदलता है
* ऑटो ID प्रति सेकंड सर्च सर्वर पुनः आरंभ के बीच 16 मिलियन से कम बार उत्पन्न होता है

ऑटो ID जनरेटर एक दस्तावेज़ ID के लिए 64-बिट पूर्णांक उत्पन्न करता है और निम्नलिखित स्कीमा का उपयोग करता है:
* बिट 0 से 23 एक काउंटर बनाते हैं जो ऑटो ID जनरेटर के हर कॉल पर बढ़ता है
* बिट 24 से 55 सर्वर स्टार्ट का यूनिक्स टाइमस्टैम्प दर्शाते हैं
* बिट 56 से 63 server_id से मेल खाते हैं

यह स्कीमा यह सुनिश्चित करता है कि उत्पन्न ID क्लस्टर में सभी नोड्स के बीच अद्वितीय है और अलग-अलग क्लस्टर नोड्स में डाटा डालने से नोड्स के बीच टकराव उत्पन्न नहीं होता है।

परिणामस्वरूप, ऑटो ID के लिए जनरेटर से प्राप्त ID 1 नहीं है बल्कि एक बड़ा नंबर है। इसके अलावा, एक टेबल में डाला गया दस्तावेज़ धारा में गैर-क्रमबद्ध ID मान हो सकते हैं अगर कॉल के बीच अन्य तालिकाओं में इंसर्ट होते हैं, क्योंकि ID जनरेटर सर्वर में एकल है और सभी तालिकाओं के बीच साझा है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
INSERT INTO products(title,price) VALUES ('Crossbody Bag with Tassel', 19.85);
INSERT INTO products VALUES (0,'Yello bag', 4.95);
select * from products;
```
<!-- response SQL -->
```
+---------------------+-----------+---------------------------+
| id                  | price     | title                     |
+---------------------+-----------+---------------------------+
| 1657860156022587404 | 19.850000 | Crossbody Bag with Tassel |
| 1657860156022587405 |  4.950000 | Yello bag                 |
+---------------------+-----------+---------------------------+
```
<!-- intro -->
##### JSON:
<!-- request JSON -->

```json
POST /insert
{
  "table":"products",
  "id":0,
  "doc":
  {
    "title" : "Yellow bag"
  }
}

GET /search
{
  "table":"products",
  "query":{
    "query_string":""
  }
}
```
<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "_id": 1657860156022587406,
        "_score": 1,
        "_source": {
          "price": 0,
          "title": "Yellow bag"
        }
      }
    ]
  }
}
```
<!-- intro -->
##### PHP:
<!-- request PHP -->

```php
$index->addDocuments([
        ['id' => 0, 'title' => 'Yellow bag']
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "Yellow bag"}})
```
<!-- intro -->

##### Javascript:

<!-- request Javascript -->

```javascript
res = await indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "Yellow bag"}});
```

<!-- intro -->
##### java:

<!-- request Java -->

``` java
newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","Yellow bag");
 }};
newdoc.index("products").id(0L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);
```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "Yellow bag");
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", id: 0, doc: doc);
var sqlresult = indexApi.Insert(newdoc);
```
<!-- end -->

<!-- example call -->
### UUID_SHORT multi-ID generation

```sql
CALL UUID_SHORT(N)
```

`CALL UUID_SHORT(N)` कथन एक ही कॉल में N अद्वितीय 64-बिट IDs उत्पन्न करने की अनुमति देता है बिना किसी दस्तावेज़ को डाले। यह विशेष रूप से तब उपयोगी है जब आपको अन्य प्रणालियों या स्टोरेज सॉल्यूशंस में उपयोग के लिए Manticore में IDs पहले से उत्पन्न करने की आवश्यकता हो। उदाहरण के लिए, आप Manticore में ऑटो-IDs उत्पन्न कर सकते हैं और फिर उन्हें दूसरे डेटा बेस, एप्लिकेशन, या वर्कफ़्लो में उपयोग कर सकते हैं, यह सुनिश्चित करते हुए कि विभिन्न वातावरणों में सुसंगत और अद्वितीय पहचानकर्ता हों।

<!-- intro -->
##### उदाहरण:
<!-- request Example -->

```sql
CALL UUID_SHORT(3)
```
<!-- response SQL -->
```
+---------------------+
| uuid_short()        |
+---------------------+
| 1227930988733973183 |
| 1227930988733973184 |
| 1227930988733973185 |
+---------------------+
```
<!-- end -->

<!-- example bulk_insert -->
## Bulk adding documents
आप केवल एक दस्तावेज़ को रियल-टाइम तालिका में डालने के बजाय, जितने चाहें उतने डाल सकते हैं। यह पूरी तरह से ठीक है कि आप रियल-टाइम तालिका में दसियों हजार दस्तावेज़ों के बैच डालें। हालाँकि, निम्नलिखित बिंदुओं को ध्यान में रखना महत्वपूर्ण है:
* बैच जितना बड़ा होगा, प्रत्येक इंसर्ट ऑपरेशन की लेटेंसी उतनी ही अधिक होगी
* बैच जितना बड़ा होगा, आप अपेक्षित अनुक्रमण गति उतनी ही अधिक होगी
* आप [max_packet_size](../../Server_settings/Searchd.md#max_packet_size) मान को बढ़ाने पर विचार कर सकते हैं ताकि बड़े बैचों की अनुमति मिल सके
* सामान्यतः, प्रत्येक बैच इंसर्ट ऑपरेशन को एकल [लेनदेन](../../Data_creation_and_modification/Transactions.md) माना जाता है जिसमें परमाणुता की गारंटी होती है, इसलिए आपके पास तालिका में एक बार में सभी नए दस्तावेज़ होंगे या, यदि विफलता होती है, तो उनमें से कोई भी नहीं जोड़ा जाएगा। "JSON" उदाहरण में एक खाली पंक्ति या किसी अन्य तालिका में स्विच करने के बारे में अधिक विवरण देखें।

ध्यान दें कि `/bulk` HTTP एंडपॉइंट स्वचालित रूप से तालिकाओं का निर्माण (ऑटो स्कीमा) का समर्थन नहीं करता है। केवल `/_bulk` (Elasticsearch जैसा) HTTP एंडपॉइंट और SQL इंटरफ़ेस इस सुविधा का समर्थन करते हैं। `/_bulk` (Elasticsearch जैसा) HTTP एंडपॉइंट में तालिका के नाम में क्लस्टर का नाम शामिल किया जा सकता है `cluster_name:table_name` प्रारूप में।
`/_bulk` समाप्ति बिंदु दस्तावेज़ आईडी को Elasticsearch के समान प्रारूप में स्वीकार करता है, और आप दस्तावेज़ के भीतर `id` भी शामिल कर सकते हैं:
```json
{ "index": { "table" : "products", "_id" : "1" } }
{ "title" : "क्रॉसबॉडी बैग विद टासल", "price": 19.85 }
```

या

```json
{ "index": { "table" : "products" } }
{ "title" : "क्रॉसबॉडी बैग विद टासल", "price": 19.85, "id": "1" }
```

#### /bulk में चंकड ट्रांसफर
`/bulk` (Manticore मोड) समाप्ति बिंदु [चंकड ट्रांसफर एन्कोडिंग](https://en.wikipedia.org/wiki/Chunked_transfer_encoding) का समर्थन करता है। आप इसका उपयोग बड़े बैचों को प्रेषित करने के लिए कर सकते हैं। यह:
* पीक RAM उपयोग को कम करता है, OOM के जोखिम को कम करता है
* प्रतिक्रिया समय को कम करता है
* आपको [max_packet_size](../Server_settings/Searchd.md#max_packet_size) को बायपास करने की अनुमति देता है और `max_packet_size` (128MB) की अधिकतम अनुमति दी गई मान से कहीं अधिक बड़े बैचों को स्थानांतरित करता है, उदाहरण के लिए, एक बार में 1GB।

<!-- intro -->
### बल्क इनसर्ट के उदाहरण
##### SQL:
<!-- request SQL -->
बल्क इनसर्ट के लिए, बस `VALUES()` के बाद ब्रैकेट में अधिक दस्तावेज़ प्रदान करें। वाक्य संरचना यह है:

```sql
INSERT INTO <table name>[(column1, column2, ...)] VALUES(value1[, value2 , ...]), (...)
```

वैकल्पिक कॉलम नाम सूची आपको तालिका में उपस्थित कुछ कॉलम के लिए मूल्यों को स्पष्ट रूप से निर्दिष्ट करने की अनुमति देती है। सभी अन्य कॉलम को उनके डिफ़ॉल्ट मूल्यों से भरा जाएगा (स्केलर प्रकारों के लिए 0, स्ट्रिंग प्रकारों के लिए रिक्त स्ट्रिंग)।

उदाहरण के लिए:

```sql
INSERT INTO products(title,price) VALUES ('क्रॉसबॉडी बैग विद टासल', 19.85), ('माइक्रोफाइबर शीट सेट', 19.99), ('पेट हेयर रिमूवर ग्लव', 7.99);
```
<!-- response SQL -->

```sql
Query OK, 3 rows affected (0.01 sec)
```

व्यक्तियों का वर्तमान में `INSERT` में समर्थन नहीं किया गया है, और मूल्यों को स्पष्ट रूप से निर्दिष्ट किया जाना चाहिए।

<!-- intro -->
##### JSON:
<!-- request JSON -->
वाक्य संरचना आम तौर पर [एकल दस्तावेज़ डालने](../../Quick_start_guide.md#Add-documents) के लिए समान होती है। बस अधिक पंक्तियाँ प्रदान करें, प्रत्येक दस्तावेज़ के लिए एक, और `/insert` के बजाय `/bulk` समाप्ति बिंदु का उपयोग करें। प्रत्येक दस्तावेज़ को "insert" नोड में लपेटें। ध्यान दें कि इसे भी आवश्यकता है:
* `Content-Type: application/x-ndjson`
* डेटा को नई-लाइन से अलग किए गए JSON (NDJSON) के रूप में स्वरूपित किया जाना चाहिए। मूलतः, इसका मतलब है कि प्रत्येक पंक्ति में बिल्कुल एक JSON घोषणा होनी चाहिए और `\n` और संभवतः `\r` के साथ समाप्त होना चाहिए।

`/bulk` समाप्ति बिंदु 'इनसर्ट', 'रिप्लेस', 'डिलीट', और 'अपडेट' क्वेरी का समर्थन करता है। ध्यान रखें कि आप ऑपरेशनों को कई तालिकाओं की ओर निर्देशित कर सकते हैं, लेकिन लेनदेन केवल एकल तालिका के लिए संभव हैं। यदि आप अधिक निर्दिष्ट करते हैं, तो Manticore एक तालिका की ओर निर्देशित ऑपरेशनों को एक एकल लेनदेन में इकट्ठा करेगा। जब तालिका बदलती है, तो यह एकत्रित ऑपरेशनों को कमिट करेगा और नई तालिका पर एक नया लेनदेन शुरू करेगा। बैचों को अलग करने वाली एक रिक्त पंक्ति भी पिछले बैच को कमिट करने और एक नया लेनदेन शुरू करने की ओर ले जाती है।

`/bulk` अनुरोध के लिए प्रतिक्रिया में, आप निम्नलिखित फ़ील्ड पा सकते हैं:
* "errors": दिखाता है कि क्या कोई त्रुटियाँ हुईं (सत्य/असत्य)
* "error": उस त्रुटि का विवरण देता है जो हुई
* "current_line": वह पंक्ति संख्या जहां निष्पादन रुका (या असफल); रिक्त पंक्तियाँ, पहले रिक्त पंक्तियों सहित, भी गिनी जाती हैं
* "skipped_lines": गैर-ट्रांज़ेक्टेड पंक्तियों की गिनती, `current_line` से शुरू करके और पीछे की ओर चलते हुए

```json
POST /bulk
-H "Content-Type: application/x-ndjson" -d '
{"insert": {"table":"products", "id":1, "doc":  {"title":"क्रॉसबॉडी बैग विद टासल","price" : 19.85}}}
{"insert":{"table":"products", "id":2, "doc":  {"title":"माइक्रोफाइबर शीट सेट","price" : 19.99}}}
'

POST /bulk
-H "Content-Type: application/x-ndjson" -d '
{"insert":{"table":"test1","id":21,"doc":{"int_col":1,"price":1.1,"title":"बल्क डॉक एक"}}}
{"insert":{"table":"test1","id":22,"doc":{"int_col":2,"price":2.2,"title":"बल्क डॉक दो"}}}

{"insert":{"table":"test1","id":23,"doc":{"int_col":3,"price":3.3,"title":"बल्क डॉक तीन"}}}
{"insert":{"table":"test2","id":24,"doc":{"int_col":4,"price":4.4,"title":"बल्क डॉक चार"}}}
{"insert":{"table":"test2","id":25,"doc":{"int_col":5,"price":5.5,"title":"बल्क डॉक पांच"}}}
'
```

<!-- response JSON -->
```json
{
  "items": [
    {
      "bulk": {
        "table": "products",
        "_id": 2,
        "created": 2,
        "deleted": 0,
        "updated": 0,
        "result": "created",
        "status": 201
      }
    }
  ],
  "current_line": 4,
  "skipped_lines": 0,
  "errors": false,
  "error": ""
}

{
  "items": [
    {
      "bulk": {
        "table": "test1",
        "_id": 22,
        "created": 2,
        "deleted": 0,
        "updated": 0,
        "result": "created",
        "status": 201
      }
    },
    {
      "bulk": {
        "table": "test1",
        "_id": 23,
        "created": 1,
        "deleted": 0,
        "updated": 0,
        "result": "created",
        "status": 201
      }
    },
    {
      "bulk": {
        "table": "test2",
        "_id": 25,
        "created": 2,
        "deleted": 0,
        "updated": 0,
        "result": "created",
        "status": 201
      }
    }
  ],
  "current_line": 8,
  "skipped_lines": 0,
  "errors": false,
  "error": ""
}
```

<!-- request Elasticsearch -->

> NOTE: `_bulk` के लिए [Manticore Buddy](../Installation/Manticore_Buddy.md) की आवश्यकता होती है यदि तालिका अभी तक मौजूद नहीं है। यदि यह काम नहीं करता है, तो सुनिश्चित करें कि बडी इंस्टॉल है।

```json
POST /_bulk
-H "Content-Type: application/x-ndjson" -d '
{ "index" : { "table" : "products" } }
{ "title" : "पीला बैग", "price": 12 }
{ "create" : { "table" : "products" } }
{ "title" : "लाल बैग", "price": 12.5, "id": 3 }
'
```
<!-- response Elasticsearch -->
```json
{
  "items": [
    {
      "table": {
        "table": "products",
        "_type": "doc",
        "_id": 1657860156022587406,
        "_version": 1,
        "result": "created",
        "_shards": {
          "total": 1,
          "successful": 1,
          "failed": 0
        },
        "_seq_no": 0,
        "_primary_term": 1,
        "status": 201
      }
    },
    {
      "create": {
        "table": "products",
        "_type": "doc",
        "_id": 3,
        "_version": 1,
        "result": "created",
        "_shards": {
          "total": 1,
          "successful": 1,
          "failed": 0
        },
        "_seq_no": 0,
        "_primary_term": 1,
        "status": 201
      }
    }
  ],
  "errors": false,
  "took": 1
}
```

<!-- intro -->
##### PHP:
<!-- request PHP -->
Use method addDocuments():

```php
$index->addDocuments([
        ['id' => 1, 'title' => 'क्रॉसबॉडी बैग विथ टास्सल', 'price' => 19.85],
        ['id' => 2, 'title' => 'माइक्रोफाइबर शीट सेट', 'price' => 19.99],
        ['id' => 3, 'title' => 'पेट हेयर रिमूवर ग्लव', 'price' => 7.99]
]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
docs = [ \
    {"insert": {"table" : "products", "id" : 1, "doc" : {"title" : "क्रॉसबॉडी बैग विथ टास्सल", "price" : 19.85}}}, \
    {"insert": {"table" : "products", "id" : 2, "doc" : {"title" : "माइक्रोफाइबर शीट सेट", "price" : 19.99}}}, \
    {"insert": {"table" : "products", "id" : 3, "doc" : {"title" : "पेट हेयर रिमूवर ग्लव", "price" : 7.99}}}
]
res = indexApi.bulk('\n'.join(map(json.dumps,docs)))
```


<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
let docs = [
    {"insert": {"table" : "products", "id" : 3, "doc" : {"title" : "क्रॉसबॉडी बैग विथ टास्सल", "price" : 19.85}}},
    {"insert": {"table" : "products", "id" : 4, "doc" : {"title" : "माइक्रोफाइबर शीट सेट", "price" : 19.99}}},
    {"insert": {"table" : "products", "id" : 5, "doc" : {"title" : "पेट हेयर रिमूवर ग्लव", "price" : 7.99}}}
];
res =  await indexApi.bulk(docs.map(e=>JSON.stringify(e)).join('\n'));
```
<!-- intro -->
##### java:

<!-- request Java -->

``` java
String body = "{\"insert\": {\"index\" : \"products\", \"id\" : 1, \"doc\" : {\"title\" : \"क्रॉसबॉडी बैग विथ टास्सल\", \"price\" : 19.85}}}"+"\n"+
    "{\"insert\": {\"index\" : \"products\", \"id\" : 4, \"doc\" : {\"title\" : \"माइक्रोफाइबर शीट सेट\", \"price\" : 19.99}}}"+"\n"+
    "{\"insert\": {\"index\" : \"products\", \"id\" : 5, \"doc\" : {\"title\" : \"पेट हेयर रिमूवर ग्लव\", \"price\" : 7.99}}}"+"\n";
BulkResponse bulkresult = indexApi.bulk(body);
```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
string body = "{\"insert\": {\"index\" : \"products\", \"id\" : 1, \"doc\" : {\"title\" : \"क्रॉसबॉडी बैग विथ टास्सल\", \"price\" : 19.85}}}"+"\n"+
    "{\"insert\": {\"index\" : \"products\", \"id\" : 4, \"doc\" : {\"title\" : \"माइक्रोफाइबर शीट सेट\", \"price\" : 19.99}}}"+"\n"+
    "{\"insert\": {\"index\" : \"products\", \"id\" : 5, \"doc\" : {\"title\" : \"पेट हेयर रिमूवर ग्लव\", \"price\" : 7.99}}}"+"\n";
BulkResponse bulkresult = indexApi.Bulk(string.Join("\n", docs));
```

<!-- end -->
<!-- example MVA_insert -->
## मल्टी-वैल्यु एट्रिब्यूट्स (MVA) मान डालना

मल्टी-वैल्यु एट्रिब्यूट्स (MVA) को संख्याओं के ऐरे के रूप में डाला जाता है।
<!-- intro -->
### उदाहरण
##### SQL
<!-- request SQL -->
```

INSERT INTO products(title, sizes) VALUES('जूते', (40,41,42,43));
```
<!-- intro -->
##### JSON
<!-- request JSON -->
```json

POST /insert
{
  "table":"products",
  "id":1,
  "doc":
  {
    "title" : "जूते",
    "sizes" : [40, 41, 42, 43]
  }
}
```

<!-- intro -->
##### Elasticsearch:
<!-- request Elasticsearch -->

```json
POST /products/_create/1
{
  "title": "जूते",
  "sizes" : [40, 41, 42, 43]
}
```

या, वैकल्पिक रूप से
```json
POST /products/_doc/
{
  "title": "जूते",
  "sizes" : [40, 41, 42, 43]
}
```


<!-- intro -->
##### PHP
<!-- request PHP -->

```php
$index->addDocument(
  ['title' => 'जूते', 'sizes' => [40,41,42,43]],
  1
);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "पीला बैग","sizes":[40,41,42,43]}})
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->

```javascript
res = await indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "पीला बैग","sizes":[40,41,42,43]}});
```


<!-- intro -->
##### java:

<!-- request Java -->

``` java
newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","पीला बैग");
    put("sizes",new int[]{40,41,42,43});
 }};
newdoc.index("products").id(0L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);
```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "पीला बैग");
doc.Add("sizes", new List<Object> {40,41,42,43});
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", id: 0, doc: doc);
var sqlresult = indexApi.Insert(newdoc);
```
<!-- end -->

<!-- example JSON_insert -->
## JSON डालना
JSON मान को [escaped](../../Searching/Full_text_matching/Escaping.md) स्ट्रिंग (SQL या JSON के माध्यम से) या JSON ऑब्जेक्ट (JSON इंटरफ़ेस के माध्यम से) के रूप में डाला जा सकता है।

<!-- intro -->
### उदाहरण
##### SQL
<!-- request SQL -->
```

INSERT INTO products VALUES (1, 'जूते', '{"size": 41, "color": "लाल"}');
```
<!-- intro -->
##### JSON
<!-- request JSON -->
JSON मान को JSON ऑब्जेक्ट के रूप में डाला जा सकता है
```json
POST /insert
{
  "table":"products",
  "id":1,
  "doc":
  {
    "title" : "जूते",
    "meta" : {
      "size": 41,
      "color": "लाल"
    }
  }
}
```

JSON मान को भी एक स्ट्रिंग के रूप में डाला जा सकता है जिसमें एस्केप्ड JSON शामिल है:
```json
POST /insert
{
  "table":"products",
  "id":1,
  "doc":
  {
    "title" : "जूते",
    "meta" : "{\"size\": 41, \"color\": \"लाल\"}"
  }
}
```

<!-- intro -->
##### Elasticsearch:
<!-- request Elasticsearch -->

```json
POST /products/_create/1
{
  "title": "जूते",
  "meta" : {
    "size": 41,
    "color": "लाल"
  }
}
```

या, वैकल्पिक रूप से
```json
POST /products/_doc/
{
  "title": "जूते",
  "meta" : {
    "size": 41,
    "color": "लाल"
  }
}
```


<!-- intro -->
##### PHP
JSON को बस एक स्ट्रिंग के रूप में मानें:
<!-- request PHP -->

```php
$index->addDocument(
  ['title' => 'जूते', 'meta' => '{"size": 41, "color": "लाल"}'],
  1
);
```
<!-- intro -->
##### Python:

<!-- request Python -->
``` python
indexApi = api = manticoresearch.IndexApi(client)
indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "पीला बैग","meta":'{"size": 41, "color": "लाल"}'}})
```
<!-- intro -->
##### Javascript:

<!-- request Javascript -->
```javascript

res = await indexApi.insert({"table" : "products", "id" : 0, "doc" : {"title" : "पीला बैग","meta":'{"size": 41, "color": "लाल"}'}});
```
<!-- intro -->
##### java:

<!-- request Java -->

``` java
newdoc = new InsertDocumentRequest();
HashMap<String,Object> doc = new HashMap<String,Object>(){{
    put("title","पीला बैग");
    put("meta",
        new HashMap<String,Object>(){{
            put("size",41);
            put("color","लाल");
        }});
 }};
newdoc.index("products").id(0L).setDoc(doc);
sqlresult = indexApi.insert(newdoc);
```

<!-- intro -->
##### C#:

<!-- request C# -->

``` clike
Dictionary<string, Object> meta = new Dictionary<string, Object>();
meta.Add("size", 41);
meta.Add("color", "लाल");
Dictionary<string, Object> doc = new Dictionary<string, Object>();
doc.Add("title", "पीला बैग");
doc.Add("meta", meta);
InsertDocumentRequest newdoc = new InsertDocumentRequest(index: "products", id: 0, doc: doc);
var sqlresult = indexApi.Insert(newdoc);
```

<!-- end -->

<!-- proofread -->


