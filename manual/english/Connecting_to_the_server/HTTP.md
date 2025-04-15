# HTTP

You can connect to Manticore Search through HTTP/HTTPS.

## Configuration
<!-- example HTTP -->
By default, Manticore listens for HTTP, HTTPS, and binary requests on ports 9308 and 9312.

In the "searchd" section of your configuration file, you can define the HTTP port using the `listen` directive as follows:

Both lines are valid and have the same meaning (except for the port number). They both define listeners that will serve all API/HTTP/HTTPS protocols. There are no special requirements, and any HTTP client can be used to connect to Manticore.

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

All HTTP endpoints return `application/json` content type. For the most part, endpoints use JSON payloads for requests. However, there are some exceptions that use NDJSON or simple URL-encoded payloads.

Currently, there is no user authentication. Therefore, make sure that the HTTP interface is not accessible to anyone outside your network. As Manticore functions like any other web server, you can use a reverse proxy, such as Nginx, to implement HTTP authentication or caching.

<!-- example HTTPS -->
The HTTP protocol also supports [SSL encryption](../Security/SSL.md):
If you specify `:https` instead of `:http` **only** secured connections will be accepted. Otherwise in case of no valid key/certificate provided, but the client trying to connect via https - the connection will be dropped. If you make not HTTPS, but an HTTP request to 9443 it will respond with HTTP code 400.

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

### VIP Connection
<!-- example VIP -->
Separate HTTP interface can be used for 'VIP' connections. In this case, the connection bypasses a thread pool and always creates a new dedicated thread. This is useful for managing Manticore Search during periods of severe overload when the server might stall or not allow regular port connections.

For more information on the `listen` directive, see [this section](../Server_settings/Searchd.md#listen).

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

Manticore provides `/sql`, `/cli`, and `/cli_json` endpoints for running SQL queries over HTTP. Each endpoint is designed for specific use cases:

* `/sql`: Suitable for programmatic usage from applications.
  - The `/sql` endpoint accepts only SELECT statements and returns the response in HTTP JSON format.
  - The `/sql?mode=raw` endpoint accepts any SQL query and returns the response in raw format, similar to what you would receive via mysql.
* `/cli`: Intended **only for manual use** (e.g., via curl or browser). **Not recommended for scripts.**
* `/cli_json`: Similar to `/cli`, but returns results in JSON format. **Not recommended for scripts.**

### /sql

<!-- example SQL_over_HTTP -->

General syntax:
* `curl "localhost:6780/sql[?mode=raw]&query={URL_ENCODED_QUERY}"`
* `curl localhost:6780/sql[?mode=raw] -d "[query={URL_ENCODED_QUERY}|{NOT_URL_ENCODED_QUERY}]"`

The `/sql` endpoint accepts an SQL query via the HTTP JSON interface:
* Without `mode=raw`- only [SELECTs](../Searching/Full_text_matching/Basic_usage.md#SQL) are allowed, returning the response in JSON format.
* With [mode=raw](../Connecting_to_the_server/HTTP.md#mode=raw) - any SQL query is permitted, returning the response in raw format.

The endpoint can handle HTTP requests using either the GET or the POST method. For sending queries, you can:
1. **Using GET:** Include the query in the `query` parameter of the URL, like `/sql?query=your_encoded_query_here`. It's **important to URL encode** this parameter to avoid errors, especially if the query includes an `=` sign, which might be interpreted as part of the URL syntax rather than the query.
2. **Using POST:** You can also send the query within the body of a POST request. When using this method:
   - If you send the query as a parameter named `query`, **ensure it is URL encoded**.
   - If you send the query directly as plain text (a raw POST body), **do not URL encode it**. This is useful when the query is long or complex, or if the query is stored in a file and you want to send it as is by pointing your HTTP client (e.g., `curl`) to it.

This approach keeps the usage of GET and POST distinct and avoids any confusion about combining methods in a single request.

Without `mode=raw` the response is a JSON containing information about the hits and the execution time. The response format is the same as the [json/search](../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) endpoint. Note that the `/sql` endpoint only supports single search requests. For processing a multi-query, see the section below about the [raw mode](../Connecting_to_the_server/HTTP.md#mode=raw).

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

The `/sql` endpoint also includes a special "raw" mode, which allows you to send **any valid SQL queries, including multi-queries**. The response is a JSON array containing one or more result sets. You can activate this mode by using the option `mode=raw`.

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

> NOTE: `/cli` requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

> NOTE: The `/cli` endpoint is designed for manual interaction with Manticore using tools like curl or a browser. It is not intended for use in automated scripts. Use the `/sql` endpoint instead.

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
