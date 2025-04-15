# चेंज लॉग

# संस्करण 9.2.14
रिलीज़ हुआ: 28 मार्च 2025

### छोटे परिवर्तन
* [कमिट ](https://github.com/manticoresoftware/manticoresearch/commit/fe9473dc3f4fcfa0eaae748b538fe853f4ce078b) `--mockstack` ध्वज लागू किया गया है ताकि पुनरावर्ती संचालन स्टैक आवश्यकताओं की गणना की जा सके। नया `--mockstack` मोड पुनरावर्ती अभिव्यक्ति मूल्यों के विश्लेषण और रिपोर्टिंग के लिए आवश्यक स्टैक आकारों का विश्लेषण करता है, पैटर्न मिलान संचालन, फ़िल्टर प्रोसेसिंग। गणना की गई स्टैक आवश्यकताएँ डिबगिंग और ऑप्टिमाइजेशन उद्देश्यों के लिए कंसोल में आउटपुट की जाती हैं।
* [इश्यू #3058](https://github.com/manticoresoftware/manticoresearch/pull/3058) डिफ़ॉल्ट रूप से [boolean_simplify](../Searching/Options.md#boolean_simplify) सक्षम किया गया।
* [इश्यू #3172](https://github.com/manticoresoftware/manticoresearch/issues/3172) एक नया कॉन्फ़िग विकल्प जोड़ा गया: `searchd.kibana_version_string`, जो कि Manticore के साथ विशेष संस्करणों के Kibana या OpenSearch डैशबोर्ड का उपयोग करते समय उपयोगी हो सकता है जो एक विशेष Elasticsearch संस्करण की अपेक्षा करते हैं।
* [इश्यू #3211](https://github.com/manticoresoftware/manticoresearch/issues/3211) [CALL SUGGEST](../Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) को 2-चर वर्णों के साथ काम करने के लिए ठीक किया गया।
* [इश्यू #490](https://github.com/manticoresoftware/manticoresearch-buddy/issues/490) [फजी सर्च](../Searching/Spell_correction.md#Fuzzy-Search) में सुधार: पहले, यह कभी-कभी "defghi" का पता नहीं लगा पाती थी जब "def ghi" की खोज की जाती थी यदि कोई अन्य मिलान दस्तावेज़ मौजूद था।
* ⚠️ ब्रेकिंग [इश्यू #3165](https://github.com/manticoresoftware/manticoresearch/issues/3165) कुछ HTTP JSON प्रतिक्रियाओं में `_id` को `id` में बदल दिया गया है ताकि संगति बनी रहे। सुनिश्चित करें कि आप अपने ऐप को तदनुसार अपडेट करें।
* ⚠️ ब्रेकिंग [इश्यू #3186](https://github.com/manticoresoftware/manticoresearch/issues/3186) प्रत्येक नोड के पास एक अद्वितीय ID सुनिश्चित करने के लिए क्लस्टर में शामिल होने के दौरान `server_id` के लिए एक जांच जोड़ी गई। `JOIN CLUSTER` संचालन अब एक त्रुटि संदेश के साथ विफल हो सकता है जो एक डुप्लिकेट [server_id](../Server_settings/Searchd.md#server_id) को इंगित करता है जब शामिल होने वाला नोड क्लस्टर में एक मौजूदा नोड के समान `server_id` रखता है। इस मुद्दे को हल करने के लिए, सुनिश्चित करें कि प्रतिकृति क्लस्टर में प्रत्येक नोड के पास एक अद्वितीय [server_id](../Server_settings/Searchd.md#server_id) है। आप अपने कॉन्फ़िगरेशन फ़ाइल के "searchd" अनुभाग में डिफ़ॉल्ट [server_id](../Server_settings/Searchd.md#server_id) को एक अद्वितीय मान में बदल सकते हैं इससे पहले कि आप क्लस्टर में शामिल होने का प्रयास करें। यह परिवर्तन प्रतिकृति प्रोटोकॉल को अपडेट करता है। यदि आप एक प्रतिकृति क्लस्टर चला रहे हैं, तो आपको:
  - सबसे पहले, सभी नोड्स को साफ़-सुथरे तरीके से रोकें
  - फिर, `--new-cluster` के साथ अंतिम रूप से रोके गए नोड को शुरू करें, लिनक्स में `manticore_new_cluster` उपकरण का उपयोग करते हुए।
  - अधिक विवरण के लिए [क्लस्टर को पुनः प्रारंभ करने](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) के बारे में पढ़ें।

### बग फिक्सेस
* [कमिट 6fda](https://github.com/manticoresoftware/manticoresearch/commit/6fdad3923dd6953f4b781943eed5ec5c28b7f808) इंतज़ार के बाद शेड्यूलर खोने के कारण होने वाले क्रैश को ठीक किया गया; अब, विशेष शेड्यूलर जैसे `serializer` को सही तरीके से पुनर्स्थापित किया जाता है।
* [कमिट c333](https://github.com/manticoresoftware/manticoresearch/commit/c333b277d4a504de5fe298e9ab570a9f9ea31e0a) एक बग ठीक किया गया जहाँ दाएँ जुड़े तालिका से वजन का उपयोग `ORDER BY` क्लॉज़ में नहीं किया जा सकता था।
* [इश्यू #2644](https://github.com/manticoresoftware/manticoresearch/issues/2644) gcc 14.2.0: `const knn::DocDist_t*&` के साथ `lower_bound` कॉल त्रुटि ठीक की गई। ❤️ धन्यवाद [@Azq2](https://github.com/Azq2) PR के लिए।
* [इश्यू #3018](https://github.com/manticoresoftware/manticoresearch/issues/3018) स्वचालित स्कीमा Inserts के दौरान बड़े अक्षरों वाले तालिका नामों को संभालने में समस्या को ठीक किया गया।
* [इश्यू #3119](https://github.com/manticoresoftware/manticoresearch/issues/3119) अवैध बेस64 इनपुट को डिकोड करते समय होने वाले क्रैश को ठीक किया गया।
* [इश्यू #3121](https://github.com/manticoresoftware/manticoresearch/issues/3121) `ALTER` पर संबंधित KNN इंडेक्स मुद्दों को ठीक किया गया: फ़्लोट वेक्टर अब उनके मूल आयाम बनाए रखते हैं, और KNN इंडेक्स अब सही तरीके से उत्पन्न होते हैं।
* [इश्यू #3123](https://github.com/manticoresoftware/manticoresearch/issues/3123) एक खाली JSON कॉलम पर द्वितीयक इंडेक्स बनाने पर होने वाले क्रैश को ठीक किया गया।
* [इश्यू #3138](https://github.com/manticoresoftware/manticoresearch/issues/3138) डुप्लिकेट प्रविष्टियों के कारण होने वाले क्रैश को ठीक किया गया।
* [इश्यू #3151](https://github.com/manticoresoftware/manticoresearch/issues/3151) ठीक किया गया: `fuzzy=1` विकल्प `ranker` या `field_weights` के साथ नहीं उपयोग किया जा सका।
* [इश्यू #3163](https://github.com/manticoresoftware/manticoresearch/issues/3163) ठीक किया गया एक बग जहाँ `SET GLOBAL timezone` का कोई प्रभाव नहीं था।
* [इश्यू #3181](https://github.com/manticoresoftware/manticoresearch/issues/3181) एक समस्या को ठीक किया गया जहाँ टेक्स्ट फ़ील्ड मानों को 2^63 से बड़े IDs का उपयोग करते समय खोया जा सकता था।
* [इश्यू #3189](https://github.com/manticoresoftware/manticoresearch/issues/3189) ठीक किया गया: `UPDATE` वक्तव्य अब सही तरीके से `query_log_min_msec` सेटिंग का सम्मान करते हैं।
* [इश्यू #3247](https://github.com/manticoresoftware/manticoresearch/issues/3247) वास्तविक समय की डिस्क चंक्स को सहेजने के दौरान एक दौड़ की स्थिति को ठीक किया गया जिससे `JOIN CLUSTER` विफल हो सकता था।


# संस्करण 7.4.6
रिलीज़ हुआ: 28 फरवरी 2025

### मुख्य परिवर्तन
* [इश्यू #832](https://github.com/manticoresoftware/manticoresearch/issues/832) डेटा विज़ुअलाइजेशन को अधिक आसान और कुशल बनाने के लिए [Kibana](../Integration/Kibana.md) के साथ एकीकरण।

### छोटे परिवर्तन
* [इश्यू #1727](https://github.com/manticoresoftware/manticoresearch/issues/1727) arm64 और x86_64 के बीच फ्लोटिंग-पॉइंट सटीकता के फासले को ठीक किया गया।
* [इश्यू #2995](https://github.com/manticoresoftware/manticoresearch/issues/2995) जोड़ बैचिंग के लिए प्रदर्शन अनुकूलन लागू किया गया।
* [इश्यू #3039](https://github.com/manticoresoftware/manticoresearch/issues/3039) ऐतिहासिक ग्राफ में EstimateValues के लिए प्रदर्शन अनुकूलन लागू किया गया।
* [इश्यू #3099](https://github.com/manticoresoftware/manticoresearch/issues/3099) Boost 1.87.0 के लिए समर्थन जोड़ा गया। ❤️ धन्यवाद, [@cho-m](https://github.com/cho-m) PR के लिए।
* [Issue #77](https://github.com/manticoresoftware/columnar/issues/77) कई मानों के साथ फ़िल्टर बनाते समय ब्लॉक डेटा पुन: उपयोग का अनुकूलन किया; विशेषता मेटाडेटा में min/max जोड़ा; min/max के आधार पर फ़िल्टर मूल्यों का प्री-फ़िल्टरिंग लागू किया।

### बग फिक्स
* [Commit 73ac](https://github.com/manticoresoftware/manticoresearch/commit/73ac22f358a7a0e734b332c3943c86e6294c06c3) बाएँ और दाएँ तालिकाओं से विशेषताओं का उपयोग करते समय शामिल किए गए प्रश्नों में अभिव्यक्तियों के हैंडलिंग को ठीक किया; दाएँ तालिका के लिए index_weights विकल्प को ठीक किया।
* [Issue #2915](https://github.com/manticoresoftware/manticoresearch/issues/2915) एक `SELECT ... JOIN` प्रश्न में `avg()` का उपयोग करने से गलत परिणाम उत्पन्न हो सकता है; इसे अब ठीक किया गया है।
* [Issue #2996](https://github.com/manticoresoftware/manticoresearch/issues/2996) शामिल प्रश्न बैचिंग सक्रिय होने पर स्वचालित कटऑफ के कारण उत्पन्न गलत परिणाम सेट को ठीक किया गया।
* [Issue #3031](https://github.com/manticoresoftware/manticoresearch/issues/3031) सक्रिय चंक मर्ज चलने पर शटडाउन के दौरान डेमॉन में क्रैश को ठीक किया।
* [Issue #3037](http://github.com/manticoresoftware/manticoresearch/issues/3037) एक समस्या को ठीक किया जहाँ `IN(...)` गलत परिणाम उत्पन्न कर सकता था।
* [Issue #3038](https://github.com/manticoresoftware/manticoresearch/issues/3038) संस्करण 7.0.0 में `max_iops` / `max_iosize` सेट करना इंडेक्सिंग प्रदर्शन को खराब कर सकता है; इसे अब ठीक किया गया है।
* [Issue #3042](https://github.com/manticoresoftware/manticoresearch/issues/3042) जोड़ी गई प्रश्न कैश में मेमोरी लीक को ठीक किया गया।
* [Issue #3052](https://github.com/manticoresoftware/manticoresearch/issues/3052) शामिल JSON प्रश्नों में प्रश्न विकल्पों के हैंडलिंग को ठीक किया गया।
* [Issue #3054](https://github.com/manticoresoftware/manticoresearch/issues/3054) ATTACH TABLE कमांड के साथ एक समस्या को ठीक किया गया।
* [Issue #3079](https://github.com/manticoresoftware/manticoresearch/issues/3079) त्रुटि संदेशों में असंगतियों को ठीक किया गया।
* [Issue #3087](https://github.com/manticoresoftware/manticoresearch/issues/3087) प्रति तालिका `diskchunk_flush_write_timeout=-1` सेट करना इंडेक्स फ्लशिंग को बंद नहीं करता था; इसे अब ठीक किया गया है।
* [Issue #3088](https://github.com/manticoresoftware/manticoresearch/issues/3088) बड़े आईडी को सामूहिक रूप से बदलने के बाद डुप्लिकेट प्रविष्टियों को हल किया गया।
* [Issue #3126](https://github.com/manticoresoftware/manticoresearch/issues/3126) एक पूर्ण पाठ प्रश्न के कारण डेमॉन क्रैश को ठीक किया गया जिसमें एकल `NOT` ऑपरेटर और एक अभिव्यक्ति रैंकर था।
* [Issue #3128](https://github.com/manticoresoftware/manticoresearch/pull/3128) CJSON पुस्तकालय में संभावित भेद्यता को ठीक किया गया। ❤️ धन्यवाद, [@tabudz](https://github.com/tabudz) PR के लिए।

# संस्करण 7.0.0
जारी हुआ: 30 जनवरी 2025

### प्रमुख परिवर्तन
* [Issue #1497](https://github.com/manticoresoftware/manticoresearch/issues/1497) आसान खोज के लिए नया [Fuzzy Search](../Searching/Spell_correction.md#Fuzzy-Search) और [Autocomplete](../Searching/Autocomplete.md#CALL-AUTOCOMPLETE) कार्यक्षमता जोड़ी गई।
* [Issue #1500](https://github.com/manticoresoftware/manticoresearch/issues/1500) [Kafka के साथ एकीकरण](../Integration/Kafka.md#Syncing-from-Kafka)।
* [Issue #1928](https://github.com/manticoresoftware/manticoresearch/issues/1928) [JSON के लिए द्वितीयक अनुक्रमणकों](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#json_secondary_indexes) का परिचय दिया गया।
* [Issue #2361](https://github.com/manticoresoftware/manticoresearch/issues/2361) अद्यतन के दौरान अद्यतन और खोज अब चंक मर्जिंग द्वारा अवरुद्ध नहीं होते हैं।
* [Issue #2787](https://github.com/manticoresoftware/manticoresearch/issues/2787) प्रदर्शन में सुधार के लिए RT तालिका के लिए स्वचालित [डिस्क चंक फ्लश](../Server_settings/Searchd.md#diskchunk_flush_write_timeout) किया गया; अब, हम स्वचालित रूप से RAM चंक को डिस्क चंक में फ्लश करते हैं, जो RAM चंक में अनुकूलनों की कमी के कारण प्रदर्शन समस्याओं को रोकता है, जो कभी-कभी चंक आकार के आधार पर अस्थिरता का कारण बन सकता है।
* [Issue #2811](https://github.com/manticoresoftware/manticoresearch/issues/2811) आसान पेजिनेशन के लिए [Scroll](../Searching/Pagination.md#Scroll-Search-Option) विकल्प।
* [Issue #931](https://github.com/manticoresoftware/manticoresearch/issues/931) बेहतर [चीनी टोकनाइजेशन](../Creating_a_table/NLP_and_tokenization/Languages_with_continuous_scripts.md) के लिए [Jieba](https://github.com/fxsjy/jieba) के साथ एकीकरण।

### मामूली परिवर्तन
* ⚠️ BREAKING [Issue #1111](https://github.com/manticoresoftware/manticoresearch/issues/1111) RT तालिकाओं में `global_idf` के लिए समर्थन को ठीक किया गया। तालिका का पुनर्निर्माण आवश्यक है।
* ⚠️ BREAKING [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) आंतरिक `cjk` चरित्र सेट से थाई वर्णों को हटा दिया गया। अपने चरित्र सेट परिभाषाएँ तदनुसार अपडेट करें: यदि आपके पास `cjk,non_cjk` है और थाई वर्ण आपके लिए महत्वपूर्ण हैं, तो इसे `cjk,thai,non_cjk` या `cont,non_cjk` में बदलें, जहाँ `cont` सभी सतत-स्क्रिप्ट भाषाओं के लिए नया नामांकन है (यानी, `cjk` + `thai`)। मौजूदा तालिकाओं को [ALTER TABLE](../Updating_table_schema_and_settings.md#Updating-table-FT-settings-in-RT-mode) का उपयोग करके संशोधित करें।
* ⚠️ BREAKING [Issue #2468](https://github.com/manticoresoftware/manticoresearch/issues/2468) [CALL SUGGEST / QSUGGEST](../Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) अब वितरित तालिकाओं के साथ संगत है। यह मास्टर/एजेंट प्रोटोकॉल संस्करण को बढ़ाता है। यदि आप कई उदाहरणों के साथ वितरित वातावरण में Manticore Search चला रहे हैं, तो सुनिश्चित करें कि पहले एजेंटों को अपग्रेड करें, फिर मास्टर को।
* ⚠️ BREAKING [Issue #2889](https://github.com/manticoresoftware/manticoresearch/issues/2889) 'Name' कॉलम का नाम बदलकर 'Variable name' कर दिया गया है PQ [SHOW META](../Node_info_and_management/SHOW_META.md#SHOW-META-for-PQ-tables) के लिए।
* ⚠️ BREAKING [Issue #879](https://github.com/manticoresoftware/manticoresearch/issues/879) ने नए विकल्पों के साथ [प्रति-तालिका बिनलॉग](../Logging/Binary_logging.md#Per-table-binary-logging-configuration) पेश किया: [binlog_common](../Logging/Binary_logging.md#Binary-logging-strategies), [binlog](../Logging/Binary_logging.md#Per-table-binary-logging-configuration) `create table` / `alter table` के लिए। आपको नए संस्करण में अपग्रेड करने से पहले Manticore उदाहरण को सही तरीके से बंद करना होगा।
* ⚠️ BREAKING [Issue #1789](https://github.com/manticoresoftware/manticoresearch/issues/1789) ने गलत पुनरुत्पादन प्रोटोकॉल संस्करण के साथ क्लस्टर में शामिल होने पर गलत त्रुटि संदेश को ठीक किया। यह परिवर्तन पुनरुत्पादन प्रोटोकॉल को अपडेट करता है। यदि आप एक पुनरुत्पादन क्लस्टर चला रहे हैं, तो आपको:
  - पहले, अपने सभी नोड्स को सही तरीके से बंद करें
  - फिर, `--new-cluster` के साथ सबसे आखिर में बंद किए गए नोड को शुरू करें, Linux में `manticore_new_cluster` टूल का उपयोग करते हुए।
  - अधिक विवरण के लिए [क्लस्टर को फिर से शुरू करने](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) के बारे में पढ़ें।
* ⚠️ BREAKING [Issue #2308](https://github.com/manticoresoftware/manticoresearch/issues/2308) ने [`ALTER CLUSTER ADD` और `DROP`](../Creating_a_cluster/Setting_up_replication/Adding_and_removing_a_table_from_a_replication_cluster.md#Adding-and-removing-a-table-from-a-replication-cluster) में कई तालिकाओं के लिए समर्थन जोड़ा। इस परिवर्तन का पुनरुत्पादन प्रोटोकॉल पर भी प्रभाव पड़ता है। इस अपडेट को संभालने के लिए मार्गदर्शन के लिए पिछले खंड का संदर्भ लें।
* [Issue #2997](https://github.com/manticoresoftware/manticoresearch/issues/2997) ने Macos पर dlopen समस्या को ठीक किया।
* [Commit 4954](https://github.com/manticoresoftware/manticoresearch/commit/4954b5de7341a29902a9b8fbb9a040f7942c77c4) ने KNN इंडेक्स वाली तालिकाओं पर OPTIMIZE TABLE का डिफ़ॉल्ट कटऑफ बदल दिया ताकि खोज प्रदर्शन में सुधार हो सके।
* [Commit cfc8](https://github.com/manticoresoftware/manticoresearch/commit/cfc87ecb6e33a8163c2235243b6b40e699dbf526) ने `FACET` और `GROUP BY` में `ORDER BY` के लिए `COUNT(DISTINCT)` समर्थन जोड़ा।
* [Issue #1103](https://github.com/manticoresoftware/manticoresearch/issues/1103) ने [लॉगिंग](../Logging/Server_logging.md#Server-logging) खंड विलय में स्पष्टता को सुधार किया।
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) ने [DBeaver](../Integration/DBeaver.md) के लिए समर्थन जोड़ा।
* [Issue #1546](https://github.com/manticoresoftware/manticoresearch/issues/1546) ने [POLY2D()](../Functions/Geo_spatial_functions.md#POLY2D%28%29)/[GEOPOLY2D()](../Functions/Geo_spatial_functions.md#GEOPOLY2D%28%29) कार्यों के लिए द्वितीयक सूचकांकों को लागू किया।
* [Issue #1630](https://github.com/manticoresoftware/manticoresearch/issues/1630) HTTP अनुरोध अब `Content-Encoding: gzip` का समर्थन करते हैं।
* [Issue #1831](https://github.com/manticoresoftware/manticoresearch/issues/1831) ने `SHOW LOCKS` आदेश जोड़ा।
* [Issue #2187](https://github.com/manticoresoftware/manticoresearch/issues/2197) ने [searchd.max_connections](../Server_settings/Searchd.md#max_connections) बाधा को बायपास करने के लिए डेमन को बडी अनुरोध की अनुमति दी।
* [Issue #2208](https://github.com/manticoresoftware/manticoresearch/issues/2208) ने JSON HTTP इंटरफेस के माध्यम से तालिकाओं को जोड़ने के लिए समर्थन जोड़ा।
* [Issue #2235](https://github.com/manticoresoftware/manticoresearch/issues/2235) ने बडी के द्वारा सफलतापूर्वक संसाधित पूछताछ को उनके मूल रूप में लॉग किया।
* [Issue #2249](https://github.com/manticoresoftware/manticoresearch/issues/2249) ने दोहराए गए तालिकाओं के लिए `mysqldump` चलाने के लिए विशेष मोड जोड़ा।
* [Issue #2268](https://github.com/manticoresoftware/manticoresearch/issues/2268) ने `CREATE TABLE` और `ALTER TABLE` बयानों के लिए कॉपी पर बाहरी फ़ाइलों के नामकरण में सुधार किया।
* [Issue #2402](https://github.com/manticoresoftware/manticoresearch/issues/2402) ने [searchd.max_packet_size](../Server_settings/Searchd.md#max_packet_size) डिफ़ॉल्ट को 128MB में अपडेट किया।
* [Issue #2419](https://github.com/manticoresoftware/manticoresearch/issues/2419) ने JSON ["match"](../Searching/Full_text_matching/Basic_usage.md#match) में [IDF बूस्ट मॉडिफायर](../Searching/Full_text_matching/Operators.md#IDF-boost-modifier) के लिए समर्थन जोड़ा।
* [Issue #2430](https://github.com/manticoresoftware/manticoresearch/issues/2430) ने त्रुटियों को रोकने के लिए [binlog](../Logging/Binary_logging.md#Binary-logging) लेखन समन्वयन में सुधार किया।
* [Issue #2458](https://github.com/manticoresoftware/manticoresearch/issues/2458) ने Windows पैकेज में zlib समर्थन को शामिल किया।
* [Issue #2479](https://github.com/manticoresoftware/manticoresearch/issues/2479) ने SHOW TABLE INDEXES आदेश के लिए समर्थन जोड़ा।
* [Issue #2485](https://github.com/manticoresoftware/manticoresearch/issues/2485) ने बडी उत्तरों के लिए सत्र मेटाडेटा सेट किया।
* [Issue #2490](https://github.com/manticoresoftware/manticoresearch/issues/2490) ने संगठनों के लिए संगतता अंत बिंदुओं पर मिलीसेकंड रिज़ॉल्यूशन जोड़ा।
* [Issue #2500](https://github.com/manticoresoftware/manticoresearch/issues/2500) ने क्लस्टर संचालन के लिए त्रुटि संदेशों को बदला जब पुनरुत्पादन शुरू करने में विफल हो जाता है।
* [Issue #2584](https://github.com/manticoresoftware/manticoresearch/pull/2584) ने नई [प्रदर्शन मैट्रिक्स को SHOW STATUS में](../Node_info_and_management/Node_status.md#Query-Time-Statistics): पिछले 1, 5, और 15 मिनट के लिए हर पूछताछ प्रकार के लिए min/max/avg/95th/99th पर्सेंटाइल जोड़ा।
* [Issue #2639](https://github.com/manticoresoftware/manticoresearch/pull/2639) ने अनुरोधों और उत्तरों में `index` के सभी उदाहरणों को `table` के साथ प्रतिस्थापित किया।
* [Issue #2643](https://github.com/manticoresoftware/manticoresearch/issues/2643) ने HTTP `/sql` अंत बिंदु समाकलन परिणामों में `distinct` कॉलम जोड़ा।
* [Issue #268](https://github.com/manticoresoftware/manticoresearch-buddy/issues/268) ने Elasticsearch से आयात किए गए डेटा प्रकारों की स्वचालित पहचान को लागू किया।
* [Issue #2744](https://github.com/manticoresoftware/manticoresearch/issues/2744) ने स्ट्रिंग JSON फ़ील्ड की तुलना अभिव्यक्तियों के लिए कोलेशन समर्थन जोड़ा।
* [Issue #2752](https://github.com/manticoresoftware/manticoresearch/issues/2752) ने चयन सूची में `uuid_short` अभिव्यक्ति के लिए समर्थन जोड़ा।
* [Issue #2783](https://github.com/manticoresoftware/manticoresearch/issues/2783) Manticore Search अब Buddy को सीधे `manticore-buddy` रैपर के बिना चलाता है।
* [Issue #2785](https://github.com/manticoresoftware/manticoresearch/issues/2785) गायब तालिकाओं और तालिकाओं के लिए जो insert संचालन का समर्थन नहीं करतीं, के लिए विभेदित त्रुटि संदेश।
* [Issue #2789](https://github.com/manticoresoftware/manticoresearch/issues/2789) OpenSSL 3 अब `searchd` में स्थैतिक रूप से निर्मित है।
* [Issue #2790](https://github.com/manticoresoftware/manticoresearch/issues/2790) कई `uuid_short` मानों के साथ अनुक्रम उत्पन्न करने के लिए `CALL uuid_short` कथन जोड़ा गया।
* [Issue #2803](https://github.com/manticoresoftware/manticoresearch/issues/2803) JOIN ऑपरेशन में दाएं तालिका के लिए अलग-अलग विकल्प जोड़े गए।
* [Issue #2810](https://github.com/manticoresoftware/manticoresearch/issues/2810) SphinxQL में `GROUP BY` से मेल खाने के लिए HTTP JSON एग्रीगेशन प्रदर्शन को बेहतर बनाया गया।
* [Issue #2854](https://github.com/manticoresoftware/manticoresearch/issues/2854) Kibana दिनांक-संबंधित अनुरोधों में `fixed_interval` के लिए समर्थन जोड़ा गया।
* [Issue #2909](https://github.com/manticoresoftware/manticoresearch/issues/2909) JOIN क्वेरियों के लिए बैचिंग लागू की गई, जो कुछ JOIN क्वेरियों के प्रदर्शन को सैकड़ों या यहां तक कि हजारों बार सुधारती है।
* [Issue #2937](https://github.com/manticoresoftware/manticoresearch/issues/2937) पूर्णस्कैन क्वेरियों में जुड़े तालिका के वजन के उपयोग की अनुमति दी गई।
* [Issue #2953](https://github.com/manticoresoftware/manticoresearch/issues/2953) JOIN क्वेरियों के लिए लॉगिंग को ठीक किया गया।
* [Issue #337](https://github.com/manticoresoftware/manticoresearch-buddy/issues/337) गैर-डीबग मोड में `searchd` लॉग से Buddy अपवादों को छिपाया गया।
* [Issue #2931](https://github.com/manticoresoftware/manticoresearch/issues/2931) यदि उपयोगकर्ता दोहराव श्रोता के लिए गलत पोर्ट सेट करता है तो डेमन त्रुटि संदेश के साथ बंद हो जाता है।

### बग सुधार
* [Commit 0c6b](https://github.com/manticoresoftware/manticoresearch/commit/0c6bdaf41ea3684e05952e1a8273893535b923f5) ठीक किया गया: 32 कॉलम से अधिक JOIN क्वेरियों में गलत परिणाम लौटाए गए।
* [Issue #2335](https://github.com/manticoresoftware/manticoresearch/issues/2335) एक मुद्दा हल किया गया जहां दो json गुणों का उपयोग करने पर तालिका जोड़ी विफल हो गई।
* [Issue #2338](https://github.com/manticoresoftware/manticoresearch/issues/2338) [cutoff](../Searching/Options.md#cutoff) के साथ मल्टीक्वेरियों में गलत total_relation ठीक किया गया।
* [Issue #2366](https://github.com/manticoresoftware/manticoresearch/issues/2366) [तालिका जोड़ी](../Searching/Joining.md) पर दाएं तालिका में `json.string` द्वारा फ़िल्टरिंग को सही किया गया।
* [Issue #2406](https://github.com/manticoresoftware/manticoresearch/issues/2406) किसी भी POST HTTP JSON अंत बिंदुओं (insert/replace/bulk) में सभी मानों के लिए `null` के उपयोग की अनुमति दी गई। इस मामले में एक डिफ़ॉल्ट मान का उपयोग किया जाता है।
* [Issue #2418](https://github.com/manticoresoftware/manticoresearch/issues/2418) पहले सॉकेट परीक्षण के लिए [max_packet_size](../Server_settings/Searchd.md#max_packet_size) नेटवर्क बफ़र के आवंटन को समायोजित करके मेमोरी खपत को अनुकूलित किया गया।
* [Issue #2420](https://github.com/manticoresoftware/manticoresearch/issues/2420) JSON इंटरफ़ेस के माध्यम से bigint गुण में unsigned int के समावेश को ठीक किया गया।
* [Issue #2422](https://github.com/manticoresoftware/manticoresearch/issues/2422) exclude फ़िल्टर और pseudo_sharding सक्षम के साथ सही ढंग से काम करने के लिए द्वितीयक अनुक्रमणकों को ठीक किया गया।
* [Issue #2423](https://github.com/manticoresoftware/manticoresearch/issues/2423) [manticore_new_cluster](../Starting_the_server/Manually.md#searchd-command-line-options) में एक बग को संबोधित किया गया।
* [Issue #2448](https://github.com/manticoresoftware/manticoresearch/issues/2448) गलत फ़ॉर्मेटेड `_update` अनुरोध पर डेमन क्रैश का समाधान किया गया।
* [Issue #2452](https://github.com/manticoresoftware/manticoresearch/issues/2452) उच्चारणों की अपव्याप्ति को धनात्मक फ़िल्टरों के साथ संभालने में असमर्थता को ठीक किया गया।
* [Issue #55](https://github.com/manticoresoftware/columnar/issues/55) वितरित तालिकाओं के खिलाफ [knn](../Searching/KNN.md#KNN-vector-search) क्वेरियों को ठीक किया गया।
* [Issue #68](https://github.com/manticoresoftware/columnar/issues/68) कॉलम एक्सेसर में तालिका एन्कोडिंग पर exclude फ़िल्टर के संचालन को बेहतर बनाया गया।
* [Commit 0eb1](https://github.com/manticoresoftware/manticoresearch/commit/0eb1579462013cb134bcaf74e06f6eeb0c2ecc4d) ठीक किया गया कि अभिव्यक्ति पार्सर पुनर्परिभाषित `thread_stack` का पालन नहीं कर रहा था।
* [Commit c304](https://github.com/manticoresoftware/manticoresearch/commit/c3041fd1d3e14773f3c5aa635343915a71f5cb95) कॉलमनार IN अभिव्यक्ति को क्लोन करते समय एक दुर्घटना ठीक की गई।
* [Commit edad](https://github.com/manticoresoftware/columnar/commit/edadc694c68d6022bdd13134263667430a42cc1d) बिटमैप इटरेटर में उलटाव समस्या को ठीक किया गया जो एक दुर्घटना का कारण बनी।
* [Commit fc30](https://github.com/manticoresoftware/executor/commit/fc302ff1117b0b835a6f5a3c07057baf5fba14d9) ठीक किया गया कि कुछ Manticore पैकेज `unattended-upgrades` द्वारा स्वचालित रूप से हटा दिए गए थे।
* [Issue #1019](https://github.com/manticoresoftware/manticoresearch/issues/1019) DbForge MySQL टूल से क्वेरियों के संचालन में सुधार किया गया।
* [Issue #1107](https://github.com/manticoresoftware/manticoresearch/issues/1107) `CREATE TABLE` और `ALTER TABLE` में विशेष वर्णों के एस्केपिंग को ठीक किया गया।  ❤️ धन्यवाद, [@subnix](https://github.com/subnix) PR के लिए।
* [Issue #116](https://github.com/manticoresoftware/manticoresearch-backup/issues/116#issuecomment-2216597206) एक रूपांतरित अनुक्रमण में बर्फबंदी गुण को अपडेट करते समय एक डेडलॉक ठीक किया गया। यह डेडलॉक अनुक्रमण को अनफ्रीज़ करने के प्रयास करते समय संघर्षशील लॉक के कारण हुआ। यह manticore-backup में भी विफलता का कारण बन सकता है।
* [Issue #1818](https://github.com/manticoresoftware/manticoresearch/issues/1818) `OPTIMIZE` अब उस समय त्रुटि फेंकता है जब तालिका बर्फबंद होती है।
* [Issue #2001](https://github.com/manticoresoftware/manticoresearch/issues/2001) फ़ंक्शन नामों का उपयोग कॉलम नामों के रूप में करने की अनुमति दी गई।
* [Issue #2153](https://github.com/manticoresoftware/manticoresearch/issues/2153) अज्ञात डिस्क_chunk के साथ तालिका सेटिंग्स को क्वेरी करते समय डेमन क्रैश को ठीक किया गया।
* [Issue #2184](https://github.com/manticoresoftware/manticoresearch/issues/2184) `FREEZE` और `FLUSH RAMCHUNK` के बाद `searchd` के रुकने में समस्या ठीक की गई।
* [Issue #2228](https://github.com/manticoresoftware/manticoresearch/issues/2228) आरक्षित शब्दों से दिनांक/समय से संबंधित टोकन (और regex) हटा दिए गए।
* [Issue #2255](https://github.com/manticoresoftware/manticoresearch/issues/2255) 5 से अधिक क्रम क्षेत्र वाले `FACET` पर क्रैश ठीक किया गया।
* [Issue #2265](https://github.com/manticoresoftware/manticoresearch/issues/2265) `index_field_lengths` सक्षम होने पर `mysqldump` को पुनर्स्थापित करने में विफलता ठीक की गई।
* [Issue #2291](https://github.com/manticoresoftware/manticoresearch/issues/2291) `ALTER TABLE` कमांड निष्पादित करते समय क्रैश ठीक किया गया।
* [Issue #2333](https://github.com/manticoresoftware/manticoresearch/issues/2333) इंडेक्सर के लिए Windows पैकेज में MySQL DLL ठीक की गई ताकि सही से काम कर सके।
* [Issue #2393](https://github.com/manticoresoftware/manticoresearch/issues/2393) GCC संकलन त्रुटि ठीक की गई। ❤️ धन्यवाद, [@animetosho](https://github.com/animetosho) PR के लिए।
* [Issue #2447](https://github.com/manticoresoftware/manticoresearch/issues/2447) [_update](../Data_creation_and_modification/Updating_documents/REPLACE.md?client=Elasticsearch-like+partial) में एकescaping समस्या ठीक की गई।
* [Issue #2460](https://github.com/manticoresoftware/manticoresearch/issues/2460) समान नाम वाले कई गुण या क्षेत्रों को घोषित करते समय इंडेक्सर के क्रैश को ठीक किया गया।
* [Issue #2467](https://github.com/manticoresoftware/manticoresearch/issues/2467) "compat" खोज से संबंधित अंत बिंदुओं के लिए नेस्टेड बोल क्वेरीज के लिए गलत परिवर्तन पर डेमॉन के क्रैश को हल किया गया।
* [Issue #2493](https://github.com/manticoresoftware/manticoresearch/issues/2493) संशोधकों के साथ वाक्यों में विस्तार ठीक किया गया।
* [Issue #2535](https://github.com/manticoresoftware/manticoresearch/issues/2535) [ZONE](../Searching/Full_text_matching/Operators.md#ZONE-limit-operator) या [ZONESPAN](../Searching/Full_text_matching/Operators.md#ZONESPAN-limit-operator) पूर्ण पाठ ऑपरेटर का उपयोग करते समय डेमॉन क्रैश को हल किया गया।
* [Issue #2552](https://github.com/manticoresoftware/manticoresearch/issues/2552) कीवर्ड डिक्शनरी के साथ साधारण और RT तालिकाओं के लिए इनफ़िक्स निर्माण ठीक किया गया।
* [Issue #2571](https://github.com/manticoresoftware/manticoresearch/issues/2571) `FACET` क्वेरी में त्रुटि जवाब को ठीक किया; `COUNT(*)` के साथ `FACET` के लिए डिफ़ॉल्ट क्रम क्रम को `DESC` पर सेट किया।
* [Issue #2580](https://github.com/manticoresoftware/manticoresearch/issues/2580) विंडोज पर स्टार्टअप के दौरान डेमॉन क्रैश ठीक किया गया।
* [Issue #2603](https://github.com/manticoresoftware/manticoresearch/issues/2603) HTTP अंत बिंदुओं `/sql` और `/sql?mode=raw` के लिए क्वेरी ट्रंकेशन ठीक किया गया; इन अंत बिंदुओं से अनुरोध करते समय बिना `query=` हेडर की आवश्यकता के अनुरोधों को स्थिर बनाया गया।
* [Issue #2623](https://github.com/manticoresoftware/manticoresearch/issues/2623) स्वचालित स्कीमा द्वारा तालिका बनाने में समस्या ठीक की गई लेकिन साथ ही विफल।
* [Issue #2627](https://github.com/manticoresoftware/manticoresearch/issues/2627) कई KNN इंडेक्स लोड करने के लिए HNSW पुस्तकालय को ठीक किया गया।
* [Issue #2630](https://github.com/manticoresoftware/manticoresearch/issues/2630) जब कई शर्तें एक साथ होती हैं तो फ्रीज समस्या ठीक की गई।
* [Issue #2645](https://github.com/manticoresoftware/manticoresearch/issues/2645) KNN खोज के साथ `OR` का उपयोग करते समय बुनियादी त्रुटि के साथ क्रैश ठीक किया गया।
* [Issue #2647](https://github.com/manticoresoftware/manticoresearch/issues/2647) `indextool --mergeidf *.idf --out global.idf` का उपयोग करते समय निर्माण के बाद आउटपुट फ़ाइल हटाने की समस्या ठीक की गई।
* [Issue #2658](https://github.com/manticoresoftware/manticoresearch/issues/2658) बाहरी चयन में `ORDER BY` स्ट्रिंग के साथ उप-चयन पर डेमॉन क्रैश ठीक किया गया।
* [Issue #2686](https://github.com/manticoresoftware/manticoresearch/issues/2686) स्ट्रिंग एट्रीब्यूट के साथ फ्लोट एट्रीब्यूट को अपडेट करते समय क्रैश ठीक किया गया।
* [Issue #2704](https://github.com/manticoresoftware/manticoresearch/issues/2704) `lemmatize_xxx_all` टोकनाइज़र से कई स्टॉपवर्ड्स द्वारा `hitpos` को बढ़ाने की समस्या ठीक की गई।
* [Issue #2708](https://github.com/manticoresoftware/manticoresearch/issues/2708) `ALTER ... ADD COLUMN ... TEXT` पर क्रैश ठीक किया गया।
* [Issue #2737](https://github.com/manticoresoftware/manticoresearch/issues/2737) एक बर्फ़ की टेबल में बाइनरी एट्रीब्यूट को अपडेट करते समय कम से कम एक RAM चंक के साथ आने वाली समस्या ठीक की गई जिससे बाद में `SELECT` क्वेरियों को तालिका के अनफ्रीज होने का इंतज़ार करना पड़ता है।
* [Issue #2742](https://github.com/manticoresoftware/manticoresearch/issues/2742) पैक्ड फैक्टरों के साथ क्वेरी कैश छोड़ने की समस्या ठीक की गई।
* [Issue #2775](https://github.com/manticoresoftware/manticoresearch/issues/2775) Manticore अब अज्ञात कार्रवाई पर क्रैश होने के बजाय `_bulk` अनुरोध पर त्रुटि की रिपोर्ट करता है।
* [Issue #2791](https://github.com/manticoresoftware/manticoresearch/issues/2791) HTTP `_bulk` अंत बिंदु के लिए डाले गए दस्तावेज़ ID की वापसी ठीक की गई।
* [Issue #2797](https://github.com/manticoresoftware/manticoresearch/issues/2797) कई तालिकाओं को संभालने के लिए समूहकर्ता में क्रैश ठीक किया गया, जिनमें से एक खाली है और दूसरी में मिलान किए गए प्रविष्टियों की अलग संख्या है।
* [Issue #2835](https://github.com/manticoresoftware/manticoresearch/issues/2835) जटिल `SELECT` क्वेरियों में क्रैश ठीक किया गया।
* [Issue #2872](https://github.com/manticoresoftware/manticoresearch/issues/2872) यदि `IN` अभिव्यक्ति में `ALL` या `ANY` तर्क JSON एट्रीब्यूट नहीं है तो त्रुटि संदेश जोड़ा गया।
* [Issue #2882](https://github.com/manticoresoftware/manticoresearch/issues/2882) बड़े तालिकाओं में MVA को अपडेट करते समय डेमॉन क्रैश ठीक किया गया।
* [Issue #2888](https://github.com/manticoresoftware/manticoresearch/issues/2888) `libstemmer` के साथ टोकनकरण विफलता पर क्रैश ठीक किया गया। ❤️ धन्यवाद, [@subnix](https://github.com/subnix) PR के लिए।
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) यह समस्या ठीक की गई जहां दाएं तालिका से सम्मिलित वजन अभिव्यक्तियों में सही से काम नहीं कर रहा था।
* [Issue #2919](https://github.com/manticoresoftware/manticoresearch/issues/2919) दाईं ओर जुड़ी टेबल वजन में अभिव्यक्तियों में काम नहीं करने की समस्या को ठीक किया गया।
* [Issue #325](https://github.com/manticoresoftware/manticoresearch-buddy/issues/325) `CREATE TABLE IF NOT EXISTS ... WITH DATA` विफलता को ठीक किया गया जब तालिका पहले से मौजूद थी।
* [Issue #351](https://github.com/manticoresoftware/manticoresearch-buddy/issues/351) दस्तावेज़ ID के साथ KNN द्वारा गणना में "id" त्रुटि को ठीक किया गया।
* [Issue #359](https://github.com/manticoresoftware/manticoresearch-buddy/issues/359) `REPLACE INTO cluster_name:table_name` कार्यक्षमता को ठीक किया गया।
* [Issue #67](https://github.com/manticoresoftware/docker/issues/67) `--network=host` के साथ Manticore Docker कंटेनर चलाने पर FATAL त्रुटि को ठीक किया गया।

# Version 6.3.8
Released: 22 नवंबर 2024

Version 6.3.8 6.3 श्रृंखला को जारी रखता है और इसमें केवल बग सुधार शामिल हैं।

### बग सुधार

* [PR #2777](https://github.com/manticoresoftware/manticoresearch/pull/2777) `threads` या `max_threads_per_query` सेटिंग्स द्वारा सीमित पूछताछ सह-अवृत्ति के समय उपलब्ध थ्रेड की गणना को ठीक किया गया।

# manticore-extra v1.1.20

Released: 7 अक्टूबर 2024

### बग सुधार

* [Issue #64](https://github.com/manticoresoftware/executor/issues/64) एक समस्या को हल किया गया जहां `unattended-upgrades` उपयोगिता, जो डेबियन-आधारित सिस्टम पर पैकेज अपडेट स्वचालित रूप से स्थापित करती है, गलत तरीके से कई Manticore पैकेजों को, जिसमें `manticore-galera`, `manticore-executor`, और `manticore-columnar-lib`, को हटाने के लिए चिह्नित कर देगी। यह `dpkg` द्वारा आभासी पैकेज `manticore-extra` को अनावश्यक के रूप में गलत समझने के कारण हुआ। यह सुनिश्चित करने के लिए परिवर्तन किए गए कि `unattended-upgrades` अब आवश्यक Manticore घटकों को हटाने की कोशिश न करे।

# Version 6.3.6
Released: 2 अगस्त 2024

Version 6.3.6 6.3 श्रृंखला को जारी रखता है और इसमें केवल बग सुधार शामिल हैं।

### बग सुधार

* [Issue #2477](https://github.com/manticoresoftware/manticoresearch/issues/2477) संस्करण 6.3.4 में पेश की गई एक दुर्घटना को ठीक किया गया, जो अभिव्यक्तियों और वितरित या कई तालिकाओं के साथ काम करते समय हो सकता है।
* [Issue #2352](https://github.com/manticoresoftware/manticoresearch/issues/2352) कई अनुक्रमांक प्राप्त करते समय `max_query_time` के कारण जल्दी बाहर निकलने पर डेमन क्रैश या आंतरिक त्रुटि को ठीक किया गया।

# Version 6.3.4
Released: 31 जुलाई 2024

Version 6.3.4 6.3 श्रृंखला को जारी रखता है और इसमें केवल छोटे सुधार और बग सुधार शामिल हैं।

### छोटे परिवर्तन
* [Issue #1130](https://github.com/manticoresoftware/manticoresearch/issues/1130) [DBeaver](../Integration/DBeaver.md#Integration-with-DBeaver) के लिए समर्थन जोड़ा गया।
* [Issue #2146](https://github.com/manticoresoftware/manticoresearch/issues/2146) शब्द रूपों और अपवादों में सीमा चिन्हों के एस्केपिंग में सुधार किया गया।
* [Issue #2315](https://github.com/manticoresoftware/manticoresearch/issues/2315) SELECT सूची अभिव्यक्तियों में स्ट्रिंग तुलना ऑपरेटर जोड़े गए।
* [Issue #2363](https://github.com/manticoresoftware/manticoresearch/issues/2363) Elastic-जैसे बल्क अनुरोधों में नल मानों के लिए समर्थन जोड़ा गया।
* [Issue #2374](https://github.com/manticoresoftware/manticoresearch/issues/2374) mysqldump संस्करण 9 के लिए समर्थन जोड़ा गया।
* [Issue #2375](https://github.com/manticoresoftware/manticoresearch/issues/2375) HTTP JSON क्वेरी में त्रुटि हैंडलिंग में सुधार किया गया जिसमें त्रुटि होने वाले नोड के लिए JSON पथ शामिल है।

### बग सुधार
* [Issue #2280](https://github.com/manticoresoftware/manticoresearch/issues/2280) डिस्क_चंक्स > 1 के साथ कई मिलानों में वाइल्डकार्ड क्वेरी में प्रदर्शन गिरावट को ठीक किया गया।
* [Issue #2332](https://github.com/manticoresoftware/manticoresearch/issues/2332) खाली MVA Arrays के लिए MVA MIN या MAX SELECT सूची अभिव्यक्तियों में क्रैश को ठीक किया गया।
* [Issue #2339](https://github.com/manticoresoftware/manticoresearch/issues/2339) Kibana के अनंत रेंज अनुरोध की गलत प्रोसेसिंग को ठीक किया गया।
* [Issue #2342](https://github.com/manticoresoftware/manticoresearch/issues/2342) SELECT सूची में गुणांक नहीं होने पर दाहिनी तालिका से कॉलम विशेषताओं पर जुड़ने वाली फ़िल्टर को ठीक किया गया।
* [Issue #2343](https://github.com/manticoresoftware/manticoresearch/issues/2343) Manticore 6.3.2 में डुप्लिकेट 'स्टेटिक' विशेषतावाचक को ठीक किया गया।
* [Issue #2344](https://github.com/manticoresoftware/manticoresearch/issues/2344) जब MATCH() दाहिनी तालिका पर उपयोग किया जाता है तो NON-MATCHING प्रविष्टियाँ लौटाने वाले LEFT JOIN को ठीक किया गया।
* [Issue #2350](https://github.com/manticoresoftware/manticoresearch/issues/2350) `hitless_words` के साथ RT अनुक्रमांक पर डिस्क चंक को सहेजने को ठीक किया गया।
* [Issue #2364](https://github.com/manticoresoftware/manticoresearch/issues/2364) 'aggs_node_sort' प्रॉपर्टी अब अन्य प्रॉपर्टीज़ के बीच किसी भी क्रम में जोड़ी जा सकती है।
* [Issue #2368](https://github.com/manticoresoftware/manticoresearch/issues/2368) JSON क्वेरी में पूर्ण पाठ बनाम फ़िल्टर क्रम पर त्रुटि को ठीक किया गया।
* [Issue #2376](https://github.com/manticoresoftware/manticoresearch/issues/2376) लंबे UTF-8 अनुरोधों के लिए गलत JSON प्रतिक्रिया से संबंधित बग को ठीक किया गया।
* [Issue #2684](https://github.com/manticoresoftware/dev/issues/2684) जुड़े गुणों पर निर्भर पूर्व-क्रम/पूर्व-फ़िल्टर अभिव्यक्तियों की गणना को ठीक किया गया।
* [Issue #301](https://github.com/manticoresoftware/manticoresearch-buddy/issues/301) मेट्रिक्स के लिए डेटा आकार की गणना के तरीके को बदल दिया गया है ताकि डेटा निर्देशिका के पूरे आकार की जाँच करने के बजाय `manticore.json` फ़ाइल से पढ़ा जा सके।
* [Issue #302](https://github.com/manticoresoftware/manticoresearch-buddy/issues/302) Vector.dev से मान्यता अनुरोधों के हैंडलिंग को जोड़ा गया।

# Version 6.3.2
Released: 26 जून 2024

Version 6.3.2 6.3 श्रृंखला को जारी रखता है और इसमें कई बग सुधार शामिल हैं, जिनमें से कुछ 6.3.0 के रिलीज़ के बाद खोजे गए थे।

### ब्रेकिंग परिवर्तन
* ⚠️[Issue #2305](https://github.com/manticoresoftware/manticoresearch/issues/2305) aggs.range मानों को संख्यात्मक रूप में अपडेट किया गया।

### बग सुधार
* [Commit c51c](https://github.com/manticoresoftware/manticoresearch/commit/c51c7fa4bd0ec4ad8d53188b888461bff29f1417) संग्रहित चेक द्वारा समूहबद्ध करना और आरसेट मर्ज ठीक किया।
* [Commit 0e85](https://github.com/manticoresoftware/manticoresearch/commit/0e85b9d5682f51681750d798aef6a11cfb93d69e) एक CRC शब्दकोश का उपयोग करते समय RT अनुक्रमणिका में वाइल्डकार्ड वर्णों के साथ क्वेरी करते समय डेमन में एक क्रैश ठीक किया और `local_df` सक्षम किया गया।
* [Issue #2200](https://github.com/manticoresoftware/manticoresearch/issues/2200) बिना GROUP BY के `count(*)` पर JOIN में एक क्रैश ठीक किया।
* [Issue #2201](https://github.com/manticoresoftware/manticoresearch/issues/2201) पूर्ण-पाठ फ़ील्ड द्वारा समूह बनाने का प्रयास करते समय JOIN से चेतावनी न लौटाने की समस्या ठीक की।
* [Issue #2230](https://github.com/manticoresoftware/manticoresearch/issues/2230) मुद्दा संबोधित किया जहां `ALTER TABLE` के माध्यम से एक विशेषता जोड़ने से KNN विकल्पों को ध्यान में नहीं रखा गया।
* [Issue #2231](https://github.com/manticoresoftware/manticoresearch/issues/2231) संस्करण 6.3.0 में `manticore-tools` Redhat पैकेज को हटाने में विफलता ठीक की।
* [Issue #2242](https://github.com/manticoresoftware/manticoresearch/issues/2242) JOIN और कई FACET बयानों के साथ मुद्दों को ठीक किया जो गलत परिणाम लौटाते थे।
* [Issue #2250](https://github.com/manticoresoftware/manticoresearch/issues/2250) यदि तालिका क्लस्टर में है तो ALTER TABLE द्वारा एक त्रुटि उत्पन्न करने की समस्या ठीक की।
* [Issue #2252](https://github.com/manticoresoftware/manticoresearch/issues/2252) SphinxQL इंटरफ़ेस से बडी में पास किया गया मूल प्रश्न ठीक किया।
* [Issue #2267](https://github.com/manticoresoftware/manticoresearch/issues/2267) डिस्क चंक्स के साथ RT अनुक्रमणिका के लिए `CALL KEYWORDS` में वाइल्डकार्ड विस्तार में सुधार किया।
* [Issue #271](https://github.com/manticoresoftware/manticoresearch-buddy/issues/271) गलत `/cli` अनुरोधों का लटकना ठीक किया।
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) समस्याओं को हल किया जहां Manticore के लिए समवर्ती अनुरोध फंस सकते थे।
* [Issue #275](https://github.com/manticoresoftware/manticoresearch-buddy/issues/275) `/cli` के माध्यम से `drop table if exists t; create table t` का लटकना ठीक किया।

### पुनरुत्पादन-संबंधी
* [Issue #2270](https://github.com/manticoresoftware/manticoresearch/issues/2270) `/_bulk` HTTP एंडपॉइंट में `cluster:name` प्रारूप के लिए समर्थन जोड़ा।

# संस्करण 6.3.0
जारी: 23 मई 2024

### प्रमुख परिवर्तन
* [Issue #839](https://github.com/manticoresoftware/manticoresearch/issues/839) [float_vector](Creating_a_table/Data_types.md#Float-vector) डेटा प्रकार लागू किया; [vector search](Searching/KNN.md#KNN-vector-search) लागू किया।
* [Issue #1673](https://github.com/manticoresoftware/manticoresearch/issues/1673) [INNER/LEFT JOIN](Searching/Joining.md) (**बीटा चरण**)।
* [Issue #1744](https://github.com/manticoresoftware/manticoresearch/issues/1744) [timestamp](../Creating_a_table/Data_types.md#Timestamps) फ़ील्ड के लिए दिनांक प्रारूपों का स्वत: पता लगाने का कार्यान्वयन किया।
* [Issue #1720](https://github.com/manticoresoftware/manticoresearch/pull/1720) Manticore सर्च लाइसेंस को GPLv2-or-later से GPLv3-or-later में बदल दिया।
* [Commit 7a55](https://github.com/manticoresoftware/manticoresearch/commit/7a5508424) अब Windows में Manticore चलाने के लिए Buddy चलाने के लिए Docker की आवश्यकता है।
* [Issue #1541](https://github.com/manticoresoftware/manticoresearch/issues/1541) एक [REGEX](Searching/Full_text_matching/Operators.md#REGEX-operator) पूर्ण-पाठ ऑपरेटर जोड़ा।
* [Issue #2091](https://github.com/manticoresoftware/manticoresearch/issues/2091) Ubuntu Noble 24.04 समर्थन।
* [Commit 514d](https://github.com/manticoresoftware/manticoresearch/commit/514d35b497f4bdb20b6473ff963752a978a4bb8d) बेहतर प्रदर्शन और नई दिनांक/समय फ़ंक्शनों के लिए समय संचालन का पुनःआविष्कार:
  - [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) - स्थानीय टाइमज़ोन में वर्तमान दिनांक लौटाता है
  - [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) - एक टाइमस्टैम्प तर्क से वर्ष की पूर्णांक तिमाही लौटाता है
  - [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) - एक दिए गए टाइमस्टैम्प तर्क के लिए वीकडे का नाम लौटाता है
  - [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) - एक दिए गए टाइमस्टैम्प तर्क के लिए महीने का नाम लौटाता है
  - [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) - एक दिए गए टाइमस्टैम्प तर्क के लिए पूर्णांक वीकडे अनुक्रमांक लौटाता है
  - [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) - एक दिए गए टाइमस्टैम्प तर्क के लिए पूर्णांक वर्ष का दिन लौटाता है
  - [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) - एक दिए गए टाइमस्टैम्प तर्क के लिए वर्तमान सप्ताह के पहले दिन का पूर्णांक वर्ष और दिन कोड लौटाता है
  - [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) - दिए गए दो टाइमस्टैम्प के बीच के दिनों की संख्या लौटाता है
  - [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) - एक टाइमस्टैम्प तर्क से दिनांक भाग को प्रारूपित करता है
  - [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) - एक टाइमस्टैम्प तर्क से समय भाग को प्रारूपित करता है
  - [timezone](Server_settings/Searchd.md#timezone) - दिनांक/समय से संबंधित कार्यों द्वारा उपयोग किया जाने वाला समय क्षेत्र।
* [Commit 30e7](https://github.com/manticoresoftware/manticoresearch/commit/30e789d9657f9b3093c280c6e5b0e46a021f6dc7) HTTP इंटरफ़ेस में [range](Searching/Faceted_search.md#Facet-over-set-of-ranges), [histogram](Searching/Faceted_search.md#Facet-over-histogram-values), [date_range](Searching/Faceted_search.md#Facet-over-set-of-date-ranges), और [date_histogram](Searching/Faceted_search.md#Facet-over-histogram-date-values) एग्रीगेट्स और SQL में समान अभिव्यक्तियों को जोड़ा।

### छोटे परिवर्तन
* [Issue #1285](https://github.com/manticoresoftware/manticoresearch/issues/1285) Filebeat संस्करण 8.10 - 8.11 का समर्थन।
* [Issue #1771](https://github.com/manticoresoftware/manticoresearch/issues/1771) [ALTER TABLE ... type='distributed'](Updating_table_schema_and_settings.md#Changing-a-distributed-table)।
* [Issue #1788](https://github.com/manticoresoftware/manticoresearch/issues/1788) टेबलों को कॉपी करने की क्षमता जोड़ी गई है [CREATE TABLE ... LIKE ... WITH DATA](Creating_a_table/Local_tables/Real-time_table.md#CREATE-TABLE-LIKE:) SQL वक्तव्य का उपयोग करके।
* [Issue #2072](https://github.com/manticoresoftware/manticoresearch/issues/2072) [टेबल संकुचन एल्गोरिदम](Securing_and_compacting_a_table/Compacting_a_table.md#Compacting-a-Table) का ऑप्टिमाइजेशन: पहले, मैनुअल [OPTIMIZE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) और स्वचालित [auto_optimize](Server_settings/Searchd.md#auto_optimize) प्रक्रियाएँ पहले टुकड़ों को एकत्रित करेंगी ताकि गिनती सीमा से अधिक न हो जाए, और फिर सभी अन्य टुकड़ों से हटाए गए दस्तावेज़ों को निकालेंगी जिनमें हटा दिए गए दस्तावेज़ शामिल थे। यह दृष्टिकोण कभी-कभी संसाधन-गहन था और इसे अक्षम कर दिया गया है। अब, टुकड़ों का सम्मिलन केवल [progressive_merge](Server_settings/Common.md#progressive_merge) सेटिंग के अनुसार होता है। हालाँकि, जिन टुकड़ों में हटाए गए दस्तावेजों की संख्या अधिक होती है, वे अधिक संभावना रखते हैं कि वे मर्ज हों।
* [Commit ce6c](https://github.com/manticoresoftware/columnar/commit/ce6c5d3381edcdc411a00a5923d13d56dd9b3010) नए संस्करण के एक माध्यमिक अनुक्रमणिका को लोड करने के खिलाफ सुरक्षा जोड़ी गई।
* [Issue #1417](https://github.com/manticoresoftware/manticoresearch/issues/1417) के माध्यम से आंशिक प्रतिस्थापन [REPLACE INTO ... SET](Data_creation_and_modification/Updating_documents/REPLACE.md#SQL-REPLACE)।
* [Commit 7c16](https://github.com/manticoresoftware/manticoresearch/commit/7c167b934ce416d3dfbd658744a231e638b14d9a) डिफ़ॉल्ट समामेलन बफर आकार अपडेट किए गए: `.spa` (Scalar attrs): 256KB -> 8MB; `.spb` (Blob attrs): 256KB -> 8MB; `.spc` (Columnar attrs): 1MB, कोई परिवर्तन नहीं; `.spds` (Docstore): 256KB -> 8MB; `.spidx` (Secondry indexes): 256KB बफर -> 128MB मेमरी सीमा; `.spi` (Dictionary): 256KB -> 16MB; `.spd` (Doclists): 8MB, कोई परिवर्तन नहीं; `.spp` (Hitlists): 8MB, कोई परिवर्तन नहीं; `.spe` (Skiplists): 256KB -> 8MB।
* [Issue #1859](https://github.com/manticoresoftware/manticoresearch/issues/1859) [संमिश्रण एकत्रीकरण](Searching/Grouping.md#GROUP-BY-multiple-fields-at-once) जोड़ा गया JSON के माध्यम से।
* [Commit 216b](https://github.com/manticoresoftware/manticoresearch-buddy/commit/216b824) PCRE.JIT को कुछ regex पैटर्न के साथ समस्याओं और कोई महत्वपूर्ण समय लाभ न होने के कारण अक्षम किया गया।
* [Commit 55cd](https://github.com/manticoresoftware/manticoresearch/commit/55cdce32) वैनिला गलेरा v.3 (api v25) के लिए समर्थन जोड़ा गया (`libgalera_smm.so` MySQL 5.x से)।
* [Commit 86f9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/86f98ab) मैट्रिक उपसर्ग को `_rate` से `_rps` में बदला गया।
* [Commit c0c1](https://github.com/manticoresoftware/manticoresearch-helm/commit/c0c1cff780031412ef57cefab987dfd16613368e) संतुलक HA समर्थन के बारे में दस्तावेज़ में सुधार किया गया।
* [Commit d1d2](https://github.com/manticoresoftware/manticoresearch/commit/d1d2ca18857c68106e89743756260e73285be739) त्रुटि संदेशों में `index` को `table` में बदला गया; बाइसन पार्सर त्रुटि संदेश सुधार को ठीक किया गया।
* [Commit fd26](https://github.com/manticoresoftware/manticoresearch/commit/fd26671b) `manticore.tbl` को टेबल नाम के रूप में समर्थन।
* [Issue #1105](https://github.com/manticoresoftware/manticoresearch/issues/1105) सिस्टमडी के माध्यम से इंडेक्सर चलाने का समर्थन ([docs](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Schedule-indexer-via-systemd)). ❤️ धन्यवाद, [@subnix](https://github.com/subnix) PR के लिए।
* [Issue #1294](https://github.com/manticoresoftware/manticoresearch/issues/1294) GEODIST() में द्वितीयक अनुक्रमणिका का समर्थन।
* [Issue #1394](https://github.com/manticoresoftware/manticoresearch/issues/1394) [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md#SHOW-THREADS) को सरल बनाया गया।
* [Issue #1424](https://github.com/manticoresoftware/manticoresearch/issues/1424) `create distributed table` वक्तव्य के लिए डिफ़ॉल्ट मान (`agent_connect_timeout` और `agent_query_timeout`) का समर्थन जोड़ा गया।
* [Issue #1442](https://github.com/manticoresoftware/manticoresearch/issues/1442) [expansion_limit](Searching/Options.md#expansion_limit) खोज क्वेरी विकल्प जोड़ा गया जो `searchd.expansion_limit` को ओवरराइड करता है।
* [Issue #1448](https://github.com/manticoresoftware/manticoresearch/issues/1448) int->bigint रूपांतरण के लिए [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode) कार्यान्वित किया गया।
* [Issue #146](https://github.com/manticoresoftware/manticoresearch/issues/1456) MySQL उत्तर में मेटा जानकारी।
* [Issue #1494](https://github.com/manticoresoftware/manticoresearch/issues/1494) [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION)।
* [Issue #1582](https://github.com/manticoresoftware/manticoresearch/issues/1582) [id array द्वारा दस्तावेज़ों को हटाने का समर्थन](Data_creation_and_modification/Deleting_documents.md?client=JSON#Deleting-documents) JSON के माध्यम से।
* [Issue #1589](https://github.com/manticoresoftware/manticoresearch/issues/1589) त्रुटि "असमर्थित मान प्रकार" में सुधार किया गया।
* [Issue #1634](https://github.com/manticoresoftware/manticoresearch/issues/1634) `SHOW STATUS` में बडी संस्करण जोड़ा गया।
* [Issue #1641](https://github.com/manticoresoftware/manticoresearch/issues/1641) किसी कीवर्ड के लिए शून्य दस्तावेज़ों के मामले में अनुरोधों का अनुकूलन।
* [Issue #1712](https://github.com/manticoresoftware/manticoresearch/issues/1712) डेटा पोस्ट करते समय "true" और "false" के स्ट्रिंग मान से बूल एट्रिब्यूट में रूपांतरण जोड़ा गया।
* [Issue #1713](https://github.com/manticoresoftware/manticoresearch/issues/1713) [access_dict](Server_settings/Searchd.md#access_dict) टेबल और searchd विकल्प जोड़ा गया।
* [Issue #1767](https://github.com/manticoresoftware/manticoresearch/issues/1767) नए विकल्प जोड़े गए: [expansion_merge_threshold_docs](Server_settings/Searchd.md#expansion_merge_threshold_docs) और [expansion_merge_threshold_hits](Server_settings/Searchd.md#expansion_merge_threshold_hits) कॉन्फ़िग के searchd सेक्शन में; विस्तृत शर्तों के छोटे शर्तों के समामेलन के लिए थ्रेशोल्ड को कॉन्फ़िगर करने योग्य बनाया गया।
* [Issue #1768](https://github.com/manticoresoftware/manticoresearch/issues/1768) `@@system.sessions` में अंतिम कमांड समय का प्रदर्शन जोड़ा गया।
* [Issue #1806](https://github.com/manticoresoftware/manticoresearch/issues/1806) बडी प्रोटोकॉल को संस्करण 2 में अपग्रेड किया गया।
* [Issue #1810](https://github.com/manticoresoftware/manticoresearch/issues/1810) लाइब्रेरी के साथ समाकलन को सरल बनाने के लिए `/sql` एंडपॉइंट पर अतिरिक्त अनुरोध प्रारूप लागू किए गए।
* [Issue #1825](https://github.com/manticoresoftware/manticoresearch/pull/1825) SHOW BUDDY PLUGINS में एक जानकारी अनुभाग जोड़ा गया।
* [Issue #1837](https://github.com/manticoresoftware/manticoresearch/issues/1837) बड़े पैकेट के साथ `CALL PQ` में मेमोरी खपत में सुधार किया गया।
* [Issue #1853](https://github.com/manticoresoftware/manticoresearch/pull/1853) कंपाइलर को Clang 15 से Clang 16 में स्विच किया गया।
* [Issue #1857](https://github.com/manticoresoftware/manticoresearch/issues/1857) स्ट्रिंग तुलना जोड़ी गई। ❤️ धन्यवाद, [@etcd](https://github.com/etcd) PR के लिए।
* [Issue #1915](https://github.com/manticoresoftware/manticoresearch/issues/1915) संयुक्त संग्रहित फ़ील्ड के लिए समर्थन जोड़ा गया।
* [Issue #1937](https://github.com/manticoresoftware/manticoresearch/issues/1937) क्वेरी-लॉग में क्लाइंट के होस्ट:पोर्ट को लॉग किया गया।
* [Issue #1981](https://github.com/manticoresoftware/manticoresearch/issues/1981) गलत त्रुटि ठीक की गई।
* [Issue #1983](https://github.com/manticoresoftware/manticoresearch/issues/1983) JSON के जरिए [क्वेरी योजना](Node_info_and_management/Profiling/Query_plan.md?client=JSON#Query-plan) के लिएverbosity स्तरों का समर्थन पेश किया गया।
* [Issue #2010](https://github.com/manticoresoftware/manticoresearch/issues/2010) बडी से क्वेरीज़ का लॉगिंग असमर्थित किया गया जब तक कि `log_level=debug` सेट न हो।
* [Issue #2035](https://github.com/manticoresoftware/manticoresearch/issues/2035) Linux Mint 21.3 का समर्थन।
* [Issue #2056](https://github.com/manticoresoftware/manticoresearch/issues/2056) Manticore Mysql 8.3+ के साथ नहीं बनाया जा सका।
* [Issue #2112](https://github.com/manticoresoftware/manticoresearch/issues/2112) वास्तविक समय तालिका खंडों के लिए `DEBUG DEDUP` कमांड जो डुप्लिकेट प्रविष्टियों के अनुभव कर सकते हैं जब एक साधारण तालिका जो डुप्लिकेट है संलग्न करें।
* [Issue #212](https://github.com/manticoresoftware/manticoresearch-buddy/issues/212) [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) में समय जोड़ा गया।
* [Issue #218](https://github.com/manticoresoftware/manticoresearch-buddy/issues/218) `@timestamp` कॉलम को टाइमस्टैम्प के रूप में संभाला गया।
* [Issue #252](https://github.com/manticoresoftware/manticoresearch-buddy/issues/250) बडी प्लगइनों को सक्षम/असक्षम करने के लिए लॉजिक लागू किया गया।
* [Issue #254](https://github.com/manticoresoftware/manticoresearch-buddy/issues/254) हाल के CVE को ठीक करने वाले ताजा संस्करण में composer को अपडेट किया गया।
* [Issue #340](https://github.com/manticoresoftware/manticoresearch/issues/340) `RuntimeDirectory` से संबंधित Manticore systemd यूनिट में छोटी अनुकूलन।
* [Issue #51](https://github.com/manticoresoftware/executor/issues/51) rdkafka समर्थन जोड़ा गया और PHP 8.3.3 में अपडेट किया गया।
* [Issue #527](https://github.com/manticoresoftware/manticoresearch/issues/527) एक RT तालिका [संलग्न करने](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md#Attaching-one-table-to-another) का समर्थन। नई कमांड [ALTER TABLE ... RENAME](Updating_table_schema_and_settings.md#Renaming-a-real-time-table)।

### ब्रेकिंग परिवर्तन और हटाए गए
* ⚠️[Issue #1436](https://github.com/manticoresoftware/manticoresearch/issues/1436) IDF गणना समस्या को ठीक किया गया। `local_df` अब डिफ़ॉल्ट है। मास्टर-एजेंट खोज प्रोटोकॉल में सुधार किया गया (संस्करण अपडेट किया गया)। यदि आप एक वितरित वातावरण में कई उदाहरणों के साथ Manticore Search चला रहे हैं, तो सुनिश्चित करें कि पहले एजेंट को अपग्रेड करें, फिर मास्टर करें।
* ⚠️[Issue #1572](https://github.com/manticoresoftware/manticoresearch/issues/1572) वितरित तालिकाओं की पुनरावृत्ति जोड़ी गई और पुनरावृत्ति प्रोटोकॉल को अपडेट किया गया। यदि आप एक पुनरावृत्ति क्लस्टर चला रहे हैं, तो आपको:
  - पहले, सभी नोड्स को साफ़ तरीके से रोकें
  - फिर, अंतिम बार बंद किए गए नोड को `--new-cluster` के साथ शुरू करें, Linux में `manticore_new_cluster` उपकरण का उपयोग करके।
  - अधिक विवरण के लिए [क्लस्टर को पुनः आरंभ करना](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) के बारे में पढ़ें।
* ⚠️[Issue #1763](https://github.com/manticoresoftware/manticoresearch/pull/1763) HTTP API एंडपॉइंट उपनाम `/json/*` को हटा दिया गया है।
* ⚠️[Issue #1982](https://github.com/manticoresoftware/manticoresearch/issues/1982) JSON में [प्रोफ़ाइल](Node_info_and_management/Profiling/Query_profile.md#Query-profile) को [योजना](Node_info_and_management/Profiling/Query_plan.md#Query-plan) में बदल दिया गया, JSON के लिए क्वेरी प्रोफाइलिंग जोड़ी गई।
* ⚠️[Commit e235](https://github.com/manticoresoftware/manticoresearch-backup/commit/e23585a) manticore-backup अब `plugin_dir` का बैकअप नहीं लेता है।
* ⚠️[Issue #171](https://github.com/manticoresoftware/manticoresearch-buddy/issues/155) प्रदर्शन और स्थिरता सुधारने के लिए बडी को Swoole में माइग्रेट किया गया। नए संस्करण में स्विच करते समय, सुनिश्चित करें कि सभी Manticore पैकेज अपडेट किए गए हैं।
* ⚠️[Issue #196](https://github.com/manticoresoftware/manticoresearch-buddy/pull/205) सभी मुख्य प्लगइनों को बडी में मर्ज किया गया और मुख्य लॉजिक को बदल दिया गया।
* ⚠️[Issue #2107](https://github.com/manticoresoftware/manticoresearch/issues/2107) `/search` प्रतिक्रियाओं में दस्तावेज़ ID को संख्याओं के रूप में मानना।
* ⚠️[Issue #38](https://github.com/manticoresoftware/executor/issues/38) Swoole जोड़ा गया, ZTS को अक्षम किया गया, और समानांतर एक्सटेंशन हटा दिया गया।
* ⚠️[Issue #1929](https://github.com/manticoresoftware/manticoresearch/issues/1929) कुछ मामलों में `charset_table` में ओवरराइडिंग काम नहीं कर रहा था।

### पुनरावृत्ति से संबंधित परिवर्तन
* [Commit 3376](https://github.com/manticoresoftware/manticoresearch/commit/3376351f73db3ff0cb6cb708a304a7bd83e69cfb) बड़े फ़ाइलों के SST पर पुनरावृत्ति त्रुटि ठीक की गई।
* [Commit 6d36](https://github.com/manticoresoftware/manticoresearch/commit/6d36c68fb7ce3cb274b467aac435788905084952) प्रतिकृति आदेशों के लिए पुनः प्रयास तंत्र जोड़ा गया; पैकेट हानि के साथ व्यस्त नेटवर्क पर प्रतिकृति जोड़ों में विफलता ठीक की गई।
* [Commit 842e](https://github.com/manticoresoftware/manticoresearch/commit/842e27e71790833ab4125d49103d2933d5df60c3) प्रतिकृति में FATAL संदेश को WARNING संदेश में बदला गया।
* [Commit 8c32](https://github.com/manticoresoftware/manticoresearch/commit/8c32bce7db39d8ed3b1a06aaa1d2c3ec372084f0) तालिकाओं के बिना या खाली तालिकाओं के लिए `gcache.page_size` की गणना ठीक की गई; गैलेरा विकल्पों को सहेजने और लोड करने में भी सुधार किया गया।
* [Commit a2af](https://github.com/manticoresoftware/manticoresearch/commit/a2af06ca36abcbf893e43c8803853130b397c54b) क्लस्टर में शामिल होने वाले नोड पर अद्यतन नोड्स प्रतिकृति आदेश को छोड़ने की क्षमता जोड़ी गई।
* [Commit c054](https://github.com/manticoresoftware/manticoresearch/commit/c0541e6892f5116b1f957b40060ba8ecd01a5afb) दस्तावेज़ों को प्रतिस्थापित करने के मुकाबले ब्लॉब विशेषताओं को अद्यतन करने के दौरान प्रतिकृति पर डेडलॉक ठीक किया गया।
* [Commit e80d](https://github.com/manticoresoftware/manticoresearch/commit/e80d505b963999016033613c40c1cf3151d648e7) प्रतिकृति के दौरान नेटवर्क को नियंत्रित करने के लिए [replication_connect_timeout](Server_settings/Searchd.md#replication_connect_timeout), [replication_query_timeout](Server_settings/Searchd.md#replication_query_timeout), [replication_retry_delay](Server_settings/Searchd.md#replication_retry_delay), [replication_retry_count](Server_settings/Searchd.md#replication_retry_count) सर्चडी कॉन्फ़िग विकल्प जोड़े गए, जो `searchd.agent_*` की तरह हैं लेकिन अलग डिफ़ॉल्ट्स के साथ।
* [Issue #1356](https://github.com/manticoresoftware/manticoresearch/issues/1356) कुछ नोड्स छूटने और इन नोड्स का नाम संसाधन करने में विफल रहने के बाद प्रतिकृति नोड्स के पुनः प्रयास को ठीक किया गया।
* [Issue #1445](https://github.com/manticoresoftware/manticoresearch/issues/1445) `show variables` पर प्रतिकृति लॉग की विवरणता स्तर ठीक की गई।
* [Issue #1482](https://github.com/manticoresoftware/manticoresearch/issues/1482) क्यूबेरनेट्स में पुनः आरंभ किए गए पॉड में क्लस्टर से कनेक्ट करने वाले जॉइनर नोड के लिए एक प्रतिकृति समस्या ठीक की गई।
* [Issue #1962](https://github.com/manticoresoftware/manticoresearch/issues/1962) अमान्य नोड नाम के साथ खाली क्लस्टर पर परिवर्तन के लिए प्रतिकृति के लिए लंबा इंतजार ठीक किया गया।

### बग सुधार
* [Commit 8a48](https://github.com/manticoresoftware/manticoresearch/commit/8a486c7ba1a293879cdecc41fb52eb7ab0cbf832) `count distinct` में अप्रयुक्त मेल की सफाई ठीक की गई जो दुर्घटना का कारण बन सकती थी।
* [Issue #1569](https://github.com/manticoresoftware/manticoresearch/issues/1569) द्विआधारी लॉग अब लेन-देन की बारीकी के साथ लिखा जाता है।
* [Issue #2089](https://github.com/manticoresoftware/manticoresearch/issues/2089) 64-बिट IDs से संबंधित एक बग को ठीक किया गया जो MySQL के माध्यम से सम्मिलित करते समय "Malformed packet" त्रुटि का कारण बन सकता था, जिससे [संपूर्ण तालिकाओं और डुप्लिकेट IDs](https://github.com/manticoresoftware/manticoresearch/issues/2090) में भ्रष्टाचार हो सकता है।
* [Issue #2160](https://github.com/manticoresoftware/manticoresearch/issues/2160) स्थानीय समय क्षेत्र के बजाय UTC के रूप में तारीखें सम्मिलित किए जाने की समस्या ठीक की गई।
* [Issue #2177](https://github.com/manticoresoftware/manticoresearch/issues/2177) एक वास्तविक समय तालिका में खोज करते समय हुई दुर्घटना ठीक की गई जिसमें गैर-खाली `index_token_filter` था।
* [Issue #2209](https://github.com/manticoresoftware/manticoresearch/issues/2209) क्रैश और गलत क्वेरी परिणामों को ठीक करने के लिए RT कॉलम भंडारण में डुप्लिकेट फ़िल्टरिंग को बदल दिया गया।
* [Commit 001d](https://github.com/manticoresoftware/manticoresearch/commit/001d82904e4b887c49387308291aeefb07c0fbc4) सम्मिलित क्षेत्र को संसाधित करने के बाद मेमोरी को भ्रष्ट करने वाले HTML स्टिपर को ठीक किया गया।
* [Commit 00eb](https://github.com/manticoresoftware/manticoresearch/commit/00eb2ebf) गलत संचार मुद्दों से बचने के लिए फ्लश के बाद स्ट्रीम को रिवाइंड करने से बचा गया।
* [Commit 0553](https://github.com/manticoresoftware/manticoresearch/commit/05534114acf08c70ffcc41a23d558dec09a34373) यदि प्रीरीड शुरू नहीं हुआ है तो समाप्त होने की प्रतीक्षा न करें।
* [Commit 055a](https://github.com/manticoresoftware/manticoresearch/commit/055aac09f877570cd29b6594cb283182eba7f73b) सर्चडी लॉग में बड़ी बडी आउटपुट स्ट्रिंग को कई पंक्तियों में विभाजित करने के लिए ठीक किया गया।
* [Commit 0a88](https://github.com/manticoresoftware/manticoresearch/commit/0a88056307f82c50fcdc458ad6dbc307a00e3f92) विफल हेडर `debugv` विवरणता स्तर के बारे में MySQL इंटरफ़ेस चेतावनी को स्थानांतरित किया गया।
* [Commit 150a](https://github.com/manticoresoftware/manticoresearch/commit/150a20286b7d98d24b844ae577d7fda4a444798d) कई क्लस्टर प्रबंधन संचालन पर दौड़ की स्थिति ठीक की गई; एक ही नाम या पथ वाले कई क्लस्टर बनाने पर प्रतिबंध लगाया गया।
* [Commit 2e40](https://github.com/manticoresoftware/manticoresearch/commit/2e405c1c9ebac6f46206e3df17ac1c19129c9aa3) पूर्णपाठ क्वेरी में अप्रत्याशित कटौती ठीक की गई; MatchExtended को टेम्पलेट भागD में विभाजित किया गया।
* [Commit 75f5](https://github.com/manticoresoftware/manticoresearch/commit/75f51421542a2a0658f9e2f9f31048c8c5224479) डेमॉन में तालिका को अनुक्रमण बनाते समय `index_exact_words` में विसंगति को ठीक किया गया।
* [Commit 7643](https://github.com/manticoresoftware/manticoresearch/commit/7643976bea142e08535d701000eff0d09a99fda2) अमान्य क्लस्टर विलोपन के लिए खोए हुए त्रुटि संदेश को ठीक किया गया।
* [Commit 7a03](https://github.com/manticoresoftware/manticoresearch/commit/7a034ac990d5ec9344320c05a6a2315583b9b858) CBO बनाम कतार संघ; CBO बनाम RT प्रमुख शार्डिंग ठीक किया गया।
* [Commit 7b4e](https://github.com/manticoresoftware/manticoresearch/commit/7b4ea95e) जब द्वितीयक अनुक्रमण (SI) पुस्तकालय और कॉन्फ़िगरेशन में पैरामीटर के बिना शुरू किया गया, तो भ्रामक चेतावनी संदेश 'WARNING: secondary_indexes set but failed to initialize secondary library' दिया गया।
* [Commit 8496](https://github.com/manticoresoftware/manticoresearch/commit/8496317f099fade8d2db463fb88483e7fcdfe85b) क्वोरम में हिट क्रमबद्धता को ठीक किया।
* [Commit 8973](https://github.com/manticoresoftware/manticoresearch-buddy/commit/8973ad3) ModifyTable प्लगइन में अपर केस विकल्पों के साथ समस्या को हल किया।
* [Commit 9935](https://github.com/manticoresoftware/manticoresearch-buddy/commit/99350d4c184379b19f3efa9751d9b5be5e6edfb5) खाली जेसन मानों (NULL के रूप में दर्शित) के साथ डंप से बहाली को हल किया।
* [Commit a28f](https://github.com/manticoresoftware/manticoresearch/commit/a28f6e087b3d957e48816070be17d0823904936a) pcon का उपयोग करके SST प्राप्त करते समय जॉइनर नोड पर SST टाइमआउट को हल किया।
* [Commit b5a5](https://github.com/manticoresoftware/manticoresearch/commit/b5a55cc26debcddcda5d9caef8366a707e0dd763) उपनामित स्ट्रिंग एट्रिब्यूट चुनने पर क्रेश को हल किया।
* [Commit c556](https://github.com/manticoresoftware/manticoresearch/commit/c556e7ed8b6da8eac8ef094650df5f0a8977916b) `morphology_skip_fields` फ़ील्ड के साथ पूर्ण-पाठ क्वेरी में टर्म को `=term` में परिवर्तित करने के लिए क्वेरी ट्रांसफॉर्म जोड़ा।
* [Commit cdc3](https://github.com/manticoresoftware/manticoresearch/commit/cdc38085145cd1025b96fcac2a46188a0425d9e1) गायब कॉन्फ़िग की (skiplist_cache_size) जोड़ी।
* [Commit cf6e](https://github.com/manticoresoftware/manticoresearch/commit/cf6ec3bfdc3f0990fb31ff53822b2331a6667727) बड़े जटिल क्वेरी के साथ एक्सप्रेशन रेंकर पर क्रेश को हल किया।
* [Commit e513](https://github.com/manticoresoftware/manticoresearch/commit/e5139ba75eef7223df375ae3cc3f138fa7d925f1) पूर्णपाठ CBO बनाम अमान्य इंडेक्स संकेतों को हल किया।
* [Commit eb05](https://github.com/manticoresoftware/manticoresearch/commit/eb0514b3a6f57f278252b0724eebd112ec116e0f) तेज़ शटडाउन के लिए शटडाउन पर प्रीरीड को इंटरप्ट किया।
* [Commit f945](https://github.com/manticoresoftware/manticoresearch/commit/f94555a293cfa51004ff81135b8bd8500b955115) जटिल क्वेरी के मामले में क्रेश से बचने के लिए पूर्णपाठ क्वेरी के लिए स्टैक गणना को बदला।
* [Issue #1262](https://github.com/manticoresoftware/manticoresearch/issues/1262) समान नाम वाले कई कॉलम के साथ SQL स्रोत को अनुक्रमित करते समय अनुक्रमणकारी के क्रेश को हल किया।
* [Issue #1273](https://github.com/manticoresoftware/manticoresearch/issues/1273) अस्तित्व न रखने वाले sysvars के लिए <empty> के बजाय 0 लौटाना।
* [Issue #1289](https://github.com/manticoresoftware/manticoresearch/issues/1289) RT तालिका के बाह्य फ़ाइलों की जांच करते समय indextool त्रुटि को हल किया।
* [Issue #1335](https://github.com/manticoresoftware/manticoresearch/issues/1335) वाक्यांश के अंदर मल्टी वर्डफॉर्म के कारण क्वेरी पार्स त्रुटि को हल किया।
* [Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) पुराने बिनलॉग संस्करणों के साथ खाली बिनलॉग फ़ाइलों के पुनरावृत्ति को जोड़ा।
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) अंतिम खाली बिनलॉग फ़ाइल को हटाने को हल किया।
* [Issue #1372](https://github.com/manticoresoftware/manticoresearch/issues/1372) परिवर्तनों के बाद गलत सापेक्ष पथ (डेमोन के प्रारंभ निर्देशिका से निरूपित) को हल किया `data_dir` वर्तमान कार्य निर्देशिका पर प्रभाव डालता है जब डेमोन प्रारंभ होता है।
* [Issue #1393](https://github.com/manticoresoftware/manticoresearch/issues/1393) hn_small में धीमी समय घटित करना: डेमोन स्टार्टअप पर fetch/cache cpu जानकारी।
* [Issue #1395](https://github.com/manticoresoftware/manticoresearch/issues/1395) अनुक्रमणिका लोड के दौरान गायब बाह्य फ़ाइल के संबंध में चेतावनी को हल किया।
* [Issue #1402](https://github.com/manticoresoftware/manticoresearch/issues/1402) डेटा ptr एट्रिब्यूट्स से मुक्त पर वैश्विक समूहक में क्रेश को हल किया।
* [Issue #1403](https://github.com/manticoresoftware/manticoresearch/issues/1403) _ADDITIONAL_SEARCHD_PARAMS काम नहीं कर रहा है।
* [Issue #1427](https://github.com/manticoresoftware/manticoresearch/issues/1427) तालिका के अनुसार `agent_query_timeout` को डिफ़ॉल्ट क्वेरी विकल्प `agent_query_timeout` द्वारा प्रतिस्थापित किया जा रहा है।
* [Issue #1444](https://github.com/manticoresoftware/manticoresearch/issues/1444) `packedfactors()` के साथ प्रत्येक मैच के लिए कई मानों का उपयोग करते समय समूहक और रेंकर में क्रेश को हल किया।
* [Issue #1458](https://github.com/manticoresoftware/manticoresearch/issues/1458) लगातार अनुक्रमणिका अपडेट के दौरान मैन्टिकोर क्रेश करता है।
* [Issue #1481](https://github.com/manticoresoftware/manticoresearch/issues/1481) पार्स त्रुटि के बाद पार्स की गई क्वेरी की सफाई पर क्रेश को हल किया।
* [Issue #1484](https://github.com/manticoresoftware/manticoresearch/issues/1484) HTTP JSON अनुरोधों कोbuddy  पर राउट नहीं होने को हल किया।
* [Issue #1499](https://github.com/manticoresoftware/manticoresearch/issues/1499) JSON एट्रिब्यूट रूट मान एक एरे नहीं हो सकता। हल किया।
* [Issue #1507](https://github.com/manticoresoftware/manticoresearch/issues/1507) लेनदेन के भीतर तालिका के पुनर्निर्माण पर क्रेश को हल किया।
* [Issue #1515](https://github.com/manticoresoftware/manticoresearch/issues/1515) RU लेमास के संक्षिप्त रूपों के विस्तार को हल किया।
* [Issue #1579](https://github.com/manticoresoftware/manticoresearch/issues/1579) [date_format](Functions/Date_and_time_functions.md#DATE_FORMAT()) अभिव्यक्ति में JSON और STRING एट्रिब्यूट्स का उपयोग हल किया।
* [Issue #1580](https://github.com/manticoresoftware/manticoresearch/issues/1580) JSON फ़ील्ड के लिए कई उपनामों के लिए समूहक को हल किया।
* [Issue #1594](https://github.com/manticoresoftware/manticoresearch/issues/1594) dev में wrong total_related: implicit cutoff बनाम सीमा को हल किया; JSON क्वेरी में बेहतर फुलस्कैन पहचान को जोड़ा।
* [Issue #1603](https://github.com/manticoresoftware/manticoresearch/issues/1603) सभी दिनांक अभिव्यक्ति में JSON और STRING एट्रिब्यूट्स का उपयोग हल किया।
* [Issue #1609](https://github.com/manticoresoftware/manticoresearch/issues/1609) LEVENSHTEIN() का उपयोग करते समय क्रेश।
* [Issue #1612](https://github.com/manticoresoftware/manticoresearch/issues/1612) हाइलाइट के साथ खोज क्वेरी पार्स त्रुटि के बाद मेमोरी भ्रष्टाचार को हल किया।
* [Issue #1614](https://github.com/manticoresoftware/manticoresearch/issues/1614) `min_prefix_len` / `min_infix_len` से कम शब्दों के लिए वाइल्डकार्ड विस्तार को निष्क्रिय कर दिया।
* [Issue #1617](https://github.com/manticoresoftware/manticoresearch/issues/1617) यदि बडी अनुरोध को सफलतापूर्वक संभालती है तो एक त्रुटि को लॉग न करने के लिए व्यवहार को संशोधित किया।
* [Issue #1635](https://github.com/manticoresoftware/manticoresearch/issues/1635) खोज क्वेरी के मेटा में सेट की गई सीमा वाले प्रश्नों के लिए कुल ठीक किया गया।
* [Issue #1640](https://github.com/manticoresoftware/manticoresearch/issues/1640) सामान्य मोड में JSON के माध्यम से बड़े अक्षर वाले टेबल का उपयोग करना असंभव।
* [Issue #1643](https://github.com/manticoresoftware/manticoresearch/issues/1643) डिफ़ॉल्ट `SPH_EXTNODE_STACK_SIZE` मान प्रदान किया गया।
* [Issue #1646](https://github.com/manticoresoftware/manticoresearch/issues/1646) MVA एट्रिब्यूट पर ALL/ANY के साथ नकारात्मक फ़िल्टर का SphinxQL लॉग ठीक किया गया।
* [Issue #1660](https://github.com/manticoresoftware/manticoresearch/issues/1660) अन्य इंडेक्स से docid किललिस्ट के आवेदन को ठीक किया गया। ❤️ धन्यवाद, [@raxoft](https://github.com/raxoft) PR के लिए।
* [Issue #1668](https://github.com/manticoresoftware/manticoresearch/issues/1668) कच्चे इंडेक्स पूर्ण स्कैन (किसी भी इंडेक्स इटरेटर के बिना) पर जल्दी निकासी के कारण चूके गए मेल को ठीक किया गया; सामान्य पंक्ति इटरेटर से कटऑफ हटा दिया गया।
* [Issue #1671](https://github.com/manticoresoftware/manticoresearch/issues/1671) एजेंट और स्थानीय तालिकाओं के साथ वितरित तालिका को क्वेरी करते समय `FACET` त्रुटि ठीक की गई।
* [Issue #1690](https://github.com/manticoresoftware/manticoresearch/issues/1690) बड़े मानों के लिए ऐतिहासिक अनुमान पर क्रैश ठीक किया गया।
* [Issue #1692](https://github.com/manticoresoftware/manticoresearch/issues/1692) टेबल tbl में कॉलम col uint जोड़ने पर क्रैश।
* [Issue #1710](https://github.com/manticoresoftware/manticoresearch/issues/1710) स्थिति `WHERE json.array IN (<value>)` के लिए खाली परिणाम।
* [Issue #172](https://github.com/manticoresoftware/manticoresearch-buddy/issues/172) `/cli` पर अनुरोध भेजने के दौरान TableFormatter के साथ एक समस्या ठीक की गई।
* [Issue #1742](https://github.com/manticoresoftware/manticoresearch/issues/1742) शब्दरूप फ़ाइल के गायब होने की स्थिति में `CREATE TABLE` विफल नहीं हो रहा था।
* [Issue #1762](https://github.com/manticoresoftware/manticoresearch/issues/1762) RT तालिकाओं में विशेषताओं का क्रम अब कॉन्फ़िगरेशन क्रम का पालन करता है।
* [Issue #1765](https://github.com/manticoresoftware/manticoresearch/issues/1765) 'should' स्थिति के साथ HTTP बूल प्रश्न गलत परिणाम लौटाता है।
* [Issue #1769](https://github.com/manticoresoftware/manticoresearch/issues/1769) स्ट्रिंग विशेषताओं द्वारा क्रमबद्ध करना `SPH_SORT_ATTR_DESC` और `SPH_SORT_ATTR_ASC` के साथ काम नहीं करता।
* [Issue #177](https://github.com/manticoresoftware/manticoresearch-buddy/issues/177) बडी के लिए कर्ल अनुरोधों के लिए `Expect: 100-continue` HTTP हेडर को निष्क्रिय कर दिया गया।
* [Issue #1791](https://github.com/manticoresoftware/manticoresearch/issues/1791) समूह द्वारा उपनाम के कारण क्रैश।
* [Issue #1792](https://github.com/manticoresoftware/manticoresearch/issues/1792) SQL मेटा सारांश विंडोज़ पर गलत समय दिखाता है।
* [Issue #1794](https://github.com/manticoresoftware/manticoresearch/issues/1794) JSON क्वेरियों के साथ एक ही अवधि के प्रदर्शन में गिरावट ठीक की गई।
* [Issue #1798](https://github.com/manticoresoftware/manticoresearch/issues/1798) असंगत फ़िल्टर `/search` पर एक त्रुटि नहीं उठाते थे।
* [Issue #1802](https://github.com/manticoresoftware/manticoresearch/issues/1802) `ALTER CLUSTER ADD` और `JOIN CLUSTER` संचालन को एक-दूसरे का इंतज़ार करने के लिए ठीक किया गया, जो एक प्रतिस्पर्धा की स्थिति को रोकता है जहाँ `ALTER` क्लस्टर में एक तालिका जोड़ता है जबकि दाता तालिकाएं जोड़ने वाले नोड को भेजता है।
* [Issue #1811](https://github.com/manticoresoftware/manticoresearch/issues/1811) `/pq/{table}/*` अनुरोधों की गलत हैंडलिंग।
* [Issue #1816](https://github.com/manticoresoftware/manticoresearch/issues/1816) कुछ मामलों में `UNFREEZE` काम नहीं कर रहा था।
* [Issue #183](https://github.com/manticoresoftware/manticoresearch-buddy/pull/183) कुछ मामलों में MVA पुनर्स्थापन में एक समस्या ठीक की गई।
* [Issue #1849](https://github.com/manticoresoftware/manticoresearch/issues/1849) MCL के साथ उपयोग करने पर शटडाउन पर indextool क्रैश ठीक किया गया।
* [Issue #1866](https://github.com/manticoresoftware/manticoresearch/issues/1866) `/cli_json` अनुरोधों के लिए अनावश्यक यूआरएल डिकोडिंग ठीक की गई।
* [Issue #1872](https://github.com/manticoresoftware/manticoresearch/issues/1872) डेमॉन प्रारंभ पर plugin_dir सेट लॉजिक्स में परिवर्तन।
* [Issue #1874](https://github.com/manticoresoftware/manticoresearch/issues/1874) टेबल में परिवर्तन करने पर ... अपवाद विफल होते हैं।
* [Issue #1891](https://github.com/manticoresoftware/manticoresearch/issues/1891#issuecomment-2051941200) डेटा डालते समय माण्टिकोर क्रैश हो जाता है `signal 11` के साथ।
* [Issue #1920](https://github.com/manticoresoftware/manticoresearch/issues/1920) [low_priority](Searching/Options.md#low_priority) के लिए थ्रॉटलिंग कम किया गया।
* [Issue #1924](https://github.com/manticoresoftware/manticoresearch/issues/1924) Mysqldump + mysql पुनर्स्थापन बग।
* [Issue #1951](https://github.com/manticoresoftware/manticoresearch/issues/1951) स्थानीय तालिका के गायब होने या गलत एजेंट विवरण के मामले में वितरित तालिका के गलत निर्माण को ठीक किया गया; अब एक त्रुटि संदेश वापस करता है।
* [Issue #1972](https://github.com/manticoresoftware/manticoresearch/issues/1972) फ्रीज / अनफ्रीज़ समस्याओं से बचने के लिए `FREEZE` काउंटर लागू किया गया।
* [Issue #1980](https://github.com/manticoresoftware/manticoresearch/issues/1980) OR नोड्स में क्वेरी टाइमआउट का पालन करें। पहले `max_query_time` कुछ मामलों में काम नहीं कर सकता था।
* [Issue #1986](https://github.com/manticoresoftware/manticoresearch/issues/1986) नए को वर्तमान [manticore.json] पर नाम बदलने में विफल।
* [Issue #1988](https://github.com/manticoresoftware/manticoresearch/issues/1988) एक पूर्ण-पाठ क्वेरी `SecondaryIndex` CBO संकेत को अनदेखा कर सकती है।
* [Issue #1990](https://github.com/manticoresoftware/manticoresearch/issues/1990) अंतिम परिणाम सेट को कई डिस्क खंडों या RAM खंडों से कॉल कीवर्ड के लिए काटने के लिए `expansion_limit` ठीक किया गया।
* [Issue #1994](https://github.com/manticoresoftware/manticoresearch/issues/1994) गलत बाहरी फ़ाइलें।
* [Issue #2021](https://github.com/manticoresoftware/manticoresearch/issues/2021) Manticore को बंद करने के बाद कुछ माण्टिकोर-निष्पादक प्रक्रियाएँ चलती रह सकती हैं।
* [Issue #2029](https://github.com/manticoresoftware/manticoresearch/issues/2029) लेवेनस्टाइन दूरी का उपयोग करते समय क्रैश।
* [Issue #2037](https://github.com/manticoresoftware/manticoresearch/issues/2037) खाली सूची पर कई अधिकतम ऑपरेटर चलाने के बाद त्रुटि मिली।
* [Issue #2052](https://github.com/manticoresoftware/manticoresearch/issues/2052) JSON.field के साथ मल्टी-ग्रुप पर क्रैश।
* [Issue #2067](https://github.com/manticoresoftware/manticoresearch/issues/2067) _update के लिए गलत अनुरोध पर मंटिकोरे क्रैश हो रहा था।
* [Issue #2069](https://github.com/manticoresoftware/manticoresearch/issues/2069) JSON इंटरफेस में बंद रेंज के लिए स्ट्रिंग फ़िल्टर तुलना के साथ एक मुद्दा ठीक किया गया।
* [Issue #2082](https://github.com/manticoresoftware/manticoresearch/issues/2082) `alter` विफल हो गया जब data_dir पथ एक सिम्लिंक पर स्थित था।
* [Issue #2102](https://github.com/manticoresoftware/manticoresearch/pull/2102) सुनिश्चित करने के लिए mysqldump में SELECT प्रश्नों के विशेष संचालन में सुधार किया गया कि परिणामी INSERT विवरण मंटिकोरे के साथ संगत हैं।
* [Issue #2103](https://github.com/manticoresoftware/manticoresearch/issues/2103) थाई वर्ण गलत वर्ण सेट में थे।
* [Issue #2124](https://github.com/manticoresoftware/manticoresearch/issues/2124) अगर मैं एक आरक्षित शब्द के साथ SQL का उपयोग करता हूं तो क्रैश।
* [Issue #2154](https://github.com/manticoresoftware/manticoresearch/issues/2154) शब्दफार्म वाली तालिकाएं आयात नहीं की जा सकीं।
* [Issue #2176](https://github.com/manticoresoftware/manticoresearch/issues/2176) जब इंजन पैरामीटर 'columnar' पर सेट था और JSON के माध्यम से डुप्लिकेट आईडी जोड़े गए थे, तो एक क्रैश को ठीक किया गया।
* [Issue #223](https://github.com/manticoresoftware/manticoresearch-buddy/issues/223) जब स्कीमा के बिना और कॉलम नामों के बिना एक दस्तावेज़ जोड़ने की कोशिश की गई तो उपयुक्त त्रुटि।
* [Issue #239](https://github.com/manticoresoftware/manticoresearch-buddy/issues/239) ऑटो-स्कीमा मल्टी-लाइन इनसर्ट विफल हो सकता है।
* [Issue #399](https://github.com/manticoresoftware/manticoresearch/issues/399) यदि डेटा स्रोत पर एक id विशेषता घोषित की गई है तो अनुक्रमण पर एक त्रुटि संदेश जोड़ा गया।
* [Issue #59](https://github.com/manticoresoftware/manticoresearch-helm/issues/59) मंटिकोरे क्लस्टर टूटना।
* [Issue #68](https://github.com/manticoresoftware/manticoresearch-helm/issues/68) optimize.php क्रैश हो जाएगा यदि पर्टोकल टेबल उपस्थित हो।
* [Issue #77](https://github.com/manticoresoftware/manticoresearch-helm/issues/77) क्यूबेरनेट्स पर तैनात करते समय त्रुटियों को ठीक किया गया।
* [Issue #274](https://github.com/manticoresoftware/manticoresearch-buddy/issues/274) बडी पर समकालीन अनुरोधों की गलत प्रोसेसिंग को ठीक किया गया।

### मंटिकोरे-बैकअप के साथ संबंधित
* [Issue #97](https://github.com/manticoresoftware/manticoresearch-backup/pull/97) उपलब्ध होने पर डिफ़ॉल्ट के रूप में VIP HTTP पोर्ट सेट करें।
विभिन्न सुधार: संस्करण जांच और स्ट्रीमिंग ZSTD डीकॉम्प्रेसन में सुधार; पुनर्स्थापना के दौरान संस्करण असमानताओं के लिए उपयोगकर्ता संकेत जोड़े गए; पुनर्स्थापना पर विभिन्न संस्करणों के लिए गलत संकेत व्यवहार को ठीक किया गया; काम करने वाली स्मृति में नहीं, बल्कि सीधे स्ट्रीम से पढ़ने के लिए डीकॉम्प्रेसन लॉजिक को बढ़ाया; `--force` ध्वज जोड़ा गया।
* [Commit 3b35](https://github.com/manticoresoftware/manticoresearch-backup/commit/3b357ea) इस चरण पर मुद्दों की पहचान करने के लिए मंटिकोरे खोज प्रारंभ के बाद बैकअप संस्करण प्रदर्शन जोड़ा गया।
* [Commit ad2e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ad2e5da) डेमन से असफल कनेक्शन के लिए त्रुटि संदेश अपडेट किया गया।
* [Commit ce5e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ce5ea85) सापेक्ष में परिवर्तित करने के लिए अनुक्रमणिका वास्तविक रूट बैकअप पथों के साथ मुद्दे को ठीक किया गया और पुनर्स्थापन के लिए लिखने योग्य जांच को हटा दिया गया ताकि विभिन्न पथों से पुनर्स्थापना सक्षम हो सके।
* [Commit db7e](https://github.com/manticoresoftware/manticoresearch-backup/commit/db7e2b9) विभिन्न परिस्थितियों के बीच संगतता सुनिश्चित करने के लिए फ़ाइल इटरेटर में सॉर्टिंग जोड़ी गई।
* [Issue #106](https://github.com/manticoresoftware/manticoresearch-backup/issues/105) कई कॉन्फ़िगरेशनों का बैकअप और पुनर्स्थापना।
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) RHEL पर स्थापना के बाद फ़ाइलों में असामान्य उपयोगकर्ता अनुमतियों से रोकने के लिए defattr जोड़ा गया।
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) यह सुनिश्चित करने के लिए अतिरिक्त chown जोड़ा गया कि फ़ाइलें उबंटू में रूट उपयोगकर्ता के लिए डिफ़ॉल्ट हों।

### MCL (कॉलम, द्वितीयक, KNN पुस्तकालयों) के साथ संबंधित
* [Commit f104](https://github.com/manticoresoftware/columnar/commit/f104a4709bbb144140d09a09bc0fc1db1895e331) वेक्टर खोज समर्थन।
* [Commit 2169](https://github.com/manticoresoftware/columnar/commit/216953beef6676f7e34c9f6f1a30c5980d746710) द्वितीयक सूचकांक निर्माण की बाधित सेटअप के दौरान अस्थायी फ़ाइलों की सफाई को ठीक किया। यह उस समस्या को हल करता है जहां डेमन `tmp.spidx` फ़ाइलें बनाते समय खुले फ़ाइलों की सीमा से अधिक हो जाता है।
* [Commit 709b](https://github.com/manticoresoftware/columnar/commit/709b9acaaac97d9a1ca8796892f9ad432021c785) कॉलम और SI के लिए अलग स्ट्रीमव्बाइट लाइब्रेरी का उपयोग करें।
* [Commit 1c26](https://github.com/manticoresoftware/manticoresearch/commit/1c2623763ced14bb94ad624dde9614a938408d89) एक चेतावनी जोड़ी गई कि कॉलम स्टोरेज json attrs का समर्थन नहीं करता है।
* [Commit 3acd](https://github.com/manticoresoftware/columnar/commit/3acd6714ffce103447d7931dfc75bb1a7449f340) SI में डेटा अनपैकिंग को ठीक किया गया।
* [Commit 574c](https://github.com/manticoresoftware/manticoresearch/commit/574c023152564fa72092c18ebd1f594abfd4b6d7) मिश्रित पंक्ति और कॉलम भंडारण के साथ एक डिस्क टुकड़े को सहेजते समय क्रैश को ठीक किया गया।
* [Commit e87f](https://github.com/manticoresoftware/columnar/commit/e87f088b458d4c5b1b6159fc72ed0d57b786891a) SI इटरेटर को ठीक किया गया जो पहले से प्रोसेस किए गए ब्लॉक पर संकेत दिया गया था।
* [Issue #1474](https://github.com/manticoresoftware/manticoresearch/issues/1474) कॉलमर इंजन के साथ पंक्तिवार MVA कॉलम के लिए अपडेट टूट गया है।
* [Issue #1510](https://github.com/manticoresoftware/manticoresearch/issues/1510) `HAVING` में उपयोग किए गए कॉलमर विशेषता में समेकन करते समय क्रैश को ठीक किया गया।
* [Issue #1519](https://github.com/manticoresoftware/manticoresearch/issues/1519) `expr` रैंकिंग में क्रैश को ठीक किया जब स्तंभ विशेषता का उपयोग किया गया।

### डॉकर से संबंधित
* ❗[Issue #42](https://github.com/manticoresoftware/docker/issues/42) पर्यावरण चर के माध्यम से [साधारण अनुक्रमण](https://github.com/manticoresoftware/docker#building-plain-tables) का समर्थन।
* ❗[Issue #47](https://github.com/manticoresoftware/docker/issues/47) पर्यावरण वेरिएबल के माध्यम से कॉन्फ़िगरेशन की लचीलेपन में सुधार।
* [Issue #54](https://github.com/manticoresoftware/docker/pull/54) डॉकर के लिए [बैकअप और पुनर्स्थापना](https://github.com/manticoresoftware/docker#backup-and-restore) प्रक्रियाओं में सुधार किया।
* [Issue #77](https://github.com/manticoresoftware/docker/issues/76) पहले प्रारंभ पर केवल बैकअप पुनर्स्थापना को संभालने के लिए एंट्रीपॉइंट में सुधार किया।
* [Commit a27c](https://github.com/manticoresoftware/docker/commit/a27c048dd9e73d8d6bea8d9a4830cafb486b82c5) स्टैंडर्ड आउट पर क्वेरी लॉगिंग को ठीक किया।
* [Issue #38](https://github.com/manticoresoftware/docker/issues/38) यदि EXTRA सेट नहीं है तो BUDDY चेतावनियों को म्यूट करें।
* [Issue #71](https://github.com/manticoresoftware/docker/pull/71) `manticore.conf.sh` में होस्टनेम को ठीक किया।

# संस्करण 6.2.12
जारी किया गया: 23 अगस्त 2023

संस्करण 6.2.12 6.2 श्रृंखला को जारी रखता है और 6.2.0 के रिलीज के बाद पाए गए मुद्दों को संबोधित करता है।

### बग फिक्स
* ❗[Issue #1351](https://github.com/manticoresoftware/manticoresearch/issues/1351) "Manticore 6.2.0 Centos 7 पर systemctl के माध्यम से शुरू नहीं होता": Centos 7 के साथ बेहतर संगतता के लिए `TimeoutStartSec` को `infinity` से `0` में संशोधित किया।
* ❗[Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) "6.0.4 से 6.2.0 में अपग्रेड करने के बाद क्रैश": पुराने बिनलॉग संस्करणों से खाली बिनलॉग फ़ाइलों के लिए पुनरावृत्ति कार्यक्षमता जोड़ी गई।
* [PR #1334](https://github.com/manticoresoftware/manticoresearch/pull/1334) "searchdreplication.cpp में टाइपो ठीक करें": `searchdreplication.cpp` में एक टाइपो को सही किया: beggining -> beginning।
* [Issue #1337](https://github.com/manticoresoftware/manticoresearch/issues/1337) "Manticore 6.2.0 चेतावनी: conn (स्थानीय)(12), sock=8088: विफल MySQL हेडर पर बाहर निकलना, AsyncNetInputBuffer_c::AppendData: त्रुटि 11 ( स्रोत अस्थायी रूप से अनुपलब्ध) लौटें -1": हेडर के बारे में MySQL इंटरफेस चेतावनी की प्रगति स्तर को logdebugv तक कम किया।
* [Issue #1355](https://github.com/manticoresoftware/manticoresearch/issues/1355) "जब node_address को हल नहीं किया जा सकता है तो जॉइन क्लस्टर लटकता है": जब कुछ नोड्स अप्राप्य होते हैं, और उनका नाम निर्धारण विफल हो जाता है तो पुनरावृत्ति पुनः प्रयास में सुधार किया। यह कुबेरनेटीज़ और डॉकर नोड्स में पुनरावृत्ति से संबंधित मुद्दों को हल करना चाहिए। पुनरावृत्ति प्रारंभ विफलताओं के लिए त्रुटि संदेश को बढ़ाया गया और परीक्षण मॉडल 376 में अपडेट किए गए। इसके अतिरिक्त, नाम निर्धारण विफलताओं के लिए एक स्पष्ट त्रुटि संदेश प्रदान किया गया।
* [Issue #1361](https://github.com/manticoresoftware/manticoresearch/issues/1361) "charset non_cjk में 'Ø' के लिए कोई लोअर केस मैपिंग नहीं": 'Ø' चरित्र के लिए मैपिंग को समायोजित किया गया।
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) "searchd क्लीन स्टॉप के बाद binlog.meta और binlog.001 छोड़ता है": सुनिश्चित किया कि अंतिम खाली बिनलॉग फ़ाइल को ठीक से हटा दिया गया है।
* [Commit 0871](https://github.com/manticoresoftware/manticoresearch/commit/0871070845e93ad702ae0dfb89b35e5f24cd0851): विंडोज़ पर परमाणु कॉपी प्रतिबंधों से संबंधित `Thd_t` निर्माण समस्या को ठीक किया।
* [Commit 1cc0](https://github.com/manticoresoftware/manticoresearch/commit/1cc04bedc174737118fd5a3663e35f4e95bd4f8c): FT CBO बनाम `ColumnarScan` के साथ एक मुद्दे को संबोधित किया।
* [Commit c6bf](https://github.com/manticoresoftware/manticoresearch/commit/c6bfb30d53e3f4b67f17769667733677a661c95b): परीक्षण 376 में सुधार किया और परीक्षण में `AF_INET` त्रुटि के लिए एक प्रतिस्थापन जोड़ा।
* [Commit cbc3](https://github.com/manticoresoftware/manticoresearch/commit/cbc38705e0b4dcc9665e3e8cf64826290ebd11cf): दस्तावेज़ों को प्रतिस्थापित करने के मुकाबले बाइनरी विशेषताओं को अपडेट करते समय पुनरावृत्ति के दौरान एक डेडलॉक समस्या को हल किया। इसके अतिरिक्त, क्योंकि यह पहले से ही एक अधिक मौलिक स्तर पर लॉक किया गया है, इसलिए प्रतिबद्धता के दौरान इंडेक्स के rlock को हटा दिया गया।

### छोटे परिवर्तन
* [Commit 4f91](https://github.com/manticoresoftware/manticoresearch/commit/4f913a36e84e01f950b2797051222b0b1094d6fe) मैनुअल में `/bulk` समाप्तियों पर जानकारी अपडेट की।

### MCL
* [Manticore कॉलम लाइब्रेरी](https://github.com/manticoresoftware/columnar) v2.2.4 का समर्थन

# संस्करण 6.2.0
जारी किया गया: 4 अगस्त 2023

### प्रमुख परिवर्तन
* क्वेरी ऑप्टिमाइज़र को पूर्ण-पाठ अनुच्छेदों का समर्थन करने के लिए संवर्धित किया गया है, जिससे खोज दक्षता और प्रदर्शन में महत्वपूर्ण सुधार हुआ है।
* एकीकरण:
  - [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) - `mysqldump` का उपयोग करके [तार्किक बैकअप](../Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump) बनाने के लिए
  - [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) और [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) Manticore में संग्रहीत डेटा को दृश्य प्रस्तुत करने के लिए
  - [HeidiSQL](https://www.heidisql.com/) और [DBForge](https://www.devart.com/dbforge/) Manticore के साथ आसान विकास के लिए
* हमने [GitHub workflows](https://github.com/manticoresoftware/manticoresearch/actions) का उपयोग करना शुरू कर दिया है, जिससे योगदानकर्ताओं के लिए पैकेज तैयार करने के लिए मुख्य टीम द्वारा लागू किए गए समान निरंतर एकीकरण (CI) प्रक्रिया का उपयोग करना सरल हो गया है। सभी कार्यों को GitHub-होस्टेड रनर्स पर चलाया जा सकता है, जो आपके Manticore Search की शाखा में परिवर्तनों के सहज परीक्षण को सक्षम बनाता है।
* हमने जटिल परिदृश्यों का परीक्षण करने के लिए [CLT](https://github.com/manticoresoftware/clt) का उपयोग करना शुरू कर दिया है। उदाहरण के लिए, हम अब यह सुनिश्चित कर सकते हैं कि एक कमिट के बाद बनाया गया एक पैकेज सभी समर्थित लिनक्स ऑपरेटिंग सिस्टम पर ठीक से स्थापित किया जा सकता है। कमांड लाइन टेस्टिंग (CLT) इंटरैक्टिव मोड में परीक्षणों को रिकॉर्ड करने और उन्हें सहजता से पुनः चलाने के लिए एक उपयोगकर्ता-अनुकूल तरीका प्रदान करता है।
* गिनती विशेष ऑपरेशन में महत्वपूर्ण प्रदर्शन सुधार हैश तालिकाओं और हाइपरलॉगलॉग के संयोजन को अपनाकर किया गया।
* भौतिक CPU कोर की संख्या तक सीमित रहते हुए, द्वितीयक अनुक्रमांक वाले प्रश्नों के बहु-धागा निष्पादन को सक्षम किया गया। इससे प्रश्न निष्पादन की गति में काफी सुधार होना चाहिए।
* `pseudo_sharding` अब मुक्त धागों की संख्या तक सीमित किया गया है। इस अपडेट से थ्रूपुट प्रदर्शन में काफी सुधार होता है।
* उपयोगकर्ताओं के पास [डिफ़ॉल्ट विशेषता भंडारण इंजन](../Server_settings/Searchd.md#engine) को कॉन्फ़िगरेशन सेटिंग्स के माध्यम से निर्दिष्ट करने का विकल्प है, जो विशिष्ट कार्यभार आवश्यकताओं के अनुसार बेहतर अनुकूलन प्रदान करता है।
* [मैन्टिकोर कॉलम्नर लाइब्रेरी 2.2.0](https://github.com/manticoresoftware/columnar/) के लिए समर्थन, जिसमें कई बग फिक्स और [द्वितीयक अनुक्रमांक](../Server_settings/Searchd.md#secondary_indexes) में सुधार शामिल हैं।

### छोटे परिवर्तन
* [बडी #153](https://github.com/manticoresoftware/manticoresearch-buddy/issues/153): [/pq](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) HTTP एंडपॉइंट अब `/json/pq` HTTP एंडपॉइंट के लिए एक उपनाम के रूप में कार्य करता है।
* [कमिट 0bf1](https://github.com/manticoresoftware/manticoresearch/commit/0bf17d9e): हमने `upper()` और `lower()` के लिए मल्टी-बाइट संगतता सुनिश्चित की है।
* [कमिट 2bb9](https://github.com/manticoresoftware/manticoresearch/commit/2bb92765): `count(*)` प्रश्नों के लिए अनुक्रमांक को स्कैन करने के बजाय, एक पूर्व-गणना की गई मान अब लौटाई जाती है।
* [कमिट 3c84](https://github.com/manticoresoftware/manticoresearch/commit/3c84091f): अव्यक्त गणनाएँ करने और `@@sysvars` प्रदर्शित करने के लिए `SELECT` का उपयोग करना अब संभव है। पहले की तरह, आप केवल एक गणना तक सीमित नहीं हैं। इसलिए, प्रश्न जैसे `select user(), database(), @@version_comment, version(), 1+1 as a limit 10` सभी कॉलम लौटाएंगे। ध्यान दें कि वैकल्पिक 'सीमा' हमेशा无视 की जाएगी।
* [कमिट 6aca](https://github.com/manticoresoftware/manticoresearch/commit/6aca32f0): `CREATE DATABASE` स्टब प्रश्न को लागू किया गया।
* [कमिट 9dc1](https://github.com/manticoresoftware/manticoresearch/commit/9dc12334): जब `ALTER TABLE table REBUILD SECONDARY` चलाया जाता है, तो द्वितीयक अनुक्रमांक अब हमेशा पुनर्निर्मित होते हैं, भले ही विशेषताएँ अद्यतन न की गई हों।
* [कमिट 46ed](https://github.com/manticoresoftware/manticoresearch/commit/46edb089): पूर्व-गणना किए गए डेटा का उपयोग करने वाले सॉर्टर्स को CBO का उपयोग करने से पहले पहचाना जाता है ताकि अनावश्यक CBO गणनाओं से बचा जा सके।
* [कमिट 102a](https://github.com/manticoresoftware/manticoresearch/commit/102ac604): डेमन क्रैश से रोकने के लिए पूर्ण-पाठ अभिव्यक्ति स्टैक का नकली और उपयोग करने का कार्यान्वयन किया गया।
* [कमिट 979f](https://github.com/manticoresoftware/manticoresearch/commit/979fa27c): उस मेल मिलान क्लोनिंग कोड के लिए एक त्वरित कोड पथ जोड़ा गया है जो स्ट्रिंग/मवास/json विशेषताओं का उपयोग नहीं करता है।
* [कमिट a073](https://github.com/manticoresoftware/manticoresearch/commit/a0735ffe): `SELECT DATABASE()` कमांड के लिए समर्थन जोड़ा गया। हालाँकि, यह हमेशा `Manticore` लौटाएगा। यह जोड़ विभिन्न MySQL टूल के साथ एकीकरण के लिए महत्वपूर्ण है।
* [कमिट bc04](https://github.com/manticoresoftware/manticoresearch/commit/bc04908): [/cli](../Connecting_to_the_server/HTTP.md#/cli) एंडपॉइंट का उत्तर प्रारूप संशोधित किया गया है, और `/cli_json` एंडपॉइंट को पिछले `/cli` के रूप में कार्य करने के लिए जोड़ा गया है।
* [कमिट d70b](https://github.com/manticoresoftware/manticoresearch/commit/d70b0d58): `SET` कथन का उपयोग करके रनटाइम के दौरान `thread_stack` को अब संशोधित किया जा सकता है। दोनों सत्र-स्थानीय और डेमन-व्यापी स्वरूप उपलब्ध हैं। वर्तमान मानों को `show variables` के आउटपुट में एक्सेस किया जा सकता है।
* [कमिट d96e](https://github.com/manticoresoftware/manticoresearch/commit/d96ec6b9): कोड को CBO में एकीकृत किया गया है ताकि स्ट्रिंग विशेषताओं पर निस्पंदन निष्पादन की जटिलता का अधिक सटीक अनुमान लगाया जा सके।
* [कमिट e77d](https://github.com/manticoresoftware/manticoresearch/commit/e77dd72f5a04531c352fad0d7afcd2a1cbae2510): DocidIndex लागत गणना में सुधार किया गया है, जिससे समग्र प्रदर्शन बढ़ता है।
* [कमिट f3ae](https://github.com/manticoresoftware/manticoresearch/commit/f3ae8bea): लोड मैट्रिक्स, जो लिनक्स पर 'uptime' के समान होते हैं, अब `SHOW STATUS` कमांड में दिखाई देते हैं।
* [कमिट f3cc](https://github.com/manticoresoftware/manticoresearch/commit/f3cc0971): `DESC` और `SHOW CREATE TABLE` के लिए फ़ील्ड और विशेषता क्रम अब `SELECT * FROM` के समान है।
* [कमिट f3d2](https://github.com/manticoresoftware/manticoresearch/commit/f3d248a6): विभिन्न आंतरिक पार्सर्स अब विभिन्न त्रुटियों के दौरान अपना आंतरिक संक्षेपण कोड (उदाहरण के लिए, `P01`) प्रदान करते हैं। यह सुधार यह पहचानने में मदद करता है कि कौन सा पार्सर त्रुटि का कारण बना और गैर-आवश्यक आंतरिक विवरणों को भी छिपाता है।
* [समस्या #271](https://github.com/manticoresoftware/manticoresearch/issues/271) "कभी-कभी CALL SUGGEST एकल अक्षर की टाइपो का सुधार नहीं सुझाता": छोटे शब्दों के लिए [SUGGEST/QSUGGEST](../Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) व्यवहार में सुधार किया गया: पूरे वाक्य को दिखाने के लिए `sentence` विकल्प जोड़ा गया
* [समस्या #696](https://github.com/manticoresoftware/manticoresearch/issues/696) "पेरकोलेट अनुक्रमांक स्टेमिंग सक्षम होने पर सटीक वाक्यांश प्रश्न द्वारा सही से खोज नहीं करता": पेरकोलेट प्रश्न को सटीक शब्द नियंत्रक को संभालने के लिए संशोधित किया गया, जिससे खोज कार्यक्षमता में सुधार हुआ।
* [समस्या #829](https://github.com/manticoresoftware/manticoresearch/issues/829) "तारीख फ़ॉर्मेटिंग विधियाँ": [date_format()](../Functions/Date_and_time_functions.md#DATE_FORMAT()) चयन सूची अभिव्यक्ति जोड़ी गई, जो `strftime()` फ़ंक्शन को उजागर करती है।
* [समस्या #961](https://github.com/manticoresoftware/manticoresearch/issues/961) "HTTP JSON API के माध्यम से बाल्टियों को क्रमबद्ध करना": HTTP इंटरफेस में प्रत्येक बैकेट के लिए एक वैकल्पिक [क्रम संपत्ति](../Searching/Faceted_search.md#HTTP-JSON) पेश किया गया।
* [Issue #1062](https://github.com/manticoresoftware/manticoresearch/issues/1062) "JSON डालने के एपीआई विफलता का त्रुटि लॉगिंग सुधारें - "समर्थित मान प्रकार नहीं"": `/bulk` अंतबिंदु एक त्रुटि की स्थिति में संसाधित और गैर-संसाधित स्ट्रिंग (दस्तावेज़) की संख्या के बारे में जानकारी रिपोर्ट करता है।
* [Issue #1070](https://github.com/manticoresoftware/manticoresearch/issues/1070) "CBO संकेत कई विशेषताओं का समर्थन नहीं करते": कई विशेषताओं को संभालने के लिए अनुक्रमणिका संकेत सक्षम किए गए हैं।
* [Issue #1106](https://github.com/manticoresoftware/manticoresearch/issues/1106) "HTTP खोज प्रश्न में टैग जोड़ें": [HTTP PQ प्रतिक्रियाओं](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) में टैग जोड़े गए हैं।
* [Issue #1301](https://github.com/manticoresoftware/manticoresearch/issues/1301) "बडी को समानांतर में तालिका नहीं बनानी चाहिए": एक समस्या का समाधान किया गया जो समानांतर CREATE TABLE क्रियाओं से विफलता का कारण बन रही थी। अब, एक समय में केवल एक `CREATE TABLE` क्रिया चल सकती है।
* [Issue #1303](https://github.com/manticoresoftware/manticoresearch/issues/1303) "स्तंभ नामों में @ का समर्थन जोड़ें"।
* [Issue #1316](https://github.com/manticoresoftware/manticoresearch/issues/1316) "टैक्सी डेटासेट पर प्रश्न धीमे होते हैं ps=1 के साथ": CBO लॉजिक को परिष्कृत किया गया है, और यादृच्छिक रूप से वितरित मानों वाली विशेषताओं के लिए बेहतर सटीकता के लिए डिफ़ॉल्ट हिस्टोग्राम समाधान को 8k पर सेट किया गया है।
* [Issue #1317](https://github.com/manticoresoftware/manticoresearch/issues/1317) "hn डेटासेट पर CBO बनाम पूर्णपाठ को ठीक करें": यह निर्धारित करने के लिए बेहतर लॉजिक लागू किया गया है कि कब बिटमैप इटरटर अंतर का उपयोग किया जाता है और कब प्राथमिकता कतार का उपयोग किया जाता है।
* [Issue #1318](https://github.com/manticoresoftware/manticoresearch/issues/1318) "कॉलम: इटरटर इंटरफ़ेस को सिंगल-कॉल में बदलें": कॉलम इटरटर अब एकल `Get` कॉल का उपयोग करते हैं, जिससे एक मान प्राप्त करने के लिए पिछले दो-चरण `AdvanceTo` + `Get` कॉल को प्रतिस्थापित किया गया है।
* [Issue #1319](https://github.com/manticoresoftware/manticoresearch/issues/1319) "एग्रीगेट कैलक स्पीडअप (CheckReplaceEntry? को हटा दें)": एग्रीगेट फ़ंक्शन की गणना को तेज करने के लिए `CheckReplaceEntry` कॉल को समूह सॉर्टर से हटा दिया गया था।
* [Issue #1320](https://github.com/manticoresoftware/manticoresearch/issues/1320) "table read_buffer_docs/hits बनाएं समझते नहीं k/m/g सिंटैक्स": अब `CREATE TABLE` विकल्प `read_buffer_docs` और `read_buffer_hits` k/m/g सिंटैक्स का समर्थन करते हैं।
* इंग्लिश, जर्मन और रूसी के लिए [भाषा पैक्स](../Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) अब Linux पर `apt/yum install manticore-language-packs` कमांड चलाकर आसानी से स्थापित किए जा सकते हैं। macOS पर, `brew install manticoresoftware/tap/manticore-language-packs` कमांड का उपयोग करें।
* फ़ील्ड और विशेषता के आदेश अब `SHOW CREATE TABLE` और `DESC` कार्यों के बीच एकसमान हैं।
* यदि `INSERT` प्रश्नों को निष्पादित करते समय डिस्क स्थान अपर्याप्त है, तो नए `INSERT` प्रश्न विफल हो जाएंगे जब तक कि पर्याप्त डिस्क स्थान उपलब्ध नहीं हो जाता।
* [UINT64()](../Functions/Type_casting_functions.md#UINT64%28%29) प्रकार रूपांतरण फ़ंक्शन जोड़ा गया है।
* `/bulk` अंतबिंदु अब खाली लाइनों को [कमिट](../Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) कमांड के रूप में संसाधित करता है। अधिक जानकारी [यहां](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real_time_table.md?client=JSON#Bulk-adding-documents) है।
* [अमान्य अनुक्रमणिका संकेतों](../Searching/Options.md#Query-optimizer-hints) के लिए चेतावनियाँ लागू की गई हैं, जो अधिक पारदर्शिता प्रदान करती हैं और त्रुटि शमन की अनुमति देती हैं।
* जब `count(*)` एक एकल फ़िल्टर के साथ उपयोग किया जाता है, तो प्रश्न अब उपलब्ध होने पर द्वितीयक अनुक्रमणिकाओं से पूर्वनिर्धारित डेटा का लाभ उठाते हैं, जिससे प्रश्न समय में काफी तेजी आती है।

### ⚠️ तोड़ने वाले परिवर्तन
* ⚠️ संस्करण 6.2.0 में बनाए गए या संशोधित तालिकाओं को पुराने संस्करणों द्वारा नहीं पढ़ा जा सकता है।
* ⚠️ दस्तावेज़ आईडी अब अनुक्रमणिका और INSERT कार्यों के दौरान बिना चिन्हित 64-बिट पूर्णांक के रूप में संभाले जाते हैं।
* ⚠️ क्वेरी ऑप्टिमाइज़र सुझावों के लिए सिंटैक्स को अपडेट किया गया है। नया प्रारूप `/*+ SecondaryIndex(uid) */` है। कृपया ध्यान दें कि पुराना सिंटैक्स अब समर्थित नहीं है।
* ⚠️ [Issue #1160](https://github.com/manticoresoftware/manticoresearch/issues/1160): तालिका नामों में `@` का उपयोग करने की अनुमति नहीं दी गई है ताकि सिंटैक्स संघर्षों को रोका जा सके।
* ⚠️ `indexed` और `attribute` के रूप में चिह्नित स्ट्रिंग फ़ील्ड / विशेषताएँ अब `INSERT`, `DESC`, और `ALTER` कार्यों के दौरान एक ही फ़ील्ड के रूप में मानी जाती हैं।
* ⚠️ [Issue #1057](https://github.com/manticoresoftware/manticoresearch/issues/1057): MCL पुस्तकालय अब उन प्रणालियों पर लोड नहीं होंगे जो SSE 4.2 का समर्थन नहीं करते।
* ⚠️ [Issue #1143](https://github.com/manticoresoftware/manticoresearch/issues/1143): [agent_query_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) टूट गया था। ठीक किया गया है और अब प्रभावी है।

### बग फिक्स
* [Commit 2a6e](https://github.com/manticoresoftware/manticoresearch/commit/2a6ea8f7) "DROP TABLE पर क्रैश": एक समस्या का समाधान किया गया जो एक DROP TABLE विवरण का निष्पादन करते समय RT तालिका पर लिखने की क्रियाओं (इ ऑप्टीमाइज़ करना, डिस्क खंड को बचाना) को पूरा करने के लिए विस्तारित प्रतीक्षा समय का कारण बन रही थी। एक चेतावनी जोड़ी गई है जब DROP TABLE कमांड निष्पादन के बाद तालिका निर्देशिका अधीन नहीं होती है।
* [Commit 2ebd](https://github.com/manticoresoftware/manticoresearch/commit/2ebd424d): कॉलम विशेषताओं का समर्थन, जो कई विशेषताओं द्वारा समूह बनाने के लिए उपयोग किए गए कोड में अनुपस्थित था, जोड़ा गया है।
* [Commit 3be4](https://github.com/manticoresoftware/manticoresearch/commit/3be4503f752216b37b8cde8e47fd2d77bbce63ae) एक क्रैश समस्या का समाधान किया गया जो संभावित रूप से डिस्क स्थान खत्म होने के कारण उत्पन्न हो रही थी, बिनलॉग में लेखन त्रुटियों को सही तरीके से संभालकर।
* [Commit 6adb](https://github.com/manticoresoftware/manticoresearch/commit/6adb0934): एक क्रैश जो कभी-कभी एक प्रश्न में कई कॉलम स्कैन इटरटर (या द्वितीयक अनुक्रमणिका इटरटर) का उपयोग करते समय होता था, उसे ठीक किया गया है।
* [Commit 6bd9](https://github.com/manticoresoftware/manticoresearch/commit/6bd9f709): फ़िल्टर तब हटाए नहीं जा रहे थे जब पूर्व-निर्धारित डेटा का उपयोग करने वाले सॉर्टर्स का उपयोग किया जाता था। इस मुद्दे को ठीक किया गया है।
* [Commit 6d03](https://github.com/manticoresoftware/manticoresearch/commit/6d03566a): CBO कोड को उन क्वेरीज़ के लिए बेहतर अनुमान प्रदान करने के लिए अद्यतन किया गया है जो मल्टी-थ्रेड में पंक्ति-वार गुणों पर फ़िल्टर का उपयोग करती हैं।
* [Commit 6dd3](https://github.com/manticoresoftware/manticoresearch/commit/6dd3964), [Helm #56](https://github.com/manticoresoftware/manticoresearch-helm/issues/56) "Kubernetes क्लस्टर में घातक क्रैश डंप": JSON रूट ऑब्जेक्ट के लिए एक दोषपूर्ण ब्लूम फ़िल्टर को ठीक किया; JSON क्षेत्र द्वारा फ़िल्टरिंग के कारण डेमन क्रैश को ठीक किया गया।
* [Commit 6e1b](https://github.com/manticoresoftware/manticoresearch/commit/6e1b849) अमान्य `manticore.json` कॉन्फ़िगरेशन के कारण डेमन क्रैश को सही किया गया।
* [Commit 6fbc](https://github.com/manticoresoftware/manticoresearch/commit/6fbc4189) int64 मूल्यों का समर्थन करने के लिए json रेंज फ़िल्टर को ठीक किया गया।
* [Commit 9c67](https://github.com/manticoresoftware/manticoresearch/commit/9c677426) `.sph` फ़ाइलें भ्रष्ट हो सकती हैं `ALTER`। ठीक किया गया।
* [Commit 77cc](https://github.com/manticoresoftware/manticoresearch/commit/77ccd8f): पुनरुत्पादन कथन के लिए एक साझा कुंजी जोड़ी गई है ताकि कई मास्टर नोड्स से पुनरुत्पादन के समय `pre_commit` त्रुटि का समाधान किया जा सके।
* [Commit 2884](https://github.com/manticoresoftware/manticoresearch/commit/2884519e) 'date_format()' जैसे कार्यों पर bigint जाँच के साथ मुद्दों को हल किया गया।
* [Commit 9513](https://github.com/manticoresoftware/manticoresearch/commit/9513aca5): अब इटरेटर [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) में प्रदर्शित नहीं होते हैं जब सॉर्टर्स पूर्व-निर्धारित डेटा का उपयोग करते हैं।
* [Commit a2a7](https://github.com/manticoresoftware/manticoresearch/commit/a2a70555): जटिल पूर्ण पाठ क्वेरीज़ पर क्रैश को रोकने के लिए पूर्ण पाठ नोड स्टैक आकार को अपडेट किया गया है।
* [Commit a062](https://github.com/manticoresoftware/manticoresearch/commit/a0626d7e): JSON और स्ट्रिंग गुणों के साथ अपडेट के पुनरुत्पादन के दौरान क्रैश का कारण बनने वाली एक बग को हल किया गया है।
* [Commit b3e6](https://github.com/manticoresoftware/manticoresearch/commit/b3e63e5d): स्ट्रिंग बिल्डर को बड़े डेटा सेट के साथ निपटने के दौरान क्रैश से बचने के लिए 64-बिट पूर्णांक का उपयोग करने के लिए अपडेट किया गया है।
* [Commit c472](https://github.com/manticoresoftware/manticoresearch/commit/c472e5b): कई इंडेक्स के बीच मात्रा का अद्वितीय काउंट होते समय उत्पन्न होने वाले क्रैश को संबोधित किया गया।
* [Commit d073](https://github.com/manticoresoftware/manticoresearch/commit/d0730272): एक मुद्दे को ठीक किया गया जहां RT इंडेक्स के डिस्क टुकड़ों पर क्वेरीज़ को कई थ्रेड्स में निष्पादित किया जा सकता था, भले ही `pseudo_sharding` अक्षम था।
* [Commit d205](https://github.com/manticoresoftware/manticoresearch/commit/d205508e) `show index status` कमांड द्वारा लौटाए गए मानों का सेट संशोधित किया गया है और अब उपयोग में इंडेक्स के प्रकार के आधार पर भिन्न होता है।
* [Commit e9bc](https://github.com/manticoresoftware/manticoresearch/commit/e9bc2f9) बल्क अनुरोधों को संसाधित करते समय एक HTTP त्रुटि और एक मुद्दा ठीक किया गया जहां त्रुटि क्लाइंट को नेट लूप से वापस नहीं की जा रही थी।
* [Commit f77c](https://github.com/manticoresoftware/manticoresearch/commit/f77ce0e6) PQ के लिए विस्तारित स्टैक का उपयोग।
* [Commit fac2](https://github.com/manticoresoftware/manticoresearch/commit/fac2d9687a20582765aaa89be9597804c4f845af) [packedfactors()](../Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29) के साथ समन्वय के लिए एक्सपोर्ट रेंकर आउटपुट को अपडेट किया गया।
* [Commit ff87](https://github.com/manticoresoftware/manticoresearch/commit/ff87cc4): SphinxQL क्वेरी लॉग के फ़िल्टर में स्ट्रिंग सूची के साथ एक मुद्दे को ठीक किया गया।
* [Issue #589](https://github.com/manticoresoftware/manticoresearch/issues/589) "चरसेट परिभाषा कोड की क्रमबद्धता पर निर्भर लगती है": डुप्लीकेट के लिए गलत चरसेट मैपिंग को ठीक किया गया।
* [Issue #811](https://github.com/manticoresoftware/manticoresearch/issues/811) "शब्द रूपों में कई शब्दों का मैपिंग कुंजीवर्ड के बीच CJK विराम चिह्नों के साथ वाक्य खोज में हस्तक्षेप करता है": शब्दरूपों के साथ वाक्य क्वेरी के भीतर ngram टोकन स्थिति को ठीक किया गया।
* [Issue #834](https://github.com/manticoresoftware/manticoresearch/issues/834) "खोज क्वेरी में समानता चिह्न अनुरोध को तोड़ता है": सुनिश्चित किया गया कि सटीक प्रतीक को भाग-भाग किया जा सके और `expand_keywords` विकल्प द्वारा डबल सटीक विस्तार को ठीक किया गया।
* [Issue #864](https://github.com/manticoresoftware/manticoresearch/issues/864) "exceptions/stopwords संघर्ष"
* [Issue #910](https://github.com/manticoresoftware/manticoresearch/issues/910) "libstemmer_fr और index_exact_words के साथ call snippets() को कॉल करते समय Manticore क्रैश": `SNIPPETS()` को कॉल करते समय क्रैश का कारण बनने वाले आंतरिक संघर्षों को हल किया गया।
* [Issue #946](https://github.com/manticoresoftware/manticoresearch/issues/946) "SELECT के दौरान डुप्लिकेट रिकॉर्ड": `not_terms_only_allowed` विकल्प के साथ एक क्वेरी के लिए परिणाम सेट में डुप्लिकेट दस्तावेज़ों के मुद्दे को ठीक किया गया।
* [Issue #967](https://github.com/manticoresoftware/manticoresearch/issues/967) "UDF कार्यों में JSON तर्कों का उपयोग क्रैश का कारण बनता है": पseudo-sharding सक्षम और JSON तर्क के साथ UDF के साथ खोज संसाधित करते समय डेमन क्रैश को ठीक किया गया।
* [Issue #1050](https://github.com/manticoresoftware/manticoresearch/issues/1050) "FEDERATED में count(*)": एक `FEDERATED` इंजन के माध्यम से क्वेरी के साथ होने वाले डेमन क्रैश को ठीक किया गया।
* [Issue #1052](https://github.com/manticoresoftware/manticoresearch/issues/1052) एक मुद्दे को ठीक किया गया जहां `rt_attr_json` कॉलम कॉलम स्टोरेज के साथ असंगत था।
* [Issue #1072](https://github.com/manticoresoftware/manticoresearch/issues/1072) "* खोज क्वेरी द्वारा ignore_chars से हटा दिया गया": इस मुद्दे को ठीक किया गया ताकि एक क्वेरी में वाइल्डकार्ड `ignore_chars` द्वारा प्रभावित न हों।
* [Issue #1075](https://github.com/manticoresoftware/manticoresearch/issues/1075) "indextool --check fails if there's a distributed table": indextool अब 'distributed' और 'template' इंडेक्स वाले इंस्टेंस के साथ कार्य करता है json config में।
* [Issue #1081](https://github.com/manticoresoftware/manticoresearch/issues/1081) "particular select on particular RT dataset leads to crash of searchd": packedfactors और बड़े आंतरिक बफर के साथ एक क्वेरी पर डेमन क्रैश को हल किया गया।
* [Issue #1095](https://github.com/manticoresoftware/manticoresearch/issues/1095) "With not_terms_only_allowed deleted documents are ignored"
* [Issue #1099](https://github.com/manticoresoftware/manticoresearch/issues/1099) "indextool --dumpdocids is not working": `--dumpdocids` कमांड की कार्यक्षमता को पुनर्स्थापित किया गया।
* [Issue #1100](https://github.com/manticoresoftware/manticoresearch/issues/1100) "indextool --buildidf is not working": indextool अब globalidf समाप्त करने के बाद फाइल को बंद कर देता है।
* [Issue #1104](https://github.com/manticoresoftware/manticoresearch/issues/1104) "Count(*) is trying to be treated as schema set in remote tables": एक मुद्दा हल किया गया जिसमें डेमन द्वारा वितरित इंडेक्स में क्वेरी के लिए एक त्रुटि संदेश भेजा जा रहा था जब एजेंट ने एक खाली परिणाम सेट लौटाया।
* [Issue #1109](https://github.com/manticoresoftware/manticoresearch/issues/1109) "FLUSH ATTRIBUTES hangs with threads=1".
* [Issue #1126](https://github.com/manticoresoftware/manticoresearch/issues/1126) "Lost connection to MySQL server during query - manticore 6.0.5": कॉलमर विशेषताएँ पर कई फ़िल्टर का उपयोग करते समय हो रही दुर्घटनाओं को संबोधित किया गया है।
* [Issue #1135](https://github.com/manticoresoftware/manticoresearch/issues/1135) "JSON string filtering case sensitivity": HTTP खोज अनुरोधों में उपयोग किए गए फ़िल्टर के लिए कोलेशन को सही किया गया।
* [Issue #1140](https://github.com/manticoresoftware/manticoresearch/issues/1140) "Match in a wrong field": `morphology_skip_fields` से संबंधित नुकसान को ठीक किया गया।
* [Issue #1155](https://github.com/manticoresoftware/manticoresearch/issues/1155) "system remote commands via API should pass g_iMaxPacketSize": नोड्स के बीच पुनरावृत्ति आदेशों के लिए `max_packet_size` जांच को बायपास करने के लिए अपडेट किया गया। इसके अलावा, नवीनतम क्लस्टर त्रुटि स्थिति प्रदर्शन में जोड़ी गई है।
* [Issue #1302](https://github.com/manticoresoftware/manticoresearch/issues/1302) "tmp files left on failed optimize": एक मुद्दा ठीक किया गया जिसमें एक मर्ज या ऑप्टिमाइज़ प्रक्रिया के दौरान त्रुटि होने के बाद अस्थायी फ़ाइलें छोड़ दी गई थीं।
* [Issue #1304](https://github.com/manticoresoftware/manticoresearch/issues/1304) "add env var for buddy start timeout": डेमन के स्टार्टअप पर एक साथी संदेश के लिए प्रतीक्षा अवधि को नियंत्रित करने के लिए वातावरण चर `MANTICORE_BUDDY_TIMEOUT` (डिफ़ॉल्ट 3 सेकंड) जोड़ा गया।
* [Issue #1305](https://github.com/manticoresoftware/manticoresearch/issues/1305) "Int overflow when saving PQ meta": बड़े PQ इंडेक्स को सहेजते समय डेमन द्वारा अत्यधिक मेमोरी खपत को कम किया गया।
* [Issue #1306](https://github.com/manticoresoftware/manticoresearch/issues/1306) "Can't recreate RT table after altering its external file": बाहरी फ़ाइलों के लिए खाली स्ट्रिंग के साथ परिवर्तन का एक त्रुटि को सही किया गया; बाहरी फ़ाइलों को बदलने के बाद RT इंडेक्स बाहरी फ़ाइलें भी सुधारी गईं।
* [Issue #1307](https://github.com/manticoresoftware/manticoresearch/issues/1307) "SELECT statement sum(value) as value doesn't work properly": चयन सूची अभिव्यक्ति में एलेयस के साथ समस्या को ठीक किया गया जो कि इंडेक्स विशेषता को छुपा सकता है; और पूर्णांक के लिए int64 में गिनती करने के लिए sum को भी ठीक किया गया।
* [Issue #1308](https://github.com/manticoresoftware/manticoresearch/issues/1308) "Avoid binding to localhost in replication": सुनिश्चित किया गया कि पुनरावृत्ति स्थानीय होस्ट से नहीं बंधित होती है जब मेज़बान नामों में कई आईपी होते हैं।
* [Issue #1309](https://github.com/manticoresoftware/manticoresearch/issues/1309) "reply to mysql client failed for data larger 16Mb": 16Mb से बड़े SphinxQL पैकेट को क्लाइंट को लौटाने की समस्या को ठीक किया गया।
* [Issue #1310](https://github.com/manticoresoftware/manticoresearch/issues/1310) "wrong reference in "paths to external files should be absolute": `SHOW CREATE TABLE` में बाहरी फ़ाइलों के पूर्ण पथ को प्रदर्शित करने में त्रुटि को ठीक किया गया।
* [Issue #1311](https://github.com/manticoresoftware/manticoresearch/issues/1311) "debug build crashes on long strings in snippets": अब, लंबे स्ट्रिंग (>255 वर्ण) `SNIPPET()` फ़ंक्शन द्वारा लक्षित टेक्स्ट में अनुमति दिए जाते हैं।
* [Issue #1312](https://github.com/manticoresoftware/manticoresearch/issues/1312) "spurious crash on use-after-delete in kqueue polling (master-agent)": जब मास्टर kqueue-चालित प्रणालियों (FreeBSD, MacOS, आदि) पर एजेंट से कनेक्ट नहीं कर सकता है, तो दुर्घटनाओं को ठीक किया गया।
* [Issue #1313](https://github.com/manticoresoftware/manticoresearch/issues/1313) "too long connect to itself": MacOS/BSD पर एजेंटों से मास्टर से कनेक्ट करते समय, अब केवल कनेक्ट के बजाय एकीकृत कनेक्ट+क्वेरी टाइमआउट का उपयोग किया जाता है।
* [Issue #1314](https://github.com/manticoresoftware/manticoresearch/issues/1314) "pq (json meta) with unreached embedded synonyms fails to load": pq में एम्बेडेड पर्यायवाची ध्वज को ठीक किया गया।
* [Issue #1315](https://github.com/manticoresoftware/manticoresearch/issues/1315) "Allow some functions (sint, fibonacci, second, minute, hour, day, month, year, yearmonth, yearmonthday) to use implicitly promoted argument values".
* [Issue #1321](https://github.com/manticoresoftware/manticoresearch/issues/1321) "Enable multithreaded SI in fullscan, but limit threads": CBO में कोड लागू किया गया है ताकि जब उन्हें पूर्ण-पाठ क्वेरी में उपयोग किया जाता है तो द्वितीयक सूचकांकों के मल्टीथ्रेडेड प्रदर्शन को बेहतर ढंग से पूर्वानुमानित किया जा सके।
* [Issue #1322](https://github.com/manticoresoftware/manticoresearch/issues/1322) "count(*) queries still slow after using precalc sorters": पहले से प्राप्त डेटा का उपयोग करने वाले सॉर्टर्स का उपयोग करते समय अब इटरटर्स प्रारंभ नहीं किए जाते हैं, जिससे हानिकारक प्रदर्शन प्रभावों को बायपास किया जाता है।
* [Issue #1411](https://github.com/manticoresoftware/manticoresearch/issues/1141) "sphinxql में क्वेरी लॉग मूल क्वेरीज को MVA के लिए संरक्षित नहीं करता": अब, `all()/any()` लॉग किया गया है।

# संस्करण 6.0.4
जारी: 15 मार्च 2023

### नई सुविधाएं
* Logstash, Beats आदि के साथ सुधारित एकीकरण जिसमें:
  - Logstash संस्करण 7.6 - 7.15, Filebeat संस्करण 7.7 - 7.12 के लिए समर्थन  
  - ऑटो-स्कीमा समर्थन।
  - Elasticsearch-जैसे प्रारूप में बल्क अनुरोधों के लिए हैंडलिंग जोड़ी गई।
* [Buddy commit ce90](https://github.com/manticoresoftware/manticoresearch-buddy/commit/ce907ea) Manticore प्रारंभ पर लॉग बडी संस्करण।

### बग फिक्सेस
* [Issue #588](https://github.com/manticoresoftware/manticoresearch/issues/588), [Issue #942](https://github.com/manticoresoftware/manticoresearch/issues/942) बड़ेग्राम इंडेक्स के लिए खोज मेटा और कॉल कीवर्ड में खराब चर को ठीक किया गया।
* [Issue #1027](https://github.com/manticoresoftware/manticoresearch/issues/1027) HTTP हेडर को लोअरकेस में अस्वीकार किया गया।
* ❗[Issue #1039](https://github.com/manticoresoftware/manticoresearch/issues/1039) बडी कंसोल के आउटपुट को पढ़ने पर डेमन में मेमोरी लीक ठीक किया गया।
* [Issue #1056](https://github.com/manticoresoftware/manticoresearch/issues/1056) प्रश्न चिह्न के अप्रत्याशित व्यवहार को ठीक किया गया।
* [Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - क्रैश का कारण बनने वाले टोकनाइज़र लोअरकेस तालिकाओं में रेस स्थिति को ठीक किया गया।
* [Commit 59bb](https://github.com/manticoresoftware/manticoresearch/commit/59bb54c) JSON इंटरफेस में दस्तावेजों के लिए बल्क लेखन प्रसंस्करण को ठीक किया गया, जिनका आईडी स्पष्ट रूप से शून्य सेट किया गया था।
* [Commit 7b6b](https://github.com/manticoresoftware/manticoresearch/commit/7b6b25f) कई समान शर्तों के लिए CALL KEYWORDS में टर्म सांख्यिकी को ठीक किया गया।
* [Commit f381](https://github.com/manticoresoftware/manticoresearch/commit/f381ad2) डिफ़ॉल्ट कॉन्फ़िग Windows इंस्टॉलर द्वारा अब बनाया गया है; रनटाइम में पथों को अब प्रतिस्थापित नहीं किया गया है।
* [Commit 6940](https://github.com/manticoresoftware/manticoresearch/commit/6940e95), [Commit cc5a](https://github.com/manticoresoftware/manticoresearch/commit/cc5a480) कई नेटवर्कों में नोड्स के साथ क्लस्टर के लिए पुनरुत्पादन समस्याओं को ठीक किया गया।
* [Commit 4972](https://github.com/manticoresoftware/manticoresearch/commit/49722ab) `/pq` HTTP एंडपॉइंट को `/json/pq` HTTP एंडपॉइंट का उपनाम बनाने के लिए ठीक किया गया।
* [Commit 3b53](https://github.com/manticoresoftware/manticoresearch/commit/3b5385a) बडी पुनःआरंभ पर डेमन क्रैश को ठीक किया गया।
* [Buddy commit fba9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/fba9c8c) प्राप्त पर अमान्य अनुरोध पर मूल त्रुटि प्रदर्शित करें।
* [Buddy commit db95](https://github.com/manticoresoftware/manticoresearch-buddy/commit/db9532c) बैकअप पथ में स्पेस की अनुमति दें और एकल उद्धरणों का समर्थन करने के लिए regexp में कुछ जादू जोड़ें।

# संस्करण 6.0.2
जारी: 10 फरवरी 2023

### बग फिक्सेस
* [Issue #1024 crash 2](https://github.com/manticoresoftware/manticoresearch/issues/1024) बड़ी संख्या में परिणामों के साथ फैसेट खोज पर क्रैश / विभाजन दोष
* ❗[Issue #1029](https://github.com/manticoresoftware/manticoresearch/issues/1029) - चेतावनी: संकलित मान KNOWN_CREATE_SIZE (16) मापा गया (208) से कम है। मान को ठीक करने पर विचार करें!
* ❗[Issue #1032](https://github.com/manticoresoftware/manticoresearch/issues/1032) - Manticore 6.0.0 साधारण इंडेक्स क्रैश होती है
* ❗[Issue #1033](https://github.com/manticoresoftware/manticoresearch/issues/1033) - डेमन पुनः आरंभ पर कई वितरित खो गए
* ❗[Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - टोकनाइज़र लोअरकेस तालिकाओं में रेस स्थिति

# संस्करण 6.0.0
जारी: 7 फरवरी 2023

इस रिलीज के साथ, Manticore Search में Manticore Buddy है, एक साइडकार डेमन जो PHP में लिखा गया है जो उच्च-स्तरीय कार्यक्षमता को संभालता है जिसे सुपर कम विलंबता या उच्च थ्रूपुट की आवश्यकता नहीं होती है। Manticore Buddy पर्दे के पीछे काम करता है, और आप यह भी नहीं समझ सकते कि यह चल रहा है। हालांकि यह अंतिम उपयोगकर्ता के लिए अदृश्य है, यह Manticore Buddy को आसानी से स्थापित करने योग्य और मुख्य C++-आधारित डेमन के साथ संगत बनाने के लिए एक महत्वपूर्ण चुनौती थी। यह महत्वपूर्ण परिवर्तन टीम को नए उच्च-स्तरीय सुविधाओं की एक विस्तृत श्रृंखला विकसित करने की अनुमति देगा, जैसे कि शार्ड्स ऑर्केस्ट्रेशन, पहुंच नियंत्रण और प्रमाणीकरण, और mysqldump, DBeaver, Grafana mysql कनेक्टर जैसी विभिन्न एकीकरण। अभी के लिए, यह पहले से ही [SHOW QUERIES](../Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES), [BACKUP](../Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) और [Auto schema](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) को संभालता है।

यह रिलीज 130 से अधिक बग फिक्स और कई सुविधाएं भी शामिल करती है, जिनमें से कई को प्रमुख माना जा सकता है।

### प्रमुख परिवर्तन
* 🔬 प्रयोगात्मक: आप अब Elasticsearch-संगत [insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?example=Elasticsearch#Adding-documents-to-a-real-time-table) और [replace](Data_creation_and_modification/Updating_documents/REPLACE.md?example=Elasticsearch#REPLACE) JSON क्वेरीज निष्पादित कर सकते हैं जो Logstash (संस्करण < 7.13), Filebeat और Beats परिवार के अन्य उपकरणों के साथ Manticore का उपयोग करने में सक्षम बनाते हैं। डिफ़ॉल्ट रूप से सक्षम। आप इसे `SET GLOBAL ES_COMPAT=off` का उपयोग करके निष्क्रिय कर सकते हैं।
* [Manticore कॉलम साझेदारी पुस्तकालय 2.0.0](https://github.com/manticoresoftware/columnar/) के लिए समर्थन जिसमें [गुणात्मक अनुक्रमण](../Server_settings/Searchd.md#secondary_indexes) में कई सुधार और फिक्स हैं। **⚠️ प्रमुख परिवर्तन**: इस रिलीज के रूप में द्वितीयक अनुक्रमण डिफ़ॉल्ट रूप से चालू हैं। यदि आप Manticore 5 से अपग्रेड कर रहे हैं तो सुनिश्चित करें कि आप [ALTER TABLE table_name REBUILD SECONDARY](../Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) करें। विस्तृत विवरण के लिए नीचे देखें।
* [Commit c436](https://github.com/manticoresoftware/manticoresearch/commit/c436f9023536f767610451911955ae36d90aa638) ऑटो-स्कीमा: आप अब एक टेबल बनाने को छोड़ सकते हैं, बस पहला दस्तावेज़ डालें और मैन्टिकोर अपने फ़ील्ड के आधार पर स्वचालित रूप से तालिका बनाएगा। इसके बारे में विस्तार से और पढ़ें [यहां](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema)। आप इसे [searchd.auto_schema](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema) का उपयोग करके चालू/बंद कर सकते हैं।
* [cost-based optimizer](../Searching/Cost_based_optimizer.md) का व्यापक पुनर्विकास, जो कई मामलों में क्वेरी प्रतिक्रिया समय को कम करता है।
  - [Issue #1008](https://github.com/manticoresoftware/manticoresearch/issues/1008) CBO में समानांतरकरण प्रदर्शन का अनुमान।
  - [Issue #1014](https://github.com/manticoresoftware/manticoresearch/issues/1014) CBO अब [गुणात्मक अनुक्रमण](../Server_settings/Searchd.md#secondary_indexes) से अवगत है और स्मार्ट तरीके से कार्य कर सकता है।
  - [Commit cef9](https://github.com/manticoresoftware/columnar/commit/cef9b54dc3a4ee9138ebc4df3a671715b996d6b3) कॉलम डेटा की सांख्यिकीय जानकारी अब मेटा डेटा में संग्रहीत की जाती है ताकि CBO समझदारी से निर्णय ले सके।
  - [Commit 2b95](https://github.com/manticoresoftware/manticoresearch/commit/2b95fb8c2ba422c9c5a66e2b61c1f7922f31c7aa) CBO के व्यवहार को बारीकी से ट्यून करने के लिए सुझाव जोड़े।
* [Telemetry](../Telemetry.md#Telemetry): हमें इस रिलीज़ में टेलीमेट्री की अतिरिक्त जानकारी की घोषणा करते हुए खुशी हो रही है। यह फ़ीचर हमें गुमनाम और निरपेक्ष मेट्रिक्स एकत्र करने की अनुमति देता है जो हमारे उत्पाद की प्रदर्शन और उपयोगकर्ता अनुभव में सुधार करने में मदद करेगा। आश्वस्त रहें, सभी एकत्रित डेटा **पूर्ण रूप से गुमनाम है और किसी भी व्यक्तिगत जानकारी से जोड़ा नहीं जाएगा**। यदि इच्छित हो, तो इस फ़ीचर को सेटिंग्स में [आसान तरीके से बंद](../Telemetry.md#Telemetry) किया जा सकता है।
* [Commit 5aaf](https://github.com/manticoresoftware/manticoresearch/commit/5aafb17d81b601e399e212a344a4738f65576cdd) [ALTER TABLE table_name REBUILD SECONDARY](../Updating_table_schema_and_settings.md#Rebuilding-a-secondary-index) को कभी भी सेकंडरी अनुक्रमणों को पुनर्निर्माण करने के लिए लागू किया जा सकता है, उदाहरण के लिए:
  - जब आप मैन्टिकोर 5 से नए संस्करण में माइग्रेट करते हैं,
  - जब आपने अनुक्रमण में किसी विशेषता का [UPDATE](../Data_creation_and_modification/Updating_documents/UPDATE.md#UPDATE) (यानी [इन-प्लेस अपडेट, नहीं बदलें](Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md)) किया 
* [Issue #821](https://github.com/manticoresoftware/manticoresearch/issues/821) `manticore-backup` उपकरण [मैन्टिकोर इंस्टेंस का बैकअप और पुनर्स्थापना](../Securing_and_compacting_a_table/Backup_and_restore.md) के लिए।
* SQL कमांड [BACKUP](../Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) मैन्टिकोर के भीतर से बैकअप करने के लिए।
* SQL कमांड [SHOW QUERIES](../Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) चल रही क्वेरियों को देखने का एक आसान तरीका।
* [Issue #551](https://github.com/manticoresoftware/manticoresearch/issues/551) SQL कमांड `KILL` एक लंबे समय तक चलने वाले `SELECT` को समाप्त करने के लिए।
* एग्रीगेशन क्वेरियों के लिए डायनामिक `max_matches` सटीकता बढ़ाने और प्रतिक्रिया समय कम करने के लिए।

### छोटे परिवर्तन
* [Issue #822](https://github.com/manticoresoftware/manticoresearch/issues/822) SQL कमांड [FREEZE/UNFREEZE](../Securing_and_compacting_a_table/Freezing_a_table.md) एक वास्तविक समय / सादा तालिका को बैकअप के लिए तैयार करने के लिए।
* [Commit c470](https://github.com/manticoresoftware/manticoresearch/commit/c47052f18ad4868134de1a97e8d7da570e531dfa) नियंत्रित एग्रीगेशन सटीकता के लिए नए सेटिंग्स `accurate_aggregation` और `max_matches_increase_threshold`।
* [Issue #718](https://github.com/manticoresoftware/manticoresearch/issues/718) साइन किए गए निगेटिव 64-बिट आईडी के लिए समर्थन। ध्यान दें, आप अभी भी ID > 2^63 का उपयोग नहीं कर सकते, लेकिन अब आप -2^63 से 0 के बीच में आईडी का उपयोग कर सकते हैं।
* जैसे-जैसे हमने सेकंडरी अनुक्रमणों के लिए समर्थन जोड़ा, चीजें भ्रमित हो गईं क्योंकि "अनुक्रमण" एक सेकंडरी अनुक्रमण, एक पूर्ण-पाठ अनुक्रमण, या एक सामान्य/वास्तविक समय `अनुक्रमण` को संदर्भित कर सकता है। भ्रम को कम करने के लिए, हम बाद के को "तालिका" नाम दे रहे हैं। निम्नलिखित SQL / कमांड लाइन कमांड इस परिवर्तन से प्रभावित हैं। उनके पुराने संस्करण अप्रचलित हैं, लेकिन फिर भी कार्यात्मक हैं:
  - `index <table name>` => `table <table name>`,
  - `searchd -i / --index` => `searchd -t / --table`,
  - `SHOW INDEX STATUS` => `SHOW TABLE STATUS`,
  - `SHOW INDEX SETTINGS` => `SHOW TABLE SETTINGS`,
  - `FLUSH RTINDEX` => `FLUSH TABLE`,
  - `OPTIMIZE INDEX` => `OPTIMIZE TABLE`,
  - `ATTACH TABLE plain TO RTINDEX rt` => `ATTACH TABLE plain TO TABLE rt`,
  - `RELOAD INDEX` => `RELOAD TABLE`,
  - `RELOAD INDEXES` => `RELOAD TABLES`.

  हम पुराने रूपों को अप्रचलित बनाने की योजना नहीं बना रहे हैं, लेकिन दस्तावेज़ों के साथ संगतता सुनिश्चित करने के लिए, हम अनुशंसा करते हैं कि आप अपने एप्लिकेशन में नाम बदलें। भविष्य की रिलीज़ में विभिन्न SQL और JSON कमांड के आउटपुट में "अनुक्रमण" को "तालिका" नाम में बदल दिया जाएगा।
* स्टेटफुल UDFs के साथ क्वेरियों को अब एकल थ्रेड में निष्पादित करने के लिए मजबूर किया गया है।
* [Issue #1011](https://github.com/manticoresoftware/manticoresearch/issues/1011) समानांतरChunks मर्ज करने के लिए पूर्वापेक्षा के रूप में समय अनुसूची से संबंधित सभी का पुनर्गठन।
* **⚠️ संभावित समस्या**: कॉलमर स्टोरेज प्रारूप को बदल दिया गया है। आपको उन तालिकाओं का पुनर्निर्माण करने की आवश्यकता है जिनमें कॉलम वाले गुण हैं।
* **⚠️ संभावित समस्या**: सेकंडरी अनुक्रमणों के फ़ाइल प्रारूप को बदल दिया गया है, इसलिए यदि आप खोज के लिए सेकंडरी अनुक्रमणों का उपयोग कर रहे हैं और आपके कॉन्फ़िगरेशन फ़ाइल में `searchd.secondary_indexes = 1` है, तो ध्यान रखें कि नया मैन्टिकोर संस्करण **सेकंडरी अनुक्रमण वाले तालिकाओं को लोड करना छोड़ देगा**। यह अनुशंसा की जाती है:
  - अपग्रेड करने से पहले कॉन्फ़िगरेशन फ़ाइल में `searchd.secondary_indexes` को 0 में बदलें।
  - उदाहरण चलाएँ। मैन्टिकोर चेतावनी के साथ तालिकाएँ लोड करेगा।
  - प्रत्येक अनुक्रमण के लिए पुनर्निर्माण करने के लिए `ALTER TABLE <table name> REBUILD SECONDARY` चलाएँ।
  यदि आप पुनरावृत्ति क्लस्टर चला रहे हैं, तो आपको सभी नोड्स पर `ALTER TABLE <table name> REBUILD SECONDARY` चलाना होगा या [इस निर्देश](../Securing_and_compacting_a_table/Compacting_a_table.md#Optimizing-clustered-tables) का पालन करें, बस परिवर्तन करते हुए: `OPTIMIZE` के स्थान पर `ALTER .. REBUILD SECONDARY` चलाएं।
* **⚠️ ब्रेकिंग चेंज**: बिनलॉग संस्करण अपडेट किया गया है, इसलिए पिछली संस्करणों के किसी भी बिनलॉग को पुनः चलाया नहीं जाएगा। यह सुनिश्चित करना महत्वपूर्ण है कि अपग्रेड प्रक्रिया के दौरान Manticore Search को ठीक से रोका जाए। इसका मतलब है कि पिछले उदाहरण को रोकने के बाद `/var/lib/manticore/binlog/` में `binlog.meta` के अलावा कोई बिनलॉग फ़ाइलें नहीं होनी चाहिए।
* [समस्या #849](https://github.com/manticoresoftware/manticoresearch/issues/849) `SHOW SETTINGS`: अब आप Manticore के अंदर कॉन्फ़िगरेशन फ़ाइल से सेटिंग्स देख सकते हैं।
* [समस्या #1007](https://github.com/manticoresoftware/manticoresearch/issues/1007) [SET GLOBAL CPUSTATS=1/0](../Server_settings/Setting_variables_online.md#SET) CPU समय ट्रैकिंग को ऑन/ऑफ करता है; [SHOW THREADS](../Node_info_and_management/SHOW_THREADS.md) अब जब CPU समय ट्रैकिंग बंद है तो CPU सांख्यिकी नहीं दिखाता।
* [समस्या #1009](https://github.com/manticoresoftware/manticoresearch/issues/1009) RT तालिका RAM चंक खंड अब RAM चंक को फ्लश करते समय मिलाए जा सकते हैं।
* [समस्या #1012](https://github.com/manticoresoftware/manticoresearch/issues/1012) [इंडेक्सर](../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) के आउटपुट में सेकंडरी इंडेक्स प्रगति जोड़ी गई।
* [समस्या #1013](https://github.com/manticoresoftware/manticoresearch/issues/1013) पहले, एक तालिका रिकॉर्ड को Manticore द्वारा सूची से हटा दिया जा सकता था यदि यह प्रारंभ करने पर इसे सेवा देने में असमर्थ था। नया व्यवहार सूची में इसे रखने का है ताकि अगले प्रारंभ पर इसे लोड करने का प्रयास किया जा सके।
* [indextool --docextract](../Miscellaneous_tools.md#indextool) अनुरोधित दस्तावेज़ से संबंधित सभी शब्दों और हिट्स को वापस करता है।
* [कमिट 2b29](https://github.com/manticoresoftware/manticoresearch/commit/2b296ee20e520b85bcbb4383e87095d31e7165dc) पर्यावरण चर `dump_corrupt_meta` एक प्रदूषित तालिका मेटा डेटा को लॉग में डंप करने की अनुमति देता है यदि searchd अनुक्रमणिका को लोड नहीं कर सकता है।
* [कमिट c7a3](https://github.com/manticoresoftware/manticoresearch/commit/c7a3136eefe0eb2f69e3d99fae9c10beb5821ea6) `DEBUG META` `max_matches` और छद्म विभाजन सांख्यिकी दिखा सकता है।
* [कमिट 6bca](https://github.com/manticoresoftware/manticoresearch/commit/6bca4848dc5c8396dbc1e057be4b9b2d579794e8) उलझन भरी "अनुक्रमणिका हेडर फ़ॉर्मेट JSON नहीं है, इसे बाइनरी के रूप में आजमाएंगे..." के बजाय एक बेहतर त्रुटि।
* [कमिट bef3](https://github.com/manticoresoftware/lemmatizer-uk/commit/bef3ff0386d3ee87ec57619782100972c1122e47) यूक्रेनी लेमेटाइज़र पथ बदल गया है।
* [कमिट 4ae7](https://github.com/manticoresoftware/manticoresearch/commit/4ae789595329a2951e194d1191ddb3121459a560) [SHOW META](../Node_info_and_management/SHOW_META.md#SHOW-META) में सेकंडरी इंडेक्स सांख्यिकी जोड़ी गई है।
* [कमिट 2e7c](https://github.com/manticoresoftware/manticoresearch/commit/2e7c585e) JSON इंटरफेस को अब Swagger Editor https://manual.manticoresearch.com/Openapi#OpenAPI-specification का उपयोग करके आसानी से दृश्यृत किया जा सकता है।
* **⚠️ ब्रेकिंग चेंज**: पुनरावृत्ति प्रोटोकॉल बदल दिया गया है। यदि आप पुनरावृत्ति क्लस्टर चला रहे हैं, तो Manticore 5 में अपग्रेड करते समय आपको:
  - पहले सभी नोड्स को ठीक से रोकना होगा
  - और फिर उस नोड को प्रारंभ करें जो अंत में रोका गया था `--new-cluster` के साथ (Linux में उपकरण `manticore_new_cluster` चलाएं)।
  - अधिक विवरण के लिए [एक क्लस्टर को पुनः प्रारंभ करना](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) के बारे में पढ़ें।

### Manticore कॉलमर लाइब्रेरी से संबंधित परिवर्तन
* कॉलमर संग्रहण के साथ सेकंडरी इंडेक्स के एकीकरण का फिर से कार्यान्वयन।
* [कमिट efe2](https://github.com/manticoresoftware/columnar/commit/efe26b2a) Manticore कॉलमर लाइब्रेरी अनुकूलन जो न्यूनतम/अधिकतम पूर्व मूल्यांकन के हिस्से के द्वारा प्रतिक्रिया समय को कम कर सकता है।
* [कमिट 2757](https://github.com/manticoresoftware/columnar/commit/2757b99b) यदि एक डिस्क चंक मर्ज बाधित होता है, तो डेमन अब MCL-संबंधित अस्थायी फ़ाइलों को साफ करता है।
* [कमिट e9c6](https://github.com/manticoresoftware/manticoresearch/commit/e9c672014b63e2b49e7a5a53880583af86c3ab4e) कॉलमर और सेकंडरी लाइब्रेरी के संस्करण दुर्घटना के समय लॉग में डंप किए जाते हैं।
* [कमिट f5e8](https://github.com/manticoresoftware/columnar/commit/f5e84eeb8b6cb102f13e3468246702427527fef9) सेकंडरी इंडेक्स के लिए त्वरित डॉकलिस्ट रिवाइंडिंग के लिए समर्थन जोड़ा गया।
* [कमिट 06df](https://github.com/manticoresoftware/manticoresearch/commit/06dfdd2e668933495dd6cb52d5ba2e0aa9b26e58) जैसे प्रश्न `select attr, count(*) from plain_index` (फिल्टरिंग के बिना) अब तेज हैं यदि आप MCL का उपयोग कर रहे हैं।
* [कमिट 0a76](https://github.com/manticoresoftware/manticoresearch/commit/0a76ecb9adbe886868b962dd425de5f475bbf55e) .net कनेक्टर के साथ संगतता के लिए HandleMysqlSelectSysvar में @@autocommit।
* **⚠️ ब्रेकिंग चेंज**: [MCL समस्या #17](https://github.com/manticoresoftware/columnar/issues/17) MCL: कॉलमर स्कैन के लिए SSE कोड जोड़ें। MCL अब कम से कम SSE4.2 की आवश्यकता है।

### पैकेजिंग से संबंधित परिवर्तन
* [कमिट 4d19](https://github.com/manticoresoftware/manticoresearch/commit/4d19f5cbe49a31228aa09253f061165bfe80e51b) **⚠️ ब्रेकिंग चेंज**: Debian Stretch और Ubuntu Xenial के लिए समर्थन बंद कर दिया गया है।
* RHEL 9 का समर्थन जिसमें Centos 9, Alma Linux 9 और Oracle Linux 9 शामिल हैं।
* [समस्या #924](https://github.com/manticoresoftware/manticoresearch/issues/924) Debian Bookworm समर्थन।
* [समस्या #636](https://github.com/manticoresoftware/manticoresearch/issues/636) पैकेजिंग: Linuxes और MacOS के लिए arm64 बिल्ड।
* [PR #26](https://github.com/manticoresoftware/docker/pull/26) मल्टी-आर्किटेक्चर (x86_64 / arm64) डॉकर इमेज।
* [योगदानकर्ताओं के लिए पैकेज निर्माण को सरल बनाया गया](../Installation/Compiling_from_sources.md#Building-using-CI-Docker)।
* अब APT का उपयोग करके एक निर्दिष्ट संस्करण स्थापित करना संभव है।
* [Commit a6b8](https://github.com/manticoresoftware/manticoresearch/commit/51fddca5c2a3ebb8576fae4c18660656ba22de0f) विंडोज installer (पहले हमने सिर्फ एक आर्काइव प्रदान किया था)।
* CLang 15 का उपयोग करके संकलन करने के लिए स्विच किया गया।
* **⚠️ BREAKING CHANGE**: कस्टम होमब्रे फार्मुला जिसमें Manticore कॉलम लाइब्रेरी के लिए फार्मूला शामिल है। Manticore, MCL और किसी अन्य आवश्यक घटकों को स्थापित करने के लिए, निम्नलिखित कमांड का उपयोग करें `brew install manticoresoftware/manticore/manticoresearch manticoresoftware/manticore/manticore-extra`।

### बग सुधार
* [Issue #479](https://github.com/manticoresoftware/manticoresearch/issues/479) नाम के साथ फ़ील्ड `text`
* [Issue #501](https://github.com/manticoresoftware/manticoresearch/issues/501) id गैर bigint नहीं हो सकता
* [Issue #646](https://github.com/manticoresoftware/manticoresearch/issues/646) ALTER बनाम नाम के साथ फ़ील्ड "text"
* ❗[Issue #652](https://github.com/manticoresoftware/manticoresearch/issues/652) संभावित बग: HTTP (JSON) ऑफसेट और सीमाfacet परिणामों को प्रभावित करती है
* ❗[Issue #827](https://github.com/manticoresoftware/manticoresearch/issues/827) Searchd अत्यधिक लोडिंग के तहत लटकता/क्रैश करता है
* ❗[Issue #996](https://github.com/manticoresoftware/manticoresearch/issues/996) PQ इंडेक्स मेमोरी से बाहर
* ❗[Commit 1041](https://github.com/manticoresoftware/manticoresearch/commit/10416ef7dddf06c0d759e32ccd6ebaa2468f7cbf) `binlog_flush = 1` हमेशा से Sphinx के बाद टूट गया है। ठीक किया गया।
* [MCL Issue #14](https://github.com/manticoresoftware/columnar/issues/14) MCL: जब बहुत सारे ft फ़ील्ड होते हैं तो चयन पर क्रैश
* [Issue #470](https://github.com/manticoresoftware/manticoresearch/issues/470) sql_joined_field संग्रहीत नहीं किया जा सकता
* [Issue #713](https://github.com/manticoresoftware/manticoresearch/issues/713) LEVENSHTEIN() का उपयोग करते समय क्रैश
* [Issue #743](https://github.com/manticoresoftware/manticoresearch/issues/743) Manticore अप्रत्याशित रूप से क्रैश करता है और सामान्य रीस्टार्ट नहीं कर सकता
* [Issue #788](https://github.com/manticoresoftware/manticoresearch/issues/788) /sql के माध्यम से CALL KEYWORDS नियंत्रण चर लौटाता है जो json को तोड़ता है
* [Issue #789](https://github.com/manticoresoftware/manticoresearch/issues/789) mariadb तालिका FEDERATED नहीं बना सकता
* [Issue #796](https://github.com/manticoresoftware/manticoresearch/issues/796) चेतावनी: dlopen() असफल: /usr/bin/lib_manticore_columnar.so: साझा वस्तु फ़ाइल नहीं खोल सकता: ऐसा कोई फ़ाइल या निर्देशिका नहीं है
* [Issue #797](https://github.com/manticoresoftware/manticoresearch/issues/797) Manticore ZONESPAN के साथ खोज करते समय क्रैश करता है जब इसे api के माध्यम से किया जाता है
* [Issue #799](https://github.com/manticoresoftware/manticoresearch/issues/799) कई इंडेक्स और facet distinct का उपयोग करते समय गलत वजन
* [Issue #801](https://github.com/manticoresoftware/manticoresearch/issues/801) SphinxQL समूह प्रश्न SQL इंडेक्स पुन: प्रसंस्करण के बाद लटकता है
* [Issue #802](https://github.com/manticoresoftware/manticoresearch/issues/802) MCL: 5.0.2 में इंडेक्सर क्रैश करता है और manticore-columnar-lib 1.15.4
* [Issue #813](https://github.com/manticoresoftware/manticoresearch/issues/813) Manticore 5.0.2 FEDERATED खाली सेट लौटाता है (MySQL 8.0.28)
* [Issue #824](https://github.com/manticoresoftware/manticoresearch/issues/824) 2 इंडेक्स पर COUNT DISTINCT का चयन करने पर जब परिणाम शून्य हो तो आंतरिक त्रुटि होती है
* [Issue #826](https://github.com/manticoresoftware/manticoresearch/issues/826) DELETE प्रश्न पर क्रैश
* [Issue #843](https://github.com/manticoresoftware/manticoresearch/issues/843) MCL: लंबे टेक्स्ट फ़ील्ड के साथ बग
* [Issue #856](https://github.com/manticoresoftware/manticoresearch/issues/856) 5.0.2 rtindex: एग्रीगेट खोज सीमा व्यवहार अपेक्षित नहीं है
* [Issue #863](https://github.com/manticoresoftware/manticoresearch/issues/863) लौटाए गए हिट Nonetype वस्तु है भले ही ऐसे खोजों के लिए जो कई परिणाम लौटानी चाहिए
* [Issue #870](https://github.com/manticoresoftware/manticoresearch/issues/870) SELECT अभिव्यक्ति में एट्रिब्यूट और स्टोर फ़ील्ड का उपयोग करते समय क्रैश
* [Issue #872](https://github.com/manticoresoftware/manticoresearch/issues/872) क्रैश के बाद तालिका अदृश्य हो जाती है
* [Issue #877](https://github.com/manticoresoftware/manticoresearch/issues/877) खोज प्रश्न में दो नकारात्मक शर्तें त्रुटि देती हैं: प्रश्न गणना नहीं किया जा सकता
* [Issue #878](https://github.com/manticoresoftware/manticoresearch/issues/878) a -b -c json query_string के माध्यम से काम नहीं कर रहा है
* [Issue #886](https://github.com/manticoresoftware/manticoresearch/issues/886) प्रश्न मिलान के साथ pseudo_sharding
* [Issue #893](https://github.com/manticoresoftware/manticoresearch/issues/893) Manticore 5.0.2 min/max फ़ंक्शन उस प्रकार से कार्य नहीं करता ...
* [Issue #896](https://github.com/manticoresoftware/manticoresearch/issues/896) फ़ील्ड "weight" सही से पार्स नहीं किया गया है
* [Issue #897](https://github.com/manticoresoftware/manticoresearch/issues/897) Manticore सेवा आरंभ के समय क्रैश होती है और बार-बार रीस्टार्ट होती है
* [Issue #900](https://github.com/manticoresoftware/manticoresearch/issues/900) j.a द्वारा समूह करना, कुछ गलत काम करता है
* [Issue #913](https://github.com/manticoresoftware/manticoresearch/issues/913) Ranker में expr का उपयोग करने पर Searchd क्रैश होता है, लेकिन केवल उन प्रश्नों के लिए जिनमें दो निकटताएँ होती हैं
* [Issue #916](https://github.com/manticoresoftware/manticoresearch/issues/916) net_throttle_action टूट गया है
* [Issue #919](https://github.com/manticoresoftware/manticoresearch/issues/919) MCL: qry निष्पादन पर Manticore क्रैश होता है और अन्य क्लस्टर रिकवरी के दौरान क्रैश होते हैं।
* [Issue #925](https://github.com/manticoresoftware/manticoresearch/issues/925) SHOW CREATE TABLE बिना बैकटिक आउटपुट करता है
* [Issue #930](https://github.com/manticoresoftware/manticoresearch/issues/930) अब JDBC कनेक्टर के माध्यम से Java से Manticore को क्वेरी करना संभव है
* [Issue #933](https://github.com/manticoresoftware/manticoresearch/issues/933) bm25f रैंकिंग समस्याएँ
* [Issue #934](https://github.com/manticoresoftware/manticoresearch/issues/934) configless इंडेक्स वॉचडॉग पर पहले-लोड स्थिति में जम गए
* [Issue #937](https://github.com/manticoresoftware/manticoresearch/issues/937) फ़ैसेट डेटा को छाँटते समय सेगफॉल्ट
* [Issue #940](https://github.com/manticoresoftware/manticoresearch/issues/940) CONCAT(TO_STRING) पर क्रैश
* [Issue #947](https://github.com/manticoresoftware/manticoresearch/issues/947) कुछ मामलों में एक साधारण सरल चयन संपूर्ण उदाहरण को स्थिर कर सकता है, इसलिए आप इसमें लॉग इन नहीं कर सकते या कोई अन्य क्वेरी नहीं चला सकते जब तक चल रहे चयन की प्रक्रिया पूरी नहीं हो जाती।
* [Issue #948](https://github.com/manticoresoftware/manticoresearch/issues/948) इंडेक्सर क्रैश
* [Issue #950](https://github.com/manticoresoftware/manticoresearch/issues/950) फैसेट विशिष्ट से गलत मात्रा
* [Issue #953](https://github.com/manticoresoftware/manticoresearch/issues/953) встроенный sph04 रेंकर में LCS गलत तरीके से गणना कर रहा है
* [Issue #955](https://github.com/manticoresoftware/manticoresearch/issues/955) 5.0.3 विकास क्रैश
* [Issue #963](https://github.com/manticoresoftware/manticoresearch/issues/963) इंजन स्तंभीय पर json के साथ FACET क्रैश
* [Issue #982](https://github.com/manticoresoftware/manticoresearch/issues/982) MCL: दूसरे स्तर के इंडेक्स से 5.0.3 क्रैश
* [PR #984](https://github.com/manticoresoftware/manticoresearch/pull/984) HandleMysqlSelectSysvar में @@autocommit
* [PR #985](https://github.com/manticoresoftware/manticoresearch/pull/985) RT इंडेक्स में थ्रेड-चंक वितरण ठीक करें
* [Issue #985](https://github.com/manticoresoftware/manticoresearch/issues/985) RT इंडेक्स में थ्रेड-चंक वितरण ठीक करें
* [Issue #986](https://github.com/manticoresoftware/manticoresearch/issues/986) गलत डिफ़ॉल्ट max_query_time
* [Issue #987](https://github.com/manticoresoftware/manticoresearch/issues/987) मल्टीथ्रेडेड निष्पादन में regex अभिव्यक्ति का उपयोग करते समय क्रैश
* [Issue #988](https://github.com/manticoresoftware/manticoresearch/issues/988) टूटे हुए पीछे के इंडेक्स संगतता
* [Issue #989](https://github.com/manticoresoftware/manticoresearch/issues/989) indextool स्तंभीय विशेषताओं की जांच करते समय त्रुटि रिपोर्ट करता है
* [Issue #990](https://github.com/manticoresoftware/manticoresearch/issues/990) json ग्रुपर क्लोन का मेमोरी लीक
* [Issue #991](https://github.com/manticoresoftware/manticoresearch/issues/991) levenshtein func क्लोनिंग का मेमोरी लीक
* [Issue #992](https://github.com/manticoresoftware/manticoresearch/issues/992) मेटा लोड करते समय त्रुटि संदेश खो गया
* [Issue #993](https://github.com/manticoresoftware/manticoresearch/issues/993) गतिशील इंडेक्स/उपकुंजी और sysvars से त्रुटियों को फैलाएँ
* [Issue #994](https://github.com/manticoresoftware/manticoresearch/issues/994) स्तंभीय संग्रह में एक स्तंभीय स्ट्रिंग पर गिनती विशिष्ट पर क्रैश
* [Issue #995](https://github.com/manticoresoftware/manticoresearch/issues/995) MCL: taxi1 से min(pickup_datetime) क्रैश करता है
* [Issue #997](https://github.com/manticoresoftware/manticoresearch/issues/997) खाली बाहरी JSON क्वेरी चयन सूची से कॉलम हटा देती है
* [Issue #998](https://github.com/manticoresoftware/manticoresearch/issues/998) वर्तमान शेड्यूलर पर चलने वाले द्वितीयक कार्य कभी-कभी असामान्य दुष्प्रभाव पैदा करते हैं
* [Issue #999](https://github.com/manticoresoftware/manticoresearch/issues/999) विभिन्न स्कीमा के साथ फैसेट विशिष्ट पर क्रैश
* [Issue #1000](https://github.com/manticoresoftware/manticoresearch/issues/1000) MCL: स्तंभीय rt इंडेक्स बिना स्तंभीय पुस्तकालय के चलाने के बाद क्षतिग्रस्त हो गया
* [Issue #1001](https://github.com/manticoresoftware/manticoresearch/issues/1001) json में निहित कटऑफ काम नहीं कर रहा है
* [Issue #1002](https://github.com/manticoresoftware/manticoresearch/issues/1002) स्तंभीय ग्रुपर समस्या
* [Issue #1003](https://github.com/manticoresoftware/manticoresearch/issues/1003) इंडेक्स से अंतिम फ़ील्ड को हटाने में असमर्थ
* [Issue #1004](https://github.com/manticoresoftware/manticoresearch/issues/1004) --new-cluster के बाद गलत व्यवहार
* [Issue #1005](https://github.com/manticoresoftware/manticoresearch/issues/1005) "स्तंभीय पुस्तकालय लोड नहीं हुआ", लेकिन इसकी आवश्यकता नहीं है
* [Issue #1006](https://github.com/manticoresoftware/manticoresearch/issues/1006) हटाने की क्वेरी के लिए कोई त्रुटि नहीं
* [Issue #1010](https://github.com/manticoresoftware/manticoresearch/issues/1010) विंडोज बिल्ड में ICU डेटा फ़ाइल स्थान सही किया गया
* [PR #1018](https://github.com/manticoresoftware/manticoresearch/pull/1018) हैंडशेक भेजने की समस्या
* [Issue #1020](https://github.com/manticoresoftware/manticoresearch/issues/1020) शो क्रिएट टेबल में id दिखाएं
* [Issue #1024 crash 1](https://github.com/manticoresoftware/manticoresearch/issues/1024) परिणामों की बड़ी संख्या के साथ फैसेट खोज पर क्रैश / विभाजन दोष।
* [Issue #1026](https://github.com/manticoresoftware/manticoresearch/issues/1026) RT इंडेक्स: जब कई दस्तावेज़ डाले जा रहे हैं और RAMchunk भरा हुआ है तब खोज "अटक" जाती है
* [Commit 4739](https://github.com/manticoresoftware/manticoresearch/commit/4739dafd) नोड्स के बीच पुनरावृत्ति के दौरान बंद होने पर थ्रेड अटक जाता है
* [Commit ab87](https://github.com/manticoresoftware/manticoresearch/commit/ab87836782e7ae43fe5f9dea739eed0d336b42c1) JSON रेंज फ़िल्टर में फ़्लोट और संख्याओं को मिलाने से Manticore फ़िल्टर को नज़रअंदाज कर सकता है
* [Commit d001](https://github.com/manticoresoftware/manticoresearch/commit/d00101c2905f6393ce3fca23c4f6dcb2506f4bd9) JSON में फ़्लोट फ़िल्टर गलत थे
* [Commit 4092](https://github.com/manticoresoftware/manticoresearch/commit/4092c6a8) यदि इंडेक्स परिवर्तित हो गया है तो बिना प्रतिबद्ध txn को छोड़ दें (या यह क्रैश हो सकता है)
* [Commit 9692](https://github.com/manticoresoftware/manticoresearch/commit/9692a840) बैकस्लेश का उपयोग करते समय क्वेरी व्याकरण त्रुटि
* [Commit 0c19](https://github.com/manticoresoftware/manticoresearch/commit/0c19ad8d) SHOW STATUS में workers_clients गलत हो सकता है
* [Commit 1772](https://github.com/manticoresoftware/manticoresearch/commit/17720e1e2f2ec38337df565bee78f7ce9d9c744a) docstores के बिना ram सेगमेंट को विलय करते समय क्रैश ठीक किया गया
* [Commit f45b](https://github.com/manticoresoftware/manticoresearch/commit/f45b337b5c265433bb7d6262ad65d02f65edbaf3) समानता JSON फ़िल्टर के लिए मिस्ड ALL/ANY स्थिति ठीक की गई
* [Commit 3e83](https://github.com/manticoresoftware/manticoresearch/commit/3e83c1f0c0e6e7a9d4297113f69a112cbabd9e64) पुनर्प्रकाशन विफल हो सकता है `got exception while reading ist stream: mkstemp(./gmb_pF6TJi) failed: 13 (Permission denied)` यदि खोज सेवा उस निर्देशिका से शुरू होती है जिसमें वह लिख नहीं सकता।
* [Commit 92e5](https://github.com/manticoresoftware/manticoresearch/commit/92e5629f7150f62524b385fca670a01ba9590f5b) 4.0.2 से दुर्घटना लॉग में केवल ऑफसेट शामिल थे। यह कमिट इसे ठीक करता है।

# संस्करण 5.0.2
रिलीज़: 30 मई 2022

### बग फिक्सेस
* ❗[समस्या #791](https://github.com/manticoresoftware/manticoresearch/issues/791) - गलत स्टैक आकार एक दुर्घटना का原因 बन सकता है।

# संस्करण 5.0.0
रिलीज़: 18 मई 2022


### प्रमुख नई विशेषताएँ
* 🔬 [Manticore कॉलम्नर लाइब्रेरी 1.15.2](https://github.com/manticoresoftware/columnar/) के लिए समर्थन, जो [गौण अनुक्रमांक](../Server_settings/Searchd.md#secondary_indexes) के बीटा संस्करण को सक्षम करता है। सामान्य और वास्तविक समय की कॉलम्नर और पंक्ति-वाइज अनुक्रमांक के लिए गौण अनुक्रमांक द्वारा डिफ़ॉल्ट रूप से सक्षम है (यदि [Manticore कॉलम्नर लाइब्रेरी](https://github.com/manticoresoftware/columnar) का उपयोग किया जा रहा है), लेकिन इसे खोजने के लिए आपको अपने कॉन्फ़िगरेशन फ़ाइल में या [SET GLOBAL](../Server_settings/Setting_variables_online.md) का उपयोग करके `secondary_indexes = 1` सेट करना होगा। यह नई कार्यक्षमता सभी ऑपरेटिंग सिस्टम में समर्थित है सिवाय पुराने Debian Stretch और Ubuntu Xenial के।
* [पठन-केवल मोड](Security/Read_only.md): अब आप श्रोता निर्दिष्ट कर सकते हैं जो केवल पढ़ने वाली क्वेरियों को संसाधित करते हैं और किसी भी लिखित डेटा को नकारते हैं।
* [/cli](../Connecting_to_the_server/HTTP.md#/cli) के लिए नया एंडपॉइंट, HTTP पर SQL क्वेरियों को चलाना और भी आसान।
* JSON के माध्यम से HTTP पर तेज़ बल्क INSERT/REPLACE/DELETE: पहले आप HTTP JSON प्रोटोकॉल के माध्यम से कई लिखने वाले कमांड प्रदान कर सकते थे, लेकिन वे एक-एक करके संसाधित होते थे, अब उन्हें एकल लेनदेन के रूप में संभाला जाता है।
* [#720](https://github.com/manticoresoftware/manticoresearch/issues/720) [नस्टेड फ़िल्टर्स](../Searching/Filters.md#Nested-bool-query) के लिए JSON प्रोटोकॉल में समर्थन। पहले आप JSON में `a=1 and (b=2 or c=3)` जैसी चीज़ें कोड नहीं कर सकते थे: `must` (AND), `should` (OR) और `must_not` (NOT) केवल सबसे ऊँचे स्तर पर काम करते थे। अब इन्हें नस्टेड किया जा सकता है।
* HTTP प्रोटोकॉल में [Chunked transfer encoding](https://en.wikipedia.org/wiki/Chunked_transfer_encoding) का समर्थन। अब आप बड़ी बैचों को ट्रांसमिट करने के लिए अपने एप्लिकेशन में chunked transfer का उपयोग कर सकते हैं जिससे संसाधन की खपत कम होती है (चूंकि `Content-Length` की गणना आवश्यक नहीं है)। सर्वर पक्ष पर, Manticore अब हमेशा स्ट्रीमिंग तरीके से इनकमिंग HTTP डेटा को संसाधित करता है, पहले की तरह पूरे बैच के स्थानांतरण की प्रतीक्षा किए बिना, जो:
  - पीक RAM उपयोग को कम करता है, ओवरमेरी ऑर्डरिंग (OOM) के जोखिम को कम करता है
  - प्रतिक्रिया समय को कम करता है (हमारे परीक्षणों ने 100MB बैच को संसाधित करने के लिए 11% की कमी दर्शायी)
  - आपको [max_packet_size](../Server_settings/Searchd.md#max_packet_size) को बायपास करने की अनुमति देता है और अधिकतम अनुमति दी गई `max_packet_size` (128MB) के मान से कहीं अधिक बड़े बैचों को ट्रांसफर करने की अनुमति देता है, उदाहरण के लिए, 1GB एक बार।
* [#719](https://github.com/manticoresoftware/manticoresearch/issues/719) `100 Continue` का HTTP इंटरफ़ेस समर्थन: अब आप `curl` से बड़े बैचों को ट्रांसफर कर सकते हैं (जिसमें विभिन्न कार्यक्रमों की भाषाओं द्वारा उपयोग की जाने वाली curl लाइब्रेरियाँ) जो डिफ़ॉल्ट रूप से `Expect: 100-continue` करती हैं और वास्तव में बैच को भेजने से पहले कुछ समय प्रतीक्षा करती हैं। पहले आपको `Expect: ` हेडर जोड़ना पड़ता था, अब इसकी आवश्यकता नहीं है।

  <details>

  पहले (प्रतिक्रिया समय पर ध्यान दें):

  ```bash
  $ time curl -v -sX POST http://localhost:9318/bulk -H "Content-Type: application/x-ndjson" --data '{"insert": {"index": "user", "doc":  {"name":"Prof. Matt Heaney IV","email":"ibergnaum@yahoo.com","description":"Tempora ullam eaque consequatur. Vero aut minima ut et ut omnis officiis vel. Molestiae quis voluptatum sint numquam.","age":15,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Prof. Boyd McKenzie","email":"carlotta11@hotmail.com","description":"Blanditiis maiores odio corporis eaque illum. Aut et rerum iste. Neque et ullam quisquam officia dignissimos quo cumque.","age":84,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Mr. Johann Smith","email":"stiedemann.tristin@ziemann.com","description":"Temporibus amet magnam consequatur omnis consequatur illo fugit. Debitis natus doloremque est tempore deserunt vero. Harum eos corrupti nemo ut.","age":89,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Hector Pouros","email":"hickle.mafalda@hotmail.com","description":" as voluptatem inventore sit. Aliquam fugit perferendis est id aut odio et sapiente.","age":64,"active":1}}}'
  *   Trying 127.0.0.1...
  * Connected to localhost (127.0.0.1) port 9318 (#0)
  > POST /bulk HTTP/1.1
  > Host: localhost:9318
  > User-Agent: curl/7.47.0
  > Accept: */*
  > Content-Type: application/x-ndjson
  > Content-Length: 1025
  > Expect: 100-continue
  >
  * Done waiting for 100-continue
  * We are completely uploaded and fine
  < HTTP/1.1 200 OK
  < Server: 4.2.0 15e927b@211223 release (columnar 1.11.4 327b3d4@211223)
  < Content-Type: application/json; charset=UTF-8
  < Content-Length: 434
  <
  * Connection #0 to host localhost left intact
  {"items":[{"insert":{"table":"user","_id":2811798918248005633,"created":true,"result":"created","status":201}},{"insert":{"table":"user","_id":2811798918248005634,"created":true,"result":"created","status":201}},{"insert":{"table":"user","_id":2811798918248005635,"created":true,"result":"created","status":201}},{"insert":{"table":"user","_id":2811798918248005636,"created":true,"result":"created","status":201}}],"errors":false}
  real	0m1.022s
  user	0m0.001s
  sys	0m0.010s
  ```

  अब: 
  ```bash
  $ time curl -v -sX POST http://localhost:9318/bulk -H "Content-Type: application/x-ndjson" --data '{"insert": {"index": "user", "doc":  {"name":"Prof. Matt Heaney IV","email":"ibergnaum@yahoo.com","description":"Tempora ullam eaque consequatur. Vero aut minima ut et ut omnis officiis vel. Molestiae quis voluptatum sint numquam.","age":15,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Prof. Boyd McKenzie","email":"carlotta11@hotmail.com","description":"Blanditiis maiores odio corporis eaque illum. Aut et rerum iste. Neque et ullam quisquam officia dignissimos quo cumque.","age":84,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Mr. Johann Smith","email":"stiedemann.tristin@ziemann.com","description":"Temporibus amet magnam consequatur omnis consequatur illo fugit. Debitis natus doloremque est tempore deserunt vero. Harum eos corrupti nemo ut.","age":89,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Hector Pouros","email":"hickle.mafalda@hotmail.com","description":" as voluptatem inventore sit. Aliquam fugit perferendis est id aut odio et sapiente.","age":64,"active":1}}}'
  *   Trying 127.0.0.1...
  * Connected to localhost (127.0.0.1) port 9318 (#0)
  > POST /bulk HTTP/1.1
  > Host: localhost:9318
  > User-Agent: curl/7.47.0
  > Accept: */*
  > Content-Type: application/x-ndjson
  > Content-Length: 1025
  > Expect: 100-continue
  >
  < HTTP/1.1 100 Continue
  < Server: 4.2.1 63e5749@220405 dev
  < Content-Type: application/json; charset=UTF-8
  < Content-Length: 0
  * We are completely uploaded and fine
  < HTTP/1.1 200 OK
  < Server: 4.2.1 63e5749@220405 dev
  < Content-Type: application/json; charset=UTF-8
  < Content-Length: 147
  <
  * Connection #0 to host localhost left intact
  {"items":[{"bulk":{"table":"user","_id":2811798919590182916,"created":4,"deleted":0,"updated":0,"result":"created","status":201}}],"errors":false}
  real	0m0.015s
  user	0m0.005s
  sys	0m0.004s
  ```

  </details>

* **⚠️ BREAKING CHANGE**: [Pseudo sharding](../Server_settings/Searchd.md#pseudo_sharding) अब डिफ़ॉल्ट रूप से सक्षम है। यदि आप इसे निष्क्रिय करना चाहते हैं तो सुनिश्चित करें कि आप अपने Manticore कॉन्फ़िगरेशन फ़ाइल के `searchd` सेक्शन में `pseudo_sharding = 0` जोड़ते हैं।
* वास्तविक समय/सादा इंडेक्स में कम से कम एक पूर्ण-पाठ क्षेत्र होना अब अनिवार्य नहीं है। आप अब Manticore का उपयोग उनकी स्थितियों में भी कर सकते हैं जिनका पूर्ण-पाठ खोज से कोई संबंध नहीं है।
* [फास्ट फेचिंग](../Creating_a_table/Data_types.md#fast_fetch) ऐसे विशेषताओं के लिए जिनका समर्थन [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) करता है: जैसे `select * from <columnar table>` क्वेरी अब पहले से कहीं अधिक तेज़ हैं, विशेष रूप से अगर स्कीमा में कई फ़ील्ड हैं।
* **⚠️ BREAKING CHANGE**: अप्रत्यक्ष [कटऑफ](../Searching/Options.md#cutoff)। Manticore अब परिणाम सेट में उन डेटा को संसाधित करने में समय और संसाधन नहीं बिता रहा है जिनकी आपको आवश्यकता नहीं है। नकारात्मक पहलू यह है कि यह [SHOW META](../Node_info_and_management/SHOW_META.md#SHOW-META) में `total_found` और JSON आउटपुट में [hits.total](../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) को प्रभावित करता है। यह केवल तब सटीक है जब आप `total_relation: eq` देखते हैं जबकि `total_relation: gte` का अर्थ है कि मेल खाने वाले दस्तावेजों की वास्तविक संख्या `total_found` मान से अधिक है जो आपने प्राप्त किया है। पिछले व्यवहार को बनाए रखने के लिए आप खोज विकल्प `cutoff=0` का उपयोग कर सकते हैं, जो `total_relation` को हमेशा `eq` बना देता है।
* **⚠️ BREAKING CHANGE**: सभी पूर्ण-पाठ क्षेत्रों को अब डिफ़ॉल्ट रूप से [स्टोर किया गया](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields) गया है। आपको सभी फ़ील्ड को गैर-स्टोर करने के लिए `stored_fields = ` (खाली मान) का उपयोग करना होगा (यानी पिछले व्यवहार पर वापस लौटें)।
* [#715](https://github.com/manticoresoftware/manticoresearch/issues/715) HTTP JSON [खोज विकल्पों](../Searching/Options.md#General-syntax) का समर्थन करता है।

### लघु परिवर्तन
* **⚠️ BREAKING CHANGE**: इंडेक्स मेटा फ़ाइल प्रारूप परिवर्तन। पहले मेटा फ़ाइलें (`.meta`, `.sph`) बाइनरी प्रारूप में थीं, अब यह केवल json है। नया Manticore संस्करण स्वचालित रूप से पुराने इंडेक्स को परिवर्तित कर देगा, लेकिन:
  - आपको ऐसा चेतावनी मिल सकता है जैसे `WARNING: ... syntax error, unexpected TOK_IDENT`
  - आप पिछले Manticore संस्करणों के साथ इंडेक्स चलाने में असमर्थ होंगे, सुनिश्चित करें कि आपके पास एक बैकअप है।
* **⚠️ BREAKING CHANGE**: [HTTP कीप-अलाइव](../Connecting_to_the_server/HTTP.md#Keep-alive) की मदद से सत्र राज्य समर्थन। जब क्लाइंट इसे भी समर्थन करता है, तो यह HTTP को स्थितिक बनाता है। उदाहरण के लिए, नए [/cli](../Connecting_to_the_server/HTTP.md#/cli) एंडपॉइंट और HTTP कीप-अलाइव (जो सभी ब्राउज़रों में डिफ़ॉल्ट रूप से चालू है) का उपयोग करके आप `SELECT` के बाद `SHOW META` बुला सकते हैं और यह उसी तरह काम करेगा जैसा यह mysql के माध्यम से काम करता है। नोट करें, पहले `Connection: keep-alive` HTTP एचेडर का समर्थन किया गया था, लेकिन इसने केवल उसी कनेक्शन को फिर से उपयोग किया। इस संस्करण से यह सत्र को स्थितिक भी बनाता है।
* आप अब `columnar_attrs = *` निर्दिष्ट कर सकते हैं ताकि आप [सादा मोड में](Read_this_first.md#Real-time-mode-vs-plain-mode) अपने सभी विशेषताओं को स्तंभाकार के रूप में परिभाषित कर सकें, जो तब उपयोगी है जब सूची लंबी हो।
* तेज़ प्रतिकृति SST
* **⚠️ BREAKING CHANGE**: प्रतिकृति प्रोटोकॉल में परिवर्तन किया गया है। यदि आप एक प्रतिकृति क्लस्टर चला रहे हैं, तो Manticore 5 में अपग्रेड करते समय आपको:
  - पहले सभी नोड्स को स्वच्छता से रोकना होगा
  - और फिर उस नोड को शुरू करें जो आखिरी बार `--new-cluster` के साथ रोका गया था (Linux में `manticore_new_cluster` उपकरण चलाएँ)।
  - अधिक जानकारी के लिए [क्लस्टर को रिस्टार्ट करना](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) के बारे में पढ़ें।
* प्रतिकृति में सुधार:
  - तेज़ SST
  - शोर प्रतिरोध जो प्रतिकृति नोड के बीच अस्थिर नेटवर्क के मामले में मदद कर सकता है
  - बेहतर लॉगिंग
* सुरक्षा में सुधार: Manticore अब `0.0.0.0` के बजाय `127.0.0.1` पर सुनता है यदि config में कोई `listen` निर्दिष्ट नहीं किया गया है। हालांकि डिफ़ॉल्ट कॉन्फ़िगरेशन में जो Manticore Search के साथ भेजा जाता है, `listen` सेटिंग निर्दिष्ट है और बिना `listen` के एक कॉन्फ़िगरेशन होना सामान्य नहीं है, फिर भी यह संभव है। पहले Manticore `0.0.0.0` पर सुनता था जो सुरक्षित नहीं है, अब यह `127.0.0.1` पर सुनता है जो आमतौर पर इंटरनेट पर नहीं खुला होता है।
* कॉलम विशेषताओं पर तेज़ संघटन।
* `AVG()` सटीकता बढ़ाई गई: पहले Manticore आंतरिक रूप से संघटनों के लिए `float` का उपयोग करता था, अब यह `double` का उपयोग करता है जो सटीकता को महत्वपूर्ण रूप से बढ़ाता है।
* JDBC MySQL ड्राइवर के लिए बेहतर समर्थन।
* [jemalloc](https://github.com/jemalloc/jemalloc) के लिए `DEBUG malloc_stats` समर्थन।
* [optimize_cutoff](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff) अब एक प्रति-तालिका सेटिंग के रूप में उपलब्ध है जिसे आप CREATE या ALTER तालिका करते समय सेट कर सकते हैं।
* **⚠️ ब्रेकिंग परिवर्तन**: [query_log_format](../Server_settings/Searchd.md#query_log_format) अब **`sphinxql` डिफ़ॉल्ट रूप से** है। यदि आप `plain` प्रारूप के आदी हैं तो आपको अपने कॉन्फ़िगरेशन फ़ाइल में `query_log_format = plain` जोड़ना होगा।
* महत्वपूर्ण मेमोरी खपत में सुधार: Manticore अब लंबे और तीव्र insert/replace/optimize कार्यभार के मामले में उपयोग किए जाने वाले फील्ड के मामले में पहले से कहीं अधिक कम RAM का उपभोग करता है।
* [shutdown_timeout](../Server_settings/Searchd.md#shutdown_timeout) का डिफ़ॉल्ट मान 3 सेकंड से बढ़ाकर 60 सेकंड कर दिया गया।
* [Commit ffd0](https://github.com/manticoresoftware/manticoresearch/commit/ffd0499d329d2c383f14c8a44c4cc84338ab56e7) Java MySQL कनेक्टर >= 6.0.3 का समर्थन: [Java mysql connection 6.0.3](https://mvnrepository.com/artifact/mysql/mysql-connector-java/6.0.3) में उन्होंने MySQL से कनेक्ट करने का तरीका बदल दिया जिससे Manticore के साथ संगतता टूट गई। नया व्यवहार अब समर्थित है।
* [Commit 1da6](https://github.com/manticoresoftware/manticoresearch/commit/1da6dbec) ने एक इंडेक्स को लोड करते समय एक नए डिस्क भाग को सहेजने को अक्षम किया (जैसे कि searchd प्रारंभिक अवस्था में)।
* [Issue #746](https://github.com/manticoresoftware/manticoresearch/issues/746) glibc >= 2.34 का समर्थन।
* [Issue #784](https://github.com/manticoresoftware/manticoresearch/issues/784) 'VIP' कनेक्शनों की गिनती सामान्य (गैर-VIP) से अलग है। पहले VIP कनेक्शन `max_connections` सीमा में गिने जाते थे, जो गैर-VIP कनेक्शनों के लिए "maxed out" त्रुटि का कारण बन सकता था। अब VIP कनेक्शनों को सीमा में नहीं गिना जाता है। VIP कनेक्शनों की वर्तमान संख्या `SHOW STATUS` और `status` में भी देखी जा सकती है।
* [ID](../Creating_a_table/Data_types.md#Document-ID) अब स्पष्ट रूप से निर्दिष्ट किया जा सकता है।
* [Issue #687](https://github.com/manticoresoftware/manticoresearch/issues/687) mysql प्रोटोकॉल के लिए zstd संपीड़न का समर्थन।

### ⚠️ अन्य छोटे ब्रेकिंग परिवर्तन
* ⚠️ BM25F सूत्र को खोज प्रासंगिकता में सुधार के लिए थोड़ा अपडेट किया गया है। यह केवल उस स्थिति में खोज परिणामों को प्रभावित करता है जब आप फ़ंक्शन [BM25F()](../Functions/Searching_and_ranking_functions.md#BM25F%28%29) का उपयोग करते हैं, यह डिफ़ॉल्ट रैंकिंग सूत्र के व्यवहार को नहीं बदलता है।
* ⚠️ REST [/sql](../Connecting_to_the_server/HTTP.md#mode=raw) एपीआई के व्यवहार में बदलाव: `/sql?mode=raw` अब एस्केपिंग की आवश्यकता होती है और एक ऐरे लौटाता है।
* ⚠️ `/bulk` INSERT/REPLACE/DELETE अनुरोधों के उत्तर के प्रारूप में बदलाव:
  - पहले प्रत्येक उप-प्रश्न एक अलग लेनदेन बनाता था और एक अलग उत्तर का परिणाम होता था
  - अब पूरा बैच एकल लेनदेन माना जाता है, जो एकल उत्तर लौटाता है
* ⚠️ खोज विकल्प `low_priority` और `boolean_simplify` अब एक मान (`0/1`) की आवश्यकता है: पहले आप `SELECT ... OPTION low_priority, boolean_simplify` कर सकते थे, अब आपको `SELECT ... OPTION low_priority=1, boolean_simplify=1` करना आवश्यक है।
* ⚠️ यदि आप पुराने [php](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.php), [python](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.py) या [java](https://github.com/manticoresoftware/manticoresearch/tree/master/api/java) ग्राहकों का उपयोग कर रहे हैं तो कृपया संबंधित लिंक का पालन करें और एक अपडेटेड संस्करण खोजें। **पुराने संस्करण Manticore 5 के साथ पूरी तरह से संगत नहीं हैं।**
* ⚠️ HTTP JSON अनुरोध अब `query_log_format=sphinxql` मोड में अलग प्रारूप में लॉग किए जाते हैं। पहले केवल पूर्ण-पाठ भाग लॉग किया जाता था, अब इसे जैसा है वैसा लॉग किया जाता है।

### नए पैकेज
* **⚠️ ब्रेकिंग परिवर्तन**: नई संरचना के कारण जब आप Manticore 5 में अपग्रेड करते हैं तो नए पैकेज स्थापित करने से पहले पुराने पैकेजों को हटाना अनुशंसित है:
  - RPM-आधारित: `yum remove manticore*`
  - Debian और Ubuntu: `apt remove manticore*`
* नए deb/rpm पैकेज संरचना। पिछले संस्करणों ने प्रदान किया:
  - `manticore-server` जिसमें `searchd` (मुख्य खोज डेमॉन) और इसके लिए सभी आवश्यक चीजें शामिल हैं
  - `manticore-tools` जिसमें `indexer` और `indextool` शामिल हैं
  - `manticore` जिसमें सब कुछ शामिल है
  - `manticore-all` RPM एक मेटा पैकेज है जो `manticore-server` और `manticore-tools` की ओर संकेत करता है

  नई संरचना है:
  - `manticore` - deb/rpm मेटा पैकेज जो उपरोक्त सभी को निर्भरता के रूप में स्थापित करता है
  - `manticore-server-core` - `searchd` और इसे अकेले चलाने के लिए सब कुछ
  - `manticore-server` - systemd फ़ाइलें और अन्य पूरक स्क्रिप्ट
  - `manticore-tools` - `indexer`, `indextool` और अन्य उपकरण
  - `manticore-common` - डिफ़ॉल्ट कॉन्फ़िगरेशन फ़ाइल, डिफ़ॉल्ट डेटा निर्देशिका, डिफ़ॉल्ट स्टॉपवर्ड्स
  - `manticore-icudata`, `manticore-dev`, `manticore-converter` में ज्यादा बदलाव नहीं हुए हैं
  - `.tgz` बंडल जिसमें सभी पैकेज शामिल हैं
* Ubuntu Jammy का समर्थन
* Amazon Linux 2 का समर्थन [YUM repo](../Installation/RHEL_and_Centos.md#YUM-repository) के माध्यम से।

### बग फिक्स
* [Issue #815](https://github.com/manticoresoftware/manticoresearch/issues/815) UDF फ़ंक्शन का उपयोग करते समय बेतरतीब ढंग से क्रैश होना।
* [Issue #287](https://github.com/manticoresoftware/manticoresearch/issues/287) RT इंडेक्स बनाते समय मेमोरी खत्म होना।
* [Issue #604](https://github.com/manticoresoftware/manticoresearch/issues/604) ब्रेकिंग चेंज 3.6.0, 4.2.0 स्पिनक्सक्वल-Parser
* [Issue #667](https://github.com/manticoresoftware/manticoresearch/issues/667) फATAL: मेमोरी खत्म (9007199254740992 बाइट आवंटित करने में असमर्थ)
* [Issue #676](https://github.com/manticoresoftware/manticoresearch/issues/676) स्ट्रिंग्स UDFs को सही ढंग से पास नहीं की गई
* ❗[Issue #698](https://github.com/manticoresoftware/manticoresearch/issues/698) टेक्स्ट कॉलम को rt इंडेक्स में जोड़ने की कोशिश करने के बाद Searchd क्रैश होता है
* [Issue #705](https://github.com/manticoresoftware/manticoresearch/issues/705) इंडेक्सर सभी कॉलम नहीं खोज सका
* ❗[Issue #709](https://github.com/manticoresoftware/manticoresearch/issues/705) json.boolean द्वारा समूह बनाना गलत काम करता है
* [Issue #716](https://github.com/manticoresoftware/manticoresearch/issues/716) इंडेक्सटूल कमांड जो इंडेक्स से संबंधित हैं (जैसे --dumpdict) विफलता
* ❗[Issue #724](https://github.com/manticoresoftware/manticoresearch/issues/724) चयन से फ़ील्ड गायब हो जाते हैं
* [Issue #727](https://github.com/manticoresoftware/manticoresearch/issues/727) .NET HttpClient Content-Type असंगतता जब `application/x-ndjson` का उपयोग किया जाता है
* [Issue #729](https://github.com/manticoresoftware/manticoresearch/issues/729) फ़ील्ड लंबाई की गणना
* ❗[Issue #730](https://github.com/manticoresoftware/manticoresearch/issues/730) create/insert into/drop कॉलम टेबल में मेमोरी लीक होती है
* [Issue #731](https://github.com/manticoresoftware/manticoresearch/issues/731) कुछ परिस्थितियों में परिणामों में खाली कॉलम
* ❗[Issue #749](https://github.com/manticoresoftware/manticoresearch/issues/749) स्टार्ट पर डेमॉन का क्रैश होना
* ❗[Issue #750](https://github.com/manticoresoftware/manticoresearch/issues/750) स्टार्ट पर डेमॉन लटकता है
* ❗[Issue #751](https://github.com/manticoresoftware/manticoresearch/issues/751) SST पर क्रैश
* [Issue #752](https://github.com/manticoresoftware/manticoresearch/issues/752) Json विशेषता को columnar के रूप में चिह्नित किया गया जब engine='columnar'
* [Issue #753](https://github.com/manticoresoftware/manticoresearch/issues/753) प्रतिकृति 0 पर सुनती है
* [Issue #754](https://github.com/manticoresoftware/manticoresearch/issues/754) columnar_attrs = * csvpipe के साथ काम नहीं कर रहा है
* ❗[Issue #755](https://github.com/manticoresoftware/manticoresearch/issues/755) rt में कॉलमर में select float पर क्रैश
* ❗[Issue #756](https://github.com/manticoresoftware/manticoresearch/issues/756) इंडेक्सटूल चेक के दौरान rt इंडेक्स बदलता है
* [Issue #757](https://github.com/manticoresoftware/manticoresearch/issues/757) श्रोताओं के पोर्ट रेंज इंटरसेक्शन के लिए जांच की आवश्यकता है
* [Issue #758](https://github.com/manticoresoftware/manticoresearch/issues/758) यदि RT इंडेक्स डिस्क भाग को सेव करने में विफल रहा तो मूल त्रुटि लॉग करें
* [Issue #759](https://github.com/manticoresoftware/manticoresearch/issues/759) केवल एक त्रुटि RE2 कॉन्फ़िग के लिए रिपोर्ट की गई
* ❗[Issue #760](https://github.com/manticoresoftware/manticoresearch/issues/760) कमिट 5463778558586d2508697fa82e71d657ac36510f में RAM उपभोग में परिवर्तन
* [Issue #761](https://github.com/manticoresoftware/manticoresearch/issues/761) 3रा नोड गंदे रिस्टार्ट के बाद एक गैर-प्राथमिक क्लस्टर नहीं बनाता है
* [Issue #762](https://github.com/manticoresoftware/manticoresearch/issues/762) अपडेट काउंटर 2 से बढ़ जाता है
* [Issue #763](https://github.com/manticoresoftware/manticoresearch/issues/763) नया संस्करण 4.2.1 4.2.0 के साथ बनाये गए विकृतियों के साथ भ्रष्ट इंडेक्स
* [Issue #764](https://github.com/manticoresoftware/manticoresearch/issues/764) json कुंजी /sql?mode=raw में कोई एस्केपिंग नहीं
* ❗[Issue #765](https://github.com/manticoresoftware/manticoresearch/issues/765) फ़ंक्शन का उपयोग करने से अन्य मान छिप जाते हैं
* ❗[Issue #766](https://github.com/manticoresoftware/manticoresearch/issues/766) FixupAttrForNetwork में एक पंक्ति द्वारा ट्रिगर किया गया मेमोरी लीक
* ❗[Issue #767](https://github.com/manticoresoftware/manticoresearch/issues/767) 4.2.0 और 4.2.1 में docstore कैश से संबंधित मेमोरी लीक
* [Issue #768](https://github.com/manticoresoftware/manticoresearch/issues/768) नेटवर्क पर संग्रहीत फ़ील्ड के साथ अजीब पिंग-पोंग
* [Issue #769](https://github.com/manticoresoftware/manticoresearch/issues/769) lemmatizer_base 'common' अनुभाग में उल्लेख न होने पर खाली पर रीसेट होता है
* [Issue #770](https://github.com/manticoresoftware/manticoresearch/issues/770) pseudo_sharding द्वारा id से SELECT धीमा होता है
* [Issue #771](https://github.com/manticoresoftware/manticoresearch/issues/771) DEBUG malloc_stats जब jemalloc का उपयोग करते हैं तो ज़ीरो आउटपुट
* [Issue #772](https://github.com/manticoresoftware/manticoresearch/issues/772) Drop/add कॉलम मान को अदृश्य बनाता है
* [Issue #773](https://github.com/manticoresoftware/manticoresearch/issues/773) कॉलमर टेबल में कॉलम bit(N) जोड़ना असंभव
* [Issue #774](https://github.com/manticoresoftware/manticoresearch/issues/774) "क्लस्टर" manticore.json में स्टार्ट पर खाली हो जाता है
* ❗[Commit 1da4](https://github.com/manticoresoftware/manticoresearch/commits/1da4ce891e4ff13727cfd3331d72771c64ee7949) HTTP क्रियाएं SHOW STATUS में ट्रैक नहीं की जाती हैं
* [Commit 3810](https://github.com/manticoresoftware/manticoresearch/commits/381000ab4af4c3c98ec8c730699ad7f39039cec8) कम दर के एकल कीवर्ड प्रश्नों के लिए pseudo_sharding को अक्षम करें
* [Commit 8003](https://github.com/manticoresoftware/manticoresearch/commits/800325cca283a87801b28929d82420fafc76a0ee) संग्रहीत गुण बनाम इंडेक्स मर्ज को ठीक किया
* [Commit cddf](https://github.com/manticoresoftware/manticoresearch/commits/cddfeed6296f2041c6aae18414e5cd9cabf08281) सामान्यीकृत अद्वितीय मूल्य फ़ेचर्स; कॉलमर स्ट्रिंग्स के लिए विशेष प्रकार के अद्वितीय फ़ेचर्स जोड़े
* [Commit fba4](https://github.com/manticoresoftware/manticoresearch/commits/fba4bb4f) docstore से शून्य पूर्णांक गुणों को लाने के लिए ठीक किया
* [Commit f300](https://github.com/manticoresoftware/manticoresearch/commit/f3009a9242fa16956adf871c4887e6e0303ba364) `ranker` को क्वेरी लॉग में दो बार निर्दिष्ट किया जा सकता है

## संस्करण 4.2.0, 23 दिसंबर 2021

### प्रमुख नई विशेषताएं
* **रीयल-टाइम इंडेक्स और पूर्ण-टेक्स्ट क्वेरीज़ के लिए छद्म-शार्डिंग समर्थन**। पिछले रिलीज़ में हमने सीमित छद्म शार्डिंग समर्थन जोड़ा था। इस संस्करण से आप छद्म शार्डिंग और अपने मल्टी-कोर प्रोसेसर के सभी लाभ प्राप्त कर सकते हैं, बस [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding) को सक्रिय करके। सबसे अच्छी बात यह है कि आपको इसके लिए अपने इंडेक्स या क्वेरीज़ के साथ कुछ भी करने की आवश्यकता नहीं है, बस इसे सक्षम करें और यदि आपके पास फ्री CPU है, तो इसका उपयोग आपके प्रतिक्रिया समय को कम करने के लिए किया जाएगा। यह पूर्ण-टेक्स्ट, फ़िल्टरिंग और विश्लेषणात्मक क्वेरीज़ के लिए साधारण और रीयल-टाइम इंडेक्स का समर्थन करता है। उदाहरण के लिए, यहां बताया गया है कि कैसे छद्म शार्डिंग को सक्षम करने से अधिकांश क्वेरीज़ का **प्रतिक्रिया समय औसतन लगभग 10x कम** हो सकता है [Hacker news curated comments dataset](https://zenodo.org/record/45901/) पर 100 गुना गुणा किया गया (एक साधारण इंडेक्स में 116 मिलियन दस्तावेज़)।

![Pseudo sharding on vs off in 4.2.0](4.2.0_ps_on_vs_off.png)

* [**डेबियन बुल्सआई**](https://manticoresearch.com/install/) अब समर्थित है।

<!-- example pq_transactions_4.2.0 -->
* PQ लेनदेन अब आणविक और अलगावित हैं। पहले PQ लेनदेन का समर्थन सीमित था। यह **PQ में तेजी से REPLACE** को सक्षम करता है, विशेष रूप से जब आपको एक साथ बहुत सारे नियमों को बदलने की आवश्यकता होती है। प्रदर्शन विवरण:

<!-- intro -->
#### 4.0.2

<!-- request 4.0.2 -->

इसमें **1M PQ नियमों को डालने में 48 सेकंड** लगते हैं और **40K को 10K बैच में REPLACE करने में 406 सेकंड** लगते हैं।

```sql
root@perf3 ~ # mysql -P9306 -h0 -e "drop table if exists pq; create table pq (f text, f2 text, j json, s string) type='percolate';"; date; for m in `seq 1 1000`; do (echo -n "insert into pq (id,query,filters,tags) values "; for n in `seq 1 1000`; do echo -n "(0,'@f (cat | ( angry dog ) | (cute mouse)) @f2 def', 'j.json.language=\"en\"', '{\"tag1\":\"tag1\",\"tag2\":\"tag2\"}')"; [ $n != 1000 ] && echo -n ","; done; echo ";")|mysql -P9306 -h0; done; date; mysql -P9306 -h0 -e "select count(*) from pq"

Wed Dec 22 10:24:30 AM CET 2021
Wed Dec 22 10:25:18 AM CET 2021
+----------+
| count(*) |
+----------+
|  1000000 |
+----------+

root@perf3 ~ # date; (echo "begin;"; for offset in `seq 0 10000 30000`; do n=0; echo "replace into pq (id,query,filters,tags) values "; for id in `mysql -P9306 -h0 -NB -e "select id from pq limit $offset, 10000 option max_matches=1000000"`; do echo "($id,'@f (tiger | ( angry bear ) | (cute panda)) @f2 def', 'j.json.language=\"de\"', '{\"tag1\":\"tag1\",\"tag2\":\"tag2\"}')"; n=$((n+1)); [ $n != 10000 ] && echo -n ","; done; echo ";"; done; echo "commit;") > /tmp/replace.sql; date
Wed Dec 22 10:26:23 AM CET 2021
Wed Dec 22 10:26:27 AM CET 2021
root@perf3 ~ # time mysql -P9306 -h0 < /tmp/replace.sql

real	6m46.195s
user	0m0.035s
sys	0m0.008s
```

<!-- intro -->

#### 4.2.0

<!-- request 4.2.0 -->

इसमें **1M PQ नियमों को डालने में 34 सेकंड** लगते हैं और **उन्हें 10K बैच में REPLACE करने में 23 सेकंड** लगते हैं।

```sql
root@perf3 ~ # mysql -P9306 -h0 -e "drop table if exists pq; create table pq (f text, f2 text, j json, s string) type='percolate';"; date; for m in `seq 1 1000`; do (echo -n "insert into pq (id,query,filters,tags) values "; for n in `seq 1 1000`; do echo -n "(0,'@f (cat | ( angry dog ) | (cute mouse)) @f2 def', 'j.json.language=\"en\"', '{\"tag1\":\"tag1\",\"tag2\":\"tag2\"}')"; [ $n != 1000 ] && echo -n ","; done; echo ";")|mysql -P9306 -h0; done; date; mysql -P9306 -h0 -e "select count(*) from pq"

Wed Dec 22 10:06:38 AM CET 2021
Wed Dec 22 10:07:12 AM CET 2021
+----------+
| count(*) |
+----------+
|  1000000 |
+----------+

root@perf3 ~ # date; (echo "begin;"; for offset in `seq 0 10000 990000`; do n=0; echo "replace into pq (id,query,filters,tags) values "; for id in `mysql -P9306 -h0 -NB -e "select id from pq limit $offset, 10000 option max_matches=1000000"`; do echo "($id,'@f (tiger | ( angry bear ) | (cute panda)) @f2 def', 'j.json.language=\"de\"', '{\"tag1\":\"tag1\",\"tag2\":\"tag2\"}')"; n=$((n+1)); [ $n != 10000 ] && echo -n ","; done; echo ";"; done; echo "commit;") > /tmp/replace.sql; date
Wed Dec 22 10:12:31 AM CET 2021
Wed Dec 22 10:14:00 AM CET 2021
root@perf3 ~ # time mysql -P9306 -h0 < /tmp/replace.sql

real	0m23.248s
user	0m0.891s
sys	0m0.047s
```

<!-- end -->

### मामूली परिवर्तन
*  [optimize_cutoff](Server_settings/Searchd.md#optimize_cutoff) अब `searchd` अनुभाग में एक कॉन्फ़िगरेशन विकल्प के रूप में उपलब्ध है। जब आप अपने सभी इंडेक्स में RT चंक्स की संख्या को वैश्विक स्तर पर एक विशेष संख्या तक सीमित करना चाहें, तो यह उपयोगी है।
* [Commit 0087](https://github.com/manticoresoftware/manticoresearch/commit/00874743823eaf43e2a09a088392fd7e0a617f47) सटीक [count(distinct ...)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) और [FACET ... distinct](Searching/Faceted_search.md#Faceting-without-duplicates) कई स्थानीय भौतिक इंडेक्स (रीयल-टाइम/साधारण) पर समान फ़ील्ड सेट/आदेश के साथ।
* [PR #598](https://github.com/manticoresoftware/manticoresearch/pull/598) `YEAR()` और अन्य टाइमस्टाम्प फ़ंक्शंस के लिए bigint समर्थन।
* [Commit 8e85](https://github.com/manticoresoftware/manticoresearch/commit/8e85d4bce596cc88f9d9615ea243a27e4afde0f7) अनुकूलन [rt_mem_limit](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit)। पहले, Manticore Search डेटा को `rt_mem_limit` तक इकट्ठा कर रहा था, इससे पहले कि एक नया डिस्कChunk डिस्क पर सहेजा जाए, और जब इसे सहेजा जा रहा था तब यह 10% अधिक (जिसे डबल-बफर कहा जाता है) इकट्ठा कर रहा था ताकि संभावित सम्मिलन निलंबन को न्यूनतम किया जा सके। यदि वह सीमा भी समाप्त हो गई, तो नए दस्तावेज़ जोड़ने को तब तक रोक दिया गया जब तक डिस्कChunk पूरी तरह से डिस्क पर सहेजा नहीं गया। नई अनुकूलन सीमा इस तथ्य पर आधारित है कि हम अब [auto-optimize](Server_settings/Searchd.md#auto_optimize) हैं, इसलिए यह कोई बड़ी बात नहीं है यदि डिस्कChunks पूरी तरह से `rt_mem_limit` का सम्मान नहीं करते हैं और पहले डिस्कChunk को फ्लश करना शुरू करते हैं। तो, अब हम `rt_mem_limit` के 50% तक इकट्ठा करते हैं और इसे एक डिस्कChunk के रूप में सहेजते हैं। सहेजने पर हम सांख्यिकी पर देखते हैं (हमने कितना सहेजा है, कितने नए दस्तावेज़ सहेजते समय आए हैं) और प्रारंभिक दर की पुनঃगणना करते हैं जो अगली बार उपयोग की जाएगी। उदाहरण के लिए, यदि हमने 90 मिलियन दस्तावेज़ सहेजे हैं, और 10 मिलियन नए दस्तावेज़ सहेजते समय आए हैं, तो दर 90% है, इसलिए हम जानते हैं कि अगली बार हम `rt_mem_limit` का 90% तक इकट्ठा कर सकते हैं इससे पहले कि एक और डिस्कChunk को फ्लश करना शुरू कर सकें। दर मूल्य को स्वचालित रूप से 33.3% से 95% के बीच गणना की जाती है।
* [Issue #628](https://github.com/manticoresoftware/manticoresearch/issues/628) [unpack_zlib](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Database_connection.md#unpack_zlib) PostgreSQL स्रोत के लिए। धन्यवाद, [Dmitry Voronin](https://github.com/dimv36) के लिए [योगदान](https://github.com/manticoresoftware/manticoresearch/pull/630)।
* [Commit 6d54](https://github.com/manticoresoftware/manticoresearch/commit/6d54cf2b319b131970c29410cc21abcbdf8083b1) `indexer -v` और `--version`। पहले आप अभी भी इंडेक्सर का संस्करण देख सकते थे, लेकिन `-v`/`--version` समर्थित नहीं थे।
* [Issue #662](https://github.com/manticoresoftware/manticoresearch/issues/662) डिफ़ॉल्ट रूप से अनंत mlock सीमा जब Manticore systemd के माध्यम से प्रारंभ किया जाता है।
* [Commit 63c8](https://github.com/manticoresoftware/manticoresearch/commit/63c8cd05cf399e705c2c14939411a92cb784735a) स्पिनलॉक -> कोरॉ rwlock के लिए ऑप कतार।
* [Commit 4113](https://github.com/manticoresoftware/manticoresearch/commit/41130ce30d1734bbf8e6b8b63c4e815caf9464f2) पर्यावरण चर `MANTICORE_TRACK_RT_ERRORS` RT सेगमेंट भ्रष्टाचार पर डिबगिंग के लिए उपयोगी है।

### टूटी हुई परिवर्तन
* बिनलॉग संस्करण बढ़ा दिया गया था, पिछले संस्करण का बिनलॉग पुन: चलाया नहीं जाएगा, इसलिए सुनिश्चित करें कि आप अपग्रेड के दौरान Manticore Search को साफ़ रूप से बंद करें: पिछले उदाहरण को रोकने के बाद `/var/lib/manticore/binlog/` में कोई बिनलॉग फ़ाइलें `binlog.meta` के अलावा नहीं होनी चाहिए।
* [Commit 3f65](https://github.com/manticoresoftware/manticoresearch/commit/3f659f36e87d99ee262205a8eae4663a255be630) `show threads option format=all` में नया कॉलम "चेन"। यह कुछ कार्य सूचना टिकटों का ढेर दिखाता है, जो प्रोफाइलिंग की जरूरतों के लिए सबसे उपयोगी है, इसलिए यदि आप `show threads` आउटपुट को पार्स कर रहे हैं तो नए कॉलम के बारे में ध्यान रखें।
* `searchd.workers` 3.5.0 से पुराने हो गए थे, अब यह अस्वीकृत है, यदि आप इसे अपनी कॉन्फ़िगरेशन फ़ाइल में रखते हैं तो यह स्टार्ट पर एक चेतावनी को प्रेरित करेगा। Manticore Search शुरू होगा, लेकिन चेतावनी के साथ।
* यदि आप Manticore तक पहुँचने के लिए PHP और PDO का उपयोग करते हैं तो आपको `PDO::ATTR_EMULATE_PREPARES` करना होगा।

### बग सुधार
* ❗[Issue #650](https://github.com/manticoresoftware/manticoresearch/issues/650) Manticore 4.0.2 Manticore 3.6.3 से धीमा। 4.0.2 बल्क इन्सर्ट के मामले में पिछले संस्करणों की तुलना में तेज़ था, लेकिन एकल दस्तावेज़ इन्सर्ट के लिए महत्वपूर्ण रूप से धीमा। इसे 4.2.0 में ठीक किया गया है।
* ❗[Commit 22f4](https://github.com/manticoresoftware/manticoresearch/commit/22f4141bed1411c83684ae6e0bae832071cce3df) RT सूचकांक को गहन REPLACE लोड के तहत भ्रष्ट हो सकता है, या यह क्रैश हो सकता है।
* [Commit 03be](https://github.com/manticoresoftware/manticoresearch/commit/03be91e4f720a3f20bee0278dd8f0d44abd3ef0e) समूहक और समूह N सॉर्टर में विलय के दौरान औसत को ठीक किया; सामग्रियों की संयोजना को ठीक किया।
* [Commit 2ea5](https://github.com/manticoresoftware/manticoresearch/commit/2ea575d34efe38661f14d858f2b552456f29fe42) `indextool --check` दुर्घटनाग्रस्त हो सकता है।
* [Commit 7ec7](https://github.com/manticoresoftware/manticoresearch/commit/7ec76d4abf01b5fca6f10a0000fe64d5d8e5fa52) UPDATE के कारण RAM समाप्ति की समस्या।
* [Commit 658a](https://github.com/manticoresoftware/manticoresearch/commit/658a727e41f3c3fbcde9754149528f34ed76ba19) डेमन INSERT पर लटक सकता है।
* [Commit 46e4](https://github.com/manticoresoftware/manticoresearch/commit/46e42b9b386e31e53247ed634e158b533e28063e) डेमन शटडाउन पर लटक सकता है।
* [Commit f8d7](https://github.com/manticoresoftware/manticoresearch/commit/f8d7d5177cc83d250b493b4cfc619732b29cf072) डेमन शटडाउन पर दुर्घटनाग्रस्त हो सकता है।
* [Commit 733a](https://github.com/manticoresoftware/manticoresearch/commit/733accf165142a1d1574689525ccca8907971158) डेमन क्रैश पर लटक सकता है।
* [Commit f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) डेमन प्रारंभ में शटडाउन हो सकता है जब यह अमान्य नोड्स सूची के साथ क्लस्टर में फिर से शामिल होने की कोशिश करता है।
* [Commit 1401](https://github.com/manticoresoftware/manticoresearch/commit/140155613821ecaddcd1a3dfc4f7369b0df6aef7) वितरित सूचकांक को RT मोड में पूरी तरह से भुला दिया जा सकता है यदि यह स्टार्ट पर अपने एजेंटों में से एक को हल नहीं कर सका।
* [Issue #683](https://github.com/manticoresoftware/manticoresearch/issues/683) attr bit(N) engine='columnar' विफल।
* [Issue #682](https://github.com/manticoresoftware/manticoresearch/issues/682) तालिका बनाना विफल रहता है, लेकिन डीर छोड़ देता है।
* [Issue #663](https://github.com/manticoresoftware/manticoresearch/issues/663) कॉन्फ़िगरेशन विफल रहता है: अज्ञात कुंजी नाम 'attr_update_reserve'।
* [Issue #632](https://github.com/manticoresoftware/manticoresearch/issues/632) बैच क्वेरी पर Manticore क्रैश।
* [Issue #679](https://github.com/manticoresoftware/manticoresearch/issues/679) बैच क्वेरी फिर से क्रैश का कारण बन रही है v4.0.3
* [Commit f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) ने अमान्य नोड्स सूची के साथ क्लस्टर में फिर से शामिल होने की कोशिश करते समय डेमन क्रैश को ठीक किया
* [Issue #643](https://github.com/manticoresoftware/manticoresearch/issues/643) Manticore 4.0.2 बैच इंसर्शन के बाद कनेक्शन स्वीकार नहीं करता
* [Issue #635](https://github.com/manticoresoftware/manticoresearch/issues/635) ORDER BY JSON.field या स्ट्रिंग एट्रिब्यूट के साथ FACET क्वेरी क्रैश कर सकती है
* [Issue #634](https://github.com/manticoresoftware/manticoresearch/issues/634) पैक किए गए फैक्टर्स के साथ क्वेरी पर क्रैश SIGSEGV
* [Commit 4165](https://github.com/manticoresoftware/manticoresearch/commits/41657f15f8fcf1e93cfad7f835bcf705d6e8349e) morphology_skip_fields को तालिका बनाने द्वारा समर्थित नहीं था

## Version 4.0.2, Sep 21 2021

### प्रमुख नए फीचर्स
- **[Manticore Columnar Library](https://github.com/manticoresoftware/columnar) का पूर्ण समर्थन**। पहले Manticore Columnar Library केवल साधारण इंडेक्स के लिए समर्थित था। अब इसका समर्थन किया गया है:
  - वास्तविक समय के इंडेक्स में `INSERT`, `REPLACE`, `DELETE`, `OPTIMIZE`
  - प्रतिकृति में
  - `ALTER` में
  - `indextool --check` में
- **स्वचालित इंडेक्स संकुचन** ([Issue #478](https://github.com/manticoresoftware/manticoresearch/issues/478))। अंततः, आपको मैन्युअल रूप से या क्रॉनटास्क के माध्यम से या किसी अन्य प्रकार की स्वचालन के द्वारा OPTIMIZE करने की आवश्यकता नहीं है। Manticore अब यह आपके लिए स्वचालित रूप से और डिफ़ॉल्ट रूप से करता है। आप [optimize_cutoff](../Server_settings/Setting_variables_online.md) वैश्विक चर के माध्यम से डिफ़ॉल्ट संकुचन सीमा सेट कर सकते हैं।
- **Chunk स्नैपशॉट और लॉक सिस्टम का नवीनीकरण**। ये परिवर्तन पहली नज़र में बाहर से अदृश्य हो सकते हैं, लेकिन वे वास्तविक समय के इंडेक्स में कई चीजों के व्यवहार को काफी बेहतर बनाते हैं। संक्षेप में, पहले अधिकतर Manticore डेटा हेरफेर ऑपरेशंस ने लॉक पर भारी निर्भर किया, अब हम इसके बजाय डिस्क चंक स्नैपशॉट का उपयोग करते हैं।
- **वास्तविक समय के इंडेक्स में बल्क INSERT प्रदर्शन में महत्वपूर्ण तेजी**। उदाहरण के लिए, [Hetzner के सर्वर AX101](https://www.hetzner.com/dedicated-rootserver/ax101) के साथ SSD, 128 GB RAM और AMD के Ryzen™ 9 5950X (16*2 कोर) **के साथ 3.6.0 में आप 236K डॉक्स प्रति सेकंड** एक तालिका में डाल सकते हैं जिसका स्कीमा `name text, email string, description text, age int, active bit(1)` (डिफ़ॉल्ट `rt_mem_limit`, बैच का आकार 25000, 16 समवर्ती इनसर्ट श्रमिक, कुल 16 मिलियन डॉक्स डाले गए)। 4.0.2 में वही समवर्तीता/बैच/गिनती **357K डॉक्स प्रति सेकंड** देती है।

  <details>

  - पढ़ी जाने वाली ऑपरेशन (जैसे SELECTs, प्रतिकृति) स्नैपशॉट के साथ की जाती हैं
  - ऑपरेशन जो केवल आंतरिक इंडेक्स संरचना को बिना स्कीमा/दस्तावेज़ों में संशोधन किए बदलते हैं (जैसे RAM खंडों का विलय, डिस्क चंक्स को सहेजना, डिस्क चंक्स का विलय) केवल पढ़ने योग्य स्नैपशॉट के साथ की जाती हैं और अंत में मौजूदा चंक्स को प्रतिस्थापित करती हैं
  - UPDATE और DELETE मौजूदा चंक्स के खिलाफ किए जाते हैं, लेकिन विलय के मामले में, जो हो सकता है, लेखन एकत्र किए जाते हैं और फिर उन्हें नए चंक्स के खिलाफ लागू किया जाता है
  - UPDATE हर चंक के लिए अनुक्रमिक रूप से एक विशिष्ट लॉक प्राप्त करता है। विलय चंक से गुणों को इकट्ठा करने के चरण में प्रवेश करते समय एक साझा लॉक प्राप्त करता है। इसलिए एक ही समय में केवल एक (विलय या अपडेट) ऑपरेशन चंक के गुणों तक पहुंच रखता है।
  - जब विलय उस चरण में पहुंचता है, जब उसे गुणों की आवश्यकता होती है, तो यह एक विशेष ध्वज निर्धारित करता है। जब UPDATE समाप्त होता है, यह ध्वज की जांच करता है, और यदि यह सेट है, तो पूरा अपडेट एक विशेष संग्रह में संग्रहीत किया जाता है। अंततः, जब विलय समाप्त होता है, यह नवजात डिस्क चंक पर सेट किए गए अपडेट लागू करता है।
  - ALTER एक विशेष लॉक के माध्यम से चलता है
  - प्रतिकृति एक सामान्य पढ़ने की ऑपरेशन के रूप में चलती है, लेकिन इसके अलावा SST से पहले गुणों को सहेजती है और SST के दौरान अपडेट को रोकती है

  </details>

- **[ALTER](Updating_table_schema_and_settings.md) एक पूर्ण-पाठ फ़ील्ड जोड़/हटाने की अनुमति देता है** (RT मोड में)। पहले यह केवल एक एट्रिब्यूट जोड़ने/हटाने की अनुमति देता था।
- 🔬 **प्रायोगिक: पूर्ण-स्कैन क्वेरी के लिए छद्म-शार्डिंग** - किसी भी गैर-पूर्ण-पाठ खोज क्वेरी को समानांतर बनाने की अनुमति देता है। हाथ से शार्ड तैयार करने के बजाय, आप अब बस नए विकल्प [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding) को सक्षम कर सकते हैं और गैर-पूर्ण-पाठ खोज क्वेरियों के लिए `CPU कोर` कम प्रतिक्रिया समय की अपेक्षा कर सकते हैं। ध्यान दें कि यह सभी मौजूदा CPU कोर को आसानी से उपयोग कर सकता है, इसलिए यदि आप केवल विलंबता के अलावा थ्रूपुट की भी परवाह करते हैं - इसका उपयोग सावधानी से करें।

### छोटे परिवर्तन
<!-- example -->
- Linux Mint और Ubuntu Hirsute Hippo को [APT repository](Installation/Debian_and_Ubuntu.md#APT-repository) के माध्यम से समर्थित किया जाता है
- कुछ मामलों में बड़े इंडेक्स में HTTP के माध्यम से id द्वारा तेज़ अपडेट (IDs वितरण पर निर्भर करता है)
- [671e65a2](https://github.com/manticoresoftware/lemmatizer-uk/commit/671e65a296800537123a3f8b324eeaa3f58b9632) - lemmatizer-uk में कैशिंग जोड़ी गई


<!-- intro -->
#### 3.6.0

<!-- request 3.6.0 -->
```
time curl -X POST -d '{"update":{"index":"idx","id":4611686018427387905,"doc":{"mode":0}}}' -H "Content-Type: application/x-ndjson" http://127.0.0.1:6358/json/bulk

real    0m43.783s
user    0m0.008s
sys     0m0.007s
```

<!-- intro -->
#### 4.0.2

<!-- request 4.0.2 -->
```
time curl -X POST -d '{"update":{"index":"idx","id":4611686018427387905,"doc":{"mode":0}}}' -H "Content-Type: application/x-ndjson" http://127.0.0.1:6358/json/bulk

real    0m0.006s
user    0m0.004s
sys     0m0.001s
```
<!-- end -->
- [custom startup flags for systemd](Starting_the_server/Linux.md#Custom-startup-flags-using-systemd)। अब आपको कुछ विशिष्ट स्टार्टअप फ्लैग के साथ Manticore चलाने के मामले में searchd को मैन्युअल रूप से शुरू करने की आवश्यकता नहीं है
- नया फ़ंक्शन [LEVENSHTEIN()](Functions/String_functions.md#LEVENSHTEIN%28%29) जो लेवेन्स्टाइन दूरी की गणना करता है
- नए [searchd startup flags](Starting_the_server/Manually.md#searchd-command-line-options) `--replay-flags=ignore-trx-errors` और `--replay-flags=ignore-all-errors` जोड़े गए ताकि आप अभी भी searchd शुरू कर सकें यदि बिनलॉग क्षतिग्रस्त हो
- [Issue #621](https://github.com/manticoresoftware/manticoresearch/issues/621) - RE2 से त्रुटियों को उजागर करें
- स्थानीय प्लेन इंडेक्स से बने वितरित इंडेक्स के लिए अधिक सटीक [COUNT(DISTINCT)](Searching/Grouping.md#COUNT%28DISTINCT-field%29)
- फ़ैसटेड खोज करते समय डुप्लिकेट हटाने के लिए [FACET DISTINCT](Searching/Faceted_search.md#Faceting-without-duplicates)
- [सटीक रूप संशोधन](Searching/Full_text_matching/Operators.md#Exact-form-modifier) अब [मॉरफोलॉजी](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) की आवश्यकता नहीं है और [इनफ़िक्स/प्रिफ़िक्स](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) खोज सक्षम वाले इंडेक्स पर कार्य करता है

### ब्रेकिंग परिवर्तन
- नया संस्करण पुराने इंडेक्स पढ़ सकता है, लेकिन पुराने संस्करण Manticore 4 के इंडेक्स को नहीं पढ़ सकते हैं
- आईडी द्वारा निहित क्रमबद्धता हटा दी गई है। यदि आवश्यक हो तो स्पष्ट रूप से क्रमबद्ध करें
- `charset_table` का डिफ़ॉल्ट मान `0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451` से `non_cjk` में परिवर्तित होता है
- `OPTIMIZE` स्वचालित रूप से होता है। यदि आपको इसकी आवश्यकता नहीं है, तो सुनिश्चित करें कि कॉन्फ़िगरेशन फ़ाइल में `searchd` अनुभाग में `auto_optimize=0` सेट करें
- [Issue #616](https://github.com/manticoresoftware/manticoresearch/issues/616) `ondisk_attrs_default` को निष्क्रिय किया गया था, अब उन्हें हटा दिया गया है
- योगदानकर्ताओं के लिए: अब हम Linux निर्माण के लिए Clang कंपाइलर का उपयोग करते हैं क्योंकि हमारे परीक्षण के अनुसार, यह एक तेज़ Manticore सर्च और Manticore कॉलम पुस्तकालय बना सकता है
- यदि खोज क्वेरी में [max_matches](Searching/Options.md#max_matches) निर्दिष्ट नहीं है, तो इसे नए कॉलम भंडारण के प्रदर्शन के लिए सबसे कम आवश्यक मान के साथ निहित रूप से अपडेट किया जाता है। यह [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) में मेट्रिक `total` को प्रभावित कर सकता है, लेकिन `total_found` को नहीं जो कि पाए गए दस्तावेज़ों की वास्तविक संख्या है।

### Manticore 3 से माइग्रेशन
- सुनिश्चित करें कि आप Manticore 3 को साफ़ तरीके से रोकें:
  - `/var/lib/manticore/binlog/` में कोई बिनलॉग फ़ाइलें नहीं होनी चाहिए (केवल `binlog.meta` को निर्देशिका में होना चाहिए)
  - अन्यथा, संकेत नहीं चलेंगे जिनके लिए Manticore 4 बिनलॉग का उत्तर नहीं दे सकता
- नया संस्करण पुराने इंडेक्स पढ़ सकता है, लेकिन पुराने संस्करण Manticore 4 के इंडेक्स को नहीं पढ़ सकते हैं, इसलिए सुनिश्चित करें कि आप एक बैकअप बनाएं यदि आप नए संस्करण को आसानी से रोलबैक करना चाहते हैं
- यदि आप एक प्रतिकृति क्लस्टर चलाते हैं, तो सुनिश्चित करें कि आप:
  - पहले सभी नोड्स को साफ तरीके से रोकें
  - और फिर उस नोड को शुरू करें जिसे अंतिम रूप से `--new-cluster` के साथ रोका गया था (Linux में टूल `manticore_new_cluster` चलाएं)।
  - अधिक विवरण के लिए [क्लस्टर को पुनः आरंभ करने](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) के बारे में पढ़ें

### बग ठीक करना
- कई प्रतिकृति मुद्दे हल किए गए हैं:
  - [Commit 696f](https://github.com/manticoresoftware/manticoresearch/commit/696f8649535f2b5285b878c9581c0d2b1139ae09) - सक्रिय इंडेक्स के साथ जॉइनर पर SST के दौरान क्रैश को ठीक किया; फ़ाइल के टुकड़ों को लिखते समय जॉइनर नोड पर sha1 सत्यापन जोड़ा गया ताकि इंडेक्स लोडिंग को तेज़ किया जा सके; इंडेक्स लोड पर जॉइनर नोड पर परिवर्तन के इंडेक्स फ़ाइलों का रोटेशन जोड़ा गया; नए इंडेक्स द्वारा सक्रिय इंडेक्स के प्रतिस्थापन पर जॉइनर नोड पर इंडेक्स फ़ाइलों का निष्कासन जोड़ा गया; फ़ाइलों और टुकड़ों को भेजने के लिए डोनर नोड पर प्रतिकृति लॉग बिंदु जोड़ा गया
  - [Commit b296](https://github.com/manticoresoftware/manticoresearch/commit/b296c55af68314a8fc66f8b104e7301d673c3b68) - यदि पता गलत है तो JOIN CLUSTER पर क्रैश
  - [Commit 418b](https://github.com/manticoresoftware/manticoresearch/commit/418bf880a5e05ffbb68f311a1c23006a9df3220e) - एक बड़े इंडेक्स के प्रारंभिक प्रतिकृति के दौरान, जॉइनिंग नोड `ERROR 1064 (42000): invalid GTID, (null)` के साथ विफल हो सकता है, डोनर दूसरे नोड के जॉइन करने के दौरान अनुत्तरीय हो सकता है
  - [Commit 6fd3](https://github.com/manticoresoftware/manticoresearch/commit/6fd350d28e4905211cf8a7298cbf7cdda101c83d) - एक बड़े इंडेक्स के लिए हैश गलत तरीके से गणना की जा सकती है जो प्रतिकृति विफलता का कारण बन सकती है
  - [Issue #615](https://github.com/manticoresoftware/manticoresearch/issues/615) - क्लस्टर पुनः आरंभ करने पर प्रतिकृति विफल
- [Issue #574](https://github.com/manticoresoftware/manticoresearch/issues/574) - `indextool --help` पैरामीटर `--rotate` को प्रदर्शित नहीं करता
- [Issue #578](https://github.com/manticoresoftware/manticoresearch/issues/578) - एक दिन के बाद Idle रहने पर searchd का उच्च CPU उपयोग
- [Issue #587](https://github.com/manticoresoftware/manticoresearch/issues/587) - .meta को तुरंत फ्लश करें
- [Issue #617](https://github.com/manticoresoftware/manticoresearch/issues/617) - manticore.json खाली हो जाता है
- [Issue #618](https://github.com/manticoresoftware/manticoresearch/issues/618) - root के तहत searchd --stopwait विफल। यह systemctl के व्यवहार को भी ठीक करता है (पहले यह ExecStop के लिए विफलता दिखा रहा था और searchd को ठीक से बंद करने के लिए पर्याप्त समय नहीं दे रहा था)
- [Issue #619](https://github.com/manticoresoftware/manticoresearch/issues/619) - INSERT/REPLACE/DELETE बनाम SHOW STATUS। `command_insert`, `command_replace` और अन्य गलत मेट्रिक्स दिखा रहे थे
- [Issue #620](https://github.com/manticoresoftware/manticoresearch/issues/620) - एक प्लेन इंडेक्स के लिए `charset_table` का गलत डिफ़ॉल्ट मान था
- [Commit 8f75](https://github.com/manticoresoftware/manticoresearch/commit/8f7536887d4b58a5bab5647fc067b2d0482c2038) - नए डिस्क टुकड़ों को mlocked नहीं किया जाता
- [Issue #607](https://github.com/manticoresoftware/manticoresearch/issues/607) - Manticore क्लस्टर नोड का नाम द्वारा नोड को हल करने में असफल होने पर क्रैश होता है
- [Issue #623](https://github.com/manticoresoftware/manticoresearch/issues/623) - अपडेटेड इंडेक्स की प्रतिकृति असंगठित स्थिति में जा सकती है
- [Commit ca03](https://github.com/manticoresoftware/manticoresearch/commit/ca03d2280c5197e9b311f0f03f551e3b702a8130) - इंडेक्सर को एक json विशेषता वाले प्लेन इंडेक्स स्रोत को इंडेक्स करते समय हैंग कर सकता है
- [Commit 53c7](https://github.com/manticoresoftware/manticoresearch/commit/53c753054558ff3f82c7af365273d32537c5338b) - PQ इंडेक्स पर समान नहीं होने वाले एक्सप्रेशन फ़िल्टर को ठीक किया
- [Commit ccf9](https://github.com/manticoresoftware/manticoresearch/commit/ccf94e022ba6763d6286ab5bac767b6f6a846846) - 1000 मेल से ऊपर की सूची क्वेरी में विंडो चयन ठीक किया गया। `SELECT * FROM pq ORDER BY id desc LIMIT 1000 , 100 OPTION max_matches=1100` पहले काम नहीं कर रहा था
- [Commit a048](https://github.com/manticoresoftware/manticoresearch/commit/a0483fe9bef3c45dffe96f5df4f4382aee29c072) - Manticore के लिए HTTPS अनुरोध "max packet size(8388608) exceeded" जैसी चेतावनी उत्पन्न कर सकता है
- [Issue #648](https://github.com/manticoresoftware/manticoresearch/issues/648) - Manticore 3 कुछ स्ट्रिंग गुणों के अपडेट के बाद लटका सकता है


## Version 3.6.0, 3 मई 2021
**Manticore 4 से पहले का रखरखाव रिलीज**

### प्रमुख नए फीचर्स
- सामान्य संकेतकों के लिए [Manticore Columnar Library](https://github.com/manticoresoftware/columnar/) का समर्थन। सामान्य संकेतकों के लिए नया सेटिंग [columnar_attrs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs)
- [यूक्रेनी लेम्माटाइज़र](https://github.com/manticoresoftware/lemmatizer-uk) का समर्थन
- पूरी तरह से पुनर्प्रस्तावित Histogram। जब एक संकेतक बनाया जाता है, तो Manticore इसमें प्रत्येक क्षेत्र के लिए Histogram भी बनाता है, जिसका उपयोग वह तेज़ फ़िल्टरिंग के लिए करता है। 3.6.0 में एल्गोरिदम को पूरी तरह से पुनरावलोकित किया गया था और यदि आपके पास बहुत सारे डेटा हैं और आप बहुत फ़िल्टर करते हैं तो आप अधिक प्रदर्शन प्राप्त कर सकते हैं।

### छोटे परिवर्तन
- `manticore_new_cluster [--force]` उपकरण जो systemd के माध्यम से एक प्रतिकृति क्लस्टर को पुनरारंभ करने में सहायक
- [--drop-src](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Merging_tables.md#Merging-tables) `indexer --merge` के लिए
- [नया मोड](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) `blend_mode='trim_all'`
- बैकटिक के साथ JSON पथ को एस्केप करने के लिए [समर्थन जोड़ा गया](Searching/Full_text_matching/Escaping.md#Escaping-json-node-names-in-SQL)
- [indextool --check](Miscellaneous_tools.md#indextool) RT मोड में काम कर सकता है
- [FORCE/IGNORE INDEX(id)](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-SQL) का उपयोगSELECT/UPDATE के लिए
- अब मर्ज किए गए डिस्क टुकड़े के लिए अलग पहचानकर्ता
- [indextool --check-disk-chunk CHUNK_NAME](Miscellaneous_tools.md#indextool)

### ऑप्टिमाइज़ेशन
- [तेज JSON पार्सिंग](https://github.com/manticoresoftware/manticoresearch/commit/0c25fc1788e3f77f60cb7fb8f1623d8466bba606), हमारे परीक्षणों से पता चलता है कि `WHERE json.a = 1` जैसे प्रश्नों पर 3-4% कम विलंबता
- स्वचालित शार्दिंग/पुनर्संतुलन के लिए एक पूर्वनिर्धारित गैर-प्रलेखित कमांड `DEBUG SPLIT`

### बग फिक्स
- [Issue #584](https://github.com/manticoresoftware/manticoresearch/issues/524) - गलत और अस्थिर FACET परिणाम
- [Issue #506](https://github.com/manticoresoftware/manticoresearch/issues/506) - MATCH के उपयोग के दौरान अजीब व्यवहार: जो लोग इस समस्या से पीड़ित हैं, उन्हें संकेतक को पुनर्निर्माण करने की आवश्यकता है क्योंकि समस्या संकेतक बनाने के चरण में थी
- [Issue #387](https://github.com/manticoresoftware/manticoresearch/issues/387) - SNIPPET() फ़ंक्शन के साथ क्वेरी चलाते समय मध्यवर्ती कोर डंप
- जटिल क्वेरी को संसाधित करने के लिए उपयोगी स्टैक ऑप्टिमाइज़ेशन:
  - [Issue #469](https://github.com/manticoresoftware/manticoresearch/issues/469) - SELECT परिणाम CRASH DUMP में
  - [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) - फ़िल्टर पेड़ों के लिए स्टैक आकार पहचान
- [Issue #461](https://github.com/manticoresoftware/manticoresearch/issues/461) - IN स्थिति का उपयोग करके अपडेट सही तरीके से लागू नहीं होता है
- [Issue #464](https://github.com/manticoresoftware/manticoresearch/issues/464) - CALL PQ के तुरंत बाद SHOW STATUS लौटाता है - [Issue #481](https://github.com/manticoresoftware/manticoresearch/issues/481) - स्थैतिक बाइनरी निर्माण ठीक किया गया
- [Issue #502](https://github.com/manticoresoftware/manticoresearch/issues/502) - मल्टी-क्वेरी में बग
- [Issue #514](https://github.com/manticoresoftware/manticoresearch/issues/514) - 'create table' का उपयोग करते समय कॉलम के लिए असामान्य नामों का उपयोग नहीं कर सकते
- [Commit d1db](https://github.com/manticoresoftware/manticoresearch/commit/d1dbe771288328c963ca8512f564c6b7cf4f466c) - स्ट्रिंग गुण के अपडेट के साथ replay binlog पर डॉमन क्रैश; बिनलॉग संस्करण को 10 पर सेट करें
- [Commit 775d](https://github.com/manticoresoftware/manticoresearch/commit/775d0555562f121911d3c30085947e743fadac2d)  - अभिव्यक्ति स्टैक फ्रेम पहचान समय के दौरान ठीक हुआ (परीक्षण 207)
- [Commit 4795](https://github.com/manticoresoftware/manticoresearch/commit/4795dc49194c7745761bb5006ecb266e8a9224fa)  - खाली संग्रहित क्वेरी के लिए फिल्टर और टैग ख़ाली थे (परीक्षण 369)
- [Commit c3f0](https://github.com/manticoresoftware/manticoresearch/commit/c3f0bf4dca0e4eae6ea899dd566878df57eba085)  - लंबी विलंबता और उच्च त्रुटि दर के साथ नेटवर्क में पुनरुत्पादन SST प्रवाह का टूटना (विभिन्न डेटा केंद्रों की पुनरुत्पादन); पुनरुत्पादन कमांड संस्करण को 1.03 पर अपडेट किया गया
- [Commit ba2d](https://github.com/manticoresoftware/manticoresearch/commit/ba2d6619b8107f3fb177741bbdff3709bac248cd)  - क्लस्टर में शामिल होने के बाद लेखन संचालन पर जॉइनर लॉक क्लस्टर (परीक्षण 385)
- [Commit de4d](https://github.com/manticoresoftware/manticoresearch/commit/de4dcb9ff6c316bc2a9ef6c62eb64769f04ee103)  - सटीक संशोधक के साथ वाइल्डकार्ड मिलान (परीक्षण 321)
- [Commit 6524](https://github.com/manticoresoftware/manticoresearch/commit/6524fc6af032a0932bd42c37874a619e1fb7f075)  - docid चेकपॉइंट बनाम docstore
- [Commit f4ab](https://github.com/manticoresoftware/manticoresearch/commit/f4ab83c291e65a5873d271c67b5491f629544f1d)  - अमान्य xml को पार्स करते समय असंगत संकेतक व्यवहार
- [Commit 7b72](https://github.com/manticoresoftware/manticoresearch/commit/7b727e22fe01b1a180d4a8d3a3fea92c18eac707)  - Stored percolate query with NOTNEAR हमेशा के लिए चलता है (परीक्षण 349)
- [Commit 812d](https://github.com/manticoresoftware/manticoresearch/commit/812dab745f0c972ecde5d5b3d4a313cd6e1e143e)  - वाइल्डकार्ड से शुरू होने वाले वाक्यांश के लिए गलत वजन
- [Commit 1771](https://github.com/manticoresoftware/manticoresearch/commit/1771afc669c75670b3511bf02ab9bfc8982b94d2)  - वाइल्डकार्ड के साथ पर्कोलेट क्वेरी मैचिंग कॉज़ेस इंटरलीव्ड हिट्स उत्पन्न करते हैं और मैचिंग को तोड़ते हैं (टेस्ट 417)
- [Commit aa0d](https://github.com/manticoresoftware/manticoresearch/commit/aa0d8c2bcb41428d41dbfb66e42a872a79a4ee4b)  - पैरललाइज्ड क्वेरी के मामले में 'कुल' की गणना ठीक की गई
- [Commit 18d8](https://github.com/manticoresoftware/manticoresearch/commit/18d81b3cfbb930b7361dae645fe13283d4fbf6ef)  - डेमोन पर कई समवर्ती सत्रों के साथ विंडोज़ में क्रैश
- [Commit 8443](https://github.com/manticoresoftware/manticoresearch/commit/84432f23d6fbf7a7d0b67c8c9412eb5933b03596)  - कुछ अनुक्रमणिका सेटिंग्स को पुन: उत्पन्न नहीं किया जा सका
- [Commit 9341](https://github.com/manticoresoftware/manticoresearch/commit/93411fe6af4246ca4acce758944d513a1660b92b)  - नए इवेंट्स जोड़ने की उच्च दर पर नेटलूप कभी-कभी फ्रीज हो जाता है क्योंकि परमाणु 'किक' इवेंट को एक समय में कई इवेंट्स के लिए एक बार संसाधित किया जा रहा है और उनसे वास्तविक क्रियाएं खो दी जा रही हैं
क्वेरी की स्थिति, सर्वर स्थिति नहीं
- [Commit d805](https://github.com/manticoresoftware/manticoresearch/commit/d805fc125244ef08d14fb3d1826e2c356862d539)  - नए फ्लश किए गए डिस्क चंक को प्रतिबद्धता पर खोया जा सकता है
- [Commit 63cb](https://github.com/manticoresoftware/manticoresearch/commit/63cbf008d50979cdf7062664fb02a672a3bb1131)  - प्रोफाइलर में 'नेट_read' गलत है
- [Commit f537](https://github.com/manticoresoftware/manticoresearch/commit/f5379bb27234b25919966c50ccb6f469ead646e2)  - अरबी (दाएं से बाएं टेक्स्ट) के साथ पर्कोलेट समस्या
- [Commit 49ee](https://github.com/manticoresoftware/manticoresearch/commit/49eeb4202d4ce4875c07c35087d7108c1e177328)  - डुप्लिकेट कॉलम नाम पर आईडी सही ढंग से नहीं उठाई गई
- [Commit refa](https://github.com/manticoresoftware/manticoresearch/commit/4b7df262bbfec9b9e95a44e72ad326ac8a786c49) नेटवर्क इवेंट्स को दुर्लभ मामलों में क्रैश ठीक करने के लिए
- [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) ''indextool --dumpheader'' में fix
- [Commit ff71](https://github.com/manticoresoftware/manticoresearch/commit/ff7163532ed39042c0d3c3c2b902ff41f94d7f0c) - TRUNCATE WITH RECONFIGURE स्टोर किए गए फ़ील्ड के साथ गलत काम करता है

### प्रमुख परिवर्तनों:
- नया बिनलॉग प्रारूप: आपको अपग्रेड करने से पहले Manticore को साफ-सुथरा रोकने की आवश्यकता है
- अनुक्रमणिका प्रारूप थोड़ा बदलता है: नया संस्करण आपकी मौजूदा अनुक्रमणिकाओं को ठीक से पढ़ सकता है, लेकिन यदि आप 3.6.0 से पूर्व संस्करण में डाउनग्रेड करने का निर्णय लेते हैं तो नए अनुक्रमणिकाएं अप्राप्य होंगी
- पुनर्प्रजनन प्रारूप परिवर्तन: पुरानी संस्करण से 3.6.0 और इसके विपरीत पुन: उत्पन्न न करें, सभी नोड्स पर एक बार नए संस्करण पर स्विच करें
- ''reverse_scan'' अप्रचलित है। सुनिश्चित करें कि आप अपनी क्वेरियों में इस विकल्प का उपयोग न करें क्योंकि 3.6.0 से वे अन्यथा विफल हो जाएंगे
- इस रिलीज़ से हम RHEL6, Debian Jessie और Ubuntu Trusty के लिए अब और निर्माण प्रदान नहीं करते हैं। यदि आपके लिए उनका समर्थन प्राप्त करना महत्वपूर्ण है तो [हमसे संपर्क करें](https://manticoresearch.com/contact-us/)

### अप्रचलितताएँ
- आईडी द्वारा किसी और को स्पष्ट रूप से क्रमबद्ध नहीं किया गया। यदि आप इस पर निर्भर हैं तो सुनिश्चित करें कि अपनी क्वेरियों को तदनुसार अपडेट करें
- खोज विकल्प ''reverse_scan'' अप्रचलित हो गया है

## संस्करण 3.5.4, 10 दिसंबर 2020

### नई विशेषताएँ
- नए पायथन, जावास्क्रिप्ट और जावा क्लाइंट अब सामान्य रूप से उपलब्ध हैं और इस मैनुअल में अच्छी तरह से दस्तावेजीकृत हैं।
- एक वास्तविक समय की अनुक्रमणिका का डिस्क चंक स्वतः गिराना। यह ऑप्टिमाइजेशन एक वास्तविक समय की अनुक्रमणिका को ऑप्टिमाइज करते समय एक डिस्क चंक को स्वतः गिराने की अनुमति देता है जब चंक स्पष्ट रूप से अब और आवश्यक नहीं है (सभी दस्तावेज़ दबा दिए गए हैं)। पहले, इसने विलय की आवश्यकता की, अब चंक को बस तुरंत गिराया जा सकता है। [कटऑफ](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) विकल्प की अनदेखी की जाती है, यानी भले ही कुछ वास्तव में विलय न किया जाए, एक अप्रचलित डिस्क चंक हटा दिया जाता है। यह उपयोगी है अगर आप अपनी अनुक्रमणिका में रिटेंशन बनाए रखते हैं और पुराने दस्तावेज़ों को हटाते हैं। अब ऐसी अनुक्रमणिकाओं का संकुचन तेज होगा।
- [स्टैंडअलोन NOT](Searching/Options.md#not_terms_only_allowed) के लिए SELECT का एक विकल्प

### मामूली परिवर्तन
- [समस्या #453](https://github.com/manticoresoftware/manticoresearch/issues/453) नया विकल्प [indexer.ignore_non_plain=1](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#ignore_non_plain) उपयोगी है यदि आप ''indexer --all'' चलाते हैं और आपके पास कॉन्फ़िगरेशन फ़ाइल में केवल सीधे अनुक्रमणिकाएं नहीं हैं। बिना 'ignore_non_plain=1' के आपको एक चेतावनी और उचित निकास कोड मिलेगा।
- [SHOW PLAN ... OPTION format=dot](Node_info_and_management/Profiling/Query_plan.md#Dot-format-for-SHOW-PLAN) और [EXPLAIN QUERY ... OPTION format=dot](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) पूर्ण-पाठ क्वेरी योजना निष्पादन का दृश्यकरण सक्षम करते हैं। जटिल क्वेरियों को समझने के लिए उपयोगी।

### अप्रचलितताएँ
- ''indexer --verbose'' अप्रचलित है क्योंकि इसने कभी भी अनुक्रमणिका आउटपुट में कुछ भी नहीं जोड़ा
- वॉचडॉग के बैकट्रेस सिग्नल के लिए अब ''USR2'' का उपयोग किया जाना है इसके स्थान पर ''USR1'' है

### बग सुधार
- [समस्या #423](https://github.com/manticoresoftware/manticoresearch/issues/423) सायरिकल चर पीरियड कॉल स्निपेट्स रिटेन मोड हाइलाइट नहीं करते हैं
- [समस्या #435](https://github.com/manticoresoftware/manticoresearch/issues/435) RTINDEX - GROUP N BY एक्सप्रेशन चयन = घातक क्रैश
- [Commit 2b3b](https://github.com/manticoresoftware/manticoresearch/commit/2b3b62bd0ce2b54544733d524a169e81f71b5388) searchd स्थिति क्लस्टर में सेगमेंटेशन फॉल्ट दिखाती है
- [Commit 9dd2](https://github.com/manticoresoftware/manticoresearch/commit/9dd25c193ed2a53750a0e0e68566fd2b833fcb0c) 'SHOW INDEX index.N SETTINGS' 9 से > चंक को संबोधित नहीं करता है
- [समस्या #389](https://github.com/manticoresoftware/manticoresearch/issues/389) बग जो Manticore को क्रैश करता है
- [Commit fba1](https://github.com/manticoresoftware/manticoresearch/commit/fba166178ff35c73a0c70d131cf3aca4a635e961) परिवर्तक टूटे हुए अनुक्रमणिकाएं बनाता है
- [Commit eecd](https://github.com/manticoresoftware/manticoresearch/commit/eecd61d861c742471175be19a5e5455034b5db5f) stopword_step=0 बनाम CALL SNIPPETS()
- [Commit ea68](https://github.com/manticoresoftware/manticoresearch/commit/ea6850e449601199190f38a387c04b8a7b7e90f6) स्थानीय अनुक्रमांक पर कम max_matches पर count distinct 0 लौटाता है
- [Commit 362f](https://github.com/manticoresoftware/manticoresearch/commit/362f27db3421f09a41c6cce04b5baace00fa4c4d) समरूपता का उपयोग करते समय स्टोर किए गए पाठ हिट में वापस नहीं आते हैं


## संस्करण 3.5.2, 1 अक्टूबर 2020

### नई विशेषताएँ

* OPTIMIZE डिस्क टुकड़ों को टुकड़ों की संख्या (डिफ़ॉल्ट `2* No. of cores`) में कम करता है, बजाय एक ही टुकड़े के। टुकड़ों की इष्टतम संख्या [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) विकल्प द्वारा नियंत्रित की जा सकती है।
* NOT ऑपरेटर अब स्वतंत्र रूप से उपयोग किया जा सकता है। डिफ़ॉल्ट रूप से यह डिसेबल है क्योंकि आकस्मिक एकल NOT प्रश्न धीमे हो सकते हैं। इसे नए searchd निर्देश [not_terms_only_allowed](Server_settings/Searchd.md#not_terms_only_allowed) को `0` सेट करके सक्षम किया जा सकता है।
* नया सेटिंग [max_threads_per_query](Server_settings/Searchd.md#max_threads_per_query) सेट करता है कि एक प्रश्न कितने थ्रेड का उपयोग कर सकता है। यदि निर्देश सेट नहीं किया गया है, तो एक प्रश्न [threads](Server_settings/Searchd.md#threads) के मान तक थ्रेड का उपयोग कर सकता है।
`SELECT` प्रश्न के अनुसार थ्रेड की संख्या [OPTION threads=N](Searching/Options.md#threads) के साथ सीमित की जा सकती है, जो वैश्विक `max_threads_per_query` को अधिकृत करती है।
* Percolate अनुक्रमांक अब [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md) के माध्यम से आयात किए जा सकते हैं।
* HTTP API `/search` नए क्वेरी नोड `aggs` द्वारा [फैसेटिंग](Searching/Faceted_search.md#HTTP-JSON)/[समूह बनाना](Searching/Grouping.md) के लिए बुनियादी समर्थन प्राप्त करता है।

### छोटी परिवर्तन

* यदि कोई प्रतिकृति सुनने का निर्देश घोषित नहीं किया गया है, तो इंजन निर्धारित 'sphinx' पोर्ट के बाद के पोर्ट का उपयोग करने का प्रयास करेगा, 200 तक।
* `listen=...:sphinx` को SphinxSE कनेक्शनों या SphinxAPI क्लाइंट के लिए स्पष्ट रूप से सेट करने की आवश्यकता है।
* [SHOW INDEX STATUS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) नए मेट्रिक्स: `killed_documents`, `killed_rate`, `disk_mapped_doclists`, `disk_mapped_cached_doclists`, `disk_mapped_hitlists` और `disk_mapped_cached_hitlists` को आउटपुट करता है।
* SQL कमांड `status` अब `Queue\Threads` और `Tasks\Threads` को आउटपुट करता है।

### धीरे-धीरे रद्द करने:

* `dist_threads` अब पूरी तरह से समाप्त हो चुका है, searchd चेतावनी का लॉग बनाएगा यदि निर्देश अभी भी उपयोग किया जाता है।

### डॉकर

आधिकारिक डॉकर छवि अब Ubuntu 20.04 LTS पर आधारित है

### पैकेजिंग

सामान्य `manticore` पैकेज के अलावा, आप घटकों द्वारा Manticore Search भी स्थापित कर सकते हैं:

- `manticore-server-core` - `searchd`, मैनपेज, लॉग डायरेक्टरी, API और गलेरा मॉड्यूल उपलब्ध कराता है। यह `manticore-common` को भी निर्भरता के रूप में स्थापित करेगा।
- `manticore-server` - कोर (init.d, systemd) के लिए स्वचालन स्क्रिप्ट्स और `manticore_new_cluster` रैपर उपलब्ध कराता है। यह `manticore-server-core` को भी निर्भरता के रूप में स्थापित करेगा।
- `manticore-common` - कॉन्फ़िग, स्टॉपवर्ड्स, सामान्य दस्तावेज़ और कंकाल फ़ोल्डर (datadir, modules, आदि) प्रदान करता है।
- `manticore-tools` - सहायक उपकरण ( `indexer`, `indextool` आदि), उनके मैनपेज और उदाहरण प्रदान करता है। यह `manticore-common` को भी निर्भरता के रूप में स्थापित करेगा।
- `manticore-icudata` (RPM) या `manticore-icudata-65l` (DEB) - icu रूपविज्ञान उपयोग के लिए ICU डेटा फ़ाइल प्रदान करता है।
- `manticore-devel` (RPM) या `manticore-dev` (DEB) - UDFs के लिए डेवलपर हेडर प्रदान करता है।

### बग फिक्सेस

1. [Commit 2a47](https://github.com/manticoresoftware/manticoresearch/commit/2a474dc1a26e8b0f8aaaae95669caf2f1d4b7746) विभिन्न टुकड़ों के साथ RT अनुक्रमांक पर समूहकर्ता में डेमोन का क्रैश
2. [Commit 57a1](https://github.com/manticoresoftware/manticoresearch/commit/57a19e5ad5663ef0ca7436595218fb1221d28c8e) खाली रिमोट दस्तावेज़ों के लिए फास्टपाथ
3. [Commit 07dd](https://github.com/manticoresoftware/manticoresearch/commit/07dd3f313c63fb82c22092f9907ef24e3475250e) अभिव्यक्ति स्टैक फ्रेम पहचान रनटाइम
4. [Commit 08ae](https://github.com/manticoresoftware/manticoresearch/commit/08ae357cf1012bc8e2da54c20b205b592efda3d4) परकोलेट अनुक्रमांक पर 32 से अधिक क्षेत्रों का मेल खाना
5. [Commit 16b9](https://github.com/manticoresoftware/manticoresearch/commit/16b9390fd4cdb07a77ac4497adb935573a1710e5) प्रतिकृति सुनने के पोर्ट रेंज
6. [Commit 5fa6](https://github.com/manticoresoftware/manticoresearch/commit/5fa671affeacb6441a59c8a88479bfd423df7c81) pq पर तालिका बनाना दिखाएं
7. [Commit 54d1](https://github.com/manticoresoftware/manticoresearch/commit/54d133b6449105a9fb0168db3f1fbb05fb5aa1f6) HTTPS पोर्ट व्यवहार
8. [Commit fdbb](https://github.com/manticoresoftware/manticoresearch/commit/fdbbe5245cc296cc5c1ae3ae2fb9cb08fb66a248) प्रतिस्थापन करते समय docstore पंक्तियों का मिश्रण
9. [Commit afb5](https://github.com/manticoresoftware/manticoresearch/commit/afb53f648ef4e64fa1776c58e66f6e716ac730ad) TFO अनुपलब्ध संदेश स्तर को 'info' में बदलें
10. [Commit 59d9](https://github.com/manticoresoftware/manticoresearch/commit/59d94cefc6e88af360b8046726a16b7eaa9f0b71) strcmp अमान्य उपयोग पर क्रैश
11. [Commit 04af](https://github.com/manticoresoftware/manticoresearch/commit/04af0349c1e784c2230d204592d6272b440bb375) सिस्टम (स्टॉपवर्ड) फ़ाइलों के साथ क्लस्टर में अनुक्रमांक जोड़ना
12. [Commit 5014](https://github.com/manticoresoftware/manticoresearch/commit/50148b4eae9f3a924082005cf5d2f9c8339af7f5) बड़े शब्दकोशों के साथ अनुक्रमांक मिलाना; बड़े डिस्क टुकड़ों का RT ऑप्टिमाइजेशन
13. [Commit a2ad](https://github.com/manticoresoftware/manticoresearch/commit/a2adf1582843f63d4dcc1dab8f7598e0917094a2) Indextool वर्तमान संस्करण से मेटा को डंप कर सकता है
14. [Commit 69f6](https://github.com/manticoresoftware/manticoresearch/commit/69f6d5f77c64b97e3f395aea8c555b11c77fb732) GROUP N में समूह क्रम में समस्या
15. [Commit 24d5](https://github.com/manticoresoftware/manticoresearch/commit/24d5d80f508576794b751d70f9f9b524564e3c14) हैंडशेक के बाद SphinxSE के लिए स्पष्ट फ्लश
16. [Commit 31c4](https://github.com/manticoresoftware/manticoresearch/commit/31c4d78ac03ee85d9a02b86075448a04ac80f9e2) जब आवश्यक न हो तो बड़े विवरण की कॉपी से बचें
17. [Commit 2959](https://github.com/manticoresoftware/manticoresearch/commit/2959e2caa8610f98fcdf2e5a82160f58687f4a27) थ्रेड प्रदर्शित करते समय नकारात्मक समय
18. [Commit f0b3](https://github.com/manticoresoftware/manticoresearch/commit/f0b357102c70f3ac1d7da260c60b67322940e647) टोकन फ़िल्टर प्लगइन बनाम शून्य स्थिति डेल्टा
19. [Commit a49e](https://github.com/manticoresoftware/manticoresearch/commit/a49e5bc13ea763e6f01dd5993e891858a84d1846) कई हिट पर 'FAIL' को 'WARNING' में बदलें

## संस्करण 3.5.0, 22 जुलाई 2020

### प्रमुख नए फीचर्स:
* यह रिलीज़ इतनी लंबी थी, क्योंकि हम थ्रेड्स से **कोरुटिन्स** में मल्टिटास्किंग मोड को बदलने पर कड़ी मेहनत कर रहे थे। यह कॉन्फ़िगरेशन को सरल बनाता है और क्वेरीज़ के समानांतरकरण को बहुत अधिक स्पष्ट बनाता है: मैन्टिकोर दिए गए थ्रेड्स की संख्या का उपयोग करता है (नए सेटिंग [threads](Server_settings/Searchd.md#threads) को देखें) और नया मोड सुनिश्चित करता है कि यह सबसे अनुकूल तरीके से किया जाए।
* [हाइलाइटिंग](Searching/Highlighting.md#Highlighting-options) में बदलाव:
  - कोई भी हाइलाइटिंग जो कई फ़ील्ड्स के साथ काम करती है (`highlight({},'field1, field2'`) या `highlight` JSON क्वेरी में) अब डिफ़ॉल्ट रूप से फ़ील्ड के अनुसार सीमा लागू करती है।
  - कोई भी हाइलाइटिंग जो प्लेन टेक्स्ट के साथ काम करती है (`highlight({}, string_attr)` या `snippet()` अब पूरे दस्तावेज़ पर सीमाएँ लागू करती है।
  - [फील्ड के अनुसार सीमाएं](Searching/Highlighting.md#limits_per_field) को `limits_per_field=0` विकल्प से वैश्विक सीमाओं में बदला जा सकता है (डिफ़ॉल्ट रूप से `1`)।
  - [allow_empty](Searching/Highlighting.md#allow_empty) अब HTTP JSON के माध्यम से हाइलाइटिंग के लिए डिफ़ॉल्ट रूप से `0` है।

* एक ही पोर्ट [अब उपयोग किया जा सकता है](Server_settings/Searchd.md#listen) HTTP, HTTPS और बाइनरी API के लिए (एक दूरस्थ मैन्टिकोर इंस्टेंस से कनेक्शन स्वीकार करने के लिए)। `listen = *:mysql` अभी भी MySQL प्रोटोकॉल के माध्यम से कनेक्शनों के लिए आवश्यक है। मैन्टिकोर अब अपने पास कनेक्ट करने के लिए प्रयास कर रहे क्लाइंट के प्रकार का स्वत: पता लगाता है, सिवाय MySQL के (प्रोटोकॉल की सीमाओं के कारण)।

* RT मोड में एक फ़ील्ड [पाठ और स्ट्रिंग एट्रिब्यूट](Creating_a_table/Data_types.md#String) दोनों हो सकता है - [GitHub मुद्दा #331](https://github.com/manticoresoftware/manticoresearch/issues/331)।

  [साधारण मोड](Read_this_first.md#Real-time-mode-vs-plain-mode) में इसे `sql_field_string` कहा जाता है। अब यह [RT मोड](Read_this_first.md#Real-time-mode-vs-plain-mode) में भी रीयल-टाइम इंडेक्स के लिए उपलब्ध है। आप इसे उदाहरण में दिखाए अनुसार उपयोग कर सकते हैं:

  <!-- more -->
  ```sql
  create table t(f string attribute indexed);
  insert into t values(0,'abc','abc');
  select * from t where match('abc');
  +---------------------+------+
  | id                  | f    |
  +---------------------+------+
  | 2810845392541843463 | abc  |
  +---------------------+------+
  1 row in set (0.01 sec)

  mysql> select * from t where f='abc';
  +---------------------+------+
  | id                  | f    |
  +---------------------+------+
  | 2810845392541843463 | abc  |
  +---------------------+------+
  1 row in set (0.00 sec)
  ```
  <!-- \more -->

### छोटे परिवर्तन
* आप अब [स्ट्रिंग एट्रिब्यूट्स को हाइलाइट कर सकते हैं](Searching/Highlighting.md#Highlighting-via-SQL)।
* SQL इंटरफ़ेस के लिए SSL और संकुचन समर्थन
* MySQL क्लाइंट [`status`](Node_info_and_management/Node_status.md#STATUS) कमांड का समर्थन।
* [प्रतिकृति](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication) अब बाहरी फ़ाइलों (स्टॉपवर्ड्स, अपवाद आदि) की प्रतिकृति कर सकती है।
* फ़िल्टर ऑपरेटर [`in`](Searching/Filters.md#Set-filters) अब HTTP JSON इंटरफ़ेस के माध्यम से उपलब्ध है।
* HTTP JSON में [`expressions`](Searching/Expressions.md#expressions)।
* [आप अब RT मोड में `rt_mem_limit` को फ्लाई पर बदल सकते हैं](https://github.com/manticoresoftware/manticoresearch/issues/344), अर्थात् आप `ALTER ... rt_mem_limit=<नई मान>` कर सकते हैं।
* आप अब [अलग CJK कैरेक्टर सेट टेबल्स](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) का उपयोग कर सकते हैं: `चाइनीज`, `जापानी` और `कोरियाई`।
* [thread_stack](Server_settings/Searchd.md#thread_stack) अब अधिकतम थ्रेड स्टैक को सीमित करता है, प्रारंभिक नहीं।
* `SHOW THREADS` आउटपुट को बेहतर बनाया गया।
* `SHOW THREADS` में लंबे `CALL PQ` की प्रगति दिखाएँ।
* cpustat, iostat, coredump को [SET](Server_settings/Setting_variables_online.md#SET) के साथ रनटाइम के दौरान बदल सकते हैं।
* `SET [GLOBAL] wait_timeout=NUM` लागू किया गया,

### ब्रेकिंग बदलाव:
* **इंडेक्स फार्मेट में बदलाव किया गया है।** 3.5.0 में बनाए गए इंडेक्स को Manticore संस्करण < 3.5.0 द्वारा लोड नहीं किया जा सकता, लेकिन Manticore 3.5.0 पुराने फ़ार्मेट्स को समझता है।
* [`INSERT INTO PQ VALUES()`](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md) (यानी, कॉलम सूची प्रदान किए बिना) पहले सटीक `(query, tags)` मान के रूप में अपेक्षित थे। इसे `(id,query,tags,filters)` में बदल दिया गया है। यदि आप इसे स्वचालित रूप से उत्पन्न करना चाहते हैं तो आईडी को 0 पर सेट किया जा सकता है।
* [`allow_empty=0`](Searching/Highlighting.md#allow_empty) HTTP JSON इंटरफेस के माध्यम से हाइलाइटिंग में एक नया डिफ़ॉल्ट है।
* बाहरी फ़ाइलों (स्टॉपवर्ड्स, अपवाद आदि) के लिए केवल पूर्ण पथ की अनुमति है `CREATE TABLE`/`ALTER TABLE` में।

### अप्रयुक्तता:
* `ram_chunks_count` का नाम बदलकर `ram_chunk_segments_count` कर दिया गया है `SHOW INDEX STATUS` में।
* `workers` अप्रचलित है। अब केवल एक कामकाजी मोड है।
* `dist_threads` अप्रचलित है। अब सभी क्वेरी संभवतः समानांतर होती हैं (जो `threads` और `jobs_queue_size` द्वारा सीमित होती हैं)।
* `max_children` अप्रचलित है। मैन्टिकोर द्वारा उपयोग होने वाले थ्रेड्स की संख्या सेट करने के लिए [threads](Server_settings/Searchd.md#threads) का उपयोग करें (डिफ़ॉल्ट रूप से CPU कोर की संख्या पर सेट किया गया)।
* `queue_max_length` अप्रचलित है। बल्कि, यदि इसकी वास्तव में आवश्यकता है तो [jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size) का उपयोग करके आंतरिक जॉब्स कतार का आकार ठीक करें (डिफ़ॉल्ट रूप से असीमित)।
* सभी `/json/*` एंडपॉइंट अब `/json/` के बिना उपलब्ध हैं, जैसे `/search`, `/insert`, `/delete`, `/pq` आदि।
* `field` का अर्थ "पूर्ण-शब्द क्षेत्र" `describe` में "text" में बदल दिया गया है।
  <!-- more -->
  **3.4.2:**
  ```sql
  mysql> describe t;
  +-------+--------+----------------+
  | Field | Type   | Properties     |
  +-------+--------+----------------+
  | id    | bigint |                |
  | f     | field  | indexed stored |
  +-------+--------+----------------+
  ```
  **3.5.0**:
  ```sql
  mysql> describe t;
  +-------+--------+----------------+
  | Field | Type   | Properties     |
  +-------+--------+----------------+
  | id    | bigint |                |
  | f     | text   | indexed stored |
  +-------+--------+----------------+
  ```
  <!-- \more -->
* टाउनिक `и` `non_cjk` charset_table में `i` में मैप नहीं होता (जो कि डिफ़ॉल्ट है) क्योंकि इसने रूसी स्टेमर्स और लेम्माटाइजर्स को बहुत अधिक प्रभावित किया।
* `read_timeout`। इसके बजाय [network_timeout](Server_settings/Searchd.md#network_timeout) का उपयोग करें जो पढ़ने और लिखने दोनों को नियंत्रित करता है।


### पैकेजेस

* उबंटू फोकल 20.04 आधिकारिक पैकेज
* deb पैकेज का नाम `manticore-bin` से `manticore` में बदल दिया गया है।

### बग फिक्सेस:
1. [Issue #351](https://github.com/manticoresoftware/manticoresearch/issues/351) searchd मेमोरी लीक
2. [Commit ceab](https://github.com/manticoresoftware/manticoresearch/commit/ceabe44f) स्निपेट्स में छोटे रीड आउट ऑफ बाउंड्स
3. [Commit 1c3e](https://github.com/manticoresoftware/manticoresearch/commit/1c3e84a3) क्रैश क्वेरीज़ के लिए स्थानीय变量 में खतरनाक लिखना
4. [Commit 26e0](https://github.com/manticoresoftware/manticoresearch/commit/26e094ab) परीक्षण 226 में सॉर्टर का छोटा मेमोरी लीक
5. [Commit d2c7](https://github.com/manticoresoftware/manticoresearch/commit/d2c7f86a) परीक्षण 226 में विशाल मेमोरी लीक
6. [Commit 0dd8](https://github.com/manticoresoftware/manticoresearch/commit/0dd80122) क्लस्टर दिखाता है कि नोड्स समन्वय में हैं, लेकिन `count(*)` अलग संख्या दिखाता है
7. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) कॉस्मेटिक: लॉग में डुप्लिकेट और कभी-कभी खोई हुई चेतावनी संदेश
8. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) कॉस्मेटिक: लॉग में (null) इंडेक्स नाम
9. [Commit 359d](https://github.com/manticoresoftware/manticoresearch/commit/359dbd30) 70M से अधिक परिणाम पुनर्प्राप्त नहीं कर सकते
10. [Commit 19f3](https://github.com/manticoresoftware/manticoresearch/commit/19f328ee) बिना कॉलम सिंटैक्स के PQ नियम डालने में असमर्थ
11. [Commit bf68](https://github.com/manticoresoftware/manticoresearch/commit/bf685d5d) क्लस्टर में इंडेक्स में दस्तावेज़ डालते समय भ्रामक错误 संदेश
12. [Commit 2cf1](https://github.com/manticoresoftware/manticoresearch/commit/2cf18c83) `/json/replace` और `json/update` घातांक रूप में id लौटाते हैं
13. [Issue #324](https://github.com/manticoresoftware/manticoresearch/issues/324) समान क्वेरी में json स्केलर गुण और mva अपडेट करें
14. [Commit d384](https://github.com/manticoresoftware/manticoresearch/commit/d38409eb) `hitless_words` RT मोड में काम नहीं करता
15. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) rt मोड में `ALTER RECONFIGURE` की मनाही होनी चाहिए
16. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) `rt_mem_limit` सर्चड पुनः प्रारंभ करने के बाद 128M पर रीसेट होता है
17. highlight() कभी-कभी लटक जाता है
18. [Commit 7cd8](https://github.com/manticoresoftware/manticoresearch/commit/7cd878f4) RT मोड में U+कोड का उपयोग करने में विफल
19. [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b213de4) RT मोड में wordforms पर वाइल्डकार्ड का उपयोग करने में विफल
20. [Commit e9d0](https://github.com/manticoresoftware/manticoresearch/commit/e9d07e68) `SHOW CREATE TABLE` बनाम कई वर्डफॉर्म फ़ाइलों में सुधार
21. [Commit fc90](https://github.com/manticoresoftware/manticoresearch/commit/fc90a84f) "query" के बिना JSON क्वेरी searchd को क्रैश करता है
22. मैन्टिकोर [अधिकारिक डॉकर](https://hub.docker.com/r/manticoresearch/manticore) mysql 8 से अनुक्रमित नहीं कर सका
23. [Commit 23e0](https://github.com/manticoresoftware/manticoresearch/commit/23e05d32) HTTP /json/insert id की आवश्यकता है
24. [Commit bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `SHOW CREATE TABLE` PQ के लिए काम नहीं करता
25. [Commit bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `CREATE TABLE LIKE` PQ के लिए ठीक से काम नहीं करता
26. [Commit 5eac](https://github.com/manticoresoftware/manticoresearch/commit/5eacf28f) सेटिंग्स में अंतिम पंक्ति "show index status" में
27. [Commit cb15](https://github.com/manticoresoftware/manticoresearch/commit/cb153228) HTTP JSON प्रतिक्रिया में "highlight" में खाली शीर्षक
28. [Issue #318](https://github.com/manticoresoftware/manticoresearch/issues/318) `CREATE TABLE LIKE` इनफिक्स त्रुटि
29. [Commit 9040](https://github.com/manticoresoftware/manticoresearch/commit/9040d22c) RT लोड के तहत क्रैश होता है
30. [cd512c7d ](https://github.com/manticoresoftware/manticoresearch/commit/cd512c7d ) RT डिस्क हिस्से पर क्रैश होने पर क्रैश लॉग खो गया
31. [Issue #323](https://github.com/manticoresoftware/manticoresearch/issues/323) तालिका आयात विफल और कनेक्शन बंद कर देता है
32. [Commit 6275](https://github.com/manticoresoftware/manticoresearch/commit/6275316a) `ALTER reconfigure` एक PQ इंडेक्स को भ्रष्ट कर देता है
33. [Commit 9c1d](https://github.com/manticoresoftware/manticoresearch/commit/9c1d221e) इंडेक्स प्रकार बदलने के बाद searchd रीलोड समस्याएँ
34. [Commit 71e2](https://github.com/manticoresoftware/manticoresearch/commit/71e2b5bb) आयात तालिका पर डेमन क्रैश होता है जब फ़ाइलें गायब होती हैं
35. [Issue #322](https://github.com/manticoresoftware/manticoresearch/issues/322) एकाधिक इंडेक्स का उपयोग करते समय चयन पर क्रैश, समूह द्वारा और रेंकर = none
36. [Commit c3f5](https://github.com/manticoresoftware/manticoresearch/commit/c3f58490) `HIGHLIGHT()` स्ट्रिंग विशेषताओं में हाइलाइट नहीं करता
37. [Issue #320](https://github.com/manticoresoftware/manticoresearch/issues/320) `FACET` स्ट्रिंग विशेषता पर क्रमबद्ध करने में विफल
38. [Commit 4f1a](https://github.com/manticoresoftware/manticoresearch/commit/4f1a1f25) डेटा डायरेक्टरी के गायब होने पर त्रुटि
39. [Commit 04f4](https://github.com/manticoresoftware/manticoresearch/commit/04f4ddd4) RT मोड में access_* का समर्थन नहीं है
40. [Commit 1c06](https://github.com/manticoresoftware/manticoresearch/commit/1c0616a2) स्ट्रिंग में खराब JSON ऑब्जेक्ट: 1. `CALL PQ` "स्ट्रिंग में खराब JSON ऑब्जेक्ट: 1" लौटाता है जब json कुछ मान से बड़ा होता है।
41. [Commit 32f9](https://github.com/manticoresoftware/manticoresearch/commit/32f943d6) RT-मोड असंगति। कुछ मामलों में मैं सूचकांक को छोड़ नहीं सकता क्योंकि यह अज्ञात है और इसे नहीं बना सकता क्योंकि निर्देशिका खाली नहीं है।
42. [Issue #319](https://github.com/manticoresoftware/manticoresearch/issues/319) चयन पर क्रैश
43. [Commit 22a2](https://github.com/manticoresoftware/manticoresearch/commit/22a28dd7) `max_xmlpipe2_field` = 2M ने 2M फ़ील्ड पर चेतावनी लौटाई
44. [Issue #342](https://github.com/manticoresoftware/manticoresearch/issues/342) प्रश्न शर्तों के निष्पादन में बग
45. [Commit dd8d](https://github.com/manticoresoftware/manticoresearch/commit/dd8dcab2) सरल 2 शर्तों की खोज केवल एक शर्त वाला दस्तावेज़ ढूंढती है
46. [Commit 9091](https://github.com/manticoresoftware/manticoresearch/commit/90919e62) PQ में बड़े अक्षरों में कीज़ के साथ एक json मेल करना असंभव था
47. [Commit 56da](https://github.com/manticoresoftware/manticoresearch/commit/56da086a) csv+docstore पर इंडेक्सर क्रैश हो जाता है
48. [Issue #363](https://github.com/manticoresoftware/manticoresearch/issues/363) centos 7 में json attr में `[null]` का उपयोग करके डाले गए डेटा को भ्रष्ट करता है
49. प्रमुख [Issue #345](https://github.com/manticoresoftware/manticoresearch/issues/345) रिकॉर्ड्स डाले नहीं जा रहे हैं, count() यादृच्छिक है, "replace into" OK लौटाता है
50. max_query_time SELECTs को बहुत धीमा कर देता है
51. [Issue #352](https://github.com/manticoresoftware/manticoresearch/issues/352) मैक ओएस पर मास्टर-एजेंट संचार विफल रहता है
52. [Issue #328](https://github.com/manticoresoftware/manticoresearch/issues/328) Connector.Net/Mysql 8.0.19 के साथ Manticore से कनेक्ट करते समय त्रुटि
53. [Commit daa7](https://github.com/manticoresoftware/manticoresearch/commit/daa760d2fe83668038318f0f384ceefc0d075e18) \0 के एस्केपिंग को ठीक किया और प्रदर्शन को अनुकूलित किया
54. [Commit 9bc5](https://github.com/manticoresoftware/manticoresearch/commit/9bc5c01a493db993b90ff6c1ad51aeae028f629c) गिनती विभिन्न और json को ठीक किया
55. [Commit 4f89](https://github.com/manticoresoftware/manticoresearch/commit/4f89a965a3af0db548ae9d1763731ebaebe21f5a) दूसरे नोड पर टेबल छोड़ने में विफल
56. [Commit 952a](https://github.com/manticoresoftware/manticoresearch/commit/952af5a5931fd72b87edc169588777778b365231) तंग चल रहे कॉल pq पर क्रैश ठीक करें


## संस्करण 3.4.2, 10 अप्रैल 2020
### महत्वपूर्ण बग फिक्स
* [Commit 2ffe](https://github.com/manticoresoftware/manticoresearch/commit/2ffe2d261793f9681265bc4abe38d695e72c121e) पुरानी संस्करण से RT इंडेक्स को डेटा अनुक्रमित करने में विफलता ठीक करें

## संस्करण 3.4.0, 26 मार्च 2020
### प्रमुख परिवर्तन
* सर्वर 2 मोड में कार्य करता है: rt-मोड और प्लेन-मोड
   *   rt-मोड को data_dir की आवश्यकता होती है और कॉन्फ़िगरेशन में कोई इंडेक्स परिभाषा नहीं होती है
   *   प्लेन-मोड में इंडेक्स कॉन्फ़िगरेशन में परिभाषित होते हैं; कोई data_dir की अनुमति नहीं है
* प्रतिकृति केवल rt-मोड में उपलब्ध है

### छोटे परिवर्तन
* charset_table का डिफ़ॉल्ट गैर_cjk उपनाम है
* rt-मोड में पूर्ण-पाठ फ़ील्ड को अनुक्रमित और संग्रहीत किया गया है
* rt-मोड में पूर्ण-पाठ फ़ील्ड का नाम 'field' से 'text' में बदल दिया गया है
* ALTER RTINDEX का नाम बदलकर ALTER TABLE दिया गया है
* TRUNCATE RTINDEX का नाम बदलकर TRUNCATE TABLE दिया गया है

### विशेषताएँ
* केवल संग्रहीत फ़ील्ड
* SHOW CREATE TABLE, IMPORT TABLE

### सुधार
* बिना लॉक वाला PQ बहुत तेज
* /sql किसी भी प्रकार के SQL कथन को mode=raw में निष्पादित कर सकता है
* mysql41 प्रोटोकॉल के लिए mysql उपनाम
* data_dir में डिफ़ॉल्ट state.sql

### बग फिक्स
* [Commit a533](https://github.com/manticoresoftware/manticoresearch/commit/a5333644e8e0c69746a9f24da8221fc965478e4a) highlight() में गलत फ़ील्ड सिंटैक्स पर क्रैश ठीक करें
* [Commit 7fbb](https://github.com/manticoresoftware/manticoresearch/commit/7fbb9f2e618096289cba7f8b834530b46aa94055) docstore के साथ replicate RT इंडेक्स पर सर्वर का क्रैश ठीक करें
* [Commit 24a0](https://github.com/manticoresoftware/manticoresearch/commit/24a04687eb43b4f4719c7c4a5fe3c19099240467) infix या prefix विकल्प के साथ इंडेक्स पर हाइलाइट करने पर क्रैश ठीक करें और बिना संग्रहीत फ़ील्ड सक्षम किए इंडेक्स करें
* [Commit 3465](https://github.com/manticoresoftware/manticoresearch/commit/3465c1ce9c860c09f6ccc5b83a63e3c68c241d8d) खाली docstore और खाली इंडेक्स के लिए dock-id लुकअप पर झूठी त्रुटि ठीक करें
* [Commit a707](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) fix #314 SQL insert कमांड के साथ अंतिम सेमीकोलन
* [Commit 9562](https://github.com/manticoresoftware/manticoresearch/commit/95628c9b9b8f16fe1bd18599a84e9673575adf1a) प्रश्न शब्दों के मिलान पर चेतावनी हटा दी
* [Commit b860](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) ICU द्वारा विभाजित स्निपेट्स में प्रश्नों को ठीक करें
* [Commit 5275](https://github.com/manticoresoftware/manticoresearch/commit/5275516ca4631c584e6e0127fa161a18d2658fd0) docstore ब्लॉक कैश में find/add दौड़ की स्थिति ठीक करें
* [Commit f06e](https://github.com/manticoresoftware/manticoresearch/commit/f06ef97af5c7390427e82525c150535290e42c3e) docstore में मेम लीक ठीक करें
* [Commit a725](https://github.com/manticoresoftware/manticoresearch/commit/a7258ba8340e246c0d8aaeb4a7a74129eba807eb) fix #316 LAST_INSERT_ID INSERT पर खाली लौटाता है
* [Commit 1ebd](https://github.com/manticoresoftware/manticoresearch/commit/1ebd5bf8d74113bcfa138ce63ce11966990a4215) fix #317 json/update HTTP एंडपॉइंट को MVA के लिए एरे और JSON एट्रिब्यूट के लिए ऑब्जेक्ट का समर्थन करने के लिए ठीक करें
* [Commit e426](https://github.com/manticoresoftware/manticoresearch/commit/e426950a536382a142e2a42ed207c341149ff685) विशेष रूप से बिना स्पष्ट आईडी के rt को डंप करने वाले इंडेक्सर की त्रुटि ठीक करें

## संस्करण 3.3.0, 4 फरवरी 2020
### विशेषताएँ
* समानांतर वास्तविक समय इंडेक्स खोज
* QUERY कमांड को समझाएँ
* इंडेक्स परिभाषाओं के बिना कॉन्फ़िगरेशन फ़ाइल (बीटा संस्करण)
* CREATE/DROP TABLE कमांड (बीटा संस्करण)
* indexer --print-rt - एक स्रोत से पढ़ सकता है और रियल-टाइम इंडेक्स के लिए INSERTs प्रिंट कर सकता है

### सुधार
* Snowball 2.0 स्टेमर्स के लिए अपडेट किया गया
* SHOW INDEX STATUS के लिए LIKE फ़िल्टर
* उच्च max_matches के लिए बेहतर मेमोरी उपयोग
* SHOW INDEX STATUS रियल-टाइम इंडेक्स के लिए ram_chunks_count जोड़ता है
* लॉकलेस PQ
* LimitNOFILE को 65536 में बदल दिया गया है


### बगफिक्स
* [Commit 9c33](https://github.com/manticoresoftware/manticoresearch/commit/9c33aab8c1fd9059c8644d23d5f58f03010043c6) ने डुप्लिकेट विशेषताओं के लिए इंडेक्स स्कीमा की जाँच जोड़ी #293
* [Commit a008](https://github.com/manticoresoftware/manticoresearch/commit/a0085f9477e191da9e0e835b91583fb9cfde698e) हिटलेस शर्तों में क्रैश को ठीक करें
* [Commit 6895](https://github.com/manticoresoftware/manticoresearch/commit/68953740af8dca1833026d0f448c71416ed3044a) ATTACH के बाद ढीला docstore ठीक करें
* [Commit d6f6](https://github.com/manticoresoftware/manticoresearch/commit/d6f696edea53ea2b9a68251e4054e4816082c285) वितरित सेटअप में docstore समस्या को ठीक करें
* [Commit bce2](https://github.com/manticoresoftware/manticoresearch/commit/bce2b7ec883d208f31094ee42a5b6f01d18a998f) сортера में FixedHash के स्थान पर OpenHash का उपयोग करें
* [Commit e0ba](https://github.com/manticoresoftware/manticoresearch/commit/e0baf7392937969e640fb5d10e8ccb312fe36446) इंडेक्स परिभाषा में डुप्लिकेट नामों के साथ विशेषताओं को ठीक करें
* [Commit ca81](https://github.com/manticoresoftware/manticoresearch/commit/ca81114bf1ac7ea7adcc8509bdc14d1f30b4f461) HIGHLIGHT() में html_strip को ठीक करें
* [Commit 493a](https://github.com/manticoresoftware/manticoresearch/commit/493a5e916d70fd4a3f14d1ee487234434f4a7ce3) HIGHLIGHT() में passage मैक्रो को ठीक करें
* [Commit a82d](https://github.com/manticoresoftware/manticoresearch/commit/a82d41c775692f837f267d554d7d4a4b499d69f0) जब RT इंडेक्स छोटे या बड़े डिस्क चंक बनाता है तो डबल बफर समस्याओं को ठीक करें
* [Commit a404](https://github.com/manticoresoftware/manticoresearch/commit/a404c85de0d41d9d1c75b7540479d8a3af9e61aa) kqueue के लिए इवेंट हटाने को ठीक करें
* [Commit 8bea](https://github.com/manticoresoftware/manticoresearch/commit/8bea0f6fd1666b817995c4dd6e67f6f1c9161638) RT इंडेक्स के rt_mem_limit के बड़े मूल्य के लिए डिस्क चंक को सेव करने में ठीक करें
* [Commit 8707](https://github.com/manticoresoftware/manticoresearch/commit/8707f0395b7ace52dc0cba9c8ed92a2bc4392932) इंडेक्सिंग पर फ्लोट ओवरफ्लो को ठीक करें
* [Commit a564](https://github.com/manticoresoftware/manticoresearch/commit/a56434ce71bb7714b4d8e75278aaa075b27ece80) RT इंडेक्स में नकारात्मक ID के साथ दस्तावेज़ встав करने में अब त्रुटि के साथ विफल होता है
* [Commit bbeb](https://github.com/manticoresoftware/manticoresearch/commit/bbebfd7551a68641154761cac962053b54806759) रैंकर फ़ील्डमास्क पर सर्वर के क्रैश को ठीक करें
* [Commit 3809](https://github.com/manticoresoftware/manticoresearch/commit/3809cc1b0863e3a5a200dc423be3f7c38be56113) क्वेरी कैश का उपयोग करते समय क्रैश को ठीक करें
* [Commit dc2a](https://github.com/manticoresoftware/manticoresearch/commit/dc2a585bc724678679e4297362200bbd49ce32bb) समानांतर इन्सर्ट के साथ RT इंडेक्स RAM खंडों का उपयोग करते समय क्रैश को ठीक करें

## संस्करण 3.2.2, 19 दिसंबर 2019
### विशेषताएँ
* RT इंडेक्स के लिए स्वचालित ID
* नई HIGHLIGHT() फ़ंक्शन के माध्यम से docstore के लिए हाईलाइट समर्थन, HTTP API में भी उपलब्ध
* SNIPPET() विशेष फ़ंक्शन QUERY() का उपयोग कर सकता है जो वर्तमान MATCH क्वेरी लौटाता है
* हाईलाइटिंग फ़ंक्शंस के लिए नया field_separator विकल्प.

### सुधार और परिवर्तन
* दूरस्थ नोड्स के लिए स्टोर की गई फ़ील्ड्स काLazy फ़ेच (प्रदर्शन को काफी बढ़ा सकता है)
* स्ट्रिंग और अभिव्यक्तियाँ अब मल्टी-क्वेरी और FACET ऑप्टिमाइजेशन को नहीं तोड़ती हैं
* RHEL/CentOS 8 निर्माण अब mariadb-connector-c-devel से mysql libclient का उपयोग करता है
* ICU डेटा फ़ाइल अब पैकेज के साथ भेजी जाती है, icu_data_dir हटा दिया गया है
* systemd सेवा फ़ाइलें 'Restart=on-failure' नीति शामिल करती हैं
* indextool अब वास्तविक समय के इंडेक्स को ऑनलाइन जांच सकता है
* मानक कॉन्फ़िगरेशन अब /etc/manticoresearch/manticore.conf है
* RHEL/CentOS पर सेवा का नाम 'searchd' से 'manticore' में बदल दिया गया है
* query_mode और exact_phrase स्निपेट के विकल्प हटा दिए गए हैं

### बगफिक्स
* [Commit 6ae4](https://github.com/manticoresoftware/manticoresearch/commit/6ae474c7894a6bee222d5b18e59a44fdbf57843a) HTTP इंटरफ़ेस पर SELECT क्वेरी पर क्रैश को ठीक करें
* [Commit 5957](https://github.com/manticoresoftware/manticoresearch/commit/59577513a49eac5a4a3c5e2cb38394d3246b5d35) RT इंडेक्स डिस्क चंक को बचाता है लेकिन कुछ दस्तावेजों को हटा नहीं करता है
* [Commit e861](https://github.com/manticoresoftware/manticoresearch/commit/e861f0fca0e88924450695d4e9d6acff7a36558a) मल्टी इंडेक्स या मल्टी क्वेरियों के साथ खोज करते समय क्रैश को ठीक करें
* [Commit 4409](https://github.com/manticoresoftware/manticoresearch/commit/440991fc977b8479800b45cdbc862eeb1ba5d965) चौड़े utf8 कोडपॉइंट्स के साथ लंबे शर्तों के लिए इनफिक्स पीढ़ी पर क्रैश को ठीक करें
* [Commit 5fd5](https://github.com/manticoresoftware/manticoresearch/commit/5fd599b48bba527c023e6aa0b262dca51ffb8a1c) IOCP में सॉकेट जोड़ने के दौरान दौड़ को ठीक करें
* [Commit cf10](https://github.com/manticoresoftware/manticoresearch/commit/cf10d7d3589f77adfaefbe52a51777bc9c67cf99) बूल क्वेरी बनाम json चयन सूची की समस्या को ठीक करें
* [Commit 996d](https://github.com/manticoresoftware/manticoresearch/commit/996de77f4c3ec103d965e28c36098fd07dba50c4) indextool जांच को गलती से गलत skiplist ऑफसेट रिपोर्ट करने के लिए ठीक करें, doc2row लुकअप की जाँच करें
* [Commit 6e3f](https://github.com/manticoresoftware/manticoresearch/commit/6e3fc9e88941c9427410f9d0adeebb7dd09a900f) इंडेक्सर बड़े डेटा पर नकारात्मक skiplist ऑफसेट के साथ खराब इंडेक्स उत्पन्न करता है उसे ठीक करें
* [Commit faed](https://github.com/manticoresoftware/manticoresearch/commit/faed3220b423a96401982bf47117edf1f62c584e) JSON केवल संख्यात्मक को स्ट्रिंग में और अभिव्यक्तियों में JSON स्ट्रिंग को संख्यात्मक रूपांतरण करता है
* [Commit 5331](https://github.com/manticoresoftware/manticoresearch/commit/533197200249d648ebcbdaca5d605861ee566348) indextool को कमांड लाइन में सेट किए गए कई आदेशों के मामले में गलत त्रुटि कोड के साथ बाहर निकलने के लिए ठीक करें
* [Commit 7955](https://github.com/manticoresoftware/manticoresearch/commit/795520ac351d22e4497ebaf6c249bf25081842b6) सुधार #275 बिनलॉग त्रुटि पर अवैध राज्य डिस्क पर कोई स्थान नहीं बचा
* [Commit 2284](https://github.com/manticoresoftware/manticoresearch/commit/2284da5e3ff77bf7be4038a4d950c0580e4159c8) सुधार #279 JSON विशेषता पर IN फ़िल्टर में क्रैश
* [Commit ce2e](https://github.com/manticoresoftware/manticoresearch/commit/ce2e4b4765111a7d331ffc256911f12770a8942d) सुधार #281 गलत पाइप बंद करने का कॉल
* [Commit 5355](https://github.com/manticoresoftware/manticoresearch/commit/535589ba8a37baf5c6056afb5ff969fd70476feb) सुधार सर्वर CALL PQ पर लटके हुए रिकर्सिव JSON विशेषता को स्ट्रिंग के रूप में एनकोड किया गया
* [Commit a5fc](https://github.com/manticoresoftware/manticoresearch/commit/a5fc8a36e1644ee051632408ab8b53e79aeb9ff4) सुधार मल्टीएंड नोड में डोकलिस्ट के अंत से आगे बढ़ना
* [Commit a362](https://github.com/manticoresoftware/manticoresearch/commit/a3628617ef4ee89a811d4ba2c4a04cbbc298c1b5) सुधार थ्रेड सार्वजनिक जानकारी की पुनर्प्राप्ति
* [Commit f8d2](https://github.com/manticoresoftware/manticoresearch/commit/f8d2d7bbbb90af3d2ee97424f97b05927f09d42b) सुधार डॉकस्टोर कैश लॉक

## संस्करण 3.2.0, 17 अक्टूबर 2019
### विशेषताएँ
* दस्तावेज़ भंडारण
* नए निर्देशांक stored_fields, docstore_cache_size, docstore_block_size, docstore_compression, docstore_compression_level

### सुधार और परिवर्तन
* SSL समर्थन में सुधार
* गैर_CJK अंतर्निहित चरित्र सेट अपडेट किया गया
* UPDATE/DELETE कथनों का SELECT लॉग में लॉगिंग अक्षम किया गया
* RHEL/CentOS 8 पैकेज

### बगफिक्स
* [Commit 301a](https://github.com/manticoresoftware/manticoresearch/commit/301a806b14725e842300ee225521407464374c41) सुधार डिस्क चंक्स में दस्तावेज़ को प्रतिस्थापित करते समय क्रैश 
* [Commit 46c1](https://github.com/manticoresoftware/manticoresearch/commit/46c1cad8fa11c1fc09cc34b20d978fdfcd4e6d27) सुधार \#269 LIMIT N OFFSET M
* [Commit 92a4](https://github.com/manticoresoftware/manticoresearch/commit/92a46edaa196ef33f3ac2225fbdba2c2b88ee45d) सुधार DELETE कथन जिसमें id स्पष्ट रूप से सेट किया गया या खोज छोड़ने के लिए id सूची प्रदान की गई
* [Commit 8ca7](https://github.com/manticoresoftware/manticoresearch/commit/8ca78c138d837caab30dc6e2343a3c4d1687bb87) सुधार विंडोजपोल्ल पोलर में नेट्लूप पर घटना को हटाने के बाद गलत अनुक्रमणिका
* [Commit 6036](https://github.com/manticoresoftware/manticoresearch/commit/603631e2bff5cb0acf25f68b85621742490eccd9) सुधार JSON के माध्यम से HTTP पर फ्लोट राउंडअप
* [Commit 62f6](https://github.com/manticoresoftware/manticoresearch/commit/62f64cb9e75ec28fc3c29f8414ce9656f5580254) सुधार दूरस्थ स्निपेट्स पहले खाली पथ की जांच करने के लिए; विंडोज परीक्षण को ठीक करना
* [Commit aba2](https://github.com/manticoresoftware/manticoresearch/commit/aba274c2c5abc1029894ba9f95b786bda93e8f22) सुधार कॉन्फ़िग को विंडोज़ पर लिनक्स की तरह काम करने के लिए
* [Commit 6b8c](https://github.com/manticoresoftware/manticoresearch/commit/6b8c4242ef77c3fa4d0ccb7d76d81714b6728f0b) सुधार \#194 PQ को morphology और stemmers के साथ काम करने के लिए
* [Commit 174d](https://github.com/manticoresoftware/manticoresearch/commit/174d312905285aa5746f4f404c53e6dc669ef42f) सुधार RT सेवानिवृत्त खंड प्रबंधन

## संस्करण 3.1.2, 22 अगस्त 2019
### विशेषताएँ और सुधार
* HTTP API के लिए प्रयोगात्मक SSL समर्थन
* CALL KEYWORDS के लिए फ़ील्ड फ़िल्टर
* /json/search के लिए max_matches
* डिफ़ॉल्ट Galera gcache.size का स्वचालित आकार
* FreeBSD समर्थन में सुधार

### बगफिक्स
* [Commit 0a1a](https://github.com/manticoresoftware/manticoresearch/commit/0a1a2c81038794983b4e30c8aefc7cc20e89b03f) उसी RT अनुक्रमणिका में जिस पर भिन्न पथ होता है, में RT अनुक्रमणिका की प्रगति को ठीक किया गया
* [Commit 4adc](https://github.com/manticoresoftware/manticoresearch/commit/4adc075294ac823289f745e2cc419f18c7dcf2e2) सुधार गतिविधि के बिना अनुक्रमणिकाओं के लिए फ़्लश पुनर्निर्धारण 
* [Commit d6c0](https://github.com/manticoresoftware/manticoresearch/commit/d6c00a6f750c04021747e9c99e0f77707a4cbfc3) RT/PQ अनुक्रमणिका को फ्लश करने के लिए पुनर्निर्धारण में सुधार 
* [Commit d0a7](https://github.com/manticoresoftware/manticoresearch/commit/d0a7c95906b9b5ba33c98267d54501cfe27508d6) सुधार \#250 index_field_lengths अनुक्रमणिका विकल्प TSV और CSV पाइप स्रोतों के लिए 
* [Commit 1266](https://github.com/manticoresoftware/manticoresearch/commit/1266d54800242709fbe2c34e72eb69b6595b23bd) सुधार indextool गलत रिपोर्ट ब्लॉक अनुक्रमणिका जांच पर खाली अनुक्रमणिका 
* [Commit 553c](https://github.com/manticoresoftware/manticoresearch/commit/553ca73ca111f28e0888d59cd48b0d6df181ecd6) सुधार Manticore SQL प्रश्न लॉग में खाली चयन सूची 
* [Commit 56c8](https://github.com/manticoresoftware/manticoresearch/commit/56c8584456293665ddd4ce4c94307d74473a78b9) सुधार अनुक्रमणिका -h/--help प्रतिक्रिया

## संस्करण 3.1.0, 16 जुलाई 2019
### विशेषताएँ और सुधार
* वास्तविक समय अनुक्रमणिकाओं के लिए प्रजनन 
* चीनी के लिए ICU टोकनाइज़र 
* नई morphology विकल्प icu_chinese 
* नया निर्देशांक icu_data_dir 
* प्रजनन के लिए कई कथनों का लेनदेन 
* LAST_INSERT_ID() और @session.last_insert_id 
* SHOW VARIABLES के लिए LIKE 'pattern' 
* पर्कोलेट अनुक्रमणिकाओं के लिए कई दस्तावेज़ INSERT 
* कॉन्फ़िग के लिए समय पार्सर जोड़ा गया 
* आंतरिक कार्य प्रबंधक 
* दस्तावेज़ और हिट सूची घटकों के लिए mlock 
* जेल स्निपेट्स पथ 

### हटाए गए
* ICU के पक्ष में RLP पुस्तकालय समर्थन हटा दिया गया; सभी rlp* निर्देशांक हटा दिए गए 
* UPDATE से दस्तावेज़ ID को अपडेट करना अक्षम किया गया 
### बगफिक्स
* [Commit f047](https://github.com/manticoresoftware/manticoresearch/commit/f047222339fd0e62a07e9ca8b2a829297d5db861) सुधार concat और group_concat में दोष 
* [Commit b081](https://github.com/manticoresoftware/manticoresearch/commit/b08147ee24712b5eaa51f261626b5d16c6242fdd) सुधार पर्कोलेट अनुक्रमिका पर प्रश्न uid को BIGINT विशेषता प्रकार बनाना 
* [Commit 4cd8](https://github.com/manticoresoftware/manticoresearch/commit/4cd85afaf76eef0ce9fa1cd073f3fed160279890) यदि नए डिस्क चंक्स के लिए प्रीऑल्क को असफल कर दें तो दुर्घटना नहीं होती 
* [Commit 1a55](https://github.com/manticoresoftware/manticoresearch/commit/1a551227886474054111088412f2b2396f485652) ALTER में अनुपस्थित टाइमस्टैम्प डेटा प्रकार जोड़ें
* [कमिट f3a8](https://github.com/manticoresoftware/manticoresearch/commit/f3a8e096fd496a6b001d398aadbb781b2dbad929) गलत mmap पढ़ने के कारण क्रैश को ठीक किया
* [कमिट 4475](https://github.com/manticoresoftware/manticoresearch/commit/447577115192db7ac7bfea70b5ba58ee2c61229f) प्रतिकृति में क्लस्टर्स लॉक के हैश को ठीक किया
* [कमिट ff47](https://github.com/manticoresoftware/manticoresearch/commit/ff476df955f057bd38e8b6ad6056325e9d6fbb9c) प्रतिकृति में प्रदाताओं के लीक को ठीक किया
* [कमिट 58dc](https://github.com/manticoresoftware/manticoresearch/commit/58dcbb779c4c30ca1560460c78e35ba114b83716) इंडेक्सर में \#246 अपरिभाषित sigmask को ठीक किया
* [कमिट 3dd8](https://github.com/manticoresoftware/manticoresearch/commit/3dd8278e747d534f69fe9c3d360eef188ab7b6d5) नेटलूप रिपोर्टिंग में रेस कंडीशन को ठीक किया
* [कमिट a02a](https://github.com/manticoresoftware/manticoresearch/commit/a02aae05a3fa3d1caf2821f43c7e237b6de76336) HA रणनीतियों के रिबैलेंसर के लिए जीरो गैप

## Version 3.0.2, 31 May 2019
### Improvements
* दस्तावेज़ और हिट सूचियों के लिए mmap रीडर्स जोड़े गए
* `/sql` HTTP एन्डपॉइंट प्रतिक्रिया अब `/json/search` प्रतिक्रिया के समान है
* नई निर्देश `access_plain_attrs`, `access_blob_attrs`, `access_doclists`, `access_hitlists`
* प्रतिकृति सेटअप के लिए नई निर्देश `server_id`

### Removals
* HTTP `/search` एन्डपॉइंट हटा दिया गया

### Deprecations
* `read_buffer`, `ondisk_attrs`, `ondisk_attrs_default`, `mlock` को `access_*` निर्देशों से प्रतिस्थापित किया गया है

### Bugfixes
* [कमिट 849c](https://github.com/manticoresoftware/manticoresearch/commit/849c16e1040f382795ba0635cb48686f9db4f2d8) चयन सूची में संख्याओं से शुरू होने वाले गुण नामों की अनुमति दी गई
* [कमिट 48e6](https://github.com/manticoresoftware/manticoresearch/commit/48e6c302ca37bec0bc49c79619c60bf787ed0708) UDFs में MVAs को ठीक किया, MVA उपनामकरण को ठीक किया
* [कमिट 0555](https://github.com/manticoresoftware/manticoresearch/commit/055586a9a2ebfa8874f68099c5bc798a145fd4ef) SENTENCE वाले क्वेरी का उपयोग करते समय \#187 क्रैश को ठीक किया
* [कमिट 93bf](https://github.com/manticoresoftware/manticoresearch/commit/93bf52f23f1c53c8a93a603387ca5463e1fc2ae8) MATCH() के चारों ओर () समर्थन को ठीक किया [\#143]
* [कमिट 599e](https://github.com/manticoresoftware/manticoresearch/commit/599ee79ccd38cce0023a55cbce8f50c3dca62e38) ALTER क्लस्टर स्टेटमेंट पर क्लस्टर स्थिति के सहेजे जाने को ठीक किया
* [कमिट 230c](https://github.com/manticoresoftware/manticoresearch/commit/230c321e277514b191629538eb3f02d219113d95) blob गुणों के साथ ALTER इंडेक्स पर सर्वर क्रैश को ठीक किया
* [कमिट 5802](https://github.com/manticoresoftware/manticoresearch/commit/5802b85aa93a81caea77073a3381ef912fdd677c) id द्वारा फ़िल्टरिंग में \#196 को ठीक किया
* [कमिट 25d2](https://github.com/manticoresoftware/manticoresearch/commit/25d2dabda4a299a29bd375405862126da9b4bfac) टेम्पलेट इंडेक्स पर खोज को अस्वीकृत किया
* [कमिट 2a30](https://github.com/manticoresoftware/manticoresearch/commit/2a30d5b49d76faab031d195f533974938769b8e6) SQL उत्तर में id कॉलम को नियमित bigint प्रकार में ठीक किया


## Version 3.0.0, 6 May 2019
### Features and improvements
* नया इंडेक्स संग्रहण। नॉन-स्केलर गुण अब प्रति इंडेक्स 4GB आकार तक सीमित नहीं हैं
* attr_update_reserve निर्देश
* String, JSON और MVAs को UPDATE का उपयोग करके अपडेट किया जा सकता है
* killlists इंडेक्स लोड टाइम पर लागू होते हैं
* killlist_target निर्देश
* मल्टी AND खोजें तेज हुईं
* बेहतर औसत प्रदर्शन और RAM उपयोग
* 2.x के साथ बनाए गए इंडेक्स को अपग्रेड करने के लिए कनवर्ट टूल
* CONCAT() फ़ंक्शन
* JOIN CLUSTER cluster AT 'nodeaddress:port'
* ALTER CLUSTER posts UPDATE nodes
* node_address निर्देश
* SHOW STATUS में मुद्रित नोड्स की सूची

### Behaviour changes
* killlists वाले इंडेक्स के मामले में, सर्वर conf में परिभाषित क्रम में इंडेक्स को रोटेट नहीं करता, बल्कि killlist targets की श्रृंखला का पालन करता है
* खोज में इंडेक्स का क्रम अब यह परिभाषित नहीं करता कि killlists किस क्रम में लागू होते हैं
* दस्तावेज़ ID अब साइन किए हुए बड़े पूर्णांक हैं

### Removed directives
* docinfo (अब हमेशा extern), inplace_docinfo_gap, mva_updates_pool
## Version 2.8.2 GA, 2 April 2019
### Features and improvements
* percolate इंडेक्स के लिए Galera प्रतिकृति
* OPTION morphology
### Compiling notes
Cmake न्यूनतम संस्करण अब 3.13 है। कंपाइल करने के लिए boost और libssl की आवश्यकता है
विकास पुस्तकालय।
### Bugfixes
* [कमिट 6967](https://github.com/manticoresoftware/manticoresearch/commit/6967fedb2ef818ec1c825d482563edd05e1c9245) कई वितरित इंडेक्स के लिए क्वेरी में चयन सूची पर कई स्टारों के कारण क्रैश को ठीक किया
* [कमिट 36df](https://github.com/manticoresoftware/manticoresearch/commit/36df1a407dc08263690e3492518613ace82d69ca) Manticore SQL इंटरफेस के माध्यम से बड़े पैकेट को ठीक किया [\#177]
* [कमिट 5793](https://github.com/manticoresoftware/manticoresearch/commit/57932aec734583fa93359faaf5034b2e2c9d352b) MVA अपडेट के साथ RT optimize पर सर्वर क्रैश को ठीक किया [\#170]
* [कमिट edb2](https://github.com/manticoresoftware/manticoresearch/commit/edb24b870423add86eba471d361e0e5aff098b18) RT इंडेक्स हटाने के बाद कॉन्‍फिग फिर से लोड पर SIGHUP के कारण binlog हटाने से सर्वर क्रैश को ठीक किया
* [कमिट bd3e](https://github.com/manticoresoftware/manticoresearch/commit/bd3e66e0085bc7e2e351b817dfc972fd8158fbce) mysql हैंडशेक auth प्लगइन payloads को ठीक किया
* [कमिट 6a21](https://github.com/manticoresoftware/manticoresearch/commit/6a217f6ed82fb10f752213b15617b9cedf1e8533) RT इंडेक्स पर phrase_boundary सेटिंग्स को ठीक किया [\#172]
* [कमिट 3562](https://github.com/manticoresoftware/manticoresearch/commit/3562f652753e4091fd7b5b0f65b58341c8cbcb31) ATTACH index को स्वयं से जोड़ते समय deadlock को ठीक किया [\#168]
* [कमिट 250b](https://github.com/manticoresoftware/manticoresearch/commit/250b3f0e74ad18dda34ba080aa13ca87798a9ac9) सर्वर क्रैश के बाद binlog द्वारा खाली meta सहेजे जाने को ठीक किया
* [Commit 4aa6](https://github.com/manticoresoftware/manticoresearch/commit/4aa6c69ad3a181cd7c5dafb990fb528121f68e59) ने RT इंडेक्स से डिस्क चंक्स में सॉर्टर पर स्ट्रिंग के कारण सर्वर की क्रैश को ठीक किया

## संस्करण 2.8.1 GA, 6 मार्च 2019
### सुविधाएँ और सुधार
* SUBSTRING_INDEX()
* पर्कोलेट क्वेरीज के लिए SENTENCE और PARAGRAPH समर्थन
* Debian/Ubuntu के लिए systemd जनरेटर; कोर डंपिंग की अनुमति देने के लिए LimitCORE भी जोड़ा गया

### बगफ़िक्सेस
* [Commit 84fe](https://github.com/manticoresoftware/manticoresearch/commit/84fe7405d17b59f4dcde15a6c8e8503923503def) ने मैच मोड सभी और खाली पूर्ण पाठ क्वेरी पर सर्वर की क्रैश को ठीक किया
* [Commit daa8](https://github.com/manticoresoftware/manticoresearch/commit/daa88b579fa373d5d2e869e43a2e178363b0fef1) ने स्टेटिक स्ट्रिंग को हटाने पर क्रैश को ठीक किया
* [Commit 2207](https://github.com/manticoresoftware/manticoresearch/commit/22078537dddbd9ce81a182dcc879f235b6e87004) ने FATAL के साथ indextool विफल होने पर निकास कोड को ठीक किया
* [Commit 0721](https://github.com/manticoresoftware/manticoresearch/commit/0721696d6780c200c65d596624a8187983fb7fcb) ने [\#109](https://github.com/manticoresoftware/manticoresearch/issues/109) पर गलत सटीक फॉर्म चेक के कारण उपसर्गों के लिए कोई मेल न मिलने की समस्या को ठीक किया
* [Commit 8af8](https://github.com/manticoresoftware/manticoresearch/commit/8af810111b8e2f87bc9f378172eff1ab9725c7e7) ने [\#161](https://github.com/manticoresoftware/manticoresearch/issues/161) पर RT इंडेक्स के लिए कॉन्फ़िग सेटिंग्स को फिर से लोड करने की समस्या को ठीक किया
* [Commit e2d5](https://github.com/manticoresoftware/manticoresearch/commit/e2d592773f4fd702ace90070a793a842a0fec6ab) ने बड़े JSON स्ट्रिंग की पहुंच पर सर्वर की क्रैश को ठीक किया
* [Commit 75cd](https://github.com/manticoresoftware/manticoresearch/commit/75cd1342d05cdc352e3dd145cdbc6f79394a165b) ने JSON दस्तावेज़ में PQ फ़ील्ड को संशोधित करने पर सिबलिंग फ़ील्ड से गलत मेल का कारण बनने की समस्या को ठीक किया
* [Commit e2f7](https://github.com/manticoresoftware/manticoresearch/commit/e2f775437df922674d772ed4417780492502e65a) ने RHEL7 बिल्ड पर JSON को पार्स करते समय सर्वर के क्रैश को ठीक किया
* [Commit 3a25](https://github.com/manticoresoftware/manticoresearch/commit/3a25a5808feb3f8b80866991b436d6c1241618c2) ने जब स्लैश किनारे पर हो तब JSON अनएस्केपिंग के क्रैश को ठीक किया
* [Commit be9f](https://github.com/manticoresoftware/manticoresearch/commit/be9f497872bea62dbdccd64fc28294fd7776c289) ने 'skip_empty' विकल्प को ठीक किया ताकि खाली दस्तावेजों को छोड़ सके और यह चेतावनी न दे कि वे सही JSON नहीं हैं
* [Commit 266e](https://github.com/manticoresoftware/manticoresearch/commit/266e0e7b088549722a805fc837bf101ff681a5e8) ने [\#140](https://github.com/manticoresoftware/manticoresearch/issues/161) पर 8 अंकों का आउटपुट विकर्ण संख्याओं पर जब 6 पर्याप्त नहीं था तो ठीक किया
* [Commit 3f6d](https://github.com/manticoresoftware/manticoresearch/commit/3f6d23891064ee8e94030c4231497cdd7da33a6a) ने खाली jsonobj निर्माण को ठीक किया
* [Commit f3c7](https://github.com/manticoresoftware/manticoresearch/commit/f3c7848a59b7eedc67c3403d6d4b37ce6fa94dc8) ने [\#160](https://github.com/manticoresoftware/manticoresearch/issues/161) पर खाली mva को NULL का आउटपुट दिया बजाय एक खाली स्ट्रिंग के
* [Commit 0afa](https://github.com/manticoresoftware/manticoresearch/commit/0afa2ed058e5759470b1d7354c722faab34f98bb) ने pthread_getname_np के बिना निर्माण में विफलता को ठीक किया
* [Commit 9405](https://github.com/manticoresoftware/manticoresearch/commit/9405fccdeb27f0302a8d9a848981d30080216777) ने थ्रेड_पूल श्रमिकों के साथ सर्वर शटडाउन पर क्रैश को ठीक किया

## संस्करण 2.8.0 GA, 28 जनवरी 2019
### सुधार
* पर्कोलेट इंडेक्स के लिए वितरित इंडेक्स
* CALL PQ नए विकल्प और परिवर्तन:
    *   skip_bad_json
    *   mode (sparsed/sharded)
    *   JSON दस्तावेज़ों को JSON ऐरे के रूप में پاس किया जा सकता है
    *   shift
    *   कॉलम नाम 'UID', 'Documents', 'Query', 'Tags', 'Filters' को 'id', 'documents', 'query', 'tags', 'filters' में नाम बदल दिया गया
* DESCRIBE pq TABLE
* UID के लिए SELECT FROM pq अब संभव नहीं है, इसके बजाय 'id' का उपयोग करें
* pq इंडेक्स पर SELECT नियमित इंडेक्स के बराबर है (उदाहरण के लिए, आप REGEX() और नियमों के माध्यम से फ़िल्टर कर सकते हैं)
* ANY/ALL को PQ टैग पर उपयोग किया जा सकता है
* अभिव्यक्तियों के लिए JSON फ़ील्ड के लिए ऑटो-परिवर्तन है, जिसमें स्पष्ट कास्टिंग की आवश्यकता नहीं है
* निर्मित 'non_cjk' charset_table और 'cjk' ngram_chars
* 50 भाषाओं के लिए निर्मित स्टॉपवर्ड संग्रह
* स्टॉपवर्ड घोषणाओं में एक से अधिक फ़ाइलों को कॉमा द्वारा भी विभाजित किया जा सकता है
* CALL PQ JSON दस्तावेज़ों के ऐरे को स्वीकार कर सकता है

### बगफ़िक्सेस
* [Commit a4e1](https://github.com/manticoresoftware/manticoresearch/commit/a4e19afee54dafdc04b0da53372dafd8c0d332d6) ने csjon से संबंधित लीक को ठीक किया
* [Commit 28d8](https://github.com/manticoresoftware/manticoresearch/commit/28d862774874891a03e361da1b0347ebe8066ce0) ने JSON में छूटी हुई मान के कारण क्रैश को ठीक किया
* [Commit bf4e](https://github.com/manticoresoftware/manticoresearch/commit/bf4e9ea27c349cdc9ae4e54e960a4a17b90e38fa) ने RT इंडेक्स के लिए खाली मेटा को बचाने में समस्या को ठीक किया
* [Commit 33b4](https://github.com/manticoresoftware/manticoresearch/commit/33b4573529e7c7c7bce19bf9d54054866f30d99a) ने लेमाटाइज़र की अनुक्रम के लिए रूप फ़्लैग (सटीक) खोने की समस्या को ठीक किया
* [Commit 6b95](https://github.com/manticoresoftware/manticoresearch/commit/6b95d48240d3b5520afade19c249d79e020a5e88) ने स्ट्रिंग attrs > 4M का उपयोग संतृप्त करने के लिए ओवरफ्लो के बजाय कर दिया
* [Commit 6214](https://github.com/manticoresoftware/manticoresearch/commit/621418b829e70af36aaa322a2f51ece3f86bc499) ने अक्षम इंडेक्स के साथ SIGHUP पर सर्वर की क्रैश को ठीक किया
* [Commit 3f7e](https://github.com/manticoresoftware/manticoresearch/commit/3f7e35d1482966cc45d5abbcb2de5de3508d66b0) ने समवर्ती API सत्र स्थिति आदेशों पर सर्वर की क्रैश को ठीक किया
* [Commit cd9e](https://github.com/manticoresoftware/manticoresearch/commit/cd9e4f1709a48ddafc128c450c2d882bc11ba50e) ने फ़ील्ड फ़िल्टरों के साथ RT इंडेक्स के लिए डिलीट क्वेरी पर सर्वर की क्रैश को ठीक किया
* [Commit 9376](https://github.com/manticoresoftware/manticoresearch/commit/9376470d455fcd256c6abff79c431a6919dfa7ac) ने खाली दस्तावेज़ों के साथ वितरित इंडेक्स पर CALL PQ पर सर्वर की क्रैश को ठीक किया
* [Commit 8868](https://github.com/manticoresoftware/manticoresearch/commit/8868b207644f4f8cc1ab0c270adb35493fed22e1) ने 512 अक्षरों से बड़े Manticore SQL त्रुटि संदेश को ठीक किया
* [Commit de9d](https://github.com/manticoresoftware/manticoresearch/commit/de9deda9c142823d6dbf529423417b5c670fae94) ने बिनलॉग के बिना पर्कोलैट इंडेक्स को सहेजने पर क्रैश को ठीक किया
* [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b219e1a32791e7740bb210b7d408f96abc6374f) ने OSX में http इंटरफेस का काम नहीं करने को ठीक किया
* [Commit e92c](https://github.com/manticoresoftware/manticoresearch/commit/e92c60240f8f9e1756bfa14fba0bbb402586a389) ने MVA की जांच पर indextool के झूठे त्रुटि संदेश को ठीक किया
* [Commit 238b](https://github.com/manticoresoftware/manticoresearch/commit/238bdea59bad89f097403f1c978658ce45f16c70) ने FLUSH RTINDEX पर लिखने के लॉक को ठीक किया ताकि सहेजने के दौरान पूरे इंडेक्स को लॉक न करें और rt_flush_period से नियमित फ्लश पर
* [Commit c26a](https://github.com/manticoresoftware/manticoresearch/commit/c26a236bd42b7082db079a51a84172066d337d64) ने ALTER पर्कोलैट इंडेक्स को खोज लोड के लिए इंतजार करते हुए फसने को ठीक किया
* [Commit 9ee5](https://github.com/manticoresoftware/manticoresearch/commit/9ee5703f6c28f57cd5bff5e705966d93fe30d267) ने max_children को 0 के मान के लिए थ्रेड_pool कार्यकर्ताओं की डिफ़ॉल्ट मात्रा का उपयोग करने के लिए ठीक किया
* [Commit 5138](https://github.com/manticoresoftware/manticoresearch/commit/5138fc048a1bd146b271ce6a72fe954344281f69) ने index_token_filter प्लगइन के साथ स्टॉपवर्ड और stopword_step=0 में डेटा को इंडेक्स करने पर त्रुटि को ठीक किया
* [Commit 2add](https://github.com/manticoresoftware/manticoresearch/commit/2add3d319ac62fe450bf60e89033100853dc2ecf) ने अभी भी इंडेक्स पर AOT लेमाटाइज़र्स का उपयोग करते समय अनुपस्थित lemmatizer_base के साथ क्रैश को ठीक किया

## Version 2.7.5 GA, 4 दिसंबर 2018
### Improvements
* REGEX फ़ंक्शन
* JSON API खोज के लिए लिमिट/ऑफसेट
* qcache के लिए प्रोफाइलर बिंदु

### Bugfixes
* [Commit eb3c](https://github.com/manticoresoftware/manticoresearch/commit/eb3c7683b9740cff4cc4cadd2ab9f990b9f7d173) ने मल्टीपल एट्रिब्यूट वाइड प्रकारों के साथ FACET पर सर्वर के क्रैश को ठीक किया
* [Commit d915](https://github.com/manticoresoftware/manticoresearch/commit/d915cf6696514f4be365ffb5981bccaa15983e65) ने FACET क्वेरी की मुख्य चयन सूची में निहित समूह द्वारा को ठीक किया
* [Commit 5c25](https://github.com/manticoresoftware/manticoresearch/commit/5c25dc271a8c11a1c82ca3c88e518acbc6a8c164) ने GROUP N BY के साथ क्वेरी पर क्रैश को ठीक किया
* [Commit 85d3](https://github.com/manticoresoftware/manticoresearch/commit/85d30a232d4ffd89916b673a9b2db946d272a945) ने मेमोरी ऑपरेशंस पर क्रैश होने पर डेडलॉक को ठीक किया
* [Commit 8516](https://github.com/manticoresoftware/manticoresearch/commit/85166b5ef2be38061d62f2b20f15acaa0311258a) ने जांच के दौरान indextool मेमोरी खपत को ठीक किया
* [Commit 58fb](https://github.com/manticoresoftware/manticoresearch/commit/58fb03166830d8b23e7ab9ce56309db7ddac22f4) ने gmock शामिल करना अब आवश्यक नहीं है क्योंकि अपस्ट्रीम स्वचालित रूप से हल करता है

## Version 2.7.4 GA, 1 नवंबर 2018
### Improvements
* रिमोट वितरित इंडेक्स के मामले में SHOW THREADS मूल क्वेरी को API कॉल के बजाय प्रिंट करता है
* SHOW THREADS नया विकल्प `format=sphinxql` सभी क्वेरियों को SQL प्रारूप में प्रिंट करता है
* SHOW PROFILE अतिरिक्त `clone_attrs` चरण प्रिंट करता है

### Bugfixes
* [Commit 4f15](https://github.com/manticoresoftware/manticoresearch/commit/4f155712a0bccc1bd01cc191647bc8cff814888e) ने malloc_stats, malloc_trim के बिना libc के साथ निर्माण विफल को ठीक किया
* [Commit f974](https://github.com/manticoresoftware/manticoresearch/commit/f974f20bda3214a56877c393a192be1a77150958) ने CALL KEYWORDS परिणाम सेट के लिए शब्दों के अंदर विशेष प्रतीकों को ठीक किया
* [Commit 0920](https://github.com/manticoresoftware/manticoresearch/commit/092083282ea8ae0b2e72fb6989c57ccec81e74ac) ने API के माध्यम से या दूरस्थ एजेंट को वितरित इंडेक्स के लिए CALL KEYWORDS को टूटा हुआ ठीक किया
* [Commit fd68](https://github.com/manticoresoftware/manticoresearch/commit/fd686bfe88b720ffd7642e36897ba45161cbd7d2) ने वितरित इंडेक्स agent_query_timeout को एजेंटों को max_query_time के रूप में प्रकट किया
* [Commit 4ffa](https://github.com/manticoresoftware/manticoresearch/commit/4ffa623e9d357a3b0b441615089e211f92f8de32) ने डिस्क_chunk पर कुल दस्तावेज़ काउंटर को ठीक किया जो OPTIMIZE कमांड द्वारा प्रभावित हुआ और वजन की गणना को तोड़ दिया
* [Commit dcaf](https://github.com/manticoresoftware/manticoresearch/commit/dcaf4e0e3c8e9b0fbec47bbf3307feddec0936a6) ने ब्लेंडेड से RT इंडेक्स पर कई टेल हिट को ठीक किया
* [Commit eee3](https://github.com/manticoresoftware/manticoresearch/commit/eee381754e902a43a8f499e1c950198a2a3e6ee0) ने घुमाव पर डेडलॉक को ठीक किया

## Version 2.7.3 GA, 26 सितंबर 2018
### Improvements
* CALL KEYWORDS के लिए sort_mode विकल्प
* DEBUG पर VIP कनेक्शन 'crash <password>' के लिए जानबूझकर SIGEGV क्रिया पर सर्वर पर प्रदर्शन कर सकता है
* DEBUG 'malloc_stats' के लिए searchd.log में malloc stats को डंप करने के लिए प्रदर्शन कर सकता है और malloc_trim' को malloc_trim() करने के लिए प्रदर्शन कर सकता है
* यदि सिस्टम पर gdb उपलब्ध है तो बैकट्रेस में सुधार हुआ

### Bugfixes
* [Commit 0f3c](https://github.com/manticoresoftware/manticoresearch/commit/0f3cc333f6129451ad1ae2c97905b6c04ba5d454) ने Windows पर नाम परिवर्तन के क्रैश या हंग को ठीक किया
* [Commit 1455](https://github.com/manticoresoftware/manticoresearch/commit/1455ba2bf8079f03ce7f861a8d9662a360d705de) ने 32-बिट सिस्टम पर सर्वर के क्रैश को ठीक किया
* [Commit ad37](https://github.com/manticoresoftware/manticoresearch/commit/ad3710d53b5cd6a28b8c60352d0f9e285b03d9fa) ने खाली SNIPPET अभिव्यक्ति पर सर्वर के क्रैश या लटके को ठीक किया
* [Commit b36d](https://github.com/manticoresoftware/manticoresearch/commit/b36d79214364500252665860f000ae85343528b6) ने टूटे हुए गैर प्रगतिशील ऑप्टिमाइज को ठीक किया और प्रगतिशील ऑप्टिमाइज को सुधार दिया कि सबसे पुराने डिस्क_chunk के लिए किल-सूची न बनाएँ
* [Commit 34b0](https://github.com/manticoresoftware/manticoresearch/commit/34b032499afd42ce47a4c7247814b4031094388a) ने थ्रेड पूल कार्यकर्ता मोड में SQL और API के लिए queue_max_length की खराब प्रतिक्रिया को ठीक किया
* [Commit ae4b](https://github.com/manticoresoftware/manticoresearch/commit/ae4b3202cbdb8014cabe2b90e269d5cf74f49871) ने regexp या rlp विकल्प सेट करते समय PQ इंडेक्स में पूर्ण-स्कैन क्वेरी जोड़ने पर क्रैश को ठीक किया
* [Commit f80f](https://github.com/manticoresoftware/manticoresearch/commit/f80f8d5d7560187078868aed9a9575f4549e98aa) ने एक PQ के बाद दूसरे को कॉल करते समय क्रैश को ठीक किया
* [Commit 9742](https://github.com/manticoresoftware/manticoresearch/commit/9742f5f0866af73f8cd8483ecd18a507ea80dd65) ने AcquireAccum को फिर से व्यवस्थित किया
* [Commit 39e5](https://github.com/manticoresoftware/manticoresearch/commit/39e5bc3751b7295222eb76407c5d72ce1dad545b) ने pq को कॉल करने के बाद मेमोरी के लीक को ठीक किया
* [Commit 21bc](https://github.com/manticoresoftware/manticoresearch/commit/21bcc6d17395f0f57dde79f6716ef303b7ea527d) कॉस्मेटिक पुनर्व्यवस्था (c++11 शैली c-trs, डिफ़ॉल्ट, nullptrs)
* [Commit 2d69](https://github.com/manticoresoftware/manticoresearch/commit/2d690398f14c736956cfdd66feb4d3091d6b3a4d) PQ इंडेक्स में डुप्लिकेट डालने की कोशिश करने पर मेमोरी लीक को ठीक किया
* [Commit 5ed9](https://github.com/manticoresoftware/manticoresearch/commit/5ed92c4b66da6423fa28f11b1fd59103ed1e1fb9) बड़े मानों के साथ JSON फ़ील्ड में IN पर क्रैश को ठीक किया
* [Commit 4a52](https://github.com/manticoresoftware/manticoresearch/commit/4a5262e2776aa261e34ba5c914ea60804f59de3f) एक्सपांशन लिमिट सेट करते समय RT इंडेक्स में CALL KEYWORDS स्टेटमेंट पर सर्वर के क्रैश को ठीक किया
* [Commit 5526](https://github.com/manticoresoftware/manticoresearch/commit/552646bb6fefde4e2525298e43e628839b421d3d) PQ मेल क्वेरी में अमान्य फ़िल्टर को ठीक किया;
* [Commit 204f](https://github.com/manticoresoftware/manticoresearch/commit/204f521408b730198a5bab2c20407a3d7df94873) ptr attrs के लिए छोटे obj allocator को पेश किया
* [Commit 2545](https://github.com/manticoresoftware/manticoresearch/commit/25453e5387152c0575d1eda1b2beb89a49c7493a) ISphFieldFilter को refcounted फ्लेवर में पुनर्व्यवस्थित किया
* [Commit 1366](https://github.com/manticoresoftware/manticoresearch/commit/1366ee06e828397074b69bec9265d7147170c60e) गैर-समाप्त स्ट्रिंग पर strtod का उपयोग करते समय ub/sigsegv को ठीक किया
* [Commit 94bc](https://github.com/manticoresoftware/manticoresearch/commit/94bc6fcbdae4d08bcf6da2966e3ec2966091289f) json परिणाम सेट प्रसंस्करण में मेमोरी लीक को ठीक किया
* [Commit e78e](https://github.com/manticoresoftware/manticoresearch/commit/e78e9c948963416caff9e4e46296a58080107835) एट्रिब्यूट जोड़ने के दौरान मेम ब्लॉक के अंत में पढ़ने को ठीक किया
* [Commit fad5](https://github.com/manticoresoftware/manticoresearch/commit/fad572fb543606b33e688eb82a485b86011545fd) refcount फ्लेवर के लिए CSphDict को ठीक किया
* [Commit fd84](https://github.com/manticoresoftware/manticoresearch/commit/fd841a472b472280c93f4af3506851d19f6adc45) AOT आंतरिक प्रकार के लीक को ठीक किया
* [Commit 5ee7](https://github.com/manticoresoftware/manticoresearch/commit/5ee7f208c7685f2ebb9e5623a8802b721e2a7f0a) टोकनाइज़र प्रबंधन में मेमोरी लीक को ठीक किया
* [Commit 116c](https://github.com/manticoresoftware/manticoresearch/commit/116c5f1abebee9a0d99afe93546a1d8e4c6c6958) ग्रुपर में मेमोरी लीक को ठीक किया
* [Commit 56fd](https://github.com/manticoresoftware/manticoresearch/commit/56fdbc9ab38973a9a0ba8ccee45f71cf33c16423) मेलों में गतिशील ptrs के लिए विशेष मुक्त/प्रतिलिपि (मेमोरी लीक ग्रुपर)
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/31db660f378541aa0d1e7cf2e7209bb0a1f47fd8) RT के लिए गतिशील स्ट्रिंग्स के मेमोरी लीक को ठीक किया
* [Commit 517b](https://github.com/manticoresoftware/manticoresearch/commit/517b9e8749f092e923cd2884b674b5bb84e20755) ग्रुपर को पुनर्व्यवस्थित किया
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) छोटे पुनर्व्यवस्था (c++11 c-trs, कुछ पुनः फॉर्मेट करें)
* [Commit 7034](https://github.com/manticoresoftware/manticoresearch/commit/7034e073f4cf2844762e0a464b7c3de05d3d122b) ISphMatchComparator को refcounted फ्लेवर में पुनर्व्यवस्थित किया
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) क्लोनर को निजीकरण किया
* [Commit efbc](https://github.com/manticoresoftware/manticoresearch/commit/efbc0511d6809c4a57453c7283f9bf53f3fb8d97) MVA_UPSIZE, DOCINFO2ID_T, DOCINFOSETID के लिए स्वदेशी लिटिल-एंडियन को सरल बनाना
* [Commit 6da0](https://github.com/manticoresoftware/manticoresearch/commit/6da0df4ac9946fc59f98d229b90e12c9221dd4c0) उबरटेस्ट के लिए valgrind समर्थन जोड़ें
* [Commit 1d17](https://github.com/manticoresoftware/manticoresearch/commit/1d17669cb7ec54ac80b392e101f37a688eb98080) कनेक्शन पर 'सफलता' ध्वज की दौड़ के कारण क्रैश को ठीक किया
* [Commit 5a09](https://github.com/manticoresoftware/manticoresearch/commit/5a09c32d7ded56b2c0bd3e2ad7968cb8d6362919) epoll को एज-ट्रिगर फ्लेवर पर स्विच किया
* [Commit 5d52](https://github.com/manticoresoftware/manticoresearch/commit/5d528682737fca03a4352a3093b1319ec27d598c) फ़िल्टर में रूपांतरण के समान प्रारूप में अभिव्यक्ति में IN स्टेटमेंट को ठीक किया
* [Commit bd8b](https://github.com/manticoresoftware/manticoresearch/commit/bd8b3c976ff8b4667f55dfdd101b20a920137ac5) बड़े docid के साथ दस्तावेज़ के प्रतिबद्धता पर RT इंडेक्स में क्रैश को ठीक किया
* [Commit ce65](https://github.com/manticoresoftware/manticoresearch/commit/ce656b83b747ce7c55795b53915770c13fb99cfe) indextool में argless विकल्प को ठीक किया
* [Commit 08c9](https://github.com/manticoresoftware/manticoresearch/commit/08c9507177820f441f534bf06a11dac5e54bebb4) विस्तारित कीवर्ड के मेमोरी लीक को ठीक किया
* [Commit 30c7](https://github.com/manticoresoftware/manticoresearch/commit/30c75a2f525ec9bda625d903acdc9d4d2e2413dc) json ग्रुपर के मेमोरी लीक को ठीक किया
* [Commit 6023](https://github.com/manticoresoftware/manticoresearch/commit/6023f269b6f2080e4d380b0957605ef8107c8c9f) वैश्विक उपयोगकर्ता चर के लीक को ठीक किया
* [Commit 7c13](https://github.com/manticoresoftware/manticoresearch/commit/7c138f15ca23c0c8717fa12041e3db7f988fcf15) प्रारंभिक अस्वीकृत मेलों पर गतिशील स्ट्रिंग्स के रिसाव को ठीक किया
* [Commit 9154](https://github.com/manticoresoftware/manticoresearch/commit/9154b18eaed5302e8330cb3a95bd968959ccb312) length(<expression>) पर रिसाव को ठीक किया
* [Commit 43fc](https://github.com/manticoresoftware/manticoresearch/commit/43fca3a4e26139765d0fac8de054200dc4875fc6) ने पार्सर में strdup() के कारण मेमोरी लीक को ठीक किया
* [Commit 71ff](https://github.com/manticoresoftware/manticoresearch/commit/71ff77737e63d1019b7c9804dca7fa2351025dba) ने रेफरेंस काउंट का सटीक पालन करने के लिए एक्सप्रेशन पार्सर को ठीक किया

## Version 2.7.2 GA, 27 अगस्त 2018
### सुधार
* MySQL 8 क्लाइंट के साथ संगतता
* [TRUNCATE](Emptying_a_table.md) WITH RECONFIGURE
* RT इंडेक्स के लिए SHOW STATUS पर रिटायर किया गया मेमोरी काउंटर
* मल्टी एजेंट का वैश्विक कैश
* विंडोज़ पर IOCP में सुधार
* HTTP प्रोटोकॉल के लिए VIP कनेक्शन
* Manticore SQL [DEBUG](Reporting_bugs.md#DEBUG) कमांड जो विभिन्न सबकमांड चला सकता है
* [shutdown_token](Server_settings/Searchd.md#shutdown_token) - SHA1 हैश पासवर्ड की आवश्यकता है जो DEBUG कमांड का उपयोग करके `shutdown` को सक्रिय करने के लिए आवश्यकता है
* SHOW AGENT STATUS (_ping, _has_perspool, _need_resolve) के लिए नए आंकड़े
* --verbose विकल्प जो **indexer** अब \[debugvv\] के लिए डिबग संदेश प्रिंट करने के लिए स्वीकार करता है

### बग सुधार
* [Commit 3900](https://github.com/manticoresoftware/manticoresearch/commit/390082a7be0a1f9539b30361d11d54de35c62a44) ने ऑप्टिमाइज पर wlock हटा दिया
* [Commit 4c33](https://github.com/manticoresoftware/manticoresearch/commit/4c337648329be22373be33333cbc9c5664d18b80) ने इंडेक्स सेटिंग्स को रीलोड करते समय wlock को ठीक किया
* [Commit b5ea](https://github.com/manticoresoftware/manticoresearch/commit/b5ea8dc0b18a23dbd739e92c66dd93b0094df14b) ने JSON फ़िल्टर के साथ क्वेरी पर मेमोरी लीक को ठीक किया
* [Commit 930e](https://github.com/manticoresoftware/manticoresearch/commit/930e831a43fd3d1ce996185be968d1df49a088d0) ने PQ परिणाम सेट में खाली दस्तावेज़ को ठीक किया
* [Commit 53de](https://github.com/manticoresoftware/manticoresearch/commit/53deecfecb374431f84f8592a1095a77407b8aea) ने हटाए गए एक के कारण कार्यों में भ्रम को ठीक किया
* [Commit cad9](https://github.com/manticoresoftware/manticoresearch/commit/cad9b9c617719aad97e9f5a2ae93491153d8e504) ने गलत दूरस्थ होस्ट गिनती को ठीक किया
* [Commit 9000](https://github.com/manticoresoftware/manticoresearch/commit/90008cc15c4684d445d37a391bb4a667bbc2e365) ने पार्स किए गए एजेंट विवरणों के मेमोरी लीक को ठीक किया
* [Commit 978d](https://github.com/manticoresoftware/manticoresearch/commit/978d839641ad6040c22c9fc3b703b1a02685f266) ने खोज में लीक को ठीक किया
* [Commit 0193](https://github.com/manticoresoftware/manticoresearch/commit/0193946105ac08913362b7d7acf0c90f9b38dc85) ने स्पष्ट/इनलाइन c-trs, ओवरराइड/फाइनल उपयोग पर कॉस्मेटिक परिवर्तन किए
* [Commit 943e](https://github.com/manticoresoftware/manticoresearch/commit/943e2997b33f9b2eda23d94a66068910f9476ebd) ने स्थानीय/दूरस्थ स्कीमा में json के लीक को ठीक किया
* [Commit 02db](https://github.com/manticoresoftware/manticoresearch/commit/02dbdd6f1d0b72e7e458ebebdd2c67f989577e6b) ने स्थानीय/दूरस्थ स्कीमा में json सॉर्टिंग col expr के लीक को ठीक किया
* [Commit c74d](https://github.com/manticoresoftware/manticoresearch/commit/c74d0b4035e4214510376845b22cb676a8da24a3) ने const उपनाम के लीक को ठीक किया
* [Commit 6e5b](https://github.com/manticoresoftware/manticoresearch/commit/6e5b57e1d3952c9695376728a143bf5434208f53) ने preread थ्रेड के लीक को ठीक किया
* [Commit 39c7](https://github.com/manticoresoftware/manticoresearch/commit/39c740fef5bf71da0bee60a35f5dd9f471af850f) ने netloop में फंसे हुए वेट के कारण प्रस्थान पर फंसे होने को ठीक किया
* [Commit adaf](https://github.com/manticoresoftware/manticoresearch/commit/adaf9772f95772ad6d5297acace6c5cb92b56ad5) ने सामान्य होस्ट में HA एजेंट को बदलने पर 'ping' व्यवहार के फंसे को ठीक किया
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/32c40eac9cdd40b15d79e58b2f15416f164f9701) डैशबोर्ड स्टोरेज के लिए अलग gc
* [Commit 511a](https://github.com/manticoresoftware/manticoresearch/commit/511a3cf2c1f1858f6e94fe9f8175b7216db3cbd6) ने रेफ-गिनती वाले ptr को ठीक किया
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/af2326cd2927effbad7bec88e8dc238a566cf27e) ने गैर-मौजूद इंडेक्स पर indextool क्रैश को ठीक किया
* [Commit 156e](https://github.com/manticoresoftware/manticoresearch/commit/156edc717dc63d3ea120f42466eba6e022da88be) ने xmlpipe इंडेक्सिंग में खत्म हुए attr/field का आउटपुट नाम ठीक किया
* [Commit cdac](https://github.com/manticoresoftware/manticoresearch/commit/cdac6d1f17d384bfc8bae49e91241b2f346c3830) ने config में कोई इंडेक्सर सेक्शन न होने पर डिफ़ॉल्ट इंडेक्सर का मान ठीक किया
* [Commit e61e](https://github.com/manticoresoftware/manticoresearch/commit/e61ec00b6b27d1d5878247e2ee817f3b1e7fde16) ने सर्वर पुनरारंभ के बाद RT इंडेक्स द्वारा डिस्क खंड में गलत अंतर्निहित स्टॉपवर्ड को ठीक किया
* [Commit 5fba](https://github.com/manticoresoftware/manticoresearch/commit/5fba49d5e28c17de4c0acbd984466127f42de6e8) ने फ़ैंटम (पहले ही बंद, लेकिन पूलर से अंततः हटाए नहीं गए) कनेक्शनों को छोड़ने को ठीक किया
* [Commit f22a](https://github.com/manticoresoftware/manticoresearch/commit/f22ae34b623906b3c5a05a06198dad2e548dc541) ने मिश्रित (अनाथ) नेटवर्क कार्यों को ठीक किया
* [Commit 4689](https://github.com/manticoresoftware/manticoresearch/commit/46890e70eb8dcd00c1bf1b030538b8f131c601c2) ने लिखने के बाद पढ़ने की क्रिया पर क्रैश को ठीक किया
* [Commit 03f9](https://github.com/manticoresoftware/manticoresearch/commit/03f9df904f611c438d5ebcaaeef911b0dc8d6c39) ने विंडोज़ पर परीक्षण चलाने के दौरान searchd के क्रैश को ठीक किया
* [Commit e925](https://github.com/manticoresoftware/manticoresearch/commit/e9255ed2704790a3d7f5273d38b85433463cb3ff) ने सामान्य connect() पर EINPROGRESS कोड को संभालने को ठीक किया
* [Commit 248b](https://github.com/manticoresoftware/manticoresearch/commit/248b72f1544b27e9e82773284050d44eeb1714e1) ने TFO के साथ काम करते समय कनेक्शन टाइमआउट को ठीक किया

## Version 2.7.1 GA, 4 जुलाई 2018
### सुधार
* PQ पर कई दस्तावेज़ों से मेल खाने पर वाइल्डकार्ड प्रदर्शन में सुधार
* PQ पर फुलस्कैन क्वेरियों के लिए समर्थन
* PQ पर MVA गुणों के लिए समर्थन
* पर्कोलेट इंडेक्स के लिए regexp और RLP समर्थन

### बग सुधार
* [Commit 6885](https://github.com/manticoresoftware/manticoresearch/commit/68856261b41f6950666f9e5122726839fb3c71d0) ने क्वेरी स्ट्रिंग के लूज को ठीक किया
* [Commit 0f17](https://github.com/manticoresoftware/manticoresearch/commit/0f1770943cba53bfd2e0edfdf0b1495ff0dd0e89) ने SHOW THREADS कथन पर खाली जानकारी को ठीक किया
* [Commit 53fa](https://github.com/manticoresoftware/manticoresearch/commit/53faa36a916bb87868e83aa79c1a1f972fb20ca0) ने NOTNEAR ऑपरेटर के साथ मिलान के दौरान क्रैश को ठीक किया
* [Commit 2602](https://github.com/manticoresoftware/manticoresearch/commit/26029a7cd77b518fb3a27588d7db6c8bffc73b0f) ने PQ हटाने के लिए खराब फ़िल्टर पर त्रुटि संदेश को ठीक किया


## Version 2.7.0 GA, 11 जून 2018
### सुधार
* मेल्टडाउन और स्पेक्टर पैच के प्रभाव से बचने के लिए syscall की संख्या को घटाया
* स्थानीय अनुक्रमणिका प्रबंधन का आंतरिक फिर से लेखन
* दूरस्थ स्निपेट्स का पुनर्गठन
* पूर्ण कॉन्फ़िगरेशन फिर से लोड
* सभी नोड कनेक्शन अब स्वतंत्र हैं
* प्रोटोटाइप सुधार
* Windows संचार अब wsapoll से IO पूर्णता पोर्ट पर स्विच किया गया
* TFO मास्टर और नोड के बीच संचार के लिए उपयोग किया जा सकता है
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) अब सर्वर संस्करण और mysql_version_string आउटपुट करता है
* CALL PQ में बुलाए गए दस्तावेज़ों के लिए `docs_id` विकल्प जोड़ा गया।
* पर्सकोलेट क्वेरी फ़िल्टर अब अभिव्यक्तियों को शामिल कर सकता है
* वितरित अनुक्रमणिकाएँ FEDERATED के साथ कार्य कर सकती हैं
* बेहतर ProxySQL संगतता (dummy SHOW NAMES COLLATE और `SET wait_timeout`)

### बग सुधार
* [Commit 5bcf](https://github.com/manticoresoftware/manticoresearch/commit/5bcff05d19e495f1a44f190885b25cbb6f217c4a) ने PQ के लिए न बराबर टैग को ठीक किया
* [Commit 9ebc](https://github.com/manticoresoftware/manticoresearch/commit/9ebc58916cd515eaa88da66d0895aebf0d1f2b5f) ने JSON दस्तावेज़ CALL PQ कथन में दस्तावेज़ आईडी फ़ील्ड को जोड़ा
* [Commit 8ae0](https://github.com/manticoresoftware/manticoresearch/commit/8ae0e593a286110d8505d88baa3ac9e433cb38c9) ने PQ अनुक्रमणिका के लिए फ़्लश कथन हैंडलर्स को ठीक किया
* [Commit c24b](https://github.com/manticoresoftware/manticoresearch/commit/c24b152344ecd77661566cddb803487f51d3c1aa) ने JSON और स्ट्रिंग विशेषताओं पर PQ फ़िल्टरिंग को ठीक किया
* [Commit 1b8b](https://github.com/manticoresoftware/manticoresearch/commit/1b8bdde19aaa362785ea4c33bdc019154f8fbac4) ने खाली JSON स्ट्रिंग के विश्लेषण को ठीक किया
* [Commit 1ad8](https://github.com/manticoresoftware/manticoresearch/commit/1ad8a07dbfc1131913a8d92c261fbb48f934e8b7) ने OR फ़िल्टर के साथ मल्टी-क्वेरी में क्रैश को ठीक किया
* [Commit 69b8](https://github.com/manticoresoftware/manticoresearch/commit/69b89806380a229e36287ad9daf6f0ea2b5453eb) ने कमांड (dumpheader) के लिए कॉन्फ़िग कॉमन सेक्शन (lemmatizer_base विकल्प) का उपयोग करने के लिए indextool को ठीक किया
* [Commit 6dbe](https://github.com/manticoresoftware/manticoresearch/commit/6dbeaf2389d2af6a9dfccb56bfc986685b9f203e) ने परिणाम सेट और फ़िल्टर में खाली स्ट्रिंग को ठीक किया
* [Commit 39c4](https://github.com/manticoresoftware/manticoresearch/commit/39c4eb55ecc85f6dd54f6c8c6d6dfcf87fd8748e) ने नकारात्मक दस्तावेज़ आईडी मानों को ठीक किया
* [Commit 266b](https://github.com/manticoresoftware/manticoresearch/commit/266b7054f98e7d30fc2829b5c2467dc9f90def92) ने अत्यधिक लंबे शब्दों के लिए शब्द क्लिप लंबाई को ठीक किया
* [Commit 4782](https://github.com/manticoresoftware/manticoresearch/commit/47823b0bd06cbd016a24345c99bb8693790b43dd) ने PQ पर वाइल्डकार्ड क्वेरी के कई दस्तावेजों के मिलान को ठीक किया


## Version 2.6.4 GA, 3 मई 2018
### विशेषताएँ और सुधार
* MySQL FEDERATED इंजन [सहायता](Extensions/FEDERATED.md)
* MySQL पैकेट अब SERVER_STATUS_AUTOCOMMIT ध्वज लौटाते हैं, जो ProxySQL के साथ संगतता जोड़ता है
* [listen_tfo](Server_settings/Searchd.md#listen_tfo) - सभी श्रोता के लिए TCP फास्ट ओपन कनेक्शन सक्षम करें
* indexer --dumpheader अब .meta फ़ाइल से RT हेडर को भी डंप कर सकता है
* Ubuntu Bionic के लिए cmake बिल्ड स्क्रिप्ट

### बग सुधार
* [Commit 355b](https://github.com/manticoresoftware/manticoresearch/commit/355b11629174813abd9cd5bf2233be0783f77745) ने RT अनुक्रमणिका के लिए अमान्य क्वेरी कैश प्रविष्टियों को ठीक किया;
* [Commit 546e](https://github.com/manticoresoftware/manticoresearch/commit/546e2297a01cc2913bc0d33052d5bcefae8737eb) ने निर्बाध रोटेशन के बाद अनुक्रमणिका सेटिंग्स खो जाने को ठीक किया
* [Commit 0c45](https://github.com/manticoresoftware/manticoresearch/commit/0c4509898393993bc87194d2aca2070395ff7f83) ने इनफिक्स बनाम प्रीफ़िक्स लंबाई सेट की; असमर्थित इनफिक्स लंबाई पर चेतावनी जोड़ी
* [Commit 8054](https://github.com/manticoresoftware/manticoresearch/commit/80542fa54d727c781635918e8d4d65a20c2fbd1e) ने RT अनुक्रमणिकाओं के ऑटो-फ्लश आदेश को ठीक किया
* [Commit 705d](https://github.com/manticoresoftware/manticoresearch/commit/705d8c5f407726ed1c6f9745ecffa375682c8969) ने कई विशेषताओं और कई अनुक्रमणिकाओं को जोड़ने के लिए परिणाम सेट स्कीमा मुद्दों को ठीक किया
* [Commit b0ba](https://github.com/manticoresoftware/manticoresearch/commit/b0ba932fff4a082f642cb737381c8a70215d4cc9) ने दस्तावेज़ डुप्लिकेट के साथ बैच समावेशन में कुछ हिट खोने को ठीक किया
* [Commit 4510](https://github.com/manticoresoftware/manticoresearch/commit/4510fa44ad54b5bbac53c591144501565130fcae) ने बड़े दस्तावेज़ों की संख्या के साथ RT अनुक्रमणिका के डिस्क टुकड़ों को विलय करने में अनुकूलन की विफलता को ठीक किया

## Version 2.6.3 GA, 28 मार्च 2018
### सुधार
* संकलन पर jemalloc। यदि सिस्टम पर jemalloc उपस्थित है, तो इसे cmake ध्वज `-DUSE_JEMALLOC=1` के साथ सक्षम किया जा सकता है

### बग सुधार
* [Commit 85a6](https://github.com/manticoresoftware/manticoresearch/commit/85a6d7e3db75f28c491042409bb677ed39eba07b) ने Manticore SQL क्वेरी लॉग में expand_keywords विकल्प को ठीक किया
* [Commit caaa](https://github.com/manticoresoftware/manticoresearch/commit/caaa3842f4dbd8507fd9e20bf3371a0c8ec0c087) ने बड़े आकार के क्वेरी को सही ढंग से संसाधित करने के लिए HTTP इंटरफ़ेस को ठीक किया
* [Commit e386](https://github.com/manticoresoftware/manticoresearch/commit/e386d842b36195719e459b2db1fb20e402f9a51f) ने index_field_lengths सक्षम करने पर RT अनुक्रमिका को DELETE पर सर्वर के क्रैश को ठीक किया
* [Commit cd53](https://github.com/manticoresoftware/manticoresearch/commit/ce528f37eb5d8284661928f6f3b340a4a1985253) ने cpustats searchd CLI विकल्प को Unsupported सिस्टम के साथ कार्य करने के लिए ठीक किया
* [Commit 8740](https://github.com/manticoresoftware/manticoresearch/commit/8740fd63ae842c2ab0e51a8fcd6180d787ed57ec) ने न्यूनतम लंबाई निर्धारित करने के साथ utf8 उपस्ट्रिंग मिलान को ठीक किया


## संस्करण 2.6.2 GA, 23 फरवरी 2018
### सुधार
* एक NOT ऑपरेटर का उपयोग करते समय और बैच दस्तावेज़ों के लिए [Percolate Queries](Searching/Percolate_query.md) के प्रदर्शन में सुधार किया गया।
* [percolate_query_call](Searching/Percolate_query.md) [dist_threads](Server_settings/Searchd.md#threads) के आधार पर एक से अधिक थ्रेड्स का उपयोग कर सकता है
* नया पूर्ण-टेक्स्ट मिलान ऑपरेटर NOTNEAR/N
* percolate इंडेक्स पर SELECT के लिए LIMIT
* [expand_keywords](Searching/Options.md#expand_keywords) 'start','exact' (जहां 'star,exact' का वही प्रभाव है जो '1' का है) स्वीकार कर सकता है
* [joined fields](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_joined_field) के लिए रेंज-मुख्य-प्रश्न जो sql_query_range द्वारा परिभाषित रेंज प्रश्न का उपयोग करता है

### बग फिक्स
* [Commit 72dc](https://github.com/manticoresoftware/manticoresearch/commit/72dcf669744e9b7d636dfc213d24df85ab301f6b) ने राम खंडों को खोजने पर क्रैश; डबल बफ़र के साथ डिस्क खंड को सहेजते समय डेडलॉक; ऑप्टिमाइज़ के दौरान डिस्क खंड को सहेजते समय डेडलॉक को ठीक किया
* [Commit 3613](https://github.com/manticoresoftware/manticoresearch/commit/36137149a1c3c0893bdda5a28fc7e8244bf2d4ae) ने खाली विशेषता नाम के साथ xml एम्बेडेड स्कीमा पर इंडेक्सर क्रैश को ठीक किया
* [Commit 48d7](https://github.com/manticoresoftware/manticoresearch/commit/48d7e8001d2a66466ca64577f27ddc5421a67251) ने नॉन-ओन्ड pid-file के गलत लिंक को ठीक किया
* [Commit a556](https://github.com/manticoresoftware/manticoresearch/commit/a5563a465ddc59ef71e65f17b68bc33f9700e838) ने कभी-कभी अस्थायी फ़ोल्डर में छोड़े गए अनाथ फिफोज़ को ठीक किया
* [Commit 2376](https://github.com/manticoresoftware/manticoresearch/commit/2376e8fc4508944b96959bd10686c6d51f5145e8) ने गलत NULL पंक्ति के साथ खाली FACET परिणाम सेट को ठीक किया
* [Commit 4842](https://github.com/manticoresoftware/manticoresearch/commit/4842b67acdd5da75672db6c08ac563b48577ddd7) ने विंडोज़ सेवा के रूप में सर्वर चलाते समय टूटी हुई इंडेक्स लॉक को ठीक किया
* [Commit be35](https://github.com/manticoresoftware/manticoresearch/commit/be35feef54ded59125750916cd211e56108eddae) ने मैक ओएस पर गलत iconv लाइब्रेरी को ठीक किया
* [Commit 8374](https://github.com/manticoresoftware/manticoresearch/commit/83744a977d7837d1d87cd506b88b0e7eb199efe6) ने गलत count(*) को ठीक किया


## संस्करण 2.6.1 GA, 26 जनवरी 2018
### सुधार
* दर्पणों के साथ एजेंटों के मामले में [agent_retry_count](Server_settings/Searchd.md#agent_retry_count) प्रति दर्पण के लिए प्रयासों का मान देता है, जबकि कुल प्रयास agent_retry_count*mirrors होते हैं।
* [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) अब प्रति इंडेक्स निर्दिष्ट किया जा सकता है, जो वैश्विक मान को ओवरराइड करता है। एक उपनाम [mirror_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count) जोड़ा गया है।
* एक retry_count एजेंट परिभाषा में निर्दिष्ट किया जा सकता है और उस मान का अर्थ है प्रति एजेंट प्रयास
* Percolate Queries अब HTTP JSON API में [/json/pq](Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ) पर हैं।
* executables में -h और -v विकल्प (सहायता और संस्करण) जोड़े गए हैं
* [morphology_skip_fields](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) रीयल-टाइम इंडेक्स के लिए समर्थन

### बग फिक्स
* [Commit a40b](https://github.com/manticoresoftware/manticore/commit/a40b0793feff65e40d10062568d9847c08d10f57) ने MVA क्षेत्र पर sql_range_step के साथ सही ढंग से काम करने के लिए ranged-main-query को ठीक किया
* [Commit f2f5](https://github.com/manticoresoftware/manticore/commit/f2f53757db45bcfb1544263ce0817e856656a621) ने काले छिद्र प्रणाली लूप के थमने और काले छिद्र एजेंटों के अज्ञात महसूस किए जाने की समस्या को ठीक किया
* [Commit 84e1](https://github.com/manticoresoftware/manticore/commit/84e1f54aef25e0fce98870ad2dd784db5116f1d6) ने प्रश्न आईडी को स्थिर करने के लिए ठीक किया, संग्रहीत प्रश्नों के लिए डुप्लिकेट आईडी को ठीक किया
* [Commit 1948](https://github.com/manticoresoftware/manticore/commit/19484231814fcb82b21763a3a4a9f45adc6b2d40) ने विभिन्न राज्यों से बंद होने पर सर्वर क्रैश को ठीक किया
* [Commit 9a70](https://github.com/manticoresoftware/manticore/commit/9a706b499a1d61a90076065a1a703029d49db958)  [Commit 3495](https://github.com/manticoresoftware/manticore/commit/3495fd70cba8846b1a50d55d6679b039414c3d2a) लंबी प्रश्नों पर टाइमआउट
* [Commit 3359](https://github.com/manticoresoftware/manticore/commit/3359bcd89b4f79a645fe84b8cf8616ce0addff02) ने kqueue-आधारित प्रणालियों (मैक ओएस एक्स, BSD) पर मास्टर-एजेंट नेटवर्क पोलिंग को पुनर्गठित किया।


## संस्करण 2.6.0, 29 दिसंबर 2017
### विशेषताएँ और सुधार
* [HTTP JSON](Connecting_to_the_server/HTTP.md): JSON प्रश्न अब विशेषताओं पर समानता कर सकते हैं, MVA और JSON विशेषताएँ सम्मिलनों और अद्यतनों में उपयोग की जा सकती हैं, वितरित इंडेक्स पर JSON API के माध्यम से अद्यतन और हटाने किया जा सकता है
* [Percolate Queries](Searching/Percolate_query.md)
* 32-बिट docids के लिए कोड से समर्थन हटा दिया गया। 32-बिट docids के साथ विरासत इंडेक्स को परिवर्तित/लोड करने के लिए सभी कोड को भी हटा दिया गया।
* [निश्चित क्षेत्रों के लिए केवल रूपविज्ञान](https://github.com/manticoresoftware/manticore/issues/7) . एक नया इंडेक्स निदेशांक morphology_skip_fields बनाना एक सूची को परिभाषित करने की अनुमति देता है जिसके लिए रूपविज्ञान लागू नहीं होता है।
* [expand_keywords अब OPTION कथन का उपयोग करके एक प्रश्न रनटाइम निदेशांक हो सकता है](https://github.com/manticoresoftware/manticore/issues/8)

### बग फिक्स
* [Commit 0cfa](https://github.com/manticoresoftware/manticore/commit/0cfae4c) ने rlp के साथ बनाए जाने पर सर्वर के डिबग निर्माण (और m.b. UB पर रिलीज) पर क्रैश को ठीक किया
* [Commit 3242](https://github.com/manticoresoftware/manticore/commit/324291e) ने गलत क्रम के साथ मर्ज की गई किल-सूचियों के साथ प्रगतिशील विकल्प सक्षम करने के साथ RT इंडेक्स ऑप्टिमाइज़ को ठीक किया
* [Commit ac0e](https://github.com/manticoresoftware/manticore/commit/ac0efee)  mac पर हल्का क्रैश
* गहन स्टेटिक कोड विश्लेषण के बाद कई छोटे फिक्स
* अन्य छोटे बगफिक्स

### अपग्रेड
इस रिलीज में हमने मास्टर और एजेंट के बीच बातचीत के लिए उपयोग किए गए आंतरिक प्रोटोकॉल में बदलाव किया है। यदि आप मैन्टिकोर सर्च को एक वितरित वातावरण में कई उदाहरणों के साथ चला रहे हैं, तो सुनिश्चित करें कि आप पहले एजेंट को अपडेट करें, फिर मास्टर को।

## संस्करण 2.5.1, 23 नवंबर 2017
### विशेषताएँ और सुधार
* [HTTP API प्रोटोकॉल](Connecting_to_the_server/HTTP.md) पर JSON क्वेरियाँ। समर्थित खोज, सम्मिलित, अपडेट, हटाने, प्रतिस्थापन कार्यक्षमता। डेटा हेरफेर आदेशों को भी बल्क में किया जा सकता है, वर्तमान में कुछ प्रतिबंध हैं क्योंकि MVA और JSON विशेषताएँ सम्मिलित, प्रतिस्थापित या अपडेट के लिए उपयोग नहीं की जा सकती हैं।
* [RELOAD INDEXES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES) कमांड
* [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md) कमांड
* [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) ऑप्टिमाइज़ेशन, रोटेशन या फ्लश के प्रगति को दिखा सकता है।
* GROUP N द्वारा MVA विशेषताओं के साथ सही तरीके से कार्य करता है
* ब्लैकहोल एजेंट अब अलग थ्रेड पर चलाए जाते हैं ताकि मास्टर क्वेरी को प्रभावित नहीं करें
* रोटेशन और उच्च लोड के कारण रुकावट से बचने के लिए सूचकांक पर संदर्भ गिनती लागू की गई है
* SHA1 हैशिंग लागू की गई है, अभी तक बाहरी रूप से प्रदर्शित नहीं किया गया है
* FreeBSD, macOS और Alpine पर संकलन के लिए सुधार

### बगफिक्स
* [Commit 9897](https://github.com/manticoresoftware/manticore/commit/989752b) ब्लॉक इंडेक्स के साथ फ़िल्टर रिग्रेशन
* [Commit b1c3](https://github.com/manticoresoftware/manticore/commit/b1c3864) PAGE_SIZE का नाम बदलकर ARENA_PAGE_SIZE किया गया है ताकि musl के साथ संगतता हो
* [Commit f213](https://github.com/manticoresoftware/manticore/commit/f2133cc) cmake < 3.1.0 के लिए googletests कोdisable किया गया
* [Commit f30e](https://github.com/manticoresoftware/manticore/commit/0839de7) सर्वर पुनरारंभ पर सॉकेट को जोड़ने में विफल
* [Commit 0807](https://github.com/manticoresoftware/manticore/commit/0807240) शटडाउन पर सर्वर के क्रैश को ठीक किया गया
* [Commit 3e3a](https://github.com/manticoresoftware/manticore/commit/3e3acc3) सिस्टम ब्लैकहोल थ्रेड के लिए थ्रेड्स दिखाने में सुधार किया गया
* [Commit 262c](https://github.com/manticoresoftware/manticore/commit/262c3fe) iconv का कॉन्फ़िगरेशन चेक सुधारित, FreeBSD और डार्विन पर निर्माण के लिए सुधार

## संस्करण 2.4.1 GA, 16 अक्टूबर 2017
### विशेषताएँ और सुधार
* WHERE क्लॉज़ में विशेषता फ़िल्टर के बीच OR ऑपरेटर
* रखरखाव मोड ( SET MAINTENANCE=1)
* [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS) वितरित सूचकांकों पर उपलब्ध हैं
* [UTC में समूहबद्ध करना](Server_settings/Searchd.md#grouping_in_utc)
* [query_log_mode](Server_settings/Searchd.md#query_log_mode) कस्टम लॉग फ़ाइलों के अनुमतियों के लिए
* फ़ील्ड वजन शून्य या नकारात्मक हो सकते हैं
* [max_query_time](Searching/Options.md#max_query_time) अब पूर्ण-स्कैन पर प्रभाव डाल सकता है
* नेटवर्क थ्रेड के तंतुविक समायोजन के लिए [net_wait_tm](Server_settings/Searchd.md#net_wait_tm), [net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) और [net_throttle_action](Server_settings/Searchd.md#net_throttle_action) जोड़े गए
* COUNT DISTINCT फ़ेसट खोजों के साथ कार्य करता है
* IN को JSON फ्लोट एरे के साथ उपयोग किया जा सकता है
* पूर्ण-प्रश्न अनुकूलन अब पूर्णांक/फ्लोट अभिव्यक्तियों द्वारा बाधित नहीं है
* [SHOW META](Node_info_and_management/SHOW_META.md) एक `multiplier` पंक्ति दिखाता है जब बहु-प्रश्न अनुकूलन का उपयोग किया जाता है

### संकलन
मैन्टिकोर सर्च को cmake का उपयोग करके बनाया गया है और संकलन के लिए आवश्यक न्यूनतम gcc संस्करण 4.7.2 है।

### फ़ोल्डर और सेवा
* मैन्टिकोर सर्च `manticore` उपयोगकर्ता के तहत चलता है।
* डिफ़ॉल्ट डेटा फ़ोल्डर अब `/var/lib/manticore/` है।
* डिफ़ॉल्ट लॉग फ़ोल्डर अब `/var/log/manticore/` है।
* डिफ़ॉल्ट पीआईडी फ़ोल्डर अब `/var/run/manticore/` है।

### बगफिक्स
* [Commit a58c](https://github.com/manticoresoftware/manticore/commit/a58c619) SHOW COLLATION बयान को ठीक किया जो जावा कनेक्टर को तोड़ता है
* [Commit 631c](https://github.com/manticoresoftware/manticore/commit/631cf4e) वितरित सूचकांकों के प्रसंस्करण पर क्रैश को ठीक किया; वितरित सूचकांक हैश के लिए लॉक जोड़े; एजेंट से मूव और कॉपी ऑपरेटर हटाए गए
* [Commit 942b](https://github.com/manticoresoftware/manticore/commit/942bec0) वितरित सूचकांकों के प्रसंस्करण में समानांतर पुनः कनेक्ट के कारण क्रैश को ठीक किया
* [Commit e5c1](https://github.com/manticoresoftware/manticore/commit/e5c1ed2) सर्वर लॉग में क्वेरी संग्रहण पर क्रैश हैंडलर पर क्रैश को ठीक किया
* [Commit 4a4b](https://github.com/manticoresoftware/manticore/commit/4a4bda5) मल्टीक्वेरीज़ में पूल्ड विशेषताओं के साथ क्रैश को ठीक किया
* [Commit 3873](https://github.com/manticoresoftware/manticore/commit/3873bfb) कोर फ़ाइल में सूचकांक पृष्ठों के समावेशन को रोककर कोर आकार में कमी को ठीक किया
* [Commit 11e6](https://github.com/manticoresoftware/manticore/commit/11e6254) अवैध एजेंटों के निर्दिष्ट होने पर स्टार्टअप पर searchd क्रैश को ठीक किया
* [Commit 4ca6](https://github.com/manticoresoftware/manticore/commit/4ca6350) sql_query_killlist क्वेरी में अनुक्रमणिका रिपोर्ट त्रुटि को ठीक किया
* [Commit 123a](https://github.com/manticoresoftware/manticore/commit/123a9f0) fold_lemmas=1 बनाम हिट गिनती को ठीक किया
* [Commit cb99](https://github.com/manticoresoftware/manticore/commit/cb99164) html_strip का असंगत व्यवहार ठीक किया
* [Commit e406](https://github.com/manticoresoftware/manticore/commit/e406761) ऑप्टिमाइज़ rt सूचकांक नई सेटिंग्स को ढीला करना; लॉक लीक के साथ समन्वय विकल्प के साथ ऑप्टिमाइज़ करने के लिए ठीक किया;
* [Commit 86ae](https://github.com/manticoresoftware/manticore/commit/86aeb82) गलत मल्टीक्वेरीज़ के प्रसंस्करण को ठीक किया
* [Commit 2645](https://github.com/manticoresoftware/manticore/commit/2645230) परिणाम सेट को बहु-प्रश्न क्रम पर निर्भर करता है
* [Commit 7239](https://github.com/manticoresoftware/manticore/commit/72395d9) खराब क्वेरी के साथ बहु-प्रश्न पर सर्वर क्रैश को ठीक किया
* [Commit f353](https://github.com/manticoresoftware/manticore/commit/f353326) साझा से विशेषता लॉक को ठीक किया
* [Commit 3754](https://github.com/manticoresoftware/manticore/commit/3754785) सूचकांक के बिना क्वेरी के लिए सर्वर क्रैश को ठीक किया
* [Commit 29f3](https://github.com/manticoresoftware/manticore/commit/29f360e) सर्वर का मृत लॉक ठीक किया

## संस्करण 2.3.3, 06 जुलाई 2017
* मैन्टिकोर ब्रांडिंग





