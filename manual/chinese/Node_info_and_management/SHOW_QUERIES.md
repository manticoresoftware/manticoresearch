# 显示查询

<!-- example SHOW QUERIES -->
```sql
SHOW QUERIES
```

> 注意：`SHOW QUERIES` 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它无法运行，请确保已安装 Buddy。

`SHOW QUERIES` 返回所有当前正在运行的查询的信息。输出是一个具有以下结构的表：

- `id`：查询 ID，可用于 [KILL](../Node_info_and_management/KILL.md) 中终止查询
- `query`：查询语句或其部分内容
- `time`：命令执行所花费的时间或查询执行的时间（在这种情况下，值将包含 `ago`）
- `protocol`：[连接协议](../Server_settings/Searchd.md#listen)，可能的值有 `sphinx`、`mysql`、`http`、`ssl`、`compressed`、`replication`，或它们的组合（例如 `http,ssl` 或 `compressed,mysql`）
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

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SHOW QUERIES"
```

<!-- response JSON -->
```
[
  {
    "total": 2,
    "error": "",
    "warning": "",
    "columns": [
      {
        "id": {
          "type": "long long"
        }
      },
      {
        "query": {
          "type": "string"
        }
      },
      {
        "time": {
          "type": "string"
        }
      },
      {
        "protocol": {
          "type": "string"
        }
      },
      {
        "host": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "id": 111,
        "query": "select",
        "time": "5ms ago",
        "protocol": "http",
        "host": "127.0.0.1:58986"
      },
      {
        "id": 96,
        "query": "SHOW QUERIES",
        "time": "255us",
        "protocol": "mysql",
        "host": "127.0.0.1:33616"
      }
    ]
  }
]

```

<!-- end -->

如果你想从线程本身的角度获得洞见，请参考 [SHOW THREADS](../Node_info_and_management/SHOW_THREADS.md)。

<!-- proofread -->

