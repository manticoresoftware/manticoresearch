# Sorting and ranking

**Full-text** query returns matches sorted. By default (if nothing specified) they're sorted by relevance, which is equivalent to `ORDER BY weight() DESC` in SQL format.

**Non-full-text** queries do not do any sorting by default.

## Extended mode

```sql
ORDER BY weight() DESC, price ASC, id DESC
```

Extended mode is automatically switched on when you explicitly provide sorting rules by adding clause `ORDER BY` in SQL format or `sort` via HTTP JSON.

<!-- example alias -->

In the sort clause you can use any combination of up to 5 columns, each followed by `asc` or `desc`. Functions and expressions are not allowed as arguments for the sort clause, except for functions `weight()` and `random()` (which can be only used via SQL in the form of `ORDER BY random()`), but you can use any expression in the SELECT list and sort by its alias.

<!-- request SQL -->
```sql
select *, a + b alias from test order by alias desc;
```

<!-- response SQL -->
```
+------+------+------+----------+-------+
| id   | a    | b    | f        | alias |
+------+------+------+----------+-------+
|    1 |    2 |    3 | document |     5 |
+------+------+------+----------+-------+
```

<!-- end -->

### HTTP

**Sorting by attributes**

Query results can be sorted by one or more attributes. Example:

```json
{
  "index":"test",
  "query":
  {
    "match": { "title": "what was" }
  },
  "sort": [ "_score", "id" ]
}
```

`"sort"` specifies an array of attributes and/or additional properties. Each element of the array can be an attribute name or `"_score"` if you want to sort by match weights. In that case sort order defaults to ascending for attributes and descending for `_score`.

You can also specify sort order explicitly. Example:

```json
"sort":
[
  { "price":"asc" },
  "id"
]
```

* `asc`: sort in ascending order
* `desc`: sort in descending order

You can also use another syntax and specify sort order via the `order` property:

```json
"sort":
[
  { "gid": { "order":"desc" } }
]
```

Sorting by MVA attributes is also supported in JSON queries. Sorting mode can be set via the `mode` property. The following modes are supported:

* `min`: sort by minimum value
* `max`: sort by maximum value

Example:

```json
"sort":
[
  { "attr_mva": { "order":"desc", "mode":"max" } }
]
```

When sorting on an attribute, match weight (score) calculation is disabled by default (no ranker is used). You can enable weight calculation by setting the `track_scores` property to true:

```json
{
  "index":"test",
  "track_scores":true,
  "query": { "match": { "title": "what was" } },
  "sort": [ { "gid": { "order":"desc" } } ]
}
```

## Ranking overview

Ranking (also known as weighting) of search results can be defined as a process of computing a so-called relevance (weight) for every given matched document with regards to a given query that matched it. So relevance is in the end just a number attached to every document that estimates how relevant the document is to the query. Search results can then be sorted based on this number and/or some additional parameters, so that the most sought after results would come up higher on the results page.

There is no single standard one-size-fits-all way to rank any document in any scenario. Moreover, there can not ever be such a way, because relevance is *subjective*. As in, what seems relevant to you might not seem relevant to me. Hence, in general case it's not just hard to compute, it's theoretically impossible.

So ranking in Manticore is configurable. It has a notion of a so-called **ranker**. A ranker can formally be defined as a function that takes document and query as its input and produces a relevance value as output. In layman's terms, a ranker controls exactly how (using which specific algorithm) will Manticore assign weights to the document.

## Available built-in rankers

Manticore ships with a number of built-in rankers suited for different purposes. A number of them uses two factors, phrase proximity (aka LCS) and BM25. Phrase proximity works on the keyword positions, while BM25 works on the keyword frequencies. Basically, the better the degree of the phrase match between the document body and the query, the higher is the phrase proximity (it maxes out when the document contains the entire query as a verbatim quote). And BM25 is higher when the document contains more rare words. We'll save the detailed discussion for later.

Currently implemented rankers are:

* `proximity_bm25`, the default ranking mode that uses and combines both phrase proximity and BM25 ranking.
* `bm25`, statistical ranking mode which uses BM25 ranking only (similar to most other full-text engines). This mode is faster but may result in worse quality on queries which contain more than 1 keyword.
* `none`, no ranking mode. This mode is obviously the fastest. A weight of 1 is assigned to all matches. This is sometimes called boolean searching that just matches the documents but does not rank them.
* `wordcount`, ranking by the keyword occurrences count. This ranker computes the per-field keyword occurrence counts, then  multiplies them by field weights, and sums the resulting values.
* `proximity` returns raw phrase proximity value as a result. This mode is internally used to emulate `SPH_MATCH_ALL` queries.
* `matchany` returns rank as it was computed in `SPH_MATCH_ANY` mode earlier, and is internally used to emulate `SPH_MATCH_ANY` queries.
* `fieldmask` returns a 32-bit mask with N-th bit corresponding to N-th fulltext field, numbering from 0. The bit will only be set when the respective field has any keyword occurrences satisfying the query.
* `sph04` is generally based on the default 'proximity_bm25' ranker, but additionally boosts the matches when they occur in the very beginning or the very end of a text field. Thus, if a field equals the exact query, `sph04` should rank it higher than a field that contains the exact query but is not equal to it. (For instance, when the query is "Hyde Park", a document entitled "Hyde Park" should be ranked higher than a one entitled "Hyde Park, London" or "The Hyde Park Cafe".)
* `expr` lets you specify the ranking formula in run time. It exposes a number of internal text factors and lets you define how the final weight should be computed from those factors. You can find more details about its syntax and a reference available factors in a [subsection below](../Searching/Sorting_and_ranking.md#Quick-summary-of-the-ranking-factors).

Ranker name is case insensitive. Example:

```sql
SELECT ... OPTION ranker=sph04;
```

## Quick summary of the ranking factors

| Name | Level | Type | Summary |
| - | - | - | - |
| max_lcs | query | int | maximum possible LCS value for the current query |
| bm25 | document | int | quick estimate of BM25(1.2, 0) without syntax support |
| bm25a(k1, b) | document | int | precise BM25() value with configurable K1, B constants and syntax support |
| bm25f(k1, b, {field=weight, ...}) | document | int | precise BM25F() value with extra configurable field weights |
| field_mask | document | int | bit mask of matched fields |
| query_word_count | document | int | number of unique inclusive keywords in a query |
| doc_word_count | document | int | number of unique keywords matched in the document |
| lcs | field | int | Longest Common Subsequence between query and document, in words |
| user_weight | field | int | user field weight |
| hit_count | field | int | total number of keyword occurrences |
| word_count | field | int | number of unique matched keywords |
| tf_idf | field | float | sum(tf\*idf) over matched keywords == sum(idf) over occurrences |
| min_hit_pos | field | int | first matched occurrence position, in words, 1-based |
| min_best_span_pos | field | int | first maximum LCS span position, in words, 1-based |
| exact_hit | field | bool | whether query == field |
| min_idf | field | float | min(idf) over matched keywords |
| max_idf | field | float | max(idf) over matched keywords |
| sum_idf | field | float | sum(idf) over matched keywords |
| exact_order | field | bool | whether all query keywords were a) matched and b) in query order |
| min_gaps | field | int | minimum number of gaps between the matched keywords over the matching spans |
| lccs | field | int | Longest Common Contiguous Subsequence between query and document, in words |
| wlccs | field | float | Weighted Longest Common Contiguous Subsequence, sum(idf) over contiguous keyword spans |
| atc | field | float |  Aggregate Term Closeness, log(1+sum(idf1\*idf2\*pow(distance, -1.75)) over the best pairs of keywords |

### Document-level ranking factors

A **document-level factor** is a numeric value computed by the ranking engine for every matched document with regards to the current query. So it differs from a plain document attribute in that the attribute do not depend on the full text query, while factors might. Those factors can be used anywhere in the ranking expression. Currently implemented document-level factors are:

* `bm25` (integer), a document-level BM25 estimate (computed without keyword occurrence filtering).
* `max_lcs` (integer), a query-level maximum possible value that the `sum(lcs*user_weight)` expression can ever take. This can be useful for weight boost scaling. For instance, `MATCHANY` ranker formula uses this to guarantee that a full phrase match in any field ranks higher than any combination of partial matches in all fields.
* `field_mask` (integer), a document-level 32-bit mask of matched fields.
* `query_word_count` (integer), the number of unique keywords in a query, adjusted for a number of excluded keywords. For instance, both `(one one one one)` and `(one !two)` queries should assign a value of 1 to this factor, because there is just one unique non-excluded keyword.
* `doc_word_count` (integer), the number of unique keywords matched in the entire document.

### Field-level ranking factors

A **field-level factor** is a numeric value computed by the ranking engine for every matched in-document text field with regards to the current query. As more than one field can be matched by a query, but the final weight needs to be a single integer value, these values need to be folded into a single one. To achieve that, field-level factors can only be used within a field aggregation function, they can **not** be used anywhere in the expression. For example, you can not use `(lcs+bm25)` as your ranking expression, as `lcs` takes multiple values (one in every matched field). You should use `(sum(lcs)+bm25)` instead, that expression sums `lcs` over all matching fields, and then adds `bm25` to that per-field sum. Currently implemented field-level factors are:

* `lcs` (integer), the length of a maximum verbatim match between the document and the query, counted in words. LCS stands for Longest Common Subsequence (or Subset). Takes a minimum value of 1 when only stray keywords were matched in a field, and a maximum value of query keywords count when the entire query was matched in a field verbatim (in the exact query keywords order). For example, if the query is 'hello world' and the field contains these two words quoted from the query (that is, adjacent to each other, and exactly in the query order), `lcs` will be 2. For example, if the query is 'hello world program' and the field contains 'hello world', `lcs` will be 2. Note that any subset of the query keyword works, not just a subset of adjacent keywords. For example, if the query is 'hello world program' and the field contains 'hello (test program)', `lcs` will be 2 just as well, because both 'hello' and 'program' matched in the same respective positions as they were in the query. Finally, if the query is 'hello world program' and the field contains 'hello world program', `lcs` will be 3. (Hopefully that is unsurprising at this point.)
* `user_weight` (integer), the user specified per-field weight (refer to [OPTION field_weights](../Searching/Options.md#field_weights) in SQL). The weights default to 1 if not specified explicitly.
* `hit_count` (integer), the number of keyword occurrences that matched in the field. Note that a single keyword may occur multiple times. For example, if 'hello' occurs 3 times in a field and 'world' occurs 5 times, `hit_count` will be 8.
* `word_count` (integer), the number of unique keywords matched in the field. For example, if 'hello' and 'world' occur anywhere in a field, `word_count` will be 2, irregardless of how many times do both keywords occur.
* `tf_idf` (float), the sum of TF/IDF over all the keywords matched in the field. IDF is the Inverse Document Frequency, a floating point value between 0 and 1 that describes how frequent is the keywords (basically, 0 for a keyword that occurs in every document indexed, and 1 for a unique keyword that occurs in just a single document). TF is the Term Frequency, the number of matched keyword occurrences in the field. As a side note, `tf_idf` is actually computed by summing IDF over all matched occurrences. That's by construction equivalent to summing TF*IDF over all matched keywords.
* `min_hit_pos` (integer), the position of the first matched keyword occurrence, counted in words. Indexing begins from position 1.
* `min_best_span_pos` (integer), the position of the first maximum LCS occurrences span. For example, assume that our query was 'hello world program' and 'hello world' subphrase was matched twice in the field, in positions 13 and 21. Assume that 'hello' and 'world' additionally occurred elsewhere in the field, but never next to each other and thus never as a subphrase match. In that case, `min_best_span_pos` will be 13. Note how for the single keyword queries `min_best_span_pos` will always equal `min_hit_pos`.
* `exact_hit` (boolean), whether a query was an exact match of the entire current field. Used in the SPH04 ranker.
* `min_idf`, `max_idf`, and `sum_idf` (float). These factors respectively represent the min(idf), max(idf) and sum(idf) over all keywords that were matched in the field.
* `exact_order` (boolean). Whether all of the query keywords were matched in the field in the exact query order. For example, '(microsoft office)' query would yield exact_order=1 in a field with the following contents: '(We use Microsoft software in our office.)'. However, the very same query in a '(Our office is Microsoft free.)' field would yield exact_order=0.
* `min_gaps` (integer), the minimum number of positional gaps between (just) the keywords matched in field. Always 0 when less than 2 keywords match; always greater or equal than 0 otherwise.
    For example, with a '[big wolf]' query, '[big bad wolf]' field would yield min_gaps=1; '[big bad hairy wolf]' field would yield min_gaps=2; '[the wolf was scary and big]' field would yield min_gaps=3; etc. However, a field like '[i heard a wolf howl]' would yield min_gaps=0, because only one keyword would be matching in that field, and, naturally, there would be no gaps between the *matched* keywords.

    Therefore, this is a rather low-level, "raw" factor that you would most likely want to *adjust* before actually using for ranking. Specific adjustments depend heavily on your data and the resulting formula, but here are a few ideas you can start with: (a) any min_gaps based boosts could be simply ignored when word_count<2;

    (b) non-trivial min_gaps values (i.e. when word_count>=2) could be clamped with a certain "worst case" constant while trivial values (i.e. when min_gaps=0 and word_count<2) could be replaced by that constant;

    (c) a transfer function like 1/(1+min_gaps) could be applied (so that better, smaller min_gaps values would maximize it and worse, bigger min_gaps values would fall off slowly); and so on.
* `lccs` (integer). Longest Common Contiguous Subsequence. A length of the longest subphrase that is common between the query and the document, computed in keywords.

    LCCS factor is rather similar to LCS but more restrictive, in a sense. While LCS could be greater than 1 though no two query words are matched next to each other, LCCS would only get greater than 1 if there are *exact*, contiguous query subphrases in the document. For example, (one two three four five) query vs (one hundred three hundred five hundred) document would yield lcs=3, but lccs=1, because even though mutual dispositions of 3 keywords (one, three, five) match between the query and the document, no 2 matching positions are actually next to each other.

    Note that LCCS still does not differentiate between the frequent and rare keywords; for that, see WLCCS.
* `wlccs` (float). Weighted Longest Common Contiguous Subsequence. A sum of IDFs of the keywords of the longest subphrase that is common between the query and the document.

    WLCCS is computed very similarly to LCCS, but every "suitable" keyword occurrence increases it by the keyword IDF rather than just by 1 (which is the case with LCS and LCCS). That lets us rank sequences of more rare and important keywords higher than sequences of frequent keywords, even if the latter are longer. For example, a query `(Zanzibar bed and breakfast)` would yield lccs=1 for a `(hotels of Zanzibar)` document, but lccs=3 against `(London bed and breakfast)`, even though "Zanzibar" is actually somewhat more rare than the entire "bed and breakfast" phrase. WLCCS factor alleviates that problem by using the keyword frequencies.
* `atc` (float). Aggregate Term Closeness. A proximity based measure that grows higher when the document contains more groups of more closely located and more important (rare) query keywords.

    **WARNING:** you should use ATC with OPTION idf='plain,tfidf_unnormalized' (see [below](../Searching/Sorting_and_ranking.md#Configuration-of-IDF-formula)); otherwise you may get unexpected results.

    ATC basically works as follows. For every keyword *occurrence* in the document, we compute the so called *term closeness*. For that,  we examine all the other closest occurrences of all the query keywords (keyword itself included too) to the left and to the right of the subject occurrence, compute a distance dampening coefficient as k = pow(distance, -1.75) for those occurrences, and sum the dampened IDFs. Thus for every occurrence of every keyword, we get a "closeness" value that describes the "neighbors" of that occurrence. We then multiply those per-occurrence closenesses by their respective subject keyword IDF, sum them all, and finally, compute a logarithm of that sum.

    Or in other words, we process the best (closest) matched keyword pairs in the document, and compute pairwise "closenesses" as the product of their IDFs scaled by the distance coefficient:

    ```
    pair_tc = idf(pair_word1) * idf(pair_word2) * pow(pair_distance, -1.75)
    ```

    We then sum such closenesses, and compute the final, log-dampened ATC value:

    ```
    atc = log(1+sum(pair_tc))
    ```

    Note that this final dampening logarithm is exactly the reason you should use OPTION idf=plain, because without it, the expression inside the log() could be negative.

    Having closer keyword occurrences actually contributes *much* more to ATC than having more frequent keywords. Indeed, when the keywords are right next to each other, distance=1 and k=1; when there just one word in between them, distance=2 and k=0.297, with two words  between, distance=3 and k=0.146, and so on. At the same time IDF attenuates somewhat slower. For example, in a 1 million document collection, the IDF values for keywords that match in 10, 100, and 1000 documents would be respectively 0.833, 0.667, and 0.500. So a keyword pair with two rather rare keywords that occur in just 10 documents each but with 2 other words in between would yield pair_tc = 0.101 and thus just barely outweigh a pair with a 100-doc and a 1000-doc keyword with 1 other word between them and pair_tc = 0.099. Moreover, a pair of two *unique*, 1-doc keywords with 3 words between them would get a pair_tc = 0.088 and lose to a pair of two 1000-doc keywords located right next to each other and yielding a pair_tc = 0.25. So, basically, while ATC does combine both keyword frequency and proximity, it is still somewhat favoring the proximity.

### Ranking factor aggregation functions

A **field aggregation function** is a single argument function that takes an expression with field-level factors, iterates it over all the matched fields, and computes the final results. Currently implemented field aggregation functions are:

* `sum`, sums the argument expression over all matched fields. For instance, `sum(1)` should return a number of matched fields.
* `top`, returns the greatest value of the argument over all matched fields.

### Formula expressions for all the built-in rankers

Most of the other rankers can actually be emulated with the expression based ranker. You just need to pass a proper expression. Such emulation is, of course, going to be slower than using the built-in, compiled ranker but still might be of interest if you want to fine-tune your ranking formula starting with one of the existing ones. Also, the formulas define the nitty gritty ranker details in a nicely readable fashion.

* proximity_bm25 (default ranker) = `sum(lcs*user_weight)*1000+bm25`
* bm25 = `bm25`
* none = `1`
* wordcount = `sum(hit_count*user_weight)`
* proximity = `sum(lcs*user_weight)`
* matchany = `sum((word_count+(lcs-1)*max_lcs)*user_weight)`
* fieldmask = `field_mask`
* sph04 = `sum((4*lcs+2*(min_hit_pos==1)+exact_hit)*user_weight)*1000+bm25`


### Configuration of IDF formula

The historically default IDF (Inverse Document Frequency) in Manticore is equivalent to `OPTION idf='normalized,tfidf_normalized'`, and those normalizations may cause several undesired effects.

First, `idf=normalized` causes keyword penalization. For instance, if you search for `the | something` and `the` occurs in more than 50% of the documents, then documents with both keywords `the` and`[something` will get less weight than documents with just one keyword  `something`. Using `OPTION idf=plain` avoids this.

Plain IDF varies in `[0, log(N)]` range, and keywords are never penalized; while the normalized IDF varies in `[-log(N), log(N)]` range, and too frequent keywords are penalized.

Second, `idf=tfidf_normalized` causes IDF drift over queries. Historically, we additionally divided IDF by query keyword count, so that the entire `sum(tf*idf)` over all keywords would still fit into `[0,1]` range. However, that means that queries `word1` and `word1 | nonmatchingword2` would assign different weights to the exactly same result set, because the IDFs for both `word1` and `nonmatchingword2` would be divided by 2. `OPTION idf='tfidf_unnormalized'` fixes that. Note that BM25, BM25A, BM25F() ranking factors will be scale accordingly  once you disable this normalization.

IDF flags can be mixed; `plain` and `normalized` are mutually exclusive;`tfidf_unnormalized` and `tfidf_normalized` are mutually exclusive; and unspecified flags in such a mutually exclusive group take their defaults. That means that `OPTION idf=plain` is equivalent to a complete `OPTION idf='plain,tfidf_normalized'` specification.    
