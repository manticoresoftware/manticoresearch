.. _show_variables_syntax:

SHOW VARIABLES syntax
---------------------

::


    SHOW [{GLOBAL | SESSION}] VARIABLES [WHERE variable_name='xxx']

**SHOW VARIABLES** statement was added to improve compatibility with
3rd party MySQL connectors and frameworks that automatically execute
this statement.

It returns the current values of a few server-wide variables. Also,
support for GLOBAL and SESSION clauses was added.

::


    mysql> SHOW GLOBAL VARIABLES;
    +----------------------+----------+
    | Variable_name        | Value    |
    +----------------------+----------+
    | autocommit           | 1        |
    | collation_connection | libc_ci  |
    | query_log_format     | sphinxql |
    | log_level            | info     |
    +----------------------+----------+
    4 rows in set (0.00 sec)

Support for WHERE variable_name clause was added, to help certain
connectors.
