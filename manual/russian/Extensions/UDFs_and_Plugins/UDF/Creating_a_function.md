# CREATE FUNCTION

```sql
CREATE FUNCTION udf_name
    RETURNS {INT | INTEGER | BIGINT | FLOAT | STRING}
    SONAME 'udf_lib_file'
```

Оператор `CREATE FUNCTION` устанавливает пользовательскую функцию [UDF](../../../Extensions/UDFs_and_Plugins/UDF.md) с указанным именем и типом из предоставленного файла библиотеки. Файл библиотеки должен находиться в доверенной директории [plugin_dir](../../../Server_settings/Common.md#plugin_dir). После успешной установки функция становится доступной для использования во всех последующих запросах, поступающих на сервер. Пример:

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

