# 连接表

Manticore Search 中的表连接使您能够通过匹配相关列，将两个表中的文档合并。这项功能支持更复杂的查询和跨多个表的增强数据检索。

## 一般语法

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
	| [..右表属性上的过滤器]
}
```

有关选择选项的更多信息，请参考[SELECT](../Searching/Intro.md#General-syntax)部分。

<!--example join_sql_json_type -->

当通过 JSON 属性的值进行连接时，需要使用 `int()` 或 `string()` 函数显式指定值的类型。

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
注意，`left` 操作数部分有 `type` 字段，在使用 json 属性连接两个表时应使用。允许的值为 `string` 和 `int`。

## 连接类型

Manticore Search 支持两种连接类型：

<!-- example inner_basic -->

1. **INNER JOIN**：只返回两个表中均有匹配的行。例如，查询在 `orders` 和 `customers` 表之间执行 INNER JOIN，只包含有匹配客户的订单。

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

2. **LEFT JOIN**：返回左表的所有行以及右表中匹配的行。如果没有匹配，则右表列返回 NULL 值。例如，该查询使用 LEFT JOIN 获取所有客户及其对应的订单。如果没有对应订单，则显示 NULL 值。结果按客户电子邮件排序，仅选择客户名和订单数量。

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

## 跨连接表的全文匹配

表连接的强大功能之一是能够在左右表上同时执行全文搜索。这允许您创建基于多个表中文本内容过滤的复杂查询。

<!-- example fulltext_basic -->

您可以在 JOIN 查询中为每个表使用独立的 `MATCH()` 函数。查询根据两个表中的文本内容过滤结果。

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

### 连接的 JSON 查询结构

在 JSON API 查询中，针对特定表的全文匹配结构与 SQL 不同：

<!-- example fulltext_json_structure -->

**主表查询**：根级的 `"query"` 字段应用于主表（由 `"table"` 指定）。

**连接表查询**：每个连接定义可以包含自己的 `"query"` 字段，专门应用于该连接的表。

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

### 理解 JOIN 操作中的查询行为

<!-- example fulltext_json_behavior -->

**1. 仅主表查询**：返回主表中所有匹配的行。对于未匹配的连接记录（LEFT JOIN），SQL 返回 NULL 值，而 JSON API 返回默认值（数字为 0，文本为空字符串）。

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
**2. 连接表查询作为过滤器**：当连接表具有查询时，仅返回同时满足连接条件和查询条件的记录。

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

**3. JOIN 类型影响过滤**：INNER JOIN 要求满足连接和查询条件，而 LEFT JOIN 即使右表条件不满足，也返回匹配的左表行。


### JOIN 中全文匹配的重要注意事项

使用连接进行全文匹配时，请注意以下几点：

1. **表特定匹配**：
   - **SQL**：每个 `MATCH()` 函数应指明搜索的表：`MATCH('term', table_name)`
   - **JSON**：根级 `"query"` 用于主表，连接定义中的 `"query"` 用于连接表

2. **查询语法灵活**：JSON API 支持使用 `"query_string"` 和 `"match"` 语法进行全文查询

3. **性能影响**：两表上的全文匹配可能影响查询性能，特别是大数据集。请考虑使用合适的索引和批处理大小。

4. **NULL/默认值处理**：使用 LEFT JOIN，当右表无匹配记录时，查询优化器会根据性能决定先评估全文条件还是过滤条件。SQL 返回 NULL 值，JSON API 返回默认值（数字为 0，文本为空字符串）。

5. **过滤行为**：连接表上的查询充当过滤器，限制结果满足连接和查询条件。

6. **全文操作符支持**：JOIN 查询支持所有[全文操作符](../Searching/Full_text_matching/Operators.md)，包括短语、邻近、字段搜索、NEAR、法定匹配和高级操作符。

7. **得分计算**：每个表维护自己的相关性得分，在 SQL 中可通过 `table_name.weight()` 访问，在 JSON 响应中为 `table_name._score`。

## 示例：复杂 JOIN 结合分面

基于前面示例，探索一个更高级的场景，结合表连接、分面和跨多表全文匹配。这展示了 Manticore JOIN 功能在复杂过滤和聚合中的强大能力。

<details>

以下示例的初始化查询：

```
drop table if exists customers; drop table if exists orders; create table customers(name text, email text, address text); create table orders(product text, customer_id int, quantity int, order_date string, tags multi, details json); insert into customers values (1, 'Alice Johnson', 'alice@example.com', '123 Maple St'), (2, 'Bob Smith', 'bob@example.com', '456 Oak St'), (3, 'Carol White', 'carol@example.com', '789 Pine St'), (4, 'John Smith', 'john@example.com', '15 Barclays St'); insert into orders values (1, 'Laptop Computer', 1, 1, '2023-01-01', (101,102), '{"price":1200,"warranty":"2 years"}'), (2, 'Smart Phone', 2, 2, '2023-01-02', (103), '{"price":800,"warranty":"1 year"}'), (3, 'Tablet Device', 1, 1, '2023-01-03', (101,104), '{"price":450,"warranty":"1 year"}'), (4, 'Monitor Display', 3, 1, '2023-01-04', (105), '{"price":300,"warranty":"1 year"}');
```

</details>

<!-- example basic_complex -->

此查询演示了跨 `customers` 和 `orders` 两个表的全文匹配，结合范围过滤和分面。它搜索名为 "Alice" 或 "Bob" 的客户及其包含 "laptop"、"phone" 或 "tablet" 并且价格高于 500 美元的订单。结果按订单 ID 排序，并按保修条款进行分面。

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
  "query":  {
      "bool": {
          "must": [
          {
              "range": {
                  "orders.details.price": {
                      "gt": 500
                  }
               },
               "query_string": "alice | bob"
          ]
      }
  },
  "join": [
    {
      "type": "left",
      "table": "orders",
      "query": {
          "query_string": "laptop | phone | tablet"
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

## 搜索选项和匹配权重

可以为联接中的查询指定分别针对左表和右表的选项：SQL 查询的语法为 `OPTION(<table_name>)`，而 JSON 查询则是在 `"options"` 下的一个或多个子对象。


<!-- example join_options -->

以下是如何为右表的全文查询指定不同字段权重的示例。要通过 SQL 获取匹配权重，请使用 `<table_name>.weight()` 表达式。
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

## 联接批处理

在执行表联接时，Manticore Search 会分批处理结果，以优化性能和资源使用。具体操作如下：

- **批处理工作原理**：
  - 首先执行左表的查询，将结果累积成一个批次。
  - 该批次作为右表查询的输入，一次性执行。
  - 此方法减少向右表发送的查询次数，提高效率。

- **配置批处理大小**：
  - 可以通过搜索选项 `join_batch_size` 来调整批处理大小。
  - 也可以在配置文件的 `searchd` 部分进行[配置](../Server_settings/Searchd.md#join_batch_size)。
  - 默认批处理大小为 `1000`，可根据实际需求增减。
  - 将 `join_batch_size` 设为 `0` 可完全禁用批处理，适用于调试或特殊场景。

- **性能考虑**：
  - 较大的批处理大小可以通过减少右表查询次数提升性能。
  - 但较大批处理可能消耗更多内存，尤其是复杂查询或大数据集。
  - 可尝试不同批处理大小，寻找性能与资源使用的最佳平衡点。

## 联接缓存

为了进一步优化联接操作，Manticore Search 对右表执行的查询采用缓存机制。您需要了解以下内容：

- **缓存工作原理**：
  - 每个右表的查询由 `JOIN ON` 条件定义。
  - 如果多个查询重复相同的 `JOIN ON` 条件，结果会被缓存并重复使用。
  - 避免了重复查询，加快了后续联接操作。

- **配置缓存大小**：
  - 使用配置文件 `searchd` 部分的 [join_cache_size](../Server_settings/Searchd.md#join_cache_size) 选项配置联接缓存大小。
  - 默认缓存大小为 `20MB`，可根据工作负载和可用内存调整。
  - 设置 `join_cache_size=0` 可完全禁用缓存。

- **内存考虑**：
  - 每个线程维护自己的缓存，总内存使用量取决于线程数和缓存大小。
  - 请确保服务器有足够内存支持缓存，特别是高并发环境。

## 联接分布式表

构成仅由本地表组成的分布式表支持用作联接查询的左侧和右侧。然而，包含远程表的分布式表不被支持。

## 注意事项和最佳实践

使用 Manticore Search 中的 JOIN 时，请注意以下要点：

1. **字段选择**：从两个表中选择字段时，不要为左表字段加前缀，但右表字段需加前缀。例如：
   ```sql
   SELECT field_name, right_table.field_name FROM ...
   ```

2. **JOIN 条件**：在 JOIN 条件中始终明确指定表名：
   ```sql
   JOIN ON table_name.some_field = another_table_name.some_field
   ```

3. **带 JOIN 的表达式**：使用结合两个表字段的表达式时，为表达式结果起别名：
   ```sql
   SELECT *, (nums2.n + 3) AS x, x * n FROM nums LEFT JOIN nums2 ON nums2.id = nums.num2_id
   ```

4. **基于别名表达式的过滤**：无法在 WHERE 子句中使用涉及两个表字段表达式的别名。

5. **JSON 属性**：联接 JSON 属性时，必须显式将值转换为相应类型：
   ```sql
   -- 正确示例：
   SELECT * FROM t1 LEFT JOIN t2 ON int(t1.json_attr.id) = t2.json_attr.id

   -- 错误示例：
   SELECT * FROM t1 LEFT JOIN t2 ON t1.json_attr.id = t2.json_attr.id
   ```

6. **NULL 处理**：可以对联接字段使用 IS NULL 和 IS NOT NULL 条件：
   ```sql
   SELECT * FROM t1 LEFT JOIN t2 ON t1.id = t2.id WHERE t2.name IS NULL
   SELECT * FROM t1 LEFT JOIN t2 ON t1.id = t2.id WHERE t2.name IS NOT NULL
   ```

7. **用 ANY 处理多值属性（MVA）**：使用 `ANY()` 函数处理联接中的多值属性时，要对联接表的多值属性起别名：
   ```sql
   SELECT *, t2.m AS alias
   FROM t
   LEFT JOIN t2 ON t.id = t2.t_id
   WHERE ANY(alias) IN (3, 5)
   ```

遵循这些指南，您可以有效利用 Manticore Search 中的 JOIN，将多个索引数据结合并执行复杂查询。

<!-- proofread -->

