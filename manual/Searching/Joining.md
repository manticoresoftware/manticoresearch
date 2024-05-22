# Joining

**WARNING: This functionality is in beta stage. Use it with caution.**

The JOIN SQL statement in Manticore Search allows combining rows from two tables based on a related column between them.

## Examples

<!-- example left_basic -->

This query retrieves all products from the `orders` table along with the corresponding `email`, `name`, and `address` of customers from the `customers` table. It performs a `LEFT JOIN`, ensuring that all records from the `orders` table are included even if there is no matching customer. The query also filters the results to include only those customers whose data matches the term 'maple' in the joined `customers` table. The results are ordered by the `email` address of the customers in ascending order.

<!-- request Example -->
```sql
SELECT product, customers.email, customers.name, customers.address
FROM orders
LEFT JOIN customers
ON customers.id = orders.customer_id
WHERE MATCH('maple', customers)
ORDER BY customers.email ASC;
```

<!-- response Example -->
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

<!-- example inner_basic -->

This query does the same as the previous one, but it performs an `INNER JOIN`, which means only the orders with matching customers are included in the result.

<!-- request Example -->
```sql
SELECT product, customers.email, customers.name, customers.address
FROM orders
INNER JOIN customers
ON customers.id = orders.customer_id
WHERE MATCH('maple', customers)
ORDER BY customers.email asc;
```

<!-- response Example -->
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

<!-- example basic_complex -->

This query retrieves products, customer names, product prices, and product tags from the `orders` table and `customers` table. It performs a `LEFT JOIN`, ensuring all customers are included even if they have not made an order. The query filters to include only those orders with a price greater than `500` and matches the products to the terms 'laptop', 'phone', or 'monitor'. The results are ordered by the `id` of the orders in ascending order. Additionally, the query facets the results based on the `warranty details` from the JSON attributes of the joined `orders` table.

<!-- request Example -->
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

<!-- response Example -->
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

## Caveates

* **Field selection**: When selecting fields from two tables in a `JOIN`, you must not prefix fields from the left table but must prefix fields from the right table. Correct usage is `SELECT field_name, right_table.field_name FROM ...`, not `SELECT left_table.field_name, right_table.field_name FROM ...`.
* **JOIN conditions**: Always explicitly specify the table names in your `JOIN` conditions. Use the format `JOIN ON table_name.some_field = another_table_name.some_field`. Omitting table names from the join condition is not supported.
* **Using expressions with JOINs**: When using expressions that combine fields from both joined tables, you must alias the result of the expression. For instance, instead of `SELECT *, (nums2.n + 3) * n FROM nums LEFT JOIN nums2 ON nums2.id = nums.num2_id`, you should write `SELECT *, (nums2.n + 3) AS x, x * n FROM nums LEFT JOIN nums2 ON nums2.id = nums.num2_id`.
* **Filtering on aliased expressions**: When you alias an expression involving fields from **both tables** (e.g., `expr(field_from_left_table, field_from_right_table) AS some_alias`), note that you cannot use this alias for filtering in the `WHERE` clause.
* **Using ANY with MVA**: The `ANY()` function with multi-valued attributes requires specific handling when used with JOINs. Direct filtering on multi-valued attributes in a WHERE clause is not supported when performing a JOIN. Instead, you must alias the multi-valued attribute from the joined table and use this alias for the `ANY()` condition. For example:
  ```sql
  SELECT *, t2.m AS alias
  FROM t
  LEFT JOIN t2 ON t.id = t2.t_id
  WHERE ANY(alias) IN (3, 5)
  ```
