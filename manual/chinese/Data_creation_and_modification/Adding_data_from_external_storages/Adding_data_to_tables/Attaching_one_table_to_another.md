# 将一个表附加到另一个表

<!-- example Example_1 -->

一个普通的表可以被转换为实时表或附加到现有的实时表。

第一种情况在需要完全重新生成实时表时有用，例如，如果需要更新分词设置。在这种情况下，准备一个普通的表并将其转换为实时表可能比创建批处理作业来插入所有数据到实时表更容易。

第二种情况是通常希望将大量新数据添加到实时表中，同样地，创建包含这些数据的普通表比填充现有实时表更容易。

你也可以将现有的实时表附加到另一个实时表。

##### 附加表 - 通用语法
`ATTACH` 语句允许将一个普通的表转换为附加到现有实时表的表。它还使一个实时表的内容能够附加到另一个实时表。

```sql
ATTACH TABLE plain_or_rt_table TO TABLE rt_table [WITH TRUNCATE]
```

成功执行 `ATTACH` 后，源普通表中原有的数据将成为目标RT表的一部分，并且源普通表变得不可用（直到下一次重建）。如果源表是一个RT表，则其内容将移动到目标RT表，源RT表保持为空。`ATTACH` 不会改变任何表的数据。本质上，它只是重命名文件（使源表成为目标RT表的新磁盘块）并更新元数据。因此，它通常是一个快速操作，可能会在不到一秒的时间内完成。

请注意，当一个表附加到一个空的RT表时，源表中的字段、属性和文本处理设置（分词器、词形变化等）会被复制并生效。配置文件中相应的RT表定义部分将被忽略。

当使用 `TRUNCATE` 选项时，在附加源普通表之前，RT表会被截断。这使得操作原子化或确保附加的源普通表将是目标RT表中的唯一数据。

`ATTACH TABLE` 具有一些限制。最值得注意的是，目标RT表当前必须为空或与源表具有相同的设置。如果源表被附加到一个非空的RT表，则迄今为止收集的RT表数据将作为常规磁盘块存储，而被附加的表将成为最新的磁盘块，具有相同ID的文档将被删除。完整的限制列表如下：
* 目标RT表需要为空或与源表具有相同的设置。
* 源表需要设置 [phrase_boundary_step](../../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary_step) 为 0 和 [stopword_step](../../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopword_step) 为 1。


<!-- intro -->
##### 示例：

<!-- request Example -->
在执行 ATTACH 之前，RT 表为空并且有 3 个字段：

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

普通的表不为空：

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

将普通的表附加到 RT 表：
```sql
mysql> ATTACH TABLE plain TO TABLE rt;
Query OK, 0 rows affected (0.00 sec)
```

现在，RT 表有 5 个字段：

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

并且它不为空：

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

在执行 ATTACH 之后，普通的表被移除并且不再可用于搜索：

```sql
mysql> SELECT * FROM plain WHERE MATCH('test');
ERROR 1064 (42000): no enabled local indexes to search
```
<!-- end -->
<!-- proofread -->

