# 列出表

Manticore Search 对表只有单层层级结构。

与其他数据库管理系统不同，Manticore 中没有将表分组到数据库中的概念。然而，为了与 SQL 方言的互操作性，Manticore 接受 `SHOW DATABASES` 语句，但该语句不返回任何结果。

<!-- example listing -->
## SHOW TABLES

通用语法：

```sql
SHOW TABLES [ LIKE pattern ]
```

`SHOW TABLES` 语句列出所有当前活动的表及其类型。现有的表类型包括 `local`、`distributed`、`rt`、`percolate` 和 `template`。


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
数据用于以下示例：

CREATE TABLE products type='distributed' local='products' agent='127.0.0.1:9312:products'
-->

<!-- example Example_2 -->
支持可选的 LIKE 子句用于按名称过滤表。


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

`DESCRIBE` 语句列出表的列及其对应类型。列包括文档 ID、全文字段和属性。顺序与 `INSERT` 和 `REPLACE` 语句中字段和属性的预期顺序相匹配。列类型包括 `field`、`integer`、`timestamp`、`ordinal`、`bool`、`float`、`bigint`、`string` 和 `mva`。ID 列类型为 `bigint`。示例：

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

支持可选的 LIKE 子句。具体语法详情请参见
[SHOW META](Node_info_and_management/SHOW_META.md) 。

### SELECT FROM name.@table

<!--
数据用于以下示例：

DROP TABLE IF EXISTS tbl;
CREATE TABLE tbl(title text indexed stored) charset_table='non_cont,cont' morphology='icu_chinese';
--> 

<!-- example name_table -->
也可以通过执行查询 `select * from <table_name>.@table` 来查看表的架构。该方法的优点是可以使用 `WHERE` 子句进行过滤：

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

你还可以对 `<your_table_name>.@table` 执行许多其他操作，将其视作一个具有整数和字符串属性列的常规 Manticore 表。

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

打印用于创建指定表的 `CREATE TABLE` 语句。

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
POST /sql?mode=raw -d "SHOW CREATE TABLE tbl;"
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

### Percolate 表结构

如果对 percolate 表使用 `DESC` 语句，它将显示外部表结构，即存储查询的结构。该结构是静态的，所有本地 percolate 表相同：

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

如果你想查看预期的文档结构，请使用以下命令：
`DESC <pq table name> table`：

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

还支持 `desc pq table like ...`，其工作方式如下：

```sql
mysql> desc pq table like '%title%';
+-------+------+----------------+
| Field | Type | Properties     |
+-------+------+----------------+
| title | text | indexed stored |
+-------+------+----------------+
1 row in set (0.00 sec)
```

