# 删除表

<!-- example drop -->

删除表的操作内部执行分为两个步骤：
1. 表被清空（类似于 [TRUNCATE](Emptying_a_table.md)）
2. 所有表文件从表文件夹中移除。所有由表使用的外部表文件（如词形、扩展或停用词）也会被删除。请注意，当使用 `CREATE TABLE` 时，这些外部文件会被复制到表文件夹，因此在 `CREATE TABLE` 中指定的原始文件不会被删除。

只有在服务器以 RT 模式运行时，才能删除表。可以删除 RT 表、PQ 表和分布式表。

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
DROP TABLE products;
```
<!-- response -->

```sql
查询成功，影响的行数 = 0 (0.02 秒)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /cli -d "DROP TABLE products"
```

<!-- response JSON -->
```JSON
{
"total":0,
"error":"",
"warning":""
}
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$params = [ 'table' => 'products' ];

$response = $client->indices()->drop($params);
```

<!-- response PHP -->
```php
数组
(
    [total] => 0
    [error] =>
    [warning] =>
)

```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('DROP TABLE products')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('DROP TABLE products');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
sqlresult = utilsApi.sql("DROP TABLE products");
```

<!-- response Java -->
```java
{total=0, error=, warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
sqlresult = utilsApi.Sql("DROP TABLE products");
```

<!-- response C# -->
```clike
{total=0, error="", warning=""}
```

<!-- end -->

以下是 SQL 中 `DROP TABLE` 语句的语法：

```sql
DROP TABLE [IF EXISTS] table_name
```

<!-- example drop-if-exists -->

通过 SQL 删除表时，可以使用 `IF EXISTS` 选项来确保只有在表存在时才会删除。如果尝试使用 `IF EXISTS` 选项删除一个不存在的表，将不会发生任何操作。

通过 PHP 删除表时，您可以添加一个可选的 `silent` 参数，其作用与 `IF EXISTS` 相同。

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
DROP TABLE IF EXISTS products;
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```http
POST /cli -d "DROP TABLE IF EXISTS products"
```

<!-- intro -->
##### PHP:

<!-- request PHP -->

```php
$params =
[
  'table' => 'products',
  'body' => ['silent' => true]
];

$client->indices()->drop($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('DROP TABLE IF EXISTS products')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```
<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('DROP TABLE IF EXISTS products');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```
<!-- intro -->
##### java:

<!-- request Java -->

```java
sqlresult = utilsApi.sql("DROP TABLE IF EXISTS products");
```

<!-- response Java -->
```java
{total=0, error=, warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
sqlresult = utilsApi.Sql("DROP TABLE IF EXISTS products");
```

<!-- response C# -->
```clike
{total=0, error="", warning=""}
```

<!-- end -->
<!-- proofread -->
