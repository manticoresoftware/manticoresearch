# Объединение таблиц

Объединение таблиц в Manticore Search позволяет комбинировать документы из двух таблиц, сопоставляя связанные столбцы. Эта функциональность позволяет создавать более сложные запросы и улучшать извлечение данных из нескольких таблиц.

## Общий синтаксис

### SQL

```sql
SELECT
	select_expr [, select_expr] ...
	FROM tbl_name
	{INNER | LEFT} JOIN tbl2_name
	ON join_condition
	[...другие параметры выбора]

join_condition: {
	left_table.attr = right_table.attr
	| left_table.json_attr.string_id = string(right_table.json_attr.string_id)
	| left_table.json_attr.int_id = int(right_table.json_attr.int_id)
}
```

Для получения дополнительной информации о параметрах выбора обратитесь к разделу [SELECT](../Searching/Intro.md#General-syntax).

<!--example join_sql_json_type -->

При объединении по значению из атрибута JSON необходимо явно указать тип значения, используя функцию `int()` или `string()`.

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
    <необязательный полный текстовый запрос против левой таблицы>
  },
  "join": [
    {
      "type": "inner" | "left",
      "table": "joined_table_name",
      "query": {
        <необязательный полный текстовый запрос против правой таблицы>
      },
      "on": [
        {
          "left": {
            "table": "left_table_name",
            "field": "field_name",
            "type": "<тип общего поля при объединении с использованием атрибутов json>"
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
Обратите внимание, что в разделе операнда `left` есть поле `type`, которое вы должны использовать при объединении двух таблиц с использованием атрибутов json. Допустимые значения: `string` и `int`.

## Виды объединения

Manticore Search поддерживает два типа объединений:

<!-- example inner_basic -->

1. **INNER JOIN**: Возвращает только те строки, в которых есть совпадения в обеих таблицах. Например, запрос выполняет INNER JOIN между таблицами `orders` и `customers`, включая только те заказы, которые имеют совпадающих клиентов.

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

2. **LEFT JOIN**: Возвращает все строки из левой таблицы и совпадающие строки из правой таблицы. Если совпадения нет, возвращаются значения NULL для столбцов правой таблицы. Например, этот запрос извлекает всех клиентов вместе с их соответствующими заказами, используя LEFT JOIN. Если соответствующий заказ отсутствует, появятся значения NULL. Результаты сортируются по электронной почте клиента, и выбираются только имя клиента и количество заказа.

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

Основываясь на предыдущих примерах, давайте рассмотрим более сложный сценарий, в котором мы комбинируем соединения таблиц с фасетированием. Это позволяет нам не только получать объединённые данные, но и агрегировать и анализировать их осмысленным образом.

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
| Ноутбук        | Элис Джонсон  |                 1200 | 101,102     |
| Телефон        | Боб Смит      |                  800 | 103         |
+----------------+---------------+----------------------+-------------+
2 строки в наборе (0.01 сек)
--- 2 из 2 результатов за 0мс ---

+-------------------------+----------+
| orders.details.warranty | count(*) |
+-------------------------+----------+
| 2 года                 |        1 |
| 1 год                  |        1 |
+-------------------------+----------+
2 строки в наборе (0.01 сек)
--- 2 из 2 результатов за 0мс ---
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
          "name": "Элис Джонсон",
          "orders.tags": [
            101,
            102
          ],
          "orders.details": {
            "price": 1200,
            "warranty": "2 года"
          },
          "orders.product": "Ноутбук"
        }
      },
      {
        "_id": 2,
        "_score": 1,
        "_source": {
          "name": "Боб Смит",
          "orders.tags": [
            103
          ],
          "orders.details": {
            "price": 800,
            "warranty": "1 год"
          },
          "orders.product": "Телефон"
        }
      }
    ]
  },
  "aggregations": {
    "group_property": {
      "buckets": [
        {
          "key": "1 год",
          "doc_count": 1
        },
        {
          "key": "2 года",
          "doc_count": 1
        }
      ]
    }
  }
}
```


<!-- end -->

## Параметры поиска и вес совпадений

Разные параметры могут быть указаны для запросов в соединении: для левой таблицы и правой таблицы. Синтаксис — `OPTION(<table_name>)` для SQL-запросов и один или несколько подпунктов под `"options"` для JSON-запросов.


<!-- example join_options -->

Вот пример того, как указать разные веса полей для полнотекстового запроса на правой таблице. Чтобы получить веса совпадений через SQL, используйте выражение `<table_name>.weight()`.
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
| Ноутбук | alice@example.com | Элис Джонсон  | 123 Maple St      |            1500680 |
| Планшет | alice@example.com | Элис Джонсон  | 123 Maple St      |            1500680 |
+---------+-------------------+----------------+-------------------+--------------------+
2 строки в наборе (0.00 сек)
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
          "product": "Ноутбук",
          "customers.email": "alice@example.com",
          "customers.name": "Элис Джонсон",
          "customers.address": "123 Maple St"
        }
      },
      {
        "_id": 3,
        "_score": 1,
        "customers._score": 15000680,
        "_source": {
          "product": "Планшет",
          "customers.email": "alice@example.com",
          "customers.name": "Элис Джонсон",
          "customers.address": "123 Maple St"
        }
      }
    ]
  }
}
```
<!-- end -->

## Пакетирование соединений

При выполнении соединений таблиц Manticore Search обрабатывает результаты пакетами для оптимизации производительности и использования ресурсов. Вот как это работает:

- **Как работает пакетирование**: 
  - Запрос к левой таблице выполняется первым, и результаты аккумулируются в пакет.
  - Этот пакет затем используется в качестве входных данных для запроса к правой таблице, который выполняется как единая операция.
  - Этот подход минимизирует количество запросов, отправляемых к правой таблице, улучшая эффективность.

- **Настройка размера пакета**:
  - Размер пакета можно регулировать с помощью опции поиска `join_batch_size`.
  - Он также [настраиваемый](../../Server_settings/Searchd.md#join_batch_size) в разделе `searchd` конфигурационного файла.
  - Размер пакета по умолчанию составляет `1000`, но его можно увеличить или уменьшить в зависимости от вашего использования.
  - Установка `join_batch_size=0` полностью отключает пакетирование, что может быть полезно для отладки или конкретных сценариев.

- **Соображения по производительности**:
  - Больший размер пакета может улучшить производительность, снижая количество запросов, выполняемых на правой таблице.
  - Однако большие пакеты могут потреблять больше памяти, особенно для сложных запросов или больших наборов данных.
  - Экспериментируйте с разными размерами пакетов, чтобы найти оптимальный баланс между производительностью и использованием ресурсов.

## Кэширование соединений

Для дальнейшей оптимизации операций соединения Manticore Search использует механизм кэширования для запросов, выполняемых на правой таблице. Вот что вам нужно знать:

- **Как работает кэширование**:
  - Каждый запрос к правой таблице определяется условиями `JOIN ON`.
  - Если одни и те же условия `JOIN ON` повторяются в нескольких запросах, результаты кэшируются и переиспользуются.
  - Это избегает избыточных запросов и ускоряет последующие операции соединения.

- **Настройка размера кэша**:
  - Размер кэша соединений можно настроить с помощью параметра [join_cache_size](../../Server_settings/Searchd.md#join_cache_size) в секции `searchd` конфигурационного файла.
  - Размер кэша по умолчанию составляет `20MB`, но вы можете настроить его в зависимости от вашей нагрузки и доступной памяти.
  - Установка `join_cache_size=0` отключает кэширование полностью.

- **Учет памяти**:
  - Каждый поток поддерживает свой собственный кэш, поэтому общее потребление памяти зависит от количества потоков и размера кэша.
  - Убедитесь, что у вашего сервера достаточно памяти для размещения кэша, особенно в средах с высокойConcurrency.


## Предостережения и лучшие практики

При использовании JOIN'ов в Manticore Search имейте в виду следующие моменты:

1. **Выбор полей**: При выборе полей из двух таблиц в JOIN не ставьте префиксы к полям из левой таблицы, но ставьте префиксы к полям из правой таблицы. Например:
   ```sql
   SELECT field_name, right_table.field_name FROM ...
   ```

2. **Условия JOIN**: Всегда явно указывайте имена таблиц в условиях JOIN:
   ```sql
   JOIN ON table_name.some_field = another_table_name.some_field
   ```

3. **Выражения с JOIN'ами**: При использовании выражений, которые объединяют поля из обеих связанных таблиц, задайте псевдоним для результата выражения:
   ```sql
   SELECT *, (nums2.n + 3) AS x, x * n FROM nums LEFT JOIN nums2 ON nums2.id = nums.num2_id
   ```

4. **Фильтрация по алиасам выражений**: Вы не можете использовать алиасы для выражений, связанных с полями из обеих таблиц в условии WHERE.

5. **Атрибуты JSON**: При соединении по атрибутам JSON вы должны явно привести значения к соответствующему типу:
   ```sql
   -- Правильно:
   SELECT * FROM t1 LEFT JOIN t2 ON int(t1.json_attr.id) = t2.json_attr.id
   
   -- Неправильно:
   SELECT * FROM t1 LEFT JOIN t2 ON t1.json_attr.id = t2.json_attr.id
   ```

6. **Обработка NULL**: Вы можете использовать условия IS NULL и IS NOT NULL для полей, связанных с JOIN:
   ```sql
   SELECT * FROM t1 LEFT JOIN t2 ON t1.id = t2.id WHERE t2.name IS NULL
   SELECT * FROM t1 LEFT JOIN t2 ON t1.id = t2.id WHERE t2.name IS NOT NULL
   ```

7. **Использование ANY с MVA**: При использовании функции `ANY()` с многозначными атрибутами в JOIN'ах создайте алиас для многозначного атрибута из связанной таблицы:
   ```sql
   SELECT *, t2.m AS alias
   FROM t
   LEFT JOIN t2 ON t.id = t2.t_id
   WHERE ANY(alias) IN (3, 5)
   ```

