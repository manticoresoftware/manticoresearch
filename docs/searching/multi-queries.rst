.. _multi_queries:

Multi-queries
-------------

Multi-queries, or query batches, let you send multiple queries to Manticore
in one go (more formally, one network request).

Two API methods that implement multi-query mechanism are
:ref:`AddQuery() <add_query>` and
:ref:`RunQueries() <run_queries>`. You can also run multiple
queries with SphinxQL, see `the section called :ref:`multi-statement_queries`. (In fact, regular
:ref:`Query() <add_query>` call is internally implemented as
a single AddQuery() call immediately followed by RunQueries() call.)
AddQuery() captures the current state of all the query settings set by
previous API calls, and memorizes the query. RunQueries() actually sends
all the memorized queries, and returns multiple result sets. There are
no restrictions on the queries at all, except just a sanity check on a
number of queries in a single batch (see :ref:`max_batch_queries`).

Why use multi-queries? Generally, it all boils down to performance.
First, by sending requests to ``searchd`` in a batch instead of one by
one, you always save a bit by doing less network roundtrips. Second, and
somewhat more important, sending queries in a batch enables ``searchd``
to perform certain internal optimizations. As new types of optimizations
are being added over time, it generally makes sense to pack all the
queries into batches where possible, so that simply upgrading Manticore to
a new version would automatically enable new optimizations. In the case
when there aren't any possible batch optimizations to apply, queries
will be processed one by one internally.

Why (or rather when) not use multi-queries? Multi-queries requires all
the queries in a batch to be independent, and sometimes they aren't.
That is, sometimes query B is based on query A results, and so can only
be set up after executing query A. For instance, you might want to
display results from a secondary index if and only if there were no
results found in a primary index. Or maybe just specify offset into 2nd
result set based on the amount of matches in the 1st result set. In that
case, you will have to use separate queries (or separate batches).

There are two major optimizations to be aware of: common query
optimization and common subtree optimization.

**Common query optimization** means that ``searchd`` will identify
all those queries in a batch where only the sorting and group-by
settings differ, and *only perform searching once*. For instance, if a
batch consists of 3 queries, all of them are for “ipod nano”, but 1st
query requests top-10 results sorted by price, 2nd query groups by
vendor ID and requests top-5 vendors sorted by rating, and 3rd query
requests max price, full-text search for “ipod nano” will only be
performed once, and its results will be reused to build 3 different
result sets.

So-called **faceted searching** is a particularly important case that
benefits from this optimization. Indeed, faceted searching can be
implemented by running a number of queries, one to retrieve search
results themselves, and a few other ones with same full-text query but
different group-by settings to retrieve all the required groups of
results (top-3 authors, top-5 vendors, etc). And as long as full-text
query and filtering settings stay the same, common query optimization
will trigger, and greatly improve performance.

**Common subtree optimization** is even more interesting. It lets
``searchd`` exploit similarities between batched full-text queries. It
identifies common full-text query parts (subtrees) in all queries, and
caches them between queries. For instance, look at the following query
batch:

.. code-block:: mysql


    donald trump president
    donald trump barack obama john mccain
    donald trump speech

There's a common two-word part (“donald trump”) that can be computed
only once, then cached and shared across the queries. And common subtree
optimization does just that. Per-query cache size is strictly controlled
by
:ref:`subtree_docs_cache <subtree_docs_cache>`
and
:ref:`subtree_hits_cache <subtree_hits_cache>`
directives (so that caching *all* sixteen gazillions of documents that
match “i am” does not exhaust the RAM and instantly kill your server).

Here's a code sample (in PHP) that fire the same query in 3 different
sorting modes:

.. code-block:: php


    require ( "sphinxapi.php" );
    $cl = new ManticoreClient ();
    $cl->SetMatchMode ( SPH_MATCH_EXTENDED );

    $cl->SetSortMode ( SPH_SORT_RELEVANCE );
    $cl->AddQuery ( "the", "lj" );
    $cl->SetSortMode ( SPH_SORT_EXTENDED, "published desc" );
    $cl->AddQuery ( "the", "lj" );
    $cl->SetSortMode ( SPH_SORT_EXTENDED, "published asc" );
    $cl->AddQuery ( "the", "lj" );
    $res = $cl->RunQueries();

How to tell whether the queries in the batch were actually optimized? If
they were, respective query log will have a “multiplier” field that
specifies how many queries were processed together:

.. code-block:: mysql


    [Sun Jul 12 15:18:17.000 2009] 0.040 sec x3 [ext/0/rel 747541 (0,20)] [lj] the
    [Sun Jul 12 15:18:17.000 2009] 0.040 sec x3 [ext/0/ext 747541 (0,20)] [lj] the
    [Sun Jul 12 15:18:17.000 2009] 0.040 sec x3 [ext/0/ext 747541 (0,20)] [lj] the

Note the “x3” field. It means that this query was optimized and
processed in a sub-batch of 3 queries. For reference, this is how the
regular log would look like if the queries were not batched:

.. code-block:: mysql


    [Sun Jul 12 15:18:17.062 2009] 0.059 sec [ext/0/rel 747541 (0,20)] [lj] the
    [Sun Jul 12 15:18:17.156 2009] 0.091 sec [ext/0/ext 747541 (0,20)] [lj] the
    [Sun Jul 12 15:18:17.250 2009] 0.092 sec [ext/0/ext 747541 (0,20)] [lj] the

Note how per-query time in multi-query case was improved by a factor of
1.5x to 2.3x, depending on a particular sorting mode. In fact, for both
common query and common subtree optimizations, there were reports of 3x
and even more improvements, and that's from production instances, not
just synthetic tests.
