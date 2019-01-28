.. _describe_syntax:

DESCRIBE syntax
---------------

.. code-block:: none


    {DESC | DESCRIBE} index [ LIKE pattern ]

DESCRIBE statement lists index columns and their associated types.
Columns are document ID, full-text fields, and attributes. The order
matches that in which fields and attributes are expected by INSERT and
REPLACE statements. Column types are ``field``, ``integer``,
``timestamp``, ``ordinal``, ``bool``, ``float``, ``bigint``, ``string``,
and ``mva``. ID column will be typed as ``bigint``. Example:

.. code-block:: mysql


    mysql> DESC rt;
    +---------+---------+
    | Field   | Type    |
    +---------+---------+
    | id      | bigint  |
    | title   | field   |
    | content | field   |
    | gid     | integer |
    +---------+---------+
    4 rows in set (0.00 sec)

An optional LIKE clause is supported. Refer to :ref:`show_meta_syntax` for its syntax details.


.. _describe_pq_syntax:

Percolate index schemas
-----------------------

If you apply ``DESC`` statement to a percolate index it will show the `outer` schema which is used to view the stored queries. That schema is fixed and the same for all local pq indexes:

.. code-block:: mysql

 mysql> DESC pq;
 +---------+--------+
 | Field   | Type   |
 +---------+--------+
 | id      | bigint |
 | query   | string |
 | tags    | string |
 | filters | string |
 +---------+--------+
 4 rows in set (0.00 sec)

If you're looking for an expected document schema use ``DESC <pq index name> table``:

.. code-block:: mysql

  mysql> DESC pq TABLE;
  +-------+--------+
  | Field | Type   |
  +-------+--------+
  | id    | bigint |
  | title | field  |
  | gid   | uint   |
  +-------+--------+
  3 rows in set (0.00 sec)

Also ``desc pq table like ...`` is possible and works as expected.
