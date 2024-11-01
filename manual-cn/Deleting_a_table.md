# 删除表

<!-- example drop -->

删除表的过程在内部执行分为两步：

1. 清空表（类似于 [TRUNCATE](Emptying_a_table.md) 操作）
2. 从表的文件夹中删除所有的表文件。所有由表使用的外部表文件（例如 wordforms、扩展或 stopwords）也会被删除。请注意，当使用 `CREATE TABLE` 创建表时，这些外部文件会被复制到表文件夹中，因此 `CREATE TABLE` 中指定的原始文件不会被删除。

删除表仅在服务器运行于 RT 模式时才可能。可以删除 RT 表、PQ 表以及分布式表。

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
DROP TABLE products;
```
<!-- response -->

```sql
Query OK, 0 rows affected (0.02 sec)
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
$params = [ 'index' => 'products' ];

$response = $client->indices()->drop($params);
```

<!-- response PHP -->
```php
Array
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

以下是 `DROP TABLE` 语句在 SQL 中的语法：

```sql
DROP TABLE [IF EXISTS] index_name
```

<!-- example drop-if-exists -->

通过 SQL 删除表时，可以使用 `IF EXISTS` 选项来确保仅在表存在时才删除。如果尝试删除一个不存在的表且使用了 `IF EXISTS` 选项，则不会发生任何操作。

通过 PHP 删除表时，你可以添加一个可选的 `silent` 参数，作用与 `IF EXISTS` 相同。

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
  'index' => 'products',
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