# तालिका आयात करना

यदि आप [Plain मोड](../../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29) से [RT मोड](../../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29) में माइग्रेट करने का निर्णय लेते हैं या कुछ अन्य मामलों में, Plain मोड में बनाई गई रियल-टाइम और पर्कोलेट तालिकाओं को RT मोड में चल रहे Manticore में `IMPORT TABLE` स्टेटमेंट का उपयोग करके आयात किया जा सकता है। सामान्य सिंटैक्स इस प्रकार है:

<!-- उदाहरण आयात -->

```sql
IMPORT TABLE table_name FROM 'path'
```

जहाँ 'path' पैरामीटर को इस प्रकार सेट किया जाना चाहिए: `/your_backup_folder/your_backup_name/data/your_table_name/your_table_name`

<!-- अनुरोध -->
```bash
mysql -P9306 -h0 -e 'create table t(f text)'

mysql -P9306 -h0 -e "backup table t to /tmp/"

mysql -P9306 -h0 -e "drop table t"

BACKUP_NAME=$(ls /tmp | grep 'backup-' | tail -n 1)

mysql -P9306 -h0 -e "import table t from '/tmp/$BACKUP_NAME/data/t/t'

mysql -P9306 -h0 -e "show tables"
```
<!-- अंत -->

इस कमांड को निष्पादित करने से निर्दिष्ट तालिका की सभी तालिका फ़ाइलें [data_dir](../../../Server_settings/Searchd.md#data_dir) में कॉपी हो जाती हैं। सभी बाहरी तालिका फ़ाइलें जैसे शब्दफॉर्म, अपवाद और स्टॉपवर्ड भी उसी `data_dir` में कॉपी की जाती हैं।
`IMPORT TABLE` के निम्नलिखित प्रतिबंध हैं:
* कॉन्फ़िग फ़ाइल में मूल रूप से निर्दिष्ट बाहरी फ़ाइलों के लिए पथ को पूर्ण होना चाहिए
* केवल रियल-टाइम और पर्कोलेट तालिकाएँ समर्थित हैं
* प्लेन तालिकाओं को पहले (प्लेन मोड में) रियल-टाइम तालिकाओं में [ATTACH TABLE](../../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md) के माध्यम से परिवर्तित करने की आवश्यकता है।

ध्यान दें, `IMPORT TABLE` कमांड का समर्थन उन तालिकाओं के लिए नहीं है जो 5.0.0 से पहले के संस्करणों में बनाई गई थीं।

## indexer --print-rt

<!-- उदाहरण print_rt -->
यदि ऊपर दिया गया तरीका एक प्लेन तालिका को RT तालिका में माइग्रेट करने के लिए संभव नहीं है, तो आप `indexer --print-rt` का उपयोग करके एक प्लेन तालिका से सीधे डेटा डंप कर सकते हैं बिना इसे RT प्रकार तालिका में परिवर्तित किए और फिर कमांड लाइन से सीधे एक RT तालिका में डंप आयात कर सकते हैं।

हालांकि, इस विधि के कुछ प्रतिबंध हैं:
* केवल SQL-आधारित स्रोतों का समर्थन किया गया है
* एमवीए का समर्थन नहीं किया गया है

<!-- अनुरोध -->
```bash
/usr/bin/indexer --rotate --config /etc/manticoresearch/manticore.conf --print-rt my_rt_index my_plain_index > /tmp/dump_regular.sql

mysql -P $9306 -h0 -e "truncate table my_rt_index"

mysql -P 9306 -h0 < /tmp/dump_regular.sql

rm /tmp/dump_regular.sql
```
<!-- अंत -->
<!-- प्रूफरीड -->
