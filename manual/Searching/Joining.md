# Joining in Manticore Search

**WARNING: This functionality is in beta stage. Use it with caution.**

Joining in Manticore Search allows you to combine rows from two tables based on a related column between them. This feature enables more complex queries and data retrieval across multiple indexes.

## Syntax

### SQL

```sql
SELECT
	select_expr [, select_expr] ...
	FROM tbl_name
	{INNER | LEFT} JOIN tbl2_name
	ON join_condition
	[...other select options]
```

You can find other select options in [SELECT](../Searching/Intro.md#SQL) section.

### JSON

```json
POST /search
{
  "index": "table_name",
  "query": {
    ...
  },
  "join": [
    {
      "type": "inner" | "left",
      "table": "joined_table_name",
      "query": {
        ...
      },
      "on": [
        {
          "left": {
            "table": "left_table_name",
            "field": "field_name",
            "type": "field_type"
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
```

## Types of Joins

Manticore Search supports two types of joins:

1. **INNER JOIN**: Returns only the rows where there is a match in both tables.
2. **LEFT JOIN**: Returns all rows from the left table and the matched rows from the right table. If there's no match, NULL values are returned for the right table's columns.

## Examples

### Left Join

<!-- example left_basic -->

This query retrieves all products from the `orders` table along with the corresponding `email`, `name`, and `address` of customers from the `customers` table using a LEFT JOIN.

<!-- request SQL -->
```sql
SELECT
product, customers.email, customers.name, customers.address
FROM orders
LEFT JOIN customers
ON customers.id = orders.customer_id
WHERE MATCH('maple', customers)
ORDER BY customers.email ASC;
```

<!-- request JSON -->
```json
POST /search
{
  "index": "orders",
  "query": {
    "match": {
      "customers": "maple"
    }
  },
  "join": [
    {
      "type": "left",
      "table": "customers",
      "on": [
        {
          "left": {
            "table": "orders",
            "field": "customer_id",
            "type": "int"
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
  "select": ["product", "customers.email", "customers.name", "customers.address"],
  "sort": [{"customers.email": "asc"}]
}
```

<!-- response -->
```
+----------+-------------------+----------------+-------------------+
| product  | customers.email   | customers.name | customers.address |
+----------+-------------------+----------------+-------------------+
| Phone    | NULL              | NULL           | NULL              |
| Monitor  | NULL              | NULL           | NULL              |
| Keyboard | NULL              | NULL           | NULL              |
| Laptop   | alice@example.com | Alice Johnson  | 123 Maple St      |
| Tablet   | alice@example.com | Alice Johnson  | 123 Maple St      |
+----------+-------------------+----------------+-------------------+
5 rows in set (0.00 sec)
```

<!-- end -->

### Inner Join

<!-- example inner_basic -->

This query performs an INNER JOIN between the `orders` and `customers` tables, including only the orders with matching customers.

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
  "query": {
    "match": {
      "customers": "maple"
    }
  },
  "join": [
    {
      "type": "inner",
      "table": "customers",
      "on": [
        {
          "left": {
            "table": "orders",
            "field": "customer_id",
            "type": "int"
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
  "select": ["product", "customers.email", "customers.name", "customers.address"],
  "sort": [{"customers.email": "asc"}]
}
```

<!-- response -->
```
+---------+-------------------+----------------+-------------------+
| product | customers.email   | customers.name | customers.address |
+---------+-------------------+----------------+-------------------+
| Laptop  | alice@example.com | Alice Johnson  | 123 Maple St      |
| Tablet  | alice@example.com | Alice Johnson  | 123 Maple St      |
+---------+-------------------+----------------+-------------------+
2 rows in set (0.00 sec)
```
<!-- end -->

### Complex Join with Faceting

<!-- example basic_complex -->

This example demonstrates a more complex LEFT JOIN with filtering, ordering, and faceting.

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
  "join": [
    {
      "type": "left",
      "table": "orders",
      "on": [
        {
          "left": {
            "table": "customers",
            "field": "id",
            "type": "int"
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
          "details.price": {
            "gt": 500
          }
        },
        "match": {
          "*": "laptop|phone|monitor"
        }
      }
    }
  ],
  "select": ["orders.product", "name", "orders.details.price", "orders.tags"],
  "sort": [{"orders.id": "asc"}],
  "facet": {
    "orders.details.warranty": {}
  }
}
```

<!-- response -->
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

