# Search options

The SQL [SELECT](../Searching/Full_text_matching/Basic_usage.md#SQL) clause and the HTTP [/search](../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) endpoint support a number of options that can be used to fine-tune search behavior.

## OPTION

### General syntax

<!-- example options -->

**SQL**:

```sql
SELECT ... [OPTION <optionname>=<value> [ , ... ]] [/*+ [NO_][ColumnarScan|DocidIndex|SecondaryIndex(<attribute>[,...])]] /*]
```

**HTTP**:
```json
POST /search
{   
    "table" : "table_name",
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
JSON:
<!-- request JSON -->

```json
POST /search
{   
    "table" : "test",
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

<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "total_relation": "eq",
    "hits": [
      {
        "_id": 1,
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

### accurate_aggregation
Integer. Enables or disables guaranteed aggregate accuracy when running groupby queries in multiple threads. Default is 0.

When running a groupby query, it can be run in parallel on a plain table with several pseudo shards (if `pseudo_sharding` is on). A similar approach works on RT tables. Each shard/chunk executes the query, but the number of groups is limited by `max_matches`. If the result sets from different shards/chunks have different groups, the group counts and aggregates may be inaccurate. Note that Manticore tries to increase `max_matches` up to [`max_matches_increase_threshold`](../Searching/Options.md#max_matches_increase_threshold) based on the number of unique values of the groupby attribute (retrieved from secondary indexes). If it succeeds, there will be no loss in accuracy.

However, if the number of unique values of the groupby attribute is high, further increasing `max_matches` may not be a good strategy because it can lead to a loss in performance and higher memory usage. Setting `accurate_aggregation` to 1 forces groupby searches to run in a single thread, which fixes the accuracy issue. Note that running in a single thread is only enforced when `max_matches` cannot be set high enough; otherwise, searches with `accurate_aggregation=1` will still run in multiple threads.

Overall, setting `accurate_aggregation` to 1 ensures group count and aggregate accuracy in RT tables and plain tables with `pseudo_sharding=1`. The drawback is that searches will run slower since they will be forced to operate in a single thread.

However, if we have an RT table and a plain table containing the same data, and we run a query with `accurate_aggregation=1`, we might still receive different results. This occurs because the daemon might choose different `max_matches` settings for the RT and plain table due to the [`max_matches_increase_threshold`](../Searching/Options.md#max_matches_increase_threshold) setting.

### agent_query_timeout
Integer. Max time in milliseconds to wait for remote queries to complete, see [this section](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout).

### boolean_simplify
`0` or `1` (`0` by default). `boolean_simplify=1` enables [simplifying the query](../Searching/Full_text_matching/Boolean_optimization.md) to speed it up.

### comment
String, user comment that gets copied to a query log file.

### cutoff
Integer. Specifies the maximum number of matches to process. If not set, Manticore will select an appropriate value automatically.

<!-- example cutoff_aggregation -->

* `N = 0`: Disables the limit on the number of matches.
* `N > 0`: Instructs Manticore to stop processing results as soon as it finds `N` matching documents.
* Not set: Manticore decides the threshold automatically.

When Manticore cannot determine the exact count of matching documents, the `total_relation` field in the query [meta information](../Node_info_and_management/SHOW_META.md#SHOW-META) will show `gte`, which stands for **Greater Than or Equal to**. This indicates that the actual count of matches is at least the reported `total_found` (in SQL) or `hits.total` (in JSON). When the count is exact, `total_relation` will display `eq`.

Note: Using `cutoff` in aggregation queries is not recommended because it can produce inaccurate or incomplete results.

<!-- request Example -->

Using `cutoff` in aggregation queries can lead to incorrect or misleading results, as shown in the following example:
```
drop table if exists t
--------------

Query OK, 0 rows affected (0.02 sec)

--------------
create table t(a int)
--------------

Query OK, 0 rows affected (0.04 sec)

--------------
insert into t(a) values(1),(2),(3),(1),(2),(3)
--------------

Query OK, 6 rows affected (0.00 sec)

--------------
select avg(a) from t option cutoff=1 facet a
--------------

+----------+
| avg(a)   |
+----------+
| 1.000000 |
+----------+
1 row in set (0.00 sec)
--- 1 out of 1 results in 0ms ---

+------+----------+
| a    | count(*) |
+------+----------+
|    1 |        1 |
+------+----------+
1 row in set (0.00 sec)
--- 1 out of 1 results in 0ms ---
```

Compare it with the same query without `cutoff`:
```
--------------
select avg(a) from t facet a
--------------

+----------+
| avg(a)   |
+----------+
| 2.000000 |
+----------+
1 row in set (0.00 sec)
--- 1 out of 1 results in 0ms ---

+------+----------+
| a    | count(*) |
+------+----------+
|    1 |        2 |
|    2 |        2 |
|    3 |        2 |
+------+----------+
3 rows in set (0.00 sec)
--- 3 out of 3 results in 0ms ---
```

<!-- end -->

### distinct_precision_threshold
Integer. Default is `3500`. This option sets the threshold below which counts returned by `count distinct` are guaranteed to be exact within a plain table.

Accepted values range from `500` to `15500`. Values outside this range will be clamped.

When this option is set to 0, it enables an algorithm that ensures exact counts. This algorithm collects `{group, value}` pairs, sorts them, and periodically eliminates duplicates. The result is precise counts within a plain table. However, this approach is not suitable for high-cardinality datasets due to its high memory consumption and slow query execution.

When `distinct_precision_threshold` is set to a value greater than `0`, Manticore employs a different algorithm. It loads counts into a hash table and returns the size of the table. If the hash table becomes too large, its contents are moved into a `HyperLogLog` data structure. At this point, the counts become approximate because HyperLogLog is a probabilistic algorithm. This approach maintains a fixed maximum memory usage per group, but there is a tradeoff in count accuracy.

The accuracy of the `HyperLogLog` and the threshold for converting from the hash table to HyperLogLog are derived from the `distinct_precision_threshold` setting. It's important to use this option with caution since doubling its value will also double the maximum memory required to calculate counts. The maximum memory usage can be roughly estimated using this formula: `64 * max_matches * distinct_precision_threshold`, although in practice, count calculations often use less memory than the worst-case scenario.

### expand_keywords
`0` or `1` (`0` by default). Expands keywords with exact forms and/or stars when possible. Refer to [expand_keywords](../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#expand_keywords) for more details.

### field_weights
Named integer list (per-field user weights for ranking).

Example:
```sql
SELECT ... OPTION field_weights=(title=10, body=3)
```

### global_idf
Use global statistics (frequencies) from the [global_idf](../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#global_idf) file for IDF computations.

### idf
Quoted, comma-separated list of IDF computation flags. Known flags are:

* `normalized`: BM25 variant, idf = log((N-n+1)/n), as per Robertson et al
* `plain`: plain variant, idf = log(N/n), as per Sparck-Jones
* `tfidf_normalized`: additionally divide IDF by query word count, so that `TF*IDF` fits into [0, 1] range
* `tfidf_unnormalized`: do not additionally divide IDF by query word count where N is the collection size and n is the number of matched documents

The historically default IDF (Inverse Document Frequency) in Manticore is equivalent to `OPTION idf='normalized,tfidf_normalized'`, and those normalizations may cause several undesired effects.

First, `idf=normalized` causes keyword penalization. For instance, if you search for `the | something` and `the` occurs in more than 50% of the documents, then documents with both keywords `the` and `something` will get less weight than documents with just one keyword `something`. Using `OPTION idf=plain` avoids this. Plain IDF varies in `[0, log(N)]` range, and keywords are never penalized; while the normalized IDF varies in `[-log(N), log(N)]` range, and too frequent keywords are penalized.

Second, `idf=tfidf_normalized` leads to IDF drift across queries. Historically, IDF was also divided by the query keyword count, ensuring the entire `sum(tf*idf)` across all keywords remained within the [0,1] range. However, this meant that queries like `word1` and `word1 | nonmatchingword2` would assign different weights to the exact same result set, as the IDFs for both `word1` and `nonmatchingword2` would be divided by 2. Using `OPTION idf='tfidf_unnormalized'` resolves this issue. Keep in mind that BM25, BM25A, BM25F() ranking factors will be adjusted accordingly when you disable this normalization.

IDF flags can be combined; `plain` and `normalized` are mutually exclusive; `tfidf_unnormalized` and `tfidf_normalized` are also mutually exclusive; and unspecified flags in such mutually exclusive groups default to their original settings. This means `OPTION idf=plain` is the same as specifying `OPTION idf='plain,tfidf_normalized'` in its entirety.

### jieba_mode
Specifies the Jieba segmentation mode for the query.

When using Jieba Chinese segmentation, it can sometimes help to use different segmentation modes for tokenizing the documents and the query. For a complete list of modes, refer to [jieba_mode](Creating_a_table/NLP_and_tokenization/Morphology.md#jieba_mode).

### index_weights
Named integer list. Per-table user weights for ranking.

### local_df
`0` or `1`, automatically sum DFs over all local parts of a distributed table, ensuring consistent (and accurate) IDF across a locally sharded table. Enabled by default for disk chunks of the RT table. Query terms with wildcards are ignored.

### low_priority
`0` or `1` (`0` by default). Setting `low_priority=1` executes the query with a lower priority, rescheduling its jobs 10 times less frequently than other queries with normal priority.

### max_matches
Integer. Per-query max matches value.

The maximum number of matches that the server retains in RAM for each table and can return to the client. The default is 1000.

Introduced to control and limit RAM usage, the `max_matches` setting determines how many matches will be kept in RAM while searching each table. Every match found is still processed, but only the best N of them will be retained in memory and returned to the client in the end. For example, suppose a table contains 2,000,000 matches for a query. It's rare that you would need to retrieve all of them. Instead, you need to scan all of them but only choose the "best" 500, for instance, based on some criteria (e.g., sorted by relevance, price, or other factors) and display those 500 matches to the end user in pages of 20 to 100 matches. Tracking only the best 500 matches is much more RAM and CPU efficient than keeping all 2,000,000 matches, sorting them, and then discarding everything but the first 20 needed for the search results page. `max_matches` controls the N in that "best N" amount.

This parameter significantly impacts per-query RAM and CPU usage. Values of 1,000 to 10,000 are generally acceptable, but higher limits should be used with caution. Carelessly increasing max_matches to 1,000,000 means that `searchd` will have to allocate and initialize a 1-million-entry matches buffer for every query. This will inevitably increase per-query RAM usage and, in some cases, can noticeably affect performance.

Refer to [max_matches_increase_threshold](../Searching/Options.md#max_matches_increase_threshold) for additional information on how it can influence the behavior of the `max_matches` option.

### max_matches_increase_threshold

Integer. Sets the threshold that `max_matches` can be increased to. Default is 16384.

Manticore may increase `max_matches` to enhance groupby and/or aggregation accuracy when `pseudo_sharding` is enabled, and if it detects that the number of unique values of the groupby attribute is less than this threshold. Loss of accuracy may occur when pseudo-sharding executes the query in multiple threads or when an RT table conducts parallel searches in disk chunks.

If the number of unique values of the groupby attribute is less than the threshold, `max_matches` will be set to this number. Otherwise, the default `max_matches` will be used.

If `max_matches` was explicitly set in query options, this threshold has no effect.

Keep in mind that if this threshold is set too high, it will result in increased memory consumption and general performance degradation.

You can also enforce a guaranteed groupby/aggregate accuracy mode using the [accurate_aggregation](../Searching/Options.md#accurate_aggregation) option.

### max_query_time
Sets the maximum search query time in milliseconds. Must be a non-negative integer. The default value is 0, which means "do not limit." Local search queries will be stopped once the specified time has elapsed. Note that if you're performing a search that queries multiple local tables, this limit applies to each table separately. Be aware that this may slightly increase the query's response time due to the overhead caused by constantly tracking whether it's time to stop the query.

### max_predicted_time
Integer. Maximum predicted search time; see [predicted_time_costs](../Server_settings/Searchd.md#predicted_time_costs).

### morphology
`none` allows replacing all query terms with their exact forms if the table was built with [index_exact_words](../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words) enabled. This is useful for preventing stemming or lemmatizing query terms.

### not_terms_only_allowed
<!-- example not_terms_only_allowed -->
`0` or `1` allows standalone [negation](../Searching/Full_text_matching/Operators.md#Negation-operator) for the query. The default is 0. See also the corresponding [global setting](../Server_settings/Searchd.md#not_terms_only_allowed).

<!-- request SQL -->
```sql
MySQL [(none)]> select * from tbl where match('-donald');
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
Choose from the following options:
* `proximity_bm25`
* `bm25`
* `none`
* `wordcount`
* `proximity`
* `matchany`
* `fieldmask`
* `sph04`
* `expr`
* `export`

For more details on each ranker, refer to [Search results ranking](../Searching/Sorting_and_ranking.md#Available-built-in-rankers).

### rand_seed
Allows you to specify a specific integer seed value for an `ORDER BY RAND()` query, for example: `... OPTION rand_seed=1234`. By default, a new and different seed value is autogenerated for every query.

### retry_count
Integer. Distributed retries count.

### retry_delay
Integer. Distributed retry delay, in milliseconds.

### scroll

String. A scroll token for paginating results using the [Scroll pagination approach](../Searching/Pagination.md#Scroll-Search-Option).

### sort_method
* `pq` - priority queue, set by default
* `kbuffer` - provides faster sorting for already pre-sorted data, e.g., table data sorted by id
The result set is the same in both cases; choosing one option or the other may simply improve (or worsen) performance.

### threads
Limits the max number of threads used for current query processing. Default - no limit (the query can occupy all [threads](../Server_settings/Searchd.md#threads) as defined globally).
For a batch of queries, the option must be attached to the very first query in the batch, and it is then applied when the working queue is created and is effective for the entire batch. This option has the same meaning as the option [max_threads_per_query](../Server_settings/Searchd.md#max_threads_per_query), but is applied only to the current query or batch of queries.

### token_filter
Quoted, colon-separated string of `library name:plugin name:optional string of settings`. A query-time token filter is created for each search when full-text is invoked by every table involved, allowing you to implement a custom tokenizer that generates tokens according to custom rules.
```sql
SELECT * FROM index WHERE MATCH ('yes@no') OPTION token_filter='mylib.so:blend:@'
```
### expansion_limit
Restricts the maximum number of expanded keywords for a single wildcard, with a default value of 0 indicating no limit. For additional details, refer to [expansion_limit](../Server_settings/Searchd.md#expansion_limit).

## Query optimizer hints

<!-- example options_force -->

In rare cases, Manticore's built-in query analyzer may be incorrect in understanding a query and determining whether a docid index, secondary indexes, or columnar scan should be used. To override the query optimizer's decisions, you can use the following hints in your query:

* `/*+ DocidIndex(id) */`  to force the use of a docid index, `/*+ NO_DocidIndex(id) */` to tell the optimizer to ignore it
* `/*+ SecondaryIndex(<attr_name1>[, <attr_nameN>]) */` to force the use of a secondary index (if available), `/*+ NO_SecondaryIndex(id) */`  to tell the optimizer to ignore it
* `/*+ ColumnarScan(<attr_name1>[, <attr_nameN>]) */`  to force the use of a columnar scan (if the attribute is columnar), `/*+ NO_ColumnarScan(id) */` to tell the optimizer to ignore it

Note that when executing a full-text query with filters, the query optimizer decides between intersecting the results of the full-text tree with the filter results or using a standard match-then-filter approach. Specifying *any* hint will force the daemon to use the code path that performs the intersection of the full-text tree results with the filter results.

For more information on how the query optimizer works, refer to the [Cost based optimizer](../Searching/Cost_based_optimizer.md) page.

<!-- request SQL -->

```sql
SELECT * FROM students where age > 21 /*+ SecondaryIndex(age) */
```

<!-- end -->

<!-- example comments -->
When using a MySQL/MariaDB client, make sure to include the `--comments` flag to enable the hints in your queries.

<!-- request mysql -->
```bash
mysql -P9306 -h0 --comments
```
<!-- end -->

<!-- proofread -->
