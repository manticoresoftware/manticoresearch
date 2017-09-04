.. _show_tables_syntax:

SHOW TABLES syntax
------------------

.. code-block:: mysql


    SHOW TABLES [ LIKE pattern ]

SHOW TABLES statement enumerates all currently active indexes along with
their types. Existing index types are ``local``, ``distributed``,
``rt``,and ``template`` respectively. Example:

.. code-block:: mysql


    mysql> SHOW TABLES;
    +-------+-------------+
    | Index | Type        |
    +-------+-------------+
    | dist1 | distributed |
    | rt    | rt          |
    | test1 | local       |
    | test2 | local       |
    +-------+-------------+
    4 rows in set (0.00 sec)

An optional LIKE clause is supported. Refer to :ref:`show_meta_syntax` for its syntax details.

.. code-block:: mysql


    mysql> SHOW TABLES LIKE '%4';
    +-------+-------------+
    | Index | Type        |
    +-------+-------------+
    | dist4 | distributed |
    +-------+-------------+
    1 row in set (0.00 sec)

