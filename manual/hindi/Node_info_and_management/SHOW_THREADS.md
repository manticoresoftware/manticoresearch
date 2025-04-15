# थ्रेड्स दिखाएं

```sql
SHOW THREADS [ OPTION columns=width[,format=sphinxql][,format=all] ]
```

<!-- example SHOW THREADS -->

`SHOW THREADS` एक SQL स्टेटमेंट है जो सभी थ्रेड्स और उनकी वर्तमान गतिविधियों के बारे में जानकारी प्रदर्शित करता है।

परिणामी तालिका में निम्नलिखित कॉलम होते हैं:

* `TID`: कर्नेल द्वारा थ्रेड को असाइन की गई ID
* `Name`: थ्रेड का नाम, जो `top`, `htop`, `ps`, और अन्य प्रोसेस-व्यूइंग टूल्स में भी दिखाई देता है
* `Proto`: कनेक्शन प्रोटोकॉल; संभावित मान हैं `sphinx`, `mysql`, `http`, `ssl`, `compressed`, `replication`, या संयोजन (जैसे `http,ssl` या `compressed,mysql`)
* `State`: थ्रेड की स्थिति; संभावित मान हैं `handshake`, `net_read`, `net_write`, `query`, `net_idle`
* `Connection from`: क्लाइंट का `ip:port`
* `ConnID`: कनेक्शन ID (0 से शुरू)
* `This/prev job time`: जब थ्रेड व्यस्त है - वर्तमान कार्य कितने समय से चल रहा है; जब थ्रेड निष्क्रिय है - पिछले कार्य की अवधि + प्रत्यय `prev`
* `Jobs done`: इस थ्रेड द्वारा पूरे किए गए कार्यों की संख्या
* `Thread status`: `idling` या `working`
* `Info`: क्वेरी के बारे में जानकारी, जिसमें कई क्वेरीज शामिल हो सकती हैं यदि क्वेरी वितरित तालिका या रीयल-टाइम तालिका को लक्षित करती है

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
                "यह/पिछला कार्य समय, सेकंड": {
                    "type": "string"
                }
            },
            {
                "CPU गतिविधि": {
                    "type": "float"
                }
            },
            {
                "कार्य किए गए": {
                    "type": "long"
                }
            },
            {
                "थ्रेड स्थिति": {
                    "type": "string"
                }
            },
            {
                "सूचना": {
                    "type": "string"
                }
            }
        ],
        "डाटा": [
            {
                "TID": 506964,
                "नाम": "work_12",
                "प्रोटोकॉल": "http",
                "स्थिति": "query",
                "संपर्क से": "127.0.0.1:36656",
                "ConnID": 2884,
                "यह/पिछला कार्य समय, सेकंड": "236us",
                "CPU गतिविधि": "91.73%",
                "कार्य किए गए": 3328,
                "थ्रेड स्थिति": "working",
                "सूचना": "show_threads"
            }
        ],
        "कुल": 1,
        "त्रुटि": "",
        "चेतावनी": ""
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
  कॉलम=[
    {
      TID={
        type=string
      }
    },
    {
      नाम={
        type=string
      }
    },
    {
      प्रोटोकॉल={
        type=string
      }
    },
    {
      स्थिति={
        type=string
      }
    },
    {
      संपर्क से={
        type=string
      }
    },
    {
      ConnID={
        type=string
      }
    },
    {
      यह/पिछला कार्य समय={
        type=string
      }
    },
    {
      CPU गतिविधि={
        type=string
      }
    },
    {
      कार्य किए गए={
        type=string
      }
    },
    {
      थ्रेड स्थिति={
        type=string
      }
    },
    {
      सूचना={
        type=string
      }
    }
  ],
  डाटा=[
    {
      TID=82,
      नाम=work_0,
      प्रोटोकॉल=http,
      स्थिति=query,
      संपर्क से=172.17.0.1:60550,
      ConnID=163,
      यह/पिछला कार्य समय=105us,
      CPU गतिविधि=44.68%,
      कार्य किए गए=849,
      थ्रेड स्थिति=working,
      सूचना=show_threads
    }
  ],
  कुल=0,
  त्रुटि=,
  चेतावनी=
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
  कॉलम=[
    {
      TID={
        type=string
      }
    },
    {
      नाम={
        type=string
      }
    },
    {
      प्रोटोकॉल={
        type=string
      }
    },
    {
      स्थिति={
        type=string
      }
    },
    {
      संपर्क से={
        type=string
      }
    },
    {
      ConnID={
        type=string
      }
    },
    {
      यह/पिछला कार्य समय= {
        type=string
      }
    },
    {
      कार्य किए गए={
        type=string
      }
    },
    {
      थ्रेड स्थिति={
        type=string
      }
    },
    {
      सूचना={
        type=string
      }
    }
  ],
  डाटा=[
    {
      TID=83,
      नाम=work_1,
      प्रोटोकॉल=http,
      स्थिति=query,
      संपर्क से=172.17.0.1:41410,
      ConnID=6,
      यह/पिछला कार्य समय=689us,
      कार्य किए गए=159,
      थ्रेड स्थिति=working,
      सूचना=show_threads
    }
  ],
  कुल=0,
  त्रुटि="",
  चेतावनी=""
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
        "कॉलम": [
            {
                "TID": {
                    "type": "long"
                }
            },
            {
                "नाम": {
                    "type": "string"
                }
            },
            {
                "प्रोटोकॉल": {
                    "type": "string"
                }
            },
            {
                "स्थिति": {
                    "type": "string"
                }
            },
            {
                "संपर्क से": {
                    "type": "string"
                }
            },
            {
                "ConnID": {
                    "type": "long long"
                }
            },
            {
                "यह/पिछला कार्य समय, सेकंड": {
                    "type": "string"
                }
            },
            {
                "CPU गतिविधि": {
                    "type": "float"
                }
            },
            {
                "कार्य किए गए": {
                    "type": "long"
                }
            },
            {
                "थ्रेड स्थिति": {
                    "type": "string"
                }
            },
            {
                "सूचना": {
                    "type": "string"
                }
            }
        ],
        "डाटा": [
            {
                "TID": 506964,
                "नाम": "work_12",
                "प्रोटोकॉल": "http",
                "स्थिति": "query",
                "संपर्क से": "127.0.0.1:36656",
                "ConnID": 2884,
                "यह/पिछला कार्य समय, सेकंड": "236us",
                "CPU गतिविधि": "91.73%",
                "कार्य किए गए": 3328,
                "Thread status": "काम कर रहा है",
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
                "Thread status": "काम कर रहा है",
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

The `Info` column displays:

* Manticore SQL इंटरफ़ेस के माध्यम से निष्पादित प्रश्नों का कच्चा पाठ
* आंतरिक Manticore बाइनरी प्रोटोकॉल (जैसे, एक दूरस्थ Manticore इंस्टेंस से) के माध्यम से चलाए गए प्रश्नों के लिए पूर्ण पाठ वाक्यविन्यास, टिप्पणियाँ, और डेटा आकार

<!-- example SHOW THREADS WIDTH -->

आप `columns=N` विकल्प निर्दिष्ट करके `Info` कॉलम की अधिकतम चौड़ाई सीमित कर सकते हैं।

डिफ़ॉल्ट रूप से, प्रश्न उनके मूल स्वरूप में प्रदर्शित होते हैं। हालाँकि, जब `format=sphinxql` विकल्प का उपयोग किया जाता है, तो प्रश्न SQL प्रारूप में दिखाए जाएंगे, चाहे निष्पादन के लिए उपयोग किए गए प्रोटोकॉल की परवाह किए बिना।

`format=all` का उपयोग सभी थ्रेड्स को दिखाएगा, जबकि निष्क्रिय और प्रणाली थ्रेड्स बिना इस विकल्प के छिपे हुए हैं (जैसे, जो [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) में व्यस्त हैं)।

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
