# Emptying a table

The table can be emptied with a `TRUNCATE TABLE` SQL statement or with a `truncate()` PHP client function.

Here is the syntax for the SQL statement:

```sql
TRUNCATE TABLE index_name [WITH RECONFIGURE]
```

<!-- example truncate -->

When this statement is executed, it clears the RT table completely. It disposes the in-memory data, unlinks all the table data files, and releases the associated binary logs.

A table can also be emptied with `DELETE FROM index WHERE id>0`, but it's not recommended as it's slower than `TRUNCATE`.

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
$params = [ 'index' => 'products' ];
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
utilsApi.sql("TRUNCATE TABLE products");
```

<!-- response Java -->
```java
{total=0, error=, warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("TRUNCATE TABLE products");
```

<!-- response C# -->
```clike
{total=0, error="", warning=""}
```

<!-- end -->

One of the possible uses of this command is before [attaching a table](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_a_plain_table_to_RT_table.md).

<!-- example truncate with RECONFIGURE -->

When `RECONFIGURE` option is used new tokenization, morphology, and other text processing settings specified in the config take effect after the table gets cleared. In case the [schema declaration](Creating_a_table/Data_types.md) in config is different from the table schema the new schema from config got applied after table get cleared.

With this option clearing and reconfiguring a table becomes one atomic operation.

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
$params = [ 'index' => 'products', 'with' => 'reconfigure' ];
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
utilsApi.sql("TRUNCATE TABLE products WITH RECONFIGURE");
```

<!-- response Java -->
```java
{total=0, error=, warning=}
```

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("TRUNCATE TABLE products WITH RECONFIGURE");
```

<!-- response C# -->
```clike
{total=0, error="", warning=""}
```
<!-- end -->
<!-- proofread -->