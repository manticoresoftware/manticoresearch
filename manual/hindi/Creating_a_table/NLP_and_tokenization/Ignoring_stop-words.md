# स्टॉप शब्दों को अनदेखा करना

स्टॉप शब्द वे शब्द होते हैं जिन्हें अनुक्रमण और खोजने के दौरान अनदेखा किया जाता है, आमतौर पर उनकी उच्च आवृत्ति और खोज परिणामों के लिए कम मूल्य के कारण। 

Manticore सर्च डिफ़ॉल्ट रूप से स्टॉप शब्दों पर [स्टेमिंग](../../Creating_a_table/NLP_and_tokenization/Morphology.md) लागू करता है, जो अनपेक्षित परिणामों की ओर ले जा सकता है, लेकिन इसे [stopwords_unstemmed](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords_unstemmed) का उपयोग करके बंद किया जा सकता है।

छोटे स्टॉप शब्द फ़ाइलें तालिका शीर्षक में संग्रहीत की जाती हैं, और उन फ़ाइलों के आकार पर एक सीमा होती है जो एम्बेड की जा सकती हैं, जैसा कि [embedded_limit](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#embedded_limit) विकल्प द्वारा परिभाषित किया गया है।

स्टॉप शब्दों को अनुक्रमित नहीं किया जाता है, लेकिन वे कीवर्ड की स्थिति को प्रभावित करते हैं। उदाहरण के लिए, यदि "the" एक स्टॉप शब्द है, और दस्तावेज़ 1 में वाक्यांश "in office" है जबकि दस्तावेज़ 2 में वाक्यांश "in the office" है, तो "in office" को सही वाक्यांश के रूप में खोजने पर केवल पहला दस्तावेज़ वापस आएगा, हालांकि "the" को दूसरे दस्तावेज़ में स्टॉप शब्द के रूप में छोड़ दिया गया है। इस व्यवहार को [stopword_step](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopword_step) निर्देशिका का उपयोग करके संशोधित किया जा सकता है।

## स्टॉपवर्ड्स

```ini
stopwords=path/to/stopwords/file[ path/to/another/file ...]
```

<!-- example stopwords -->
स्टॉपवर्ड्स सेटिंग वैकल्पिक है और डिफ़ॉल्ट रूप से खाली है। यह आपको एक या एक से अधिक स्टॉप शब्द फ़ाइलों के पथ को निर्दिष्ट करने की अनुमति देता है, जो रिक्तियों से अलग होती हैं। सभी फ़ाइलें लोड की जाएँगी। वास्तविक समय मोड में, केवल पूर्ण पथ ही अनुमति है।

स्टॉप शब्द फ़ाइल प्रारूप साधारण प्लेन टेक्स्ट है जिसमें UTF-8 एन्कोडिंग है। फ़ाइल डेटा को [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) सेटिंग्स के संदर्भ में टोकनाइज़ किया जाएगा, इसलिए आप अनुक्रमित डेटा में समान विभाजक का उपयोग कर सकते हैं।

स्टॉप शब्द फ़ाइलें मैन्युअल रूप से या अर्ध-स्वचालित रूप से बनाई जा सकती हैं। [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) एक मोड प्रदान करता है जो तालिका का फ़्रीक्वेंसी डिक्शनरी बनाता है, जिसे कीवर्ड की आवृत्ति के अनुसार क्रमबद्ध किया जाता है। उस डिक्शनरी से शीर्ष कीवर्ड को आमतौर पर स्टॉप शब्दों के रूप में उपयोग किया जा सकता है। विवरण के लिए [--buildstops](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) और [--buildfreqs](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-command-line-arguments) स्विच देखें। उस डिक्शनरी से शीर्ष कीवर्ड को आमतौर पर स्टॉप शब्दों के रूप में उपयोग किया जा सकता है।


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) stopwords = '/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) stopwords = '/usr/local/manticore/data/stopwords.txt stopwords-ru.txt stopwords-en.txt'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'stopwords' => '/usr/local/manticore/data/stopwords.txt stopwords-ru.txt stopwords-en.txt'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = '/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) stopwords = '/usr/local/manticore/data/stopwords.txt /usr/local/manticore/data/stopwords-ru.txt /usr/local/manticore/data/stopwords-en.txt'");
```

<!-- request CONFIG -->

```ini
table products {
  stopwords = /usr/local/manticore/data/stopwords.txt
  stopwords = stopwords-ru.txt stopwords-en.txt

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

वैकल्पिक रूप से आप Manticore के साथ आने वाली एक डिफ़ॉल्ट स्टॉप शब्द फ़ाइलों में से एक का उपयोग कर सकते हैं। वर्तमान में 50 भाषाओं के लिए स्टॉप शब्द उपलब्ध हैं। उनके लिए संक्षिप्त विवरण यहां है:

* af - अफ़्रीकी
* ar - अरबी
* bg - बुल्गारियाई
* bn - बांग्ला
* ca - कैटलन
* ckb- कुर्दिश
* cz - चेक
* da - डैनीश
* de - जर्मन
* el - ग्रीक
* en - अंग्रेजी
* eo - एस्पेरेंटो
* es - स्पेनिश
* et - एस्टोनियाई
* eu - बास्क
* fa - फारसी
* fi - फिनिश
* fr - फ्रेंच
* ga - आयरिश
* gl - गैलिशियन
* hi - हिंदी
* he - हिब्रू
* hr - क्रोएशियाई
* hu - हंगेरियन
* hy - आर्मेनियन
* id - इंडोनेशियाई
* it - इतालवी
* ja - जापानी
* ko - कोरियाई
* la - लैटिन
* lt - लिथुआनियाई
* lv - लातवियाई
* mr - मराठी
* nl - डच
* no - नॉर्वेजियन
* pl - पोलिश
* pt - पुर्तगाली
* ro - रोमानियाई
* ru - रूसी
* sk - स्लोवाक
* sl - स्लोवेनियन
* so - सोमाली
* st - सोथो
* sv - स्वीडिश
* sw - स्वाहिली
* th - थाई
* tr - तुर्की
* yo - योरूबा
* zh - चीनी
* zu - ज़ुलू

<!-- example stopwords 1 -->
उदाहरण के लिए, इटालियन भाषा के लिए स्टॉप वर्ड्स का उपयोग करने के लिए बस अपने कॉन्फ़िग फ़ाइल में निम्नलिखित पंक्ति डालें:


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) stopwords = 'it'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) stopwords = 'it'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'stopwords' => 'it'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'it\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'it\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = 'it'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) stopwords = 'it'");
```

<!-- request CONFIG -->

```ini
table products {
  stopwords = it

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

<!-- example stopwords 2 -->
यदि आपको कई भाषाओं के लिए स्टॉप वर्ड्स का उपयोग करना है, तो आपको उनके सभी उपनामों को अल्पविराम (RT मोड) या स्पेस (साधारण मोड) से अलग करके सूचीबद्ध करना चाहिए:


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) stopwords = 'en, it, ru'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) stopwords = 'en, it, ru'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'stopwords' => 'en, it, ru'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en, it, ru\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en, it, ru\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = 'en, it, ru'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = 'en, it, ru'");
```

<!-- request CONFIG -->

```ini
table products {
  stopwords = en it ru

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## stopword_step

```ini
stopword_step={0|1}
```

<!-- example stopword_step -->
[स्टॉप वर्ड्स](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords) पर position_increment सेटिंग वैकल्पिक है, और अनुमत मान 0 और 1 हैं, जिनका डिफ़ॉल्ट मान 1 है।


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) stopwords = 'en' stopword_step = '1'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) stopwords = 'en' stopword_step = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'stopwords' => 'en, it, ru',
            'stopword_step' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopword_step = \'1\'");
```

<!-- request CONFIG -->

```ini
table products {
  stopwords = en
  stopword_step = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->

## stopwords_unstemmed

```ini
stopwords_unstemmed={0|1}
```

<!-- example stopwords_unstemmed -->
यह निर्णय लेना कि स्टॉप वर्ड्स को स्टेमिंग से पहले या बाद में लागू किया जाए। वैकल्पिक, डिफ़ॉल्ट 0 है (स्टेमिंग के बाद स्टॉप वर्ड फ़िल्टर लागू करना)।

डिफ़ॉल्ट रूप से, स्टॉप वर्ड्स खुद स्टेम किए जाते हैं, और फिर परिवर्तनों के बाद *स्टेमिंग* (या किसी अन्य रूपिकी प्रसंस्करण) पर लागू होते हैं। इसका मतलब है कि एक टोकन को रोका जाता है जब stem(token) stem(stopword) के समान होता है। यह डिफ़ॉल्ट व्यवहार तब अप्रत्याशित परिणामों की ओर ले जा सकता है जब एक टोकन को गलती से रोके गए मूल पर स्टेम किया गया हो। उदाहरण के लिए, "Andes" को "and" पर स्टेम किया जा सकता है, इसलिए जब "and" एक स्टॉप वर्ड है, "Andes" भी छोड़ा गया है।
हालांकि, आप `stopwords_unstemmed` निर्देशिका को सक्रिय करके इस व्यवहार को बदल सकते हैं। जब यह सक्रिय होता है, तो स्टॉप शब्दों को स्टेमिंग से पहले लागू किया जाता है (और इसलिए मूल शब्द रूपों पर), और तब टोकन को छोड़ दिया जाता है जब टोकन स्टॉपवर्ड के बराबर होता है।

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) stopwords = 'en' stopwords_unstemmed = '1'
```

<!-- request JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) stopwords = 'en' stopwords_unstemmed = '1'"
```

<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'stopwords' => 'en, it, ru',
            'stopwords_unstemmed' => '1'
        ]);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'')
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) stopwords = \'en\' stopwords_unstemmed = \'1\'");
```

<!-- request CONFIG -->

```ini
table products {
  stopwords = en
  stopwords_unstemmed = 1

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->
<!-- proofread -->

