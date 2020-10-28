# Deleting an index

<!-- example drop -->

Deleting an index is performed in 2 steps:
1. Index is cleared (similar to [TRUNCATE](Emptying_an_index.md))
2. All index files are removed from the index folder. All the external index files that were used by the index (such as wordforms, extensions or stopwords) are also deleted. Note that these external files are copied to index folder when `CREATE TABLE` is used, so the original files specified in `CREATE TABLE` will not be deleted.

Deleting an index is possible only when the server is running in RT mode. It is possible to delete RT indexes, PQ indexes and distributed indexes.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
drop table products;
```
<!-- response -->

```sql
Query OK, 0 rows affected (0.02 sec)
```

<!-- intro -->
##### HTTP:

<!-- request HTTP -->

```http
POST /sql -d "mode=raw&query=drop table products"
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

```php
utilsApi.sql('mode=raw&query=DROP TABLE products')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```

<!-- end -->

Here is the syntax of the `DROP TABLE` statement in SQL:

```sql
DROP TABLE [IF EXISTS] index_name
```

<!-- example drop-if-exists -->

When deleting an index via SQL, adding `IF EXISTS` can be used to delete the index only if it exists. If you try to delete a non-existing index with the `IF EXISTS` option, nothing happens.

When deleting an index via PHP, you can add an optional `silent` parameter which works the same as `IF EXISTS`.

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
POST /sql -d "mode=raw&query=DROP TABLE IF EXISTS products"
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

```php
utilsApi.sql('mode=raw&query=DROP TABLE IF EXISTS products')
```

<!-- response Python -->
```python
{u'error': u'', u'total': 0, u'warning': u''}
```
<!-- end -->
