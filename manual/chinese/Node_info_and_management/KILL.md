# KILL

<!-- example KILL -->
```sql
KILL <query id>
```

`KILL` 通过其 ID 终止查询的执行，您可以在 [SHOW QUERIES](../Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) 中找到它。

<!-- request SQL -->
```sql
mysql> KILL 4;
Query OK, 1 row affected (0.00 sec)
```

<!-- end -->
<!-- proofread -->
