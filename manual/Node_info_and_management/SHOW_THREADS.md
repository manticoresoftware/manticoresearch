# SHOW THREADS

```sql
SHOW THREADS [ OPTION columns=width[,format=sphinxql][,format=all] ]
```

<!-- example SHOW THREADS -->

`SHOW THREADS` is an SQL statement that displays information about all threads and their current activities.

The resulting table contains the following columns:

* `TID`: ID assigned to the thread by the kernel
* `Name`: Thread name, also visible in `top`, `htop`, `ps`, and other Unix tools for monitoring thread statistics
* `Proto`: Connection protocol; possible values include `sphinx`, `mysql`, `http`, `ssl`, `compressed`, `replication`, or a combination (e.g., `http,ssl` or `compressed,mysql`)
* `State`: Thread state; possible values are `handshake`, `net_read`, `net_write`, `query`, `net_idle`
* `Connection from`: Client's `ip:port`
* `ConnID`: Connection ID (starting from 0)
* `This/prev job time`: When the thread is busy - how long the current job has been running, in seconds when the thread is idling - previous job duration + suffix  (prev)
* `Jobs done`: Number of jobs completed by this thread
* `Thread status`: Whether the stream is currently in standby mode or in run mode
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
                TID: 83
               Name: work_1
              Proto: mysql
              State: query
    Connection from: 172.17.0.1:43300
             ConnID: 8
 This/prev job time: 630us
       CPU activity: 94.15%
          Jobs done: 2490
      Thread status: working
               Info: SHOW THREADS 
*************************** 2. row ***************************
                TID: 84
               Name: work_2
              Proto: mysql
              State: query
    Connection from: 172.17.0.1:43301
             ConnID: 9
 This/prev job time: 689us
       CPU activity: 89.23%
          Jobs done: 1830
      Thread status: working
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
      "TID": {
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
      "Connection from": {
        "type": "string"
      }
    },
    {
      "ConnID": {
        "type": "string"
      }
    },
    {
      "This/prev job time": {
        "type": "string"
      }
    },
    {
      "Jobs done": {
        "type": "string"
      }
    },
    {
      "Thread status": {
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
      "TID": 83,
      "Name": "work_1",
      "Proto": "http",
      "State": "query",
      "Connection from": "172.17.0.1:41410",
      "ConnID": 6,
      "This/prev job time": "689us",
      "Jobs done": 159,
      "Thread status": "working",
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
    [83] => Array
        (
            [Name] => work_1
            [Proto] => mysql
            [State] => query
            [Connection from] => 172.17.0.1:43300
            [ConnID] => 8
            [This/prev job time] => 630us
            [CPU activity] => 94.15%
            [Jobs done] => 2490
            [Thread status] => working
            [Info] => insert into t values(0,'abc'),(0,'def')
        )

    [84] => Array
        (
            [Name] => work_2
            [Proto] => http
            [State] => query
            [Connection from] => 172.17.0.1:43301
            [ConnID] => 9
            [This/prev job time] => 689us
            [CPU activity] => 89.23%
            [Jobs done] => 1830
            [Thread status] => working
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
      u'TID': {
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
    {
      u'Connection from': {
        u'type': u'string'
      }
    },
    {
      u'ConnID': {
        u'type': u'string'
      }
    },
    {
      u'This/prev job time': {
        u'type': u'string'
      }
    },
    {
      u'CPU activity': {
        u'type': u'string'
      }
    },
    {
      u'Jobs done': {
        u'type': u'string'
      }
    },
    {
      u'Thread status': {
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
      u'TID': 83,
      u'Name': u'work_1',
      u'Proto': u'http',
      u'State': u'query',
      u'Connection from': u'172.17.0.1:43300',
      u'ConnID': 8,
      u'This/prev job time': ;'630us' ,
      u'CPU activity': u'94.15%',
      u'Jobs done': u'2490',
      u'Thread status': u'working',
      u'Info': u'show_threads'
    }
  ],
  u'total': 0,
  u'error': u'',
  u'warning': u''
}

```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('SHOW THREADS');
```

<!-- response Javascript -->

```javascript
{
  "columns": [
    {
      "TID": {
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
      "Connection from": {
        "type": "string"
      }
    },
    {
      "ConnID": {
        "type": "string"
      }
    },
    {
      "This/prev job time": {
        "type": "string"
      }
    },
    {
      "CPU activity": {
        "type": "string"
      }
    },
    {
      "Jobs done": {
        "type": "string"
      }
    },
    {
      "Thread status": {
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
      "TID": 82,
      "Name": "work_0",
      "Proto": "http",
      "State": "query",
      "Connection from": "172.17.0.1:60550",
      "ConnID": 163,
      "This/prev job time": "105us",
      "CPU activity": "44.68%",
      "Jobs done": 849,
      "Thread status": "working",
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
      TID={
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
      Connection from={
        type=string
      }
    },
    {
      ConnID={
        type=string
      }
    },
    {
      This/prev job time={
        type=string
      }
    },
    {
      CPU activity={
        type=string
      }
    },
    {
      Jobs done={
        type=string
      }
    },
    {
      Thread status={
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
      TID=82,
      Name=work_0,
      Proto=http,
      State=query,
      Connection from=172.17.0.1:60550,
      ConnID=163,
      This/prev job time=105us,
      CPU activity=44.68%,
      Jobs done=849,
      Thread status=working,
      Info=show_threads
    }
  ],
  total=0,
  error=,
  warning=
}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```java
utilsApi.Sql("SHOW THREADS");
```
<!-- response C# -->

```clike

{
  columns=[
    {
      TID={
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
      Connection from={
        type=string
      }
    },
    {
      ConnID={
        type=string
      }
    },
    {
      This/prev job time= {
        type=string
      }
    },
    {
      Jobs done={
        type=string
      }
    },
    {
      Thread status={
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
      TID=83,
      Name=work_1,
      Proto=http,
      State=query,
      Connection from=172.17.0.1:41410,
      ConnID=6,
      This/prev job time=689us,
      Jobs done=159,
      Thread status=working,
      Info=show_threads
    }
  ],
  total=0,
  error="",
  warning=""
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
               TID: 82
              Name: work_0
             Proto: mysql
             State: query
   Connection from: 172.17.0.1:60550
            ConnID: 163
This/prev job time: 105us
      CPU activity: 44.68%
         Jobs done: 849
     Thread status: working
              Info: insert into t values(0,'abc'),
*************************** 2. row ***************************
               TID: 83
              Name: work_1
             Proto: mysql
             State: query
   Connection from: 172.17.0.1:60550
            ConnID: 163
This/prev job time: 75us
      CPU activity: 52.20%
         Jobs done: 23124
    Thread status: working
             Info: SHOW THREADS OPTION columns=30
2 row in set (0.00 sec)

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
      "TID": {
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
      "Connection from": {
        "type": "string"
      }
    },
    {
      "ConnID": {
        "type": "string"
      }
    },
    {
      "This/prev job time": {
        "type": "string"
      }
    },
    {
      "CPU activity": {
        "type": "string"
      }
    },
    {
      "Jobs done": {
        "type": "string"
      }
    },
    {
      "Thread status": {
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
      "TID": 83,
      "Name": "work_1",
      "Proto": "http",
      "State": "query",
      "Connection from": "172.17.0.1:60550",
      "ConnID": 163,
      "This/prev job time": "75us",
      "CPU activity": "52.20%",
      "Jobs done": 23124,
      "Thread status": "working",
      "Info": "SHOW THREADS OPTION columns=30"
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
     [82] => Array
     (
        [TID] => 82,
        [Name] => work_0,
        [Proto] => mysql,
        [State] => query,
        [Connection from] => 172.17.0.1:60550,
        [ConnID] => 163,
        [This/prev job time] => 105us,
        [CPU activity] => 44.68%,
        [Jobs done] => 849,
        [Thread status] => working,
        [Info] => SHOW THREADS OPTION columns=30
    )
    [83] => Array
    (
        [TID] => 83,
        [Name] => work_1,
        [Proto] => mysql,
        [State] => query,
        [Connection from] => 172.17.0.1:60550,
        [ConnID] => 163,
        [This/prev job time] => 75us,
        [CPU activity] => 52.20%,
        [Jobs done] => 23124,
        [Thread status] => working,
        [Info] => SHOW THREADS OPTION columns=30
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
      u'TID': {
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
      u'Connection from': {
        u'type': u'string'
      }
    },
    {
      u'ConnID': {
        u'type': u'string'
      }
    },
    {
      u'This/prev job time': {
        u'type': u'string'
      }
    },
    {
      u'CPU activity': {
        u'type': u'string'
      }
    },
    {
      u'Jobs done': {
        u'type': u'string'
      }
    },
    {
      u'Thread status': {
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
      u'TID': 82,
      u'Name': u'work_0',
      u'Proto': u'http',
      u'State': u'query',
      u'Connection from': u'172.17.0.1:60550',
      u'ConnID': 163,
      u'This/prev job time': u'105us',
      u'CPU activity': u'44.68%',
      u'Jobs done': 849,
      u'Thread status': u'working',
      u'Info': u'SHOW THREADS OPTION columns=30'
    }
    
  ],
  u'total': 0,
  u'error': u'',
  u'warning': u''
}

```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('SHOW THREADS OPTION columns=30');
```

<!-- response Javascript -->

```javascript
{
  "columns": [
    {
      "TID": {
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
      "Connection from": {
        "type": "string"
      }
    },
    {
      "ConnID": {
        "type": "string"
      }
    },
    {
      "This/prev job time": {
        "type": "string"
      }
    },
    {
      "CPU activity": {
        "type": "string"
      }
    },
    {
      "Jobs done": {
        "type": "string"
      }
    },
    {
      "Thread status": {
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
      "TID": 6844,
      "Name": "work_3",
      "Proto": "http",
      "State": "query",
      "Connection from": "127.0.0.1:51752",
      "ConnID": 91,
      "This/prev job time": "2h",
      "CPU activity": "0us",
      "Jobs done": 1073587960,
      "Thread status": "working",
      "Info": "SHOW THREADS OPTION columns=30"
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
      TID={
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
      Connection from={
        type=string
      }
    },
    {
      ConnID={
        type=string
      }
    },
    {
      This/prev job time={
        type=string
      }
    },
    {
      CPU activity={
        type=string
      }
    },
    {
      Jobs done={
        type=string
      }
    },
    {
      Thread status={
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
      TID:=82,
      Name=work_0,
      Proto=http,
      State=query,
      Connection from=172.17.0.1:60550,
      ConnID=163,
      This/prev job time=105us,
      CPU activity=44.68%,
      Jobs done=849,
      Thread status=working,
      Info=SHOW THREADS OPTION columns=30
    }
  ],
  total=0,
  error=,
  warning=
}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("SHOW THREADS OPTION columns=30");
```
<!-- response C# -->

```clike

{
  columns=[
    {
      TID={
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
      Connection from={
        type=string
      }
    },
    {
      ConnID={
        type=string
      }
    },
    {
      This/prev job time={
        type=string
      }
    },
    {
      CPU activity={
        type=string
      }
    },
    {
      Jobs done={
        type=string
      }
    },
    {
      Thread status={
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
      TID=82,
      Name=work_0,
      Proto=http,
      State=query,
      Connection from=172.17.0.1:60550,
      ConnID=163,
      This/prev job time=105us,
      CPU activity=44.68%,
      Jobs done=849,
      Thread status=working,
      Info=SHOW THREADS OPTION columns=30
    }
  ],
  total=0,
  error="",
  warning=""
}

```

<!-- end -->
<!-- proofread -->