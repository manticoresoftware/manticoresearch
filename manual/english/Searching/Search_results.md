# Search results

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
When you run a query via SQL over the MySQL protocol, you receive the requested columns as a result or an empty result set if nothing is found.

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
Additionally, you can use the [SHOW META](../Node_info_and_management/SHOW_META.md) call to see extra meta-information about the latest query.

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
In some cases, such as when performing a [faceted search](../Searching/Faceted_search.md), you may receive multiple result sets as a response to your SQL query.

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
In case of a warning, the result set will include a warning flag, and you can see the warning using [SHOW WARNINGS](../Node_info_and_management/SHOW_WARNINGS.md).
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
data for the following example:

DROP TABLE IF EXISTS t;
CREATE TABLE t(name text);
INSERT INTO t(name) VALUES
('joe');
-->

<!-- example sql5 -->
If your query fails, you will receive an error:

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

Via the HTTP JSON interface, the query result is sent as a JSON document. Example:

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

* `took`: time in milliseconds it took to execute the search
* `timed_out`: whether the query timed out or not
* `hits`: search results, with the following properties:
  - `total`: total number of matching documents
  - `hits`: an array containing matches

The query result can also include query profile information. See [Query profile](../Node_info_and_management/Profiling/Query_profile.md).

Each match in the `hits` array has the following properties:

* `_id`: match id
* `_score`: match weight, calculated by the ranker
* `_source`: an array containing the attributes of this match

### Source selection

By default, all attributes are returned in the `_source` array. You can use the `_source` property in the request payload to select the fields you want to include in the result set. Example:

```json
{
  "table":"test",
  "_source":"attr*",
  "query": { "match_all": {} }
}
```

You can specify the attributes you want to include in the query result as a string (`"_source": "attr*"`) or as an array of strings (`"_source": [ "attr1", "attri*" ]"`). Each entry can be an attribute name or a wildcard (`*`, `%` and `?` symbols are supported).

You can also explicitly specify which attributes you want to include and which to exclude from the result set using the `includes` and `excludes` properties:

```json
"_source":
{
  "includes": [ "attr1", "attri*" ],
  "excludes": [ "*desc*" ]
}
```

An empty list of includes is interpreted as "include all attributes," while an empty list of excludes does not match anything. If an attribute matches both the includes and excludes, then the excludes win.

<!-- proofread -->




