# SHOW QUERIES

```sql
SHOW QUERIES
```

`SHOW QUERIES` is a statement that returns information about all the current queries run by Manticore.
It outputs a table with the following structure:

- `id`: connection id of the query
- `query`: query statement
- `protocol`: connection protocol, possible values are `sphinx` , `mysql` , `http` , `ssl` , `compressed` and `replication` or combination (e.g. `http,ssl` or `compressed,mysql` )
- `host`: `ip:port` of the client

```sql
mysql> SHOW QUERIES;
+------+--------------+----------+-----------------+
| id   | query        | protocol | host            |
+--------------------------+-----------------------+
|    6 | select       | http     | 127.0.0.1:41128 |
|    4 | show queries | mysql    | 127.0.0.1:56672 |
+------+--------------+----------+-----------------+
2 rows in set (0.61 sec)
```
