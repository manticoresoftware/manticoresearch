# Other functions

### LAST_INSERT_ID()
Returns ids of documents inserted or replaced by last statement in the current session.

The same value can be also received via `@@session.last_insert_id` variable:

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

### SELECT @@system_variable


```sql
SELECT @@system_variable [LIMIT [offset,] row_count]
```

This is currently a placeholder query that does nothing and reports success. That is in order to keep compatibility with frameworks and connectors that automatically execute this statement.

However `@@session.last_insert_id` and `LAST_INSERT_ID()` report `ID` of documents these were inserted or replaced well at last statement.

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

### CONNECTION_ID()

Returns current connection id.

```sql
mysql> select CONNECTION_ID();
+-----------------+
| CONNECTION_ID() |
+-----------------+
| 6               |
+-----------------+
1 row in set (0.00 sec)
```
