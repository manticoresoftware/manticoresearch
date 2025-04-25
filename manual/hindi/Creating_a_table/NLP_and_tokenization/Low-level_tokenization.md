# निम्न-स्तरीय टोकनाइजेशन  

जब पाठ Manticore में अनुक्रमित होता है, तो इसे शब्दों में बाँटा जाता है और इसमें मामला मोड़ना किया जाता है ताकि "Abc", "ABC", और "abc" जैसे शब्दों को एक ही शब्द माना जाए।

इन प्रक्रियाओं को ठीक से करने के लिए, Manticore को जानना आवश्यक है:
* स्रोत पाठ का एन्कोडिंग (जो हमेशा UTF-8 होना चाहिए)
* कौन से वर्ण अक्षर माने जाते हैं और कौन से नहीं
* कौन से अक्षरों को अन्य अक्षरों में मोड़ा जाना चाहिए

आप इन सेटिंग्स को प्रति-तालिका आधार पर [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) विकल्प का उपयोग करके कॉन्फ़िगर कर सकते हैं। charset_table एक ऐसा एरे निर्दिष्ट करता है जो अक्षर वर्णों को उनके केस-मोड़े हुए संस्करणों (या किसी अन्य वर्ण जो आप पसंद करते हैं) से मानचित्रित करता है। जो वर्ण एरे में नहीं हैं उन्हें गैर-अक्षरों के रूप में माना जाता है और इस तालिका में अनुक्रमण या खोज के दौरान शब्द विभाजक के रूप में व्यवहार किया जाएगा।

डिफ़ॉल्ट वर्ण सेट `non_cont` है, जिसमें [अधिकांश भाषाएं](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md) शामिल हैं।

आप टेक्स्ट पैटर्न प्रतिस्थापन नियम भी परिभाषित कर सकते हैं। उदाहरण के लिए, निम्नलिखित नियमों के साथ:

```ini
regexp_filter = \**(\d+)\" => \1 इंच
regexp_filter = (BLUE|RED) => COLOR
```

टेक्स्ट `RED TUBE 5" LONG` को `COLOR TUBE 5 INCH LONG` के रूप में अनुक्रमित किया जाएगा, और `PLANK 2" x 4"` को `PLANK 2 INCH x 4 INCH` के रूप में अनुक्रमित किया जाएगा। ये नियम निर्दिष्ट क्रम में लागू होते हैं। नियम क्वेरीज़ पर भी लागू होते हैं, इसलिए `BLUE TUBE` के लिए खोज वास्तव में `COLOR TUBE` के लिए खोजेगी।

आप [regexp_filter के बारे में अधिक जान सकते हैं](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter)।

## अनुक्रमणिका कॉन्फ़िगरेशन विकल्प

### charset_table

```ini
# डिफॉल्ट
charset_table = non_cont

# केवल अंग्रेजी और रूसी अक्षर
charset_table = 0..9, A..Z->a..z, _, a..z, \
U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451

# अंग्रेजी वर्णक्रम जो उपनाम के साथ परिभाषित है
charset_table = 0..9, english, _

# आप अक्षर मैपिंग को फिर से परिभाषित करके ओवरराइड कर सकते हैं, उदाहरण के लिए, जर्मन उमेउट के साथ केस-इनसेंसिटिव खोज के लिए आप उपयोग कर सकते हैं:
charset_table = non_cont, U+00E4, U+00C4->U+00E4, U+00F6, U+00D6->U+00F6, U+00FC, U+00DC->U+00FC, U+00DF, U+1E9E->U+00DF
```

<!-- उदाहरण charset_table -->
`charset_table` एक ऐसा एरे निर्दिष्ट करता है जो अक्षर वर्णों को उनके केस-मोड़े हुए संस्करणों (या किसी अन्य वर्ण यदि आप पसंद करते हैं) से मानचित्रित करता है। डिफ़ॉल्ट वर्ण सेट `non_cont` है, जिसमें अधिकांश भाषाएं [गैर-निरंतर](https://en.wikipedia.org/wiki/Scriptio_continua) स्क्रिप्ट शामिल हैं।

`charset_table` Manticore के टोकनाइजेशन प्रक्रिया की एक कार्यhorse है, जो दस्तावेज़ पाठ या क्वेरी पाठ से कीवर्ड निकालता है। यह नियंत्रित करता है कि कौन से वर्ण वैध के रूप में स्वीकार किए जाते हैं और उन्हें कैसे रूपांतरित किया जाना चाहिए (जैसे, यदि मामला हटाया जाना चाहिए या नहीं)।

डिफ़ॉल्ट रूप से, हर वर्ण 0 पर मैप होता है, जिसका अर्थ है कि इसे मान्य कीवर्ड के रूप में नहीं माना गया है और इसे एक विभाजक के रूप में व्यवहार किया जाता है। एक बार जब किसी वर्ण का उल्लेख तालिका में किया जाता है, तो इसे दूसरे वर्ण पर मैप किया जाता है (अधिकतर, या तो स्वयं या एक छोटी अक्षर पर) और इसे एक मान्य कीवर्ड भाग के रूप में माना जाता है।

charset_table वर्णों को मान्य घोषित करने या उन्हें अन्य वर्णों के लिए मैप करने के लिए मैपिंग के एक अल्पविराम से सूची का उपयोग करता है। एक बार में वर्णों की श्रृंखला को मैप करने के लिए सिंटैक्स शॉर्टकट उपलब्ध हैं:

* सिंगल चार मैपिंग: `A->a`। स्रोत वर्ण 'A' को कीवर्ड के भीतर अनुमति दी गई के रूप में घोषित करता है और इसे गंतव्य वर्ण 'a' पर मैप करता है (लेकिन 'a' को अनुमति दी गई के रूप में घोषित नहीं करता)।
* परास मैपिंग: `A..Z->a..z`। स्रोत सीमा में सभी वर्णों को अनुमति दी गई घोषित करता है और उन्हें गंतव्य सीमा पर मैप करता है। गंतव्य सीमा को अनुमति दी गई के रूप में घोषित नहीं करता। दोनों सीमाओं की लंबाई की जाँच करता है।
* स्ट्रे चार मैपिंग: `a`। एक वर्ण को अनुमति दी गई के रूप में घोषित करता है और इसे स्वयं पर मैप करता है। `a->a` सिंगल चार मैपिंग के समान है।
* स्ट्रे रेंज मैपिंग: `a..z`। इस सीमा में सभी वर्णों को अनुमति दी गई के रूप में घोषित करता है और उन्हें स्वयं पर मैप करता है। `a..z->a..z` रेंज मैपिंग के समान है।
* चेकर्सबोर्ड रेंज मैपिंग: `A..Z/2`। हर वर्ण के जोड़े को दूसरे वर्ण पर मैप करता है। उदाहरण के लिए, `A..Z/2` `A->B, B->B, C->D, D->D, ..., Y->Z, Z->Z` के बराबर है। यह मैपिंग शॉर्टकट उन यूनिकोड ब्लॉकों के लिए सहायक है जहाँ बड़े और छोटे अक्षर एक इंटरलीव्ड क्रम में जाते हैं।

कोड 0 से 32 के बीच के वर्ण, और जो 127 से 8-बिट ASCII और यूनिकोड वर्णों की सीमा में हैं, उन्हें Manticore हमेशा विभाजक के रूप में मानता है। कॉन्फ़िगरेशन फ़ाइल एन्कोडिंग की समस्याओं से बचने के लिए, 8-बिट ASCII वर्णों और यूनिकोड वर्णों को `U+XXX` रूप में निर्दिष्ट किया जाना चाहिए, जहाँ `XXX` एक हेक्साडेसिमल कोड पॉइंट संख्या है। न्यूनतम स्वीकार्य यूनिकोड वर्ण कोड `U+0021` है।

यदि डिफ़ॉल्ट मैपिंग आपकी आवश्यकताओं के लिए अपर्याप्त हैं, तो आप किसी अन्य मैपिंग के साथ उन्हें फिर से निर्दिष्ट करके वर्ण मैपिंग को फिर से परिभाषित कर सकते हैं। उदाहरण के लिए, यदि अंतर्निहित `non_cont` एरे में वर्ण `Ä` और `ä` शामिल हैं और दोनों को ASCII वर्ण `a` पर मैप करता है, तो आप उन वर्णों को फिर से परिभाषित कर सकते हैं जैसे कि उनके लिए यूनिकोड कोड पॉइंट जोड़कर:

```
charset_table = non_cont,U+00E4,U+00C4
```

केस संवेदनशील खोज के लिए या

```
charset_table = non_cont,U+00E4,U+00C4->U+00E4
```

केस संवेदनहीन खोज के लिए।

<!-- अनुरोध SQL -->

```sql
CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'
```

<!-- अनुरोध JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'"
```

<!-- अनुरोध PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
Here is the translated document in Hindi:
            'charset_table' => '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'
        ]);
```
<!-- परिचय -->
##### पायथन:

<!-- अनुरोध पायथन -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451\'')
```

<!-- परिचय -->
##### पायथन-एसिंक्रो:

<!-- अनुरोध पायथन-एसिंक्रो -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451\'')
```

<!-- परिचय -->
##### जावास्क्रिप्ट:

<!-- अनुरोध जावास्क्रिप्ट -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451\'');
```
<!-- परिचय -->
##### जावा:

<!-- अनुरोध जावा -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'", true);
```

<!-- परिचय -->
##### C#:

<!-- अनुरोध C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'", true);
```

<!-- परिचय -->
##### रस्ट:

<!-- अनुरोध रस्ट -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) charset_table = '0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451'", Some(true)).await;
```
<!-- अनुरोध कॉन्फ़िग -->

```ini
table products {
  charset_table = 0..9, A..Z->a..z, _, a..z, \
    U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- समाप्त -->

<!-- चार्सेट_टेबल उदाहरण 2 -->
वर्णों और मैपिंग की परिभाषाओं के अलावा, कई अंतर्निहित उपनाम हैं जिनका उपयोग किया जा सकता है। वर्तमान उपनाम हैं:
* `चीनी`
* `cjk`
* `cont`
* `अंग्रेजी`
* `जापानी`
* `कोरियाई`
* `non_cont` (`non_cjk`)
* `रूसी`
* `थाई`

<!-- अनुरोध SQL -->

```sql
CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'
```

<!-- अनुरोध JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'"
```

<!-- अनुरोध PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'charset_table' => '0..9, english, _'
        ]);
```
<!-- परिचय -->
##### पायथन:

<!-- अनुरोध पायथन -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, english, _\'')
```

<!-- परिचय -->
##### पायथन-एसिंक्रो:

<!-- अनुरोध पायथन-एसिंक्रो -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, english, _\'')
```

<!-- परिचय -->
##### जावास्क्रिप्ट:

<!-- अनुरोध जावास्क्रिप्ट -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'0..9, english, _\'');
```

<!-- परिचय -->
##### जावा:

<!-- अनुरोध जावा -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'", true);
```

<!-- परिचय -->
##### C#:

<!-- अनुरोध C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'", true);
```

<!-- परिचय -->
##### रस्ट:

<!-- अनुरोध रस्ट -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) charset_table = '0..9, english, _'", Some(true)).await;
```

<!-- अनुरोध कॉन्फ़िग -->

```ini
table products {
  charset_table = 0..9, english, _

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- समाप्त -->

अगर आप अपने खोज में विभिन्न भाषाओं का समर्थन करना चाहते हैं, तो सभी के लिए वैध वर्णों और फ़ोल्डिंग नियमों को परिभाषित करना एक श्रमसाध्य कार्य हो सकता है। हमने इसे आपके लिए उपयोग करने योग्य डिफ़ॉल्ट चार्सेट टेबल प्रदान करके सरल बना दिया है - `non_cont` और `cont`, जो क्रमशः असतत और निरंतर (चीनी, जापानी, कोरियाई, थाई) स्क्रिप्ट वाली भाषाओं को कवर करते हैं। अधिकांश मामलों में, ये चार्सेट आपकी आवश्यकताओं के लिए पर्याप्त होने चाहिए।

कृपया ध्यान दें कि निम्न भाषाएँ वर्तमान में **समर्थित नहीं** हैं:
* असमिया
* बिश्नुप्रिया
* बुहिद
* गारो
* ह्मोंग
* हो
* कोमी
* बड़ा फूलदार माओ
* माबा
* मैथिली
* मराठी
* मेंडे
* मरू
* म्येने
* नगाम्बे
* ओडिया
* संताली
* सिंधी
* सिलहेटी

[यूनिकोड भाषाओं की सूची](http://www.unicode.org/cldr/charts/latest/supplemental/languages_and_scripts.html/) में दिए गए अन्य सभी भाषाएँ डिफ़ॉल्ट रूप से समर्थित हैं।

<!-- चार्सेट_टेबल उदाहरण 3 -->
cont और non-cont दोनों भाषाओं के साथ काम करने के लिए, अपनी कॉन्फ़िगरेशन फ़ाइल में नीचे दिखाए गए विकल्प सेट करें (चीनी के लिए [अपवाद](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md) के साथ):

<!-- अनुरोध SQL -->

```sql
CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'
```

<!-- अनुरोध JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'"
```

<!-- अनुरोध PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'charset_table' => 'non_cont',
             'ngram_len' => '1',
             'ngram_chars' => 'cont'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'non_cont\' ngram_len = \'1\' ngram_chars = \'cont\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'non_cont\' ngram_len = \'1\' ngram_chars = \'cont\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) charset_table = \'non_cont\' ngram_len = \'1\' ngram_chars = \'cont\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) charset_table = 'non_cont' ngram_len = '1' ngram_chars = 'cont'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  charset_table       = non_cont
  ngram_len           = 1
  ngram_chars         = cont

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

यदि आपको निरंतर-स्क्रिप्ट भाषाओं के लिए सहायता की आवश्यकता नहीं है, तो आप बस [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len) और [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars) को बाहर रख सकते हैं।
विकल्प। इन विकल्पों के बारे में अधिक जानकारी के लिए, संबंधित दस्तावेज़ अनुभागों का संदर्भ लें।

एक वर्ण को कई वर्णों में या इसके विपरीत मैप करने के लिए, आप [regexp_filter](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter) का उपयोग कर सकते हैं।

### blend_chars

```ini
blend_chars = +, &, U+23
blend_chars = +, &->+
```

<!-- example blend_chars -->
मिश्रित वर्णों की सूची। वैकल्पिक, डिफ़ॉल्ट खाली है।

मिश्रित वर्णों को विभाजक और वैध वर्णों के रूप में अनुक्रमित किया जाता है। उदाहरण के लिए, जब `&` एक मिश्रित वर्ण के रूप में परिभाषित किया गया है और `AT&T` एक अनुक्रमित दस्तावेज़ में दिखाई देता है, तो तीन अलग-अलग कीवर्ड अनुक्रमित किए जाएंगे, `at&t`, `at` और `t`।

इसके अलावा, मिश्रित वर्ण ऐसे तरीके से अनुक्रमण को प्रभावित कर सकते हैं कि कीवर्ड को इस तरह अनुक्रमित किया जाता है जैसे कि मिश्रित वर्णों को बिल्कुल टाइप नहीं किया गया था। यह व्यवहार विशेष रूप से स्पष्ट होता है जब `blend_mode = trim_all` निर्दिष्ट किया जाता है। उदाहरण के लिए, वाक्यांश `some_thing` को `some`, `something`, और `thing` के रूप में अनुक्रमित किया जाएगा जब `blend_mode = trim_all` होगा।

मिश्रित वर्णों का उपयोग करते समय सावधानी बरतनी चाहिए क्योंकि किसी वर्ण को मिश्रित के रूप में परिभाषित करना मतलब है कि यह अब विभाजक नहीं है।
* इसलिए, यदि आप `blend_chars` में एक अल्पविराम डालते हैं और `dog,cat` के लिए खोज करते हैं, तो यह इसे एकल टोकन `dog,cat` के रूप में मान लेगा। यदि `dog,cat` को **नहीं** अनुक्रमित किया गया था बल्कि केवल `dog cat` के रूप में छोड़ा गया था, तो यह मेल नहीं खा जाएगा।
* इसलिए, इस व्यवहार को [blend_mode](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_mode) सेटिंग के साथ नियंत्रित किया जाना चाहिए।

मिश्रित वर्णों के साथ सफेद स्थान के साथ प्रतिस्थापित टोकन के लिए पद सौंपे जाते हैं जैसा सामान्य है, और नियमित कीवर्ड को इस तरह अनुक्रमित किया जाएगा जैसे कि कोई `blend_chars` निर्दिष्ट नहीं किए गए थे। एक अतिरिक्त टोकन जो मिश्रित और गैर-मिश्रित वर्णों को मिलाता है उसे प्रारंभिक स्थिति पर रखा जाएगा। उदाहरण के लिए, यदि `AT&T company` तुरंत टेक्स्ट फ़ील्ड की शुरुआत में प्रकट होता है, तो `at` को स्थिति 1, `t` को स्थिति 2, `company` को स्थिति 3 दी जाएगी, और `AT&T` को भी स्थिति 1 दी जाएगी, जो प्रारंभिक नियमित कीवर्ड के साथ मिश्रित होता है। परिणामस्वरूप, `AT&T` या सिर्फ `AT` के लिए क्वेरी उस दस्तावेज़ से मेल खा जाएगी। वाक्यांश क्वेरी `"AT T"` भी मेल खाएगी, साथ ही एक वाक्यांश क्वेरी `"AT&T company"` के लिए।

मिश्रित वर्ण ऐसे विशेष वर्णों के साथ ओवरलैप कर सकते हैं जो क्वेरी सिंटैक्स में उपयोग किए जाते हैं, जैसे `T-Mobile` या `@twitter`। जहां भी संभव हो, क्वेरी पार्सर मिश्रित वर्ण को मिश्रित के रूप में संभाल लेगा। उदाहरण के लिए, यदि `hello @twitter` उद्धरणों के भीतर है (एक वाक्यांश ऑपरेटर), तो क्वेरी पार्सर `@` प्रतीक को मिश्रित के रूप में संभाल लेगा। हालांकि, यदि `@` प्रतीक उद्धरणों के भीतर नहीं था, तो इसे एक ऑपरेटर के रूप में संभाला जाएगा। इसलिए, कीवर्ड को सुरक्षा के लिए एस्केप करना अनुशंसित है।

मिश्रित वर्णों को फिर से मैप किया जा सकता है ताकि कई विभिन्न मिश्रित वर्णों को एक मूल रूप में सामान्यीकृत किया जा सके। यह तब उपयोगी है जब कई वैकल्पिक यूनिकोड कोडपॉइंट्स को समकक्ष ग्लिफ़ के साथ अनुक्रमित किया जाए।

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'blend_chars' => '+, &, U+23, @->_'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = \'+, &, U+23, @->_\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = \'+, &, U+23, @->_\'')
```

<!-- intro -->
##### जावास्क्रिप्ट:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) blend_chars = \'+, &, U+23, @->_\');
```

<!-- intro -->
##### जावा:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'", true);
```

<!-- intro -->
##### रस्ट:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) blend_chars = '+, &, U+23, @->_'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  blend_chars = +, &, U+23, @->_

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### blend_mode

```ini
blend_mode = option [, option [, ...]]
option = trim_none | trim_head | trim_tail | trim_both | trim_all | skip_pure
```

<!-- example blend_mode -->
मिश्रित टोकन अनुक्रमण मोड को blend_mode निर्देशिका द्वारा सक्षम किया जाता है।

डिफ़ॉल्ट रूप से, मिश्रित और गैर-मिश्रित वर्णों को मिलाने वाले टोकन को पूरी तरह से अनुक्रमित किया जाता है। उदाहरण के लिए, जब एक @ चिह्न और एक विस्मयादिबोधक `blend_chars` में होते हैं, तो स्ट्रिंग `@dude!` दो टोकन के रूप में अनुक्रमित किया जाएगा: `@dude!` (सभी मिश्रित वर्णों के साथ) और `dude` (किसी भी के बिना)। परिणामस्वरूप, `@dude` का एक प्रश्न इसे **मेल नहीं करेगा**।

`blend_mode` इस अनुक्रमण व्यवहार में लचीलापन जोड़ता है। यह विकल्पों की एक अल्पविराम से पृथक सूची लेता है, प्रत्येक में जो एक टोकन अनुक्रमण रूपांतर को निर्दिष्ट करता है।

यदि एक से अधिक विकल्प निर्दिष्ट किए गए हैं, तो उसी टोकन के कई रूपांतर अनुक्रमित किए जाएंगे। नियमित कीवर्ड (उस टोकन से जो मिश्रित वर्णों को एक पृथक के साथ बदलने से प्राप्त होता है) हमेशा अनुक्रमित होते हैं।

विकल्प हैं:

* `trim_none` - पूरे टोकन को अनुक्रमित करें
* `trim_head` - प्रारंभिक मिश्रित वर्णों को ट्रिम करें, और परिणामस्वरूप टोकन को अनुक्रमित करें
* `trim_tail` - अंतिम मिश्रित वर्णों को ट्रिम करें, और परिणामस्वरूप टोकन को अनुक्रमित करें
* `trim_both`- प्रारंभिक और अंतिम मिश्रित वर्णों दोनों को ट्रिम करें, और परिणामस्वरूप टोकन को अनुक्रमित करें
* `trim_all` - प्रारंभिक, अंतिम, और मध्य मिश्रित वर्णों को ट्रिम करें, और परिणामस्वरूप टोकन को अनुक्रमित करें
* `skip_pure` - यदि टोकन पूरी तरह से मिश्रित है, यानी केवल मिश्रित वर्णों से बना है, तो टोकन को अनुक्रमित न करें

उदाहरण स्वरूप `@dude!` स्ट्रिंग के साथ `blend_mode` का उपयोग करते हुए, सेटिंग `blend_mode = trim_head, trim_tail` के परिणामस्वरूप दो अनुक्रमित टोकन होंगे: `@dude` और `dude!`। `trim_both` का उपयोग करने का कोई प्रभाव नहीं होगा क्योंकि दोनों मिश्रित वर्णों को ट्रिम करने से `dude` बनता है, जो पहले से ही एक नियमित कीवर्ड के रूप में अनुक्रमित है। `@U.S.A.` को `trim_both` के साथ अनुक्रमित करने (और मान लेते हुए कि बिंदु मिश्रित है) का परिणाम `U.S.A` अनुक्रमित होना होगा। अंततः, `skip_pure` आपको केवल मिश्रित वर्णों के अनुक्रमों को नजरअंदाज करने की अनुमति देता है। उदाहरण के लिए, `one @@@ two` को `one two` के रूप में अनुक्रमित किया जाएगा, और इसे एक वाक्यांश के रूप में मेल करेगा। यह डिफ़ॉल्ट रूप से ऐसा नहीं है क्योंकि एक पूरी तरह से मिश्रित टोकन अनुक्रमित होता है और दूसरे कीवर्ड की स्थिति को ऑफसेट करता है।

डिफ़ॉल्ट व्यवहार पूरे टोकन को अनुक्रमित करना है, जो `blend_mode = trim_none` के समान है।

यह ध्यान में रखें कि मिश्रण मोड का उपयोग करते समय आपकी खोज को सीमित करता है, यहां तक कि डिफ़ॉल्ट मोड `trim_none` के साथ यदि आप मान लेते हैं कि `.` एक मिश्रित वर्ण है:
* `.dog.` अनुक्रमण के दौरान `.dog. dog` बन जाएगा
* और आप इसे `dog.` द्वारा नहीं ढूंढ पाएंगे।

अधिक मोड का उपयोग करने से आपके कीवर्ड मेल खाने की संभावना बढ़ जाती है।

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'blend_mode' => 'trim_tail, skip_pure',
            'blend_chars' => '+, &'
        ]);
```
<!-- intro -->
##### पायथन:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = \'trim_tail, skip_pure\' blend_chars = \'+, &\'')
```

<!-- intro -->
##### पायथन-ऐसिंक्रोनस:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = \'trim_tail, skip_pure\' blend_chars = \'+, &\'')
```

<!-- intro -->
##### जावास्क्रिप्ट:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) blend_mode = \'trim_tail, skip_pure\' blend_chars = \'+, &\');
```
<!-- intro -->
##### जावा:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'", true);
```

<!-- intro -->
##### रस्ट:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) blend_mode = 'trim_tail, skip_pure' blend_chars = '+, &'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  blend_mode = trim_tail, skip_pure
  blend_chars = +, &

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### min_word_len

```ini
min_word_len = लंबाई
```

<!-- example min_word_len -->

min_word_len मैनटिकोर में एक वैकल्पिक सूचकांक कॉन्फ़िगरेशन विकल्प है जो न्यूनतम सूचीबद्ध शब्द की लंबाई निर्दिष्ट करता है। डिफ़ॉल्ट मान 1 है, जिसका अर्थ है कि सब कुछ सूचीबद्ध है।

केवल उन्हीं शब्दों को सूचीबद्ध किया जाएगा जो इस न्यूनतम से छोटे नहीं हैं। उदाहरण के लिए, यदि min_word_len 4 है, तो 'the' सूचीबद्ध नहीं होगा, लेकिन 'they' होगा।

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) min_word_len = '4'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) min_word_len = '4'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'min_word_len' => '4'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = \'4\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = \'4\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_word_len = \'4\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) min_word_len = '4'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) min_word_len = '4'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) min_word_len = '4'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  min_word_len = 4

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### ngram_len

```ini
ngram_len = 1
```

<!-- example ngram_len -->
N-ग्राम लंबाई N-ग्राम सूचीकरण के लिए। वैकल्पिक, डिफ़ॉल्ट 0 है (N-ग्राम सूचीकरण अक्षम)। ज्ञात मान 0 और 1 हैं।

N-ग्राम अखंड लिपि वाली भाषाओं में बिना विभाजन वाले पाठों में बुनियादी समर्थन प्रदान करते हैं। निरंतर लिपि का उपयोग करने वाली भाषाओं में खोज की समस्या शब्दों के बीच स्पष्ट विभाजक का अभाव है। कुछ मामलों में, आप शब्दकोश-आधारित विभाजन का उपयोग नहीं करना चाह सकते, जैसे [चीनी के लिए उपलब्ध](../../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)। ऐसी स्थितियों में, N-ग्राम विभाजन भी अच्छी तरह काम कर सकता है।

जब यह सुविधा सक्षम होती है, तो इस तरह की भाषाओं (या [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars) में परिभाषित किसी अन्य अक्षर) के प्रवाह को N-ग्राम के रूप में सूचीबद्ध किया जाता है। उदाहरण के लिए, यदि आने वाला पाठ "ABCDEF" है (जहां A से F किसी भाषा के अक्षर हैं) और ngram_len 1 है, तो इसे "A B C D E F" के रूप में सूचीबद्ध किया जाएगा। वर्तमान में केवल ngram_len=1 समर्थित है। केवल वे अक्षर जो [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars) तालिका में सूचीबद्ध हैं, उन्हें इस तरह विभाजित किया जाएगा; अन्य प्रभावित नहीं होंगे।

ध्यान दें कि यदि खोज क्वेरी विभाजित है, यानी व्यक्तिगत शब्दों के बीच विभाजक हैं, तो शब्दों को उद्धरण चिह्नों में लपेटने और विस्तारित मोड का उपयोग करने से उचित मिलान मिलेगा, भले ही पाठ **नहीं** विभाजित हो। मान लीजिए कि मूल क्वेरी `BC DEF` है। उद्धरण चिह्नों में लपेटने के बाद, यह `"BC" "DEF"` (*उद्धरण के साथ*) दिखेगा। यह क्वेरी मैनटिकोर को पारित की जाएगी और आंतरिक रूप से 1-ग्राम में विभाजित होगी, जिससे `"B C" "D E F"` क्वेरी बनेगी, अभी भी उद्धरण चिह्न जो वाक्यांश मिलान ऑपरेटर हैं। और यह पाठ से मेल खाएगा, भले ही पाठ में कोई विभाजक न हों।

यदि खोज क्वेरी विभाजित नहीं है, तो भी मैनटिकोर अच्छे परिणाम उत्पन्न करना चाहिए, फ्रेज-आधारित रैंकिंग के कारण: यह करीबी वाक्यांश मिलान (जिसमें N-ग्राम शब्दों के मामले में बहु-अक्षर शब्द मिलान का अर्थ हो सकता है) को शीर्ष पर खींच लेगा।

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'ngram_chars' => 'cont',
             'ngram_len' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  ngram_chars = cont
  ngram_len = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### ngram_chars

```ini
ngram_chars = cont

ngram_chars = cont, U+3000..U+2FA1F
```

<!-- example ngram_chars -->
N-ग्राम अक्षरों की सूची। वैकल्पिक, डिफ़ॉल्ट खाली है।

To be used in conjunction with in [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len), this list defines characters, sequences of which are subject to N-ग्राम निकासी। अन्य अक्षरों से मिलकर बने शब्दों पर N-ग्राम अनुक्रमांकित करने की विशेषता का कोई प्रभाव नहीं होगा। मान प्रारूप [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) के समान है। N-ग्राम अक्षर [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) में नहीं दिखाई दे सकते हैं।

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'ngram_chars' => 'U+3000..U+2FA1F',
             'ngram_len' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'U+3000..U+2FA1F\' ngram_len = \'1\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'U+3000..U+2FA1F\' ngram_len = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```java
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'U+3000..U+2FA1F\' ngram_len = \'1\'');
```

<!-- intro -->
##### Java:

<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) ngram_chars = 'U+3000..U+2FA1F' ngram_len = '1'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  ngram_chars = U+3000..U+2FA1F
  ngram_len = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```

<!-- end -->

<!-- example ngram_chars 2 -->
Also you can use an alias for our default N-gram table as in the example. It should be sufficient in most cases.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'ngram_chars' => 'cont',
             'ngram_len' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ngram_chars = \'cont\' ngram_len = \'1\'');
```

<!-- intro -->
##### Java:

<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) ngram_chars = 'cont' ngram_len = '1'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  ngram_chars = cont
  ngram_len = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### ignore_chars

```ini
ignore_chars = U+AD
```

<!-- example ignore_chars -->
Ignored characters list. Optional, default is empty.
कुछ मामलों में जब कुछ अक्षर, जैसे कि soft hyphenation mark (U+00AD), को केवल विभाजक के रूप में नहीं बल्कि पूरी तरह से अनदेखा किया जाना चाहिए, तब यह उपयोगी होता है। उदाहरण के लिए, अगर '-' बस charset_table में नहीं है, तो "abc-def" टेक्स्ट को "abc" और "def" कीवर्ड के रूप में अनुक्रमित किया जाएगा। इसके विपरीत, अगर '-' को ignore_chars सूची में जोड़ा जाता है, तो वही टेक्स्ट एक एकल "abcdef" कीवर्ड के रूप में अनुक्रमित होगा.

[charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) के लिए सिंटैक्स समान है, लेकिन यहाँ केवल अक्षरों को घोषित करना अनुमति है, उन्हें मैप करना अनुमति नहीं है। साथ ही, अनदेखे किए गए अक्षर charset_table में मौजूद नहीं होने चाहिए.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'ignore_chars' => 'U+AD'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = \'U+AD\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = \'U+AD\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) ignore_chars = \'U+AD\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) ignore_chars = 'U+AD'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  ignore_chars = U+AD

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### bigram_index

```ini
bigram_index = {none|all|first_freq|both_freq}
```

<!-- example bigram_index -->
बिग्राम अनुक्रमण मोड. वैकल्पिक, डिफ़ॉल्ट रूप से none है.

बिग्राम अनुक्रमण एक विशेषता है जो वाक्यांश खोजों को तीव्र करने के लिए है। अनुक्रमण करते समय, यह इंडेक्स में या तो सभी या कुछ सन्निकट शब्द जोड़ी के लिए एक दस्तावेज़ सूची संग्रहीत करता है। ऐसी सूची का उपयोग खोज के समय वाक्यांश या उप-वाक्यांश मिलान को काफी तेज करने के लिए किया जा सकता है.

`bigram_index` विशिष्ट शब्द जोड़ों के चयन को नियंत्रित करता है। ज्ञात मोड हैं:

* `all`, प्रत्येक एकल शब्द जोड़ी का अनुक्रमण करें
* `first_freq`, केवल उन शब्द जोड़ों का अनुक्रमण करें जहाँ *पहला* शब्द अक्सर प्रयुक्त शब्दों की सूची में हो (देखें [bigram_freq_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_freq_words)). उदाहरण के लिए, `bigram_freq_words = the, in, i, a` के साथ, "alone in the dark" टेक्स्ट का अनुक्रमण करने पर "in the" और "the dark" जोड़ी बिग्राम के रूप में संग्रहीत होंगी, क्योंकि वे एक प्रचलित कीवर्ड (क्रमशः "in" या "the") से शुरू होती हैं, लेकिन "alone in" का अनुक्रमण **नहीं** किया जाएगा, क्योंकि "in" उस जोड़ी में *दूसरे* शब्द के रूप में है.
* `both_freq`, केवल उन शब्द जोड़ों का अनुक्रमण करें जहाँ दोनों शब्द प्रचलित हों. उसी उदाहरण को जारी रखते हुए, इस मोड में "alone in the dark" का अनुक्रमण करते समय केवल "in the" (खोज के दृष्टिकोण से सबसे खराब) ही बिग्राम के रूप में संग्रहीत होगी, लेकिन अन्य शब्द जोड़ी का कोई अनुक्रमण नहीं होगा.

ज्यादातर उपयोग मामलों के लिए, `both_freq` सबसे अच्छा मोड होगा, लेकिन आपके परिणाम भिन्न हो सकते हैं.

यह ध्यान देना महत्वपूर्ण है कि `bigram_index` केवल टोकनाइजेशन स्तर पर काम करता है और `morphology`, `wordforms` या `stopwords` जैसी परिवर्तनों को ध्यान में नहीं रखता. इसका अर्थ है कि यह जो टोकन बनाता है वे बहुत सटीक होते हैं, जो वाक्यांशों के मिलान को अधिक सटीक और कठोर बनाता है. जबकि इससे वाक्यांश मिलान की सटीकता में सुधार हो सकता है, यह सिस्टम को शब्दों के विभिन्न रूपों या उनके प्रकट होने में विविधताओं को पहचानने में कम सक्षम बनाता है.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'bigram_freq_words' => 'the, a, you, i',
            'bigram_index' => 'both_freq'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'both_freq\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'both_freq\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'both_freq\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'both_freq'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  bigram_index = both_freq
  bigram_freq_words = the, a, you, i

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### bigram_freq_words

```ini
bigram_freq_words = the, a, you, i
```

<!-- example bigram_freq_words -->
एक कीवर्ड की सूची जिसे "बार-बार" माना जाता है जब बायग्राम को अनुक्रमित किया जाता है। वैकल्पिक, डिफ़ॉल्ट खाली है।

कुछ बायग्राम अनुक्रमण modos (देखें [bigram_index](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)) को सामान्य कीवर्ड की एक सूची परिभाषित करने की आवश्यकता होती है। इन्हें स्टॉप वर्ड के साथ भ्रमित नहीं किया जाना चाहिए। स्टॉप वर्ड को अनुक्रमण और खोज दोनों के दौरान पूरी तरह से समाप्त कर दिया जाता है। सामान्य कीवर्ड का उपयोग केवल बायग्राम द्वारा यह निर्धारित करने के लिए किया जाता है कि क्या वर्तमान शब्द जोड़े को अनुक्रमित करना है या नहीं।

`bigram_freq_words` आपको ऐसे कीवर्ड की एक सूची परिभाषित करने देता है।

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'bigram_freq_words' => 'the, a, you, i',
            'bigram_index' => 'first_freq'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'first_freq\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'first_freq\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) bigram_freq_words = \'the, a, you, i\' bigram_index = \'first_freq\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) bigram_freq_words = 'the, a, you, i' bigram_index = 'first_freq'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  bigram_freq_words = the, a, you, i
  bigram_index = first_freq

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### dict

```ini
dict = {keywords|crc}
```

<!-- example dict -->
कीवर्ड शब्दकोश के प्रकार को दो ज्ञात मानों में से एक 'crc' या 'keywords' द्वारा पहचाना जाता है। यह वैकल्पिक है, 'keywords' को डिफ़ॉल्ट के रूप में।

कीवर्ड शब्दकोश मोड (dict=keywords) का उपयोग करना अनुक्रमण के बोझ को काफी कम कर सकता है और बड़े संग्रह पर उपस्ट्रिंग खोजों को सक्षम कर सकता है। इस मोड का उपयोग सामान्य और RT तालिकाओं दोनों के लिए किया जा सकता है।

CRC शब्दकोश मौलिक कीवर्ड टेक्स्ट को अनुक्रम में संग्रहीत नहीं करते हैं। इसके बजाय, वे अनुक्रमण और खोज प्रक्रियाओं के दौरान कीवर्ड को नियंत्रण योग मान (FNV64 का उपयोग करके गणना की गई) से बदल देते हैं। यह मान केवल अनुक्रम में आंतरिक रूप से उपयोग किया जाता है। इस दृष्टिकोण के दो दोष हैं:
* सबसे पहले, विभिन्न कीवर्ड जोड़े के बीच नियंत्रण योग टकराव का जोखिम होता है। यह जोखिम अनुक्रम में अद्वितीय कीवर्ड की संख्या के आनुपातिक रूप से बढ़ता है। फिर भी, यह चिंता छोटी है क्योंकि 1 अरब प्रविष्टियों के एक शब्दकोश में एकल FNV64 टकराव की संभावना लगभग 1 में 16 या 6.25 प्रतिशत है। अधिकांश शब्दकोशों में 1 अरब कीवर्ड से कहीं कम होंगे, यह देखते हुए कि एक सामान्य बोले गए मानव भाषा में 1 से 10 मिलियन शब्द रूप होते हैं।
* दूसरी बात, और अधिक महत्वपूर्ण, नियंत्रण योग के साथ उपस्ट्रिंग खोज करना सरल नहीं है। मैन्टिकोर ने सभी संभावित उपस्ट्रिंग को अलग कीवर्ड के रूप में पूर्व-नियुक्त करके इस मुद्दे को हल किया (देखें [min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len), [min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len) निर्देश)। इस विधि का एक और लाभ यह है कि यह उपस्ट्रिंग को सबसे तेज़ तरीके से मेल खाता है। फिर भी, सभी उपस्ट्रिंग को पूर्व-नियुक्त करना अनुक्रम के आकार को काफी बढ़ा देता है (अक्सर 3-10 गुना या उससे अधिक) और इसके पश्चात अनुक्रमण समय को प्रभावित करता है, जिससे बड़े अनुक्रमों में उपस्ट्रिंग खोजें अव्यवहारिक हो जाती हैं।
कीवर्ड डिक्शनरी इन दोनों मुद्दों का समाधान करती है। यह इंडेक्स में कीवर्ड्स को संग्रहीत करती है और खोज के समय वाइल्डकार्ड विस्तार करती है। उदाहरण के लिए, `test*` प्रीफिक्स के लिए खोज के दौरान डिक्शनरी की सामग्री के आधार पर 'test|tests|testing' क्वेरी में आंतरिक रूप से विस्तारित किया जा सकता है। यह विस्तार प्रक्रिया पूरी तरह से एप्लिकेशन के लिए अदृश्य है, सिवाय इसके कि सभी मेल खाने वाले कीवर्ड्स के लिए अलग-अलग प्रति-कीवर्ड सांख्यिकी अब भी रिपोर्ट की जाती हैं।

उपस्ट्रिंग (इन्फिक्स) खोज के लिए, विस्तारित वाइल्डकार्ड का उपयोग किया जा सकता है। विशेष वर्ण जैसे `?` और `%` उपस्ट्रिंग (इन्फिक्स) खोज के लिए संगत हैं (जैसे, `t?st*`, `run%`, `*abc*`)। ध्यान दें कि [वाइल्डकार्ड ऑपरेटर](Searching/Full_text_matching/Operators.md#Wildcard-operators) और [REGEX](../../Searching/Full_text_matching/Operators.md#REGEX-operator) केवल `dict=keywords` के साथ कार्य करते हैं।

कीवर्ड्स डिक्शनरी के साथ इंडेक्सिंग सामान्य, गैर-उपस्ट्रिंग इंडेक्सिंग की तुलना में लगभग 1.1x से 1.3x धीमी होती है - फिर भी उपस्ट्रिंग इंडेक्सिंग (या प्रीफिक्स या इन्फिक्स) की तुलना में काफी तेज होती है। इंडेक्स का आकार मानक गैर-उपस्ट्रिंग टेबल की तुलना में केवल थोड़ा बड़ा होना चाहिए, कुल अंतर 1..10% प्रतिशत के साथ। सामान्य कीवर्ड खोज के लिए जो समय लगता है, वह चर्चा किए गए सभी तीन इंडेक्स प्रकारों (CRC गैर-उपस्ट्रिंग, CRC उपस्ट्रिंग, कीवर्ड्स) के बीच लगभग समान या समान होना चाहिए। उपस्ट्रिंग खोज का समय इस बात पर निर्भर कर सकता है कि दिए गए उपस्ट्रिंग से कितने वास्तविक कीवर्ड मेल खाते हैं (यानी, खोज शब्द कितने कीवर्ड्स में विस्तारित होता है)। मेल खाने वाले कीवर्ड्स की अधिकतम संख्या [expansion_limit](../../Server_settings/Searchd.md#expansion_limit) निर्देशक द्वारा सीमित होती है।

संक्षेप में, कीवर्ड्स और CRC डिक्शनरी उपस्ट्रिंग खोज के लिए दो विभिन्न ट्रेड-ऑफ निर्णय प्रदान करती हैं। आप या तो सबसे तेज़ सबसे खराब मामलों की खोज को प्राप्त करने के लिए इंडेक्सिंग समय और इंडेक्स आकार का बलिदान करने का विकल्प चुन सकते हैं (CRC डिक्शनरी), या इंडेक्सिंग समय को न्यूनतम प्रभावित कर सकते हैं लेकिन सबसे खराब मामलों में खोज का समय बलिदान कर सकते हैं जब प्रीफिक्स एक उच्च संख्या में कीवर्ड्स में फैलता है (कीवर्ड्स डिक्शनरी)।

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) dict = 'keywords'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) dict = 'keywords'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'dict' => 'keywords'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) dict = \'keywords\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) dict = \'keywords\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) dict = \'keywords\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) dict = 'keywords'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) dict = 'keywords'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) dict = 'keywords'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  dict = keywords

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### embedded_limit

```ini
embedded_limit = size
```

<!-- example embedded_limit -->
एंबेडेड अपवाद, वर्डफॉर्म, या स्टॉप वर्ड फ़ाइल आकार सीमा। वैकल्पिक, डिफ़ॉल्ट 16K है।

जब आप एक तालिका तैयार करते हैं, तो उपरोक्त उल्लेखित फ़ाइलें या तो तालिका के साथ बाहरी रूप से संग्रहीत की जा सकती हैं या तालिका में सीधे एंबेड की जा सकती हैं। `embedded_limit` से छोटे आकार की फ़ाइलें तालिका में संग्रहीत की जाती हैं। बड़े फ़ाइलों के लिए, केवल फ़ाइल के नाम संग्रहीत होते हैं। इससे तालिका फ़ाइलों को किसी अन्य मशीन में ले जाना भी सरल हो जाता है; आप केवल एक फ़ाइल कॉपी करके काम कर सकते हैं।

छोटे फ़ाइलों के साथ, ऐसी एंबेडिंग तालिका पर निर्भर बाहरी फ़ाइलों की संख्या को कम करती है, और रखरखाव में मदद करती है। लेकिन साथ ही यह कोई anlam नहीं बनाता कि एक 100 MB वर्डफॉर्म डिक्शनरी को एक छोटे डेल्टा टेबल में एंबेड करें। इसलिए एक आकार सीमा होनी चाहिए, और `embedded_limit` वह सीमा है।

<!-- request CONFIG -->

```ini
table products {
  embedded_limit = 32K

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### global_idf

```ini
global_idf = /path/to/global.idf
```

<!-- example global_idf -->
वैश्विक (क्लस्टर-व्यापी) कीवर्ड IDFs वाली फ़ाइल का पथ। वैकल्पिक, डिफ़ॉल्ट खाली है (स्थानीय IDFs का उपयोग करें)।

एक बहु-तालिका क्लस्टर पर, प्रति-कीवर्ड आवृत्तियां विभिन्न तालिकाओं में भिन्न होने की संभावना है। अर्थात, जब रैंकिंग फ़ंक्शन TF-IDF आधारित मानों का उपयोग करता है, जैसे कि BM25 फैक्टर का परिवार, तो परिणाम थोड़े भिन्न ढंग से रैंक किए जा सकते हैं कि वे किस क्लस्टर नोड में स्थित हैं।
उस समस्या को हल करने का सबसे आसान तरीका एक वैश्विक आवृत्ति शब्दकोश या संक्षिप्त में एक वैश्विक IDF फ़ाइल बनाना और उपयोग करना है। यह निदेश आपको उस फ़ाइल का स्थान निर्दिष्ट करने देता है। सुझाव दिया गया है (लेकिन आवश्यक नहीं) कि एक .idf एक्सटेंशन का उपयोग करें। जब किसी दिए गए तालिका के लिए IDF फ़ाइल निर्दिष्ट की जाती है *और* [OPTION global_idf](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#global_idf) को 1 पर सेट किया जाता है, तो इंजन कीवर्ड आवृत्तियों और संग्रह दस्तावेज़ की गणनाओं का उपयोग वैश्विक_idf फ़ाइल से करेगा, न कि केवल स्थानीय तालिका से। इस तरह, IDFs और उन पर निर्भर मान क्लस्टर में सुसंगत रहेंगे।

IDF फ़ाइलें कई तालिकाओं के बीच साझा की जा सकती हैं। `searchd` द्वारा केवल IDF फ़ाइल की एक ही प्रति लोड की जाएगी, भले ही कई तालिकाएं उस फ़ाइल का संदर्भ देती हों। यदि IDF फ़ाइल की सामग्री बदलती है, तो नई सामग्री को SIGHUP के साथ लोड किया जा सकता है।

आप [indextool](../../Miscellaneous_tools.md#indextool) उपयोगिता का उपयोग करके एक .idf फ़ाइल बना सकते हैं, पहले `--dumpdict dict.txt --stats` स्विच का उपयोग करके शब्दकोशों को डंप करके, फिर `--buildidf` का उपयोग करके उन्हें .idf प्रारूप में परिवर्तित करते हुए, और फिर `--mergeidf` का उपयोग करके क्लस्टर भर में सभी .idf फ़ाइलों को मिलाते हुए।

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'global_idf' => '/usr/local/manticore/var/global.idf'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = \'/usr/local/manticore/var/global.idf\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = \'/usr/local/manticore/var/global.idf\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) global_idf = \'/usr/local/manticore/var/global.idf\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) global_idf = '/usr/local/manticore/var/global.idf'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  global_idf = /usr/local/manticore/var/global.idf

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### hitless_words

```ini
hitless_words = {all|path/to/file}
```

<!-- example hitless_words -->
Hitless शब्दों की सूची। वैकल्पिक, अनुमति प्राप्त मान 'all' या एक सूची फ़ाइल नाम हैं।

डिफ़ॉल्ट रूप से, Manticore पूर्ण-पाठ इंडेक्स न केवल प्रत्येक दिए गए कीवर्ड के लिए मिलान करने वाले दस्तावेज़ों की एक सूची स्टोर करता है, बल्कि इसकी इन-डॉक्यूमेंट स्थितियों (जिसे हिटलिस्ट कहा जाता है) की एक सूची भी स्टोर करता है। हिटलिस्ट वाक्यांश, निकटता, सख्त क्रम और अन्य उन्नत प्रकारों की खोज के साथ-साथ वाक्यांश निकटता रैंकिंग को सक्षम करती है। हालाँकि, विशिष्ट सामान्य कीवर्डों के लिए हिटलिस्ट (जो किसी कारण से अक्सर होने के बावजूद रुक नहीं सकती) विशाल हो सकती हैं और इस प्रकार क्वेरी करते समय संसाधित करने में धीमी हो सकती हैं। इसके अलावा, कुछ मामलों में हम केवल बूलियन कीवर्ड मिलान की परवाह कर सकते हैं, और कभी भी स्थिति-आधारित खोज ऑपरेटरों (जैसे वाक्यांश मिलान) या वाक्यांश रैंकिंग की आवश्यकता नहीं हो सकती।

`hitless_words` आपको ऐसे इंडेक्स बनाने की अनुमति देता है जिनमें या तो किसी प्रकार की स्थिति संबंधी जानकारी (हिटलिस्ट) नहीं होती है, या विशिष्ट कीवर्ड के लिए इसे छोड़ देते हैं।

हिटलेस इंडेक्स आमतौर पर संबंधित नियमित पूर्ण-पाठ इंडेक्स की तुलना में कम स्थान का उपयोग करेगा (लगभग 1.5x की अपेक्षा की जा सकती है)। इंडेक्सिंग और खोज करना दोनों तेज होना चाहिए, स्थिति आधारित क्वेरी और रैंकिंग समर्थन की कीमत पर।  

यदि स्थिति संबंधी क्वेरीज़ (जैसे वाक्यांश क्वेरीज़) में उपयोग किया जाता है तो हिटलेस शब्दों को उनमें से बाहर ले जाया जाता है और बिना किसी स्थिति के ऑपरेन्ड के रूप में उपयोग किया जाता है। उदाहरण के लिए यदि "hello" और "world" हिटलेस हैं और "simon" और "says" हिटलेस नहीं हैं, तो वाक्यांश क्वेरी `"simon says hello world"` को `("simon says" & hello & world)` में परिवर्तित किया जाएगा, दस्तावेज़ में "hello" और "world" को कहीं भी मिलाते हुए और "simon says" को एक सटीक वाक्यांश के रूप में।

एक स्थिति संबंधी क्वेरी जिसमें केवल हिटलेस शब्द शामिल हैं, एक खाली वाक्यांश नोड का परिणाम देगा, इसलिए पूरी क्वेरी एक खाली परिणाम और एक चेतावनी लौटाएगी। यदि पूरा शब्दकोश हिटलेस है (जो `all` का उपयोग कर रहा है) तो केवल संबंधित इंडेक्स पर बूलियन मिलान का उपयोग किया जा सकता है।



<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) hitless_words = 'all'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) hitless_words = 'all'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'hitless_words' => 'all'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = \'all\'')
```

<!-- intro -->
##### Python-asyncio:
<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = \'all\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) hitless_words = \'all\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) hitless_words = 'all'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) hitless_words = 'all'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) hitless_words = 'all'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  hitless_words = all

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### index_field_lengths

```ini
index_field_lengths = {0|1}
```

<!-- example index_field_lengths -->
Enables computing and storing of field lengths (both per-document and average per-index values) into the full-text index. Optional, default is 0 (do not compute and store).

When `index_field_lengths` is set to 1 Manticore will:
* create a respective length attribute for every full-text field, sharing the same name but with `__len` suffix
* compute a field length (counted in keywords) for every document and store in to a respective attribute
* compute the per-index averages. The lengths attributes will have a special TOKENCOUNT type, but their values are in fact regular 32-bit integers, and their values are generally accessible.

[BM25A()](../../Functions/Searching_and_ranking_functions.md#BM25A%28%29) and [BM25F()](../../Functions/Searching_and_ranking_functions.md#BM25F%28%29) functions in the expression ranker are based on these lengths and require `index_field_lengths` to be enabled. Historically, Manticore used a simplified, stripped-down variant of BM25 that, unlike the complete function, did **not** account for document length. There's also support for both a complete variant of BM25, and its extension towards multiple fields, called BM25F. They require per-document length and per-field lengths, respectively. Hence the additional directive.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) index_field_lengths = '1'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) index_field_lengths = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'index_field_lengths' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = \'1\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_field_lengths = \'1\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_field_lengths = '1'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_field_lengths = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) index_field_lengths = '1'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  index_field_lengths = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### index_token_filter

```ini
index_token_filter = my_lib.so:custom_blend:chars=@#&
```

<!-- example index_token_filter -->
Index-time token filter for full-text indexing. Optional, default is empty.

The index_token_filter directive specifies an optional index-time token filter for full-text indexing. This directive is used to create a custom tokenizer that makes tokens according to custom rules. The filter is created by the indexer on indexing source data into a plain table or by an RT table on processing `INSERT` or `REPLACE` statements. The plugins are defined using the format `library name:plugin name:optional string of settings`. For example, `my_lib.so:custom_blend:chars=@#&`.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'index_token_filter' => 'my_lib.so:custom_blend:chars=@#&'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = \'my_lib.so:custom_blend:chars=@#&\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = \'my_lib.so:custom_blend:chars=@#&\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_token_filter = \'my_lib.so:custom_blend:chars=@#&\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) index_token_filter = 'my_lib.so:custom_blend:chars=@#&'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  index_token_filter = my_lib.so:custom_blend:chars=@#&

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### overshort_step

```ini
overshort_step = {0|1}
```

<!-- example overshort_step -->
Position increment on overshort (less than [min_word_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len)) keywords. Optional, allowed values are 0 and 1, default is 1.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) overshort_step = '1'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) overshort_step = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'overshort_step' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = \'1\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = \'1\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) overshort_step = \'1\'');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) overshort_step = '1'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) overshort_step = '1'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) overshort_step = '1'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  overshort_step = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### phrase_boundary

```ini
phrase_boundary = ., ?, !, U+2026 # horizontal ellipsis
```

<!-- example phrase_boundary -->
Phrase boundary characters list. Optional, default is empty.

This list controls what characters will be treated as phrase boundaries, in order to adjust word positions and enable phrase-level search emulation through proximity search. The syntax is similar to [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table), but mappings are not allowed and the boundary characters must not overlap with anything else.

On phrase boundary, additional word position increment (specified by [phrase_boundary_step](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary_step)) will be added to current word position. This enables phrase-level searching through proximity queries: words in different phrases will be guaranteed to be more than phrase_boundary_step distance away from each other; so proximity search within that distance will be equivalent to phrase-level search.

Phrase boundary condition will be raised if and only if such character is followed by a separator; this is to avoid abbreviations such as S.T.A.L.K.E.R or URLs being treated as several phrases.

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'phrase_boundary' => '., ?, !, U+2026',
             'phrase_boundary_step' => '10'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = \'., ?, !, U+2026\' phrase_boundary_step = \'10\'')
```

<!-- intro -->
##### Pytho-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = \'., ?, !, U+2026\' phrase_boundary_step = \'10\'')
```

<!-- intro -->
##### Javascript:
<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary = \'., ?, !, U+2026\' phrase_boundary_step = \'10\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) phrase_boundary = '., ?, !, U+2026' phrase_boundary_step = '10'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  phrase_boundary = ., ?, !, U+2026
  phrase_boundary_step = 10

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### phrase_boundary_step

```ini
phrase_boundary_step = 100
```

<!-- example phrase_boundary_step -->
वाक्य सीमा शब्द स्थिति वृद्धि। वैकल्पिक, डिफ़ॉल्ट 0 है।

वाक्य सीमा पर, वर्तमान शब्द स्थिति इस संख्या द्वारा अतिरिक्त रूप से बढ़ा दी जाएगी।

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'phrase_boundary_step' => '100',
             'phrase_boundary' => '., ?, !, U+2026'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = \'100\' phrase_boundary = \'., ?, !, U+2026\'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = \'100\' phrase_boundary = \'., ?, !, U+2026\'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) phrase_boundary_step = \'100\' phrase_boundary = \'., ?, !, U+2026\'');
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) phrase_boundary_step = '100' phrase_boundary = '., ?, !, U+2026'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  phrase_boundary_step = 100
  phrase_boundary = ., ?, !, U+2026

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

### regexp_filter

```ini
# index '13"' as '13inch'
regexp_filter = \b(\d+)\" => \1inch

# index 'blue' or 'red' as 'color'
regexp_filter = (blue|red) => color
```

<!-- example regexp_filter -->
नियमित अभिव्यक्तियाँ (regexps) जो क्षेत्रों और क्वेरीज़ को फ़िल्टर करने के लिए उपयोग की जाती हैं। यह निर्देशन वैकल्पिक है, बहु-मूल्यवान है, और इसका डिफ़ॉल्ट एक खाली सूची है नियमित अभिव्यक्तियों की। नियमित अभिव्यक्तियों का इंजन जिसका उपयोग Manticore Search करता है, वह गूगल का RE2 है, जो अपनी गति और सुरक्षा के लिए जाना जाता है। RE2 द्वारा समर्थित रूपरेखा पर विस्तृत जानकारी के लिए, आप [RE2 syntax guide](https://github.com/google/re2/wiki/Syntax) पर जा सकते हैं।

कुछ अनुप्रयोगों जैसे उत्पाद खोज में, एक उत्पाद, मॉडल, या संपत्ति का संदर्भ देने के लिए कई तरीके हो सकते हैं। उदाहरण के लिए, `iPhone 3gs` और `iPhone 3 gs` (या यहां तक कि `iPhone3 gs`) संभवतः एक ही उत्पाद को संदर्भित करते हैं। एक और उदाहरण हो सकता है लैपटॉप स्क्रीन का आकार व्यक्त करने के लिए विभिन्न तरीके, जैसे `13-inch`, `13 inch`, `13"`, या `13in`।

Regexps एक ऐसा तंत्र प्रदान करते हैं जो ऐसे मामलों को संभालने के लिए नियम निर्धारित करने का कार्य करता है। पहले उदाहरण में, आप संभवतः कुछ iPhone मॉडलों को संभालने के लिए एक शब्दरूप फ़ाइल का उपयोग कर सकते हैं, लेकिन दूसरे उदाहरण में, "13-inch" और "13in" को समान कुछ में सामान्यीकृत करने के लिए नियम निर्दिष्ट करना बेहतर है।

`regexp_filter` में सूचीबद्ध नियमित अभिव्यक्तियाँ उन क्रम में लागू की जाती हैं जिनमें वे सूचीबद्ध हैं, सबसे पहले संभव चरण पर, किसी अन्य प्रसंस्करण से पहले (जिसमें [exceptions](../../Creating_a_table/NLP_and_tokenization/Exceptions.md#exceptions) शामिल हैं), यहां तक कि tokenization से पहले। अर्थात, regexps को इंडेक्सिंग के दौरान कच्चे स्रोत क्षेत्रों पर लागू किया जाता है, और खोज करते समय कच्चे खोज क्वेरी पाठ पर लागू किया जाता है।

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'regexp_filter' => '(blue|red) => color'
        ]);

```
<!-- intro -->
##### पायथन:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) regexp_filter = \'(blue|red) => color\'')
```

<!-- intro -->
##### पायथन-ऐसिंक्रोनस:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) regexp_filter = \'(blue|red) => color\'')
```

<!-- intro -->
##### जावास्क्रिप्ट:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) regexp_filter = \'(blue|red) => color\'');
```

<!-- intro -->
##### जावा:

<!-- request Java -->

```java
utilsApi.sql("CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'", true);
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'", true);
```

<!-- intro -->
##### रस्ट:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) regexp_filter = '(blue|red) => color'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  # index '13"' as '13inch'
  regexp_filter = \b(\d+)\" => \1inch

  # index 'blue' or 'red' as 'color'
  regexp_filter = (blue|red) => color

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->
<!-- proofread -->






