# CREATE FUNCTION

```sql
CREATE FUNCTION udf_name
    RETURNS {INT | INTEGER | BIGINT | FLOAT | STRING}
    SONAME 'udf_lib_file'
```

`CREATE FUNCTION` statement installs a user-defined function [UDF](../../../Extensions/UDFs_and_Plugins/UDF.md) with the specified name and type from the provided library file. The library file must be located in a trusted [plugin_dir](../../../Server_settings/Common.md#plugin_dir) directory. Upon successful installation, the function becomes available for use in all subsequent queries received by the server. Example:

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