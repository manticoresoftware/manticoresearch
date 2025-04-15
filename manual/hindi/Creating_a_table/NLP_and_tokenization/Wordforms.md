# शब्द रूप

शब्द रूप आने वाले पाठ को [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) नियमों द्वारा टोकनाइज करने के बाद लागू होते हैं। वे मूल रूप से आपको एक शब्द को दूसरे शब्द से बदलने की अनुमति देते हैं। सामान्यतः, इसका उपयोग विभिन्न शब्द रूपों को एक सामान्य रूप (जैसे "walks", "walked", "walking" को सामान्य रूप "walk" में सामान्य बनाने) में लाने के लिए किया जाता है। इसका उपयोग [stemming](../../Creating_a_table/NLP_and_tokenization/Morphology.md) अपवाद लागू करने के लिए भी किया जा सकता है, क्योंकि शब्द रूप सूची में पाए गए शब्दों पर स्टेमिंग लागू नहीं होती है।

## शब्द रूप

```ini
wordforms = path/to/wordforms.txt
wordforms = path/to/alternateforms.txt
wordforms = path/to/dict*.txt
```

<!-- उदाहरण शब्द रूप -->
शब्द रूप शब्दकोष। वैकल्पिक, डिफ़ॉल्ट खाली है।

शब्द रूप शब्दकोष का उपयोग आने वाले शब्दों को सामान्य बनाने के लिए किया जाता है, दोनों अनुक्रमण करते समय और खोज करते समय। इसलिए, जब [सादा तालिका](../../Creating_a_table/Local_tables/Plain_table.md) की बात आती है, तो शब्द रूप फ़ाइल में परिवर्तनों को उठाने के लिए तालिका को घुमाना आवश्यक है।

<!-- परिचय -->
##### SQL:

<!-- अनुरोध SQL -->

```sql
CREATE TABLE products(title text, price float) wordforms = '/var/lib/manticore/wordforms.txt' wordforms = '/var/lib/manticore/alternateforms.txt /var/lib/manticore/dict*.txt'
```

<!-- अनुरोध JSON -->

```json
POST /cli -d "
CREATE TABLE products(title text, price float) wordforms = '/var/lib/manticore/wordforms.txt' wordforms = '/var/lib/manticore/alternateforms.txt' wordforms = '/var/lib/manticore/dict*.txt'"
```

<!-- अनुरोध PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
            'title'=>['type'=>'text'],
            'price'=>['type'=>'float']
        ],[
            'wordforms' => [
                '/var/lib/manticore/wordforms.txt',
                '/var/lib/manticore/alternateforms.txt',
                '/var/lib/manticore/dict*.txt'
            ]
        ]);
```
<!-- परिचय -->
##### Python:

<!-- अनुरोध Python -->

```python
utilsApi.sql('CREATE TABLE products(title text, price float) wordforms = \'/var/lib/manticore/wordforms.txt\' wordforms = \'/var/lib/manticore/alternateforms.txt\' wordforms = \'/var/lib/manticore/dict*.txt\'')
```
<!-- परिचय -->
##### Javascript:

<!-- अनुरोध javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, price float)wordforms = \'/var/lib/manticore/wordforms.txt\' wordforms = \'/var/lib/manticore/alternateforms.txt\' wordforms = \'/var/lib/manticore/dict*.txt\'');
```

<!-- परिचय -->
##### Java:
<!-- अनुरोध Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, price float) wordforms = '/var/lib/manticore/wordforms.txt' wordforms = '/var/lib/manticore/alternateforms.txt' wordforms = '/var/lib/manticore/dict*.txt'");
```

<!-- परिचय -->
##### C#:
<!-- अनुरोध C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, price float) wordforms = '/var/lib/manticore/wordforms.txt' wordforms = '/var/lib/manticore/alternateforms.txt' wordforms = '/var/lib/manticore/dict*.txt'");
```

<!-- परिचय -->
##### सादा मोड उदाहरण:

<!-- अनुरोध CONFIG -->

```ini
table products {
  wordforms = /var/lib/manticore/wordforms.txt
  wordforms = /var/lib/manticore/alternateforms.txt
  wordforms = /var/lib/manticore/dict*.txt

  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
}
```
<!-- समाप्त -->

Manticore में शब्द रूप समर्थन बड़े शब्दकोषों को अच्छी तरह से संभालने के लिए डिजाइन किया गया है। ये अनुक्रमण गति पर मध्यम प्रभाव डालते हैं; उदाहरण के लिए, 1 मिलियन प्रविष्टियों वाला शब्दकोष पूर्ण पाठ अनुक्रमण को लगभग 1.5 गुना धीमा कर देता है। खोजने की गति पर कोई प्रभाव नहीं पड़ता है। अतिरिक्त RAM प्रभाव लगभग शब्दकोष फ़ाइल के आकार के बराबर है, और शब्दकोष तालिकाओं के बीच साझा किए जाते हैं। उदाहरण के लिए, यदि 10 विभिन्न तालिकाओं के लिए बिल्कुल उसी 50 एमबी शब्द रूप फ़ाइल को निर्दिष्ट किया गया है, तो अतिरिक्त `searchd` RAM उपयोग लगभग 50 एमबी होगा।

<!-- उदाहरण wf_simple -->
शब्दकोष फ़ाइल को एक सरल सादा पाठ प्रारूप में होना चाहिए। प्रत्येक पंक्ति में स्रोत और गंतव्य शब्द रूपों को UTF-8 एन्कोडिंग में शामिल होना चाहिए, जो 'बड़े से बड़ा' संकेत के द्वारा अलग किए जाते हैं। फ़ाइल लोड करते समय [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table) के नियम लागू होंगे। इसलिए, यदि आप `charset_table` को संशोधित नहीं करते हैं, तो आपके शब्द रूप केस-संवेदनशील नहीं होंगे, जैसे कि आपके अन्य पूर्ण-पाठ अनुक्रमण किए गए डेटा। नीचे फ़ाइल सामग्री का एक नमूना है:

<!-- अनुरोध उदाहरण -->
```ini
walks > walk
walked > walk
walking > walk
```
<!-- समाप्त -->

एक बंडल किया हुआ उपयोगिता [Spelldump](../../Miscellaneous_tools.md#spelldump) है, जो आपको Manticore पढ़ सकता है ऐसा शब्दकोष फ़ाइल बनाने में मदद करता है। यह उपयोगिता 'ispell' या 'MySpell' प्रारूप में स्रोत `.dict` और `.aff` शब्दकोष फ़ाइलों से पढ़ सकती है, जैसे कि OpenOffice के साथ बंडल किया गया है।

आप कई स्रोत शब्दों को एकल गंतव्य शब्द में मैप कर सकते हैं। यह प्रक्रिया टोकन पर होती है, स्रोत पाठ पर नहीं, इसलिए सफेद जगह और मार्कअप में भिन्नताएँ अनदेखी की जाती हैं।

<!-- उदाहरण wf_more_complex -->
आप `>` के बजाय `=>` प्रतीक का उपयोग कर सकते हैं। टिप्पणियाँ (जो `#` से शुरू होती हैं) भी अनुमन्य हैं। अंततः, यदि कोई पंक्ति एक तिल्ड (`~`) से शुरू होती है, तो शब्द रूपMorphology के बाद लागू किया जाएगा, पहले नहीं (ध्यान दें कि इस मामले में केवल एकल स्रोत और गंतव्य शब्द समर्थित हैं)।

<!-- अनुरोध उदाहरण -->
```ini
core 2 duo > c2d
e6600 > c2d
core 2duo => c2d # कुछ लोग '2duo' को एक साथ लिखते हैं...
~run > walk # morphology को सक्षम करने के साथ 'run', 'running', 'runs' (और किसी अन्य शब्द को जो केवल 'run' तक पहुंचता है) को 'walk' में बदलता है
```
<!-- समाप्त -->

<!-- उदाहरण wf_escaping -->
यदि आपको `>`, `=` या `~` को सामान्य वर्णों के रूप में उपयोग करने की आवश्यकता है, तो आप प्रत्येक को बैकस्लैश (`\`) के साथ पहले लिखकर बचा सकते हैं। इस प्रकार `>` और `=` दोनों को बचाना चाहिए। यहां एक उदाहरण है:

<!-- request Example -->
```ini
a\> > abc
\>b > bcd
c\=\> => cde
\=\>d => def
\=\>a \> f \> => foo
\~g => bar
```
<!-- end -->

<!-- example wf_multiple_tokens -->
आप कई गंतव्य टोकन निर्दिष्ट कर सकते हैं:

<!-- request Example -->
```ini
s02e02 > season 2 episode 2
s3 e3 > season 3 episode 3
```
<!-- end -->

<!-- example wf_multiple_files -->
आप एक से अधिक फ़ाइलें निर्दिष्ट कर सकते हैं, केवल एक नहीं। मास्क पैटर्न के रूप में उपयोग किया जा सकता है, और सभी मेल खाते फ़ाइलों को सरल वृद्धि क्रम में संसाधित किया जाएगा:

RT मोड में, केवल पूर्ण पथ की अनुमति है।

यदि मल्टी-байट कोडपेज का उपयोग किया जा रहा है और फ़ाइल नाम गैर-लैटिन वर्णों शामिल करते हैं, तो परिणामस्वरूप क्रम सटीक वर्णानुक्रम में नहीं हो सकता है। यदि एक ही शब्दरूप परिभाषा कई फ़ाइलों में पाई जाती है, तो बाद वाली का उपयोग किया जाता है और पूर्व की परिभाषाओं को अधिलेखित कर दिया जाता है।

<!-- request SQL -->
```sql
create table tbl1 ... wordforms='/tmp/wf*'
create table tbl2 ... wordforms='/tmp/wf, /tmp/wf2'
```

<!-- request Config -->
```ini
wordforms=/tmp/wf
wordforms=/tmp/wf2
wordforms=/tmp/wf_new*
```

<!-- end -->


<!-- proofread -->
