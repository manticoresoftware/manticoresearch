# DELETE PLUGIN

```sql
DROP PLUGIN plugin_name TYPE 'plugin_type'
```

Marks the designated plugin for unloading. The unloading process is **not** instantaneous, as concurrent queries may still be utilizing it. Nevertheless, following a `DROP`, new queries will no longer have access to the plugin. Subsequently, when all ongoing queries involving the plugin have finished, the plugin will be unloaded. If all plugins from the specified library are unloaded, the library will also be automatically unloaded.

```sql
mysql> DROP PLUGIN myranker TYPE 'ranker';
Query OK, 0 rows affected (0.00 sec)
```

## DELETE BUDDY PLUGIN

<!-- example delete_buddy_plugin -->

```sql
DELETE BUDDY PLUGIN <username/package name on https://packagist.org/>
```

> NOTE: `DELETE BUDDY PLUGIN` requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

This action instantly and permanently removes the installed plugin from the [plugin_dir](../../Server_settings/Common.md#plugin_dir). Once removed, the plugin's features will no longer be available.

<!-- intro -->
### Example

<!-- request Example -->
```sql
DELETE BUDDY PLUGIN manticoresoftware/buddy-plugin-show-hostname
```

<!-- end -->
<!-- proofread -->
