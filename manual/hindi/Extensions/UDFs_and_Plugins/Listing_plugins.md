# लिस्टिंग प्लगइन्स

## दिखाएँ प्लगइन्स
<!-- example Example -->

```sql
SHOW PLUGINS
```

सभी लोड किए गए प्लगइन्स (बडी प्लगइन्स को छोड़कर, नीचे देखें) और UDFs को प्रदर्शित करता है। "Type" कॉलम में `udf`, `ranker`, `index_token_filter`, या `query_token_filter` में से एक होना चाहिए। "Users" कॉलम उस प्लगइन का उपयोग कर रहे थ्रेड की संख्या है जो वर्तमान में एक क्वेरी में है। "Extra" कॉलम विभिन्न अतिरिक्त प्लगइन-प्रकार विशेष जानकारी के लिए है; वर्तमान में, यह UDFs का रिटर्न प्रकार दिखाता है और अन्य सभी प्लगइन प्रकारों के लिए यह खाली है।


<!-- intro -->
##### उदाहरण:

<!-- request Example -->

```sql
SHOW PLUGINS;
```

<!-- response -->

```sql
+------+----------+----------------+-------+-------+
| Type | Name     | Library        | Users | Extra |
+------+----------+----------------+-------+-------+
| udf  | sequence | udfexample.dll | 0     | INT   |
+------+----------+----------------+-------+-------+
1 row in set (0.00 sec)
```

<!-- end -->

## SHOW BUDDY PLUGINS

<!-- example Example_buddy -->

```sql
SHOW BUDDY PLUGINS
```

> नोट: `SHOW BUDDY PLUGINS` को [Manticore Buddy](../Installation/Manticore_Buddy.md) की आवश्यकता होती है। यदि यह काम नहीं करता है, तो सुनिश्चित करें कि बडी स्थापित है।

यह सभी उपलब्ध प्लगइन्स को प्रदर्शित करेगा, जिसमें कोर और स्थानीय शामिल हैं।
किसी प्लगइन को हटाने के लिए, सुनिश्चित करें कि पैकेज कॉलम में सूचीबद्ध नाम का उपयोग करें।

<!-- request Example -->

```sql
SHOW BUDDY PLUGINS;
```

<!-- response -->

```
+------------------------------------------------+-----------------+---------+------+----------------------------------------------------------------------------------------------------------------------------------------------------------+
| Package                                        | Plugin          | Version | Type | Info                                                                                                                                                     |
+------------------------------------------------+-----------------+---------+------+----------------------------------------------------------------------------------------------------------------------------------------------------------+
| manticoresoftware/buddy-plugin-empty-string    | empty-string    | 2.1.5   | core | खाली क्वेरी को संभालता है, जो टिप्पणियों को ट्रिम करते समय या उन विशेष SQL प्रोटोकॉल निर्देशों से निपटते समय हो सकता है जो टिप्पणियों में समर्थित नहीं हैं      |
| manticoresoftware/buddy-plugin-backup          | backup          | 2.1.5   | core | BACKUP SQL कथन                                                                                                                                     |
| manticoresoftware/buddy-plugin-emulate-elastic | emulate-elastic | 2.1.5   | core | कुछ इलेस्टिक क्वेरियों का सिमुलेट करता है और प्रतिक्रियाएं उत्पन्न करता है जैसे कि वे ES द्वारा बनाई गई थीं                                                                         |
| manticoresoftware/buddy-plugin-insert          | insert          | 2.1.5   | core | ऑटो स्कीमा समर्थन। जब एक इनसर्ट ऑपरेशन किया जाता है और तालिका मौजूद नहीं होती है, तो यह डेटा प्रकारों के ऑटो-डिटेक्शन के साथ इसे बनाता है                    |
| manticoresoftware/buddy-plugin-alias           | alias           | 2.1.5   | core |                                                                                                                                                          |
| manticoresoftware/buddy-plugin-select          | select          | 2.1.5   | core | mysqldump और अन्य सॉफ़्टवेयर समर्थन के लिए आवश्यक विभिन्न SELECTs हैंडलर, जो मुख्य रूप से MySQL के समान कार्य करने का प्रयास करते हैं                                       |
| manticoresoftware/buddy-plugin-show            | show            | 2.1.5   | core | विभिन्न "शो" क्वेरी हैंडलर, उदाहरण के लिए, `show queries`, `show fields`, `show full tables`, आदि                                                     |
| manticoresoftware/buddy-plugin-cli-table       | cli-table       | 2.1.5   | core | /cli_json पर आधारित /cli एपीआई - क्वेरी परिणाम को तालिका के रूप में आउटपुट करता है                                                                                       |
| manticoresoftware/buddy-plugin-plugin          | plugin          | 2.1.5   | core | प्लगइन समर्थन और सहायक के लिए कोर लॉजिक। इसके अलावा `create buddy plugin`, `delete buddy plugin`, और `show buddy plugins` को संभालता है                           |
| manticoresoftware/buddy-plugin-test            | test            | 2.1.5   | core | परीक्षण प्लगइन, विशेष रूप से परीक्षणों के लिए उपयोग किया जाता है                                                                                                                  |
| manticoresoftware/buddy-plugin-insert-mva      | insert-mva      | 2.1.5   | core | mysqldump के साथ MVA क्षेत्रों की बहाली का प्रबंधन करता है                                                                                                     |
| manticoresoftware/buddy-plugin-modify-table    | modify-table    | 2.1.5   | core | तालिका बनाने और बदलने के कथनों में विकल्पों को मानकीकरण करने में मदद करता है, जिसमें integers के लिए option=1 दिखाना शामिल है। इसके अलावा, शार्दित तालिकाओं को बनाने के लिए लॉजिक को प्रबंधित करता है। |
| manticoresoftware/buddy-plugin-knn             | knn             | 2.1.5   | core | दस्तावेज़ आईडी द्वारा KNN सक्षम करता है                                                                                                                               |
| manticoresoftware/buddy-plugin-replace         | replace         | 2.1.5   | core | आंशिक प्रतिस्थापन सक्षम करता है                                                                                                                                 |
+------------------------------------------------+-----------------+---------+------+----------------------------------------------------------------------------------------------------------------------------------------------------------+-----+
```

<!-- end -->
<!-- proofread -->
