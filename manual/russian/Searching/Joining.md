# Соединение таблиц

Соединения таблиц в Manticore Search позволяют объединять документы из двух таблиц по совпадающим столбцам. Эта функциональность обеспечивает выполнение более сложных запросов и расширенный поиск данных по нескольким таблицам.

## Общий синтаксис

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

Для получения дополнительной информации о параметрах select обратитесь к разделу [SELECT](../Searching/Intro.md#General-syntax).

<!--example join_sql_json_type -->

При объединении по значению из JSON-атрибута необходимо явно указать тип значения с помощью функций `int()` или `string()`.

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
Обратите внимание, что в разделе операнда `left` есть поле `type`, которое следует использовать при объединении двух таблиц с помощью json-атрибутов. Разрешённые значения — `string` и `int`.

## Типы соединений

Manticore Search поддерживает два типа соединений:

<!-- example inner_basic -->

1. **INNER JOIN**: Возвращает только те строки, в которых есть совпадение в обеих таблицах. Например, запрос выполняет INNER JOIN между таблицами `orders` и `customers`, включая только заказы, у которых есть соответствующие клиенты.

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

2. **LEFT JOIN**: Возвращает все строки из левой таблицы и совпадающие строки из правой таблицы. Если совпадений нет, для столбцов правой таблицы возвращаются NULL. Например, данный запрос извлекает всех клиентов вместе с их заказами с помощью LEFT JOIN. Если соответствующий заказ отсутствует, в результатах будут значения NULL. Результаты сортируются по email клиента, выбираются только имя клиента и количество заказов.

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

## Полнотекстовый поиск по объединённым таблицам

Одной из мощных функций соединений в Manticore Search является возможность выполнять полнотекстовый поиск одновременно в левой и правой таблицах. Это позволяет создавать сложные запросы, фильтрующие данные по текстовому содержимому в нескольких таблицах.

<!-- example fulltext_basic -->

Вы можете использовать отдельные функции `MATCH()` для каждой таблицы в вашем JOIN-запросе. Запрос фильтрует результаты по текстовому содержимому обеих таблиц.

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

### Структура JSON-запроса для соединений

В JSON API запросах полнотекстовый поиск, специфичный для таблиц, структурирован иначе чем в SQL:

<!-- example fulltext_json_structure -->

**Запрос к основной таблице**: Поле `"query"` на корневом уровне применяется к основной таблице (указанной в `"table"`).

**Запрос к присоединённой таблице**: Каждое описание соединения может включать собственное поле `"query"`, которое применяется специально к этой присоединённой таблице.

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

### Понимание поведения запроса при JOIN-операциях

<!-- example fulltext_json_behavior -->

**1. Запрос только к основной таблице**: Возвращает все совпадающие строки из основной таблицы. Для несопоставленных записей присоединённой таблицы (LEFT JOIN) в SQL возвращаются NULL, а в JSON API — значения по умолчанию (0 для чисел, пустые строки для текста).

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
**2. Запрос к присоединённой таблице действует как фильтр**: При наличии запроса к присоединённой таблице возвращаются только записи, удовлетворяющие как условию объединения, так и условию запроса.

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

**3. Тип JOIN влияет на фильтрацию**: INNER JOIN требует выполнения и условия объединения, и условия запроса, тогда как LEFT JOIN возвращает совпадающие строки левой таблицы даже при отсутствии совпадений в правой.


### Важные моменты при полнотекстовом поиске в JOIN

При использовании полнотекстового поиска с соединениями учитывайте следующие моменты:

1. **Поиск, специфичный для таблиц**: 
   - **SQL**: Каждая функция `MATCH()` должна указывать, по какой таблице искать: `MATCH('term', table_name)`
   - **JSON**: Используйте поле `"query"` на корневом уровне для основной таблицы и `"query"` внутри определения каждого соединения для присоединённых таблиц

2. **Гибкость синтаксиса запроса**: JSON API поддерживает синтаксисы `"query_string"` и `"match"` для полнотекстовых запросов

3. **Влияние на производительность**: Полнотекстовый поиск по обеим таблицам может влиять на производительность, особенно при работе с большими наборами данных. Рекомендуется использовать подходящие индексы и размеры батчей.

4. **Обработка NULL/значений по умолчанию**: При LEFT JOIN, если отсутствует совпадающая запись в правой таблице, оптимизатор запроса решает, в каком порядке выполнять полнотекстовые или фильтрующие условия для лучшей производительности. В SQL возвращаются NULL, а в JSON API — значения по умолчанию (0 для чисел, пустые строки для текста).

5. **Поведение фильтрации**: Запросы к присоединённым таблицам действуют как фильтры — они ограничивают результаты записями, удовлетворяющими и условиям соединения, и условиям запроса.

6. **Поддержка полнотекстовых операторов**: Все [операторы полнотекстового поиска](../Searching/Full_text_matching/Operators.md) поддерживаются в JOIN-запросах, включая поиск фраз, близость, поиск по полям, NEAR, согласование кворума и продвинутые операторы.

7. **Вычисление оценки релевантности**: Каждая таблица поддерживает собственный рейтинг релевантности, доступный через `table_name.weight()` в SQL или `table_name._score` в JSON-ответах.

## Пример: Сложное JOIN с фасетными запросами

Основываясь на предыдущих примерах, рассмотрим более продвинутый сценарий, в котором комбинируются соединения таблиц с фасетами и полнотекстовым поиском по нескольким таблицам. Это демонстрирует полную мощь возможностей JOIN в Manticore с комплексной фильтрацией и агрегацией.

<details>

Инициализационные запросы для следующего примера:

```
drop table if exists customers; drop table if exists orders; create table customers(name text, email text, address text); create table orders(product text, customer_id int, quantity int, order_date string, tags multi, details json); insert into customers values (1, 'Alice Johnson', 'alice@example.com', '123 Maple St'), (2, 'Bob Smith', 'bob@example.com', '456 Oak St'), (3, 'Carol White', 'carol@example.com', '789 Pine St'), (4, 'John Smith', 'john@example.com', '15 Barclays St'); insert into orders values (1, 'Laptop Computer', 1, 1, '2023-01-01', (101,102), '{"price":1200,"warranty":"2 years"}'), (2, 'Smart Phone', 2, 2, '2023-01-02', (103), '{"price":800,"warranty":"1 year"}'), (3, 'Tablet Device', 1, 1, '2023-01-03', (101,104), '{"price":450,"warranty":"1 year"}'), (4, 'Monitor Display', 3, 1, '2023-01-04', (105), '{"price":300,"warranty":"1 year"}');
```

</details>

<!-- example basic_complex -->

Этот запрос демонстрирует полнотекстовый поиск одновременно по таблицам `customers` и `orders`, в сочетании с фильтрацией по диапазону и фасетированием. Он ищет клиентов с именами "Alice" или "Bob" и их заказы, содержащие "laptop", "phone" или "tablet" с ценами выше 500 долларов. Результаты упорядочены по идентификатору заказа и фасетированы по условиям гарантии.

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

## Параметры поиска и веса совпадений

Отдельные параметры могут задаваться для запросов в соединении: для левой и правой таблиц. Синтаксис для SQL-запросов — `OPTION(<table_name>)`, для JSON-запросов — один или несколько подобъектов в разделе `"options"`.


<!-- example join_options -->

Вот пример задания различных весов полей для полнотекстового запроса по правой таблице. Чтобы получить веса совпадений через SQL, используйте выражение `<table_name>.weight()`.
В JSON-запросах этот вес представлен как `<table_name>._score`.

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

## Пакетная обработка при объединениях

При выполнении соединений таблиц Manticore Search обрабатывает результаты пакетами для оптимизации производительности и использования ресурсов. Вот как это работает:

- **Как работает пакетирование**:
  - Сначала выполняется запрос по левой таблице, и результаты собираются в пакет.
  - Этот пакет затем используется как входные данные для запроса по правой таблице, который выполняется за одну операцию.
  - Такой подход минимизирует количество запросов к правой таблице, повышая эффективность.

- **Настройка размера пакета**:
  - Размер пакета можно настроить с помощью опции поиска `join_batch_size`.
  - Также его можно [настроить](../Server_settings/Searchd.md#join_batch_size) в разделе `searchd` конфигурационного файла.
  - Размер пакета по умолчанию — `1000`, но вы можете увеличить или уменьшить его в зависимости от сценария.
  - Установка `join_batch_size=0` полностью отключает пакетирование, что может быть полезно для отладки или конкретных случаев.

- **Особенности производительности**:
  - Больший размер пакета может улучшить производительность, уменьшая количество запросов к правой таблице.
  - Однако большие пакеты могут потреблять больше оперативной памяти, особенно для сложных запросов или больших наборов данных.
  - Экспериментируйте с разными размерами пакетов, чтобы найти оптимальный баланс между производительностью и использованием ресурсов.

## Кэширование при объединениях

Для дальнейшей оптимизации операций объединения Manticore Search использует механизм кэширования запросов к правой таблице. Вот что важно знать:

- **Как работает кэширование**:
  - Каждый запрос к правой таблице определяется условиями `JOIN ON`.
  - Если одинаковые условия `JOIN ON` повторяются в нескольких запросах, результаты кэшируются и переиспользуются.
  - Это позволяет избежать избыточных запросов и ускорить последующие операции объединения.

- **Настройка размера кэша**:
  - Размер кэша объединений можно настроить с помощью параметра [join_cache_size](../Server_settings/Searchd.md#join_cache_size) в разделе `searchd` конфигурационного файла.
  - Размер кэша по умолчанию — `20MB`, но его можно менять в зависимости от рабочей нагрузки и доступной памяти.
  - Установка `join_cache_size=0` полностью отключает кэширование.

- **Особенности использования памяти**:
  - Каждый поток поддерживает свой собственный кэш, поэтому общее использование памяти зависит от числа потоков и размера кэша.
  - Убедитесь, что на сервере достаточно памяти для кэша, особенно при высококонкурентных нагрузках.

## Использование распределённых таблиц в объединениях

Распределённые таблицы, состоящие только из локальных таблиц, поддерживаются как с левой, так и с правой стороны запроса объединения. Однако распределённые таблицы, включающие удалённые таблицы, не поддерживаются.

## Особенности и рекомендации

При использовании JOIN в Manticore Search учитывайте следующие моменты:

1. **Выбор полей**: При выборе полей из двух таблиц в JOIN не используйте префиксы для полей левой таблицы, но используйте префиксы для полей правой таблицы. Например:
   ```sql
   SELECT field_name, right_table.field_name FROM ...
   ```

2. **Условия JOIN**: Всегда явно указывайте имена таблиц в условиях JOIN:
   ```sql
   JOIN ON table_name.some_field = another_table_name.some_field
   ```

3. **Выражения с JOIN**: При использовании выражений, объединяющих поля обеих таблиц, давайте результату выражения псевдоним:
   ```sql
   SELECT *, (nums2.n + 3) AS x, x * n FROM nums LEFT JOIN nums2 ON nums2.id = nums.num2_id
   ```

4. **Фильтрация по псевдонимам выражений**: Нельзя использовать псевдонимы выражений, содержащих поля из обеих таблиц, в разделе WHERE.

5. **JSON-атрибуты**: При объединении по JSON-атрибутам необходимо явно приводить значения к нужному типу:
   ```sql
   -- Правильно:
   SELECT * FROM t1 LEFT JOIN t2 ON int(t1.json_attr.id) = t2.json_attr.id

   -- Неправильно:
   SELECT * FROM t1 LEFT JOIN t2 ON t1.json_attr.id = t2.json_attr.id
   ```

6. **Обработка NULL**: Можно использовать условия IS NULL и IS NOT NULL для объединённых полей:
   ```sql
   SELECT * FROM t1 LEFT JOIN t2 ON t1.id = t2.id WHERE t2.name IS NULL
   SELECT * FROM t1 LEFT JOIN t2 ON t1.id = t2.id WHERE t2.name IS NOT NULL
   ```

7. **Использование ANY с MVA**: При использовании функции `ANY()` с мультизначными атрибутами в JOIN, давайте псевдоним мультизначному атрибуту из присоединённой таблицы:
   ```sql
   SELECT *, t2.m AS alias
   FROM t
   LEFT JOIN t2 ON t.id = t2.t_id
   WHERE ANY(alias) IN (3, 5)
   ```

Следуя этим рекомендациям, вы сможете эффективно использовать JOIN в Manticore Search для объединения данных из нескольких индексов и выполнения сложных запросов.

<!-- proofread -->

