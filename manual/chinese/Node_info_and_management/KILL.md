# KILL

<!-- example KILL -->
```sql
KILL <query id>
```

`KILL` 通过查询的 ID 终止其执行，您可以在 [SHOW QUERIES](../Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) 中找到该 ID。

<!-- request SQL -->
```sql
mysql> KILL 4;
Query OK, 1 row affected (0.00 sec)
```

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "KILL 4"
[
  {
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->
<!-- proofread -->

