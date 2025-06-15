# Обновление схемы таблицы

## Обновление схемы таблицы в режиме RT

<!-- example ALTER -->

```sql
ALTER TABLE table ADD COLUMN column_name [{INTEGER|INT|BIGINT|FLOAT|BOOL|MULTI|MULTI64|JSON|STRING|TIMESTAMP|TEXT [INDEXED [ATTRIBUTE]]}] [engine='columnar']

ALTER TABLE table DROP COLUMN column_name

ALTER TABLE table MODIFY COLUMN column_name bigint
```

Эта функция поддерживает только добавление одного поля за раз для RT таблиц или расширение столбца `int` до `bigint`. Поддерживаемые типы данных:
* `int` - целочисленный атрибут
* `timestamp` - атрибут временной метки
* `bigint` - атрибут большого целого числа
* `float` - атрибут с плавающей точкой
* `bool` - булев атрибут
* `multi` - мультизначный целочисленный атрибут
* `multi64` - мультизначный атрибут bigint
* `json` - json атрибут
* `string` / `text attribute` / `string attribute` - строковый атрибут
* `text` / `text indexed stored` / `string indexed stored` - полнотекстовое индексированное поле с оригинальным значением, хранящимся в docstore
* `text indexed` / `string indexed` - полнотекстовое индексированное поле, только индексируется (оригинальное значение в docstore не хранится)
* `text indexed attribute` / `string indexed attribute` - полнотекстовое индексированное поле + строковый атрибут (оригинальное значение в docstore не хранится)
* `text stored` / `string stored` - значение хранится только в docstore, не полнотекстово индексировано, не является строковым атрибутом
* добавление `engine='columnar'` к любому атрибуту (кроме json) приведет к его хранению в [колоночном хранилище](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)

#### Важные примечания:
* ❗Рекомендуется **создавать резервные копии файлов таблиц** перед выполнением `ALTER`, чтобы избежать повреждения данных при внезапном отключении питания или других подобных проблемах.
* Запросы к таблице невозможны во время добавления столбца.
* Значения новосозданного атрибута устанавливаются в 0.
* `ALTER` не работает для распределенных таблиц и таблиц без атрибутов.
* Нельзя удалять столбец `id`.
* При удалении поля, которое одновременно является полнотекстовым полем и строковым атрибутом, первый `ALTER DROP` удаляет атрибут, второй удаляет полнотекстовое поле.
* Добавление/удаление полнотекстовых полей поддерживается только в [RT режиме](Read_this_first.md#Real-time-mode-vs-plain-mode).

<!-- request Example -->
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
+------------+-----------+------------+
```

<!-- end -->

## Обновление настроек полнотекстового поиска таблицы в режиме RT

<!-- example ALTER FT -->

```sql
ALTER TABLE table ft_setting='value'[, ft_setting2='value']
```

Можно использовать `ALTER` для изменения настроек полнотекстового поиска вашей таблицы в [RT режиме](Read_this_first.md#Real-time-mode-vs-plain-mode). Однако это влияет только на новые документы, существующие останутся без изменений.
Пример:
* создайте таблицу с полнотекстовым полем и `charset_table`, разрешающим только 3 символа для поиска: `a`, `b` и `c`.
* затем вставляем документ 'abcd' и ищем его запросом `abcd`, символ `d` игнорируется, поскольку его нет в массиве `charset_table`
* потом понимаем, что хотим, чтобы `d` также искался, и добавляем его с помощью `ALTER`
* но тот же запрос `where match('abcd')` все еще сообщает, что поиск выполнен по `abc`, так как существующий документ помнит предыдущие настройки `charset_table`
* затем добавляем другой документ `abcd` и снова ищем по `abcd`
* теперь он находит оба документа, и `show meta` сообщает, что использованы два ключевых слова: `abc` (для поиска старого документа) и `abcd` (для нового).

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

## Переименование таблицы в режиме реального времени

<!-- example Renaming RT tables -->

Вы можете изменить имя таблицы в режиме RT.
```sql
ALTER TABLE table_name RENAME new_table_name;
```

> ПРИМЕЧАНИЕ: Для переименования таблицы в режиме реального времени требуется [Manticore Buddy](Installation/Manticore_Buddy.md). Если не работает, убедитесь, что Buddy установлен.

<!-- request Example -->
```sql
ALTER TABLE table_name RENAME new_table_name;
```

<!-- response Example -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->

## Обновление настроек полнотекстового поиска в plain режиме

<!-- example ALTER RECONFIGURE -->
```sql
ALTER TABLE table RECONFIGURE
```

`ALTER` также может переконфигурировать RT таблицу в [plain режиме](Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29), чтобы новые токенизация, морфология и другие настройки обработки текста из конфигурационного файла применялись к новым документам. Обратите внимание, что существующие документы остаются без изменений. Внутренне это принудительно сохраняет текущий RAM chunk как новый диск-чанк и корректирует заголовок таблицы, чтобы новые документы индексировались с учетом обновленных настроек полнотекстового поиска.

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

## Перестройка вторичного индекса

<!-- example ALTER REBUILD SECONDARY -->
```sql
ALTER TABLE table REBUILD SECONDARY
```

Вы также можете использовать `ALTER` для перестройки вторичных индексов в заданной таблице. Иногда вторичный индекс может быть отключен для всей таблицы или для одного или нескольких атрибутов в таблице:
* При обновлении атрибута его вторичный индекс отключается.
* Если Manticore загружает таблицу с устаревшей версией вторичных индексов, которая больше не поддерживается, вторичные индексы будут отключены для всей таблицы.

`ALTER TABLE table REBUILD SECONDARY` перестраивает вторичные индексы по данным атрибутов и заново их включает.

Кроме того, устаревшая версия вторичных индексов может поддерживаться, но будет лишена некоторых функций. `REBUILD SECONDARY` можно использовать для обновления вторичных индексов.

<!-- request Example -->
```sql
ALTER TABLE rt REBUILD SECONDARY;
```

<!-- response Example -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->


## Перестройка KNN индекса

<!-- example ALTER REBUILD KNN -->
```sql
ALTER TABLE table REBUILD KNN
```

Команда повторно обрабатывает все векторные данные таблицы и перестраивает KNN индекс с нуля.

<!-- request Example -->
```sql
ALTER TABLE rt REBUILD KNN;
```

<!-- response Example -->
```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->

## Изменение распределенной таблицы

<!-- example local_dist -->

Чтобы изменить список локальных или удаленных узлов в распределенной таблице, используйте тот же синтаксис, что вы использовали при [создании таблицы](Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table). Просто замените `CREATE` на `ALTER` в команде и удалите `type='distributed'`:

```sql
ALTER TABLE `distr_table_name` [[local='local_table_name'], [agent='host:port:remote_table'] ... ]
```

> ПРИМЕЧАНИЕ: Изменение схемы распределенной таблицы онлайн требует [Manticore Buddy](Installation/Manticore_Buddy.md). Если не работает, убедитесь, что Buddy установлен.

<!-- request Example -->
```sql
ALTER TABLE local_dist local='index1' local='index2' agent='127.0.0.1:9312:remote_table';
```

<!-- end -->
<!-- proofread -->

