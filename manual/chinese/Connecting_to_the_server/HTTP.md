# HTTP

你可以通过HTTP/HTTPS连接到Manticore Search。

## 配置
<!-- example HTTP -->
默认情况下，Manticore会在端口9308和9312上监听HTTP、HTTPS和二进制请求。

在配置文件的“searchd”部分，你可以使用`listen`指令定义HTTP端口，如下所示：

两行都有同样的含义（除了端口号）。它们都定义了将为所有API/HTTP/HTTPS协议提供服务的监听器。没有特殊要求，任何HTTP客户端都可以用来连接到Manticore。

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

所有HTTP端点返回`application/json`内容类型。大多数情况下，端点使用JSON负载进行请求。但是，有一些例外使用NDJSON或简单的URL编码负载。

目前没有用户认证。因此，请确保HTTP接口对网络外部的任何人都是不可访问的。由于Manticore像其他任何Web服务器一样工作，你可以使用Nginx等反向代理来实现HTTP认证或缓存。

<!-- example HTTPS -->
HTTP协议还支持[SSL加密](../Security/SSL.md)：
如果你指定`:https`而不是`:http`，**只有**经过加密的连接才会被接受。否则，如果客户端尝试通过https连接但没有有效的密钥/证书提供，连接将被中断。如果你对9443端口进行非HTTPS的HTTP请求，它将返回HTTP状态码400。

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

### VIP连接
<!-- example VIP -->
可以使用单独的HTTP接口进行'VIP'连接。在这种情况下，连接会绕过线程池并总是创建一个新的专用线程。在服务器严重过载期间，这很有用，当服务器可能停滞或不允许常规端口连接时。

有关`listen`指令的更多信息，请参见[此处](../Server_settings/Searchd.md#listen)。

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

## SQL over HTTP

Manticore提供了`/sql`、`/cli`和`/cli_json`端点，用于通过HTTP运行SQL查询。每个端点都针对特定的使用场景设计：

* `/sql`：适合应用程序的程序化使用。
  - `/sql`端点只接受SELECT语句，并以HTTP JSON格式返回响应。
  - `/sql?mode=raw`端点接受任何SQL查询，并以原始格式返回响应，类似于通过mysql接收的响应。
* `/cli`：仅用于手动使用（例如，通过curl或浏览器）。**不建议用于脚本。**
* `/cli_json`：类似于`/cli`，但返回结果以JSON格式。**不建议用于脚本。**

### /sql

<!-- example SQL_over_HTTP -->

通用语法：
* `curl "localhost:6780/sql[?mode=raw]&query={URL_ENCODED_QUERY}"`
* `curl localhost:6780/sql[?mode=raw] -d "[query={URL_ENCODED_QUERY}|{NOT_URL_ENCODED_QUERY}]"`

`/sql`端点通过HTTP JSON接口接受SQL查询：
* 不带`mode=raw`时- 只允许[SELECTs](../Searching/Full_text_matching/Basic_usage.md#SQL)，返回JSON格式的响应。
* 带有`mode=raw` - 允许任何SQL查询，返回原始格式的响应。

该端点可以使用GET或POST方法处理HTTP请求。发送查询时，你可以：
1. **使用GET**：将查询包含在URL的`query`参数中，如`/sql?query=your_encoded_query_here`。**重要的是要对这个参数进行URL编码**，以避免错误，特别是如果查询包含`=`符号，这可能会被解释为URL语法的一部分而不是查询的一部分。
2. **使用POST**：你也可以将查询放在POST请求的主体中。使用此方法时：
   - 如果你将查询作为名为`query`的参数发送，**确保对其进行URL编码**。
   - 如果你直接将查询作为纯文本（原始POST主体）发送，**不要对其进行URL编码**。这在查询较长或复杂，或者查询存储在文件中且你想以原样发送时非常有用，可以通过指向HTTP客户端（例如`curl`）来实现。

这种方法保持了GET和POST的使用方式不同，避免了在单个请求中混淆方法。

不带`mode=raw`的响应是一个包含命中信息和执行时间的JSON。响应格式与[json/search](../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON)端点相同。注意，`/sql`端点只支持单个搜索请求。处理多查询，请参阅下面关于[原始模式](../Connecting_to_the_server/HTTP.md#mode=raw)的章节。

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

`/sql`端点还包括一个特殊的"raw"模式，允许你发送**任何有效的SQL查询，包括多查询**。通过使用`mode=raw`选项可以激活此模式。

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

> 注意：`/cli`需要[Manticore Buddy](../Installation/Manticore_Buddy.md)。如果不起作用，请确保已安装Buddy。

> 注意：`/cli`端点是使用curl或浏览器等工具与Manticore进行手动交互设计的。它不适用于自动化脚本。请使用`/sql`端点。

While the `/sql` 端点对于从应用程序中以编程方式控制 Manticore 是有用的，还有一个 `/cli` 端点。这使得使用 curl 或浏览器 **手动维护 Manticore 实例** 更加容易。它接受 POST 和 GET HTTP 方法。在 `/cli?` 之后输入的所有内容都被 Manticore 理解，即使它没有通过 curl 手动转义或由浏览器自动编码。不需要 `query` 参数。重要的是，`+` 符号不会被转换为空格，消除了编码它的必要。对于 POST 方法，Manticore 接受一切内容，没有任何更改。响应格式为表格形式，类似于 MySQL 客户端中看到的 SQL 结果集。

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

![使用 /cli 在浏览器中](cli_browser.png)

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
> 注意：`/cli_json` 端点旨在使用 curl 或浏览器等工具进行手动与 Manticore 交互。它不适用于自动化脚本。请使用 `/sql` 端点。

<!-- example cli_json -->
`/cli_json` 端点提供了与 `/cli` 相同的功能，但响应格式为 JSON。它包括：
- `columns` 部分描述了模式。
- `data` 部分包含实际数据。
- 总结部分包含 "total"、"error" 和 "warning"。

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

持久连接意味着客户端不仅发送一个查询然后断开连接，而是保持连接打开并发送多个查询。因此，如果有名称解析，它只发生一次；TCP 窗口大小也会被建立。此外，守护进程可能会提供连接范围内的状态，例如元信息和之前查询的配置文件。

通过 HTTP 1.0 协议连接时，需要添加 `Connection: keep-alive` 头来保持持久连接。

通过 HTTP 1.1 协议连接时，默认情况下连接是持久的。在这种情况下，建议在终止查询时添加 `Connection: close` 头以明确表示连接已结束并被断开。

### HTTP 状态

在已建立的连接上，守护进程会保留一些状态，这些状态可以为后续查询提供。该状态保留对于 `/sql`、`/sql?mode=raw` 和 `/cli_json` 端点有效，但不适用于 `/cli` 端点。此功能允许通过 HTTP JSON 接口进行有状态交互。例如，使用 `/cli_json` 端点，可以在 `SELECT` 查询之后执行 `SHOW META` 命令，其行为类似于通过 MySQL 客户端与 Manticore 交互。

要使用 `curl` 通过一个连接运行多个 sphinxql 查询，您需要使用 `--next` 关键字链接您的命令：
curl -s localhost:9312/cli_json -d "CALL PQ ('pq', ('{"title":"angry", "gid":3 }'))" --next localhost:9312/cli_json -d 'show meta'

```
然而，请注意，这将不起作用：
```

curl -s localhost:9312/cli_json -d "CALL PQ ('pq', ('{"title":"angry", "gid":3 }')); show meta"
```
这是因为 sphinxql 对批量查询有特殊处理，或 [multi-query](../Searching/Multi-queries.md)，它有自己的优点和限制。
```

This is because sphinxql uses a special case for batch queries, or  [multi-query](../Searching/Multi-queries.md), which has its own benefits and limitations.

