# 事务

Manticore 支持基本的事务功能，用于删除和插入实时表和过滤表中的数据，但不包括尝试向包含实时或过滤表的分布式表写入的情况。每次对表的更改首先保存在一个内部变更集里，然后才实际提交到表中。默认情况下，每个命令都包裹在一个单独的自动事务中，使其透明：你只需“插入”一些内容，在完成之后可以看到插入的结果，而无需担心事务。然而，这种行为可以通过手动开始和提交事务来显式管理。

支持事务的命令如下：
* [INSERT](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md)
* [REPLACE](../Data_creation_and_modification/Updating_documents/REPLACE.md)
* [DELETE](../Data_creation_and_modification/Deleting_documents.md)

以下命令不支持事务：
* UPDATE（这与REPLACE不同，因为UPDATE执行就地属性更新）
* ALTER - 用于[更新表结构](../Updating_table_schema_and_settings.md)
* TRUNCATE - 用于[清空实时表](../Emptying_a_table.md)
* ATTACH - 用于[将普通表附加到实时表](../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md)
* CREATE - 用于[创建表](../Creating_a_table/Local_tables.md)
* DROP - 用于[删除表](../Deleting_a_table.md)

请注意，Manticore 中的事务并不旨在提供隔离性。Manticore 中事务的目的在于允许你累积多个写操作，并在提交时一次性执行所有这些操作，或者如果需要的话，可以回滚所有这些操作。事务与[二进制日志](../Logging/Binary_logging.md)集成以确保持久性和一致性。

## 自动模式和手动模式

```sql
SET AUTOCOMMIT = {0 | 1}
```

`SET AUTOCOMMIT` 控制当前会话的自动提交模式。默认情况下，`AUTOCOMMIT` 设置为 1。在这种默认设置下，你不需要担心事务，因为任何对任何表进行更改的语句都会隐式地包裹在一个单独的事务中。将其设置为 0 允许你手动管理事务，这意味着它们不会在你显式提交之前可见。

事务仅限于一个实时或过滤表，并且也有大小限制。它们是原子的、一致的、过度隔离的和持久的。过度隔离意味着这些更改不仅对并发事务不可见，甚至对当前会话本身也不可见。

## BEGIN、COMMIT 和 ROLLBACK

```sql
START TRANSACTION | BEGIN
COMMIT
ROLLBACK
```

`BEGIN` 语句（或其别名 `START TRANSACTION`）强制提交任何现有的事务（如果有），并开始一个新的事务。

`COMMIT` 语句提交当前事务，使所有更改永久化。

`ROLLBACK` 语句回滚当前事务，取消所有更改。

## /bulk 中的事务

当你使用其中一个 `/bulk` JSON 端点（[批量插入](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents)，[批量替换](../Data_creation_and_modification/Updating_documents/REPLACE.md?client=JSON#Bulk-replace)，[批量删除](../Data_creation_and_modification/Deleting_documents.md?client=JSON#Bulk-deletion)）时，可以通过在文档后添加一个空行来强制提交一批文档。

## 示例

### 自动提交（默认）

```sql
insert into indexrt (id, content, title, channel_id, published) values (1, 'aa', 'blabla', 1, 10);
Query OK, 1 rows affected (0.00 sec)

select * from indexrt where id=1;
+------+------------+-----------+--------+
| id   | channel_id | published | title  |
+------+------------+-----------+--------+
|    1 |          1 |        10 | blabla |
+------+------------+-----------+--------+
1 row in set (0.00 sec)
```

插入的值立即在随后的 'select' 语句中可见。

### 手动提交（autocommit=0）

```sql
set autocommit=0;
Query OK, 0 rows affected (0.00 sec)

insert into indexrt (id, content, title, channel_id, published) values (3, 'aa', 'bb', 1, 1);
Query OK, 1 row affected (0.00 sec)

insert into indexrt (id, content, title, channel_id, published) values (4, 'aa', 'bb', 1, 1);
Query OK, 1 row affected (0.00 sec)

select * from indexrt where id=3;
Empty set (0.01 sec)

select * from indexrt where id=4;
Empty set (0.00 sec)
```

在这种情况下，更改不会自动提交。因此，即使在同一会话中，插入的内容也是不可见的，因为它们尚未提交。尽管没有 `BEGIN` 语句，一个事务仍然隐式启动。

要使更改可见，你需要提交事务：

```sql
commit;
Query OK, 0 rows affected (0.00 sec)

select * from indexrt where id=4;
+------+------------+-----------+-------+
| id   | channel_id | published | title |
+------+------------+-----------+-------+
|    4 |          1 |         1 | bb    |
+------+------------+-----------+-------+
1 row in set (0.00 sec)

select * from indexrt where id=3;
+------+------------+-----------+-------+
| id   | channel_id | published | title |
+------+------------+-----------+-------+
|    3 |          1 |         1 | bb    |
+------+------------+-----------+-------+
1 row in set (0.00 sec)
```

在提交语句之后，插入的内容在表中可见。

### 手动事务

通过使用 `BEGIN` 和 `COMMIT`，你可以明确定义事务的边界，因此在这种情况下不需要担心自动提交。

```sql
begin;
Query OK, 0 rows affected (0.00 sec)

insert into indexrt (id, content, title, channel_id, published) values (2, 'aa', 'bb', 1, 1);
Query OK, 1 row affected (0.00 sec)

select * from indexrt where id=2;
Empty set (0.01 sec)

commit;
Query OK, 0 rows affected (0.01 sec)

select * from indexrt where id=2;
+------+------------+-----------+-------+
| id   | channel_id | published | title |
+------+------------+-----------+-------+
|    2 |          1 |         1 | bb    |
+------+------------+-----------+-------+
1 row in set (0.01 sec)
```


<!-- proofread -->

