# SHOW QUERIES

<!-- example SHOW QUERIES -->
```sql
SHOW QUERIES
```

> NOTE: `SHOW QUERIES` requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

`SHOW QUERIES` सभी वर्तमान में चल रहे क्वेरीज के बारे में जानकारी लौटाता है। आउटपुट एक तालिका है जिसमें निम्नलिखित संरचना है:

- `id`: क्वेरी ID जिसका उपयोग [KILL](../Node_info_and_management/KILL.md) में क्वेरी समाप्त करने के लिए किया जा सकता है
- `query`: क्वेरी कथन या इसका एक हिस्सा
- `time`: कमांड निष्पादन पर ली गई समय या क्वेरी कब की गई थी (इस मामले में, मान में `ago` शामिल होगा)
- `protocol`: [कनेक्शन प्रोटोकॉल](../Server_settings/Searchd.md#listen), संभावित मान `sphinx`, `mysql`, `http`, `ssl`, `compressed`, `replication`, या एक संयोजन (जैसे, `http,ssl` या `compressed,mysql`)
- `host`: क्लाइंट का `ip:port`


<!-- request SQL -->
```sql
mysql> SHOW QUERIES;
```

<!-- response SQL -->
```
+------+--------------+---------+----------+-----------------+
| id   | query        | time    | protocol | host            |
+------+--------------+---------+----------+-----------------+
|  111 | select       | 5ms ago | http     | 127.0.0.1:58986 |
|   96 | SHOW QUERIES | 255us   | mysql    | 127.0.0.1:33616 |
+------+--------------+---------+----------+-----------------+
2 rows in set (0.61 sec)
```

<!-- end -->

यदि आप स्वयं थ्रेड के दृष्टिकोण से अंतर्दृष्टि प्राप्त करना चाहते हैं, तो [SHOW THREADS](../Node_info_and_management/SHOW_THREADS.md) को देखें।

<!-- proofread -->
