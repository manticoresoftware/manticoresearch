# Attaching a plain index to a real-time index 

<!-- example Example_1 -->

A plain index can be converted into a real-time index or added to an existing real-time index.

The first case is useful when you need to regenerated a real-time index completely which may be needed for example if tokenization settings need an update. Then preparing a plain index and converting it into a real-time index may be easier than preparing a batch job to perform INSERTs for adding all the data into a real-time index.

In the second you normally want to add a large bulk of new data to a real-time index and again creating a plain index with that data is easier than populating the existing real-time index.

##### General syntax
The `ATTACH` statement allows to convert a plain index to be attached to an existing real-time index.

```sql
ATTACH INDEX diskindex TO RTINDEX rtindex [WITH TRUNCATE]
```

`ATTACH INDEX` statement lets you move data from a plain index to an RT index.

After a successful `ATTACH` the data originally stored in the source plain index becomes a part of the target RT index, and the source plain index becomes unavailable (until the next rebuild). `ATTACH` does not result in any index data changes. Basically, it just renames the files (making the source index a new disk chunk of the target RT index) and updates the metadata. So it is a generally quick operation which might (frequently) complete as fast as under a second.

Note that when an index is attached to an empty RT index the fields, attributes and text processing settings (tokenizer, wordforms, etc) from the *source* index are copied over and take effect. The respective parts of the RT index definition from the configuration file will be ignored.

When `TRUNCATE` option is used RT index gets truncated prior to attaching source plain index. This allows to make operation atomic or make sure that the attached source plain index will be the only data in the target RT index.

`ATTACH INDEX` comes with a number of restrictions. Most notably, the target RT index is currently required to be either empty or have the same settings as the source plain index. In case the source plain index gets attached to a non-empty RT index the RT index data collected so far gets stored as a regular disk chunk and index being attached becomes the newest disk chunk and documents with same IDs get killed. The complete list is as follows: 
* Target RT index needs to be either empty or have same settings 
* Source plain index needs to have [phrase_boundary_step](Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary_step)=0, [stopword_step](Creating_an_index/NLP_and_tokenization/Ignoring_stop-words.md#stopword_step)=1.


<!-- intro -->
##### Example:

<!-- request Example -->
Before ATTACH the RT index is empty and has 3 fields:

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

The plain index is not empty:

```sql
mysql> SELECT * FROM disk WHERE MATCH('test');
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
mysql> ATTACH INDEX disk TO RTINDEX rt;
Query OK, 0 rows affected (0.00 sec)
```

The RT index now has 5 fields:

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

The plain index was removed:

```sql
mysql> SELECT * FROM disk WHERE MATCH('test');
ERROR 1064 (42000): no enabled local indexes to search
```
<!-- end -->
