# 搜索结果

## SQL

<!--
以下示例的数据：

DROP TABLE IF EXISTS tbl;
CREATE TABLE tbl(id bigint, age int, name string);
INSERT INTO tbl (id, age, name) VALUES
(1, 25, 'joe'),
(2, 25, 'mary'),
(3, 33, 'albert');
-->

<!-- example sql1 -->
当你通过 MySQL 协议运行 SQL 查询时，你会收到请求的列作为结果，如果没有找到任何内容，则返回空结果集。

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
        "age": 25,
        "name": "joe"
      },
      {
        "id": 2,
        "age": 25,
        "name": "mary"
      },
      {
        "id": 3,
        "age": 33,
        "name": "albert"
      }
    ],
    "total": 3,
    "error": "",
    "warning": ""
  }
]

```

<!-- end -->

<!--
以下示例的数据：

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
此外，你可以使用 [SHOW META](../Node_info_and_management/SHOW_META.md) 命令查看最新查询的额外元信息。

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

<!-- end -->

<!-- example sql3 -->
在某些情况下，比如执行[分面搜索](../Searching/Faceted_search.md)时，你可能会收到多个结果集作为 SQL 查询的响应。

<!--
以下示例的数据：

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

<!-- end -->

<!-- example sql4 -->
如果有警告，结果集将包含一个警告标记，你可以使用 [SHOW WARNINGS](../Node_info_and_management/SHOW_WARNINGS.md) 查看警告。
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

<!-- end -->

<!--
以下示例的数据：

DROP TABLE IF EXISTS t;
CREATE TABLE t(name text);
INSERT INTO t(name) VALUES
('joe');
-->

<!-- example sql5 -->
如果你的查询失败，你将收到一个错误：

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
POST /sql?mode=raw -d "SELECT * from t where match('@surname joe');"
```

<!-- response JSON -->
```JSON
{
  "error": "table t: query error: no field 'surname' found in schema"
}
```

<!-- end -->


## HTTP

通过 HTTP JSON 接口，查询结果以 JSON 文档的形式发送。示例：

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

* `took`：执行搜索所花费的时间（毫秒）
* `timed_out`：查询是否超时
* `hits`：搜索结果，具有以下属性：
  - `total`：匹配的文档总数
  - `hits`：包含匹配项的数组

查询结果还可以包含查询分析信息。详见 [查询分析](../Node_info_and_management/Profiling/Query_profile.md)。

`hits` 数组中的每个匹配项具有以下属性：

* `_id`：匹配项 ID
* `_score`：匹配权重，由排序器计算
* `_source`：包含该匹配项属性的数组

### 源选择

默认情况下，所有属性都包含在 `_source` 数组中。你可以在请求负载中使用 `_source` 属性来选择你希望包含在结果集中的字段。示例：

```json
{
  "table":"test",
  "_source":"attr*",
  "query": { "match_all": {} }
}
```

你可以指定想要包含在查询结果中的属性，格式可以是字符串（`"_source": "attr*"`）或者字符串数组（`"_source": [ "attr1", "attri*" ]`）。每个条目可以是属性名称或通配符（支持 `*`、`%` 和 `?` 符号）。

你也可以使用 `includes` 和 `excludes` 属性显式指定想要包含或排除的属性：

```json
"_source":
{
  "includes": [ "attr1", "attri*" ],
  "excludes": [ "*desc*" ]
}
```

空的 includes 列表被解释为“包含所有属性”，而空的 excludes 列表不会匹配任何内容。如果某个属性同时匹配 includes 和 excludes，则 excludes 优先。

<!-- proofread -->




