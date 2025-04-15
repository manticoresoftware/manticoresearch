# Обновление схемы таблицы

## Обновление схемы таблицы в режиме RT

<!-- пример ALTER -->

```sql
ALTER TABLE table ADD COLUMN column_name [{INTEGER|INT|BIGINT|FLOAT|BOOL|MULTI|MULTI64|JSON|STRING|TIMESTAMP|TEXT [INDEXED [ATTRIBUTE]]}] [engine='columnar']

ALTER TABLE table DROP COLUMN column_name

ALTER TABLE table MODIFY COLUMN column_name bigint
```

Эта функция поддерживает добавление только одного поля за раз для RT таблиц или расширение столбца `int` до `bigint`. Поддерживаемые типы данных:
* `int` - целочисленный атрибут
* `timestamp` - атрибут времени
* `bigint` - большой целочисленный атрибут
* `float` - атрибут с плавающей точкой
* `bool` - логический атрибут
* `multi` - многозначный целочисленный атрибут
* `multi64` - многозначный атрибут bigint
* `json` - атрибут json
* `string` / `text attribute` / `string attribute` - строковый атрибут
* `text` / `text indexed stored` / `string indexed stored` - текстовое поле с индексом, оригинальное значение хранится в docstore
* `text indexed` / `string indexed` - текстовое поле с индексом, индексируется только (оригинальное значение не хранится в docstore)
* `text indexed attribute` / `string indexed attribute` - текстовое поле с полным индексом + строковый атрибут (не хранящее оригинальное значение в docstore)
* `text stored` / `string stored` - значение будет храниться только в docstore, не проиндексировано по полному тексту, не строковый атрибут
* добавление `engine='columnar'` к любому атрибуту (за исключением json) сделает его хранимым в [колоночном хранилище](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)

#### Важные примечания:
* ❗Рекомендуется **создать резервную копию файлов таблицы** перед выполнением `ALTER`, чтобы избежать повреждения данных в случае неожиданного отключения питания или других подобных проблем.
* Запрос к таблице невозможен, пока добавляется столбец.
* Значения нового атрибута устанавливаются в 0.
* `ALTER` не будет работать для распределенных таблиц и таблиц без атрибутов.
* Нельзя удалить столбец `id`.
* При удалении поля, которое является одновременно полем с полным текстом и строковым атрибутом, первая команда `ALTER DROP` удаляет атрибут, вторая удаляет поле с полным текстом.
* Добавление/удаление поля с полным текстом поддерживается только в [режиме RT](Read_this_first.md#Real-time-mode-vs-plain-mode).

<!-- пример запроса -->
```sql

mysql> desc rt;
+------------+-----------+
| Field      | Type      |
+------------+-----------+
| id         | bigint    |
| text       | field     |
| group_id   | uint      |
| date_added | timestamp |
+------------+-----------+

mysql> alter table rt add column test integer;

mysql> desc rt;
+------------+-----------+
| Field      | Type      |
+------------+-----------+
| id         | bigint    |
| text       | field     |
| group_id   | uint      |
| date_added | timestamp |
| test       | uint      |
+------------+-----------+

mysql> alter table rt drop column group_id;

mysql> desc rt;
+------------+-----------+
| Field      | Type      |
+------------+-----------+
| id         | bigint    |
| text       | field     |
| date_added | timestamp |
| test       | uint      |
+------------+-----------+

mysql> alter table rt add column title text indexed;

mysql> desc rt;
+------------+-----------+------------+
| Field      | Type      | Properties |
+------------+-----------+------------+
| id         | bigint    |            |
| text       | text      | indexed    |
| title      | text      | indexed    |
| date_added | timestamp |            |
| test       | uint      |            |
+------------+-----------+------------+

mysql> alter table rt add column title text attribute;

mysql> desc rt;
+------------+-----------+------------+
| Field      | Type      | Properties |
+------------+-----------+------------+
| id         | bigint    |            |
| text       | text      | indexed    |
| title      | text      | indexed    |
| date_added | timestamp |            |
| test       | uint      |            |
| title      | string    |            |
+------------+-----------+------------+

mysql> alter table rt drop column title;

mysql> desc rt;
+------------+-----------+------------+
| Field      | Type      | Properties |
+------------+-----------+------------+
| id         | bigint    |            |
| text       | text      | indexed    |
| title      | text      | indexed    |
| date_added | timestamp |            |
| test       | uint      |            |
+------------+-----------+------------+
mysql> alter table rt drop column title;

mysql> desc rt;
+------------+-----------+------------+
| Field      | Type      | Properties |
+------------+-----------+------------+
| id         | bigint    |            |
| text       | text      | indexed    |
| date_added | timestamp |            |
| test       | uint      |            |
+------------+-----------+-----------+
```

<!-- end -->

## Обновление настроек FT таблицы в режиме RT

<!-- пример ALTER FT -->

```sql
ALTER TABLE table ft_setting='value'[, ft_setting2='value']
```

Вы можете использовать `ALTER`, чтобы изменить настройки полнотекстового поиска вашей таблицы в [режиме RT](Read_this_first.md#Real-time-mode-vs-plain-mode). Однако это затрагивает только новые документы, а не существующие.
Пример:
* создайте таблицу с полем полнотекстового поиска и `charset_table`, который позволяет только 3 искомых символа: `a`, `b` и `c`.
* затем мы вставляем документ 'abcd' и находим его по запросу `abcd`, буква `d` просто игнорируется, так как ее нет в массиве `charset_table`
* затем мы понимаем, что хотим, чтобы `d` также было доступно для поиска, поэтому добавляем его с помощью `ALTER`
* но тот же самый запрос `where match('abcd')` все еще говорит, что искал по `abc`, потому что существующий документ помнит предыдущие содержимое `charset_table`
* затем мы добавляем другой документ `abcd` и снова ищем по `abcd`
* теперь он находит оба документа, а `show meta` говорит, что использовал два ключевых слова: `abc` (для поиска старого документа) и `abcd` (для нового).

<!-- request Example -->
```sql
mysql> create table rt(title text) charset_table='a,b,c';

mysql> insert into rt(title) values('abcd');

mysql> select * from rt where match('abcd');
+---------------------+-------+
| id                  | title |
+---------------------+-------+
| 1514630637682688054 | abcd  |
+---------------------+-------+

mysql> show meta;
+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| total         | 1     |
| total_found   | 1     |
| time          | 0.000 |
| keyword[0]    | abc   |
| docs[0]       | 1     |
| hits[0]       | 1     |
+---------------+-------+

mysql> alter table rt charset_table='a,b,c,d';
mysql> select * from rt where match('abcd');
+---------------------+-------+
| id                  | title |
+---------------------+-------+
| 1514630637682688054 | abcd  |
+---------------------+-------+

mysql> show meta
+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| total         | 1     |
| total_found   | 1     |
| time          | 0.000 |
| keyword[0]    | abc   |
| docs[0]       | 1     |
| hits[0]       | 1     |
+---------------+-------+

mysql> insert into rt(title) values('abcd');
mysql> select * from rt where match('abcd');
+---------------------+-------+
| id                  | title |
+---------------------+-------+
| 1514630637682688055 | abcd  |
| 1514630637682688054 | abcd  |
+---------------------+-------+

mysql> show meta;
+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| total         | 2     |
| total_found   | 2     |
| time          | 0.000 |
| keyword[0]    | abc   |
| docs[0]       | 1     |
| hits[0]       | 1     |
| keyword[1]    | abcd  |
| docs[1]       | 1     |
| hits[1]       | 1     |
+---------------+-------+
```

<!-- end -->

## Переименование таблицы в реальном времени

<!-- example Renaming RT tables -->

Вы можете изменить имя таблицы в реальном времени в режиме RT.
```sql
ALTER TABLE table_name RENAME new_table_name;
```

> ПРИМЕЧАНИЕ: Переименование таблицы в реальном времени требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если это не сработает, убедитесь, что Buddy установлен.

<!-- request Example -->
```sql
ALTER TABLE table_name RENAME new_table_name;
```

<!-- response Example -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->

## Обновление настроек таблицы FT в обычном режиме

<!-- example ALTER RECONFIGURE -->
```sql
ALTER TABLE table RECONFIGURE
```

`ALTER` также может переконфигурировать таблицу RT в [обычном режиме](Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29), чтобы новые токенизации, морфология и другие параметры обработки текста из файла конфигурации действовали для новых документов. Обратите внимание, что существующий документ останется без изменений. Внутри он принудительно сохраняет текущий RAM блок как новый диск блок и корректирует заголовок таблицы, так что новые документы токенизируются с использованием обновленных полных текстовых настроек.

<!-- request Example -->
```sql
mysql> show table rt settings;
+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| settings      |       |
+---------------+-------+
1 row in set (0.00 sec)

mysql> alter table rt reconfigure;
Query OK, 0 rows affected (0.00 sec)

mysql> show table rt settings;
+---------------+----------------------+
| Variable_name | Value                |
+---------------+----------------------+
| settings      | morphology = stem_en |
+---------------+----------------------+
1 row in set (0.00 sec)
```
<!-- end -->

## Восстановление вторичного индекса

<!-- example ALTER REBUILD SECONDARY -->
```sql
ALTER TABLE table REBUILD SECONDARY
```

Вы также можете использовать `ALTER`, чтобы восстановить вторичные индексы в данной таблице. Иногда вторичный индекс может быть отключен для всей таблицы или для одного или нескольких атрибутов в таблице:
* Когда атрибут обновляется, его вторичный индекс отключается.
* Если Manticore загружает таблицу со старой версией вторичных индексов, которая больше не поддерживается, то вторичные индексы будут отключены для всей таблицы.

`ALTER TABLE table REBUILD SECONDARY` восстанавливает вторичные индексы из данных атрибута и снова включает их.

Кроме того, старая версия вторичных индексов может поддерживаться, но будет иметь определенные ограничения. `REBUILD SECONDARY` можно использовать для обновления вторичных индексов.

<!-- request Example -->
```sql
ALTER TABLE rt REBUILD SECONDARY;
```

<!-- response Example -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->

## Изменение распределенной таблицы

<!-- example local_dist -->

Чтобы изменить список локальных или удаленных узлов в распределенной таблице, следуйте тому же синтаксису, который вы использовали для [создания таблицы](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table). Просто замените `CREATE` на `ALTER` в команде и уберите `type='distributed'`:

```sql
ALTER TABLE `distr_table_name` [[local='local_table_name'], [agent='host:port:remote_table'] ... ]
```

> ПРИМЕЧАНИЕ: Изменение схемы распределенной таблицы в онлайн-режиме требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если это не сработает, убедитесь, что Buddy установлен.

<!-- request Example -->
```sql
ALTER TABLE local_dist local='index1' local='index2' agent='127.0.0.1:9312:remote_table';
```

<!-- end -->
<!-- proofread -->
