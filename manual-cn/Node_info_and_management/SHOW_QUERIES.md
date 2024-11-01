# SHOW QUERIES

<!-- example SHOW QUERIES -->
```sql
SHOW QUERIES
```

> 注意: `SHOW QUERIES` 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它无法工作，请确保已安装 Buddy。

`SHOW QUERIES` 返回所有当前正在执行的查询信息。输出的表格结构如下：

- **id**: 查询的 ID，可用于 [KILL](../Node_info_and_management/KILL.md) 命令来终止查询。
- **query**: 查询语句或其一部分。
- **time**: 执行命令所用的时间或查询执行的时间（该值可能包含 `ago`，表示过去的时间）。
- **protocol**: [连接协议](../Server_settings/Searchd.md#listen)，可能的值包括 `sphinx`, `mysql`, `http`, `ssl`, `compressed`, `replication`，或者它们的组合（例如，`http,ssl` 或 `compressed,mysql`）。
- **host**: 客户端的 `ip:port`。


<!-- request SQL -->
```sql
mysql> SHOW QUERIES;
```

<!-- response SQL -->
```
+------+--------------+---------+----------+-----------------+
| id   | query        | time    | protocol | host            |
+------+--------------+---------+----------+-----------------+
|  111 | select       | 5ms ago | http     | 127.0.0.1:58986 |
|   96 | SHOW QUERIES | 255us   | mysql    | 127.0.0.1:33616 |
+------+--------------+---------+----------+-----------------+
2 rows in set (0.61 sec)
```

<!-- end -->

果你希望从线程的角度查看更多信息，可以参考 [SHOW THREADS](../Node_info_and_management/SHOW_THREADS.md) 命令。

<!-- proofread -->
