# 搜索结果

## SQL

<!-- example sql1 -->
当您通过 MySQL 协议运行 SQL 查询时，您将收到请求的列作为结果，如果未找到任何内容，则返回一个空结果集。

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
此外，您可以使用 [SHOW META](../Node_info_and_management/SHOW_META.md) 调用来查看有关最新查询的额外元信息。

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
在某些情况下，例如执行 [分面搜索](../Searching/Faceted_search.md) 时，您可能会收到多个结果集作为对您的 SQL 查询的响应。

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
如果有警告，结果集将包含警告标志，您可以使用 [SHOW WARNINGS](../Node_info_and_management/SHOW_WARNINGS.md) 查看警告。
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

<!-- example sql5 -->
如果您的查询失败，您将收到一个错误：

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

通过 HTTP JSON 接口，查询结果以 JSON 文档格式发送。示例：

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

* `took`：执行搜索所用时间，单位为毫秒
* `timed_out`：查询是否超时
* `hits`：搜索结果，包含以下属性：
  - `total`：匹配文档的总数
  - `hits`：包含匹配项的数组

查询结果还可以包含查询配置文件信息。参见 [查询配置文件](../Node_info_and_management/Profiling/Query_profile.md)。

`hits` 数组中的每个匹配项具有以下属性：

* `_id`：匹配 ID
* `_score`：匹配权重，由排序器计算得出
* `_source`：包含此匹配项属性的数组

### 源选择

默认情况下，所有属性都返回在 `_source` 数组中。您可以在请求负载中使用 `_source` 属性选择要包含在结果集中的字段。示例：

```json
{
  "table":"test",
  "_source":"attr*",
  "query": { "match_all": {} }
}
```

您可以将要包含在查询结果中的属性指定为字符串 (`"_source": "attr*"`) 或字符串数组 (`"_source": [ "attr1", "attri*" ]`)。每个条目可以是属性名称或通配符（支持 `*`、`%` 和 `?` 符号）。

您还可以使用 `includes` 和 `excludes` 属性显式指定您希望包含或从结果集中排除的属性：

```json
"_source":
{
  "includes": [ "attr1", "attri*" ],
  "excludes": [ "*desc*" ]
}
```

空的 includes 列表被解释为“包含所有属性”，而空的 excludes 列表不会匹配任何内容。如果一个属性同时匹配 includes 和 excludes，则以 excludes 为准。

<!-- proofread -->




