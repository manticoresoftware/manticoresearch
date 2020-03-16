.. _drop_table_syntax:

DROP TABLE [IF EXISTS] syntax
-----------------------------

.. code-block:: mysql


    DROP TABLE name

DROP TABLE removes an index.
Available for :ref:`rt_mode` only.
Optionally 'IF EXISTS' can be used to not return an error in case the index already doesn't exist.

.. code-block:: mysql


    mysql> DROP TABLE test;
    Query OK, 0 rows affected (0.00 sec)

