.. _show_plan_syntax:

SHOW PLAN syntax
----------------

.. code-block:: mysql


    SHOW PLAN

SHOW PLAN displays the execution plan of the previous SELECT statement.
The plan gets generated and stored during the actual execution, so
profiling must be enabled in the current session **before** running
that statement. That can be done with a ``SET profiling=1`` statement.

Here's a complete instrumentation example:

.. code-block:: mysql


    mysql> SET profiling=1 \G
    Query OK, 0 rows affected (0.00 sec)

    mysql> SELECT id FROM lj WHERE MATCH('the i') LIMIT 1 \G
    *************************** 1\. row ***************************
    id: 39815
    1 row in set (1.53 sec)

    mysql> SHOW PLAN \G
    *************************** 1\. row ***************************
    Variable: transformed_tree
       Value: AND(
      AND(KEYWORD(the, querypos=1)),
      AND(KEYWORD(i, querypos=2)))
    1 row in set (0.00 sec)

And here's a less trivial example that shows how the actually evaluated
query tree can be rather different from the original one because of
expansions and other transformations:

.. code-block:: mysql


    mysql> SELECT * FROM test WHERE MATCH('@title abc* @body hey') \G SHOW PLAN \G
    ...
    *************************** 1\. row ***************************
    Variable: transformed_tree
       Value: AND(
      OR(fields=(title), KEYWORD(abcx, querypos=1, expanded), KEYWORD(abcm, querypos=1, expanded)),
      AND(fields=(body), KEYWORD(hey, querypos=2)))
    1 row in set (0.00 sec)

