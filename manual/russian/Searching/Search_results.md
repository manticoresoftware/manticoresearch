# Результаты поиска

## SQL

<!--
data for the following example:

DROP TABLE IF EXISTS tbl;
CREATE TABLE tbl(id bigint, age int, name string);
INSERT INTO tbl (id, age, name) VALUES
(1, 25, 'joe'),
(2, 25, 'mary'),
(3, 33, 'albert');
-->

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
POST /sql -d "SELECT * FROM tbl"
```

<!-- response JSON -->
```JSON
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 3,
    "total_relation": "eq",
    "hits": [
      {
        "_id": 1,
        "_score": 2500,
        "_source": {
          "age": 25,
          "name": "joe"
        }
      },
      {
        "_id": 2,
        "_score": 2500,
        "_source": {
          "age": 25,
          "name": "mary"
        }
      },
      {
        "_id": 3,
        "_score": 2500,
        "_source": {
          "age": 33,
          "name": "albert"
        }
      }
    ]
  }
}
```

<!-- end -->

<!--
data for the following example:

DROP TABLE IF EXISTS hn_small;
CREATE TABLE hn_small(story_author string, comment_author string);
INSERT INTO hn_small(story_author, comment_author) VALUES
('anewkid', 'skrtio'),
('sasjri', 'samb'),
('bks', 'jsjenkins168'),
('joe', 'SwellJoe'),
('joe', 'samb'),
('joe', 'jsjenkins168');
--> 

<!-- example sql2 -->
Кроме того, вы можете использовать вызов [SHOW META](../Node_info_and_management/SHOW_META.md) для просмотра дополнительной мета-информации о последнем запросе.

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

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SELECT id,f1,f2 FROM t WHERE f2=2 LIMIT 1; SHOW META"
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

<!-- end -->

<!-- example sql3 -->
В некоторых случаях, например при выполнении [фасетного поиска](../Searching/Faceted_search.md), вы можете получить несколько наборов результатов в ответ на ваш SQL-запрос.

<!--
data for the following examples:

DROP TABLE IF EXISTS tbl;
CREATE TABLE tbl(id bigint, age int, name string);
INSERT INTO tbl (id, age, name) VALUES
(1, 25, 'joe'),
(2, 25, 'mary'),
(3, 33, 'albert');
-->

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

<!-- request JSON -->
```JSON
POST /sql -d "SELECT * FROM tbl WHERE MATCH('b') FACET f2"
```

<!-- response JSON -->
```JSON
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "total_relation": "eq",
    "hits": [
      {
        "_score": 2500,
        "_source": {
          "f1": "b"
        }
      },
      {
        "_score": 2500,
        "_source": {
          "f1": "b"
        }
      }
    ]
  },
  "aggregations": {
    "rating": {
      "buckets": [
        {
          "key": 2,
          "doc_count": 2
        }
      ]
    }
  }
}
```

<!-- end -->

<!-- example sql4 -->
В случае предупреждения в наборе результатов будет включён флаг предупреждения, а вы можете увидеть предупреждение с помощью [SHOW WARNINGS](../Node_info_and_management/SHOW_WARNINGS.md).
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

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "SELECT * from t where match('\"a\"/3'); show warnings"
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

<!-- end -->

<!--
data for the following example:

DROP TABLE IF EXISTS t;
CREATE TABLE t(name text);
INSERT INTO t(name) VALUES
('joe');
-->

<!-- example sql5 -->
Если ваш запрос не выполнится, вы получите ошибку:

<!-- request SQL -->
```sql
SELECT * from tbl where match('@surname joe');
```

<!-- response SQL -->
```sql
ERROR 1064 (42000): index idx: query error: no field 'surname' found in schema
```

<!-- request JSON -->
```JSON
POST /sql -d "SELECT * from t where match('@surname joe')"
```

<!-- response JSON -->
```JSON
{
  "error": "table t: query error: no field 'surname' found in schema"
}
```

<!-- end -->


## HTTP

Через HTTP JSON интерфейс результат запроса отправляется в виде JSON-документа. Пример:

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
* `timed_out`: истёкло ли время выполнения запроса
* `hits`: результаты поиска, со следующими свойствами:
  - `total`: общее количество совпадающих документов
  - `hits`: массив, содержащий совпадения

Результат запроса также может включать информацию о профилировании запроса. Смотрите [Профиль запроса](../Node_info_and_management/Profiling/Query_profile.md).

Каждое совпадение в массиве `hits` имеет следующие свойства:

* `_id`: id совпадения
* `_score`: вес совпадения, вычисленный ранкером
* `_source`: массив, содержащий атрибуты этого совпадения

### Выбор исходных данных

По умолчанию в массиве `_source` возвращаются все атрибуты. Вы можете использовать свойство `_source` в теле запроса, чтобы выбрать поля, которые хотите включить в набор результатов. Пример:

```json
{
  "table":"test",
  "_source":"attr*",
  "query": { "match_all": {} }
}
```

Вы можете указать атрибуты, которые хотите включить в результат запроса, в виде строки (`"_source": "attr*"`) или массива строк (`"_source": [ "attr1", "attri*" ]"`). Каждое значение может быть именем атрибута или шаблоном (поддерживаются символы `*`, `%` и `?`).

Вы также можете явно указать, какие атрибуты хотите включить, а какие исключить из набора результатов, используя свойства `includes` и `excludes`:

```json
"_source":
{
  "includes": [ "attr1", "attri*" ],
  "excludes": [ "*desc*" ]
}
```

Пустой список includes интерпретируется как «включить все атрибуты», в то время как пустой список excludes не соответствует ничему. Если атрибут совпадает и в includes, и в excludes, приоритет у excludes.

<!-- proofread -->




