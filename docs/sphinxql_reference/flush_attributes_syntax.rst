.. _flush_attributes_syntax:

FLUSH ATTRIBUTES syntax
-----------------------

.. code-block:: mysql


    FLUSH ATTRIBUTES

Flushes all in-memory attribute updates in all the active disk indexes
to disk. Returns a tag that identifies the result on-disk state
(basically, a number of actual disk attribute saves performed since the
daemon startup).

.. code-block:: mysql


    mysql> UPDATE testindex SET channel_id=1107025 WHERE id=1;
    Query OK, 1 row affected (0.04 sec)

    mysql> FLUSH ATTRIBUTES;
    +------+
    | tag  |
    +------+
    |    1 |
    +------+
    1 row in set (0.19 sec)

