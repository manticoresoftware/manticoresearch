.. _explain_query_syntax:

EXPLAIN QUERY syntax
--------------------

.. code-block:: none


    EXPLAIN QUERY index query

EXPLAIN QUERY statement displays the execution tree of the provided full-text query.
This statements works for any kind of indexes such as template, plain, real-time, percolate.

.. code-block:: none


    mysql> EXPLAIN QUERY index_base 'Sally and I'\G
    *************************** 1\. row ***************************
    Variable: transformed_tree
       Value: AND(
      AND(KEYWORD(sally, querypos=1)),
      AND(KEYWORD(and, querypos=2)),	  
      AND(KEYWORD(i, querypos=3)))
    1 row in set (0.00 sec)

And here's a less trivial example that shows how the actually evaluated
query tree can be rather different from the original one because transformations:

.. code-block:: none


    mysql> EXPLAIN QUERY index_base '@title running @body dog'\G
    *************************** 1\. row ***************************
    Variable: transformed_tree
       Value: AND(
	  OR(
		AND(fields=(title), KEYWORD(run, querypos=1, morphed)),
		AND(fields=(title), KEYWORD(running, querypos=1, morphed))))
      AND(fields=(body), KEYWORD(dog, querypos=2, morphed)))
    1 row in set (0.00 sec)


It is good habit to create template index as base with all settings set 
and inherit all other indexes from it like main, delta and other to eliminate multiple copies
of the same settings at every index. This template index could be used in statements such as
:ref:`call_snippets_syntax` :ref:`call_keywords_syntax` :ref:`explain_query_syntax` and so on.
