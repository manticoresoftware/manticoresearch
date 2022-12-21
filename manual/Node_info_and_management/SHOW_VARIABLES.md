# SHOW VARIABLES

```sql
SHOW [{GLOBAL | SESSION}] VARIABLES LIKE 'pattern'
```

It returns the current values of a few server-wide variables. Also, support for `GLOBAL` and `SESSION` clauses was added.

```sql
mysql> SHOW GLOBAL VARIABLES;
+--------------------------+-----------+
| Variable_name            | Value     |
+--------------------------+-----------+
| autocommit               | 1         |
| collation_connection     | libc_ci   |
| query_log_format         | sphinxql  |
| log_level                | info      |
| max_allowed_packet       | 134217728 |
| character_set_client     | utf8      |
| character_set_connection | utf8      |
| grouping_in_utc          | 0         |
| last_insert_id           | 123, 200  |
+--------------------------+-----------+
9 rows in set (0.00 sec)
```

```sql
mysql> show variables like '%log%';
+------------------+----------+
| Variable_name    | Value    |
+------------------+----------+
| query_log_format | sphinxql |
| log_level        | info     |
+------------------+----------+
2 rows in set (0.00 sec)
```

```sql
mysql> show session variables like 'autocommit';
+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| autocommit    | 0     |
+---------------+-------+
1 row in set (0.00 sec)
```
