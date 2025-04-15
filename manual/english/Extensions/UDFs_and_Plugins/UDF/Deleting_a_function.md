# DROP FUNCTION

```sql
DROP FUNCTION udf_name
```

`DROP FUNCTION` statement uninstalls a user-defined function [UDF](../../../Extensions/UDFs_and_Plugins/UDF.md) with the specified name. Upon successful removal, the function will no longer be available for use in subsequent queries. However, ongoing concurrent queries will not be affected, and if necessary, the library unloading will be delayed until those queries are completed. Example:

```sql
mysql> DROP FUNCTION avgmva;
Query OK, 0 rows affected (0.00 sec)
```
<!-- proofread -->