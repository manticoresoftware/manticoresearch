# DELETE PLUGIN

```sql
DROP PLUGIN plugin_name TYPE 'plugin_type'
```

Marks the specified plugin for unloading. The unloading is **not**  immediate, because the concurrent queries might be using it. However, after a `DROP` new queries will not be able to use it. Then, once all the currently executing queries using it are completed, the plugin will be unloaded. Once all the plugins from the given library are unloaded, the library is also automatically unloaded.

```sql
mysql> DROP PLUGIN myranker TYPE 'ranker';
Query OK, 0 rows affected (0.00 sec)
```

## DELETE BUDDY PLUGIN

<!-- example delete_buddy_plugin -->

```sql
DELETE BUDDY PLUGIN <username/package name on https://packagist.org/>
```

This action will immediately and irreversibly remove the installed plugin from the [plugin_dir](../../Server_settings/Common.md#plugin_dir). Once deleted, the plugin's functionality will no longer be accessible.

<!-- intro -->
### Example

<!-- request Example -->
```sql
DELETE BUDDY PLUGIN manticoresoftware/buddy-plugin-show-hostname
```

<!-- end -->
