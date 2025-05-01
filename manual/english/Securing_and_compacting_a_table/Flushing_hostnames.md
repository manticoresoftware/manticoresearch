# FLUSH HOSTNAMES

```sql
FLUSH HOSTNAMES
```

The FLUSH HOSTNAMES command is used to renew IP addresses associated with agent host names. If you want to always query the DNS for getting the host name IP, you can use the [hostname_lookup](../Server_settings/Searchd.md#hostname_lookup) directive.

```sql
mysql> FLUSH HOSTNAMES;
Query OK, 5 rows affected (0.01 sec)
```

<!-- proofread -->