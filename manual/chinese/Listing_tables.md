# 列出表格

Manticore 搜索具有单一层次结构的表格。

与其他数据库管理系统不同，Manticore 没有将表格分组到数据库中的概念。然而，为了与 SQL 方言的互操作性，Manticore 接受 `SHOW DATABASES` 语句以与 SQL 方言进行互操作，但该语句不返回任何结果。

<!-- 示例列表 -->
## 显示表格

一般语法：

```sql
SHOW TABLES [ LIKE pattern ]
```

`SHOW TABLES` 语句列出所有当前活动的表格及其类型。现有的表格类型有 `local`、`distributed`、`rt`、`percolate` 和 `template`。


<!-- 引言 -->
##### SQL:

<!-- 请求 SQL -->

```sql
SHOW TABLES;
```

<!-- 响应 SQL -->

```sql
+----------+-------------+
| 索引     | 类型        |
+----------+-------------+
| dist     | distributed |
| plain    | local       |
| pq       | percolate   |
| rt       | rt          |
| template | template    |
+----------+-------------+
5 行在集内 (0.00 秒)
```

<!-- 请求 PHP -->

```php
$client->nodes()->table();
```

<!-- 响应 PHP -->

```php
Array
(
    [dist1] => distributed
    [rt] => rt
    [products] => rt
)

```
<!-- 引言 -->
##### Python:

<!-- 请求 Python -->

```python
utilsApi.sql('SHOW TABLES')
```

<!-- 响应 Python -->
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

<!-- 引言 -->
##### Python-asyncio:

<!-- 请求 Python-asyncio -->

```python
await utilsApi.sql('SHOW TABLES')
```

<!-- 响应 Python-asyncio -->
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

<!-- 引言 -->
##### Javascript:

<!-- 请求 javascript -->

```javascript
res = await utilsApi.sql('SHOW TABLES');
```

<!-- 响应 javascript -->
```javascript
{"columns":[{"Index":{"type":"string"}},{"Type":{"type":"string"}}],"data":[{"Index":"products","Type":"rt"}],"total":0,"error":"","warning":""}
```

<!-- 引言 -->
##### java:

<!-- 请求 Java -->

```java
utilsApi.sql("SHOW TABLES", true)
```

<!-- 响应 Java -->
```java
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error=, warning=}
```

<!-- 引言 -->
##### C#:

<!-- 请求 C# -->

```clike
utilsApi.Sql("SHOW TABLES", true)
```

<!-- 响应 C# -->
```C#
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error="", warning=""}
```

<!-- 引言 -->
##### Rust:

<!-- 请求 Rust -->

```rust
utils_api.sql("SHOW TABLES", Some(true)).await
```

<!-- 响应 Rust -->
```Rust
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error="", warning=""}
```

<!-- 结束 -->

<!-- 示例 Example_2 -->
可选的 LIKE 子句支持通过名称过滤表格。


<!-- 引言 -->
##### SQL:

<!-- 请求 SQL -->

```sql
SHOW TABLES LIKE 'pro%';
```

<!-- 响应 SQL -->

```sql
+----------+-------------+
| 索引     | 类型        |
+----------+-------------+
| products | distributed |
+----------+-------------+
1 行在集内 (0.00 秒)
```

<!-- 请求 PHP -->

```php
$client->nodes()->table(['body'=>['pattern'=>'pro%']]);
```

<!-- 响应 PHP -->

```php
Array
(
    [products] => distributed
)

```


<!-- 引言 -->
##### Python:

<!-- 请求 Python -->

```python
utilsApi.sql('SHOW TABLES LIKE 'pro%'');
```

<!-- 响应 Python -->
```python
{u'columns': [{u'Index': {u'type': u'string'}},
              {u'Type': {u'type': u'string'}}],
 u'data': [{u'Index': u'products', u'Type': u'rt'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```

<!-- 引言 -->
##### Python-asyncio:

<!-- 请求 Python-asyncio -->

```python
await utilsApi.sql('SHOW TABLES LIKE 'pro%'');
```

<!-- 响应 Python-asyncio -->
```python
{u'columns': [{u'Index': {u'type': u'string'}},
              {u'Type': {u'type': u'string'}}],
 u'data': [{u'Index': u'products', u'Type': u'rt'}],
 u'error': u'',
 u'total': 0,
 u'warning': u''}
```

<!-- 引言 -->
##### Javascript:

<!-- 请求 javascript -->

```javascript
utilsApi.sql('SHOW TABLES LIKE 'pro%'')
```

<!-- 响应 javascript -->
```javascript
{"columns":[{"Index":{"type":"string"}},{"Type":{"type":"string"}}],"data":[{"Index":"products","Type":"rt"}],"total":0,"error":"","warning":""}
```


<!-- 引言 -->
##### java:

<!-- 请求 Java -->

```java
utilsApi.sql("SHOW TABLES LIKE 'pro%'", true)
```

<!-- 响应 Java -->
```java
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error=, warning=}
```

<!-- 引言 -->
##### C#:

<!-- 请求 C# -->

```clike
utilsApi.Sql("SHOW TABLES LIKE 'pro%'", true)
```

<!-- 响应 C# -->
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

## 描述

```sql
{DESC | DESCRIBE} table_name [ LIKE pattern ]
```

`DESCRIBE` 语句列出表的列及其相关类型。列包括文档 ID、全文字段和属性。顺序与 `INSERT` 和 `REPLACE` 语句所期望的字段和属性的顺序相匹配。列类型包括 `field`、`integer`、`timestamp`、`ordinal`、`bool`、`float`、`bigint`、`string` 和 `mva`。 ID 列将被定为 `bigint`。示例：

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

支持可选的 LIKE 子句。有关其语法详细信息，请参阅
[SHOW META](Node_info_and_management/SHOW_META.md)。

### 从 name.@table 选择

<!-- example name_table -->
您还可以通过执行查询 `select * from <table_name>.@table` 来查看表结构。此方法的好处是可以使用 `WHERE` 子句进行筛选：

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

您还可以对 `<your_table_name>.@table` 执行许多其他操作，将其视为具有整数和字符串属性的常规 Manticore 表。

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

### 滤波表架构

如果您在滤波表上使用 `DESC` 语句，它将显示外部表架构，即存储查询的架构。此架构是静态的，对于所有本地滤波表都是相同的：

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

如果要查看预期的文档架构，请使用以下命令：
`DESC <pq 表名称> table`：

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

同时支持 `desc pq table like ...`，其工作方式如下：

```sql
mysql> desc pq table like '%title%';
+-------+------+----------------+
| Field | Type | Properties     |
+-------+------+----------------+
| title | text | indexed stored |
+-------+------+----------------+
1 row in set (0.00 sec)
```
# 列出表格

Manticore Search 有一个单层次的表格层次结构。

与其他数据库管理系统不同，Manticore中没有将表格分组到数据库中的概念。然而，为了与SQL方言的互操作性，Manticore接受 `SHOW DATABASES` 语句以实现与SQL方言的互操作，但该语句不会返回任何结果。

<!-- example listing -->
## SHOW TABLES

一般语法：

```sql
SHOW TABLES [ LIKE pattern ]
```

`SHOW TABLES` 语句列出所有当前活动的表格及其类型。现有的表格类型有 `local`、`distributed`、`rt`、`percolate` 和 `template`。


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW TABLES;
```

<!-- response SQL -->

```sql
+----------+-------------+
| 索引      | 类型        |
+----------+-------------+
| dist     | distributed |
| plain    | local       |
| pq       | percolate   |
| rt       | rt          |
| template | template    |
+----------+-------------+
5 行在集合中 (0.00 秒)
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
utilsApi.sql("SHOW TABLES")
```

<!-- response Java -->
```java
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error=, warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("SHOW TABLES")
```

<!-- response C# -->
```C#
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error="", warning=""}
```

<!-- end -->

<!-- example Example_2 -->
可选的 LIKE 子句支持按名称过滤表格。


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SHOW TABLES LIKE 'pro%';
```

<!-- response SQL -->

```sql
+----------+-------------+
| 索引      | 类型        |
+----------+-------------+
| products | distributed |
+----------+-------------+
1 行在集合中 (0.00 秒)
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
utilsApi.sql("SHOW TABLES LIKE 'pro%'")
```

<!-- response Java -->
```java
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error=, warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("SHOW TABLES LIKE 'pro%'")
```

<!-- response C# -->
```clike
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error="", warning=""}
```


<!-- end -->

## 描述

```sql
{DESC | DESCRIBE} table_name [ LIKE pattern ]
```

`DESCRIBE` 语句列出表格列及其关联的类型。列包括文档ID、全文字段和属性。顺序与 `INSERT` 和 `REPLACE` 语句期待的字段和属性顺序匹配。列类型包括 `field`、`integer`、`timestamp`、`ordinal`、`bool`、`float`、`bigint`、`string` 和 `mva`。ID列的类型为 `bigint`。示例如下：

```sql
mysql> DESC rt;
+---------+---------+
| 字段    | 类型    |
+---------+---------+
| id      | bigint  |
| title   | field   |
| content | field   |
| gid     | integer |
+---------+---------+
4 行在集合中 (0.00 秒)
```

可选的 LIKE 子句是支持的。请参考
[SHOW META](Node_info_and_management/SHOW_META.md) 以获取其语法细节。

### 从 name.@table 选择

<!-- example name_table -->
您还可以通过执行查询 `select * from <table_name>.@table` 来查看表格架构。此方法的好处是您可以使用 `WHERE` 子句进行过滤：

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

您还可以对 `<your_table_name>.@table` 执行许多其他操作，将其视为具有整数和字符串属性的常规 Manticore 表。

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
<!-- end -->

### Percolate table schemas

如果您在渗透表上使用 `DESC` 语句，它将显示外部表架构，即存储查询的架构。该架构为静态，所有本地渗透表相同：

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

如果您想查看预期的文档架构，请使用以下命令：
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

同时支持 `desc pq table like ...` 并按如下方式工作：

```sql
mysql> desc pq table like '%title%';
+-------+------+----------------+
| Field | Type | Properties     |
+-------+------+----------------+
| title | text | indexed stored |
+-------+------+----------------+
1 row in set (0.00 sec)
```
