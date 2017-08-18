.. _grouping_clustering_search_results:

Grouping (clustering) search results
------------------------------------

Sometimes it could be useful to group (or in other terms, cluster)
search results and/or count per-group match counts - for instance, to
draw a nice graph of how much matching blog posts were there per each
month; or to group Web search results by site; or to group matching
forum posts by author; etc.

In theory, this could be performed by doing only the full-text search in
Manticore and then using found IDs to group on SQL server side. However, in
practice doing this with a big result set (10K-10M matches) would
typically kill performance.

To avoid that, Manticore offers so-called grouping mode. It is enabled with
SetGroupBy() API call. When grouping, all matches are assigned to
different groups based on group-by value. This value is computed from
specified attribute using one of the following built-in functions:

-  SPH_GROUPBY_DAY, extracts year, month and day in YYYYMMDD format
   from timestamp;

-  SPH_GROUPBY_WEEK, extracts year and first day of the week number
   (counting from year start) in YYYYNNN format from timestamp;

-  SPH_GROUPBY_MONTH, extracts month in YYYYMM format from timestamp;

-  SPH_GROUPBY_YEAR, extracts year in YYYY format from timestamp;

-  SPH_GROUPBY_ATTR, uses attribute value itself for grouping.

The final search result set then contains one best match per group.
Grouping function value and per-group match count are returned along as
“virtual” attributes named **@group** and **@count** respectively.

The result set is sorted by group-by sorting clause, with the syntax
similar to ```SPH_SORT_EXTENDED`` sorting
clause <SPH_SORT_EXTENDED_mode>` syntax. In addition
to ``@id`` and ``@weight``, group-by sorting clause may also include:

-  @group (groupby function value),

-  @count (amount of matches in group).

The default mode is to sort by groupby value in descending order, ie. by
``@group desc``.

On completion, ``total_found`` result parameter would contain total
amount of matching groups over he whole index.

**WARNING:** grouping is done in fixed memory and thus its results are
only approximate; so there might be more groups reported in
``total_found`` than actually present. ``@count`` might also be
underestimated. To reduce inaccuracy, one should raise ``max_matches``.
If ``max_matches`` allows to store all found groups, results will be
100% correct.

For example, if sorting by relevance and grouping by
``"published"`` attribute with ``SPH_GROUPBY_DAY`` function,
then the result set will contain

-  one most relevant match per each day when there were any matches
   published,

-  with day number and per-day match count attached,

-  sorted by day number in descending order (ie. recent days first).

Aggregate functions (AVG(), MIN(), MAX(), SUM()) are supported through
:ref:`SetSelect() <set_select>` API call when
using GROUP BY.
