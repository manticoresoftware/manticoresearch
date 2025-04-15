# SphinxSE

SphinxSE एक MySQL स्टोरेज इंजन है जिसे MySQL/MariaDB सर्वरों में उनके प्लगइन आर्किटेक्चर का उपयोग करके संकलित किया जा सकता है।

इसके नाम के बावजूद, SphinxSE वास्तव में *किसी भी डेटा* को स्वयं संग्रहीत नहीं करता है। इसके बजाय, यह एक अंतर्निहित क्लाइंट के रूप में कार्य करता है जो MySQL सर्वर को `searchd` के साथ संवाद करने, खोज क्वेरी निष्पादित करने और खोज परिणाम प्राप्त करने की अनुमति देता है। सभी अनुक्रमण और खोज MySQL के बाहर होती हैं।

कुछ सामान्य SphinxSE अनुप्रयोगों में शामिल हैं:
* Manticore पर MySQL पूर्ण-टेक्स्ट खोज (FTS) अनुप्रयोगों का पोर्टिंग सरल बनाना;
* Manticore का उपयोग उन प्रोग्रामिंग भाषाओं के साथ सक्षम करना जिनके लिए मूल API अभी तक उपलब्ध नहीं हैं;
* जब MySQL पक्ष पर अतिरिक्त Manticore परिणाम सेट प्रसंस्करण की आवश्यकता होती है तो अनुकूलन प्रदान करना (जैसे, मूल दस्तावेज़ तालिकाओं या अतिरिक्त MySQL-पक्ष फ़िल्टरिंग के साथ JOINS)।

## SphinxSE स्थापित करना

आपको MySQL स्रोतों की एक प्रति प्राप्त करनी होगी, उन्हें तैयार करना होगा, और फिर MySQL बाइनरी को फिर से संकलित करना होगा। MySQL स्रोत (mysql-5.x.yy.tar.gz) को <http://dev.mysql.com> वेबसाइट से प्राप्त किया जा सकता है।

### SphinxSE के साथ MySQL 5.0.x संकलित करना

1.  `sphinx.5.0.yy.diff` पैच फ़ाइल को MySQL स्रोत निर्देशिका में कॉपी करें और चलाएँ
```bash
$ patch -p1 < sphinx.5.0.yy.diff
```
यदि आपके आवश्यक विशिष्ट संस्करण के लिए कोई .diff फ़ाइल नहीं है: निर्माण करें, निकटतम संस्करण नंबर के साथ .diff लगाने का प्रयास करें। यह महत्वपूर्ण है कि पैच बिना किसी अस्वीकृति के लागू होना चाहिए।
2.  MySQL स्रोत निर्देशिका में चलाएँ
```bash
$ sh BUILD/autorun.sh
```
3.  MySQL स्रोत निर्देशिका में, `sql/sphinx` निर्देशिका बनाएं और Manticore स्रोतों से `mysqlse` निर्देशिका में सभी फ़ाइलों को वहां कॉपी करें। उदाहरण:
```bash
$ cp -R /root/builds/sphinx-0.9.7/mysqlse /root/builds/mysql-5.0.24/sql/sphinx
```
4.  MySQL को कॉन्फ़िगर करें और नए इंजन को सक्षम करें:
```bash
$ ./configure --with-sphinx-storage-engine
```
5.  MySQL का निर्माण और स्थापना करें:
```bash
$ make
$ make install
```

### SphinxSE के साथ MySQL 5.1.x संकलित करना

1. MySQL स्रोत निर्देशिका में, एक `storage/sphinx` निर्देशिका बनाएँ और Manticore स्रोतों में `mysqlse` निर्देशिका से सभी फ़ाइलों को इस नए स्थान पर कॉपी करें। उदाहरण:
```bash
$ cp -R /root/builds/sphinx-0.9.7/mysqlse /root/builds/mysql-5.1.14/storage/sphinx
```
2.  MySQL स्रोत निर्देशिका में, चलाएँ:
```bash
$ sh BUILD/autorun.sh
```
3. MySQL को कॉन्फ़िगर करें और Manticore इंजन को सक्षम करें:
```bash
$ ./configure --with-plugins=sphinx
```
4. MySQL का निर्माण और स्थापना करें:
```bash
$ make
$ make install
```

### SphinxSE स्थापना की जांच


<!-- example Example_1 -->

यह सत्यापित करने के लिए कि SphinxSE को सफलतापूर्वक MySQL में संकलित किया गया है, नए बनाए गए सर्वर को प्रारंभ करें, MySQL क्लाइंट चलाएँ, और `SHOW ENGINES` क्वेरी जारी करें। आपको सभी उपलब्ध इंजनों की एक सूची देखनी चाहिए। Manticore मौजूद होना चाहिए, और "सहारा" कॉलम "YES" प्रदर्शित करना चाहिए:

<!-- request -->

``` sql
mysql> show engines;
```

<!-- response -->
```sql
+------------+----------+-------------------------------------------------------------+
| Engine     | Support  | Comment                                                     |
+------------+----------+-------------------------------------------------------------+
| MyISAM     | DEFAULT  | MySQL 3.23 के रूप में डिफ़ॉल्ट इंजन जिसमें महान प्रदर्शन      |
  ...
| SPHINX     | YES      | Manticore स्टोरेज इंजन                                       |
  ...
+------------+----------+-------------------------------------------------------------+
13 पंक्तियाँ सेट में (0.00 सेकंड)
```

<!-- end -->

## SphinxSE का उपयोग करना

SphinxSE का उपयोग करके खोजने के लिए, आपको एक विशेष ENGINE=SPHINX "खोज तालिका" बनानी होगी और फिर खोज कॉलम के लिए `WHERE` क्लॉज में पूर्ण-टेक्स्ट क्वेरी के साथ एक `SELECT` कथन का उपयोग करना होगा।

यहाँ एक उदाहरण बनाने का कथन और खोज क्वेरी है:

```sql
CREATE TABLE t1
(
    id          INTEGER UNSIGNED NOT NULL,
    weight      INTEGER NOT NULL,
    query       VARCHAR(3072) NOT NULL,
    group_id    INTEGER,
    INDEX(query)
) ENGINE=SPHINX CONNECTION="sphinx://localhost:9312/test";

SELECT * FROM t1 WHERE query='test it;mode=any';
```

एक खोज तालिका में, पहले तीन कॉलमों के निम्नलिखित प्रकार *जरूरी* हैं: 1st कॉलम (दस्तावेज़ ID) के लिए `INTEGER UNSIGNED` या `BIGINT`, 2nd कॉलम (मैच वजन) के लिए `INTEGER` या `BIGINT`, और 3rd कॉलम (आपकी क्वेरी) के लिए `VARCHAR` या `TEXT`। यह मैपिंग निश्चित है; आप इनमें से किसी भी तीन आवश्यक कॉलम को छोड़ नहीं सकते, उन्हें स्थानांतरित नहीं कर सकते या उनके प्रकार बदल नहीं सकते। इसके अतिरिक्त, क्वेरी कॉलम को अनुक्रमित किया जाना चाहिए, जबकि सभी अन्य को अनियमित रहना चाहिए। कॉलम नामों को अनदेखा किया जाता है, इसलिए आप किसी भी मनचाही नाम का उपयोग कर सकते हैं।

अतिरिक्त कॉलम या तो `INTEGER`, `TIMESTAMP`, `BIGINT`, `VARCHAR`, या `FLOAT` होने चाहिए। उन्हें Manticore परिणाम सेट में नाम द्वारा प्रदान किए गए विशेषताओं से बंधा जाएगा, इसलिए उनके नाम `sphinx.conf` में निर्दिष्ट विशेषता नामों से मेल खाने चाहिए। यदि Manticore खोज परिणामों में कोई मेल खाता हुआ विशेषता नाम नहीं है, तो कॉलम में `NULL` मान होंगे।

विशेष "वर्चुअल" विशेषता नाम भी SphinxSE कॉलम से बंध सकते हैं। इसके लिए `_sph_` का उपयोग करें। उदाहरण के लिए, `@groupby`, `@count`, या `@distinct` वर्चुअल विशेषताओं के मान प्राप्त करने के लिए, क्रमशः `_sph_groupby`, `_sph_count`, या `_sph_distinct` कॉलम नामों का उपयोग करें।

`CONNECTION` स्ट्रिंग पैरामीटर का उपयोग Manticore होस्ट, पोर्ट, और तालिका निर्दिष्ट करने के लिए किया जाता है। यदि `CREATE TABLE` में कोई कनेक्शन स्ट्रिंग निर्दिष्ट नहीं है, तो तालिका का नाम `*` (यानी, सभी तालिकाओं की खोज) और `localhost:9312` मान लिया जाता है। कनेक्शन स्ट्रिंग का व्याकरण इस प्रकार है:

```
CONNECTION="sphinx://HOST:PORT/TABLENAME"
```

आप इन पैरामीटर को बाद में बदल सकते हैं:

```sql
mysql> ALTER TABLE t1 CONNECTION="sphinx://NEWHOST:NEWPORT/NEWTABLENAME";
```

आप प्रत्येक क्वेरी के आधार पर इन पैरामीटर को भी ओवरराइड कर सकते हैं।
उदाहरण में दिखाए अनुसार, प्रश्न पाठ और खोज विकल्पों को खोज क्वेरी कॉलम में `WHERE` खंड में रखा जाना चाहिए (यानी, 3री कॉलम)। विकल्पों को सेमीकोलन से अलग किया गया है और उनके नामों को मानों से समानता चिह्न द्वारा अलग किया गया है। किसी भी संख्या में विकल्प निर्दिष्ट किए जा सकते हैं। उपलब्ध विकल्प हैं:

* प्रश्न - प्रश्न पाठ;
* मोड - मिलान मोड। "all", "any", "phrase", "boolean", या "extended" में से एक होना चाहिए। डिफ़ॉल्ट "all" है;
* сорт - मिलान क्रमबद्धता मोड। "relevance", "attr_desc", "attr_asc", "time_segments", या "extended" में से एक होना चाहिए। "relevance" के अलावा अन्य सभी मोड में, एक विशेषता नाम (या "extended" के लिए क्रमबद्धता खंड) भी कॉलन के बाद आवश्यक है:
```
... WHERE query='test;sort=attr_asc:group_id';
... WHERE query='test;sort=extended:@weight desc, group_id asc';
```
* ऑफसेट - परिणाम सेट में ऑफसेट; डिफ़ॉल्ट 0 है;
* सीमा - परिणाम सेट से प्राप्त करने के लिए मिलानों की संख्या; डिफ़ॉल्ट 20 है;
* इंडेक्स - खोज के लिए तालिकाओं के नाम:
```sql
... WHERE query='test;index=test1;';
... WHERE query='test;index=test1,test2,test3;';
```
* minid, maxid - मेल खाने के लिए न्यूनतम और अधिकतम दस्तावेज़ आईडी;
* वेट्स - Manticore पूर्ण-पाठ क्षेत्रों को आवंटित किए जाने वाले वज़नों की कॉमा द्वारा अलग की गई सूची:
```sql
... WHERE query='test;weights=1,2,3;';
```
* फ़िल्टर, !filter - मेल खाने के लिए कॉमा द्वारा अलग की गई विशेषता नाम और मानों का सेट:
```sql
# केवल समूह 1, 5 और 19 शामिल करें
... WHERE query='test;filter=group_id,1,5,19;';
# समूह 3 और 11 को बाहर करें
... WHERE query='test;!filter=group_id,3,11;';
```
* रेंज, !range - मेल खाने के लिए कॉमा द्वारा अलग की गई (पूर्णांक या बिगइंट) Manticore विशेषता नाम, और न्यूनतम और अधिकतम मान:
```sql
# 3 से 7 के बीच के समूह शामिल करें, समावेशी
... WHERE query='test;range=group_id,3,7;';
# 5 से 25 के बीच के समूहों को बाहर करें
... WHERE query='test;!range=group_id,5,25;';
```
* floatrange, !floatrange - मेल खाने के लिए कॉमा द्वारा अलग की गई (फ्लोटिंग प्वाइंट) Manticore विशेषता नाम, और न्यूनतम और अधिकतम मान:
```sql
# एक फ्लोट आकार द्वारा छानें
... WHERE query='test;floatrange=size,2,3;';
# geoanchor से 1000 मीटर के भीतर सभी परिणाम चुनें
... WHERE query='test;floatrange=@geodist,0,1000;';
```
* maxmatches - maxmatches - प्रति-प्रश्न अधिकतम मैच मान, जैसे [max_matches खोज विकल्प](../Searching/Options.md#max_matches):
```sql
... WHERE query='test;maxmatches=2000;';
```
* कटऑफ - अधिकतम अनुमोदित मेल, जैसे [cutoff खोज विकल्प](../Searching/Options.md#cutoff):
```sql
... WHERE query='test;cutoff=10000;';
```
* maxquerytime - अधिकतम अनुमोदित प्रश्न समय (मिलीसेकंड में), जैसे [max_query_time खोज विकल्प](../Searching/Options.md#max_query_time):
```sql
... WHERE query='test;maxquerytime=1000;';
```
* groupby - समूह द्वारा कार्य और विशेषता। खोज परिणामों को समूहित करने के बारे में [यह पढ़ें](../Searching/Grouping.md#Just-Grouping):
```sql
... WHERE query='test;groupby=day:published_ts;';
... WHERE query='test;groupby=attr:group_id;';
```
* groupsort - समूह द्वारा क्रमबद्धता खंड:
```sql
... WHERE query='test;groupsort=@count desc;';
```
* distinct - [COUNT(DISTINCT)](../Searching/Grouping.md#COUNT%28DISTINCT-field%29) को समूह-जब समूह करने के लिए गणना करने के लिए एक विशेषता:
```sql
... WHERE query='test;groupby=attr:country_id;distinct=site_id';
```
* indexweights - कई तालिकाओं के माध्यम से खोज करते समय उपयोग करने के लिए तालिका नामों और वज़नों की कॉमा द्वारा अलग की गई सूची:
```sql
... WHERE query='test;indexweights=tbl_exact,2,tbl_stemmed,1;';
```
* fieldweights - ऐसे प्रति-क्षेत्र वज़नों की कॉमा द्वारा अलग की गई सूची जो रेंकर द्वारा उपयोग की जा सकती हैं:
```sql
... WHERE query='test;fieldweights=title,10,abstract,3,content,1;';
```
* टिप्पणी - प्रश्न लॉग में इस प्रश्न को चिह्नित करने के लिए एक स्ट्रिंग, जैसे [comment खोज विकल्प](../Searching/Options.md#comment):
```sql
... WHERE query='test;comment=marker001;';
```
* चयन - गणना करने के लिए अभिव्यक्तियों के साथ एक स्ट्रिंग:
```sql
... WHERE query='test;select=2*a+3*** as myexpr;';
```
* होस्ट, पोर्ट - क्रमशः रिमोट `searchd` होस्ट नाम और TCP पोर्ट:
```sql
... WHERE query='test;host=sphinx-test.loc;port=7312;';
```
* रेंकर - "extended" मिलान मोड के साथ उपयोग करने के लिए एक रैंकिंग कार्य, जैसे [ranker](../Searching/Options.md#ranker)। ज्ञात मान हैं "proximity_bm25", "bm25", "none", "wordcount", "proximity", "matchany", "fieldmask", "sph04", "expr:EXPRESSION" सिंटैक्स एक्सप्रेशन-आधारित रेंकर का समर्थन करने के लिए (जहां एक्सप्रेशन को आपके विशिष्ट रैंकिंग सूत्र के साथ प्रतिस्थापित किया जाना चाहिए), और "export:EXPRESSION":
```sql
... WHERE query='test;mode=extended;ranker=bm25;';
... WHERE query='test;mode=extended;ranker=expr:sum(lcs);';
```
"export" रेंकर रेंकर=expr के समान कार्य करता है, लेकिन यह प्रति-दस्तावेज़ कारक मानों को बनाए रखता है, जबकि रेंकर=expr उन्हें अंतिम `WEIGHT()` मान की गणना के बाद छोड़ देता है। ध्यान रखें कि रेंकर=export का उपयोग असामान्य मामलों में, जैसे मशीन लर्निंग (ML) कार्य को प्रशिक्षित करने या अपने स्वयं के रैंकिंग कार्य को मैन्युअल रूप से परिभाषित करने के लिए किया जाना है, और इसे वास्तविक उत्पादन में नहीं किया जाना चाहिए। इस रेंकर का उपयोग करते समय, आप संभवतः `RANKFACTORS()` कार्य का आउटपुट देखना चाहेंगे, जो प्रत्येक दस्तावेज़ के लिए सभी क्षेत्र-स्तरीय कारकों को उत्पन्न करता है।

<!-- उदाहरण SQL Example_2 -->
<!-- अनुरोध -->

``` sql
SELECT *, WEIGHT(), RANKFACTORS()
    FROM myindex
    WHERE MATCH('dog')
    OPTION ranker=export('100*bm25');
```

<!-- प्रतिक्रिया -->

``` sql
*************************** 1\. row ***************************
           id: 555617
    published: 1110067331
   channel_id: 1059819
        title: 7
      content: 428
     weight(): 69900
rankfactors(): bm25=699, bm25a=0.666478, field_mask=2,
doc_word_count=1, field1=(lcs=1, hit_count=4, word_count=1,
tf_idf=1.038127, min_idf=0.259532, max_idf=0.259532, sum_idf=0.259532,
min_hit_pos=120, min_best_span_pos=120, exact_hit=0,
max_window_hits=1), word1=(tf=4, idf=0.259532)
*************************** 2\. row ***************************
           id: 555313
    published: 1108438365
   channel_id: 1058561
        title: 8
      content: 249
     weight(): 68500
rankfactors(): bm25=685, bm25a=0.675213, field_mask=3,
doc_word_count=1, field0=(lcs=1, hit_count=1, word_count=1,
tf_idf=0.259532, min_idf=0.259532, max_idf=0.259532, sum_idf=0.259532,
min_hit_pos=8, min_best_span_pos=8, exact_hit=0, max_window_hits=1),
field1=(lcs=1, hit_count=2, word_count=1, tf_idf=0.519063,
min_idf=0.259532, max_idf=0.259532, sum_idf=0.259532, min_hit_pos=36,
min_best_span_pos=36, exact_hit=0, max_window_hits=1), word1=(tf=3,
idf=0.259532)
```

<!-- end -->

* geoanchor - जियोडिस्टैंस एंकर। जियो-सर्च के बारे में [इस अनुभाग में](../Searching/Geo_search.md) और जानें। यह 4 पैरामीटर्स लेता है, जो कि लेटिट्यूड और लोंगीट्यूड एट्रीब्यूट नाम, और एंकर पॉइंट कोऑर्डिनेट्स, क्रमशः हैं:
```sql
... WHERE query='test;geoanchor=latattr,lonattr,0.123,0.456';
```

एक **बहुत महत्वपूर्ण** नोट यह है कि मैक्स मैचेस काउंट बढ़ाने और MySQL साइड पर `WHERE`, `ORDER BY`, और `LIMIT` क्लॉज़ का उपयोग करने की तुलना में मैन्टिकोर को सॉर्टिंग, फिल्टरिंग और परिणाम सेट को स्लाइस करने के लिए अधिक कुशलता से संभालने देना **बहुत ज्यादा** बेहतर है। इसके दो कारण हैं। पहला, मैन्टिकोर विभिन्न ऑप्टिमाइजेशन का उपयोग करता है और इन कार्यों को MySQL से बेहतर करता है। दूसरा, कम डेटा को searchd द्वारा पैक, स्थानांतरित, और SphinxSE द्वारा अनपैक करने की आवश्यकता होगी।


<!-- example Example_3 -->

`SHOW ENGINE SPHINX STATUS` कथन का उपयोग करके आप क्वेरी परिणामों से संबंधित अतिरिक्त जानकारी प्राप्त कर सकते हैं:

<!-- request -->


``` sql
mysql> SHOW ENGINE SPHINX STATUS;
```

<!-- response -->
``` sql
+--------+-------+-------------------------------------------------+
| Type   | Name  | Status                                          |
+--------+-------+-------------------------------------------------+
| SPHINX | stats | total: 25, total found: 25, time: 126, words: 2 |
| SPHINX | words | sphinx:591:1256 soft:11076:15945                |
+--------+-------+-------------------------------------------------+
2 rows in set (0.00 sec)
```

<!-- end -->


<!-- example Example_4 -->

आप स्थिति वेरिएबल्स के माध्यम से इस जानकारी तक भी पहुंच सकते हैं। ध्यान दें कि इस विधि का उपयोग करते समय सुपर-यूज़र विशेषाधिकारों की आवश्यकता नहीं है।

<!-- request -->

``` sql
mysql> SHOW STATUS LIKE 'sphinx_%';
```

<!-- response -->
``` sql
+--------------------+----------------------------------+
| Variable_name      | Value                            |
+--------------------+----------------------------------+
| sphinx_total       | 25                               |
| sphinx_total_found | 25                               |
| sphinx_time        | 126                              |
| sphinx_word_count  | 2                                |
| sphinx_words       | sphinx:591:1256 soft:11076:15945 |
+--------------------+----------------------------------+
5 rows in set (0.00 sec)
```

<!-- end -->


<!-- example SQL Example_5 -->

SphinxSE सर्च टेबल्स को अन्य इंजनों का उपयोग करके टेबल्स के साथ जोड़ा जा सकता है। यहां example.sql से "documents" टेबल का उपयोग करते हुए एक उदाहरण है:

<!-- request -->

``` sql
mysql> SELECT content, date_added FROM test.documents docs
-> JOIN t1 ON (docs.id=t1.id)
-> WHERE query="one document;mode=any";

mysql> SHOW ENGINE SPHINX STATUS;
```

<!-- response -->

``` sql
+-------------------------------------+---------------------+
| content                             | docdate             |
+-------------------------------------+---------------------+
| this is my test document number two | 2006-06-17 14:04:28 |
| this is my test document number one | 2006-06-17 14:04:28 |
+-------------------------------------+---------------------+
2 rows in set (0.00 sec)

+--------+-------+---------------------------------------------+
| Type   | Name  | Status                                      |
+--------+-------+---------------------------------------------+
| SPHINX | stats | total: 2, total found: 2, time: 0, words: 2 |
| SPHINX | words | one:1:2 document:2:2                        |
+--------+-------+---------------------------------------------+
2 rows in set (0.00 sec)
```

<!-- end -->

## MySQL के माध्यम से स्निपेट्स बनाना


SphinxSE में एक UDF फंक्शन भी है जो आपको MySQL का उपयोग करके स्निपेट्स बनाने की अनुमति देता है। यह कार्यक्षमता [HIGHLIGHT()](../../Searching/Highlighting.md#Highlighting) के समान है, लेकिन MySQL+SphinxSE के माध्यम से एक्सेस किया जा सकता है।

UDF प्रदान करने वाली बाइनरी को `sphinx.so` कहा जाता है और इसे SphinxSE के साथ स्वचालित रूप से उपयुक्त स्थान पर बनाया और स्थापित किया जाना चाहिए। यदि यह किसी कारणवश स्वचालित रूप से स्थापित नहीं होता है, तो बिल्ट निर्देशिका में `sphinx.so` ढूंढें और इसे आपके MySQL इंस्टेंस के प्लगइन्स डायरेक्टरी में कॉपी करें। एक बार ऐसा करने के बाद, निम्नलिखित कथन के साथ UDF को पंजीकृत करें:

```sql
CREATE FUNCTION sphinx_snippets RETURNS STRING SONAME 'sphinx.so';
```

फंक्शन का नाम *sphinx_snippets* होना **चाहिए**; आप कोई मनमाना नाम उपयोग नहीं कर सकते। फंक्शन के तर्क निम्नानुसार हैं:

**प्रोटोटाइप:** function sphinx_snippets ( document, table, words [, options] );

दस्तावेज़ और शब्द तर्क स्ट्रिंग या टेबल कॉलम दोनों हो सकते हैं। विकल्प इस तरह से निर्दिष्ट किए जाने चाहिए: `'value' AS option_name`। समर्थित विकल्पों की सूची के लिए, [हाइलाइटिंग अनुभाग](../Searching/Highlighting.md) देखें। UDF के साथ विशिष्ट अतिरिक्त विकल्प को `sphinx` कहा जाता है और यह आपको searchd स्थान (होस्ट और पोर्ट) निर्दिष्ट करने की अनुमति देता है।

उपयोग के उदाहरण:

```sql
SELECT sphinx_snippets('hello world doc', 'main', 'world',
    'sphinx://192.168.1.1/' AS sphinx, true AS exact_phrase,
    '[**]' AS before_match, '[/**]' AS after_match)
FROM documents;

SELECT title, sphinx_snippets(text, 'table', 'mysql php') AS text
    FROM sphinx, documents
    WHERE query='mysql php' AND sphinx.id=documents.id;
```
<!-- proofread -->

