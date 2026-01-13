# HTTP

您可以通过 HTTP/HTTPS 连接到 Manticore Search。

## 配置
<!-- example HTTP -->
默认情况下，Manticore 在端口 9308 和 9312 上监听 HTTP、HTTPS 和二进制请求。

在配置文件的 "searchd" 部分，您可以使用 `listen` 指令定义 HTTP 端口，如下所示：

这两行都是有效的，含义相同（除了端口号）。它们都定义了将为所有 API/HTTP/HTTPS 协议提供服务的监听器。没有特殊要求，任何 HTTP 客户端都可以用来连接到 Manticore。

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

所有 HTTP 端点返回 `application/json` 内容类型。大多数端点使用 JSON 负载进行请求。然而，有一些例外情况使用 NDJSON 或简单的 URL 编码负载。

目前没有用户认证。因此，请确保 HTTP 接口在您的网络之外不可访问。由于 Manticore 的功能与其他 Web 服务器类似，您可以使用反向代理（如 Nginx）来实现 HTTP 认证或缓存。

<!-- example HTTPS -->
HTTP 协议也支持 [SSL 加密](../Security/SSL.md)：
如果您指定 `:https` 而不是 `:http`，**仅**接受安全连接。否则，如果未提供有效的密钥/证书，但客户端尝试通过 HTTPS 连接，连接将被丢弃。如果您向 9443 发送非 HTTPS 的 HTTP 请求，它将返回 HTTP 状态码 400。

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
可以使用单独的 HTTP 接口进行“VIP”连接。在这种情况下，连接会绕过线程池，始终创建一个新的专用线程。这在服务器可能停滞或不允许常规端口连接的严重过载期间管理 Manticore Search 时很有用。

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

## 通过 HTTP 的 SQL

Manticore 提供了 `/sql`、`/cli` 和 `/cli_json` 端点，用于通过 HTTP 运行 SQL 查询。每个端点都设计用于特定的使用场景：

* `/sql`：适用于从应用程序进行编程使用。
  - `/sql` 端点仅接受 SELECT 语句，并以 HTTP JSON 格式返回响应。
  - `/sql?mode=raw` 端点接受任何 SQL 查询，并以类似于通过 mysql 接收的原始格式返回响应。
* `/cli`：**仅适用于手动使用**（例如通过 curl 或浏览器）。**不推荐用于脚本。**
* `/cli_json`：类似于 `/cli`，但以 JSON 格式返回结果。**不推荐用于脚本。**

### /sql

<!-- example SQL_over_HTTP -->

一般语法：
* `curl "localhost:6780/sql[?mode=raw]&query={URL_ENCODED_QUERY}"`
* `curl localhost:6780/sql[?mode=raw] -d "[query={URL_ENCODED_QUERY}|{NOT_URL_ENCODED_QUERY}]"`

`/sql` 端点通过 HTTP JSON 接口接受 SQL 查询：
* 不带 `mode=raw` - 仅允许 [SELECTs](../Searching/Full_text_matching/Basic_usage.md#SQL)，返回 JSON 格式的响应。
* 带 [mode=raw](../Connecting_to_the_server/HTTP.md#mode=raw) - 允许任何 SQL 查询，返回原始格式的响应。

该端点可以使用 GET 或 POST 方法处理 HTTP 请求。发送查询时，您可以：
1. **使用 GET：** 在 URL 的 `query` 参数中包含查询，例如 `/sql?query=your_encoded_query_here`。**重要的是要对这个参数进行 URL 编码**，以避免错误，特别是如果查询包含 `=` 符号，这可能会被解释为 URL 语法的一部分，而不是查询的一部分。
2. **使用 POST：** 您也可以在 POST 请求的正文中发送查询。使用此方法时：
   - 如果您以名为 `query` 的参数发送查询，**请确保对其进行 URL 编码**。
   - 如果您直接以纯文本（原始 POST 正文）发送查询，**不要进行 URL 编码**。这在查询较长或较复杂，或者查询存储在文件中且您希望按原样发送时很有用，只需将 HTTP 客户端（例如 `curl`）指向它即可。

这种方法保持了 GET 和 POST 的使用方式不同，并避免了在单个请求中混合方法的混淆。

不带 `mode=raw` 时，响应是一个包含命中信息和执行时间的 JSON。响应格式与 [json/search](../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) 端点相同。请注意，`/sql` 端点仅支持单个搜索请求。要处理多个查询，请参见下面关于 [raw 模式](../Connecting_to_the_server/HTTP.md#mode=raw) 的部分。

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

`/sql` 端点还包括一个特殊的 "raw" 模式，允许您发送 **任何有效的 SQL 查询，包括多查询**。响应是一个包含一个或多个结果集的 JSON 数组。您可以通过使用选项 `mode=raw` 启用此模式。

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

> 注意：`/cli` 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果不起作用，请确保已安装 Buddy。

> 注意：`/cli` 端点旨在使用 curl 或浏览器等工具手动与 Manticore 交互。它不适用于自动化脚本。请改用 `/sql` 端点。

虽然 /sql 端点对于从您的应用程序以编程方式控制 Manticore 非常有用，但还有一个 /cli 端点。这使得通过 curl 或浏览器**手动维护 Manticore 实例**更加容易。它接受 POST 和 GET HTTP 方法。/cli? 之后输入的所有内容都会被 Manticore 理解，即使没有通过 curl 手动转义或浏览器自动编码。不需要 `query` 参数。重要的是，`+` 符号不会被转换为空格，因此无需对其进行编码。对于 POST 方法，Manticore 会原样接受所有内容，不会做任何更改。响应以表格格式呈现，类似于您在 MySQL 客户端中可能看到的 SQL 结果集。

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

![在浏览器中使用 /cli](cli_browser.png)

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
> 注意：/cli_json 端点是为使用 curl 或浏览器等工具手动与 Manticore 交互而设计的。它不适用于自动化脚本。请改用 /sql 端点。

<!-- example cli_json -->
/cli_json 端点提供的功能与 /cli 相同，但响应格式为 JSON。它包括：
- `columns` 部分，描述模式。
- `data` 部分，包含实际数据。
- 包含 "total"、"error" 和 "warning" 的摘要部分。

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

### 持久连接

持久连接意味着客户端保持 TCP 连接打开，并通过该连接发送多个查询，而不是为每个查询打开新连接。这避免了重复的名称解析和连接设置，并允许守护进程保留每个连接的状态，例如元信息和查询配置文件。

在 HTTP/1.0 中，添加 `Connection: keep-alive` 以请求持久连接。

在 HTTP/1.1 中，连接默认是持久的。在最后一个请求上发送 `Connection: close` 以明确结束会话。

### HTTP 状态

在持久连接上，守护进程会保留一些状态，后续查询可以使用这些状态。此状态对于 /sql、/sql?mode=raw 和 /cli_json 端点是保留的，但不适用于 /cli。这使得可以通过 HTTP JSON 进行有状态的交互。例如，当您使用 [/cli_json](../Connecting_to_the_server/HTTP.md#/cli_json) 时，可以在同一连接上 `SELECT` 之后运行 `SHOW META`，类似于使用 MySQL 客户端。

要通过一个连接使用 sphinxql 运行多个查询，您需要使用 curl 的 `--next` 键将命令链接起来：
curl -s localhost:9312/cli_json -d "CALL PQ ('pq', ('{"title":"angry", "gid":3 }'))" --next localhost:9312/cli_json -d 'show meta'

```
但是请注意，以下内容将**无法工作**：
```

curl -s localhost:9312/cli_json -d "CALL PQ ('pq', ('{"title":"angry", "gid":3 }')); show meta"
```
这是因为 Manticore 将分号分隔的 SQL 批处理视为 [多查询](../Searching/Multi-queries.md)，它有自己的行为和限制。
```

This is because Manticore treats a semicolon-separated SQL batch as a [multi-query](../Searching/Multi-queries.md), which has its own behavior and limitations.
