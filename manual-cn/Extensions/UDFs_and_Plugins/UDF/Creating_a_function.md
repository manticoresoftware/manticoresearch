# CREATE FUNCTION

```sql
CREATE FUNCTION udf_name
    RETURNS {INT | INTEGER | BIGINT | FLOAT | STRING}
    SONAME 'udf_lib_file'
```

`CREATE FUNCTION` 语句用于从指定的库文件中安装具有指定名称和类型的用户定义函数（UDF）。库文件必须位于受信任的 [plugin_dir](../../../Server_settings/Common.md#plugin_dir) 目录中。安装成功后，该函数可用于服务器接收到的所有后续查询中。示例：

```sql
mysql> CREATE FUNCTION avgmva RETURNS INTEGER SONAME 'udfexample.dll';
Query OK, 0 rows affected (0.03 sec)

mysql> SELECT *, AVGMVA(tag) AS q from test1;
+------+--------+---------+-----------+
| id   | weight | tag     | q         |
+------+--------+---------+-----------+
|    1 |      1 | 1,3,5,7 | 4.000000  |
|    2 |      1 | 2,4,6   | 4.000000  |
|    3 |      1 | 15      | 15.000000 |
|    4 |      1 | 7,40    | 23.500000 |
+------+--------+---------+-----------+
```
<!-- proofread -->