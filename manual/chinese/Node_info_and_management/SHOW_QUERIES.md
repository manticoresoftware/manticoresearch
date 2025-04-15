# 显示查询

<!-- example SHOW QUERIES -->
```sql
SHOW QUERIES
```

> 注意: `SHOW QUERIES` 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不工作，请确保 Buddy 已安装。

`SHOW QUERIES` 返回有关当前运行的所有查询的信息。输出是一个具有以下结构的表：

- `id`: 查询 ID，可用于 [KILL](../Node_info_and_management/KILL.md) 终止查询
- `query`: 查询语句或其一部分
- `time`: 命令执行所需的时间或查询执行的时间（在这种情况下，值将包括 `ago`）
- `protocol`: [连接协议](../Server_settings/Searchd.md#listen)，可能的值包括 `sphinx`，`mysql`，`http`，`ssl`，`compressed`，`replication`，或组合（例如，`http,ssl` 或 `compressed,mysql`）
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
|  111 | select       | 5毫秒前 | http     | 127.0.0.1:58986 |
|   96 | SHOW QUERIES | 255微秒  | mysql    | 127.0.0.1:33616 |
+------+--------------+---------+----------+-----------------+
2 行在结果中 (0.61 秒)
```

<!-- end -->

请参阅 [SHOW THREADS](../Node_info_and_management/SHOW_THREADS.md)，如果您想从线程本身的角度获取见解。

<!-- proofread -->
