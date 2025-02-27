# SHOW VERSION

<!-- example SHOW VERSION -->
```sql
SHOW VERSION
```

> NOTE: `SHOW VERSION` requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

`SHOW VERSION` provides detailed version information of various components of the Manticore Search instance. This command is particularly useful for administrators and developers who need to verify the version of Manticore Search they are running, along with the versions of its associated components.

The output table includes two columns:
- `Component`: This column names the specific component of Manticore Search.
- `Version`: This column displays the version information for the respective component.

<!-- request SQL -->
```sql
mysql> SHOW VERSION;
```

<!-- response SQL -->
```
+-----------+--------------------------------+
| Component | Version                        |
+-----------+--------------------------------+
| Daemon    | 6.2.13 61cfe38d2@24011520 dev  |
| Columnar  | columnar 2.2.5 214ce90@240115  |
| Secondary | secondary 2.2.5 214ce90@240115 |
| KNN       | knn 2.2.5 214ce90@240115       |
| Buddy     | buddy v2.0.11                  |
+-----------+--------------------------------+
```

<!-- end -->

<!-- proofread -->
