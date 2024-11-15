# DELETE PLUGIN

```sql
DROP PLUGIN plugin_name TYPE 'plugin_type'
```

标记指定的插件进行卸载。卸载过程并**非**即时完成，因为并发查询可能仍在使用该插件。不过，执行 `DROP` 后，新查询将不再能够访问该插件。随后，当所有涉及该插件的查询完成后，该插件将被卸载。如果从指定库中卸载了所有插件，该库也将自动卸载。

```sql
mysql> DROP PLUGIN myranker TYPE 'ranker';
Query OK, 0 rows affected (0.00 sec)
```

## DELETE BUDDY PLUGIN

<!-- example delete_buddy_plugin -->

```sql
DELETE BUDDY PLUGIN <username/package name on https://packagist.org/>
```

> 注意: `DELETE BUDDY PLUGIN` 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果命令无法运行，请确认 Buddy 已安装。

该操作将立即且永久地从 [plugin_dir](../../../Server_settings/Common.md#plugin_dir) 中移除已安装的插件。插件移除后，其功能将不再可用。

<!-- intro -->
### 示例

<!-- request Example -->
```sql
DELETE BUDDY PLUGIN manticoresoftware/buddy-plugin-show-hostname
```

<!-- end -->
<!-- proofread -->
