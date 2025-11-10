# 事务

Manticore 支持对实时表和感知表进行基本的删除和插入数据的事务操作，但不支持对包含实时表或感知表的分布式表进行写操作。对表的每次更改首先保存在内部的变更集中，然后实际提交到表中。默认情况下，每个命令都被包装在一个独立的自动事务中，使其对用户透明：您只需执行“插入”操作，完成后即可看到插入的结果，无需担心事务。然而，这种行为可以通过手动启动和提交事务来显式管理。

以下命令支持事务：
* [INSERT](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md)
* [REPLACE](../Data_creation_and_modification/Updating_documents/REPLACE.md)
* [DELETE](../Data_creation_and_modification/Deleting_documents.md)

以下命令不支持事务：
* UPDATE（与 REPLACE 不同，详见[这里](../Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md)，因为它执行的是就地属性更新）
* ALTER - 用于[更新表结构](../Updating_table_schema_and_settings.md)
* TRUNCATE - 用于[清空实时表](../Emptying_a_table.md)
* ATTACH - 用于[将普通表附加到实时表](../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md)
* CREATE - [创建表](../Creating_a_table/Local_tables.md)
* DROP - [删除表](../Deleting_a_table.md)

请注意，Manticore 中的事务并不旨在提供隔离性。Manticore 中事务的目的是允许您累积多个写操作，并在提交时一次性执行，或者在必要时全部回滚。事务与[二进制日志](../Logging/Binary_logging.md)集成，以保证持久性和一致性。

## 自动和手动模式

```sql
SET AUTOCOMMIT = {0 | 1}
```

`SET AUTOCOMMIT` 控制当前会话的自动提交模式。默认情况下，`AUTOCOMMIT` 设置为 1。默认设置下，您无需担心事务，因为对任何表进行更改的每条语句都会隐式地包装在一个独立的事务中。将其设置为 0 则允许您手动管理事务，即事务在您显式提交之前不会生效。

事务仅限于单个实时表或感知表，并且大小有限。它们是原子性的、一致的、过度隔离的且持久的。过度隔离意味着更改不仅对并发事务不可见，甚至对当前会话本身也不可见。

## BEGIN、COMMIT 和 ROLLBACK

```sql
START TRANSACTION | BEGIN
COMMIT
ROLLBACK
```

`BEGIN` 语句（或其别名 `START TRANSACTION`）会强制提交任何未完成的事务（如果存在），然后开始一个新的事务。

`COMMIT` 语句提交当前事务，使其所有更改永久生效。

`ROLLBACK` 语句回滚当前事务，取消所有更改。

## /bulk 中的事务

使用 `/bulk` JSON 端点（[批量插入](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents)、[批量替换](../Data_creation_and_modification/Updating_documents/REPLACE.md?client=JSON#Bulk-replace)、[批量删除](../Data_creation_and_modification/Deleting_documents.md?client=JSON#Bulk-deletion)）时，可以通过在文档批次后添加一个空行来强制提交该批次的文档。

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

插入的值在随后的 'select' 语句中立即可见。

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

在这种情况下，更改不会自动提交。因此，即使在同一会话中，插入的内容也不可见，因为它们尚未提交。尽管没有 `BEGIN` 语句，事务仍然隐式启动。

要使更改可见，您需要提交事务：

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

提交语句执行后，插入的内容在表中可见。

### 手动事务

通过使用 `BEGIN` 和 `COMMIT`，您可以显式定义事务的边界，因此无需担心自动提交。

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

