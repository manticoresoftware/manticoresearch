.. _drop_plugin_syntax:

DROP PLUGIN syntax
------------------

::


    DROP PLUGIN plugin_name TYPE 'plugin_type'

Markes the specified plugin for unloading. The unloading is **not**
immediate, because the concurrent queries might be using it. However,
after a DROP new queries will not be able to use it. Then, once all the
currently executing queries using it are completed, the plugin will be
unloaded. Once all the plugins from the given library are unloaded, the
library is also automatically unloaded.

::


    mysql> DROP PLUGIN myranker TYPE 'ranker';
    Query OK, 0 rows affected (0.00 sec)

