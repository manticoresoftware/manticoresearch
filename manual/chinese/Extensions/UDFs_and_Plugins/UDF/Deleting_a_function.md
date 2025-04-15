# 删除函数


```sql

DROP FUNCTION udf_name

```


`DROP FUNCTION` 语句卸载具有指定名称的用户定义函数 [UDF](../../../Extensions/UDFs_and_Plugins/UDF.md)。成功移除后，函数将不再可用于后续查询。然而，正在进行的并发查询不会受到影响，如有必要，库的卸载将延迟到这些查询完成为止。示例：


