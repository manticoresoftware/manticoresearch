# FLUSH ATTRIBUTES

```sql
FLUSH ATTRIBUTES
```

The FLUSH ATTRIBUTES command flushes all in-memory attribute updates in all the active disk tables to disk. It returns a tag that identifies the result on-disk state (which is basically a number of actual disk attribute saves performed since the server startup).

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