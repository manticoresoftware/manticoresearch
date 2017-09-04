.. _begin,_commit,_and_rollback_syntax:

BEGIN, COMMIT, and ROLLBACK syntax
----------------------------------

.. code-block:: mysql


    START TRANSACTION | BEGIN
    COMMIT
    ROLLBACK
    SET AUTOCOMMIT = {0 | 1}

BEGIN statement (or its START TRANSACTION alias) forcibly commits
pending transaction, if any, and begins a new one. COMMIT statement
commits the current transaction, making all its changes permanent.
ROLLBACK statement rolls back the current transaction, canceling all its
changes. SET AUTOCOMMIT controls the autocommit mode in the active
session.

AUTOCOMMIT is set to 1 by default, meaning that every statement that
performs any changes on any index is implicitly wrapped in BEGIN and
COMMIT.

Transactions are limited to a single RT index, and also limited in size.
They are atomic, consistent, overly isolated, and durable. Overly
isolated means that the changes are not only invisible to the concurrent
transactions but even to the current session itself.
