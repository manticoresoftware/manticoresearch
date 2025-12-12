# SHOW META

```sql
SHOW META [ LIKE pattern ]
```

<!-- example show meta -->
`SHOW META` — это SQL-оператор, который отображает дополнительную мета-информацию о выполненном запросе, включая время выполнения запроса, статистику по ключевым словам и информацию об используемых вторичных индексах. Синтаксис следующий:

Включаемые элементы:
* `total`: Количество совпадений, которые фактически были получены и отправлены клиенту. Это значение обычно ограничивается параметром поиска [LIMIT/size](../Searching/Pagination.md#Pagination-of-search-results).
* `total_found`:
  - Примерное общее количество совпадений для запроса в индексе. Если необходимо получить точное количество совпадений, используйте `SELECT COUNT(*)` вместо опоры на это значение.
  - Для запросов с `GROUP BY` `total_found` представляет число групп, а не отдельных совпадений.
  - При использовании `HAVING` с `GROUP BY` `total_found` отражает число групп **после** применения фильтра `HAVING`. Это обеспечивает корректную постраничную навигацию с условиями `HAVING`.
  - Для запросов типа [GROUP N BY](../Searching/Grouping.md#Give-me-N-rows) `total_found` по-прежнему означает число групп, независимо от значения `N`.
* `total_relation`: Указывает, является ли значение `total_found` точным или приблизительным.
  - Если Manticore не может определить точное значение `total_found`, в этом поле отображается `total_relation: gte` — то есть фактическое количество совпадений **больше или равно** указанному `total_found`.
  - Если значение `total_found` точно, отображается `total_relation: eq`.
* `time`: Время обработки поискового запроса (в секундах).
* `keyword[N]`: N-ое ключевое слово, использованное в поисковом запросе. Обратите внимание, что ключевое слово может быть задано с использованием подстановочного знака, например, `abc*`.
* `docs[N]`: Общее количество документов (или записей), содержащих N-ое ключевое слово из поискового запроса. Если ключевое слово задано с подстановочным знаком, это значение представляет сумму документов по всем расширенным под-ключевым словам, и может превышать реальное количество совпадающих документов.
* `hits[N]`: Общее количество вхождений (попаданий) N-го ключевого слова по всем документам.
* `index`: Информация об используемом индексе (например, вторичном).

<!--
data for the following examples:

DROP TABLE IF EXISTS hn_small;
CREATE TABLE hn_small(story_author text, comment_ranking int);
INSERT INTO hn_small(story_author, comment_ranking) VALUES
('anewkid', 4),
('sasjri', 1),
('bks', 5);
--> 

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SELECT id, story_author FROM hn_small WHERE MATCH('one|two|three') and comment_ranking > 2 limit 5;
show meta;
```

<!-- response SQL -->

```sql
+---------+--------------+
| id      | story_author |
+---------+--------------+
|  151171 | anewkid      |
|  302758 | bks          |
|  805806 | drRoflol     |
| 1099245 | tnorthcutt   |
|  303252 | whiten       |
+---------+--------------+
5 rows in set (0.00 sec)

+----------------+---------------------------------------+
| Variable_name  | Value                                 |
+----------------+---------------------------------------+
| total          | 5                                     |
| total_found    | 2308                                  |
| total_relation | eq                                    |
| time           | 0.001                                 |
| keyword[0]     | one                                   |
| docs[0]        | 224387                                |
| hits[0]        | 310327                                |
| keyword[1]     | three                                 |
| docs[1]        | 18181                                 |
| hits[1]        | 21102                                 |
| keyword[2]     | two                                   |
| docs[2]        | 63251                                 |
| hits[2]        | 75961                                 |
| index          | comment_ranking:SecondaryIndex (100%) |
+----------------+---------------------------------------+
14 rows in set (0.00 sec)
```

<!-- intro -->
##### JSON:
<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "SELECT id, story_author FROM hn_small WHERE MATCH('one|two|three') and comment_ranking > 2 limit 5; SHOW META"
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
        "story_author": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "id": 151171,
        "story_author": "anewkid"
      },
      {
        "id": 302758,
        "story_author": "bks"
      },
      {
        "id": 805806,
        "story_author": "drRoflol"
      },
      {
        "id": 1099245,
        "story_author": "tnorthcutt"
      },
      {
        "id": 303252,
        "story_author": "whiten"
      }
    ],
    "total": 5,
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
        "Value": "5"
      },
      {
        "Variable_name": "total_found",
        "Value": "2308"
      },
      {
        "Variable_name": "total_relation",
        "Value": "eq"
      },
      {
        "Variable_name": "time",
        "Value": "0.001"
      },
      {
        "Variable_name": "keyword[0]",
        "Value": "one"
      },
      {
        "Variable_name": "docs[0]",
        "Value": "224387"
      },
      {
        "Variable_name": "hits[0]",
        "Value": "310327"
      },
      {
        "Variable_name": "keyword[1]",
        "Value": "three"
      },
      {
        "Variable_name": "docs[1]",
        "Value": "18181"
      },
      {
        "Variable_name": "hits[1]",
        "Value": "21102"
      },
      {
        "Variable_name": "keyword[2]",
        "Value": "two"
      },
      {
        "Variable_name": "docs[2]",
        "Value": "63251"
      },
      {
        "Variable_name": "hits[2]",
        "Value": "75961"
      },
      {
        "Variable_name": "index",
        "Value": "comment_ranking:SecondaryIndex (100%)"
      }
    ],
    "total": 14,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

<!--
data for the following example:

DROP TABLE IF EXISTS records;
CREATE TABLE records(content text, channel_id int);
INSERT INTO records(story_author, channel_id) VALUES
('record one', 4),
('record two', 1),
('record three', 3),
('record four', 4),
('record twenty one', 2),
('record twenty two', 2),
-->

<!-- example show meta iostats cpustats -->
`SHOW META` может отображать счетчики ввода-вывода и ЦП, но они будут доступны только если searchd был запущен с переключателями `--iostats` и `--cpustats` соответственно.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SELECT id,channel_id FROM records WHERE MATCH('one|two|three') limit 5;

SHOW META;
```

<!-- response SQL -->

```sql
+--------+--------------+
| id     | story_author |
+--------+--------------+
| 300263 | throwaway37  |
| 713503 | mahmud       |
| 716804 | mahmud       |
| 776906 | jimbokun     |
| 753332 | foxhop       |
+--------+--------------+
5 rows in set (0.01 sec)

+-----------------------+--------+
| Variable_name         | Value  |
+-----------------------+--------+
| total                 | 5      |
| total_found           | 266385 |
| total_relation        | eq     |
| time                  | 0.011  |
| cpu_time              | 18.004 |
| agents_cpu_time       | 0.000  |
| io_read_time          | 0.000  |
| io_read_ops           | 0      |
| io_read_kbytes        | 0.0    |
| io_write_time         | 0.000  |
| io_write_ops          | 0      |
| io_write_kbytes       | 0.0    |
| agent_io_read_time    | 0.000  |
| agent_io_read_ops     | 0      |
| agent_io_read_kbytes  | 0.0    |
| agent_io_write_time   | 0.000  |
| agent_io_write_ops    | 0      |
| agent_io_write_kbytes | 0.0    |
| keyword[0]            | one    |
| docs[0]               | 224387 |
| hits[0]               | 310327 |
| keyword[1]            | three  |
| docs[1]               | 18181  |
| hits[1]               | 21102  |
| keyword[2]            | two    |
| docs[2]               | 63251  |
| hits[2]               | 75961  |
+-----------------------+--------+
27 rows in set (0.00 sec)
```

<!-- intro -->
##### JSON:
<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "SELECT id,channel_id FROM records WHERE MATCH('one|two|three') limit 5; SHOW META"
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
        "story_author": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "id": 300263,
        "story_author": "throwaway37"
      },
      {
        "id": 713503,
        "story_author": "mahmud"
      },
      {
        "id": 716804,
        "story_author": "mahmud"
      },
      {
        "id": 776906,
        "story_author": "jimbokun"
      },
      {
        "id": 753332,
        "story_author": "foxhop"
      }
    ],
    "total": 5,
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
        "Value": "5"
      },
      {
        "Variable_name": "total_found",
        "Value": "266385"
      },
      {
        "Variable_name": "total_relation",
        "Value": "eq"
      },
      {
        "Variable_name": "time",
        "Value": "0.001"
      },
      {
        "Variable_name": "cpu_time",
        "Value": "18.004"
      },
      {
        "Variable_name": "agents_cpu_time",
        "Value": "0.000"
      },
      {
        "Variable_name": "io_read_time",
        "Value": "0.000"
      },
      {
        "Variable_name": "io_read_ops",
        "Value": "0"
      },
      {
        "Variable_name": "io_read_kbytes",
        "Value": "0.0"
      },
      {
        "Variable_name": "io_write_time",
        "Value": "0.000"
      },
      {
        "Variable_name": "io_write_ops",
        "Value": "0"
      },
      {
        "Variable_name": "io_write_kbytes",
        "Value": "0.0"
      },
      {
        "Variable_name": "agent_io_read_time",
        "Value": "0.000"
      },
      {
        "Variable_name": "agent_io_read_ops",
        "Value": "0"
      },
      {
        "Variable_name": "agent_io_read_kbytes",
        "Value": "0.0"
      },
      {
        "Variable_name": "agent_io_write_time",
        "Value": "0.000"
      },
      {
        "Variable_name": "agent_io_write_ops",
        "Value": "0"
      },
      {
        "Variable_name": "agent_io_write_kbytes",
        "Value": "0.0"
      },
      {
        "Variable_name": "keyword[0]",
        "Value": "one"
      },
      {
        "Variable_name": "docs[0]",
        "Value": "224387"
      },
      {
        "Variable_name": "hits[0]",
        "Value": "310327"
      },
      {
        "Variable_name": "keyword[1]",
        "Value": "three"
      },
      {
        "Variable_name": "docs[1]",
        "Value": "18181"
      },
      {
        "Variable_name": "hits[1]",
        "Value": "21102"
      },
      {
        "Variable_name": "keyword[2]",
        "Value": "two"
      },
      {
        "Variable_name": "docs[2]",
        "Value": "63251"
      },
      {
        "Variable_name": "hits[2]",
        "Value": "75961"
      }
    ],
    "total": 27,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

<!--
data for the following examples:

DROP TABLE IF EXISTS hn_small;
CREATE TABLE hn_small(story_author text, comment_ranking int);
INSERT INTO hn_small(story_author, comment_ranking) VALUES
('anewkid', 4),
('sasjri', 1),
('bks', 5);
-->

<!-- example show meta predicted_time -->
Дополнительные значения, такие как `predicted_time`, `dist_predicted_time`, `local_fetched_docs`, `local_fetched_hits`, `local_fetched_skips` и их соответствующие `dist_fetched_*` аналоги, будут доступны только если `searchd` был настроен с [прогнозированными временем и затратами](../Server_settings/Searchd.md#predicted_time_costs) и в запросе был включен параметр `predicted_time` в разделе `OPTION`.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SELECT id,story_author FROM hn_small WHERE MATCH('one|two|three') limit 5 option max_predicted_time=100;

SHOW META;
```

<!-- response SQL -->

```sql
+--------+--------------+
| id     | story_author |
+--------+--------------+
| 300263 | throwaway37  |
| 713503 | mahmud       |
| 716804 | mahmud       |
| 776906 | jimbokun     |
| 753332 | foxhop       |
+--------+--------------+
5 rows in set (0.01 sec)

mysql> show meta;
+---------------------+--------+
| Variable_name       | Value  |
+---------------------+--------+
| total               | 5      |
| total_found         | 266385 |
| total_relation      | eq     |
| time                | 0.012  |
| local_fetched_docs  | 307212 |
| local_fetched_hits  | 407390 |
| local_fetched_skips | 24     |
| predicted_time      | 56     |
| keyword[0]          | one    |
| docs[0]             | 224387 |
| hits[0]             | 310327 |
| keyword[1]          | three  |
| docs[1]             | 18181  |
| hits[1]             | 21102  |
| keyword[2]          | two    |
| docs[2]             | 63251  |
| hits[2]             | 75961  |
+---------------------+--------+
17 rows in set (0.00 sec)
```

<!-- intro -->
##### JSON:
<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "SELECT id,story_author FROM hn_small WHERE MATCH('one|two|three') limit 5 option max_predicted_time=100; SHOW META"
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
        "story_author": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "id": 300263,
        "story_author": "throwaway37"
      },
      {
        "id": 713503,
        "story_author": "mahmud"
      },
      {
        "id": 716804,
        "story_author": "mahmud"
      },
      {
        "id": 776906,
        "story_author": "jimbokun"
      },
      {
        "id": 753332,
        "story_author": "foxhop"
      }
    ],
    "total": 5,
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
        "Value": "5"
      },
      {
        "Variable_name": "total_found",
        "Value": "266385"
      },
      {
        "Variable_name": "total_relation",
        "Value": "eq"
      },
      {
        "Variable_name": "time",
        "Value": "0.012"
      },
      {
        "Variable_name": "local_fetched_docs",
        "Value": "307212"
      },
      {
        "Variable_name": "local_fetched_hits",
        "Value": "407390"
      },
      {
        "Variable_name": "local_fetched_skips",
        "Value": "24"
      },
      {
        "Variable_name": "predicted_time",
        "Value": "56"
      },
      {
        "Variable_name": "keyword[0]",
        "Value": "one"
      },
      {
        "Variable_name": "docs[0]",
        "Value": "224387"
      },
      {
        "Variable_name": "hits[0]",
        "Value": "310327"
      },
      {
        "Variable_name": "keyword[1]",
        "Value": "three"
      },
      {
        "Variable_name": "docs[1]",
        "Value": "18181"
      },
      {
        "Variable_name": "hits[1]",
        "Value": "21102"
      },
      {
        "Variable_name": "keyword[2]",
        "Value": "two"
      },
      {
        "Variable_name": "docs[2]",
        "Value": "63251"
      },
      {
        "Variable_name": "hits[2]",
        "Value": "75961"
      }
    ],
    "total": 17,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

<!--
data for the following examples:

DROP TABLE IF EXISTS hn_small;
CREATE TABLE hn_small(story_author text, comment_ranking int);
INSERT INTO hn_small(story_author, comment_ranking) VALUES
('anewkid', 4),
('sasjri', 1),
('bks', 5);
-->

<!-- example show meta single statement -->

`SHOW META` должен выполняться немедленно после запроса в **той же** сессии. Поскольку некоторые коннекторы/библиотеки MySQL используют пулы соединений, выполнение `SHOW META` в отдельном операторе может привести к неожиданным результатам, например, получению метаданных по другому запросу. В таких случаях (и это рекомендуется в целом) выполняйте составной запрос, включающий и сам запрос, и `SHOW META`. Некоторые коннекторы/библиотеки поддерживают мультизапросы в одном методе с единым оператором, тогда как другие могут требовать специального метода для мультизапросов или установки определенных опций при настройке соединения.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SELECT id,story_author FROM hn_small WHERE MATCH('one|two|three') LIMIT 5; SHOW META;
```

<!-- response SQL -->

```sql
+--------+--------------+
| id     | story_author |
+--------+--------------+
| 300263 | throwaway37  |
| 713503 | mahmud       |
| 716804 | mahmud       |
| 776906 | jimbokun     |
| 753332 | foxhop       |
+--------+--------------+
5 rows in set (0.01 sec)

+----------------+--------+
| Variable_name  | Value  |
+----------------+--------+
| total          | 5      |
| total_found    | 266385 |
| total_relation | eq     |
| time           | 0.011  |
| keyword[0]     | one    |
| docs[0]        | 224387 |
| hits[0]        | 310327 |
| keyword[1]     | three  |
| docs[1]        | 18181  |
| hits[1]        | 21102  |
| keyword[2]     | two    |
| docs[2]        | 63251  |
| hits[2]        | 75961  |
+----------------+--------+
13 rows in set (0.00 sec)
```

<!-- intro -->
##### JSON:
<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "SELECT id,story_author FROM hn_small WHERE MATCH('one|two|three') LIMIT 5; SHOW META"
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
        "story_author": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "id": 300263,
        "story_author": "throwaway37"
      },
      {
        "id": 713503,
        "story_author": "mahmud"
      },
      {
        "id": 716804,
        "story_author": "mahmud"
      },
      {
        "id": 776906,
        "story_author": "jimbokun"
      },
      {
        "id": 753332,
        "story_author": "foxhop"
      }
    ],
    "total": 5,
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
        "Value": "5"
      },
      {
        "Variable_name": "total_found",
        "Value": "266385"
      },
      {
        "Variable_name": "total_relation",
        "Value": "eq"
      },
      {
        "Variable_name": "time",
        "Value": "0.011"
      },
      {
        "Variable_name": "keyword[0]",
        "Value": "one"
      },
      {
        "Variable_name": "docs[0]",
        "Value": "224387"
      },
      {
        "Variable_name": "hits[0]",
        "Value": "310327"
      },
      {
        "Variable_name": "keyword[1]",
        "Value": "three"
      },
      {
        "Variable_name": "docs[1]",
        "Value": "18181"
      },
      {
        "Variable_name": "hits[1]",
        "Value": "21102"
      },
      {
        "Variable_name": "keyword[2]",
        "Value": "two"
      },
      {
        "Variable_name": "docs[2]",
        "Value": "63251"
      },
      {
        "Variable_name": "hits[2]",
        "Value": "75961"
      }
    ],
    "total": 13,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

<!-- example SHOW META LIKE -->

Также можно использовать опциональный оператор LIKE, который позволяет выбирать только переменные, соответствующие заданному шаблону. Синтаксис шаблона следует стандартным SQL-подстановкам, где `%` означает любое количество любых символов, а `_` — любой одиночный символ.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SHOW META LIKE 'total%';
```

<!-- response SQL -->

```sql
+----------------+--------+
| Variable_name  | Value  |
+----------------+--------+
| total          | 5      |
| total_found    | 266385 |
| total_relation | eq     |
+----------------+--------+
3 rows in set (0.00 sec)
```

<!-- intro -->
##### JSON:
<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "SHOW META LIKE 'total%'"
```

<!-- response JSON -->

```JSON
[
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
        "Value": "5"
      },
      {
        "Variable_name": "total_found",
        "Value": "266385"
      },
      {
        "Variable_name": "total_relation",
        "Value": "eq"
      }
    ],
    "total": 3,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

## SHOW META и фасеты


<!--
data for the following example:

DROP TABLE IF EXISTS facetdemo;
CREATE TABLE facetdemo(title text, brand_name string, brand_id int, categories multi, price float, j json, property string);
INSERT INTO facetdemo(title, brand_name, brand_id, categories, price, j, property) VALUES
('Product Ten Three', 'Brand One', 1, (1,2), 100,  '{"prop1":66,"prop2":91,"prop3":"One"}', 'Six_Ten'),
('Product Ten Four', 'Brand One', 1, (2,3), 100,  '{"prop1":67,"prop2":92,"prop3":"Two"}', 'Six_Ten'),
('Product Ten Nine', 'Brand Two', 2), (1,2), 120,  '{"prop1":67,"prop2":93,"prop3":"Nine"}', 'Six_Nine'),
('Product Ten Ten', 'Brand Two', 2), (1,2), 150,  '{"prop1":66,"prop2":94,"prop3":"Ten"}', 'Six_Nine');
--> 

<!-- example show meta facets -->

При использовании [фасетного поиска](../Searching/Faceted_search.md) вы можете проверить поле `multiplier` в выводе команды `SHOW META`, чтобы определить, сколько запросов было выполнено в оптимизированной группе.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SELECT * FROM facetdemo FACET brand_id FACET price FACET categories;
SHOW META LIKE 'multiplier';
```

<!-- response SQL -->

```sql
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
| id   | price | brand_id | title               | brand_name  | property    | j                                     | categories |
+------+-------+----------+---------------------+-------------+-------------+---------------------------------------+------------+
|    1 |   306 |        1 | Product Ten Three   | Brand One   | Six_Ten     | {"prop1":66,"prop2":91,"prop3":"One"} | 10,11      |
...

+----------+----------+
| brand_id | count(*) |
+----------+----------+
|        1 |     1013 |
...

+-------+----------+
| price | count(*) |
+-------+----------+
|   306 |        7 |
...

+------------+----------+
| categories | count(*) |
+------------+----------+
|         10 |     2436 |
...

+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| multiplier    | 4     |
+---------------+-------+
1 row in set (0.00 sec)
```

<!-- intro -->
##### JSON:
<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "SELECT brand_name FROM facetdemo FACET brand_id FACET price FACET categories; SHOW META LIKE 'multiplier'"
```

<!-- response JSON -->

```JSON
[
  {
    "columns": [
      {
        "brand_name": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "brand_name": "Brand One"
      }
      ...
    ],
    "total": 1013,
    "error": "",
    "warning": ""
  },
  {
    "columns": [
      {
        "brand_id": {
          "type": "long long"
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
        "brand_id": 1,
        "count(*)": 1013
      }
      ...
    ],
    "total": 1013,
    "error": "",
    "warning": ""
  },
  {
    "columns": [
      {
        "price": {
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
        "price": 1,
        "count(*)": 7
      }
      ...
    ],
    "total": 658,
    "error": "",
    "warning": ""
  },
  {
    "columns": [
      {
        "categories": {
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
        "categories": 10,
        "count(*)": 2436
      }
      ...
    ],
    "total": 15,
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
        "Variable_name": "multiplier",
        "Value": "4"
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

## SHOW META и оптимизатор запросов

<!-- example of show meta vs query optimizer -->

Когда [оптимизатор запросов на основе стоимости](../Searching/Cost_based_optimizer.md) выбирает использование `DocidIndex`, `ColumnarScan` или `SecondaryIndex` вместо простого фильтра, это отражается в команде `SHOW META`.

Переменная `index` показывает имена и типы вторичных индексов, использованных во время выполнения запроса. Процент указывает, сколько дисковых чанков (в случае RT-таблицы) или псевдо-шардов (в случае простой таблицы) использовали вторичный индекс.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
SELECT count(*) FROM taxi1 WHERE tip_amount = 5;
SHOW META;
```

<!-- response SQL -->

```sql
+----------------+----------------------------------+
| Variable_name  | Value                            |
+----------------+----------------------------------+
| total          | 1                                |
| total_found    | 1                                |
| total_relation | eq                               |
| time           | 0.016                            |
| index          | tip_amount:SecondaryIndex (100%) |
+----------------+----------------------------------+
5 rows in set (0.00 sec)
```

<!-- intro -->
##### JSON:
<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "SELECT count(*) FROM taxi1 WHERE tip_amount = 5; SHOW META"
```

<!-- response JSON -->

```JSON
[
  {
    "columns": [
      {
        "count(*)": {
          "type": "long long"
        }
      }
    ],
    "data": [
      {
        "count(*)": 1
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
        "Value": "eq"
      },
      {
        "Variable_name": "time",
        "Value": "0.016"
      },
      {
        "Variable_name": "index",
        "Value": "tip_amount:SecondaryIndex (100%)"
      }
    ],
    "total": 5,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

## SHOW META для PQ таблиц

<!-- example show meta PQ -->

Команда `SHOW META` может использоваться после выполнения оператора [CALL PQ](../Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ), в этом случае она предоставляет другой вывод.

`SHOW META` после оператора `CALL PQ` включает:

* `total` — Общее время, затраченное на сопоставление документа(ов)
* `queries_matched` — Количество сохранённых запросов, которые соответствуют документу(ам)
* `document_matches` — Количество документов, которые совпали с сохранёнными в таблице запросами
* `total_queries_stored` — Общее количество запросов, сохранённых в таблице
* `term_only_queries` — Количество запросов в таблице, которые содержат только термины; остальные запросы используют расширенный синтаксис запросов.

<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CALL PQ ('pq', ('{"title":"angry", "gid":3 }')); SHOW META;
```

<!-- response SQL -->

```sql
+------+
| id   |
+------+
|    2 |
+------+
1 row in set (0.00 sec)

+-----------------------+-----------+
| Variable name         | Value     |
+-----------------------+-----------+
| total                 | 0.000 sec |
| queries_matched       | 1         |
| queries_failed        | 0         |
| document_matched      | 1         |
| total_queries_stored  | 2         |
| term_only_queries     | 2         |
| fast_rejected_queries | 1         |
+-----------------------+-----------+
7 rows in set (0.00 sec)
```

<!-- end -->

<!-- example call pq verbose meta  -->

Использование `CALL PQ` с опцией `verbose` предоставляет более детальный вывод.

Он включает следующие дополнительные записи:

* `Setup` — Время, затраченное на начальную настройку процесса сопоставления, например парсинг документов и установку опций
* `Queries failed` — Количество запросов, которые завершились с ошибкой
* `Fast rejected queries` — Количество запросов, не полностью оценённых, но быстро сопоставленных и отклонённых с помощью фильтров или других условий
* `Time per query` — Подробное время для каждого запроса
* `Time of matched queries` — Общее время, затраченное на запросы, которые соответствовали каким-либо документам


<!-- intro -->
##### SQL:
<!-- request SQL -->

```sql
CALL PQ ('pq', ('{"title":"angry", "gid":3 }'), 1 as verbose); SHOW META;
```

<!-- response SQL -->

```sql
+------+
| id   |
+------+
|    2 |
+------+
1 row in set (0.00 sec)

+-------------------------+-----------+
| Variable name           | Value     |
+-------------------------+-----------+
| total                   | 0.000 sec |
| setup                   | 0.000 sec |
| queries_matched         | 1         |
| queries_failed          | 0         |
| document_matched        | 1         |
| total_queries_stored    | 2         |
| term_only_queries       | 2         |
| fast_rejected_queries   | 1         |
| time_per_query          | 69        |
| time_of_matched_queries | 69        |
+-------------------------+-----------+
10 rows in set (0.00 sec)
```

<!-- end -->

<!-- proofread -->

