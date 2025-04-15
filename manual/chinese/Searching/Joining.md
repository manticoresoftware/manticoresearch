# 连接表

Manticore Search 中的表连接使您能够通过匹配相关列来结合两个表中的文档。此功能允许进行更复杂的查询以及跨多个表的数据检索。

## 一般语法

### SQL

```sql
SELECT
	select_expr [, select_expr] ...
	FROM tbl_name
	{INNER | LEFT} JOIN tbl2_name
	ON join_condition
	[...其他选择选项]

join_condition: {
	left_table.attr = right_table.attr
	| left_table.json_attr.string_id = string(right_table.json_attr.string_id)
	| left_table.json_attr.int_id = int(right_table.json_attr.int_id)
}
```

有关选择选项的更多信息，请参见 [SELECT](../Searching/Intro.md#General-syntax) 部分。

<!--example join_sql_json_type -->

当通过 JSON 属性中的值进行连接时，您需要使用 `int()` 或 `string()` 函数明确指定值的类型。

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
    <可选的对左表的全文查询>
  },
  "join": [
    {
      "type": "inner" | "left",
      "table": "joined_table_name",
      "query": {
        <可选的对右表的全文查询>
      },
      "on": [
        {
          "left": {
            "table": "left_table_name",
            "field": "field_name",
            "type": "<使用 JSON 属性连接时的通用字段类型>"
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
注意，在 `left` 操作数部分有一个 `type` 字段，您在使用 JSON 属性连接两个表时应该使用该字段。允许的值为 `string` 和 `int`。

## 连接类型

Manticore Search 支持两种类型的连接：

<!-- example inner_basic -->

1. **INNER JOIN**：仅返回在两个表中都存在匹配项的行。例如，查询在 `orders` 和 `customers` 表之间执行 INNER JOIN，仅包括匹配的客户的订单。

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

2. **LEFT JOIN**：返回左表中的所有行和右表中的匹配行。如果没有匹配项，则右表的列将返回 NULL 值。例如，此查询使用 LEFT JOIN 检索所有客户及其对应的订单。如果没有对应的订单，则会出现 NULL 值。结果按客户的电子邮件排序，仅选择客户的姓名和订单数量。

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

### 示例：复杂的JOIN与面向

在之前的例子基础上，让我们探讨一个更高级的场景，其中我们结合表连接和面向。这使我们不仅可以检索连接的数据，还可以以有意义的方式聚合和分析数据。

<!-- example basic_complex -->

此查询从 `orders` 和 `customers` 表中检索产品、客户名称、产品价格和产品标签。它执行 `LEFT JOIN`，确保即使没有下订单的客户也包含在内。查询将结果过滤为仅包括价格大于 `500` 的订单，并将产品与 'laptop'、 'phone' 或 'monitor' 进行匹配。结果按订单的 `id` 以升序排列。此外，查询基于连接的 `orders` 表的 JSON 属性中的保修信息对结果进行面向。

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
| 笔记本电脑      | 爱丽丝·约翰逊  |                 1200 | 101,102     |
| 手机           | 鲍勃·史密斯     |                  800 | 103         |
+----------------+---------------+----------------------+-------------+
2 行中 (0.01 秒)
--- 2 行中 2 个结果在 0 毫秒内 ---

+-------------------------+----------+
| orders.details.warranty | count(*) |
+-------------------------+----------+
| 2 年                    |        1 |
| 1 年                    |        1 |
+-------------------------+----------+
2 行中 (0.01 秒)
--- 2 行中 2 个结果在 0 毫秒内 ---
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
          "name": "爱丽丝·约翰逊",
          "orders.tags": [
            101,
            102
          ],
          "orders.details": {
            "price": 1200,
            "warranty": "2 年"
          },
          "orders.product": "笔记本电脑"
        }
      },
      {
        "_id": 2,
        "_score": 1,
        "_source": {
          "name": "鲍勃·史密斯",
          "orders.tags": [
            103
          ],
          "orders.details": {
            "price": 800,
            "warranty": "1 年"
          },
          "orders.product": "手机"
        }
      }
    ]
  },
  "aggregations": {
    "group_property": {
      "buckets": [
        {
          "key": "1 年",
          "doc_count": 1
        },
        {
          "key": "2 年",
          "doc_count": 1
        }
      ]
    }
  }
}
```


<!-- end -->

## 搜索选项和匹配权重

可以为连接中的查询分别指定选项：左表和右表。 SQL 查询的语法为 `OPTION(<table_name>)`，JSON 查询的语法为 `"options"` 下的一个或多个子对象。


<!-- example join_options -->

这是一个如何为右表的全文查询指定不同字段权重的示例。要通过 SQL 检索匹配权重，请使用 `<table_name>.weight()` 表达式。
在 JSON 查询中，这个权重表示为 `<table_name>._score`。

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
| 笔记本电脑 | alice@example.com | 爱丽丝·约翰逊  | 123 Maple St      |            1500680 |
| 平板电脑    | alice@example.com | 爱丽丝·约翰逊  | 123 Maple St      |            1500680 |
+---------+-------------------+----------------+-------------------+--------------------+
2 行中 (0.00 秒)
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
          "product": "笔记本电脑",
          "customers.email": "alice@example.com",
          "customers.name": "爱丽丝·约翰逊",
          "customers.address": "123 Maple St"
        }
      },
      {
        "_id": 3,
        "_score": 1,
        "customers._score": 15000680,
        "_source": {
          "product": "平板电脑",
          "customers.email": "alice@example.com",
          "customers.name": "爱丽丝·约翰逊",
          "customers.address": "123 Maple St"
        }
      }
    ]
  }
}
```
<!-- end -->

## 连接批处理

在执行表连接时，Manticore Search 以批处理的方式处理结果，以优化性能和资源使用。以下是其工作原理：

- **批处理的工作原理**： 
  - 首先执行左表的查询，并将结果累积到一个批次中。
  - 然后将该批次作为输入用于右表的查询，该查询作为单个操作执行。
  - 这种方法可以最小化发送到右表的查询数，提高效率。

- **配置批处理大小**：
  - 可以使用 `join_batch_size` 搜索选项调整批处理大小。
  - 它也可以在配置文件的 `searchd` 部分进行[配置](../../Server_settings/Searchd.md#join_batch_size)。
  - 默认批处理大小为 `1000`，但可以根据用例增加或减少。
  - 设置 `join_batch_size=0` 会完全禁用批处理，这在调试或特定场景中可能很有用。

- **性能考虑**：
  - 较大的批量大小可以通过减少在右表上执行的查询数量来提高性能。
  - 然而，较大的批量可能会消耗更多的内存，特别是在复杂查询或大型数据集的情况下。
  - 尝试不同的批量大小，以找到性能和资源使用之间的最佳平衡。

## 连接缓存

为了进一步优化连接操作，Manticore Search 对在右表上执行的查询采用了缓存机制。以下是您需要了解的内容：

- **缓存工作原理**：
  - 每个在右表上的查询由 `JOIN ON` 条件定义。
  - 如果相同的 `JOIN ON` 条件在多个查询中重复，结果会被缓存并重用。
  - 这可以避免冗余查询，加快后续连接操作的速度。

- **配置缓存大小**：
  - 连接缓存的大小可以使用配置文件中 `searchd` 部分的 [join_cache_size](../../Server_settings/Searchd.md#join_cache_size) 选项进行配置。
  - 默认的缓存大小为 `20MB`，但您可以根据工作负载和可用内存调整它。
  - 设置 `join_cache_size=0` 会完全禁用缓存。

- **内存注意事项**：
  - 每个线程维护自己的缓存，因此总内存使用量取决于线程数量和缓存大小。
  - 确保您的服务器有足够的内存来容纳缓存，特别是在高并发环境中。


## 注意事项和最佳实践

在 Manticore Search 中使用 JOIN 时，请记住以下几点：

1. **字段选择**：在从两个表中选择字段时，不要为左表的字段加前缀，但要为右表的字段加前缀。例如：
   ```sql
   SELECT field_name, right_table.field_name FROM ...
   ```

2. **JOIN 条件**：在您的 JOIN 条件中始终显式指定表名：
   ```sql
   JOIN ON table_name.some_field = another_table_name.some_field
   ```

3. **使用 JOIN 的表达式**：在使用组合了两个连接表字段的表达式时，为表达式的结果起别名：
   ```sql
   SELECT *, (nums2.n + 3) AS x, x * n FROM nums LEFT JOIN nums2 ON nums2.id = nums.num2_id
   ```

4. **对别名表达式进行过滤**：在 WHERE 子句中，您不能对涉及两个表字段的表达式使用别名。

5. **JSON 属性**：连接 JSON 属性时，必须显式将值转换为适当的类型：
   ```sql
   -- 正确：
   SELECT * FROM t1 LEFT JOIN t2 ON int(t1.json_attr.id) = t2.json_attr.id
   
   -- 不正确：
   SELECT * FROM t1 LEFT JOIN t2 ON t1.json_attr.id = t2.json_attr.id
   ```

6. **NULL 处理**：您可以在连接字段上使用 IS NULL 和 IS NOT NULL 条件：
   ```sql
   SELECT * FROM t1 LEFT JOIN t2 ON t1.id = t2.id WHERE t2.name IS NULL
   SELECT * FROM t1 LEFT JOIN t2 ON t1.id = t2.id WHERE t2.name IS NOT NULL
   ```

7. **使用 ANY 与 MVA**：在 JOIN 中使用 `ANY()` 函数处理多值属性时，要为连接表中的多值属性起别名：
   ```sql
   SELECT *, t2.m AS alias
   FROM t
   LEFT JOIN t2 ON t.id = t2.t_id
   WHERE ANY(alias) IN (3, 5)
   ```

通过遵循这些指导方针，您可以有效地在 Manticore Search 中使用 JOIN 来结合来自多个索引的数据并执行复杂查询。

<!-- proofread -->
