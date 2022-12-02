# Deleting a table

<!-- example drop -->

Deleting a table is performed in 2 steps internally:
1. Table is cleared (similar to [TRUNCATE](Emptying_a_table.md))
2. All table files are removed from the table folder. All the external table files that were used by the table (such as wordforms, extensions or stopwords) are also deleted. Note that these external files are copied to the table folder when `CREATE TABLE` is used, so the original files specified in `CREATE TABLE` will not be deleted.

Deleting a table is possible only when the server is running in the RT mode. It is possible to delete RT tables, PQ tables and distributed tables.

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
##### HTTP:

<!-- request HTTP -->

```http
POST /cli -d "DROP TABLE products"
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
##### javascript:

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
<!-- end -->

Here is the syntax of the `DROP TABLE` statement in SQL:

```sql
DROP TABLE [IF EXISTS] index_name
```

<!-- example drop-if-exists -->

When deleting a table via SQL, adding `IF EXISTS` can be used to delete the table only if it exists. If you try to delete a non-existing table with the `IF EXISTS` option, nothing happens.

When deleting a table via PHP, you can add an optional `silent` parameter which works the same as `IF EXISTS`.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
DROP TABLE IF EXISTS products;
```

<!-- intro -->
##### HTTP:

<!-- request HTTP -->

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
##### javascript:

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

<!-- end -->
