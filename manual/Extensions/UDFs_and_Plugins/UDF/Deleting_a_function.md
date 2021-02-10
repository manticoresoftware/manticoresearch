# DROP FUNCTION

```sql
DROP FUNCTION udf_name
```

`DROP FUNCTION` statement deinstalls a `user-defined function [UDF](../../../Extensions/UDFs_and_Plugins/UDF.md) with the given name. On success, the function is no longer available for use in subsequent queries. Pending concurrent queries will not be affected and the library unload, if necessary, will be postponed until those queries complete. Example:

```sql
mysql> DROP FUNCTION avgmva;
Query OK, 0 rows affected (0.00 sec)
```
