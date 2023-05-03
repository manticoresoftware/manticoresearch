# String functions

### CONCAT()
Concatenates two or more strings into one. Non-string arguments must be explicitly converted to string using the `TO_STRING()` function.

```sql
CONCAT(TO_STRING(float_attr), ',', TO_STRING(int_attr), ',', title)
```

### LEVENSHTEIN()
`LEVENSHTEIN ( pattern, source, {normalize=0, length_delta=0})` returns number (Levenshtein distance) of single-character edits (insertions, deletions or substitutions) between `pattern` and `source` strings required to make in `pattern` to make it `source`.

   * `pattern`, `source` - constant string, string field name, JSON field name, or any expression that produces a string (like e.g., [SUBSTRING_INDEX()](../Functions/String_functions.md#SUBSTRING_INDEX%28%29))
   * `normalize` - option to return the distance as a float number in the range `[0.0 - 1.0]`, where 0.0 is an exact match, and 1.0 is the maximum difference. The default value is 0, meaning not to normalize and provide the result as an integer.
   * `length_delta` - skips Levenshtein distance calculation and returns `max(strlen(pattern), strlen(source))` if the option is set and the lengths of the strings differ by more than the `length_delta` value. The default value is 0, meaning to calculate Levenshtein distance for any input strings. This option can be useful when checking mostly similar strings.

``` sql
SELECT LEVENSHTEIN('gily', attr1) AS dist, WEIGHT() AS w FROM test WHERE MATCH('test') ORDER BY w DESC, dist ASC;
SELECT LEVENSHTEIN('gily', j.name, {length_delta=6}) AS dist, WEIGHT() AS w FROM test WHERE MATCH('test') ORDER BY w DESC;
SELECT LEVENSHTEIN(title, j.name, {normalize=1}) AS dist, WEIGHT() AS w FROM test WHERE MATCH ('test') ORDER BY w DESC, dist ASC;
```

### REGEX()
The `REGEX(attr,expr)` function returns 1 if a regular expression matches the attribute's string, and 0 otherwise. It works with both string and JSON attributes.

```sql
SELECT REGEX(content, 'box?') FROM test;
SELECT REGEX(j.color, 'red | pink') FROM test;
```

Expressions should adhere to the RE2 syntax. To perform a case-insensitive search, for instance, you can use:
```sql
SELECT REGEX(content, '(?i)box') FROM test;
```

### SNIPPET()
The `SNIPPET()` function can be used to highlight search results within a given text. The first two arguments are: the text to be highlighted, and a query. [Options](../Searching/Highlighting.md#Highlighting-options) can be passed to the function as the third, fourth, and so on arguments. `SNIPPET()` can obtain the text for highlighting directly from the table. In this case, the first argument should be the field name:

```sql 
SELECT SNIPPET(body, QUERY()) FROM myIndex WHERE MATCH('my.query') 
```

In this example, the `QUERY()`  expression returns the current full-text query. `SNIPPET()` can also highlight non-indexed text:

```sql
mysql  SELECT id, SNIPPET('text to highlight', 'my.query', 'limit=100') FROM myIndex WHERE MATCH('my.query')
```

Additionally, it can be used to highlight text fetched from other sources using a User-Defined Function (UDF):

```sql
SELECT id, SNIPPET(myUdf(id), 'my.query', 'limit=100') FROM myIndex WHERE MATCH('my.query')
```

In this context, `myUdf()` is a User-Defined Function (UDF) that retrieves a document by its ID from an external storage source. The `SNIPPET()` function is considered a "post limit" function, which means that the computation of snippets is delayed until the entire final result set is prepared, and even after the `LIMIT` clause has been applied. For instance, if a `LIMIT 20,10` clause is used, `SNIPPET()` will be called no more than 10 times.

It is important to note that `SNIPPET()` does not support field-based limitations. For this functionality, use [HIGHLIGHT()](../Searching/Highlighting.md#Highlighting-via-SQL) instead.

### SUBSTRING_INDEX()
`SUBSTRING_INDEX(string, delimiter, number)` returns a substring of the original string, based on a specified number of delimiter occurrences:

   *   string - The original string, which can be a constant string or a string from a string/JSON attribute.
   *   delimiter - The delimiter to search for.
   *   number - The number of times to search for the delimiter. This can be either a positive or negative number. If it is a positive number, the function will return everything to the left of the delimiter. If it is a negative number, the function will return everything to the right of the delimiter.

```sql
SELECT SUBSTRING_INDEX('www.w3schools.com', '.', 2) FROM test;
SELECT SUBSTRING_INDEX(j.coord, ' ', 1) FROM test;
```
<!-- proofread -->
