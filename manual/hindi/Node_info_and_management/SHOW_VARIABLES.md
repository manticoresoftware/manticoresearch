# SHOW VARIABLES

```sql
SHOW [{GLOBAL | SESSION}] VARIABLES LIKE 'pattern'
```

यह कुछ सर्वर-व्यापी चर के वर्तमान मानों को वापस करता है। इसके अलावा, `GLOBAL` और `SESSION` खंडों के लिए समर्थन जोड़ा गया था।

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
<!-- proofread -->
