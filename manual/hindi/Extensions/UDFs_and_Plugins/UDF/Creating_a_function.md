# फ़ंक्शन बनाएँ

```sql
CREATE FUNCTION udf_name
    RETURNS {INT | INTEGER | BIGINT | FLOAT | STRING}
    SONAME 'udf_lib_file'
```

`CREATE FUNCTION` स्टेटमेंट निर्दिष्ट नाम और प्रकार के साथ उपयोगकर्ता-परिभाषित फ़ंक्शन [UDF](../../../Extensions/UDFs_and_Plugins/UDF.md) स्थापित करता है जो प्रदान की गई पुस्तकालय फ़ाइल से है। पुस्तकालय फ़ाइल को एक विश्वसनीय [plugin_dir](../../../Server_settings/Common.md#plugin_dir) निर्देशिका में स्थित होना चाहिए। सफल स्थापना पर, फ़ंक्शन सभी बाद में प्राप्त क्वेरीज़ में उपयोग के लिए उपलब्ध हो जाता है। उदाहरण:

```sql
mysql> CREATE FUNCTION avgmva RETURNS INTEGER SONAME 'udfexample.dll';
Query OK, 0 rows affected (0.03 sec)

mysql> SELECT *, AVGMVA(tag) AS q from test1;
+------+--------+---------+-----------+
| id   | weight | tag     | q         |
+------+--------+---------+-----------+
|    1 |      1 | 1,3,5,7 | 4.000000  |
|    2 |      1 | 2,4,6   | 4.000000  |
|    3 |      1 | 15      | 15.000000 |
|    4 |      1 | 7,40    | 23.500000 |
+------+--------+---------+-----------+
```
<!-- proofread -->
