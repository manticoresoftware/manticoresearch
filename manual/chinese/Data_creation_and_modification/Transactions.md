# 事务

Manticore 支持对实时表和检索表进行数据删除和插入的基本事务处理，但不支持尝试写入包含实时表或检索表的分布式表。对表的每次更改首先保存在内部变更集（changeset）中，然后实际提交到表。默认情况下，每个命令都被包装在独立的自动事务中，使其对用户透明：您只需执行“插入”操作，并且在完成后即可看到插入的结果，无需担心事务。然而，这种行为可以通过手动启动和提交事务来显式管理。

事务支持以下命令：
* [INSERT](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md)
* [REPLACE](../Data_creation_and_modification/Updating_documents/REPLACE.md)
* [DELETE](../Data_creation_and_modification/Deleting_documents.md)

事务不支持：
* UPDATE（它与 REPLACE [不同](../Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md)，因为它执行的是就地属性更新）
* ALTER - 用于[更新表结构](../Updating_table_schema_and_settings.md)
* TRUNCATE - 用于[清空实时表](../Emptying_a_table.md)
* ATTACH - 用于[将普通表挂载到实时表](../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md)
* CREATE - [用于创建表](../Creating_a_table/Local_tables.md)
* DROP - [用于删除表](../Deleting_a_table.md)

请注意，Manticore 中的事务不旨在提供隔离性。Manticore 事务的目的是允许您累积多次写操作，并在提交时一次性执行所有操作，或在必要时全部回滚。事务与[二进制日志](../Logging/Binary_logging.md) 集成，以确保持久性和一致性。

## 自动与手动模式

```sql
SET AUTOCOMMIT = {0 | 1}
```

`SET AUTOCOMMIT` 控制当前会话的自动提交模式。默认情况下，`AUTOCOMMIT` 设置为 1。默认设置下，您无需担心事务，因为每条对任何表进行更改的语句都会隐式包装在单独的事务中。将其设置为 0 则允许您手动管理事务，这意味着更改在您显式提交之前不可见。

事务限制为单个实时表或检索表，并且大小有限。事务是原子性的、一致性的、过度隔离（overly isolated）的且持久的。过度隔离意味着更改不仅对并发事务不可见，甚至对当前会话自身也是不可见的。

## BEGIN、COMMIT 和 ROLLBACK

```sql
START TRANSACTION | BEGIN
COMMIT
ROLLBACK
```

`BEGIN` 语句（或其别名 `START TRANSACTION`）会强制提交任何待处理事务（如果存在），然后开始一个新的事务。

`COMMIT` 语句提交当前事务，使所有更改永久生效。

`ROLLBACK` 语句回滚当前事务，取消所有更改。

## /bulk 中的事务

当使用 `/bulk` JSON 端点之一（[批量插入](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents)、[批量替换](../Data_creation_and_modification/Updating_documents/REPLACE.md?client=JSON#Bulk-replace)、[批量删除](../Data_creation_and_modification/Deleting_documents.md?client=JSON#Bulk-deletion)）时，您可以通过在文档后添加一个空行来强制提交这一批文档。

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

插入的值在随后的“select”语句中立即可见。

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

在这种情况下，更改不会自动提交。因此，即使在同一会话内，插入内容也不可见，因为它们尚未被提交。并且，尽管没有显式的 `BEGIN` 语句，事务也会隐式启动。

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

在执行提交语句后，插入内容在表中可见。

### 手动事务

通过使用 `BEGIN` 和 `COMMIT`，您可以显式定义事务边界，因此无需担心自动提交设置。

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

