# उन्नत संरचना
संरचना पूर्व-प्रसंस्करण शब्दों पर अनुक्रमण के दौरान विभिन्न रूपों को सामान्य बनाने और विभाजन में सुधार करने के लिए लागू किया जा सकता है। उदाहरण के लिए, एक अंग्रेजी स्टेमर "dogs" और "dog" को "dog" में सामान्य बना सकता है, जिसके परिणामस्वरूप दोनों कीवर्ड के लिए समान खोज परिणाम मिलते हैं।
मैन्टिकोर में चार अंतर्निर्मित संरचना पूर्व-प्रसंस्करण होते हैं:
*   **लेम्माटाइज़र**: एक शब्द को इसके मूल या लेम्मा में घटित करता है। उदाहरण के लिए, "running" को "run" में और "octopi" को "octopus" में घटित किया जा सकता है। ध्यान दें कि कुछ शब्दों के कई संबंधित मूल रूप हो सकते हैं। उदाहरण के लिए, "dove" "dive" का भूतकाल हो सकता है या एक पक्षी का नाम हो सकता है, जैसे "एक सफेद कबूतर कुकू के घोंसले के ऊपर उड़ गया।" इस मामले में, एक लेम्माटाइज़र सभी संभावित मूल रूपों का निर्माण कर सकता है।
*   **स्टेमर**: एक शब्द को ज्ञात प्रत्ययों को हटा या प्रतिस्थापित करके उसके स्टेम में घटित करता है। परिणामी स्टेम जरूरी नहीं कि एक मान्य शब्द हो। उदाहरण के लिए, पोर्टर अंग्रेजी स्टेमर "running" को "run" में, "business" को "busi" (जो एक मान्य शब्द नहीं है) में घटित करता है, और "octopi" को बिल्कुल भी नहीं घटित करता है।
*   **ध्वन्यात्मक एल्गोरिदम**: शब्दों को ध्वन्यात्मक कोड के साथ प्रतिस्थापित करते हैं जो समान होते हैं, भले ही शब्द भिन्न हों लेकिन ध्वन्यात्मक रूप से निकट हों।
*   **शब्द तोड़ने के एल्गोरिदम**: पाठ को शब्दों में विभाजित करते हैं। वर्तमान में केवल चीनी के लिए उपलब्ध है।
## संरचना
```ini
morphology = morphology1[, morphology2, ...]
```
<!-- उदाहरण संरचना -->
संरचना निर्देश एक सूची निर्धारित करता है कि अनुक्रमित शब्दों पर कौन से संरचना पूर्व-प्रसंस्करण लागू किए जाने हैं। यह एक वैकल्पिक सेटिंग है, जिसमें डिफ़ॉल्ट कोई पूर्व-प्रसंस्करण लागू नहीं है।
मैन्टिकोर में निम्नलिखित के लिए अंतर्निर्मित संरचनात्मक पूर्व-प्रसंस्करण आते हैं:
* अंग्रेजी, रूसी और जर्मन लेम्माटाइज़र
* अंग्रेजी, रूसी, अरबी और चेक स्टेमर
* साउंडएक्स और मेटाफोन ध्वन्यात्मक एल्गोरिदम
* चीनी शब्द तोड़ने का एल्गोरिदम
* स्नोबॉल (libstemmer) स्टेमर  [15 अन्य भाषाओं](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md) के लिए भी उपलब्ध हैं।
लेम्माटाइज़र को शब्दकोश `.pak` फ़ाइलों की आवश्यकता होती है जिन्हें `manticore-language-packs` पैकेज का उपयोग करके स्थापित किया जा सकता है या [मैन्टिकोर वेबसाइट से डाउनलोड किया जा सकता है](https://manticoresearch.com/install/#other-downloads)। बाद के मामले में शब्दकोशों को [lemmatizer_base](../../Server_settings/Common.md#lemmatizer_base) द्वारा निर्दिष्ट निर्देशिका में रखा जाना चाहिए। 

इसके अतिरिक्त, [lemmatizer_cache](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#lemmatizer_cache) सेटिंग का उपयोग अधिक RAM खर्च करके लेम्माटाइजिंग की गति को बढ़ाने के लिए किया जा सकता है।
चीनी भाषा का विभाजन [ICU](http://site.icu-project.org/) या [Jieba](https://github.com/yanyiwu/cppjieba) का उपयोग करके किया जा सकता है (जिसके लिए पैकेज `manticore-language-packs` की आवश्यकता होती है)। दोनों पुस्तकालय n-grams की तुलना में अधिक सटीक विभाजन प्रदान करते हैं, लेकिन थोड़े धीमे होते हैं। [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) में सभी चीनी अक्षरों को शामिल करना आवश्यक है, जो कि `cont`, `cjk` या `chinese` वर्ण सेट का उपयोग करके किया जा सकता है। जब आप `morphology=icu_chinese` या `morphology=jieba_chinese` सेट करते हैं, तो दस्तावेज़ पहले ICU या Jieba द्वारा पूर्व-प्रसंस्कृत होते हैं। फिर, टोकनाइज़र charset_table के अनुसार परिणाम को प्रोसेस करता है, और अंततः, `morphology` विकल्प से अन्य संरचना प्रोसेसर लागू होते हैं। केवल उन पाठ के भाग जो चीनी अक्षरों को शामिल करते हैं, उन्हें ICU/Jieba के लिए विभाजन के लिए दिया जाता है, जबकि अन्य भागों को विभिन्न तरीकों जैसे विभिन्न संरचनाओं या `charset_table` द्वारा संशोधित किया जा सकता है।
अंतर्निर्मित अंग्रेजी और रूसी स्टेमर अपने libstemmer समकक्षों की तुलना में तेज होते हैं लेकिन थोड़ा अलग परिणाम दे सकते हैं।
साउंडेक्स कार्यान्वयन MySQL के समान है। मेटाफोन कार्यान्वयन डबल मेटाफोन एल्गोरिदम पर आधारित है और प्राथमिक कोड को अनुक्रमित करता है।
`morphology` विकल्प का उपयोग करने के लिए, अंतर्निर्मित विकल्पों में से एक या एक से अधिक निर्दिष्ट करें, जिनमें शामिल हैं:
* none: कोई संरचना प्रक्रिया नहीं करें
* lemmatize_ru - रूसी लेम्माटाइज़र लागू करें और एकल मूल रूप चुनें
* lemmatize_uk - यूक्रेनी लेम्माटाइज़र लागू करें और एकल मूल रूप चुनें (पहले इसे [Centos](../../Installation/RHEL_and_Centos.md#Ukrainian-lemmatizer) या [Ubuntu/Debian](../../Installation/Debian_and_Ubuntu.md#Ukrainian-lemmatizer) में स्थापित करें)। लेम्माटाइज़र के सही काम करने के लिए सुनिश्चित करें कि विशेष यूक्रेनी अक्षर आपके `charset_table` में संरक्षित हैं क्योंकि डिफ़ॉल्ट रूप से वे नहीं हैं। इसके लिए उन्हें इस प्रकार ओवरराइड करें: `charset_table='non_cont,U+0406->U+0456,U+0456,U+0407->U+0457,U+0457,U+0490->U+0491,U+0491'`। [यहां](https://play.manticoresearch.com/ua-lemmatizer/) यूक्रेनी लेम्माटाइज़र को स्थापित और उपयोग करने के बारे में एक इंटरैक्टिव पाठ्यक्रम है।
* lemmatize_en - अंग्रेजी लेम्माटाइज़र लागू करें और एकल मूल रूप चुनें
* lemmatize_de - जर्मन लेम्माटाइज़र लागू करें और एकल मूल रूप चुनें
* lemmatize_ru_all - रूसी लेम्माटाइज़र लागू करें और सभी संभावित मूल रूपों को अनुक्रमित करें
* lemmatize_uk_all - यूक्रेनी लेम्माटाइज़र लागू करें और सभी संभावित मूल रूपों को अनुक्रमित करें। स्थापना के लिंक ऊपर देखें और `charset_table` का ध्यान रखें।
* lemmatize_en_all - अंग्रेजी लेम्माटाइज़र लागू करें और सभी संभावित मूल रूपों को अनुक्रमित करें
* lemmatize_de_all - जर्मन लेम्माटाइज़र लागू करें और सभी संभावित मूल रूपों को अनुक्रमित करें
* stem_en - पोर्टर की अंग्रेजी स्टेमर लागू करें
* stem_ru - पोर्टर का रूसी स्टेमर लागू करें
* stem_enru - पोर्टर की अंग्रेजी और रूसी स्टेमर लागू करें
* stem_cz - चेक स्टेमर लागू करें
* stem_ar - अरबी स्टेमर लागू करें
* soundex - कीवर्ड को उनके SOUNDEX कोड के साथ प्रतिस्थापित करें
* metaphone - कीवर्ड को उनके METAPHONE कोड के साथ प्रतिस्थापित करें
* icu_chinese - ICU का उपयोग करके चीनी पाठ विभाजन लागू करें
* jieba_chinese - Jieba का उपयोग करके चीनी पाठ विभाजन लागू करें (जिसके लिए पैकेज `manticore-language-packs` की आवश्यकता होती है)
* libstemmer_* . विवरण के लिए [समर्थित भाषाओं की सूची](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md) देखें
कई स्टेमर निर्दिष्ट किए जा सकते हैं, जिन्हें अल्पविराम से अलग किया जाएगा। इन्हें उन आने वाले शब्दों पर लागू किया जाएगा जिन क्रम में इन्हें सूचीबद्ध किया गया है, और जब इनमें से कोई एक स्टेमर शब्द को संशोधित करता है तो प्रोसेसिंग रुकेगी। इसके अतिरिक्त, जब [शब्द रूप](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) फीचर सक्षम किया जाता है, तो शब्द को पहले शब्द रूप के शब्दकोश में देखा जाएगा। यदि शब्दकोश में कोई मिलान प्रविष्टि है, तो स्टेमर बिल्कुल भी लागू नहीं किए जाएंगे। [शब्द रूप](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) का उपयोग स्टेमिंग अपवाद लागू करने के लिए किया जा सकता है।

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) morphology = 'stem_en, libstemmer_sv'
```

<!-- request JSON -->

```json
POST /cli -d "CREATE TABLE products(title text, price float)  morphology = 'stem_en, libstemmer_sv'"
```

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'morphology' => 'stem_en, libstemmer_sv'
        ]);
```
<!-- intro -->
##### पायथन:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) morphology = 'stem_en, libstemmer_sv'')
```

<!-- intro -->
##### पायथन-असिंक्रोनस:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = 'stem_en, libstemmer_sv'')
```

<!-- intro -->
##### जावास्क्रिप्ट:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = 'stem_en, libstemmer_sv'');
```

<!-- intro -->
##### जावा:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology = 'stem_en, libstemmer_sv'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology = 'stem_en, libstemmer_sv'", true);
```

<!-- intro -->
##### रस्ट:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) morphology = 'stem_en, libstemmer_sv'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  morphology = stem_en, libstemmer_sv

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## morphology_skip_fields

<!-- example morphology_skip_fields -->

```ini
morphology_skip_fields = field1[, field2, ...]
```

मोर्फोलॉजी पूर्वप्रक्रिया को छोड़ने के लिए फ़ील्ड की एक सूची। वैकल्पिक, डिफ़ॉल्ट खाली है (सभी फ़ील्ड पर पूर्वप्रसंस्करण लागू करें)।

<!-- request SQL -->

```sql
CREATE TABLE products(title text, name text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, name text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'"
```

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'morphology_skip_fields' => 'name',
            'morphology' => 'stem_en'
        ]);
```
<!-- intro -->
##### पायथन:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'')
```

<!-- intro -->
##### पायथन-असिंक्रोनस:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'')
```

<!-- intro -->
##### जावास्क्रिप्ट:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'');
```

<!-- intro -->
##### जावा:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'", true);
```

<!-- intro -->
##### रस्ट:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  morphology_skip_fields = name
  morphology = stem_en

  type = rt
  path = tbl
  rt_field = title
  rt_field = name
  rt_attr_uint = price
}
```
<!-- end -->

## min_stemming_len

<!-- example min_stemming_len -->

```ini
min_stemming_len = length
```

स्टेमिंग सक्षम करने के लिए न्यूनतम शब्द की लंबाई। वैकल्पिक, डिफ़ॉल्ट 1 है (सभी को स्टेम करें)।
स्टेमर्स परिपूर्ण नहीं होते हैं, और कभी-कभी अप्रत्याशित परिणाम उत्पन्न कर सकते हैं। उदाहरण के लिए, "gps" कीवर्ड को अंग्रेजी के पॉर्टर स्टेमर के माध्यम से चलाने पर "gp" परिणाम मिलता है, जो वास्तव में इरादा नहीं है। `min_stemming_len` विशेषता आपको स्रोत शब्द की लंबाई के आधार पर स्टेमिंग को दबाने की अनुमति देती है, अर्थात्, बहुत छोटे शब्दों की स्टेमिंग से बचने के लिए। जो कीवर्ड दिए गए थ्रेशोल्ड से छोटे हैं, उन्हें स्टेम नहीं किया जाएगा। ध्यान दें कि जो कीवर्ड ठीक उतनी लंबाई के हैं जितनी निर्दिष्ट की गई है **उनकी** स्टेमिंग की जाएगी। इसलिए 3-चरित्र के कीवर्ड की स्टेमिंग से बचने के लिए, आपको मान के लिए 4 निर्दिष्ट करना चाहिए। अधिक बारीकी से नियंत्रण के लिए, [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) विशेषता से संदर्भित करें।

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'"
```

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'min_stemming_len' => '4',
             'morphology' => 'stem_en'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asycnio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  min_stemming_len = 4
  morphology = stem_en

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## index_exact_words

<!-- example index_exact_words -->

```ini
index_exact_words = {0|1}
```

यह विकल्प मूल कीवर्डों के अनुक्रमण की अनुमति देता है साथ ही उनके रूपात्मक रूपांतरित संस्करणों के। हालांकि, मूल कीवर्ड जो [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) और [exceptions](../../Creating_a_table/NLP_and_tokenization/Exceptions.md) द्वारा पुनःमैप किए गए हैं, अनुक्रमित नहीं किए जा सकते। डिफ़ॉल्ट मान 0 है, जो दर्शाता है कि यह विशेषता डिफ़ॉल्ट रूप से निष्क्रिय है।

यह [exact form operator](../../Searching/Full_text_matching/Operators.md#Exact-form-modifier) का उपयोग करने की अनुमति देता है क्वेरी भाषा में। इस विशेषता को सक्षम करने से पूर्ण-पाठ अनुक्रमण की मात्रा और अनुक्रमण समय बढ़ेगा, लेकिन यह खोज प्रदर्शन पर प्रभाव नहीं डालेगा।

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'"
```

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'index_exact_words' => '1',
             'morphology' => 'stem_en'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  index_exact_words = 1
  morphology = stem_en

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## jieba_hmm

<!-- example jieba_hmm -->

```ini
jieba_hmm = {0|1}
```

Jieba विभाजन उपकरण में HMM को सक्षम या अक्षम करें। वैकल्पिक; डिफ़ॉल्ट 1 है।
Jieba में, HMM (Hidden Markov Model) विकल्प उस एल्गोरिदम को संदर्भित करता है जिसका उपयोग शब्द विभाजन के लिए किया जाता है। विशेष रूप से, यह Jieba को अज्ञात शब्दों को पहचानने द्वारा चीनी शब्द विभाजन करने की अनुमति देता है, विशेष रूप से वे जो इसके शब्दकोश में नहीं हैं।

Jieba मुख्य रूप से ज्ञात शब्दों को विभाजित करने के लिए एक शब्दकोश-आधारित विधि का उपयोग करता है, लेकिन जब HMM विकल्प सक्षम किया जाता है, तो यह एक सांख्यिकीय मॉडल लागू करता है जो उन शब्दों या वाक्यांशों के लिए संभावित शब्द सीमाओं की पहचान करने के लिए है जो इसके शब्दकोश में नहीं हैं। यह नए या दुर्लभ शब्दों, नामों और स्लैंग को विभाजित करने के लिए विशेष रूप से उपयोगी है।

संक्षेप में, `jieba_hmm` विकल्प विभाजन की सटीकता में सुधार करने में मदद करता है जबकि अनुक्रमण प्रदर्शन की लागत पर। इसका उपयोग `morphology = jieba_chinese` के साथ किया जाना चाहिए, देखें [चीनी, जापानी और कोरियाई (CJK) और थाई भाषाएँ](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)।

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_hmm = '0'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_hmm = '0'"
```

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[

morphology' => 'jieba_chinese',


jieba_hmm'='1'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_hmm = '0'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_hmm = '0'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_hmm = '0'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_hmm = '0'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_hmm = '0'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_hmm = '0'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  morphology = jieba_chinese
  jieba_hmm = 0

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## jieba_mode

<!-- example jieba_mode -->

```ini
jieba_mode = {accurate|full|search}
```

Jieba विभाजन मोड। वैकल्पिक; डिफ़ॉल्ट `सटीक` है।

सटीक मोड में, Jieba वाक्य को शब्दकोश मिलान का उपयोग करके सबसे सटीक शब्दों में विभाजित करता है। यह मोड सटीकता पर ध्यान केंद्रित करता है, यह सुनिश्चित करते हुए कि विभाजन यथासंभव सटीक है।

पूर्ण मोड में, Jieba वाक्य को प्रत्येक संभव शब्द संयोजन में विभाजित करने की कोशिश करता है, सभी संभावित शब्दों को शामिल करने का लक्ष्य रखता है। यह मोड पुनः प्राप्ति को अधिकतम करने पर ध्यान केंद्रित करता है, जिसका अर्थ है कि यह जितने संभव हो सके उतने शब्दों की पहचान करता है, भले ही उनमें से कुछ ओवरलैप करते हों या कम सामान्य उपयोग में हों। यह अपने शब्दकोश में पाए गए सभी शब्दों को लौटाता है।

खोज मोड में, Jieba पाठ को पूरे शब्दों और छोटे भागों में तोड़ता है, अतिरिक्त विवरण प्रदान करते हुए सटीक विभाजन के साथ ओवरलैपिंग शब्द टुकड़ों को मिलाता है। यह मोड सटीकता और पुनः प्राप्ति के बीच संतुलन बनाता है, जिससे यह खोज इंजन के लिए उपयोगी हो जाता है।

`jieba_mode` का उपयोग `morphology = jieba_chinese` के साथ किया जाना चाहिए। देखें [चीनी, जापानी, कोरियाई (CJK) और थाई भाषाएँ](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)।

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_mode = 'full'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_mode = 'full'"
```

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[

morphology' => 'jieba_chinese',


jieba_mode'='full'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_mode = 'full'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_mode = 'full'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_mode = 'full'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_mode = 'full'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_mode = 'full'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_mode = 'full'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  morphology = jieba_chinese
  jieba_mode = full

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## jieba_user_dict_path

<!-- example jieba_user_dict_path -->

```ini
jieba_user_dict_path = path/to/stopwords/file
```

Jieba उपयोगकर्ता शब्दकोश का पथ। वैकल्पिक।

Jieba, एक चीनी पाठ विभाजन पुस्तकालय, शब्द विभाजन में सहायता के लिए शब्दकोश फ़ाइलों का उपयोग करता है। इन शब्दकोश फ़ाइलों का फ़ॉर्मेट इस प्रकार है: प्रत्येक पंक्ति में एक शब्द होता है, जिसे स्पेस से विभाजित तीन भागों में बाँटा गया है - शब्द स्वयं, शब्द आवृत्ति, और भाषाशास्त्र (POS) टैग। शब्द आवृत्ति और POS टैग वैकल्पिक हैं और छोड़े जा सकते हैं। शब्दकोश फ़ाइल UTF-8 में कोडित होनी चाहिए।

उदाहरण:

```
创新办 3 i
云计算 5
凱特琳 nz
台中
```

`jieba_user_dict_path` का उपयोग `morphology = jieba_chinese` के साथ किया जाना चाहिए। अधिक विवरण के लिए देखें [चीनी, जापानी, कोरियाई (CJK), और थाई भाषाएँ](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md).

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_user_dict_path = '/usr/local/manticore/data/user-dict.txt'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_user_dict_path = '/usr/local/manticore/data/user-dict.txt'"
```

<!-- request PHP -->

```php
$index = new ManticoresearchIndex($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[

morphology' => 'jieba_chinese',
             'jieba_user_dict_path' = '/usr/local/manticore/data/user-dict.txt'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_user_dict_path = '/usr/local/manticore/data/user-dict.txt'')
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_user_dict_path = '/usr/local/manticore/data/user-dict.txt'')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_user_dict_path = '/usr/local/manticore/data/user-dict.txt'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_user_dict_path = '/usr/local/manticore/data/user-dict.txt'", true);
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_user_dict_path = '/usr/local/manticore/data/user-dict.txt'", true);
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_user_dict_path = '/usr/local/manticore/data/user-dict.txt'", Some(true)).await;
```

<!-- request CONFIG -->

```ini
table products {
  morphology = jieba_chinese
  jieba_user_dict_path = /usr/local/manticore/data/user-dict.txt

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->
<!-- proofread -->









# उन्नत रूपविज्ञान

रूपविज्ञान पूर्व-प्रसंस्करण को शब्दों पर अनुक्रमण के दौरान लागू किया जा सकता है ताकि एक ही शब्द के विभिन्न रूपों को सामान्यीकृत किया जा सके और विभाजन में सुधार किया जा सके। उदाहरण के लिए, एक अंग्रेजी स्टेमर "dogs" और "dog" को "dog" में सामान्यीकृत कर सकता है, जिसके परिणामस्वरूप दोनों कीवर्ड के लिए समान खोज परिणाम प्राप्त होते हैं।

मैन्टिकोर में चार अंतर्निर्मित रूपविज्ञान पूर्व-प्रसंस्करण होते हैं:

*   **लेम्माटाइज़र**: एक शब्द को इसके मूल या लेम्मा में कम करता है। उदाहरण के लिए, "running" को "run" में और "octopi" को "octopus" में कम किया जा सकता है। ध्यान दें कि कुछ शब्दों के कई संबंधित मूल रूप हो सकते हैं। उदाहरण के लिए, "dove" "dive" का अतीत रूप हो सकता है या एक पक्षी का अर्थ वाले संज्ञा के रूप में, जैसे "एक सफेद कबूतर कुकू के घोंसले के ऊपर उड़ गया।" इस मामले में, एक लेम्माटाइज़र सभी संभावित मूल रूपों को उत्पन्न कर सकता है।
*   **स्टेमर**: एक शब्द को उसके स्टेम में कम करता है, कुछ ज्ञात प्रत्ययों को हटाकर या बदलकर। परिणामी स्टेम जरूरी नहीं कि एक मान्य शब्द हो। उदाहरण के लिए, पोर्टर अंग्रेजी स्टेमर "running" को "run" में, "business" को "busi" (एक मान्य शब्द नहीं) में, और "octopi" को बिल्कुल कम नहीं करता।
*   **ध्वनिक एल्गोरिदम**: शब्दों को ध्वनिक कोड के साथ बदलते हैं जो समान होते हैं, भले ही शब्द भिन्न हों लेकिन ध्वनिक रूप से निकट हों।
*   **शब्द तोड़ने वाले एल्गोरिदम**: पाठ को शब्दों में विभाजित करते हैं। वर्तमान में केवल चीनी के लिए उपलब्ध है।

## रूपविज्ञान

```ini
morphology = morphology1[, morphology2, ...]
```

<!-- example morphology -->
रूपविज्ञान निर्देश एक सूची निर्दिष्ट करता है जो शब्दों पर लागू होने वाले रूपविज्ञान पूर्व-प्रसंस्करण को निर्धारित करता है। यह एक वैकल्पिक सेटिंग है, जिसका डिफ़ॉल्ट कोई पूर्व-प्रसंस्करण लागू नहीं होता।

मैन्टिकोर में निम्न के लिए अंतर्निर्मित रूपविज्ञान पूर्व-प्रसंस्करण हैं:

* अंग्रेजी, रूसी, और जर्मन लेम्माटाइज़र
* अंग्रेजी, रूसी, अरबी, और चेक स्टेमर
* साउंडएक्स और मेटाफोन ध्वनिक एल्गोरिदम
* चीनी शब्द तोड़ने वाला एल्गोरिदम
* [15 अन्य भाषाओं](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md) के लिए स्नोबॉल (libstemmer) स्टेमर भी उपलब्ध हैं।

लेम्माटाइज़र को शब्दकोश `.pak` फ़ाइलों की आवश्यकता होती है, जिन्हें `manticore-language-packs` पैकेजों का उपयोग करके स्थापित किया जा सकता है या [मैन्टिकोर वेबसाइट से डाउनलोड किया जा सकता है](https://manticoresearch.com/install/#other-downloads)। अंतिम मामले में, शब्दकोशों को [lemmatizer_base](../../Server_settings/Common.md#lemmatizer_base) द्वारा निर्दिष्ट निर्देशिका में रखा जाना चाहिए। 

इसके अतिरिक्त, [lemmatizer_cache](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#lemmatizer_cache) सेटिंग को बिना संकुचित शब्दकोश कैश के लिए अधिक RAM खर्च करके लेम्माटाइजिंग को तेज करने के लिए उपयोग किया जा सकता है।

चीनी भाषा का विभाजन [ICU](http://site.icu-project.org/) या [Jieba](https://github.com/yanyiwu/cppjieba) का उपयोग करके किया जा सकता है (पैकेज `manticore-language-packs` की आवश्यकता होती है)। दोनों पुस्तकालय n-grams की तुलना में अधिक सटीक विभाजन प्रदान करते हैं, लेकिन थोड़े धीमे होते हैं। [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) में सभी चीनी वर्ण शामिल होने चाहिए, जिसकी पूर्ति `cont`, `cjk` या `chinese` वर्ण सेटों का उपयोग करके की जा सकती है। जब आप सेट करते हैं `morphology=icu_chinese` या `morphology=jieba_chinese`, दस्तावेजों को पहले ICU या Jieba द्वारा पूर्व-प्रसंस्कृत किया जाता है। फिर, टोकनाइज़र charset_table के अनुसार परिणाम को संसाधित करता है, और अंततः, `morphology` विकल्प से अन्य रूपविज्ञान प्रोसेसर लागू होते हैं। केवल उन्हीं भागों का पाठ जो चीनी होते हैं, को विभाजन के लिए ICU/Jieba को भेजा जाता है, जबकि अन्य भागों को विभिन्न तरीकों से संशोधित किया जा सकता है, जैसे कि विभिन्न रूपविज्ञान या `charset_table`।

अंतर्निर्मित अंग्रेजी और रूसी स्टेमर अपने libstemmer समकक्षों की तुलना में तेज होते हैं लेकिन थोड़ी भिन्न परिणाम उत्पन्न कर सकते हैं

साउंडेक्स कार्यान्वयन MySQL के कार्यान्वयन से मेल खाती है। मेटाफोन कार्यान्वयन डबल मेटाफोन एल्गोरिदम पर आधारित है और प्राथमिक कोड का अनुक्रमण करता है।

`morphology` विकल्प का उपयोग करने के लिए, अंतर्निर्मित विकल्पों में से एक या कई निर्दिष्ट करें, जिनमें शामिल हैं:
* none: कोई रूपविज्ञान प्रसंस्करण नहीं करें
* lemmatize_ru - रूसी लेम्माटाइज़र लागू करें और एकल मूल रूप चुनें
* lemmatize_uk - यूक्रेनी लेम्माटाइज़र लागू करें और एकल मूल रूप चुनें (पहले इसे [Centos](../../Installation/RHEL_and_Centos.md#Ukrainian-lemmatizer) या [Ubuntu/Debian](../../Installation/Debian_and_Ubuntu.md#Ukrainian-lemmatizer) में स्थापित करें)। लेम्माटाइज़र का सही कार्य सुनिश्चित करने के लिए सुनिश्चित करें कि आपकी `charset_table` में विशिष्ट यूक्रेनी वर्ण सुरक्षित रखे गए हैं, क्योंकि डिफ़ॉल्ट रूप से ये नहीं होते। इसके लिए इन्हें अधिभूत करें, जैसे: `charset_table='non_cont,U+0406->U+0456,U+0456,U+0407->U+0457,U+0457,U+0490->U+0491,U+0491'`। [यहां](https://play.manticoresearch.com/ua-lemmatizer/) यूक्रेनी लेम्माटाइज़र को स्थापित करने और उपयोग करने के बारे में एक इंटरैक्टिव पाठ्यक्रम है।
* lemmatize_en - अंग्रेजी लेम्माटाइज़र लागू करें और एकल मूल रूप चुनें
* lemmatize_de - जर्मन लेम्माटाइज़र लागू करें और एकल मूल रूप चुनें
* lemmatize_ru_all - रूसी लेम्माटाइज़र लागू करें और सभी संभावित मूल रूपों का अनुक्रमण करें
* lemmatize_uk_all - यूक्रेनी लेम्माटाइज़र लागू करें और सभी संभावित मूल रूपों का अनुक्रमण करें। स्थापना के लिए लिंक ऊपर दिए गए हैं और `charset_table` का ध्यान रखें।
* lemmatize_en_all - अंग्रेजी लेम्माटाइज़र लागू करें और सभी संभावित मूल रूपों का अनुक्रमण करें
* lemmatize_de_all - जर्मन लेम्माटाइज़र लागू करें और सभी संभावित मूल रूपों का अनुक्रमण करें
* stem_en - पोर्टर का अंग्रेजी स्टेमर लागू करें
* stem_ru - पोर्टर का रूसी स्टेमर लागू करें
* stem_enru - पोर्टर का अंग्रेजी और रूसी स्टेमर लागू करें
* stem_cz - चेक स्टेमर लागू करें
* stem_ar - अरबी स्टेमर लागू करें
* soundex - कीवर्ड को उनके SOUNDEX कोड के साथ बदलें
* metaphone - कीवर्ड को उनके METAPHONE कोड के साथ बदलें
* icu_chinese - ICU का उपयोग करके चीनी पाठ विभाजन लागू करें
* jieba_chinese - Jieba का उपयोग करके चीनी पाठ विभाजन लागू करें (पैकेज `manticore-language-packs` की आवश्यकता होती है)
* libstemmer_* . विवरण के लिए [समर्थित भाषाओं की सूची](../../Creating_a_table/NLP_and_tokenization/Supported_languages.md) देखें
कई स्टेमर निर्दिष्ट किए जा सकते हैं, जिन्हें अल्पविराम से अलग किया जाता है। वे उस क्रम में आने वाले शब्दों पर लागू किए जाएंगे जिसमें वे सूचीबद्ध हैं, और प्रक्रिया तब रोक दी जाएगी जब उनमें से एक स्टेमर शब्द को संशोधित करता है। इसके अतिरिक्त, जब [शब्दरूप](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) सुविधा सक्षम होती है, तो सबसे पहले शब्द को शब्द रूप शब्दकोश में देखा जाएगा। यदि शब्दकोश में एक मेल खाती प्रविष्टि है, तो स्टेमर लागू नहीं किए जाएंगे। [शब्दरूप](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) का उपयोग स्टेमिंग अपवादों को लागू करने के लिए किया जा सकता है।

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) morphology = 'stem_en, libstemmer_sv'
```

<!-- request JSON -->

```json
POST /cli -d "CREATE TABLE products(title text, price float)  morphology = 'stem_en, libstemmer_sv'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'morphology' => 'stem_en, libstemmer_sv'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'stem_en, libstemmer_sv\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'stem_en, libstemmer_sv\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology = 'stem_en, libstemmer_sv'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology = 'stem_en, libstemmer_sv'");
```

<!-- request CONFIG -->

```ini
table products {
  morphology = stem_en, libstemmer_sv

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## morphology_skip_fields

<!-- example morphology_skip_fields -->

```ini
morphology_skip_fields = field1[, field2, ...]
```

रूपांतर पूर्व प्रसंस्करण को छोड़ने के लिए फ़ील्ड की एक सूची। वैकल्पिक, डिफ़ॉल्ट खाली है (सभी फ़ील्ड पर पूर्व प्रसंस्करण लागू करें)।

<!-- request SQL -->

```sql
CREATE TABLE products(title text, name text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, name text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'morphology_skip_fields' => 'name',
            'morphology' => 'stem_en'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) morphology_skip_fields = \'name\' morphology = \'stem_en\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology_skip_fields = \'name\' morphology = \'stem_en\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology_skip_fields = 'name' morphology = 'stem_en'");
```

<!-- request CONFIG -->

```ini
table products {
  morphology_skip_fields = name
  morphology = stem_en

  type = rt
  path = tbl
  rt_field = title
  rt_field = name
  rt_attr_uint = price
}
```
<!-- end -->

## min_stemming_len

<!-- example min_stemming_len -->

```ini
min_stemming_len = length
```

स्टेमिंग सक्षम करने के लिए न्यूनतम शब्द की लंबाई। वैकल्पिक, डिफ़ॉल्ट 1 है (सब कुछ स्टेम करें)।

स्टेमर्स परिपूर्ण नहीं होते हैं, और कभी-कभी undesired परिणाम उत्पन्न कर सकते हैं। उदाहरण के लिए, "gps" कीवर्ड को अंग्रेजी के पोर्टर स्टेमर के माध्यम से चलाने से "gp" के परिणाम मिलते हैं, जो वास्तव में इरादा नहीं है। `min_stemming_len` सुविधा आपको स्रोत शब्द की लंबाई के आधार पर स्टेमिंग को रोकने की अनुमति देती है, यानी। बहुत छोटे शब्दों की स्टेमिंग से बचने के लिए। जो कीवर्ड दिए गए थ्रेशोल्ड से छोटे होते हैं उन्हें स्टेम नहीं किया जाएगा। ध्यान दें कि जो कीवर्ड ठीक से निर्दिष्ट लंबाई के होते हैं **स्टेम** किए जाएंगे। इसलिए 3-चर कीवर्ड की स्टेमिंग से बचने के लिए, आपको मान के लिए 4 निर्दिष्ट करना चाहिए। अधिक बारीक नियंत्रण के लिए, [शब्दरूप](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) सुविधा का संदर्भ लें।

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'min_stemming_len' => '4',
             'morphology' => 'stem_en'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) min_stemming_len = \'4\' morphology = \'stem_en\'')
```
<!-- intro -->
##### Javascript:
<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) min_stemming_len = \'4\' morphology = \'stem_en\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) min_stemming_len = '4' morphology = 'stem_en'");
```

<!-- request CONFIG -->

```ini
table products {
  min_stemming_len = 4
  morphology = stem_en

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## index_exact_words

<!-- example index_exact_words -->

```ini
index_exact_words = {0|1}
```

यह विकल्प मूल कीवर्ड को उनके आकृतिविज्ञान संशोधित संस्करणों के साथ इंडेक्स करने की अनुमति देता है। हालांकि, [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) और [exceptions](../../Creating_a_table/NLP_and_tokenization/Exceptions.md) द्वारा पुनर्मानचित्रित मूल कीवर्ड को इंडेक्स नहीं किया जा सकता। डिफ़ॉल्ट मान 0 है, जो दर्शाता है कि यह सुविधा डिफ़ॉल्ट रूप से अक्षम है।

यह क्वेरी भाषा में [सटीक रूप ऑपरेटर](../../Searching/Full_text_matching/Operators.md#Exact-form-modifier) का उपयोग करने की अनुमति देता है। इस सुविधा को सक्षम करने से पूर्ण-पाठ इंडेक्स का आकार और इंडेक्सिंग समय बढ़ जाएगा, लेकिन खोज प्रदर्शन पर प्रभाव नहीं पड़ेगा।

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
             'index_exact_words' => '1',
             'morphology' => 'stem_en'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) index_exact_words = \'1\' morphology = \'stem_en\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) index_exact_words = \'1\' morphology = \'stem_en\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) index_exact_words = '1' morphology = 'stem_en'");
```

<!-- request CONFIG -->

```ini
table products {
  index_exact_words = 1
  morphology = stem_en

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## jieba_hmm

<!-- example jieba_hmm -->

```ini
jieba_hmm = {0|1}
```

Hidden Markov Model (HMM) को Jieba विभाजन उपकरण में सक्षम या अक्षम करें। वैकल्पिक; डिफ़ॉल्ट 1 है।

Jieba में, HMM (Hidden Markov Model) विकल्प शब्द विभाजन के लिए एक एल्गोरिदम को संदर्भित करता है। विशेष रूप से, यह Jieba को अपने शब्दकोश में मौजूद न होने वाले अज्ञात शब्दों को पहचानने में मदद करता है।

Jieba मुख्य रूप से ज्ञात शब्दों को विभाजित करने के लिए शब्दकोश-आधारित विधि का उपयोग करता है, लेकिन जब HMM विकल्प सक्षम होता है, तो यह अपने शब्दकोश में न होने वाले शब्दों या वाक्यांशों के संभावित शब्द सीमाओं को पहचानने के लिए एक सांख्यिकीय मॉडल लागू करता है। यह नए या दुर्लभ शब्दों, नामों और स्लैंग को विभाजित करने में विशेष रूप से उपयोगी है।

संक्षेप में, `jieba_hmm` विकल्प इंडेक्सिंग प्रदर्शन की कीमत पर विभाजन की सटीकता में सुधार करता है। इसका उपयोग `morphology = jieba_chinese` के साथ किया जाना चाहिए, देखें [चीनी, जापानी और कोरियाई (CJK) और थाई भाषाएं](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)।

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_hmm = '0'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_hmm = '0'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
			 'morphology' => 'jieba_chinese',
	  		 'jieba_hmm'='1'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_hmm = \'0\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_hmm = \'0\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_hmm = '0'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_hmm = '0'");
```

<!-- request CONFIG -->

```ini
table products {
  morphology = jieba_chinese
  jieba_hmm = 0

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## jieba_mode

<!-- example jieba_mode -->

```ini
jieba_mode = {सटीक|पूर्ण|खोज}
```

Jieba विभाजन मोड। वैकल्पिक; डिफ़ॉल्ट `सटीक` है।

सटीक मोड में, Jieba वाक्य को शब्दकोश मिलान का उपयोग करके सबसे सटीक शब्दों में विभाजित करता है। यह मोड सटीकता पर ध्यान केंद्रित करता है, यह सुनिश्चित करता है कि विभाजन यथासंभव सटीक है।

पूर्ण मोड में, Jieba वाक्य को हर संभव शब्द संयोजन में विभाजित करने की कोशिश करता है, सभी संभावित शब्दों को शामिल करने का लक्ष्य रखता है। यह मोड पुनः प्राप्ति को अधिकतम करने पर ध्यान केंद्रित करता है, जिसका अर्थ है कि यह जितने संभव हो सके उतने शब्दों की पहचान करता है, भले ही उनमें से कुछ ओवरलैप हो या कम सामान्य रूप से उपयोग किए जाते हों। यह अपने शब्दकोश में पाई गई सभी शब्द लौटाता है।

खोज मोड में, Jieba टेक्स्ट को पूरे शब्दों और छोटे हिस्सों में विभाजित करता है, सटीक विभाजन के साथ अतिरिक्त विवरण प्रदान करते हुए ओवरलैपिंग शब्द खंडों को जोड़ता है। यह मोड सटीकता और पुनः प्राप्ति का संतुलन बनाता है, जिससे यह खोज इंजन के लिए उपयोगी होता है।

`jieba_mode` का उपयोग `morphology = jieba_chinese` के साथ किया जाना चाहिए। देखें [चीन, जापान, कोरिया (CJK) और थाई भाषाएँ](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)।

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_mode = 'full'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_mode = 'full'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
			 'morphology' => 'jieba_chinese',
	  		 'jieba_mode'='full'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_mode = \'full\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_mode = \'full\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_mode = 'full'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_mode = 'full'");
```

<!-- request CONFIG -->

```ini
table products {
  morphology = jieba_chinese
  jieba_mode = full

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## jieba_user_dict_path

<!-- example jieba_user_dict_path -->

```ini
jieba_user_dict_path = path/to/stopwords/file
```

Jieba उपयोगकर्ता शब्दकोश का पथ। वैकल्पिक।

Jieba, एक चीनी टेक्स्ट विभाजन पुस्तकालय, शब्द विभाजन सहायता के लिए शब्दकोष फ़ाइलों का उपयोग करता है। इन शब्दकोष फ़ाइलों का स्वरूप इस प्रकार है: प्रत्येक पंक्ति में एक शब्द होता है, जिसे तीन भागों में स्पेस द्वारा विभाजित किया जाता है - स्वयं शब्द, शब्द आवृत्ति, और भागों का भाषाई टैग (POS)। शब्द आवृत्ति और POS टैग वैकल्पिक होते हैं और छोड़े जा सकते हैं। शब्दकोष फ़ाइल का UTF-8 एन्कोडेड होना आवश्यक है।

उदाहरण:

```
创新办 3 i
云计算 5
凱特琳 nz
台中
```

`jieba_user_dict_path` का उपयोग `morphology = jieba_chinese` के साथ किया जाना चाहिए। अधिक जानकारी के लिए, देखें [चीनी, जापानी, कोरियाई (CJK) और थाई भाषाएँ](Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md)।

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_user_dict_path = '/usr/local/manticore/data/user-dict.txt'
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_user_dict_path = '/usr/local/manticore/data/user-dict.txt'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
			 'morphology' => 'jieba_chinese',
             'jieba_user_dict_path' = '/usr/local/manticore/data/user-dict.txt'
        ]);

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_user_dict_path = \'/usr/local/manticore/data/user-dict.txt\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) morphology = \'jieba_chinese\' jieba_user_dict_path = \'/usr/local/manticore/data/user-dict.txt\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_user_dict_path = '/usr/local/manticore/data/user-dict.txt'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) morphology = 'jieba_chinese' jieba_user_dict_path = '/usr/local/manticore/data/user-dict.txt'");
```

<!-- request CONFIG -->

```ini
table products {
  morphology = jieba_chinese
  jieba_user_dict_path = /usr/local/manticore/data/user-dict.txt

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->
<!-- proofread -->


