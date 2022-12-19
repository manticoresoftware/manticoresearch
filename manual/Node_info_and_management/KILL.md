# KILL

```sql
KILL cid
```

It terminates the execution of the query statement with `connection_id`=`cid`.  One can see the current queries that can be killed by running the [SHOW QUERIES](../Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) statement.

```sql
mysql> KILL 4;
Query OK, 1 row affected (0.00 sec)
```
