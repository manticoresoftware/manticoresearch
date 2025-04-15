# Manticore और Kibana का एकीकरण

[Kibana](https://www.elastic.co/kibana) एक दृश्य इंटरफेस है जो आपको अपने लॉग डेटा का अन्वेषण, दृश्यता और डैशबोर्ड बनाने की अनुमति देता है। Manticore सर्च के साथ Kibana का एकीकरण Kibana दृश्यावलियों को Elasticsearch की तुलना में 3 गुना तेज़ लोड करने में मदद कर सकता है, जैसा कि इस [डेमो](https://github.com/manticoresoftware/kibana-demo#manticore-search-kibana-demo) में प्रदर्शित किया गया है। यह एकीकरण उपयोगकर्ताओं को इंटरएक्टिव डैशबोर्ड, कस्टम दृश्यावलियों और वास्तविक समय की खोज क्षमताओं का उपयोग करते हुए अपने डेटा का सहजता से विश्लेषण करने की अनुमति देता है। यह विविध डेटा स्रोतों को संभालना सरल बनाता है, जैसे Logstash और Filebeat जैसी उपकरणों का समर्थन करके डेटा संग्रह को सुगम बनाता है, जिससे यह लॉग विश्लेषण कार्यप्रवाहों के लिए एक उत्कृष्ट विकल्प बन जाता है।

## पूर्वापेक्षाएँ
1. **Kibana डाउनलोड करें**: सुनिश्चित करें कि आप Manticore के साथ संगत Kibana संस्करण डाउनलोड करें। वर्तमान में, संस्करण 7.6.0 का परीक्षण किया गया है और अनुशंसित है। अन्य 7.x संस्करण कार्य कर सकते हैं लेकिन समस्याएँ पैदा कर सकते हैं। संस्करण 8.x का समर्थन नहीं किया जाता है।
2. **Manticore की पुष्टि करें**: सुनिश्चित करें कि आपका Manticore इंस्टेंस चल रहा है और इसका HTTP API पहुंच योग्य है (डिफ़ॉल्ट: `http://localhost:9308`)।

## कॉन्फ़िगरेशन
1. [Kibana कॉन्फ़िगरेशन फ़ाइल](https://www.elastic.co/guide/en/kibana/current/settings.html) (`kibana.yml`) खोलें।
2. अपने Manticore इंस्टेंस का URL सेट करें:
   ```yaml
   elasticsearch.hosts: ["http://localhost:9308"]
   ```
3. Kibana शुरू करें और इसे अपने ब्राउज़र में `http://localhost:5601` पर खोलें। यदि आवश्यक हो तो `localhost` को अपने सर्वर के IP या होस्टनेम से बदलें।

> नोट: Kibana के साथ काम करते समय Manticore को प्रमाणीकरण सेटअप की आवश्यकता नहीं होती है।

## समर्थित सुविधाएँ
### खोजें
- डेटा को इंटरएक्टिव रूप से खोजने और फ़िल्टर करने के लिए Kibana में **खोजें** टैब का उपयोग करें।
- [Kibana क्वेरी भाषा](https://www.elastic.co/guide/en/kibana/current/kuery-query.html) में सरल क्वेरियों के साथ क्वेरी बार का उपयोग करके अपनी खोजों को परिष्कृत करें।

### दृश्यावलियाँ
- कस्टम दृश्यावलियाँ बनाने के लिए **दृश्यों** पर जाएं:
  - यदि कोई पहले से मौजूद नहीं है तो एक तालिका पैटर्न बनाएं (Kibana में इसे 'इंडेक्स पैटर्न' कहा जाता है) ताकि आप अपने डेटा स्रोत को परिभाषित कर सकें।
  - एक दृश्यावलि प्रकार चुनें (जैसे, बार चार्ट, रेखा चार्ट, या पाई चार्ट)।
  - अपनी दृश्यावलि कॉन्फ़िगर करें, इसे निष्पादित करें, और अपने डेटा का अन्वेषण करें।
  - भविष्य के उपयोग के लिए अपनी दृश्यावलियों को सहेजें।

### डैशबोर्ड
- इंटरएक्टिव डैशबोर्ड बनाने या देखने के लिए **डैशबोर्ड** पर पहुँचें:
  - एक व्यक्तिगत अनुभव के लिए दृश्यावलियों, फ़िल्टरों या नियंत्रणों को जोड़ें।
  - डैशबोर्ड से सीधे अपने डेटा के साथ इंटरएक्ट करें।
  - भविष्य के उपयोग के लिए डैशबोर्ड को सहेजें।

### प्रबंधन
- डिफ़ॉल्ट समय क्षेत्रों और दृश्यृण प्राथमिकताओं जैसी सेटिंग्स को अनुकूलित करने के लिए **प्रबंधन > Kibana** पर जाएं।

## सीमाएँ
- वर्तमान में, Kibana संस्करण 7.6.0 का परीक्षण किया गया है और अनुशंसित है। अन्य 7.x संस्करण कार्य कर सकते हैं लेकिन समस्याएँ पैदा कर सकते हैं। संस्करण 8.x का समर्थन नहीं किया जाता है।
- निम्नलिखित Elasticsearch-विशिष्ट फ़ील्ड प्रकारों का समर्थन नहीं किया जाता है:
  - [स्थान डेटा प्रकार](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#spatial_datatypes)
  - [संरचित डेटा प्रकार](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#structured-data-types)
  - [दस्तावेज़ रैंकिंग प्रकार](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#document-ranking-types)
  - [पाठ खोज प्रकार](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#text-search-types) (साधारण 'पाठ' को छोड़कर)
  - [संबंधित डेटा प्रकार](https://www.elastic.co/guide/en/elasticsearch/reference/current/mapping-types.html#object-types)
- मैट्रिक संग्रह कार्यक्षमताएँ [Manticore द्वारा समर्थित](../Searching/Grouping.md#Aggregation-functions) तक सीमित हैं।
- निम्नलिखित Kibana उपकरणों का समर्थन नहीं किया जाता है:
  - [Canvas](https://www.elastic.co/guide/en/kibana/7.6/canvas.html) – डेटा को रंगों और चित्रों के साथ संयोजित करने के लिए एक दृश्य और प्रस्तुति उपकरण।
  - [Elastic Maps](https://www.elastic.co/guide/en/kibana/7.6/maps.html) – भौगोलिक डेटा का विश्लेषण करने के लिए एक उपकरण।
  - [Metrics](https://www.elastic.co/guide/en/kibana/7.6/xpack-infra.html) – बुनियादी ढांचे की मैट्रिक की निगरानी के लिए एक ऐप।
  - [Logs](https://www.elastic.co/guide/en/kibana/7.6/xpack-logs.html) – सामान्य सेवाओं से लॉग का अन्वेषण करने के लिए एक कंसोल-जैसी डिस्प्ले।
  - निगरानी:
    - [Uptime](https://www.elastic.co/guide/en/kibana/7.6/xpack-uptime.html) – HTTP/S, TCP, और ICMP के माध्यम से नेटवर्क अंत बिंदुओं की स्थिति की निगरानी करता है।
    - [APM (एप्लिकेशन प्रदर्शन निगरानी)](https://www.elastic.co/guide/en/kibana/7.6/xpack-apm.html) – अनुप्रयोगों से गहन प्रदर्शन मैट्रिक्स एकत्र करता है।
    - [SIEM (सुरक्षा जानकारी और घटना प्रबंधन)](https://www.elastic.co/guide/en/kibana/7.6/xpack-siem.html) – सुरक्षा टीमों के लिए एक इंटरएक्टिव कार्यक्षेत्र जो घटनाओं का विश्लेषण करने और प्रारंभिक जांच करने में मदद करता है।
    - [ILM (इंडेक्स जीवनचक्र प्रबंधन)](https://www.elastic.co/guide/en/elasticsearch/reference/7.6/index-lifecycle-management.html) - प्रदर्शन, स्थिरता, और पुनरावृत्ति आवश्यकताओं के अनुसार स्वचालित रूप से इंडेक्स प्रबंधित करता है।
    - [Stack Monitoring](https://www.elastic.co/guide/en/kibana/7.6/xpack-monitoring.html) – Elastic Stack के भीतर निगरानी डेटा के दृश्यावलियां प्रदान करता है।
  - [Elasticsearch प्रबंधन](https://www.elastic.co/guide/en/kibana/7.6/management.html) – Elastic Stack वस्तुओं को प्रबंधित करने के लिए एक UI, जिसमें ILM (इंडेक्स जीवनचक्र प्रबंधन) आदि शामिल हैं।

## डेटा संग्रह और अन्वेषण
Manticore को [[Logstash](../Integration/Logstash.md)], [Filebeat](../Integration/Filebeat.md), [Fluentbit](https://manticoresearch.com/blog/integration-of-manticore-with-fluentbit/), या [Vector.dev](https://manticoresearch.com/blog/integration-of-manticore-with-vectordev/) जैसे उपकरणों के साथ एकीकृत करें ताकि आप वेब लॉग जैसे स्रोतों से डेटा एकत्र कर सकें। एक बार जब डेटा Manticore में लोड हो जाता है, तो आप इसे Kibana में अन्वेषण और दृश्यता कर सकते हैं।
