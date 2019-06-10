.. _select_systemvariable_syntax:

SELECT @@system_variable syntax
-------------------------------

.. code-block:: mysql


    SELECT @@system_variable [LIMIT [offset,] row_count]

This is currently a placeholder query that does nothing and reports
success. That is in order to keep compatibility with frameworks and
connectors that automatically execute this statement.

However ``@@session.last_insert_id`` and ``LAST_INSERT_ID()`` report
``ID`` of documents these were inserted or replaced well at last statement.

.. code-block:: mysql


    mysql> select @@session.last_insert_id;
    +--------------------------+
    | @@session.last_insert_id |
    +--------------------------+
    | 11,32                    |
    +--------------------------+
    1 rows in set

    mysql> select LAST_INSERT_ID();
    +------------------+
    | LAST_INSERT_ID() |
    +------------------+
    | 25,26,29         |
    +------------------+
    1 rows in set	