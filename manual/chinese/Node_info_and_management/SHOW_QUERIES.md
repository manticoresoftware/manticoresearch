# 显示查询

<!-- example SHOW QUERIES -->
```sql
SHOW QUERIES
```

> 注意: `SHOW QUERIES` 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果不起作用，请确保已安装 Buddy。

`SHOW QUERIES` 返回所有当前运行查询的信息。输出是一个具有以下结构的表格：

- `id`: 可用于在 [KILL](../Node_info_and_management/KILL.md) 中终止查询的查询 ID
- `query`: 查询语句或其一部分
- `time`: 命令执行所花时间或查询执行多久之前（在这种情况下，值将包含 `ago`）
- `protocol`: [连接协议](../Server_settings/Searchd.md#listen)，可能的值为 `sphinx`、`mysql`、`http`、`ssl`、`compressed`、`replication` 或它们的组合（例如 `http,ssl` 或 `compressed,mysql`）
- `host`: 客户端的 `ip:port`


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

如需从线程本身的视角获得见解，请参阅 [SHOW THREADS](../Node_info_and_management/SHOW_THREADS.md)。

<!-- proofread -->

