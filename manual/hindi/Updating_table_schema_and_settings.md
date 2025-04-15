# तालिका स्कीमा को अपडेट करना

## RT मोड में तालिका स्कीमा को अपडेट करना

<!-- उदाहरण ALTER -->

```sql
ALTER TABLE table ADD COLUMN column_name [{INTEGER|INT|BIGINT|FLOAT|BOOL|MULTI|MULTI64|JSON|STRING|TIMESTAMP|TEXT [INDEXED [ATTRIBUTE]]}] [engine='columnar']

ALTER TABLE table DROP COLUMN column_name

ALTER TABLE table MODIFY COLUMN column_name bigint
```

यह फ़ीचर RT तालिकाओं के लिए एक समय में एक फ़ील्ड जोड़ने या `int` कॉलम को `bigint` में विस्तारित करने का समर्थन करता है। समर्थित डेटा प्रकार हैं:
* `int` - पूर्णांक विशेषता
* `timestamp` - समय मुहर विशेषता
* `bigint` - बड़ा पूर्णांक विशेषता
* `float` - फ़्लोट विशेषता
* `bool` - बूलियन विशेषता
* `multi` - बहु-मान वाला पूर्णांक विशेषता
* `multi64` - बहु-मान वाला बड़ा पूर्णांक विशेषता
* `json` - जेसन विशेषता
* `string` / `text attribute` / `string attribute` - स्ट्रिंग विशेषता
* `text` / `text indexed stored` / `string indexed stored` - मूल मूल्य को docstore में संग्रहीत किए गए पूर्ण-टेक्स्ट अनुक्रमित फ़ील्ड
* `text indexed` / `string indexed` - पूर्ण-टेक्स्ट अनुक्रमित फ़ील्ड, केवल अनुक्रमित (मूल मूल्य docstore में संग्रहीत नहीं है)
* `text indexed attribute` / `string indexed attribute` - पूर्ण टेक्स्ट अनुक्रमित फ़ील्ड + स्ट्रिंग विशेषता (मूल मूल्य को docstore में संग्रहीत नहीं कर रहा है)
* `text stored` / `string stored` - मूल्य केवल docstore में संग्रहीत होगा, पूर्ण-टेक्स्ट अनुक्रमित नहीं है, ना ही यह एक स्ट्रिंग विशेषता है
* किसी भी विशेषता में `engine='columnar'` जोड़ने से (जेसन को छोड़कर) इसे [columnar storage](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) में संग्रहीत किया जाएगा

#### महत्वपूर्ण नोट्स:
* ❗ `ALTER` करने से पहले **तालिका फ़ाइलों का बैकअप** लेना अनुशंसित है ताकि अचानक पावर बाधा या अन्य समान समस्याओं के मामले में डेटा भ्रष्टाचार को रोका जा सके।
* एक कॉलम जोड़ा जा रहा है जबकि तालिका का प्रश्न करना असंभव है।
* नव निर्मित विशेषता के मान 0 पर सेट होते हैं।
* `ALTER` वितरित तालिकाओं और बिना किसी विशेषताओं वाली तालिकाओं के लिए काम नहीं करेगा।
* आप `id` कॉलम को हटा नहीं सकते।
* जब एक फ़ील्ड को हटाना जो पूर्ण-टेक्स्ट फ़ील्ड और स्ट्रिंग विशेषता दोनों है, पहला `ALTER DROP` विशेषता को गिरा देता है, दूसरा पूर्ण-टेक्स्ट फ़ील्ड को गिरा देता है।
* पूर्ण-टेक्स्ट फ़ील्ड को जोड़ने/हटाने का समर्थन केवल [RT मोड](Read_this_first.md#Real-time-mode-vs-plain-mode) में किया जाता है।

<!-- अनुरोध उदाहरण -->
```sql

mysql> desc rt;
+------------+-----------+
| Field      | Type      |
+------------+-----------+
| id         | bigint    |
| text       | field     |
| group_id   | uint      |
| date_added | timestamp |
+------------+-----------+

mysql> alter table rt add column test integer;

mysql> desc rt;
+------------+-----------+
| Field      | Type      |
+------------+-----------+
| id         | bigint    |
| text       | field     |
| group_id   | uint      |
| date_added | timestamp |
| test       | uint      |
+------------+-----------+

mysql> alter table rt drop column group_id;

mysql> desc rt;
+------------+-----------+
| Field      | Type      |
+------------+-----------+
| id         | bigint    |
| text       | field     |
| date_added | timestamp |
| test       | uint      |
+------------+-----------+

mysql> alter table rt add column title text indexed;

mysql> desc rt;
+------------+-----------+------------+
| Field      | Type      | Properties |
+------------+-----------+------------+
| id         | bigint    |            |
| text       | text      | indexed    |
| title      | text      | indexed    |
| date_added | timestamp |            |
| test       | uint      |            |
+------------+-----------+------------+

mysql> alter table rt add column title text attribute;

mysql> desc rt;
+------------+-----------+------------+
| Field      | Type      | Properties |
+------------+-----------+------------+
| id         | bigint    |            |
| text       | text      | indexed    |
| title      | text      | indexed    |
| date_added | timestamp |            |
| test       | uint      |            |
| title      | string    |            |
+------------+-----------+------------+

mysql> alter table rt drop column title;

mysql> desc rt;
+------------+-----------+------------+
| Field      | Type      | Properties |
+------------+-----------+------------+
| id         | bigint    |            |
| text       | text      | indexed    |
| title      | text      | indexed    |
| date_added | timestamp |            |
| test       | uint      |            |
+------------+-----------+------------+
mysql> alter table rt drop column title;

mysql> desc rt;
+------------+-----------+------------+
| Field      | Type      | Properties |
+------------+-----------+------------+
| id         | bigint    |            |
| text       | text      | indexed    |
| date_added | timestamp |            |
| test       | uint      |            |
+------------+-----------+------------+
```

<!-- अंत -->

## RT मोड में तालिका FT सेटिंग्स को अपडेट करना

<!-- उदाहरण ALTER FT -->

```sql
ALTER TABLE table ft_setting='value'[, ft_setting2='value']
```

आप अपने तालिका के पूर्ण-टेक्स्ट सेटिंग्स को [RT मोड](Read_this_first.md#Real-time-mode-vs-plain-mode) में संशोधित करने के लिए `ALTER` का उपयोग कर सकते हैं। हालांकि, इसका प्रभाव केवल नए दस्तावेज़ों पर होता है और मौजूदा दस्तावेज़ों पर नहीं।
उदाहरण:
* एक तालिका बनाएँ जिसमें एक पूर्ण-टेक्स्ट फ़ील्ड और `charset_table` हो जो केवल 3 खोजने योग्य वर्णों की अनुमति देता है: `a`, `b` और `c`।
* फिर हम दस्तावेज़ 'abcd' को सम्मिलित करते हैं और इसे प्रश्न `abcd` के माध्यम से खोजते हैं, `d` को अनदेखा किया जाता है क्योंकि यह `charset_table` एरे में नहीं है।
* फिर हम समझते हैं, कि हम चाहते हैं कि `d` भी खोजने योग्य हो, इसलिए हम `ALTER` की मदद से इसे जोड़ते हैं।
* लेकिन वही प्रश्न `where match('abcd')` अभी भी कहता है कि यह `abc` के द्वारा खोजा गया था, क्योंकि मौजूदा दस्तावेज़ की याददाश्त पहले के `charset_table` की सामग्री को याद रखती है।
* फिर हम एक और दस्तावेज़ `abcd` जोड़ते हैं और फिर `abcd` से खोज करते हैं
* अब यह दोनों दस्तावेज़ों को ढूंढता है और `show meta` कहता है कि यह दो कीवर्ड्स का उपयोग करता है: `abc` (पुराने दस्तावेज़ को ढूंढने के लिए) और `abcd` (नए के लिए)।

<!-- request Example -->
```sql
mysql> create table rt(title text) charset_table='a,b,c';

mysql> insert into rt(title) values('abcd');

mysql> select * from rt where match('abcd');
+---------------------+-------+
| id                  | title |
+---------------------+-------+
| 1514630637682688054 | abcd  |
+---------------------+-------+

mysql> show meta;
+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| total         | 1     |
| total_found   | 1     |
| time          | 0.000 |
| keyword[0]    | abc   |
| docs[0]       | 1     |
| hits[0]       | 1     |
+---------------+-------+

mysql> alter table rt charset_table='a,b,c,d';
mysql> select * from rt where match('abcd');
+---------------------+-------+
| id                  | title |
+---------------------+-------+
| 1514630637682688054 | abcd  |
+---------------------+-------+

mysql> show meta
+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| total         | 1     |
| total_found   | 1     |
| time          | 0.000 |
| keyword[0]    | abc   |
| docs[0]       | 1     |
| hits[0]       | 1     |
+---------------+-------+

mysql> insert into rt(title) values('abcd');
mysql> select * from rt where match('abcd');
+---------------------+-------+
| id                  | title |
+---------------------+-------+
| 1514630637682688055 | abcd  |
| 1514630637682688054 | abcd  |
+---------------------+-------+

mysql> show meta;
+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| total         | 2     |
| total_found   | 2     |
| time          | 0.000 |
| keyword[0]    | abc   |
| docs[0]       | 1     |
| hits[0]       | 1     |
| keyword[1]    | abcd  |
| docs[1]       | 1     |
| hits[1]       | 1     |
+---------------+-------+
```

<!-- end -->

## एक वास्तविक समय तालिका का नाम बदलना

<!-- example Renaming RT tables -->

आप RT मोड में एक वास्तविक समय की तालिका का नाम बदलने के लिए सकते हैं।
```sql
ALTER TABLE table_name RENAME new_table_name;
```

> NOTE: एक वास्तविक समय की तालिका का नाम बदलने के लिए [Manticore Buddy](../Installation/Manticore_Buddy.md) की आवश्यकता होती है। यदि यह काम नहीं करता है, तो सुनिश्चित करें कि Buddy स्थापित है।

<!-- request Example -->
```sql
ALTER TABLE table_name RENAME new_table_name;
```

<!-- response Example -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->

## सामान्य मोड में तालिका FT सेटिंग्स को अपडेट करना

<!-- example ALTER RECONFIGURE -->
```sql
ALTER TABLE table RECONFIGURE
```

`ALTER` सामान्य मोड में RT तालिका को भी फिर से कॉन्फ़िगर कर सकता है, ताकि नई टोकनाइज़ेशन, सांस्कृतिक और अन्य पाठ प्रक्रिया सेटिंग्स कॉन्फ़िगरेशन फ़ाइल से नए दस्तावेज़ों के लिए प्रभावी हो सकें। ध्यान दें कि मौजूदा दस्तावेज़ को सही सलामत रखा जाएगा। आंतरिक रूप से, यह वर्तमान RAM खंड को एक नए डिस्क खंड के रूप में जबरन सहेजता है और तालिका के शीर्षक को समायोजित करता है, ताकि नए दस्तावेज़ों को अद्यतन पूर्ण-पाठ सेटिंग्स का उपयोग करके टोकनाइज़ किया जा सके।

<!-- request Example -->
```sql
mysql> show table rt settings;
+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| settings      |       |
+---------------+-------+
1 row in set (0.00 sec)

mysql> alter table rt reconfigure;
Query OK, 0 rows affected (0.00 sec)

mysql> show table rt settings;
+---------------+----------------------+
| Variable_name | Value                |
+---------------+----------------------+
| settings      | morphology = stem_en |
+---------------+----------------------+
1 row in set (0.00 sec)
```
<!-- end -->

## एक द्वितीयक अनुक्रमणिका का पुनर्निर्माण करना

<!-- example ALTER REBUILD SECONDARY -->
```sql
ALTER TABLE table REBUILD SECONDARY
```

आप किसी दिए गए तालिका में द्वितीयक अनुक्रमणिका को पुनर्निर्माण करने के लिए `ALTER` का भी उपयोग कर सकते हैं। कभी-कभी, एक द्वितीयक अनुक्रमणिका को पूरी तालिका के लिए या तालिका के भीतर एक या अधिक गुणों के लिए निष्क्रिय किया जा सकता है:
* जब एक गुण अपडेट होता है, तो उसकी द्वितीयक अनुक्रमणिका निष्क्रिय हो जाती है।
* यदि Manticore एक पुरानी संस्करण की द्वितीयक अनुक्रमणिका में तालिका लोड करता है जो अब समर्थित नहीं है, तो द्वितीयक अनुक्रमणिका को पूरी तालिका के लिए निष्क्रिय कर दिया जाएगा।

`ALTER TABLE table REBUILD SECONDARY` गुण डेटा से द्वितीयक अनुक्रमणिका को पुनर्निर्माण करता है और उन्हें फिर से सक्षम करता है।

इसके अलावा, द्वितीयक अनुक्रमणिका का एक पुराना संस्करण समर्थित हो सकता है लेकिन इसमें कुछ सुविधाएँ नहीं होंगी। `REBUILD SECONDARY` का उपयोग द्वितीयक अनुक्रमणिका को अपडेट करने के लिए किया जा सकता है।

<!-- request Example -->
```sql
ALTER TABLE rt REBUILD SECONDARY;
```

<!-- response Example -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->

## एक वितरित तालिका को बदलना

<!-- example local_dist -->

एक वितरित तालिका में स्थानीय या दूरस्थ नोड्स की सूची को बदलने के लिए, उसी सिंटैक्स का पालन करें जिसका उपयोग आपने [तालिका बनाने](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table) के लिए किया था। बस `CREATE` को आदेश में `ALTER` से बदलें और `type='distributed'` को हटा दें:

```sql
ALTER TABLE `distr_table_name` [[local='local_table_name'], [agent='host:port:remote_table'] ... ]
```

> NOTE: ऑनलाइन वितरित तालिका की स्कीमा को बदलने के लिए [Manticore Buddy](../Installation/Manticore_Buddy.md) की आवश्यकता होती है। यदि यह काम नहीं करता है, तो सुनिश्चित करें कि Buddy स्थापित है।

<!-- request Example -->
```sql
ALTER TABLE local_dist local='index1' local='index2' agent='127.0.0.1:9312:remote_table';
```

<!-- end -->
<!-- proofread -->
