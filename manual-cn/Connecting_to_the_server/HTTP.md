# HTTP

您可以通过 HTTP/HTTPS 连接到 Manticore Search。

## 配置
<!-- example HTTP -->

默认情况下，Manticore 监听端口 9308 和 9312，处理 HTTP、HTTPS 和二进制请求。

在配置文件的 "searchd" 部分，您可以使用 `listen` 指令定义 HTTP 端口，如下所示：

以下两行都是有效的，且意义相同（除了端口号）。它们都定义了监听器，用于服务所有的 API/HTTP/HTTPS 协议。没有特殊要求，任何 HTTP 客户端都可以用于连接到 Manticore。

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

所有 HTTP 端点返回的内容类型均为 `application/json`。大多数端点使用 JSON 负载进行请求，但也有一些例外使用 NDJSON 或简单的 URL 编码负载。

当前没有用户身份验证。因此，请确保 HTTP 接口对网络外部的任何人不可访问。Manticore 像其他 Web 服务器一样，您可以使用 Nginx 等反向代理来实现 HTTP 认证或缓存。

<!-- example HTTPS -->

HTTP 协议还支持 [SSL 加密](../Security/SSL.md)： 如果您指定 `:https` 而不是 `:http`，则**仅接受**安全连接。否则，如果没有提供有效的密钥/证书，客户端尝试通过 https 连接时——连接将会被拒绝。如果您通过 HTTP 而非 HTTPS 请求 9443 端口，它将响应 HTTP 状态码 400。

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

### VIP 连接
<!-- example VIP -->

可以为 "VIP" 连接使用单独的 HTTP 接口。在这种情况下，连接将绕过线程池，并始终创建一个新的专用线程。这对于在服务器过载严重期间管理 Manticore Search 非常有用，此时服务器可能会停滞或不允许通过常规端口连接。

有关 `listen` 指令的更多信息，请参见[此部分](../Server_settings/Searchd.md#listen)。

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

## 通过 HTTP 发送 SQL 查询

端点 `/sql` 和 `/cli` 允许通过 HTTP 运行 SQL 查询。

- `/sql` 端点仅接受 SELECT 语句，并以 HTTP JSON 格式返回响应。
- `/sql?mode=raw` 端点接受任何 SQL 查询，并以原始格式返回响应，类似于您通过 MySQL 获得的响应。
- `/cli` 端点接受任何 SQL 查询，并以原始格式返回响应，类似于通过 MySQL 获得的响应。与 `/sql` 和 `/sql?mode=raw` 端点不同，`query` 参数不需要 URL 编码。该端点主要用于通过浏览器或命令行 HTTP 客户端（如 curl）进行手动操作，不建议在脚本中使用 `/cli` 端点。

### /sql

<!-- example SQL_over_HTTP -->

通用语法：

- `curl "localhost:6780/sql[?mode=raw]&query={URL_ENCODED_QUERY}"`
- `curl localhost:6780/sql[?mode=raw] -d "[query={URL_ENCODED_QUERY}|{NOT_URL_ENCODED_QUERY}]"`

`/sql` 端点通过 HTTP JSON 接口接受 SQL 查询：

- 没有 `mode=raw` 时 - 只允许 [SELECT 语句](../Searching/Full_text_matching/Basic_usage.md#SQL)，并以 JSON 格式返回响应。
- 使用 [mode=raw](../Connecting_to_the_server/HTTP.md#mode=raw) 时 - 允许任何 SQL 查询，并以原始格式返回响应。

该端点可以使用 GET 或 POST 方法处理 HTTP 请求。发送查询时，您可以：

1. **使用 GET：** 将查询包含在 URL 的 `query` 参数中，例如 `/sql?query=your_encoded_query_here`。**重要的是要对该参数进行 URL 编码**，以避免出现错误，特别是当查询包含 `=` 符号时，可能会被解释为 URL 语法的一部分而非查询。

2. 使用 POST：

    您也可以在 POST 请求的正文中发送查询。使用此方法时：

   - 如果您将查询作为名为 `query` 的参数发送，请确保**对其进行 URL 编码**。
   - 如果您将查询直接作为纯文本发送（原始 POST 正文），则**不要对其进行 URL 编码**。这在查询很长或复杂时非常有用，或者当查询存储在文件中时，您可以通过 HTTP 客户端（例如 `curl`）将其直接发送。

这种方法保持了 GET 和 POST 的独立使用，避免了在单个请求中组合方法的混淆。

没有 `mode=raw` 时，响应是包含匹配结果和执行时间的 JSON 格式。响应格式与 [json/search](../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) 端点相同。请注意，`/sql` 端点只支持单一搜索请求。要处理多查询请求，请参见下文有关 [raw 模式](../Connecting_to_the_server/HTTP.md#mode=raw) 的部分。

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

`/sql` 端点还包括一个特殊的 "raw" 模式，允许您发送**任何有效的 SQL 查询，包括多重查询**。响应是一个 JSON 数组，包含一个或多个结果集。您可以通过使用 `mode=raw` 选项来激活该模式。

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

> 注意：`/cli` 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保 Buddy 已安装。

虽然 `/sql` 端点适合从应用程序中以编程方式控制 Manticore，但还有一个 `/cli` 端点。它使得**手动维护 Manticore 实例**变得更加容易，可以使用 curl 或浏览器操作。该端点同时接受 POST 和 GET HTTP 方法。在 `/cli?` 之后输入的任何内容都可以被 Manticore 理解，即使它没有通过 curl 手动转义或通过浏览器自动编码。无需 `query` 参数。重要的是，`+` 符号不会被转换为空格，因此不需要对其进行编码。对于 POST 方法，Manticore 接受一切输入内容，完全不作任何更改。响应是表格格式，类似于在 MySQL 客户端中看到的 SQL 结果集。

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
<!-- example cli_json -->

`/cli_json` 端点提供与 `/cli` 相同的功能，但响应格式为 JSON。响应内容包括：

- `columns` 部分，描述了表的模式。
- `data` 部分，包含实际的数据。
- 摘要部分，包含 "total"、"error" 和 "warning" 信息。

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

HTTP 支持 Keep-alive（除了 `/cli` 端点），这允许通过 HTTP JSON 接口进行有状态的交互，前提是客户端也支持 Keep-alive。例如，使用 [/cli_json](../Connecting_to_the_server/HTTP.md#/cli_json) 端点，您可以在执行 `SELECT` 命令后运行 `SHOW META`，这将类似于通过 MySQL 客户端与 Manticore 交互的方式。


<!-- proofread -->
