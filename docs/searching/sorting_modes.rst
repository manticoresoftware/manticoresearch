.. _sorting_modes:

Sorting modes
-------------

There are the following result sorting modes available:

-  SPH_SORT_RELEVANCE mode, that sorts by relevance in descending
   order (best matches first);

-  SPH_SORT_ATTR_DESC mode, that sorts by an attribute in descending
   order (bigger attribute values first);

-  SPH_SORT_ATTR_ASC mode, that sorts by an attribute in ascending
   order (smaller attribute values first);

-  SPH_SORT_TIME_SEGMENTS mode, that sorts by time segments (last
   hour/day/week/month) in descending order, and then by relevance in
   descending order;

-  SPH_SORT_EXTENDED mode, that sorts by SQL-like combination of
   columns in ASC/DESC order;

-  SPH_SORT_EXPR mode, that sorts by an arithmetic expression.

SPH_SORT_RELEVANCE ignores any additional parameters and always sorts
matches by relevance rank. All other modes require an additional sorting
clause, with the syntax depending on specific mode.
SPH_SORT_ATTR_ASC, SPH_SORT_ATTR_DESC and
SPH_SORT_TIME_SEGMENTS modes require simply an attribute name.
SPH_SORT_RELEVANCE is equivalent to sorting by “@weight DESC, @id ASC”
in extended sorting mode, SPH_SORT_ATTR_ASC is equivalent to
“attribute ASC, @weight DESC, @id ASC”, and SPH_SORT_ATTR_DESC to
“attribute DESC, @weight DESC, @id ASC” respectively.

.. _SPH_SORT_TIME_SEGMENTS_mode:

SPH_SORT_TIME_SEGMENTS mode
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In SPH_SORT_TIME_SEGMENTS mode, attribute values are split into
so-called time segments, and then sorted by time segment first, and by
relevance second.

The segments are calculated according to the *current timestamp* at the
time when the search is performed, so the results would change over
time. The segments are as follows:

-  last hour,

-  last day,

-  last week,

-  last month,

-  last 3 months,

-  everything else.

These segments are hardcoded, but it is trivial to change them if
necessary.

This mode was added to support searching through blogs, news headlines,
etc. When using time segments, recent records would be ranked higher
because of segment, but within the same segment, more relevant records
would be ranked higher - unlike sorting by just the timestamp attribute,
which would not take relevance into account at all.

.. _SPH_SORT_EXTENDED_mode:

SPH_SORT_EXTENDED mode
~~~~~~~~~~~~~~~~~~~~~~~~

In SPH_SORT_EXTENDED mode, you can specify an SQL-like sort expression
with up to 5 attributes (including internal attributes), eg:

::


    @relevance DESC, price ASC, @id DESC

Both internal attributes (that are computed by the engine on the fly)
and user attributes that were configured for this index are allowed.
Internal attribute names must start with magic @-symbol; user attribute
names can be used as is. In the example above, ``@relevance`` and
``@id`` are internal attributes and ``price`` is user-specified.

Known internal attributes are:

-  @id (match ID)

-  @weight (match weight)

-  @rank (match weight)

-  @relevance (match weight)

-  @random (return results in random order)

``@rank`` and ``@relevance`` are just additional aliases to ``@weight``.

.. _SPH_SORT_EXPR_mode:

SPH_SORT_EXPR mode
~~~~~~~~~~~~~~~~~~~~

Expression sorting mode lets you sort the matches by an arbitrary
arithmetic expression, involving attribute values, internal attributes
(@id and @weight), arithmetic operations, and a number of built-in
functions. Here's an example:

::


    $cl->SetSortMode ( SPH_SORT_EXPR,
        "@weight + ( user_karma + ln(pageviews) )*0.1" );

The operators and functions supported in the expressions are discussed
in :ref:`expressions,_functions,_and_operators`.
