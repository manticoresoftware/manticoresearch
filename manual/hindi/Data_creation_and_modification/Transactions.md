# लेनदेन

Manticore वास्तविक समय और पर्कोलेट таблиयों में डेटा को हटाने और डालने के लिए बुनियादी लेनदेन का समर्थन करता है, जब तक कि एक वितरित तालिका में लिखने का प्रयास नहीं किया जाता है जिसमें एक वास्तविक समय या पर्कोलेट तालिका शामिल है। तालिका में किया गया प्रत्येक परिवर्तन पहले एक आंतरिक परिवर्तन सेट में सहेजा जाता है और फिर वास्तव में तालिका में प्रतिबद्ध किया जाता है। डिफ़ॉल्ट रूप से, प्रत्येक आदेश एक व्यक्तिगत स्वचालित लेनदेन में लिपटा होता है, जिससे यह पारदर्शी बनता है: आप बस 'डालें' कुछ और देख सकते हैं जो इसे पूरा करने के बाद डाले गए परिणाम को देखकर, लेनदेन के बारे में चिंता किए बिना। हालाँकि, इस व्यवहार को मैन्युअल रूप से लेनदेन शुरू करके और प्रतिबद्ध करके स्पष्ट रूप से प्रबंधित किया जा सकता है।

लेनदेन के लिए निम्नलिखित आदेशों का समर्थन किया जाता है:
* [INSERT](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md)
* [REPLACE](../Data_creation_and_modification/Updating_documents/REPLACE.md)
* [DELETE](../Data_creation_and_modification/Deleting_documents.md)

लेनदेन का समर्थन नहीं किया जाता है:
* UPDATE (जो [REPLACE](../Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md) से [अलग](../Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md) है, क्योंकि यह इन-प्लेस विशेषता अद्यतन करता है)
* ALTER - [तालिका स्कीमा को अपडेट करने के लिए](../Updating_table_schema_and_settings.md)
* TRUNCATE - [एक वास्तविक समय तालिका को खाली करने के लिए](../Emptying_a_table.md)
* ATTACH - [एक साधारण तालिका को एक वास्तविक समय तालिका से जोड़ने के लिए](../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md)
* CREATE - [एक तालिका बनाने के लिए](../Creating_a_table/Local_tables.md)
* DROP - [एक तालिका को हटाने के लिए](../Deleting_a_table.md)

कृपया ध्यान दें कि Manticore में लेनदेन पृथक्करण प्रदान करने का लक्ष्य नहीं है। Manticore में लेनदेन का उद्देश्य आपको कई लेखों को संचित करने और उन्हें प्रतिबद्ध करने पर एक साथ निष्पादित करने या आवश्यकता होने पर उन्हें वापस रोल करना है। लेनदेन [बाइनरी लॉग](../Logging/Binary_logging.md) के साथ स्थिरता और स्थिरता के लिए एकीकृत होते हैं।

## स्वचालित और मैन्युअल मोड

```sql
SET AUTOCOMMIT = {0 | 1}
```

`SET AUTOCOMMIT` सक्रिय सत्र में स्वचालित लेनदेन मोड को नियंत्रित करता है। `AUTOCOMMIT` डिफ़ॉल्ट रूप से 1 पर सेट है। डिफ़ॉल्ट सेटिंग के साथ, आपको लेनदेन के बारे में चिंता करने की आवश्यकता नहीं है, क्योंकि कोई भी कथन जो किसी भी तालिका में बदलाव करता है उसे अपनी अलग लेनदेन में स्वचालित रूप से लपेटा जाता है। इसे 0 पर सेट करने से आप मैन्युअल रूप से लेनदेन का प्रबंधन कर सकते हैं, जिसका अर्थ है कि वे तब तक दिखाई नहीं देंगे जब तक कि आप उन्हें स्पष्ट रूप से प्रतिबद्ध नहीं करते।

लेनदेन केवल एक वास्तविक समय या पर्कोलेट तालिका तक सीमित हैं और आकार में भी सीमित हैं। वे परमाणु, स्थिर, अत्यधिक पृथक और टिकाऊ होते हैं। अत्यधिक पृथक का अर्थ है कि परिवर्तन केवल समवर्ती लेनदेन के लिए ही नहीं बल्कि वर्तमान सत्र के लिए भी अदृश्य होते हैं।

## BEGIN, COMMIT, और ROLLBACK

```sql
START TRANSACTION | BEGIN
COMMIT
ROLLBACK
```

`BEGIN` कथन (या इसके `START TRANSACTION` उपनाम) किसी भी लंबित लेनदेन को मजबूरन प्रतिबद्ध करता है, यदि उपस्थित है, और एक नया शुरू करता है।

`COMMIT` कथन वर्तमान लेनदेन को प्रतिबद्ध करता है, सभी परिवर्तनों को स्थायी बनाता है।

`ROLLBACK` कथन वर्तमान लेनदेन को वापस रोल करता है, सभी परिवर्तनों को रद्द करता है।

## लेनदेन /bulk में

जब आप `/bulk` JSON अंत बिंदुओं में से किसी एक का उपयोग करते हैं ( [कुल вставा](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents), [कुल प्रतिस्थापित](../Data_creation_and_modification/Updating_documents/REPLACE.md?client=JSON#Bulk-replace), [कुल हटाना](../Data_creation_and_modification/Deleting_documents.md?client=JSON#Bulk-deletion) ), आप दस्तावेजों के एक बैच को प्रतिबद्ध करने के लिए उनके बाद एक खाली पंक्ति जोड़कर मजबूर कर सकते हैं।

## उदाहरण

### स्वचालित प्रतिबद्धताएँ (डिफ़ॉल्ट)

```sql
insert into indexrt (id, content, title, channel_id, published) values (1, 'aa', 'blabla', 1, 10);
Query OK, 1 rows affected (0.00 sec)

select * from indexrt where id=1;
+------+------------+-----------+--------+
| id   | channel_id | published | title  |
+------+------------+-----------+--------+
|    1 |          1 |        10 | blabla |
+------+------------+-----------+--------+
1 row in set (0.00 sec)
```

डाला गया मान तुरंत अगले 'select' कथन में दिखाई देता है।

### मैन्युअल प्रतिबद्धताएँ (autocommit=0)

```sql
set autocommit=0;
Query OK, 0 rows affected (0.00 sec)

insert into indexrt (id, content, title, channel_id, published) values (3, 'aa', 'bb', 1, 1);
Query OK, 1 row affected (0.00 sec)

insert into indexrt (id, content, title, channel_id, published) values (4, 'aa', 'bb', 1, 1);
Query OK, 1 row affected (0.00 sec)

select * from indexrt where id=3;
Empty set (0.01 sec)

select * from indexrt where id=4;
Empty set (0.00 sec)
```

इस मामले में, परिवर्तन स्वचालित रूप से प्रतिबद्ध नहीं होते हैं। परिणामस्वरूप, सम्मिलन दृश्यमान नहीं होते हैं, यहां तक कि उसी सत्र में भी, क्योंकि उन्हें प्रतिबद्ध नहीं किया गया है। इसके अलावा, `BEGIN` कथन की अनुपस्थिति के बावजूद, एक लेनदेन स्वचालित रूप से शुरू किया गया है।

परिवर्तनों को दृश्य बनाने के लिए, आपको लेनदेन को प्रतिबद्ध करना होगा:

```sql
commit;
Query OK, 0 rows affected (0.00 sec)

select * from indexrt where id=4;
+------+------------+-----------+-------+
| id   | channel_id | published | title |
+------+------------+-----------+-------+
|    4 |          1 |         1 | bb    |
+------+------------+-----------+-------+
1 row in set (0.00 sec)

select * from indexrt where id=3;
+------+------------+-----------+-------+
| id   | channel_id | published | title |
+------+------------+-----------+-------+
|    3 |          1 |         1 | bb    |
+------+------------+-----------+-------+
1 row in set (0.00 sec)
```

प्रतिबद्धता के बाद, सम्मिलन तालिका में दिखाई देते हैं।

### मैन्युअल लेनदेन
`BEGIN` और `COMMIT` का उपयोग करके, आप एक लेन-देन की सीमाओं को स्पष्ट रूप से परिभाषित कर सकते हैं, इसलिए इस मामले में ऑटो-कमिट के बारे में चिंता करने की आवश्यकता नहीं है।

```sql
begin;
Query OK, 0 rows affected (0.00 sec)

insert into indexrt (id, content, title, channel_id, published) values (2, 'aa', 'bb', 1, 1);
Query OK, 1 row affected (0.00 sec)

select * from indexrt where id=2;
Empty set (0.01 sec)

commit;
Query OK, 0 rows affected (0.01 sec)

select * from indexrt where id=2;
+------+------------+-----------+-------+
| id   | channel_id | published | title |
+------+------------+-----------+-------+
|    2 |          1 |         1 | bb    |
+------+------------+-----------+-------+
1 row in set (0.01 sec)
```


<!-- proofread -->

