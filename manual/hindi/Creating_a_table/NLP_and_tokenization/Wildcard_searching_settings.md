# वाईल्डकार्ड खोज सेटिंग्स

वाईल्डकार्ड खोज एक सामान्य टेक्स्ट खोज प्रकार है। मैन्टिकोर में, यह शब्दकोश स्तर पर किया जाता है। डिफ़ॉल्ट रूप से, सामान्य और आरटी तालिकाएँ [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) नामक एक शब्दकोश प्रकार का उपयोग करती हैं। इस मोड में, शब्दों को जैसे हैं वैसे ही स्टोर किया जाता है, इसलिए वाईल्डकार्डिंग सक्षम करना तालिका के आकार को प्रभावित नहीं करता है। जब एक वाईल्डकार्ड खोज की जाती है, तो शब्दकोश को वाईल्डकार्ड किए गए शब्द के सभी संभावित विस्तार खोजने के लिए खोजा जाता है। जब विस्तारित शब्द कई विस्तार या विशाल हिट सूचियों का उत्पादन करता है, विशेष रूप से तब जब वाईल्डकार्ड शब्द के प्रारंभ और अंत में जोड़ा जाता है, तो यह गणना के मामले में समस्या उत्पन्न कर सकता है। ऐसी समस्याओं से बचने के लिए, [expansion_limit](../../Server_settings/Searchd.md#expansion_limit) का उपयोग किया जा सकता है।

## min_prefix_len

<!-- example min_prefix_len -->

```ini
min_prefix_len = length
```

यह सेटिंग अनुक्रमण और खोज के लिए न्यूनतम शब्द उपसर्ग लंबाई का निर्धारण करती है। डिफ़ॉल्ट रूप से, इसे 0 पर सेट किया गया है, जिसका अर्थ है उपसर्ग की अनुमति नहीं है।

उपसर्ग वाईल्डकार्ड खोज के लिए `wordstart*` वाईल्डकार्ड की अनुमति देता है।

उदाहरण के लिए, यदि शब्द "example" को min_prefix_len=3 के साथ अनुक्रमित किया गया है, तो इसे "exa", "exam", "examp", "exampl" के साथ-साथ पूर्ण शब्द द्वारा खोजा जा सकता है।

ध्यान दें कि [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc के साथ min_prefix_len पूर्ण-पाठ अनुक्रमण के आकार को प्रभावित करेगा क्योंकि प्रत्येक शब्द विस्तार अतिरिक्त रूप से संग्रहीत किया जाएगा।

मैन्टिकोर पूर्ण शब्द मिलानों को उपसर्ग मिलानों से अलग कर सकता है और यदि निम्नलिखित शर्तें पूरी होती हैं तो पूर्व को उच्चतर रैंक कर सकता है:
* [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords (डिफ़ॉल्ट रूप से चालू)
* [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words)=1 (डिफ़ॉल्ट रूप से बंद),
* [expand_keywords](../../Searching/Options.md#expand_keywords)=1 (डिफ़ॉल्ट रूप से भी बंद)

ध्यान दें कि यदि [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc मोड या उपरोक्त में से कोई भी विकल्प अक्षम है, तो उपसर्गों और पूर्ण शब्दों के बीच भेद करना संभव नहीं है, और पूर्ण शब्द मिलानों को उच्चतर रैंक नहीं किया जा सकता है।

जब [न्यूनतम इन्फिक्स लंबाई](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) को सकारात्मक संख्या पर सेट किया जाता है, तो न्यूनतम उपसर्ग लंबाई हमेशा 1 मानी जाती है।


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) min_prefix_len = '3'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) min_prefix_len = '3'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'min_prefix_len' => '3'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) min_prefix_len = \'3\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_prefix_len = \'3\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) min_prefix_len = '3'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) min_prefix_len = '3'");
```

<!-- request CONFIG -->

```ini
table products {
  min_prefix_len = 3

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## min_infix_len


<!-- example min_infix_len -->

```ini
min_infix_len = length
```
min_infix_len सेटिंग अनुक्रमण और खोज के लिए इन्फिक्स उपसर्ग की न्यूनतम लंबाई का निर्धारण करती है। यह वैकल्पिक है और इसका डिफ़ॉल्ट मान 0 है, जिसका अर्थ है कि इन्फिक्स की अनुमति नहीं है। अनुमत न्यूनतम गैर-शून्य मान 2 है।

जब सक्षम किया जाता है, तो इन्फिक्स वाईल्डकार्ड खोज के लिए `start*`, `*end`, `*middle*`, आदि जैसे शब्द पैटर्न की अनुमति देते हैं। यह आपको बहुत छोटे वाईल्डकार्ड को अक्षम करने की भी अनुमति देता है यदि उन्हें खोजने में बहुत महंगा हो।

यदि निम्नलिखित शर्तें पूरी होती हैं, तो मैन्टिकोर पूर्ण शब्द मिलानों को इन्फिक्स मिलानों से अलग कर सकता है और पूर्व को उच्चतर रैंक कर सकता है:
* [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=keywords (डिफ़ॉल्ट रूप से चालू)
* [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=1 (डिफ़ॉल्ट रूप से बंद),
* [expand_keywords](../../Searching/Options.md#expand_keywords)=1 (डिफ़ॉल्ट रूप से भी बंद)

ध्यान दें कि [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)=crc मोड या उपरोक्त में से कोई भी विकल्प अक्षम होने पर, इन्फिक्स और पूर्ण शब्दों के बीच भेद करने का कोई तरीका नहीं है, और इस प्रकार पूर्ण शब्द मिलानों को उच्चतर रैंक नहीं किया जा सकता है।

इन्फिक्स वाईल्डकार्ड खोज क्वेरी का समय बहुत भिन्न हो सकता है, इस पर निर्भर करता है कि उपसर्ग वास्तव में कितने कीवर्ड में विस्तारित होगा। छोटे और आवर्ती स्वर जैसे `*in*` या `*ti*` बहुत अधिक कीवर्ड में विस्तारित हो सकते हैं, जिनमें से सभी को मेल खाना और संसाधित करना आवश्यक होगा। इसलिए, सामान्यतः उपसर्ग खोज को सक्षम करने के लिए, आप min_infix_len को 2 पर सेट करेंगे। बहुत छोटे वाईल्डकार्ड के साथ वाईल्डकार्ड खोजों के प्रभाव को सीमित करने के लिए, आप इसे अधिक सेट कर सकते हैं।

इन्फिक्स की लंबाई कम से कम 2 अक्षर होनी चाहिए, और प्रदर्शन कारणों से `*a*` जैसे वाईल्डकार्ड की अनुमति नहीं है।
जब min_infix_len को एक सकारात्मक संख्या पर सेट किया जाता है, तो [न्यूनतम प्रीफिक्स लंबाई](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len) को 1 माना जाता है। [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) शब्दों में इन्फिक्सिंग और प्रिफिक्सिंग दोनों को एक ही समय में सक्षम नहीं किया जा सकता है। [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) और अन्य क्षेत्रों के लिए प्रीफिक्स [prefix_fields](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#prefix_fields) के साथ घोषित किए जाने पर, दोनों सूचियों में उसी क्षेत्र को घोषित करना निषिद्ध है।

यदि dict=keywords है, तो वाइल्डकार्ड `*` के अलावा दो अन्य वाइल्डकार्ड वर्णों का उपयोग किया जा सकता है:
* `?` किसी भी (एक) वर्ण से मेल खा सकता है:  `t?st` `test` से मेल खाएगा, लेकिन `teast` से नहीं
* `%` शून्य या एक वर्ण से मेल खा सकता है:  `tes%` `tes` या `test` से मेल खाएगा, लेकिन `testing` से नहीं


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) min_infix_len = '3'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) min_infix_len = '3'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'min_infix_len' => '3'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) min_infix_len = \'3\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_infix_len = \'3\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) min_infix_len = '3'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) min_infix_len = '3'");
```

<!-- request CONFIG -->

```ini
table products {
  min_infix_len = 3

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## prefix_fields

<!-- example prefix_fields -->

```ini
prefix_fields = field1[, field2, ...]
```

prefix_fields सेटिंग को [dict=crc](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) मोड में विशेष पूर्ण-टेक्स्ट क्षेत्रों के लिए प्रीफिक्स अनुक्रमण को सीमित करने के लिए उपयोग किया जाता है। डिफ़ॉल्ट रूप से, सभी क्षेत्रों को प्रीफिक्स मोड में अनुक्रमित किया जाता है, लेकिन चूंकि प्रीफिक्स अनुक्रमण अनुक्रमण और खोज प्रदर्शन दोनों को प्रभावित कर सकता है, इसे कुछ क्षेत्रों तक सीमित करना वांछित हो सकता है।

विशिष्ट क्षेत्रों के लिए प्रीफिक्स अनुक्रमण को सीमित करने के लिए, prefix_fields सेटिंग का उपयोग करें और उसके बाद एक कॉमा से अलग सूची क्षेत्र नामों की। यदि prefix_fields सेट नहीं किया गया है, तो सभी क्षेत्रों को प्रीफिक्स मोड में अनुक्रमित किया जाएगा।

<!-- intro -->
##### CONFIG:

<!-- request CONFIG -->

```ini
table products {
  prefix_fields = title, name
  min_prefix_len = 3
  dict = crc
```
<!-- end -->

## infix_fields

<!-- example infix_fields -->

```ini
infix_fields = field1[, field2, ...]
```

infix_fields सेटिंग आपको पूर्ण-टेक्स्ट क्षेत्रों की एक सूची निर्दिष्ट करने की अनुमति देती है जिसकी इन्फिक्स अनुक्रमण को सीमित किया जा सके। यह केवल [dict=crc](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) पर लागू होता है और यह वैकल्पिक है, जिससे सभी क्षेत्रों को इन्फिक्स मोड में अनुक्रमित किया जाता है।
यह सेटिंग [prefix_fields](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) के समान है, लेकिन इसके बजाय आपको इन्फिक्स अनुक्रमण को कुछ विशेष क्षेत्रों तक सीमित करने की अनुमति देता है।


<!-- intro -->
##### CONFIG:

<!-- request CONFIG -->

```ini
table products {
  infix_fields = title, name
  min_infix_len = 3
  dict = crc
```
<!-- end -->

## max_substring_len

<!-- example max_substring_len -->

```ini
max_substring_len = length
```

max_substring_len निर्देश यह निर्धारित करता है कि प्रीफिक्स या इन्फिक्स खोजों के लिए अधिकतम उपस्ट्रिंग लंबाई अनुक्रमित की जाए। यह सेटिंग वैकल्पिक है, और इसका डिफ़ॉल्ट मान 0 है (जिसका अर्थ है कि सभी संभावित उपस्ट्रिंग को अनुक्रमित किया गया है)। यह केवल [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) पर लागू होता है।

डिफ़ॉल्ट रूप से, [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict) में उपस्ट्रिंग अनुक्रमण **सभी** संभावित उपस्ट्रिंग को अलग कीवर्ड के रूप में अनुक्रमित करता है, जो कि पूर्ण-टेक्स्ट अनुक्रमण अनुक्रमण का एक अत्यधिक बड़ा रूप हो सकता है। इसलिए, max_substring_len निर्देश आपको उन बहुत लंबी उपस्ट्रिंग को छोड़ने की अनुमति देता है जिनकी संभावना है कि उन्हें कभी भी खोजा नहीं जाएगा।

उदाहरण के लिए, 10,000 ब्लॉग पोस्ट के परीक्षण तालिका विभिन्न सेटिंग्स के आधार पर विभिन्न मात्रा में डिस्क स्पेस लेती है:
* 6.4 MB आधार (कोई उपस्ट्रिंग नहीं)
* 24.3 MB (3.8x) min_prefix_len = 3 के साथ
* 22.2 MB (3.5x) min_prefix_len = 3, max_substring_len = 8 के साथ
* 19.3 MB (3.0x) min_prefix_len = 3, max_substring_len = 6 के साथ
* 94.3 MB (14.7x) min_infix_len = 3 के साथ
* 84.6 MB (13.2x) min_infix_len = 3, max_substring_len = 8 के साथ
* 70.7 MB (11.0x) min_infix_len = 3, max_substring_len = 6 के साथ

इसलिए, अधिकतम उपस्ट्रिंग लंबाई को सीमित करना तालिका आकार के 10-15% को बचा सकता है।

जब dict=keywords मोड का उपयोग कर रहे हैं, तो उपस्ट्रिंग लंबाई से संबंधित कोई प्रदर्शन प्रभाव नहीं होता है। इसलिए, यह निर्देश लागू नहीं है और जानबूझकर उस स्थिति में निषिद्ध है। हालाँकि, यदि आवश्यक हो, तो आप अभी भी अपने एप्लिकेशन कोड में खोजे गए उपस्ट्रिंग की लंबाई को सीमित कर सकते हैं।


<!-- intro -->
##### CONFIG:

<!-- request CONFIG -->

```ini
table products {
  max_substring_len = 12
  min_infix_len = 3
  dict = crc
```
<!-- end -->

## expand_keywords

<!-- example expand_keywords -->

```ini
expand_keywords = {0|1|exact|star}
```

यह सेटिंग कीवर्ड्स को उनके सटीक रूपों और/या संभव होने पर सितारों के साथ विस्तारित करती है। समर्थित मान हैं:
* 1 - दोनों सटीक रूप और सितारों के साथ रूप में विस्तारित करें। उदाहरण के लिए, `running` `(running | *running* | =running)` में बदल जाएगा।
* `exact` - कीवर्ड को केवल इसके सटीक रूप के साथ बढ़ाएं। उदाहरण के लिए, `running` `(running | =running)` में बदल जाएगा।
* `star` - कीवर्ड के चारों ओर `*` जोड़कर कीवर्ड को बढ़ाएँ। उदाहरण के लिए, `running` `(running | *running*)` में बदल जाएगा।
यह सेटिंग वैकल्पिक है, और डिफ़ॉल्ट मान 0 है (कीवर्ड का विस्तार नहीं किया गया)।

`expand_keywords` फीचर सक्षम टेबल्स के खिलाफ क्वेरियों को आंतरिक रूप से निम्नलिखित तरीके से विस्तारित किया जाता है: यदि टेबल को पूर्वसूचक या अंतसूचक अनुक्रमण सक्षम किया गया था, तो प्रत्येक कीवर्ड को उसके स्वयं और संबंधित पूर्वसूचक या अंतसूचक (कीवर्ड के साथ सितारे) के विलोपन के साथ स्वतः बदल दिया जाता है। यदि टेबल को दोनों स्टेमिंग और [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words) सक्षम किया गया है, तो सटीक रूप भी जोड़ा जाता है।

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) expand_keywords = '1'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) expand_keywords = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'expand_keywords' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) expand_keywords = \'1\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) expand_keywords = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) expand_keywords = '1'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) expand_keywords = '1'");
```

<!-- request CONFIG -->

```ini
table products {
  expand_keywords = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

<!-- example expand_keywords2 -->

विस्तारित क्वेरियाँ स्वाभाविक रूप से पूरी होने में अधिक समय ले सकती हैं, लेकिन यह खोज की गुणवत्ता में सुधार कर सकती हैं, क्योंकि सटीक रूप मेल खाने वाले दस्तावेज सामान्यतः स्टेम किए गए या अंतसूचक मेल खाने वाले दस्तावेजों की तुलना में उच्च रैंक किए जाने चाहिए।

**ध्यान दें कि मौजूदा क्वेरी सिंटैक्स इस प्रकार के विस्तार की नकल करने की अनुमति नहीं देता**, क्योंकि आंतरिक विस्तार कीवर्ड स्तर पर काम करता है और वाक्यांश या मतदान ऑपरेटरों के भीतर भी कीवर्ड का विस्तार करता है (जो क्वेरी सिंटैक्स के माध्यम से संभव नहीं है)। उदाहरणों पर एक नज़र डालें और कैसे expand_keywords खोज परिणाम भारों को प्रभावित करता है और कैसे "runsy" को "runs" द्वारा खोजा जाता है बिना सितारे जोड़ने की आवश्यकता:

<!-- intro -->
##### expand_keywords सक्षम है
<!-- request expand_keywords_enabled -->
```sql
mysql> create table t(f text) min_infix_len='2' expand_keywords='1' morphology='stem_en';
Query OK, 0 rows affected, 1 warning (0.00 sec)

mysql> insert into t values(1,'running'),(2,'runs'),(3,'runsy');
Query OK, 3 rows affected (0.00 sec)

mysql> select *, weight() from t where match('runs');
+------+---------+----------+
| id   | f       | weight() |
+------+---------+----------+
|    2 | runs    |     1560 |
|    1 | running |     1500 |
|    3 | runsy   |     1500 |
+------+---------+----------+
3 rows in set (0.01 sec)

mysql> drop table t;
Query OK, 0 rows affected (0.01 sec)

mysql> create table t(f text) min_infix_len='2' expand_keywords='exact' morphology='stem_en';
Query OK, 0 rows affected, 1 warning (0.00 sec)

mysql> insert into t values(1,'running'),(2,'runs'),(3,'runsy');
Query OK, 3 rows affected (0.00 sec)

mysql> select *, weight() from t where match('running');
+------+---------+----------+
| id   | f       | weight() |
+------+---------+----------+
|    1 | running |     1590 |
|    2 | runs    |     1500 |
+------+---------+----------+
2 rows in set (0.00 sec)
```

<!-- intro -->
##### expand_keywords अक्षम है

<!-- request expand_keywords_disabled -->

```sql
mysql> create table t(f text) min_infix_len='2' morphology='stem_en';
Query OK, 0 rows affected, 1 warning (0.00 sec)

mysql> insert into t values(1,'running'),(2,'runs'),(3,'runsy');
Query OK, 3 rows affected (0.00 sec)

mysql> select *, weight() from t where match('runs');
+------+---------+----------+
| id   | f       | weight() |
+------+---------+----------+
|    1 | running |     1500 |
|    2 | runs    |     1500 |
+------+---------+----------+
2 rows in set (0.00 sec)

mysql> drop table t;
Query OK, 0 rows affected (0.01 sec)

mysql> create table t(f text) min_infix_len='2' morphology='stem_en';
Query OK, 0 rows affected, 1 warning (0.00 sec)

mysql> insert into t values(1,'running'),(2,'runs'),(3,'runsy');
Query OK, 3 rows affected (0.00 sec)

mysql> select *, weight() from t where match('running');
+------+---------+----------+
| id   | f       | weight() |
+------+---------+----------+
|    1 | running |     1500 |
|    2 | runs    |     1500 |
+------+---------+----------+
2 rows in set (0.00 sec)
```
<!-- end -->

यह निर्देष किसी भी तरह [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) को प्रभावित नहीं करता है, यह केवल [searchd](../../Starting_the_server/Manually.md) को प्रभावित करता है।
## विस्तार_सीमा

```ini
विस्तार_सीमा = संख्या
```

एकल वाइल्डकार्ड के लिए विस्तारित कुंजियों की अधिकतम संख्या। विवरण [यहाँ](../../Server_settings/Searchd.md#expansion_limit) हैं।

<!-- proofread -->



