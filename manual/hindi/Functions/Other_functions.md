# अन्य फ़ंक्शन्स

### CONNECTION_ID()

वर्तमान कनेक्शन आईडी लौटाता है।

```sql
mysql> select CONNECTION_ID();
+-----------------+
| CONNECTION_ID() |
+-----------------+
| 6               |
+-----------------+
1 row in set (0.00 sec)
```

### KNN_DIST()

[KNN वेक्टर खोज](../Searching/KNN.md) की दूरी लौटाता है।

```sql
mysql> select id, knn_dist() from test where knn ( image_vector, 5, (0.286569,-0.031816,0.066684,0.032926) ) and match('white') and id < 10;
+------+------------+
| id   | knn_dist() |
+------+------------+
|    2 | 0.81527930 |
+------+------------+
1 row in set (0.00 sec)
```

### LAST_INSERT_ID()
दस्तावेज़ों के आईडी लौटाता है जिन्हें वर्तमान सत्र में अंतिम बयान द्वारा डाला गया या प्रतिस्थापित किया गया था।

वही मान `@@session.last_insert_id` वेरिएबल के माध्यम से भी प्राप्त किया जा सकता है।

```sql
mysql> select @@session.last_insert_id;
+--------------------------+
| @@session.last_insert_id |
+--------------------------+
| 11,32                    |
+--------------------------+
1 rows in set

mysql> select LAST_INSERT_ID();
+------------------+
| LAST_INSERT_ID() |
+------------------+
| 25,26,29         |
+------------------+
1 rows in set   
```

### UUID_SHORT()

"संक्षिप्त" विश्व पहचानकर्ता को 63-बिट unsigned integer के रूप में लौटाता है जो [ऑटो-आईडी जनरेशन](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) के लिए समान एल्गोरिदम का अनुसरण करता है।

> NOTE: `uuid_short()` का उपयोग करने से ऑटो-आईडी के लिए उपयोग किए जाने वाले काउंटर में वृद्धि होगी, इसलिए इसका सावधानीपूर्वक उपयोग करें। जबकि आप ऐसे क्वेरी में `uuid_short()` चला सकते हैं जैसे `SELECT uuid_short() FROM ...`, यह दृष्टिकोण आदर्श नहीं है क्योंकि इससे आपके ऑटो-निर्मित आईडी में बड़े गैप हो सकते हैं।

```sql
mysql> select uuid_short();
+---------------------+
| uuid_short()        |
+---------------------+
| 5839598169280741377 |
+---------------------+
1 row in set (0.00 sec)

mysql> select uuid_short();
+---------------------+
| uuid_short()        |
+---------------------+
| 5839598169280741378 |
+---------------------+
1 row in set (0.00 sec)
```

<!-- proofread -->
