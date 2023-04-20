# Node status

## STATUS

<!-- example status -->
The easiest way to view high-level information about your Manticore node is by running `status` in the MySQL client. It will display information about various aspects, such as:
* Current version
* Whether SSL is in effect or not
* Current TCP port/Unix socket
* Uptime
* Number of [threads](../Server_settings/Searchd.md#threads)
* Number of [jobs in queue](../Server_settings/Searchd.md#jobs_queue_size)
* Number of connections (`clients`)
* Number of tasks being processed currently
* Number of queries executed since the start
* Number of jobs in queue and number of tasks, normalized by the number of threads

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

`SHOW STATUS` is an SQL statement that presents various helpful performance counters. IO and CPU counters will only be available if `searchd` was started with the `--iostats` and `--cpustats` switches, respectively (or if they were enabled via `SET GLOBAL iostats/cpustats=1`).

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SHOW STATUS;
```

<!-- response SQL -->
```sql
+-----------------------+---------------------------+
| Counter               | Value                     |
+-----------------------+---------------------------+
| uptime                | 1385                      |
| connections           | 11                        |
| maxed_out             | 0                         |
| version               | 3.4.3 ab7cbe5d@200511 dev |
| mysql_version         | 3.4.3 ab7cbe5d@200511 dev |
| command_search        | 2                         |
| command_excerpt       | 0                         |
| command_update        | 0                         |
| command_delete        | 0                         |
| command_keywords      | 0                         |
| command_persist       | 0                         |
| command_status        | 1                         |
| command_flushattrs    | 0                         |
| command_set           | 1                         |
| command_insert        | 0                         |
| command_replace       | 0                         |
| command_commit        | 0                         |
| command_suggest       | 0                         |
| command_json          | 0                         |
| command_callpq        | 0                         |
| agent_connect         | 0                         |
| agent_retry           | 0                         |
| queries               | 12                        |
| dist_queries          | 0                         |
| workers_total         | 30                        |
| workers_active        | 1                         |
| workers_clients       | 0                         |
| workers_clients_vip   | 1                         |
| work_queue_length     | 1                         |
| query_wall            | 10.805                    |
| query_cpu             | OFF                       |
| dist_wall             | 0.000                     |
| dist_local            | 0.000                     |
| dist_wait             | 0.000                     |
| query_reads           | OFF                       |
| query_readkb          | OFF                       |
| query_readtime        | OFF                       |
| avg_query_wall        | 0.900                     |
| avg_query_cpu         | OFF                       |
| avg_dist_wall         | 0.000                     |
| avg_dist_local        | 0.000                     |
| avg_dist_wait         | 0.000                     |
| avg_query_reads       | OFF                       |
| avg_query_readkb      | OFF                       |
| avg_query_readtime    | OFF                       |
| qcache_max_bytes      | 0                         |
| qcache_thresh_msec    | 3000                      |
| qcache_ttl_sec        | 60                        |
| qcache_cached_queries | 0                         |
| qcache_used_bytes     | 0                         |
| qcache_hits           | 0                         |
+-----------------------+---------------------------+
49 rows in set (0.00 sec)
```

<!-- end -->

<!-- example show status like -->

An optional `LIKE` clause is supported, allowing you to select only the variables that match a specific pattern. The pattern syntax follows standard SQL wildcards, where `%` represents any number of any characters, and `_` represents a single character.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SHOW STATUS LIKE 'qcache%';
```

<!-- response SQL -->
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
6 rows in set (0.00 sec)
```

<!-- end -->

## SHOW SETTINGS

<!-- example show settings -->

`SHOW SETTINGS` is an SQL statement that displays the current settings from your configuration file. The setting names are represented in the following format: `'config_section_name'.'setting_name'`

The result also includes two additional values:
- `configuration_file` - The path to the configuration file
- `worker_pid` - The process ID of the running `searchd` instance

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

`SHOW AGENT STATUS` displays the statistics of [remote agents](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) or a distributed table. It includes values such as the age of the last request, last answer, the number of various types of errors and successes, and so on. Statistics are displayed for every agent for the last 1, 5, and 15 intervals, each consisting of [ha_period_karma](../Server_settings/Searchd.md#ha_period_karma) seconds.

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
##### javascript:

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
<!-- end -->


<!-- example SHOW AGENT LIKE -->

An optional `LIKE` clause is supported, with the syntax being the same as in `SHOW STATUS`.

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
##### javascript:

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
<!-- end -->


<!-- example show specific agent -->

You can specify a particular agent by its address. In this case, only that agent's data will be displayed. Additionally, the `agent_` prefix will be used instead of `ag_N_`:

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
##### javascript:

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
<!-- end -->
<!-- example show agent table status -->

Finally, you can check the status of the agents in a specific distributed table using the `SHOW AGENT index_name STATUS` statement. This statement displays the table's HA status (i.e., whether or not it uses agent mirrors at all) and provides information on the mirrors, including: address, blackhole and persistent flags, and the mirror selection probability used when one of the [weighted probability strategies](../Creating_a_cluster/Remote_nodes/Load_balancing.md) is in effect.

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
##### javascript:

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

<!-- end -->

<!-- proofread -->
