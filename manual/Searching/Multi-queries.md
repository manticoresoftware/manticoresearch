# Multi-queries

Multi-queries, or query batches, let you send multiple search queries to Manticore in one go (more formally, one network request).

👍 Why use multi-queries?

Generally, it all boils down to performance. First, by sending requests to Manticore in a batch instead of one by one, you always save a bit by doing less network round-trips. Second, and somewhat more important, sending queries in a batch enables Manticore to perform certain internal optimizations. In the case when there aren't any possible batch optimizations to apply, queries will be processed one by one internally.

⛔ When not to use multi-queries?

Multi-queries require all the search queries in a batch to be independent, and sometimes they aren't. That is, sometimes query B is based on query A results, and so can only be set up after executing query A. For instance, you might want to display results from a secondary index if and only if there were no results found in a primary table. Or maybe just specify offset into 2nd result set based on the amount of matches in the 1st result set. In that case, you will have to use separate queries (or separate batches).

<!-- example multi-query 1 -->
You can run multiple search queries with SQL by just separating them with a semicolon. When Manticore receives a query formatted like that from a client all the inter-statement optimizations will be applied.

Multi-queries don't support queries with `FACET`. The number of multi-queries in one batch shoudln't exceed [max_batch_queries](../Server_settings/Searchd.md#max_batch_queries).


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price DESC; SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price ASC
```
<!-- end -->

## Multi-queries optimizations

There are two major optimizations to be aware of: common query optimization and common subtree optimization.

**Common query optimization** means that `searchd` will identify all those queries in a batch where only the sorting and group-by settings differ, and *only perform searching once*. For instance, if a batch consists of 3 queries, all of them are for "ipod nano", but 1st query requests top-10 results sorted by price, 2nd query groups by vendor ID and requests top-5 vendors sorted by rating, and 3rd query requests max price, full-text search for "ipod nano" will only be performed once, and its results will be reused to build 3 different result sets.

[Faceted search](../Searching/Faceted_search.md) is a particularly important case that benefits from this optimization. Indeed, faceted searching can be implemented by running a number of queries, one to retrieve search results themselves, and a few other ones with same full-text query but different group-by settings to retrieve all the required groups of results (top-3 authors, top-5 vendors, etc). And as long as full-text query and filtering settings stay the same, common query optimization will trigger, and greatly improve performance.

**Common subtree optimization** is even more interesting. It lets `searchd` exploit similarities between batched full-text queries. It identifies common full-text query parts (subtrees) in all queries, and caches them between queries. For instance, look at the following query batch:

```bash
donald trump president
donald trump barack obama john mccain
donald trump speech
```

There's a common two-word part `donald trump` that can be computed only once, then cached and shared across the queries. And common subtree optimization does just that. Per-query cache size is strictly controlled by [subtree_docs_cache](../Server_settings/Searchd.md#subtree_docs_cache) and [subtree_hits_cache](../Server_settings/Searchd.md#subtree_hits_cache) directives (so that caching *all* sixteen gazillions of documents that match "i am" does not exhaust the RAM and instantly kill your server).

<!-- example multi-query 2 -->
How to tell whether the queries in the batch were actually optimized? If they were, respective query log will have a "multiplier" field that specifies how many queries were processed together:

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

Note how per-query time in multi-query case was improved by a factor of 1.5x to 2.3x, depending on a particular sorting mode.
