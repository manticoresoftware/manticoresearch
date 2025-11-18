# 将一个表附加到另一个表

<!-- example Example_1 -->

一个普通表可以转换为实时表，或者附加到现有的实时表中。

第一种情况在你需要完全重新生成实时表时非常有用，例如，当分词设置需要更新时。在这种情况下，准备一个普通表并将其转换为实时表可能比准备批处理作业执行 INSERT 操作将所有数据添加到实时表中更简单。

在第二种情况下，通常你想向实时表添加大量新数据，同样，创建一个包含这些数据的普通表比填充现有的实时表更容易。

你也可以将一个现有的实时表附加到另一个实时表。

##### 附加表 - 通用语法
`ATTACH` 语句允许你将普通表转换为附加到现有实时表的表。它还允许你将一个实时表的内容附加到另一个实时表。

```sql
ATTACH TABLE plain_or_rt_table TO TABLE rt_table [WITH TRUNCATE]
```

成功执行 `ATTACH` 后，原本存储在源普通表中的数据成为目标实时表的一部分，源普通表变得不可用（直到下一次重建）。如果源表是实时表，其内容会被移动到目标实时表中，源实时表保持为空。`ATTACH` 不会导致任何表数据的更改。实质上，它只是重命名文件（使源表成为目标实时表的新磁盘块）并更新元数据。因此，这通常是一个快速操作，通常可以在不到一秒内完成。

请注意，当一个表附加到一个空的实时表时，来自*源*表的字段、属性和文本处理设置（分词器、词形变化等）会被复制并生效。配置文件中实时表定义的相应部分将被忽略。

当使用 `TRUNCATE` 选项时，实时表在附加源普通表之前会被截断。这允许操作是原子的，或者确保附加的源普通表将是目标实时表中唯一的数据。

`ATTACH TABLE` 有许多限制。最显著的是，目标实时表目前必须为空，或者与源表具有相同的设置。如果源表附加到非空的实时表，迄今为止收集的实时表数据将作为常规磁盘块存储，附加的表成为最新的磁盘块，具有相同 ID 的文档将被删除。完整的限制列表如下：
* 目标实时表需要为空，或者与源表具有相同的设置。
* 源表需要将 [phrase_boundary_step](../../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary_step) 设置为 0，将 [stopword_step](../../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopword_step) 设置为 1。


<!-- intro -->
##### 示例：

<!-- request Example -->
在 ATTACH 之前，实时表为空且有 3 个字段：

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

普通表不为空：

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

将普通表附加到实时表：
```sql
mysql> ATTACH TABLE plain TO TABLE rt;
Query OK, 0 rows affected (0.00 sec)
```

实时表现在有 5 个字段：

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

且不为空：

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

ATTACH 之后，普通表被移除，不再可用于搜索：

```sql
mysql> SELECT * FROM plain WHERE MATCH('test');
ERROR 1064 (42000): no enabled local indexes to search
```
<!-- end -->
<!-- proofread -->

