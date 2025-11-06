# 节点状态

## STATUS

<!-- example status -->
查看 Manticore 节点高层信息的最简单方法是在 MySQL 客户端中运行 `status`。它会显示各个方面的信息，例如：
* 当前版本
* 是否启用 SSL
* 当前 TCP 端口/Unix 套接字
* 运行时间
* [线程](../Server_settings/Searchd.md#threads) 数量
* [队列中的作业](../Server_settings/Searchd.md#jobs_queue_size) 数量
* 连接数（`clients`）
* 当前正在处理的任务数
* 启动以来执行的查询数
* 队列中的作业数和任务数，按线程数归一化

<!-- request SQL -->
```mysql
mysql> status
```

<!-- response SQL -->
```mysql
--------------
mysql  Ver 14.14 Distrib 5.7.30, for Linux (x86_64) using  EditLine wrapper

Connection id:		378
Current database:	Manticore
Current user:		Usual
SSL:			Not in use
Current pager:		stdout
Using outfile:		''
Using delimiter:	;
Server version:		3.4.3 a48c61d6@200702 coroutines git branch coroutines_work_junk...origin/coroutines_work_junk
Protocol version:	10
Connection:		0 via TCP/IP
Server characterset:
Db     characterset:
Client characterset:	utf8
Conn.  characterset:	utf8
TCP port:		8306
Uptime:			23 hours 6 sec

Threads: 12  Queue: 3  Clients: 1  Vip clients: 0  Tasks: 5  Queries: 318967  Wall: 7h  CPU: 0us
Queue/Th: 0.2  Tasks/Th: 0.4
--------------
```


<!-- end-->

## SHOW STATUS

```sql
SHOW STATUS [ LIKE pattern ]
```

<!-- example show status -->

`SHOW STATUS` 是一条 SQL 语句，展示各种有用的性能计数器。IO 和 CPU 计数器只有在 `searchd` 启动时使用 `--iostats` 和 `--cpustats` 开关（或通过 `SET GLOBAL iostats/cpustats=1` 启用）时才可用。

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SHOW STATUS;
```

<!-- response SQL -->
```sql
+-------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------+
| Counter                       | Value                                                                                                                                          |
+-------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------+
| uptime                        | 182                                                                                                                                            |
| connections                   | 368                                                                                                                                            |
| maxed_out                     | 0                                                                                                                                              |
| version                       | 6.3.7 b10359434@24100213 dev (columnar 2.3.1 bd59d08@24093015) (secondary 2.3.1 bd59d08@24093015) (knn 2.3.1 bd59d08@24093015) (buddy v2.3.13) |
| mysql_version                 | 6.3.7 b10359434@24100213 dev (columnar 2.3.1 bd59d08@24093015) (secondary 2.3.1 bd59d08@24093015) (knn 2.3.1 bd59d08@24093015)                 |
| command_search                | 4                                                                                                                                              |
| command_excerpt               | 0                                                                                                                                              |
| command_update                | 2                                                                                                                                              |
| command_keywords              | 0                                                                                                                                              |
| command_persist               | 0                                                                                                                                              |
| command_status                | 8                                                                                                                                              |
| command_flushattrs            | 0                                                                                                                                              |
| command_sphinxql              | 0                                                                                                                                              |
| command_ping                  | 0                                                                                                                                              |
| command_delete                | 3                                                                                                                                              |
| command_set                   | 0                                                                                                                                              |
| command_insert                | 3                                                                                                                                              |
| command_replace               | 3                                                                                                                                              |
| command_commit                | 0                                                                                                                                              |
| command_suggest               | 0                                                                                                                                              |
| command_json                  | 0                                                                                                                                              |
| command_callpq                | 0                                                                                                                                              |
| command_cluster               | 0                                                                                                                                              |
| command_getfield              | 0                                                                                                                                              |
| insert_replace_stats_ms_avg   | 42.625 35.895 35.895                                                                                                                           |
| insert_replace_stats_ms_min   | 0.096 0.096 0.096                                                                                                                              |
| insert_replace_stats_ms_max   | 85.154 85.154 85.154                                                                                                                           |
| insert_replace_stats_ms_pct95 | 42.625 42.625 42.625                                                                                                                           |
| insert_replace_stats_ms_pct99 | 42.625 42.625 42.625                                                                                                                           |
| search_stats_ms_avg           | 0.209 0.188 0.188                                                                                                                              |
| search_stats_ms_min           | 0.205 0.058 0.058                                                                                                                              |
| search_stats_ms_max           | 0.214 0.278 0.278                                                                                                                              |
| search_stats_ms_pct95         | 0.214 0.278 0.278                                                                                                                              |
| search_stats_ms_pct99         | 0.214 0.278 0.278                                                                                                                              |
| update_stats_ms_avg           | 0.024 0.024 0.024                                                                                                                              |
| update_stats_ms_min           | 0.007 0.007 0.007                                                                                                                              |
| update_stats_ms_max           | 0.042 0.042 0.042                                                                                                                              |
| update_stats_ms_pct95         | 0.042 0.042 0.042                                                                                                                              |
| update_stats_ms_pct99         | 0.042 0.042 0.042                                                                                                                              |
| agent_connect                 | 0                                                                                                                                              |
| agent_tfo                     | 0                                                                                                                                              |
| agent_retry                   | 0                                                                                                                                              |
| queries                       | 6                                                                                                                                              |
| dist_queries                  | 0                                                                                                                                              |
| workers_total                 | 32                                                                                                                                             |
| workers_active                | 3                                                                                                                                              |
| workers_clients               | 1                                                                                                                                              |
| workers_clients_vip           | 0                                                                                                                                              |
| workers_clients_buddy         | 1                                                                                                                                              |
| work_queue_length             | 6                                                                                                                                              |
| load                          | 0.03 0.03 0.03                                                                                                                                 |
| load_primary                  | 0.00 0.00 0.00                                                                                                                                 |
| load_secondary                | 0.00 0.00 0.00                                                                                                                                 |
| query_wall                    | 0.000                                                                                                                                          |
| query_cpu                     | OFF                                                                                                                                            |
| dist_wall                     | 0.000                                                                                                                                          |
| dist_local                    | 0.000                                                                                                                                          |
| dist_wait                     | 0.000                                                                                                                                          |
| query_reads                   | OFF                                                                                                                                            |
| query_readkb                  | OFF                                                                                                                                            |
| query_readtime                | OFF                                                                                                                                            |
| avg_query_wall                | 0.000                                                                                                                                          |
| avg_query_cpu                 | OFF                                                                                                                                            |
| avg_dist_wall                 | 0.000                                                                                                                                          |
| avg_dist_local                | 0.000                                                                                                                                          |
| avg_dist_wait                 | 0.000                                                                                                                                          |
| avg_query_reads               | OFF                                                                                                                                            |
| avg_query_readkb              | OFF                                                                                                                                            |
| avg_query_readtime            | OFF                                                                                                                                            |
| qcache_max_bytes              | 16777216                                                                                                                                       |
| qcache_thresh_msec            | 3000                                                                                                                                           |
| qcache_ttl_sec                | 60                                                                                                                                             |
| qcache_cached_queries         | 0                                                                                                                                              |
| qcache_used_bytes             | 0                                                                                                                                              |
| qcache_hits                   | 0                                                                                                                                              |
+-------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------+
```

<!-- intro -->
##### JSON:
<!-- request JSON -->

```JSON
SHOW STATUS;
```

<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "Counter": {
          "type": "string"
        }
      },
      {
        "Value": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "Counter": "uptime",
        "Value": "107191"
      },
      {
        "Counter": "connections",
        "Value": "214577"
      },
      {
        "Counter": "maxed_out",
        "Value": "0"
      },
      {
        "Counter": "version",
        "Value": "13.13.4 0bc5a9641@25101507 dev (columnar 8.1.0 e1522a2@25100213) (secondary 8.1.0 e1522a2@25100213) (knn 8.1.0 e1522a2@25100213) (embeddings 1.0.1) (buddy v3.35.1+25090418-41d9811f-dev)"
      },
      {
        "Counter": "mysql_version",
        "Value": "13.13.4 0bc5a9641@25101507 dev (columnar 8.1.0 e1522a2@25100213) (secondary 8.1.0 e1522a2@25100213) (knn 8.1.0 e1522a2@25100213) (embeddings 1.0.1)"
      },
      {
        "Counter": "command_search",
        "Value": "47"
      },
      {
        "Counter": "command_excerpt",
        "Value": "3"
      },
      {
        "Counter": "command_update",
        "Value": "0"
      },
      {
        "Counter": "command_keywords",
        "Value": "0"
      },
      {
        "Counter": "command_persist",
        "Value": "0"
      },
      {
        "Counter": "command_status",
        "Value": "361"
      },
      {
        "Counter": "command_flushattrs",
        "Value": "0"
      },
      {
        "Counter": "command_sphinxql",
        "Value": "0"
      },
      {
        "Counter": "command_ping",
        "Value": "0"
      },
      {
        "Counter": "command_delete",
        "Value": "1"
      },
      {
        "Counter": "command_set",
        "Value": "0"
      },
      {
        "Counter": "command_insert",
        "Value": "13"
      },
      {
        "Counter": "command_replace",
        "Value": "0"
      },
      {
        "Counter": "command_commit",
        "Value": "0"
      },
      {
        "Counter": "command_suggest",
        "Value": "0"
      },
      {
        "Counter": "command_json",
        "Value": "0"
      },
      {
        "Counter": "command_callpq",
        "Value": "8"
      },
      {
        "Counter": "command_cluster",
        "Value": "0"
      },
      {
        "Counter": "command_getfield",
        "Value": "0"
      },
      {
        "Counter": "insert_replace_stats_ms_avg",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "insert_replace_stats_ms_min",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "insert_replace_stats_ms_max",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "insert_replace_stats_ms_pct95",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "insert_replace_stats_ms_pct99",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "search_stats_ms_avg",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "search_stats_ms_min",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "search_stats_ms_max",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "search_stats_ms_pct95",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "search_stats_ms_pct99",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "update_stats_ms_avg",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "update_stats_ms_min",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "update_stats_ms_max",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "update_stats_ms_pct95",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "update_stats_ms_pct99",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "agent_connect",
        "Value": "0"
      },
      {
        "Counter": "agent_tfo",
        "Value": "0"
      },
      {
        "Counter": "agent_retry",
        "Value": "0"
      },
      {
        "Counter": "queries",
        "Value": "44"
      },
      {
        "Counter": "dist_queries",
        "Value": "0"
      },
      {
        "Counter": "workers_total",
        "Value": "8"
      },
      {
        "Counter": "workers_active",
        "Value": "3"
      },
      {
        "Counter": "workers_clients",
        "Value": "1"
      },
      {
        "Counter": "workers_clients_vip",
        "Value": "0"
      },
      {
        "Counter": "workers_clients_buddy",
        "Value": "1"
      },
      {
        "Counter": "work_queue_length",
        "Value": "8"
      },
      {
        "Counter": "load",
        "Value": "0.00 0.00 0.00"
      },
      {
        "Counter": "load_primary",
        "Value": "0.00 0.00 0.00"
      },
      {
        "Counter": "load_secondary",
        "Value": "0.00 0.00 0.00"
      },
      {
        "Counter": "query_wall",
        "Value": "0.007"
      },
      {
        "Counter": "query_cpu",
        "Value": "OFF"
      },
      {
        "Counter": "dist_wall",
        "Value": "0.000"
      },
      {
        "Counter": "dist_local",
        "Value": "0.000"
      },
      {
        "Counter": "dist_wait",
        "Value": "0.000"
      },
      {
        "Counter": "query_reads",
        "Value": "OFF"
      },
      {
        "Counter": "query_readkb",
        "Value": "OFF"
      },
      {
        "Counter": "query_readtime",
        "Value": "OFF"
      },
      {
        "Counter": "avg_query_wall",
        "Value": "0.000"
      },
      {
        "Counter": "avg_query_cpu",
        "Value": "OFF"
      },
      {
        "Counter": "avg_dist_wall",
        "Value": "0.000"
      },
      {
        "Counter": "avg_dist_local",
        "Value": "0.000"
      },
      {
        "Counter": "avg_dist_wait",
        "Value": "0.000"
      },
      {
        "Counter": "avg_query_reads",
        "Value": "OFF"
      },
      {
        "Counter": "avg_query_readkb",
        "Value": "OFF"
      },
      {
        "Counter": "avg_query_readtime",
        "Value": "OFF"
      },
      {
        "Counter": "qcache_max_bytes",
        "Value": "16777216"
      },
      {
        "Counter": "qcache_thresh_msec",
        "Value": "3000"
      },
      {
        "Counter": "qcache_ttl_sec",
        "Value": "60"
      },
      {
        "Counter": "qcache_cached_queries",
        "Value": "0"
      },
      {
        "Counter": "qcache_used_bytes",
        "Value": "0"
      },
      {
        "Counter": "qcache_hits",
        "Value": "0"
      }
    ],
    "total": 75,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

<!-- example show status like -->

支持可选的 `LIKE` 子句，允许你只选择匹配特定模式的变量。模式语法遵循标准 SQL 通配符，其中 `%` 表示任意数量的任意字符，`_` 表示单个字符。

<!-- request qcache -->

```sql
SHOW STATUS LIKE 'qcache%';
```

<!-- response qcache -->
```sql
+-----------------------+-------+
| Counter               | Value |
+-----------------------+-------+
| qcache_max_bytes      | 0     |
| qcache_thresh_msec    | 3000  |
| qcache_ttl_sec        | 60    |
| qcache_cached_queries | 0     |
| qcache_used_bytes     | 0     |
| qcache_hits           | 0     |
+-----------------------+-------+
```

<!-- request qcache(JSON) -->

```JSON
POST /sql?mode=raw -d "SHOW STATUS LIKE 'qcache%';"
```

<!-- response qcache(JSON) -->
```JSON
[
  {
    "columns": [
      {
        "Counter": {
          "type": "string"
        }
      },
      {
        "Value": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "Counter": "qcache_max_bytes",
        "Value": "16777216"
      },
      {
        "Counter": "qcache_thresh_msec",
        "Value": "3000"
      },
      {
        "Counter": "qcache_ttl_sec",
        "Value": "60"
      },
      {
        "Counter": "qcache_cached_queries",
        "Value": "0"
      },
      {
        "Counter": "qcache_used_bytes",
        "Value": "0"
      },
      {
        "Counter": "qcache_hits",
        "Value": "0"
      }
    ],
    "total": 6,
    "error": "",
    "warning": ""
  }
]
```

<!-- request perf_stats -->

```sql
SHOW STATUS LIKE '%stats_ms%';
```

<!-- response perf_stats -->
```sql
+-------------------------------+-------------------+
| Counter                       | Value             |
+-------------------------------+-------------------+
| insert_replace_stats_ms_avg   | N/A 35.895 35.895 |
| insert_replace_stats_ms_min   | N/A 0.096 0.096   |
| insert_replace_stats_ms_max   | N/A 85.154 85.154 |
| insert_replace_stats_ms_pct95 | N/A 42.625 42.625 |
| insert_replace_stats_ms_pct99 | N/A 42.625 42.625 |
| search_stats_ms_avg           | N/A 0.188 0.188   |
| search_stats_ms_min           | N/A 0.058 0.058   |
| search_stats_ms_max           | N/A 0.278 0.278   |
| search_stats_ms_pct95         | N/A 0.278 0.278   |
| search_stats_ms_pct99         | N/A 0.278 0.278   |
| update_stats_ms_avg           | N/A 0.024 0.024   |
| update_stats_ms_min           | N/A 0.007 0.007   |
| update_stats_ms_max           | N/A 0.042 0.042   |
| update_stats_ms_pct95         | N/A 0.042 0.042   |
| update_stats_ms_pct99         | N/A 0.042 0.042   |
+-------------------------------+-------------------+
```

<!-- request perf_stats(JSON) -->

```JSON
POST /sql?mode=raw -d "SHOW STATUS LIKE '%stats_ms%';"
```

<!-- response perf_stats(JSON) -->
```JSON
[
  {
    "columns": [
      {
        "Counter": {
          "type": "string"
        }
      },
      {
        "Value": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "Counter": "insert_replace_stats_ms_avg",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "insert_replace_stats_ms_min",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "insert_replace_stats_ms_max",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "insert_replace_stats_ms_pct95",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "insert_replace_stats_ms_pct99",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "search_stats_ms_avg",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "search_stats_ms_min",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "search_stats_ms_max",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "search_stats_ms_pct95",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "search_stats_ms_pct99",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "update_stats_ms_avg",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "update_stats_ms_min",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "update_stats_ms_max",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "update_stats_ms_pct95",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "update_stats_ms_pct99",
        "Value": "N/A N/A N/A"
      }
    ],
    "total": 15,
    "error": "",
    "warning": ""
  }
]
```
<!-- end -->

### 查询时间统计

<!-- example show status like stats_ms -->

`SHOW STATUS` 命令提供 Manticore 各项性能指标的详细报告，包括插入/替换、搜索和更新查询的查询时间统计。这些统计基于 1、5 和 15 分钟的滑动窗口，显示查询时间的平均值、最小值、最大值以及 95%/99% 百分位值。

这些指标帮助跟踪特定时间间隔内的性能，便于发现查询响应时间趋势和潜在瓶颈。

以下指标是 `SHOW STATUS` 输出的一部分：
- `*_avg`：每种查询类型在最近 1、5 和 15 分钟内的平均查询时间。
- `*_min`：每种查询类型记录的最短查询时间。
- `*_max`：每种查询类型记录的最长查询时间。
- `*_pct95`：95% 查询完成所需的时间。
- `*_pct99`：99% 查询完成所需的时间。

这些统计分别针对插入/替换 (`insert_replace_stats_*`)、搜索 (`search_stats_*`) 和更新 (`update_stats_*`) 查询提供，详细洞察不同操作的性能。

如果监控期间无查询执行，系统将显示 `N/A`。

<!-- request perf_stats -->

```sql
SHOW STATUS LIKE '%stats_ms%';
```

<!-- response perf_stats -->
```sql
+-------------------------------+-------------------+
| Counter                       | Value             |
+-------------------------------+-------------------+
| insert_replace_stats_ms_avg   | N/A 35.895 35.895 |
| insert_replace_stats_ms_min   | N/A 0.096 0.096   |
| insert_replace_stats_ms_max   | N/A 85.154 85.154 |
| insert_replace_stats_ms_pct95 | N/A 42.625 42.625 |
| insert_replace_stats_ms_pct99 | N/A 42.625 42.625 |
| search_stats_ms_avg           | N/A 0.188 0.188   |
| search_stats_ms_min           | N/A 0.058 0.058   |
| search_stats_ms_max           | N/A 0.278 0.278   |
| search_stats_ms_pct95         | N/A 0.278 0.278   |
| search_stats_ms_pct99         | N/A 0.278 0.278   |
| update_stats_ms_avg           | N/A 0.024 0.024   |
| update_stats_ms_min           | N/A 0.007 0.007   |
| update_stats_ms_max           | N/A 0.042 0.042   |
| update_stats_ms_pct95         | N/A 0.042 0.042   |
| update_stats_ms_pct99         | N/A 0.042 0.042   |
+-------------------------------+-------------------+
```

<!-- request perf_stats(JSON) -->

```JSON
POST /sql?mode=raw -d "SHOW STATUS LIKE '%stats_ms%';"
```

<!-- response perf_stats(JSON) -->
```JSON
[
  {
    "columns": [
      {
        "Counter": {
          "type": "string"
        }
      },
      {
        "Value": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "Counter": "insert_replace_stats_ms_avg",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "insert_replace_stats_ms_min",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "insert_replace_stats_ms_max",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "insert_replace_stats_ms_pct95",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "insert_replace_stats_ms_pct99",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "search_stats_ms_avg",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "search_stats_ms_min",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "search_stats_ms_max",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "search_stats_ms_pct95",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "search_stats_ms_pct99",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "update_stats_ms_avg",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "update_stats_ms_min",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "update_stats_ms_max",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "update_stats_ms_pct95",
        "Value": "N/A N/A N/A"
      },
      {
        "Counter": "update_stats_ms_pct99",
        "Value": "N/A N/A N/A"
      }
    ],
    "total": 15,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

## SHOW SETTINGS

<!-- example show settings -->

`SHOW SETTINGS` 是一条 SQL 语句，用于显示配置文件中的当前设置。设置名称格式为：`'config_section_name'.'setting_name'`

结果还包括两个额外值：
- `configuration_file` - 配置文件路径
- `worker_pid` - 正在运行的 `searchd` 实例的进程 ID

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SHOW SETTINGS;
```

<!-- response SQL -->
```sql
+--------------------------+-------------------------------------+
| Setting_name             | Value                               |
+--------------------------+-------------------------------------+
| configuration_file       | /etc/manticoresearch/manticore.conf |
| worker_pid               | 658                                 |
| searchd.listen           | 0.0.0:9312                          |
| searchd.listen           | 0.0.0:9306:mysql                    |
| searchd.listen           | 0.0.0:9308:http                     |
| searchd.log              | /var/log/manticore/searchd.log      |
| searchd.query_log        | /var/log/manticore/query.log        |
| searchd.pid_file         | /run/manticore/searchd.pid          |
| searchd.data_dir         | /var/lib/manticore                  |
| searchd.query_log_format | sphinxql                            |
| searchd.binlog_path      | /var/lib/manticore/binlog           |
| common.plugin_dir        | /usr/local/lib/manticore            |
| common.lemmatizer_base   | /usr/share/manticore/morph/         |
+--------------------------+-------------------------------------+
13 rows in set (0.00 sec)
```

<!-- intro -->
##### JSON:
<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "SHOW SETTINGS;"
```

<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "Setting_name": {
          "type": "string"
        }
      },
      {
        "Value": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "Setting_name": "configuration_file",
        "Value": "/etc/manticoresearch/manticore.conf.sh"
      },
      {
        "Setting_name": "worker_pid",
        "Value": 1
      },
      {
        "Setting_name": "searchd.listen",
        "Value": "9306:mysql41"
      },
      {
        "Setting_name": "searchd.listen",
        "Value": "/var/run/mysqld/mysqld.sock:mysql41"
      },
      {
        "Setting_name": "searchd.listen",
        "Value": "9308:http"
      },
      {
        "Setting_name": "searchd.listen",
        "Value": "172.17.0.2:9312"
      },
      {
        "Setting_name": "searchd.listen",
        "Value": "172.17.0.2:9315-9325:replication"
      },
      {
        "Setting_name": "searchd.pid_file",
        "Value": "/run/manticore/searchd.pid"
      },
      {
        "Setting_name": "searchd.data_dir",
        "Value": "/var/lib/manticore"
      },
      {
        "Setting_name": "searchd.binlog_path",
        "Value": "/var/lib/manticore/binlog"
      },
      {
        "Setting_name": "common.plugin_dir",
        "Value": "/usr/local/lib/manticore"
      }
    ],
    "total": 11,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

## SHOW AGENT STATUS
```sql
SHOW AGENT ['agent_or_index'] STATUS [ LIKE pattern ]
```

<!-- example SHOW AGENT STATUS -->

`SHOW AGENT STATUS` 显示[远程代理](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)或分布式表的统计信息。它包括最后请求时间、最后响应时间、各种错误与成功次数等值。对于每个代理，列出了最近 1、5 和 15 个时间段的统计，每个时间段由 [ha_period_karma](../Server_settings/Searchd.md#ha_period_karma) 秒组成。

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SHOW AGENT STATUS;
```

<!-- response SQL -->
```sql
+------------------------------------+----------------------------+
| Variable_name                      | Value                      |
+------------------------------------+----------------------------+
| status_period_seconds              | 60                         |
| status_stored_periods              | 15                         |
| ag_0_hostname                      | 192.168.0.202:6713         |
| ag_0_references                    | 2                          |
| ag_0_lastquery                     | 0.41                       |
| ag_0_lastanswer                    | 0.19                       |
| ag_0_lastperiodmsec                | 222                        |
| ag_0_pingtripmsec                  | 10.521                     |
| ag_0_errorsarow                    | 0                          |
| ag_0_1periods_query_timeouts       | 0                          |
| ag_0_1periods_connect_timeouts     | 0                          |
| ag_0_1periods_connect_failures     | 0                          |
| ag_0_1periods_network_errors       | 0                          |
| ag_0_1periods_wrong_replies        | 0                          |
| ag_0_1periods_unexpected_closings  | 0                          |
| ag_0_1periods_warnings             | 0                          |
| ag_0_1periods_succeeded_queries    | 27                         |
| ag_0_1periods_msecsperquery        | 232.31                     |
| ag_0_5periods_query_timeouts       | 0                          |
| ag_0_5periods_connect_timeouts     | 0                          |
| ag_0_5periods_connect_failures     | 0                          |
| ag_0_5periods_network_errors       | 0                          |
| ag_0_5periods_wrong_replies        | 0                          |
| ag_0_5periods_unexpected_closings  | 0                          |
| ag_0_5periods_warnings             | 0                          |
| ag_0_5periods_succeeded_queries    | 146                        |
| ag_0_5periods_msecsperquery        | 231.83                     |
| ag_1_hostname                      | 192.168.0.202:6714         |
| ag_1_references                    | 2                          |
| ag_1_lastquery                     | 0.41                       |
| ag_1_lastanswer                    | 0.19                       |
| ag_1_lastperiodmsec                | 220                        |
| ag_1_pingtripmsec                  | 10.004                     |
| ag_1_errorsarow                    | 0                          |
| ag_1_1periods_query_timeouts       | 0                          |
| ag_1_1periods_connect_timeouts     | 0                          |
| ag_1_1periods_connect_failures     | 0                          |
| ag_1_1periods_network_errors       | 0                          |
| ag_1_1periods_wrong_replies        | 0                          |
| ag_1_1periods_unexpected_closings  | 0                          |
| ag_1_1periods_warnings             | 0                          |
| ag_1_1periods_succeeded_queries    | 27                         |
| ag_1_1periods_msecsperquery        | 231.24                     |
| ag_1_5periods_query_timeouts       | 0                          |
| ag_1_5periods_connect_timeouts     | 0                          |
| ag_1_5periods_connect_failures     | 0                          |
| ag_1_5periods_network_errors       | 0                          |
| ag_1_5periods_wrong_replies        | 0                          |
| ag_1_5periods_unexpected_closings  | 0                          |
| ag_1_5periods_warnings             | 0                          |
| ag_1_5periods_succeeded_queries    | 146                        |
| ag_1_5periods_msecsperquery        | 230.85                     |
+------------------------------------+----------------------------+
50 rows in set (0.01 sec)
```

<!-- intro -->
##### JSON:
<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "SHOW AGENT STATUS;"
```

<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "Key": {
          "type": "string"
        }
      },
      {
        "Value": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "Key": "status_period_seconds",
        "Value": "60"
      },
      {
        "Key": "status_stored_periods",
        "Value": "15"
      }
    ],
    "total": 2,
    "error": "",
    "warning": ""
  }
]
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$client->nodes()->agentstatus();
```

<!-- response PHP -->

```php
Array(
	[status_period_seconds] => 60
	[status_stored_periods] => 15
	[ag_0_hostname] => 192.168.0.202:6713
	[ag_0_references] => 2
	[ag_0_lastquery] => 0.41
	[ag_0_lastanswer] => 0.19
	[ag_0_lastperiodmsec] => 222
	[ag_0_errorsarow] => 0
	[ag_0_1periods_query_timeouts] => 0
	[ag_0_1periods_connect_timeouts] => 0
	[ag_0_1periods_connect_failures] => 0
	[ag_0_1periods_network_errors] => 0
	[ag_0_1periods_wrong_replies] => 0
	[ag_0_1periods_unexpected_closings] => 0
	[ag_0_1periods_warnings] => 0
	[ag_0_1periods_succeeded_queries] => 27
	[ag_0_1periods_msecsperquery] => 232.31
	[ag_0_5periods_query_timeouts] => 0
	[ag_0_5periods_connect_timeouts] => 0
	[ag_0_5periods_connect_failures] => 0
	[ag_0_5periods_network_errors] => 0
	[ag_0_5periods_wrong_replies] => 0
	[ag_0_5periods_unexpected_closings] => 0
	[ag_0_5periods_warnings] => 0
	[ag_0_5periods_succeeded_queries] => 146
	[ag_0_5periods_msecsperquery] => 231.83
	[ag_1_hostname 192.168.0.202:6714
	[ag_1_references] => 2
	[ag_1_lastquery] => 0.41
	[ag_1_lastanswer] => 0.19
	[ag_1_lastperiodmsec] => 220
	[ag_1_errorsarow] => 0
	[ag_1_1periods_query_timeouts] => 0
	[ag_1_1periods_connect_timeouts] => 0
	[ag_1_1periods_connect_failures] => 0
	[ag_1_1periods_network_errors] => 0
	[ag_1_1periods_wrong_replies] => 0
	[ag_1_1periods_unexpected_closings] => 0
	[ag_1_1periods_warnings] => 0
	[ag_1_1periods_succeeded_queries] => 27
	[ag_1_1periods_msecsperquery] => 231.24
	[ag_1_5periods_query_timeouts] => 0
	[ag_1_5periods_connect_timeouts] => 0
	[ag_1_5periods_connect_failures] => 0
	[ag_1_5periods_network_errors] => 0
	[ag_1_5periods_wrong_replies] => 0
	[ag_1_5periods_unexpected_closings
	[ag_1_5periods_warnings] => 0
	[ag_1_5periods_succeeded_queries] => 146
	[ag_1_5periods_msecsperquery] => 230.85
)
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('SHOW AGENT STATUS')
```
<!-- response Python -->

```python
{u'columns': [{u'Key': {u'type': u'string'}},
              {u'Value': {u'type': u'string'}}],
 u'data': [
	{u'Key': u'status_period_seconds', u'Value': u'60'},
	{u'Key': u'status_stored_periods', u'Value': u'15'},
	{u'Key': u'ag_0_hostname', u'Value': u'192.168.0.202:6713'},
	{u'Key': u'ag_0_references', u'Value': u'2'},
	{u'Key': u'ag_0_lastquery', u'Value': u'0.41'},
	{u'Key': u'ag_0_lastanswer', u'Value': u'0.19'},
	{u'Key': u'ag_0_lastperiodmsec', u'Value': u'222'},
	{u'Key': u'ag_0_errorsarow', u'Value': u'0'},
	{u'Key': u'ag_0_1periods_query_timeouts', u'Value': u'0'},
	{u'Key': u'ag_0_1periods_connect_timeouts', u'Value': u'0'},
	{u'Key': u'ag_0_1periods_connect_failures', u'Value': u'0'},
	{u'Key': u'ag_0_1periods_network_errors', u'Value': u'0'},
	{u'Key': u'ag_0_1periods_wrong_replies', u'Value': u'0'},
	{u'Key': u'ag_0_1periods_unexpected_closings', u'Value': u'0'},
	{u'Key': u'ag_0_1periods_warnings', u'Value': u'0'},
	{u'Key': u'ag_0_1periods_succeeded_queries', u'Value': u'27'},
	{u'Key': u'ag_0_1periods_msecsperquery', u'Value': u'232.31'},
	{u'Key': u'ag_0_5periods_query_timeouts', u'Value': u'0'},
	{u'Key': u'ag_0_5periods_connect_timeouts', u'Value': u'0'},
	{u'Key': u'ag_0_5periods_connect_failures', u'Value': u'0'},
	{u'Key': u'ag_0_5periods_network_errors', u'Value': u'0'},
	{u'Key': u'ag_0_5periods_wrong_replies', u'Value': u'0'},
	{u'Key': u'ag_0_5periods_unexpected_closings', u'Value': u'0'},
	{u'Key': u'ag_0_5periods_warnings', u'Value': u'0'},
	{u'Key': u'ag_0_5periods_succeeded_queries', u'Value': u'146'},
	{u'Key': u'ag_0_5periods_msecsperquery', u'Value': u'231.83'},
	{u'Key': u'ag_1_hostname 192.168.0.202:6714'},
	{u'Key': u'ag_1_references', u'Value': u'2'},
	{u'Key': u'ag_1_lastquery', u'Value': u'0.41'},
	{u'Key': u'ag_1_lastanswer', u'Value': u'0.19'},
	{u'Key': u'ag_1_lastperiodmsec', u'Value': u'220'},
	{u'Key': u'ag_1_errorsarow', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_query_timeouts', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_connect_timeouts', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_connect_failures', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_network_errors', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_wrong_replies', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_unexpected_closings', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_warnings', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_succeeded_queries', u'Value': u'27'},
	{u'Key': u'ag_1_1periods_msecsperquery', u'Value': u'231.24'},
	{u'Key': u'ag_1_5periods_query_timeouts', u'Value': u'0'},
	{u'Key': u'ag_1_5periods_connect_timeouts', u'Value': u'0'},
	{u'Key': u'ag_1_5periods_connect_failures', u'Value': u'0'},
	{u'Key': u'ag_1_5periods_network_errors', u'Value': u'0'},
	{u'Key': u'ag_1_5periods_wrong_replies', u'Value': u'0'},
	{u'Key': u'ag_1_5periods_warnings', u'Value': u'0'},
	{u'Key': u'ag_1_5periods_succeeded_queries', u'Value': u'146'},
	{u'Key': u'ag_1_5periods_msecsperquery', u'Value': u'230.85'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('SHOW AGENT STATUS')
```
<!-- response Python-asyncio -->

```python
{u'columns': [{u'Key': {u'type': u'string'}},
              {u'Value': {u'type': u'string'}}],
 u'data': [
	{u'Key': u'status_period_seconds', u'Value': u'60'},
	{u'Key': u'status_stored_periods', u'Value': u'15'},
	{u'Key': u'ag_0_hostname', u'Value': u'192.168.0.202:6713'},
	{u'Key': u'ag_0_references', u'Value': u'2'},
	{u'Key': u'ag_0_lastquery', u'Value': u'0.41'},
	{u'Key': u'ag_0_lastanswer', u'Value': u'0.19'},
	{u'Key': u'ag_0_lastperiodmsec', u'Value': u'222'},
	{u'Key': u'ag_0_errorsarow', u'Value': u'0'},
	{u'Key': u'ag_0_1periods_query_timeouts', u'Value': u'0'},
	{u'Key': u'ag_0_1periods_connect_timeouts', u'Value': u'0'},
	{u'Key': u'ag_0_1periods_connect_failures', u'Value': u'0'},
	{u'Key': u'ag_0_1periods_network_errors', u'Value': u'0'},
	{u'Key': u'ag_0_1periods_wrong_replies', u'Value': u'0'},
	{u'Key': u'ag_0_1periods_unexpected_closings', u'Value': u'0'},
	{u'Key': u'ag_0_1periods_warnings', u'Value': u'0'},
	{u'Key': u'ag_0_1periods_succeeded_queries', u'Value': u'27'},
	{u'Key': u'ag_0_1periods_msecsperquery', u'Value': u'232.31'},
	{u'Key': u'ag_0_5periods_query_timeouts', u'Value': u'0'},
	{u'Key': u'ag_0_5periods_connect_timeouts', u'Value': u'0'},
	{u'Key': u'ag_0_5periods_connect_failures', u'Value': u'0'},
	{u'Key': u'ag_0_5periods_network_errors', u'Value': u'0'},
	{u'Key': u'ag_0_5periods_wrong_replies', u'Value': u'0'},
	{u'Key': u'ag_0_5periods_unexpected_closings', u'Value': u'0'},
	{u'Key': u'ag_0_5periods_warnings', u'Value': u'0'},
	{u'Key': u'ag_0_5periods_succeeded_queries', u'Value': u'146'},
	{u'Key': u'ag_0_5periods_msecsperquery', u'Value': u'231.83'},
	{u'Key': u'ag_1_hostname 192.168.0.202:6714'},
	{u'Key': u'ag_1_references', u'Value': u'2'},
	{u'Key': u'ag_1_lastquery', u'Value': u'0.41'},
	{u'Key': u'ag_1_lastanswer', u'Value': u'0.19'},
	{u'Key': u'ag_1_lastperiodmsec', u'Value': u'220'},
	{u'Key': u'ag_1_errorsarow', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_query_timeouts', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_connect_timeouts', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_connect_failures', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_network_errors', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_wrong_replies', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_unexpected_closings', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_warnings', u'Value': u'0'},
	{u'Key': u'ag_1_1periods_succeeded_queries', u'Value': u'27'},
	{u'Key': u'ag_1_1periods_msecsperquery', u'Value': u'231.24'},
	{u'Key': u'ag_1_5periods_query_timeouts', u'Value': u'0'},
	{u'Key': u'ag_1_5periods_connect_timeouts', u'Value': u'0'},
	{u'Key': u'ag_1_5periods_connect_failures', u'Value': u'0'},
	{u'Key': u'ag_1_5periods_network_errors', u'Value': u'0'},
	{u'Key': u'ag_1_5periods_wrong_replies', u'Value': u'0'},
	{u'Key': u'ag_1_5periods_warnings', u'Value': u'0'},
	{u'Key': u'ag_1_5periods_succeeded_queries', u'Value': u'146'},
	{u'Key': u'ag_1_5periods_msecsperquery', u'Value': u'230.85'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql("SHOW AGENT STATUS");
```
<!-- response javascript -->

```javascript
{"columns": [{"Key": {"type": "string"}},
              {"Value": {"type": "string"}}],
 "data": [
	{"Key": "status_period_seconds", "Value": "60"},
	{"Key": "status_stored_periods", "Value": "15"},
	{"Key": "ag_0_hostname", "Value": "192.168.0.202:6713"},
	{"Key": "ag_0_references", "Value": "2"},
	{"Key": "ag_0_lastquery", "Value": "0.41"},
	{"Key": "ag_0_lastanswer", "Value": "0.19"},
	{"Key": "ag_0_lastperiodmsec", "Value": "222"},
	{"Key": "ag_0_errorsarow", "Value": "0"},
	{"Key": "ag_0_1periods_query_timeouts", "Value": "0"},
	{"Key": "ag_0_1periods_connect_timeouts", "Value": "0"},
	{"Key": "ag_0_1periods_connect_failures", "Value": "0"},
	{"Key": "ag_0_1periods_network_errors", "Value": "0"},
	{"Key": "ag_0_1periods_wrong_replies", "Value": "0"},
	{"Key": "ag_0_1periods_unexpected_closings", "Value": "0"},
	{"Key": "ag_0_1periods_warnings", "Value": "0"},
	{"Key": "ag_0_1periods_succeeded_queries", "Value": "27"},
	{"Key": "ag_0_1periods_msecsperquery", "Value": "232.31"},
	{"Key": "ag_0_5periods_query_timeouts", "Value": "0"},
	{"Key": "ag_0_5periods_connect_timeouts", "Value": "0"},
	{"Key": "ag_0_5periods_connect_failures", "Value": "0"},
	{"Key": "ag_0_5periods_network_errors", "Value": "0"},
	{"Key": "ag_0_5periods_wrong_replies", "Value": "0"},
	{"Key": "ag_0_5periods_unexpected_closings", "Value": "0"},
	{"Key": "ag_0_5periods_warnings", "Value": "0"},
	{"Key": "ag_0_5periods_succeeded_queries", "Value": "146"},
	{"Key": "ag_0_5periods_msecsperquery", "Value": "231.83"},
	{"Key": "ag_1_hostname 192.168.0.202:6714"},
	{"Key": "ag_1_references", "Value": "2"},
	{"Key": "ag_1_lastquery", "Value": "0.41"},
	{"Key": "ag_1_lastanswer", "Value": "0.19"},
	{"Key": "ag_1_lastperiodmsec", "Value": "220"},
	{"Key": "ag_1_errorsarow", "Value": "0"},
	{"Key": "ag_1_1periods_query_timeouts", "Value": "0"},
	{"Key": "ag_1_1periods_connect_timeouts", "Value": "0"},
	{"Key": "ag_1_1periods_connect_failures", "Value": "0"},
	{"Key": "ag_1_1periods_network_errors", "Value": "0"},
	{"Key": "ag_1_1periods_wrong_replies", "Value": "0"},
	{"Key": "ag_1_1periods_unexpected_closings", "Value": "0"},
	{"Key": "ag_1_1periods_warnings", "Value": "0"},
	{"Key": "ag_1_1periods_succeeded_queries", "Value": "27"},
	{"Key": "ag_1_1periods_msecsperquery", "Value": "231.24"},
	{"Key": "ag_1_5periods_query_timeouts", "Value": "0"},
	{"Key": "ag_1_5periods_connect_timeouts", "Value": "0"},
	{"Key": "ag_1_5periods_connect_failures", "Value": "0"},
	{"Key": "ag_1_5periods_network_errors", "Value": "0"},
	{"Key": "ag_1_5periods_wrong_replies", "Value": "0"},
	{"Key": "ag_1_5periods_warnings", "Value": "0"},
	{"Key": "ag_1_5periods_succeeded_queries", "Value": "146"},
	{"Key": "ag_1_5periods_msecsperquery", "Value": "230.85"}],
 "error": "",
 "total": 0,
 "warning": ""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("SHOW AGENT STATUS");
```
<!-- response Java -->

```java
{columns=[{ Key : { type=string }},
              { Value : { type=string }}],
  data : [
	{ Key=status_period_seconds ,  Value=60 },
	{ Key=status_stored_periods ,  Value=15 },
	{ Key=ag_0_hostname ,  Value=192.168.0.202:6713 },
	{ Key=ag_0_references ,  Value=2 },
	{ Key=ag_0_lastquery ,  Value=0.41 },
	{ Key=ag_0_lastanswer ,  Value=0.19 },
	{ Key=ag_0_lastperiodmsec ,  Value=222 },
	{ Key=ag_0_errorsarow ,  Value=0 },
	{ Key=ag_0_1periods_query_timeouts ,  Value=0 },
	{ Key=ag_0_1periods_connect_timeouts ,  Value=0 },
	{ Key=ag_0_1periods_connect_failures ,  Value=0 },
	{ Key=ag_0_1periods_network_errors ,  Value=0 },
	{ Key=ag_0_1periods_wrong_replies ,  Value=0 },
	{ Key=ag_0_1periods_unexpected_closings ,  Value=0 },
	{ Key=ag_0_1periods_warnings ,  Value=0 },
	{ Key=ag_0_1periods_succeeded_queries ,  Value=27 },
	{ Key=ag_0_1periods_msecsperquery ,  Value=232.31 },
	{ Key=ag_0_5periods_query_timeouts ,  Value=0 },
	{ Key=ag_0_5periods_connect_timeouts ,  Value=0 },
	{ Key=ag_0_5periods_connect_failures ,  Value=0 },
	{ Key=ag_0_5periods_network_errors ,  Value=0 },
	{ Key=ag_0_5periods_wrong_replies ,  Value=0 },
	{ Key=ag_0_5periods_unexpected_closings ,  Value=0 },
	{ Key=ag_0_5periods_warnings ,  Value=0 },
	{ Key=ag_0_5periods_succeeded_queries ,  Value=146 },
	{ Key=ag_0_5periods_msecsperquery ,  Value=231.83 },
	{ Key=ag_1_hostname 192.168.0.202:6714 },
	{ Key=ag_1_references ,  Value=2 },
	{ Key=ag_1_lastquery ,  Value=0.41 },
	{ Key=ag_1_lastanswer ,  Value=0.19 },
	{ Key=ag_1_lastperiodmsec ,  Value=220 },
	{ Key=ag_1_errorsarow ,  Value=0 },
	{ Key=ag_1_1periods_query_timeouts ,  Value=0 },
	{ Key=ag_1_1periods_connect_timeouts ,  Value=0 },
	{ Key=ag_1_1periods_connect_failures ,  Value=0 },
	{ Key=ag_1_1periods_network_errors ,  Value=0 },
	{ Key=ag_1_1periods_wrong_replies ,  Value=0 },
	{ Key=ag_1_1periods_unexpected_closings ,  Value=0 },
	{ Key=ag_1_1periods_warnings ,  Value=0 },
	{ Key=ag_1_1periods_succeeded_queries ,  Value=27 },
	{ Key=ag_1_1periods_msecsperquery ,  Value=231.24 },
	{ Key=ag_1_5periods_query_timeouts ,  Value=0 },
	{ Key=ag_1_5periods_connect_timeouts ,  Value=0 },
	{ Key=ag_1_5periods_connect_failures ,  Value=0 },
	{ Key=ag_1_5periods_network_errors ,  Value=0 },
	{ Key=ag_1_5periods_wrong_replies ,  Value=0 },
	{ Key=ag_1_5periods_warnings ,  Value=0 },
	{ Key=ag_1_5periods_succeeded_queries ,  Value=146 },
	{ Key=ag_1_5periods_msecsperquery ,  Value=230.85 }],
  error= ,
  total=0,
  warning= }
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("SHOW AGENT STATUS");
```
<!-- response C# -->

```clike
{columns=[{ Key : { type=string }},
              { Value : { type=string }}],
  data : [
	{ Key=status_period_seconds ,  Value=60 },
	{ Key=status_stored_periods ,  Value=15 },
	{ Key=ag_0_hostname ,  Value=192.168.0.202:6713 },
	{ Key=ag_0_references ,  Value=2 },
	{ Key=ag_0_lastquery ,  Value=0.41 },
	{ Key=ag_0_lastanswer ,  Value=0.19 },
	{ Key=ag_0_lastperiodmsec ,  Value=222 },
	{ Key=ag_0_errorsarow ,  Value=0 },
	{ Key=ag_0_1periods_query_timeouts ,  Value=0 },
	{ Key=ag_0_1periods_connect_timeouts ,  Value=0 },
	{ Key=ag_0_1periods_connect_failures ,  Value=0 },
	{ Key=ag_0_1periods_network_errors ,  Value=0 },
	{ Key=ag_0_1periods_wrong_replies ,  Value=0 },
	{ Key=ag_0_1periods_unexpected_closings ,  Value=0 },
	{ Key=ag_0_1periods_warnings ,  Value=0 },
	{ Key=ag_0_1periods_succeeded_queries ,  Value=27 },
	{ Key=ag_0_1periods_msecsperquery ,  Value=232.31 },
	{ Key=ag_0_5periods_query_timeouts ,  Value=0 },
	{ Key=ag_0_5periods_connect_timeouts ,  Value=0 },
	{ Key=ag_0_5periods_connect_failures ,  Value=0 },
	{ Key=ag_0_5periods_network_errors ,  Value=0 },
	{ Key=ag_0_5periods_wrong_replies ,  Value=0 },
	{ Key=ag_0_5periods_unexpected_closings ,  Value=0 },
	{ Key=ag_0_5periods_warnings ,  Value=0 },
	{ Key=ag_0_5periods_succeeded_queries ,  Value=146 },
	{ Key=ag_0_5periods_msecsperquery ,  Value=231.83 },
	{ Key=ag_1_hostname 192.168.0.202:6714 },
	{ Key=ag_1_references ,  Value=2 },
	{ Key=ag_1_lastquery ,  Value=0.41 },
	{ Key=ag_1_lastanswer ,  Value=0.19 },
	{ Key=ag_1_lastperiodmsec ,  Value=220 },
	{ Key=ag_1_errorsarow ,  Value=0 },
	{ Key=ag_1_1periods_query_timeouts ,  Value=0 },
	{ Key=ag_1_1periods_connect_timeouts ,  Value=0 },
	{ Key=ag_1_1periods_connect_failures ,  Value=0 },
	{ Key=ag_1_1periods_network_errors ,  Value=0 },
	{ Key=ag_1_1periods_wrong_replies ,  Value=0 },
	{ Key=ag_1_1periods_unexpected_closings ,  Value=0 },
	{ Key=ag_1_1periods_warnings ,  Value=0 },
	{ Key=ag_1_1periods_succeeded_queries ,  Value=27 },
	{ Key=ag_1_1periods_msecsperquery ,  Value=231.24 },
	{ Key=ag_1_5periods_query_timeouts ,  Value=0 },
	{ Key=ag_1_5periods_connect_timeouts ,  Value=0 },
	{ Key=ag_1_5periods_connect_failures ,  Value=0 },
	{ Key=ag_1_5periods_network_errors ,  Value=0 },
	{ Key=ag_1_5periods_wrong_replies ,  Value=0 },
	{ Key=ag_1_5periods_warnings ,  Value=0 },
	{ Key=ag_1_5periods_succeeded_queries ,  Value=146 },
	{ Key=ag_1_5periods_msecsperquery ,  Value=230.85 }],
  error="" ,
  total=0,
  warning="" }
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```clike
utils_api.sql("SHOW AGENT STATUS", Some(true)).await;
```
<!-- response Rust -->

```rust
{columns=[{ Key : { type=string }},
              { Value : { type=string }}],
  data : [
	{ Key=status_period_seconds ,  Value=60 },
	{ Key=status_stored_periods ,  Value=15 },
	{ Key=ag_0_hostname ,  Value=192.168.0.202:6713 },
	{ Key=ag_0_references ,  Value=2 },
	{ Key=ag_0_lastquery ,  Value=0.41 },
	{ Key=ag_0_lastanswer ,  Value=0.19 },
	{ Key=ag_0_lastperiodmsec ,  Value=222 },
	{ Key=ag_0_errorsarow ,  Value=0 },
	{ Key=ag_0_1periods_query_timeouts ,  Value=0 },
	{ Key=ag_0_1periods_connect_timeouts ,  Value=0 },
	{ Key=ag_0_1periods_connect_failures ,  Value=0 },
	{ Key=ag_0_1periods_network_errors ,  Value=0 },
	{ Key=ag_0_1periods_wrong_replies ,  Value=0 },
	{ Key=ag_0_1periods_unexpected_closings ,  Value=0 },
	{ Key=ag_0_1periods_warnings ,  Value=0 },
	{ Key=ag_0_1periods_succeeded_queries ,  Value=27 },
	{ Key=ag_0_1periods_msecsperquery ,  Value=232.31 },
	{ Key=ag_0_5periods_query_timeouts ,  Value=0 },
	{ Key=ag_0_5periods_connect_timeouts ,  Value=0 },
	{ Key=ag_0_5periods_connect_failures ,  Value=0 },
	{ Key=ag_0_5periods_network_errors ,  Value=0 },
	{ Key=ag_0_5periods_wrong_replies ,  Value=0 },
	{ Key=ag_0_5periods_unexpected_closings ,  Value=0 },
	{ Key=ag_0_5periods_warnings ,  Value=0 },
	{ Key=ag_0_5periods_succeeded_queries ,  Value=146 },
	{ Key=ag_0_5periods_msecsperquery ,  Value=231.83 },
	{ Key=ag_1_hostname 192.168.0.202:6714 },
	{ Key=ag_1_references ,  Value=2 },
	{ Key=ag_1_lastquery ,  Value=0.41 },
	{ Key=ag_1_lastanswer ,  Value=0.19 },
	{ Key=ag_1_lastperiodmsec ,  Value=220 },
	{ Key=ag_1_errorsarow ,  Value=0 },
	{ Key=ag_1_1periods_query_timeouts ,  Value=0 },
	{ Key=ag_1_1periods_connect_timeouts ,  Value=0 },
	{ Key=ag_1_1periods_connect_failures ,  Value=0 },
	{ Key=ag_1_1periods_network_errors ,  Value=0 },
	{ Key=ag_1_1periods_wrong_replies ,  Value=0 },
	{ Key=ag_1_1periods_unexpected_closings ,  Value=0 },
	{ Key=ag_1_1periods_warnings ,  Value=0 },
	{ Key=ag_1_1periods_succeeded_queries ,  Value=27 },
	{ Key=ag_1_1periods_msecsperquery ,  Value=231.24 },
	{ Key=ag_1_5periods_query_timeouts ,  Value=0 },
	{ Key=ag_1_5periods_connect_timeouts ,  Value=0 },
	{ Key=ag_1_5periods_connect_failures ,  Value=0 },
	{ Key=ag_1_5periods_network_errors ,  Value=0 },
	{ Key=ag_1_5periods_wrong_replies ,  Value=0 },
	{ Key=ag_1_5periods_warnings ,  Value=0 },
	{ Key=ag_1_5periods_succeeded_queries ,  Value=146 },
	{ Key=ag_1_5periods_msecsperquery ,  Value=230.85 }],
  error="" ,
  total=0,
  warning="" }
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->

```typescript
res = await utilsApi.sql("SHOW AGENT STATUS");
```
<!-- response javascript -->

```typescript
{
	"columns":
	[{
		"Key":
		{
			"type": "string"
		}
	},
    {
    	"Value":
    	{
    		"type": "string"
    	}
    }],
 	"data":
 	[
		{"Key": "status_period_seconds", "Value": "60"},
		{"Key": "status_stored_periods", "Value": "15"},
		{"Key": "ag_0_hostname", "Value": "192.168.0.202:6713"},
		{"Key": "ag_0_references", "Value": "2"},
		{"Key": "ag_0_lastquery", "Value": "0.41"},
		{"Key": "ag_0_lastanswer", "Value": "0.19"},
		{"Key": "ag_0_lastperiodmsec", "Value": "222"},
		{"Key": "ag_0_errorsarow", "Value": "0"},
		{"Key": "ag_0_1periods_query_timeouts", "Value": "0"},
		{"Key": "ag_0_1periods_connect_timeouts", "Value": "0"},
		{"Key": "ag_0_1periods_connect_failures", "Value": "0"},
		{"Key": "ag_0_1periods_network_errors", "Value": "0"},
		{"Key": "ag_0_1periods_wrong_replies", "Value": "0"},
		{"Key": "ag_0_1periods_unexpected_closings", "Value": "0"},
		{"Key": "ag_0_1periods_warnings", "Value": "0"},
		{"Key": "ag_0_1periods_succeeded_queries", "Value": "27"},
		{"Key": "ag_0_1periods_msecsperquery", "Value": "232.31"},
		{"Key": "ag_0_5periods_query_timeouts", "Value": "0"},
		{"Key": "ag_0_5periods_connect_timeouts", "Value": "0"},
		{"Key": "ag_0_5periods_connect_failures", "Value": "0"},
		{"Key": "ag_0_5periods_network_errors", "Value": "0"},
		{"Key": "ag_0_5periods_wrong_replies", "Value": "0"},
		{"Key": "ag_0_5periods_unexpected_closings", "Value": "0"},
		{"Key": "ag_0_5periods_warnings", "Value": "0"},
		{"Key": "ag_0_5periods_succeeded_queries", "Value": "146"},
		{"Key": "ag_0_5periods_msecsperquery", "Value": "231.83"},
		{"Key": "ag_1_hostname 192.168.0.202:6714"},
		{"Key": "ag_1_references", "Value": "2"},
		{"Key": "ag_1_lastquery", "Value": "0.41"},
		{"Key": "ag_1_lastanswer", "Value": "0.19"},
		{"Key": "ag_1_lastperiodmsec", "Value": "220"},
		{"Key": "ag_1_errorsarow", "Value": "0"},
		{"Key": "ag_1_1periods_query_timeouts", "Value": "0"},
		{"Key": "ag_1_1periods_connect_timeouts", "Value": "0"},
		{"Key": "ag_1_1periods_connect_failures", "Value": "0"},
		{"Key": "ag_1_1periods_network_errors", "Value": "0"},
		{"Key": "ag_1_1periods_wrong_replies", "Value": "0"},
		{"Key": "ag_1_1periods_unexpected_closings", "Value": "0"},
		{"Key": "ag_1_1periods_warnings", "Value": "0"},
		{"Key": "ag_1_1periods_succeeded_queries", "Value": "27"},
		{"Key": "ag_1_1periods_msecsperquery", "Value": "231.24"},
		{"Key": "ag_1_5periods_query_timeouts", "Value": "0"},
		{"Key": "ag_1_5periods_connect_timeouts", "Value": "0"},
		{"Key": "ag_1_5periods_connect_failures", "Value": "0"},
		{"Key": "ag_1_5periods_network_errors", "Value": "0"},
		{"Key": "ag_1_5periods_wrong_replies", "Value": "0"},
		{"Key": "ag_1_5periods_warnings", "Value": "0"},
		{"Key": "ag_1_5periods_succeeded_queries", "Value": "146"},
		{"Key": "ag_1_5periods_msecsperquery", "Value": "230.85"}
	],
 	"error": "",
 	"total": 0,
 	"warning": ""
 }
```

<!-- intro -->
##### Go:

<!-- request Go -->

```go
res := apiClient.UtilsAPI.Sql(context.Background()).Body("SHOW AGENT STATUS").Execute()
```
<!-- response Go -->

```go
{
	"columns":
	[{
		"Key":
		{
			"type": "string"
		}
	},
    {
    	"Value":
    	{
    		"type": "string"
    	}
    }],
 	"data":
 	[
		{"Key": "status_period_seconds", "Value": "60"},
		{"Key": "status_stored_periods", "Value": "15"},
		{"Key": "ag_0_hostname", "Value": "192.168.0.202:6713"},
		{"Key": "ag_0_references", "Value": "2"},
		{"Key": "ag_0_lastquery", "Value": "0.41"},
		{"Key": "ag_0_lastanswer", "Value": "0.19"},
		{"Key": "ag_0_lastperiodmsec", "Value": "222"},
		{"Key": "ag_0_errorsarow", "Value": "0"},
		{"Key": "ag_0_1periods_query_timeouts", "Value": "0"},
		{"Key": "ag_0_1periods_connect_timeouts", "Value": "0"},
		{"Key": "ag_0_1periods_connect_failures", "Value": "0"},
		{"Key": "ag_0_1periods_network_errors", "Value": "0"},
		{"Key": "ag_0_1periods_wrong_replies", "Value": "0"},
		{"Key": "ag_0_1periods_unexpected_closings", "Value": "0"},
		{"Key": "ag_0_1periods_warnings", "Value": "0"},
		{"Key": "ag_0_1periods_succeeded_queries", "Value": "27"},
		{"Key": "ag_0_1periods_msecsperquery", "Value": "232.31"},
		{"Key": "ag_0_5periods_query_timeouts", "Value": "0"},
		{"Key": "ag_0_5periods_connect_timeouts", "Value": "0"},
		{"Key": "ag_0_5periods_connect_failures", "Value": "0"},
		{"Key": "ag_0_5periods_network_errors", "Value": "0"},
		{"Key": "ag_0_5periods_wrong_replies", "Value": "0"},
		{"Key": "ag_0_5periods_unexpected_closings", "Value": "0"},
		{"Key": "ag_0_5periods_warnings", "Value": "0"},
		{"Key": "ag_0_5periods_succeeded_queries", "Value": "146"},
		{"Key": "ag_0_5periods_msecsperquery", "Value": "231.83"},
		{"Key": "ag_1_hostname 192.168.0.202:6714"},
		{"Key": "ag_1_references", "Value": "2"},
		{"Key": "ag_1_lastquery", "Value": "0.41"},
		{"Key": "ag_1_lastanswer", "Value": "0.19"},
		{"Key": "ag_1_lastperiodmsec", "Value": "220"},
		{"Key": "ag_1_errorsarow", "Value": "0"},
		{"Key": "ag_1_1periods_query_timeouts", "Value": "0"},
		{"Key": "ag_1_1periods_connect_timeouts", "Value": "0"},
		{"Key": "ag_1_1periods_connect_failures", "Value": "0"},
		{"Key": "ag_1_1periods_network_errors", "Value": "0"},
		{"Key": "ag_1_1periods_wrong_replies", "Value": "0"},
		{"Key": "ag_1_1periods_unexpected_closings", "Value": "0"},
		{"Key": "ag_1_1periods_warnings", "Value": "0"},
		{"Key": "ag_1_1periods_succeeded_queries", "Value": "27"},
		{"Key": "ag_1_1periods_msecsperquery", "Value": "231.24"},
		{"Key": "ag_1_5periods_query_timeouts", "Value": "0"},
		{"Key": "ag_1_5periods_connect_timeouts", "Value": "0"},
		{"Key": "ag_1_5periods_connect_failures", "Value": "0"},
		{"Key": "ag_1_5periods_network_errors", "Value": "0"},
		{"Key": "ag_1_5periods_wrong_replies", "Value": "0"},
		{"Key": "ag_1_5periods_warnings", "Value": "0"},
		{"Key": "ag_1_5periods_succeeded_queries", "Value": "146"},
		{"Key": "ag_1_5periods_msecsperquery", "Value": "230.85"}
	],
 	"error": "",
 	"total": 0,
 	"warning": ""
 }
```

<!-- end -->


<!-- example SHOW AGENT LIKE -->

支持可选的 `LIKE` 子句，语法同 `SHOW STATUS`。

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SHOW AGENT STATUS LIKE '%5period%msec%';
```

<!-- response SQL -->
```sql
+-----------------------------+--------+
| Key                         | Value  |
+-----------------------------+--------+
| ag_0_5periods_msecsperquery | 234.72 |
| ag_1_5periods_msecsperquery | 233.73 |
| ag_2_5periods_msecsperquery | 343.81 |
+-----------------------------+--------+
3 rows in set (0.00 sec)
```

<!-- intro -->
##### JSON:
<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "SHOW AGENT STATUS LIKE '%5period%msec%';"
```

<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "Key": {
          "type": "string"
        }
      },
      {
        "Value": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "Key": "ag_0_5periods_msecsperquery",
        "Value": "234.72"
      },
      {
        "Key": "ag_1_5periods_msecsperquery",
        "Value": "234.72"
      },
      {
        "Key": "ag_2_5periods_msecsperquery",
        "Value": "234.72"
      }
    ],
    "total": 3,
    "error": "",
    "warning": ""
  }
]
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$client->nodes()->agentstatus(
    ['body'=>
        ['pattern'=>'%5period%msec%']
    ]
);
```

<!-- response PHP -->

```php
Array(
    [ag_0_5periods_msecsperquery] => 234.72
    [ag_1_5periods_msecsperquery] => 233.73
    [ag_2_5periods_msecsperquery] => 343.81
)
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('SHOW AGENT STATUS LIKE \'%5period%msec%\'')
```
<!-- response Python -->

```python
{u'columns': [{u'Key': {u'type': u'string'}},
              {u'Value': {u'type': u'string'}}],
 u'data': [
	{u'Key': u'ag_0_5periods_msecsperquery', u'Value': u'234.72'},
	{u'Key': u'ag_1_5periods_msecsperquery', u'Value': u'233.73'},
	{u'Key': u'ag_2_5periods_msecsperquery', u'Value': u'343.81'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('SHOW AGENT STATUS LIKE \'%5period%msec%\'')
```
<!-- response Python-asyncio -->

```python
{u'columns': [{u'Key': {u'type': u'string'}},
              {u'Value': {u'type': u'string'}}],
 u'data': [
	{u'Key': u'ag_0_5periods_msecsperquery', u'Value': u'234.72'},
	{u'Key': u'ag_1_5periods_msecsperquery', u'Value': u'233.73'},
	{u'Key': u'ag_2_5periods_msecsperquery', u'Value': u'343.81'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql("SHOW AGENT STATUS LIKE \"%5period%msec%\"");
```
<!-- response javascript -->

```javascript
{"columns": [{"Key": {"type": "string"}},
              {"Value": {"type": "string"}}],
 "data": [
	{"Key": "ag_0_5periods_msecsperquery", "Value": "234.72"},
	{"Key": "ag_1_5periods_msecsperquery", "Value": "233.73"},
	{"Key": "ag_2_5periods_msecsperquery", "Value": "343.81"}],
 "error": "",
 "total": 0,
 "warning": ""}
```


<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("SHOW AGENT STATUS LIKE \"%5period%msec%\"");
```
<!-- response Java -->

```java
{columns: [{Key={type=string}},
              {Value={type=string}}],
 data: [
	{Key=ag_0_5periods_msecsperquery, Value=234.72},
	{Key=ag_1_5periods_msecsperquery, Value=233.73},
	{Key=ag_2_5periods_msecsperquery, Value=343.81}],
 error: ,
 total: 0,
 warning: }
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("SHOW AGENT STATUS LIKE \"%5period%msec%\"");
```
<!-- response C# -->

```clike
{columns: [{Key={type=string}},
              {Value={type=string}}],
 data: [
	{Key=ag_0_5periods_msecsperquery, Value=234.72},
	{Key=ag_1_5periods_msecsperquery, Value=233.73},
	{Key=ag_2_5periods_msecsperquery, Value=343.81}],
 error: "",
 total: 0,
 warning: ""}
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("SHOW AGENT STATUS LIKE \"%5period%msec%\"", Some(true)).await;
```
<!-- response Rust -->

```rust
{columns: [{Key={type=string}},
              {Value={type=string}}],
 data: [
	{Key=ag_0_5periods_msecsperquery, Value=234.72},
	{Key=ag_1_5periods_msecsperquery, Value=233.73},
	{Key=ag_2_5periods_msecsperquery, Value=343.81}],
 error: "",
 total: 0,
 warning: ""}
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->

```typescript
res = await utilsApi.sql("SHOW AGENT STATUS LIKE \"%5period%msec%\"");
```
<!-- response TypeScript -->

```typescript
{
	"columns":
	[{
		"Key": {"type": "string"}
	},
    {
    	"Value": {"type": "string"}
    }],
	"data":
	[
		{"Key": "ag_0_5periods_msecsperquery", "Value": "234.72"},
		{"Key": "ag_1_5periods_msecsperquery", "Value": "233.73"},
		{"Key": "ag_2_5periods_msecsperquery", "Value": "343.81"}
	],
 	"error": "",
 	"total": 0,
 	"warning": ""
}
```

<!-- intro -->
##### Go:

<!-- request Go -->

```go
apiClient.UtilsAPI.Sql(context.Background()).Body("SHOW AGENT STATUS LIKE \"%5period%msec%\"").Execute()
```
<!-- response Go -->

```go
{
	"columns":
	[{
		"Key": {"type": "string"}
	},
    {
    	"Value": {"type": "string"}
    }],
	"data":
	[
		{"Key": "ag_0_5periods_msecsperquery", "Value": "234.72"},
		{"Key": "ag_1_5periods_msecsperquery", "Value": "233.73"},
		{"Key": "ag_2_5periods_msecsperquery", "Value": "343.81"}
	],
 	"error": "",
 	"total": 0,
 	"warning": ""
}
```

<!-- end -->


<!-- example show specific agent -->

您可以通过其地址指定特定的代理。在这种情况下，只会显示该代理的数据。此外，将使用 `agent_` 前缀代替 `ag_N_`：

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SHOW AGENT '192.168.0.202:6714' STATUS LIKE '%15periods%';
```

<!-- response SQL -->

```sql
+-------------------------------------+--------+
| Variable_name                       | Value  |
+-------------------------------------+--------+
| agent_15periods_query_timeouts      | 0      |
| agent_15periods_connect_timeouts    | 0      |
| agent_15periods_connect_failures    | 0      |
| agent_15periods_network_errors      | 0      |
| agent_15periods_wrong_replies       | 0      |
| agent_15periods_unexpected_closings | 0      |
| agent_15periods_warnings            | 0      |
| agent_15periods_succeeded_queries   | 439    |
| agent_15periods_msecsperquery       | 231.73 |
+-------------------------------------+--------+
9 rows in set (0.00 sec)
```

<!-- intro -->
##### JSON:
<!-- request JSON -->

```JSON
POST /sql? mode=raw -d "SHOW AGENT '192.168.0.202:6714' STATUS LIKE '%15periods%';"
```

<!-- response JSON -->

```JSON
[
  {
    "columns": [
      {
        "Key": {
          "type": "string"
        }
      },
      {
        "Value": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "Key": "agent_15periods_query_timeouts",
        "Value": "0"
      },
      {
        "Key": "agent_15periods_connect_timeouts",
        "Value": "0"
      },
      {
        "Key": "agent_15periods_connect_failures",
        "Value": "0"
      },
      {
        "Key": "agent_15periods_network_errors",
        "Value": "0"
      },
      {
        "Key": "agent_15periods_wrong_replies",
        "Value": "0"
      },
      {
        "Key": "agent_15periods_unexpected_closings",
        "Value": "0"
      },
      {
        "Key": "agent_15periods_warnings",
        "Value": "0"
      },
      {
        "Key": "agent_15periods_succeeded_queries",
        "Value": "439"
      },
      {
        "Key": "agent_15periods_msecsperquery",
        "Value": "231.73"
      }
    ],
    "total": 9,
    "error": "",
    "warning": ""
  }
]
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$client->nodes()->agentstatus(
    ['body'=>
        ['agent'=>'192.168.0.202:6714'],
        ['pattern'=>'%5period%msec%']
    ]
);
```

<!-- response PHP -->

```php
Array(
    [agent_15periods_query_timeouts] => 0
    [agent_15periods_connect_timeouts] => 0
    [agent_15periods_connect_failures] => 0
    [agent_15periods_network_errors] => 0
    [agent_15periods_wrong_replies] => 0
    [agent_15periods_unexpected_closings] => 0
    [agent_15periods_warnings] => 0
    [agent_15periods_succeeded_queries] => 439
    [agent_15periods_msecsperquery] => 231.73
)
```


<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('SHOW AGENT \'192.168.0.202:6714\' STATUS LIKE \'%15periods%\'')
```
<!-- response Python -->

```python
{u'columns': [{u'Key': {u'type': u'string'}},
              {u'Value': {u'type': u'string'}}],
 u'data': [
	{u'Key': u'agent_15periods_query_timeouts', u'Value': u'0'},
	{u'Key': u'agent_15periods_connect_timeouts', u'Value': u'0'},
	{u'Key': u'agent_15periods_connect_failures', u'Value': u'0'},
    {u'Key': u'agent_15periods_network_errors', u'Value': u'0'},
    {u'Key': u'agent_15periods_connect_failures', u'Value': u'0'},
    {u'Key': u'agent_15periods_wrong_replies', u'Value': u'0'},
    {u'Key': u'agent_15periods_unexpected_closings', u'Value': u'0'},
    {u'Key': u'agent_15periods_warnings', u'Value': u'0'},
    {u'Key': u'agent_15periods_succeeded_queries', u'Value': u'439'},
    {u'Key': u'agent_15periods_msecsperquery', u'Value': u'233.73'},
    ],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('SHOW AGENT \'192.168.0.202:6714\' STATUS LIKE \'%15periods%\'')
```
<!-- response Python-asyncio -->

```python
{u'columns': [{u'Key': {u'type': u'string'}},
              {u'Value': {u'type': u'string'}}],
 u'data': [
	{u'Key': u'agent_15periods_query_timeouts', u'Value': u'0'},
	{u'Key': u'agent_15periods_connect_timeouts', u'Value': u'0'},
	{u'Key': u'agent_15periods_connect_failures', u'Value': u'0'},
    {u'Key': u'agent_15periods_network_errors', u'Value': u'0'},
    {u'Key': u'agent_15periods_connect_failures', u'Value': u'0'},
    {u'Key': u'agent_15periods_wrong_replies', u'Value': u'0'},
    {u'Key': u'agent_15periods_unexpected_closings', u'Value': u'0'},
    {u'Key': u'agent_15periods_warnings', u'Value': u'0'},
    {u'Key': u'agent_15periods_succeeded_queries', u'Value': u'439'},
    {u'Key': u'agent_15periods_msecsperquery', u'Value': u'233.73'},
    ],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql("SHOW AGENT \"192.168.0.202:6714\" STATUS LIKE \"%15periods%\"");
```
<!-- response javascript -->

```javascript
{"columns": [{"Key": {"type": "string"}},
              {"Value": {"type": "string"}}],
 "data": [
	{"Key": "agent_15periods_query_timeouts", "Value": "0"},
	{"Key": "agent_15periods_connect_timeouts", "Value": "0"},
	{"Key": "agent_15periods_connect_failures", "Value": "0"},
    {"Key": "agent_15periods_network_errors", "Value": "0"},
    {"Key": "agent_15periods_connect_failures", "Value": "0"},
    {"Key": "agent_15periods_wrong_replies", "Value": "0"},
    {"Key": "agent_15periods_unexpected_closings", "Value": "0"},
    {"Key": "agent_15periods_warnings", "Value": "0"},
    {"Key": "agent_15periods_succeeded_queries", "Value": "439"},
    {"Key": "agent_15periods_msecsperquery", "Value": "233.73"},
    ],
 "error": "",
 "total": 0,
 "warning": ""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("SHOW AGENT \"192.168.0.202:6714\" STATUS LIKE \"%15periods%\"");
```
<!-- response Java -->

```java
{columns=[{Key={type=string}},
              {Value={type=string}}],
 data=[
	{Key=agent_15periods_query_timeouts, Value=0},
	{Key=agent_15periods_connect_timeouts, Value=0},
	{Key=agent_15periods_connect_failures, Value=0},
    {Key=agent_15periods_network_errors, Value=0},
    {Key=agent_15periods_connect_failures, Value=0},
    {Key=agent_15periods_wrong_replies, Value=0},
    {Key=agent_15periods_unexpected_closings, Value=0},
    {Key=agent_15periods_warnings, Value=0},
    {Key=agent_15periods_succeeded_queries, Value=439},
    {Key=agent_15periods_msecsperquery, Value=233.73},
    ],
 error=,
 total=0,
 warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("SHOW AGENT \"192.168.0.202:6714\" STATUS LIKE \"%15periods%\"");
```
<!-- response C# -->

```clike
{columns=[{Key={type=string}},
              {Value={type=string}}],
 data=[
	{Key=agent_15periods_query_timeouts, Value=0},
	{Key=agent_15periods_connect_timeouts, Value=0},
	{Key=agent_15periods_connect_failures, Value=0},
    {Key=agent_15periods_network_errors, Value=0},
    {Key=agent_15periods_connect_failures, Value=0},
    {Key=agent_15periods_wrong_replies, Value=0},
    {Key=agent_15periods_unexpected_closings, Value=0},
    {Key=agent_15periods_warnings, Value=0},
    {Key=agent_15periods_succeeded_queries, Value=439},
    {Key=agent_15periods_msecsperquery, Value=233.73},
    ],
 error="",
 total=0,
 warning=""}
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("SHOW AGENT \"192.168.0.202:6714\" STATUS LIKE \"%15periods%\"", Some(true)).await;
```
<!-- response Rust -->

```rust
{columns=[{Key={type=string}},
              {Value={type=string}}],
 data=[
	{Key=agent_15periods_query_timeouts, Value=0},
	{Key=agent_15periods_connect_timeouts, Value=0},
	{Key=agent_15periods_connect_failures, Value=0},
    {Key=agent_15periods_network_errors, Value=0},
    {Key=agent_15periods_connect_failures, Value=0},
    {Key=agent_15periods_wrong_replies, Value=0},
    {Key=agent_15periods_unexpected_closings, Value=0},
    {Key=agent_15periods_warnings, Value=0},
    {Key=agent_15periods_succeeded_queries, Value=439},
    {Key=agent_15periods_msecsperquery, Value=233.73},
    ],
 error="",
 total=0,
 warning=""}
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->

```typescript
res = await utilsApi.sql("SHOW AGENT \"192.168.0.202:6714\" STATUS LIKE \"%15periods%\"");
```
<!-- response TypeScript -->

```typescript
{
	"columns":
	[{
		{"Key": {"type": "string"}
	},
    	{"Value": {"type": "string"}
    }],
	"data":
	[
		{"Key": "agent_15periods_query_timeouts", "Value": "0"},
		{"Key": "agent_15periods_connect_timeouts", "Value": "0"},
		{"Key": "agent_15periods_connect_failures", "Value": "0"},
	    {"Key": "agent_15periods_network_errors", "Value": "0"},
	    {"Key": "agent_15periods_connect_failures", "Value": "0"},
	    {"Key": "agent_15periods_wrong_replies", "Value": "0"},
	    {"Key": "agent_15periods_unexpected_closings", "Value": "0"},
	    {"Key": "agent_15periods_warnings", "Value": "0"},
	    {"Key": "agent_15periods_succeeded_queries", "Value": "439"},
	    {"Key": "agent_15periods_msecsperquery", "Value": "233.73"},
    ],
	"error": "",
	"total": 0,
	"warning": ""
}
```

<!-- intro -->
##### Go:

<!-- request Go -->

```go
apiClient.UtilsAPI.Sql(context.Background()).Body("SHOW AGENT \"192.168.0.202:6714\" STATUS LIKE \"%15periods%\").Execute()
```
<!-- response Go -->

```go
{
	"columns":
	[{
		{"Key": {"type": "string"}
	},
    	{"Value": {"type": "string"}
    }],
	"data":
	[
		{"Key": "agent_15periods_query_timeouts", "Value": "0"},
		{"Key": "agent_15periods_connect_timeouts", "Value": "0"},
		{"Key": "agent_15periods_connect_failures", "Value": "0"},
	    {"Key": "agent_15periods_network_errors", "Value": "0"},
	    {"Key": "agent_15periods_connect_failures", "Value": "0"},
	    {"Key": "agent_15periods_wrong_replies", "Value": "0"},
	    {"Key": "agent_15periods_unexpected_closings", "Value": "0"},
	    {"Key": "agent_15periods_warnings", "Value": "0"},
	    {"Key": "agent_15periods_succeeded_queries", "Value": "439"},
	    {"Key": "agent_15periods_msecsperquery", "Value": "233.73"},
    ],
	"error": "",
	"total": 0,
	"warning": ""
}
```

<!-- end -->
<!-- example show agent table status -->

最后，您可以使用 `SHOW AGENT table_name STATUS` 语句检查特定分布式表中代理的状态。该语句显示表的高可用性 (HA) 状态（即是否使用代理镜像）并提供有关镜像的信息，包括：地址、blackhole 和持久标志，以及当采用[加权概率策略](../Creating_a_cluster/Remote_nodes/Load_balancing.md)之一时使用的镜像选择概率。

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SHOW AGENT dist_index STATUS;
```

<!-- response SQL -->
```sql
+--------------------------------------+--------------------------------+
| Variable_name                        | Value                          |
+--------------------------------------+--------------------------------+
| dstindex_1_is_ha                     | 1                              |
| dstindex_1mirror1_id                 | 192.168.0.202:6713:loc         |
| dstindex_1mirror1_probability_weight | 0.372864                       |
| dstindex_1mirror1_is_blackhole       | 0                              |
| dstindex_1mirror1_is_persistent      | 0                              |
| dstindex_1mirror2_id                 | 192.168.0.202:6714:loc         |
| dstindex_1mirror2_probability_weight | 0.374635                       |
| dstindex_1mirror2_is_blackhole       | 0                              |
| dstindex_1mirror2_is_persistent      | 0                              |
| dstindex_1mirror3_id                 | dev1.manticoresearch.com:6714:loc |
| dstindex_1mirror3_probability_weight | 0.252501                       |
| dstindex_1mirror3_is_blackhole       | 0                              |
| dstindex_1mirror3_is_persistent      | 0                              |
+--------------------------------------+--------------------------------+
13 rows in set (0.00 sec)
```

<!-- intro -->
##### JSON:
<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "SHOW AGENT dist_index STATUS;"
```

<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "Key": {
          "type": "string"
        }
      },
      {
        "Value": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "Key": "dstindex_1_is_ha",
        "Value": "1"
      },
      {
        "Key": "dstindex_1mirror1_id",
        "Value": "192.168.0.202:6713:loc"
      },
      {
        "Key": "dstindex_1mirror1_probability_weight",
        "Value": "0.372864"
      },
      {
        "Key": "dstindex_1mirror1_is_blackhole",
        "Value": "0"
      },
      {
        "Key": "dstindex_1mirror1_is_persistent",
        "Value": "0"
      },
      {
        "Key": "dstindex_1mirror2_id",
        "Value": "192.168.0.202:6714:loc"
      },
      {
        "Key": "dstindex_1mirror2_probability_weight",
        "Value": "0.3746350"
      },
      {
        "Key": "dstindex_1mirror2_is_blackhole",
        "Value": "0"
      },
      {
        "Key": "dstindex_1mirror2_is_persistent",
        "Value": "0"
      },
      {
        "Key": "dstindex_1mirror3_id",
        "Value": "dev1.manticoresearch.com:6714:loc"
      },
      {
        "Key": "dstindex_1mirror3_probability_weight",
        "Value": "0.252501"
      },
      {
        "Key": "dstindex_1mirror3_is_blackhole",
        "Value": "0"
      },
      {
        "Key": "dstindex_1mirror3_is_persistent",
        "Value": "0"
      }
    ],
    "total": 13,
    "error": "",
    "warning": ""
  }
]
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$client->nodes()->agentstatus(
    ['body'=>
        ['agent'=>'dist_index']
    ]
);
```

<!-- response PHP -->

```php
Array(
    [dstindex_1_is_ha] => 1
    [dstindex_1mirror1_id] => 192.168.0.202:6713:loc
    [dstindex_1mirror1_probability_weight] => 0.372864
    [dstindex_1mirror1_is_blackhole] => 0
    [dstindex_1mirror1_is_persistent] => 0
    [dstindex_1mirror2_id] => 192.168.0.202:6714:loc
    [dstindex_1mirror2_probability_weight] => 0.374635
    [dstindex_1mirror2_is_blackhole] => 0
    [dstindex_1mirror2_is_persistent] => 0
    [dstindex_1mirror3_id] => dev1.manticoresearch.com:6714:loc
    [dstindex_1mirror3_probability_weight] => 0.252501
    [dstindex_1mirror3_is_blackhole] => 0
    [dstindex_1mirror3_is_persistent] => 0
)
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('SHOW AGENT \'192.168.0.202:6714\' STATUS LIKE \'%15periods%\'')
```
<!-- response Python -->

```python
{u'columns': [{u'Key': {u'type': u'string'}},
              {u'Value': {u'type': u'string'}}],
 u'data': [
	{u'Key': u'dstindex_1_is_ha', u'Value': u'1'},
	{u'Key': u'dstindex_1mirror1_id', u'Value': u'192.168.0.202:6713:loc'},
	{u'Key': u'dstindex_1mirror1_probability_weight', u'Value': u'0.372864'},
    {u'Key': u'dstindex_1mirror1_is_blackhole', u'Value': u'0'},
    {u'Key': u'dstindex_1mirror1_is_persistent', u'Value': u'0'},
    {u'Key': u'dstindex_1mirror2_id', u'Value': u'192.168.0.202:6714:loc'},
    {u'Key': u'dstindex_1mirror2_probability_weight', u'Value': u'0.374635'},
    {u'Key': u'dstindex_1mirror2_is_blackhole', u'Value': u'0'},
    {u'Key': u'dstindex_1mirror2_is_persistent', u'Value': u'439'},
    {u'Key': u'dstindex_1mirror3_id', u'Value': u'dev1.manticoresearch.com:6714:loc'},
    {u'Key': u'dstindex_1mirror3_probability_weight', u'Value': u' 0.252501'},
    {u'Key': u'dstindex_1mirror3_is_blackhole', u'Value': u'0'},
    {u'Key': u'dstindex_1mirror3_is_persistent', u'Value': u'439'}
    ],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('SHOW AGENT \'192.168.0.202:6714\' STATUS LIKE \'%15periods%\'')
```
<!-- response Python-asyncio -->

```python
{u'columns': [{u'Key': {u'type': u'string'}},
              {u'Value': {u'type': u'string'}}],
 u'data': [
	{u'Key': u'dstindex_1_is_ha', u'Value': u'1'},
	{u'Key': u'dstindex_1mirror1_id', u'Value': u'192.168.0.202:6713:loc'},
	{u'Key': u'dstindex_1mirror1_probability_weight', u'Value': u'0.372864'},
    {u'Key': u'dstindex_1mirror1_is_blackhole', u'Value': u'0'},
    {u'Key': u'dstindex_1mirror1_is_persistent', u'Value': u'0'},
    {u'Key': u'dstindex_1mirror2_id', u'Value': u'192.168.0.202:6714:loc'},
    {u'Key': u'dstindex_1mirror2_probability_weight', u'Value': u'0.374635'},
    {u'Key': u'dstindex_1mirror2_is_blackhole', u'Value': u'0'},
    {u'Key': u'dstindex_1mirror2_is_persistent', u'Value': u'439'},
    {u'Key': u'dstindex_1mirror3_id', u'Value': u'dev1.manticoresearch.com:6714:loc'},
    {u'Key': u'dstindex_1mirror3_probability_weight', u'Value': u' 0.252501'},
    {u'Key': u'dstindex_1mirror3_is_blackhole', u'Value': u'0'},
    {u'Key': u'dstindex_1mirror3_is_persistent', u'Value': u'439'}
    ],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql("SHOW AGENT \"192.168.0.202:6714\" STATUS LIKE \"%15periods%\"");
```
<!-- response javascript -->

```javascript
{"columns": [{"Key": {"type": "string"}},
              {"Value": {"type": "string"}}],
 "data": [
	{"Key": "dstindex_1_is_ha", "Value": "1"},
	{"Key": "dstindex_1mirror1_id", "Value": "192.168.0.202:6713:loc"},
	{"Key": "dstindex_1mirror1_probability_weight", "Value": "0.372864"},
    {"Key": "dstindex_1mirror1_is_blackhole", "Value": "0"},
    {"Key": "dstindex_1mirror1_is_persistent", "Value": "0"},
    {"Key": "dstindex_1mirror2_id", "Value": "192.168.0.202:6714:loc"},
    {"Key": "dstindex_1mirror2_probability_weight", "Value": "0.374635"},
    {"Key": "dstindex_1mirror2_is_blackhole", "Value": "0"},
    {"Key": "dstindex_1mirror2_is_persistent", "Value": "439"},
    {"Key": "dstindex_1mirror3_id", "Value": "dev1.manticoresearch.com:6714:loc"},
    {"Key": "dstindex_1mirror3_probability_weight", "Value": " 0.252501"},
    {"Key": "dstindex_1mirror3_is_blackhole", "Value": "0"},
    {"Key": "dstindex_1mirror3_is_persistent", "Value": "439"}
    ],
 "error": "",
 "total": 0,
 "warning": ""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("SHOW AGENT \"192.168.0.202:6714\" STATUS LIKE \"%15periods%\"");
```
<!-- response Java -->

```java
{columns=[{Key={type=string}},
              {Value={type=string}}],
 data=[
	{Key=dstindex_1_is_ha, Value=1},
	{Key=dstindex_1mirror1_id, Value=192.168.0.202:6713:loc},
	{Key=dstindex_1mirror1_probability_weight, Value=0.372864},
    {Key=dstindex_1mirror1_is_blackhole, Value=0},
    {Key=dstindex_1mirror1_is_persistent, Value=0},
    {Key=dstindex_1mirror2_id, Value=192.168.0.202:6714:loc},
    {Key=dstindex_1mirror2_probability_weight, Value=0.374635},
    {Key=dstindex_1mirror2_is_blackhole, Value=0},
    {Key=dstindex_1mirror2_is_persistent, Value=439},
    {Key=dstindex_1mirror3_id, Value=dev1.manticoresearch.com:6714:loc},
    {Key=dstindex_1mirror3_probability_weight, Value= 0.252501},
    {Key=dstindex_1mirror3_is_blackhole, Value=0},
    {Key=dstindex_1mirror3_is_persistent, Value=439}
    ],
 error=,
 total=0,
 warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("SHOW AGENT \"192.168.0.202:6714\" STATUS LIKE \"%15periods%\"");
```
<!-- response C# -->

```clike
{columns=[{Key={type=string}},
              {Value={type=string}}],
 data=[
	{Key=dstindex_1_is_ha, Value=1},
	{Key=dstindex_1mirror1_id, Value=192.168.0.202:6713:loc},
	{Key=dstindex_1mirror1_probability_weight, Value=0.372864},
    {Key=dstindex_1mirror1_is_blackhole, Value=0},
    {Key=dstindex_1mirror1_is_persistent, Value=0},
    {Key=dstindex_1mirror2_id, Value=192.168.0.202:6714:loc},
    {Key=dstindex_1mirror2_probability_weight, Value=0.374635},
    {Key=dstindex_1mirror2_is_blackhole, Value=0},
    {Key=dstindex_1mirror2_is_persistent, Value=439},
    {Key=dstindex_1mirror3_id, Value=dev1.manticoresearch.com:6714:loc},
    {Key=dstindex_1mirror3_probability_weight, Value= 0.252501},
    {Key=dstindex_1mirror3_is_blackhole, Value=0},
    {Key=dstindex_1mirror3_is_persistent, Value=439}
    ],
 error="",
 total=0,
 warning=""}
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("SHOW AGENT \"192.168.0.202:6714\" STATUS LIKE \"%15periods%\"", Some(true)).await;
```
<!-- response Rust -->

```rust
{columns=[{Key={type=string}},
              {Value={type=string}}],
 data=[
	{Key=dstindex_1_is_ha, Value=1},
	{Key=dstindex_1mirror1_id, Value=192.168.0.202:6713:loc},
	{Key=dstindex_1mirror1_probability_weight, Value=0.372864},
    {Key=dstindex_1mirror1_is_blackhole, Value=0},
    {Key=dstindex_1mirror1_is_persistent, Value=0},
    {Key=dstindex_1mirror2_id, Value=192.168.0.202:6714:loc},
    {Key=dstindex_1mirror2_probability_weight, Value=0.374635},
    {Key=dstindex_1mirror2_is_blackhole, Value=0},
    {Key=dstindex_1mirror2_is_persistent, Value=439},
    {Key=dstindex_1mirror3_id, Value=dev1.manticoresearch.com:6714:loc},
    {Key=dstindex_1mirror3_probability_weight, Value= 0.252501},
    {Key=dstindex_1mirror3_is_blackhole, Value=0},
    {Key=dstindex_1mirror3_is_persistent, Value=439}
    ],
 error="",
 total=0,
 warning=""}
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->

```typescript
res = await utilsApi.sql("SHOW AGENT \"192.168.0.202:6714\" STATUS LIKE \"%15periods%\"");
```
<!-- response TypeScript -->

```typescript
{
	"columns":
	[{
		"Key": {"type": "string"}},
        {"Value": {"type": "string"}
	}],
	"data":
	[
		{"Key": "dstindex_1_is_ha", "Value": "1"},
		{"Key": "dstindex_1mirror1_id", "Value": "192.168.0.202:6713:loc"},
		{"Key": "dstindex_1mirror1_probability_weight", "Value": "0.372864"},
	    {"Key": "dstindex_1mirror1_is_blackhole", "Value": "0"},
	    {"Key": "dstindex_1mirror1_is_persistent", "Value": "0"},
	    {"Key": "dstindex_1mirror2_id", "Value": "192.168.0.202:6714:loc"},
	    {"Key": "dstindex_1mirror2_probability_weight", "Value": "0.374635"},
	    {"Key": "dstindex_1mirror2_is_blackhole", "Value": "0"},
	    {"Key": "dstindex_1mirror2_is_persistent", "Value": "439"},
	    {"Key": "dstindex_1mirror3_id", "Value": "dev1.manticoresearch.com:6714:loc"},
	    {"Key": "dstindex_1mirror3_probability_weight", "Value": " 0.252501"},
	    {"Key": "dstindex_1mirror3_is_blackhole", "Value": "0"},
	    {"Key": "dstindex_1mirror3_is_persistent", "Value": "439"}
    ],
	"error": "",
	"total": 0,
	"warning": ""
}
```

<!-- intro -->
##### Go:

<!-- request Go -->

```go
apiClient.UtilsAPI.Sql(context.Background()).Body("SHOW AGENT \"192.168.0.202:6714\" STATUS LIKE \"%15periods%\"").Execute()
```
<!-- response Go -->

```go
{
	"columns":
	[{
		"Key": {"type": "string"}},
        {"Value": {"type": "string"}
	}],
	"data":
	[
		{"Key": "dstindex_1_is_ha", "Value": "1"},
		{"Key": "dstindex_1mirror1_id", "Value": "192.168.0.202:6713:loc"},
		{"Key": "dstindex_1mirror1_probability_weight", "Value": "0.372864"},
	    {"Key": "dstindex_1mirror1_is_blackhole", "Value": "0"},
	    {"Key": "dstindex_1mirror1_is_persistent", "Value": "0"},
	    {"Key": "dstindex_1mirror2_id", "Value": "192.168.0.202:6714:loc"},
	    {"Key": "dstindex_1mirror2_probability_weight", "Value": "0.374635"},
	    {"Key": "dstindex_1mirror2_is_blackhole", "Value": "0"},
	    {"Key": "dstindex_1mirror2_is_persistent", "Value": "439"},
	    {"Key": "dstindex_1mirror3_id", "Value": "dev1.manticoresearch.com:6714:loc"},
	    {"Key": "dstindex_1mirror3_probability_weight", "Value": " 0.252501"},
	    {"Key": "dstindex_1mirror3_is_blackhole", "Value": "0"},
	    {"Key": "dstindex_1mirror3_is_persistent", "Value": "439"}
    ],
	"error": "",
	"total": 0,
	"warning": ""
}
```

<!-- end -->

## Prometheus Exporter

<!-- example prometheus exporter -->

Manticore Search 内置了 Prometheus 导出器。
要请求指标，请确保 HTTP 端口已暴露，然后调用 /metrics 端点。

**注意：**导出器需要启用 **Buddy**。

<!-- intro -->
##### HTTP:

<!-- request http -->

```go
curl -s 0:9308/metrics
```
<!-- response http -->

```http
# HELP manticore_uptime_seconds Time in seconds since start
# TYPE manticore_uptime_seconds counter
manticore_uptime_seconds 25
# HELP manticore_connections_count Connections count since start
# TYPE manticore_connections_count gauge
manticore_connections_count 55
# HELP manticore_maxed_out_error_count Count of maxed_out errors since start
# TYPE manticore_maxed_out_error_count counter
manticore_maxed_out_error_count 0
# HELP manticore_version Manticore Search version
# TYPE manticore_version gauge
manticore_version {version="0.0.0 c88e811b2@25060409 (columnar 5.0.1 59c7092@25060304) (secondary 5.0.1 59c7092@25060304) (knn 5.0.1 59c7092@25060304) (embeddings 1.0.0) (buddy v3.28.6-7-g14ee10)"} 1
# HELP manticore_mysql_version Manticore Search version
# TYPE manticore_mysql_version gauge
manticore_mysql_version {version="0.0.0 c88e811b2@25060409 (columnar 5.0.1 59c7092@25060304) (secondary 5.0.1 59c7092@25060304) (knn 5.0.1 59c7092@25060304) (embeddings 1.0.0)"} 1
# HELP manticore_command_search_count Count of search queries since start
# TYPE manticore_command_search_count counter
manticore_command_search_count 1
......
```

<!-- end -->

<!-- proofread -->

