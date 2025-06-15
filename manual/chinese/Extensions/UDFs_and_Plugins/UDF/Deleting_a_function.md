# DROP FUNCTION

```sql
DROP FUNCTION udf_name
```

`DROP FUNCTION` 语句用于卸载具有指定名称的用户自定义函数 [UDF](../../../Extensions/UDFs_and_Plugins/UDF.md)。成功移除后，该函数将不再可用于后续查询。然而，正在进行的并发查询不会受到影响，并且如果有必要，库的卸载将被延迟，直到这些查询完成。示例：

```sql
mysql> DROP FUNCTION avgmva;
Query OK, 0 rows affected (0.00 sec)
```
<!-- proofread -->

