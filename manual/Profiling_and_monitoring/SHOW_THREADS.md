# SHOW THREADS

```sql
SHOW THREADS [ OPTION columns=width[,format=sphinxql][,format=all] ]
```

<!-- example SHOW THREADS -->

`SHOW THREADS` is an SQL statement that lists all threads information about what they are doing at the moment.

It returns a table with following columns:

* `Tid`: id assigned to the thread by the kernel
* `Name`: thread name, you can also see this name in `top`, `htop`, `ps` and other unix tools to monitor thread's statistics
* `Proto`: connection protocol, possible values are `sphinx`, `mysql`, `http`, `ssl`, `compressed` and `replication` or combination (e.g. `http,ssl` or `compressed,mysql`)
* `State`: thread state, possible values are `handshake`, `net_read`, `net_write`, `query`, `net_idle`
* `Host`: `ip:port` of the client
* `ConnID`: connection id (starting from 0)
* `Time`: current job's duration (in seconds, with microsecond precision) or uptime of the thread in case of `format=all` when the thread is idling
* `Work time`: uptime of the thread
* `Work time CPU`: effective CPU time (requires [`--cpustats`](../Starting_the_server/Manually.md#searchd-command-line-options))
* `Jobs done`: how many jobs this thread has completed
* `Last job took`: last job's duration
* `In idle`: whether the thread is idling now or when it was last idling
* `Info`: information about the query, can have multiple queries in case the query is against a distributed table or a real-time table

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
##### HTTP:

<!-- request HTTP -->

```http
POST /cli -d "SHOW THREADS"
```
<!-- response HTTP -->
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

`Info` column shows:

* raw text of queries run through Manticore SQL interface
* full text syntax, comments and data size for queries run via internal Manticore binary protocol (e.g. from a remote Manticore instance)

Maximum width of the `Info` column can be limited by specifying the `columns=N` option (notice the second row in the example table).

By default, queries are shown in their original format. When the `format=sphinxql` option is specified, the queries will be shown in SQL format regardless of protocol through which they were executed.

`format=all` displays all threads, otherwise idling and system threads are hidden (e.g. those busy with [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE))

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
##### HTTP:

<!-- request HTTP -->

```http
POST /cli -d "SHOW THREADS OPTION columns=30"
```
<!-- response HTTP -->
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
