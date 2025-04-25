# ПОКАЗАТЬ ПОТОКИ

```sql
ПОКАЗАТЬ ПОТОКИ [ OPTION columns=width[,format=sphinxql][,format=all] ]
```

<!-- example SHOW THREADS -->

`ПОКАЗАТЬ ПОТОКИ` - это SQL-запрос, который отображает информацию обо всех потоках и их текущих действиях.

Результирующая таблица содержит следующие столбцы:

* `TID`: ID, присвоенный потоку ядром
* `Name`: Имя потока, также видимое в `top`, `htop`, `ps` и других инструментах просмотра процессов
* `Proto`: Протокол соединения; возможные значения включают `sphinx`, `mysql`, `http`, `ssl`, `compressed`, `replication` или комбинацию (например, `http,ssl` или `compressed,mysql`)
* `State`: Состояние потока; возможные значения: `handshake`, `net_read`, `net_write`, `query`, `net_idle`
* `Connection from`: `ip:port` клиента
* `ConnID`: ID соединения (начиная с 0)
* `This/prev job time`: Когда поток занят - как долго выполняется текущая задача; когда поток бездействует - продолжительность предыдущей задачи + суффикс `prev`
* `Jobs done`: Количество задач, завершенных этим потоком
* `Thread status`: `idling` или `working`
* `Info`: Информация о запросе, которая может включать несколько запросов, если запрос нацеливается на распределенную таблицу или таблицу в реальном времени

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
ПОКАЗАТЬ ПОТОКИ;
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
               Info: ПОКАЗАТЬ ПОТОКИ 
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
               Info: показывать потоки

```
<!-- intro -->
##### JSON:

<!-- request JSON -->

```http
POST /cli -d "ПОКАЗАТЬ ПОТОКИ"
```

<!-- response JSON -->
```
+--------+---------+-------+-------+-----------------+--------+-----------------------+-----------+---------------+--------------+
| TID    | Name    | Proto | State | Connection from | ConnID | This/prev job time, s | Jobs done | Thread status | Info         |
+--------+---------+-------+-------+-----------------+--------+-----------------------+-----------+---------------+--------------+
| 501494 | work_23 | http  | query | 127.0.0.1:41300 | 1473   | 249us                 | 1681      | working       | показывать_потоки |
+--------+---------+-------+-------+-----------------+--------+-----------------------+-----------+---------------+--------------+
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
require_once __DIR__ . '/vendor/autoload.php';
$config = ['host'=>'127.0.0.1','port'=>9308];
$client = new ManticoresearchClient($config);
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
            [Info] => показывать_потоки
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
print(utilsApi.sql('ПОКАЗАТЬ ПОТОКИ'))
```
<!-- response Python -->

```python
[{'columns': [{'TID': {'type': 'long'}}, {'Name': {'type': 'string'}}, {'Proto': {'type': 'string'}}, {'State': {'type': 'string'}}, {'Connection from': {'type': 'string'}}, {'ConnID': {'type': 'long long'}}, {'This/prev job time, s': {'type': 'string'}}, {'CPU activity': {'type': 'float'}}, {'Jobs done': {'type': 'long'}}, {'Thread status': {'type': 'string'}}, {'Info': {'type': 'string'}}], 'data': [{'TID': 506958, 'Name': 'work_6', 'Proto': 'http', 'State': 'query', 'Connection from': '127.0.0.1:38600', 'ConnID': 834, 'This/prev job time, s': '206us', 'CPU activity': '91.85%', 'Jobs done': 943, 'Thread status': 'working', 'Info': 'показывать_потоки'}], 'total': 1, 'error': '', 'warning': ''}]
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
import manticoresearch
config = manticoresearch.Configuration(
            host = "http://127.0.0.1:9308"
            )
client = manticoresearch.ApiClient(config)
utilsApi = manticoresearch.UtilsApi(client)
res = await utilsApi.sql('ПОКАЗАТЬ ПОТОКИ')
print(res)
```
<!-- response Python-asyncio -->

```python
[{'columns': [{'TID': {'type': 'long'}}, {'Name': {'type': 'string'}}, {'Proto': {'type': 'string'}}, {'State': {'type': 'string'}}, {'Connection from': {'type': 'string'}}, {'ConnID': {'type': 'long long'}}, {'This/prev job time, s': {'type': 'string'}}, {'CPU activity': {'type': 'float'}}, {'Jobs done': {'type': 'long'}}, {'Thread status': {'type': 'string'}}, {'Info': {'type': 'string'}}], 'data': [{'TID': 506958, 'Name': 'work_6', 'Proto': 'http', 'State': 'query', 'Connection from': '127.0.0.1:38600', 'ConnID': 834, 'This/prev job time, s': '206us', 'CPU activity': '91.85%', 'Jobs done': 943, 'Thread status': 'working', 'Info': 'показывать_потоки'}], 'total': 1, 'error': '', 'warning': ''}]
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
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("SHOW THREADS", Some(true)).await;
```
<!-- response Rust -->

```rust

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

Столбец `Info` отображает:

* Исходный текст запросов, выполненных через интерфейс Manticore SQL
* Синтаксис полного текста, комментарии и размер данных для запросов, выполняемых через внутренний бинарный протокол Manticore (например, из удаленного экземпляра Manticore)

<!-- example SHOW THREADS WIDTH -->

Вы можете ограничить максимальную ширину столбца `Info`, указав опцию `columns=N`.

По умолчанию запросы отображаются в их исходном формате. Однако при использовании опции `format=sphinxql` запросы будут показаны в формате SQL, независимо от протокола, используемого для выполнения.

Использование `format=all` покажет все потоки, в то время как потоки в состоянии ожидания и системные потоки скрыты без этой опции (например, те, которые заняты [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE)).

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SHOW THREADS OPTION columns=30G
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
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('SHOW THREADS OPTION columns=30')
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
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("SHOW THREADS OPTION columns=30", Some(true)).await;
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

# ПОКАЗАТЬ НИППЕЛИ

```sql
ПОКАЗАТЬ НИППЕЛИ [ ОПЦИЯ столбцы=ширина[,формат=sphinxql][,формат=все] ]
```

<!-- пример ПОКАЗАТЬ НИППЕЛИ -->

`ПОКАЗАТЬ НИППЕЛИ` - это SQL выражение, которое отображает информацию о всех нитях и их текущей активности.

Полученная таблица содержит следующие столбцы:

* `TID`: ID, присвоенный нити ядром
* `Name`: Имя нити, также видимое в `top`, `htop`, `ps` и других инструментах для просмотра процессов
* `Proto`: Протокол соединения; возможные значения включают `sphinx`, `mysql`, `http`, `ssl`, `compressed`, `replication` или комбинацию (например, `http,ssl` или `compressed,mysql`)
* `State`: Состояние нити; возможные значения: `handshake`, `net_read`, `net_write`, `query`, `net_idle`
* `Connection from`: `ip:порт` клиента
* `ConnID`: ID соединения (начиная с 0)
* `This/prev job time`: Когда нить занята - как долго выполняется текущая работа; когда нить бездействует - продолжительность предыдущей работы + суффикс `prev`
* `Jobs done`: Количество завершенных работ этой нитью
* `Thread status`: `бездействующий` или `работающий`
* `Info`: Информация о запросе, которая может включать несколько запросов, если запрос нацелен на распределенную таблицу или таблицу реального времени

<!-- ввод -->
##### SQL:
<!-- запрос SQL -->

```sql
ПОКАЗАТЬ НИППЕЛИ;
```

<!-- ответ SQL -->

```sql
*************************** 1. строка ***************************
                TID: 83
               Name: работа_1
              Proto: mysql
              State: query
    Connection from: 172.17.0.1:43300
             ConnID: 8
 This/prev job time: 630us
       CPU activity: 94.15%
          Jobs done: 2490
      Thread status: работающий
               Info: ПОКАЗАТЬ НИППЕЛИ 
*************************** 2. строка ***************************
                TID: 84
               Name: работа_2
              Proto: mysql
              State: query
    Connection from: 172.17.0.1:43301
             ConnID: 9
 This/prev job time: 689us
       CPU activity: 89.23%
          Jobs done: 1830
      Thread status: работающий
               Info: показать нити

```
<!-- ввод -->
##### JSON:

<!-- запрос JSON -->

```http
POST /cli -d "ПОКАЗАТЬ НИППЕЛИ"
```

<!-- ответ JSON -->
```
+--------+---------+-------+-------+-----------------+--------+-----------------------+-----------+---------------+--------------+
| TID    | Name    | Proto | State | Connection from | ConnID | This/prev job time, s | Jobs done | Thread status | Info         |
+--------+---------+-------+-------+-----------------+--------+-----------------------+-----------+---------------+--------------+
| 501494 | работа_23 | http  | query | 127.0.0.1:41300 | 1473   | 249us                 | 1681      | работающий       | показать_ниппели |
+--------+---------+-------+-------+-----------------+--------+-----------------------+-----------+---------------+--------------+
```

<!-- ввод -->
##### PHP:

<!-- запрос PHP -->

```php
require_once __DIR__ . '/vendor/autoload.php';
$config = ['host'=>'127.0.0.1','port'=>9308];
$client = new \Manticoresearch\Client($config);
print_r($client->nodes()->threads());
```

<!-- ответ PHP -->
```php
Array
(
    [0] => Array
        (
            [TID] => 506960
            [Name] => работа_8
            [Proto] => http
            [State] => query
            [Connection from] => 127.0.0.1:38072
            [ConnID] => 17
            [This/prev job time, s] => 231us
            [CPU activity] => 93.54%
            [Jobs done] => 8
            [Thread status] => работающий
            [Info] => показать_ниппели
        )

)
```
<!-- ввод -->
##### Python:

<!-- запрос Python -->

```python
import manticoresearch
config = manticoresearch.Configuration(
            host = "http://127.0.0.1:9308"
            )
client = manticoresearch.ApiClient(config)
utilsApi = manticoresearch.UtilsApi(client)
print(utilsApi.sql('ПОКАЗАТЬ НИППЕЛИ'))
```
<!-- ответ Python -->

```python
[{'columns': [{'TID': {'type': 'long'}}, {'Name': {'type': 'string'}}, {'Proto': {'type': 'string'}}, {'State': {'type': 'string'}}, {'Connection from': {'type': 'string'}}, {'ConnID': {'type': 'long long'}}, {'This/prev job time, s': {'type': 'string'}}, {'CPU activity': {'type': 'float'}}, {'Jobs done': {'type': 'long'}}, {'Thread status': {'type': 'string'}}, {'Info': {'type': 'string'}}], 'data': [{'TID': 506958, 'Name': 'работа_6', 'Proto': 'http', 'State': 'query', 'Connection from': '127.0.0.1:38600', 'ConnID': 834, 'This/prev job time, s': '206us', 'CPU activity': '91.85%', 'Jobs done': 943, 'Thread status': 'работающий', 'Info': 'показать_ниппели'}], 'total': 1, 'error': '', 'warning': ''}]
```
<!-- ввод -->
##### Javascript:

<!-- запрос javascript -->

```javascript
var Manticoresearch = require('manticoresearch');

var utilsApi = new Manticoresearch.UtilsApi();
async function showThreads() {
    res = await utilsApi.sql('ПОКАЗАТЬ НИППЕЛИ');
    console.log(JSON.stringify(res, null, 4));
}

showThreads();
```

<!-- ответ javascript -->

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
                "Thread status": "работает",
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
                "Thread status": "работает",
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

Столбец `Info` отображает:

* Исходный текст запросов, выполненных через интерфейс Manticore SQL
* Полный синтаксис текста, комментарии и размер данных для запросов, выполненных через внутренний бинарный протокол Manticore (например, из удаленного экземпляра Manticore)

<!-- example SHOW THREADS WIDTH -->

Вы можете ограничить максимальную ширину столбца `Info`, указав опцию `columns=N`.

По умолчанию запросы отображаются в их исходном формате. Однако, когда используется опция `format=sphinxql`, запросы будут показаны в формате SQL, независимо от протокола, используемого для выполнения.

Использование `format=all` покажет все потоки, в то время как простаивающие и системные потоки скрыты без этой опции (например, те, которые заняты [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE)).

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
