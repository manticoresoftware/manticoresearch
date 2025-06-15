# 连接表

Manticore Search 中的表连接使您能够通过匹配相关列来组合两个表中的文档。此功能允许更复杂的查询和跨多个表的增强数据检索。

## 通用语法

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

有关选择选项的详细信息，请参阅[SELECT](../Searching/Intro.md#General-syntax)部分。

<!--example join_sql_json_type -->

当按 JSON 属性中的值进行连接时，您需要显式指定该值的类型，使用 `int()` 或 `string()` 函数。

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
注意，`left` 操作数部分中有 `type` 字段，当使用 json 属性连接两个表时，您应使用该字段。允许的值为 `string` 和 `int`。

## 连接类型

Manticore Search 支持两种类型的连接：

<!-- example inner_basic -->

1. **INNER JOIN**：只返回两个表中都有匹配的行。例如，该查询执行 `orders` 表和 `customers` 表之间的 INNER JOIN，只包含有匹配客户的订单。

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

2. **LEFT JOIN**：返回左表中的所有行以及右表中匹配的行。如果没有匹配，则右表的列返回 NULL 值。例如，此查询使用 LEFT JOIN 检索所有客户及其对应的订单。如果不存在对应的订单，则显示 NULL 值。结果按客户邮箱排序，并且只选择客户姓名和订单数量。

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

### 示例：带聚类功能的复杂 JOIN

基于之前的示例，让我们来探讨一个更高级的场景，将表连接与聚类结合使用。这样我们不仅可以检索连接数据，还能以有意义的方式聚合和分析数据。

<!-- example basic_complex -->

该查询从 `orders` 和 `customers` 表中检索产品、客户名称、产品价格和产品标签。它执行 `LEFT JOIN`，确保包括所有客户，即使他们没有下订单。查询过滤结果，仅包含价格大于 `500` 的订单，并将产品限定为 'laptop'、'phone' 或 'monitor'。结果按订单的 `id` 升序排序。此外，查询基于连接的 `orders` 表的 JSON 属性中的保修信息对结果进行分面。

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
  "table": "customers",
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

## 搜索选项和匹配权重

可以为连接中的查询分别指定选项：分别针对左表和右表。SQL 查询的语法为 `OPTION(<table_name>)`，JSON 查询则是在 `"options"` 下有一个或多个子对象。


<!-- example join_options -->

以下示例展示了如何为右表的全文查询指定不同的字段权重。要通过 SQL 获取匹配权重，请使用 `<table_name>.weight()` 表达式。
在 JSON 查询中，该权重表示为 `<table_name>._score`。

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

## 连接批处理

执行表连接时，Manticore Search 会以批处理方式处理结果，以优化性能和资源使用。工作原理如下：

- **批处理工作原理**：
  - 首先执行左表的查询，结果累积成一个批次。
  - 该批次作为输入用于右表的查询，作为单次操作执行。
  - 这种方法减少了发送到右表的查询次数，提高了效率。

- **配置批处理大小**：
  - 批处理大小可以通过搜索选项 `join_batch_size` 调整。
  - 也可在配置文件的 `searchd` 部分通过 [join_batch_size](../Server_settings/Searchd.md#join_batch_size) 配置。
  - 默认批处理大小为 `1000`，您可根据实际需求增减。
  - 将 `join_batch_size=0` 设置为完全禁用批处理，这在调试或特定场景下可能有用。

- **性能考虑**：
  - 较大的批处理大小可通过减少右表执行的查询次数提升性能。
  - 但较大的批次可能会消耗更多内存，尤其是对于复杂查询或大型数据集。
  - 您可以尝试不同的批处理大小以找到性能和资源使用之间的最佳平衡。

## 连接缓存

为进一步优化连接操作，Manticore Search 对右表执行的查询使用缓存机制。以下是相关要点：

- **缓存工作原理**：
  - 每个右表查询由 `JOIN ON` 条件定义。
  - 如果多个查询中重复使用相同的 `JOIN ON` 条件，结果将被缓存并复用。
  - 这避免了冗余查询并加快后续连接操作。

- **配置缓存大小**：
  - 可以通过配置文件中 `searchd` 部分的 [join_cache_size](../Server_settings/Searchd.md#join_cache_size) 选项调整连接缓存大小。
  - 默认缓存大小为 `20MB`，您可以根据工作负载和可用内存调整。
  - 设置 `join_cache_size=0` 将完全禁用缓存。

- **内存考虑**：
  - 每个线程维护自己的缓存，因此总内存使用量取决于线程数量和缓存大小。
  - 确保您的服务器有足够的内存来容纳缓存，尤其是在高并发环境中。


## 注意事项和最佳实践

在 Manticore Search 中使用 JOIN 时，请注意以下几点：

1. **字段选择**：在 JOIN 中从两个表选择字段时，不要为左表的字段添加前缀，但要为右表的字段添加前缀。例如：
   ```sql
   SELECT field_name, right_table.field_name FROM ...
   ```

2. **JOIN 条件**：始终在 JOIN 条件中显式指定表名：
   ```sql
   JOIN ON table_name.some_field = another_table_name.some_field
   ```

3. **使用 JOIN 的表达式**：当使用结合两个联接表字段的表达式时，为表达式结果使用别名：
   ```sql
   SELECT *, (nums2.n + 3) AS x, x * n FROM nums LEFT JOIN nums2 ON nums2.id = nums.num2_id
   ```

4. **对别名表达式的过滤**：不能在 WHERE 子句中对涉及两个表字段的表达式使用别名。

5. **JSON 属性**：在连接 JSON 属性时，必须显式将值强制转换为适当的类型：
   ```sql
   -- 正确：
   SELECT * FROM t1 LEFT JOIN t2 ON int(t1.json_attr.id) = t2.json_attr.id

   -- 错误：
   SELECT * FROM t1 LEFT JOIN t2 ON t1.json_attr.id = t2.json_attr.id
   ```

6. **NULL 处理**：可以对联接字段使用 IS NULL 和 IS NOT NULL 条件：
   ```sql
   SELECT * FROM t1 LEFT JOIN t2 ON t1.id = t2.id WHERE t2.name IS NULL
   SELECT * FROM t1 LEFT JOIN t2 ON t1.id = t2.id WHERE t2.name IS NOT NULL
   ```

7. **在 MVA 上使用 ANY**：在 JOIN 中使用多值属性的 `ANY()` 函数时，为来自联接表的多值属性使用别名：
   ```sql
   SELECT *, t2.m AS alias
   FROM t
   LEFT JOIN t2 ON t.id = t2.t_id
   WHERE ANY(alias) IN (3, 5)
   ```

遵循这些指导原则，您可以有效地在 Manticore Search 中使用 JOIN，从多个索引组合数据并执行复杂查询。

<!-- proofread -->

