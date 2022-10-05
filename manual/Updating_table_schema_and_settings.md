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
* ❗It's recommended to **backup index files** before `ALTER`ing it to avoid data corruption in case of a sudden power interruption or other similar issues.
* Querying an index is impossible while a column is being added.
* Newly created attribute's values are set to 0.
* `ALTER` will not work for distributed indexes and indexes without any attributes.
* `DROP COLUMN` will fail if an index has only one field.
* When dropping a field which is both a full-text field and a string attribute the first `ALTER DROP` drops the attribute, the second one drops the full-text field.
* Adding/dropping full-text field is only supported in [RT mode](Read_this_first.md#Real-time-mode-vs-plain-mode).

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

## Updating index FT settings in RT mode

<!-- example ALTER FT -->

```sql
ALTER RTINDEX index ft_setting='value'[, ft_setting2='value']
```

You can also use `ALTER` to modify full-text settings of your index in [RT mode](Read_this_first.md#Real-time-mode-vs-plain-mode). Just remember that it doesn't affect existing documents, it only affects new ones. Take a look at the example where we:
* create an index with a full-text field and `charset_table` that allows only 3 searchable characters: `a`, `b` and `c`.
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

## Updating index FT settings in plain mode

<!-- example ALTER RECONFIGURE -->
```sql
ALTER RTINDEX index RECONFIGURE
```

`ALTER` can also reconfigure an RT index in [plain mode](Creating_an_index/Local_indexes.md#Defining-index-schema-in-config-%28Plain-mode%29), so that new tokenization, morphology and other text processing settings from the configuration file take effect for new documents. Note, that the existing document will be left intact. Internally, it forcibly saves the current RAM chunk as a new disk chunk and adjusts the index header, so that new documents are tokenized using the updated full-text settings.

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

## Rebuild secondary index

<!-- example ALTER REBUILD SECONDARY -->
```sql
ALTER TABLE table REBUILD SECONDARY
```

`ALTER` can also be used to rebuild secondary indexes in a given table. Sometimes a secondary index can be disabled for a whole index or for one/multiple attributes in the index. `ALTER TABLE table REBUILD SECONDARY` rebuilds the secondary index from the attribute data and enables it again. Secondary indexes get disabled:
* on `UPDATE` of an attribute: in this case its secondary index gets disabled
* in case Manticore loads a table with old formatted secondary indexes: in this case secondary indexes will be disabled for the whole table

<!-- request Example -->
```sql
ALTER TABLE rt REBUILD SECONDARY;
```

<!-- response Example -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->
