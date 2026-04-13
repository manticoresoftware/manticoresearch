# 清空表

<!-- example truncate -->

表可以通过 `TRUNCATE TABLE` SQL 语句或 `truncate()` PHP 客户端函数来清空。

以下是 SQL 语句的语法：

```sql
TRUNCATE TABLE table_name [WITH RECONFIGURE]
```

执行此语句时，会完全清除 RT 或分布式表。它会处理内存中的数据，解除所有表数据文件的链接，并释放相关的二进制日志。

对于清空分布式表，请使用不带 `with reconfigure` 选项的语法。只需对您的分布式表执行标准的 TRUNCATE 语句即可。

```sql
TRUNCATE TABLE distributed_table
```

> 注意：清空分布式表需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果无法使用，请确保 Buddy 已安装。

也可以使用 `DELETE FROM index WHERE id>0` 来清空表，但不推荐这样做，因为它比 `TRUNCATE` 慢。

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
TRUNCATE TABLE products;
```
<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.02 sec)
```

<!-- intro -->
##### JSON:

<!-- request JSON -->

```http
POST /cli -d "TRUNCATE TABLE products"
```

<!-- response JSON -->
```http
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
$response = $client->indices()->truncate($params);
```

<!-- response PHP -->
```php
Array(
    [total] => 0
    [error] =>
    [warning] =>
)
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('TRUNCATE TABLE products')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('TRUNCATE TABLE products')
```

<!-- response Python-asyncio -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('TRUNCATE TABLE products');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("TRUNCATE TABLE products", true);
```

<!-- response Java -->
```java
{total=0, error=, warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("TRUNCATE TABLE products", true);
```

<!-- response C# -->
```clike
{total=0, error="", warning=""}
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("TRUNCATE TABLE products", Some(true)).await;
```

<!-- response Rust -->
```rust
{total=0, error="", warning=""}
```

<!-- end -->

此命令的一个可能用途是在 [附加表](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md) 之前。

<!-- example truncate with RECONFIGURE -->

当使用 `RECONFIGURE` 选项时，清空表后，配置中指定的新分词、形态学和其他文本处理设置将生效。如果配置中的 [schema 声明](Creating_a_table/Data_types.md) 与表的 schema 不同，表清空后将应用配置中的新 schema。

> 注意：`RECONFIGURE` 选项仅在 [Plain 模式](Read_this_first.md#Real-time-mode-vs-plain-mode) 下有意义，它应用配置文件中的设置。请注意，`TRUNCATE` 仅支持 RT 表，且 `RECONFIGURE` 选项仅能在 Manticore 以 Plain 模式运行的 RT 表上使用。

使用此选项时，清空和重新配置表将成为一个原子操作。

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
TRUNCATE TABLE products with reconfigure;
```
<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.02 sec)
```

<!-- intro -->
##### JSON:

<!-- request HTTP -->

```http
POST /cli -d "TRUNCATE TABLE products with reconfigure"
```

<!-- response HTTP -->
```http
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
$params = [ 'table' => 'products', 'with' => 'reconfigure' ];
$response = $client->indices()->truncate($params);
```

<!-- response PHP -->
```php
Array(
    [total] => 0
    [error] =>
    [warning] =>
)
```
<!-- intro -->
##### Python:

<!-- request Python -->

```python
utilsApi.sql('TRUNCATE TABLE products WITH RECONFIGURE')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->

```python
await utilsApi.sql('TRUNCATE TABLE products WITH RECONFIGURE')
```

<!-- response Python-asyncio -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- intro -->
##### Javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('TRUNCATE TABLE products WITH RECONFIGURE');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("TRUNCATE TABLE products WITH RECONFIGURE", true);
```

<!-- response Java -->
```java
{total=0, error=, warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("TRUNCATE TABLE products WITH RECONFIGURE" ,true);
```

<!-- response C# -->
```clike
{total=0, error="", warning=""}
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("TRUNCATE TABLE products WITH RECONFIGURE", Some(true)).await;
```

<!-- response C# -->
```clike
{total=0, error="", warning=""}
```

<!-- end -->
<!-- proofread -->

