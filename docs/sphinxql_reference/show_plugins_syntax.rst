.. _show_plugins_syntax:

SHOW PLUGINS syntax
-------------------

::


    SHOW PLUGINS

Displays all the loaded plugins and UDFs. “Type” column should be one of
the udf, ranker, index_token_filter, or query_token_filter. “Users”
column is the number of thread that are currently using that plugin in a
query. “Extra” column is intended for various additional plugin-type
specific information; currently, it shows the return type for the UDFs
and is empty for all the other plugin types.

::


    mysql> SHOW PLUGINS;
    +------+----------+----------------+-------+-------+
    | Type | Name     | Library        | Users | Extra |
    +------+----------+----------------+-------+-------+
    | udf  | sequence | udfexample.dll | 0     | INT   |
    +------+----------+----------------+-------+-------+
    1 row in set (0.00 sec)

