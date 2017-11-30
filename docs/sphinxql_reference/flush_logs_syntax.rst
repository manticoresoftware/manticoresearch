.. _flush_logs_syntax:

FLUSH LOGS syntax
-----------------

.. code-block:: mysql


    FLUSH LOGS

Works same as system USR1 signal.
Initiate reopen of searchd log and query log files, letting you implement log file rotation.
Command is non-blocking (i.e., returns immediately).

.. code-block:: mysql


    mysql> FLUSH LOGS;
    Query OK, 0 rows affected (0.01 sec)

