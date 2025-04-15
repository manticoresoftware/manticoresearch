# 搜索结果

## SQL

<!-- example sql1 -->
当您通过 SQL 在 MySQL 协议上运行查询时，您将作为结果接收请求的列，或者如果未找到任何结果，则接收一个空的结果集。

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
<!-- end -->

<!-- example sql2 -->
此外，您可以使用 [SHOW META](../Node_info_and_management/SHOW_META.md) 调用查看有关最新查询的额外元信息。

<!-- request SQL -->
```sql
SELECT id,story_author,comment_author FROM hn_small WHERE story_author='joe' LIMIT 3; SHOW META;
```

<!-- response SQL -->
```sql
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
<!-- end -->

<!-- example sql4 -->
如果出现警告，结果集将包含一个警告标志，您可以使用 [SHOW WARNINGS](../Node_info_and_management/SHOW_WARNINGS.md) 查看警告。
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
如果您的查询失败，您将收到一个错误：

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

通过 HTTP JSON 接口，查询结果作为 JSON 文档发送。示例：

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

* `took`: 执行搜索所花费的时间（毫秒）
* `timed_out`: 查询是否超时
* `hits`: 搜索结果，具有以下属性：
  - `total`: 匹配文档的总数
  - `hits`: 包含匹配内容的数组

查询结果还可以包括查询概要信息。请参见 [查询概要](../Node_info_and_management/Profiling/Query_profile.md)。

`hits` 数组中的每个匹配项具有以下属性：

* `_id`: 匹配 ID
* `_score`: 匹配权重，由排名器计算
* `_source`: 包含该匹配属性的数组

### 源选择

默认情况下，所有属性都返回在 `_source` 数组中。您可以在请求有效载荷中使用 `_source` 属性来选择要包含在结果集中的字段。示例：

```json
{
  "table":"test",
  "_source":"attr*",
  "query": { "match_all": {} }
}
```

您可以将要在查询结果中包含的属性指定为字符串（`"_source": "attr*"`）或字符串数组（`"_source": [ "attr1", "attri*" ]`）。每个条目可以是属性名称或通配符（支持 `*`、`%` 和 `?` 符号）。

您还可以显式指定要包含和排除的属性使用 `includes` 和 `excludes` 属性：

```json
"_source":
{
  "includes": [ "attr1", "attri*" ],
  "excludes": [ "*desc*" ]
}
```

包含的空列表被解释为“包含所有属性”，而排除的空列表不匹配任何内容。如果某个属性同时匹配包括和排除，则排除优先。

<!-- proofread -->



