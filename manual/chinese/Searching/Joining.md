# 连接表

Manticore Search中的表连接使您能够通过匹配相关列来组合两个表中的文档。此功能允许更复杂的查询和跨多个表的增强数据检索。

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
}
```

有关选择选项的更多信息，请参阅 [SELECT](../Searching/Intro.md#General-syntax) 部分。

<!--example join_sql_json_type -->

当通过JSON属性中的值进行连接时，您需要使用 `int()` 或 `string()` 函数显式指定该值的类型。

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
注意，`left` 操作数部分中有一个 `type` 字段，当使用JSON属性连接两个表时应使用它。允许的值为 `string` 和 `int`。

## 连接类型

Manticore Search支持两种类型的连接：

<!-- example inner_basic -->

1. **INNER JOIN**：只返回两个表中匹配的行。例如，该查询在 `orders` 和 `customers` 表之间执行 INNER JOIN，仅包含具有匹配客户的订单。

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

2. **LEFT JOIN**：返回左表中的所有行以及右表中匹配的行。如果没有匹配，则右表的列返回NULL值。例如，该查询使用LEFT JOIN检索所有客户及其对应的订单。如果不存在对应的订单，将显示NULL值。结果按客户的电子邮件排序，只选择客户的姓名和订单数量。

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

Manticore Search中的表连接强大功能之一是能够同时对左右表执行全文搜索。这允许您创建基于多个表中文本内容过滤的复杂查询。

<!-- example fulltext_basic -->

您可以在JOIN查询中为每个表使用单独的 `MATCH()` 函数。查询根据两个表中的文本内容过滤结果。

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

### 连接的JSON查询结构

在JSON API查询中，特定表的全文匹配结构不同于SQL：

<!-- example fulltext_json_structure -->

**主表查询**：根级的 `"query"` 字段适用于主表（在 `"table"` 中指定）。

**连接表查询**：每个连接定义可以包含其自己的 `"query"` 字段，专门适用于该连接表。

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

### 理解JOIN操作中的查询行为

<!-- example fulltext_json_behavior -->

**1. 仅在主表查询**：返回主表中所有匹配的行。对于未匹配的连接记录（LEFT JOIN），SQL返回NULL值，而JSON API返回默认值（数字为0，文本为空字符串）。

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
**2. 连接表中的查询作为过滤器**：当连接表具有查询时，只有同时满足连接条件和查询条件的记录才会返回。

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

**3. JOIN类型影响过滤**：INNER JOIN要求满足连接和查询条件，而LEFT JOIN即使右表条件不成立，也返回匹配的左表行。


### JOIN中全文匹配的重要注意事项

使用全文匹配结合连接时，请注意以下几点：

1. **表特定匹配**：
   - **SQL**：每个 `MATCH()` 函数应指定要搜索的表：`MATCH('term', table_name)`
   - **JSON**：使用根级 `"query"` 对主表进行匹配，连接定义中各自的 `"query"` 用于连接表

2. **查询语法灵活性**：JSON API支持 `"query_string"` 和 `"match"` 两种全文查询语法

3. **性能影响**：对两个表进行全文匹配可能影响查询性能，尤其是数据集较大时。请考虑使用合适的索引和批量大小。

4. **NULL/默认值处理**：使用LEFT JOIN时，如果右表没有匹配记录，查询优化器会根据性能决定优先评估全文条件还是过滤条件。SQL返回NULL值，而JSON API返回默认值（数字为0，文本为空字符串）。

5. **过滤行为**：连接表上的查询作为过滤器，限制结果为同时满足连接和查询条件的记录。

6. **全文操作符支持**：JOIN查询支持所有[全文操作符](../Searching/Full_text_matching/Operators.md)，包括短语、邻近、字段搜索、NEAR、仲裁匹配和高级操作符。

7. **评分计算**：每个表维护自己的相关性评分，SQL中通过 `table_name.weight()` 访问，JSON响应中通过 `table_name._score` 访问。

## 示例：复杂JOIN结合分面

基于之前的示例，我们探讨一个更高级的场景，将表连接与分面及跨多表的全文匹配结合。这展示了Manticore JOIN功能在复杂过滤和聚合中的全部威力。

<details>

以下示例的初始化查询：

```
drop table if exists customers; drop table if exists orders; create table customers(name text, email text, address text); create table orders(product text, customer_id int, quantity int, order_date string, tags multi, details json); insert into customers values (1, 'Alice Johnson', 'alice@example.com', '123 Maple St'), (2, 'Bob Smith', 'bob@example.com', '456 Oak St'), (3, 'Carol White', 'carol@example.com', '789 Pine St'), (4, 'John Smith', 'john@example.com', '15 Barclays St'); insert into orders values (1, 'Laptop Computer', 1, 1, '2023-01-01', (101,102), '{"price":1200,"warranty":"2 years"}'), (2, 'Smart Phone', 2, 2, '2023-01-02', (103), '{"price":800,"warranty":"1 year"}'), (3, 'Tablet Device', 1, 1, '2023-01-03', (101,104), '{"price":450,"warranty":"1 year"}'), (4, 'Monitor Display', 3, 1, '2023-01-04', (105), '{"price":300,"warranty":"1 year"}');
```

</details>

<!-- example basic_complex -->

此查询演示了跨`customers`和`orders`表的全文匹配，结合了范围过滤和分面功能。它搜索名为“Alice”或“Bob”的客户及其订单，订单中包含“laptop”、“phone”或“tablet”，且价格超过500美元。结果按订单ID排序，并按保修期限进行分面。

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

## 搜索选项和匹配权重

可以为联接中的查询分别指定选项：针对左表和右表。语法为SQL查询中的`OPTION(<table_name>)`，以及JSON查询中`"options"`下的一个或多个子对象。


<!-- example join_options -->

以下是如何为右表的全文查询指定不同字段权重的示例。要通过SQL检索匹配权重，请使用`<table_name>.weight()`表达式。
在JSON查询中，此权重表示为`<table_name>._score`。

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

执行表联接时，Manticore Search 会批量处理结果，以优化性能和资源使用。工作原理如下：

- **批处理工作原理**：
  - 首先执行左表查询，并将结果积累到一个批次中。
  - 然后该批次作为输入，用于右表的查询，该查询作为单次操作执行。
  - 此方法最小化了发送到右表的查询次数，提高了效率。

- **配置批处理大小**：
  - 可以使用`join_batch_size`搜索选项调整批次大小。
  - 也可在配置文件的`searchd`部分通过[配置](../Server_settings/Searchd.md#join_batch_size)设置。
  - 默认批次大小为`1000`，根据使用情况可增减。
  - 设置`join_batch_size=0`将完全禁用批处理，有助于调试或特定场景。

- **性能注意事项**：
  - 更大的批次大小可减少对右表执行的查询次数，从而提高性能。
  - 但较大的批次可能消耗更多内存，尤其是对于复杂查询或大型数据集。
  - 建议尝试不同批次大小，寻找性能与资源消耗之间的最佳平衡。

## 联接缓存

为进一步优化联接操作，Manticore Search 对右表执行的查询采用缓存机制。具体如下：

- **缓存工作原理**：
  - 右表的每个查询由`JOIN ON`条件定义。
  - 如果多个查询中的`JOIN ON`条件相同，结果会被缓存并重用。
  - 这避免了冗余查询，加快了后续联接操作速度。

- **配置缓存大小**：
  - 可以通过配置文件中`searchd`部分的[join_cache_size](../Server_settings/Searchd.md#join_cache_size)选项调整联接缓存大小。
  - 默认缓存大小为`20MB`，可根据工作负载和内存情况调整。
  - 设置`join_cache_size=0`将完全禁用缓存。

- **内存注意事项**：
  - 每个线程维护自己的缓存，因此总内存使用量取决于线程数和缓存大小。
  - 请确保服务器内存充足，特别是在高并发环境下以容纳缓存。

## 联接分布式表

仅由本地表组成的分布式表，在联接查询的左侧和右侧均受支持。但包含远程表的分布式表不受支持。

## 注意事项和最佳实践

使用Manticore Search中的JOIN时，请注意以下几点：

1. **字段选择**：从两个表中选择字段时，左表字段不带前缀，右表字段需带表名前缀。例如：
   ```sql
   SELECT field_name, right_table.field_name FROM ...
   ```

2. **JOIN条件**：在JOIN条件中务必显式指定表名：
   ```sql
   JOIN ON table_name.some_field = another_table_name.some_field
   ```

3. **表达式与JOIN**：使用结合两个表字段的表达式时，应为表达式结果设置别名：
   ```sql
   SELECT *, (nums2.n + 3) AS x, x * n FROM nums LEFT JOIN nums2 ON nums2.id = nums.num2_id
   ```

4. **对带别名表达式的过滤**：不能在WHERE子句中使用涉及两个表字段的表达式别名。

5. **JSON属性**：对JSON属性进行联接时，必须显式转换值为适当类型：
   ```sql
   -- 正确示例：
   SELECT * FROM t1 LEFT JOIN t2 ON int(t1.json_attr.id) = t2.json_attr.id

   -- 错误示例：
   SELECT * FROM t1 LEFT JOIN t2 ON t1.json_attr.id = t2.json_attr.id
   ```

6. **NULL处理**：可以在联接字段上使用IS NULL和IS NOT NULL条件：
   ```sql
   SELECT * FROM t1 LEFT JOIN t2 ON t1.id = t2.id WHERE t2.name IS NULL
   SELECT * FROM t1 LEFT JOIN t2 ON t1.id = t2.id WHERE t2.name IS NOT NULL
   ```

7. **MVA的ANY使用**：在JOIN中对多值属性（MVA）使用`ANY()`函数时，应为来自联接表的多值属性设置别名：
   ```sql
   SELECT *, t2.m AS alias
   FROM t
   LEFT JOIN t2 ON t.id = t2.t_id
   WHERE ANY(alias) IN (3, 5)
   ```

遵循以上指南，您可以有效利用Manticore Search中的JOIN，实现多索引数据结合和复杂查询。 

<!-- proofread -->

