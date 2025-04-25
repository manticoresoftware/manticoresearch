# उन्नत HTML टोकनाइजेशन
## HTML टैग को हटाना
### html_strip
<!-- example html_strip -->
```ini
html_strip = {0|1}
```
यह विकल्प निर्धारित करता है कि इनकमिंग पूर्ण-पाठ डेटा से HTML मार्कअप हटाया जाना चाहिए या नहीं। डिफ़ॉल्ट मान 0 है, जो हटाने को अक्षम करता है। हटाने को सक्षम करने के लिए, मान को 1 पर सेट करें।
HTML टैग और एन्टिटीज को मार्कअप के रूप में माना जाता है और उन्हें प्रोसेस किया जाएगा।
HTML टैग हटा दिए जाते हैं, जबकि उनके बीच की सामग्री (जैसे `<p>` और `</p>` के बीच सब कुछ) अछूती रहती है। आप टैग के गुणों को रखने और अनुक्रमित करने का चयन कर सकते हैं (जैसे A टैग में HREF गुण या IMG टैग में ALT)। कुछ प्रसिद्ध इनलाइन टैग, जैसे A, B, I, S, U, BASEFONT, BIG, EM, FONT, IMG, LABEL, SMALL, SPAN, STRIKE, STRONG, SUB, SUP, और TT, पूरी तरह से हटा दिए जाते हैं। सभी अन्य टैग को ब्लॉक स्तर के रूप में माना जाता है और उन्हें खाली स्थान से बदला जाता है। उदाहरण के लिए, टेक्स्ट `te<b>st</b>` को एकल कीवर्ड 'test' के रूप में अनुक्रमित किया जाएगा, जबकि `te<p>st</p>` को दो कीवर्ड 'te' और 'st' के रूप में अनुक्रमित किया जाएगा।
HTML एन्टिटीज को डिकोड किया जाता है और उनके संबंधित UTF-8 अक्षरों के साथ बदल दिया जाता है। स्ट्रिपर दोनों संख्यात्मक रूपों (जैसे `&#239;`) और पाठ रूपों (जैसे `&oacute;` या `&nbsp;`) के एन्टिटीज का समर्थन करता है, और HTML4 मानक द्वारा निर्दिष्ट सभी एन्टिटीज का समर्थन करता है।
स्ट्रिपर को उचित रूप से बनाए गए HTML और XHTML के साथ काम करने के लिए डिज़ाइन किया गया है, लेकिन यह बिगड़े हुए इनपुट (जैसे HTML जिसमें बेतरतीब `<'s` या अनक्लोज़ `>'s` हों) पर अप्रत्याशित परिणाम दे सकता है।
कृपया ध्यान दें कि केवल टैग स्वयं, साथ ही HTML टिप्पणियाँ, हटाई जाती हैं। टैग की सामग्री को हटाने के लिए, जिसमें एम्बेडेड स्क्रिप्ट शामिल हैं, [html_remove_elements](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_remove_elements) विकल्प देखें। टैग नामों पर कोई प्रतिबंध नहीं हैं, अर्थात् जो कुछ भी एक मान्य टैग प्रारंभ, समाप्ति, या टिप्पणी की तरह दिखता है, उसे हटा दिया जाएगा।
<!-- intro -->
##### SQL:
<!-- request SQL -->
```sql
CREATE TABLE products(title text, price float) html_strip = '1'
```
<!-- request JSON -->
```json
POST /cli -d "
CREATE TABLE products(title text, price float) html_strip = '1'"
```
<!-- request PHP -->
```php
$index = new ManticoresearchIndex($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'html_strip' => '1'
        ]);
```
<!-- intro -->
##### Python:
<!-- request Python -->
```python
utilsApi.sql('CREATE TABLE products(title text, price float) html_strip = '1'')
```
<!-- intro -->
##### Python-asyncio:
<!-- request Python-asyncio -->
```python
await utilsApi.sql('CREATE TABLE products(title text, price float) html_strip = '1'')
```
<!-- intro -->
##### Javascript:
<!-- request javascript -->
```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) html_strip = '1'');
```
<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) html_strip = '1'");
```
<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) html_strip = '1'");
```
<!-- intro -->
##### Rust:
<!-- request Rust -->
```rust
utils_api.sql("CREATE TABLE products(title text, price float) html_strip = '1'", Some(true)).await;
```
<!-- request CONFIG -->
```ini
table products {
  html_strip = 1
  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->
### html_index_attrs
<!-- example html_index_attrs -->
```ini
html_index_attrs = img=alt,title; a=title;
```
html_index_attrs विकल्प आपको यह निर्दिष्ट करने की अनुमति देता है कि कौन से HTML मार्कअप गुणों को अनुक्रमित किया जाना चाहिए, भले ही अन्य HTML मार्कअप को हटा दिया जाए। डिफ़ॉल्ट मान खाली है, जिसका अर्थ है कि कोई गुण अनुक्रमित नहीं किया जाएगा।
विकल्प का प्रारूप प्रत्येक टैग के अनुक्रमित गुणों की एक सूची है, जैसा कि उपरोक्त उदाहरण में प्रदर्शित किया गया है। निर्दिष्ट गुणों की सामग्री को रखा जाएगा और अनुक्रमित किया जाएगा, जो आपके पूर्ण-पाठ डेटा से अतिरिक्त जानकारी निकालने का एक तरीका प्रदान करता है।
<!-- intro -->
##### SQL:
<!-- request SQL -->
```sql
CREATE TABLE products(title text, price float) html_index_attrs = 'img=alt,title; a=title;' html_strip = '1'
```
<!-- request JSON -->
```json
POST /cli -d "
CREATE TABLE products(title text, price float) html_index_attrs = 'img=alt,title; a=title;' html_strip = '1'"
```
<!-- request PHP -->
```php
$index = new ManticoresearchIndex($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'html_index_attrs' => 'img=alt,title; a=title;',
            'html_strip' => '1'
        ]);
```
<!-- intro -->
##### Python:
<!-- request Python -->
```python
utilsApi.sql('CREATE TABLE products(title text, price float) html_index_attrs = 'img=alt,title; a=title;' html_strip = '1'')
```
<!-- intro -->
##### Python-asyncio:
<!-- request Python-asyncio -->
```python
await utilsApi.sql('CREATE TABLE products(title text, price float) html_index_attrs = 'img=alt,title; a=title;' html_strip = '1'')
```
<!-- intro -->
##### Javascript:
<!-- request javascript -->
```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) html_index_attrs = 'img=alt,title; a=title;' html_strip = '1'');
```
<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) html_index_attrs = 'img=alt,title; a=title;' html_strip = '1'");
```
<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) html_index_attrs = 'img=alt,title; a=title;' html_strip = '1'");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) html_index_attrs = 'img=alt,title; a=title;' html_strip = '1'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  html_index_attrs = img=alt,title; a=title;
  html_strip = 1
  
  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### html_remove_elements

<!-- example html_remove_elements -->

```ini
html_remove_elements = element1[, element2, ...]
```

HTML तत्वों की एक सूची जिनकी सामग्री, तत्वों के साथ-साथ, हटा दी जाएगी। वैकल्पिक, डिफ़ॉल्ट खाली स्ट्रिंग है (किसी भी तत्व की सामग्री को हटा न दें)।

यह विकल्प आपको तत्वों की सामग्री को हटाने की अनुमति देता है, जिसका अर्थ है खोलने और बंद करने वाले टैग के बीच की सभी चीजें। यह एम्बेडेड स्क्रिप्ट, CSS आदि को हटाने के लिए उपयोगी है। खाली तत्वों के लिए छोटे टैग प्रारूप (जैसे <br/>) को ठीक से समर्थित किया जाता है, और ऐसे टैग के बाद का टेक्स्ट नहीं हटाया जाएगा।

मान तत्व (टैग) नामों की एक अल्पविराम-अलग सूची है, जिनकी सामग्री को हटाया जाना चाहिए। टैग के नाम अक्षर-संवेदनशील नहीं हैं।


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) html_remove_elements = 'style, script' html_strip = '1'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) html_remove_elements = 'style, script' html_strip = '1'"
```

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'html_remove_elements' => 'style, script',
            'html_strip' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) html_remove_elements = 'style, script' html_strip = '1'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) html_remove_elements = 'style, script' html_strip = '1'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) html_remove_elements = 'style, script' html_strip = '1'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) html_remove_elements = 'style, script' html_strip = '1'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) html_remove_elements = 'style, script' html_strip = '1'");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) html_remove_elements = 'style, script' html_strip = '1'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  html_remove_elements = style, script
  html_strip = 1
  
  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## HTML से महत्वपूर्ण भाग निकालना

### index_sp

<!-- example index_sp -->

```ini
index_sp = {0|1}
```

वाक्य और अनुच्छेद सीमाओं का पता लगाने और अनुक्रमण करने को नियंत्रित करता है। वैकल्पिक, डिफ़ॉल्ट 0 है (कोई पता लगाना या अनुक्रमण नहीं)।

यह निर्देश वाक्य और अनुच्छेद सीमाओं का पता लगाने और अनुक्रमण करने को सक्षम बनाता है, जिससे [SENTENCE](../../Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators) और [PARAGRAPH](../../Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators) ऑपरेटर काम कर सकते हैं। वाक्य सीमा पता लगाने साधारण पाठ विश्लेषण पर आधारित है, और केवल `index_sp = 1` सेट करके इसे सक्षम किया जा सकता है। हालांकि, अनुच्छेद पता लगाने HTML मार्कअप पर निर्भर करता है और [HTML स्ट्रिपिंग प्रक्रिया](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip) के दौरान होता है। इसलिए, अनुच्छेद सीमाओं को अनुक्रमित करने के लिए, index_sp निर्देश और html_strip निर्देश दोनों को 1 पर सेट किया जाना चाहिए।

वाक्य सीमाओं को निर्धारित करने के लिए निम्न नियम उपयोग किए जाते हैं:

* प्रश्न चिह्न (?) और विस्मय चिह्न (!) हमेशा एक वाक्य सीमा इंगित करते हैं।
* ट्रेलिंग बिंदु (.) एक वाक्य सीमा इंगित करते हैं, सिवाय निम्न मामलों के:
    * जब अक्षर के बाद आता है। इसे एक संक्षिप्त नाम माना जाता है (जैसे "S.T.A.L.K.E.R." या "Goldman Sachs S.p.A.")।
    * जब अल्पविराम के बाद आता है। इसे एक अल्पविराम के साथ संक्षिप्त नाम माना जाता है (जैसे "Telecom Italia S.p.A., 1994 में स्थापित")।
    * जब स्पेस और एक लोअरकेस अक्षर के बाद आता है। इसे एक वाक्य के भीतर एक संक्षिप्त नाम माना जाता है (जैसे "News Corp. ने फरवरी में घोषणा की")।
    * जब स्पेस और एक बड़े अक्षर के पहले, और स्पेस के बाद आता है। इसे एक मध्य आद्याक्षर माना जाता है (जैसे "John D. Doe")।
        
अनुच्छेद सीमाएं हर ब्लॉक-स्तर के HTML टैग पर पता लगाई जाती हैं, जिनमें शामिल हैं: ADDRESS, BLOCKQUOTE, CAPTION, CENTER, DD, DIV, DL, DT, H1, H2, H3, H4, H5, LI, MENU, OL, P, PRE, TABLE, TBODY, TD, TFOOT, TH, THEAD, TR, और UL।

वाक्य और अनुच्छेद दोनों कीवर्ड स्थिति काउंटर को 1 से बढ़ाते हैं।


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) index_sp = '1' html_strip = '1'
```

<!-- request JSON -->

```json
POST /cli -d "
उत्पाद बनाएँ (title text, price float) index_sp = '1' html_strip = '1'"
```

<!-- अनुरोध PHP -->

```php
$index = new ManticoresearchIndex($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'index_sp' => '1',
            'html_strip' => '1'
        ]);
```
<!-- आरंभ -->
##### पायथन:

<!-- अनुरोध पायथन -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) index_sp = '1' html_strip = '1'')
```

<!-- आरंभ -->
##### पायथन-ऐसिंक:

<!-- अनुरोध पायथन-ऐसिंक -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) index_sp = '1' html_strip = '1'')
```

<!-- आरंभ -->
##### जावास्क्रिप्ट:

<!-- अनुरोध जावास्क्रिप्ट -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_sp = '1' html_strip = '1'');
```

<!-- आरंभ -->
##### जावा:
<!-- अनुरोध जावा -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_sp = '1' html_strip = '1'", true);
```

<!-- आरंभ -->
##### C#:
<!-- अनुरोध C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_sp = '1' html_strip = '1'", true);
```

<!-- आरंभ -->
##### रस्ट:

<!-- अनुरोध रस्ट -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) index_sp = '1' html_strip = '1'", Some(true)).await;
```

<!-- अनुरोध CONFIG -->

```ini
table products {
  index_sp = 1
  html_strip = 1
  
  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- समाप्त -->


### index_zones

<!-- उदाहरण index_zones -->

```ini
index_zones = h*, th, title
```

एक फ़ील्ड के भीतर HTML/XML क्षेत्रों की सूची जो अनुक्रमित किए जाने हैं। डिफ़ॉल्ट एक खाली स्ट्रिंग है (कोई क्षेत्र अनुक्रमित नहीं होंगे)।

एक "क्षेत्र" को एक मानक उद्घाटन और एक मेल खाने वाले समापन टैग के बीच सब कुछ के रूप में परिभाषित किया गया है, और सभी स्पैन जो समान टैग नाम साझा करते हैं, उन्हें "क्षेत्र" के रूप में संदर्भित किया जाता है। उदाहरण के लिए, एक दस्तावेज़ क्षेत्र में `<H1>` और `</H1>` के बीच सब कुछ H1 क्षेत्र में आता है।

`index_zones` निर्देश क्षेत्र अनुक्रमण को सक्षम करता है, लेकिन HTML [stripper](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip) को भी सक्षम किया जाना चाहिए (सेट करके `html_strip = 1`)। `index_zones` का मान टैग नामों और वाइल्डकार्ड्स (जो एक एस्टरिस्क के साथ समाप्त होता है) की एक अल्पविराम से पृथक सूची होनी चाहिए, जो क्षेत्रों के रूप में अनुक्रमित होंगे।

क्षेत्र को नेस्ट और ओवरलैप किया जा सकता है, जब तक हर उद्घाटन टैग के पास एक मेल खाने वाला टैग हो। क्षेत्रों का उपयोग ZONE ऑपरेटर के साथ मेल खाने के लिए भी किया जा सकता है, जैसा कि [extended_query_syntax](../../Searching/Full_text_matching/Operators.md#ZONE-limit-operator) में वर्णित है।


<!-- आरंभ -->
##### SQL:

<!-- अनुरोध SQL -->

```sql
CREATE TABLE products(title text, price float) index_zones = 'h, th, title' html_strip = '1'
```

<!-- अनुरोध JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) index_zones = 'h, th, title' html_strip = '1'"
```

<!-- अनुरोध PHP -->

```php
$index = new ManticoresearchIndex($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'index_zones' => 'h*,th,title',
            'html_strip' => '1'
        ]);
```
<!-- आरंभ -->
##### पायथन:

<!-- अनुरोध पायथन -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) index_zones = 'h, th, title' html_strip = '1'')
```

<!-- आरंभ -->
##### पायथन-ऐसिंक:

<!-- अनुरोध पायथन-ऐसिंक -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) index_zones = 'h, th, title' html_strip = '1'')
```

<!-- आरंभ -->
##### जावास्क्रिप्ट:

<!-- अनुरोध जावास्क्रिप्ट -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_zones = 'h, th, title' html_strip = '1'');
```

<!-- आरंभ -->
##### जावा:
<!-- अनुरोध जावा -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_zones = 'h, th, title' html_strip = '1'", true);
```

<!-- आरंभ -->
##### C#:
<!-- अनुरोध C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_zones = 'h, th, title' html_strip = '1'", true);
```

<!-- आरंभ -->
##### रस्ट:

<!-- अनुरोध रस्ट -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) index_zones = 'h, th, title' html_strip = '1'", Some(true)).await;
```

<!-- अनुरोध CONFIG -->

```ini
table products {
  index_zones = h*, th, title
  html_strip = 1
  
  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- समाप्त -->
<!-- प्रूफरीड -->
























































# उन्नत HTML टोकनाइजेशन

## HTML टैग हटाना

### html_strip

<!-- example html_strip -->

```ini
html_strip = {0|1}
```

यह विकल्प निर्धारित करता है कि क्या HTML मार्कअप को आने वाले पूर्ण-पाठ डेटा से हटाया जाना चाहिए। डिफ़ॉल्ट मान 0 है, जो हटाने को निष्क्रिय करता है। हटाने को सक्षम करने के लिए, मान को 1 पर सेट करें।

HTML टैग और एंटिटीज़ को मार्कअप माना जाता है और इसे प्रोसेस किया जाएगा। 

HTML टैग हटा दिए जाते हैं, जबकि उनके बीच की सामग्री (उदाहरण: `<p>` और `</p>` के बीच की सब कुछ) बरकरार रखी जाती है। आप टैग विशेषताओं को रखने और अनुक्रमित करने का विकल्प चुन सकते हैं (उदाहरण: A टैग में HREF विशेषता या IMG टैग में ALT)। कुछ प्रसिद्ध इनलाइन टैग, जैसे A, B, I, S, U, BASEFONT, BIG, EM, FONT, IMG, LABEL, SMALL, SPAN, STRIKE, STRONG, SUB, SUP, और TT, पूरी तरह से हटा दिए जाते हैं। सभी अन्य टैग को ब्लॉक स्तर माना जाता है और इसे व्हाइटस्पेस से बदला जाता है। उदाहरण के लिए, टेक्स्ट `te<b>st</b>` को एकल कीवर्ड 'test' के रूप में अनुक्रमित किया जाएगा, जबकि `te<p>st</p>` को दो कीवर्ड 'te' और 'st' के रूप में अनुक्रमित किया जाएगा।

HTML एंटिटीज़ को डिकोड किया जाता है और उनके संबंधित UTF-8 पात्रों के साथ बदला जाता है। स्ट्रिपर दोनों संख्यात्मक रूप (जैसे `&#239;`) और पाठ रूप (जैसे `&oacute;` या `&nbsp;`) के एंटिटीज़ का समर्थन करता है, और HTML4 मानक द्वारा निर्दिष्ट सभी एंटिटीज़ का समर्थन करता है।

स्ट्रिपर को ठीक से बनाए गए HTML और XHTML के साथ काम करने के लिए डिज़ाइन किया गया है, लेकिन यह गलत प्रारूपित इनपुट (जैसे HTML जिसमें बेतरतीब `<'s` या अनियंत्रित `>'s`) पर अप्रत्याशित परिणाम दे सकता है।

कृपया ध्यान दें कि केवल स्वयं टैग के साथ-साथ HTML टिप्पणियाँ हटाई जाती हैं। टैग की सामग्री को हटाने के लिए, जिसमें एम्बेडेड स्क्रिप्ट शामिल हैं, [html_remove_elements](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_remove_elements) विकल्प को देखें। टैग नामों पर कोई प्रतिबंध नहीं है, जिसका अर्थ है कि जो कुछ भी एक वैध टैग प्रारंभ, अंत, या टिप्पणी के रूप में दिखता है, उसे हटा दिया जाएगा।


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) html_strip = '1'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) html_strip = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'html_strip' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) html_strip = \'1\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) html_strip = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) html_strip = '1'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) html_strip = '1'");
```

<!-- request CONFIG -->

```ini
table products {
  html_strip = 1
  
  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### html_index_attrs

<!-- example html_index_attrs -->

```ini
html_index_attrs = img=alt,title; a=title;
```

html_index_attrs विकल्प आपको यह निर्दिष्ट करने की अनुमति देता है कि कौन से HTML मार्कअप विशेषताएँ अनुक्रमित की जानी चाहिए हालांकि अन्य HTML मार्कअप को हटा दिया गया है। डिफ़ॉल्ट मान खाली है, जिसका अर्थ है कि कोई विशेषताएँ अनुक्रमित नहीं की जाएँगी। 
विकल्प का प्रारूप एक टैग-प्रतिगणना है जिसमें अनुक्रमित विशेषताओं की सूची होती है, जैसा कि ऊपर दिए गए उदाहरण में दर्शाया गया है। निर्दिष्ट विशेषताओं की सामग्री को बनाए रखा जाएगा और अनुक्रमित किया जाएगा, जिससे आपके पूर्ण-पाठ डेटा से अतिरिक्त जानकारी निकालने का एक तरीका प्रदान किया जा सके।


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) html_index_attrs = 'img=alt,title; a=title;' html_strip = '1'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) html_index_attrs = 'img=alt,title; a=title;' html_strip = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'html_index_attrs' => 'img=alt,title; a=title;',
            'html_strip' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) html_index_attrs = \'img=alt,title; a=title;\' html_strip = \'1\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) html_index_attrs = \'img=alt,title; a=title;\' html_strip = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) html_index_attrs = \'img=alt,title; a=title;\' html_strip = '1'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) html_index_attrs = \'img=alt,title; a=title;\' html_strip = '1'");
```

<!-- request CONFIG -->

```ini
table products {
  html_index_attrs = img=alt,title; a=title;
  html_strip = 1
  
  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### html_remove_elements

<!-- example html_remove_elements -->

```ini
html_remove_elements = element1[, element2, ...]
```
A list of HTML elements whose contents, along with the elements themselves, will be stripped. Optional, the default is an empty string (do not strip contents of any elements).

यह विकल्प आपको तत्वों की सामग्री को हटाने की अनुमति देता है, जिसका अर्थ है कि उद्घाटन और बंद टैग के बीच सब कुछ। यह अंतर्निहित स्क्रिप्ट, CSS आदि को हटाने के लिए उपयोगी है। खाली तत्वों के लिए संक्षिप्त टैग रूप (जैसे <br/>) को ठीक से समर्थन किया गया है, और ऐसे टैग के बाद का पाठ नहीं हटाया जाएगा।

मूल्य तत्व (टैग) नामों की एक अल्पविराम से भरी सूची है, जिनकी सामग्री को हटाया जाना चाहिए। टैग नाम केस-गुणवत्ता के प्रति संवेदनशील नहीं होते हैं।


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) html_remove_elements = 'style, script' html_strip = '1'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) html_remove_elements = 'style, script' html_strip = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'html_remove_elements' => 'style, script',
            'html_strip' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) html_remove_elements = \'style, script\' html_strip = \'1\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) html_remove_elements = \'style, script\' html_strip = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) html_remove_elements = \'style, script\' html_strip = '1'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) html_remove_elements = \'style, script\' html_strip = '1'");
```

<!-- request CONFIG -->

```ini
table products {
  html_remove_elements = style, script
  html_strip = 1
  
  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## Extracting important parts from HTML

### index_sp

<!-- example index_sp -->

```ini
index_sp = {0|1}
```

Controls detection and indexing of sentence and paragraph boundaries. Optional, default is 0 (no detection or indexing).

यह निर्देशन वाक्य और अनुच्छेद सीमाओं के निर्धारण और अनुक्रमण की अनुमति देता है, जिससे [SENTENCE](../../Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators) और [PARAGRAPH](../../Searching/Full_text_matching/Operators.md#SENTENCE-and-PARAGRAPH-operators) ऑपरेटर काम कर सकें। वाक्य सीमा का निर्धारण सामान्य पाठ विश्लेषण पर आधारित है, और इसे सक्षम करने के लिए केवल `index_sp = 1` सेट करना आवश्यक है। हालांकि, अनुच्छेद का निर्धारण HTML मार्कअप पर निर्भर करता है और [HTML स्ट्रिपिंग प्रक्रिया](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip) के दौरान होता है। इसलिए, अनुच्छेद सीमाओं को अनुक्रमित करने के लिए, दोनों index_sp निर्देशन और html_strip निर्देशन को 1 पर सेट किया जाना चाहिए।

वाक्य की सीमाओं को निर्धारित करने के लिए निम्नलिखित नियमों का उपयोग किया जाता है:

* प्रश्न चिह्न (?) और विस्मयादिबोधक चिह्न (!) हमेशा वाक्य सीमा का संकेत देते हैं। 
* अंत में बिंदियाँ (.) वाक्य की सीमा को संकेत करती हैं, सिवाय निम्नलिखित मामलों में:
    * जब एक अक्षर के द्वारा अनुसरण किया जाता है। इसे संक्षिप्त रूप का हिस्सा माना जाता है (जैसे "S.T.A.L.K.E.R." या "Goldman Sachs S.p.A.")।
    * जब एक अल्पविराम द्वारा अनुसरण किया जाता है। इसे अल्पविराम के साथ अनुसरण किया हुआ संक्षिप्त रूप माना जाता है (जैसे "Telecom Italia S.p.A., founded in 1994")।
    * जब एक स्पेस और एक छोटे अक्षर द्वारा अनुसरण किया जाता है। इसे एक वाक्य के भीतर संक्षिप्त रूप माना जाता है (जैसे "News Corp. announced in February")।
    * जब एक स्पेस और एक बड़े अक्षर के द्वारा पहले आ रहा होता है, और फिर एक स्पेस द्वारा अनुसरण किया जाता है। इसे एक मध्यवर्ती नाम माना जाता है (जैसे "John D. Doe")।
        
अनुच्छेद की सीमाएँ प्रत्येक ब्लॉक-स्तरीय HTML टैग पर निर्धारित की जाती हैं, जिनमें शामिल हैं: ADDRESS, BLOCKQUOTE, CAPTION, CENTER, DD, DIV, DL, DT, H1, H2, H3, H4, H5, LI, MENU, OL, P, PRE, TABLE, TBODY, TD, TFOOT, TH, THEAD, TR, और UL।

दोनों वाक्य और अनुच्छेद कीवर्ड स्थिति काउंटर को 1 से बढ़ाते हैं।


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) index_sp = '1' html_strip = '1'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) index_sp = '1' html_strip = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'index_sp' => '1',
            'html_strip' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) index_sp = \'1\' html_strip = \'1\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_sp = \'1\' html_strip = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_sp = \'1\' html_strip = '1'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_sp = \'1\' html_strip = '1'");
```

<!-- request CONFIG -->

```ini
table products {
  index_sp = 1
  html_strip = 1
  
  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->


### index_zones

<!-- example index_zones -->

```ini
index_zones = h*, th, title
```

HTML/XML क्षेत्र की सूची जिसे अनुक्रमित किया जाना है। डिफ़ॉल्ट एक खाली स्ट्रिंग है (कोई क्षेत्र अनुक्रमित नहीं होंगे)।

"क्षेत्र" को एक उद्घाटन और एक मेल खाने वाले समापन टैग के बीच सब कुछ के रूप में परिभाषित किया गया है, और सभी स्पैन जो समान टैग नाम साझा करते हैं उन्हें "क्षेत्र" कहा जाता है। उदाहरण के लिए, दस्तावेज़ क्षेत्र में `<H1>` और `</H1>` के बीच सब कुछ H1 क्षेत्र में आता है।

`index_zones` निर्देश क्षेत्र अनुक्रमण को सक्षम करता है, लेकिन HTML [stripper](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip) को भी सक्षम किया जाना चाहिए (द्वारा `html_strip = 1` सेट करके)। `index_zones` का मान टैग नामों और वाइल्डकार्ड (तारे के साथ समाप्त) की एक अल्पविकसित सूची होना चाहिए जिसे क्षेत्रों के रूप में अनुक्रमित किया जाना है।

क्षेत्र को नेस्ट और ओवरलैप किया जा सकता है, जब तक हर उद्घाटन टैग का एक मेल खाने वाला टैग होता है। क्षेत्रों का उपयोग ज़ोन ऑपरेटर के साथ मेल खाने के लिए भी किया जा सकता है, जैसा कि [extended_query_syntax](../../Searching/Full_text_matching/Operators.md#ZONE-limit-operator) में वर्णित है।


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) index_zones = 'h, th, title' html_strip = '1'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) index_zones = 'h, th, title' html_strip = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'index_zones' => 'h*,th,title',
            'html_strip' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) index_zones = \'h, th, title\' html_strip = \'1\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_zones = \'h, th, title\' html_strip = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_zones = 'h, th, title' html_strip = '1'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_zones = 'h, th, title' html_strip = '1'");
```

<!-- request CONFIG -->

```ini
table products {
  index_zones = h*, th, title
  html_strip = 1
  
  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->
<!-- proofread -->

