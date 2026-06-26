# Транзакции

Manticore поддерживает базовые транзакции для удаления и вставки данных в реальном времени и в percolate таблицы, за исключением случаев, когда попытка записи производится в распределенную таблицу, которая включает таблицу реального времени или percolate таблицу. Каждое изменение таблицы сначала сохраняется во внутреннем наборе изменений, а затем фактически фиксируется в таблице. По умолчанию каждая команда оборачивается в отдельную автоматическую транзакцию, делая это прозрачным: вы просто 'вставляете' что-то и можете видеть результат вставки после ее завершения, не беспокоясь о транзакциях. Однако это поведение можно явно контролировать, начиная и фиксируя транзакции вручную.

Транзакции поддерживаются для следующих команд:
* [INSERT](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md)
* [REPLACE](../Data_creation_and_modification/Updating_documents/REPLACE.md)
* [DELETE](../Data_creation_and_modification/Deleting_documents.md)

Транзакции не поддерживаются для:
* UPDATE (который [отличается](../Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md) от REPLACE, так как выполняет обновление атрибутов на месте)
* ALTER - для [обновления схемы таблицы](../Updating_table_schema_and_settings.md)
* TRUNCATE - для [очистки таблицы реального времени](../Emptying_a_table.md)
* ATTACH - для [присоединения обычной таблицы к таблице реального времени](../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md)
* CREATE - [для создания таблицы](../Creating_a_table/Local_tables.md)
* DROP - [для удаления таблицы](../Deleting_a_table.md)

Обратите внимание, что транзакции в Manticore не стремятся обеспечить изоляцию. Цель транзакций в Manticore - позволить вам накапливать несколько записей и выполнять их все сразу при фиксации или откатывать их все при необходимости. Транзакции интегрированы с [бинарным логом](../Logging/Binary_logging.md) для обеспечения долговечности и согласованности.

## Автоматический и ручной режим

```sql
SET AUTOCOMMIT = {0 | 1}
```

`SET AUTOCOMMIT` управляет режимом автозавершения в активной сессии. По умолчанию `AUTOCOMMIT` установлен в 1. При настройках по умолчанию вам не нужно беспокоиться о транзакциях, так как каждое выражение, вносящее изменения в любую таблицу, неявно оборачивается в отдельную транзакцию. Установка значения 0 позволяет управлять транзакциями вручную, что означает, что они не будут видны до тех пор, пока вы явно их не зафиксируете.

Транзакции ограничены одной таблицей реального времени или percolate таблицей, а также ограничены по размеру. Они атомарны, согласованы, чрезмерно изолированы и долговечны. Чрезмерно изолированы означает, что изменения невидимы не только для параллельных транзакций, но даже для самой текущей сессии.

## BEGIN, COMMIT и ROLLBACK

```sql
START TRANSACTION | BEGIN
COMMIT
ROLLBACK
```

Оператор `BEGIN` (или его алиас `START TRANSACTION`) принудительно фиксирует любую ожидающую транзакцию, если она есть, и начинает новую.

Оператор `COMMIT` фиксирует текущую транзакцию, делая все ее изменения постоянными.

Оператор `ROLLBACK` откатывает текущую транзакцию, отменяя все ее изменения.

## Транзакции в /bulk

При использовании одной из конечных точек JSON `/bulk` ( [пакетная вставка](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents), [пакетная замена](../Data_creation_and_modification/Updating_documents/REPLACE.md?client=JSON#Bulk-replace), [пакетное удаление](../Data_creation_and_modification/Deleting_documents.md?client=JSON#Bulk-deletion) ), вы можете принудительно зафиксировать пакет документов, добавив пустую строку после них.

## Примеры

### Автоматическое завершение (по умолчанию)

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

Вставленное значение сразу становится видимым в следующем операторе 'select'.

### Ручное завершение (autocommit=0)

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

В этом случае изменения НЕ фиксируются автоматически. В результате вставки не видны, даже в той же сессии, поскольку они не были зафиксированы. Также, несмотря на отсутствие оператора `BEGIN`, транзакция неявно начинается.

Чтобы изменения стали видимыми, необходимо зафиксировать транзакцию:

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

После оператора commit вставки становятся видимыми в таблице.

### Ручная транзакция

Используя `BEGIN` и `COMMIT`, вы можете явно определить границы транзакции, поэтому в этом случае не нужно беспокоиться об автозавершении.

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

