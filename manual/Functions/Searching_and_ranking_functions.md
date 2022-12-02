# Searching and ranking functions

### BM25A()
`BM25A(k1,b)` returns precise `BM25A()`. Requires ranker `expr` and enabled `index_field_lengths`. Parameters `k` and `b` must be float.

### BM25F()
`BM25F(k1,b, {field=weight, ...})` returns precise `BM25F()` and `index_field_lengths` to be enabled. Requires `expr` ranker. `k` and `b` parameters must be float.

### EXIST()
Replaces non-existent columns with default values. It returns either a value of an attribute specified by 'attr-name', or 'default-value' if that attribute does not exist. It does not support STRING or MVA attributes. This function is handy when you are searching through several tables with different schemas.

```sql
SELECT *, EXIST('gid', 6) as cnd FROM i1, i2 WHERE cnd>5
```

### MIN_TOP_SORTVAL()
Returns sort key value of the worst found element in the current top-N matches if sort key is float and 0 otherwise.

### MIN_TOP_WEIGHT()
Returns weight of the worst found element in the current top-N matches.

### PACKEDFACTORS()
`PACKEDFACTORS()` can be used in queries, either to just see all the weighting factors calculated when doing the matching, or to provide a binary attribute that can be used to write a custom ranking UDF. This function works only if expression ranker is specified and the query is not a full scan, otherwise it will return an error. `PACKEDFACTORS()` can take an optional argument that disables ATC ranking factor calculation: `PACKEDFACTORS({no_atc=1})` Calculating ATC slows down query processing considerably, so this option can be useful if you need to see the ranking factors, but do not need ATC. `PACKEDFACTORS()` can also be told to format its output as JSON: `PACKEDFACTORS({json=1})` The respective outputs in either key-value pair or JSON format would look as follows below. (Note that the examples below are wrapped for readability; actual returned values would be single-line.)

```sql
mysql> SELECT id, PACKEDFACTORS() FROM test1
    -> WHERE MATCH('test one') OPTION ranker=expr('1') \G
*************************** 1\. row ***************************
             id: 1
packedfactors(): bm25=569, bm25a=0.617197, field_mask=2, doc_word_count=2,
    field1=(lcs=1, hit_count=2, word_count=2, tf_idf=0.152356,
        min_idf=-0.062982, max_idf=0.215338, sum_idf=0.152356, min_hit_pos=4,
        min_best_span_pos=4, exact_hit=0, max_window_hits=1, min_gaps=2,
        exact_order=1, lccs=1, wlccs=0.215338, atc=-0.003974),
    word0=(tf=1, idf=-0.062982),
    word1=(tf=1, idf=0.215338)
1 row in set (0.00 sec)
```

```sql
mysql> SELECT id, PACKEDFACTORS({json=1}) FROM test1
    -> WHERE MATCH('test one') OPTION ranker=expr('1') \G
*************************** 1\. row ***************************
                     id: 1
packedfactors({json=1}):
{

    "bm25": 569,
    "bm25a": 0.617197,
    "field_mask": 2,
    "doc_word_count": 2,
    "fields": [
        {
            "lcs": 1,
            "hit_count": 2,
            "word_count": 2,
            "tf_idf": 0.152356,
            "min_idf": -0.062982,
            "max_idf": 0.215338,
            "sum_idf": 0.152356,
            "min_hit_pos": 4,
            "min_best_span_pos": 4,
            "exact_hit": 0,
            "max_window_hits": 1,
            "min_gaps": 2,
            "exact_order": 1,
            "lccs": 1,
            "wlccs": 0.215338,
            "atc": -0.003974
        }
    ],
    "words": [
        {
            "tf": 1,
            "idf": -0.062982
        },
        {
            "tf": 1,
            "idf": 0.215338
        }
    ]

}
1 row in set (0.01 sec)
```

This function can be used to implement custom ranking functions in UDFs, as in

```sql
SELECT *, CUSTOM_RANK(PACKEDFACTORS()) AS r
FROM my_index
WHERE match('hello')
ORDER BY r DESC
OPTION ranker=expr('1');
```

Where `CUSTOM_RANK()` is a function implemented in an UDF. It should declare a `SPH_UDF_FACTORS` structure (defined in sphinxudf.h), initialize this structure, unpack the factors into it before usage, and deinitialize it afterwards, as follows:

```sql
SPH_UDF_FACTORS factors;
sphinx_factors_init(&factors);
sphinx_factors_unpack((DWORD*)args->arg_values[0], &factors);
// ... can use the contents of factors variable here ...
sphinx_factors_deinit(&factors);
```

`PACKEDFACTORS()` data is available at all query stages, not just when doing the initial matching and ranking pass. That enables another particularly interesting application of `PACKEDFACTORS()`, namely re-ranking.

In the example just above, we used an expression-based ranker with a dummy expression, and sorted the result set by the value computed by our UDF. In other words, we used the UDF to rank all our results. Assume now, for the sake of an example, that our UDF is extremely expensive to compute and has a throughput of just 10,000 calls per second. Assume that our query matches 1,000,000 documents. To maintain reasonable performance, we would then want to use a (much) simpler expression to do most of our ranking, and then apply the expensive UDF to only a few top results, say, top-100 results. Or, in other words, build top-100 results using a simpler ranking function and then re-rank those with a complex one. We can do that just as well with subselects:

```sql
SELECT * FROM (
    SELECT *, CUSTOM_RANK(PACKEDFACTORS()) AS r
    FROM my_index WHERE match('hello')
    OPTION ranker=expr('sum(lcs)*1000+bm25')
    ORDER BY WEIGHT() DESC
    LIMIT 100
) ORDER BY r DESC LIMIT 10
```

In this example, expression-based ranker will be called for every matched document to compute `WEIGHT()`. So it will get called 1,000,000 times. But the UDF computation can be postponed until the outer sort. And it also will be done for just the top-100 matches by `WEIGHT()`, according to the inner limit. So the UDF will only get called 100 times. And then the final top-10 matches by UDF value will be selected and returned to the application.

For reference, in the distributed case `PACKEDFACTORS()` data gets sent from the agents to master in a binary format, too. This makes it technically feasible to implement additional re-ranking pass (or passes) on the master node, if needed.

If used in SQL, but not called from any UDFs, the result of `PACKEDFACTORS()` is simply formatted as plain text, which can be used to manually assess the ranking factors. Note that this feature is not currently supported by the Manticore API.


### REMOVE_REPEATS()
`REMOVE_REPEATS ( result_set, column, offset, limit )` - removes repeated adjusted rows with the same 'column' value.

```sql
SELECT REMOVE_REPEATS((SELECT * FROM dist1), gid, 0, 10)
```

### WEIGHT()
`WEIGHT()` function returns the calculated matching score. If no ordering specified, the result is sorted descending by the score provided by `WEIGHT()`. In this example we order first by weight and then by an integer attribute.

The search above does a simple matching, where all words need to be present. But we can do more (and this is just a simple example):

```sql
mysql> SELECT *,WEIGHT() FROM testrt WHERE MATCH('"list of business laptops"/3');
+------+------+-------------------------------------+---------------------------+----------+
| id   | gid  | title                               | content                   | weight() |
+------+------+-------------------------------------+---------------------------+----------+
|    1 |   10 | List of HP business laptops         | Elitebook Probook         |     2397 |
|    2 |   10 | List of Dell business laptops       | Latitude Precision Vostro |     2397 |
|    3 |   20 | List of Dell gaming laptops         | Inspirion Alienware       |     2375 |
|    5 |   30 | List of ASUS ultrabooks and laptops | Zenbook Vivobook          |     2375 |
+------+------+-------------------------------------+---------------------------+----------+
4 rows in set (0.00 sec)


mysql> SHOW META;
+----------------+----------+
| Variable_name  | Value    |
+----------------+----------+
| total          | 4        |
| total_found    | 4        |
| total_relation | eq       |
| time           | 0.000    |
| keyword[0]     | list     |
| docs[0]        | 5        |
| hits[0]        | 5        |
| keyword[1]     | of       |
| docs[1]        | 4        |
| hits[1]        | 4        |
| keyword[2]     | business |
| docs[2]        | 2        |
| hits[2]        | 2        |
| keyword[3]     | laptops  |
| docs[3]        | 5        |
| hits[3]        | 5        |
+----------------+----------+
16 rows in set (0.00 sec)
```

Here we search for 4 words, but we can have a match even if only 3 words (of 4) are found. The search will rank higher first the documents that contain all the words.


### ZONESPANLIST()
`ZONESPANLIST()` function returns pairs of matched zone spans. Each pair contains the matched zone span identifier, a colon, and the order number of the matched zone span. For example, if a document reads `<emphasis role="bold"><i>text</i> the <i>text</i></emphasis>`, and you query for `'ZONESPAN:(i,b) text'`, then `ZONESPANLIST()` will return the string `"1:1 1:2 2:1"` meaning that the first zone span matched "text" in spans 1 and 2, and the second zone span in span 1 only.  

### QUERY()
`QUERY()` returns the current search query. `QUERY()` is a postlimit expression and is intended to be used with [SNIPPET()](../Functions/String_functions.md#SNIPPET%28%29).

Table functions is a mechanism of post-query result set processing. Table functions take an arbitrary result set as their input, and return a new, processed set as their output. The first argument should be the input result set, but a table function can optionally take and handle more arguments. Table functions can completely change the result set, including the schema. For now, only built in table functions are supported. Table functions work for both outer `SELECT` and [nested SELECT](../Searching/Sub-selects.md).
