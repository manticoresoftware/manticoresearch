# Транзакции

Manticore поддерживает основные транзакции для удаления и вставки данных в таблицы реального времени и перколяции, за исключением случаев, когда пытаются записать в распределенную таблицу, которая включает в себя таблицу реального времени или перколяции. Каждое изменение в таблице сначала сохраняется во внутреннем наборе изменений, а затем фактически фиксируется в таблице. По умолчанию каждая команда обернута в отдельную автоматическую транзакцию, что делает это прозрачным: вы просто 'вставляете' что-то и можете увидеть вставленный результат после его завершения, не беспокоясь о транзакциях. Однако это поведение может быть явно управляемым, начиная и фиксируя транзакции вручную.

Транзакции поддерживаются для следующих команд:
* [ВСТАВИТЬ](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md)
* [ЗАМЕНИТЬ](../Data_creation_and_modification/Updating_documents/REPLACE.md)
* [УДАЛИТЬ](../Data_creation_and_modification/Deleting_documents.md)

Транзакции не поддерживаются для:
* UPDATE (который [отличается](../Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md) от ЗАМЕНИТЬ, так как выполняет обновление атрибута на месте)
* ALTER - для [обновления схемы таблицы](../Updating_table_schema_and_settings.md)
* TRUNCATE - для [опустошения таблицы реального времени](../Emptying_a_table.md)
* ATTACH - для [присоединения простой таблицы к таблице реального времени](../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md)
* CREATE - [для создания таблицы](../Creating_a_table/Local_tables.md)
* DROP - [для удаления таблицы](../Deleting_a_table.md)

Пожалуйста, обратите внимание, что транзакции в Manticore не предназначены для обеспечения изоляции. Цель транзакций в Manticore - позволить вам накапливать множество записей и выполнять их все одновременно при фиксации, или отменить их все, если это необходимо. Транзакции интегрированы с [двойным журналом](../Logging/Binary_logging.md) для долговечности и согласованности.

## Автоматический и ручной режим

```sql
SET AUTOCOMMIT = {0 | 1}
```

`SET AUTOCOMMIT` управляет режимом автоматической фиксации в активной сессии. По умолчанию `AUTOCOMMIT` установлен в 1. С настройкой по умолчанию вам не нужно беспокоиться о транзакциях, так как каждое утверждение, которое вносит изменения в любую таблицу, неявно обернуто в отдельную транзакцию. Установка его в 0 позволяет вам управлять транзакциями вручную, что означает, что они не будут видны, пока вы их явно не зафиксируете.

Транзакции ограничены одной таблицей реального времени или перколяции и также ограничены по размеру. Они атомарны, согласованны, чрезмерно изолированы и устойчивы. Чрезмерная изоляция означает, что изменения не только невидимы для конкурентных транзакций, но даже для самой текущей сессии.

## BEGIN, COMMIT и ROLLBACK

```sql
START TRANSACTION | BEGIN
COMMIT
ROLLBACK
```

Утверждение `BEGIN` (или его псевдоним `START TRANSACTION`) принудительно фиксирует любую ожидающую транзакцию, если она есть, и начинает новую.

Утверждение `COMMIT` фиксирует текущую транзакцию, делая все ее изменения постоянными.

Утверждение `ROLLBACK` отменяет текущую транзакцию, отменяя все ее изменения.

## Транзакции в /bulk

При использовании одного из JSON эндпоинтов `/bulk` ( [массовая вставка](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents), [массовая замена](../Data_creation_and_modification/Updating_documents/REPLACE.md?client=JSON#Bulk-replace), [массовое удаление](../Data_creation_and_modification/Deleting_documents.md?client=JSON#Bulk-deletion) ), вы можете заставить пакет документов быть зафиксированным, добавив пустую строку после них.

## Примеры

### Автоматические фиксации (по умолчанию)

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

Вставленное значение немедленно видно в следующем операторе 'select'.

### Ручные фиксации (autocommit=0)

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

В этом случае изменения НЕ фиксируются автоматически. В результате вставки не видны, даже в той же сессии, поскольку они не были зафиксированы. Также, несмотря на отсутствие заявления `BEGIN`, транзакция неявно начинает.

Чтобы сделать изменения видимыми, вам необходимо зафиксировать транзакцию:

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

После команды фиксации вставления видны в таблице.

### Ручная транзакция

Используя `BEGIN` и `COMMIT`, вы можете явно определить границы транзакции, поэтому в этом случае не нужно беспокоиться об автозаписи.

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
