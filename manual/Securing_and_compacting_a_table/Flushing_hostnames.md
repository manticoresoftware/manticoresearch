# FLUSH HOSTNAMES

```sql
FLUSH HOSTNAMES
```

Renews IPs associates to agent host names. To always query the DNS for getting the host name IP, see [hostname_lookup](../Server_settings/Searchd.md#hostname_lookup) directive.

```sql
mysql> FLUSH HOSTNAMES;
Query OK, 5 rows affected (0.01 sec)
```