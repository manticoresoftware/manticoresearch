# SHOW THREADS

```sql
SHOW THREADS [ OPTION columns=width[,format=sphinxql][,format=all] ]
```

<!-- example SHOW THREADS -->

`SHOW THREADS` 是一个 SQL 语句，用于显示所有线程及其当前活动的详细信息。

返回的表格包含以下列：

- **TID**: 由内核分配给线程的 ID
- **Name**: 线程名称，可在 `top`、`htop`、`ps` 和其他进程查看工具中看到
- **Proto**: 连接协议，可能的值包括 `sphinx`, `mysql`, `http`, `ssl`, `compressed`, `replication`，或者它们的组合（例如，`http,ssl` 或 `compressed,mysql`）
- **State**: 线程状态，可能的值包括 `handshake`, `net_read`, `net_write`, `query`, `net_idle`
- **Connection from**: 客户端的 `ip:port`
- **ConnID**: 连接 ID（从 0 开始）
- **This/prev job time**: 当线程正在工作时，显示当前任务已运行的时间；当线程空闲时，显示上一个任务的持续时间，并附加后缀 `prev`
- **Jobs done**: 此线程已完成的任务数量
- **Thread status**: `idling`（空闲）或 `working`（工作中）
- **Info**: 查询的详细信息，如果查询目标是分布式表或实时表，可能包含多个查询

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
```
+--------+---------+-------+-------+-----------------+--------+-----------------------+-----------+---------------+--------------+
| TID    | Name    | Proto | State | Connection from | ConnID | This/prev job time, s | Jobs done | Thread status | Info         |
+--------+---------+-------+-------+-----------------+--------+-----------------------+-----------+---------------+--------------+
| 501494 | work_23 | http  | query | 127.0.0.1:41300 | 1473   | 249us                 | 1681      | working       | show_threads |
+--------+---------+-------+-------+-----------------+--------+-----------------------+-----------+---------------+--------------+
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
require_once __DIR__ . '/vendor/autoload.php';
$config = ['host'=>'127.0.0.1','port'=>9308];
$client = new \Manticoresearch\Client($config);
print_r($client->nodes()->threads());
```

<!-- response PHP -->
```php
Array
(
    [0] => Array
        (
            [TID] => 506960
            [Name] => work_8
            [Proto] => http
            [State] => query
            [Connection from] => 127.0.0.1:38072
            [ConnID] => 17
            [This/prev job time, s] => 231us
            [CPU activity] => 93.54%
            [Jobs done] => 8
            [Thread status] => working
            [Info] => show_threads
        )

)
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
import manticoresearch
config = manticoresearch.Configuration(
            host = "http://127.0.0.1:9308"
            )
client = manticoresearch.ApiClient(config)
utilsApi = manticoresearch.UtilsApi(client)
print(utilsApi.sql('SHOW THREADS'))
```
<!-- response Python -->

```python
[{'columns': [{'TID': {'type': 'long'}}, {'Name': {'type': 'string'}}, {'Proto': {'type': 'string'}}, {'State': {'type': 'string'}}, {'Connection from': {'type': 'string'}}, {'ConnID': {'type': 'long long'}}, {'This/prev job time, s': {'type': 'string'}}, {'CPU activity': {'type': 'float'}}, {'Jobs done': {'type': 'long'}}, {'Thread status': {'type': 'string'}}, {'Info': {'type': 'string'}}], 'data': [{'TID': 506958, 'Name': 'work_6', 'Proto': 'http', 'State': 'query', 'Connection from': '127.0.0.1:38600', 'ConnID': 834, 'This/prev job time, s': '206us', 'CPU activity': '91.85%', 'Jobs done': 943, 'Thread status': 'working', 'Info': 'show_threads'}], 'total': 1, 'error': '', 'warning': ''}]
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
var Manticoresearch = require('manticoresearch');

var utilsApi = new Manticoresearch.UtilsApi();
async function showThreads() {
    res = await utilsApi.sql('SHOW THREADS');
    console.log(JSON.stringify(res, null, 4));
}

showThreads();
```

<!-- response javascript -->

```javascript
[
    {
        "columns": [
            {
                "TID": {
                    "type": "long"
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
                    "type": "long long"
                }
            },
            {
                "This/prev job time, s": {
                    "type": "string"
                }
            },
            {
                "CPU activity": {
                    "type": "float"
                }
            },
            {
                "Jobs done": {
                    "type": "long"
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
                "TID": 506964,
                "Name": "work_12",
                "Proto": "http",
                "State": "query",
                "Connection from": "127.0.0.1:36656",
                "ConnID": 2884,
                "This/prev job time, s": "236us",
                "CPU activity": "91.73%",
                "Jobs done": 3328,
                "Thread status": "working",
                "Info": "show_threads"
            }
        ],
        "total": 1,
        "error": "",
        "warning": ""
    }
]
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

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->

```typescript
res = await utilsApi.sql('SHOW THREADS');
```

<!-- response TypeScript -->

```typescript
[
    {
        "columns": [
            {
                "TID": {
                    "type": "long"
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
                    "type": "long long"
                }
            },
            {
                "This/prev job time, s": {
                    "type": "string"
                }
            },
            {
                "CPU activity": {
                    "type": "float"
                }
            },
            {
                "Jobs done": {
                    "type": "long"
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
                "TID": 506964,
                "Name": "work_12",
                "Proto": "http",
                "State": "query",
                "Connection from": "127.0.0.1:36656",
                "ConnID": 2884,
                "This/prev job time, s": "236us",
                "CPU activity": "91.73%",
                "Jobs done": 3328,
                "Thread status": "working",
                "Info": "show_threads"
            }
        ],
        "total": 1,
        "error": "",
        "warning": ""
    }
]
```

<!-- intro -->
##### Go:

<!-- request Go -->

```go
apiClient.UtilsAPI.Sql(context.Background()).Body("SHOW THREADS").Execute()
```

<!-- response Go -->

```go
[
    {
        "columns": [
            {
                "TID": {
                    "type": "long"
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
                    "type": "long long"
                }
            },
            {
                "This/prev job time, s": {
                    "type": "string"
                }
            },
            {
                "CPU activity": {
                    "type": "float"
                }
            },
            {
                "Jobs done": {
                    "type": "long"
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
                "TID": 506964,
                "Name": "work_12",
                "Proto": "http",
                "State": "query",
                "Connection from": "127.0.0.1:36656",
                "ConnID": 2884,
                "This/prev job time, s": "236us",
                "CPU activity": "91.73%",
                "Jobs done": 3328,
                "Thread status": "working",
                "Info": "show_threads"
            }
        ],
        "total": 1,
        "error": "",
        "warning": ""
    }
]
```


<!-- end -->

`Info` 列显示的内容包括：

- 通过 Manticore SQL 接口执行的查询的原始文本。
- 使用内部 Manticore 二进制协议执行的查询（例如，从远程 Manticore 实例）时，显示查询的完整文本语法、注释及数据大小。

<!-- example SHOW THREADS WIDTH -->

可以通过指定 `columns=N` 选项来限制 `Info` 列的最大宽度。

默认情况下，查询将以原始格式显示。但如果使用了 `format=sphinxql` 选项，查询将以 SQL 格式显示，无论执行协议是什么。

使用 `format=all` 将显示所有线程，而不使用此选项时，空闲线程和系统线程将被隐藏（例如，那些忙于 [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) 的线程）。

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SHOW THREADS OPTION columns=30\G
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "SHOW THREADS OPTION columns=30"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$client->nodes()->threads(['body'=>['columns'=>30]]);
```

<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('SHOW THREADS OPTION columns=30')
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('SHOW THREADS OPTION columns=30');
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("SHOW THREADS OPTION columns=30");
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("SHOW THREADS OPTION columns=30");
```

<!-- intro -->
##### TypeScript:

<!-- request TypeScript -->

```typescript
res = await utilsApi.sql('SHOW THREADS OPTION columns=30');
```

<!-- intro -->
##### Go:

<!-- request Go -->

```go
apiClient.UtilsAPI.Sql(context.Background()).Body("SHOW THREADS OPTION columns=30").Execute()
```

<!-- end -->
<!-- proofread -->
