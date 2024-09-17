# Fuzzy Search

The Fuzzy Search feature allows for more flexible matching by accounting for slight variations or misspellings in the search query. It works similarly to a normal `SELECT` SQL statement or a `/search` JSON request but provides additional parameters to control the fuzzy matching behavior.

## General syntax

### SQL

<!-- example Fuzzy_Search_SQL -->

```sql
SELECT
  ...
  MATCH('...')
  ...
  OPTION fuzzy={0|1}
  [, distance=N]
  [, layouts='{be,bg,br,ch,de,dk,es,fr,uk,gr,it,no,pt,ru,se,ua,us}']
}
```

The `OPTION fuzzy=1` clause turns on fuzzy search. You can fine-tune the fuzzy matching with additional options:

- `distance=2`: Sets the Levenshtein distance for matching. The default is 2.
- `layouts='layout1,layout2'`: Specifies keyboard layouts to check for typing errors. By default, all layouts are enabled. Use an empty string (`''`) to disable this feature. Supported layouts include: `be`, `bg`, `br`, `ch`, `de`, `dk`, `es`, `fr`, `uk`, `gr`, `it`, `no`, `pt`, `ru`, `se`, `ua`, `us`.

Note: The SQL version currently has limitations and does not support operators in the MATCH clause. The MATCH should be simple and contain only the words you want to match.

<!-- intro -->
##### SQL:

<!-- request Example -->

```sql
SELECT * FROM mytable WHERE MATCH('someting') OPTION fuzzy=1, layouts='en', distance=2;
```
<!-- response Example -->

```sql
+------+-------------+
| id   | content     |
+------+-------------+
|    1 | something   |
|    2 | some thing  |
+------+-------------+
2 rows in set (0.00 sec)
```

<!-- request With additional filters -->
Example of a more complex Fuzzy search query with additional filters:

```sql
SELECT * FROM mytable WHERE MATCH('someting') OPTION fuzzy=1 AND (category='books' AND price < 20);
```

<!-- end -->

### JSON

<!-- example Fuzzy_Search_JSON -->

```json
POST /search
{
  "index": "table_name",
  "query": {
    <full-text query>
  },
  "options": {
    "fuzzy": {true|false}
    [,"layouts": ["be","bg","br","ch","de","dk","es","fr","uk","gr","it","no","pt","ru","se","ua","us"]]
    [,"distance": N]
  }
}
```

The options object lets you turn on fuzzy search and adjust its settings:

- `"fuzzy": true|false`: Turn fuzzy search on or off.
- `"distance": N`: Set the Levenshtein distance for matching. The default is 2.
- `"layouts": ["layout1", "layout2", ...]`: Choose keyboard layouts to check for typing errors. All layouts are used by default. Use an empty array (`[]`) to turn this off.

<!-- request Example -->

```json
{
  "index": "test",
  "query": {
    "bool": {
      "must": [
        {
          "match": {
            "*": "собака"
          }
        },
        {
          "match": {
            "*": "ghbdtn"
          }
        }
      ]
    }
  },
  "options": {
    "fuzzy": true,
	"layouts": ["en", "ru"],
    "distance": 2
  }
}
```
<!-- end -->
