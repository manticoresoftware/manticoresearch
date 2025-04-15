# 删除插件

```sql
DROP PLUGIN plugin_name TYPE 'plugin_type'
```

标记指定的插件以进行卸载。卸载过程是 **非** 即时的，因为并发查询仍可能在使用它。然而，在执行 `DROP` 之后，新查询将不再能够访问该插件。随后，当所有涉及该插件的正在进行的查询完成后，插件将被卸载。如果从指定库卸载所有插件，该库也将被自动卸载。

```sql
mysql> DROP PLUGIN myranker TYPE 'ranker';
Query OK, 0 rows affected (0.00 sec)
```

## 删除好友插件

<!-- example delete_buddy_plugin -->

```sql
DELETE BUDDY PLUGIN <username/package name on https://packagist.org/>
```

> 注意: `DELETE BUDDY PLUGIN` 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保 Buddy 已安装。

此操作会立即永久删除安装的插件，无法恢复。移除后，插件的功能将不再可用。

<!-- intro -->
### 示例

<!-- request Example -->
```sql
DELETE BUDDY PLUGIN manticoresoftware/buddy-plugin-show-hostname
```

<!-- end -->
<!-- proofread -->
