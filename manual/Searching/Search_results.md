# Search results

## SQL

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

Query result can also include query profile information, see [Query profile](Profiling_and_monitoring/Profiling/Query_profile.md).

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





