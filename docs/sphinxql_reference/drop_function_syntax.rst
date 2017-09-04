.. _drop_function_syntax:

DROP FUNCTION syntax
--------------------

.. code-block:: mysql


    DROP FUNCTION udf_name

DROP FUNCTION statement deinstalls a :ref:`user-defined function
(UDF) <udfs_user_defined_functions>` with the given name.
On success, the function is no longer available for use in subsequent
queries. Pending concurrent queries will not be affected and the library
unload, if necessary, will be postponed until those queries complete.
Example:

.. code-block:: mysql


    mysql> DROP FUNCTION avgmva;
    Query OK, 0 rows affected (0.00 sec)

