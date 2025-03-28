# DROP FUNCTION

```sql
DROP FUNCTION udf_name
```

`DROP FUNCTION` 语句用于卸载具有指定名称的用户定义函数（UDF）。成功移除后，该函数将无法在后续查询中使用。然而，正在进行的并发查询不会受到影响，如果有必要，库文件的卸载将会延迟到这些查询完成为止。示例：

```sql
mysql> DROP FUNCTION avgmva;
Query OK, 0 rows affected (0.00 sec)
```
<!-- proofread -->