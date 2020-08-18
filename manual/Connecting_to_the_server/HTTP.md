# HTTP

You can connect to Manticore Search using the HTTP protocol.

## Configuration
**The HTTP protocol is by default available on ports 9308 and 9312**. It shares same port as api using multiprotocol feature.

In the searchd section of the configuration file the HTTP port can be defined with directive `listen` like this:

```ini
searchd {
...
   listen = 127.0.0.1:9308
   listen = 127.0.0.1:9312:http
...
}
```

Both lines are valid and equal by meaning (except port num). `http` is just left for back-compatibility, and both lines in example above
 defines listeners which will serve all api/http/https protocols. There are no special requirements and any HTTP client can be used to connect to Manticore. 

All endpoints respond with `application/json` content type. Most endpoints use JSON payload for requests, however there are some exceptions that use NDJSON or simple URL encoded payload.

There is no user authentication implemented at the moment, so make sure the HTTP interface is not reachable by anyone outside your network. Since Manticore acts like any other web server, you can use a reverse proxy like Nginx to add HTTP  auth or caching.

The HTTP protocol also supports [SSL encryption](Security/SSL.md).

### VIP connection
A separate HTTP interface can be used to perform 'VIP' connections. A connection to this port bypasses the thread pool and always forcibly creates a new dedicated thread. That's useful for managing in case of a severe overload when the server would either stall or not let you connect via a regular port.

```ini
searchd {
...
   listen = 127.0.0.1:9308
   listen = 127.0.0.1:9318:_vip
...
}
``` 
The HTTP protocol also supports [SSL encryption](Security/SSL.md). It may be used on the same port as http. Daemon just determines protocol by first
few bytes came from client and behaves according to it. However, https is about security, so you can strength listener by using special connection type `https`.

```ini
searchd {
...
   listen = 127.0.0.1:9308
   listen = 127.0.0.1:9443:https
...
}
``` 

Here you can connect using https to both ports. However to 9308 you may also connect using http, or provide that point as remote agent in distr indexes.
When trying to connect with http, it will just work. For https, if valid key/cert provided in config, it also will just work.
If no valid key/cert provided, but client tries to connect via https - it will silently fallback to unsecured http.
In the same time 9443 is strictly glued to https only. When trying to connect using http, it will answer with code 403.
If clients tries to connect with https, but daemon can't serve it by any reason (most probably - because it has no valid key/cert ) - it will answer
with 526 error code. No silent fall-back to unencrypted http will happen anyway.

Apart ssl encryption there is no difference between http and https.

## Connecting with cURL
Performing a quick search is as easy as:

```bash
POST /search 
    -d '{"index":"test","query":{"match":{"title":"keyword"}}}'
```

## SQL over HTTP
Endpoint `/sql` allows running an SQL [SELECT](Searching/Full_text_matching/Basic_usage.md#SQL) query via HTTP JSON interface.

The query payload **must** be URL encoded, otherwise query statements with `=` (filtering or setting options) will result in an error.

```bash
POST /sql 
--data-urlencode "query=select id,subject,author_id  from forum where match('@subject php manticore') group by
author_id order by id desc limit 0,5"
```

The response is in JSON format and contains hits information and time of execution. The response shares the same format as [json/search](Searching/Full_text_matching/Basic_usage.md#HTTP) endpoint.

```json
{
  "took":10
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

For comfortable debugging in your browser you can set HTTP parameter `mode` to `raw`, and then the rest of the query after 'query=' will be passed inside without any substitutions/url decoding.

```bash
POST /sql -d "query=select id,packedfactors() from movies where match('star') option ranker=expr('1')"
```

```json
{"error":"query missing"}
```

```bash
POST /sql -d "mode=raw&query=query=select id,packedfactors() from movies where match('star') option ranker=expr('1')"
```

```json
{
  "took":0,
  "timed_out":false,
  "hits":{
    "total":72,
    "hits":[
      {
        "_id":"5",
        "_score":1,
        "_source":{
          "packedfactors()":{
            "bm25":612,
            "bm25a":0.69104159,
            "field_mask":32,
            "doc_word_count":1,
            "fields":[
              {
                "field":5,
                "lcs":1,
                "hit_count":1,
                "word_count":1,
                "tf_idf":0.24835411,
                "min_idf":0.24835411,
                "max_idf":0.24835411,
                "sum_idf":0.24835411,
                "min_hit_pos":1,
                "min_best_span_pos":1,
                "exact_hit":0,
                "max_window_hits":1,
                "min_gaps":0,
                "exact_order":1,
                "lccs":1,
                "wlccs":0.24835411,
                "atc":0.000000
              }
            ],
            "words":[
              {
                "tf":1,
                "idf":0.24835411
              }
            ]
          }
        }
      },
...
    ]
  }
}
```
