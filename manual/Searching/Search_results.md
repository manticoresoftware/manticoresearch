# Search results

## SQL

<!-- example sql1 -->
When you run a query via SQL over mysql protocol as a result you get the requested columns back or empty result set in case nothing is found.  

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
In addition to that you can use [SHOW META](../Node_info_and_management/SHOW_META.md) call to see additional meta-information about the latest query. 

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

<!-- example sql3 -->
In some cases, e.g. when you do [faceted search](../Searching/Faceted_search.md) you can get multiple result sets as a response to your SQL query. 

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
In case of a warning the result set will include a warning flag and you can see the warning using [SHOW WARNINGS](../Node_info_and_management/SHOW_WARNINGS.md).
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
If your query fails you will get an error:

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

Via HTTP JSON iterface query result is sent as a JSON document. Example:

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
        "_id": "1",
        "_score": 1,
        "_source": { "gid": 11 }
      },
      {
        "_id": "2",
        "_score": 1,
        "_source": { "gid": 12 }
      }
    ]
  }
}
```

* `took`: time in milliseconds it took to execute the search
* `timed_out`: if the query timed out or not
* `hits`: search results. has the following properties:
* `total`: total number of matching documents
* `hits`: an array containing matches

Query result can also include query profile information, see [Query profile](../Node_info_and_management/Profiling/Query_profile.md).

Each match in the `hits` array has the following properties:

* `_id`: match id
* `_score`: match weight, calculated by ranker
* `_source`: an array containing the attributes of this match

### Source selection

By default all attributes are returned in the `_source` array. You can use the `_source` property in the request payload to select the fields you want to be included in the result set. Example:

```json
{
  "index":"test",
  "_source":"attr*",
  "query": { "match_all": {} }
}
```

You can specify the attributes which you want to include in the query result as a string (`"_source": "attr*"`) or as an array of strings (`"_source": [ "attr1", "attri*" ]"`). Each entry can be an attribute name or a wildcard (`*`, `%` and `?` symbols are supported).

You can also explicitly specify which attributes you want to include and which to exclude from the result set using the `includes` and `excludes` properties:

```json
"_source":
{
  "includes": [ "attr1", "attri*" ],
  "excludes": [ "*desc*" ]
}
```

An empty list of includes is interpreted as “include all attributes” while an empty list of excludes does not match anything. If an attribute matches both the includes and excludes, then the excludes win.





