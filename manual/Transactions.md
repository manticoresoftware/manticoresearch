# Transactions

Manticore supports basic transactions when performing deleting and insertion into real-time and percolate tables. That is: each change to a table is first saved into an internal changeset, and then is actually committed to the table. By default each command is wrapped into an individual automatic transaction, making it transparent: you just 'insert' something, and can see the inserted result after it completes, having no care about transactions. However that behaviour can be explicitly managed by starting and committing transactions manually.

Transactions are supported for the following commands:
* [INSERT](Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md)
* [REPLACE](Updating_documents/REPLACE.md)
* [DELETE](Deleting_documents.md)

Transactions are not supported for:
* UPDATE (which is [different](Updating_documents/REPLACE_vs_UPDATE.md) from the REPLACE in that it does in-place attribute update).
* ALTER - for [updating table schema](Updating_table_schema_and_settings.md)
* TRUNCATE - for [emptying a real-time table](Emptying_a_table.md)
* ATTACH - for [attaching a plain table to a real-time table](Adding_data_from_external_storages/Adding_data_to_tables/Attaching_a_plain_table_to_RT_table.md)
* CREATE - [for creating a table](Creating_a_table/Local_tables.md)
* DROP - [for deleting a table](Deleting_a_table.md)

Please also note, that transactions in Manticore do not aim to provide isolation. The idea of transactions in Manticore is to let you accumulate a number of writes and run them at once at commit or rollback them all if needed. Transactions are integrated with [binary log](Logging/Binary_logging.md) which gives durability and consistency.

## Automatic and manual mode

```sql
SET AUTOCOMMIT = {0 | 1}
```

`SET AUTOCOMMIT` controls the autocommit mode in the active session. `AUTOCOMMIT` is set to 1 by default. With default you have not to care about transactions, since every statement that performs any changes on any table is implicitly wrapped into separate transaction. Setting it to 0 allows you to manage transactions manually. I.e., they will not be visible until you explicitly commit them.

Transactions are limited to a single RT or percolate table, and also limited in size. They are atomic, consistent, overly isolated, and durable. Overly isolated means that the changes are not only invisible to the concurrent transactions but even to the current session itself.


## BEGIN, COMMIT, and ROLLBACK

```sql
START TRANSACTION | BEGIN
COMMIT
ROLLBACK
```

`BEGIN` statement (or its `START TRANSACTION` alias) forcibly commits pending transaction, if any, and begins a new one.

`COMMIT` statement commits the current transaction, making all its changes permanent.

`ROLLBACK` statement rolls back the current transaction, canceling all its changes.

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

Inserted value immediately visible by following 'select' statement.

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

Here changes is NOT automatically committed. So, insertion is not visible even in the same session, since they're not committed. Also, despite absent `BEGIN` statement, transaction is implicitly started.

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

Using `BEGIN` and `COMMIT` you can define bounds of transaction explicitly, so no need to care about autocommit in this case.

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

## SET TRANSACTION

```sql
SET TRANSACTION ISOLATION LEVEL { READ UNCOMMITTED
    | READ COMMITTED
    | REPEATABLE READ
    | SERIALIZABLE }
```

`SET TRANSACTION` statement does nothing. It was implemented to maintain compatibility with 3rd party MySQL client libraries, connectors, and frameworks that may need to run this statement when connecting. They just goes across syntax parser and then returns 'ok'. Nothing usable for your own programs, just a stubs to make third-party clients happy.

```sql
mysql> SET TRANSACTION ISOLATION LEVEL READ UNCOMMITTED;
Query OK, 0 rows affected (0.00 sec)
```
