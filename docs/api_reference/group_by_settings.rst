GROUP BY settings
-----------------
.. _set_groupby:

SetGroupBy
~~~~~~~~~~

**Prototype:** function SetGroupBy ( $attribute, $func,
$groupsort=“@group desc” )

Sets grouping attribute, function, and groups sorting mode; and enables
grouping (as described in :ref:`grouping_clustering_search_results`).

``$attribute`` is a string that contains group-by attribute name.
``$func`` is a constant that chooses a function applied to the attribute
value in order to compute group-by key. ``$groupsort`` is a clause that
controls how the groups will be sorted. Its syntax is similar to that
described in :ref:`sorting_modes`.

Grouping feature is very similar in nature to GROUP BY clause from SQL.
Results produces by this function call are going to be the same as
produced by the following pseudo code:

.. code-block:: mysql


    SELECT ... GROUP BY $func($attribute) ORDER BY $groupsort

Note that it's ``$groupsort`` that affects the order of matches in the
final result set. Sorting mode (see :ref:`set_sort_mode`)
affect the ordering of matches *within* group, ie. what match will be
selected as the best one from the group. So you can for instance order
the groups by matches count and select the most relevant match within
each group at the same time.

Aggregate functions (AVG(), MIN(), MAX(), SUM()) are supported through
:ref:`SetSelect() <set_select>` API call
when using GROUP BY.

Grouping on string attributes is supported, with respect to current
collation.

.. _set_group_distinct:

SetGroupDistinct
~~~~~~~~~~~~~~~~

**Prototype:** function SetGroupDistinct ( $attribute )

Sets attribute name for per-group distinct values count calculations.
Only available for grouping queries.

``$attribute`` is a string that contains the attribute name. For each
group, all values of this attribute will be stored (as RAM limits
permit), then the amount of distinct values will be calculated and
returned to the client. This feature is similar to ``COUNT(DISTINCT)``
clause in standard SQL; so these Manticore calls:

.. code-block:: php


    $cl->SetGroupBy ( "category", SPH_GROUPBY_ATTR, "@count desc" );
    $cl->SetGroupDistinct ( "vendor" );

can be expressed using the following SQL clauses:

.. code-block:: mysql


    SELECT id, weight, all-attributes,
        COUNT(DISTINCT vendor) AS @distinct,
        COUNT(*) AS @count
    FROM products
    GROUP BY category
    ORDER BY @count DESC

In the sample pseudo code shown just above, ``SetGroupDistinct()`` call
corresponds to ``COUNT(DISINCT vendor)`` clause only. ``GROUP BY``,
``ORDER BY``, and ``COUNT(*)`` clauses are all an equivalent of
``SetGroupBy()`` settings. Both queries will return one matching row for
each category. In addition to indexed attributes, matches will also
contain total per-category matches count, and the count of distinct
vendor IDs within each category.
