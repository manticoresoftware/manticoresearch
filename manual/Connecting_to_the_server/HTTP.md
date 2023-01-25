# HTTP

You can connect to Manticore Search over HTTP/HTTPS.

## Configuration
<!-- example HTTP -->
By default Manticore listens for HTTP, HTTPS and binary requests on ports 9308 and 9312.

In section "searchd" of your configuration file the HTTP port can be defined with directive `listen` like this:

Both lines are valid and equal by meaning (except for the port number), they both define listeners that will serve all api/http/https protocols. There are no special requirements and any HTTP client can be used to connect to Manticore.

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

All HTTP endpoints respond with `application/json` content type. Most endpoints use JSON payload for requests, however there are some exceptions that use NDJSON or simple URL encoded payload.

There is no user authentication implemented at the moment, so make sure the HTTP interface is not reachable by anyone outside your network. Since Manticore acts like any other web server, you can use a reverse proxy like Nginx to add HTTP  authentication or caching.

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

### VIP connection
<!-- example VIP -->
Separate HTTP interface can be used to perform 'VIP' connections. A connection in this case bypasses a thread pool and always forcibly creates a new dedicated thread. That's useful for managing Manticore Search in case of a severe overload when the server would either stall or not let you connect via a regular port otherwise.

Read more about `listen` in [this section](../Server_settings/Searchd.md#listen).

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

Endpoints `/sql` and `/cli` allow running SQL queries via HTTP.

* `/sql` endpoint accepts only SELECT statements and returns the response in HTTP JSON format. The query parameter should be URL-encoded.
* The `/sql?mode=raw` endpoint accepts any SQL query and returns the response in raw format, similar to what you would receive via mysql. The `query` parameter should also be URL-encoded.
* The `/cli` endpoint accepts any SQL query and returns the response in raw format, similar to what you would receive via mysql. Unlike the `/sql` and `/sql?mode=raw` endpoints, the `query` parameter should not be URL-encoded. This endpoint is intended for manual actions using a browser or command line HTTP clients such as curl. It is not recommended to use the `/cli` endpoint in scripts.


### /sql

<!-- example SQL_over_HTTP -->

`/sql` accepts an **SQL [SELECT](../Searching/Full_text_matching/Basic_usage.md#SQL) query** via HTTP JSON interface.

Query payload **must** be URL encoded, otherwise query statements with `=` (filtering or setting options) will result in an error.

It returns a JSON response which contains hits information and execution time. The response has the same format as [json/search](../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) endpoint. Note, that `/sql` endpoint supports only single search requests. If you are looking for processing a multi-query see below.

<!-- request HTTP -->
```bash
POST /sql -d "query=select%20id%2Csubject%2Cauthor_id%20%20from%20forum%20where%20match%28%27%40subject%20php%20manticore%27%29%20group%20by%20author_id%20order%20by%20id%20desc%20limit%200%2C5"
```

<!-- response HTTP -->
```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "total_relation": "eq",
    "hits": [
      {
        "_id": "2",
        "_score": 2356,
        "_source": {
          "subject": "php manticore",
          "author_id": 12
        }
      },
      {
        "_id": "1",
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

<!-- example SQL_over_HTTP_2 -->
### /sql?mode=raw

`/sql` endpoint also has a special mode **"raw"**, which allows to send **any valid sphinxql queries including multi-queries**. The returned value is a json array of one or more result sets.

<!-- request HTTP -->
```bash
POST /sql?mode=raw -d "query=desc%20test"
```

<!-- response HTTP -->
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
<!-- end -->

<!-- example SQL_over_HTTP_4 -->
### /cli
While the `/sql` endpoint is useful to control Manticore programmatically from your application, there's also endpoint `/cli` which makes it eaiser to maintain a Manticore instance via curl or your browser manually. It accepts POST and GET HTTP methods. Everything after `/cli?` is taken by Manticore as is even if you don't escape it manually via curl or let the browser encode it automaticaly. The `+` sign is not decoded to space as well, eliminating the necessity of encoding it. The response format is the same as in the `/sql?mode=raw`.

<!-- request HTTP -->

```bash
POST /cli -d "select id,1+2 as a, packedfactors() from test where match('tes*') option ranker=expr('1')"
```

<!-- response HTTP -->

```json
[
  {
    "columns": [
      {
        "id": {
          "type": "long long"
        }
      },
      {
        "a": {
          "type": "long"
        }
      },
      {
        "packedfactors()": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "id": 1,
        "a": 3,
        "packedfactors()": "bm25=616, bm25a=0.69689077, field_mask=1, doc_word_count=1, field0=(lcs=1, hit_count=1, word_count=1, tf_idf=0.25595802, min_idf=0.25595802, max_idf=0.25595802, sum_idf=0.25595802, min_hit_pos=1, min_best_span_pos=1, exact_hit=0, max_window_hits=1, min_gaps=0, exact_order=1, lccs=1, wlccs=0.25595802, atc=0.000000), word0=(tf=1, idf=0.25595802)"
      },
      {
        "id": 2,
        "a": 3,
        "packedfactors()": "bm25=616, bm25a=0.69689077, field_mask=1, doc_word_count=1, field0=(lcs=1, hit_count=1, word_count=1, tf_idf=0.25595802, min_idf=0.25595802, max_idf=0.25595802, sum_idf=0.25595802, min_hit_pos=1, min_best_span_pos=1, exact_hit=0, max_window_hits=1, min_gaps=0, exact_order=1, lccs=1, wlccs=0.25595802, atc=0.000000), word0=(tf=1, idf=0.25595802)"
      },
      {
        "id": 8,
        "a": 3,
        "packedfactors()": "bm25=616, bm25a=0.69689077, field_mask=1, doc_word_count=1, field0=(lcs=1, hit_count=1, word_count=1, tf_idf=0.25595802, min_idf=0.25595802, max_idf=0.25595802, sum_idf=0.25595802, min_hit_pos=2, min_best_span_pos=2, exact_hit=0, max_window_hits=1, min_gaps=0, exact_order=1, lccs=1, wlccs=0.25595802, atc=0.000000), word0=(tf=1, idf=0.25595802)"
      }
    ],
    "total": 3,
    "error": "",
    "warning": ""
  }
]
```

<!-- request Browser -->

![using /cli in browser](cli_browser.png)

<!-- end -->

### Keep-alive

HTTP keep-alive is also supported, which makes working via the HTTP JSON interface stateful as long as the client supports keep-alive too. For example, using the new [/cli](../Connecting_to_the_server/HTTP.md#/cli) endpoint you can call `SHOW META` after `SELECT` and it will work the same way it works via mysql.
