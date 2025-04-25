# Перечень таблиц

Manticore Search имеет один уровень иерархии для таблиц.

В отличие от других СУБД, в Manticore нет концепции группировки таблиц в базы данных. Однако, для совместимости с SQL-диалектами, Manticore принимает операторы `SHOW DATABASES`, но этот оператор не возвращает никаких результатов.

<!-- example listing -->
## SHOW TABLES

Общий синтаксис:

```sql
SHOW TABLES [ LIKE pattern ]
```

Оператор `SHOW TABLES` перечисляет все текущие активные таблицы вместе с их типами. Существующие типы таблиц: `local`, `distributed`, `rt`, `percolate` и `template`.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW TABLES;
```

<!-- response SQL -->

```sql
+----------+-------------+
| Index    | Type        |
+----------+-------------+
| dist     | distributed |
| plain    | local       |
| pq       | percolate   |
| rt       | rt          |
| template | template    |
+----------+-------------+
5 rows in set (0.00 sec)
```

<!-- request PHP -->

```php
$client->nodes()->table();
```

<!-- response PHP -->

```php
Array
(
    [dist1] => distributed
    [rt] => rt
    [products] => rt
)

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('SHOW TABLES')
```

<!-- response Python -->
```python
{u'columns': [{u'Index': {u'type': u'string'}},
              {u'Type': {u'type': u'string'}}],
 u'data': [{u'Index': u'dist1', u'Type': u'distributed'},
           {u'Index': u'rt', u'Type': u'rt'},
           {u'Index': u'products', u'Type': u'rt'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('SHOW TABLES')
```

<!-- response Python-asyncio -->
```python
{u'columns': [{u'Index': {u'type': u'string'}},
              {u'Type': {u'type': u'string'}}],
 u'data': [{u'Index': u'dist1', u'Type': u'distributed'},
           {u'Index': u'rt', u'Type': u'rt'},
           {u'Index': u'products', u'Type': u'rt'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('SHOW TABLES');
```

<!-- response javascript -->
```javascript
{"columns":[{"Index":{"type":"string"}},{"Type":{"type":"string"}}],"data":[{"Index":"products","Type":"rt"}],"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("SHOW TABLES", true)
```

<!-- response Java -->
```java
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error=, warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("SHOW TABLES", true)
```

<!-- response C# -->
```C#
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error="", warning=""}
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("SHOW TABLES", Some(true)).await
```

<!-- response Rust -->
```Rust
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error="", warning=""}
```

<!-- end -->

<!-- example Example_2 -->
Дополнительный оператор LIKE поддерживается для фильтрации таблиц по имени.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW TABLES LIKE 'pro%';
```

<!-- response SQL -->

```sql
+----------+-------------+
| Index    | Type        |
+----------+-------------+
| products | distributed |
+----------+-------------+
1 row in set (0.00 sec)
```

<!-- request PHP -->

```php
$client->nodes()->table(['body'=>['pattern'=>'pro%']]);
```

<!-- response PHP -->

```php
Array
(
    [products] => distributed
)

```


<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('SHOW TABLES LIKE 'pro%'');
```

<!-- response Python -->
```python
{u'columns': [{u'Index': {u'type': u'string'}},
              {u'Type': {u'type': u'string'}}],
 u'data': [{u'Index': u'products', u'Type': u'rt'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('SHOW TABLES LIKE 'pro%'');
```

<!-- response Python-asyncio -->
```python
{u'columns': [{u'Index': {u'type': u'string'}},
              {u'Type': {u'type': u'string'}}],
 u'data': [{u'Index': u'products', u'Type': u'rt'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
utilsApi.sql('SHOW TABLES LIKE 'pro%'')
```

<!-- response javascript -->
```javascript
{"columns":[{"Index":{"type":"string"}},{"Type":{"type":"string"}}],"data":[{"Index":"products","Type":"rt"}],"total":0,"error":"","warning":""}
```


<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("SHOW TABLES LIKE 'pro%'", true)
```

<!-- response Java -->
```java
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error=, warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("SHOW TABLES LIKE 'pro%'", true)
```

<!-- response C# -->
```clike
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error="", warning=""}
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("SHOW TABLES LIKE 'pro%'", Some(true)).await
```

<!-- response Rust -->
```rust
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error="", warning=""}
```


<!-- end -->

## ОПИСАТЬ

```sql
{DESC | DESCRIBE} table_name [ LIKE pattern ]
```

Оператор `DESCRIBE` перечисляет столбцы таблицы и их соответствующие типы. Столбцы — это идентификатор документа, текстовые поля и атрибуты. Порядок соответствует порядку, в котором поля и атрибуты ожидаются в операторах `INSERT` и `REPLACE`. Типы столбцов включают `field`, `integer`, `timestamp`, `ordinal`, `bool`, `float`, `bigint`, `string` и `mva`. Столбец ID будет иметь тип `bigint`. Пример:

```sql
mysql> DESC rt;
+---------+---------+
| Field   | Type    |
+---------+---------+
| id      | bigint  |
| title   | field   |
| content | field   |
| gid     | integer |
+---------+---------+
4 rows in set (0.00 sec)
```

Поддерживается необязательный оператор LIKE. Смотрите
[SHOW META](Node_info_and_management/SHOW_META.md) для подробностей о его синтаксисе.

### SELECT FROM name.@table

<!-- example name_table -->
Вы также можете просмотреть схему таблицы, выполнив запрос `select * from <table_name>.@table`. Преимущество этого метода заключается в том, что вы можете использовать оператор `WHERE` для фильтрации:

<!-- request SQL -->
```sql
select * from tbl.@table where type='text';
```

<!-- response SQL -->
```sql
+------+-------+------+----------------+
| id   | field | type | properties     |
+------+-------+------+----------------+
|    2 | title | text | indexed stored |
+------+-------+------+----------------+
1 row in set (0.00 sec)
```

<!-- end -->

<!-- example name_table2 -->

Вы также можете выполнять множество других действий над `<your_table_name>.@table`, рассматривая его как обычную таблицу Manticore с колонками, состоящими из атрибутов целого числа и строки.

<!-- request SQL -->

```sql
select field from tbl.@table;
select field, properties from tbl.@table where type in ('text', 'uint');
select * from tbl.@table where properties any ('stored');
```

<!-- end -->

## ПОКАЗАТЬ СОЗДАНИЕ ТАБЛИЦЫ

<!-- example show_create -->
```sql
SHOW CREATE TABLE table_name
```

Печатает оператор `CREATE TABLE`, используемый для создания указанной таблицы.

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
SHOW CREATE TABLE tblG
```

<!-- response SQL -->
```sql
       Table: tbl
Create Table: CREATE TABLE tbl (
f text indexed stored
) charset_table='non_cont,cont' morphology='icu_chinese'
1 row in set (0.00 sec)
```
<!-- end -->

### Схемы таблиц перколяторов

Если вы используете оператор `DESC` на таблице перколяторов, он отобразит схему внешней таблицы, которая является схемой сохраненных запросов. Эта схема статична и одинакова для всех локальных таблиц перколяторов:

```sql
mysql> DESC pq;
+---------+--------+
| Field   | Type   |
+---------+--------+
| id      | bigint |
| query   | string |
| tags    | string |
| filters | string |
+---------+--------+
4 rows in set (0.00 sec)
```

Если вы хотите просмотреть ожидаемую схему документа, используйте следующую команду:
`DESC <pq table name> table`:

```sql
mysql> DESC pq TABLE;
+-------+--------+
| Field | Type   |
+-------+--------+
| id    | bigint |
| title | text   |
| gid   | uint   |
+-------+--------+
3 rows in set (0.00 sec)
```

Также поддерживается `desc pq table like ...` и работает следующим образом:

```sql
mysql> desc pq table like '%title%';
+-------+------+----------------+
| Field | Type | Properties     |
+-------+------+----------------+
| title | text | indexed stored |
+-------+------+----------------+
1 row in set (0.00 sec)
```
# Перечисление таблиц

Manticore Search имеет один уровень иерархии для таблиц.

В отличие от других СУБД, в Manticore нет концепции группировки таблиц в базы данных. Однако для совместимости с диалектами SQL Manticore принимает команды `SHOW DATABASES`, но команда не возвращает никаких результатов.

<!-- example listing -->
## Показать таблицы

Общий синтаксис:

```sql
SHOW TABLES [ LIKE pattern ]
```

Команда `SHOW TABLES` перечисляет все активные таблицы вместе с их типами. Существующие типы таблиц: `local`, `distributed`, `rt`, `percolate` и `template`.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW TABLES;
```

<!-- response SQL -->

```sql
+----------+-------------+
| Индекс   | Тип         |
+----------+-------------+
| dist     | distributed |
| plain    | local       |
| pq       | percolate   |
| rt       | rt          |
| template | template    |
+----------+-------------+
5 строк в наборе (0.00 сек)
```

<!-- request PHP -->

```php
$client->nodes()->table();
```

<!-- response PHP -->

```php
Array
(
    [dist1] => distributed
    [rt] => rt
    [products] => rt
)

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('SHOW TABLES')
```

<!-- response Python -->
```python
{u'columns': [{u'Индекс': {u'type': u'string'}},
              {u'Тип': {u'type': u'string'}}],
 u'data': [{u'Индекс': u'dist1', u'Тип': u'distributed'},
           {u'Индекс': u'rt', u'Тип': u'rt'},
           {u'Индекс': u'products', u'Тип': u'rt'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('SHOW TABLES');
```

<!-- response javascript -->
```javascript
{"columns":[{"Индекс":{"type":"string"}},{"Тип":{"type":"string"}}],"data":[{"Индекс":"products","Тип":"rt"}],"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("SHOW TABLES")
```

<!-- response Java -->
```java
{columns=[{Индекс={type=string}}, {Тип={type=string}}], data=[{Индекс=products, Тип=rt}], total=0, error=, warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("SHOW TABLES")
```

<!-- response C# -->
```C#
{columns=[{Индекс={type=string}}, {Тип={type=string}}], data=[{Индекс=products, Тип=rt}], total=0, error="", warning=""}
```

<!-- end -->

<!-- example Example_2 -->
Дополнительный оператор LIKE поддерживается для фильтрации таблиц по имени.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW TABLES LIKE 'pro%';
```

<!-- response SQL -->

```sql
+----------+-------------+
| Индекс   | Тип         |
+----------+-------------+
| products | distributed |
+----------+-------------+
1 строка в наборе (0.00 сек)
```

<!-- request PHP -->

```php
$client->nodes()->table(['body'=>['pattern'=>'pro%']]);
```

<!-- response PHP -->

```php
Array
(
    [products] => distributed
)

```


<!-- intro -->
##### Python:

<!-- request Python -->

```python
res = await utilsApi.sql('SHOW TABLES LIKE \'pro%\'');
```

<!-- response Python -->
```python
{u'columns': [{u'Индекс': {u'type': u'string'}},
              {u'Тип': {u'type': u'string'}}],
 u'data': [{u'Индекс': u'products', u'Тип': u'rt'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
utilsApi.sql('SHOW TABLES LIKE \'pro%\'')
```

<!-- response javascript -->
```javascript
{"columns":[{"Индекс":{"type":"string"}},{"Тип":{"type":"string"}}],"data":[{"Индекс":"products","Тип":"rt"}],"total":0,"error":"","warning":""}
```


<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("SHOW TABLES LIKE 'pro%'")
```

<!-- response Java -->
```java
{columns=[{Индекс={type=string}}, {Тип={type=string}}], data=[{Индекс=products, Тип=rt}], total=0, error=, warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("SHOW TABLES LIKE 'pro%'")
```

<!-- response C# -->
```clike
{columns=[{Индекс={type=string}}, {Тип={type=string}}], data=[{Индекс=products, Тип=rt}], total=0, error="", warning=""}
```


<!-- end -->

## ОПИСАТЬ

```sql
{DESC | ОПИСАТЬ} имя_таблицы [ LIKE pattern ]
```

Команда `ОПИСАТЬ` перечисляет столбцы таблицы и их соответствующие типы. Столбцы — это идентификатор документа, полнотекстовые поля и атрибуты. Порядок соответствует порядку, в котором поля и атрибуты ожидаются в командах `INSERT` и `REPLACE`. Типы столбцов включают `field`, `integer`, `timestamp`, `ordinal`, `bool`, `float`, `bigint`, `string` и `mva`. Столбец ID будет иметь тип `bigint`. Пример:

```sql
mysql> DESC rt;
+---------+---------+
| Поле    | Тип     |
+---------+---------+
| id      | bigint  |
| title   | field   |
| content | field   |
| gid     | integer |
+---------+---------+
4 строки в наборе (0.00 сек)
```

Дополнительный оператор LIKE поддерживается. Смотрите
[SHOW META](Node_info_and_management/SHOW_META.md) для получения подробностей о его синтаксисе.

### SELECT FROM имя.@таблица

<!-- example name_table -->
Вы также можете просмотреть схему таблицы, выполнив запрос `select * from <table_name>.@table`. Преимущество этого метода в том, что вы можете использовать оператор `WHERE` для фильтрации:

<!-- request SQL -->
```sql
select * from tbl.@table where type='text';
```

<!-- response SQL -->
```sql
+------+-------+------+----------------+
| id   | field | type | properties     |
+------+-------+------+----------------+
|    2 | title | text | indexed stored |
+------+-------+------+----------------+
1 row in set (0.00 sec)
```

<!-- end -->

<!-- пример name_table2 -->

Вы также можете выполнять многие другие действия с `<your_table_name>.@table`, рассматривая его как обычную таблицу Manticore с колонками, состоящими из атрибутов целого числа и строки.

<!-- запрос SQL -->

```sql
select field from tbl.@table;
select field, properties from tbl.@table where type in ('text', 'uint');
select * from tbl.@table where properties any ('stored');
```

<!-- end -->

## SHOW CREATE TABLE

<!-- пример show_create -->
```sql
SHOW CREATE TABLE table_name
```

Печатает оператор `CREATE TABLE`, использованный для создания указанной таблицы.

<!-- intro -->
##### SQL:

<!-- запрос SQL -->
```sql
SHOW CREATE TABLE tbl\G
```

<!-- ответ SQL -->
```sql
       Table: tbl
Create Table: CREATE TABLE tbl (
f text indexed stored
) charset_table='non_cont,cont' morphology='icu_chinese'
1 row in set (0.00 sec)
```
<!-- end -->

### Схемы таблицы перколятор

Если вы используете оператор `DESC` на таблице перколятор, он отобразит схему внешней таблицы, которая является схемой сохраненных запросов. Эта схема статична и одинакова для всех локальных таблиц перколятор:

```sql
mysql> DESC pq;
+---------+--------+
| Field   | Type   |
+---------+--------+
| id      | bigint |
| query   | string |
| tags    | string |
| filters | string |
+---------+--------+
4 rows in set (0.00 sec)
```

Если вы хотите просмотреть ожидаемую схему документа, используйте следующую команду:
`DESC <имя таблицы pq> table`:

```sql
mysql> DESC pq TABLE;
+-------+--------+
| Field | Type   |
+-------+--------+
| id    | bigint |
| title | text   |
| gid   | uint   |
+-------+--------+
3 rows in set (0.00 sec)
```

Также поддерживается `desc pq table like ...` и работает следующим образом:

```sql
mysql> desc pq table like '%title%';
+-------+------+----------------+
| Field | Type | Properties     |
+-------+------+----------------+
| title | text | indexed stored |
+-------+------+----------------+
1 row in set (0.00 sec)
```
