# SHOW VERSION

<!-- example SHOW VERSION -->
```sql
SHOW VERSION
```

`SHOW VERSION` returns information about all currently running queries. The output is a table with the following structure:

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
