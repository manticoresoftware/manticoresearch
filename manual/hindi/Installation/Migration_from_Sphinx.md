# Sphinx सर्च से माइग्रेशन

## Sphinx 2.x -> Manticore 2.x
Manticore Search 2.x Sphinxsearch 2.x के साथ संगतता बनाए रखता है और Sphinxsearch द्वारा बनाए गए मौजूदा तालिकाओं को लोड कर सकता है। अधिकांश मामलों में, अपग्रेड करना सिर्फ बाइनरी को बदलने का मामला है।

sphinx.conf (Linux में सामान्यतः `/etc/sphinxsearch/sphinx.conf` पर स्थित) के बजाय, Manticore डिफ़ॉल्ट रूप से `/etc/manticoresearch/manticore.conf` का उपयोग करता है। यह एक अलग उपयोगकर्ता के तहत भी चलता है और अलग-अलग फ़ोल्डर का उपयोग करता है।

Systemd सेवा का नाम `sphinx/sphinxsearch` से `manticore` में बदल गया है और यह सेवा उपयोगकर्ता `manticore` के तहत चलती है (Sphinx `sphinx` या `sphinxsearch` का उपयोग कर रहा था)। यह PID फ़ाइल के लिए भी एक अलग फ़ोल्डर का उपयोग करता है।

डिफ़ॉल्ट रूप से उपयोग किए जाने वाले Folder हैं `/var/lib/manticore`, `/var/log/manticore`, `/var/run/manticore`। आप अभी भी मौजूदा Sphinx कॉन्फ़िगरेशन का उपयोग कर सकते हैं, लेकिन आपको `/var/lib/sphinxsearch` और `/var/log/sphinxsearch` फ़ोल्डरों के लिए मैन्युअल रूप से अनुमतियाँ बदलनी होंगी। या, बस सिस्टम फ़ाइलों में 'sphinx' को 'manticore' में वैश्विक रूप से नाम बदलें। यदि आप अन्य फ़ोल्डरों का उपयोग करते हैं (डेटा, वर्डफॉर्म फ़ाइलें आदि के लिए), तो स्वामित्व को भी उपयोगकर्ता `manticore` पर स्विच करना होगा। `pid_file` स्थान को `manticore.service` के साथ मेल खाने के लिए `/var/run/manticore/searchd.pid` में बदलना चाहिए।

यदि आप Manticore फ़ोल्डर का उपयोग करना चाहते हैं, तो तालिका फ़ाइलों को नए डेटा फ़ोल्डर (`/var/lib/manticore`) में स्थानांतरित करना होगा और अनुमतियों को उपयोगकर्ता `manticore` पर बदलना होगा।

## Sphinx 2.x / Manticore 2.x -> Manticore 3.x
Sphinx / Manticore 2.x से 3.x में अपग्रेड करना सीधा नहीं है, क्योंकि तालिका भंडारण इंजन में बहुत बड़ा अपग्रेड हुआ है और नया searchd पुराने तालिकाओं को लोड नहीं कर सकता है और उन्हें नए प्रारूप में ऑन-द-फ्लाई अपग्रेड नहीं कर सकता है।

Manticore Search 3 ने एक नया डिज़ाइन किया गया तालिका भंडारण प्राप्त किया है। Manticore/Sphinx 2.x के साथ बनाई गई तालिकाएँ Manticore Search 3 द्वारा बिना [conversion](../Installation/Migration_from_Sphinx.md#index_converter) के लोड नहीं की जा सकती हैं। 4GB की सीमा के कारण, 2.x में एक रीयल-टाइम तालिका के पास ऑप्टिमाइज़ ऑपरेशन के बाद अभी भी कई डिस्क.chunk हो सकते हैं। 3.x में अपग्रेड करने के बाद, इन तालिकाओं को अब सामान्य [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) कमांड के साथ 1-डिस्क चंक्स के लिए ऑप्टिमाइज़ किया जा सकता है। अनुक्रमणिका फ़ाइलें भी बदल गई हैं। एकमात्र घटक जो कोई संरचनात्मक परिवर्तन नहीं हुआ है वह है `.spp` फ़ाइल (हिट लिस्ट)। `.sps` (स्ट्रिंग/json) और `.spm` (MVA) अब `.spb` (var-length attributes) द्वारा रखी जाती हैं। नए प्रारूप में एक `.spm` फ़ाइल है, लेकिन इसका उपयोग पंक्ति मानचित्र के लिए किया जाता है (पहले यह MVA विशेषताओं के लिए समर्पित था)। नए एक्सटेंशन जोड़े गए हैं `.spt` (docid लुकअप), `.sphi` (द्वितीयक अनुक्रमणिका हिस्टोग्राम), `.spds` (डॉक्यूमेंट स्टोरेज)। यदि आप स्क्रिप्ट का उपयोग कर रहे हैं जो तालिका फ़ाइलों में हेरफेर करती हैं, तो उन्हें नए फ़ाइल एक्सटेंशन के लिए अनुकूलित करना चाहिए।

अपग्रेड प्रक्रिया आपके सेटअप के आधार पर भिन्न हो सकती है (क्लस्टर में सर्वरों की संख्या, क्या आपके पास उच्च उपलब्धता है या नहीं, आदि), लेकिन सामान्यतः, इसमें नए 3.x तालिका संस्करण बनाना और आपके मौजूदा संस्करणों को बदलना शामिल है, साथ ही पुराने 2.x बाइनरीज़ को नए के साथ बदलना भी शामिल है।

आपको दो विशेष आवश्यकताओं का ध्यान रखना चाहिए:

* रीयल-टाइम तालिकाओं को [FLUSH RAMCHUNK](../Securing_and_compacting_a_table/Flushing_RAM_chunk_to_a_new_disk_chunk.md#FLUSH-RAMCHUNK) का उपयोग करके फ्लश करना होगा।
* किल-लिस्ट वालीPlain तालिकाओं में तालिका कॉन्फ़िगरेशन में एक नया निर्देश जोड़ने की आवश्यकता होती है (देखें [killlist_target](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target))

Manticore Search 3 में एक नया उपकरण शामिल है - [index_converter](../Installation/Migration_from_Sphinx.md#index_converter) - जो Sphinx 2.x / Manticore 2.x तालिकाओं को 3.x प्रारूप में परिवर्तित कर सकता है। `index_converter` एक अलग पैकेज में आता है जिसे पहले स्थापित किया जाना चाहिए। कन्वर्ट टूल का उपयोग करके अपने तालिकाओं के 3.x संस्करण बनाएं। `index_converter` मौजूदा डेटा फ़ोल्डर में नए फ़ाइलों को लिख सकता है और पुराने फ़ाइलों का बैकअप ले सकता है या यह नए फ़ाइलों को चुने हुए फ़ोल्डर में लिख सकता है।

## बुनियादी अपग्रेड निर्देश

यदि आपके पास एकल सर्वर है:

* manticore-converter पैकेज स्थापित करें।
* मौजूदा डेटा फ़ोल्डर के बजाय किसी अलग फ़ोल्डर में तालिकाओं के नए संस्करण बनाने के लिए index_converter का उपयोग करें (जैसे `--output-dir` विकल्प का उपयोग करके)
* मौजूदा Manticore/Sphinx को रोकें, 3.0 में अपग्रेड करें, नए तालिकाओं को डेटा फ़ोल्डर में ले जाएं और Manticore प्रारंभ करें।

डाउनटाइम को न्यूनतम करने के लिए, आप 2.x तालिकाओं, कॉन्फ़िगरेशन (यहाँ तालिकाओं, लॉग और विभिन्न पोर्ट के लिए पथ संपादित करने की आवश्यकता होगी) और बाइनरीज़ को एक अलग स्थान पर कॉपी कर सकते हैं और इसे एक अलग पोर्ट पर शुरू कर सकते हैं। अपने एप्लिकेशन को इसके लिए इंगित करें। 3.0 में अपग्रेड करने और नए सर्वर को चालू करने के बाद, आप एप्लिकेशन को सामान्य पोर्ट पर वापस इंगित कर सकते हैं। यदि सब कुछ ठीक है, तो 2.x कॉपी को रोकें और फ़ाइलों को मुक्त करने के लिए हटा दें।

यदि आपके पास एक फालतू बॉक्स है (जैसे परीक्षण या स्टेजिंग सर्वर), तो आप पहले वहाँ तालिका अपग्रेड कर सकते हैं और यहां तक कि कई परीक्षण करने के लिए Manticore 3 स्थापित कर सकते हैं। यदि सब कुछ ठीक है, तो नए तालिका फ़ाइलों को प्रोडक्शन सर्वर पर कॉपी करें। अगर आपके पास कई सर्वर हैं जिन्हें उत्पादन से बाहर निकाला जा सकता है, तो इसे एक-एक करके करें और प्रत्येक पर अपग्रेड करें। वितरण सेटअप के लिए, 2.x searchd एक मास्टर के रूप में काम कर सकता है जिसमें 3.x नोड हैं, इसलिए आप पहले डेटा नोड पर अपग्रेड कर सकते हैं, और फिर मास्टर नोड पर।

क्लाइंट्स को इंजन से जुड़ने के तरीके पर या क्वेरी मोड या क्वेरीज़ के व्यवहार में कोई बदलाव नहीं किया गया है।

## Sphinx / Manticore 2.x बनाम Manticore 3.x में किल-लिस्ट
[Kill-lists](../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md) को Manticore Search 3 में फिर से डिज़ाइन किया गया है। पिछले संस्करणों में, किल-लिस्ट को प्रत्येक पूर्व में खोजी गई तालिका द्वारा प्रदान किए गए परिणाम को लागू किया गया था।

इसलिए, 2.x में, क्वेरी समय पर तालिका का आदेश महत्वपूर्ण था। उदाहरण के लिए, यदि एक डेल्टा तालिका में किल-लिस्ट थी, तो इसे मुख्य तालिका के खिलाफ लागू करने के लिए, क्रम मुख्य, डेल्टा (या तो विभाजित तालिका में या FROM क्लॉज़ में) होना आवश्यक था।
In Manticore 3, kill-lists को एक तालिका पर लागू किया जाता है जब इसे searchd शुरूआत के दौरान या घुमाते समय लोड किया जाता है। तालिका कॉन्फ़िगरेशन में नया निर्देश `killlist_target` लक्षित तालिकाओं को निर्दिष्ट करता है और परिभाषित करता है कि स्रोत तालिका से कौन से doc ids दमन के लिए उपयोग किए जाने चाहिए। ये परिभाषित kill-list में ids, तालिका के वास्तविक doc ids या दोनों हो सकते हैं।

kill-lists से दस्तावेज़ लक्षित तालिकाओं से हटा दिए जाते हैं, वे परिणामों में वापस नहीं आते हैं भले ही खोज में उस तालिका को शामिल नहीं किया गया हो जिसने kill-lists प्रदान की हैं। इसके कारण, खोजने के लिए तालिकाओं का क्रम अब कोई फर्क नहीं पड़ता। अब, `delta, main` और `main, delta` समान परिणाम प्रदान करेगा।

पिछले संस्करणों में, तालिकाएँ कॉन्फ़िगरेशन फ़ाइल से आदेश का पालन करते हुए घुमाई जाती थीं। Manticore 3 में तालिका घुमाने का क्रम बहुत स्मार्ट है और killlist लक्ष्यों के अनुसार काम करता है। तालिकाओं को घुमाने शुरू करने से पहले, सर्वर `killlist_target` परिभाषाओं द्वारा तालिकाओं के चेन की तलाश करता है। फिर यह पहले उन तालिकाओं को घुमाएगा जो कहीं भी kill-lists लक्ष्यों के रूप में संदर्भित नहीं हैं। उसके बाद, यह पहले से घुमाई गई तालिकाओं द्वारा लक्षित तालिकाओं को घुमाएगा और इसी तरह। उदाहरण के लिए, यदि हम  `indexer --all` करते हैं और हमारे पास 3 तालिकाएँ हैं: main, delta_big (जो main पर लक्षित है) और delta_small (जो delta_big पर लक्षित है), पहले, delta_small घुमाई जाती है, फिर delta_big और अंततः main। यह यह सुनिश्चित करने के लिए है कि जब एक निर्भर तालिका घुमाई जाती है तो इसे अन्य तालिकाओं से सबसे हाल के kill-list मिलते हैं।

## Manticore 3.x में हटा दिए गए कॉन्फ़िगरेशन कुंजी
* `docinfo` - अब सब कुछ extern है
* `inplace_docinfo_gap` - अब और आवश्यक नहीं है
* `mva_updates_pool` - MVAs के लिए अब अपडेट्स के लिए कोई समर्पित पूल नहीं है, क्योंकि अब उन्हें सीधे blob में अपडेट किया जा सकता है (नीचे देखें)।

## Manticore 3.x में var-length गुणों को अपडेट करना
String, JSON और MVA गुणों को Manticore 3.x में `UPDATE` कथन का उपयोग करके अपडेट किया जा सकता है।

2.x में string गुणों के लिए `REPLACE` की आवश्यकता होती थी, JSON के लिए केवल स्केलर गुणों को अपडेट करना संभव था (क्योंकि वे निश्चित-चौड़ाई के थे) और MVAs को MVA पूल का उपयोग करके अपडेट किया जा सकता था। अब अपडेट सीधे blob घटक पर किए जाते हैं। एक सेटिंग जो समायोजित करने की आवश्यकता हो सकती है वह है [attr_update_reserve](../Data_creation_and_modification/Updating_documents/UPDATE.md#attr_update_reserve) जो blob के अंत में आवंटित अतिरिक्त स्थान को बदलने की अनुमति देती है जिसका उपयोग नए मानों के बड़े होने की स्थिति में बार-बार आकार के परिवर्तन से बचने के लिए किया जाता है।

## Manticore 3.x में दस्तावेज़ IDs
Doc ids पहले UNSIGNED 64-बिट पूर्णांक होते थे। अब वे POSITIVE SIGNED 64-बिट पूर्णांक हैं।

## Manticore 3.x में RT मोड
[RT मोड](../Read_this_first.md#Real-time-mode-vs-plain-mode) के बारे में यहां पढ़ें।

## Manticore 3.x से विशेष उपसर्ग
Manticore 3.x विशेष उपसर्गों को पहचानता है और उन्हें पार्स करता है जो विशेष अर्थ वाले संख्यात्मक मानों का उपयोग करना आसान बनाता है। उनके लिए सामान्य रूप Integer संख्या + अदितीय होता है, जैसे 10k या 100d, लेकिन 40.3s नहीं (क्योंकि 40.3 पूर्णांक नहीं है), या नहीं 2d 4h (क्योंकि वहाँ दो है, एक नहीं)। अदितीय मामले-संवेदनशील नहीं होते हैं, इसलिए 10W और 10w एक समान होते हैं। वर्तमान में 2 प्रकार के ऐसे उपसर्ग समर्थित हैं:

* आकार उपसर्ग - सीधे उस चीज के आकार को परिभाषित करने वाले मापदंडों में उपयोग किया जा सकता है (मेमोरी बफर, डिस्क फ़ाइल, RAM की सीमा, आदि) बाइट्स में। उन स्थानों पर "नग्न" संख्याएँ सही अर्थ में बाइट्स में आकार को दर्शाती हैं (ऑक्टेट्स)। आकार के मान उपसर्ग `k` के लिए किलोबाइट्स (1k=1024), `m` के लिए मेगाबाइट्स (1m=1024k), `g` के लिए गीगाबाइट्स (1g=1024m) और `t` के लिए टेराबाइट्स (1t=1024g) लेते हैं।
* समय उपसर्ग - उन मापदंडों में उपयोग किया जा सकता है जो किसी समय अंतराल के मानों को परिभाषित करते हैं जैसे देरी, टाइमआउट, आदि। उन मापदंडों के लिए "नग्न" मानों का आमतौर पर प्रलेखित स्केल होता है, और आपको जानना होगा कि क्या उनके संख्याएँ, कहें, 100, का मतलब '100 सेकंड' या '100 मिलीसेकंड' है। हालाँकि अनुमान लगाने के बजाय आप बस उपसर्गित मान लिख सकते हैं और इसे इसके उपसर्ग द्वारा पूरी तरह से निर्धारित किया जाएगा। समय के मान उपसर्ग `us` के लिए माइक्रोसेकंड (microseconds), `ms` के लिए मिलीसेकंड, `s` के लिए सेकंड, `m` के लिए मिनट, `h` के लिए घंटे, `d` के लिए दिन और `w` के लिए सप्ताह लेते हैं।

## index_converter

`index_converter` एक उपकरण है जो Sphinx/Manticore Search 2.x के साथ बनाई गई तालिकाओं को Manticore Search 3.x तालिका प्रारूप में बदलने के लिए है। इस उपकरण का उपयोग कई अलग-अलग तरीकों से किया जा सकता है:

#### एक समय में एक तालिका को बदलें

```ini
$ index_converter --config /home/myuser/manticore.conf --index tablename
```

#### सभी तालिकाओं को बदलें

```ini
$ index_converter --config /home/myuser/manticore.conf --all
```

#### एक फ़ोल्डर में मिली तालिकाओं को बदलें

```ini
$ index_converter  --path /var/lib/manticoresearch/data --all
```

तालिका का नया संस्करण डिफ़ॉल्ट रूप से उसी फ़ोल्डर में लिखा जाता है। पिछले संस्करण की फाइलें उनके नाम में `.old` एक्सटेंशन के साथ सहेजी जाती हैं। एक अपवाद `.spp` (hitlists) फ़ाइल है, जो तालिका घटक का एकमात्र हिस्सा है जिसमें नए प्रारूप में कोई बदलाव नहीं है।

आप `-–output-dir` विकल्प का उपयोग करके नई तालिका संस्करण को एक अलग फ़ोल्डर में सहेज सकते हैं

```ini
$ index_converter --config /home/myuser/manticore.conf --all --output-dir /new/path
```

#### kill lists को बदलें

तालिकाओं के लिए एक विशेष मामला है जिसमें kill-lists शामिल हैं। जैसा कि kill-lists के काम करने के तरीके में बदलाव आया है (देखें [killlist_target](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target)), delta तालिका को यह जानना चाहिए कि लक्ष्य तालिकाएँ कौन सी हैं जिन्हें kill-lists लगाने के लिए। kill-lists लागू करने के लिए लक्षित तालिकाएँ सेट करने के लिए एक रूपांतरित तालिका तैयार करने के 3 तरीके हैं:

* तालिका को बदलते समय `-–killlist-target` का उपयोग करें
  ```ini
  $ index_converter --config /home/myuser/manticore.conf --index deltaindex --killlist-target mainindex:kl
  ```
* रूपांतरण से पहले कॉन्फ़िगरेशन में killlist_target जोड़ें
* रूपांतरण के बाद [ALTER ... KILLIST_TARGET](../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#killlist_target) आदेश का उपयोग करें

#### index_converter विकल्पों की पूर्ण सूची
Here's the complete list of `index_converter` options:

* `--config <file>` (`-c <file>` for short) tells index_converter to use the given file as its configuration. Normally, it will look for manticore.conf in the installation directory (e.g. `/usr/local/manticore/etc/manticore.conf` if installed into `/usr/local/sphinx`), followed by the current directory you are in when calling index_converter from the shell.
* `--index` specifies which table should be converted
* `--path` - instead of using a config file, a path containing table(s) can be used
* `--strip-path` - strips path from filenames referenced by table: stopwords, exceptions and wordforms
* `--large-docid` - allows to convert documents with ids larger than 2^63 and display a warning, otherwise it will just exit on the large id with an error. This option was added as in Manticore 3.x doc ids are signed bigint, while previously they were unsigned
* `--output-dir <dir>` - writes the new files in a chosen folder rather than the same location as with the existing table files. When this option set, existing table files will remain untouched at their location.
* `--all` - converts all tables from the config
* `--killlist-target <targets>` sets the target tables for which kill-lists will be applied. This option should be used only in conjunction with the `--index` option

<!-- proofread -->


