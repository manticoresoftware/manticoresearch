# HTTP

Вы можете подключаться к Manticore Search через HTTP/HTTPS.

## Конфигурация
<!-- example HTTP -->
По умолчанию Manticore принимает HTTP, HTTPS и бинарные запросы на портах 9308 и 9312.

В разделе "searchd" вашего конфигурационного файла вы можете определить HTTP-порт с помощью директивы `listen` следующим образом:

Обе строки допустимы и имеют одинаковое значение (за исключением номера порта). Они обе определяют слушатели, которые будут обслуживать все протоколы API/HTTP/HTTPS. Нет никаких особых требований, и для подключения к Manticore можно использовать любой HTTP-клиент.

<!-- request HTTP -->
```ini
searchd {
...
   listen = 127.0.0.1:9308
   listen = 127.0.0.1:9312:http
...
}
```
<!-- end -->

Все HTTP-конечные точки возвращают тип содержимого `application/json`. В основном, конечные точки используют JSON-формат для запросов. Однако есть исключения, которые используют NDJSON или простые URL-кодированные данные.

В настоящее время нет аутентификации пользователей. Поэтому убедитесь, что HTTP-интерфейс недоступен никому за пределами вашей сети. Поскольку Manticore функционирует как любой другой веб-сервер, вы можете использовать обратный прокси, например, Nginx, для реализации HTTP-аутентификации или кэширования.

<!-- example HTTPS -->
HTTP-протокол также поддерживает [SSL-шифрование](../Security/SSL.md):
Если вы указываете `:https` вместо `:http`, то **только** защищённые соединения будут приняты. В противном случае в случае отсутствия действительного ключа/сертификата и попытки клиента подключиться по https — соединение будет прервано. Если вы сделаете не HTTPS, а HTTP-запрос на 9443, он ответит с HTTP-кодом 400.

<!-- request HTTPS -->
```ini
searchd {
...
   listen = 127.0.0.1:9308
   listen = 127.0.0.1:9443:https
...
}
```
<!-- end -->

### VIP-соединение
<!-- example VIP -->
Отдельный HTTP-интерфейс может быть использован для 'VIP'-соединений. В этом случае соединение обходит пул потоков и всегда создаёт новый выделенный поток. Это полезно для управления Manticore Search в периоды сильной перегрузки, когда сервер может зависать или не допускать обычные подключения к порту.

Для дополнительной информации по директиве `listen` смотрите [этот раздел](../Server_settings/Searchd.md#listen).

<!-- request VIP -->
```ini
searchd {
...
   listen = 127.0.0.1:9308
   listen = 127.0.0.1:9318:_vip
...
}
```
<!-- end -->

## SQL через HTTP

Manticore предоставляет конечные точки `/sql`, `/cli` и `/cli_json` для выполнения SQL-запросов через HTTP. Каждая конечная точка предназначена для определённых случаев использования:

* `/sql`: Подходит для программного использования из приложений.
  - Конечная точка `/sql` принимает только SELECT-запросы и возвращает ответ в формате HTTP JSON.
  - Конечная точка `/sql?mode=raw` принимает любой SQL-запрос и возвращает ответ в необработанном формате, аналогично тому, что вы получили бы через mysql.
* `/cli`: Предназначена **только для ручного использования** (например, через curl или браузер). **Не рекомендуется для скриптов.**
* `/cli_json`: Аналогично `/cli`, но возвращает результаты в формате JSON. **Не рекомендуется для скриптов.**

### /sql

<!-- example SQL_over_HTTP -->

Общий синтаксис:
* `curl "localhost:6780/sql[?mode=raw]&query={URL_ENCODED_QUERY}"`
* `curl localhost:6780/sql[?mode=raw] -d "[query={URL_ENCODED_QUERY}|{NOT_URL_ENCODED_QUERY}]"`

Конечная точка `/sql` принимает SQL-запрос через HTTP JSON интерфейс:
* Без `mode=raw` разрешены только [SELECT-запросы](../Searching/Full_text_matching/Basic_usage.md#SQL), возвращающие ответ в формате JSON.
* С параметром [mode=raw](../Connecting_to_the_server/HTTP.md#mode=raw) разрешён любой SQL-запрос, возвращая ответ в необработанном формате.

Конечная точка может обрабатывать HTTP-запросы двумя методами: GET или POST. Для отправки запросов вы можете:
1. **С использованием GET:** включить запрос в параметр `query` URL, например `/sql?query=your_encoded_query_here`. Важно **закодировать URL** этот параметр, чтобы избежать ошибок, особенно если в запросе есть знак `=`, который может быть интерпретирован как часть синтаксиса URL, а не запроса.
2. **С использованием POST:** также можно отправить запрос в теле POST-запроса. При использовании этого метода:
   - Если запрос отправляется как параметр с именем `query`, **удостоверьтесь, что он URL-кодирован**.
   - Если запрос отправляется непосредственно как простой текст (сырой POST-тело), **не кодируйте URL**. Это удобно, когда запрос длинный или сложный, или если запрос хранится в файле, и вы хотите отправить его как есть, указав HTTP-клиенту (например, `curl`) путь к этому файлу.

Такой подход поддерживает раздельное использование GET и POST, избегая путаницы с объединением методов в одном запросе.

Без `mode=raw` ответ — это JSON с информацией о попаданиях и времени выполнения. Формат ответа такой же, как у конечной точки [json/search](../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON). Обратите внимание, что `/sql` поддерживает только одиночные поисковые запросы. Для обработки мультизапроса см. раздел ниже про [raw mode](../Connecting_to_the_server/HTTP.md#mode=raw).

<!-- request POST -->
```bash
POST /sql
select id,subject,author_id  from forum where match('@subject php manticore') group by author_id order by id desc limit 0,5
```

<!-- response POST -->
```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "total_relation": "eq",
    "hits": [
      {
        "_id": 2,
        "_score": 2356,
        "_source": {
          "subject": "php manticore",
          "author_id": 12
        }
      },
      {
        "_id": 1,
        "_score": 2356,
        "_source": {
          "subject": "php manticore",
          "author_id": 11
        }
      }
    ]
  }
}
```

<!-- request POST URL-encoded -->
```bash
POST /sql query=select%20id%2Csubject%2Cauthor_id%20%20from%20forum%20where%20match%28%27%40subject%20php%20manticore%27%29%20group%20by%20author_id%20order%20by%20id%20desc%20limit%200%2C5
```

<!-- response POST URL-encoded -->
```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "total_relation": "eq",
    "hits": [
      {
        "_id": 2,
        "_score": 2356,
        "_source": {
          "subject": "php manticore",
          "author_id": 12
        }
      },
      {
        "_id": 1,
        "_score": 2356,
        "_source": {
          "subject": "php manticore",
          "author_id": 11
        }
      }
    ]
  }
}
```

<!-- request GET URL-encoded -->
```bash
GET /sql?query=select%20id%2Csubject%2Cauthor_id%20%20from%20forum%20where%20match%28%27%40subject%20php%20manticore%27%29%20group%20by%20author_id%20order%20by%20id%20desc%20limit%200%2C5
```

<!-- response GET URL-encoded -->
```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "total_relation": "eq",
    "hits": [
      {
        "_id": 2,
        "_score": 2356,
        "_source": {
          "subject": "php manticore",
          "author_id": 12
        }
      },
      {
        "_id": 1,
        "_score": 2356,
        "_source": {
          "subject": "php manticore",
          "author_id": 11
        }
      }
    ]
  }
}
```

<!-- end -->

#### mode=raw

<!-- example mode=raw -->

Конечная точка `/sql` также включает специальный "raw" режим, который позволяет отправлять **любые валидные SQL-запросы, включая мультизапросы**. Ответ — это JSON-массив, содержащий один или более наборов результатов. Этот режим можно активировать, используя опцию `mode=raw`.

<!-- request POST -->
```bash
POST /sql?mode=raw
desc test
```

<!-- response POST -->
```json
[
  {
    "columns": [
      {
        "Field": {
          "type": "string"
        }
      },
      {
        "Type": {
          "type": "string"
        }
      },
      {
        "Properties": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "Field": "id",
        "Type": "bigint",
        "Properties": ""
      },
      {
        "Field": "title",
        "Type": "text",
        "Properties": "indexed"
      },
      {
        "Field": "gid",
        "Type": "uint",
        "Properties": ""
      },
      {
        "Field": "title",
        "Type": "string",
        "Properties": ""
      },
      {
        "Field": "j",
        "Type": "json",
        "Properties": ""
      },
      {
        "Field": "new1",
        "Type": "uint",
        "Properties": ""
      }
    ],
    "total": 6,
    "error": "",
    "warning": ""
  }
]
```

<!-- request POST URL-encoded -->
```bash
POST /sql?mode=raw
query=desc%20test
```

<!-- response POST URL-encoded -->
```json
[
  {
    "columns": [
      {
        "Field": {
          "type": "string"
        }
      },
      {
        "Type": {
          "type": "string"
        }
      },
      {
        "Properties": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "Field": "id",
        "Type": "bigint",
        "Properties": ""
      },
      {
        "Field": "title",
        "Type": "text",
        "Properties": "indexed"
      },
      {
        "Field": "gid",
        "Type": "uint",
        "Properties": ""
      },
      {
        "Field": "title",
        "Type": "string",
        "Properties": ""
      },
      {
        "Field": "j",
        "Type": "json",
        "Properties": ""
      },
      {
        "Field": "new1",
        "Type": "uint",
        "Properties": ""
      }
    ],
    "total": 6,
    "error": "",
    "warning": ""
  }
]
```

<!-- request POST URL-encoded 2nd way -->
```bash
POST /sql
mode=raw&query=desc%20test
```

<!-- response POST URL-encoded 2nd way -->
```json
[
  {
    "columns": [
      {
        "Field": {
          "type": "string"
        }
      },
      {
        "Type": {
          "type": "string"
        }
      },
      {
        "Properties": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "Field": "id",
        "Type": "bigint",
        "Properties": ""
      },
      {
        "Field": "title",
        "Type": "text",
        "Properties": "indexed"
      },
      {
        "Field": "gid",
        "Type": "uint",
        "Properties": ""
      },
      {
        "Field": "title",
        "Type": "string",
        "Properties": ""
      },
      {
        "Field": "j",
        "Type": "json",
        "Properties": ""
      },
      {
        "Field": "new1",
        "Type": "uint",
        "Properties": ""
      }
    ],
    "total": 6,
    "error": "",
    "warning": ""
  }
]
```

<!-- request GET URL-encoded -->
```bash
GET /sql?mode=raw&query=desc%20test
```

<!-- response GET URL-encoded -->
```json
[
  {
    "columns": [
      {
        "Field": {
          "type": "string"
        }
      },
      {
        "Type": {
          "type": "string"
        }
      },
      {
        "Properties": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "Field": "id",
        "Type": "bigint",
        "Properties": ""
      },
      {
        "Field": "title",
        "Type": "text",
        "Properties": "indexed"
      },
      {
        "Field": "gid",
        "Type": "uint",
        "Properties": ""
      },
      {
        "Field": "title",
        "Type": "string",
        "Properties": ""
      },
      {
        "Field": "j",
        "Type": "json",
        "Properties": ""
      },
      {
        "Field": "new1",
        "Type": "uint",
        "Properties": ""
      }
    ],
    "total": 6,
    "error": "",
    "warning": ""
  }
]
```

<!-- request curl examples -->
```bash
# POST:
curl localhost:9308/sql?mode=raw -d 'SHOW TABLES'

# POST, URL-encoded:
curl localhost:9308/sql?mode=raw -d 'query=SHOW%20TABLES'

# POST, URL-encoded, 2nd way:
curl localhost:9308/sql -d 'mode=raw&query=SHOW%20TABLES'

# POST, URL-non-encoded:
curl localhost:9308/sql -d 'mode=raw&query=SHOW TABLES'
```

<!-- end -->


### /cli

<!-- example cli -->

> ПРИМЕЧАНИЕ: `/cli` требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если не работает, убедитесь, что Buddy установлен.

> ПРИМЕЧАНИЕ: Конечная точка `/cli` предназначена для ручного взаимодействия с Manticore с помощью инструментов, таких как curl или браузер. Не предназначена для автоматизированных скриптов. Используйте вместо неё конечную точку `/sql`.

В то время как конечная точка `/sql` полезна для программного управления Manticore из вашего приложения, существует также конечная точка `/cli`. Это упрощает **ручное управление экземпляром Manticore** с помощью curl или вашего браузера. Она принимает как POST, так и GET HTTP методы. Все, что вводится после `/cli?`, понимается Manticore, даже если это не экранировано вручную с помощью curl или не закодировано автоматически браузером. Параметр `query` не требуется. Важно, что знак `+` не меняется на пробел, что исключает необходимость его кодирования. Для метода POST Manticore принимает все ровно так, как есть, без изменений. Ответ представлен в табличном формате, похожем на набор результатов SQL, который вы можете увидеть в MySQL клиенте.

<!-- request POST -->

```bash
POST /cli
desc test
```

<!-- response POST -->

```bash
+-------+--------+----------------+
| Field | Type   | Properties     |
+-------+--------+----------------+
| id    | bigint |                |
| body  | text   | indexed stored |
| title | string |                |
+-------+--------+----------------+
3 rows in set (0.001 sec)
```

<!-- request GET -->

```bash
GET /cli?desc%20test
```

<!-- response GET -->

```bash
+-------+--------+----------------+
| Field | Type   | Properties     |
+-------+--------+----------------+
| id    | bigint |                |
| body  | text   | indexed stored |
| title | string |                |
+-------+--------+----------------+
3 rows in set (0.001 sec)
```

<!-- request Browser -->

![using /cli in browser](cli_browser.png)

<!-- request curl example -->

```bash
curl 0:9308/cli -d 'desc test'
```

<!-- response curl example -->

```sql
+-------+--------+----------------+
| Field | Type   | Properties     |
+-------+--------+----------------+
| id    | bigint |                |
| title | text   | indexed stored |
+-------+--------+----------------+
2 rows in set (0.001 sec)
```

<!-- end -->

### /cli_json
> ПРИМЕЧАНИЕ: Конечная точка `/cli_json` предназначена для ручного взаимодействия с Manticore с помощью таких инструментов, как curl или браузер. Она не предназначена для использования в автоматизированных скриптах. Используйте вместо нее конечную точку `/sql`.

<!-- example cli_json -->
Конечная точка `/cli_json` предоставляет ту же функциональность, что и `/cli`, но формат ответа - JSON. Он включает:
- раздел `columns`, описывающий схему.
- раздел `data` с фактическими данными.
- Итоговый раздел с "total", "error" и "warning".

<!-- request POST -->

```bash
POST /cli_json
desc test
```

<!-- response POST -->

```json
[
   {
      "columns":[
         {
            "Field":{
               "type":"string"
            }
         },
         {
            "Type":{
               "type":"string"
            }
         },
         {
            "Properties":{
               "type":"string"
            }
         }
      ],
      "data":[
         {
            "Field":"id",
            "Type":"bigint",
            "Properties":""
         },
         {
            "Field":"body",
            "Type":"text",
            "Properties":"indexed stored"
         },
         {
            "Field":"title",
            "Type":"string",
            "Properties":""
         }
      ],
      "total":3,
      "error":"",
      "warning":""
   }
]
```

<!-- request GET -->

```bash
GET /cli_json?desc%20test
```

<!-- response GET -->

```json
[
   {
      "columns":[
         {
            "Field":{
               "type":"string"
            }
         },
         {
            "Type":{
               "type":"string"
            }
         },
         {
            "Properties":{
               "type":"string"
            }
         }
      ],
      "data":[
         {
            "Field":"id",
            "Type":"bigint",
            "Properties":""
         },
         {
            "Field":"body",
            "Type":"text",
            "Properties":"indexed stored"
         },
         {
            "Field":"title",
            "Type":"string",
            "Properties":""
         }
      ],
      "total":3,
      "error":"",
      "warning":""
   }
]
```

<!-- request curl example -->

```bash
curl 0:9308/cli_json -d 'desc test'
```

<!-- response curl example -->

```json
[{
"columns":[{"Field":{"type":"string"}},{"Type":{"type":"string"}},{"Properties":{"type":"string"}}],
"data":[
{"Field":"id","Type":"bigint","Properties":""},
{"Field":"body","Type":"text","Properties":"indexed stored"},
{"Field":"title","Type":"string","Properties":""}
],
"total":3,
"error":"",
"warning":""
}]
```

<!-- end -->

### Keep-alive

HTTP keep-alive поддерживается для конечных точек `/sql`, `/sql?mode=raw` и `/cli_json`, но не для конечной точки `/cli`. Эта функция позволяет выполнять состояние взаимодействия через HTTP JSON интерфейс, при условии, что клиент также поддерживает keep-alive. Например, используя конечную точку [/cli_json](../Connecting_to_the_server/HTTP.md#/cli_json), вы можете выполнить команду `SHOW META` после запроса `SELECT`, и это будет вести себя аналогично взаимодействиям с Manticore через MySQL клиент.

<!-- proofread -->

