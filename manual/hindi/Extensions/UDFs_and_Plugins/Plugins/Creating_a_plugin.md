# सिस्टम प्लगइन्स
## प्लगइन बनाएं

```sql
CREATE PLUGIN plugin_name TYPE 'plugin_type' SONAME 'plugin_library'
```

दिए गए पुस्तकालय को लोड करता है (यदि यह पहले से लोड नहीं है) और इसे से निर्दिष्ट प्लगइन लोड करता है। उपलब्ध प्लगइन प्रकारों में शामिल हैं:

- `ranker`
- `index_token_filter`
- `query_token_filter`

प्लगइन्स लिखने के बारे में अधिक जानकारी के लिए, कृपया [प्लगइन्स](../../../Extensions/UDFs_and_Plugins/UDFs_and_Plugins.md#Plugins) दस्तावेज़ देखें।

```sql
mysql> CREATE PLUGIN myranker TYPE 'ranker' SONAME 'myplugins.so';
Query OK, 0 rows affected (0.00 sec)
```

## बड्डी प्लगइन बनाएं

<!-- example create_buddy_plugin -->

बड्डी प्लगइन्स Manticore सर्च की कार्यक्षमता को बढ़ा सकते हैं और कुछ क्वेरीज़ को सक्षम कर सकते हैं जिन्हें मूल रूप से समर्थित नहीं किया गया है। बड्डी प्लगइन्स बनाने के बारे में अधिक जानने के लिए, हम [इस लेख](https://manticoresearch.com/blog/manticoresearch-buddy-pluggable-design/) को पढ़ने की सिफारिश करते हैं।

बड्डी प्लगइन बनाने के लिए, निम्नलिखित SQL आदेश चलाएँ:

```sql
CREATE PLUGIN <username/package name on https://packagist.org/> TYPE 'buddy' VERSION <package version>
```

> नोट: `CREATE BUDDY PLUGIN` के लिए [Manticore Buddy](../Installation/Manticore_Buddy.md) की आवश्यकता है। यदि यह काम नहीं करता है, तो सुनिश्चित करें कि बड्डी स्थापित है।

आप बड्डी प्लगइन्स के लिए विशेष रूप से बनाए गए उपनाम आदेश का भी उपयोग कर सकते हैं, जिसे याद रखना आसान है:

```sql
CREATE BUDDY PLUGIN <username/package name on https://packagist.org/> VERSION <package version>
```

यह आदेश `show-hostname` प्लगइन को [plugin_dir](../../Server_settings/Common.md#plugin_dir) पर स्थापित करेगा और इसे सर्वर को पुनरारंभ किए बिना सक्षम करेगा।

<!-- intro -->
### उदाहरण

<!-- request Example -->

```sql
CREATE PLUGIN manticoresoftware/buddy-plugin-show-hostname TYPE 'buddy' VERSION 'dev-main';

CREATE BUDDY PLUGIN manticoresoftware/buddy-plugin-show-hostname VERSION 'dev-main';
```

<!-- end -->
<!-- proofread -->
