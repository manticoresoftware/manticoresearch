# Listing tables

Manticore Search имеет однорівневу иєрархію таблиць.

На відміну від інших СУБД, у Manticore немає поняття групування таблиць у бази даних. Проте для сумісності з SQL-діалектами Manticore підтримує оператори `SHOW DATABASES` для сумісності з SQL-діалектом, але цей оператор не повертає жодних результатів.

<!-- example listing -->
## SHOW TABLES

Загальний синтаксис:

```sql
SHOW TABLES [ LIKE pattern ]
```

Оператор `SHOW TABLES` показує всі активні таблиці разом з їх типами. Існуючі типи таблиць — `local`, `distributed`, `rt`, `percolate` і `template`.


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


<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SHOW TABLES"
```

<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "Table": {
          "type": "string"
        }
      },
      {
        "Type": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "Table": "dist",
        "Type": "distributed"
      },
      {
        "Table": "plain",
        "Type": "local"
      },
      {
        "Table": "pq",
        "Type": "percolate"
      },{
        "Table": "rt",
        "Type": "rt"
      },{
        "Table": "template",
        "Type": "template"
      }
    ],
    "total": 5,
    "error": "",
    "warning": ""
  }
]

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

<!--
data for the following examples:

CREATE TABLE products type='distributed' local='products' agent='127.0.0.1:9312:products'
-->

<!-- example Example_2 -->
Підтримується необов’язковий оператор LIKE для фільтрування таблиць за іменем.


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

<!-- request JSON -->

```sql
POST /sql?mode=raw -d "SHOW TABLES LIKE 'pro%';"
```

<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "Table": {
          "type": "string"
        }
      },
      {
        "Type": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "Table": "products",
        "Type": "distributed"
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
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
utilsApi.sql('SHOW TABLES LIKE \'pro%\'');
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
await utilsApi.sql('SHOW TABLES LIKE \'pro%\'');
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
utilsApi.sql('SHOW TABLES LIKE \'pro%\'')
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

## DESCRIBE

```sql
{DESC | DESCRIBE} table_name [ LIKE pattern ]
```

Оператор `DESCRIBE` показує стовпці таблиці та їхні типи. Стовпці — це ID документа, повнотекстові поля й атрибути. Порядок збігається з порядком, у якому поля та атрибути очікуються у операторах `INSERT` і `REPLACE`. Типи стовпців включають `field`, `integer`, `timestamp`, `ordinal`, `bool`, `float`, `bigint`, `string` та `mva`. Стовпець ID буде типу `bigint`. Приклад:

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

Підтримується необов’язковий оператор LIKE. Деталі синтаксису дивіться у
[SHOW META](Node_info_and_management/SHOW_META.md).

### SELECT FROM name.@table

<!--
data for the following examples:

DROP TABLE IF EXISTS tbl;
CREATE TABLE tbl(title text indexed stored) charset_table='non_cont,cont' morphology='icu_chinese';
--> 

<!-- example name_table -->
Також можна переглянути схему таблиці, виконавши запит `select * from <table_name>.@table`. Перевага цього способу в тому, що можна застосувати оператор `WHERE` для фільтрування:

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

<!-- request JSON -->
```sql
POST /sql?mode=raw -d "select * from tbl.@table where type='text';"
```

<!-- response JSON -->
```JSON
[{
"columns":[{"id":{"type":"long long"}},{"field":{"type":"string"}},{"type":{"type":"string"}},{"properties":{"type":"string"}}],
"data":[
{"id":2,"field":"title","type":"text","properties":"indexed stored"}
],
"total":1,
"error":"",
"warning":""
}]
```

<!-- end -->

<!-- example name_table2 -->

Також можна виконувати багато інших операцій над `<your_table_name>.@table`, розглядаючи її як звичайну таблицю Manticore зі стовпцями, що містять цілочисельні та строкові атрибути.

<!-- request SQL -->

```sql
select field from tbl.@table;
select field, properties from tbl.@table where type in ('text', 'uint');
select * from tbl.@table where properties any ('stored');
```

<!-- end -->

## SHOW CREATE TABLE

<!-- example show_create -->
```sql
SHOW CREATE TABLE table_name
```

Виводить оператор `CREATE TABLE`, яким створено вказану таблицю.

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
SHOW CREATE TABLE tbl\G
```

<!-- response SQL -->
```sql
       Table: tbl
Create Table: CREATE TABLE tbl (
f text indexed stored
) charset_table='non_cont,cont' morphology='icu_chinese'
1 row in set (0.00 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SHOW CREATE TABLE tbl"
```

<!-- response JSON -->
```JSON
[{
"columns":[{"Table":{"type":"string"}},{"Create Table":{"type":"string"}}],
"data":[
{"Table":"tbl","Create Table":"CREATE TABLE tbl (\nf text)"}
],
"total":1,
"error":"",
"warning":""
}]
```

<!-- end -->

### Схеми таблиць перколейту

Якщо виконати оператор `DESC` для таблиці перколейту, буде показана зовнішня схема таблиці, яка є схемою збережених запитів. Ця схема статична і однакова для всіх локальних таблиць перколейту:

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

Якщо потрібно переглянути очікувану схему документа, використовуйте команду:
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

Також підтримується `desc pq table like ...` і працює наступним чином:

```sql
mysql> desc pq table like '%title%';
+-------+------+----------------+
| Field | Type | Properties     |
+-------+------+----------------+
| title | text | indexed stored |
+-------+------+----------------+
1 row in set (0.00 sec)
```

