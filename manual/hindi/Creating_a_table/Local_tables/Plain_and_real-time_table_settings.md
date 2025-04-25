# सामान्य और वास्तविक समय तालिका सेटिंग्स

<!-- example config -->
## एक कॉन्फ़िगरेशन फ़ाइल में तालिका स्कीमा को परिभाषित करना

```ini
table <table_name>[:<parent table name>] {
...
}
```

<!-- intro -->
##### एक कॉन्फ़िगरेशन फ़ाइल में एक सामान्य तालिका का उदाहरण
<!-- request Plain -->

```ini
table <table name> {
  type = plain
  path = /path/to/table
  source = <source_name>
  source = <another source_name>
  [stored_fields = <comma separated list of full-text fields that should be stored, all are stored by default, can be empty>]
}
```
<!-- intro -->
##### एक कॉन्फ़िगरेशन फ़ाइल में वास्तविक समय तालिका का उदाहरण
<!-- request Real-time -->

```ini
table <table name> {
  type = rt
  path = /path/to/table

  rt_field = <full-text field name>
  rt_field = <another full-text field name>
  [rt_attr_uint = <integer field name>]
  [rt_attr_uint = <another integer field name, limit by N bits>:N]
  [rt_attr_bigint = <bigint field name>]
  [rt_attr_bigint = <another bigint field name>]
  [rt_attr_multi = <multi-integer (MVA) field name>]
  [rt_attr_multi = <another multi-integer (MVA) field name>]
  [rt_attr_multi_64 = <multi-bigint (MVA) field name>]
  [rt_attr_multi_64 = <another multi-bigint (MVA) field name>]
  [rt_attr_float = <float field name>]
  [rt_attr_float = <another float field name>]
  [rt_attr_float_vector = <float vector field name>]
  [rt_attr_float_vector = <another float vector field name>]
  [rt_attr_bool = <boolean field name>]
  [rt_attr_bool = <another boolean field name>]
  [rt_attr_string = <string field name>]
  [rt_attr_string = <another string field name>]
  [rt_attr_json = <json field name>]
  [rt_attr_json = <another json field name>]
  [rt_attr_timestamp = <timestamp field name>]
  [rt_attr_timestamp = <another timestamp field name>]

  [stored_fields = <comma separated list of full-text fields that should be stored, all are stored by default, can be empty>]

  [rt_mem_limit = <RAM chunk max size, default 128M>]
  [optimize_cutoff = <max number of RT table disk chunks>]

}
```
<!-- end -->

### सामान्य सामान्य और वास्तविक समय तालिकाओं की सेटिंग्स

#### प्रकार

```ini
type = plain

type = rt
```

तालिका प्रकार: "सामान्य" या "rt" (वास्तविक समय)

मूल्य: **सामान्य** (डिफ़ॉल्ट), rt

#### पथ

```ini
path = path/to/table
```

जहाँ तालिका संग्रहीत या स्थित होगी, वहाँ का पथ, या तो पूर्ण या सापेक्ष, बिना एक्सटेंशन के।

मूल्य: तालिका का पथ, **अनिवार्य**

#### संग्रहीत_क्षेत्र

```ini
stored_fields = title, content
```

<!-- example stored_fields -->

डिफ़ॉल्ट रूप से, पूर्ण-टेक्स्ट फ़ील्ड की मूल सामग्री को एक कॉन्फ़िगरेशन फ़ाइल में तालिका परिभाषित करते समय अनुक्रमित और संग्रहीत किया जाता है। यह सेटिंग आपको उन फ़ील्ड को निर्दिष्ट करने की अनुमति देती है जिनमें उनके मूल मान संग्रहीत होने चाहिए।

मूल्य: **पूर्ण-टेक्स्ट** फ़ील्डों की एक अल्पविराम से पृथक सूची जिन्हें संग्रहीत किया जाना चाहिए। एक खाली मान (जैसे `stored_fields =` ) सभी फ़ील्ड के लिए मूल मानों के संग्रह को अक्षम कर देता है।

नोट: एक वास्तविक समय तालिका के मामले में, `stored_fields` में सूचीबद्ध फ़ील्ड को [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field) के रूप में भी घोषित किया जाना चाहिए।

इसके अलावा, ध्यान दें कि आपको `stored_fields` में गुणों को सूचीबद्ध करने की आवश्यकता नहीं है, क्योंकि उनके मूल मान वैसे भी संग्रहीत होते हैं। `stored_fields` केवल पूर्ण-टेक्स्ट फ़ील्ड के लिए उपयोग किया जा सकता है।

कागजात संग्रहण संकुचन विकल्पों के लिए [docstore_block_size](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_block_size), [docstore_compression](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_compression) देखें।


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)"
```

<!-- request PHP -->

```php
$params = [
    'body' => [
        'columns' => [
            'title'=>['type'=>'text'],
            'content'=>['type'=>'text', 'options' => ['indexed', 'stored']],
            'name'=>['type'=>'text', 'options' => ['indexed']],
            'price'=>['type'=>'float']
        ]
    ],
    'table' => 'products'
];
$index = new \Manticoresearch\Index($client);
$index->create($params);
```
<!-- intro -->
##### पायथन:
<!-- request Python -->
```python
utilsApi.sql('CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)')
```

<!-- intro -->
##### पायथन-ऐसिंक:
<!-- request Python-asyncio -->
```python
await utilsApi.sql('CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)')
```

<!-- intro -->
##### जावास्क्रिप्ट:

<!-- request Javascript -->
```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)');
```

<!-- intro -->
##### जावा:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)");
```

<!-- intro -->
##### सी#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)");
```

<!-- intro -->
##### रस्ट:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)", Some(true)).await;
```

<!-- intro -->
##### टाइपस्क्रिप्ट:

<!-- request Typescript -->
```typescript
res = await utilsApi.sql('CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)');
```

<!-- intro -->
##### गो:

<!-- request Go -->
```go
utilsAPI.Sql(context.Background()).Body("CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)").Execute()
```

<!-- request CONFIG -->

```ini
table products {
  stored_fields = title, content # we want to store only "title" and "content", "name" shouldn't be stored

  type = rt
  path = tbl
  rt_field = title
  rt_field = content
  rt_field = name
  rt_attr_uint = price
}
```
<!-- end -->

#### केवल_संग्रहीत_क्षेत्र

```ini
stored_only_fields = title,content
```

जब आप चाहते हैं कि मंटिकोर किसी सामान्य या वास्तविक समय तालिका के कुछ फ़ील्ड **डिस्क पर संग्रहीत करे लेकिन उन्हें अनुक्रमित न करे**, तब `stored_only_fields` का उपयोग करें। ये फ़ील्ड पूर्ण-टेक्स्ट प्रश्नों के साथ खोज करने योग्य नहीं होंगे, लेकिन आप अभी भी खोज परिणामों में उनके मानों को प्राप्त कर सकते हैं।

उदाहरण के लिए, यदि आप JSON दस्तावेज़ जैसे डेटा को संग्रहीत करना चाहते हैं, जो प्रत्येक परिणाम के साथ लौटाया जाना चाहिए, लेकिन जिसकी आवश्यकता नहीं है खोजा जाए, छांटा जाए, या समूहित किया जाए। उस मामले में, केवल इन्हें संग्रहीत करना - और उन्हें अनुक्रमित नहीं करना - मेमोरी बचाता है और प्रदर्शन में सुधार करता है।

आप इसे दो तरीकों से कर सकते हैं:
- एक तालिका कॉन्फ़िगरेशन में [सामान्य मोड](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29) में, `stored_only_fields` सेटिंग का उपयोग करें।
- SQL इंटरफेस में ([RT मोड](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)), एक टेक्स्ट फ़ील्ड को परिभाषित करते समय [stored](../../Creating_a_table/Data_types.md#Storing-binary-data-in-Manticore) प्रॉपर्टी का उपयोग करें (इसके बजाय `indexed` या `indexed stored`)। SQL में, आपको `stored_only_fields` शामिल करने की आवश्यकता नहीं है - इसे `CREATE TABLE` बयानों में समर्थित नहीं किया गया है।

`stored_only_fields` का मान फ़ील्ड नामों की एक अल्पविराम से पृथक सूची है। डिफ़ॉल्ट रूप से, यह खाली है। यदि आप एक वास्तविक समय तालिका का उपयोग कर रहे हैं, तो `stored_only_fields` में सूचीबद्ध प्रत्येक फ़ील्ड को भी [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field) के रूप में घोषित किया जाना चाहिए।

नोट: आपको `stored_only_fields` में गुणों को सूचीबद्ध करने की आवश्यकता नहीं है, क्योंकि उनके मूल मान वैसे भी संग्रहीत होते हैं।

सिर्फ-संग्रहीत क्षेत्रों की तुलना स्ट्रिंग गुणों से:

- **केवल-संग्रहीत क्षेत्र**:
  - केवल डिस्क पर संग्रहीत
  - संकुचित फॉर्मेट
  - केवल पुनः प्राप्त किया जा सकता है (फिल्टरिंग, छँटाई आदि के लिए उपयोग नहीं किया जा सकता)

- **स्ट्रिंग गुण**:
  - डिस्क और मेमोरी दोनों पर संग्रहीत
  - असंकुचित फॉर्मेट (जब तक आप कॉलम संग्रह का उपयोग नहीं कर रहे हैं)
  - छँटाई, फ़िल्टरिंग, समूह बनाने आदि के लिए उपयोग किया जा सकता है।

यदि आप चाहते हैं कि मंटिकोर आपके लिए टेक्स्ट डेटा संग्रहीत करे जिसे आप _केवल_ डिस्क पर संग्रहीत करना चाहते हैं (जैसे: JSON डेटा जो हर परिणाम के साथ लौटाया जाता है), और न कि मेमोरी में या खोज योग्य/फ़िल्टर करने योग्य/समूहित करने योग्य, तो `stored_only_fields` का उपयोग करें, या `stored` को अपने टेक्स्ट फ़ील्ड प्रॉपर्टी के रूप में उपयोग करें।

जब आप SQL इंटरफेस का उपयोग करके अपनी तालिकाएँ बना रहे हैं, तो अपने टेक्स्ट फ़ील्ड को `stored` के रूप में लेबल करें (और `indexed` या `indexed stored` के रूप में नहीं)। आपको अपने `CREATE TABLE` बयान में `stored_only_fields` विकल्प की आवश्यकता नहीं होगी; इसे शामिल करने से एक असफल क्वेरी हो सकती है।

#### json_secondary_indexes

```ini
json_secondary_indexes = json_attr
```

<!-- example json_secondary_indexes -->

डिफ़ॉल्ट रूप से, सभी गुणों के लिए द्वितीयक अनुक्रमांक उत्पन्न होते हैं सिवाय JSON गुणों के। हालाँकि, JSON गुणों के लिए द्वितीयक अनुक्रमांक स्पष्ट रूप से `json_secondary_indexes` सेटिंग का उपयोग करके उत्पन्न किया जा सकता है। जब किसी JSON गुण को इस विकल्प में शामिल किया जाता है, तो इसकी सामग्री को कई द्वितीयक अनुक्रमांक में समतल किया जाता है। इन अनुक्रमकों का उपयोग क्वेरी ऑप्टिमाइज़र द्वारा क्वेरी को गति देने के लिए किया जा सकता है।

आप उपलब्ध द्वितीयक अनुक्रमांक को [SHOW TABLE <tbl> INDEXES](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_INDEXES.md) कमांड का उपयोग करके देख सकते हैं।

मूल्य: JSON गुणों की एक अल्पविराम से पृथक सूची जिसके लिए द्वितीयक अनुक्रमांक उत्पन्न किए जाने चाहिए।

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, j json secondary_index='1')
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, j json secondary_index='1')"
```

<!-- request PHP -->

```php
$params = [
    'body' => [
        'columns' => [
            'title'=>['type'=>'text'],
            'j'=>['type'=>'json', 'options' => ['secondary_index' => 1]]
        ]
    ],
    'table' => 'products'
];
$index = new \Manticoresearch\Index($client);
$index->create($params);
```
<!-- intro -->
##### पायथन:
<!-- request Python -->
```python
utilsApi.sql('CREATE TABLE products(title text, j json secondary_index='1')')
```

<!-- intro -->
##### पायथन-ऐसिंक:
<!-- request Python-asyncio -->
```python
await utilsApi.sql('CREATE TABLE products(title text, j json secondary_index='1')')
```

<!-- intro -->
##### जावास्क्रिप्ट:

<!-- request Javascript -->
```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, j json secondary_index=\'1\')');
```

<!-- परिचय -->
##### जावा:
<!-- अनुरोध जावा -->
```java
utilsApi.sql("CREATE TABLE products(title text, j json secondary_index='1')");
```

<!-- परिचय -->
##### C#:
<!-- अनुरोध C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, j json secondary_index='1')");
```

<!-- परिचय -->
##### रस्ट:

<!-- अनुरोध रस्ट -->

```rust
utils_api.sql("CREATE TABLE products(title text, j json secondary_index='1')", Some(true)).await;
```

<!-- परिचय -->
##### टाइपस्क्रिप्ट:

<!-- अनुरोध टाइपस्क्रिप्ट -->
```typescript
res = await utilsApi.sql('CREATE TABLE products(title text, j json secondary_index=\'1\')');
```

<!-- परिचय -->
##### गो:

<!-- अनुरोध गो -->
```go
utilsAPI.Sql(context.Background()).Body("CREATE TABLE products(title text, j json secondary_index='1')").Execute()
```


<!-- अनुरोध CONFIG -->

```ini
table products {
  json_secondary_indexes = j

  type = rt
  path = tbl
  rt_field = title
  rt_attr_json = j
}
```
<!-- अंत -->

### वास्तविक समय तालिका सेटिंग्स:

#### diskchunk_flush_search_timeout

```ini
diskchunk_flush_search_timeout = 10s
```

तालिका में कोई खोज न होने पर RAM चंक को स्वचालित रूप से फ्लश करने से रोकने का समय सीमा। अधिक जानकारी [यहां](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout) देखें।

#### diskchunk_flush_write_timeout

```ini
diskchunk_flush_write_timeout = 60s
```

यदि इसमें कोई लेखन नहीं है तो RAM चंक को स्वचालित रूप से फ्लश करने का समय सीमा। अधिक जानकारी [यहां](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout) देखें।

#### optimize_cutoff

RT तालिका के लिए डिस्क चंक की अधिकतम संख्या। अधिक जानकारी [यहां](../../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) देखें।

#### rt_field

```ini
rt_field = subject
```

यह फील्ड घोषणा निर्धारित करती है कि किन पूर्ण-पाठ फील्ड को अनुक्रमित किया जाएगा। फील्ड के नाम अद्वितीय होने चाहिए, और क्रम बरकरार रखा जाता है। डेटा डालते समय, फील्ड मान कॉन्फ़िगरेशन में निर्दिष्ट क्रम के समान होने चाहिए।

यह एक बहु-मान, वैकल्पिक फील्ड है।

#### rt_attr_uint

```ini
rt_attr_uint = gid
```

यह घोषणा एक असाइन किए गए पूर्णांक विशेषता को परिभाषित करती है।

मान: फील्ड नाम या field_name:N (जहां N रखने के लिए अधिकतम बिट्स की संख्या है)।

#### rt_attr_bigint

```ini
rt_attr_bigint = gid
```

यह घोषणा एक BIGINT विशेषता को परिभाषित करती है।

मान: फील्ड नाम, कई रिकॉर्ड अनुमत।

#### rt_attr_multi

```ini
rt_attr_multi = tags
```

असाइन किए गए 32-बिट पूर्णांक मानों के साथ बहु-मान विशेषता (MVA) घोषित करता है।

मान: फील्ड नाम। कई रिकॉर्ड अनुमत।

#### rt_attr_multi_64

```ini
rt_attr_multi_64 = wide_tags
```

हस्ताक्षरित 64-बिट BIGINT मानों के साथ बहु-मान विशेषता (MVA) घोषित करता है।

मान: फील्ड नाम। कई रिकॉर्ड अनुमत।

#### rt_attr_float

```ini
rt_attr_float = lat
rt_attr_float = lon
```

32-बिट IEEE 754 प्रारूप में एकल सटीकता के साथ फ्लोटिंग पॉइंट विशेषताएं घोषित करता है।

मान: फील्ड नाम। कई रिकॉर्ड अनुमत।

#### rt_attr_float_vector

```ini
rt_attr_float_vector = image_vector
```

फ्लोटिंग-पॉइंट मानों का एक सदिश घोषित करता है।

मान: फील्ड नाम। कई रिकॉर्ड अनुमत।

#### rt_attr_bool

```ini
rt_attr_bool = available
```

1-बिट असाइन किए गए पूर्णांक मानों के साथ बूलियन विशेषता घोषित करता है।

मान: फील्ड नाम।

#### rt_attr_string

```ini
rt_attr_string = title
```

स्ट्रिंग विशेषता घोषणा।

मान: फील्ड नाम।

#### rt_attr_json

```ini
rt_attr_json = properties
```

JSON विशेषता घोषित करता है।

मान: फील्ड नाम।

#### rt_attr_timestamp

```ini
rt_attr_timestamp = date_added
```

टाइमस्टैंप विशेषता घोषित करता है।

मान: फील्ड नाम।

#### rt_mem_limit

```ini
rt_mem_limit = 512M
```

तालिका के RAM चंक के लिए मेमोरी सीमा। वैकल्पिक, डिफ़ॉल्ट 128M है।

RT तालिकाएं कुछ डेटा मेमोरी में संग्रहीत करती हैं, जिसे "RAM चंक" के रूप में जाना जाता है, और साथ ही कई डिस्क तालिकाओं को बनाए रखती हैं, जिन्हें "डिस्क चंक" के रूप में संदर्भित किया जाता है। यह निर्देश RAM चंक के आकार को नियंत्रित करने की अनुमति देता है। जब मेमोरी में रखने के लिए बहुत अधिक डेटा होता है, तो RT तालिकाएं इसे डिस्क पर फ्लश करेंगी, एक नया डिस्क चंक सक्रिय करेंगी और RAM चंक को रीसेट करेंगी।

कृपया ध्यान दें कि सीमा सख्त है, और RT तालिकाएं rt_mem_limit में निर्दिष्ट मेमोरी से अधिक कभी भी आवंटित नहीं करेंगी। इसके अलावा, मेमोरी पहले से आवंटित नहीं की जाती है, इसलिए 512MB की सीमा निर्दिष्ट करने और केवल 3MB डेटा डालने पर 512MB नहीं, बल्कि केवल 3MB आवंटित होगा।

`rt_mem_limit` कभी भी पार नहीं किया जाता, लेकिन वास्तविक RAM चंक का आकार सीमा से काफी कम हो सकता है। RT तालिकाएं डेटा सम्मिलन की गति के अनुकूल होती हैं और मेमोरी उपयोग को कम करने और डेटा लेखन गति को अधिकतम करने के लिए वास्तविक सीमा को गतिशील रूप से समायोजित करती हैं। यह कैसे काम करता है:
* डिफ़ॉल्ट रूप से, RAM चंक का आकार `rt_mem_limit` का 50% होता है, जिसे "`rt_mem_limit`" के रूप में संदर्भित किया जाता है।
* जैसे ही RAM चंक `rt_mem_limit * दर` डेटा (डिफ़ॉल्ट रूप से `rt_mem_limit` का 50%) संचित करता है, मंटिकोर एक नए डिस्क चंक के रूप में RAM चंक को बचाना शुरू कर देता है।
* जबकि एक नया डिस्क चंक सहेजा जा रहा है, मंटिकोर नए/अपडेट किए गए दस्तावेजों की संख्या का मूल्यांकन करता है।
* एक नया डिस्क चंक सहेजने के बाद, `rt_mem_limit` दर अपडेट की जाती है।
* दर searchd को पुनः शुरू करने पर हर बार 50% पर रीसेट कर दिया जाता है।

उदाहरण के लिए, यदि 90MB डेटा एक डिस्क चंक में सहेजा जाता है और सेव के दौरान 10MB अतिरिक्त डेटा आता है, तो दर 90% होगी। अगली बार, RT तालिका डेटा को `rt_mem_limit` के 90% तक एकत्र करेगी। सम्मिलन गति जितनी तेज होगी, `rt_mem_limit` दर उतनी ही कम होगी। दर 33.3% से 95% के बीच बदलती रहती है। आप [SHOW TABLE <tbl> STATUS](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) कमांड का उपयोग करके किसी तालिका की वर्तमान दर देख सकते हैं।

##### rt_mem_limit और optimize_cutoff कैसे बदलें

वास्तविक समय मोड में, आप `ALTER TABLE` कथन का उपयोग करके RAM चंक के आकार की सीमा और डिस्क चंक की अधिकतम संख्या को समायोजित कर सकते हैं। "t" तालिका के लिए `rt_mem_limit` को 1 गीगाबाइट पर सेट करने के लिए, निम्न क्वेरी चलाएं: `ALTER TABLE t rt_mem_limit='1G'`। डिस्क चंक की अधिकतम संख्या बदलने के लिए, क्वेरी चलाएं: `ALTER TABLE t optimize_cutoff='5'`।

सादे मोड में, आप तालिका कॉन्फ़िगरेशन अपडेट करके या `ALTER TABLE <table_name> RECONFIGURE` कमांड चलाकर `rt_mem_limit` और `optimize_cutoff` के मान बदल सकते हैं।

##### RAM चंक के बारे में महत्वपूर्ण नोट्स

* वास्तविक समय तालिकाएं [वितरित](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table) के समान होती हैं जो कई स्थानीय तालिकाओं से बनी होती हैं, जिन्हें डिस्क चंक भी कहा जाता है।
* प्रत्येक RAM चंक कई खंडों से बना होता है, जो विशेष RAM-ओनली तालिकाएं होती हैं।
* जबकि डिस्क चंक डिस्क पर संग्रहीत होते हैं, RAM चंक मेमोरी में संग्रहीत होते हैं।
* वास्तविक समय तालिका में किए गए प्रत्येक लेनदेन से एक नया खंड उत्पन्न होता है, और RAM चंक खंड प्रत्येक लेनदेन कमिट के बाद मर्ज किए जाते हैं। RAM चंक खंड विलय से होने वाले ओवरहेड को कम करने के लिए एक दस्तावेज के साथ कई अलग-अलग INSERTs के बजाह सैकड़ों या हजारों दस्तावेजों के साथ बल्क INSERTs करना अधिक कुशल है।
* जब खंडों की संख्या 32 से अधिक हो जाती है, तो उन्हें 32 से नीचे रखने के लिए मर्ज किया जाएगा।
* वास्तविक समय की तालिकाओं में हमेशा एक RAM चंक होता है (जो खाली भी हो सकता है) और एक या अधिक डिस्क चंक होते हैं।
* बड़े सेगमेंटों को मर्ज करने में अधिक समय लगता है, इसलिए बहुत बड़े RAM चंक (और इसलिए `rt_mem_limit`) से बचना सबसे अच्छा है।
* डिस्क चंक की संख्या तालिका में डेटा और `rt_mem_limit` सेटिंग पर निर्भर करती है।
* सर्च्ड RAM चंक को डिस्क पर फ्लश करता है (एक संग्रहीत फ़ाइल के रूप में, डिस्क चंक के रूप में नहीं) शटडाउन पर और समय-समय पर [rt_flush_period](../../Server_settings/Searchd.md#rt_flush_period) सेटिंग के अनुसार। कई गीगाबाइट्स को डिस्क पर फ्लश करने में कुछ समय लग सकता है।
* एक बड़ा RAM चंक स्टोरेज पर अधिक दबाव डालता है, जब फ्लश करते समय डिस्क पर `.ram` फ़ाइल में और जब RAM चंक भर जाता है और डिस्क चंक के रूप में डिस्क पर डंप होता है।
* RAM चंक को [बाइनरी लॉग](../../Logging/Binary_logging.md) द्वारा बैकअप किया जाता है जब तक कि इसे डिस्क पर फ्लश नहीं किया जाता है, और बड़े `rt_mem_limit` सेटिंग के साथ बाइनरी लॉग को पुनः चलाने और RAM चंक को पुनर्प्राप्त करने में लगने वाला समय बढ़ जाएगा।
* RAM चंक एक डिस्क चंक की तुलना में थोड़ा धीमा हो सकता है।
* हालांकि RAM चंक स्वयं `rt_mem_limit` से अधिक मेमोरी का उपयोग नहीं करता है, Manticore कुछ मामलों में अधिक मेमोरी ले सकता है, जैसे जब आप डेटा डालने के लिए लेनदेन शुरू करते हैं और इसे कुछ समय के लिए प्रतिबद्ध नहीं करते हैं। इस मामले में, लेनदेन के भीतर आपने जो डेटा ट्रांसमिट किया है, वह मेमोरी में बना रहेगा।

`rt_mem_limit` के अलावा, RAM चंक के फ्लशिंग व्यवहार को निम्नलिखित सेटिंग्स द्वारा भी प्रभावित किया जाता है: [diskchunk_flush_write_timeout](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout) और [diskchunk_flush_search_timeout](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout)。

* `diskchunk_flush_write_timeout`: यह विकल्प RAM चंक के स्वतः फ्लशिंग के लिए समय सीमा को परिभाषित करता है यदि इसके लिए कोई लिखाई नहीं होती है। यदि इस समय के भीतर कोई लेखन नहीं होता है, तो चंक को डिस्क पर फ्लश किया जाएगा। इसे `-1` पर सेट करने से लेखन गतिविधि के आधार पर स्वतः फ्लशिंग अक्षम हो जाती है। डिफ़ॉल्ट मान 1 सेकंड है।
* `diskchunk_flush_search_timeout`: यह विकल्प डिस्क चंक को स्वतः फ्लशिंग से रोकने के लिए समय सीमा निर्धारित करता है यदि तालिका में कोई खोज नहीं होती है। स्वतः फ्लशिंग केवल तब होगी जब इस समय में कम से कम एक खोज हुई हो। डिफ़ॉल्ट मान 30 सेकंड है।

ये समय सीमा एक साथ काम करती हैं। यदि *किसी भी* समय सीमा को पार किया गया है तो RAM चंक को फ्लश किया जाएगा। यह सुनिश्चित करता है कि यदि कोई लेखन नहीं होते हैं, तो डेटा अंततः डिस्क पर संग्रहीत किया जाएगा, और इसके विपरीत, यदि निरंतर लेखन होते हैं लेकिन कोई खोज नहीं होती है, तो डेटा भी संग्रहीत होगा। ये सेटिंग्स RAM चंक के फ्लशिंग को कितनी बार नियंत्रित करती हैं, इसके लिए अधिक तरकीब प्रदान करती हैं, डेटा की स्थिरता की आवश्यकता और प्रदर्शन की विचारणाओं के बीच संतुलन बनाती हैं। इन सेटिंग्स के लिए प्रति-तालिका निर्देश उच्च प्राथमिकता रखते हैं और उदाहरण-व्यापी डिफ़ॉल्ट्स को ओवरराइड करेंगे।

### साधारण तालिका सेटिंग्स:

#### स्रोत

```ini
source = srcpart1
source = srcpart2
source = srcpart3
```

स्रोत फ़ील्ड उस स्रोत को निर्दिष्ट करता है जिससे दस्तावेज़ वर्तमान तालिका के अनुक्रमण के दौरान प्राप्त किए जाएंगे। कम से कम एक स्रोत होना चाहिए। स्रोत विभिन्न प्रकार के हो सकते हैं (जैसे, एक MySQL हो सकता है, दूसरा PostgreSQL)। बाहरी स्टोरेज से अनुक्रमण के बारे में अधिक जानकारी के लिए, [बाहरी स्टोरेज से अनुक्रमण यहाँ](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)

मूल्य: स्रोत का नाम **अनिवार्य** है। एक से अधिक मानों की अनुमति है।

#### killlist_target

```ini
killlist_target = main:kl
```

यह सेटिंग उन तालिकाओं को निर्धारित करती है जिन पर किल-लिस्ट लागू की जाएगी। लक्षित तालिका में जो मिलान वर्तमान तालिका में अपडेट या हटाए गए हैं, उन्हें दबा दिया जाएगा। `:kl` मोड में, दबाने के लिए दस्तावेज़ [किल-लिस्ट](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md) से लिए जाते हैं। `:id` मोड में, वर्तमान तालिका से सभी दस्तावेज़ आईडी लक्षित तालिका में दबा दिए जाते हैं। यदि कोई भी निर्दिष्ट नहीं किया गया है, तो दोनों मोड प्रभावी होंगे। [किल-लिस्ट के बारे में यहाँ अधिक जानें](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)

मूल्य: **निर्दिष्ट नहीं** (डिफ़ॉल्ट), target_table_name:kl, target_table_name:id, target_table_name। एक से अधिक मानों की अनुमति है।

#### columnar_attrs

```ini
columnar_attrs = *
columnar_attrs = id, attr1, attr2, attr3
```

यह कॉन्फ़िगरेशन सेटिंग निर्धारित करती है कि कौन से गुण [कॉलम स्टोरेज](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) में संग्रहित किए जाने चाहिए बजाय कि पंक्ता-वायज़ स्टोरेज में।

आप सभी समर्थित डेटा प्रकारों को कॉलम स्टोरेज में संग्रहित करने के लिए `columnar_attrs = *` सेट कर सकते हैं।

अतिरिक्त रूप से, `id` एक समर्थित गुण है जिसे कॉलम स्टोरेज में संग्रहित किया जा सकता है।

#### columnar_strings_no_hash

```ini
columnar_strings_no_hash = attr1, attr2, attr3
```

डिफ़ॉल्ट रूप से, कॉलम स्टोरेज में संग्रहित सभी स्ट्रिंग गुण पूर्व-कलित हैश को संग्रहित करते हैं। ये हैश समूहबद्ध करने और फ़िल्टर करने के लिए उपयोग किए जाते हैं। हालांकि, ये अतिरिक्त स्थान का उपयोग करते हैं, और यदि आपको उस गुण द्वारा समूहबद्ध करने की आवश्यकता नहीं है, तो आप हैश जनरेशन को अक्षम करके स्थान बचा सकते हैं।

### CREATE TABLE के माध्यम से ऑनलाइन वास्तविक समय की तालिका बनाना

<!-- उदाहरण rt_mode -->
##### CREATE TABLE की सामान्य संबंधी वाक्यविन्यास

```sql
CREATE TABLE [IF NOT EXISTS] name ( <field name> <field data type> [data type options] [, ...]) [table_options]
```

##### डेटा प्रकार:

डेटा प्रकारों के बारे में अधिक जानकारी के लिए, [यहाँ डेटा प्रकारों के बारे में अधिक जानें](../../Creating_a_table/Data_types.md)।

| प्रकार | कॉन्फ़िगरेशन फ़ाइल में समकक्ष | नोट्स | उपनाम |
| - | - | - | - |
| [text](../../Creating_a_table/Data_types.md#Text) | [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)  | विकल्प: अनुक्रमित, संग्रहीत। डिफ़ॉल्ट: **दोनों**। यदि पाठ को संग्रहीत रखना है, लेकिन अनुक्रमित, केवल "संग्रहीत" निर्दिष्ट करें। यदि पाठ को केवल अनुक्रमित रखना है, तो केवल "अनुक्रमित" निर्दिष्ट करें। | स्ट्रिंग |
| [integer](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint)
| integer
 | int, uint |
| [bigint](../../Creating_a_table/Data_types.md#Big-Integer) | [rt_attr_bigint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bigint)
| बड़ा पूर्णांक
 |   |
| [float](../../Creating_a_table/Data_types.md#Float) | [rt_attr_float](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float)   | फ्लोट  |   |
| [float_vector](../../Creating_a_table/Data_types.md#Float-vector) | [rt_attr_float_vector](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float_vector) | फ्लोट वैल्यूज़ का वेक्टर  |   |
| [multi](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) | [rt_attr_multi](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi)   | मल्टी-इंटीजर |   |
| [multi64](../../Creating_a_table/Data_types.md#Multi-value-big-integer) | [rt_attr_multi_64](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi_64) | मल्टी-बिगइंट  |   |
| [bool](../../Creating_a_table/Data_types.md#Boolean) | [rt_attr_bool](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bool) | बूलियन |   |

| [json](../../Creating_a_table/Data_types.md#JSON) | [rt_attr_json](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_json) | JSON |   |
| [string](../../Creating_a_table/Data_types.md#String) | [rt_attr_string](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_string) | स्ट्रिंग। विकल्प `indexed, attribute` मान को पूर्ण-टेक्स्ट अनुक्रमित और फ़िल्टर करने योग्य, सॉर्ट करने योग्य और समूहबद्ध बनाने के लिए सेट करेगा  |   |
| [timestamp](../../Creating_a_table/Data_types.md#Timestamps) |

[rt_attr_timestamp](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_timestamp) | टाइमस्टैम्प  |   |
| [bit(n)](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint field_name:N](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint) | N अधिकतम बिट्स की संख्या है जिसे बनाए रखना है  |   |
<!-- intro -->

##### CREATE TABLE के माध्यम से वास्तविक समय तालिका बनाने के उदाहरण

<!-- request SQL -->
```sql
CREATE TABLE products (title text, price float) morphology='stem_en'
```
यह "products" तालिका को "title" (पूर्ण-पाठ) और "price" (float) दो क्षेत्रों के साथ बनाता है, और "morphology" को "stem_en" पर सेट करता है।
```sql
CREATE TABLE products (title text indexed, description text stored, author text, price float)
```


यह "products" तालिका को तीन क्षेत्रों के साथ बनाता है:

* "title" अनुक्रमित है, लेकिन संग्रहीत नहीं है।
* "description" संग्रहीत है, लेकिन अनुक्रमित नहीं है।
* "author" दोनों संग्रहीत और अनुक्रमित है।
<!-- end -->

## इंजन

```ini

create table ... engine='columnar';
create table ... engine='rowwise';
```


इंजन सेटिंग तालिका के सभी गुणों के लिए डिफ़ॉल्ट [गुण भंडारण](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) को बदलती है। आप प्रत्येक गुण के लिए `engine` [अलग से निर्दिष्ट](../../Creating_a_table/Data_types.md#How-to-switch-between-the-storages) भी कर सकते हैं।
साधारण तालिका के लिए स्तंभ भंडारण कैसे सक्षम करें, इस पर जानकारी के लिए देखें  [columnar_attrs](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs) ।

मूल्य:

* columnar - सभी तालिका गुणों के लिए स्तंभ भंडारण सक्षम करता है, [json](../../Creating_a_table/Data_types.md#JSON) को छोड़कर
* **rowwise (डिफ़ॉल्ट)** - कुछ भी नहीं बदलता है और तालिका के लिए पारंपरिक पंक्ति-वार भंडारण का उपयोग करता है।

## अन्य सेटिंग्स

निम्नलिखित सेटिंग्स वास्तविक समय और साधारण तालिकाओं दोनों के लिए लागू होती हैं, चाहे उन्हें एक कॉन्फ़िगरेशन फ़ाइल में निर्दिष्ट किया गया हो या `CREATE` या `ALTER` कमांड का उपयोग करके ऑनलाइन सेट किया गया हो।

### प्रदर्शन से संबंधित

#### तालिका फ़ाइलों तक पहुँच
Manticore तालिका डेटा पढ़ने के लिए दो एक्सेस मोड का समर्थन करता है: seek+read और mmap।
seek+read मोड में, सर्वर दस्तावेज़ सूचियों और कीवर्ड स्थानों को पढ़ने के लिए `pread` सिस्टम कॉल का उपयोग करता है, जिसे `*.spd` और `*.spp` फ़ाइलों द्वारा प्रदर्शित किया जाता है। सर्वर पढ़ने की प्रक्रिया को अनुकूलित करने के लिए आंतरिक पढ़ने वाले बफ़रों का उपयोग करता है, और इन बफ़रों के आकार को विकल्पों [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) और [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) का उपयोग करके समायोजित किया जा सकता है। वहाँ विकल्प [preopen](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#preopen) भी है जो यह नियंत्रित करता है कि Manticore प्रारंभ में फ़ाइलों को कैसे खोलता है।
mmap पहुँच मोड में, खोज сервер तालिका की फ़ाइल को `mmap` सिस्टम कॉल का उपयोग करके मेमोरी में मैप करता है, और OS फ़ाइल की सामग्री को कैश करता है। विकल्प [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) और [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) इस मोड में संबंधित फ़ाइलों के लिए कोई प्रभाव नहीं डालते हैं। mmap रीडर तालिका के डेटा को मेमोरी में लॉक करने के लिए `mlock` विशेषाधिकार कॉल भी कर सकता है, जो OS को कैश किए गए डेटा को डिस्क पर स्वैप करने से रोकता है।
कौन सा एक्सेस मोड उपयोग करना है, इसे नियंत्रित करने के लिए **access_plain_attrs**, **access_blob_attrs**, **access_doclists**, **access_hitlists** और **access_dict** विकल्प उपलब्ध हैं, जिनके निम्नलिखित मान हैं:
| मान | विवरण |


| - | - |
| फ़ाइल | सर्वर फ़ाइल एक्सेस पर आंतरिक बफ़रों का उपयोग करके खोज + पढ़ने के साथ डिस्क से तालिका फ़ाइलों को पढ़ता है |
| mmap | सर्वर तालिका फ़ाइलों को मेमोरी में मैप करता है और फ़ाइल एक्सेस पर OS उसकी सामग्री को कैश करता है |
| mmap_preread | सर्वर तालिका फ़ाइलों को मेमोरी में मैप करता है और एक बैकग्राउंड थ्रेड इसे गर्म करने के लिए एक बार पढ़ता है |
| mlock | सर्वर तालिका फ़ाइलों को मेमोरी में मैप करता है और फिर फ़ाइल की सामग्री को कैश करने और इसे मेमोरी में लॉक करने के लिए mlock() सिस्टम कॉल का निष्पादन करता है ताकि इसे स्वैप नहीं किया जा सके |
| सेटिंग | मान | विवरण |
| - | - | - |

| access_plain_attrs  | mmap, **mmap_preread** (डिफ़ॉल्ट), mlock | यह नियंत्रित करता है कि `*.spa` (साधारण गुण) `*.spe` (स्किप सूचियाँ) `*.spt` (खोजें) `*.spm` (किल किए गए दस्तावेज़) को कैसे पढ़ा जाएगा |

| access_blob_attrs   | mmap, **mmap_preread** (डिफ़ॉल्ट), mlock  | यह नियंत्रित करता है कि `*.spb` (ब्लॉब गुण) (स्ट्रिंग, mva और json गुण) को कैसे पढ़ा जाएगा |
| access_doclists   | **फ़ाइल** (डिफ़ॉल्ट), mmap, mlock  | यह नियंत्रित करता है कि `*.spd` (दस्तावेज़ सूचियाँ) डेटा को कैसे पढ़ा जाएगा |
| access_hitlists   | **फ़ाइल** (डिफ़ॉल्ट), mmap, mlock  | यह नियंत्रित करता है कि `*.spp` (हिट सूचियाँ) डेटा को कैसे पढ़ा जाएगा |
| access_dict   | mmap, **mmap_preread** (डिफ़ॉल्ट), mlock  | यह नियंत्रित करता है कि `*.spi` (शब्दकोश) को कैसे पढ़ा जाएगा |
यहाँ एक तालिका है जो आपको आपकी इच्छित मोड का चयन करने में मदद कर सकती है:
| तालिका भाग |
डिस्क पर रखें |
मेमोरी में रखें |

cached in memory on server start | lock it in memory |

| - | - | - | - | - |
| plain attributes in [row-wise](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) (non-columnar) storage, skip lists, word lists, lookups, killed docs | 
mmap | mmap |
**mmap_preread** (default) | mlock |
| row-wise string, multi-value attributes (MVA) and json attributes | mmap | mmap | **mmap_preread** (default) | mlock |

| [columnar](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) numeric, string and multi-value attributes | हमेशा | केवल OS के माध्यम से | नहीं | समर्थित नहीं |
| doc lists | **फाइल** (डिफ़ॉल्ट) | mmap | नहीं
| mlock |
| hit lists | **फाइल** (डिफ़ॉल्ट) | mmap | नहीं
| mlock |

| dictionary | mmap | mmap | **mmap_preread** (default) | mlock |

##### The recommendations are:

* For the **सबसे तेज़ खोज प्रतिक्रिया समय** और पर्याप्त मेमोरी उपलब्धता के लिए, [row-wise](../../Creating_a_table/Data_types.md#JSON) विशेषताओं का उपयोग करें और उन्हें मेमोरी में `mlock` का उपयोग करके लॉक करें। इसके अतिरिक्त, doclists/hitlists के लिए mlock का उपयोग करें।

* यदि आप **शुरुआत के बाद कम प्रदर्शन सहन नहीं कर सकते** और लंबे स्टार्टअप समय को बलिदान देने के लिए तैयार हैं, तो [--force-preread](../../Starting_the_server/Manually.md#searchd-command-line-options). विकल्प का उपयोग करें। यदि आप तेज़ searchd पुनः प्रारंभ चाहते हैं, तो डिफ़ॉल्ट `mmap_preread` विकल्प पर टिकें।
* यदि आप **मेमोरी बचाना चाहते हैं**, जबकि सभी विशेषताओं के लिए अभी भी पर्याप्त मेमोरी है, तो `mlock` का उपयोग करने से बचें। ऑपरेटिंग सिस्टम यह तय करेगा कि क्या मेमोरी में रखा जाना चाहिए, जो बार-बार डिस्क पढ़ने पर आधारित है।
* यदि row-wise विशेषताएँ **मेमोरी में नहीं आती हैं**, तो [columnar attributes](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) का विकल्प चुनें।

* यदि पूर्ण-पाठ खोज **प्रदर्शन की चिंता नहीं है**, और आप मेमोरी बचाना चाहते हैं, तो `access_doclists/access_hitlists=file` का उपयोग करें।

डिफ़ॉल्ट मोड संतुलन प्रदान करता है:

* mmap,

* गैर-स्तंभ विशेषताओं का प्रीरीडिंग,
* बिना प्रीरीड के स्तंभ विशेषताओं को खोजने और पढ़ने,
* बिना प्रीरीड के doclists/hitlists को खोजने और पढ़ने।

यह अधिकांश परिदृश्यों में उचित खोज प्रदर्शन, सर्वोत्तम मेमोरी उपयोग और तेज़ searchd पुनः प्रारंभ प्रदान करता है।

### अन्य प्रदर्शन संबंधी सेटिंग्स

#### attr_update_reserve

```ini
attr_update_reserve = 256k
```

यह सेटिंग मल्टी-वैल्यू एट्रीब्यूट्स (MVA), स्ट्रिंग और JSON जैसी blob एट्रीब्यूट्स के अपडेट के लिए अतिरिक्त स्पेस आरक्षित करती है। डिफ़ॉल्ट मूल्य 128k है। जब इन एट्रीब्यूट्स को अपडेट किया जाता है, तो उनका आकार बदल सकता है। यदि अपडेट की गई स्ट्रिंग पहले वाली से छोटी है, तो यह `*.spb` फाइल में पुराना डाटा ओवरराइट कर देगी। यदि अपडेट की गई स्ट्रिंग पहले वाली से लंबी है, तो इसे `*.spb` फाइल के अंत में लिखा जाएगा। यह फ़ाइल मेमोरी-मैप की गई है, जिससे इसे आकार बदलना एक संभावित धीमी प्रक्रिया हो सकती है, जो ऑपरेटिंग सिस्टम की मेमोरी-मैप की गई फ़ाइल कार्यान्वयन पर निर्भर करती है। बार-बार आकार बदलने से बचने के लिए, आप इस सेटिंग का उपयोग करके .spb फ़ाइल के अंत में अतिरिक्त स्पेस आरक्षित कर सकते हैं।

मूल्य: आकार, डिफ़ॉल्ट **128k**।

#### docstore_block_size

```ini
docstore_block_size = 32k
```

यह सेटिंग दस्तावेज़ स्टोरेज द्वारा उपयोग किए जाने वाले ब्लॉकों के आकार को नियंत्रित करती है। डिफ़ॉल्ट मूल्य 16kb है। जब मूल दस्तावेज़ पाठ को stored_fields या stored_only_fields का उपयोग करके संग्रहीत किया जाता है, तो इसे तालिका में संग्रहीत किया जाता है और दक्षता के लिए संकुचित किया जाता है। छोटे दस्तावेजों के लिए डिस्क पहुँच और संकुचन अनुपात को अनुकूलित करने के लिए, इन दस्तावेज़ों को ब्लॉकों में जोड़ा जाता है। अनुक्रमण प्रक्रिया दस्तावेजों को इकट्ठा करती है जब तक उनका कुल आकार इस विकल्प द्वारा निर्दिष्ट थ्रेशोल्ड तक नहीं पहुँच जाता है। उस बिंदु पर, दस्तावेजों का ब्लॉक संकुचित किया जाता है। इस विकल्प को बेहतर संकुचन अनुपात (ब्लॉक का आकार बढ़ाकर) प्राप्त करने या दस्तावेज़ पाठ तक जल्दी पहुँच (ब्लॉक का आकार घटाकर) करने के लिए समायोजित किया जा सकता है।

मूल्य: आकार, डिफ़ॉल्ट **16k**।

#### docstore_compression

```ini
docstore_compression = lz4hc
```

यह सेटिंग दस्तावेज़ स्टोरेज में संग्रहीत दस्तावेज़ ब्लॉकों को संकुचित करने के लिए उपयोग की जाने वाली संकुचन के प्रकार को निर्धारित करती है। यदि stored_fields या stored_only_fields निर्दिष्ट किए जाते हैं, तो दस्तावेज़ स्टोरेज संकुचित दस्तावेज़ ब्लॉकों को संग्रहीत करता है। 'lz4' तेज़ संकुचन और डिकंप्रेशन गति प्रदान करता है, जबकि 'lz4hc' (उच्च संकुचन) कुछ संकुचन गति को एक बेहतर संकुचन अनुपात के लिए बलिदान करता है। 'none' संकुचन को पूरी तरह से अक्षम कर देता है।

मूल्य: **lz4** (डिफ़ॉल्ट), lz4hc, none।

#### docstore_compression_level

```ini
docstore_compression_level = 12
```

जब दस्तावेज़ स्टोरेज में 'lz4hc' संकुचन लागू किया जाता है, तो उपयोग की जाने वाली संकुचन स्तर। संकुचन स्तर को समायोजित करके, आप 'lz4hc' संकुचन के उपयोग के समय प्रदर्शन और संकुचन अनुपात के बीच सही संतुलन खोज सकते हैं। ध्यान दें कि यह विकल्प 'lz4' संकुचन के समय लागू नहीं होता है।

मूल्य: 1 से 12 के बीच एक पूर्णांक, डिफ़ॉल्ट **9**।

#### preopen

```ini
preopen = 1
```

यह सेटिंग यह इंगित करती है कि searchd को स्टार्टअप या रोटेशन पर सभी तालिका फ़ाइलों को खोलना चाहिए, और उन्हें चलाते समय खोला रखना चाहिए। डिफ़ॉल्ट रूप से, फ़ाइलें पूर्व-खुली नहीं होती हैं। पूर्व-खुली तालिकाओं को प्रति तालिका कुछ फ़ाइल विवरणकों की आवश्यकता होती है, लेकिन वे प्रति-प्रश्न open() कॉल की आवश्यकता को समाप्त कर देती हैं और उच्च लोड के तहत तालिका रोटेशन के दौरान होने वाली दौड़ की स्थितियों के प्रति अडिग होती हैं। हालाँकि, यदि आप कई तालिकाओं की सेवा कर रहे हैं, तो फ़ाइल विवरणकों को बचाने के लिए उन्हें प्रति-प्रश्न आधार पर खोलना अधिक प्रभावी हो सकता है।

मूल्य: **0** (डिफ़ॉल्ट), या 1।

#### read_buffer_docs

```ini

read_buffer_docs = 1M

```
कुंजी शब्द के अनुसार दस्तावेज़ों की सूची को संग्रहीत करने के लिए बफ़र का आकार। इस मान को बढ़ाने पर प्रश्न निष्पादन के दौरान उच्च मेमोरी उपयोग होगा, लेकिन यह I/O समय को कम कर सकता है।
मूल्य: आकार, डिफ़ॉल्ट **256k**, न्यूनतम मान 8k है।

#### read_buffer_hits

```ini

read_buffer_hits = 1M

```


कुंजी शब्द के अनुसार हिट्स की सूची को स्टोर करने के लिए बफ़र का आकार। इस मान को बढ़ाने पर प्रश्न निष्पादन के दौरान उच्च मेमोरी उपयोग होगा, लेकिन यह I/O समय को कम कर सकता है।
मूल्य: आकार, डिफ़ॉल्ट **256k**, न्यूनतम मान 8k है।

### Plain table disk footprint settings

#### inplace_enable
<!-- example inplace_enable -->
```ini

inplace_enable = {0|1}
```
इन-प्लेस टेबल इन्वर्ज़न को सक्षम करता है। वैकल्पिक, डिफ़ॉल्ट 0 (अलग अस्थायी फ़ाइलों का उपयोग करता है)।
`inplace_enable` विकल्प सामान्य तालिकाओं की अनुक्रमण के दौरान डिस्क का पदचिह्न कम करता है, जबकि अनुक्रमण को थोड़ी धीमी गति करता है (यह लगभग 2 गुना कम डिस्क का उपयोग करता है, लेकिन मूल प्रदर्शन का लगभग 90-95% देता है)।
अनुक्रमण दो प्राथमिक चरणों में होता है। पहले चरण के दौरान, दस्तावेज़ एकत्रित, संसाधित और कीवर्ड द्वारा आंशिक रूप से क्रमबद्ध किए जाते हैं, और अंतरिम परिणाम अस्थायी फ़ाइलों में लिखे जाते हैं (.tmp*)। दूसरे चरण के दौरान, दस्तावेज़ पूरी तरह से क्रमबद्ध होते हैं और अंतिम तालिका फ़ाइलें बनाई जाती हैं। उत्पादन तालिका को ऑन-द-फ्लाई पुनर्निर्माण करने के लिए लगभग 3 गुना चरम डिस्क का पदचिह्न आवश्यक है: पहले अंतरिम अस्थायी फ़ाइलों के लिए, दूसरे नई निर्मित प्रति के लिए, और तीसरे पुराने तालिका के लिए जो इस बीच उत्पादन प्रश्नों की सेवा करेगी। (अंतरिम डेटा का आकार अंतिम तालिका के समान है।) यह बड़े डेटा संग्रह के लिए डिस्क का पदचिह्न हो सकता है, और `inplace_enable` विकल्प का उपयोग इसे कम करने के लिए किया जा सकता है। जब सक्षम किया जाता है, तो यह अस्थायी फ़ाइलों का पुनः उपयोग करता है, अंतिम डेटा को वापस इन्हीं में आउटपुट करता है, और समाप्ति पर उनका नाम बदलता है। हालांकि, इसके लिए अतिरिक्त अस्थायी डेटा खंड पुनर्स्थान की आवश्यकता हो सकती है, जहाँ से प्रदर्शन का प्रभाव आता है।

यह निर्देश [searchd](../../Starting_the_server/Manually.md) पर कोई प्रभाव नहीं डालता है, यह केवल [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) को प्रभावित करता है।

<!-- intro -->

##### CONFIG:
<!-- request CONFIG -->
```ini

table products {

  inplace_enable = 1

  path = products
  source = src_base

}

```
<!-- end -->
#### inplace_hit_gap
<!-- example inplace_hit_gap -->

```ini
inplace_hit_gap = size
```
विकल्प [In-place inversion](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#inplace_enable) ठीक-ठाक करने वाला विकल्प। पूर्वनिर्धारित हिटलिस्ट गैप आकार को नियंत्रित करता है। वैकल्पिक, डिफ़ॉल्ट 0 है।
यह निर्देश केवल [searchd](../../Starting_the_server/Manually.md) उपकरण को प्रभावित करता है, और [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) पर कोई प्रभाव नहीं डालता है।

<!-- intro -->

##### CONFIG:

<!-- request CONFIG -->
```ini
table products {

  inplace_hit_gap = 1M

  inplace_enable = 1

  path = products
  source = src_base

}

```
<!-- end -->
#### inplace_reloc_factor
<!-- example inplace_reloc_factor -->

```ini
inplace_reloc_factor = 0.1
```
inplace_reloc_factor सेटिंग अनुक्रमण के दौरान उपयोग किए जाने वाले मेमोरी एरेनॉ में पुनर्स्थान बफ़र के आकार को निर्धारित करती है। डिफ़ॉल्ट मान 0.1 है।
यह विकल्प वैकल्पिक है और केवल [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) उपकरण को प्रभावित करता है, [searchd](../../Starting_the_server/Manually.md) सर्वर को नहीं।

<!-- intro -->

##### CONFIG:

<!-- request CONFIG -->
```ini
table products {

  inplace_reloc_factor = 0.1

  inplace_enable = 1


  path = products
  source = src_base

}

```
<!-- end -->
#### inplace_write_factor
<!-- example inplace_write_factor -->

```ini
inplace_write_factor = 0.1
```
अनुक्रमण के दौरान इन-प्लेस लेखन के लिए उपयोग किए जाने वाले बफ़र के आकार को नियंत्रित करता है। वैकल्पिक, डिफ़ॉल्ट मान 0.1 है।
यह ध्यान रखना महत्वपूर्ण है कि यह निर्देश केवल [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) उपकरण को प्रभावित करता है और [searchd](../../Starting_the_server/Manually.md) सर्वर को नहीं।

<!-- intro -->
##### CONFIG:
<!-- request CONFIG -->
```ini
table products {
  inplace_write_factor = 0.1
  inplace_enable = 1
  path = products
  source = src_base
}
```
<!-- end -->
### प्राकृतिक भाषा प्रसंस्करण विशिष्ट सेटिंग्स
निम्नलिखित सेटिंग्स समर्थित हैं। इन्हें सभी को [NLP और टोकनाइज़ेशन](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md) अनुभाग में वर्णित किया गया है।
* [bigram_freq_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_freq_words)
* [bigram_index](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)
* [blend_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_chars)
* [blend_mode](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_mode)
* [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)
* [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)
* [embedded_limit](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#embedded_limit)
* [exceptions](../../Creating_a_table/NLP_and_tokenization/Exceptions.md)
* [expand_keywords](../../Searching/Options.md#expand_keywords)
* [global_idf](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#global_idf)
* [hitless_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#hitless_words)
* [html_index_attrs](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_index_attrs)
* [html_remove_elements](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_remove_elements)
* [html_strip](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip)
* [ignore_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ignore_chars)
* [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words)
* [index_field_lengths](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#index_field_lengths)
* [index_sp](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp)
* [index_token_filter](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#index_token_filter)
* [index_zones](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_zones)
* [infix_fields](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#infix_fields)
* [killlist_target](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target)
* [max_substring_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#max_substring_len)
* [min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)
* [min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)
* [min_stemming_len](../../Creating_a_table/NLP_and_tokenization/Morphology.md#min_stemming_len)
* [min_word_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len)
* [morphology](../../Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)
* [morphology_skip_fields](../../Creating_a_table/NLP_and_tokenization/Morphology.md#morphology_skip_fields)
* [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)
* [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len)
