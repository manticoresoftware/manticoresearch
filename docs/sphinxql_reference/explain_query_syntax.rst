.. _explain_query_syntax:

EXPLAIN QUERY syntax
--------------------

.. code-block:: none


    EXPLAIN QUERY index query

EXPLAIN QUERY displays execution tree of the provided full-text query.
This statement works for any kind of indexes such as template, plain, real-time or percolate.

.. code-block:: none


    mysql> EXPLAIN QUERY index_base 'Sally and I'\G
    *************************** 1\. row ***************************
    Variable: transformed_tree
       Value: AND(
      AND(KEYWORD(sally, querypos=1)),
      AND(KEYWORD(and, querypos=2)),
      AND(KEYWORD(i, querypos=3)))
    1 row in set (0.00 sec)

Here's a less trivial example that shows how the evaluated query tree can be rather
different from the original one because of transformations:

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


It is a good habit to create a template index as a base one with all needed settings set
and inherit all other indexes from it, for example: main, delta and other to avoid
config code duplication in every index. That template index can then be used in
statements such as :ref:`explain_query_syntax`, :ref:`call_snippets_syntax`,
:ref:`call_keywords_syntax` and others.
