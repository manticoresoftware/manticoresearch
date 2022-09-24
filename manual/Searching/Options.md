# Search options

SQL [SELECT](../Searching/Full_text_matching/Basic_usage.md#SQL) clause and HTTP [/search](../Searching/Full_text_matching/Basic_usage.md#HTTP) endpoint support a number of options that can be used to fine-tune search behaviour.

## OPTION

### General syntax

<!-- example options -->

**SQL**:

```sql
SELECT ... [OPTION <optionname>=<value> [ , ... ]] [FORCE|IGNORE INDEX(id)]
```

**HTTP**:
```json
POST /search
{   
    "index" : "index_name",
    "options":   
    {
        "optionname": "value",
        "optionname2": <value2>
    }
}
```


<!-- intro -->
SQL:
<!-- request SQL -->
```sql
SELECT * FROM test WHERE MATCH('@title hello @body world')
OPTION ranker=bm25, max_matches=3000,
field_weights=(title=10, body=3), agent_query_timeout=10000
```

<!-- response SQL -->
```sql
+------+-------+-------+
| id   | title | body  |
+------+-------+-------+
|    1 | hello | world |
+------+-------+-------+
1 row in set (0.00 sec)
```

<!-- intro -->
HTTP:
<!-- request HTTP -->

```json
POST /search
{   
    "index" : "test",
    "query": {
      "match": {
        "title": "hello"
      },
      "match": {
        "body": "world"     
      }
    },
    "options":   
    {
        "ranker": "bm25",
        "max_matches": 3000,
        "field_weights": {
            "title": 10,
            "body": 3
        },
        "agent_query_timeout": 10000
    }
}
```

<!-- response HTTP -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "total_relation": "eq",
    "hits": [
      {
        "_id": "1",
        "_score": 10500,
        "_source": {
          "title": "hello",
          "body": "world"
        }
      }
    ]
  }
}
```

<!-- end -->

Supported options are:

### agent_query_timeout
Integer. Max time in milliseconds to wait for remote queries to complete, see [this section](../Creating_an_index/Creating_a_distributed_index/Remote_indexes.md#agent_query_timeout).

### boolean_simplify
`0` or `1` (`0` by default). `boolean_simplify=1` enables [simplifying the query](../Searching/Full_text_matching/Boolean_optimization.md) to speed it up.

### comment
String, user comment that gets copied to a query log file.

### cutoff
Integer. Max found matches threshold. The value is selected automatically if not specified.

* `N` = 0 disables the threshold
* `N > 0`: instructs Manticore to stop looking for results as soon as it finds `N` documents.
* not set: Manticore will decide automatically what the value should be.

In case Manticore cannot calculate the exact matching documents count you will see `total_relation: gte` in the query [meta information](../Profiling_and_monitoring/SHOW_META.md#SHOW-META), which means that the actual count is **Greater Than or Equal** to the total (`total_found` in `SHOW META` via SQL, `hits.total` in JSON via HTTP). If the total value is precise you'll get `total_relation: eq`.

### disable_ps_threshold
Integer. Disables pseudo sharding when the number of unique values of a groupby attribute is greater than the threshold. Default is 65536.

When a groupby query is executed, Manticore estimates the number of unique values of groupby attribute using secondary indexes. If that number is greater than the threshold,
pseudo sharding is disabled in order to increase accuracy of count() and aggregates. Loss of accuracy may occur because pseudo sharding runs queries in several threads,
each thread getting max_matches groups which are merged later. If there are a lot of unique groupby values, each thread may get its own set of groups, and groups present in
other threads may not make it to max_matches. This may be avoided by increasing max_matches, but increasing max_matches with pseudo_sharding enabled leads to increased
memory consumption.

See also [max_matches_increase_threshold](../Searching/Options.md#max_matches_increase_threshold), which can affect the behavior of the max_matches option.

### expand_keywords
`0` or `1` (`0` by default). Expands keywords with exact forms and/or stars when possible. Refer to [expand_keywords](../Creating_an_index/NLP_and_tokenization/Wildcard_searching_settings.md#expand_keywords) for more details.

### field_weights
Named integer list (per-field user weights for ranking).

Example:
```sql
SELECT ... OPTION field_weights=(title=10, body=3)
```

### global_idf
Use global statistics (frequencies) from the [global_idf](../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#global_idf) file for IDF computations.

### idf
Quoted, comma-separated list of IDF computation flags. Known flags are:

* `normalized`: BM25 variant, idf = log((N-n+1)/n), as per Robertson et al
* `plain`: plain variant, idf = log(N/n), as per Sparck-Jones
* `tfidf_normalized`: additionally divide IDF by query word count, so that `TF*IDF` fits into [0, 1] range
* `tfidf_unnormalized`: do not additionally divide IDF by query word count where N is the collection size and n is the number of matched documents

The historically default IDF (Inverse Document Frequency) in Manticore is equivalent to `OPTION idf='normalized,tfidf_normalized'`, and those normalizations may cause several undesired effects.

First, `idf=normalized` causes keyword penalization. For instance, if you search for `the | something` and `the` occurs in more than 50% of the documents, then documents with both keywords `the` and `something` will get less weight than documents with just one keyword `something`. Using `OPTION idf=plain` avoids this. Plain IDF varies in `[0, log(N)]` range, and keywords are never penalized; while the normalized IDF varies in `[-log(N), log(N)]` range, and too frequent keywords are penalized.

Second, `idf=tfidf_normalized` causes IDF drift over queries. Historically, we additionally divided IDF by query keyword count, so that the entire `sum(tf*idf)` over all keywords would still fit into [0,1] range. However, that means that queries `word1` and `word1 | nonmatchingword2` would assign different weights to the exactly same result set, because the IDFs for both `word1` and `nonmatchingword2` would be divided by 2. `OPTION idf='tfidf_unnormalized'` fixes that. Note that BM25, BM25A, BM25F() ranking factors will be scale accordingly once you disable this normalization.

IDF flags can be mixed; `plain` and `normalized` are mutually exclusive; `tfidf_unnormalized` and `tfidf_normalized` are mutually exclusive; and unspecified flags in such a mutually exclusive group take their defaults. That means that `OPTION idf=plain` is equivalent to a complete `OPTION idf='plain,tfidf_normalized'` specification.

### index_weights
Named integer list. Per-index user weights for ranking.

### local_df
`0` or `1`,automatically sum DFs over all the local parts of a distributed index, so that the IDF is consistent (and precise) over a locally sharded index.

### low_priority
`0` or `1` (`0` by default). `low_priority=1` runs the query with low priority in terms of Linux CPU scheduling. Consider also option `threads=1` instead, or use that together with `low_priority=1`, as it might be better in some use cases.

### max_matches
Integer. Per-query max matches value.

Maximum amount of matches that the server keeps in RAM for each index and can return to the client. Default is 1000.

Introduced in order to control and limit RAM usage, `max_matches` setting defines how much matches will be kept in RAM while searching each index. Every match found will still be processed; but only best N of them will be kept in memory and return to the client in the end. Assume that the index contains 2,000,000 matches for the query. You rarely (if ever) need to retrieve all of them. Rather, you need to scan all of them, but only choose “best” at most, say, 500 by some criteria (ie. sorted by relevance, or price, or anything else), and display those 500 matches to the end user in pages of 20 to 100 matches. And tracking only the best 500 matches is much more RAM and CPU efficient than keeping all 2,000,000 matches, sorting them, and then discarding everything but the first 20 needed to display the search results page. `max_matches` controls N in that "best N" amount.

This parameter noticeably affects per-query RAM and CPU usage. Values of 1,000 to 10,000 are generally fine, but higher limits must be used with care. Recklessly raising max_matches to 1,000,000 means that `searchd` will have to allocate and initialize 1-million-entry matches buffer for every query. That will obviously increase per-query RAM usage, and in some cases can also noticeably impact performance.

### max_matches_increase_threshold

Integer. Sets the threshold that max_matches can be increased to. Default is 16384.

Manticore may increase max_matches to improve groupby and/or aggregation accuracy when pseudo_sharding is enabled and if it detects that the number of unique values
of groupby attribute is less than this threshold. Loss of accuracy may occur when pseudo sharding executes the query in several threads or RT index performs
parallel searches in disk chunks. See [disable_ps_threshold](../Searching/Options.md#disable_ps_threshold) for more details.

If the number of unique values of groupby attribute is less than the treshold, max_matches will be set to this number. Otherwise, default max_matches will be used.

If max_matches was set explicitly in query options, this threshold has no effect.

Note that if this threshold is set too high, the result will be increased memory consumption and general performance degradation.

### max_query_time
Sets maximum search query time, in milliseconds. Must be a non-negative integer. Default value is 0 which means "do not limit". Local search queries will be stopped once that much time has elapsed. Note that if you're performing a search which queries several local indexes, this limit applies to each index separately. Note it may increase the query's response time a little bit, the overhead is caused by constant tracking if it's time to stop the query.

### max_predicted_time
Integer. Max predicted search time, see [predicted_time_costs](../Server_settings/Searchd.md#predicted_time_costs).

### morphology
`none` allows to replace all query terms with their exact forms if index was built with [index_exact_words](../Creating_an_index/NLP_and_tokenization/Morphology.md#index_exact_words) enabled. Useful to prevent stemming or lemmatizing query terms.

### not_terms_only_allowed
<!-- example not_terms_only_allowed -->
`0` or `1`, allows standalone [negation](../Searching/Full_text_matching/Operators.md#Negation-operator) for the query. Default is 0. See also corresponding [global setting](../Server_settings/Searchd.md#not_terms_only_allowed).

<!-- request SQL -->
```sql
MySQL [(none)]> select * from idx where match('-donald');
ERROR 1064 (42000): index t: query error: query is non-computable (single NOT operator)
MySQL [(none)]> select * from t where match('-donald') option not_terms_only_allowed=1;
+---------------------+-----------+
| id                  | field     |
+---------------------+-----------+
| 1658178727135150081 | smth else |
+---------------------+-----------+
```
<!-- end -->

### ranker
Any of:
* `proximity_bm25`
* `bm25`
* `none`
* `wordcount`
* `proximity`
* `matchany`
* `fieldmask`
* `sph04`
* `expr`
* or `export`

Refer to [Search results ranking](../Searching/Sorting_and_ranking.md) for more details on each ranker.

### rand_seed
Lets you specify a specific integer seed value for an `ORDER BY RAND()` query, for example: `... OPTION rand_seed=1234`. By default, a new and different seed value is autogenerated for every query

### retry_count
Integer. Distributed retries count.

### retry_delay
Integer. Distributed retry delay, msec.

### sort_method
* `pq` - priority queue, set by default
* `kbuffer` - gives faster sorting for already pre-sorted data, e.g. index data sorted by id
The result set is in both cases the same; picking one option or the other may just improve (or worsen!) performance.

### threads
Limits max number of threads to use for current query processing. Default - no limit (the query can occupy all [threads](../Server_settings/Searchd.md#threads) as defined globally).
For batch of queries the option must be attached to the very first query in the batch, and it is then applied when working queue is created and then is effective for the whole batch. This option has same meaning as option [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query), but applied only to the current query or batch of queries.

### token_filter
Quoted, colon-separated of `library name:plugin name:optional string of settings`. Query-time token filter gets created on search each time full-text invoked by every index involved and let you implement a custom tokenizer that makes tokens according to custom rules.
```sql
SELECT * FROM index WHERE MATCH ('yes@no') OPTION token_filter='mylib.so:blend:@'
```

## FORCE and IGNORE INDEX

<!-- example options_force -->

In rare cases Manticore's built-in query analyzer can be wrong in understanding a query and whether an index by id or another field (in case [secondary indexes](../Server_settings/Searchd.md#secondary_indexes) is on) should be used or not. Adding `FORCE INDEX(field{, fieldN})` will force Manticore use indexes of the specified fields. `IGNORE INDEX(field{, fieldN})` will force ignore that.

<!-- request SQL -->

```sql
SELECT * FROM students where age > 21 FORCE INDEX(age)
```

<!-- end -->
