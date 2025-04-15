# FLUSH ATTRIBUTES

```sql
FLUSH ATTRIBUTES
```

FLUSH ATTRIBUTES कमांड सभी सक्रिय डिस्क टेबल में सभी इन-मेमोरी विशेषता अपडेट को डिस्क पर फ्लश करता है। यह एक टैग लौटाता है जो डिस्क पर परिणाम की स्थिति की पहचान करता है (जो मूल रूप से सर्वर के स्टार्टअप के बाद किए गए वास्तविक डिस्क विशेषता सहेजने की संख्या है)।

```sql
mysql> UPDATE testindex SET channel_id=1107025 WHERE id=1;
Query OK, 1 row affected (0.04 sec)

mysql> FLUSH ATTRIBUTES;
+------+
| tag  |
+------+
|    1 |
+------+
1 row in set (0.19 sec)
```

<!-- proofread -->
