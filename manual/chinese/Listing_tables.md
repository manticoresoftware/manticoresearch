# 表格列表

Manticore搜索具有单层表结构层次。

与其他数据库管理系统不同，Manticore没有概念的表分组到数据库的层次结构。然而，为了与SQL方言的互操作性，Manticore接受`SHOW DATABASES`语句，但该语句不会返回任何结果。

<!-- example listing -->
## SHOW TABLES

测试1
通用语法：

```sql
SHOW TABLES [ LIKE pattern ]
```

`SHOW TABLES`语句列出所有当前活动表，以及它们的类型。现有的表类型包括`local`、`distributed`、`rt`、`percolate`和`template`。


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
##### JavaScript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('SHOW TABLES');
```

<!-- response javascript -->
```javascript
{"columns":[{"Index":{"type":"string"}},{"Type":{"type":"string"}}],"data":[{"Index":"products","Type":"rt"}],"total":0,"error":"","warning":""}
```

<!-- intro -->
##### Java:

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
可选的LIKE子句用于根据表名进行过滤。


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
##### JavaScript:

<!-- request javascript -->

```javascript
utilsApi.sql('SHOW TABLES LIKE \'pro%\'')
```

<!-- response javascript -->
```javascript
{"columns":[{"Index":{"type":"string"}},{"Type":{"type":"string"}}],"data":[{"Index":"products","Type":"rt"}],"total":0,"error":"","warning":""}
```


<!-- intro -->
##### Java:

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

`DESCRIBE`语句列出表的列及其关联类型。列包括文档ID、全文字段和属性。列的顺序与`INSERT`和`REPLACE`语句期望的顺序一致。列类型包括`field`、`integer`、`timestamp`、`ordinal`、`bool`、`float`、`bigint`、`string`和`mva`。ID列将被类型化为`bigint`。示例：

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

可选的LIKE子句支持。请参阅[SHOW META](Node_info_and_management/SHOW_META.md)以获取其语法细节。
### SELECT FROM name.@table

你可以也可以执行查询`select * from <table_name>.@table`，以查看表的结构。该方法的优势在于可以使用`WHERE`子句进行过滤：

<!-- example name_table -->
select * from tbl.@table where type='text';

<!-- request SQL -->
```sql
select * from tbl.@table where type='text';
```

<!-- response SQL -->
```sql
你可以也可以对`<your_table_name>.@table`执行许多其他操作，考虑到它是一个Manticore表，包含整数和字符串属性。
select field from tbl.@table;
select field, properties from tbl.@table where type in ('text', 'uint');
select * from tbl.@table where properties any ('stored');
## SHOW CREATE TABLE
SHOW CREATE TABLE table_name
```

<!-- end -->

<!-- example name_table2 -->

打印用于创建指定表的`CREATE TABLE`语句。

<!-- request SQL -->

```sql
##### SQL:
SHOW CREATE TABLE tbl\G
SHOW CREATE TABLE tbl\G
```

<!-- end -->

### 多表结构

<!-- example show_create -->
```sql
如果使用`DESC`语句对一个percolate表进行查询，它会显示外表的结构，该结构是存储查询的表的结构。该结构是静态的，所有本地的percolate表都使用相同的结构：
```

mysql> DESC pq;

<!-- intro -->
+---------+--------+

<!-- request SQL -->
```sql
| Field   | Type   |
```

<!-- response SQL -->
```sql
+---------+--------+
| id      | bigint |
| query   | string |
| tags    | string |
| filters | string |
```
<!-- end -->

+---------+--------+

4 rows in set (0.00 sec)

```sql
如果要查看预期的文档结构，请使用以下命令：
`DESC <pq table name> table`
mysql> DESC pq TABLE;
+-------+--------+
| Field | Type   |
+-------+--------+
| id    | bigint |
| title | text   |
| gid   | uint   |
+-------+--------+
```

3 rows in set (0.00 sec)
此外，`desc pq table like ...`也支持，并且工作如下：

```sql
mysql> desc pq table like '%title%';
+-------+------+----------------+
| Field | Type | Properties     |
+-------+------+----------------+
| title | text | indexed stored |
+-------+------+----------------+
1 row in set (0.00 sec)
mysql> desc pq table like '%title%';
+-------+------+----------------+
```

| Field | Type | Properties     |

```sql
+-------+------+----------------+
| title | text | indexed stored |
+-------+------+----------------+
1 row in set (0.00 sec)
| title | text | indexed stored |
+-------+------+----------------+
1 row in set (0.00 sec)
```

