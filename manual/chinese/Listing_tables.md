# 列表表格

Manticore Search 具有单一层次结构的表格。

与其他DBMS不同，在Manticore中没有将表格分组到数据库中的概念。然而，为了与SQL方言的互操作性，Manticore接受`SHOW DATABASES`语句，但该语句不会返回任何结果。

<!-- example listing -->
## SHOW TABLES

通用语法：

```sql
SHOW TABLES [ LIKE pattern ]
```

`SHOW TABLES` 语句列出所有当前活动的表格及其类型。现有的表格类型包括 `local`，`distributed`，`rt`，`percolate` 和 `template`。


<!-- intro -->
##### SQL：

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
##### Python：

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
##### Python-asyncio：

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
##### Javascript：

<!-- request javascript -->

```javascript
res = await utilsApi.sql('SHOW TABLES');
```

<!-- response javascript -->
```javascript
{"columns":[{"Index":{"type":"string"}},{"Type":{"type":"string"}}],"data":[{"Index":"products","Type":"rt"}],"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java：

<!-- request Java -->

```java
utilsApi.sql("SHOW TABLES", true)
```

<!-- response Java -->
```java
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error=, warning=}
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
utilsApi.Sql("SHOW TABLES", true)
```

<!-- response C# -->
```C#
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error="", warning=""}
```

<!-- intro -->
##### Rust：

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
支持使用 LIKE 子句按名称过滤表格。


<!-- intro -->
##### SQL：

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
##### Python：

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
##### Python-asyncio：

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
##### Javascript：

<!-- request javascript -->

```javascript
utilsApi.sql('SHOW TABLES LIKE \'pro%\'')
```

<!-- response javascript -->
```javascript
{"columns":[{"Index":{"type":"string"}},{"Type":{"type":"string"}}],"data":[{"Index":"products","Type":"rt"}],"total":0,"error":"","warning":""}
```


<!-- intro -->
##### java：

<!-- request Java -->

```java
utilsApi.sql("SHOW TABLES LIKE 'pro%'", true)
```

<!-- response Java -->
```java
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error=, warning=}
```

<!-- intro -->
##### C#：

<!-- request C# -->

```clike
utilsApi.Sql("SHOW TABLES LIKE 'pro%'", true)
```

<!-- response C# -->
```clike
{columns=[{Index={type=string}}, {Type={type=string}}], data=[{Index=products, Type=rt}], total=0, error="", warning=""}
```

<!-- intro -->
##### Rust：

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

`DESCRIBE` 语句列出表格的列及其关联的类型。列包括文档ID、全文字段和属性。顺序与 `INSERT` 和 `REPLACE` 语句中预期的字段和属性顺序相同。列类型包括 `field`，`integer`，`timestamp`，`ordinal`，`bool`，`float`，`bigint`，`string` 和 `mva`。ID 列将被类型化为 `bigint`。示例：

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

支持使用 LIKE 子句。参见
[SHOW META](Node_info_and_management/SHOW_META.md) 以获取其语法细节。

### SELECT FROM name.@table

<!-- example name_table -->
您还可以通过执行查询 `select * from <table_name>.@table` 查看表格的模式。此方法的好处是您可以使用 `WHERE` 子句进行过滤：

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

您还可以将 `<your_table_name>.@table` 作为常规 Manticore 表进行许多其他操作，其列由整数和字符串属性组成。

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

打印用于创建指定表格的 `CREATE TABLE` 语句。

<!-- intro -->
##### SQL：

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

### Percolate 表模式

如果您在 percolate 表上使用 `DESC` 语句，它将显示外部表格模式，即存储查询的模式。此模式是静态的，并且所有本地 percolate 表相同：

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

如果您想查看预期的文档模式，请使用以下命令：
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

`desc pq table like ...` 也支持并按如下方式工作：

```sql
mysql> desc pq table like '%title%';
+-------+------+----------------+
| Field | Type | Properties     |
+-------+------+----------------+
| title | text | indexed stored |
+-------+------+----------------+
1 row in set (0.00 sec)
```

