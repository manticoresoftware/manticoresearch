# KILL

<!-- example KILL -->
```sql
KILL <query id>
```

`KILL` terminates execution of a query by its id which you can find in [SHOW QUERIES](../Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES).

<!-- request SQL -->
```sql
mysql> KILL 4;
Query OK, 1 row affected (0.00 sec)
```

<!-- end -->
