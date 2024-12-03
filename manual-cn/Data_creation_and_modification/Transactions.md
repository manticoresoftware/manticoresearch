# 事务

Manticore 支持基本的事务操作，用于在实时表和预注册表中删除和插入数据，除了写入包含实时表或预注册表的分布式表时。每个对表的更改首先保存在内部的变更集（changeset）中，然后实际提交到表中。默认情况下，每条命令都自动封装在一个单独的事务中，使操作透明：您只需执行“插入”，在完成后就可以看到插入的结果，无需担心事务管理。然而，这种行为也可以通过手动启动和提交事务来显式管理。

以下命令支持事务：

- [INSERT](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md)
- [REPLACE](../Data_creation_and_modification/Updating_documents/REPLACE.md)
- [DELETE](../Data_creation_and_modification/Deleting_documents.md)

以下操作不支持事务：

- UPDATE（与 REPLACE 不同，详见[REPLACE 与 UPDATE 的区别](../Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md)，UPDATE 是就地更新属性）
- ALTER - 用于[更新表模式](../Updating_table_schema_and_settings.md)
- TRUNCATE - 用于[清空实时表](../Emptying_a_table.md)
- ATTACH - 用于[将普通表附加到实时表](../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md)
- CREATE - 用于[创建表](../Creating_a_table/Local_tables.md)
- DROP - 用于[删除表](../Deleting_a_table.md)

请注意，Manticore 中的事务并不提供隔离性。Manticore 事务的目的是允许您积累多个写操作，并在提交时一次性执行，或者在必要时回滚所有更改。事务与[二进制日志](../Logging/Binary_logging.md)集成，以确保持久性和一致性。

## 自动模式和手动模式

```sql
SET AUTOCOMMIT = {0 | 1}
```

`SET AUTOCOMMIT` 控制当前会话中的自动提交模式。`AUTOCOMMIT` 默认为 1。默认情况下，您无需担心事务管理，因为每个对表有更改的语句都会自动封装在一个独立的事务中。将其设置为 0 允许您手动管理事务，这意味着更改在显式提交前是不可见的。

事务仅限于单个实时表或预注册表，并且有大小限制。它们是原子的、一致的、超隔离的并且是持久的。超隔离意味着更改不仅对并发事务不可见，甚至对当前会话本身也是不可见的。

## BEGIN、COMMIT 和 ROLLBACK

```sql
START TRANSACTION | BEGIN
COMMIT
ROLLBACK
```

`BEGIN` 语句（或其别名 `START TRANSACTION`）会强制提交任何未完成的事务（如果存在），并开始一个新的事务。

`COMMIT` 语句提交当前事务，使所有更改永久生效。

`ROLLBACK` 语句回滚当前事务，取消所有更改。

## `/bulk` 中的事务

当使用 `/bulk` JSON 端点之一（[批量插入](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents)、[批量替换](../Data_creation_and_modification/Updating_documents/REPLACE.md?client=JSON#Bulk-replace)、[批量删除](../Data_creation_and_modification/Deleting_documents.md?client=JSON#Bulk-deletion)）时，您可以通过在文档后添加一个空行来强制提交一批文档。

## 示例

### 自动提交（默认模式）

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

插入的值在随后的 `select` 语句中立即可见。

### 手动提交 (autocommit=0)

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

在这种情况下，更改不会自动提交。因此，插入的数据即使在同一个会话中也是不可见的，因为它们尚未提交。此外，尽管没有 `BEGIN` 语句，但事务隐式地已经开始。

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

提交语句之后，插入的数据在表中可见。

### 手动事务

通过使用 `BEGIN` 和 `COMMIT`，您可以显式定义事务的边界，因此在这种情况下不需要担心自动提交。

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
