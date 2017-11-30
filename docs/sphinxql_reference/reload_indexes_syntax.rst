.. _reload_indexes_syntax:

RELOAD INDEXES syntax
---------------------

.. code-block:: mysql


    RELOAD INDEXES

Works same as system HUP signal.
Initiates index rotation. Depending on the value of
:ref:`seamless_rotate <seamless_rotate>`
setting, new queries might be shortly stalled; clients will receive
temporary errors.
Command is non-blocking (i.e., returns immediately).



.. code-block:: mysql


    mysql> RELOAD INDEXES;
    Query OK, 0 rows affected (0.01 sec)

