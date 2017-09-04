.. _create_plugin_syntax:

CREATE PLUGIN syntax
--------------------

.. code-block:: mysql


    CREATE PLUGIN plugin_name TYPE 'plugin_type' SONAME 'plugin_library'

Loads the given library (if it is not loaded yet) and loads the
specified plugin from it. The known plugin types are:

-  ranker

-  index_token_filter

-  query_token_filter

Refer to :ref:`plugins`
for more information regarding writing the plugins.

.. code-block:: mysql


    mysql> CREATE PLUGIN myranker TYPE 'ranker' SONAME 'myplugins.so';
    Query OK, 0 rows affected (0.00 sec)

