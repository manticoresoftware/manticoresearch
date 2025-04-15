# Other functions

### CONNECTION_ID()

Returns the current connection ID.

```sql
mysql> select CONNECTION_ID();
+-----------------+
| CONNECTION_ID() |
+-----------------+
| 6               |
+-----------------+
1 row in set (0.00 sec)
```

### KNN_DIST()

Returns [KNN vector search](../Searching/KNN.md) distance.

```sql
mysql> select id, knn_dist() from test where knn ( image_vector, 5, (0.286569,-0.031816,0.066684,0.032926) ) and match('white') and id < 10;
+------+------------+
| id   | knn_dist() |
+------+------------+
|    2 | 0.81527930 |
+------+------------+
1 row in set (0.00 sec)
```

### LAST_INSERT_ID()
Returns the IDs of documents that were inserted or replaced by the last statement in the current session.

The same value can also be obtained via the `@@session.last_insert_id` variable.

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

### UUID_SHORT()

Returns a "short" universal identifier as a 63-bit unsigned integer following the same algorithm as for [auto-id generation](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID).

> NOTE: Using `uuid_short()` will increase the counter used for auto-IDs, so use it carefully. While you can run `uuid_short()` in queries like `SELECT uuid_short() FROM ...`, this approach isn't ideal as it may create large gaps in your auto-generated IDs.

```sql
mysql> select uuid_short();
+---------------------+
| uuid_short()        |
+---------------------+
| 5839598169280741377 |
+---------------------+
1 row in set (0.00 sec)

mysql> select uuid_short();
+---------------------+
| uuid_short()        |
+---------------------+
| 5839598169280741378 |
+---------------------+
1 row in set (0.00 sec)
```

<!-- proofread -->
