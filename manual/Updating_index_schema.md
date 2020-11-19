# Updating index schema

## Updating index schema in RT mode

<!-- example ALTER -->

```sql
ALTER TABLE index {ADD|DROP} COLUMN column_name [{INTEGER|INT|BIGINT|FLOAT|BOOL|MULTI|MULTI64|JSON|STRING|TIMESTAMP}]
```

It supports adding one attribute at a time for both plain and RT indexes. The supported attribute types are:
* int
* bigint
* float
* bool
* multi-valued
* multi-valued 64bit
* json
* string 

#### Important notes:
* Querying an index is impossible (because of a write lock) while adding a column. 
* Newly created attribute's values are set to 0. 
* `ALTER` will not work for distributed indexes and indexes without any attributes. 
* `DROP COLUMN` will fail if an index has only one attribute.

<!-- request Example -->
```sql

mysql> desc rt;
+------------+-----------+
| Field      | Type      |
+------------+-----------+
| id         | bigint    |
| text       | field     |
| group_id   | uint      |
| date_added | timestamp |
+------------+-----------+
4 rows in set (0.01 sec)

mysql> alter table rt add column test integer;
Query OK, 0 rows affected (0.04 sec)

mysql> desc rt;
+------------+-----------+
| Field      | Type      |
+------------+-----------+
| id         | bigint    |
| text       | field     |
| group_id   | uint      |
| date_added | timestamp |
| test       | uint      |
+------------+-----------+
5 rows in set (0.00 sec)

mysql> alter table rt drop column group_id;
Query OK, 0 rows affected (0.01 sec)

mysql> desc rt;
+------------+-----------+
| Field      | Type      |
+------------+-----------+
| id         | bigint    |
| text       | field     |
| date_added | timestamp |
| test       | uint      |
+------------+-----------+
4 rows in set (0.00 sec)
```

<!-- end -->

## Updating index FT settings in plain mode

<!-- example ALTER RECONFIGURE -->
```sql
ALTER RTINDEX index RECONFIGURE
```

`ALTER` can also reconfigure an RT index in plain mode, so that new tokenization, morphology, and other text processing settings from the configuration file take effect on the newly INSERT-ed rows, while retaining the existing rows as they were. Internally, it forcibly saves the current RAM chunk as a new disk chunk, and adjusts the index header, so that the new rows are tokenized using the new rules. 

<!-- request Example -->
```sql
mysql> show index rt settings;
+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| settings      |       |
+---------------+-------+
1 row in set (0.00 sec)

mysql> alter rtindex rt reconfigure;
Query OK, 0 rows affected (0.00 sec)

mysql> show index rt settings;
+---------------+----------------------+
| Variable_name | Value                |
+---------------+----------------------+
| settings      | morphology = stem_en |
+---------------+----------------------+
1 row in set (0.00 sec)
```
<!-- end -->