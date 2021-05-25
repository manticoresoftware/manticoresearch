# String functions

### CONCAT()
Concatenates two or more strings into one. Non-string arguments must be explicitly converted to string using `TO_STRING()` function

```sql
CONCAT(TO_STRING(float_attr), ',', TO_STRING(int_attr), ',', title)
```

### LEVENSHTEIN()
`LEVENSHTEIN ( pattern, source, {normalize=0, length_delta=0})` returns number (Levenshtein distance) of single-character edits (insertions, deletions or substitutions) between `pattern` and `source` strings required to make in `pattern` to make it `source`.

   * `pattern`, `source` - constant string, string field name, JSON field name or any expression which produces a string (like e.g. [SUBSTRING_INDEX()](../Functions/String_functions.md#SUBSTRING_INDEX%28%29))
   * `normalize` - option to get distance as a float number in the range of `[0.0 - 1.0]`, where 0.0 is the exact match and 1.0 is the maximum difference. Default value is 0, means to not normalize and provide the result as integer.
   * `length_delta` - to skip Levenshtein distance calculation and return `max(strlen(pattern), strlen(source))` in case the option is set and lengths of the strings differ more than by `length_delta` value. Default value is 0, means to calculate Levenshtein distance for any input strings. This option could be useful in case of checking mostly similar strings.

``` sql
SELECT LEVENSHTEIN('gily', attr1) AS dist, WEIGHT() AS w FROM test WHERE MATCH('test') ORDER BY w DESC, dist ASC;
SELECT LEVENSHTEIN('gily', j.name, {length_delta=6}) AS dist, WEIGHT() AS w FROM test WHERE MATCH('test') ORDER BY w DESC;
SELECT LEVENSHTEIN(title, j.name, {normalize=1}) AS dist, WEIGHT() AS w FROM test WHERE MATCH ('test') ORDER BY w DESC, dist ASC;
```

### REGEX()
`REGEX(attr,expr)` function returns 1 if regular expression matched to string of attribute and 0 otherwise. It works with both string and JSON attributes.

```sql
SELECT REGEX(content, 'box?') FROM test;
SELECT REGEX(j.color, 'red | pink') FROM test;
```

### SNIPPET()
`SNIPPET()` can be used to highlight search results in a given text. The first two arguments are: the text to highlight, and a query. It's possible to pass [options](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#options) to function as third, fourth and so on arguments. `SNIPPET()` can fetch the text to use in highlighting from index itself. First argument in this case is field name:

```sql         
SELECT SNIPPET(body,QUERY()) FROM myIndex WHERE MATCH('my.query')   
```

`QUERY()` expression in this example returns the current fulltext query. `SNIPPET()` can also highlight non-indexed text:

```sql
mysql  SELECT id, SNIPPET('text to highlight', 'my.query', 'limit=100') FROM myIndex WHERE MATCH('my.query')
```

It can also be used to highlight text fetched from other sources using an UDF:

```sql
SELECT id, SNIPPET(myUdf(id), 'my.query', 'limit=100') FROM myIndex WHERE MATCH('my.query')
```

where `myUdf()` would be a UDF that fetches a document by its ID from some external storage. This enables applications to fetch the entire result set directly from Manticore in one query, without having to separately fetch the documents in the application and then send them back to Manticore for highlighting. `SNIPPET()` is a so-called "post limit" function, meaning that computing snippets is postponed not just until the entire final result set is ready, but even after the `LIMIT` clause is applied. For example, with a `LIMIT 20,10` clause, `SNIPPET()` will be called at most 10 times.  

### SUBSTRING_INDEX()
`SUBSTRING_INDEX(string, delimiter, number)` returns a substring of a string before a specified number of delimiter occurs

   *   string - The original string. Can be a constant string or a string from a string/json attribute.
   *   delimiter - The delimiter to search for
   *   number - The number of times to search for the delimiter. Can be both a positive or negative number.If it is a positive number, this function will return all to the left of the delimiter. If it is a negative number, this function will return all to the right of the delimiter.

```sql
SELECT SUBSTRING_INDEX('www.w3schools.com', '.', 2) FROM test;
SELECT SUBSTRING_INDEX(j.coord, ' ', 1) FROM test;
```
