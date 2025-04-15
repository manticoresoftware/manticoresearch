# 交易

Manticore 支持基本交易，用于在实时表和渗透表中删除和插入数据，除非尝试写入包含实时表或渗透表的分布式表。对表的每次更改首先保存在内部更改集中，然后实际提交到表中。默认情况下，每个命令都被包裹在单独的自动事务中，使其透明：您只需“插入”某些内容，并可以在完成后查看插入的结果，而无需担心事务。然而，这种行为可以通过手动启动和提交事务来明确管理。

交易支持以下命令：
* [INSERT](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md)
* [REPLACE](../Data_creation_and_modification/Updating_documents/REPLACE.md)
* [DELETE](../Data_creation_and_modification/Deleting_documents.md)

不支持的事务包括：
* UPDATE（与 [REPLACE](../Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md) 不同，因为它执行就地属性更新）
* ALTER - 用于 [更新表架构](../Updating_table_schema_and_settings.md)
* TRUNCATE - 用于 [清空实时表](../Emptying_a_table.md)
* ATTACH - 用于 [将普通表附加到实时表](../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md)
* CREATE - [用于创建表](../Creating_a_table/Local_tables.md)
* DROP - [用于删除表](../Deleting_a_table.md)

请注意，Manticore 中的事务并不旨在提供隔离。Manticore 中事务的目的在于允许您累积多次写入，并在提交时一次性执行它们，或在必要时全部回滚。事务与 [二进制日志](../Logging/Binary_logging.md) 集成，以增强耐久性和一致性。

## 自动和手动模式

```sql
SET AUTOCOMMIT = {0 | 1}
```

`SET AUTOCOMMIT` 控制活动会话中的自动提交模式。默认情况下，将 `AUTOCOMMIT` 设置为 1。在默认设置下，您无需担心事务，因为对任何表进行任何更改的每个语句都会隐式地包裹在一个单独的事务中。将其设置为 0 允许您手动管理事务，这意味着在您明确提交之前，它们将不可见。

事务限于单个实时或渗透表，并且在大小上也有限制。它们是原子性的、一致的、过度隔离和持久的。过度隔离意味着这些更改不仅对并发事务不可见，甚至对当前会话本身也是看不见的。

## BEGIN、COMMIT 和 ROLLBACK

```sql
START TRANSACTION | BEGIN
COMMIT
ROLLBACK
```

`BEGIN` 语句（或其 `START TRANSACTION` 别名）强制提交任何待处理的事务（如果存在），并开始一个新的事务。

`COMMIT` 语句提交当前事务，使其所有更改永久生效。

`ROLLBACK` 语句回滚当前事务，取消其所有更改。

## /bulk 中的事务

当使用其中一个 `/bulk` JSON 端点（ [批量插入](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents)， [批量替换](../Data_creation_and_modification/Updating_documents/REPLACE.md?client=JSON#Bulk-replace)， [批量删除](../Data_creation_and_modification/Deleting_documents.md?client=JSON#Bulk-deletion) ）时，您可以通过在它们后面添加一个空行来强制将一批文档提交。

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

插入的值在后续的“选择”语句中立即可见。

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

在这种情况下，更改不会自动提交。因此，即使在同一会话中，插入也不可见，因为它们尚未被提交。此外，尽管缺少 `BEGIN` 语句，但事务仍会隐式启动。

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

在提交语句之后，插入的数据在表中可见。

### 手动事务

通过使用 `BEGIN` 和 `COMMIT`，您可以明确地定义事务的边界，因此在这种情况下无需担心自动提交。

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
