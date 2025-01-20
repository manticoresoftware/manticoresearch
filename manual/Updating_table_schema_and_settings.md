# Updating table schema

## Updating table schema in RT mode

<!-- example ALTER -->

```sql
ALTER TABLE table ADD COLUMN column_name [{INTEGER|INT|BIGINT|FLOAT|BOOL|MULTI|MULTI64|JSON|STRING|TIMESTAMP|TEXT [INDEXED [ATTRIBUTE]]}] [engine='columnar']

ALTER TABLE table DROP COLUMN column_name

ALTER TABLE table MODIFY COLUMN column_name bigint
```

This feature only supports adding one field at a time for RT tables or the expansion of an `int` column to `bigint`. The supported data types are:
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
* adding `engine='columnar'` to any attribute (except for json) will make it stored in the [columnar storage](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)

#### Important notes:
* ❗It's recommended to **backup table files** before `ALTER`ing it to avoid data corruption in case of a sudden power interruption or other similar issues.
* Querying a table is impossible while a column is being added.
* Newly created attribute's values are set to 0.
* `ALTER` will not work for distributed tables and tables without any attributes.
* You can't delete the `id` column.
* When dropping a field which is both a full-text field and a string attribute the first `ALTER DROP` drops the attribute, the second one drops the full-text field.
* Adding/dropping full-text field is only supported in the [RT mode](Read_this_first.md#Real-time-mode-vs-plain-mode).

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

## Updating table FT settings in RT mode

<!-- example ALTER FT -->

```sql
ALTER TABLE table ft_setting='value'[, ft_setting2='value']
```

You can use `ALTER` to modify the full-text settings of your table in  [RT mode](Read_this_first.md#Real-time-mode-vs-plain-mode). However, it only affects new documents and not existing ones.
Example:
* create a table with a full-text field and `charset_table` that allows only 3 searchable characters: `a`, `b` and `c`.
* then we insert document 'abcd' and find it by query `abcd`, the `d` just gets ignored since it's not in the `charset_table` array
* then we understand, that we want `d` to be searchable too, so we add it with help of `ALTER`
* but the same query `where match('abcd')` still says it searched by `abc`, because the existing document remembers previous contents of `charset_table`
* then we add another document `abcd` and search by `abcd` again
* now it finds the both documents and `show meta` says it used two keywords: `abc` (to find the old document) and `abcd` (for the new one).

<!-- request Example -->
```sql
mysql> create table rt(title text) charset_table='a,b,c';

mysql> insert into rt(title) values('abcd');

mysql> select * from rt where match('abcd');
+---------------------+-------+
| id                  | title |
+---------------------+-------+
| 1514630637682688054 | abcd  |
+---------------------+-------+

mysql> show meta;
+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| total         | 1     |
| total_found   | 1     |
| time          | 0.000 |
| keyword[0]    | abc   |
| docs[0]       | 1     |
| hits[0]       | 1     |
+---------------+-------+

mysql> alter table rt charset_table='a,b,c,d';
mysql> select * from rt where match('abcd');
+---------------------+-------+
| id                  | title |
+---------------------+-------+
| 1514630637682688054 | abcd  |
+---------------------+-------+

mysql> show meta
+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| total         | 1     |
| total_found   | 1     |
| time          | 0.000 |
| keyword[0]    | abc   |
| docs[0]       | 1     |
| hits[0]       | 1     |
+---------------+-------+

mysql> insert into rt(title) values('abcd');
mysql> select * from rt where match('abcd');
+---------------------+-------+
| id                  | title |
+---------------------+-------+
| 1514630637682688055 | abcd  |
| 1514630637682688054 | abcd  |
+---------------------+-------+

mysql> show meta;
+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| total         | 2     |
| total_found   | 2     |
| time          | 0.000 |
| keyword[0]    | abc   |
| docs[0]       | 1     |
| hits[0]       | 1     |
| keyword[1]    | abcd  |
| docs[1]       | 1     |
| hits[1]       | 1     |
+---------------+-------+
```

<!-- end -->

## Renaming a real-time table

<!-- example Renaming RT tables -->

You can change the name of a real-time table in RT mode.
```sql
ALTER TABLE table_name RENAME new_table_name;
```

> NOTE: Renaming a real-time table requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

<!-- request Example -->
```sql
ALTER TABLE table_name RENAME new_table_name;
```

<!-- response Example -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->

## Updating table FT settings in plain mode

<!-- example ALTER RECONFIGURE -->
```sql
ALTER TABLE table RECONFIGURE
```

`ALTER` can also reconfigure an RT table in the [plain mode](Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29), so that new tokenization, morphology and other text processing settings from the configuration file take effect for new documents. Note, that the existing document will be left intact. Internally, it forcibly saves the current RAM chunk as a new disk chunk and adjusts the table header, so that new documents are tokenized using the updated full-text settings.

<!-- request Example -->
```sql
mysql> show table rt settings;
+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| settings      |       |
+---------------+-------+
1 row in set (0.00 sec)

mysql> alter table rt reconfigure;
Query OK, 0 rows affected (0.00 sec)

mysql> show table rt settings;
+---------------+----------------------+
| Variable_name | Value                |
+---------------+----------------------+
| settings      | morphology = stem_en |
+---------------+----------------------+
1 row in set (0.00 sec)
```
<!-- end -->

## Rebuilding a secondary index

<!-- example ALTER REBUILD SECONDARY -->
```sql
ALTER TABLE table REBUILD SECONDARY
```

You can also use `ALTER` to rebuild secondary indexes in a given table. Sometimes, a secondary index can be disabled for the entire table or for one or multiple attributes within the table:
* When an attribute is updated, its secondary index gets disabled.
* If Manticore loads a table with an old version of secondary indexes that is no longer supported, the secondary indexes will be disabled for the entire table.

`ALTER TABLE table REBUILD SECONDARY` rebuilds secondary indexes from attribute data and enables them again.

Additionally, an old version of secondary indexes may be supported but will lack certain features. `REBUILD SECONDARY` can be used to update secondary indexes.

<!-- request Example -->
```sql
ALTER TABLE rt REBUILD SECONDARY;
```

<!-- response Example -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->

## Changing a distributed table

<!-- example local_dist -->

To change the list of local or remote nodes in a distributed table, follow the same syntax you used to [create the table](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table). Just replace `CREATE` with `ALTER` in the command and remove `type='distributed'`:

```sql
ALTER TABLE `distr_table_name` [[local='local_table_name'], [agent='host:port:remote_table'] ... ]
```

> NOTE: Renaming a real-time table requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

<!-- request Example -->
```sql
ALTER TABLE local_dist local='index1' local='index2' agent='127.0.0.1:9312:remote_table';
```

<!-- end -->
<!-- proofread -->
