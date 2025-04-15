# SHOW META

```sql
SHOW META [ LIKE pattern ]
```

<!-- example show meta -->
`SHOW META` एक SQL बयान है जो प्रोसेस की गई क्वेरी के बारे में अतिरिक्त मेटा-जानकारी दिखाता है, जिसमें क्वेरी समय, कीवर्ड सांख्यिकी, और उपयोग किए गए द्वितीयक सूचकांकों की जानकारी शामिल है। इसका सिंटैक्स है:

शामिल आइटम हैं:
* `total`: साथ ही प्राप्य संख्या जो वास्तव में पुनः प्राप्त की गई और क्लाइंट को भेजी गई। इस मान को आमतौर पर [LIMIT/size](Searching/Pagination.md#Pagination-of-search-results) खोज विकल्प द्वारा सीमित किया जाता है।
* `total_found`:
  - इंडेक्स में क्वेरी के लिए अनुमानित कुल मिलान संख्या। यदि आपको मिलान की सटीक संख्या चाहिए, तो इस मान पर निर्भर रहने के बजाय `SELECT COUNT(*)` का उपयोग करें।
  - `GROUP BY` के साथ क्वेरियों के लिए, `total_found` समूहों की संख्या को व्यक्तिगत मिलानों के बजाय दर्शाता है।
  - [GROUP N BY](../Searching/Grouping.md#Give-me-N-rows) क्वेरियों के लिए, `total_found` अभी भी समूहों की संख्या को दर्शाता है, भले ही `N` का मान क्या हो।
* `total_relation`: यह बताता है कि `total_found` मान सटीक है या एक अनुमान।
  - यदि Manticore सटीक `total_found` मान निर्धारित नहीं कर सकता है, तो यह फ़ील्ड `total_relation: gte` दिखाएगी, जिसका अर्थ है कि वास्तविक मिलानों की संख्या रिपोर्ट की गई `total_found` के बराबर या उससे अधिक है।
  - यदि `total_found` मान सटीक है, तो `total_relation: eq` दिखाया जाएगा।
* `time`: खोज क्वेरी को प्रोसेस करने में लगा समय (सेकंड में)।
* `keyword[N]`: खोज क्वेरी में उपयोग किया गया nवां कीवर्ड। ध्यान दें कि कीवर्ड को वाइल्डकार्ड के रूप में प्रस्तुत किया जा सकता है, जैसे `abc*`।
* `docs[N]`: खोज क्वेरी से nवें कीवर्ड को含 करने वाले दस्तावेजों (या रिकॉर्ड) की कुल संख्या। यदि कीवर्ड को वाइल्डकार्ड के रूप में प्रस्तुत किया गया है, तो यह मान सभी विस्तारित उप-कीवर्ड के लिए दस्तावेजों का योग दर्शाता है, जो कि मेल खा रहे दस्तावेजों की वास्तविक संख्या से अधिक हो सकता है।
* `hits[N]`: सभी दस्तावेजों में nवें कीवर्ड की कुल आवृत्तियाँ (या हिट) की संख्या।
* `index`: उपयोग किए गए इंडेक्स के बारे में जानकारी (जैसे, द्वितीयक इंडेक्स)।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SELECT id, story_author FROM hn_small WHERE MATCH('one|two|three') and comment_ranking > 2 limit 5;
show meta;
```

<!-- response SQL -->

```sql
+---------+--------------+
| id      | story_author |
+---------+--------------+
|  151171 | anewkid      |
|  302758 | bks          |
|  805806 | drRoflol     |
| 1099245 | tnorthcutt   |
|  303252 | whiten       |
+---------+--------------+
5 rows in set (0.00 sec)

+----------------+---------------------------------------+
| Variable_name  | Value                                 |
+----------------+---------------------------------------+
| total          | 5                                     |
| total_found    | 2308                                  |
| total_relation | eq                                    |
| time           | 0.001                                 |
| keyword[0]     | one                                   |
| docs[0]        | 224387                                |
| hits[0]        | 310327                                |
| keyword[1]     | three                                 |
| docs[1]        | 18181                                 |
| hits[1]        | 21102                                 |
| keyword[2]     | two                                   |
| docs[2]        | 63251                                 |
| hits[2]        | 75961                                 |
| index          | comment_ranking:SecondaryIndex (100%) |
+----------------+---------------------------------------+
14 rows in set (0.00 sec)
```

<!-- end -->

<!-- example show meta iostats cpustats -->
`SHOW META` I/O और CPU काउंटर दिखा सकता है, लेकिन वे केवल तब उपलब्ध होंगे जब searchd को `--iostats` और `--cpustats` स्विच के साथ शुरू किया गया हो।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SELECT id,channel_id FROM records WHERE MATCH('one|two|three') limit 5;

SHOW META;
```

<!-- response SQL -->

```sql
+--------+--------------+
| id     | story_author |
+--------+--------------+
| 300263 | throwaway37  |
| 713503 | mahmud       |
| 716804 | mahmud       |
| 776906 | jimbokun     |
| 753332 | foxhop       |
+--------+--------------+
5 rows in set (0.01 sec)

+-----------------------+--------+
| Variable_name         | Value  |
+-----------------------+--------+
| total                 | 5      |
| total_found           | 266385 |
| total_relation        | eq     |
| time                  | 0.011  |
| cpu_time              | 18.004 |
| agents_cpu_time       | 0.000  |
| io_read_time          | 0.000  |
| io_read_ops           | 0      |
| io_read_kbytes        | 0.0    |
| io_write_time         | 0.000  |
| io_write_ops          | 0      |
| io_write_kbytes       | 0.0    |
| agent_io_read_time    | 0.000  |
| agent_io_read_ops     | 0      |
| agent_io_read_kbytes  | 0.0    |
| agent_io_write_time   | 0.000  |
| agent_io_write_ops    | 0      |
| agent_io_write_kbytes | 0.0    |
| keyword[0]            | one    |
| docs[0]               | 224387 |
| hits[0]               | 310327 |
| keyword[1]            | three  |
| docs[1]               | 18181  |
| hits[1]               | 21102  |
| keyword[2]            | two    |
| docs[2]               | 63251  |
| hits[2]               | 75961  |
+-----------------------+--------+
27 rows in set (0.00 sec)
```

<!-- end -->

<!-- example show meta predicted_time -->
अतिरिक्त मान, जैसे कि `predicted_time`, `dist_predicted_time`, `local_fetched_docs`, `local_fetched_hits`, `local_fetched_skips`, और उनके संबंधित `dist_fetched_*` समकक्ष, केवल तभी उपलब्ध होंगे यदि `searchd` को [पूर्वानुमानित समय लागत](../Server_settings/Searchd.md#predicted_time_costs) के साथ कॉन्फ़िगर किया गया हो और क्वेरी में `OPTION` खंड में `predicted_time` शामिल हो।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SELECT id,story_author FROM hn_small WHERE MATCH('one|two|three') limit 5 option max_predicted_time=100;

SHOW META;
```

<!-- response SQL -->

```sql
+--------+--------------+
| id     | story_author |
+--------+--------------+
| 300263 | throwaway37  |
| 713503 | mahmud       |
| 716804 | mahmud       |
| 776906 | jimbokun     |
| 753332 | foxhop       |
+--------+--------------+
5 rows in set (0.01 sec)

mysql> show meta;
+---------------------+--------+
| Variable_name       | Value  |
+---------------------+--------+
| total               | 5      |
| total_found         | 266385 |
| total_relation      | eq     |
| time                | 0.012  |
| local_fetched_docs  | 307212 |
| local_fetched_hits  | 407390 |
| local_fetched_skips | 24     |
| predicted_time      | 56     |
| keyword[0]          | one    |
| docs[0]             | 224387 |
| hits[0]             | 310327 |
| keyword[1]          | three  |
| docs[1]             | 18181  |
| hits[1]             | 21102  |
| keyword[2]          | two    |
| docs[2]             | 63251  |
| hits[2]             | 75961  |
+---------------------+--------+
17 rows in set (0.00 sec)
```

<!-- end -->

<!-- example show meta single statement -->

`SHOW META` को क्वेरी के तुरंत बाद **same** सत्र में निष्पादित किया जाना चाहिए। चूंकि कुछ MySQL कनेक्टर/लाइब्रेरी कनेक्शन पूल का उपयोग करते हैं, अलग कथन में `SHOW META` चलाने से अप्रत्याशित परिणाम मिल सकते हैं, जैसे कि किसी अन्य क्वेरी से मेटाडेटा प्राप्त करना। इन मामलों में (और सामान्य रूप से अनुशंसित), एक बहु-कथन चलाएँ जिसमें दोनों क्वेरी और `SHOW META` शामिल हो। कुछ कनेक्टर/लाइब्रेरी एकल कथन के लिए एक ही विधि में बहु-प्रश्नों का समर्थन करती हैं, जबकि अन्य को बहु-प्रश्नों के लिए समर्पित विधियों का उपयोग करने या कनेक्शन सेटअप के दौरान विशिष्ट विकल्प सेट करने की आवश्यकता हो सकती है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SELECT id,story_author FROM hn_small WHERE MATCH('one|two|three') LIMIT 5; SHOW META;
```

<!-- response SQL -->

```sql
+--------+--------------+
| id     | story_author |
+--------+--------------+
| 300263 | throwaway37  |
| 713503 | mahmud       |
| 716804 | mahmud       |
| 776906 | jimbokun     |
| 753332 | foxhop       |
+--------+--------------+
5 rows in set (0.01 sec)

+----------------+--------+
| Variable_name  | Value  |
+----------------+--------+
| total          | 5      |
| total_found    | 266385 |
| total_relation | eq     |
| time           | 0.011  |
| keyword[0]     | one    |
| docs[0]        | 224387 |
| hits[0]        | 310327 |
| keyword[1]     | three  |
| docs[1]        | 18181  |
| hits[1]        | 21102  |
| keyword[2]     | two    |
| docs[2]        | 63251  |
| hits[2]        | 75961  |
+----------------+--------+
13 rows in set (0.00 sec)
```

<!-- end -->

<!-- example SHOW META LIKE -->

आप वैकल्पिक LIKE खंड का भी उपयोग कर सकते हैं, जो आपको केवल उन चर को चुनने की अनुमति देता है जो एक विशिष्ट पैटर्न के साथ मेल खाते हैं। पैटर्नyntax मानक SQL वाइल्डकार्ड का पालन करता है, जहां `%` किसी भी संख्या के किसी भी अक्षरों का प्रतिनिधित्व करता है, और `_` एकल अक्षर का प्रतिनिधित्व करता है।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SHOW META LIKE 'total%';
```

<!-- response SQL -->

```sql
+----------------+--------+
| Variable_name  | Value  |
+----------------+--------+
| total          | 5      |
| total_found    | 266385 |
| total_relation | eq     |
+----------------+--------+
3 rows in set (0.00 sec)
```

<!-- end -->

## SHOW META और श्रेणियाँ

<!-- example show meta facets -->

जब आप [फैसिटेड सर्च](../Searching/Faceted_search.md) का उपयोग करते हैं, तो आप यह देखने के लिए `SHOW META` आउटपुट में `multiplier` फ़ील्ड की जांच कर सकते हैं कि कितनी क्वेरी एक अनुकूलित समूह में निष्पादित की गई थीं।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SELECT * FROM facetdemo FACET brand_id FACET price FACET categories;
SHOW META LIKE 'multiplier';
```

<!-- response SQL -->

```sql
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
| id   | price | brand_id | title               | brand_name  | property    | j                                     | categories |
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
|    1 |   306 |        1 | Product Ten Three   | Brand One   | Six_Ten     | {"prop1":66,"prop2":91,"prop3":"One"} | 10,11      |
...

+----------+----------+
| brand_id | count(*) |
+----------+----------+
|        1 |     1013 |
...

+-------+----------+
| price | count(*) |
+-------+----------+
|   306 |        7 |
...

+------------+----------+
| categories | count(*) |
+------------+----------+
|         10 |     2436 |
...

+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| multiplier    | 4     |
+---------------+-------+
1 row in set (0.00 sec)
```

<!-- end -->

## SHOW META और क्वेरी ऑप्टिमाइज़र

<!-- example of show meta vs query optimizer -->
जब [लागत-आधारित क्वेरी ऑप्टिमाइज़र](../Searching/Cost_based_optimizer.md) `DocidIndex`, `ColumnarScan` या `SecondaryIndex` को एक साधारण फ़िल्टर के बजाय उपयोग करने का चयन करता है, तो यह `SHOW META` कमांड में दर्शाया जाता है।

`index` वेरिएबल क्वेरी निष्पादन के दौरान उपयोग किए गए सेकंडरी इंडेक्स के नाम और प्रकार प्रदर्शित करता है। प्रतिशत यह दिखाता है कि कितने डिस्क चंक्स (RT तालिका के मामले में) या псuedo शार्ड्स (साधारण तालिका के मामले में) ने सेकंडरी इंडेक्स का उपयोग किया।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SELECT count(*) FROM taxi1 WHERE tip_amount = 5;
SHOW META;
```

<!-- response SQL -->

```sql
+----------------+----------------------------------+
| Variable_name  | Value                            |
+----------------+----------------------------------+
| total          | 1                                |
| total_found    | 1                                |
| total_relation | eq                               |
| time           | 0.016                            |
| index          | tip_amount:SecondaryIndex (100%) |
+----------------+----------------------------------+
5 rows in set (0.00 sec)
```

<!-- end -->

## PQ तालिकाओं के लिए SHOW META

<!-- example show meta PQ -->

`SHOW META` को [CALL PQ](../Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ) स्टेटमेंट निष्पादित करने के बाद उपयोग किया जा सकता है, जिस स्थिति में यह विभिन्न आउटपुट प्रदान करता है।

`CALL PQ` स्टेटमेंट के बाद `SHOW META` में शामिल हैं:

* `total` - दस्तावेज़ों से मेल खाने में बिताया गया कुल समय
* `queries_matched` - मौजूदा दस्तावेज़(ों) से मेल खाने वाले संग्रहीत क्वेरी की संख्या
* `document_matches` - उन दस्तावेज़ों की संख्या जो तालिका में संग्रहीत क्वेरियों से मेल खाती हैं
* `total_queries_stored` - तालिका में संग्रहीत कुल क्वेरियों की संख्या
* `term_only_queries` - तालिका में ऐसे क्वेरियों की संख्या जिनमें केवल शर्तें हैं; शेष क्वेरियां विस्तारित क्वेरी सिंटैक्स का उपयोग करती हैं।

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CALL PQ ('pq', ('{"title":"angry", "gid":3 }')); SHOW META;
```

<!-- response SQL -->

```sql
+------+
| id   |
+------+
|    2 |
+------+
1 row in set (0.00 sec)

+-----------------------+-----------+
| Variable name         | Value     |
+-----------------------+-----------+
| total                 | 0.000 sec |
| queries_matched       | 1         |
| queries_failed        | 0         |
| document_matched      | 1         |
| total_queries_stored  | 2         |
| term_only_queries     | 2         |
| fast_rejected_queries | 1         |
+-----------------------+-----------+
7 rows in set (0.00 sec)
```

<!-- end -->

<!-- example call pq verbose meta  -->

`CALL PQ` का उपयोग करते समय `verbose` विकल्प अधिक विस्तृत आउटपुट प्रदान करता है।

इसमें निम्नलिखित अतिरिक्त प्रव Entries शामिल हैं:

* `Setup` - मिलान प्रक्रिया के प्रारंभिक सेटअप में बिताया गया समय, जैसे डॉक पार्सिंग और विकल्प सेट करना
* `Queries failed` - विफल क्वेरियों की संख्या
* `Fast rejected queries` - क्वेरियों की संख्या जो पूरी तरह से मूल्यांकन नहीं की गई थीं लेकिन जल्दी से मेल खा गईं और फ़िल्टर या अन्य संवेदनाओं का उपयोग करके अस्वीकृत कर दी गईं
* `Time per query` - प्रत्येक क्वेरी के लिए विस्तृत समय
* `Time of matched queries` - उन क्वेरियों पर कुल समय जो किसी भी दस्तावेज़ से मेल खाती हैं


<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CALL PQ ('pq', ('{"title":"angry", "gid":3 }'), 1 as verbose); SHOW META;
```

<!-- response SQL -->

```sql
+------+
| id   |
+------+
|    2 |
+------+
1 row in set (0.00 sec)

+-------------------------+-----------+
| Variable name           | Value     |
+-------------------------+-----------+
| total                   | 0.000 sec |
| setup                   | 0.000 sec |
| queries_matched         | 1         |
| queries_failed          | 0         |
| document_matched        | 1         |
| total_queries_stored    | 2         |
| term_only_queries       | 2         |
| fast_rejected_queries   | 1         |
| time_per_query          | 69        |
| time_of_matched_queries | 69        |
+-------------------------+-----------+
10 rows in set (0.00 sec)
```

<!-- end -->

<!-- proofread -->

