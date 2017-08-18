.. _set_transaction_syntax:

SET TRANSACTION syntax
----------------------

::


    SET TRANSACTION ISOLATION LEVEL { READ UNCOMMITTED
        | READ COMMITTED
        | REPEATABLE READ
        | SERIALIZABLE }

SET TRANSACTION statement does nothing. It was implemented to maintain
compatibility with 3rd party MySQL client libraries, connectors, and
frameworks that may need to run this statement when connecting.

Example:

::


    mysql> SET TRANSACTION ISOLATION LEVEL READ UNCOMMITTED;
    Query OK, 0 rows affected (0.00 sec)

