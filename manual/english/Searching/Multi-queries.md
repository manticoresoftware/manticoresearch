# Multi-queries

Multi-queries, or query batches, allow you to send multiple search queries to Manticore in a single network request.

👍 Why use multi-queries?

The primary reason is performance. By sending requests to Manticore in a batch instead of one by one, you save time by reducing network round-trips. Additionally, sending queries in a batch allows Manticore to perform certain internal optimizations. If no batch optimizations can be applied, queries will be processed individually.

⛔ When not to use multi-queries?

Multi-queries require all search queries in a batch to be independent, which isn't always the case. Sometimes query B depends on query A's results, meaning query B can only be set up after executing query A. For example, you might want to display results from a secondary index only if no results were found in the primary table, or you may want to specify an offset into the 2nd result set based on the number of matches in the 1st result set. In these cases, you'll need to use separate queries (or separate batches).

When using connector libraries, such as `mysqli` in PHP, you can add multiple queries and then run them all as a single batch. This will work as a single multi-query batch.

Note: If you use a console MySQL client, by default it interprets the semicolon (;) as the delimiter itself, and sends each query to the server individually; this is not a multi-query batch. To override this behavior, redefine the separator on the client-side to another character using the internal command `delimiter`. After making this change, the client will send the entire string with semicolons unchanged, allowing the "multi-query magic" to work.

This aside behavior of the console client can often be confusing because you might notice that one and the same sequence of commands behaves differently in the MySQL client console compared to another protocol like SQL-over-HTTP. This is exactly because the MySQL console client itself divides queries using semicolons, but other protocols may send an entire sequence as a single batch.

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

From a console MySQL/MariaDB client:
<!-- request SQL -->

```sql
DELIMITER _
SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price DESC; SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price ASC_
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price DESC; SELECT id, price FROM products WHERE MATCH('remove hair') ORDER BY price ASC"
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

## Multi-Query Limitations and Flow

Multi-queries are mainly supported for batching queries and receiving meta-info after such batches. Because of this limitation, only a small subset of statements is allowed in batches. In one batch, you can combine only `SELECT`, `SHOW`, and `SET` statements.

You can use `SELECT` as usual; however, notice that all queries will be run together in a single pass. If queries are not related, there is no benefit from multi-querying. The daemon will detect this and run the queries one-by-one.

You can use `SHOW` for processing *warnings*, *status*, *agent status*, *meta*, *profile*, and *plan*. All other `SHOW` statements in batches will be silently ignored with no output. For example, you cannot execute `SHOW TABLES`, `SHOW THREADS`, or `SHOW VARIABLES`, or any other statement not mentioned above, when batching.

You can use `SET` only for `SET PROFILING`. All other `SET ...` commands will be silently ignored.

The order of execution is also different. The daemon processes batches in two passes.

First, it collects all `SELECT` statements and runs all `SET PROFILING` statements it sees simultaneously. As a side effect, only the last `SET PROFILING` statement is effective. If you execute a multi-query like this: `SET PROFILING=1; SELECT...; SHOW META; SHOW PROFILE; SET PROFILING=0`, you will not see any profile, because on the first pass, the daemon executes `SET PROFILING=1` and then immediately `SET PROFILING=0`.

Second, the daemon attempts to execute a single batch query with all collected `SELECT` statements. If statements are not related, it will execute them one-by-one.

Finally, it iterates over the initial batch sequence and returns the sub-result data and meta from the resultset for
each `SELECT` and `SHOW`. Since all `SET PROFILING` statements were executed in the first pass, they are skipped on this second pass.