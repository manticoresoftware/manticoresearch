.. _create_function_syntax:

CREATE FUNCTION syntax
----------------------

.. code-block:: none


    CREATE FUNCTION udf_name
        RETURNS {INT | INTEGER | BIGINT | FLOAT | STRING}
        SONAME 'udf_lib_file'

CREATE FUNCTION statement installs a :ref:`user-defined function
(UDF) <udfs_user_defined_functions>` with the given name
and type from the given library file. The library file must reside in a
trusted
:ref:`plugin_dir <plugin_dir>`
directory. On success, the function is available for use in all
subsequent queries that the server receives. Example:

.. code-block:: mysql


    mysql> CREATE FUNCTION avgmva RETURNS INTEGER SONAME 'udfexample.dll';
    Query OK, 0 rows affected (0.03 sec)

    mysql> SELECT *, AVGMVA(tag) AS q from test1;
    +------+--------+---------+-----------+
    | id   | weight | tag     | q         |
    +------+--------+---------+-----------+
    |    1 |      1 | 1,3,5,7 | 4.000000  |
    |    2 |      1 | 2,4,6   | 4.000000  |
    |    3 |      1 | 15      | 15.000000 |
    |    4 |      1 | 7,40    | 23.500000 |
    +------+--------+---------+-----------+

