# साधारण और वास्तविक-समय तालिका सेटिंग्स

<!-- उदाहरण कॉन्फ़िगरेशन -->

## कॉन्फ़िगरेशन फ़ाइल में तालिका स्किमा को परिभाषित करना

```ini

table <table_name>[:<parent table name>] {

...

}

```

<!-- परिचय -->

##### एक साधारण तालिका का उदाहरण एक कॉन्फ़िगरेशन फ़ाइल में

<!-- अनुरोध साधारण -->

```ini

table <table name> {

  type = plain

  path = /path/to/table

  source = <source_name>

  source = <another source_name>

  [stored_fields = <कमों से अलग सूची पूर्ण-पाठ फ़ील्ड जो संग्रहीत किए जाने चाहिए, सभी डिफ़ॉल्ट द्वारा संग्रहीत होते हैं, खाली हो सकता है>]

}

```

<!-- परिचय -->

##### एक वास्तविक-समय तालिका का उदाहरण एक कॉन्फ़िगरेशन फ़ाइल में

<!-- अनुरोध वास्तविक-समय -->

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

  [stored_fields = <कमों से अलग सूची पूर्ण-पाठ फ़ील्ड जो संग्रहीत किए जाने चाहिए, सभी डिफ़ॉल्ट द्वारा संग्रहीत होते हैं, खाली हो सकता है>]

  [rt_mem_limit = <RAM भाग अधिकतम आकार, डिफ़ॉल्ट 128M>]

  [optimize_cutoff = <RT तालिका डिस्क भागों की अधिकतम संख्या>]

}

```

<!-- अंत -->

### सामान्य साधारण और वास्तविक-समय तालिकाएँ सेटिंग्स

#### प्रकार

```ini

type = plain

type = rt

```

तालिका प्रकार: "plain" या "rt" (वास्तविक-समय)

मान: **plain** (डिफ़ॉल्ट), rt

#### पथ

```ini

path = path/to/table

```

उस स्थान का पथ जहाँ तालिका संग्रहीत या स्थित होगी, या तो पूर्ण या सापेक्ष, बिना विस्तार के।

मान: तालिका का पथ, **अनिवार्य**

#### stored_fields

```ini

stored_fields = title, content

```

<!-- उदाहरण stored_fields -->

डिफ़ॉल्ट रूप से, पूर्ण-पाठ फ़ील्ड की मूल सामग्री को एक कॉन्फ़िगरेशन फ़ाइल में तालिका परिभाषित करते समय अनुक्रमित और संग्रहीत किया जाता है। यह सेटिंग आपको यह निर्दिष्ट करने की अनुमति देती है कि कौन-से फ़ील्ड के मूल मान संग्रहीत होने चाहिए।

मान: **पूर्ण-पाठ** फ़ील्ड की एक कमों से अलग सूची जो संग्रहीत होने चाहिए। एक खाली मान (जैसे `stored_fields =`) सभी फ़ील्ड के लिए मूल मानों के भंडारण को निष्क्रिय कर देता है।

नोट: वास्तविक-समय तालिका के मामले में, `stored_fields` में सूचीबद्ध फ़ील्ड को [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field) के रूप में भी घोषित किया जाना चाहिए।

इसके अलावा, ध्यान दें कि आपको `stored_fields` में विशेषताएँ सूचीबद्ध करने की आवश्यकता नहीं है, क्योंकि उनके मूल मान वैसे भी संग्रहीत होते हैं। `stored_fields` केवल पूर्ण-पाठ फ़ील्ड के लिए उपयोग किया जा सकता है।

दस्तावेज़ भंडारण संकुचन विकल्पों के लिए [docstore_block_size](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_block_size), [docstore_compression](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_compression) पर भी देखें।

<!-- परिचय -->

##### SQL:

<!-- अनुरोध SQL -->

```sql

CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)

```

<!-- अनुरोध JSON -->

```JSON

POST /cli -d "

CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)"

```

<!-- अनुरोध PHP -->

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

<!-- परिचय -->

##### Python:

<!-- अनुरोध Python -->

```python

utilsApi.sql('CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)')

```

<!-- परिचय -->

##### Javascript:

<!-- अनुरोध Javascript -->

```javascript

res = await utilsApi.sql('CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)');

```

<!-- परिचय -->

##### Java:

<!-- अनुरोध Java -->

```java

utilsApi.sql("CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)");

```

<!-- परिचय -->

##### C#:

<!-- अनुरोध C# -->

```clike

utilsApi.Sql("CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)");

```

<!-- परिचय -->

##### Typescript:

<!-- अनुरोध Typescript -->

```typescript

res = await utilsApi.sql('CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)');

```

<!-- परिचय -->

##### Go:

<!-- अनुरोध Go -->

```go

utilsAPI.Sql(context.Background()).Body("CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)").Execute()

```

<!-- अनुरोध CONFIG -->

```ini

table products {

  stored_fields = title, content # हम केवल "title" और "content" को संग्रहीत करना चाहते हैं, "name" को संग्रहीत नहीं करना चाहिए


  type = rt
  path = tbl
  rt_field = title
  rt_field = content
  rt_field = name
  rt_attr_uint = price
}
```
<!-- end -->

#### stored_only_fields

```ini
stored_only_fields = title,content
```

टेबल में स्टोर किए जाने वाले फ़ील्ड की सूची, लेकिन अनुक्रमित नहीं। यह सेटिंग [stored_fields](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields) के समान काम करती है, सिवाय इसके कि जब किसी फ़ील्ड को `stored_only_fields` में निर्दिष्ट किया जाता है, तो यह केवल स्टोर किया जाएगा, अनुक्रमित नहीं होगा, और पूर्ण-पाठ क्वेरी का उपयोग करके खोजा नहीं जा सकता। इसे केवल खोज परिणामों में प्राप्त किया जा सकता है।

मान एक अल्पविराम से विभाजित फ़ील्ड की सूची है जो केवल स्टोर की जानी चाहिए, अनुक्रमित नहीं। डिफ़ॉल्ट रूप से, यह मान खाली है। यदि एक वास्तविक समय तालिका स्थापित की जा रही है, तो `stored_only_fields` में सूचीबद्ध फ़ील्ड को [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field) के रूप में भी घोषित किया जाना चाहिए।

यह भी ध्यान दें कि आपको `stored_only_fields` में गुणों को सूचीबद्ध करने की आवश्यकता नहीं है, क्योंकि उनकी मूल मान anyway स्टोर की गई हैं। यदि `stored_only_fields` की तुलना स्ट्रिंग गुणों से की जाए तो पूर्व (स्टोर किया गया फ़ील्ड):
* डिस्क पर स्टोर किया गया है और मेमोरी का उपयोग नहीं करता
* संकुचित प्रारूप में स्टोर किया जाता है
* इसे केवल प्राप्त किया जा सकता है, इसका उपयोग क्रमबद्धता, फ़िल्टरिंग या समूह बनाने के लिए नहीं किया जा सकता है।

इसके विपरीत, बाद वाला (स्ट्रिंग गुण):
* डिस्क और मेमोरी पर स्टोर किया गया है
* बिना संकुचन प्रारूप में स्टोर किया जाता है
* इसे क्रमबद्धता, समूह बनाने, फ़िल्टरिंग, और किसी अन्य क्रियाओं के लिए उपयोग किया जा सकता है जो आप गुणों के साथ करना चाहते हैं।

#### json_secondary_indexes

```ini
json_secondary_indexes = json_attr
```

<!-- example json_secondary_indexes -->

डिफ़ॉल्ट रूप से, सभी गुणों के लिए दूसरे अनुक्रमित बनाए जाते हैं, सिवाय JSON गुणों के। हालाँकि, JSON गुणों के लिए दूसरे अनुक्रमित को स्पष्ट रूप से `json_secondary_indexes` सेटिंग का उपयोग करके उत्पन्न किया जा सकता है। जब किसी JSON गुण को इस विकल्प में शामिल किया जाता है, तो इसकी सामग्री को कई द्वितीयक अनुक्रमित में फ़्लैट किया जाता है। इन अनुक्रमितों का उपयोग प्रश्न ऑप्टिमाइज़र द्वारा प्रश्नों को तेज करने के लिए किया जा सकता है।

आप [SHOW TABLE <tbl> INDEXES](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_INDEXES.md) आदेश का उपयोग करके उपलब्ध द्वितीयक अनुक्रमित देख सकते हैं।

मान: एक अल्पविराम से विभाजित सूची JSON गुणों की जिनके लिए द्वितीयक अनुक्रमित उत्पन्न किए जाने चाहिए।

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
##### Python:
<!-- request Python -->
```python
utilsApi.sql('CREATE TABLE products(title text, j json secondary_index='1')')
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->
```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, j json secondary_index=\'1\')');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, j json secondary_index='1')");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, j json secondary_index='1')");
```

<!-- intro -->
##### Typescript:

<!-- request Typescript -->
```typescript
res = await utilsApi.sql('CREATE TABLE products(title text, j json secondary_index=\'1\')');
```

<!-- intro -->
##### Go:

<!-- request Go -->
```go
utilsAPI.Sql(context.Background()).Body("CREATE TABLE products(title text, j json secondary_index='1')").Execute()
```


<!-- request CONFIG -->

```ini
table products {
  json_secondary_indexes = j

  type = rt
  path = tbl
  rt_field = title
  rt_attr_json = j
}
```
<!-- end -->

### Real-time table settings:

#### diskchunk_flush_search_timeout

```ini
diskchunk_flush_search_timeout = 10s
```

तालिका में कोई खोजें नहीं होने पर RAM भाग के स्वचालित फ्लशिंग को रोकने के लिए टाइमआउट। इसके बारे में अधिक जानें [यहाँ](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout).

#### diskchunk_flush_write_timeout

```ini
diskchunk_flush_write_timeout = 60s
```

जब इसके लिए कोई लेखन नहीं होता है तो RAM भाग के स्वचालित फ्लशिंग के लिए टाइमआउट। इसके बारे में अधिक जानें [यहाँ](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout).

#### optimize_cutoff

RT तालिका के लिए अधिकतम संख्या में डिस्क भाग। इसके बारे में अधिक जानें [यहाँ](../../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks).

#### rt_field

```ini
rt_field = subject
```

यह फ़ील्ड घोषणा निर्धारित करती है कि कौन से पूर्ण-पाठ फ़ील्ड अनुक्रमित किए जाएंगे। फ़ील्ड के नाम अद्वितीय होने चाहिए, और क्रम को बनाए रखा जाएगा। डेटा डालते समय, फ़ील्ड के मान वही क्रम में होने चाहिए जो कॉन्फ़िगरेशन में निर्दिष्ट किया गया है।

यह एक बहु-मूल्य, वैकल्पिक फ़ील्ड है।

#### rt_attr_uint

```ini
rt_attr_uint = gid
```

यह घोषणा एक असाइनित पूर्णांक गुण को परिभाषित करती है।

मान: फ़ील्ड का नाम या field_name:N (जहाँ N अधिकतम संख्या है जिसे बनाए रखना है)।

#### rt_attr_bigint

```ini
rt_attr_bigint = gid
```

यह घोषणा एक BIGINT गुण को परिभाषित करती है।

मान: फ़ील्ड का नाम, कई रिकॉर्ड की अनुमति है।

#### rt_attr_multi

```ini
rt_attr_multi = tags
```

असाइनित 32-बिट पूर्णांक मानों के साथ एक बहु-मूल्य गुण (MVA) को घोषित करता है।

मान: फ़ील्ड का नाम। कई रिकॉर्ड की अनुमति है।

#### rt_attr_multi_64

```ini
rt_attr_multi_64 = wide_tags
```

64-बिट BIGINT मानों के साथ बहु-मूल्य वाली विशेषता (MVA) घोषित करता है।

मान: फ़ील्ड नाम। कई रिकॉर्ड की अनुमति है।

#### rt_attr_float

```ini
rt_attr_float = lat
rt_attr_float = lon
```

32-बिट IEEE 754 प्रारूप के साथ एकल सटीकता वाले तैरते बिंदु विशेषताओं की घोषणा करता है।

मान: फ़ील्ड नाम। कई रिकॉर्ड की अनुमति है।

#### rt_attr_float_vector

```ini
rt_attr_float_vector = image_vector
```

तैरती-बिंदु मानों का एक वेक्टर घोषित करता है।

मान: फ़ील्ड नाम। कई रिकॉर्ड की अनुमति है।

#### rt_attr_bool

```ini
rt_attr_bool = available
```

1-बिट असाइन किए गए पूर्णांक मानों के साथ एक बूलियन विशेषता घोषित करता है।

मान: फ़ील्ड नाम।

#### rt_attr_string

```ini
rt_attr_string = title
```

स्ट्रिंग विशेषता की घोषणा।

मान: फ़ील्ड नाम।

#### rt_attr_json

```ini
rt_attr_json = properties
```

एक JSON विशेषता घोषित करता है।

मान: फ़ील्ड नाम।

#### rt_attr_timestamp

```ini
rt_attr_timestamp = date_added
```

एक टाइमस्टैम्प विशेषता की घोषणा करता है।

मान: फ़ील्ड नाम।

#### rt_mem_limit

```ini
rt_mem_limit = 512M
```

तालिका के RAM चंक के लिए मेमोरी सीमा। वैकल्पिक, डिफ़ॉल्ट 128M है।

RT तालिकाएँ कुछ डेटा को मेमोरी में, जिसे "RAM चंक" कहा जाता है, में संग्रहीत करती हैं, और कुछ ऑन-डिस्क तालिकाओं को भी बनाए रखती हैं, जिन्हें "डिस्क चंक" कहा जाता है। यह निदेशालय आपको RAM चंक के आकार को नियंत्रित करने की अनुमति देता है। जब मेमोरी में रखने के लिए बहुत अधिक डेटा होता है, RT तालिकाएँ इसे डिस्क पर फ्लश करेंगी, एक नए बनाए गए डिस्क चंक को सक्रिय करेंगी, और RAM चंक को वापस सेट करेंगी।

कृपया ध्यान दें कि सीमा सख्त है, और RT तालिकाएँ कभी भी rt_mem_limit में निर्दिष्ट मात्रा से अधिक मेमोरी आवंटित नहीं करेंगी। इसके अतिरिक्त, मेमोरी को पूर्व-आवंटित नहीं किया जाता है, इसलिए 512MB सीमा निर्दिष्ट करना और केवल 3MB डेटा सम्मिलित करना केवल 3MB आवंटित करने का परिणाम देगा, 512MB नहीं।

`rt_mem_limit` कभी भी पार नहीं की जाती, लेकिन वास्तविक RAM चंक का आकार सीमा से काफी कम हो सकता है। RT तालिकाएँ डेटा सम्मिलन की गति के अनुसार समर्पण करती हैं और मेमोरी उपयोग को कम करने और डेटा लिखने की गति को अधिकतम करने के लिए वास्तविक सीमा को गतिशील रूप से समायोजित करती हैं। यह इस प्रकार काम करता है:
* डिफ़ॉल्ट रूप से, RAM चंक का आकार `rt_mem_limit` का 50% होता है, जिसे "`rt_mem_limit`" कहा जाता है।
* जैसे ही RAM चंक में डेटा `rt_mem_limit * rate` डेटा (डिफ़ॉल्ट रूप से `rt_mem_limit` का 50%) के बराबर जमा हो जाता है, Manticore RAM चंक को नए डिस्क चंक के रूप में सहेजना शुरू कर देती है।
* जब एक नया डिस्क चंक सहेजा जा रहा है, तो Manticore नए/अपडेटेड दस्तावेजों की संख्या का आकलन करती है।
* नए डिस्क चंक को सहेजने के बाद, `rt_mem_limit` दर को अपडेट किया जाता है।
* प्रत्येक बार जब आप searchd को पुनरारंभ करते हैं, तो दर को 50% पर रीसेट किया जाता है।

उदाहरण के लिए, यदि 90MB डेटा एक डिस्क चंक में सहेजा गया है और एक अतिरिक्त 10MB डेटा सहेजने की प्रक्रिया के दौरान आता है, तो दर 90% होगी। अगली बार, RT तालिका डेटा को फ्लश करने से पहले `rt_mem_limit` के 90% तक डेटा एकत्र करेगी। जितनी तेज़ी से सम्मिलन की गति होगी, `rt_mem_limit` की दर उतनी कम होगी। दर 33.3% से 95% के बीच भिन्न होती है। आप [SHOW TABLE <tbl> STATUS](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) कमांड का उपयोग करके तालिका की वर्तमान दर देख सकते हैं।

##### rt_mem_limit और optimize_cutoff को कैसे बदलें

वास्तविक समय मोड में, आप RAM चंकों के आकार की सीमा और अधिकतम संख्या में डिस्क चंकों को `ALTER TABLE` कथन का उपयोग करके समायोजित कर सकते हैं। तालिका "t" के लिए `rt_mem_limit` को 1 गीगाबाइट सेट करने के लिए, निम्नलिखित क्वेरी चलाएँ: `ALTER TABLE t rt_mem_limit='1G'`। अधिकतम संख्या में डिस्क चंकों को बदलने के लिए, क्वेरी चलाएँ: `ALTER TABLE t optimize_cutoff='5'`.

साधारण मोड में, आप तालिका कॉन्फ़िगरेशन को अपडेट करके या `ALTER TABLE <table_name> RECONFIGURE` कमांड चलाकर `rt_mem_limit` और `optimize_cutoff` के मान बदल सकते हैं।

##### RAM चंकों के बारे में महत्वपूर्ण नोट्स

* वास्तविक समय तालिकाएँ [वितरित](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table) की तरह होती हैं, जिनमें कई स्थानीय तालिकाएँ शामिल होती हैं, जिन्हें डिस्क चंक भी कहा जाता है।
* प्रत्येक RAM चंक कई खंडों से बना होता है, जो विशेष RAM-केवल तालिकाएँ होती हैं।
* जबकि डिस्क चंकों को डिस्क पर संग्रहीत किया जाता है, RAM चंकों को मेमोरी में संग्रहीत किया जाता है।
* वास्तविक समय तालिका पर किए गए प्रत्येक लेनदेन से एक नया खंड उत्पन्न होता है, और RAM चंक खंड हर लेनदेन की प्रतिबंधित करने के बाद विलय कर दिए जाते हैं। RAM चंक खंडों के विलयन के ओवरहेड को कम करने के लिए कई अलग-अलग INSERT के बजाय सैकड़ों या हजारों दस्तावेज़ों के सामूहिक INSERT करना अधिक प्रभावी है।
* जब खंडों की संख्या 32 से अधिक होती है, तो उन्हें 32 के नीचे रखने के लिए विलय किया जाएगा।
* वास्तविक समय तालिकाओं में हमेशा एक RAM चंक (जो खाली हो सकता है) और एक या अधिक डिस्क चंक होते हैं।
* बड़े खंडों का विलय अधिक समय लेता है, इसलिए बहुत बड़े RAM चंक (और इसलिए `rt_mem_limit`) न रखने से बचना बेहतर है।
* डिस्क चंकों की संख्या तालिका में डेटा और `rt_mem_limit` सेटिंग पर निर्भर करती है।
* Searchd RAM चंक को डिस्क पर फ्लश करता है (एक स्थायी फ़ाइल के रूप में, न कि डिस्क चंक के रूप में) बंद करने पर और [rt_flush_period](../../Server_settings/Searchd.md#rt_flush_period) सेटिंग के अनुसार समय-समय पर। डिस्क पर कई गीगाबाइट्स को फ्लश करने में कुछ समय लग सकता है।
* एक बड़ा RAM चंक स्टोरेज पर अधिक दबाव डालता है, फ्लश करने के समय `.ram` फ़ाइल में और जब RAM चंक भर जाता है और डिस्क चंक के रूप में डिस्क पर डंप किया जाता है।
* RAM चंक को डिस्क पर फ्लश किए जाने तक एक [बाइनरी लॉग](../../Logging/Binary_logging.md) द्वारा समर्थित किया जाता है, और एक बड़ा `rt_mem_limit` सेटिंग बाइनरी लॉग को पुनर्प्रयोजन और RAM चंक को पुनर्प्राप्त करने में लगने वाले समय को बढ़ा देगा।
* RAM चंक डिस्क चंक की तुलना में थोड़ा धीमा हो सकता है।
* हालाँकि RAM चंक स्वयं `rt_mem_limit` से अधिक मेमोरी नहीं लेता है, Manticore कुछ मामलों में अधिक मेमोरी ले सकता है, जैसे जब आप डेटा सम्मिलित करने के लिए एक लेन-देन शुरू करते हैं और उसे थोड़ी देर के लिए प्रतिबंधित नहीं करते हैं। इस मामले में, लेन-देन के भीतर आपने पहले से हस्तांतरित किया गया डेटा मेमोरी में रहेगा।

`rt_mem_limit` के अलावा, RAM चंक्स के फ्लशिंग व्यवहार पर निम्नलिखित सेटिंग्स भी प्रभाव डालती हैं: [diskchunk_flush_write_timeout](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout) और [diskchunk_flush_search_timeout](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout).

* `diskchunk_flush_write_timeout`: यह विकल्प स्वचालित रूप से RAM चंक को फ्लश करने के लिए समय सीमा को परिभाषित करता है यदि इसमें कोई लिखावट नहीं है।  यदि इस समय के भीतर कोई लिखावट नहीं होती है, तो चंक को डिस्क पर फ्लश किया जाएगा। इसे `-1` पर सेट करने से लिखने की गतिविधि के आधार पर स्वचालित फ्लशिंग अक्षम हो जाती है। डिफ़ॉल्ट मान 1 सेकंड है।
* `diskchunk_flush_search_timeout`: यह विकल्प उस समय सीमा को सेट करता है जिससे एक RAM चंक को स्वचालित रूप से फ्लश करने से रोका जा सके यदि तालिका में कोई खोज नहीं हो रही है। स्वचालित फ़्लशिंग केवल तभी होगी जब इस समय के भीतर कम से कम एक खोज हुई हो। डिफ़ॉल्ट मान 30 सेकंड है।

ये टाइमआउट एक साथ काम करते हैं।  एक RAM चंक को फ्लश किया जाएगा यदि *या तो* टाइमआउट प्राप्त होता है।  यह सुनिश्चित करता है कि भले ही कोई लिखावट न हो, डेटा अंततः डिस्क पर संग्रहीत हो जाएगा, और इसके विपरीत, भले ही निरंतर लिखावट हो लेकिन कोई खोज न हो, डेटा भी संग्रहीत हो जाएगा।  ये सेटिंग्स RAM चंक्स के फ्लश होने की आवृत्ति पर अधिक सूक्ष्म नियंत्रण प्रदान करती हैं, डेटा की स्थिरता की आवश्यकता को प्रदर्शन विचारों के साथ संतुलित करती हैं। इन सेटिंग्स के लिए प्रति-तालिका निर्देश उच्च प्राथमिकता रखते हैं और इंस्टेंस-वाइड डिफ़ॉल्ट्स को ओवरराइड करेंगे।

### साधारण तालिका सेटिंग्स:

#### स्रोत

```ini
source = srcpart1
source = srcpart2
source = srcpart3
```

स्रोत फ़ील्ड उस स्रोत को निर्दिष्ट करता है जिससे दस्तावेज़ वर्तमान तालिका के अनुक्रमण के दौरान प्राप्त किए जाएंगे। वहाँ कम से कम एक स्रोत होना अनिवार्य है। स्रोत विभिन्न प्रकार के हो सकते हैं (उदाहरण के लिए, एक MySQL हो सकता है, दूसरा PostgreSQL)। बाहरी संग्रहों से अनुक्रमण के बारे में अधिक जानकारी के लिए, [यहां बाहरी संग्रहों से अनुक्रमण](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)

मान: स्रोत का नाम **अनिवार्य** है। कई मूल्यों की अनुमति है।

#### killlist_target

```ini
killlist_target = main:kl
```

यह सेटिंग उन तालिकाओं को निर्धारित करती है जिन पर किल-लिस्ट लागू की जाएगी। लक्षित तालिका में मेल जिनका वर्तमान तालिका में अद्यतन या हटाया गया है, दबा दिए जाएंगे। `:kl` मोड में, दबाने के लिए दस्तावेज़ [किल-लिस्ट](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md) से लिए जाते हैं। `:id` मोड में, वर्तमान तालिका से सभी दस्तावेज़ आईडी लक्षित तालिका में दबाए जाते हैं। यदि कोई भी निर्दिष्ट नहीं है, तो दोनों मोड प्रभावी होंगे। [किल-लिस्ट के बारे में अधिक जानें यहां](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)

मान: **निर्दिष्ट नहीं किया गया** (पूर्व निर्धारित), target_table_name:kl, target_table_name:id, target_table_name। कई मूल्यों की अनुमति है।

#### columnar_attrs

```ini
columnar_attrs = *
columnar_attrs = id, attr1, attr2, attr3
```

यह कॉन्फ़िगरेशन सेटिंग निर्धारित करती है कि कौन से गुण [स्तंभात्मक संग्रह](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) में संग्रहीत किए जाने चाहिए, पंक्ति-वार संग्रह के बजाय।

आप `columnar_attrs = *` सेट कर सकते हैं सभी समर्थित डेटा प्रकारों को स्तंभात्मक संग्रह में संग्रहीत करने के लिए।

इसके अलावा, `id` एक समर्थित गुण है जिसे स्तंभात्मक संग्रह में संग्रहीत किया जा सकता है।

#### columnar_strings_no_hash

```ini
columnar_strings_no_hash = attr1, attr2, attr3
```

डिफ़ॉल्ट रूप से, सभी स्ट्रिंग गुण जो स्तंभात्मक संग्रह में संग्रहीत होते हैं पूर्व-गणना किए गए हैश को संग्रहीत करते हैं। ये हैश समूहबद्धता और फ़िल्टरिंग के लिए उपयोग किए जाते हैं। हालाँकि, ये अतिरिक्त स्थान घेर लेते हैं, और यदि आपको उस गुण के आधार पर समूह बनाने की आवश्यकता नहीं है, तो आप हैश जनरेशन को निष्क्रिय करके स्थान बचा सकते हैं।

### CREATE TABLE के माध्यम से रियल-टाइम तालिका ऑनलाइन बनाना

<!-- example rt_mode -->
##### CREATE TABLE की सामान्य वाक्यSyntax

```sql
CREATE TABLE [IF NOT EXISTS] name ( <field name> <field data type> [data type options] [, ...]) [table_options]
```

##### डेटा प्रकार:

डेटा प्रकारों के बारे में अधिक जानकारी के लिए, [यहां डेटा प्रकारों के बारे में अधिक जानकारी देखें](../../Creating_a_table/Data_types.md).

| प्रकार | कॉन्फ़िगरेशन फ़ाइल में समकक्ष | नोट्स | उपनाम |
| - | - | - | - |
| [text](../../Creating_a_table/Data_types.md#Text) | [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)  | विकल्प: अनुक्रमित, संग्रहीत। डिफ़ॉल्ट: **दोनों**। टेक्स्ट को संग्रहीत रखने के लिए, लेकिन अनुक्रमित, केवल "stored" निर्दिष्ट करें। केवल टेक्स्ट को अनुक्रमित रखने के लिए, केवल "indexed" निर्दिष्ट करें। | string |
| [integer](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint)	| integer	 | int, uint |
| [bigint](../../Creating_a_table/Data_types.md#Big-Integer) | [rt_attr_bigint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bigint)	| बड़ा पूर्णांक	 |   |
| [float](../../Creating_a_table/Data_types.md#Float) | [rt_attr_float](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float)   | फ़्लोट  |   |
| [float_vector](../../Creating_a_table/Data_types.md#Float-vector) | [rt_attr_float_vector](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float_vector) | फ़्लोट मूल्यों का एक वेक्टर  |   |
| [multi](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) | [rt_attr_multi](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi)   | मल्टी-इंटीजर |   |
| [multi64](../../Creating_a_table/Data_types.md#Multi-value-big-integer) | [rt_attr_multi_64](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi_64) | मल्टी-बिगइंट  |   |
| [bool](../../Creating_a_table/Data_types.md#Boolean) | [rt_attr_bool](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bool) | बूलियन |   |
| [json](../../Creating_a_table/Data_types.md#JSON) | [rt_attr_json](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_json) | JSON |   |
| [string](../../Creating_a_table/Data_types.md#String) | [rt_attr_string](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_string) | string. विकल्प `indexed, attribute` मान को पूर्ण-पाठ अनुक्रमित और फ़िल्टर करने योग्य, क्रमबद्ध और समूहबद्ध करता है  |   |
| [timestamp](../../Creating_a_table/Data_types.md#Timestamps) |	[rt_attr_timestamp](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_timestamp) | timestamp  |   |
| [bit(n)](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint field_name:N](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint) | N बिट्स की अधिकतम संख्या है जिसे बनाए रखा जाएगा  |   |

<!-- intro -->
##### CREATE TABLE के माध्यम से वास्तविक समय तालिका बनाने के उदाहरण
<!-- request SQL -->

```sql
CREATE TABLE products (title text, price float) morphology='stem_en'
```

यह "products" तालिका को दो फ़ील्ड के साथ बनाता है: "title" (पूर्ण-पाठ) और "price" (फ्लोट), और "morphology" को "stem_en" पर सेट करता है।

```sql
CREATE TABLE products (title text indexed, description text stored, author text, price float)
```
यह "products" तालिका को तीन फ़ील्ड के साथ बनाता है:
* "title" अनुक्रमित है, लेकिन संग्रहीत नहीं है।
* "description" संग्रहीत है, लेकिन अनुक्रमित नहीं है।
* "author" दोनों संग्रहीत और अनुक्रमित है।
<!-- end -->


## इंजन

```ini
create table ... engine='columnar';
create table ... engine='rowwise';
```

इंजन सेटिंग तालिका में सभी विशेषताओं के लिए डिफ़ॉल्ट [attribute storage](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) बदलती है। आप प्रत्येक विशेषता के लिए `engine` को [अलग से](../../Creating_a_table/Data_types.md#How-to-switch-between-the-storages) भी निर्दिष्ट कर सकते हैं।

एक साधारण तालिका के लिए कॉलम आधारित संग्रह को सक्रिय करने की जानकारी के लिए, [columnar_attrs](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs) देखें।

मान:
* columnar - सभी तालिका विशेषताओं के लिए कॉलम आधारित संग्रह को सक्षम करता है, सिवाय [json](../../Creating_a_table/Data_types.md#JSON)
* **rowwise (डिफ़ॉल्ट)** - कुछ भी नहीं बदलता है और तालिका के लिए पारंपरिक पंक्ति-आधारित संग्रह का उपयोग करता है।


## अन्य सेटिंग्स
निम्नलिखित सेटिंग्स वास्तविक समय और साधारण तालिकाओं के लिए लागू हैं, चाहे उन्हें एक संरचना फ़ाइल में निर्दिष्ट किया गया हो या `CREATE` या `ALTER` कमांड का उपयोग करके ऑनलाइन सेट किया गया हो।

### प्रदर्शन से संबंधित

#### तालिका फ़ाइलों तक पहुँच
Manticore तालिका डेटा पढ़ने के लिए दो एक्सेस मोड का समर्थन करता है:.seek+read और mmap।

seek+read मोड में, सर्वर दस्तावेज़ सूचियों और कीवर्ड स्थितियों को पढ़ने के लिए `pread` सिस्टम कॉल का उपयोग करता है, जिसे `*.spd` और `*.spp` फ़ाइलों से दर्शाया गया है। सर्वर पढ़ने की प्रक्रिया को अनुकूलित करने के लिए आंतरिक पढ़ने के बफर का उपयोग करता है, और इन बफरों के आकार को [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) और [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) विकल्पों का उपयोग करके समायोजित किया जा सकता है। इसके अलावा, [preopen](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#preopen) विकल्प है जो नियंत्रित करता है कि Manticore स्टार्टअप पर फ़ाइलें कैसे खोलता है।

mmap एक्सेस मोड में, खोज सर्वर तालिका की फ़ाइल को `mmap` सिस्टम कॉल का उपयोग करके मेमोरी में मैप करता है, और OS फ़ाइल सामग्री को कैश करता है। इस मोड में संबंधित फ़ाइलों के लिए [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) और [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) विकल्पों का कोई प्रभाव नहीं है। mmap रीडर तालिका के डेटा को मेमोरी में लॉक भी कर सकता है, जिससे OS कैश की गई डेटा को डिस्क पर स्वैप करने से रोकता है।

किस एक्सेस मोड का उपयोग करना है, इसे नियंत्रित करने के लिए, निम्नलिखित मानों के साथ विकल्प **access_plain_attrs**, **access_blob_attrs**, **access_doclists**, **access_hitlists** और **access_dict** उपलब्ध हैं:

| मान | विवरण |
| - | - |
| file | सर्वर तालिका फ़ाइलों को खोजने के लिए आंतरिक बफ़र्स के साथ डिस्क से seek+read के माध्यम से पढ़ता है |
| mmap | सर्वर तालिका फ़ाइलों को मेमोरी में मैप करता है और OS फ़ाइल पहुँच पर इसकी सामग्री को कैश करता है |
| mmap_preread | सर्वर तालिका फ़ाइलों को मेमोरी में मैप करता है और एक बैकग्राउंड थ्रेड इसे एक बार पढ़ता है ताकि कैश गर्म हो सके |
| mlock | सर्वर तालिका फ़ाइलों को मेमोरी में मैप करता है और फिर फ़ाइल सामग्री को कैश करने और इसे स्वैप आउट होने से रोकने के लिए mlock() सिस्टम कॉल को निष्पादित करता है |


| सेटिंग | मान | विवरण |
| - | - | - |
| access_plain_attrs  | mmap, **mmap_preread** (डिफ़ॉल्ट), mlock | नियंत्रित करता है कि `*.spa` (साधारण विशेषताएँ) `*.spe` (स्किप सूचियाँ) `*.spt` (लुकअप) `*.spm` (मार दिए गए दस्तावेज़) कैसे पढ़े जाएंगे |
| access_blob_attrs   | mmap, **mmap_preread** (डिफ़ॉल्ट), mlock  | नियंत्रित करता है कि `*.spb` (ब्लॉब विशेषताएँ) (स्ट्रिंग, MVA और json विशेषताएँ) कैसे पढ़ी जाएंगी |
| access_doclists   | **file** (डिफ़ॉल्ट), mmap, mlock  | नियंत्रित करता है कि `*.spd` (दस्तावेज़ सूचियाँ) डेटा कैसे पढ़ा जाएगा |
| access_hitlists   | **file** (डिफ़ॉल्ट), mmap, mlock  | नियंत्रित करता है कि `*.spp` (हिट सूचियाँ) डेटा कैसे पढ़ा जाएगा |
| access_dict   | mmap, **mmap_preread** (डिफ़ॉल्ट), mlock  | नियंत्रित करता है कि `*.spi` (शब्दकोश) कैसे पढ़ा जाएगा |

यहाँ एक तालिका है जो आपकी इच्छित मोड का चयन करने में मदद कर सकती है:

| तालिका भाग |	डिस्क पर इसे बनाए रखें |	मेमोरी में इसे बनाए रखें |	सर्वर स्टार्ट पर मेमोरी में कैश किया गया | मेमोरी में इसे लॉक करें |
| - | - | - | - | - |
| [row-wise](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) (गैर-कॉलम के) संग्रह में साधारण विशेषताएँ, स्किप सूचियाँ, शब्द सूचियाँ, लुकअप, मारे गए दस्तावेज़ | 	mmap | mmap |	**mmap_preread** (डिफ़ॉल्ट) | mlock |
| पंक्ति-आधारित स्ट्रिंग, बहु-मान विशेषताएँ (MVA) और json विशेषताएँ | mmap | mmap | **mmap_preread** (डिफ़ॉल्ट) | mlock |
| [columnar](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) संख्यात्मक, स्ट्रिंग और बहु-मूल्य विशेषताएँ | हमेशा  | केवल OS के माध्यम से  | नहीं  | समर्थित नहीं है |
| डोक सूचियाँ | **फाइल** (डिफ़ॉल्ट) | mmap | नहीं	| mlock |
| हिट सूचियाँ | **फाइल** (डिफ़ॉल्ट) | mmap | नहीं	| mlock |
| शब्दकोष | mmap | mmap | **mmap_preread** (डिफ़ॉल्ट) | mlock |

##### अनुशंसाएँ हैं:

* **सबसे तेज़ खोज प्रतिक्रिया समय** और पर्याप्त मेमोरी उपलब्धता के लिए, [रो-वाईज़](../../Creating_a_table/Data_types.md#JSON) विशेषताओं का उपयोग करें और उन्हें मेमोरी में `mlock` का उपयोग करके लॉक करें। इसके अतिरिक्त, डोक सूचियों/हिट सूचियों के लिए mlock का उपयोग करें।
* यदि आप **स्टार्ट के बाद निम्न प्रदर्शन नहीं सहन कर सकते** हैं और लंबे स्टार्टअप समय के लिए तैयार हैं, तो [--force-preread](../../Starting_the_server/Manually.md#searchd-command-line-options) विकल्प का उपयोग करें। यदि आपको तेज़ खोजd पुनःआरंभ की आवश्यकता है, तो डिफ़ॉल्ट `mmap_preread` विकल्प पर टिके रहें।
* यदि आप **मेमोरी का संरक्षण** करना चाहते हैं, जबकि सभी विशेषताओं के लिए पर्याप्त मेमोरी हो, तो `mlock` का उपयोग छोड़ दें। ऑपरेटिंग सिस्टम यह निर्धारित करेगा कि मेमोरी में क्या रखा जाना चाहिए आवधिक डिस्क पढ़ने के आधार पर।
* यदि रो-वाईज़ विशेषताएँ **मेमोरी में नहीं फिट होती**, तो [कॉलम्नार विशेषताओं](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) का विकल्प चुनें
* यदि पूर्ण पाठ खोज **प्रदर्शन की चिंता नहीं है**, और आप मेमोरी बचाना चाहते हैं, तो `access_doclists/access_hitlists=file` का उपयोग करें।

डिफ़ॉल्ट मोड निम्नलिखित का संतुलन प्रदान करता है:
* mmap,
* गैर-कॉलम्नार विशेषताओं का पूर्व-पढ़ना,
* बिना पूर्व-पढ़े कॉलम्नार विशेषताओं की खोज और पढ़ना,
* बिना पूर्व-पढ़े डोक सूचियों/हिट सूचियों की खोज और पढ़ना।

यह अधिकांश परिदृश्यों में उचित खोज प्रदर्शन, आदर्श मेमोरी उपयोग, और तेज़ खोजd पुनःआरंभ प्रदान करता है।

### अन्य प्रदर्शन संबंधित सेटिंग्स

#### attr_update_reserve

```ini
attr_update_reserve = 256k
```

यह सेटिंग बहु-मूल्य विशेषताओं (MVA), स्ट्रिंग और JSON जैसी ब्लॉब विशेषताओं के लिए अपडेट के लिए अतिरिक्त स्थान रिजर्व करती है। डिफ़ॉल्ट मान 128k है। इन विशेषताओं को अपडेट करते समय, उनकी लंबाई बदल सकती है। यदि अपडेट की गई स्ट्रिंग पहले वाली से छोटी है, तो यह `*.spb` फ़ाइल में पुराने डेटा को खत्म कर देगी। यदि अपडेट की गई स्ट्रिंग लंबी है, तो यह `*.spb` फ़ाइल के अंत में लिखी जाएगी। यह फ़ाइल मेमोरी-मैप की गई है, जिससे इसे फिर से आकार देना एक धीमी प्रक्रिया बन सकती है, ऑपरेटिंग सिस्टम की मेमोरी-मैप फ़ाइल कार्यान्वयन के आधार पर। बार-बार आकार बदलने से बचने के लिए, आप इस सेटिंग का उपयोग करके .spb फ़ाइल के अंत में अतिरिक्त स्थान सुरक्षित कर सकते हैं।

मान: आकार, डिफ़ॉल्ट **128k**।

#### docstore_block_size

```ini
docstore_block_size = 32k
```

यह सेटिंग दस्तावेज़ भंडारण द्वारा उपयोग किए जाने वाले ब्लॉकों के आकार को नियंत्रित करती है। डिफ़ॉल्ट मान 16kb है। जब मूल दस्तावेज़ टेक्स्ट को stored_fields या stored_only_fields का उपयोग करके संग्रहित किया जाता है, तो इसे तालिका के भीतर संग्रहीत किया जाता है और कुशलता के लिए संकुचित किया जाता है। छोटे दस्तावेज़ों के लिए डिस्क पहुंच और संकुचन अनुपात को अनुकूलित करने के लिए, इन दस्तावेज़ों को ब्लॉकों में जोड़ दिया जाता है। अनुक्रमण प्रक्रिया दस्तावेज़ों को इकट्ठा करती है जब तक उनकी कुल लंबाई उस थ्रेसहोल्ड तक नहीं पहुँच जाती जो इस विकल्प द्वारा निर्धारित है। उस समय, दस्तावेजों का ब्लॉक संकुचित हो जाता है। इस विकल्प को बेहतर संकुचन अनुपात (ब्लॉक का आकार बढ़ाकर) या दस्तावेज़ टेक्स्ट के लिए तेज़ पहुंच प्राप्त करने (ब्लॉक का आकार कम करके) के लिए समायोजित किया जा सकता है।

मान: आकार, डिफ़ॉल्ट **16k**।

#### docstore_compression

```ini
docstore_compression = lz4hc
```

यह सेटिंग दस्तावेज़ भंडारण में संग्रहीत दस्तावेज़ों के ब्लॉकों को संकुचित करने के लिए उपयोग की जाने वाली संकुचन के प्रकार को निर्धारित करती है। यदि stored_fields या stored_only_fields निर्दिष्ट हैं, तो दस्तावेज़ भंडारण संकुचित दस्तावेज़ ब्लॉकों को संचित करता है। 'lz4' तेज़ संकुचन और डिकम्प्रेशन गति प्रदान करता है, जबकि 'lz4hc' (उच्च संकुचन) बेहतर संकुचन अनुपात के लिए कुछ संकुचन गति को बलिदान करता है। 'none' संकुचन को पूरी तरह से खत्म कर देता है।

मान: **lz4** (डिफ़ॉल्ट), lz4hc, none।

#### docstore_compression_level

```ini
docstore_compression_level = 12
```

जब 'lz4hc' संकुचन दस्तावेज़ भंडारण में लागू किया जाता है, तो उपयोग की जाने वाली संकुचन स्तर। संकुचन स्तर को समायोजित करके, आप 'lz4hc' संकुचन का उपयोग करते समय प्रदर्शन और संकुचन अनुपात के बीच सही संतुलन प्राप्त कर सकते हैं। ध्यान दें कि इस विकल्प का उपयोग 'lz4' संकुचन करते समय लागू नहीं होता है।

मान: 1 और 12 के बीच एक पूर्णांक, डिफ़ॉल्ट **9** है।

#### preopen

```ini
preopen = 1
```

यह सेटिंग इंगित करती है कि खोजd को स्टार्टअप या रोटेशन पर सभी तालिका फ़ाइलों को खोलना चाहिए, और चलते समय उन्हें खुले रखना चाहिए। डिफ़ॉल्ट रूप से, फ़ाइलें पूर्व-खुली नहीं होती हैं। पूर्व-खुली तालिकाओं को प्रत्येक तालिका के लिए कुछ फ़ाइल डिस्क्रिप्टर्स की आवश्यकता होती है, लेकिन वे पर-पूछने open() कॉल की आवश्यकता को खत्म करती हैं और उच्च लोड के तहत तालिका रोटेशन के दौरान हो सकने वाली प्रतिस्पर्धा की स्थितियों से मुक्त होती हैं। हालांकि, यदि आप कई तालिकाएं सेवा कर रहे हैं, तो फ़ाइल डिस्क्रिप्टर्स को संरक्षित करने के लिए उन्हें पर-पूछने के आधार पर खोलना अभी भी अधिक कुशल हो सकता है।

मान: **0** (डिफ़ॉल्ट), या 1।

#### read_buffer_docs

```ini
read_buffer_docs = 1M
```

कीवर्ड प्रति दस्तावेजों की सूची को संग्रहीत करने के लिए बफर का आकार। इस मान को बढ़ाने से क्वेरी निष्पादन के दौरान अधिक मेमोरी उपयोग होगा, लेकिन यह I/O समय को कम कर सकता है।

मान: आकार, डिफ़ॉल्ट **256k**, न्यूनतम मान 8k है।

#### read_buffer_hits

```ini
read_buffer_hits = 1M
```

कीवर्ड प्रति हिट की सूची को संग्रहीत करने के लिए बफर का आकार। इस मान को बढ़ाने से क्वेरी निष्पादन के दौरान अधिक मेमोरी उपयोग होगा, लेकिन यह I/O समय को कम कर सकता है।

मान: आकार, डिफ़ॉल्ट **256k**, न्यूनतम मान 8k है।

### प्लेन टेबल डिस्क फुटप्रिंट सेटिंग्स

#### inplace_enable

<!-- उदाहरण inplace_enable -->

```ini
inplace_enable = {0|1}
```

इन-प्लेस तालिका इनवर्ज़न को सक्षम करता है। वैकल्पिक, डिफ़ॉल्ट 0 है (अलग अस्थायी फ़ाइलों का उपयोग करता है)।

The `inplace_enable` विकल्प साधारण तालिकाओं को अनुक्रमित करते समय डिस्क का आकार कम करता है, जबकि अनुक्रमण को थोड़ा धीमा कर देता है (यह लगभग 2 गुना कम डिस्क उपयोग करता है, लेकिन मूल प्रदर्शन का लगभग 90-95% देता है)।

अनुक्रमण दो प्राथमिक चरणों में विभाजित होता है। पहले चरण के दौरान, दस्तावेज़ इकट्ठा किए जाते हैं, संसाधित किए जाते हैं, और कीवर्ड द्वारा आंशिक रूप से क्रमबद्ध किए जाते हैं, और मध्यवर्ती परिणाम अस्थायी फ़ाइलों (.tmp*) में लिखे जाते हैं। दूसरे चरण के दौरान, दस्तावेज़ पूरी तरह से क्रमबद्ध होते हैं और अंतिम तालिका फ़ाइलें बनाई जाती हैं। उत्पादन तालिका को ऑन-द-फ्लाई फिर से बनाना लगभग 3 गुना अधिकतम डिस्क स्थान की आवश्यकता होती है: पहले अस्थायी मध्यवर्ती फ़ाइलों के लिए, दूसरे नए निर्मित प्रति के लिए, और तीसरे पुराने तालिका के लिए जो इस बीच उत्पादन प्रश्नों का सेवा करेगी। (मध्यवर्ती डेटा आकार में अंतिम तालिका के बराबर होता है।) यह बड़े डेटा संग्रह के लिए बहुत अधिक डिस्क स्थान हो सकता है, और `inplace_enable` विकल्प का उपयोग इसे कम करने के लिए किया जा सकता है। जब सक्षम किया जाता है, तो यह अस्थायी फ़ाइलों का पुनः उपयोग करता है, अंतिम डेटा को फिर से उन पर आउटपुट करता है, और पूर्ण होने पर उनका नाम बदलता है। हालाँकि, इसके लिए अतिरिक्त अस्थायी डेटा भाग स्थानांतरण की आवश्यकता हो सकती है, जिससे प्रदर्शन पर प्रभाव पड़ता है।

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

विकल्प [In-place inversion](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#inplace_enable) ट्यूनिंग विकल्प। पूर्व-आवंटित हिटलिस्ट गैप आकार को नियंत्रित करता है। वैकल्पिक, डिफ़ॉल्ट 0 है।

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
