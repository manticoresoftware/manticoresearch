# FLUSH HOSTNAMES

```sql
FLUSH HOSTNAMES
```

Команда FLUSH HOSTNAMES используется для обновления IP-адресов, связанных с именами хостов агентов. Если вы хотите всегда запрашивать DNS для получения IP-адреса имени хоста, вы можете использовать директиву [hostname_lookup](../Server_settings/Searchd.md#hostname_lookup).

```sql
mysql> FLUSH HOSTNAMES;
Query OK, 5 rows affected (0.01 sec)
```

<!-- proofread -->

