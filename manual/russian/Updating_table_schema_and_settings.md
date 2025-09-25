# Обновление схемы таблицы

## Обновление схемы таблицы в RT режиме

<!-- example ALTER -->

```sql
ALTER TABLE table ADD COLUMN column_name [{INTEGER|INT|BIGINT|FLOAT|BOOL|MULTI|MULTI64|JSON|STRING|TIMESTAMP|TEXT [INDEXED [ATTRIBUTE]]}] [engine='columnar']

ALTER TABLE table DROP COLUMN column_name

ALTER TABLE table MODIFY COLUMN column_name bigint
```

Эта функция поддерживает только добавление одного поля за раз для RT таблиц или расширение столбца `int` до `bigint`. Поддерживаемые типы данных:
* `int` - целочисленный атрибут
* `timestamp` - атрибут временной метки
* `bigint` - большой целочисленный атрибут
* `float` - атрибут с плавающей точкой
* `bool` - булев атрибут
* `multi` - многозначный целочисленный атрибут
* `multi64` - многозначный большой целочисленный атрибут
* `json` - json атрибут
* `string` / `text attribute` / `string attribute` - строковый атрибут
* `text` / `text indexed stored` / `string indexed stored` - полнотекстовое индексированное поле с оригинальным значением, сохраненным в docstore
* `text indexed` / `string indexed` - полнотекстовое индексированное поле, только индекс (оригинальное значение не сохраняется в docstore)
* `text indexed attribute` / `string indexed attribute` - полнотекстовое индексированное поле + строковый атрибут (без сохранения оригинального значения в docstore)
* `text stored` / `string stored` - значение будет только сохранено в docstore, не полнотекстово индексировано, не является строковым атрибутом
* добавление `engine='columnar'` к любому атрибуту (кроме json) приведёт к его хранению в [колоночном хранилище](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)

#### Важные замечания:
* ❗Рекомендуется **создавать резервную копию файлов таблицы** перед выполнением `ALTER`, чтобы избежать повреждения данных в случае внезапного отключения питания или других подобных проблем.
* Запросы к таблице невозможны во время добавления столбца.
* Значения вновь созданного атрибута устанавливаются в 0.
* `ALTER` не работает для распределённых таблиц и таблиц без каких-либо атрибутов.
* Нельзя удалить столбец `id`.
* При удалении поля, являющегося одновременно полнотекстовым полем и строковым атрибутом, первый `ALTER DROP` удаляет атрибут, второй — полнотекстовое поле.
* Добавление/удаление полнотекстового поля поддерживается только в [RT режиме](Read_this_first.md#Real-time-mode-vs-plain-mode).

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

## Обновление настроек полнотекстового поиска в RT режиме

<!-- example ALTER FT -->

```sql
ALTER TABLE table ft_setting='value'[, ft_setting2='value']
```

Вы можете использовать `ALTER` для изменения настроек полнотекстового поиска вашей таблицы в [RT режиме](Read_this_first.md#Real-time-mode-vs-plain-mode). Однако это влияет только на новые документы, а не на существующие.
Пример:
* создаём таблицу с полнотекстовым полем и `charset_table`, позволяющей только 3 ищущихся символа: `a`, `b` и `c`.
* затем вставляем документ 'abcd' и ищем его запросом `abcd`, символ `d` игнорируется, так как он отсутствует в массиве `charset_table`
* затем понимаем, что хотим, чтобы `d` также индексировался, и добавляем его с помощью `ALTER`
* но тот же запрос `where match('abcd')` по-прежнему ищет только `abc`, потому что существующий документ запоминает прежнее содержимое `charset_table`
* затем добавляем ещё один документ `abcd` и снова ищем `abcd`
* теперь находятся оба документа, а команда `show meta` показывает, что использовались два ключевых слова: `abc` (для старого документа) и `abcd` (для нового).

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

Вы можете изменить имя таблицы в RT режиме.
```sql
ALTER TABLE table_name RENAME new_table_name;
```

> ПРИМЕЧАНИЕ: Переименование таблицы в реальном времени требует [Manticore Buddy](Installation/Manticore_Buddy.md). Если не работает, убедитесь, что Buddy установлен.

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

`ALTER` также может перенастроить RT таблицу в [plain режиме](Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29), чтобы новые настройки токенизации, морфологии и другой обработки текста из конфигурационного файла применялись к новым документам. Обратите внимание, что существующие документы останутся без изменений. Внутренне это принудительно сохраняет текущий сегмент RAM как новый сегмент на диске и корректирует заголовок таблицы, чтобы новые документы обрабатывались с обновлёнными полнотекстовыми настройками.

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

## Перестроение вторичного индекса

<!-- example ALTER REBUILD SECONDARY -->
```sql
ALTER TABLE table REBUILD SECONDARY
```

Вы также можете использовать `ALTER` для перестроения вторичных индексов в указанной таблице. Иногда вторичный индекс может быть отключён для всей таблицы или для одного или нескольких атрибутов внутри таблицы:
* Когда обновляется атрибут, его вторичный индекс отключается.
* Если Manticore загружает таблицу со старой версией вторичных индексов, которая больше не поддерживается, вторичные индексы будут отключены для всей таблицы.

`ALTER TABLE table REBUILD SECONDARY` перестраивает вторичные индексы на основе данных атрибутов и снова включает их.

Кроме того, старая версия вторичных индексов может поддерживаться, но не иметь некоторых функций. Команда `REBUILD SECONDARY` позволяет обновить вторичные индексы.

<!-- request Example -->
```sql
ALTER TABLE rt REBUILD SECONDARY;
```

<!-- response Example -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->


## Перестроение KNN индекса

<!-- example ALTER REBUILD KNN -->
```sql
ALTER TABLE table REBUILD KNN
```

Эта команда повторно обрабатывает все векторные данные в таблице и заново строит KNN индекс с нуля.

<!-- request Example -->
```sql
ALTER TABLE rt REBUILD KNN;
```

<!-- response Example -->
```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->

## Обновление ключа API атрибута (для генерации встраиваний) в RT режиме

<!-- example api_key -->

`ALTER` можно использовать для изменения API ключа при использовании удалённой модели для авто-встраиваний:

```sql
ALTER TABLE table_name MODIFY COLUMN column_name API_KEY='key';
```

<!-- request Example -->
```sql
ALTER TABLE rt MODIFY COLUMN vector API_KEY='key';
```

<!-- end -->

## Изменение распределённой таблицы

<!-- example local_dist -->

Для изменения списка локальных или удалённых узлов в распределённой таблице используйте тот же синтаксис, который применяли для [создания таблицы](Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table). Просто замените `CREATE` на `ALTER` в команде и удалите `type='distributed'`:

```sql
ALTER TABLE `distr_table_name` [[local='local_table_name'], [agent='host:port:remote_table'] ... ]
```

> ПРИМЕЧАНИЕ: Онлайн изменение схемы распределённой таблицы требует [Manticore Buddy](Installation/Manticore_Buddy.md). Если не работает, убедитесь, что Buddy установлен.

<!-- request Example -->
```sql
ALTER TABLE local_dist local='index1' local='index2' agent='127.0.0.1:9312:remote_table';
```

<!-- end -->
<!-- proofread -->

