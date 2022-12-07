# Listing tables

Manticore Search has a single level of hierarchy of tables.

There is no concept of grouping tables in databases like in other DBMS. Still, Manticore accepts `SHOW DATABASES` statements for interoperability with SQL dialect, but the statement doesn't return anything.

<!-- example listing -->
## SHOW TABLES

General syntax:

```sql
SHOW TABLES [ LIKE pattern ]
```

`SHOW TABLES` statement enumerates all currently active tables along with their types. Existing table types are `local`, `distributed`, `rt`, `percolate` and `template`.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW TABLES;
```

<!-- response SQL -->

```sql
+----------+-------------+
| Index    | Type        |
+----------+-------------+
| dist     | distributed |
| plain    | local       |
| pq       | percolate   |
| rt       | rt          |
| template | template    |
+----------+-------------+
5 rows in set (0.00 sec)
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
    [products] => rt
)

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('SHOW TABLES')
```

<!-- response Python -->
```python
{u'columns': [{u'Index': {u'type': u'string'}},
              {u'Type': {u'type': u'string'}}],
 u'data': [{u'Index': u'dist1', u'Type': u'distributed'},
           {u'Index': u'rt', u'Type': u'rt'},
           {u'Index': u'products', u'Type': u'rt'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('SHOW TABLES');
```

<!-- response javascript -->
```javascript
{"columns":[{"Index":{"type":"string"}},{"Type":{"type":"string"}}],"data":[{"Index":"products","Type":"rt"}],"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("SHOW TABLES")
```

<!-- response Java -->
```java
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error=, warning=}
```

<!-- end -->

<!-- example Example_2 -->
Optional LIKE clause is supported for filtering tables by name.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW TABLES LIKE 'pro%';
```

<!-- response SQL -->

```sql
+----------+-------------+
| Index    | Type        |
+----------+-------------+
| products | distributed |
+----------+-------------+
1 row in set (0.00 sec)
```

<!-- request PHP -->

```php
$client->nodes()->table(['body'=>['pattern'=>'pro%']]);
```

<!-- response PHP -->

```php
Array
(
    [products] => distributed
)

```


<!-- intro -->
##### Python:

<!-- request Python -->

```python
res = await utilsApi.sql('SHOW TABLES LIKE \'pro%\'');
```

<!-- response Python -->
```python
{u'columns': [{u'Index': {u'type': u'string'}},
              {u'Type': {u'type': u'string'}}],
 u'data': [{u'Index': u'products', u'Type': u'rt'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
utilsApi.sql('SHOW TABLES LIKE \'pro%\'')
```

<!-- response javascript -->
```javascript
{"columns":[{"Index":{"type":"string"}},{"Type":{"type":"string"}}],"data":[{"Index":"products","Type":"rt"}],"total":0,"error":"","warning":""}
```


<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("SHOW TABLES LIKE 'pro%'")
```

<!-- response Java -->
```java
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error=, warning=}
```


<!-- end -->

## DESCRIBE

```sql
{DESC | DESCRIBE} table [ LIKE pattern ]
```

`DESCRIBE` statement lists table columns and their associated types. Columns are document ID, full-text fields, and attributes. The order matches that in which fields and attributes are expected by `INSERT` and `REPLACE` statements. Column types are `field`, `integer`, `timestamp`, `ordinal`, `bool`, `float`, `bigint`, `string`, and `mva`. ID column will be typed as `bigint`. Example:

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

### SELECT FROM name.table

<!-- example name_table -->
You can also see table schema by executing the query `select * from <table_name>.table`. The benefit of this method is that you can use `WHERE` for filtering:

<!-- request SQL -->
```sql
select * from tbl.table where type='text';
```

<!-- response SQL -->
```sql
+------+-------+------+----------------+
| id   | field | type | properties     |
+------+-------+------+----------------+
|    2 | title | text | indexed stored |
+------+-------+------+----------------+
1 row in set (0.00 sec)
```

<!-- end -->

<!-- example name_table2 -->

You can also do many other things, consider `<your_table_name>.table` just a regular Manticore table where the columns are integer and string attributes.

<!-- request SQL -->

```sql
select field from tbl.table;
select field, properties from tbl.table where type in ('text', 'uint');
select * from tbl.table where properties any ('stored');
```

<!-- end -->

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
SHOW CREATE TABLE tbl\G
```

<!-- response SQL -->
```sql
       Table: tbl
Create Table: CREATE TABLE tbl (
f text indexed stored
) charset_table='non_cjk,cjk' morphology='icu_chinese'
1 row in set (0.00 sec)
```
<!-- end -->

### Percolate table schemas

If you apply `DESC` statement to a percolate table it will show the outer table schema, i.e. the schema of stored queries. It's static and the same for all local percolate tables:

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
`DESC <pq table name> table`:

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
