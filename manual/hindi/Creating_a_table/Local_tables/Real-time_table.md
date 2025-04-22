# रियल-टाइम टेबल

A **रियल-टाइम टेबल** Manticore में एक मुख्य प्रकार की तालिका है। यह आपको दस्तावेज़ जोड़ने, अपडेट करने और हटाने की अनुमति देती है, और आप ये बदलाव तुरंत देख सकते हैं। आप एक रियल-टाइम टेबल को एक कॉन्फ़िगरेशन फ़ाइल में सेट कर सकते हैं या `CREATE`, `UPDATE`, `DELETE`, या `ALTER` जैसे आदेशों का उपयोग कर सकते हैं।

आंतरिक रूप से एक रियल-टाइम टेबल में एक या अधिक [साधारण तालिकाएँ](../../Creating_a_table/Local_tables/Plain_table.md) होती हैं जिन्हें **चंक** कहा जाता है। दो प्रकार के चंक होते हैं:

* कई **डिस्क चंक** - ये एक डिस्क पर संग्रहित होते हैं और [साधारण तालिका](../../Creating_a_table/Local_tables/Plain_table.md) की तरह संरचित होते हैं।
* एकल **राम चंक** - यह मेमोरी में रखा जाता है और सभी परिवर्तनों को एकत्र करता है।

RAM चंक के आकार को [rt_mem_limit](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit) सेटिंग द्वारा नियंत्रित किया जाता है। जब यह सीमा पहुँच जाती है, RAM चंक को डिस्क चंक के रूप में डिस्क में स्थानांतरित किया जाता है। यदि बहुत सारे डिस्क चंक होते हैं, तो Manticore [कुछ को एकीकृत करता है](../../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) ताकि प्रदर्शन में सुधार हो सके।

### रियल-टाइम टेबल बनाना:

आप दो तरीकों से एक नया रियल-टाइम टेबल बना सकते हैं: `CREATE TABLE` आदेश का उपयोग करके या HTTP JSON API के [_mapping endpoint](../../Creating_a_table/Local_tables/Real-time_table.md#_mapping-API:) के माध्यम से।

#### CREATE TABLE आदेश:

<!-- example rt -->

आप इस आदेश का उपयोग SQL और HTTP दोनों प्रोटोकॉल के माध्यम से कर सकते हैं:

<!-- intro -->
##### SQL प्रोटोकॉल के माध्यम से रियल-टाइम टेबल बनाना:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) morphology='stem_en';
```
<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- intro -->
##### HTTP के माध्यम से JSON के जरिए रियल-टाइम टेबल बनाना:
<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, price float)  morphology='stem_en'"
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
##### PHP क्लाइंट के माध्यम से:
<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
    'price'=>['type'=>'float'],
]);
```

<!-- intro -->
##### पायथन:
<!-- request Python -->
```python
utilsApi.sql('CREATE TABLE forum(title text, price float)')
```

<!-- intro -->
##### जावास्क्रिप्ट:

<!-- request Javascript -->
```javascript
res = await utilsApi.sql('CREATE TABLE forum(title text, price float)');
```

<!-- intro -->
##### जावा:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE forum(title text, price float)");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE forum(title text, price float)");
```

<!-- intro -->
##### टाइपस्क्रिप्ट:

<!-- request Typescript -->
```typescript
res = await utilsApi.sql('CREATE TABLE forum(title text, price float)');
```

<!-- intro -->
##### गो:

<!-- request Go -->
```go
utilsAPI.Sql(context.Background()).Body("CREATE TABLE forum(title text, price float)").Execute()
```


<!-- intro -->
##### एक कॉन्फ़िगरेशन फ़ाइल के माध्यम से रियल-टाइम टेबल बनाना:
<!-- request CONFIG -->

```ini
table products {
  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
  stored_fields = title
}
```
<!-- end -->

#### _mapping API:

> NOTE: `_mapping` API को [Manticore Buddy](../Installation/Manticore_Buddy.md) की आवश्यकता है। यदि यह काम नहीं करता है, तो सुनिश्चित करें कि Buddy स्थापित है।

<!-- example rt-mapping -->

वैकल्पिक रूप से, आप `_mapping` endpoint के माध्यम से एक नई तालिका बना सकते हैं। यह endpoint आपको एक Elasticsearch-जैसी तालिका संरचना को परिभाषित करने की अनुमति देता है जिसे Manticore तालिका में परिवर्तित किया जा सके।

आपके अनुरोध का शरीर निम्नलिखित संरचना का होना चाहिए:

```json
"properties"
{
  "FIELD_NAME_1":
  {
    "type": "FIELD_TYPE_1"
  },
  "FIELD_NAME_2":
  {
    "type": "FIELD_TYPE_2"
  },

  ...

  "FIELD_NAME_N":
  {
    "type": "FIELD_TYPE_M"
  }
}
```

जब एक तालिका बनाई जाती है, तो Elasticsearch डेटा प्रकारों को Manticore प्रकारों में निम्नलिखित नियमों के अनुसार मैप किया जाएगा:
-    aggregate_metric => json
-    binary => string
-    boolean => bool
-    byte => int
-    completion => string
-    date => timestamp
-    date_nanos => bigint
-    date_range => json
-    dense_vector => json
-    flattened => json
-    flat_object => json
-    float => float
-    float_range => json
-    geo_point => json
-    geo_shape => json
-    half_float => float
-    histogram => json
-    integer => int
-    integer_range => json
-    ip => string
-    ip_range => json
-    keyword => string
-    knn_vector => float_vector
-    long => bigint
-    long_range => json
-    match_only_text => text
-    object => json
-    point => json
-    scaled_float => float
-    search_as_you_type => text
-    shape => json
-    short => int
-    text => text
-    unsigned_long => int
-    version => string

<!-- intro -->
##### _mapping endpoint के माध्यम से रियल-टाइम टेबल बनाना:
<!-- request JSON -->

```JSON
POST /your_table_name/_mapping -d '
{
  "properties": {
    "price": {
        "type": "float"
    },
    "title": {
        "type": "text"
    }
  }
}
'
```

<!-- response JSON -->

```json
{
"total":0,
"error":"",
"warning":""
}
```
<!-- end -->


#### CREATE TABLE LIKE:

<!-- example create-like -->
आप एक वास्तविक-समय तालिका की एक प्रति बना सकते हैं, चाहे उसके डेटा के साथ हो या बिना। कृपया ध्यान दें कि यदि तालिका बड़ी है, तो डेटा के साथ इसकी नकल करने में थोड़ा समय लग सकता है। नकल समकालिक मोड में काम करती है, लेकिन यदि कनेक्शन टूटा, तो यह पृष्ठभूमि में जारी रखेगी।

```sql
CREATE TABLE [IF NOT EXISTS] table_name LIKE old_table_name [WITH DATA]
```

> नोट: तालिका की नकल करने के लिए [Manticore Buddy](../Installation/Manticore_Buddy.md) की आवश्यकता है। यदि यह काम नहीं करता है, तो सुनिश्चित करें कि Buddy स्थापित है।

<!-- intro -->
##### उदाहरण:
<!-- request SQL -->

```sql
create table products LIKE old_products;
```

<!-- intro -->
##### उदाहरण (डेटा के साथ):
<!-- request Example (WITH DATA) -->
```sql
create table products LIKE old_products WITH DATA;
```

<!-- end -->

### 👍 आप एक वास्तविक-समय तालिका के साथ क्या कर सकते हैं:
* [दस्तावेज़ जोड़ें](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md).
* [अपडेट](../../Quick_start_guide.md#Update) प्रक्रिया का उपयोग करके गुण और पूर्ण-पाठ फ़ील्ड अपडेट करें।
* [दस्तावेज़ हटाएँ](../../Quick_start_guide.md#Delete).
* [तालिका को खाली करें](../../Emptying_a_table.md).
* `ALTER` आदेश का उपयोग करके ऑनलाइन स्कीमा बदलें, जैसा कि [ऑनलाइन स्कीमा बदलें](../../Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode) में समझाया गया है।
* [तालिका को परिभाषित करें](../../Creating_a_table/Local_tables/Real-time_table.md) के अनुसार एक कॉन्फ़िगरेशन फ़ाइल में तालिका को परिभाषित करें।
* स्वचालित आईडी प्रावधान के लिए [UUID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real_time_table.md#Auto-ID) सुविधा का उपयोग करें।

### ⛔ आप एक वास्तविक-समय तालिका के साथ क्या नहीं कर सकते:
* [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) सुविधा का उपयोग करके डेटा खींचें।
* बाहरी भंडारण से आसान अनुक्रमण के लिए इसे [sources](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Execution_of_fetch_queries.md) से कनेक्ट करें।
* [killlist_target](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target) को अपडेट करें, क्योंकि इसे वास्तविक-समय तालिका द्वारा स्वचालित रूप से प्रबंधित किया जाता है।

#### वास्तविक-समय तालिका फ़ाइलों की संरचना

निम्नलिखित तालिका विभिन्न फ़ाइल एक्सटेंशन और उनके संबंधित विवरण को एक वास्तविक-समय तालिका में प्रस्तुत करती है:

| एक्सटेंशन | विवरण |
| - | - |
| `.lock` | एक लॉक फ़ाइल जो सुनिश्चित करती है कि केवल एक प्रक्रिया एक समय में तालिका तक पहुँच सकें। |
| `.ram` | तालिका का RAM टुकड़ा, जिसे मेमोरी में संग्रहीत किया जाता है और परिवर्तनों का संचयक के रूप में इस्तेमाल किया जाता है। |
| `.meta` | वास्तविक-समय तालिका के शीर्षक जो इसकी संरचना और सेटिंग्स को परिभाषित करते हैं। |
| `.*.sp*` | डिस्क टुकड़े जो डिस्क पर समान प्रारूप के साथ सहेजे जाते हैं जैसे कि साधारण तालिकाएँ। ये तब बनाए जाते हैं जब RAM टुकड़े का आकार rt_mem_limit से अधिक हो जाता है।|

 डिस्क टुकड़ों की संरचना के बारे में अधिक जानकारी के लिए, [साधारण तालिका फ़ाइलों की संरचना](../../Creating_a_table/Local_tables/Plain_table.md#Plain-table-files-structure) पर देखिए।
<!-- proofread -->

