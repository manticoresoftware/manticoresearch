# 清除属性

```sql
FLUSH ATTRIBUTES
```

FLUSH ATTRIBUTES 命令将所有活跃磁盘表中内存中的属性更新刷新到磁盘。它返回一个标记，用于标识结果的磁盘状态（基本上是自服务器启动以来实际执行的磁盘属性保存次数）。

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

