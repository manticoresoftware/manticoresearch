.. _show_character_set_syntax:

SHOW CHARACTER SET syntax
-------------------------

.. code-block:: mysql


    SHOW CHARACTER SET

This is currently a placeholder query that does nothing and reports that
a UTF-8 character set is available. It was added in order to keep
compatibility with frameworks and connectors that automatically execute
this statement.

.. code-block:: mysql


    mysql> SHOW CHARACTER SET;
    +---------+---------------+-------------------+--------+
    | Charset | Description   | Default collation | Maxlen |
    +---------+---------------+-------------------+--------+
    | utf8    | UTF-8 Unicode | utf8_general_ci   | 3      |
    +---------+---------------+-------------------+--------+
    1 row in set (0.00 sec)

