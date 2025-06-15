# 刷新主机名

```sql
FLUSH HOSTNAMES
```

FLUSH HOSTNAMES 命令用于更新与代理主机名相关联的IP地址。如果您希望始终查询DNS以获取主机名的IP，可以使用 [hostname_lookup](../Server_settings/Searchd.md#hostname_lookup) 指令。

```sql
mysql> FLUSH HOSTNAMES;
Query OK, 5 rows affected (0.01 sec)
```

<!-- proofread -->

