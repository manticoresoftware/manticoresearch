# Joining tables

**WARNING: This functionality is in beta stage. Use it with caution.**

Table joins in Manticore Search enable you to combine documents from two tables by matching related columns. This functionality allows for more complex queries and enhanced data retrieval across multiple tables.

## General syntax

### SQL

<!--example join_sql_json_type -->

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
  "index": "table_name",
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

## Types of Joins

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
  "index": "orders",
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
	"index": "customers",
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

### Complex Join with Faceting

<!-- example basic_complex -->

This query retrieves products, customer names, product prices, and product tags from the `orders` and `customers` tables. It performs a `LEFT JOIN`, ensuring all customers are included even if they have not made an order. The query filters the results to include only orders with a price greater than `500` and matches the products to the terms 'laptop', 'phone', or 'monitor'. The results are ordered by the `id` of the orders in ascending order. Additionally, the query facets the results based on the warranty details from the JSON attributes of the joined `orders` table.

<!-- request SQL -->
```sql
SELECT orders.product, name, orders.details.price, orders.tags
FROM customers
LEFT JOIN orders
ON customers.id = orders.customer_id
WHERE orders.details.price > 500
AND MATCH('laptop|phone|monitor', orders)
ORDER BY orders.id ASC
FACET orders.details.warranty;
```

<!-- request JSON -->
```json
POST /search
{
  "index": "customers",
	"_source": ["orders.product", "name", "orders.details", "orders.tags"],
  "sort": [{"orders.id": "asc"}],
  "join": [
    {
      "type": "left",
      "table": "orders",
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
      ],
      "query": {
        "range": {
          "orders.details.price": {
            "gt": 500
          }
        },
        "match": {
          "*": "laptop|phone|monitor"
        }
      }
    }
  ],
	"aggs":	{
		"group_property": {
			"terms": {
				"field": "orders.details.warranty"
			}
		}
	}
}
```

<!-- response SQL -->
```
+----------------+---------------+----------------------+-------------+
| orders.product | name          | orders.details.price | orders.tags |
+----------------+---------------+----------------------+-------------+
| Laptop         | Alice Johnson |                 1200 | 101,102     |
| Phone          | Bob Smith     |                  800 | 103         |
+----------------+---------------+----------------------+-------------+
2 rows in set (0.01 sec)
--- 2 out of 2 results in 0ms ---

+-------------------------+----------+
| orders.details.warranty | count(*) |
+-------------------------+----------+
| 2 years                 |        1 |
| 1 year                  |        1 |
+-------------------------+----------+
2 rows in set (0.01 sec)
--- 2 out of 2 results in 0ms ---
```

<!-- response JSON -->

```
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
          "name": "Alice Johnson",
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
          "orders.tags": [
            103
          ],
          "orders.details": {
            "price": 800,
            "warranty": "1 year"
          },
          "orders.product": "Phone"
        }
      }
    ]
  },
  "aggregations": {
    "group_property": {
      "buckets": [
        {
          "key": "1 year",
          "doc_count": 1
        },
        {
          "key": "2 years",
          "doc_count": 1
        }
      ]
    }
  }
}
```


<!-- end -->

## Caveats and Best Practices

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

5. **Using ANY with MVA**: When using the `ANY()` function with multi-valued attributes in JOINs, alias the multi-valued attribute from the joined table:
   ```sql
   SELECT *, t2.m AS alias
   FROM t
   LEFT JOIN t2 ON t.id = t2.t_id
   WHERE ANY(alias) IN (3, 5)
   ```

By following these guidelines, you can effectively use JOINs in Manticore Search to combine data from multiple indexes and perform complex queries.

<!-- proofread -->
