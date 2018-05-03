.. _faceted_search:

Faceted search
--------------

In addition to the standard set of search results, a faceted search provides facet results from dynamic clustering of items or search results into categories.
Facets allow users to **navigate** the search query by narrowing it using the facet results.

In Manticore, facets can be added to any query and the facet can be any attribute or an expression. A facet results contains the facet values along with the facet counts.
Facets are available using SphinxQL :ref:`SELECT <select_facet>` statement by declaring them at the very end of the query in the format:

.. code-block:: none

       FACET {expr_list} [BY {expr_list}] [ORDER BY {expr | FACET()} {ASC | DESC}] [LIMIT [offset,] count]

Multiple facet declarations need to be separate by an whitespace.


Aggregations
~~~~~~~~~~~~

The facet values can come from an attribute, JSON property from a JSON attribute or expression. Facet values can be also aliased, however the **alias must be unique** across all result sets (main query results set and other facets results sets). The facet value is taken from the aggregated attribute/expression, but it can also come from another attribute/expression.

.. code-block:: none
   
   MySQL [(none)]> SELECT *, price as aprice FROM facetdemo LIMIT 10 FACET price LIMIT 10 FACET brand_id LIMIT 5;
   +------+-------+----------+---------------------+------------+-------------+---------------------------------------+------------+--------+
   | id   | price | brand_id | title               | brand_name | property    | j                                     | categories | aprice |
   +------+-------+----------+---------------------+------------+-------------+---------------------------------------+------------+--------+
   |    1 |   306 |        1 | Product Ten Three   | Brand One  | Six_Ten     | {"prop1":66,"prop2":91,"prop3":"One"} | 10,11      |    306 |
   |    2 |   400 |       10 | Product Three One   | Brand Ten  | Four_Three  | {"prop1":69,"prop2":19,"prop3":"One"} | 13,14      |    400 |
   ...
   |    9 |   560 |        6 | Product Two Five    | Brand Six  | Eight_Two   | {"prop1":90,"prop2":84,"prop3":"One"} | 13,14      |    560 |
   |   10 |   229 |        9 | Product Three Eight | Brand Nine | Seven_Three | {"prop1":84,"prop2":39,"prop3":"One"} | 12,13      |    229 |
   +------+-------+----------+---------------------+------------+-------------+---------------------------------------+------------+--------+
   10 rows in set (0.00 sec)
   +-------+----------+
   | price | count(*) |
   +-------+----------+
   |   306 |        7 |
   |   400 |       13 |
   ...
   |   229 |        9 |
   |   595 |       10 |
   +-------+----------+
   10 rows in set (0.00 sec)
   +----------+----------+
   | brand_id | count(*) |
   +----------+----------+
   |        1 |     1013 |
   |       10 |      998 |
   |        5 |     1007 |
   |        8 |     1033 |
   |        7 |      965 |
   +----------+----------+
   5 rows in set (0.00 sec)


Data can be faceted by aggregating another attribute or expression. For example if the documents contain both the brand id and name, we can return in facet the brand names, but aggregate the brand ids. This can be done by using ``FACET {expr1} BY {expr2}``



.. code-block:: none
   
   MySQL [(none)]> SELECT * FROM facetdemo FACET brand_name by brand_id;
   +------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
   | id   | price | brand_id | title               | brand_name  | property    | j                                     | categories |
   +------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
   |    1 |   306 |        1 | Product Ten Three   | Brand One   | Six_Ten     | {"prop1":66,"prop2":91,"prop3":"One"} | 10,11      |
   |    2 |   400 |       10 | Product Three One   | Brand Ten   | Four_Three  | {"prop1":69,"prop2":19,"prop3":"One"} | 13,14      |
   ....
   |   19 |   855 |        1 | Product Seven Two   | Brand One   | Eight_Seven | {"prop1":63,"prop2":78,"prop3":"One"} | 10,11,12   |
   |   20 |    31 |        9 | Product Four One    | Brand Nine  | Ten_Four    | {"prop1":79,"prop2":42,"prop3":"One"} | 12,13,14   |
   +------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
   20 rows in set (0.00 sec)
   +-------------+----------+
   | brand_name  | count(*) |
   +-------------+----------+
   | Brand One   |     1013 |
   | Brand Ten   |      998 |
   | Brand Five  |     1007 |
   | Brand Nine  |      944 |
   | Brand Two   |      990 |
   | Brand Six   |     1039 |
   | Brand Three |     1016 |
   | Brand Four  |      994 |
   | Brand Eight |     1033 |
   | Brand Seven |      965 |
   +-------------+----------+
   10 rows in set (0.00 sec)


Facets can aggregate over multi-level grouping, the result set being the same as the query would perform a multi-level grouping:

.. code-block:: none
   
   MySQL [(none)]> select *,INTERVAL(price,200,400,600,800) as price_range from facetdemo facet price_range as fprice_range,brand_name order by brand_name asc;
   +------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+-------------+
   | id   | price | brand_id | title               | brand_name  | property    | j                                     | categories | price_range |
   +------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+-------------+
   |    1 |   306 |        1 | Product Ten Three   | Brand One   | Six_Ten     | {"prop1":66,"prop2":91,"prop3":"One"} | 10,11      |           1 |
   ...
   +------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+-------------+
   20 rows in set (0.00 sec)
   
   +--------------+-------------+----------+
   | fprice_range | brand_name  | count(*) |
   +--------------+-------------+----------+
   |            1 | Brand Eight |      197 |
   |            4 | Brand Eight |      235 |
   |            3 | Brand Eight |      203 |
   |            2 | Brand Eight |      201 |
   |            0 | Brand Eight |      197 |
   |            4 | Brand Five  |      230 |
   |            2 | Brand Five  |      197 |
   |            1 | Brand Five  |      204 |
   |            3 | Brand Five  |      193 |
   |            0 | Brand Five  |      183 |
   |            1 | Brand Four  |      195 |
   ...

Facets support order clause as same as a standard query. In addition, special ``FACET()`` function can be used, which provides the aggregated data values.


.. code-block:: none
   
   MySQL [(none)]> select * from facetdemo facet brand_name by brand_id order by facet() asc facet brand_name by brand_id order by brand_name asc facet brand_name by brand_id order by count(*) desc;
   +------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
   | id   | price | brand_id | title               | brand_name  | property    | j                                     | categories |
   +------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
   |    1 |   306 |        1 | Product Ten Three   | Brand One   | Six_Ten     | {"prop1":66,"prop2":91,"prop3":"One"} | 10,11      |
   ...
   |   20 |    31 |        9 | Product Four One    | Brand Nine  | Ten_Four    | {"prop1":79,"prop2":42,"prop3":"One"} | 12,13,14   |
   +------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
   20 rows in set (0.01 sec)
   
   +-------------+----------+
   | brand_name  | count(*) |
   +-------------+----------+
   | Brand One   |     1013 |
   | Brand Two   |      990 |
   | Brand Three |     1016 |
   | Brand Four  |      994 |
   | Brand Five  |     1007 |
   | Brand Six   |     1039 |
   | Brand Seven |      965 |
   | Brand Eight |     1033 |
   | Brand Nine  |      944 |
   | Brand Ten   |      998 |
   +-------------+----------+
   10 rows in set (0.01 sec)
   
   +-------------+----------+
   | brand_name  | count(*) |
   +-------------+----------+
   | Brand Eight |     1033 |
   | Brand Five  |     1007 |
   | Brand Four  |      994 |
   | Brand Nine  |      944 |
   | Brand One   |     1013 |
   | Brand Seven |      965 |
   | Brand Six   |     1039 |
   | Brand Ten   |      998 |
   | Brand Three |     1016 |
   | Brand Two   |      990 |
   +-------------+----------+
   10 rows in set (0.01 sec)
   
   +-------------+----------+
   | brand_name  | count(*) |
   +-------------+----------+
   | Brand Six   |     1039 |
   | Brand Eight |     1033 |
   | Brand Three |     1016 |
   | Brand One   |     1013 |
   | Brand Five  |     1007 |
   | Brand Ten   |      998 |
   | Brand Four  |      994 |
   | Brand Two   |      990 |
   | Brand Seven |      965 |
   | Brand Nine  |      944 |
   +-------------+----------+
   10 rows in set (0.01 sec)

Returned result set
~~~~~~~~~~~~~~~~~~~

As it can be seen in the examples above, a faceted search will return a multiple result sets response.  The MySQL client/library/connector used **must** have support (most do) for multiple result sets in order to be able to access the facet result sets.

By default, the facet result is not ordered and only the first 20 facet values are returned. The number of facet values can be controlled with ``LIMIT`` clause individually for each facet. The maximum facet values that can be returned is limited by the query's ``max_matches`` setting. In case dynamic max_matches (limiting max_matches to offset+per page for better performance) is wanted to be implemented, it must be taken in account that a too low max_matches value can hurt the number of facet values. In this case, a minimum max_matches value should be used good enough to cover the number of facet values.

Performance
~~~~~~~~~~~
Internally, the FACET is a shorthand for executing a multi-query where the first query contains the main search query and the rest of the queries in the batch have each a clustering.
As in the case of multi-query, the :ref:`common query optimization <common_query_optimization>` can kick-in for a faceted search, meaning the  search query is executed only once and the facets operates on the search query result, each facet adding only a fraction of time to the total query time. 

To check if the faceted search ran in an optimized mode can be seen in query.log, where all the logged queries will contain a ``xN`` string, where ``N`` is the number of queries that ran in the optimized group or checking the output of :ref:`SHOW META <show_meta_syntax>` statement which will exhibit a ``multiplier`` metric:


.. code-block:: none
   
   MySQL [(none)]> select * from facetdemo facet brand_id facet price facet categories;show meta like 'multiplier';
   +------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
   | id   | price | brand_id | title               | brand_name  | property    | j                                     | categories |
   +------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
   |    1 |   306 |        1 | Product Ten Three   | Brand One   | Six_Ten     | {"prop1":66,"prop2":91,"prop3":"One"} | 10,11      |
   ...
   
   +----------+----------+
   | brand_id | count(*) |
   +----------+----------+
   |        1 |     1013 |
   ...
   
   +-------+----------+
   | price | count(*) |
   +-------+----------+
   |   306 |        7 |
   ...
   
   +------------+----------+
   | categories | count(*) |
   +------------+----------+
   |         10 |     2436 |
   ...
   
   +---------------+-------+
   | Variable_name | Value |
   +---------------+-------+
   | multiplier    | 4     |
   +---------------+-------+
   1 row in set (0.00 sec)

