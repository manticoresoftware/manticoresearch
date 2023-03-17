# Transactions

Manticore supports basic transactions for deleting and inserting data into real-time and percolate tables. Each change to a table is first saved in an internal changeset and then actually committed to the table. By default, each command is wrapped in an individual automatic transaction, making it transparent: you simply 'insert' something and can see the inserted result after it completes, without worrying about transactions. However, this behavior can be explicitly managed by starting and committing transactions manually.

Transactions are supported for the following commands:
* [INSERT](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md)
* [REPLACE](../Data_creation_and_modification/Updating_documents/REPLACE.md)
* [DELETE](../Data_creation_and_modification/Deleting_documents.md)

Transactions are not supported for:
* UPDATE (which is [different](../Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md) from REPLACE, as it performs an in-place attribute update)
* ALTER - for [updating table schema](../Updating_table_schema_and_settings.md)
* TRUNCATE - for [emptying a real-time table](../Emptying_a_table.md)
* ATTACH - for [attaching a plain table to a real-time table](../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_a_plain_table_to_RT_table.md)
* CREATE - [for creating a table](../Creating_a_table/Local_tables.md)
* DROP - [for deleting a table](../Deleting_a_table.md)

Please note that transactions in Manticore do not aim to provide isolation. The purpose of transactions in Manticore is to allow you to accumulate multiple writes and execute them all at once upon commit, or to roll them all back if necessary. Transactions are integrated with [binary log](../Logging/Binary_logging.md) for durability and consistency.

## Automatic and manual mode

```sql
SET AUTOCOMMIT = {0 | 1}
```

`SET AUTOCOMMIT` controls the autocommit mode in the active session. `AUTOCOMMIT` is set to 1 by default. With the default setting, you don't have to worry about transactions, as every statement that makes any changes to any table is implicitly wrapped in a separate transaction. Setting it to 0 allows you to manage transactions manually, meaning they will not be visible until you explicitly commit them.

Transactions are limited to a single real-time or percolate table and are also limited in size. They are atomic, consistent, overly isolated, and durable. Overly isolated means that the changes are not only invisible to concurrent transactions but even to the current session itself.

## BEGIN, COMMIT, and ROLLBACK

```sql
START TRANSACTION | BEGIN
COMMIT
ROLLBACK
```

The `BEGIN` statement (or its `START TRANSACTION` alias) forcibly commits any pending transaction, if present, and starts a new one.

The `COMMIT` statement commits the current transaction, making all its changes permanent.

The `ROLLBACK` statement rolls back the current transaction, canceling all its changes.

## Transactions in /bulk

When using the [/bulk JSON endpoint](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents), you can force a batch of documents to be committed by adding an empty line after them.

## Examples

### Automatic commits (default)

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

The inserted value is immediately visible in the following 'select' statement.

### Manual commits (autocommit=0)

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

In this case, changes are NOT automatically committed. As a result, the insertions are not visible, even in the same session, since they have not been committed. Also, despite the absence of a `BEGIN` statement, a transaction is implicitly started.

So, let's finally commit it:

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

Now it is finished and visible.

### Manual transaction

By using `BEGIN` and `COMMIT`, you can define the bounds of a transaction explicitly, so there's no need to worry about autocommit in this case.

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
