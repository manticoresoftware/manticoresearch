# Attaching a plain table to a real-time table

<!-- example Example_1 -->

A plain table can be converted into a real-time table or added to an existing real-time table.

The first case is useful when you need to regenerated a real-time table completely which may be needed for example if tokenization settings need an update. Then preparing a plain table and converting it into a real-time table may be easier than preparing a batch job to perform INSERTs for adding all the data into a real-time table.

In the second you normally want to add a large bulk of new data to a real-time table and again creating a plain table with that data is easier than populating the existing real-time table.

##### Attaching table - general syntax
The `ATTACH` statement allows to convert a plain table to be attached to an existing real-time table.

```sql
ATTACH TABLE plain_table TO TABLE rt_table [WITH TRUNCATE]
```

`ATTACH TABLE` statement lets you move data from a plain table to an RT table.

After a successful `ATTACH` the data originally stored in the source plain table becomes a part of the target RT table, and the source plain table becomes unavailable (until the next rebuild). `ATTACH` does not result in any table data changes. Basically, it just renames the files (making the source table a new disk chunk of the target RT table) and updates the metadata. So it is a generally quick operation which might (frequently) complete as fast as under a second.

Note that when a table is attached to an empty RT table the fields, attributes and text processing settings (tokenizer, wordforms, etc) from the *source* table are copied over and take effect. The respective parts of the RT table definition from the configuration file will be ignored.

When `TRUNCATE` option is used RT table gets truncated prior to attaching source plain table. This allows to make operation atomic or make sure that the attached source plain table will be the only data in the target RT table.

`ATTACH TABLE` comes with a number of restrictions. Most notably, the target RT table is currently required to be either empty or have the same settings as the source plain table. In case the source plain table gets attached to a non-empty RT table the RT table data collected so far gets stored as a regular disk chunk and table being attached becomes the newest disk chunk and documents with same IDs get killed. The complete list is as follows:
* Target RT table needs to be either empty or have same settings
* Source plain table needs to have [phrase_boundary_step](../../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary_step)=0, [stopword_step](../../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopword_step)=1.


<!-- intro -->
##### Example:

<!-- request Example -->
Before ATTACH the RT table is empty and has 3 fields:

```sql
mysql> DESC rt;
Empty set (0.00 sec)

mysql> SELECT * FROM rt;
+-----------+---------+
| Field     | Type    |
+-----------+---------+
| id        | integer |
| testfield | field   |
| testattr  | uint    |
+-----------+---------+
3 rows in set (0.00 sec)
```

The plain table is not empty:

```sql
mysql> SELECT * FROM plain WHERE MATCH('test');
+------+--------+----------+------------+
| id   | weight | group_id | date_added |
+------+--------+----------+------------+
|    1 |   1304 |        1 | 1313643256 |
|    2 |   1304 |        1 | 1313643256 |
|    3 |   1304 |        1 | 1313643256 |
|    4 |   1304 |        1 | 1313643256 |
+------+--------+----------+------------+
4 rows in set (0.00 sec)
```

Attaching:
```sql
mysql> ATTACH TABLE plain TO TABLE rt;
Query OK, 0 rows affected (0.00 sec)
```

The RT table now has 5 fields:

```sql
mysql> DESC rt;
+------------+-----------+
| Field      | Type      |
+------------+-----------+
| id         | integer   |
| title      | field     |
| content    | field     |
| group_id   | uint      |
| date_added | timestamp |
+------------+-----------+
5 rows in set (0.00 sec)
```

And it's not empty:

```sql
mysql> SELECT * FROM rt WHERE MATCH('test');
+------+--------+----------+------------+
| id   | weight | group_id | date_added |
+------+--------+----------+------------+
|    1 |   1304 |        1 | 1313643256 |
|    2 |   1304 |        1 | 1313643256 |
|    3 |   1304 |        1 | 1313643256 |
|    4 |   1304 |        1 | 1313643256 |
+------+--------+----------+------------+
4 rows in set (0.00 sec)
```

The plain table was removed:

```sql
mysql> SELECT * FROM plain WHERE MATCH('test');
ERROR 1064 (42000): no enabled local indexes to search
```
<!-- end -->
