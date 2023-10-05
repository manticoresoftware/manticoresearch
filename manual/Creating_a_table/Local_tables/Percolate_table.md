# Percolate table

<!-- example pq -->
A percolate table is a special table that stores queries rather than documents. It is used for prospective searches, or "search in reverse."

* To learn more about performing a search query against a percolate table, see the section [Percolate query](../../Searching/Percolate_query.md).
* To learn how to prepare a table for searching, see the section [Adding rules to a percolate table](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md).

The schema of a percolate table is fixed and contains the following fields:

| Field | Description |
| - | - |
| ID| An unsigned 64-bit integer with auto-increment functionality. It can be omitted when adding a PQ rule, as described in  [add a PQ rule](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md) |
| Query | [Full-text query](../../Searching/Full_text_matching/Basic_usage.md) of the rule, which can be thought of as the value of [MATCH clause](../../Searching/Full_text_matching/Basic_usage.md) or [JSON /search](../../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON). If [per field operators](../../Searching/Full_text_matching/Operators.md) are used inside the query, the full-text fields need to be declared in the percolate table configuration. If the stored query is only for attribute filtering (without full-text querying), the query value can be empty or omitted. The value of this field should correspond to the expected document schema, which is specified when creating the percolate table. |
| Filters | Optional. Filters are an optional string containing attribute filters and/or expressions, defined the same way as in the [WHERE clause](../../Searching/Filters.md#WHERE) or [JSON filtering](../../Searching/Filters.md#HTTP-JSON). The value of this field should correspond to the expected document schema, which is specified when creating the percolate table. |
| Tags | Optional. Tags represent a list of string labels separated by commas that can be used for filtering/deleting PQ rules. The tags can also be returned along with matching documents when performing a [Percolate query](../../Searching/Percolate_query.md) |

Note that you do not need to add the above fields when creating a percolate table.

What you need to keep in mind when creating a new percolate table is to specify the expected schema of a document, which will be checked against the rules you will add later. This is done in the same way as for [any other local table](../../Creating_a_table/Local_tables.md).


<!-- intro -->
##### SQL:

<!-- request SQL -->
##### Creating a percolate table via MySQL protocol:

```sql
CREATE TABLE products(title text, meta json) type='pq';
```
<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- request JSON -->
##### Creating a percolate table via JSON over HTTP:

```json
POST /cli -d "CREATE TABLE products(title text, meta json) type='pq'"
```

<!-- response JSON -->

```json
{
"total":0,
"error":"",
"warning":""
}
```

<!-- request PHP -->
##### Creating a percolate table via PHP client:

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
##### Javascript:

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

<!-- intro -->
##### C#:

<!-- request C# -->

```clike
utilsApi.Sql("CREATE TABLE products(title text, meta json) type='pq'");
```
<!-- request CONFIG -->
##### Creating a percolate table via config:

```ini
table products {
  type = percolate
  path = tbl_pq
  rt_field = title
  rt_attr_json = meta
}
```
<!-- end -->
<!-- proofread -->