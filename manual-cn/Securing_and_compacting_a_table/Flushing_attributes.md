# FLUSH ATTRIBUTES

```sql
FLUSH ATTRIBUTES
```

`FLUSH ATTRIBUTES` 命令将所有活动磁盘表中的内存属性更新刷新到磁盘。它返回一个标记，用于识别磁盘上的结果状态（基本上是自服务器启动以来执行的实际磁盘属性保存的数量）。

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