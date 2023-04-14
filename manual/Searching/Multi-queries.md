# Multi-queries

Multi-queries, or query batches, allow you to send multiple search queries to Manticore in a single network request.

👍 Why use multi-queries?

The primary reason is performance. By sending requests to Manticore in a batch instead of one by one, you save time by reducing network round-trips. Additionally, sending queries in a batch allows Manticore to perform certain internal optimizations. If no batch optimizations can be applied, queries will be processed individually.

⛔ When not to use multi-queries?

Multi-queries require all search queries in a batch to be independent, which isn't always the case. Sometimes query B depends on query A's results, meaning query B can only be set up after executing query A. For example, you might want to display results from a secondary index only if no results were found in the primary table, or you may want to specify an offset into the 2nd result set based on the number of matches in the 1st result set. In these cases, you'll need to use separate queries (or separate batches).

<!-- example multi-query 1 -->
You can run multiple search queries with SQL by separating them with a semicolon. When Manticore receives a query formatted like this from a client, all inter-statement optimizations will be applied.

Multi-queries don't support queries with `FACET`. The number of multi-queries in one batch shouldn't exceed [max_batch_queries](../Server_settings/Searchd.md#max_batch_queries).


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price DESC; SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price ASC
```
<!-- end -->

## Multi-queries optimizations

There are two major optimizations to be aware of: common query optimization and common subtree optimization.

**Common query optimization** means that `searchd` will identify all those queries in a batch where only the sorting and group-by settings differ, and *only perform searching once*. For example, if a batch consists of 3 queries, all of them are for "ipod nano", but the 1st query requests the top-10 results sorted by price, the 2nd query groups by vendor ID and requests the top-5 vendors sorted by rating, and the 3rd query requests the max price, full-text search for "ipod nano" will only be performed once, and its results will be reused to build 3 different result sets.

[Faceted search](../Searching/Faceted_search.md) is a particularly important case that benefits from this optimization. Indeed, faceted searching can be implemented by running several queries, one to retrieve search results themselves, and a few others with the same full-text query but different group-by settings to retrieve all the required groups of results (top-3 authors, top-5 vendors, etc). As long as the full-text query and filtering settings stay the same, common query optimization will trigger, and greatly improve performance.

**Common subtree optimization** is even more interesting. It allows `searchd` to exploit similarities between batched full-text queries. It identifies common full-text query parts (subtrees) in all queries and caches them between queries. For example, consider the following query batch:

```bash
donald trump president
donald trump barack obama john mccain
donald trump speech
```

There's a common two-word part `donald trump` that can be computed only once, then cached and shared across the queries. And common subtree optimization does just that. Per-query cache size is strictly controlled by [subtree_docs_cache](../Server_settings/Searchd.md#subtree_docs_cache) and [subtree_hits_cache](../Server_settings/Searchd.md#subtree_hits_cache) directives (so that caching *all* sixteen gazillions of documents that match "i am" does not exhaust the RAM and instantly kill your server).

<!-- example multi-query 2 -->
How can you tell if the queries in the batch were actually optimized? If they were, the respective query log will have a "multiplier" field that specifies how many queries were processed together:

Note the "x3" field. It means that this query was optimized and processed in a sub-batch of 3 queries.


<!-- intro -->
##### log:

<!-- request log -->
```bash
[Sun Jul 12 15:18:17.000 2009] 0.040 sec x3 [ext/0/rel 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.000 2009] 0.040 sec x3 [ext/0/ext 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.000 2009] 0.040 sec x3 [ext/0/ext 747541 (0,20)] [lj] the
```
<!-- end -->

<!-- example multi-query 3 -->
For reference, this is how the regular log would look like if the queries were not batched:


<!-- intro -->
##### log:

<!-- request log -->
```bash
[Sun Jul 12 15:18:17.062 2009] 0.059 sec [ext/0/rel 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.156 2009] 0.091 sec [ext/0/ext 747541 (0,20)] [lj] the
[Sun Jul 12 15:18:17.250 2009] 0.092 sec [ext/0/ext 747541 (0,20)] [lj] the
```
<!-- end -->

Notice how the per-query time in the multi-query case improved by a factor of 1.5x to 2.3x, depending on the specific sorting mode.

<!-- proofread -->