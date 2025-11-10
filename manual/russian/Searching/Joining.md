# Объединение таблиц

Объединения таблиц (joins) в Manticore Search позволяют комбинировать документы из двух таблиц путем сопоставления связанных столбцов. Эта функциональность позволяет выполнять более сложные запросы и улучшает извлечение данных из нескольких таблиц.

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
	| [..фильтры по атрибутам правой таблицы]
}
```

Для получения дополнительной информации об опциях select обращайтесь к разделу [SELECT](../Searching/Intro.md#General-syntax).

<!--example join_sql_json_type -->

При объединении по значению из JSON-атрибута необходимо явно указать тип значения с помощью функции `int()` или `string()`.

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
Обратите внимание, что в секции операнда `left` есть поле `type`, которое следует использовать при объединении двух таблиц с помощью JSON-атрибутов. Допустимые значения: `string` и `int`.

## Типы объединений

Manticore Search поддерживает два типа объединений:

<!-- example inner_basic -->

1. **INNER JOIN**: Возвращает только те строки, где есть совпадение в обеих таблицах. Например, запрос выполняет INNER JOIN между таблицами `orders` и `customers`, включая только заказы, которые имеют соответствующих клиентов.

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

2. **LEFT JOIN**: Возвращает все строки из левой таблицы и совпадающие строки из правой таблицы. Если совпадений нет, в столбцах правой таблицы возвращаются значения NULL. Например, этот запрос извлекает всех клиентов с их соответствующими заказами с использованием LEFT JOIN. Если соответствующего заказа нет, будут показаны значения NULL. Результаты сортируются по электронной почте клиента, и выбираются только имя клиента и количество заказов.

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

Одна из мощных функций объединений таблиц в Manticore Search — возможность выполнять полнотекстовый поиск одновременно по левой и правой таблицам. Это позволяет создавать сложные запросы, фильтрующие данные на основе текстового содержания в нескольких таблицах.

<!-- example fulltext_basic -->

Вы можете использовать отдельные функции `MATCH()` для каждой таблицы в вашем JOIN-запросе. Запрос фильтрует результаты по текстовому содержанию в обеих таблицах.

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

### Структура JSON-запроса для объединений

В запросах через JSON API полнотекстовый поиск по каждой таблице структурирован иначе, чем в SQL:

<!-- example fulltext_json_structure -->

**Запрос основной таблицы**: поле `"query"` на корневом уровне применяется к основной таблице (указанной в `"table"`).

**Запрос присоединённой таблицы**: каждый элемент определения JOIN может содержать собственное поле `"query"`, которое применяется именно к этой присоединенной таблице.

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

### Понимание поведения запроса в операциях JOIN

<!-- example fulltext_json_behavior -->

**1. Запрос только к основной таблице**: Возвращает все совпадающие строки из основной таблицы. Для несопоставленных записей присоединённой таблицы (LEFT JOIN) SQL возвращает значения NULL, а JSON API — значения по умолчанию (0 для чисел, пустые строки для текста).

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
**2. Запрос к присоединённой таблице действует как фильтр**: Когда в присоединённой таблице указан запрос, возвращаются только те записи, которые удовлетворяют как условию объединения, так и условию запроса.

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

**3. Тип JOIN влияет на фильтрацию**: INNER JOIN требует выполнения и условия объединения, и условия запроса, а LEFT JOIN возвращает совпадающие строки из левой таблицы, даже если условия правой таблицы не выполняются.


### Важные моменты для полнотекстового поиска при JOIN

При использовании полнотекстового поиска с объединениями обратите внимание на следующее:

1. **Поиск по конкретным таблицам**: 
   - **SQL**: Каждая функция `MATCH()` должна указывать, в какой таблице искать: `MATCH('term', table_name)`
   - **JSON**: Используется корневое поле `"query"` для основной таблицы, а `"query"` внутри определения join — для присоединённых таблиц

2. **Гибкость синтаксиса запросов**: JSON API поддерживает синтаксисы `"query_string"` и `"match"` для полнотекстовых запросов

3. **Влияние на производительность**: Полнотекстовый поиск по обеим таблицам может влиять на производительность, особенно при больших объемах данных. Рекомендуется использовать соответствующие индексы и размеры пакетов.

4. **Обработка NULL/значений по умолчанию**: При LEFT JOIN, если соответствующая запись отсутствует в правой таблице, оптимизатор запроса решает, оценивать ли полнотекстовые условия или условия фильтрации первыми с учетом производительности. SQL возвращает NULL, JSON API — значения по умолчанию (0 для чисел, пустые строки для текста).

5. **Поведение фильтрации**: Запросы к присоединённым таблицам действуют как фильтры — они ограничивают результаты записями, которые удовлетворяют как условию объединения, так и условию запроса.

6. **Поддержка полнотекстовых операторов**: В JOIN-запросах поддерживаются все [операторы полнотекстового поиска](../Searching/Full_text_matching/Operators.md), включая поиск фраз, приблизительный поиск, поиск по полям, NEAR, кворум и продвинутые операторы.

7. **Вычисление оценки релевантности**: Каждая таблица хранит собственный счет релевантности, доступный через `table_name.weight()` в SQL или `table_name._score` в JSON-ответах.

## Пример: Сложный JOIN с фасетированием

Опираясь на предыдущие примеры, рассмотрим более сложный сценарий, где объединяются таблицы с фасетированием и полнотекстовым поиском по нескольким таблицам. Это демонстрирует полную мощь возможностей JOIN в Manticore с комплексной фильтрацией и агрегацией.

<details>

Инициализационные запросы для следующего примера:

```
drop table if exists customers; drop table if exists orders; create table customers(name text, email text, address text); create table orders(product text, customer_id int, quantity int, order_date string, tags multi, details json); insert into customers values (1, 'Alice Johnson', 'alice@example.com', '123 Maple St'), (2, 'Bob Smith', 'bob@example.com', '456 Oak St'), (3, 'Carol White', 'carol@example.com', '789 Pine St'), (4, 'John Smith', 'john@example.com', '15 Barclays St'); insert into orders values (1, 'Laptop Computer', 1, 1, '2023-01-01', (101,102), '{"price":1200,"warranty":"2 years"}'), (2, 'Smart Phone', 2, 2, '2023-01-02', (103), '{"price":800,"warranty":"1 year"}'), (3, 'Tablet Device', 1, 1, '2023-01-03', (101,104), '{"price":450,"warranty":"1 year"}'), (4, 'Monitor Display', 3, 1, '2023-01-04', (105), '{"price":300,"warranty":"1 year"}');
```

</details>

<!-- example basic_complex -->

Этот запрос демонстрирует полнотекстовый поиск по обеим таблицам `customers` и `orders`, в сочетании с фильтрацией по диапазону и фасетным поиском. Он ищет клиентов с именами "Alice" или "Bob" и их заказы, содержащие "laptop", "phone" или "tablet" с ценами выше $500. Результаты упорядочены по ID заказа и фасетированы по условиям гарантии.

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

## Опции поиска и веса совпадений

Отдельные опции могут быть заданы для запросов в join: для левой таблицы и правой таблицы. Синтаксис — `OPTION(<table_name>)` для SQL-запросов и один или несколько подобъектов под `"options"` для JSON-запросов.


<!-- example join_options -->

Вот пример того, как задать разные веса полей для полнотекстового запроса к правой таблице. Чтобы получить веса совпадений через SQL, используйте выражение `<table_name>.weight()`.
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

## Пакетная обработка join

При выполнении join Manticore Search обрабатывает результаты пакетами для оптимизации производительности и использования ресурсов. Вот как это работает:

- **Как работает пакетная обработка**:
  - Сначала выполняется запрос к левой таблице, и результаты накапливаются в пакет.
  - Этот пакет затем используется как входные данные для запроса к правой таблице, который выполняется как единая операция.
  - Такой подход минимизирует количество запросов к правой таблице, повышая эффективность.

- **Настройка размера пакета**:
  - Размер пакета можно настроить с помощью опции поиска `join_batch_size`.
  - Его также можно [настроить](../Server_settings/Searchd.md#join_batch_size) в разделе `searchd` файла конфигурации.
  - Размер пакета по умолчанию — `1000`, но вы можете увеличить или уменьшить его в зависимости от вашего сценария использования.
  - Значение `join_batch_size=0` полностью отключает пакетную обработку, что может быть полезно для отладки или специфических случаев.

- **Особенности производительности**:
  - Больший размер пакета может улучшить производительность за счет уменьшения количества запросов к правой таблице.
  - Однако большие пакеты могут потреблять больше памяти, особенно при сложных запросах или больших данных.
  - Экспериментируйте с разными размерами пакетов, чтобы найти оптимальный баланс между производительностью и использованием ресурсов.

## Кэширование join

Для дальнейшей оптимизации операций join Manticore Search использует механизм кэширования для запросов, выполняемых к правой таблице. Вот что важно знать:

- **Как работает кэширование**:
  - Каждый запрос к правой таблице определяется условиями `JOIN ON`.
  - Если одни и те же условия `JOIN ON` повторяются в нескольких запросах, результаты кэшируются и повторно используются.
  - Это предотвращает избыточные запросы и ускоряет последующие операции join.

- **Настройка размера кэша**:
  - Размер кэша join можно настроить с помощью опции [join_cache_size](../Server_settings/Searchd.md#join_cache_size) в разделе `searchd` файла конфигурации.
  - Размер кэша по умолчанию — `20MB`, но вы можете изменить его в зависимости от нагрузки и доступной памяти.
  - Значение `join_cache_size=0` полностью отключает кэширование.

- **Особенности использования памяти**:
  - Каждый поток поддерживает собственный кэш, поэтому общее потребление памяти зависит от количества потоков и размера кэша.
  - Убедитесь, что ваш сервер имеет достаточный объем памяти для размещения кэша, особенно в условиях высокой конкуренции.

## Join с распределёнными таблицами

Распределённые таблицы, состоящие только из локальных таблиц, поддерживаются с обеих сторон оператора join. Однако распределённые таблицы, включающие удалённые таблицы, не поддерживаются.

## Особенности и лучшие практики

При использовании JOIN в Manticore Search учитывайте следующие моменты:

1. **Выбор полей**: При выборе полей из двух таблиц в JOIN не указывайте префиксы для полей левой таблицы, но указывайте префиксы для полей правой таблицы. Например:
   ```sql
   SELECT field_name, right_table.field_name FROM ...
   ```

2. **Условия JOIN**: Всегда явно указывайте имена таблиц в условиях JOIN:
   ```sql
   JOIN ON table_name.some_field = another_table_name.some_field
   ```

3. **Выражения с JOIN**: При использовании выражений, объединяющих поля обеих таблиц, назначайте псевдоним результату выражения:
   ```sql
   SELECT *, (nums2.n + 3) AS x, x * n FROM nums LEFT JOIN nums2 ON nums2.id = nums.num2_id
   ```

4. **Фильтрация по псевдонимам выражений**: В WHERE нельзя использовать псевдонимы для выражений, включающих поля обеих таблиц.

5. **JSON-атрибуты**: При join по JSON-атрибутам необходимо явно приводить значения к нужному типу:
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

7. **Использование ANY с MVA**: При использовании функции `ANY()` с многозначными атрибутами в JOIN назначайте псевдоним многозначному атрибуту из объединённой таблицы:
   ```sql
   SELECT *, t2.m AS alias
   FROM t
   LEFT JOIN t2 ON t.id = t2.t_id
   WHERE ANY(alias) IN (3, 5)
   ```

Следуя этим рекомендациям, вы сможете эффективно использовать JOIN в Manticore Search для объединения данных из нескольких индексов и выполнения сложных запросов.

<!-- proofread -->

