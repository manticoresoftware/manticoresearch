# प्लगइन हटाएं

```sql
DROP PLUGIN plugin_name TYPE 'plugin_type'
```

निर्दिष्ट प्लगइन को लोड से निकालने के लिए मार्क करता है। लोड से निकालने की प्रक्रिया **तत्काल** नहीं होती है, क्योंकि समवर्ती प्रश्न अभी भी इसका उपयोग कर सकते हैं। फिर भी, `DROP` के बाद, नए प्रश्नों को प्लगइन तक पहुंच नहीं होगी। इसके बाद, जब सभी जारी प्रश्न जो प्लगइन में शामिल हैं, समाप्त हो जाएंगे, प्लगइन को लोड से निकाल दिया जाएगा। यदि निर्दिष्ट पुस्तकालय से सभी प्लगइन्स को लोड से निकाला जाता है, तो पुस्तकालय भी स्वचालित रूप से लोड से निकाल दिया जाएगा।

```sql
mysql> DROP PLUGIN myranker TYPE 'ranker';
Query OK, 0 rows affected (0.00 sec)
```

## बडी प्लगइन हटाएं

<!-- example delete_buddy_plugin -->

```sql
DELETE BUDDY PLUGIN <username/package name on https://packagist.org/>
```

> नोट: `DELETE BUDDY PLUGIN` के लिए [Manticore Buddy](../Installation/Manticore_Buddy.md) की आवश्यकता होती है। यदि यह काम नहीं करता है, तो सुनिश्चित करें कि बडी स्थापित है।

यह क्रिया इंस्टैंटली और स्थायी रूप से स्थापित प्लगइन को [plugin_dir](../../Server_settings/Common.md#plugin_dir) से हटा देती है। एक बार हटा देने के बाद, प्लगइन की सुविधाएँ उपलब्ध नहीं रहेंगी।

<!-- intro -->
### उदाहरण

<!-- request Example -->
```sql
DELETE BUDDY PLUGIN manticoresoftware/buddy-plugin-show-hostname
```

<!-- end -->
<!-- proofread -->
