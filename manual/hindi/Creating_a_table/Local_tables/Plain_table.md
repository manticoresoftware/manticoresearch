# साधारण तालिका

**साधारण तालिका** गैर-[परक्यूलेट](../../Creating_a_table/Local_tables/Percolate_table.md) खोज के लिए एक मूल तत्व है। इसे केवल [साधारण मोड](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29) का उपयोग करते हुए एक कॉन्फ़िगरेशन फ़ाइल में परिभाषित किया जा सकता है, और इसे [RT मोड](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29) में समर्थन नहीं किया जाता है। इसे आमतौर पर किसी [स्रोत](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#source) के साथ मिलकर डेटा को [बाहरी संग्रहण से](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md) संसाधित करने के लिए उपयोग किया जाता है और इसे बाद में **वास्तविक-समय की तालिका** से [जुडने](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md) के लिए उपयोग किया जा सकता है।

#### साधारण तालिका का निर्माण

एक साधारण तालिका बनाने के लिए, आपको इसे एक कॉन्फ़िगरेशन फ़ाइल में परिभाषित करना होगा। इसे `CREATE TABLE` कमांड द्वारा समर्थन नहीं किया जाता है।

<!-- example plain -->

यहाँ एक साधारण तालिका कॉन्फ़िगरेशन और एक स्रोत का उदाहरण है जो MySQL डेटाबेस से डेटा लाने के लिए उपयोग किया जाता है:

<!-- intro -->
#### साधारण तालिका कैसे बनाएं

<!-- request Plain table example -->

```ini
source source {
  type             = mysql
  sql_host         = localhost
  sql_user         = myuser
  sql_pass         = mypass
  sql_db           = mydb
  sql_query        = SELECT id, title, description, category_id  from mytable
  sql_attr_uint    = category_id
  sql_field_string = title
 }

table tbl {
  type   = plain
  source = source
  path   = /path/to/table
 }
```
<!-- end -->

### 👍 आप साधारण तालिका के साथ क्या कर सकते हैं:
  * इसे [स्रोत](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#source) और [सूचक](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) का उपयोग करके बाहरी संग्रहण से बनाएँ
  * [पूर्णांक, फ़्लोट, स्ट्रिंग और MVA विशेषता](../../Creating_a_table/Data_types.md) का चालू अद्यतन करें
  * इसके `killlist_target` को [अद्यतन](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#killlist_target) करें

### ⛔ आप साधारण तालिका के साथ क्या नहीं कर सकते:
  * तालिका के निर्माण के बाद उसमें अतिरिक्त डेटा सम्मिलित करें
  * तालिका से डेटा हटाएं
  * तालिका स्कीमा को ऑनलाइन बनाएं, हटाएं या संशोधित करें
  * स्वचालित ID जनरेशन के लिए [UUID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) का उपयोग करें (बाहरी संग्रहण से डेटा में एक अद्वितीय पहचानकर्ता शामिल होना चाहिए)

संख्या विशेषताएँ, जिसमें [MVAs](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) शामिल हैं, वह केवल तत्व हैं जिन्हें साधारण तालिका में अपडेट किया जा सकता है। तालिका में अन्य सभी डेटा अचल हैं। यदि अपडेट या नए रिकॉर्ड की आवश्यकता है, तो तालिका को फिर से बनाना होगा। पुनर्निर्माण प्रक्रिया के दौरान, मौजूदा तालिका अनुरोधों को सेवा देने के लिए उपलब्ध रहती है, और जब नया संस्करण तैयार होता है, तो [रोटेशन](../../Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md) नामक प्रक्रिया की जाती है, जो इसे ऑनलाइन लाती है और पुराने संस्करण को अस्वीकार करती है।

#### साधारण तालिका निर्माण प्रदर्शन
साधारण तालिका को अनुक्रमित करने की गति कई कारकों पर निर्भर करती है, जिसमें शामिल हैं:
* डेटा स्रोत पुनर्प्राप्ति की गति
* [टोकनाइजेशन सेटिंग्स](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md)
* हार्डवेयर विनिर्देश (जैसे CPU, RAM, और डिस्क प्रदर्शन)

#### साधारण तालिका निर्माण परिदृश्य
##### आवश्यक होने पर पूरी तरह से पुनर्निर्माण
छोटे डेटा सेट के लिए, सबसे सरल विकल्प यह है कि एक ही साधारण तालिका हो जिसे आवश्यकतानुसार पूरी तरह से पुनर्निर्मित किया जाता है। यह दृष्टिकोण तब स्वीकार्य होता है जब:
* तालिका में डेटा स्रोत में डेटा की तुलना में ताजा नहीं है
* जैसे-जैसे डेटा सेट बढ़ता है, तालिका बनाने में लगने वाला समय बढ़ता है

##### मुख्य+डेल्टा परिदृश्य
बड़े डेटा सेट के लिए, एक साधारण तालिका का उपयोग [वास्तविक-समय](../../Creating_a_table/Local_tables/Real-time_table.md) के बजाय किया जा सकता है। मुख्य+डेल्टा परिदृश्य में शामिल हैं:
* वृद्धिशील अनुक्रमण के लिए एक छोटा तालिका बनाना
* एक [वितरित तालिका](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) का उपयोग करके दोनों तालिकाओं को संयोजित करना

यह दृष्टिकोण बड़ी तालिका के पुनर्निर्माण को कम किया गया और स्रोत से अद्य更新ों की अधिक बार प्रसंस्करण की अनुमति देता है। छोटे तालिका को अधिक बार (जैसे हर मिनट या यहां तक कि हर कुछ सेकंड) पुनर्निर्मित किया जा सकता है।

हालाँकि, समय के साथ, छोटे तालिका के लिए अनुक्रमण की अवधि बहुत लंबी हो जाएगी, जिससे बड़ी तालिका का पुनर्निर्माण और छोटे तालिका का खाली होना आवश्यक हो जाएगा।

मुख्य+डेल्टा स्कीमा को [इस इंटरैक्टिव पाठ्यक्रम](https://play.manticoresearch.com/maindelta/) में विस्तृत रूप से समझाया गया है।

**किल सूची** और [killlist_target](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target) निर्देश का तंत्र यह सुनिश्चित करने के लिए प्रयोग किया जाता है कि वर्तमान तालिका से दस्तावेज़ अन्य तालिका के दस्तावेज़ों पर प्राथमिकता लें।

इस विषय पर अधिक जानकारी के लिए, [यहाँ](../../Data_creation_and_modification/Adding_data_from_external_storages/Main_delta.md) देखें।

#### साधारण तालिका फ़ाइल संरचना
निम्नलिखित तालिका विभिन्न फ़ाइल एक्सटेंशन को दर्शाती है जो साधारण तालिका में उपयोग की जाती हैं और उनके संबंधित वर्णन:

| एक्सटेंशन | वर्णन |
| - | - |
|`.spa` | [पंक्ति-वार मोड](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) में दस्तावेज़ विशेषताओं को संग्रहीत करता है |
|`.spb` | [पंक्ति-वार मोड](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) में ब्लॉब विशेषताओं को संग्रहीत करता है: स्ट्रिंग, MVA, json |
|`.spc` | [कॉलम-वार मोड](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) में दस्तावेज़ विशेषताओं को संग्रहीत करता है  |
|`.spd` | प्रत्येक शब्द ID के लिए मिलान दस्तावेज़ ID सूचियाँ संग्रहीत करता है |
|`.sph` | तालिका हेडर जानकारी संग्रहीत करता है |
|`.sphi` | विशेषताओं के मानों के लिए हिस्टोग्राम संग्रहीत करता है |
|`.spi` | शब्द सूचियाँ (शब्द आईडी और `.spd` फ़ाइल के लिए पॉइंटर्स) संग्रहीत करता है |
|`.spidx` | द्वितीयक अनुक्रमांक डेटा संग्रहीत करता है |
|`.spjidx` | JSON विशेषताओं के लिए उत्पन्न द्वितीयक अनुक्रमांकों का डेटा संग्रहीत करता है |
|`.spk` | किल-लिस्ट संग्रहीत करता है |
|`.spl` | लॉक फ़ाइल |
|`.spm` | मारे गए दस्तावेज़ों का बिटमैप संग्रहीत करता है |
|`.spp` | प्रत्येक शब्द आईडी के लिए हिट (जिसे पोस्टिंग कहा जाता है, जिसे शब्द प्रकट होने के रूप में भी जाना जाता है) सूचियाँ संग्रहीत करता है |
|`.spt` | दस्तावेज़ आईडी द्वारा लुकअप को तेज़ करने के लिए अतिरिक्त डेटा संरचनाएँ संग्रहीत करता है |
|`.spe` | दस्तावेज़ सूची फ़िल्टरिंग को तेज़ करने के लिए स्किप-लिस्ट संग्रहीत करता है |
|`.spds` | दस्तावेज़ पाठ संग्रहीत करता है |
|`.tmp*` | index_settings_and_status के दौरान अस्थायी फ़ाइलें |
|`.new.sp*` | घुमाव से पहले एक सामान्य तालिका का नया संस्करण |
|`.old.sp*` | घुमाव के बाद एक सामान्य तालिका का पुराना संस्करण |

<!-- proofread -->
