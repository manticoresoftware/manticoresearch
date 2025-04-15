# KILL

<!-- example KILL -->
```sql
KILL <query id>
```

`KILL` एक क्वेरी को उसके आईडी द्वारा समाप्त करता है, जिसे आप [SHOW QUERIES](../Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) में पा सकते हैं।

<!-- request SQL -->
```sql
mysql> KILL 4;
Query OK, 1 row affected (0.00 sec)
```

<!-- end -->
<!-- proofread -->
