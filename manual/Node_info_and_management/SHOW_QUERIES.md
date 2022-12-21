# SHOW QUERIES

<!-- example SHOW QUERIES -->
```sql
SHOW QUERIES
```

`SHOW QUERIES` returns information about all the queries running now. It outputs a table with the following structure:

- `id`: query id which can be used in [KILL](../Node_info_and_management/KILL.md) to terminate the query
- `query`: query statement or a part of it
- `protocol`: [connection protocol](../Server_settings/Searchd.md#listen), the possible values are `sphinx` , `mysql` , `http` , `ssl` , `compressed` and `replication` or combination (e.g. `http,ssl` or `compressed,mysql` )
- `host`: `ip:port` of the client

<!-- request SQL -->
```sql
mysql> SHOW QUERIES;
```

<!-- response SQL -->
```
+------+--------------+----------+-----------------+
| id   | query        | protocol | host            |
+--------------------------+-----------------------+
|    6 | select       | http     | 127.0.0.1:41128 |
|    4 | show queries | mysql    | 127.0.0.1:56672 |
+------+--------------+----------+-----------------+
2 rows in set (0.61 sec)
```

<!-- end -->

See also [SHOW THREADS](../Node_info_and_management/SHOW_THREADS.md) if you want to know what's going on from the threads angle of view.
