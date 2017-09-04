.. _querying:

Querying
--------

.. _add_query:

AddQuery
~~~~~~~~

**Prototype:** function AddQuery ( $query, $index=“\*“, $comment=”" )

Adds additional query with current settings to multi-query batch.
``$query`` is a query string. ``$index`` is an index name (or names)
string. Additionally if provided, the contents of ``$comment`` are sent
to the query log, marked in square brackets, just before the search
terms, which can be very useful for debugging. Currently, this is
limited to 128 characters. Returns index to results array returned from
:ref:`run_queries`.

Batch queries (or multi-queries) enable ``searchd`` to perform internal
optimizations if possible. They also reduce network connection overheads
and search process creation overheads in all cases. They do not result
in any additional overheads compared to simple queries. Thus, if you run
several different queries from your web page, you should always consider
using multi-queries.

For instance, running the same full-text query but with different
sorting or group-by settings will enable ``searchd`` to perform
expensive full-text search and ranking operation only once, but compute
multiple group-by results from its output.

This can be a big saver when you need to display not just plain search
results but also some per-category counts, such as the amount of
products grouped by vendor. Without multi-query, you would have to run
several queries which perform essentially the same search and retrieve
the same matches, but create result sets differently. With multi-query,
you simply pass all these queries in a single batch and Manticore optimizes
the redundant full-text search internally.

``AddQuery()`` internally saves full current settings state along with
the query, and you can safely change them afterwards for subsequent
``AddQuery()`` calls. Already added queries will not be affected;
there's actually no way to change them at all. Here's an example:

.. code-block:: php


    $cl->SetSortMode ( SPH_SORT_RELEVANCE );
    $cl->AddQuery ( "hello world", "documents" );

    $cl->SetSortMode ( SPH_SORT_ATTR_DESC, "price" );
    $cl->AddQuery ( "ipod", "products" );

    $cl->AddQuery ( "harry potter", "books" );

    $results = $cl->RunQueries ();

With the code above, 1st query will search for “hello world” in
“documents” index and sort results by relevance, 2nd query will search
for “ipod” in “products” index and sort results by price, and 3rd query
will search for “harry potter” in “books” index while still sorting by
price. Note that 2nd ``SetSortMode()`` call does not affect the first
query (because it's already added) but affects both other subsequent
queries.

Additionally, any filters set up before an ``AddQuery()`` will fall
through to subsequent queries. So, if ``SetFilter()`` is called before
the first query, the same filter will be in place for the second (and
subsequent) queries batched through ``AddQuery()`` unless you call
``ResetFilters()`` first. Alternatively, you can add additional filters
as well.

This would also be true for grouping options and sorting options; no
current sorting, filtering, and grouping settings are affected by this
call; so subsequent queries will reuse current query settings.

``AddQuery()`` returns an index into an array of results that will be
returned from ``RunQueries()`` call. It is simply a sequentially
increasing 0-based integer, ie. first call will return 0, second will
return 1, and so on. Just a small helper so you won't have to track the
indexes manually if you need then.

.. _query:

Query
~~~~~

**Prototype:** function Query ( $query, $index=“\*“, $comment=”" )

Connects to ``searchd`` server, runs given search query with current
settings, obtains and returns the result set.

``$query`` is a query string. ``$index`` is an index name (or names)
string. Returns false and sets ``GetLastError()`` message on general
error. Returns search result set on success. Additionally, the contents
of ``$comment`` are sent to the query log, marked in square brackets,
just before the search terms, which can be very useful for debugging.
Currently, the comment is limited to 128 characters.

Default value for ``$index`` is ``"*"`` that means to query
all local indexes. Characters allowed in index names include Latin
letters (a-z), numbers (0-9) and underscore (_); everything else is
considered a separator. Note that index name should not start with
underscore character. Therefore, all of the following samples calls are
valid and will search the same two indexes:

.. code-block:: php


    $cl->Query ( "test query", "main delta" );
    $cl->Query ( "test query", "main;delta" );
    $cl->Query ( "test query", "main, delta" );

Index specification order matters. If document with identical IDs are
found in two or more indexes, weight and attribute values from the very
last matching index will be used for sorting and returning to client
(unless explicitly overridden with
:ref:`SetIndexWeights() <set_index_weights>`).
Therefore, in the example above, matches from “delta” index will always
win over matches from “main”.

On success, ``Query()`` returns a result set that contains some of the
found matches (as requested by
:ref:`SetLimits() <set_limits>`) and
additional general per-query statistics. The result set is a hash (PHP
specific; other languages might utilize other structures instead of
hash) with the following keys and values:

-  “matches”:
-  Hash which maps found document IDs to another small hash containing
   document weight and attribute values (or an array of the similar
   small hashes if
   :ref:`SetArrayResult() <set_array_result>`
   was enabled).

-  “total”:
-  Total amount of matches retrieved *on server* (ie. to the server side
   result set) by this query. You can retrieve up to this amount of
   matches from server for this query text with current query settings.

-  “total_found”:
-  Total amount of matching documents in index (that were found and
   processed on server).

-  “words”:
-  Hash which maps query keywords (case-folded, stemmed, and otherwise
   processed) to a small hash with per-keyword statistics (“docs”,
   “hits”).

-  “error”:
-  Query error message reported by ``searchd`` (string, human readable).
   Empty if there were no errors.

-  “warning”:
-  Query warning message reported by ``searchd`` (string, human
   readable). Empty if there were no warnings.

It should be noted that ``Query()`` carries out the same actions as
``AddQuery()`` and ``RunQueries()`` without the intermediate steps; it
is analogous to a single ``AddQuery()`` call, followed by a
corresponding ``RunQueries()``, then returning the first array element
of matches (from the first, and only, query.)

.. _run_queries:

RunQueries
~~~~~~~~~~

**Prototype:** function RunQueries ()

Connect to searchd, runs a batch of all queries added using
``AddQuery()``, obtains and returns the result sets. Returns false and
sets ``GetLastError()`` message on general error (such as network I/O
failure). Returns a plain array of result sets on success.

Each result set in the returned array is exactly the same as the result
set returned from :ref:`query`.

Note that the batch query request itself almost always succeeds - unless
there's a network error, blocking index rotation in progress, or another
general failure which prevents the whole request from being processed.

However individual queries within the batch might very well fail. In
this case their respective result sets will contain non-empty
``&quot;error&quot;`` message, but no matches or query statistics. In
the extreme case all queries within the batch could fail. There still
will be no general error reported, because API was able to successfully
connect to ``searchd``, submit the batch, and receive the results - but
every result set will have a specific error message.

.. _reset_filters:

ResetFilters
~~~~~~~~~~~~

**Prototype:** function ResetFilters ()

Clears all currently set filters.

This call is only normally required when using multi-queries. You might
want to set different filters for different queries in the batch. To do
that, you should call ``ResetFilters()`` and add new filters using the
respective calls.


.. _reset_group_by:

ResetGroupBy
~~~~~~~~~~~~

**Prototype:** function ResetGroupBy ()

Clears all currently group-by settings, and disables group-by.

This call is only normally required when using multi-queries. You can
change individual group-by settings using ``SetGroupBy()`` and
``SetGroupDistinct()`` calls, but you can not disable group-by using
those calls. ``ResetGroupBy()`` fully resets previous group-by settings
and disables group-by mode in the current state, so that subsequent
``AddQuery()`` calls can perform non-grouping searches.
