# Joining tables

Table joins in Manticore Search enable you to combine documents from two tables by matching related columns. This functionality allows for more complex queries and enhanced data retrieval across multiple tables.

## General syntax

### SQL

```sql
SELECT
	select_expr [, select_expr] ...
	FROM tbl_name
	{INNER | LEFT} JOIN tbl2_name
	ON join_condition
	[...other select options]

join_condition: {
	left_table.attr = right_table.attr
	| left_table.json_attr.string_id = string(right_table.json_attr.string_id)
	| left_table.json_attr.int_id = int(right_table.json_attr.int_id)
}
```

For more information on select options, refer to the [SELECT](../Searching/Intro.md#General-syntax) section.

<!--example join_sql_json_type -->

When joining by a value from a JSON attribute, you need to explicitly specify the value's type using the `int()` or `string()` function.

<!-- request String JSON attribute -->
```sql
SELECT ... ON left_table.json_attr.string_id = string(right_table.json_attr.string_id)
```

<!-- request Int JSON attribute -->
```sql
SELECT ... ON left_table.json_attr.int_id = int(right_table.json_attr.int_id)
```

<!-- end -->

### JSON

```json
POST /search
{
  "table": "table_name",
  "query": {
    <optional full-text query against the left table>
  },
  "join": [
    {
      "type": "inner" | "left",
      "table": "joined_table_name",
      "query": {
        <optional full-text query against the right table>
      },
      "on": [
        {
          "left": {
            "table": "left_table_name",
            "field": "field_name",
            "type": "<common field's type when joining using json attributes>"
          },
          "operator": "eq",
          "right": {
            "table": "right_table_name",
            "field": "field_name"
          }
        }
      ]
    }
  ],
  "options": {
    ...
  }
}

on.type: {
	int
	| string
}
```
Note, there is the `type` field in the `left` operand section which you should use when joining two tables using json attributes. The allowed values are `string` and `int`.

## Types of joins

Manticore Search supports two types of joins:

<!-- example inner_basic -->

1. **INNER JOIN**: Returns only the rows where there is a match in both tables. For example, the query performs an INNER JOIN between the `orders` and `customers` tables, including only the orders that have matching customers.

<!-- request SQL -->
```sql
SELECT product, customers.email, customers.name, customers.address
FROM orders
INNER JOIN customers
ON customers.id = orders.customer_id
WHERE MATCH('maple', customers)
ORDER BY customers.email ASC;
```

<!-- request JSON -->
```json
POST /search
{
  "table": "orders",
  "join": [
    {
      "type": "inner",
      "table": "customers",
      "query": {
        "query_string": "maple"
      },
      "on": [
        {
          "left": {
            "table": "orders",
            "field": "customer_id"
          },
          "operator": "eq",
          "right": {
            "table": "customers",
            "field": "id"
          }
        }
      ]
    }
  ],
  "_source": ["product", "customers.email", "customers.name", "customers.address"],
  "sort": [{"customers.email": "asc"}]
}
```

<!-- response SQL -->

```sql
+---------+-------------------+----------------+-------------------+
| product | customers.email   | customers.name | customers.address |
+---------+-------------------+----------------+-------------------+
| Laptop  | alice@example.com | Alice Johnson  | 123 Maple St      |
| Tablet  | alice@example.com | Alice Johnson  | 123 Maple St      |
+---------+-------------------+----------------+-------------------+
2 rows in set (0.00 sec)
```

<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "total_relation": "eq",
    "hits": [
      {
        "_id": 1,
        "_score": 1,
        "_source": {
          "product": "Laptop",
          "customers.email": "alice@example.com",
          "customers.name": "Alice Johnson",
          "customers.address": "123 Maple St"
        }
      },
      {
        "_id": 3,
        "_score": 1,
        "_source": {
          "product": "Tablet",
          "customers.email": "alice@example.com",
          "customers.name": "Alice Johnson",
          "customers.address": "123 Maple St"
        }
      }
    ]
  }
}
```
<!-- end -->

<!-- example left_basic -->

2. **LEFT JOIN**: Returns all rows from the left table and the matched rows from the right table. If there is no match, NULL values are returned for the right table's columns. For example, this query retrieves all customers along with their corresponding orders using a LEFT JOIN. If no corresponding order exists, NULL values will appear. The results are sorted by the customer's email, and only the customer's name and the order quantity are selected.

<!-- request SQL -->
```sql
SELECT
name, orders.quantity
FROM customers
LEFT JOIN orders
ON orders.customer_id = customers.id
ORDER BY email ASC;
```

<!-- request JSON -->
```json
POST /search
{
	"table": "customers",
	"_source": ["name", "orders.quantity"],
	"join": [
    {
      "type": "left",
      "table": "orders",
      "on": [
        {
          "left": {
            "table": "orders",
            "field": "customer_id"
          },
          "operator": "eq",
          "right": {
            "table": "customers",
            "field": "id"
          }
        }
      ]
    }
  ],
  "sort": [{"email": "asc"}]
}
```

<!-- response SQL -->
```
+---------------+-----------------+-------------------+
| name          | orders.quantity | @int_attr_email   |
+---------------+-----------------+-------------------+
| Alice Johnson |               1 | alice@example.com |
| Alice Johnson |               1 | alice@example.com |
| Bob Smith     |               2 | bob@example.com   |
| Carol White   |               1 | carol@example.com |
| John Smith    |            NULL | john@example.com  |
+---------------+-----------------+-------------------+
5 rows in set (0.00 sec)
```

<!-- response JSON -->

```
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 5,
    "total_relation": "eq",
    "hits": [
      {
        "_id": 1,
        "_score": 1,
        "_source": {
          "name": "Alice Johnson",
          "address": "123 Maple St",
          "email": "alice@example.com",
          "orders.id": 3,
          "orders.customer_id": 1,
          "orders.quantity": 1,
          "orders.order_date": "2023-01-03",
          "orders.tags": [
            101,
            104
          ],
          "orders.details": {
            "price": 450,
            "warranty": "1 year"
          },
          "orders.product": "Tablet"
        }
      },
      {
        "_id": 1,
        "_score": 1,
        "_source": {
          "name": "Alice Johnson",
          "address": "123 Maple St",
          "email": "alice@example.com",
          "orders.id": 1,
          "orders.customer_id": 1,
          "orders.quantity": 1,
          "orders.order_date": "2023-01-01",
          "orders.tags": [
            101,
            102
          ],
          "orders.details": {
            "price": 1200,
            "warranty": "2 years"
          },
          "orders.product": "Laptop"
        }
      },
      {
        "_id": 2,
        "_score": 1,
        "_source": {
          "name": "Bob Smith",
          "address": "456 Oak St",
          "email": "bob@example.com",
          "orders.id": 2,
          "orders.customer_id": 2,
          "orders.quantity": 2,
          "orders.order_date": "2023-01-02",
          "orders.tags": [
            103
          ],
          "orders.details": {
            "price": 800,
            "warranty": "1 year"
          },
          "orders.product": "Phone"
        }
      },
      {
        "_id": 3,
        "_score": 1,
        "_source": {
          "name": "Carol White",
          "address": "789 Pine St",
          "email": "carol@example.com",
          "orders.id": 4,
          "orders.customer_id": 3,
          "orders.quantity": 1,
          "orders.order_date": "2023-01-04",
          "orders.tags": [
            105
          ],
          "orders.details": {
            "price": 300,
            "warranty": "1 year"
          },
          "orders.product": "Monitor"
        }
      },
      {
        "_id": 4,
        "_score": 1,
        "_source": {
          "name": "John Smith",
          "address": "15 Barclays St",
          "email": "john@example.com",
          "orders.id": 0,
          "orders.customer_id": 0,
          "orders.quantity": 0,
          "orders.order_date": "",
          "orders.tags": [],
          "orders.details": null,
          "orders.product": ""
        }
      }
    ]
  }
}
```

<!-- end -->

## Full-text matching across joined tables

One of the powerful features of table joins in Manticore Search is the ability to perform full-text searches on both the left and right tables simultaneously. This allows you to create complex queries that filter based on text content in multiple tables.

<!-- example fulltext_basic -->

You can use separate `MATCH()` functions for each table in your JOIN query. The query filters results based on text content in both tables.

<!-- request SQL -->
```sql
SELECT t1.f, t2.f 
FROM t1 
LEFT JOIN t2 ON t1.id = t2.id 
WHERE MATCH('hello', t1) AND MATCH('goodbye', t2);
```

<!-- request JSON -->
```json
POST /search
{
  "table": "t1",
  "query": {
    "query_string": "hello"
  },
  "join": [
    {
      "type": "left",
      "table": "t2",
      "query": {
        "query_string": "goodbye"
      },
      "on": [
        {
          "left": {
            "table": "t1",
            "field": "id"
          },
          "operator": "eq",
          "right": {
            "table": "t2",
            "field": "id"
          }
        }
      ]
    }
  ],
  "_source": ["f", "t2.f"]
}
```

<!-- response SQL -->

```sql
+-------------+---------------+
| f           | t2.f          |
+-------------+---------------+
| hello world | goodbye world |
+-------------+---------------+
1 row in set (0.00 sec)
```

<!-- response JSON -->

```json
{
  "took": 1,
  "timed_out": false,
  "hits": {
    "total": 1,
    "total_relation": "eq",
    "hits": [
      {
        "_id": 2,
        "_score": 1680,
        "t2._score": 1680,
        "_source": {
          "f": "hello world",
          "t2.f": "goodbye world"
        }
      }
    ]
  }
}
```

<!-- end -->

### JSON query structure for joins

In JSON API queries, table-specific full-text matching is structured differently than SQL:

<!-- example fulltext_json_structure -->

**Main table query**: The `"query"` field at the root level applies to the main table (specified in `"table"`).

**Joined table query**: Each join definition can include its own `"query"` field that applies specifically to that joined table.

<!-- request JSON -->
```json
POST /search
{
  "table": "t1",
  "query": {
    "query_string": "hello"
  },
  "join": [
    {
      "type": "left",
      "table": "t2",
      "query": {
        "match": {
          "*": "goodbye"
        }
      },
      "on": [
        {
          "left": {
            "table": "t1",
            "field": "id"
          },
          "operator": "eq",
          "right": {
            "table": "t2",
            "field": "id"
          }
        }
      ]
    }
  ]
}
```

<!-- response JSON -->

```json
{
  "took": 1,
  "timed_out": false,
  "hits": {
    "total": 1,
    "total_relation": "eq",
    "hits": [
      {
        "_id": 1,
        "_score": 1680,
        "t2._score": 1680,
        "_source": {
          "f": "hello world",
          "t2.id": 1,
          "t2.f": "goodbye world"
        }
      }
    ]
  }
}
```

<!-- end -->

### Understanding query behavior in JOIN operations

<!-- example fulltext_json_behavior -->

**1. Query on main table only**: Returns all matching rows from the main table. For unmatched joined records (LEFT JOIN), SQL returns NULL values while JSON API returns default values (0 for numbers, empty strings for text).

<!-- request SQL -->
```sql
SELECT * FROM t1 
LEFT JOIN t2 ON t1.id = t2.id 
WHERE MATCH('database', t1);
```

<!-- response SQL -->

```sql
+------+-----------------+-------+------+
| id   | f               | t2.id | t2.f |
+------+-----------------+-------+------+
|    3 | database search |  NULL | NULL |
+------+-----------------+-------+------+
1 row in set (0.00 sec)
```

<!-- request JSON -->
```json
POST /search
{
  "table": "t1",
  "query": {
    "query_string": "database"
  },
  "join": [
    {
      "type": "left",
      "table": "t2",
      "on": [
        {
          "left": {
            "table": "t1",
            "field": "id"
          },
          "operator": "eq",
          "right": {
            "table": "t2",
            "field": "id"
          }
        }
      ]
    }
  ]
}
```

<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "total_relation": "eq",
    "hits": [
      {
        "_id": 3,
        "_score": 1680,
        "t2._score": 0,
        "_source": {
          "f": "database search",
          "t2.id": 0,
          "t2.f": ""
        }
      }
    ]
  }
}
```
<!--end -->

<!-- example fulltext_json_behavior_2 -->
**2. Query on joined table acts as filter**: When a joined table has a query, only records matching both the join condition AND the query condition are returned.

<!-- request JSON -->
```json
POST /search
{
  "table": "t1",
  "query": {
    "query_string": "database"
  },
  "join": [
    {
      "type": "left",
      "table": "t2",
      "query": {
        "query_string": "nonexistent"
      },
      "on": [
        {
          "left": {
            "table": "t1",
            "field": "id"
          },
          "operator": "eq",
          "right": {
            "table": "t2",
            "field": "id"
          }
        }
      ]
    }
  ]
}
```

<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 0,
    "total_relation": "eq",
    "hits": []
  }
}
```
<!-- end -->

**3. JOIN type affects filtering**: INNER JOIN requires both join and query conditions to be satisfied, while LEFT JOIN returns matching left table rows even when right table conditions fail.


### Important considerations for full-text matching in JOINs

When using full-text matching with joins, keep these points in mind:

1. **Table-specific matching**: 
   - **SQL**: Each `MATCH()` function should specify which table to search in: `MATCH('term', table_name)`
   - **JSON**: Use the root-level `"query"` for the main table and `"query"` within each join definition for joined tables

2. **Query syntax flexibility**: JSON API supports both `"query_string"` and `"match"` syntaxes for full-text queries

3. **Performance implications**: Full-text matching on both tables may impact query performance, especially with large datasets. Consider using appropriate indexes and batch sizes.

4. **NULL/default value handling**: With LEFT JOIN, if there's no matching record in the right table, full-text conditions on the right table won't be evaluated. SQL returns NULL values while JSON API returns default values (0 for numbers, empty strings for text).

5. **Filtering behavior**: Queries on joined tables act as filters - they restrict results to records that satisfy both join and query conditions.

6. **Full-text operator support**: All [full-text operators](../Full_text_matching/Operators.md) are supported in JOIN queries, including phrase, proximity, field search, NEAR, quorum matching, and advanced operators.

7. **Score calculation**: Each table maintains its own relevance score, accessible via `table_name.weight()` in SQL or `table_name._score` in JSON responses.

## Example: Complex JOIN with faceting

Building on the previous examples, let's explore a more advanced scenario where we combine table joins with faceting and full-text matching across multiple tables. This demonstrates the full power of Manticore's JOIN capabilities with complex filtering and aggregation.

<details>

Init queries for the following example:

```
drop table if exists customers; drop table if exists orders; create table customers(name text, email text, address text); create table orders(product text, customer_id int, quantity int, order_date string, tags multi, details json); insert into customers values (1, 'Alice Johnson', 'alice@example.com', '123 Maple St'), (2, 'Bob Smith', 'bob@example.com', '456 Oak St'), (3, 'Carol White', 'carol@example.com', '789 Pine St'), (4, 'John Smith', 'john@example.com', '15 Barclays St'); insert into orders values (1, 'Laptop Computer', 1, 1, '2023-01-01', (101,102), '{"price":1200,"warranty":"2 years"}'), (2, 'Smart Phone', 2, 2, '2023-01-02', (103), '{"price":800,"warranty":"1 year"}'), (3, 'Tablet Device', 1, 1, '2023-01-03', (101,104), '{"price":450,"warranty":"1 year"}'), (4, 'Monitor Display', 3, 1, '2023-01-04', (105), '{"price":300,"warranty":"1 year"}');
```

</details>

<!-- example basic_complex -->

This query demonstrates full-text matching across both the `customers` and `orders` tables, combined with range filtering and faceting. It searches for customers named "Alice" or "Bob" and their orders containing "laptop", "phone", or "tablet" with prices above $500. The results are ordered by order ID and faceted by warranty terms.

<!-- request SQL -->
```sql
SELECT orders.product, name, orders.details.price, orders.tags
FROM customers
LEFT JOIN orders ON customers.id = orders.customer_id
WHERE orders.details.price > 500
AND MATCH('laptop | phone | tablet', orders)
AND MATCH('alice | bob', customers)
ORDER BY orders.id ASC
FACET orders.details.warranty;
```

<!-- request JSON -->
```json
POST /search
{
  "table": "customers",
  "query": {
    "query_string": "alice | bob"
  },
  "join": [
    {
      "type": "left",
      "table": "orders",
      "query": {
        "bool": {
          "must": [
            {
              "range": {
                "details.price": {
                  "gt": 500
                }
              }
            },
            {
              "query_string": "laptop | phone | tablet"
            }
          ]
        }
      },
      "on": [
        {
          "left": {
            "table": "customers",
            "field": "id"
          },
          "operator": "eq",
          "right": {
            "table": "orders",
            "field": "customer_id"
          }
        }
      ]
    }
  ],
  "_source": ["orders.product", "name", "orders.details.price", "orders.tags"],
  "sort": [{"orders.id": "asc"}],
  "aggs": {
    "warranty_facet": {
      "terms": {
        "field": "orders.details.warranty"
      }
    }
  }
}
```

<!-- response SQL -->
```sql
+-----------------+---------------+----------------------+-------------+
| orders.product  | name          | orders.details.price | orders.tags |
+-----------------+---------------+----------------------+-------------+
| Laptop Computer | Alice Johnson |                 1200 | 101,102     |
| Smart Phone     | Bob Smith     |                  800 | 103         |
+-----------------+---------------+----------------------+-------------+
2 rows in set (0.00 sec)

+-------------------------+----------+
| orders.details.warranty | count(*) |
+-------------------------+----------+
| 2 years                 |        1 |
| 1 year                  |        1 |
+-------------------------+----------+
2 rows in set (0.00 sec)
```

<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 3,
    "total_relation": "eq",
    "hits": [
      {
        "_id": 1,
        "_score": 1,
        "orders._score": 1565,
        "_source": {
          "name": "Alice Johnson",
          "orders.tags": [
            101,
            102
          ],
          "orders.product": "Laptop Computer"
        }
      },
      {
        "_id": 2,
        "_score": 1,
        "orders._score": 1565,
        "_source": {
          "name": "Bob Smith",
          "orders.tags": [
            103
          ],
          "orders.product": "Smart Phone"
        }
      },
      {
        "_id": 1,
        "_score": 1,
        "orders._score": 1565,
        "_source": {
          "name": "Alice Johnson",
          "orders.tags": [
            101,
            104
          ],
          "orders.product": "Tablet Device"
        }
      }
    ]
  },
  "aggregations": {
    "warranty_facet": {
      "buckets": [
        {
          "key": "2 years",
          "doc_count": 1
        },
        {
          "key": "1 year",
          "doc_count": 2
        }
      ]
    }
  }
}
```


<!-- end -->

## Search options and match weights

Separate options can be specified for queries in a join: for the left table and the right table. The syntax is `OPTION(<table_name>)` for SQL queries and one or more subobjects under `"options"` for JSON queries.


<!-- example join_options -->

Here's an example of how to specify different field weights for a full-text query on the right table. To retrieve match weights via SQL, use the `<table_name>.weight()` expression.
In JSON queries, this weight is represented as `<table_name>._score`.

<!-- request SQL -->
```sql
SELECT product, customers.email, customers.name, customers.address, customers.weight()
FROM orders
INNER JOIN customers
ON customers.id = orders.customer_id
WHERE MATCH('maple', customers)
OPTION(customers) field_weights=(address=1500);
```

<!-- request JSON -->
```json
POST /search
{
  "table": "orders",
  "options": {
    "customers": {
      "field_weights": {
        "address": 1500
      }
    }
  },
  "join": [
    {
      "type": "inner", 
      "table": "customers",
      "query": {
        "query_string": "maple"
      },
      "on": [
        {
          "left": {
            "table": "orders",
            "field": "customer_id"
          },
          "operator": "eq",
          "right": {
            "table": "customers",
            "field": "id"
          }
        }
      ]
    }
  ],
  "_source": ["product", "customers.email", "customers.name", "customers.address"]
}
```

<!-- response SQL -->

```sql
+---------+-------------------+----------------+-------------------+--------------------+
| product | customers.email   | customers.name | customers.address | customers.weight() |
+---------+-------------------+----------------+-------------------+--------------------+
| Laptop  | alice@example.com | Alice Johnson  | 123 Maple St      |            1500680 |
| Tablet  | alice@example.com | Alice Johnson  | 123 Maple St      |            1500680 |
+---------+-------------------+----------------+-------------------+--------------------+
2 rows in set (0.00 sec)
```

<!-- response JSON -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "total_relation": "eq",
    "hits": [
      {
        "_id": 1,
        "_score": 1,
        "customers._score": 15000680,
        "_source": {
          "product": "Laptop",
          "customers.email": "alice@example.com",
          "customers.name": "Alice Johnson",
          "customers.address": "123 Maple St"
        }
      },
      {
        "_id": 3,
        "_score": 1,
        "customers._score": 15000680,
        "_source": {
          "product": "Tablet",
          "customers.email": "alice@example.com",
          "customers.name": "Alice Johnson",
          "customers.address": "123 Maple St"
        }
      }
    ]
  }
}
```
<!-- end -->

## Join batching

When performing table joins, Manticore Search processes the results in batches to optimize performance and resource usage. Here's how it works:

- **How Batching Works**: 
  - The query on the left table is executed first, and the results are accumulated into a batch.
  - This batch is then used as input for the query on the right table, which is executed as a single operation.
  - This approach minimizes the number of queries sent to the right table, improving efficiency.

- **Configuring Batch Size**:
  - The size of the batch can be adjusted using the `join_batch_size` search option.
  - It is also [configurable](../Server_settings/Searchd.md#join_batch_size) in the `searchd` section of the configuration file.
  - The default batch size is `1000`, but you can increase or decrease it depending on your use case.
  - Setting `join_batch_size=0` disables batching entirely, which may be useful for debugging or specific scenarios.

- **Performance Considerations**:
  - A larger batch size can improve performance by reducing the number of queries executed on the right table.
  - However, larger batches may consume more memory, especially for complex queries or large datasets.
  - Experiment with different batch sizes to find the optimal balance between performance and resource usage.

## Join caching

To further optimize join operations, Manticore Search employs a caching mechanism for queries executed on the right table. Here's what you need to know:

- **How Caching Works**:
  - Each query on the right table is defined by the `JOIN ON` conditions.
  - If the same `JOIN ON` conditions are repeated across multiple queries, the results are cached and reused.
  - This avoids redundant queries and speeds up subsequent join operations.

- **Configuring Cache Size**:
  - The size of the join cache can be configured using the [join_cache_size](../Server_settings/Searchd.md#join_cache_size) option in the `searchd` section of the configuration file.
  - The default cache size is `20MB`, but you can adjust it based on your workload and available memory.
  - Setting `join_cache_size=0` disables caching entirely.

- **Memory Considerations**:
  - Each thread maintains its own cache, so the total memory usage depends on the number of threads and the cache size.
  - Ensure your server has sufficient memory to accommodate the cache, especially for high-concurrency environments.

## Joining distributed tables

Distributed tables consisting only of local tables are supported on both the left and right sides of a join query. However, distributed tables that include remote tables are not supported.

## Caveats and best practices

When using JOINs in Manticore Search, keep the following points in mind:

1. **Field selection**: When selecting fields from two tables in a JOIN, do not prefix fields from the left table, but do prefix fields from the right table. For example:
   ```sql
   SELECT field_name, right_table.field_name FROM ...
   ```

2. **JOIN conditions**: Always explicitly specify the table names in your JOIN conditions:
   ```sql
   JOIN ON table_name.some_field = another_table_name.some_field
   ```

3. **Expressions with JOINs**: When using expressions that combine fields from both joined tables, alias the result of the expression:
   ```sql
   SELECT *, (nums2.n + 3) AS x, x * n FROM nums LEFT JOIN nums2 ON nums2.id = nums.num2_id
   ```

4. **Filtering on aliased expressions**: You cannot use aliases for expressions involving fields from both tables in the WHERE clause.

5. **JSON attributes**: When joining on JSON attributes, you must explicitly cast the values to the appropriate type:
   ```sql
   -- Correct:
   SELECT * FROM t1 LEFT JOIN t2 ON int(t1.json_attr.id) = t2.json_attr.id
   
   -- Incorrect:
   SELECT * FROM t1 LEFT JOIN t2 ON t1.json_attr.id = t2.json_attr.id
   ```

6. **NULL handling**: You can use IS NULL and IS NOT NULL conditions on joined fields:
   ```sql
   SELECT * FROM t1 LEFT JOIN t2 ON t1.id = t2.id WHERE t2.name IS NULL
   SELECT * FROM t1 LEFT JOIN t2 ON t1.id = t2.id WHERE t2.name IS NOT NULL
   ```

7. **Using ANY with MVA**: When using the `ANY()` function with multi-valued attributes in JOINs, alias the multi-valued attribute from the joined table:
   ```sql
   SELECT *, t2.m AS alias
   FROM t
   LEFT JOIN t2 ON t.id = t2.t_id
   WHERE ANY(alias) IN (3, 5)
   ```

By following these guidelines, you can effectively use JOINs in Manticore Search to combine data from multiple indexes and perform complex queries.

<!-- proofread -->
