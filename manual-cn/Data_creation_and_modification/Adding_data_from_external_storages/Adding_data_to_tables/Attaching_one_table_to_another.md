# 将一个表附加到另一个表

<!-- example Example_1 -->

普通表可以转换为实时表，或添加到现有的实时表中。

第一种情况在你需要完全重新生成实时表时非常有用，例如，当需要更新分词设置时。在这种情况下，准备一个普通表并将其转换为实时表可能比准备批处理作业以执行INSERT操作更容易。

第二种情况通常是你想要将大量新数据添加到实时表中，再次创建一个包含这些数据的普通表比填充现有的实时表更容易。

你也可以将现有的实时表附加到另一个实时表中。

##### 附加表 - 通用语法

`ATTACH` 语句允许你将普通表转换并附加到现有的实时表中。它也可以用于将一个实时表的内容附加到另一个实时表。

```sql
ATTACH TABLE plain_or_rt_table TO TABLE rt_table [WITH TRUNCATE]
```

在成功执行 `ATTACH` 之后，最初存储在源普通表中的数据将成为目标RT表的一部分，源普通表将不可用（直到下一次重建）。如果源表是一个实时表，则其内容将被移动到目标RT表中，而源实时表将变为空表。`ATTACH` 不会导致任何表数据的更改。基本上，它只是重命名文件（将源表变为目标RT表的新磁盘块）并更新元数据。所以通常这是一个快速操作，常常可以在一秒钟内完成。

需要注意的是，当一个表附加到空的RT表时，字段、属性和文本处理设置（分词器、词形变换等）会从*源表*复制过来并生效。配置文件中RT表定义的相应部分将被忽略。

当使用 `TRUNCATE` 选项时，RT表在附加源普通表之前将被截断。这允许操作具有原子性，或确保附加的源普通表将成为目标RT表中的唯一数据。

`ATTACH TABLE` 有许多限制。最显著的是目标RT表当前需要是空的或与源表具有相同的设置。在源表附加到非空的RT表时，RT表中迄今为止收集的数据将被存储为常规磁盘块，并且附加的表将成为最新的磁盘块，具有相同ID的文档将被删除。完整的限制列表如下：

- 目标RT表需要为空或与源表具有相同的设置。
- 源表需要将 [phrase_boundary_step](../../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary_step) 设置为 0，并将 [stopword_step](../../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopword_step) 设置为 1。


<!-- intro -->
##### 示例：

<!-- request Example -->
在 ATTACH 之前，RT 表为空，具有 3 个字段:

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

将普通表附加到RT表：
```sql
mysql> ATTACH TABLE plain TO TABLE rt;
Query OK, 0 rows affected (0.00 sec)
```

现在RT表有5个字段：

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

并且它不是空的：

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

附加操作之后，普通表被移除，不再可用于搜索：

```sql
mysql> SELECT * FROM plain WHERE MATCH('test');
ERROR 1064 (42000): no enabled local indexes to search
```
<!-- end -->
<!-- proofread -->
