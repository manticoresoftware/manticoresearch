# Listing indexes

Manticore Search has a single level of hierarchy of indexes.

There is no concept of grouping tables in databases like in other DBMS. Still, Manticore accepts `SHOW DATABASES` statements for interoperability with SQL dialect, but the statement doesn't return anything.

While the data collections in Manticore are called indexes, the statement that displays them is `SHOW TABLES` for compability with miscellaneous SQL clients.

<!-- example listing -->
## SHOW TABLES

General syntax:

```sql
SHOW TABLES [ LIKE pattern ]
```

`SHOW TABLES` statement enumerates all currently active indexes along with their types. Existing index types are `local`, `distributed`, `rt` and `template`. 


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW TABLES;
```

<!-- response SQL -->

```sql
+-------+-------------+
| Index | Type        |
+-------+-------------+
| dist1 | distributed |
| rt    | rt          |
| test1 | local       |
| test2 | local       |
+-------+-------------+
4 rows in set (0.00 sec)
```

<!-- request PHP -->

```php
$client->nodes()->table();
```

<!-- response PHP -->

```php
Array
(
    [dist1] => distributed
    [rt] => rt
    [test1] => local
    [test2] => local
)

```

<!-- end -->

<!-- example Example_2 -->
Optional LIKE clause is supported for filtering indexes by name.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW TABLES LIKE '%4';
```

<!-- response SQL -->

```sql
+-------+-------------+
| Index | Type        |
+-------+-------------+
| dist4 | distributed |
+-------+-------------+
1 row in set (0.00 sec)
```

<!-- request PHP -->

```php
$client->nodes()->table(['body'=>['pattern'=>'%4']]);
```

<!-- response PHP -->

```php
Array
(
    [dist4] => distributed
)

```
<!-- end -->


## DESCRIBE

```sql
{DESC | DESCRIBE} index [ LIKE pattern ]
```

`DESCRIBE` statement lists index columns and their associated types. Columns are document ID, full-text fields, and attributes. The order matches that in which fields and attributes are expected by `INSERT` and `REPLACE` statements. Column types are `field`, `integer`, `timestamp`, `ordinal`, `bool`, `float`, `bigint`, `string`, and `mva`. ID column will be typed as `bigint`. Example:

```sql
mysql> DESC rt;
+---------+---------+
| Field   | Type    |
+---------+---------+
| id      | bigint  |
| title   | field   |
| content | field   |
| gid     | integer |
+---------+---------+
4 rows in set (0.00 sec)
```

An optional LIKE clause is supported. Refer to
[SHOW META](Profiling_and_monitoring/SHOW_META.md) for its syntax details.

## SHOW CREATE TABLE

<!-- example show_create -->
```sql
SHOW CREATE TABLE name
```

Prints the `CREATE TABLE` statement that creates the named table.


<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
SHOW CREATE TABLE idx\G
```

<!-- response SQL -->
```sql
       Table: idx
Create Table: CREATE TABLE idx (
f text indexed stored
) charset_table='non_cjk,cjk' morphology='icu_chinese'
1 row in set (0.00 sec)
```
<!-- end -->

### Percolate index schemas

If you apply `DESC` statement to a percolate index it will show the outer index schema, i.e. the schema of stored queries. It's static and the same for all local pq indexes:

```sql
mysql> DESC pq;
+---------+--------+
| Field   | Type   |
+---------+--------+
| id      | bigint |
| query   | string |
| tags    | string |
| filters | string |
+---------+--------+
4 rows in set (0.00 sec)
```

If you're looking for an expected document schema use
`DESC <pq index name> table`:

```sql
mysql> DESC pq TABLE;
+-------+--------+
| Field | Type   |
+-------+--------+
| id    | bigint |
| title | text   |
| gid   | uint   |
+-------+--------+
3 rows in set (0.00 sec)
```

Also `desc pq table like ...` is supported and works as follows:

```sql
mysql> desc pq table like '%title%';
+-------+------+----------------+
| Field | Type | Properties     |
+-------+------+----------------+
| title | text | indexed stored |
+-------+------+----------------+
1 row in set (0.00 sec)
```

