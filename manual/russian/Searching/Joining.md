# Соединение таблиц

Соединения таблиц в Manticore Search позволяют комбинировать документы из двух таблиц, сопоставляя связанные столбцы. Эта функциональность позволяет выполнять более сложные запросы и улучшать извлечение данных из нескольких таблиц.

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

Для получения дополнительной информации по параметрам select обращайтесь к разделу [SELECT](../Searching/Intro.md#General-syntax).

<!--example join_sql_json_type -->

При соединении по значению из JSON-атрибута необходимо явно указать тип значения с помощью функции `int()` или `string()`.

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
Обратите внимание, что в разделе операнда `left` есть поле `type`, которое следует использовать при соединении двух таблиц через JSON-атрибуты. Допустимые значения — `string` и `int`.

## Типы соединений

Manticore Search поддерживает два типа соединений:

<!-- example inner_basic -->

1. **INNER JOIN**: Возвращает только те строки, где есть совпадение в обеих таблицах. Например, запрос выполняет INNER JOIN между таблицами `orders` и `customers`, включая только заказы, у которых есть соответствующие клиенты.

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

2. **LEFT JOIN**: Возвращает все строки из левой таблицы и совпадающие строки из правой таблицы. Если совпадения нет, для столбцов правой таблицы возвращаются значения NULL. Например, этот запрос извлекает всех клиентов вместе с их заказами с помощью LEFT JOIN. Если сопоставимого заказа нет, будут отображены значения NULL. Результаты сортируются по электронной почте клиента, а выбираются только имя клиента и количество заказов.

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

### Пример: Сложное соединение с фасетированием

Расширяя предыдущие примеры, рассмотрим более сложный сценарий, где мы объединяем соединения таблиц с фасетированием. Это позволяет не только извлекать объединённые данные, но и агрегировать и анализировать их значимым образом.

<!-- example basic_complex -->

Этот запрос извлекает продукты, имена клиентов, цены продуктов и теги продуктов из таблиц `orders` и `customers`. Он выполняет `LEFT JOIN`, обеспечивая включение всех клиентов, даже если они не сделали заказ. Запрос фильтрует результаты, включая только заказы с ценой больше `500` и сопоставляет продукты с терминами 'laptop', 'phone' или 'monitor'. Результаты сортируются по возрастанию `id` заказов. Дополнительно запрос фасетирует результаты на основе данных о гарантии из JSON-атрибутов объединённой таблицы `orders`.

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

## Опции поиска и веса совпадений

В параметрах запроса в соединении можно задать отдельные опции как для левой, так и для правой таблицы. Синтаксис: `OPTION(<table_name>)` для SQL-запросов и один или несколько подобъектов внутри `"options"` для JSON-запросов.


<!-- example join_options -->

Пример задания разных весов полей для полнотекстового запроса к правой таблице. Чтобы получить веса совпадений через SQL, используйте выражение `<table_name>.weight()`.
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

## Пакетная обработка соединений

При выполнении соединений таблиц Manticore Search обрабатывает результаты пакетами для оптимизации производительности и использования ресурсов. Работа происходит следующим образом:

- **Как работает пакетная обработка**:
  - Сначала выполняется запрос к левой таблице, и результаты накапливаются в пакет.
  - Затем этот пакет используется в качестве входных данных для запроса к правой таблице, который выполняется за одну операцию.
  - Такой подход минимизирует количество запросов к правой таблице, повышая эффективность.

- **Настройка размера пакета**:
  - Размер пакета можно изменить с помощью опции поиска `join_batch_size`.
  - Он также настраивается в разделе `searchd` конфигурационного файла [конфигурации](../Server_settings/Searchd.md#join_batch_size).
  - Размер пакета по умолчанию — `1000`, но вы можете увеличить или уменьшить его в зависимости от задачи.
  - Установка `join_batch_size=0` полностью отключает пакетную обработку, что может быть полезно для отладки или специфических сценариев.

- **Рекомендации по производительности**:
  - Больший размер пакета улучшает производительность за счёт уменьшения количества запросов к правой таблице.
  - Однако большие пакеты могут потреблять больше памяти, особенно для сложных запросов или больших наборов данных.
  - Экспериментируйте с размером пакета для нахождения оптимального баланса между производительностью и использованием ресурсов.

## Кэширование соединений

Для дополнительной оптимизации операций соединения Manticore Search использует механизм кэширования запросов, выполняемых к правой таблице. Вот основные сведения:

- **Как работает кэширование**:
  - Каждый запрос к правой таблице определяется условиями `JOIN ON`.
  - Если одни и те же условия `JOIN ON` повторяются в нескольких запросах, результаты кэшируются и используются повторно.
  - Это предотвращает дублирование запросов и ускоряет последующие операции соединения.

- **Настройка размера кэша**:
  - Размер кэша соединения конфигурируется через параметр [join_cache_size](../Server_settings/Searchd.md#join_cache_size) в разделе `searchd` конфигурационного файла.
  - Размер кэша по умолчанию — `20MB`, но вы можете изменить его в зависимости от нагрузки и доступной памяти.
  - Установка `join_cache_size=0` полностью отключает кэширование.

- **Особенности использования памяти**:
  - Каждый поток поддерживает собственный кэш, поэтому суммарное использование памяти зависит от количества потоков и размера кэша.
  - Убедитесь, что на вашем сервере достаточно памяти для кеша, особенно в средах с высокой конкурентностью.


## Особенности и рекомендации

При использовании JOIN в Manticore Search учитывайте следующие моменты:

1. **Выбор полей**: При выборе полей из двух таблиц в JOIN не указывайте префикс для полей из левой таблицы, но указывайте префикс для полей из правой таблицы. Например:
   ```sql
   SELECT field_name, right_table.field_name FROM ...
   ```

2. **Условия JOIN**: Всегда явно указывайте имена таблиц в условиях JOIN:
   ```sql
   JOIN ON table_name.some_field = another_table_name.some_field
   ```

3. **Выражения с JOIN**: При использовании выражений, объединяющих поля из обеих таблиц, присваивайте псевдоним результату выражения:
   ```sql
   SELECT *, (nums2.n + 3) AS x, x * n FROM nums LEFT JOIN nums2 ON nums2.id = nums.num2_id
   ```

4. **Фильтрация по псевдонимам выражений**: Нельзя использовать псевдонимы для выражений, включающих поля из обеих таблиц, в условии WHERE.

5. **JSON-атрибуты**: При соединении по JSON-атрибутам необходимо явно приводить значения к соответствующему типу:
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

7. **Использование ANY с MVA**: При использовании функции `ANY()` с мультизначными атрибутами в JOIN присваивайте псевдоним мультизначному атрибуту из присоединённой таблицы:
   ```sql
   SELECT *, t2.m AS alias
   FROM t
   LEFT JOIN t2 ON t.id = t2.t_id
   WHERE ANY(alias) IN (3, 5)
   ```

Следуя этим рекомендациям, вы сможете эффективно использовать JOIN в Manticore Search для объединения данных из нескольких индексов и выполнения сложных запросов.

<!-- proofread -->

