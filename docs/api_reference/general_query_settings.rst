General query settings
----------------------

.. _set_select:

SetSelect
~~~~~~~~~

**Prototype:** function SetSelect ( $clause )

Sets the select clause, listing specific attributes to fetch, and
:ref:`sorting_modes`
to compute and fetch. Clause syntax mimics SQL.

SetSelect() is very similar to the part of a typical SQL query between
SELECT and FROM. It lets you choose what attributes (columns) to fetch,
and also what expressions over the columns to compute and fetch. A
certain difference from SQL is that expressions **must** always be
aliased to a correct identifier (consisting of letters and digits) using
‘AS’ keyword. SQL also lets you do that but does not require to. Manticore
enforces aliases so that the computation results can always be returned
under a “normal” name in the result set, used in other clauses, etc.

Everything else is basically identical to SQL. Star ('\*') is supported.
Functions are supported. Arbitrary amount of expressions is supported.
Computed expressions can be used for sorting, filtering, and grouping,
just as the regular attributes.

When using GROUP BY agregate functions (AVG(), MIN(), MAX(), SUM()) are
supported.

Expression sorting (:ref:`sorting_modes`) and
geodistance functions (:ref:`set_geo_anchor`)
are now internally implemented using this computed expressions
mechanism, using magic names ‘@expr’ and ‘@geodist’ respectively.

Example:


.. code-block:: php


    $cl->SetSelect ( "*, @weight+(user_karma+ln(pageviews))*0.1 AS myweight" );
    $cl->SetSelect ( "exp_years, salary_gbp*{$gbp_usd_rate} AS salary_usd,
       IF(age>40,1,0) AS over40" );
    $cl->SetSelect ( "*, AVG(price) AS avgprice" );

.. _set_limits:

SetLimits
~~~~~~~~~

**Prototype:** function SetLimits ( $offset, $limit,
$max_matches=1000, $cutoff=0 )

Sets offset into server-side result set (``$offset``) and amount of
matches to return to client starting from that offset (``$limit``). Can
additionally control maximum server-side result set size for current
query (``$max_matches``) and the threshold amount of matches to stop
searching at (``$cutoff``). All parameters must be non-negative
integers.

First two parameters to SetLimits() are identical in behavior to MySQL
LIMIT clause. They instruct ``searchd`` to return at most ``$limit``
matches starting from match number ``$offset``. The default offset and
limit settings are 0 and 20, that is, to return first 20 matches.

``max_matches`` setting controls how much matches ``searchd`` will keep
in RAM while searching. **All** matching documents will be normally
processed, ranked, filtered, and sorted even if ``max_matches`` is set
to 1. But only best N documents are stored in memory at any given moment
for performance and RAM usage reasons, and this setting controls that N.
Note that there are **two** places where ``max_matches`` limit is
enforced. Per-query limit is controlled by this API call, but there also
is per-server limit controlled by ``max_matches`` setting in the config
file. To prevent RAM usage abuse, server will not allow to set per-query
limit higher than the per-server limit.

You can't retrieve more than ``max_matches`` matches to the client
application. The default limit is set to 1000. Normally, you must not
have to go over this limit. One thousand records is enough to present to
the end user. And if you're thinking about pulling the results to
application for further sorting or filtering, that would be **much**
more efficient if performed on Manticore side.

``$cutoff`` setting is intended for advanced performance control. It
tells ``searchd`` to forcibly stop search query once ``$cutoff`` matches
had been found and processed.

.. _set_max_query_time:

SetMaxQueryTime
~~~~~~~~~~~~~~~

**Prototype:** function SetMaxQueryTime ( $max_query_time )

Sets maximum search query time, in milliseconds. Parameter must be a
non-negative integer. Default value is 0 which means “do not limit”.

Similar to ``$cutoff`` setting from
:ref:`SetLimits() <set_limits>`, but limits
elapsed query time instead of processed matches count. Local search
queries will be stopped once that much time has elapsed. Note that if
you're performing a search which queries several local indexes, this
limit applies to each index separately.


.. _set_override:

SetOverride
~~~~~~~~~~~

**DEPRECATED**

**Prototype:** function SetOverride ( $attrname, $attrtype, $values )

Sets temporary (per-query) per-document attribute value overrides. Only
supports scalar attributes. $values must be a hash that maps document
IDs to overridden attribute values.

Override feature lets you “temporary” update attribute values for some
documents within a single query, leaving all other queries unaffected.
This might be useful for personalized data. For example, assume you're
implementing a personalized search function that wants to boost the
posts that the user's friends recommend. Such data is not just dynamic,
but also personal; so you can't simply put it in the index because you
don't want everyone's searches affected. Overrides, on the other hand,
are local to a single query and invisible to everyone else. So you can,
say, setup a “friends_weight” value for every document, defaulting to
0, then temporary override it with 1 for documents 123, 456 and 789
(recommended by exactly the friends of current user), and use that value
when ranking.
