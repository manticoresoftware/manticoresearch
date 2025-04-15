# 将一个表附加到另一个表

<!-- example Example_1 -->

一个普通表可以被转换为实时表或附加到现有的实时表中。

第一种情况是在你需要完全重新生成实时表时很有用，例如，如果需要更新标记化设置。在这种情况下，准备一个普通表并将其转换为实时表，可能比准备一个批处理作业执行 INSERT 以将所有数据添加到实时表中更为简单。

在第二种情况下，你通常希望将大量新数据添加到实时表中，再次地，创建一个包含该数据的普通表比填充现有的实时表更简单。

你也可以将一个现有的实时表附加到另一个表中。

##### 附加表 - 一般语法
`ATTACH` 语句允许你将普通表转换为附加到现有实时表。它还使你能够将一个实时表的内容附加到另一个实时表。

```sql
ATTACH TABLE plain_or_rt_table TO TABLE rt_table [WITH TRUNCATE]
```

成功 `ATTACH` 后，最初存储在源普通表中的数据成为目标 RT 表的一部分，而源普通表变为不可用（直到下次重建）。如果源表是一个 RT 表，其内容将被移动到目标 RT 表中，源 RT 表将保持为空。`ATTACH` 不会导致任何表数据的更改。实际上，它只是重命名文件（使源表成为目标 RT 表的新磁盘块）并更新元数据。因此，这通常是一个快速的操作，可能（通常）在一秒内完成。

请注意，当一个表附加到一个空的 RT 表时，*源* 表的字段、属性和文本处理设置（标记器、词形等）会被复制并生效。配置文件中的 RT 表定义中的相应部分将被忽略。

当使用 `TRUNCATE` 选项时，RT 表会在附加源普通表之前被截断。这使得操作可以是原子性的，或确保附加的源普通表将是目标 RT 表中的唯一数据。

`ATTACH TABLE` 有若干限制。最明显的是，目标 RT 表目前需要为空或具有与源表相同的设置。如果源表被附加到一个非空的 RT 表，当前已收集的 RT 表数据将作为常规磁盘块存储，而被附加的表成为最新的磁盘块，具有相同 ID 的文档将被删除。限制的完整列表如下：
* 目标 RT 表需要为空或具有与源表相同的设置。
* 源表需要将 [phrase_boundary_step](../../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary_step) 设置为 0，将 [stopword_step](../../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopword_step) 设置为 1。


<!-- intro -->
##### 示例：

<!-- request Example -->
在 ATTACH 之前，RT 表为空且有 3 个字段：

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

普通表并不为空：

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

将普通表附加到 RT 表：
```sql
mysql> ATTACH TABLE plain TO TABLE rt;
Query OK, 0 rows affected (0.00 sec)
```

RT 表现在有 5 个字段：

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

它现在不为空：

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

ATTACH 后，普通表被移除，不再可用于搜索：

```sql
mysql> SELECT * FROM plain WHERE MATCH('test');
ERROR 1064 (42000): no enabled local indexes to search
```
<!-- end -->
<!-- proofread -->
