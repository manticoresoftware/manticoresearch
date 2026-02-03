# Обновление схемы таблицы

## Обновление схемы таблицы в режиме RT

<!-- example ALTER -->

```sql
ALTER TABLE table ADD COLUMN column_name [{INTEGER|INT|BIGINT|FLOAT|BOOL|MULTI|MULTI64|JSON [secondary_index='1']|STRING|TEXT [INDEXED [ATTRIBUTE]]|TIMESTAMP}] [engine='columnar']

ALTER TABLE table DROP COLUMN column_name

ALTER TABLE table MODIFY COLUMN column_name bigint
```

Эта функция поддерживает добавление только одного поля за раз для RT-таблиц или расширение столбца `int` до `bigint`. Поддерживаемые типы данных:
* `int` - целочисленный атрибут
* `timestamp` - атрибут временной метки
* `bigint` - атрибут большого целого числа
* `float` - атрибут с плавающей запятой
* `bool` - булев атрибут
* `multi` - многозначный целочисленный атрибут
* `multi64` - многозначный атрибут bigint
* `json` - атрибут json; используйте `secondary_index='1'` для создания вторичного индекса по JSON (см. примечание ниже)
* `string` / `text attribute` / `string attribute` - строковый атрибут
* `text` / `text indexed stored` / `string indexed stored` - полнотекстовое индексируемое поле с исходным значением, хранящимся в docstore
* `text indexed` / `string indexed` - полнотекстовое индексируемое поле, только индексируемое (исходное значение не хранится в docstore)
* `text indexed attribute` / `string indexed attribute` - полнотекстовое индексируемое поле + строковый атрибут (исходное значение не хранится в docstore)
* `text stored` / `string stored` - значение будет храниться только в docstore, не полнотекстово индексироваться и не является строковым атрибутом
* добавление `engine='columnar'` к любому атрибуту (кроме json) приведет к его хранению в [колоночном хранилище](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)

#### Важные замечания:
* ❗Рекомендуется **создать резервную копию файлов таблицы** перед выполнением `ALTER`, чтобы избежать повреждения данных в случае внезапного отключения питания или других подобных проблем.
* Запросы к таблице невозможны, пока добавляется столбец.
* Значения вновь созданного атрибута устанавливаются в 0.
* `ALTER` не будет работать для распределенных таблиц и таблиц без каких-либо атрибутов.
* Нельзя удалить столбец `id`.
* При удалении поля, которое одновременно является полнотекстовым полем и строковым атрибутом, первый `ALTER DROP` удаляет атрибут, второй — полнотекстовое поле.
* Добавление/удаление полнотекстового поля поддерживается только в [режиме RT](Read_this_first.md#Real-time-mode-vs-plain-mode).

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

## Обновление FT-настроек таблицы в режиме RT

<!-- example ALTER FT -->

```sql
ALTER TABLE table ft_setting='value'[, ft_setting2='value']
```

Вы можете использовать `ALTER` для изменения полнотекстовых настроек вашей таблицы в [режиме RT](Read_this_first.md#Real-time-mode-vs-plain-mode). Однако это влияет только на новые документы, а не на существующие.
Пример:
* создаем таблицу с полнотекстовым полем и `charset_table`, которая позволяет только 3 поисковых символа: `a`, `b` и `c`.
* затем мы вставляем документ 'abcd' и находим его по запросу `abcd`, `d` просто игнорируется, так как его нет в массиве `charset_table`
* затем мы понимаем, что хотим, чтобы `d` тоже был доступен для поиска, поэтому добавляем его с помощью `ALTER`
* но тот же запрос `where match('abcd')` все равно говорит, что поиск был по `abc`, потому что существующий документ помнит предыдущее содержимое `charset_table`
* затем мы добавляем еще один документ `abcd` и снова ищем по `abcd`
* теперь он находит оба документа, и `show meta` говорит, что использовалось два ключевых слова: `abc` (для поиска старого документа) и `abcd` (для нового).

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

## Переименование реальной таблицы

<!-- example Renaming RT tables -->

Вы можете изменить имя реальной таблицы в режиме RT.
```sql
ALTER TABLE table_name RENAME new_table_name;
```

> ПРИМЕЧАНИЕ: Переименование реальной таблицы требует наличия [Manticore Buddy](Installation/Manticore_Buddy.md). Если это не работает, убедитесь, что Buddy установлен.

<!-- request Example -->
```sql
ALTER TABLE table_name RENAME new_table_name;
```

<!-- response Example -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->

## Обновление FT-настроек таблицы в обычном режиме

<!-- example ALTER RECONFIGURE -->
```sql
ALTER TABLE table RECONFIGURE
```

`ALTER` также может перенастроить RT-таблицу в [обычном режиме](Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29), чтобы новые настройки токенизации, морфологии и другие настройки обработки текста из файла конфигурации вступили в силу для новых документов. Обратите внимание, что существующие документы останутся нетронутыми. Внутренне он принудительно сохраняет текущий RAM-чанк как новый дисковый чанк и корректирует заголовок таблицы, чтобы новые документы токенизировались с использованием обновленных полнотекстовых настроек.

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

Вы также можете использовать `ALTER` для перестроения вторичных индексов в заданной таблице. Иногда вторичный индекс может быть отключен для всей таблицы или для одного или нескольких атрибутов внутри таблицы:
* Когда атрибут обновляется, его вторичный индекс отключается.
* Если Manticore загружает таблицу со старой версией вторичных индексов, которая больше не поддерживается, вторичные индексы будут отключены для всей таблицы.

`ALTER TABLE table REBUILD SECONDARY` перестраивает вторичные индексы из данных атрибутов и снова включает их.

Кроме того, старая версия вторичных индексов может поддерживаться, но будет лишена определенных функций. `REBUILD SECONDARY` можно использовать для обновления вторичных индексов.

<!-- request Example -->
```sql
ALTER TABLE rt REBUILD SECONDARY;
```

<!-- response Example -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->


## Перестроение KNN-индекса

<!-- example ALTER REBUILD KNN -->
```sql
ALTER TABLE table REBUILD KNN
```

Команда перерабатывает все векторные данные в таблице и перестраивает KNN-индекс с нуля.

<!-- request Example -->
```sql
ALTER TABLE rt REBUILD KNN;
```

<!-- response Example -->
```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->

## Обновление ключа API атрибута (для генерации эмбеддингов) в режиме RT

<!-- example api_key -->

`ALTER` можно использовать для изменения ключа API, когда удаленная модель используется для автоэмбеддингов:

```sql
ALTER TABLE table_name MODIFY COLUMN column_name API_KEY='key';
```

<!-- request Example -->
```sql
ALTER TABLE rt MODIFY COLUMN vector API_KEY='key';
```

<!-- end -->

## Изменение распределенной таблицы

<!-- example local_dist -->

Чтобы изменить список локальных или удаленных узлов в распределенной таблице, используйте тот же синтаксис, который вы использовали для [создания таблицы](Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table). Просто замените `CREATE` на `ALTER` в команде и удалите `type='distributed'`:

```sql
ALTER TABLE `distr_table_name` [[local='local_table_name'], [agent='host:port:remote_table'] ... ]
```

> ПРИМЕЧАНИЕ: Изменение схемы распределенной таблицы онлайн требует наличия [Manticore Buddy](Installation/Manticore_Buddy.md). Если это не работает, убедитесь, что Buddy установлен.

<!-- request Example -->
```sql
ALTER TABLE local_dist local='index1' local='index2' agent='127.0.0.1:9312:remote_table';
```

<!-- end -->
<!-- proofread -->

