# 其他函数

### CONNECTION_ID()

返回当前连接ID。

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

返回[KNN向量搜索](../Searching/KNN.md)距离。

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
返回当前会话中最后一条语句插入或替换的文档ID。

同样的值也可以通过变量 `@@session.last_insert_id` 获得。

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

返回一个“短”通用标识符，作为63位无符号整数，遵循与[自动ID生成](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID)相同的算法。

> 注意：使用 `uuid_short()` 会增加用于自动ID的计数器，因此请谨慎使用。虽然你可以在查询中运行 `uuid_short()`，例如 `SELECT uuid_short() FROM ...`，但这种方法并不理想，因为它可能会在自动生成的ID中产生较大的间隙。

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

