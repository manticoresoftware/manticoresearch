# SHOW VERSION

<!-- example SHOW VERSION -->
```sql
SHOW VERSION
```

> NOTE: `SHOW VERSION` requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

`SHOW VERSION` विभिन्न Manticore Search घटकों की विस्तृत संस्करण जानकारी प्रदान करता है। यह कमांड विशेष रूप से प्रशासनिक और विकासकर्ताओं के लिए उपयोगी है जिन्हें यह सत्यापित करने की आवश्यकता होती है कि वे किस संस्करण के Manticore Search का उपयोग कर रहे हैं, साथ ही इसके संबंधित घटकों के संस्करण भी।

आउटपुट तालिका में दो कॉलम होते हैं:
- `Component`: यह कॉलम Manticore Search के विशेष घटक का नाम देता है।
- `Version`: यह कॉलम संबंधित घटक के लिए संस्करण जानकारी प्रदर्शित करता है।

<!-- request SQL -->
```sql
mysql> SHOW VERSION;
```

<!-- response SQL -->
```
+-----------+--------------------------------+
| Component | Version                        |
+-----------+--------------------------------+
| Daemon    | 6.2.13 61cfe38d2@24011520 dev  |
| Columnar  | columnar 2.2.5 214ce90@240115  |
| Secondary | secondary 2.2.5 214ce90@240115 |
| KNN       | knn 2.2.5 214ce90@240115       |
| Buddy     | buddy v2.0.11                  |
+-----------+--------------------------------+
```

<!-- end -->

<!-- proofread -->
