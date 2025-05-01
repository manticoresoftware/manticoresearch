# अपवाद

अपवाद (जिन्हें पर्यायवाची भी कहा जाता है) एक या एक से अधिक टोकन (उन टोकनों सहित जिनमें ऐसे अक्षर होते हैं जो सामान्यतः बाहर किए जाते हैं) को एकल कुंजी शब्द में मैप करने की अनुमति देते हैं। वे [शब्दरूपों](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) के समान होते हैं कि वे भी मैपिंग करते हैं लेकिन उनमे कई महत्वपूर्ण अंतर होते हैं।

[शब्दरूपों](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) से अंतर का संक्षिप्त सारांश इस प्रकार है:

| अपवाद | शब्द रूप |
| - | - |
| केस संवेदनशील | केस असंवेदनशील |
| [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) में नहीं होने वाले विशेष अक्षरों का उपयोग कर सकते हैं | [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) का पूरी तरह पालन किया जाता है |
| विशाल शब्दकोशों पर कमज़ोर प्रदर्शन करते हैं | लाखों प्रविष्टियों को संभालने के लिए डिज़ाइन किए गए |

## अपवाद

```ini
exceptions = path/to/exceptions.txt
```

<!-- उदाहरण अपवाद -->
अपवाद फ़ाइल को टोकन करना। वैकल्पिक, डिफ़ॉल्ट खाली है।
RT मोड में, केवल पूर्ण पथ की अनुमति है।

अपेक्षित फ़ाइल स्वरूप सामान्य पाठ है, जिसमें प्रत्येक अपवाद के लिए एक पंक्ति होती है। पंक्ति का स्वरूप इस प्रकार है:

```ini
map-from-tokens => map-to-token
```

उदाहरण फ़ाइल:

```ini
at & t => at&t
AT&T => AT&T
Standarten   Fuehrer => standartenfuhrer
Standarten Fuhrer => standartenfuhrer
MS Windows => ms windows
Microsoft Windows => ms windows
C++ => cplusplus
c++ => cplusplus
C plus plus => cplusplus
=>abc> => abc
```

यहाँ सभी टोकन केस संवेदनशील हैं और [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) नियमों द्वारा **प्रक्रिया में नहीं** लिए जाएंगे। इस प्रकार, उपरोक्त उदाहरण अपवाद फ़ाइल के साथ, `at&t` पाठ को छोटे अक्षरों के कारण `at` और `t` के रूप में दो कुंजी शब्दों के रूप में टोकन किया जाएगा। दूसरी ओर, `AT&T` पूरी तरह से मेल खाएगा और एकल `AT&T` कुंजी शब्द का उत्पादन करेगा।

यदि आप `>` या `=` को सामान्य अक्षरों के रूप में उपयोग करना चाहते हैं, तो आप प्रत्येक के आगे एक बैकस्लैश (``) लगाकर उसे एस्केप कर सकते हैं। दोनों `>` और `=` को इस तरीके से एस्केप किया जाना चाहिए।

ध्यान दें कि मैप-टू कुंजी शब्द:
* हमेशा *एकल* शब्द के रूप में व्याख्या किए जाते हैं
* केस और स्थान संवेदनशील होते हैं

उपरोक्त नमूने में, `ms windows` क्वेरी `MS Windows` पाठ के साथ *मेल* नहीं खाएगी। क्वेरी को `ms` और `windows` के लिए दो कुंजी शब्दों के लिए क्वेरी के रूप में व्याख्यायित किया जाएगा। `MS Windows` के लिए मैपिंग एकल कुंजी शब्द `ms windows` है, जिसके मध्य में एक स्थान है। दूसरी ओर, `standartenfuhrer` दस्तावेज़ों को `Standarten Fuhrer` या `Standarten Fuehrer` सामग्री (सटीक रूप से इस तरह बड़े अक्षरों में) से पुनः प्राप्त करेगा, या कुंजी शब्द के स्वयं के किसी भी बड़े अक्षर रूपांतर, जैसे कि `staNdarTenfUhreR`। (हालांकि यह `standarten fuhrer` को पकड़ नहीं पाएगा: यह पाठ किसी भी सूचीबद्ध अपवाद का मेल नहीं खाता क्योंकि यह केस संवेदनशीलता के कारण है और दो अलग-अलग कुंजी शब्दों के रूप में अनुक्रमित किया गया है।)

मैप-फ्रॉम टोकन सूचि में अंतराल महत्वपूर्ण होता है, परंतु इसकी मात्रा नहीं। मैप-फॉर्म सूची में किसी भी मात्रा का अंतराल अनुक्रमित दस्तावेज़ या क्वेरी में किसी भी अन्य मात्रा के अंतराल से मेल खाएगा। उदाहरण के लिए, `AT & T` मैप-फ्रॉम टोकन `AT & T` पाठ के साथ मेल खाएगा, चाहे दोनों मैप-फ्रॉम भाग और अनुक्रमित पाठ में कितनी भी जगह हो। इस प्रकार का पाठ, इसलिए, पहले प्रविष्टि के लिए विशेष `AT&T` कुंजी शब्द के रूप में अनुक्रमित किया जाएगा, नमूने के लिए धन्यवाद।

अपवाद विशेष अक्षरों को भी कैप्चर करने की अनुमति देते हैं (जो सामान्य `charset_table` नियमों से अपवाद हैं; इसलिए नाम)। मान लीजिए कि आप सामान्यतः `+` को एक मान्य अक्षर के रूप में नहीं मानते, पर फिर भी इस नियम के कुछ अपवादों जैसे `C++` को खोजने की क्षमता रखना चाहते हैं। उपरोक्त उदाहरण बिल्कुल वही करेगा, यह तालिका में क्या अक्षर हैं और क्या नहीं है पर पूरी तरह से स्वतंत्र है।

[सादा तालिका](../../Creating_a_table/Local_tables/Plain_table.md) का उपयोग करते समय, अपवाद फ़ाइल से परिवर्तन को शामिल करने के लिए तालिका को घुमाना आवश्यक है। वास्तविक समय की तालिका के मामले में, परिवर्तन केवल नए दस्तावेज़ों पर लागू होंगे।

<!-- अनुरोध SQL -->

```sql
CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'
```

<!-- अनुरोध JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'"
```

<!-- अनुरोध PHP -->

```php
$index = new ManticoresearchIndex($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'exceptions' => '/usr/local/manticore/data/exceptions.txt'
        ]);
```
<!-- परिचय -->
##### पाइथन:

<!-- अनुरोध पाइथन -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'')
```

<!-- परिचय -->
##### पाइथन-असिंक्रोनस:

<!-- अनुरोध पाइथन-असिंक्रोनस -->

```python
await utilsApi.sql('CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'')
```

<!-- परिचय -->
##### जावास्क्रिप्ट:

<!-- अनुरोध जावास्क्रिप्ट -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'');
```

<!-- परिचय -->
##### जावा:
<!-- अनुरोध जावा -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'", true);
```

<!-- परिचय -->
##### C#:
<!-- अनुरोध C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'", true);
```

<!-- परिचय -->
#####Rust:

<!-- अनुरोध Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'", Some(true)).await;
```
<!-- request CONFIG -->

```ini
table products {
  exceptions = /usr/local/manticore/data/exceptions.txt

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- end -->
<!-- proofread -->

# अपवाद

अपवाद (जिसे पर्यायवाची भी कहा जाता है) एक या अधिक टोकन (जिसमें वे टोकन शामिल हैं जिनमें सामान्यतः बाहर किये गए वर्ण होते हैं) को एक एकल कीवर्ड से मैप करने की अनुमति देते हैं। वे [शब्द रूपों](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) के समान होते हैं क्योंकि वे भी मैपिंग करते हैं लेकिन इसमें कुछ महत्वपूर्ण भिन्नताएँ होती हैं।

[शब्द रूपों](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms) से भिन्नताओं का एक संक्षिप्त सारांश इस प्रकार है:

| अपवाद | शब्द रूप |
| - | - |
| केस संवेदनशील | केस अप्रवणशील |
| विशेष वर्णों का उपयोग कर सकते हैं जो [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) में नहीं हैं | पूरी तरह से [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) का पालन करते हैं |
| विशाल शब्दकोशों पर कम प्रदर्शन करते हैं | लाखों प्रविष्टियों को संभालने के लिए डिज़ाइन किया गया |

## अपवाद

```ini
अपवाद = path/to/exceptions.txt
```

<!-- उदाहरण अपवाद -->
टोकनाइजिंग अपवाद फ़ाइल। वैकल्पिक, डिफ़ॉल्ट खाली है।
RT मोड में, केवल पूर्ण पथ की अनुमति है।

अपेक्षित फ़ाइल फ़ॉर्मेट सामान्य पाठ है, जिसमें प्रत्‍येक अपवाद के लिए एक पंक्ति है। पंक्ति का फ़ॉर्मेट इस प्रकार है:

```ini
map-from-tokens => map-to-token
```

उदाहरण फ़ाइल:

```ini
at & t => at&t
AT&T => AT&T
Standarten   Fuehrer => standartenfuhrer
Standarten Fuhrer => standartenfuhrer
MS Windows => ms windows
Microsoft Windows => ms windows
C++ => cplusplus
c++ => cplusplus
C plus plus => cplusplus
\=\>abc\> => abc
```

यहाँ सभी टोकन केस संवेदनशील हैं और [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) नियमों द्वारा **नहीं** संसाधित किए जाएंगे। इसलिए, उपरोक्त उदाहरण अपवाद फ़ाइल के साथ, `at&t` पाठ को दो कीवर्ड `at` और `t` के रूप में टोकनाइज किया जाएगा क्योंकि छोटे अक्षर हैं। दूसरी ओर, `AT&T` सटीक रूप से मेल खाएगा और एकल `AT&T` कीवर्ड उत्पन्न करेगा।

यदि आपको `>` या `=` को सामान्य वर्णों के रूप में उपयोग करना है, तो आप प्रत्येक को एक बैकस्लेश (`\`) के साथ आगे रखकर बचा सकते हैं। दोनों `>` और `=` को इस तरीके से बचाना चाहिए।

ध्यान दें कि मैप-टू कीवर्ड:
* हमेशा एक *एकल* शब्द के रूप में व्याख्यायित होते हैं
* दोनों केस और स्पेस संवेदनशील होते हैं

उपरोक्त उदाहरण में, `ms windows` क्वेरी `MS Windows` पाठ के साथ *नहीं* मेल खाएगी। क्वेरी को दो कीवर्ड `ms` और `windows` के लिए क्वेरी के रूप में व्याख्यायित किया जाएगा। `MS Windows` का मैपिंग एक एकल कीवर्ड `ms windows` है, जिसमें बीच में एक स्पेस है। दूसरी ओर, `standartenfuhrer` ऐसे दस्तावेज़ प्राप्त करेगा जिनमें `Standarten Fuhrer` या `Standarten Fuehrer` सामग्री (जैसे इस हिज्जे में सटीक रूप से बड़े अक्षरों में) हैं, या कीवर्ड के स्वयं के किसी भी बड़े अक्षर के संस्करण, जैसे कि `staNdarTenfUhreR`। (हालांकि `standarten fuhrer` को नहीं पकड़ेगा: यह पाठ सूचीबद्ध अपवादों में से किसी से मेल नहीं खाता है क्योंकि यह केस संवेदनशीलता के कारण है और इसे दो अलग-अलग कीवर्ड के रूप में अनुक्रमित किया गया है।)

मैप-फ्रम टोकन सूची मेंWhitespace महत्वपूर्ण है, लेकिन इसकी मात्रा महत्वपूर्ण नहीं है। मैप-फॉर्म सूची में किसी भी मात्रा केWhitespace अनुक्रमित दस्तावेज़ या क्वेरी में किसी अन्य राशि केWhitespace से मेल खाएगी। उदाहरण के लिए, `AT & T` मैप-फ्रम टोकन `AT & T` पाठ के साथ मेल खाएगा, चाहे मैप-फ्रम भाग और अनुक्रमित पाठ मेंWhitespace की मात्रा कितनी भी हो। इसलिए, ऐसा पाठ विशेष `AT&T` कीवर्ड के रूप में अनुक्रमित किया जाएगा, पहले प्रविष्टि के लिए धन्यवाद।

अपवाद विशेष वर्णों को कैप्चर करने की भी अनुमति देते हैं (जो सामान्य `charset_table` नियमों से अपवाद हैं; इसलिए नाम)। मान लीजिए कि आप सामान्यतः `+` को एक मान्य वर्ण के रूप में व्यवहार नहीं करना चाहते, लेकिन फिर भी इस नियम के कुछ अपवाद जैसे `C++` के लिए खोज करने में सक्षम होना चाहते हैं। उपरोक्त उदाहरण ऐसा ही करेगा, पूरी तरह से इस बात से स्वतंत्र कि टेबल में कौन से वर्ण हैं और कौन से नहीं।

[सादा तालिका](../../Creating_a_table/Local_tables/Plain_table.md) का उपयोग करते समय, यह आवश्यक है कि अपवाद फ़ाइल से परिवर्तनों को सम्मिलित करने के लिए तालिका को घुमाया जाए। वास्तविक समय की तालिका के मामले में, परिवर्तन केवल नए दस्तावेज़ों पर लागू होंगे।

<!-- अनुरोध SQL -->

```sql
CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'
```

<!-- अनुरोध JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'"
```

<!-- अनुरोध PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'exceptions' => '/usr/local/manticore/data/exceptions.txt'
        ]);
```
<!-- परिचय -->
##### पायथन:

<!-- अनुरोध पायथन -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) exceptions = \'/usr/local/manticore/data/exceptions.txt\'')
```
<!-- परिचय -->
##### जावास्क्रिप्ट:

<!-- अनुरोध जावास्क्रिप्ट -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float) exceptions = \'/usr/local/manticore/data/exceptions.txt\'');
```

<!-- परिचय -->
##### जावा:
<!-- अनुरोध जावा -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'");
```

<!-- परिचय -->
##### C#:
<!-- अनुरोध C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) exceptions = '/usr/local/manticore/data/exceptions.txt'");
```

<!-- अनुरोध CONFIG -->

```ini
table products {
  exceptions = /usr/local/manticore/data/exceptions.txt

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- अंत -->
<!-- प्रूफरीड -->
