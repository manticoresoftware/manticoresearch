.. _truncate_rtindex_syntax:

TRUNCATE RTINDEX syntax
-----------------------

.. code-block:: mysql


    TRUNCATE RTINDEX rtindex [WITH RECONFIGURE]

TRUNCATE RTINDEX clears the RT index completely. It disposes the
in-memory data, unlinks all the index data files, and releases the
associated binary logs.

.. code-block:: mysql


    mysql> TRUNCATE RTINDEX rt;
    Query OK, 0 rows affected (0.05 sec)

You may want to use this if you are using RT indices as “delta index”
files; when you build the main index, you need to wipe the delta index,
and thus TRUNCATE RTINDEX. You also might use this command before
attaching an index; see :ref:`attach_index_syntax`.

When RECONFIGURE option is used new tokenization, morphology, and other
text processing settings from config take effect right after index got
cleared. This allows to make operations atomic.