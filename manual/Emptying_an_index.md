# Emptying an index

The index can be emptied with a `TRUNCATE TABLE` SQL statement or with a `truncate()` PHP client function.

Here is the syntax for the SQL statement:

```sql
TRUNCATE TABLE index_name [WITH RECONFIGURE]
```

<!-- example truncate -->

When this statement is executed, it clears the RT index completely. It disposes the in-memory data, unlinks all the index data files, and releases the associated binary logs.

An index can also be emptied with `DELETE FROM index WHERE id>0`, but it's not recommended as it's much slower than `TRUNCATE`.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
truncate table products;
```
<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.02 sec)
```

<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```http
POST /sql -d "mode=raw&query=truncate table products"
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
utilsApi.sql('mode=raw&query=TRUNCATE TABLE products')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('mode=raw&query=TRUNCATE TABLE products');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("mode=raw&query=TRUNCATE TABLE products");
```

<!-- response Java -->
```java
{total=0, error=, warning=}
```

<!-- end -->

One of the possible uses of this command is before [attaching an index](Adding_data_from_external_storages/Adding_data_from_indexes/Attaching_a_plain_index_to_RT_index.md).

<!-- example truncate with RECONFIGURE -->

When `RECONFIGURE` option is used new tokenization, morphology, and other text processing settings specified in the config take effect after the index gets cleared. With this option clearing and reconfiguring an index becomes one atomic operation.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
truncate table products with reconfigure;
```
<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.02 sec)
```

<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```http
POST /sql -d "mode=raw&query=truncate table products with reconfigure"
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
utilsApi.sql('mode=raw&query=TRUNCATE TABLE products WITH RECONFIGURE')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('mode=raw&query=TRUNCATE TABLE products WITH RECONFIGURE');
```

<!-- response javascript -->
```javascript
{"total":0,"error":"","warning":""}
```

<!-- intro -->
##### java:

<!-- request Java -->

```java
utilsApi.sql("mode=raw&query=TRUNCATE TABLE products WITH RECONFIGURE");
```

<!-- response Java -->
```java
{total=0, error=, warning=}
```
<!-- end -->
