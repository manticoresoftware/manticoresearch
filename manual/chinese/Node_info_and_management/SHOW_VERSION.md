# SHOW VERSION

<!-- example SHOW VERSION -->
```sql
SHOW VERSION
```

> 注意：`SHOW VERSION` 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保已安装 Buddy。

`SHOW VERSION` 提供 Manticore Search 实例中各个组件的详细版本信息。该命令对于需要验证所运行的 Manticore Search 及其相关组件版本的管理员和开发人员特别有用。

输出表格包括两列：
- `Component`：此列命名 Manticore Search 的具体组件。
- `Version`：此列显示相应组件的版本信息。

<!-- request SQL -->
```sql
mysql> SHOW VERSION;
```

<!-- response SQL -->
```
+------------+--------------------------------+
| Component  | Version                        |
+------------+--------------------------------+
| Daemon     | 6.2.13 61cfe38d2@24011520 dev  |
| Columnar   | columnar 2.2.5 214ce90@240115  |
| Secondary  | secondary 2.2.5 214ce90@240115 |
| Knn        | knn 2.2.5 214ce90@240115       |
| Embeddings | embeddings 1.0.0               |
| Buddy      | buddy v2.0.11                  |
+------------+--------------------------------+
```

<!-- end -->

<!-- proofread -->

