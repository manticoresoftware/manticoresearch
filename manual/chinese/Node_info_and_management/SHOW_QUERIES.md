# SHOW QUERIES

<!-- example SHOW QUERIES -->
```sql
SHOW QUERIES
```

> 注意：`SHOW QUERIES` 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果无法使用，请确保已安装 Buddy。

`SHOW QUERIES` 返回所有当前正在运行查询的信息。输出是一个具有以下结构的表格：

- `id`：查询 ID，可用于 [KILL](../Node_info_and_management/KILL.md) 终止查询
- `query`：查询语句或其部分内容
- `time`：命令执行所花时间或查询执行的时间（在这种情况下，值将包括 `ago`）
- `protocol`：[连接协议](../Server_settings/Searchd.md#listen)，可能的值有 `sphinx`、`mysql`、`http`、`ssl`、`compressed`、`replication`，或其组合（例如 `http,ssl` 或 `compressed,mysql`）
- `host`：客户端的 `ip:port`


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

如果您想从线程本身的角度获取信息，请参考 [SHOW THREADS](../Node_info_and_management/SHOW_THREADS.md)。

<!-- proofread -->

