# पर्कोलेट टेबल

<!-- example pq -->
एक पर्कोलेट टेबल एक विशेष टेबल होती है जो दस्तावेजों के बजाय क्वेरियों को संग्रहीत करती है। इसका उपयोग संभावित खोजों के लिए किया जाता है, या "उल्टी खोज।"

* पर्कोलेट टेबल के खिलाफ खोज क्वेरी करने के बारे में अधिक जानने के लिए, [पर्कोलेट क्वेरी](../../Searching/Percolate_query.md) अनुभाग देखें।
* खोजने के लिए टेबल कैसे तैयार करें, इसके बारे में जानने के लिए, [पर्कोलेट टेबल में नियम जोड़ना](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md) अनुभाग देखें।

एक पर्कोलेट टेबल की स्कीमा निश्चित होती है और इसमें निम्नलिखित फ़ील्ड होते हैं:

| फ़ील्ड | विवरण |
| - | - |
| ID| एकUnsigned 64-बिट पूर्णांक जो ऑटो-इंक्रीमेंट कार्यक्षमता के साथ होता है। इसे एक PQ नियम जोड़ते समय छोड़ दिया जा सकता है, जैसा कि [एक PQ नियम जोड़ें](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md) में वर्णित है |
| क्वेरी | नियम की [पूर्ण-पाठ क्वेरी](../../Searching/Full_text_matching/Basic_usage.md), जिसे [MATCH क्लॉज](../../Searching/Full_text_matching/Basic_usage.md) या [JSON /search](../../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) के मान के रूप में सोचा जा सकता है। यदि क्वेरी के अंदर [प्रत्येक फ़ील्ड ऑपरेटर](../../Searching/Full_text_matching/Operators.md) का उपयोग किया जाता है, तो पूर्ण-पाठ फ़ील्ड को पर्कोलेट टेबल कॉन्फ़िगरेशन में घोषित करना आवश्यक है। यदि संग्रहीत क्वेरी केवल विशेषता फ़िल्टरिंग (पूर्ण-पाठ क्वेरिंग के बिना) के लिए है, तो क्वेरी मान खाली या छोड़ा जा सकता है। इस फ़ील्ड का मान अपेक्षित दस्तावेज़ स्कीमा के अनुरूप होना चाहिए, जिसे पर्कोलेट टेबल बनाते समय निर्दिष्ट किया गया है। |
| फ़िल्टर | वैकल्पिक। फ़िल्टर एक वैकल्पिक स्ट्रिंग है जिसमें विशेषता फ़िल्टर और/या एक्सप्रेशन होते हैं, जिसे [WHERE क्लॉज](../../Searching/Filters.md#WHERE) या [JSON फ़िल्टरिंग](../../Searching/Filters.md#HTTP-JSON) में उसी तरह परिभाषित किया गया है। इस फ़ील्ड का मान अपेक्षित दस्तावेज़ स्कीमा के अनुरूप होना चाहिए, जिसे पर्कोलेट टेबल बनाते समय निर्दिष्ट किया गया है। |
| टैग | वैकल्पिक। टैग कॉमा से अलग की गई स्ट्रिंग लेबलों की सूची का प्रतिनिधित्व करते हैं, जिन्हें PQ नियमों को फ़िल्टरिंग/हटाने के लिए उपयोग किया जा सकता है। जब [पर्कोलेट क्वेरी](../../Searching/Percolate_query.md) करते समय मिलते-जुलते दस्तावेज़ लौटाए जाते हैं, तो टैग भी लौटाए जा सकते हैं। |

ध्यान दें कि पर्कोलेट टेबल बनाते समय आपको ऊपर के फ़ील्ड नहीं जोड़ने की आवश्यकता है।

जब आप एक नई पर्कोलेट टेबल बना रहे हैं, तो आपको यह ध्यान में रखना चाहिए कि एक दस्तावेज़ की अपेक्षित स्कीमा को निर्दिष्ट करें, जिसे बाद में आप जो नियम जोड़ेंगे उनके खिलाफ проверится। यह किसी अन्य स्थानीय टेबल के समान तरीके से किया जाता है। 


<!-- intro -->
##### SQL के माध्यम से पर्कोलेट टेबल बनाना:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, meta json) type='pq';
```
<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- intro -->
##### HTTP पर JSON के माध्यम से पर्कोलेट टेबल बनाना:

<!-- request JSON -->

```json
POST /cli -d "CREATE TABLE products(title text, meta json) type='pq'"
```

<!-- response JSON -->

```json
{
"total":0,
"error":"",
"warning":""
}
```

<!-- intro -->
##### PHP क्लाइंट के माध्यम से पर्कोलेट टेबल बनाना:

<!-- request PHP -->

```php
$index = [
    'table' => 'products',
    'body' => [
        'columns' => [
            'title' => ['type' => 'text'],
            'meta' => ['type' => 'json']
        ],
        'settings' => [
            'type' => 'pq'
        ]
    ]
];
$client->indices()->create($index);
```
<!-- response PHP -->
```php
Array(
    [total] => 0
    [error] =>
    [warning] =>
)
```

<!-- intro -->
##### पायथन:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, meta json) type='pq'')
```

<!-- intro -->
##### पायथन-संबंधी:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, meta json) type='pq'')
```

<!-- intro -->
##### जावास्क्रिप्ट:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, meta json) type='pq'');
```
<!-- intro -->
##### जावा:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, meta json) type='pq'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, meta json) type='pq'");
```

<!-- intro -->
##### रस्ट:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, meta json) type='pq'", Some(true)).await;
```

<!-- intro -->
##### टाइपस्क्रिप्ट:

<!-- request typescript -->

```typescript
res = await utilsApi.sql("CREATE TABLE products(title text, meta json) type='pq'");
```

<!-- intro -->
##### गो:

<!-- request go -->

```go
apiClient.UtilsAPI.Sql(context.Background()).Body("CREATE TABLE products(title text, meta json) type='pq'").Execute()
```

<!-- intro -->
##### कॉन्फ़िग के माध्यम से पर्कोलेट टेबल बनाना:

<!-- request CONFIG -->

```ini
table products {
  type = percolate
  path = tbl_pq
  rt_field = title
  rt_attr_json = meta
}
```
<!-- end -->
<!-- proofread -->
# पर्कोलेट टेबल

<!-- example pq -->
एक पर्कोलेट टेबल एक विशेष तालिका है जो दस्तावेजों के बजाय प्रश्नों को संग्रहीत करती है। इसका उपयोग संभावित खोजों के लिए, या "वापस खोजें।" के लिए किया जाता है।

* पर्कोलेट टेबल के खिलाफ खोज प्रश्न कैसे करें, इसके बारे में अधिक जानने के लिए, अनुभाग [पर्कोलेट प्रश्न](../../Searching/Percolate_query.md) देखें।
* खोज के लिए तालिका को तैयार करने के तरीके के बारे में जानने के लिए, अनुभाग [पर्कोलेट टेबल में नियम जोड़ना](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md) देखें।

एक पर्कोलेट टेबल का स्कीमा निश्चित है और इसमें निम्नलिखित फ़ील्ड होते हैं:

| फ़ील्ड | विवरण |
| - | - |
| ID| स्वचालित वृद्धि कार्यक्षमता के साथ एक बिना साइन वाला 64-बिट पूर्णांक। इसे PQ नियम जोड़ते समय छोड़ दिया जा सकता है, जैसा कि [PQ नियम जोड़ें](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md) में वर्णित है |
| प्रश्न | नियम का [पूर्ण-टेक्स्ट प्रश्न](../../Searching/Full_text_matching/Basic_usage.md), जिसे [MATCH क्लॉज](../../Searching/Full_text_matching/Basic_usage.md) या [JSON /search](../../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) के मान के रूप में सोचा जा सकता है। यदि प्रश्न के अंदर [प्रत्येक फ़ील्ड ऑपरेटर](../../Searching/Full_text_matching/Operators.md) का उपयोग किया जाता है, तो पूर्ण-टेक्स्ट फ़ील्ड को पर्कोलेट टेबल कॉन्फ़िगरेशन में घोषित किया जाना चाहिए। यदि संग्रहीत प्रश्न केवल गुण फ़िल्टरिंग के लिए है (पूर्ण-टेक्स्ट प्रश्न के बिना), तो प्रश्न का मान खाली या छोड़ा जा सकता है। इस फ़ील्ड का मान अपेक्षित दस्तावेज़ स्कीमा के अनुरूप होना चाहिए, जो पर्कोलेट टेबल बनाने के समय निर्दिष्ट किया गया है। |
| फ़िल्टर | वैकल्पिक। फ़िल्टर एक वैकल्पिक स्ट्रिंग है जिसमें गुण फ़िल्टर और/या अभिव्यक्तियाँ शामिल हैं, जिन्हें [WHERE क्लॉज](../../Searching/Filters.md#WHERE) या [JSON फ़िल्टरिंग](../../Searching/Filters.md#HTTP-JSON) में उसी तरह परिभाषित किया गया है। इस फ़ील्ड का मान अपेक्षित दस्तावेज़ स्कीमा के अनुरूप होना चाहिए, जो पर्कोलेट टेबल बनाने के समय निर्दिष्ट किया गया है। |
| टैग्स | वैकल्पिक। टैग्स एक स्ट्रिंग लेबल की सूची का प्रतिनिधित्व करते हैं जो अल्पविराम द्वारा अलग किए गए होते हैं और जिन्हें PQ नियमों को फ़िल्टर/हटाने के लिए उपयोग किया जा सकता है। टैग्स को [पर्कोलेट प्रश्न](../../Searching/Percolate_query.md) करते समय मेल खाने वाले दस्तावेजों के साथ भी लौटाया जा सकता है |

ध्यान दें कि आपको पर्कोलेट टेबल बनाते समय उपरोक्त फ़ील्ड जोड़ने की आवश्यकता नहीं है।

एक नया पर्कोलेट टेबल बनाते समय ध्यान में रखने वाली बात यह है कि एक दस्तावेज़ का अपेक्षित स्कीमा निर्दिष्ट करना है, जिसे आप बाद में जोड़ने वाले नियमों के खिलाफ जांचा जाएगा। यह [किसी अन्य स्थानीय तालिका](../../Creating_a_table/Local_tables.md) के लिए किए जाने के समान ही किया जाता है।


<!-- intro -->
##### SQL के माध्यम से पर्कोलेट टेबल बनाना:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, meta json) type='pq';
```
<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- intro -->
##### HTTP के माध्यम से JSON के माध्यम से पर्कोलेट टेबल बनाना:

<!-- request JSON -->

```json
POST /cli -d "CREATE TABLE products(title text, meta json) type='pq'"
```

<!-- response JSON -->

```json
{
"total":0,
"error":"",
"warning":""
}
```

<!-- intro -->
##### PHP क्लाइंट के माध्यम से पर्कोलेट टेबल बनाना:

<!-- request PHP -->

```php
$index = [
    'table' => 'products',
    'body' => [
        'columns' => [
            'title' => ['type' => 'text'],
            'meta' => ['type' => 'json']
        ],
        'settings' => [
            'type' => 'pq'
        ]
    ]
];
$client->indices()->create($index);
```
<!-- response PHP -->
```php
Array(
    [total] => 0
    [error] =>
    [warning] =>
)
```

<!-- intro -->
##### पाइथन:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, meta json) type=\'pq\'')
```
<!-- intro -->
##### जावास्क्रिप्ट:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, meta json) type=\'pq\'');
```
<!-- intro -->
##### जावा:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, meta json) type='pq'");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, meta json) type='pq'");
```

<!-- intro -->
##### टाइपस्क्रिप्ट:

<!-- request typescript -->

```typescript
res = await utilsApi.sql("CREATE TABLE products(title text, meta json) type='pq'");
```

<!-- intro -->
##### गो:

<!-- request go -->

```go
apiClient.UtilsAPI.Sql(context.Background()).Body("CREATE TABLE products(title text, meta json) type='pq'").Execute()
```

<!-- intro -->
##### कॉन्फ़िग के माध्यम से पर्कोलेट टेबल बनाना:

<!-- request CONFIG -->

```ini
table products {
  type = percolate
  path = tbl_pq
  rt_field = title
  rt_attr_json = meta
}
```
<!-- end -->
<!-- proofread -->
