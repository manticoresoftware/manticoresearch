# HTTP

Вы можете подключаться к Manticore Search через HTTP/HTTPS.

## Конфигурация
<!-- example HTTP -->
По умолчанию Manticore слушает HTTP, HTTPS и бинарные запросы на портах 9308 и 9312.

В разделе "searchd" вашего конфигурационного файла вы можете определить HTTP-порт с помощью директивы `listen` следующим образом:

Обе строки допустимы и имеют одинаковое значение (за исключением номера порта). Обе они определяют слушатели, которые будут обслуживать все протоколы API/HTTP/HTTPS. Нет никаких специальных требований, и любой HTTP-клиент может использоваться для подключения к Manticore.

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

Все HTTP-эндпоинты возвращают контент типа `application/json`. В основном, эндпоинты используют JSON-пейлоады для запросов. Однако есть некоторые исключения, которые используют NDJSON или простые URL-кодированные пейлоады.

В настоящее время нет аутентификации пользователей. Поэтому убедитесь, что HTTP-интерфейс недоступен для всех вне вашей сети. Поскольку Manticore функционирует как любой другой веб-сервер, вы можете использовать обратный прокси, например Nginx, для реализации HTTP-аутентификации или кэширования.

<!-- example HTTPS -->
HTTP-протокол также поддерживает [SSL-шифрование](../Security/SSL.md):
Если вы укажете `:https` вместо `:http`, **будут приниматься только защищённые соединения**. В противном случае, если не предоставлен действительный ключ/сертификат, а клиент пытается подключиться через https — соединение будет разорвано. Если вы сделаете не HTTPS, а HTTP-запрос на 9443, он ответит HTTP-кодом 400.

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

### VIP-подключение
<!-- example VIP -->
Отдельный HTTP-интерфейс может использоваться для 'VIP' подключений. В этом случае соединение обходит пул потоков и всегда создаёт новый выделенный поток. Это полезно для управления Manticore Search в периоды сильной перегрузки, когда сервер может зависать или не разрешать обычные подключения к порту.

Для получения дополнительной информации о директиве `listen` смотрите [этот раздел](../Server_settings/Searchd.md#listen).

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

Manticore предоставляет эндпоинты `/sql`, `/cli` и `/cli_json` для выполнения SQL-запросов через HTTP. Каждый эндпоинт предназначен для определённых случаев использования:

* `/sql`: Подходит для программного использования из приложений.
  - Эндпоинт `/sql` принимает только SELECT-запросы и возвращает ответ в формате HTTP JSON.
  - Эндпоинт `/sql?mode=raw` принимает любой SQL-запрос и возвращает ответ в сыром формате, аналогично тому, что вы получили бы через mysql.
* `/cli`: Предназначен **только для ручного использования** (например, через curl или браузер). **Не рекомендуется для скриптов.**
* `/cli_json`: Похож на `/cli`, но возвращает результаты в формате JSON. **Не рекомендуется для скриптов.**

### /sql

<!-- example SQL_over_HTTP -->

Общий синтаксис:
* `curl "localhost:6780/sql[?mode=raw]&query={URL_ENCODED_QUERY}"`
* `curl localhost:6780/sql[?mode=raw] -d "[query={URL_ENCODED_QUERY}|{NOT_URL_ENCODED_QUERY}]"`

Эндпоинт `/sql` принимает SQL-запрос через HTTP JSON интерфейс:
* Без `mode=raw` разрешены только [SELECT](../Searching/Full_text_matching/Basic_usage.md#SQL), возвращая ответ в формате JSON.
* С [mode=raw](../Connecting_to_the_server/HTTP.md#mode=raw) разрешён любой SQL-запрос, возвращая ответ в сыром формате.

Эндпоинт может обрабатывать HTTP-запросы как методом GET, так и POST. Для отправки запросов вы можете:
1. **Используя GET:** Включите запрос в параметр `query` URL, например `/sql?query=your_encoded_query_here`. Важно **кодировать этот параметр в URL**, чтобы избежать ошибок, особенно если запрос содержит знак `=`, который может быть интерпретирован как часть синтаксиса URL, а не запроса.
2. **Используя POST:** Вы также можете отправить запрос в теле POST-запроса. При использовании этого метода:
   - Если вы отправляете запрос как параметр с именем `query`, **убедитесь, что он URL-кодирован**.
   - Если вы отправляете запрос напрямую как простой текст (сырой POST body), **не кодируйте его в URL**. Это полезно, когда запрос длинный или сложный, или если запрос хранится в файле и вы хотите отправить его как есть, указав HTTP-клиенту (например, `curl`) на этот файл.

Такой подход сохраняет различие между использованием GET и POST и избегает путаницы при комбинировании методов в одном запросе.

Без `mode=raw` ответ — это JSON, содержащий информацию о найденных совпадениях и времени выполнения. Формат ответа такой же, как у эндпоинта [json/search](../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON). Обратите внимание, что эндпоинт `/sql` поддерживает только одиночные поисковые запросы. Для обработки мультизапроса смотрите раздел ниже о [сыром режиме](../Connecting_to_the_server/HTTP.md#mode=raw).

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

Эндпоинт `/sql` также включает специальный "сырой" режим, который позволяет отправлять **любые валидные SQL-запросы, включая мультизапросы**. Ответ — это JSON-массив, содержащий один или несколько наборов результатов. Вы можете активировать этот режим, используя опцию `mode=raw`.

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

> ПРИМЕЧАНИЕ: `/cli` требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если он не работает, убедитесь, что Buddy установлен.

> ПРИМЕЧАНИЕ: Эндпоинт `/cli` предназначен для ручного взаимодействия с Manticore с помощью таких инструментов, как curl или браузер. Он не предназначен для использования в автоматизированных скриптах. Используйте вместо него эндпоинт `/sql`.

While the `/sql` endpoint is useful for controlling Manticore programmatically from your application, there's also the `/cli` endpoint. This makes it easier to **manually maintain a Manticore instance** using curl or your browser. It accepts both POST and GET HTTP methods. Everything inputted after `/cli?` is understood by Manticore, even if it's not manually escaped with curl or automatically encoded by the browser. No `query` parameter is required. Importantly, the `+` sign is not changed to a space, eliminating the need for encoding it. For the POST method, Manticore accepts everything exactly as it is, without any changes. The response is in tabular format, similar to an SQL result set you might see in a MySQL client.

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
> NOTE: The `/cli_json` endpoint is designed for manual interaction with Manticore using tools like curl or a browser. It is not intended for use in automated scripts. Use the `/sql` endpoint instead.

<!-- example cli_json -->
The `/cli_json` endpoint provides the same functionality as `/cli`, but the response format is JSON. It includes:
- `columns` section describing the schema.
- `data` section with the actual data.
- Summary section with "total", "error", and "warning".

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

HTTP keep-alive is supported for the `/sql`, `/sql?mode=raw`, and `/cli_json` endpoints, but not for the `/cli` endpoint. This feature enables stateful interactions via the HTTP JSON interface, provided the client also supports keep-alive. For example, using the [/cli_json](../Connecting_to_the_server/HTTP.md#/cli_json) endpoint, you can run a `SHOW META` command after a `SELECT` query, and it will behave similarly to interactions with Manticore through a MySQL client.

<!-- proofread -->

