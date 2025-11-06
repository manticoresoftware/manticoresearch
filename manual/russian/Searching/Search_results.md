# Результаты поиска

## SQL

<!-- example sql1 -->
Когда вы выполняете запрос через SQL по протоколу MySQL, вы получаете в результате запрошенные столбцы или пустой набор результатов, если ничего не найдено.

<!-- request SQL -->
```sql
SELECT * FROM tbl;
```

<!-- response SQL -->
```sql
+------+------+--------+
| id   | age  | name   |
+------+------+--------+
|    1 |   25 | joe    |
|    2 |   25 | mary   |
|    3 |   33 | albert |
+------+------+--------+
3 rows in set (0.00 sec)
```
<!-- request JSON -->
```JSON
POST /sql -d "SELECT * FROM tbl;"
```

<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "id": {
          "type": "long long"
        }
      },
      {
        "age": {
          "type": "long"
        }
      },
      {
        "name": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "id": 1,
        "age": "joe",
        "name": 25
      },
      {
        "id": 2,
        "age": "mary",
        "name": 255
      },
      {
        "id": 3,
        "age": "albert",
        "name": 33
      }
    ],
    "total": 3,
    "error": "",
    "warning": ""
  }
]

```

<!-- end -->

<!-- example sql2 -->
Дополнительно вы можете использовать вызов [SHOW META](../Node_info_and_management/SHOW_META.md), чтобы увидеть дополнительную метаинформацию о последнем запросе.

<!-- request SQL -->
```sql
SELECT id,story_author,comment_author FROM hn_small WHERE story_author='joe' LIMIT 3; SHOW META;
```

<!-- response SQL -->
```sql
++--------+--------------+----------------+
| id     | story_author | comment_author |
+--------+--------------+----------------+
| 152841 | joe          | SwellJoe       |
| 161323 | joe          | samb           |
| 163735 | joe          | jsjenkins168   |
+--------+--------------+----------------+
3 rows in set (0.01 sec)

+----------------+-------+
| Variable_name  | Value |
+----------------+-------+
| total          | 3     |
| total_found    | 20    |
| total_relation | gte   |
| time           | 0.010 |
+----------------+-------+
4 rows in set (0.00 sec)
```
<!-- end -->
<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SELECT id,f1,f2 FROM t WHERE f2=2 LIMIT 1; SHOW META;"
```

<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "id": {
          "type": "long long"
        }
      },
      {
        "f1": {
          "type": "string"
        }
      },
      {
        "f2": {
          "type": "long"
        }
      }
    ],
    "data": [
      {
        "id": 724024784404348900,
        "f1": "b",
        "f2": 2
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  },
  {
    "columns": [
      {
        "Variable_name": {
          "type": "string"
        }
      },
      {
        "Value": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "Variable_name": "total",
        "Value": "1"
      },
      {
        "Variable_name": "total_found",
        "Value": "1"
      },
      {
        "Variable_name": "total_relation",
        "Value": "gte"
      },
      {
        "Variable_name": "time",
        "Value": "0.000"
      }
    ],
    "total": 4,
    "error": "",
    "warning": ""
  }
]
```


<!-- example sql3 -->
В некоторых случаях, например при выполнении [фасетного поиска](../Searching/Faceted_search.md), вы можете получить несколько наборов результатов в ответ на ваш SQL-запрос.

<!-- request SQL -->
```sql
SELECT * FROM tbl WHERE MATCH('joe') FACET age;
```

<!-- response SQL -->
```sql
+------+------+
| id   | age  |
+------+------+
|    1 |   25 |
+------+------+
1 row in set (0.00 sec)

+------+----------+
| age  | count(*) |
+------+----------+
|   25 |        1 |
+------+----------+
1 row in set (0.00 sec)
```
<!-- end -->

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SELECT * FROM tbl WHERE MATCH('b') FACET f2;"
```

<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "id": {
          "type": "long long"
        }
      },
      {
        "f1": {
          "type": "string"
        }
      },
      {
        "f2": {
          "type": "long"
        }
      }
    ],
    "data": [
      {
        "id": 724024784404348900,
        "f1": "b",
        "f2": 2
      },
      {
        "id": 724024784404348900,
        "f1": "b",
        "f2": 2
      }
    ],
    "total": 2,
    "error": "",
    "warning": ""
  },
  {
    "columns": [
      {
        "f2": {
          "type": "long"
        }
      },
      {
        "count(*)": {
          "type": "long long"
        }
      }
    ],
    "data": [
      {
        "f2": 2,
        "count(*)": 2
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```

<!-- example sql4 -->
В случае предупреждения набор результатов будет включать флаг предупреждения, и вы можете увидеть предупреждение с помощью [SHOW WARNINGS](../Node_info_and_management/SHOW_WARNINGS.md).
<!-- request SQL -->
```sql
SELECT * from tbl where match('"joe"/3'); show warnings;
```

<!-- response SQL -->
```sql
+------+------+------+
| id   | age  | name |
+------+------+------+
|    1 |   25 | joe  |
+------+------+------+
1 row in set, 1 warning (0.00 sec)

+---------+------+--------------------------------------------------------------------------------------------+
| Level   | Code | Message                                                                                    |
+---------+------+--------------------------------------------------------------------------------------------+
| warning | 1000 | quorum threshold too high (words=1, thresh=3); replacing quorum operator with AND operator |
+---------+------+--------------------------------------------------------------------------------------------+
1 row in set (0.00 sec)
```
<!-- end -->

<!-- example sql5 -->
<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SELECT * from t where match('\"a\"/3'); show warnings;"
```

<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "id": {
          "type": "long long"
        }
      },
      {
        "f2": {
          "type": "long"
        }
      },
      {
        "f1": {
          "type": "string"
        }
      }
    ],
    "data": [],
    "total": 0,
    "error": "",
    "warning": ""
  },
  {
    "columns": [
      {
        "Level": {
          "type": "string"
        }
      },
      {
        "Code": {
          "type": "decimal"
        }
      },
      {
        "Message": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "Level": "warning",
        "Code": "1000",
        "Message": "table t: quorum threshold too high (words=1, thresh=3); replacing quorum operator with AND operator"
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```

Если ваш запрос завершается с ошибкой, вы получите сообщение об ошибке:

<!-- request SQL -->
```sql
SELECT * from tbl where match('@surname joe');
```

<!-- response SQL -->
```sql
ERROR 1064 (42000): index idx: query error: no field 'surname' found in schema
```

<!-- end -->


## HTTP
<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SELECT * from t where match('@surname joe');"
```

<!-- response JSON -->
```JSON
{
  "error": "table t: query error: no field 'surname' found in schema"
}
```


Через HTTP JSON интерфейс результат запроса передается в виде JSON-документа. Пример:

```json
{
  "took":10,
  "timed_out": false,
  "hits":
  {
    "total": 2,
    "hits":
    [
      {
        "_id": 1,
        "_score": 1,
        "_source": { "gid": 11 }
      },
      {
        "_id": 2,
        "_score": 1,
        "_source": { "gid": 12 }
      }
    ]
  }
}
```

* `took`: время в миллисекундах, затраченное на выполнение поиска
* `timed_out`: произошла ли тайм-аут запросa или нет
* `hits`: результаты поиска, с следующими свойствами:
  - `total`: общее количество совпадающих документов
  - `hits`: массив, содержащий совпадения

Результат запроса может также включать информацию о профилировании запроса. См. [Профилирование запроса](../Node_info_and_management/Profiling/Query_profile.md).

Каждое совпадение в массиве `hits` имеет следующие свойства:

* `_id`: идентификатор совпадения
* `_score`: вес совпадения, вычисленный ранкером
* `_source`: массив, содержащий атрибуты этого совпадения

### Выбор источника

По умолчанию, все атрибуты возвращаются в массиве `_source`. Вы можете использовать свойство `_source` в теле запроса, чтобы выбрать поля, которые хотите включить в результирующий набор. Пример:

```json
{
  "table":"test",
  "_source":"attr*",
  "query": { "match_all": {} }
}
```

Вы можете указать атрибуты, которые хотите включить в результат запроса, как строку (`"_source": "attr*"`) или как массив строк (`"_source": [ "attr1", "attri*" ]"`). Каждая запись может быть именем атрибута или шаблоном с подстановочными знаками (`*`, `%` и `?` поддерживаются).

Вы также можете явно указать, какие атрибуты вы хотите включить, а какие исключить из результата, используя свойства `includes` и `excludes`:

```json
"_source":
{
  "includes": [ "attr1", "attri*" ],
  "excludes": [ "*desc*" ]
}
```

Пустой список includes интерпретируется как «включить все атрибуты», тогда как пустой список excludes не совпадает ни с чем. Если атрибут совпадает и с includes, и с excludes, то выигрывает excludes.

<!-- proofread -->




