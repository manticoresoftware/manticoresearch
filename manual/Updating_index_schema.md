# Updating index schema

## Updating index schema in RT mode

<!-- example ALTER -->

```sql
ALTER TABLE index {ADD|DROP} COLUMN column_name [{INTEGER|INT|BIGINT|FLOAT|BOOL|MULTI|MULTI64|JSON|STRING|TIMESTAMP|TEXT [INDEXED [ATTRIBUTE]]}]
```

It supports adding one field at a time for RT indexes. Supported data types are:
* `int` - integer attribute
* `bigint` - big integer attribute
* `float` - float attribute
* `bool` - boolean attribute
* `multi` - multi-valued integer attribute
* `multi64` - multi-valued bigint attribute
* `json` - json attribute
* `string` / `text attribute` - string attribute
* `text` / `text indexed stored` / `string indexed stored` - full-text indexed field with original value stored in docstore
* `text indexed` / `string indexed` - full-text indexed field, indexed only (not storing the original value)
* `text indexed attribute` / `string indexed attribute` - fill text indexed field + string attribute (not storing the original value in docstore)
* `text indexed attribute stored` / `string indexed attribute stored` - fill text indexed field (stored in docstore) + string attribute

#### Important notes:
* Querying an index is impossible (because of a write lock) while adding a column. 
* Newly created attribute's values are set to 0. 
* `ALTER` will not work for distributed indexes and indexes without any attributes. 
* `DROP COLUMN` will fail if an index has only one field.
* When dropping a field which is both a full-text field and a string attribute the first `ALTER DROP` run drops the attribute, the second one drops the full-text field.

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

mysql> alter table rt add column test integer;

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

mysql> alter table rt drop column group_id;

mysql> desc rt;
+------------+-----------+
| Field      | Type      |
+------------+-----------+
| id         | bigint    |
| text       | field     |
| date_added | timestamp |
| test       | uint      |
+------------+-----------+

mysql> alter table rt add column title text indexed;

mysql> desc rt;
+------------+-----------+------------+
| Field      | Type      | Properties |
+------------+-----------+------------+
| id         | bigint    |            |
| text       | text      | indexed    |
| title      | text      | indexed    |
| date_added | timestamp |            |
| test       | uint      |            |
+------------+-----------+------------+

mysql> alter table rt add column title text attribute;

mysql> desc rt;
+------------+-----------+------------+
| Field      | Type      | Properties |
+------------+-----------+------------+
| id         | bigint    |            |
| text       | text      | indexed    |
| title      | text      | indexed    |
| date_added | timestamp |            |
| test       | uint      |            |
| title      | string    |            |
+------------+-----------+------------+

mysql> alter table rt drop column title;

mysql> desc rt;
+------------+-----------+------------+
| Field      | Type      | Properties |
+------------+-----------+------------+
| id         | bigint    |            |
| text       | text      | indexed    |
| title      | text      | indexed    |
| date_added | timestamp |            |
| test       | uint      |            |
+------------+-----------+------------+
mysql> alter table rt drop column title;

mysql> desc rt;
+------------+-----------+------------+
| Field      | Type      | Properties |
+------------+-----------+------------+
| id         | bigint    |            |
| text       | text      | indexed    |
| date_added | timestamp |            |
| test       | uint      |            |
+------------+-----------+------------+
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
