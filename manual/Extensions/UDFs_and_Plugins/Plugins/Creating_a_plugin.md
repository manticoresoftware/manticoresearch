# CREATE PLUGIN

```sql
CREATE PLUGIN plugin_name TYPE 'plugin_type' SONAME 'plugin_library'
```

Loads the given library (if it is not loaded yet) and loads the specified plugin from it. The known plugin types are:

* `ranker`
* `index_token_filter`
* `query_token_filter`

Refer to [plugins](../../../Extensions/UDFs_and_Plugins/UDFs_and_Plugins.md#Plugins) for more information regarding writing the plugins.

```sql
mysql> CREATE PLUGIN myranker TYPE 'ranker' SONAME 'myplugins.so';
Query OK, 0 rows affected (0.00 sec)
```
