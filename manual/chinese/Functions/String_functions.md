# 字符串函数

### CONCAT()
将两个或多个字符串连接成一个。非字符串参数必须使用`TO_STRING()`函数显式转换为字符串。

```sql
CONCAT(TO_STRING(float_attr), ',', TO_STRING(int_attr), ',', title)
```

### LEVENSHTEIN()
`LEVENSHTEIN ( pattern, source, {normalize=0, length_delta=0})` 返回需要在 `pattern` 中进行的单字符编辑（插入、删除或替换）的数量（Levenshtein距离），以使其变为`source`字符串。

   * `pattern`，`source` - 常量字符串、字符串字段名、JSON字段名或任何产生字符串的表达式（例如 [SUBSTRING_INDEX()](../Functions/String_functions.md#SUBSTRING_INDEX%28%29)）
   * `normalize` - 选项以将距离返回为 `[0.0 - 1.0]` 范围内的浮点数，其中 0.0 是完全匹配，1.0 是最大差异。默认值为 0，意味着不进行归一化并提供结果为整数。
   * `length_delta` - 跳过 Levenshtein 距离计算，并在选项设置且字符串长度相差超过 `length_delta` 的值时返回 `max(strlen(pattern), strlen(source))`。默认值为 0，意味着对任何输入字符串计算 Levenshtein 距离。此选项在检查大致相似的字符串时非常有用。

``` sql
SELECT LEVENSHTEIN('gily', attr1) AS dist, WEIGHT() AS w FROM test WHERE MATCH('test') ORDER BY w DESC, dist ASC;
SELECT LEVENSHTEIN('gily', j.name, {length_delta=6}) AS dist, WEIGHT() AS w FROM test WHERE MATCH('test') ORDER BY w DESC;
SELECT LEVENSHTEIN(title, j.name, {normalize=1}) AS dist, WEIGHT() AS w FROM test WHERE MATCH ('test') ORDER BY w DESC, dist ASC;
```

### REGEX()
`REGEX(attr,expr)` 函数如果正则表达式匹配属性的字符串则返回 1，否则返回 0。它适用于字符串和 JSON 属性。

```sql
SELECT REGEX(content, 'box?') FROM test;
SELECT REGEX(j.color, 'red | pink') FROM test;
```

表达式应遵循 RE2 语法。例如，要进行不区分大小写的搜索，可以使用：
```sql
SELECT REGEX(content, '(?i)box') FROM test;
```

### SNIPPET()
`SNIPPET()` 函数可用于在给定文本中突出显示搜索结果。前两个参数是：要突出显示的文本和查询。[选项](../Searching/Highlighting.md#Highlighting-options)可以作为第三个、第四个及其他参数传递给该函数。`SNIPPET()` 可以直接从表中获取要突出显示的文本。在这种情况下，第一个参数应为字段名称：

```sql
SELECT SNIPPET(body, QUERY()) FROM myIndex WHERE MATCH('my.query')
```

在这个示例中，`QUERY()` 表达式返回当前的全文查询。`SNIPPET()` 还可以突出显示未索引的文本：

```sql
mysql  SELECT id, SNIPPET('text to highlight', 'my.query', 'limit=100') FROM myIndex WHERE MATCH('my.query')
```

此外，它还可以用于使用用户定义函数（UDF）从其他来源获取文本并突出显示：

```sql
SELECT id, SNIPPET(myUdf(id), 'my.query', 'limit=100') FROM myIndex WHERE MATCH('my.query')
```

在这个上下文中，`myUdf()` 是一个用户定义函数（UDF），用于通过其 ID 从外部存储源检索文档。`SNIPPET()` 函数被视为“后限制”函数，这意味着在准备整个最终结果集之前，片段的计算是延迟的，甚至在应用 `LIMIT` 子句之后。例如，如果使用 `LIMIT 20,10` 子句，则`SNIPPET()` 最多将调用 10 次。

重要的是要注意，`SNIPPET()` 不支持基于字段的限制。对于此功能，请改用 [HIGHLIGHT()](../Searching/Highlighting.md#Highlighting-via-SQL)。

### SUBSTRING_INDEX()

<!-- example substring_index -->
`SUBSTRING_INDEX(string, delimiter, number)` 返回原始字符串的子字符串，根据指定的分隔符出现的次数：

   *   string - 原始字符串，可以是常量字符串或来自字符串/JSON 属性的字符串。
   *   delimiter - 要搜索的分隔符。
   *   number - 搜索分隔符的次数。这可以是正数或负数。如果是正数，则函数将返回分隔符左侧的所有内容。如果是负数，则函数将返回分隔符右侧的所有内容。

`SUBSTRING_INDEX()` 默认返回字符串，但在必要时也可以强制转换为其他类型（例如整数或浮点数）。数值可以使用特定函数（如 `BIGINT()`、`DOUBLE()` 等）进行转换。

<!-- request SQL -->
```sql
SELECT SUBSTRING_INDEX('www.w3schools.com', '.', 2) FROM test;
SELECT SUBSTRING_INDEX(j.coord, ' ', 1) FROM test;
SELECT          SUBSTRING_INDEX('1.2 3.4', ' ',  1);  /* '1.2' */
SELECT          SUBSTRING_INDEX('1.2 3.4', ' ', -1);  /* '3.4' */
SELECT sint (   SUBSTRING_INDEX('1.2 3.4', ' ',  1)); /* 1 */
SELECT sint (   SUBSTRING_INDEX('1.2 3.4', ' ', -1)); /* 3 */
SELECT double ( SUBSTRING_INDEX('1.2 3.4', ' ',  1)); /* 1.200000 */
SELECT double ( SUBSTRING_INDEX('1.2 3.4', ' ', -1)); /* 3.400000 */
```

<!-- end -->

### UPPER() 和 LOWER()

`UPPER(string)` 将参数转换为大写，`LOWER(string)` 将参数转换为小写。

结果也可以提升为数值，但仅当字符串参数可转换为数字时。数值可以通过任意函数（`BITINT`、`DOUBLE`等）提升。

```sql
SELECT upper('www.w3schools.com', '.', 2); /* WWW.W3SCHOOLS.COM  */
SELECT double (upper ('1.2e3')); /* 1200.000000 */
SELECT integer (lower ('12345')); /* 12345 */
```
<!-- proofread -->
