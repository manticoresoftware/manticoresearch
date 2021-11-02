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

The response is in JSON format and contains hits information and time of execution. The response shares the same format as [json/search](../Searching/Full_text_matching/Basic_usage.md#HTTP) endpoint.

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
For comfortable debugging in your browser you can set HTTP parameter `mode` to `raw`, and then the rest of the query after 'query=' will be passed inside without any substitutions/url decoding. Here's an example of how it can fail w/o the `mode=raw`:

<!-- request HTTP -->
```bash
POST /sql -d "query=select id,packedfactors() from movies where match('star') option ranker=expr('1')"
```
<!-- response HTTP -->
```json
{"error":"query missing"}
```
<!-- end -->

<!-- example SQL_over_HTTP_3 -->
Adding `mode=raw` fixes that:

<!-- request HTTP -->
```bash
POST /sql -d "mode=raw&query=select id,packedfactors() from movies where match('star') option ranker=expr('1')"
```

<!-- response HTTP -->
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
<!-- end -->
