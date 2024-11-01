# 其他函数

### LAST_INSERT_ID()
返回当前会话中最后一条插入或替换语句插入的文档ID。

相同的值也可以通过 `@@session.last_insert_id` 变量获取。

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

返回当前连接的ID。

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

返回[KNN 向量搜索](../Searching/KNN.md)的距离。

```sql
mysql> select id, knn_dist() from test where knn ( image_vector, 5, (0.286569,-0.031816,0.066684,0.032926) ) and match('white') and id < 10;
+------+------------+
| id   | knn_dist() |
+------+------------+
|    2 | 0.81527930 |
+------+------------+
1 row in set (0.00 sec)
```

<!-- proofread -->
