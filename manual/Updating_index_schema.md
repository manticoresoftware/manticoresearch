# Updating index schema

## Updating index schema in RT mode

<!-- example ALTER -->

```sql
ALTER TABLE index ADD COLUMN column_name [{INTEGER|INT|BIGINT|FLOAT|BOOL|MULTI|MULTI64|JSON|STRING|TIMESTAMP|TEXT [INDEXED [ATTRIBUTE]]}] [engine='columnar']

ALTER TABLE index DROP COLUMN column_name
```

It supports adding one field at a time for RT indexes. Supported data types are:
* `int` - integer attribute
* `timestamp` - timestamp attribute
* `bigint` - big integer attribute
* `float` - float attribute
* `bool` - boolean attribute
* `multi` - multi-valued integer attribute
* `multi64` - multi-valued bigint attribute
* `json` - json attribute
* `string` / `text attribute` / `string attribute` - string attribute
* `text` / `text indexed stored` / `string indexed stored` - full-text indexed field with original value stored in docstore
* `text indexed` / `string indexed` - full-text indexed field, indexed only (the original value is not stored in docstore)
* `text indexed attribute` / `string indexed attribute` - full text indexed field + string attribute (not storing the original value in docstore)
* `text stored` / `string stored` - the value will be only stored in docstore, not full-text indexed, not a string attribute
* adding `engine='columnar'` to any attribute (except for json) will make it stored in the [columnar storage](Creating_an_index/Data_types.md#Row-wise-and-columnar-attribute-storages)

#### Important notes:
* Querying an index is impossible while a column is being added.
* Newly created attribute's values are set to 0.
* `ALTER` will not work for distributed indexes and indexes without any attributes.
* `DROP COLUMN` will fail if an index has only one field.
* When dropping a field which is both a full-text field and a string attribute the first `ALTER DROP` drops the attribute, the second one drops the full-text field.

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

`ALTER` can also reconfigure an RT index in [plain mode](Creating_an_index/Local_indexes.md#Defining-index-schema-in-config-%28Plain-mode%29), so that new tokenization, morphology and other text processing settings from the configuration file take effect on the newly INSERT-ed rows, while retaining the existing rows as they were. Internally, it forcibly saves the current RAM chunk as a new disk chunk and adjusts the index header, so that the new rows are tokenized using the new rules.

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
