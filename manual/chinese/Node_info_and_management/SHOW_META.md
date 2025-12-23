# SHOW META

```sql
SHOW META [ LIKE pattern ]
```

<!-- example show meta -->
`SHOW META` 是一个 SQL 语句，用于显示有关已处理查询的附加元信息，包括查询时间、关键字统计以及使用的二级索引信息。语法如下：

包含的项目有：
* `total`：实际检索并发送给客户端的匹配数。此值通常受 [LIMIT/size](../Searching/Pagination.md#Pagination-of-search-results) 搜索选项限制。
* `total_found`：
  - 索引中查询匹配的估计总数。如果需要精确的匹配数，请使用 `SELECT COUNT(*)`，不要依赖此值。
  - 对于含 `GROUP BY` 的查询，`total_found` 表示分组数而非单个匹配数。
  - 当与 `GROUP BY` 一起使用 `HAVING` 时，`total_found` 反映的是应用 `HAVING` 过滤后组的数量。这确保了包含 `HAVING` 子句时的正确分页。
  - 对于 [GROUP N BY](../Searching/Grouping.md#Give-me-N-rows) 查询，`total_found` 仍代表组数，无论 `N` 的值是多少。
* `total_relation`：指示 `total_found` 值是精确还是估计。
  - 如果 Manticore 无法确定精确的 `total_found` 值，此字段会显示 `total_relation: gte`，意味着实际匹配数**大于或等于**报告的 `total_found`。
  - 如果 `total_found` 值是精确的，则显示 `total_relation: eq`。
* `time`：处理搜索查询所耗费的时间（秒）。
* `keyword[N]`：搜索查询中第 n 个关键字。关键字可能以通配符形式呈现，如 `abc*`。
* `docs[N]`：包含搜索查询中第 n 个关键字的文档（或记录）总数。如果关键字是通配符形式，此值表示所有扩展子关键字文档数的总和，可能超过实际匹配文档数。
* `hits[N]`：第 n 个关键字在所有文档中的总出现次数（命中数）。
* `index`：有关所用索引的信息（例如二级索引）。

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
`SHOW META` 可以显示 I/O 和 CPU 计数器，但仅当 searchd 分别使用 `--iostats` 和 `--cpustats` 参数启动时才可用。

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
额外的值，如 `predicted_time`、`dist_predicted_time`、`local_fetched_docs`、`local_fetched_hits`、`local_fetched_skips` 及其各自的 `dist_fetched_*` 对应值，只有当 `searchd` 配置了[预测时间成本](../Server_settings/Searchd.md#predicted_time_costs)且查询在 `OPTION` 子句中包含 `predicted_time` 时才可用。

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

`SHOW META` 必须在**同一**会话中紧接查询后执行。由于某些 MySQL 连接器/库使用连接池，单独执行 `SHOW META` 可能会导致意外结果，比如获取到其他查询的元数据。在这些情况下（且一般建议做法），运行一个包含查询和 `SHOW META` 的多语句。有些连接器/库支持单个方法中多查询语句，而其他可能需要专门的多查询方法或在连接设置时指定特定选项。

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

您也可以使用可选的 LIKE 子句，选择仅匹配特定模式的变量。模式语法遵循标准 SQL 通配符，其中 `%` 表示任意数量的任意字符，`_` 表示单个字符。

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

## SHOW META 和 facets


<!--
data for the following example:

DROP TABLE IF EXISTS facetdemo;
CREATE TABLE facetdemo(title text, brand_name string, brand_id int, categories multi, price float, j json, property string);
INSERT INTO facetdemo(title, brand_name, brand_id, categories, price, j, property) VALUES
('产品十 三', '品牌一', 1, (1,2), 100,  '{"prop1":66,"prop2":91,"prop3":"One"}', 'Six_Ten'),
('产品十 四', '品牌一', 1, (2,3), 100,  '{"prop1":67,"prop2":92,"prop3":"Two"}', 'Six_Ten'),
('产品十 九', '品牌二', 2), (1,2), 120,  '{"prop1":67,"prop2":93,"prop3":"Nine"}', 'Six_Nine'),
('产品十 十', '品牌二', 2), (1,2), 150,  '{"prop1":66,"prop2":94,"prop3":"Ten"}', 'Six_Nine');
--> 

<!-- example show meta facets -->

使用[分面搜索](../Searching/Faceted_search.md)时，您可以查看 `SHOW META` 输出中的 `multiplier` 字段，以确定优化组中执行了多少查询。

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

## SHOW META 和查询优化器

<!-- example of show meta vs query optimizer -->

当[基于成本的查询优化器](../Searching/Cost_based_optimizer.md)选择使用 `DocidIndex`、`ColumnarScan` 或 `SecondaryIndex` 代替普通过滤器时，这将在 `SHOW META` 命令中体现。

`index` 变量显示的是查询执行过程中使用的二级索引名称和类型。百分比表示使用二级索引的磁盘块数（对于 RT 表）或伪分片数（对于普通表）的比例。

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

## PQ 表的 SHOW META

<!-- example show meta PQ -->

执行[调用 PQ](../Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ)语句后，可以使用 `SHOW META`，此时它提供不同的输出。

紧跟 `CALL PQ` 语句的 `SHOW META` 包含：

* `total` - 匹配文档所花费的总时间
* `queries_matched` - 匹配文档的存储查询数
* `document_matches` - 匹配表中存储查询的文档数
* `total_queries_stored` - 表中存储的查询总数
* `term_only_queries` - 表中仅含术语的查询数；其余查询使用扩展查询语法。

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

使用带有 `verbose` 选项的 `CALL PQ` 会提供更详细的输出。

它包含以下附加条目：

* `Setup` - 匹配过程初始设置所花费的时间，如解析文档和设置选项
* `Queries failed` - 失败的查询数
* `Fast rejected queries` - 未完全评估但通过过滤器或其他条件快速匹配并拒绝的查询数
* `Time per query` - 每个查询的详细时间
* `Time of matched queries` - 匹配任何文档的查询所花费的总时间


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

