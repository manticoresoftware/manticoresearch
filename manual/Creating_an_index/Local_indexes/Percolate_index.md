# Percolate index

<!-- example pq -->
Percolate index is a special index which stores queries instead of documents. It is used for prospective searches (or "search in reverse").

* See section [Percolate query](../../Searching/Percolate_query.md) for more details about performing a search query against a percolate index.
* See section [Adding rules to a percolate index](../../Adding_documents_to_an_index/Adding_rules_to_a_percolate_index.md) to learn how to prepare an index for searching.

The schema of a percolate index is fixed and contains the following fields:

| Field | Description |
| - | - |
| ID| Unsigned 64-bit integer with autoincrement functionality therefore it can be omitted when you [add a PQ rule](../../Adding_documents_to_an_index/Adding_rules_to_a_percolate_index.md) |
| Query | [Full-text query](../../Searching/Full_text_matching/Basic_usage.md) of the rule. You can think of it as the value of a [MATCH clause](../../Searching/Full_text_matching/Basic_usage.md) or [JSON /search](../../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON). If [per field operators](../../Searching/Full_text_matching/Operators.md) are used inside the query, the full text fields need to be declared in the percolate index configuration. If the stored query is supposed to do only attribute filtering (no full-text querying), the query value can be empty or simply omitted. The value of this field should correspond to the expected document schema which you specify when you create a percolate index |
| Filters | Filters is an optional string containing attribute filters and/or expressions the same way they are defined in the [WHERE clause](../../Searching/Filters.md#WHERE) or [JSON filtering](../../Searching/Filters.md#HTTP-JSON). The value of this field should correspond to the expected document schema which you specify when you create a percolate index |
| Tags | Optional. Tags represent a list of string labels separated by comma that can be used for filtering/deleting PQ rules. The tags can be returned along with matching documents when you [Percolate query](../../Searching/Percolate_query.md) |

You don't need to worry about adding the above fields when you create a percolate index.

What you need to take care of when you add a new index is to **specify the expected schema of a document** which is to be checked against the rules you will add later. This is done the same way as for [any other local index](../../Creating_an_index/Local_indexes.md).


<!-- intro -->
##### SQL:

<!-- request SQL -->
##### Creating a percolate index via MySQL protocol:

```sql
CREATE TABLE products(title text, meta json) type='pq';
```
<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- request HTTP -->
##### Creating a percolate index via JSON over HTTP:

```json
POST /cli -d "CREATE TABLE products(title text, meta json) type='pq'"
```

<!-- response HTTP -->

```json
{
"total":0,
"error":"",
"warning":""
}
```

<!-- request PHP -->
##### Creating a percolate index via PHP client:

```php
$index = [
    'index' => 'products',
    'body' => [
        'columns' => [
            'title' => ['type' => 'text'],
            'meta' => ['type' => 'json']
        ],
        'settings' => [
            'type' => 'pq'
        ]
    ]
];
$client->indices()->create($index);
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
utilsApi.sql('CREATE TABLE products(title text, meta json) type=\'pq\'')
```
<!-- intro -->
##### javascript:

<!-- request javascript -->

```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, meta json) type=\'pq\'');
```
<!-- intro -->
##### java:

<!-- request java -->

```java
utilsApi.sql("CREATE TABLE products(title text, meta json) type='pq'");
```
<!-- request CONFIG -->
##### Creating a percolate index via config:

```ini
index products {
  type = percolate
  path = idx_pq
  rt_field = title
  rt_attr_json = meta
}
```
<!-- end -->
