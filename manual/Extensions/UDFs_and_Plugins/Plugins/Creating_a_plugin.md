# System plugins
## CREATE PLUGIN

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

## CREATE BUDDY PLUGIN

<!-- example create_buddy_plugin -->
With Buddy plugins, you can enhance Manticore Search's capabilities and accommodate certain queries that Manticore doesn't support natively. We strongly recommend reading [this article](https://manticoresearch.com/blog/manticoresearch-buddy-pluggable-design/) to learn the basics of working with plugins.

```sql
CREATE PLUGIN <username/package name on https://packagist.org/> TYPE 'buddy' VERSION <package version>
```

There is also an alias that is created specifically for use with Buddy and is easier to remember.

```sql
CREATE BUDDY PLUGIN <username/package name on https://packagist.org/> VERSION <package version>
```

This will install the `show-hostname` plugin to [plugin_dir](../../Server_settings/Common.md#plugin_dir) and will enable it without the need to restart the server.

<!-- intro -->
### Examples

<!-- request Example -->

```sql
CREATE PLUGIN manticoresoftware/buddy-plugin-show-hostname TYPE 'buddy' VERSION 'dev-main';

CREATE BUDDY PLUGIN manticoresoftware/buddy-plugin-show-hostname VERSION 'dev-main';
```

<!-- end -->
