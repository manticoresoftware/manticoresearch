# DELETE PLUGIN

```sql
DROP PLUGIN plugin_name TYPE 'plugin_type'
```

标记指定的插件以便卸载。卸载过程**不是**即时的，因为并发查询可能仍在使用它。然而，在执行 `DROP` 后，新查询将不再能够访问该插件。随后，当所有正在进行的涉及该插件的查询完成后，插件将被卸载。如果指定库中的所有插件都被卸载，该库也将被自动卸载。

```sql
mysql> DROP PLUGIN myranker TYPE 'ranker';
Query OK, 0 rows affected (0.00 sec)
```

## DELETE BUDDY PLUGIN

<!-- example delete_buddy_plugin -->

```sql
DELETE BUDDY PLUGIN <username/package name on https://packagist.org/>
```

> 注意：`DELETE BUDDY PLUGIN` 需要 [Manticore Buddy](../../../Installation/Manticore_Buddy.md)。如果无法使用，请确保已安装 Buddy。

此操作会立即且永久地从 [plugin_dir](../../../Server_settings/Common.md#plugin_dir) 中移除已安装的插件。移除后，插件的功能将不再可用。

<!-- intro -->
### 示例

<!-- request Example -->
```sql
DELETE BUDDY PLUGIN manticoresoftware/buddy-plugin-show-hostname
```

<!-- end -->
<!-- proofread -->

