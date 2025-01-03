# 节点状态

## STATUS

<!-- example status -->

查看 Manticore 节点高层次信息的最简单方法是通过 MySQL 客户端运行 `status` 命令。它将显示关于以下方面的信息：

- 当前版本
- 是否启用了 SSL
- 当前 TCP 端口/Unix 套接字
- 运行时间（Uptime）
- [线程](../Server_settings/Searchd.md#threads) 数量
- [队列中的作业](../Server_settings/Searchd.md#jobs_queue_size) 数量
- 连接数（`clients`）
- 当前正在处理的任务数
- 启动以来执行的查询数
- 队列中的作业数和任务数，按线程数进行归一化显示

<!-- request SQL -->
```sql
mysql> status
```

<!-- response SQL -->
```sql
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

`SHOW STATUS` 是一个 SQL 语句，用于显示各种有用的性能计数器。IO 和 CPU 计数器仅在 `searchd` 以 `--iostats` 和 `--cpustats` 参数启动时可用（或通过 `SET GLOBAL iostats/cpustats=1` 启用）。

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

<!-- end -->

<!-- example show status like -->

`SHOW STATUS` 语句支持可选的 `LIKE` 子句，允许您仅选择符合特定模式的变量。模式语法遵循标准的 SQL 通配符规则，其中 `%` 表示任意数量的任意字符，`_` 表示任意单个字符。

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

<!-- end -->

### 查询时间统计

<!-- example show status like stats_ms -->

`SHOW STATUS` 命令提供了 Manticore 各种性能指标的详细报告，包括插入/替换、搜索和更新查询的查询时间统计数据。这些统计数据是通过滑动窗口计算的，时间段为最近的 1 分钟、5 分钟和 15 分钟，显示了查询时间的平均值、最小值、最大值，以及 95 百分位和 99 百分位的值。

这些指标有助于跟踪特定时间间隔内的性能，使得更容易发现查询响应时间的趋势并找到可能的瓶颈。

以下指标是 `SHOW STATUS` 输出的一部分：

- `*_avg`: 每种查询类型在过去 1 分钟、5 分钟和 15 分钟内的平均查询时间。
- `*_min`: 每种查询类型记录的最短查询时间。
- `*_max`: 每种查询类型记录的最长查询时间。
- `*_pct95`: 95% 的查询在此时间内完成。
- `*_pct99`: 99% 的查询在此时间内完成。

这些统计数据分别针对插入/替换（`insert_replace_stats_*`）、搜索（`search_stats_*`）和更新（`update_stats_*`）查询，提供了不同操作的详细性能分析。

如果在监控时间间隔内没有执行任何查询，系统将显示 `N/A`。

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

<!-- end -->

## SHOW SETTINGS

<!-- example show settings -->

`SHOW SETTINGS` 是一个 SQL 语句，用于显示当前配置文件中的设置。设置名称以以下格式表示：`'config_section_name'.'setting_name'

结果中还包含两个额外的值：

- `configuration_file` - 配置文件的路径
- `worker_pid` - 运行中的 `searchd` 实例的进程 ID

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
| searchd.pid_file         | /var/run/manticore/searchd.pid      |
| searchd.data_dir         | /var/lib/manticore                  |
| searchd.query_log_format | sphinxql                            |
| searchd.binlog_path      | /var/lib/manticore/binlog           |
| common.plugin_dir        | /usr/local/lib/manticore            |
| common.lemmatizer_base   | /usr/share/manticore/morph/         |
+--------------------------+-------------------------------------+
13 rows in set (0.00 sec)
```

<!-- end -->

## SHOW AGENT STATUS
```sql
SHOW AGENT ['agent_or_index'] STATUS [ LIKE pattern ]
```

<!-- example SHOW AGENT STATUS -->

`SHOW AGENT STATUS` 显示[远程代理](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent)或分布式表的统计信息。它包括诸如最后一次请求的时间、最后一次响应、各种类型的错误和成功的次数等值。统计数据显示每个代理在最近 1 分钟、5 分钟和 15 分钟内的情况，每个时间间隔由 [ha_period_karma](../Server_settings/Searchd.md#ha_period_karma) 秒组成。

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

支持可选的 `LIKE` 子句，其语法与 `SHOW STATUS` 中的语法相同。

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

您可以通过其地址指定特定的代理。在这种情况下，将只显示该代理的数据。此外，`agent_` 前缀将用于代替 `ag_N_`：

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

最后，您可以使用 `SHOW AGENT index_name STATUS` 语句检查特定分布式表中代理的状态。该语句会显示表的 HA 状态（即是否使用代理镜像），并提供有关镜像的信息，包括：地址、blackhole 和 persistent 标志，以及当使用其中一种[加权概率策略](../Creating_a_cluster/Remote_nodes/Load_balancing.md) 时的镜像选择概率。

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

<!-- proofread -->
