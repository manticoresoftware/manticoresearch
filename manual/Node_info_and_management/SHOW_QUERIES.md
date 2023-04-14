# SHOW QUERIES

<!-- example SHOW QUERIES -->
```sql
SHOW QUERIES
```

`SHOW QUERIES` returns information about all currently running queries. The output is a table with the following structure:

- `id`: Query ID that can be used in [KILL](../Node_info_and_management/KILL.md) to terminate the query
- `query`: Query statement or a portion of it
- `protocol`: [Connection protocol](../Server_settings/Searchd.md#listen), with possible values being `sphinx`, `mysql`, `http`, `ssl`, `compressed`, `replication`, or a combination (e.g., `http,ssl` or `compressed,mysql`)
- `host`: Client's `ip:port`


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

Refer to [SHOW THREADS](../Node_info_and_management/SHOW_THREADS.md) if you'd like to gain insight from the perspective of the threads themselves.

<!-- proofread -->