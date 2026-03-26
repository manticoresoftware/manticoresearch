# 删除表

<!-- example drop -->

删除表在内部执行分两个步骤：
1. 清空表（类似于 [TRUNCATE](Emptying_a_table.md)）
2. 删除表文件夹中的所有表文件。所有表使用的外部表文件（例如词形变化、扩展或停用词）也会被删除。注意，当使用 `CREATE TABLE` 时，这些外部文件会被复制到表文件夹，因此不会删除 `CREATE TABLE` 中指定的原始文件。

只有在服务器以 RT 模式运行时才可以删除表。可以删除 RT 表、PQ 表和分布式表。

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
$params = [ 'table' => 'products' ];

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
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('DROP TABLE products')
```

<!-- response Python-asyncio -->
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
sqlresult = utilsApi.sql("DROP TABLE products", true);
```

<!-- response Java -->
```java
{total=0, error=, warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
sqlresult = utilsApi.Sql("DROP TABLE products", true);
```

<!-- response C# -->
```clike
{total=0, error="", warning=""}
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
let sqlresult = utils_api.sql("DROP TABLE products", Some(true)).await;
```

<!-- response Rust -->
```rust
{total=0, error="", warning=""}
```

<!-- end -->

以下是 SQL 中 `DROP TABLE` 语句的语法：

```sql
DROP TABLE [IF EXISTS] table_name
```

<!-- example drop-if-exists -->

通过 SQL 删除表时，添加 `IF EXISTS` 可以仅在表存在时删除该表。如果尝试删除不存在的表且使用了 `IF EXISTS` 选项，则不会有任何操作。

通过 PHP 删除表时，可以添加可选的 `silent` 参数，其作用与 `IF EXISTS` 相同。

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
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('DROP TABLE IF EXISTS products')
```

<!-- response Python-asyncio -->
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
sqlresult = utilsApi.sql("DROP TABLE IF EXISTS products", true);
```

<!-- response Java -->
```java
{total=0, error=, warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
sqlresult = utilsApi.Sql("DROP TABLE IF EXISTS products", true);
```

<!-- response C# -->
```clike
{total=0, error="", warning=""}
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
let sqlresult = utils_api.sql("DROP TABLE IF EXISTS products", Some(true)).await;
```

<!-- response Rust -->
```rust
{total=0, error="", warning=""}
```

<!-- end -->
<!-- proofread -->

