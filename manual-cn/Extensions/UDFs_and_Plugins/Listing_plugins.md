# 列出插件

## SHOW PLUGINS
<!-- example Example -->

```sql
SHOW PLUGINS
```

显示所有已加载的插件（Buddy 插件除外，见下文）和用户定义函数（UDFs）。“Type”列应为以下之一：`udf`、`ranker`、`index_token_filter` 或 `query_token_filter`。“Users”列表示当前在查询中使用该插件的线程数量。“Extra”列用于显示各种插件类型的额外信息；当前，它显示 UDFs 的返回类型，对于其他插件类型则为空。


<!-- intro -->
##### 示例：

<!-- request Example -->

```sql
SHOW PLUGINS;
```

<!-- response -->

```sql
+------+----------+----------------+-------+-------+
| Type | Name     | Library        | Users | Extra |
+------+----------+----------------+-------+-------+
| udf  | sequence | udfexample.dll | 0     | INT   |
+------+----------+----------------+-------+-------+
1 row in set (0.00 sec)
```

<!-- end -->

## SHOW BUDDY PLUGINS

<!-- example Example_buddy -->

```sql
SHOW BUDDY PLUGINS
```

> 注意：`SHOW BUDDY PLUGINS` 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果无法使用该命令，请确保已经安装了 Buddy。

该命令将显示所有可用的插件，包括核心插件和本地插件。 要移除插件，请确保使用显示在“Package”列中的名称。

<!-- request Example -->

```sql
SHOW BUDDY PLUGINS;
```

<!-- response -->

```
+------------------------------------------------+-----------------+---------+------+----------------------------------------------------------------------------------------------------------------------------------------------------------+
| Package                                        | Plugin          | Version | Type | Info                                                                                                                                                     |
+------------------------------------------------+-----------------+---------+------+----------------------------------------------------------------------------------------------------------------------------------------------------------+
| manticoresoftware/buddy-plugin-empty-string    | empty-string    | 2.1.5   | core | Handles empty queries, which can occur when trimming comments or dealing with specific SQL protocol instructions in comments that are not supported      |
| manticoresoftware/buddy-plugin-backup          | backup          | 2.1.5   | core | BACKUP sql statement                                                                                                                                     |
| manticoresoftware/buddy-plugin-emulate-elastic | emulate-elastic | 2.1.5   | core | Emulates some Elastic queries and generates responses as if they were made by ES                                                                         |
| manticoresoftware/buddy-plugin-insert          | insert          | 2.1.5   | core | Auto schema support. When an insert operation is performed and the table does not exist, it creates it with data types auto-detection                    |
| manticoresoftware/buddy-plugin-alias           | alias           | 2.1.5   | core |                                                                                                                                                          |
| manticoresoftware/buddy-plugin-select          | select          | 2.1.5   | core | Various SELECTs handlers needed for mysqldump and other software support, mostly aiming to work similarly to MySQL                                       |
| manticoresoftware/buddy-plugin-show            | show            | 2.1.5   | core | Various "show" queries handlers, for example, `show queries`, `show fields`, `show full tables`, etc                                                     |
| manticoresoftware/buddy-plugin-cli-table       | cli-table       | 2.1.5   | core | /cli endpoint based on /cli_json - outputs query result as a table                                                                                       |
| manticoresoftware/buddy-plugin-plugin          | plugin          | 2.1.5   | core | Core logic for plugin support and helpers. Also handles `create buddy plugin`, `delete buddy plugin`, and `show buddy plugins`                           |
| manticoresoftware/buddy-plugin-test            | test            | 2.1.5   | core | Test plugin, used exclusively for tests                                                                                                                  |
| manticoresoftware/buddy-plugin-insert-mva      | insert-mva      | 2.1.5   | core | Manages the restoration of MVA fields with mysqldump                                                                                                     |
| manticoresoftware/buddy-plugin-modify-table    | modify-table    | 2.1.5   | core | Assists in standardizing options in create and alter table statements to show option=1 for integers. Also manages the logic for creating sharded tables. |
| manticoresoftware/buddy-plugin-knn             | knn             | 2.1.5   | core | Enables KNN by document id                                                                                                                               |
| manticoresoftware/buddy-plugin-replace         | replace         | 2.1.5   | core | Enables partial replaces                                                                                                                                 |
+------------------------------------------------+-----------------+---------+------+----------------------------------------------------------------------------------------------------------------------------------------------------------+-----+
```

<!-- end -->
<!-- proofread -->
