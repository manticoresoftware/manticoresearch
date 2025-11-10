# Транзакции

Manticore поддерживает базовые транзакции для удаления и вставки данных в таблицы реального времени и перколяции, за исключением попыток записи в распределённую таблицу, которая включает таблицу реального времени или перколяции. Каждое изменение таблицы сначала сохраняется во внутреннем наборе изменений, а затем фактически фиксируется в таблице. По умолчанию каждая команда оборачивается в отдельную автоматическую транзакцию, что делает процесс прозрачным: вы просто 'вставляете' что-то и видите вставленный результат после завершения, не беспокоясь о транзакциях. Однако это поведение можно явно управлять, начиная и фиксируя транзакции вручную.

Транзакции поддерживаются для следующих команд:
* [INSERT](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md)
* [REPLACE](../Data_creation_and_modification/Updating_documents/REPLACE.md)
* [DELETE](../Data_creation_and_modification/Deleting_documents.md)

Транзакции не поддерживаются для:
* UPDATE (который [отличается](../Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md) от REPLACE, так как выполняет обновление атрибутов на месте)
* ALTER - для [обновления схемы таблицы](../Updating_table_schema_and_settings.md)
* TRUNCATE - для [очистки таблицы реального времени](../Emptying_a_table.md)
* ATTACH - для [присоединения простой таблицы к таблице реального времени](../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md)
* CREATE - [для создания таблицы](../Creating_a_table/Local_tables.md)
* DROP - [для удаления таблицы](../Deleting_a_table.md)

Обратите внимание, что транзакции в Manticore не предназначены для обеспечения изоляции. Цель транзакций в Manticore — позволить вам накопить несколько записей и выполнить их все одновременно при фиксации, или отменить их все при необходимости. Транзакции интегрированы с [бинарным журналом](../Logging/Binary_logging.md) для обеспечения надёжности и согласованности.

## Автоматический и ручной режим

```sql
SET AUTOCOMMIT = {0 | 1}
```

`SET AUTOCOMMIT` управляет режимом автокоммита в активной сессии. По умолчанию `AUTOCOMMIT` установлен в 1. При настройке по умолчанию вам не нужно беспокоиться о транзакциях, так как каждое выражение, вносящее изменения в любую таблицу, неявно оборачивается в отдельную транзакцию. Установка значения 0 позволяет управлять транзакциями вручную, то есть они не будут видны, пока вы явно не зафиксируете их.

Транзакции ограничены одной таблицей реального времени или перколяции и также ограничены по размеру. Они атомарны, согласованы, чрезмерно изолированы и надёжны. Чрезмерная изоляция означает, что изменения не видны не только для параллельных транзакций, но даже для самой текущей сессии.

## BEGIN, COMMIT и ROLLBACK

```sql
START TRANSACTION | BEGIN
COMMIT
ROLLBACK
```

Оператор `BEGIN` (или его псевдоним `START TRANSACTION`) принудительно фиксирует любую ожидающую транзакцию, если она есть, и начинает новую.

Оператор `COMMIT` фиксирует текущую транзакцию, делая все её изменения постоянными.

Оператор `ROLLBACK` откатывает текущую транзакцию, отменяя все её изменения.

## Транзакции в /bulk

При использовании одного из JSON эндпоинтов `/bulk` ( [bulk insert](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents), [bulk replace](../Data_creation_and_modification/Updating_documents/REPLACE.md?client=JSON#Bulk-replace), [bulk delete](../Data_creation_and_modification/Deleting_documents.md?client=JSON#Bulk-deletion) ), вы можете принудительно зафиксировать пакет документов, добавив пустую строку после них.

## Примеры

### Автоматические коммиты (по умолчанию)

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

Вставленное значение сразу же видно в следующем операторе 'select'.

### Ручные коммиты (autocommit=0)

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

В этом случае изменения НЕ фиксируются автоматически. В результате вставки не видны, даже в той же сессии, так как они не были зафиксированы. Также, несмотря на отсутствие оператора `BEGIN`, транзакция запускается неявно.

Чтобы сделать изменения видимыми, необходимо зафиксировать транзакцию:

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

Используя `BEGIN` и `COMMIT`, вы можете явно определить границы транзакции, поэтому в этом случае не нужно беспокоиться об автокоммите.

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

