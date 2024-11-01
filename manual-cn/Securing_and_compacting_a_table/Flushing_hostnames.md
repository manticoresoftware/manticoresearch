# FLUSH HOSTNAMES

```sql
FLUSH HOSTNAMES
```

`FLUSH HOSTNAMES` 命令用于更新与代理主机名相关联的 IP 地址。如果您希望始终通过 DNS 查询获取主机名的 IP，可以使用 [hostname_lookup](../Server_settings/Searchd.md#hostname_lookup) 指令。

```sql
mysql> FLUSH HOSTNAMES;
Query OK, 5 rows affected (0.01 sec)
```

<!-- proofread -->