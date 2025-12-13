# Обновление схемы таблицы

## Обновление схемы таблицы в режиме RT

<!-- example ALTER -->

```sql
ALTER TABLE table ADD COLUMN column_name [{INTEGER|INT|BIGINT|FLOAT|BOOL|MULTI|MULTI64|JSON|STRING|TIMESTAMP|TEXT [INDEXED [ATTRIBUTE]]}] [engine='columnar']

ALTER TABLE table DROP COLUMN column_name

ALTER TABLE table MODIFY COLUMN column_name bigint
```

Эта функция поддерживает только добавление одного поля за раз для RT-таблиц или расширение столбца `int` до `bigint`. Поддерживаемые типы данных:
* `int` - целочисленное атрибут
* `timestamp` - атрибут временной метки
* `bigint` - атрибут большого целого
* `float` - атрибут с плавающей точкой
* `bool` - булев атрибут
* `multi` - многозначный целочисленный атрибут
* `multi64` - многозначный атрибут большого целого
* `json` - json атрибут
* `string` / `text attribute` / `string attribute` - строковый атрибут
* `text` / `text indexed stored` / `string indexed stored` - поле с полнотекстовым индексом с хранением оригинального значения в docstore
* `text indexed` / `string indexed` - полнотекстовое индексированное поле, только индексируется (оригинальное значение не хранится в docstore)
* `text indexed attribute` / `string indexed attribute` - полнотекстовое индексированное поле + строковый атрибут (оригинальное значение не хранится в docstore)
* `text stored` / `string stored` - значение будет храниться только в docstore, не полнотекстово индексировано, не строковый атрибут
* добавление `engine='columnar'` к любому атрибуту (кроме json) заставит его храниться в [столбцовом хранилище](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)

#### Важные заметки:
* ❗Рекомендуется **создавать резервную копию файлов таблицы** перед выполнением `ALTER`, чтобы избежать повреждения данных при внезапном отключении питания или других аналогичных проблемах.
* Запросы к таблице невозможны во время добавления столбца.
* Значения только что созданного атрибута устанавливаются в 0.
* `ALTER` не работает для распределённых таблиц и таблиц без атрибутов.
* Нельзя удалять столбец `id`.
* При удалении поля, которое одновременно является полнотекстовым и строковым атрибутом, первый `ALTER DROP` удаляет атрибут, второй удаляет полнотекстовое поле.
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

## Обновление полнотекстовых настроек таблицы в режиме RT

<!-- example ALTER FT -->

```sql
ALTER TABLE table ft_setting='value'[, ft_setting2='value']
```

Вы можете использовать `ALTER` для изменения настроек полнотекста вашей таблицы в [RT режиме](Read_this_first.md#Real-time-mode-vs-plain-mode). Однако это влияет только на новые документы, а не на существующие.
Пример:
* создаём таблицу с полнотекстовым полем и `charset_table`, позволяющей искать только 3 символа: `a`, `b` и `c`.
* затем добавляем документ 'abcd' и ищем его по запросу `abcd`, символ `d` игнорируется, так как его нет в массиве `charset_table`
* затем понимаем, что хотим тоже искать по `d`, добавляем его с помощью `ALTER`
* но тот же запрос `where match('abcd')` всё ещё считает, что индексируется по `abc`, потому что существующий документ запомнил прежнее содержимое `charset_table`
* затем добавляем ещё один документ `abcd` и ищем по `abcd` снова
* теперь находятся оба документа, и `show meta` показывает, что используются два ключевых слова: `abc` (чтобы найти старый документ) и `abcd` (для нового).

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

## Переименование таблицы в режиме RT

<!-- example Renaming RT tables -->

Вы можете изменить имя таблицы в режиме RT.
```sql
ALTER TABLE table_name RENAME new_table_name;
```

> ПРИМЕЧАНИЕ: Переименование таблицы в режиме RT требует [Manticore Buddy](Installation/Manticore_Buddy.md). Если это не работает, убедитесь, что Buddy установлен.

<!-- request Example -->
```sql
ALTER TABLE table_name RENAME new_table_name;
```

<!-- response Example -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->

## Обновление полнотекстовых настроек таблицы в plain режиме

<!-- example ALTER RECONFIGURE -->
```sql
ALTER TABLE table RECONFIGURE
```

`ALTER` также может переконфигурировать RT таблицу в [plain режиме](Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29), чтобы новые настройки токенизации, морфологии и другие настройки обработки текста из конфигурационного файла применялись к новым документам. Обратите внимание, что существующие документы останутся без изменений. Внутренне операция принудительно сохраняет текущий RAM-чанк как новый дисковый чанк и корректирует заголовок таблицы так, чтобы новые документы индексировались с использованием обновлённых полнотекстовых настроек.

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

Вы также можете использовать `ALTER` для перестроения вторичных индексов в заданной таблице. Иногда вторичный индекс может быть отключён для всей таблицы или для одного/нескольких атрибутов таблицы:
* При обновлении атрибута его вторичный индекс отключается.
* Если Manticore загружает таблицу с устаревшей версией вторичных индексов, которая больше не поддерживается, вторичные индексы будут отключены для всей таблицы.

`ALTER TABLE table REBUILD SECONDARY` перестраивает вторичные индексы на основе данных атрибутов и повторно активирует их.

Кроме того, может поддерживаться старая версия вторичных индексов, но она будет лишена некоторых функций. `REBUILD SECONDARY` можно использовать для обновления вторичных индексов.

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

Команда повторно обрабатывает все векторные данные в таблице и перестраивает KNN индекс с нуля.

<!-- request Example -->
```sql
ALTER TABLE rt REBUILD KNN;
```

<!-- response Example -->
```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->

## Обновление параметров API атрибута (для генерации embeddings) в режиме RT

<!-- example api_key -->

`ALTER` может использоваться для изменения параметров API при использовании удалённой модели для авто-эмбеддингов:

```sql
ALTER TABLE table_name MODIFY COLUMN column_name API_KEY='key';
ALTER TABLE table_name MODIFY COLUMN column_name API_URL='url';
ALTER TABLE table_name MODIFY COLUMN column_name API_TIMEOUT='seconds';
```

<!-- request Example -->
```sql
ALTER TABLE rt MODIFY COLUMN vector API_KEY='new-key';
ALTER TABLE rt MODIFY COLUMN vector API_URL='https://custom-api.example.com/v1/embeddings';
ALTER TABLE rt MODIFY COLUMN vector API_TIMEOUT='30';
```

**Заметки:**
- `API_KEY`: Новый API ключ валидируется во время операции ALTER путём реального запроса к API.
- `API_URL`: Для возврата к стандартной конечной точке провайдера установите пустую строку (`''`).
- `API_TIMEOUT`: Установите в `'0'`, чтобы использовать таймаут по умолчанию (10 секунд). Должен быть неотрицательным целым числом.

<!-- end -->

## Изменение распределённой таблицы

<!-- example local_dist -->

Чтобы изменить список локальных или удалённых узлов в распределённой таблице, используйте тот же синтаксис, который вы применяли для [создания таблицы](Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table). Просто замените `CREATE` на `ALTER` в команде и удалите `type='distributed'`:

```sql
ALTER TABLE `distr_table_name` [[local='local_table_name'], [agent='host:port:remote_table'] ... ]
```

> ПРИМЕЧАНИЕ: Для изменения схемы распределённой таблицы в онлайн-режиме требуется [Manticore Buddy](Installation/Manticore_Buddy.md). Если это не работает, убедитесь, что Buddy установлен.

<!-- request Example -->
```sql
ALTER TABLE local_dist local='index1' local='index2' agent='127.0.0.1:9312:remote_table';
```

<!-- end -->
<!-- proofread -->

