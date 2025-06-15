# 清空表

<!-- example truncate -->

表可以通过 `TRUNCATE TABLE` SQL 语句或 `truncate()` PHP 客户端函数清空。

以下是 SQL 语句的语法：

```sql
TRUNCATE TABLE table_name [WITH RECONFIGURE]
```

当执行此语句时，它会完全清除 RT 表或分布式表。它会处理内存中的数据，解除所有表数据文件的链接，并释放相关的二进制日志。

对于清空分布式表，使用不带 `with reconfigure` 选项的语法。只需对分布式表执行标准的 TRUNCATE 语句即可。

```sql
TRUNCATE TABLE distributed_table
```

> 注意：清空分布式表需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果不起作用，请确保已安装 Buddy。

也可以用 `DELETE FROM index WHERE id>0` 清空表，但不推荐这样做，因为它比 `TRUNCATE` 慢。

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

该命令的一个可能用途是在[附加一个表](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md)之前。

<!-- example truncate with RECONFIGURE -->

当使用 `RECONFIGURE` 选项时，配置中指定的新分词、形态学和其他文本处理设置在表被清空后生效。如果配置中的[模式声明](Creating_a_table/Data_types.md)与表的模式不同，表被清空后配置中的新模式将被应用。

> 注意：`RECONFIGURE` 选项仅在[纯模式](Read_this_first#Real-time-mode-vs-plain-mode)下有意义，它应用来自配置文件的设置。注意，`TRUNCATE` 只支持 RT 表，且 `RECONFIGURE` 选项只能在 Manticore 以纯模式运行时用于 RT 表。

使用此选项，清空和重新配置表变为一个原子操作。

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

