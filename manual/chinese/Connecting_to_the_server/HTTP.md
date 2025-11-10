# HTTP

您可以通过 HTTP/HTTPS 连接到 Manticore Search。

## 配置
<!-- example HTTP -->
默认情况下，Manticore 在端口 9308 和 9312 上监听 HTTP、HTTPS 和二进制请求。

在配置文件的 "searchd" 部分，您可以使用 `listen` 指令定义 HTTP 端口，如下所示：

这两行都是有效的，且含义相同（端口号除外）。它们都定义了将服务所有 API/HTTP/HTTPS 协议的监听器。没有特殊要求，任何 HTTP 客户端都可以用来连接 Manticore。

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

所有 HTTP 端点返回 `application/json` 内容类型。大多数情况下，端点使用 JSON 负载进行请求。然而，也有一些例外，使用 NDJSON 或简单的 URL 编码负载。

目前没有用户认证。因此，请确保 HTTP 接口不对网络外部的任何人开放。由于 Manticore 的功能类似于其他 Web 服务器，您可以使用反向代理（如 Nginx）来实现 HTTP 认证或缓存。

<!-- example HTTPS -->
HTTP 协议也支持 [SSL 加密](../Security/SSL.md)：
如果您指定 `:https` 而不是 `:http`，**只接受安全连接**。否则，如果没有提供有效的密钥/证书，但客户端尝试通过 https 连接，连接将被断开。如果您对 9443 端口发起非 HTTPS 而是 HTTP 请求，它将返回 HTTP 代码 400。

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
可以为“VIP”连接使用单独的 HTTP 接口。在这种情况下，连接绕过线程池，总是创建一个新的专用线程。这在服务器严重过载、可能停滞或不允许常规端口连接时管理 Manticore Search 非常有用。

有关 `listen` 指令的更多信息，请参见[本节](../Server_settings/Searchd.md#listen)。

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

## 通过 HTTP 使用 SQL

Manticore 提供 `/sql`、`/cli` 和 `/cli_json` 端点，用于通过 HTTP 运行 SQL 查询。每个端点针对特定用例设计：

* `/sql`：适合应用程序的编程使用。
  - `/sql` 端点仅接受 SELECT 语句，并以 HTTP JSON 格式返回响应。
  - `/sql?mode=raw` 端点接受任何 SQL 查询，并以原始格式返回响应，类似于通过 mysql 接收的格式。
* `/cli`：仅供手动使用（例如通过 curl 或浏览器）。**不推荐用于脚本。**
* `/cli_json`：类似于 `/cli`，但以 JSON 格式返回结果。**不推荐用于脚本。**

### /sql

<!-- example SQL_over_HTTP -->

通用语法：
* `curl "localhost:6780/sql[?mode=raw]&query={URL_ENCODED_QUERY}"`
* `curl localhost:6780/sql[?mode=raw] -d "[query={URL_ENCODED_QUERY}|{NOT_URL_ENCODED_QUERY}]"`

`/sql` 端点通过 HTTP JSON 接口接受 SQL 查询：
* 不带 `mode=raw` - 只允许 [SELECT](../Searching/Full_text_matching/Basic_usage.md#SQL) 语句，返回 JSON 格式响应。
* 带有 [mode=raw](../Connecting_to_the_server/HTTP.md#mode=raw) - 允许任何 SQL 查询，返回原始格式响应。

该端点可以使用 GET 或 POST 方法处理 HTTP 请求。发送查询时，您可以：
1. **使用 GET：** 将查询包含在 URL 的 `query` 参数中，如 `/sql?query=your_encoded_query_here`。**重要的是对该参数进行 URL 编码**，以避免错误，特别是当查询中包含 `=` 符号时，否则可能被解释为 URL 语法的一部分而非查询内容。
2. **使用 POST：** 您也可以在 POST 请求的正文中发送查询。使用此方法时：
   - 如果将查询作为名为 `query` 的参数发送，**确保它是 URL 编码的**。
   - 如果直接以纯文本（原始 POST 正文）发送查询，**不要进行 URL 编码**。当查询较长或复杂，或者查询存储在文件中并希望通过指向该文件的 HTTP 客户端（例如 `curl`）直接发送时，这种方式很有用。

这种方法保持了 GET 和 POST 的使用区分，避免了在单个请求中混合方法的混淆。

不带 `mode=raw` 时，响应是包含命中信息和执行时间的 JSON。响应格式与 [json/search](../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) 端点相同。请注意，`/sql` 端点仅支持单个搜索请求。有关处理多查询，请参见下面关于 [raw 模式](../Connecting_to_the_server/HTTP.md#mode=raw) 的部分。

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

`/sql` 端点还包括一个特殊的“raw”模式，允许您发送**任何有效的 SQL 查询，包括多查询**。响应是一个包含一个或多个结果集的 JSON 数组。您可以通过使用选项 `mode=raw` 来激活此模式。

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

> 注意：`/cli` 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果无法使用，请确保已安装 Buddy。

> 注意：`/cli` 端点设计用于使用 curl 或浏览器等工具手动与 Manticore 交互。不适合自动化脚本使用。请改用 `/sql` 端点。

虽然 `/sql` 端点对于从您的应用程序以编程方式控制 Manticore 非常有用，但还有 `/cli` 端点。它使得使用 curl 或浏览器**手动维护 Manticore 实例**变得更容易。它接受 POST 和 GET 两种 HTTP 方法。`/cli?` 之后输入的所有内容都会被 Manticore 理解，即使没有通过 curl 手动转义或浏览器自动编码。无需 `query` 参数。重要的是，`+` 符号不会被转换为空格，消除了对其进行编码的需求。对于 POST 方法，Manticore 接受所有内容，完全不做任何更改。响应以表格格式返回，类似于您在 MySQL 客户端中看到的 SQL 结果集。

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
> 注意：`/cli_json` 端点设计用于使用 curl 或浏览器等工具与 Manticore 进行手动交互。不适合用于自动化脚本。请改用 `/sql` 端点。

<!-- example cli_json -->
`/cli_json` 端点提供与 `/cli` 相同的功能，但响应格式为 JSON。它包括：
- 描述模式的 `columns` 部分。
- 包含实际数据的 `data` 部分。
- 包含 “total”、“error” 和 “warning” 的摘要部分。

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

HTTP keep-alive 支持 `/sql`、`/sql?mode=raw` 和 `/cli_json` 端点，但不支持 `/cli` 端点。此功能使得通过 HTTP JSON 接口进行有状态交互成为可能，前提是客户端也支持 keep-alive。例如，使用 [/cli_json](../Connecting_to_the_server/HTTP.md#/cli_json) 端点，您可以在执行 `SELECT` 查询后运行 `SHOW META` 命令，其行为类似于通过 MySQL 客户端与 Manticore 的交互。

<!-- proofread -->

