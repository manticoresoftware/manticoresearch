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
If you specify `:https` instead of `:http` **only** secured connections will be accepted. Otherwise if no valid key/cert provided, but client tries to connect via https - the connection will be dropped. If you send not HTTPS, but an HTTP request to 9443 it will answer with HTTP code 400.

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

## Connecting with cURL
<!-- example CURL -->
Performing a quick search is as easy as:

<!-- request CURL -->
```bash
curl -sX POST http://localhost:9308/search -d ' {"index":"test","query":{"match":{"title":"keyword"}}}'
```
<!-- end -->

## SQL over HTTP
<!-- example SQL_over_HTTP -->
Endpoint `/sql` allows running an SQL [SELECT](../Searching/Full_text_matching/Basic_usage.md#SQL) query via HTTP JSON interface.

The query payload **must** be URL encoded, otherwise query statements with `=` (filtering or setting options) will result in an error.

The response is in JSON format and contains hits information and time of execution. The response shares the same format as [json/search](../Searching/Full_text_matching/Basic_usage.md#HTTP) endpoint. Note, that since such schema, 'naked' `/sql` endpoint supports only single 'search' requests.

<!-- request HTTP -->
```bash
POST /sql --data-urlencode "query=select id,subject,author_id  from forum where match('@subject php manticore') group by author_id order by id desc limit 0,5"
```

<!-- response HTTP -->
```json
{
  "took":10,
  "timed_out": false,
  "hits":
  {
    "total": 2,
    "hits":
    [
      {
        "_id": "1",
        "_score": 1,
        "_source": { "gid": 11 }
      },
      {
        "_id": "2",
        "_score": 1,
        "_source": { "gid": 12 }
      }
    ]
  }
}
```

<!-- end -->

<!-- example SQL_over_HTTP_2 -->
`/sql` endpoint also has special 'raw' mode, where it allows to fire any valid sphinxql queries, and also multiple queries
a time. Response format in that case is array of one or more resultsets, wrapped into json. 

<!-- request HTTP -->
```bash
POST /sql -d "mode=raw&query=desc test"
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

<!-- example SQL_over_HTTP_3 -->
Notice caveat with '+' sign. That is part of urlencoding and usually get decoded into ' ' (space). So, it need special care. Query below might fail (depending from client), because it has '+' sign, which get url-decoded into space and finally fall into invalid query:

<!-- request HTTP -->
```bash
POST /sql -d "mode=raw&query=select id,1+2 as a, packedfactors() from test where match('tes*') option ranker=expr('1')"
```

<!-- response HTTP -->
```json
{
    "error": "query missing"
}
```
<!-- end -->


<!-- example SQL_over_HTTP_4 -->
Also, for experimenting, we have special `/cli` endpoint. That is intended for fast experiments via console 'curl', or from browser line. Everything after `?` in request is just passed to sql engine. Also, '+' sign is not decoded to space, eliminating necessity of urlencoding it. Responce format is the same as for `/sql` in raw mode.

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
<!-- end -->