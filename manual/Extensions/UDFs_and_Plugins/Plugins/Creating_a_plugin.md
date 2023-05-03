# System plugins
## CREATE PLUGIN

```sql
CREATE PLUGIN plugin_name TYPE 'plugin_type' SONAME 'plugin_library'
```

Loads the given library (if it is not already loaded) and loads the specified plugin from it. The available plugin types include:

- `ranker`
- `index_token_filter`
- `query_token_filter`

For more information on writing plugins, please refer to the [plugins](../../../Extensions/UDFs_and_Plugins/UDFs_and_Plugins.md#Plugins) documentation.

```sql
mysql> CREATE PLUGIN myranker TYPE 'ranker' SONAME 'myplugins.so';
Query OK, 0 rows affected (0.00 sec)
```

## CREATE BUDDY PLUGIN

<!-- example create_buddy_plugin -->
Buddy plugins can extend Manticore Search's functionality and enable certain queries that are not natively supported. To learn more about creating Buddy plugins, we recommend reading [this article](https://manticoresearch.com/blog/manticoresearch-buddy-pluggable-design/).

To create a Buddy plugin, run the following SQL command:

```sql
CREATE PLUGIN <username/package name on https://packagist.org/> TYPE 'buddy' VERSION <package version>
```

You can also use an alias command specifically created for Buddy plugins, which is easier to remember:

```sql
CREATE BUDDY PLUGIN <username/package name on https://packagist.org/> VERSION <package version>
```

This command will install the `show-hostname` plugin to the [plugin_dir](../../Server_settings/Common.md#plugin_dir) and enable it without the need to restart the server.

<!-- intro -->
### Examples

<!-- request Example -->

```sql
CREATE PLUGIN manticoresoftware/buddy-plugin-show-hostname TYPE 'buddy' VERSION 'dev-main';

CREATE BUDDY PLUGIN manticoresoftware/buddy-plugin-show-hostname VERSION 'dev-main';
```

<!-- end -->
<!-- proofread -->