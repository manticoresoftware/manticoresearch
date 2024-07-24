# Fuzzy Search in Manticore Search

The Fuzzy Search feature allows for more flexible matching by accounting for slight variations or misspellings in the search query. It works similarly to a normal SELECT statement but exposes additional parameters to control the fuzzy matching behavior.

## SQL

<!-- example Fuzzy_Search_1 -->

```sql
SELECT * FROM mytable WHERE MATCH('something') OPTION fuzzy=1;
```

The `OPTION fuzzy=1` clause enables fuzzy search. Additional options can be used to fine-tune the fuzzy matching:

- `layouts='ru,en'`: Specifies keyboard layouts to check for misspellings. Use an empty string ('') to disable this feature.
- `distance=2`: Controls the fuzziness by setting the Levenshtein distance to use in matching. Default is 2.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT * FROM mytable WHERE MATCH('someting') OPTION fuzzy=1, layouts='en', distance=2;
```
<!-- response SQL -->

```sql
+------+-------------+
| id   | content     |
+------+-------------+
|    1 | something   |
|    2 | some thing  |
+------+-------------+
2 rows in set (0.00 sec)
```

<!-- request Fuzzy Search with additional filters -->
An example of a more complex query using Fuzzy Search with WHERE filters:

```sql
SELECT * FROM mytable WHERE MATCH('someting') OPTION fuzzy=1 AND (category='books' AND price < 20);
```

<!-- end -->

## HTTP JSON

<!-- example Fuzzy_Search_11 -->

Fuzzy Search is also available in the `/search` endpoint and in HTTP-based clients. You can include the fuzzy search options in your JSON query:

```json
{
  "index": "mytable",
  "query": {
    "match": {
      "*": "someting"
    }
  },
  "options": {
    "fuzzy": true,
	"layouts": ["en", "ru"],
    "distance": 2
  }
}
```

In this example, fuzzy search is enabled with the Russian and English keyboard layout and a Levenshtein distance of 2.

You also can use more complex queries, for example:

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

Note: Currently, the SQL version has its own limitations and does not support operators in the MATCH clause. The MATCH should be simple and contain only the words you want to match.

