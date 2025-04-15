# मुख्य+डेल्टा स्कीमा

<!-- उदाहरण मुख्यडेल्टा -->
कई स्थितियों में, कुल डेटासेट इतनी बड़ी होती है कि इसे लगातार एक नई शुरुआत से नहीं बनाया जा सकता, जबकि नए रिकॉर्ड की संख्या अपेक्षाकृत छोटी रहती है। उदाहरण के लिए, एक फोरम में 1,000,000 संग्रहित पोस्ट हो सकते हैं लेकिन प्रतिदिन केवल 1,000 नई पोस्ट मिलती हैं।

ऐसे मामलों में, "लाइव" (लगभग वास्तविक समय) तालिका अपडेट लागू करना "मुख्य+डेल्टा" योजना का उपयोग करके प्राप्त किया जा सकता है।

यह अवधारणा दो स्रोतों और दो तालिकाओं की स्थापना में शामिल होती है, जिनमें एक "मुख्य" तालिका होती है जिसका डेटा शायद ही कभी बदलता है (यदि कभी) और एक "डेल्टा" तालिका होती है नए दस्तावेजों के लिए। उदाहरण में, 1,000,000 संग्रहित पोस्ट मुख्य तालिका में संग्रहित की जाएंगी, जबकि 1,000 नई दैनिक पोस्ट डेल्टा तालिका में रखी जाएंगी। डेल्टा तालिका को फिर से अक्सर बनाया जा सकता है, जिससे दस्तावेजों को सेकंड या मिनटों में खोजने के लिए उपलब्ध कराया जा सकता है। यह निर्धारित करना कि कौन सी दस्तावेजें किस तालिका से संबंधित हैं और मुख्य तालिका को पुनर्निर्मित करना पूरी तरह से स्वचालित किया जा सकता है। एक दृष्टिकोण यह है कि एक काउंटर तालिका बनाई जाए जो दस्तावेजों को विभाजित करने के लिए उपयोग किए जाने वाले आईडी को ट्रैक करती है और जब भी मुख्य तालिका का पुनर्निर्माण होता है इसे अपडेट करती है।

विभाजन चर के रूप में टाइमस्टैम्प कॉलम का उपयोग करना आईडी का उपयोग करने की तुलना में अधिक प्रभावी है क्योंकि टाइमस्टैम्प न केवल नए दस्तावेजों को ट्रैक कर सकते हैं बल्कि संशोधित दस्तावेजों को भी।

उन डेटासेट्स के लिए जो संशोधित या हटाए गए दस्तावेजों को शामिल कर सकते हैं, डेल्टा तालिका को प्रभावित दस्तावेजों की एक सूची प्रदान करनी चाहिए, यह सुनिश्चित करते हुए कि उन्हें दबा दिया जाए और खोज क्वेरियों से बाहर रखा जाए। यह 'किल लिस्ट्स' नाम की एक विशेषता का उपयोग करके किया जाता है। हटाए जाने वाले दस्तावेज़ों के आईडी को एक सहायक क्वेरी में निर्दिष्ट किया जा सकता है जो [sql_query_killlist](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#Table-kill-list) द्वारा परिभाषित किया गया है। डेल्टा तालिका को उन लक्षित तालिकाओं को इंगित करना चाहिए जिन पर किल लिस्ट लागू की जाएंगी [killlist_target](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#killlist_target) निर्देश का उपयोग करके। किल लिस्ट का प्रभाव लक्षित तालिका पर स्थायी होता है, अर्थात यदि डेल्टा तालिका के बिना खोज की जाती है, तो दबाए गए दस्तावेज खोज परिणामों में नहीं दिखाई देंगे।

ध्यान दें कि हम डेल्टा स्रोत में `sql_query_pre` को ओवरराइड कर रहे हैं। हमें इस ओवरराइड को स्पष्ट रूप से शामिल करना चाहिए। यदि हम ऐसा नहीं करते हैं, तो `REPLACE` क्वेरी डेल्टा स्रोत के निर्माण के दौरान भी निष्पादित होगी, जिससे यह बेकार हो जाएगा।

<!-- अनुरोध उदाहरण -->
```ini
# MySQL में
CREATE TABLE deltabreaker (
  index_name VARCHAR(50) NOT NULL,
  created_at TIMESTAMP NOT NULL  DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (index_name)
);

# manticore.conf में
source main {
  ...
  sql_query_pre = REPLACE INTO deltabreaker SET index_name = 'main', created_at = NOW()
  sql_query =  SELECT id, title, UNIX_TIMESTAMP(updated_at) AS updated FROM documents WHERE deleted=0 AND  updated_at  >=FROM_UNIXTIME($start) AND updated_at  <=FROM_UNIXTIME($end)
  sql_query_range  = SELECT ( SELECT UNIX_TIMESTAMP(MIN(updated_at)) FROM documents) min, ( SELECT UNIX_TIMESTAMP(created_at)-1 FROM deltabreaker WHERE index_name='main') max
  sql_query_post_index = REPLACE INTO deltabreaker set index_name = 'delta', created_at = (SELECT created_at FROM deltabreaker t WHERE index_name='main')
  ...
  sql_attr_timestamp = updated
}

source delta : main {
  sql_query_pre =
  sql_query_range = SELECT ( SELECT UNIX_TIMESTAMP(created_at) FROM deltabreaker WHERE index_name='delta') min, UNIX_TIMESTAMP() max
  sql_query_killlist = SELECT id FROM documents WHERE updated_at >=  (SELECT created_at FROM deltabreaker WHERE index_name='delta')
}

table main {
  path = /var/lib/manticore/main
  source = main
}

table delta {
  path = /var/lib/manticore/delta
  source = delta
  killlist_target = main:kl
}
```

<!-- अंत -->


<!-- प्रूफरीड -->
