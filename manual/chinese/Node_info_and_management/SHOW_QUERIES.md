# SHOW QUERIES

<!-- example SHOW QUERIES -->
```sql
SHOW QUERIES
```

> 注意：`SHOW QUERIES` 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果无法工作，请确保 Buddy 已安装。

`SHOW QUERIES` 返回所有当前正在运行的查询的信息。输出是一个结构如下的表：

- `id`：查询 ID，可用于 [KILL](../Node_info_and_management/KILL.md) 中终止查询
- `query`：查询语句或其部分内容
- `time`：命令执行所用时间或距查询执行的时间（此情况下，值将包含 `ago`）
- `protocol`： [连接协议](../Server_settings/Searchd.md#listen)，可能的值有 `sphinx`、`mysql`、`http`、`ssl`、`compressed`、`replication`，或组合形式（例如 `http,ssl` 或 `compressed,mysql`）
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
POST /sql?mode=raw -d "SHOW QUERIES;"
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

如果您想从线程本身的角度了解情况，请参考 [SHOW THREADS](../Node_info_and_management/SHOW_THREADS.md)。

<!-- proofread -->

