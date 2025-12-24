# HTTP

Вы можете подключиться к Manticore Search через HTTP/HTTPS.

## Конфигурация
<!-- example HTTP -->
По умолчанию Manticore прослушивает HTTP, HTTPS и бинарные запросы на портах 9308 и 9312.

В разделе "searchd" вашего конфигурационного файла вы можете определить HTTP-порт с помощью директивы `listen` следующим образом:

Обе строки являются допустимыми и имеют одинаковый смысл (за исключением номера порта). Они обе определяют слушателей, которые будут обслуживать все протоколы API/HTTP/HTTPS. Нет никаких специальных требований, и для подключения к Manticore можно использовать любой HTTP-клиент.

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

Все HTTP-эндпоинты возвращают тип содержимого `application/json`. В большинстве случаев эндпоинты используют JSON-полезные данные для запросов. Однако есть некоторые исключения, которые используют NDJSON или простые URL-кодированные полезные данные.

В настоящее время аутентификация пользователя отсутствует. Поэтому убедитесь, что HTTP-интерфейс недоступен для всех за пределами вашей сети. Поскольку Manticore функционирует как любой другой веб-сервер, вы можете использовать обратный прокси, такой как Nginx, для реализации HTTP-аутентификации или кэширования.

<!-- example HTTPS -->
Протокол HTTP также поддерживает [шифрование SSL](../Security/SSL.md):
Если вы укажете `:https` вместо `:http`, будут приниматься **только** защищенные соединения. В противном случае, если действительный ключ/сертификат не предоставлен, но клиент пытается подключиться через https - соединение будет разорвано. Если вы сделаете не HTTPS, а HTTP-запрос на порт 9443, он ответит с HTTP-кодом 400.

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
Отдельный HTTP-интерфейс может использоваться для 'VIP'-соединений. В этом случае соединение обходит пул потоков и всегда создает новый выделенный поток. Это полезно для управления Manticore Search в периоды сильной перегрузки, когда сервер может зависнуть или не позволять подключения к обычному порту.

Для получения дополнительной информации о директиве `listen` см. [этот раздел](../Server_settings/Searchd.md#listen).

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

Manticore предоставляет эндпоинты `/sql`, `/cli` и `/cli_json` для выполнения SQL-запросов через HTTP. Каждый эндпоинт предназначен для конкретных случаев использования:

* `/sql`: Подходит для программного использования из приложений.
  - Эндпоинт `/sql` принимает только операторы SELECT и возвращает ответ в формате HTTP JSON.
  - Эндпоинт `/sql?mode=raw` принимает любой SQL-запрос и возвращает ответ в сыром формате, аналогично тому, что вы получили бы через mysql.
* `/cli`: Предназначен **только для ручного использования** (например, через curl или браузер). **Не рекомендуется для скриптов.**
* `/cli_json`: Аналогичен `/cli`, но возвращает результаты в формате JSON. **Не рекомендуется для скриптов.**

### /sql

<!-- example SQL_over_HTTP -->

Общий синтаксис:
* `curl "localhost:6780/sql[?mode=raw]&query={URL_ENCODED_QUERY}"`
* `curl localhost:6780/sql[?mode=raw] -d "[query={URL_ENCODED_QUERY}|{NOT_URL_ENCODED_QUERY}]"`

Эндпоинт `/sql` принимает SQL-запрос через HTTP JSON-интерфейс:
* Без `mode=raw` - разрешены только [SELECT](../Searching/Full_text_matching/Basic_usage.md#SQL), возвращающие ответ в формате JSON.
* С [mode=raw](../Connecting_to_the_server/HTTP.md#mode=raw) - разрешен любой SQL-запрос, возвращающий ответ в сыром формате.

Эндпоинт может обрабатывать HTTP-запросы с использованием либо метода GET, либо метода POST. Для отправки запросов вы можете:
1. **Используя GET:** Включите запрос в параметр `query` URL, например `/sql?query=your_encoded_query_here`. **Важно URL-кодировать** этот параметр, чтобы избежать ошибок, особенно если запрос включает знак `=`, который может быть интерпретирован как часть синтаксиса URL, а не запроса.
2. **Используя POST:** Вы также можете отправить запрос в теле POST-запроса. При использовании этого метода:
   - Если вы отправляете запрос как параметр с именем `query`, **убедитесь, что он URL-кодирован**.
   - Если вы отправляете запрос напрямую как обычный текст (сырое тело POST), **не URL-кодируйте его**. Это полезно, когда запрос длинный или сложный, или если запрос хранится в файле и вы хотите отправить его как есть, указав на него ваш HTTP-клиент (например, `curl`).

Этот подход сохраняет использование GET и POST различным и избегает путаницы в объединении методов в одном запросе.

Без `mode=raw` ответ представляет собой JSON, содержащий информацию о совпадениях и времени выполнения. Формат ответа такой же, как у эндпоинта [json/search](../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON). Обратите внимание, что эндпоинт `/sql` поддерживает только одиночные поисковые запросы. Для обработки мульти-запроса см. раздел ниже о [сыром режиме](../Connecting_to_the_server/HTTP.md#mode=raw).

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

Эндпоинт `/sql` также включает специальный "сырой" режим, который позволяет отправлять **любые допустимые SQL-запросы, включая мульти-запросы**. Ответ представляет собой JSON-массив, содержащий один или несколько наборов результатов. Вы можете активировать этот режим, используя опцию `mode=raw`.

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

> ПРИМЕЧАНИЕ: Эндпоинт `/cli` предназначен для ручного взаимодействия с Manticore с использованием таких инструментов, как curl или браузер. Он не предназначен для использования в автоматизированных скриптах. Вместо этого используйте эндпоинт `/sql`.

Хотя конечная точка `/sql` полезна для программного управления Manticore из вашего приложения, существует также конечная точка `/cli`. Это упрощает **ручное обслуживание экземпляра Manticore** с помощью curl или вашего браузера. Она принимает оба HTTP-метода POST и GET. Всё, что введено после `/cli?`, понимается Manticore, даже если оно не экранировано вручную с помощью curl или автоматически закодировано браузером. Параметр `query` не требуется. Важно, что знак `+` не преобразуется в пробел, что устраняет необходимость его кодирования. Для метода POST Manticore принимает всё точно так, как есть, без каких-либо изменений. Ответ предоставляется в табличном формате, аналогичном набору результатов SQL, который вы можете увидеть в клиенте MySQL.

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

![использование /cli в браузере](cli_browser.png)

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
> ПРИМЕЧАНИЕ: Конечная точка `/cli_json` предназначена для ручного взаимодействия с Manticore с помощью таких инструментов, как curl или браузер. Она не предназначена для использования в автоматизированных скриптах. Вместо этого используйте конечную точку `/sql`.

<!-- example cli_json -->
Конечная точка `/cli_json` предоставляет те же возможности, что и `/cli`, но формат ответа — JSON. Он включает:
- раздел `columns`, описывающий схему.
- раздел `data` с фактическими данными.
- Сводный раздел с "total", "error" и "warning".

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

### Постоянные соединения

Постоянное соединение подразумевает, что клиент отправляет не один запрос и затем разрывает соединение, а поддерживает соединение установленным и отправляет несколько запросов. В результате разрешение имён (если оно есть) происходит только один раз; также устанавливается размер TCP-окна. Более того, демон может предоставлять состояние, общее для всего соединения, например, метаинформацию и профили предыдущих запросов.

При подключении по протоколу HTTP 1.0 необходимо добавить заголовок `Connection: keep-alive` для поддержания постоянного соединения.

При подключении по протоколу HTTP 1.1 соединение по умолчанию является постоянным. В этом случае рекомендуется добавить заголовок `Connection: close` в завершающем запросе, чтобы явно сигнализировать о завершении и последующем разрыве соединения.

### Состояние HTTP

На установленном соединении демон сохраняет некоторое состояние, которое может быть предоставлено для последующих запросов. Состояние сохраняется для конечных точек `/sql`, `/sql?mode=raw` и `/cli_json`, но не для конечной точки `/cli`. Эта функция позволяет осуществлять взаимодействие с сохранением состояния через HTTP JSON интерфейс. Например, используя конечную точку [/cli_json](../Connecting_to_the_server/HTTP.md#/cli_json), вы можете выполнить команду `SHOW META` после запроса `SELECT`, и она будет вести себя аналогично взаимодействию с Manticore через клиент MySQL.

Чтобы выполнить несколько запросов с использованием sphinxql через одно соединение с помощью `curl`, вам нужно объединить ваши команды с помощью ключа
`--next`:

```
curl -s localhost:9312/cli_json -d "CALL PQ ('pq', ('{"title":"angry", "gid":3 }'))" --next localhost:9312/cli_json -d 'show meta'
```

Обратите внимание, однако, что это НЕ сработает:
```
curl -s localhost:9312/cli_json -d "CALL PQ ('pq', ('{"title":"angry", "gid":3 }')); show meta"
```

Это связано с тем, что sphinxql использует особый случай для пакетных запросов или [мультизапросов](../Searching/Multi-queries.md), которые имеют свои преимущества и ограничения.

