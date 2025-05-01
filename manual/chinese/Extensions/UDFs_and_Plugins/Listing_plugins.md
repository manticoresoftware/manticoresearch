# 列出插件

## 显示插件
<!-- example Example -->

```sql
SHOW PLUGINS
```

显示所有已加载的插件（除了 Buddy 插件，见下文）和 UDF。类型列应为 `udf`、`ranker`、`index_token_filter` 或 `query_token_filter` 中的一个。“用户”列是当前在查询中使用该插件的线程数。“额外”列用于各种附加的插件类型特定信息；目前，它显示 UDF 的返回类型，对于其他所有插件类型则为空。


<!-- intro -->
##### 示例：

<!-- request Example -->

```sql
SHOW PLUGINS;
```

<!-- response -->

```sql
+------+----------+----------------+-------+-------+
| 类型 | 名称     | 库            | 用户  | 额外  |
+------+----------+----------------+-------+-------+
| udf  | sequence | udfexample.dll | 0     | INT   |
+------+----------+----------------+-------+-------+
1 行在结果中 (0.00 sec)
```

<!-- end -->

## 显示 Buddy 插件

<!-- example Example_buddy -->

```sql
SHOW BUDDY PLUGINS
```

> 注意: `SHOW BUDDY PLUGINS` 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保 Buddy 已安装。

这将显示所有可用插件，包括核心和本地插件。
要删除插件，请确保使用“包”列中列出的名称。

<!-- request Example -->

```sql
SHOW BUDDY PLUGINS;
```

<!-- response -->

```
+------------------------------------------------+-----------------+---------+------+----------------------------------------------------------------------------------------------------------------------------------------------------------+
| 包                                            | 插件            | 版本   | 类型 | 信息                                                                                                                                                     |
+------------------------------------------------+-----------------+---------+------+----------------------------------------------------------------------------------------------------------------------------------------------------------+
| manticoresoftware/buddy-plugin-empty-string    | empty-string    | 2.1.5   | core | 处理空查询，这种情况可能在修剪注释或处理不支持的 SQL 协议指令的注释时发生                                                                     |
| manticoresoftware/buddy-plugin-backup          | backup          | 2.1.5   | core | BACKUP SQL 语句                                                                                                                                         |
| manticoresoftware/buddy-plugin-emulate-elastic | emulate-elastic | 2.1.5   | core | 模拟一些 Elastic 查询并生成响应，就好像它们是由 ES 发出的                                                                                         |
| manticoresoftware/buddy-plugin-insert          | insert          | 2.1.5   | core | 自动模式支持。当执行插入操作且表不存在时，它会自动检测数据类型创建该表                                                                      |
| manticoresoftware/buddy-plugin-alias           | alias           | 2.1.5   | core |                                                                                                                                                          |
| manticoresoftware/buddy-plugin-select          | select          | 2.1.5   | core | 针对 mysqldump 和其他软件支持需要的各种 SELECT 处理程序，主要目标是类似于 MySQL 的工作                                                       |
| manticoresoftware/buddy-plugin-show            | show            | 2.1.5   | core | 各种“显示”查询处理程序，例如，`show queries`、`show fields`、`show full tables` 等                                                          |
| manticoresoftware/buddy-plugin-cli-table       | cli-table       | 2.1.5   | core | 基于 /cli_json 的 /cli 端点 - 以表格形式输出查询结果                                                                                                 |
| manticoresoftware/buddy-plugin-plugin          | plugin          | 2.1.5   | core | 支持插件和助手的核心逻辑。同时处理`create buddy plugin`、`delete buddy plugin` 和 `show buddy plugins`  |
| manticoresoftware/buddy-plugin-test            | test            | 2.1.5   | core | 测试插件，仅用于测试                                                                                                                                  |
| manticoresoftware/buddy-plugin-insert-mva      | insert-mva      | 2.1.5   | core | 管理 MVA 字段的恢复与 mysqldump                                                                                                                      |
| manticoresoftware/buddy-plugin-modify-table    | modify-table    | 2.1.5   | core | 帮助标准化创建和修改表语句中的选项，以显示 integer 的 option=1。还管理创建分片表的逻辑。                                                    |
| manticoresoftware/buddy-plugin-knn             | knn             | 2.1.5   | core | 通过文档 ID 启用 KNN                                                                                                                                    |
| manticoresoftware/buddy-plugin-replace         | replace         | 2.1.5   | core | 启用部分替换                                                                                                                                           |
+------------------------------------------------+-----------------+---------+------+----------------------------------------------------------------------------------------------------------------------------------------------------------+-----+
```

<!-- end -->
<!-- proofread -->
