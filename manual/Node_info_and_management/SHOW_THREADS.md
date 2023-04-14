# SHOW THREADS

```sql
SHOW THREADS [ OPTION columns=width[,format=sphinxql][,format=all] ]
```

<!-- example SHOW THREADS -->

`SHOW THREADS` is an SQL statement that displays information about all threads and their current activities.

The resulting table contains the following columns:

* `Tid`: ID assigned to the thread by the kernel
* `Name`: Thread name, also visible in `top`, `htop`, `ps`, and other Unix tools for monitoring thread statistics
* `Proto`: Connection protocol; possible values include `sphinx`, `mysql`, `http`, `ssl`, `compressed`, `replication`, or a combination (e.g., `http,ssl` or `compressed,mysql`)
* `State`: Thread state; possible values are `handshake`, `net_read`, `net_write`, `query`, `net_idle`
* `Host`: Client's `ip:port`
* `ConnID`: Connection ID (starting from 0)
* `Time`: Current job's duration (in seconds, with microsecond precision) or thread uptime when using `format=all` and the thread is idling
* `Work time`: Thread uptime
* `Work time CPU`: Effective CPU time (requires [`--cpustats`](../Starting_the_server/Manually.md#searchd-command-line-options))
* `Jobs done`: Number of jobs completed by this thread
* `Last job took`: Duration of the last job
* `In idle`: Whether the thread is currently idling or when it last idled
* `Info`: Information about the query, which may include multiple queries if the query targets a distributed table or a real-time table

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SHOW THREADS;
```

<!-- response SQL -->

```sql
*************************** 1. row ***************************
           Tid: 31797
          Name: work_3
         Proto: mysql
         State: query
          Host: 127.0.0.1:43388
        ConnID: 4931
          Time: 0.000903
     Work time: 2s
 Work time CPU: 0us
Thd efficiency: 0.00%
     Jobs done: 2066
 Last job took: 930us
       In idle: No (working)
          Info: insert into t values(0,'abc'),(0,'def')
*************************** 2. row ***************************
           Tid: 31799
          Name: work_5
         Proto: mysql
         State: query
          Host: 127.0.0.1:43390
        ConnID: 4932
          Time: 0.000042
     Work time: 2s
 Work time CPU: 0us
Thd efficiency: 0.00%
     Jobs done: 2072
 Last job took: 66us
       In idle: No (working)
          Info: show threads
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```http
POST /cli -d "SHOW THREADS"
```
<!-- response JSON -->
```json
{
  "columns": [
    {
      "Tid": {
        "type": "string"
      }
    },
    {
      "Name": {
        "type": "string"
      }
    },
    {
      "Proto": {
        "type": "string"
      }
    },
    {
      "State": {
        "type": "string"
      }
    },
    {
      "Host": {
        "type": "string"
      }
    },
    {
      "ConnID": {
        "type": "string"
      }
    },
    {
      "Time": {
        "type": "string"
      }
    },
    {
      "Work time": {
        "type": "string"
      }
    },
    {
      "Work time CPU": {
        "type": "string"
      }
    },
    {
      "Thd efficiency": {
        "type": "string"
      }
    },
    {
      "Jobs done": {
        "type": "string"
      }
    },
    {
      "Last job took": {
        "type": "string"
      }
    },
    {
      "In idle": {
        "type": "string"
      }
    },
    {
      "Info": {
        "type": "string"
      }
    }
  ],
  "data": [
    {
      "Tid": 6844,
      "Name": "work_3",
      "Proto": "http",
      "State": "query",
      "Host": "127.0.0.1:51752",
      "ConnID": 91,
      "Time": 3245,
      "Work time": "2h",
      "Work time CPU": "0us",
      "Thd efficiency": "0.00%",
      "Jobs done": 1073587960,
      "Last job took": "16ms",
      "In idle": "No (working)",
      "Info": "show_threads"
    }
  ],
  "total": 0,
  "error": "",
  "warning": ""
}
```
<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$client->nodes()->threads();
```

<!-- response PHP -->
```php
Array
(
    [31796] => Array
        (
            [Name] => work_2
            [Proto] => mysql
            [State] => query
            [Host] => 127.0.0.1:39320
            [ConnID] => 2897
            [Time] => 517
            [Work time] => 1s
            [Work time CPU] => 0us
            [Thd efficiency] => 0.00%
            [Jobs done] => 1228
            [Last job took] => 526us
            [In idle] => No (working)
            [Info] => insert into t values(0,'abc'),(0,'def')
        )

    [31798] => Array
        (
            [Name] => work_4
            [Proto] => http
            [State] => query
            [Host] => 127.0.0.1:37748
            [ConnID] => 2898
            [Time] => 38
            [Work time] => 883ms
            [Work time CPU] => 0us
            [Thd efficiency] => 0.00%
            [Jobs done] => 1237
            [Last job took] => 76us
            [In idle] => No (working)
            [Info] => show_threads
        )

)
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('SHOW THREADS')
```
<!-- response Python -->

```python
{
  u'columns': [
    {
      u'Tid': {
        u'type': u'string'
      }
    },
    {
      u'Name': {
        u'type': u'string'
      }
    },
    {
      u'Proto': {
        u'type': u'string'
      }
    },
    {
      u'State': {
        u'type': u'string'
      }
    },
    {
      u'Host': {
        u'type': u'string'
      }
    },
    {
      u'ConnID': {
        u'type': u'string'
      }
    },
    {
      u'Time': {
        u'type': u'string'
      }
    },
    {
      u'Work time': {
        u'type': u'string'
      }
    },
    {
      u'Work time CPU': {
        u'type': u'string'
      }
    },
    {
      u'Thd efficiency': {
        u'type': u'string'
      }
    },
    {
      u'Jobs done': {
        u'type': u'string'
      }
    },
    {
      u'Last job took': {
        u'type': u'string'
      }
    },
    {
      u'In idle': {
        u'type': u'string'
      }
    },
    {
      u'Info': {
        u'type': u'string'
      }
    }
  ],
  u'data': [
    {
      u'Tid': 6844,
      u'Name': u'work_3',
      u'Proto': u'http',
      u'State': u'query',
      u'Host': u'127.0.0.1:51752',
      u'ConnID': 91,
      u'Time': 3245,
      u'Work time': u'2h',
      u'Work time CPU': u'0us',
      u'Thd efficiency': u'0.00%',
      u'Jobs done': 1073587960,
      u'Last job took': u'16ms',
      u'In idle': u'No (working)',
      u'Info': u'show_threads'
    }
  ],
  u'total': 0,
  u'error': u'',
  u'warning': u''
}
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('SHOW THREADS');
```

<!-- response Javascript -->

```javascript
{
  "columns": [
    {
      "Tid": {
        "type": "string"
      }
    },
    {
      "Name": {
        "type": "string"
      }
    },
    {
      "Proto": {
        "type": "string"
      }
    },
    {
      "State": {
        "type": "string"
      }
    },
    {
      "Host": {
        "type": "string"
      }
    },
    {
      "ConnID": {
        "type": "string"
      }
    },
    {
      "Time": {
        "type": "string"
      }
    },
    {
      "Work time": {
        "type": "string"
      }
    },
    {
      "Work time CPU": {
        "type": "string"
      }
    },
    {
      "Thd efficiency": {
        "type": "string"
      }
    },
    {
      "Jobs done": {
        "type": "string"
      }
    },
    {
      "Last job took": {
        "type": "string"
      }
    },
    {
      "In idle": {
        "type": "string"
      }
    },
    {
      "Info": {
        "type": "string"
      }
    }
  ],
  "data": [
    {
      "Tid": 6844,
      "Name": "work_3",
      "Proto": "http",
      "State": "query",
      "Host": "127.0.0.1:51752",
      "ConnID": 91,
      "Time": 3245,
      "Work time": "2h",
      "Work time CPU": "0us",
      "Thd efficiency": "0.00%",
      "Jobs done": 1073587960,
      "Last job took": "16ms",
      "In idle": "No (working)",
      "Info": "show_threads"
    }
  ],
  "total": 0,
  "error": "",
  "warning": ""
}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("SHOW THREADS");
```
<!-- response Java -->

```java

{
  columns=[
    {
      Tid={
        type=string
      }
    },
    {
      Name={
        type=string
      }
    },
    {
      Proto={
        type=string
      }
    },
    {
      State={
        type=string
      }
    },
    {
      Host={
        type=string
      }
    },
    {
      ConnID={
        type=string
      }
    },
    {
      Time={
        type=string
      }
    },
    {
      Work time={
        type=string
      }
    },
    {
      Work time CPU={
        type=string
      }
    },
    {
      Thd efficiency={
        type=string
      }
    },
    {
      Jobs done={
        type=string
      }
    },
    {
      Last job took={
        type=string
      }
    },
    {
      In idle={
        type=string
      }
    },
    {
      Info={
        type=string
      }
    }
  ],
  data=[
    {
      Tid=6844,
      Name=work_3,
      Proto=http,
      State=query,
      Host=127.0.0.1:51752,
      ConnID=91,
      Time=3245,
      Work time=2h,
      Work time CPU=0us,
      Thd efficiency=0.00%,
      Jobs done=1073587960,
      Last job took=16ms,
      In idle=No (working),
      Info=show_threads
    }
  ],
  total=0,
  error=,
  warning=
}
```


<!-- end -->

<!-- example SHOW THREADS WIDTH -->

The `Info` column displays:

* Raw text of queries executed via the Manticore SQL interface
* Full text syntax, comments, and data size for queries run through the internal Manticore binary protocol (e.g., from a remote Manticore instance)

You can limit the maximum width of the `Info` column by specifying the `columns=N` option (note the second row in the example table).

By default, queries are displayed in their original format. However, when the `format=sphinxql` option is used, queries will be shown in SQL format, regardless of the protocol used for execution.

Using `format=all` will show all threads, while idling and system threads are hidden without this option (e.g., those busy with [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE)).

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SHOW THREADS OPTION columns=30\G
```

<!-- response SQL -->

```sql
mysql> show threads option columns=30\G
*************************** 1. row ***************************
           Tid: 9156
          Name: work_2
         Proto: mysql
         State: query
          Host: 127.0.0.1:53298
        ConnID: 20112
          Time: 0.002291
     Work time: 12s
 Work time CPU: 0us
Thd efficiency: 0.00%
     Jobs done: 8497
 Last job took: 2ms
       In idle: No (working)
          Info: insert into t values(0,'abc'),
*************************** 2. row ***************************
           Tid: 9159
          Name: work_5
         Proto: mysql
         State: query
          Host: 127.0.0.1:57698
        ConnID: 8196
          Time: 0.000042
     Work time: 11s
 Work time CPU: 0us
Thd efficiency: 0.00%
     Jobs done: 8547
 Last job took: 78us
       In idle: No (working)
          Info: show threads option columns=30
2 rows in set (0.00 sec)
```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "SHOW THREADS OPTION columns=30"
```
<!-- response JSON -->
```json
{
  "columns": [
    {
      "Tid": {
        "type": "string"
      }
    },
    {
      "Name": {
        "type": "string"
      }
    },
    {
      "Proto": {
        "type": "string"
      }
    },
    {
      "State": {
        "type": "string"
      }
    },
    {
      "Host": {
        "type": "string"
      }
    },
    {
      "ConnID": {
        "type": "string"
      }
    },
    {
      "Time": {
        "type": "string"
      }
    },
    {
      "Work time": {
        "type": "string"
      }
    },
    {
      "Work time CPU": {
        "type": "string"
      }
    },
    {
      "Thd efficiency": {
        "type": "string"
      }
    },
    {
      "Jobs done": {
        "type": "string"
      }
    },
    {
      "Last job took": {
        "type": "string"
      }
    },
    {
      "In idle": {
        "type": "string"
      }
    },
    {
      "Info": {
        "type": "string"
      }
    }
  ],
  "data": [
    {
      "Tid": 6844,
      "Name": "work_3",
      "Proto": "http",
      "State": "query",
      "Host": "127.0.0.1:51752",
      "ConnID": 91,
      "Time": 3245,
      "Work time": "2h",
      "Work time CPU": "0us",
      "Thd efficiency": "0.00%",
      "Jobs done": 1073587960,
      "Last job took": "16ms",
      "In idle": "No (working)",
      "Info": "show_threads"
    }
  ],
  "total": 0,
  "error": "",
  "warning": ""
}
```
<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$client->nodes()->threads(['body'=>['columns'=>30]]);
```

<!-- response PHP -->
```php
Array
(
    [31795] => Array
        (
            [Name] => work_1
            [Proto] => mysql
            [State] => query
            [Host] => 127.0.0.1:57676
            [ConnID] => 12075
            [Time] => 326
            [Work time] => 3s
            [Work time CPU] => 0us
            [Thd efficiency] => 0.00%
            [Jobs done] => 5092
            [Last job took] => 335us
            [In idle] => No (working)
            [Info] => insert into t values(0,'abc'),
        )

    [31797] => Array
        (
            [Name] => work_3
            [Proto] => http
            [State] => query
            [Host] => 127.0.0.1:56104
            [ConnID] => 12076
            [Time] => 33
            [Work time] => 4s
            [Work time CPU] => 0us
            [Thd efficiency] => 0.00%
            [Jobs done] => 5045
            [Last job took] => 82us
            [In idle] => No (working)
            [Info] => show_threads
        )

)
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('SHOW THREADS OPTION columns=30')
```
<!-- response Python -->

```python
{
  u'columns': [
    {
      u'Tid': {
        u'type': u'string'
      }
    },
    {
      u'Name': {
        u'type': u'string'
      }
    },
    {
      u'Proto': {
        u'type': u'string'
      }
    },
    {
      u'State': {
        u'type': u'string'
      }
    },
    {
      u'Host': {
        u'type': u'string'
      }
    },
    {
      u'ConnID': {
        u'type': u'string'
      }
    },
    {
      u'Time': {
        u'type': u'string'
      }
    },
    {
      u'Work time': {
        u'type': u'string'
      }
    },
    {
      u'Work time CPU': {
        u'type': u'string'
      }
    },
    {
      u'Thd efficiency': {
        u'type': u'string'
      }
    },
    {
      u'Jobs done': {
        u'type': u'string'
      }
    },
    {
      u'Last job took': {
        u'type': u'string'
      }
    },
    {
      u'In idle': {
        u'type': u'string'
      }
    },
    {
      u'Info': {
        u'type': u'string'
      }
    }
  ],
  u'data': [
    {
      u'Tid': 6844,
      u'Name': u'work_3',
      u'Proto': u'http',
      u'State': u'query',
      u'Host': u'127.0.0.1:51752',
      u'ConnID': 91,
      u'Time': 3245,
      u'Work time': u'2h',
      u'Work time CPU': u'0us',
      u'Thd efficiency': u'0.00%',
      u'Jobs done': 1073587960,
      u'Last job took': u'16ms',
      u'In idle': u'No (working)',
      u'Info': u'show_threads'
    }
  ],
  u'total': 0,
  u'error': u'',
  u'warning': u''
}
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('SHOW THREADS OPTION columns=30');
```

<!-- response Javascript -->

```javascript
{
  "columns": [
    {
      "Tid": {
        "type": "string"
      }
    },
    {
      "Name": {
        "type": "string"
      }
    },
    {
      "Proto": {
        "type": "string"
      }
    },
    {
      "State": {
        "type": "string"
      }
    },
    {
      "Host": {
        "type": "string"
      }
    },
    {
      "ConnID": {
        "type": "string"
      }
    },
    {
      "Time": {
        "type": "string"
      }
    },
    {
      "Work time": {
        "type": "string"
      }
    },
    {
      "Work time CPU": {
        "type": "string"
      }
    },
    {
      "Thd efficiency": {
        "type": "string"
      }
    },
    {
      "Jobs done": {
        "type": "string"
      }
    },
    {
      "Last job took": {
        "type": "string"
      }
    },
    {
      "In idle": {
        "type": "string"
      }
    },
    {
      "Info": {
        "type": "string"
      }
    }
  ],
  "data": [
    {
      "Tid": 6844,
      "Name": "work_3",
      "Proto": "http",
      "State": "query",
      "Host": "127.0.0.1:51752",
      "ConnID": 91,
      "Time": 3245,
      "Work time": "2h",
      "Work time CPU": "0us",
      "Thd efficiency": "0.00%",
      "Jobs done": 1073587960,
      "Last job took": "16ms",
      "In idle": "No (working)",
      "Info": "show_threads"
    }
  ],
  "total": 0,
  "error": "",
  "warning": ""
}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("SHOW THREADS OPTION columns=30");
```
<!-- response Java -->

```java

{
  columns=[
    {
      Tid={
        type=string
      }
    },
    {
      Name={
        type=string
      }
    },
    {
      Proto={
        type=string
      }
    },
    {
      State={
        type=string
      }
    },
    {
      Host={
        type=string
      }
    },
    {
      ConnID={
        type=string
      }
    },
    {
      Time={
        type=string
      }
    },
    {
      Work time={
        type=string
      }
    },
    {
      Work time CPU={
        type=string
      }
    },
    {
      Thd efficiency={
        type=string
      }
    },
    {
      Jobs done={
        type=string
      }
    },
    {
      Last job took={
        type=string
      }
    },
    {
      In idle={
        type=string
      }
    },
    {
      Info={
        type=string
      }
    }
  ],
  data=[
    {
      Tid=6844,
      Name=work_3,
      Proto=http,
      State=query,
      Host=127.0.0.1:51752,
      ConnID=91,
      Time=3245,
      Work time=2h,
      Work time CPU=0us,
      Thd efficiency=0.00%,
      Jobs done=1073587960,
      Last job took=16ms,
      In idle=No (working),
      Info=show_threads
    }
  ],
  total=0,
  error=,
  warning=
}
```
<!-- end -->
<!-- proofread -->